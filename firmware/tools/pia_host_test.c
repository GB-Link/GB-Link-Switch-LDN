/* Host test: the Pia host (pia_host.c, board leads the room) against the firmware's
   joiner (pia_link.c, pia_conn.c, pia_reliable.c, pia_crypto.c) over an in-memory
   network with loss and reordering, both ticking at 59.727 Hz.
   Checks: the joiner gets the status update, joins, connects, sends WC and gets WA.
   Under 5% loss each way, child frames reach the host in order, and parent frames reach
   the joiner in order in the expected WT wrapper or are counted as dropped by the host's
   pacing. Then teardown. Host datagrams are decrypted and checked against the layouts a
   Switch host sends.
   A group scenario has three joiners in one room: each hears the others in the station
   lists and probes them, connects one at a time, gets its own slot, and the GBA's frames
   reach all of them; one is dropped and the other two carry on.

   PSA crypto calls from pia_crypto.c use OpenSSL; the vendored zstd decoder is built in.
   Build and run from the firmware directory:

       cc -std=c11 -Wall -Wextra -I tools/host_stubs -I common -I common/components/zstd/lib \
           -DZSTD_DISABLE_ASM=1 -DZSTD_MULTITHREAD=0 -DZSTD_DECODER_INTERNAL_BUFFER=8192 \
           tools/pia_host_test.c common/pia_host.c common/pia_link.c common/pia_conn.c \
           common/pia_reliable.c common/pia_crypto.c \
           common/components/zstd/lib/common/[a-z]*.c common/components/zstd/lib/decompress/[a-z]*.c \
           -lcrypto -o /tmp/pia_host_test && /tmp/pia_host_test
*/

#include <openssl/evp.h>
#include <openssl/params.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psa/crypto.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "pia_host.h"
#include "pia_link.h"

/* ---- PSA over OpenSSL ------------------------------------------------------------ */

static struct { bool used; uint8_t data[64]; size_t len; } g_keys[16];

psa_status_t psa_crypto_init(void) { return PSA_SUCCESS; }

psa_status_t psa_import_key(const psa_key_attributes_t *attributes, const uint8_t *data, size_t length,
                            mbedtls_svc_key_id_t *key)
{
    (void)attributes;
    if (length > sizeof(g_keys[0].data)) return -1;
    for (size_t i = 0; i < sizeof(g_keys) / sizeof(g_keys[0]); ++i)
        if (!g_keys[i].used)
        {
            g_keys[i].used = true;
            memcpy(g_keys[i].data, data, length);
            g_keys[i].len = length;
            *key = (mbedtls_svc_key_id_t)(i + 1);
            return PSA_SUCCESS;
        }
    return -1;
}

psa_status_t psa_destroy_key(mbedtls_svc_key_id_t key)
{
    if (key == 0 || key > sizeof(g_keys) / sizeof(g_keys[0])) return -1;
    g_keys[key - 1].used = false;
    return PSA_SUCCESS;
}

static const uint8_t *key_of(mbedtls_svc_key_id_t key, size_t *len)
{
    if (key == 0 || key > sizeof(g_keys) / sizeof(g_keys[0]) || !g_keys[key - 1].used) return NULL;
    *len = g_keys[key - 1].len;
    return g_keys[key - 1].data;
}

static psa_status_t ecb(mbedtls_svc_key_id_t key, const uint8_t *in, size_t in_len, uint8_t *out,
                        size_t out_size, size_t *out_len, bool encrypt)
{
    size_t klen;
    const uint8_t *k = key_of(key, &klen);
    if (!k || klen != 16 || in_len != 16 || out_size < 16) return -1;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, fin = 0;
    EVP_CipherInit_ex(ctx, EVP_aes_128_ecb(), NULL, k, NULL, encrypt ? 1 : 0);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    bool ok = EVP_CipherUpdate(ctx, out, &len, in, 16) && EVP_CipherFinal_ex(ctx, out + len, &fin);
    EVP_CIPHER_CTX_free(ctx);
    *out_len = (size_t)(len + fin);
    return ok ? PSA_SUCCESS : -1;
}

psa_status_t psa_cipher_encrypt(mbedtls_svc_key_id_t key, psa_algorithm_t alg, const uint8_t *input,
                                size_t input_length, uint8_t *output, size_t output_size, size_t *output_length)
{ (void)alg; return ecb(key, input, input_length, output, output_size, output_length, true); }

psa_status_t psa_cipher_decrypt(mbedtls_svc_key_id_t key, psa_algorithm_t alg, const uint8_t *input,
                                size_t input_length, uint8_t *output, size_t output_size, size_t *output_length)
{ (void)alg; return ecb(key, input, input_length, output, output_size, output_length, false); }

static bool gcm(const uint8_t *k, const uint8_t *nonce, size_t nonce_len, const uint8_t *aad, size_t aad_len,
                const uint8_t *in, size_t in_len, uint8_t *out, uint8_t *tag, size_t tag_len, bool encrypt)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    bool ok = false;
    int len = 0, tmp = 0, fin = 0;
    if (EVP_CipherInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL, encrypt ? 1 : 0) &&
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)nonce_len, NULL) &&
        EVP_CipherInit_ex(ctx, NULL, NULL, k, nonce, encrypt ? 1 : 0))
    {
        if (aad_len) EVP_CipherUpdate(ctx, NULL, &tmp, aad, (int)aad_len);
        if (!encrypt) EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)tag_len, tag);
        if (EVP_CipherUpdate(ctx, out, &len, in, (int)in_len) && EVP_CipherFinal_ex(ctx, out + len, &fin))
            ok = encrypt ? EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)tag_len, tag) == 1 : true;
    }
    EVP_CIPHER_CTX_free(ctx);
    return ok;
}

psa_status_t psa_aead_encrypt(mbedtls_svc_key_id_t key, psa_algorithm_t alg, const uint8_t *nonce,
                              size_t nonce_length, const uint8_t *additional_data, size_t additional_data_length,
                              const uint8_t *plaintext, size_t plaintext_length, uint8_t *ciphertext,
                              size_t ciphertext_size, size_t *ciphertext_length)
{
    size_t klen, tag = PSA_ALG_AEAD_TAG_LENGTH(alg);
    const uint8_t *k = key_of(key, &klen);
    if (!k || klen != 16 || ciphertext_size < plaintext_length + tag) return -1;
    if (!gcm(k, nonce, nonce_length, additional_data, additional_data_length, plaintext, plaintext_length,
             ciphertext, ciphertext + plaintext_length, tag, true))
        return -1;
    *ciphertext_length = plaintext_length + tag;
    return PSA_SUCCESS;
}

psa_status_t psa_aead_decrypt(mbedtls_svc_key_id_t key, psa_algorithm_t alg, const uint8_t *nonce,
                              size_t nonce_length, const uint8_t *additional_data, size_t additional_data_length,
                              const uint8_t *ciphertext, size_t ciphertext_length, uint8_t *plaintext,
                              size_t plaintext_size, size_t *plaintext_length)
{
    size_t klen, tag = PSA_ALG_AEAD_TAG_LENGTH(alg);
    const uint8_t *k = key_of(key, &klen);
    if (!k || klen != 16 || ciphertext_length < tag || plaintext_size < ciphertext_length - tag) return -1;
    uint8_t t[16];
    memcpy(t, ciphertext + ciphertext_length - tag, tag);
    if (!gcm(k, nonce, nonce_length, additional_data, additional_data_length, ciphertext,
             ciphertext_length - tag, plaintext, t, tag, false))
        return -1;
    *plaintext_length = ciphertext_length - tag;
    return PSA_SUCCESS;
}

psa_status_t psa_hash_compute(psa_algorithm_t alg, const uint8_t *input, size_t input_length, uint8_t *hash,
                              size_t hash_size, size_t *hash_length)
{
    (void)alg;
    if (hash_size < 32) return -1;
    unsigned int n = 0;
    EVP_Digest(input, input_length, hash, &n, EVP_sha256(), NULL);
    *hash_length = n;
    return PSA_SUCCESS;
}

psa_status_t psa_mac_compute(mbedtls_svc_key_id_t key, psa_algorithm_t alg, const uint8_t *input,
                             size_t input_length, uint8_t *mac, size_t mac_size, size_t *mac_length)
{
    (void)alg;
    size_t klen;
    const uint8_t *k = key_of(key, &klen);
    if (!k || mac_size < 32) return -1;
    EVP_MAC *m = EVP_MAC_fetch(NULL, "HMAC", NULL);
    EVP_MAC_CTX *ctx = EVP_MAC_CTX_new(m);
    OSSL_PARAM params[] = {OSSL_PARAM_construct_utf8_string("digest", "SHA256", 0), OSSL_PARAM_construct_end()};
    EVP_MAC_init(ctx, k, klen, params);
    EVP_MAC_update(ctx, input, input_length);
    EVP_MAC_final(ctx, mac, mac_length, 32);
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(m);
    return PSA_SUCCESS;
}

/* ---- random and clock ------------------------------------------------------------ */

static uint32_t g_rng = 0x1234abcdu;
uint32_t esp_random(void) { g_rng = g_rng * 1664525u + 1013904223u; return g_rng; }
void esp_fill_random(void *buf, size_t len)
{
    uint8_t *p = buf;
    for (size_t i = 0; i < len; ++i) p[i] = (uint8_t)(esp_random() >> 16);
}
static double chance(void) { return (esp_random() >> 8) / 16777216.0; }

static int g_tick;
static int64_t g_now_us;
int64_t esp_timer_get_time(void) { return g_now_us; }

/* ---- network -------------------------------------------------------------------- */

#define HOST_IP "169.254.47.1"
#define JOINER_IP "169.254.47.2"
#define PIPE_SLOTS 512

typedef struct { uint8_t data[PIA_MAX_DATAGRAM]; size_t len; int due, order, from; bool used; } datagram_t;
typedef struct { datagram_t q[PIPE_SLOTS]; double loss, reorder; int sent, dropped, delayed, order; } pipe_t;
static pipe_t g_to_joiner, g_to_host;

static void pipe_put_from(pipe_t *p, const uint8_t *data, size_t len, int from)
{
    ++p->sent;
    if (chance() < p->loss) { ++p->dropped; return; }
    int due = g_tick + 1;
    if (chance() < p->reorder) { due += 2; ++p->delayed; }
    for (int i = 0; i < PIPE_SLOTS; ++i)
        if (!p->q[i].used)
        {
            memcpy(p->q[i].data, data, len);
            p->q[i].len = len;
            p->q[i].due = due;
            p->q[i].order = p->order++;
            p->q[i].from = from;
            p->q[i].used = true;
            return;
        }
    fprintf(stderr, "pipe full\n");
    exit(2);
}

static void pipe_put(pipe_t *p, const uint8_t *data, size_t len) { pipe_put_from(p, data, len, 0); }

static datagram_t *pipe_take(pipe_t *p)
{
    datagram_t *best = NULL;
    for (int i = 0; i < PIPE_SLOTS; ++i)
        if (p->q[i].used && p->q[i].due <= g_tick && (!best || p->q[i].order < best->order)) best = &p->q[i];
    if (best) best->used = false;
    return best;
}

/* ---- endpoints ------------------------------------------------------------------ */

static const uint8_t kSsid[16] = {0x4f, 0x3b, 0x51, 0x39, 0x85, 0xd8, 0x3e, 0xda, 0x42, 0x14, 0xc2, 0xa1, 0xe6, 0xd4, 0xea, 0x54};
static const uint8_t kHostMac[6] = {0xca, 0x0c, 0x06, 0xea, 0xf9, 0x74};
static const uint8_t kJoinerMac[6] = {0x8a, 0xca, 0xba, 0x4f, 0x18, 0x8f};
/* Room app data: member count at 22, host name at 26 (length, 1, text). */
static uint8_t g_app_data[122] = {
    0x00, 0x5c, 0x16, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x06, 0x01, 'A', 's', 'h', 't', 'o', 'n'};

static pia_host_t g_host;
static pia_link_t g_link;

static int g_failures;
#define CHECK(cond) do { \
    if (!(cond)) { printf("  FAIL %s (line %d, tick %d)\n", #cond, __LINE__, g_tick); ++g_failures; } \
} while (0)
#define CHECK_EQ(a, b) do { \
    long long va_ = (long long)(a), vb_ = (long long)(b); \
    if (va_ != vb_) { printf("  FAIL %s == %s: %lld vs %lld (line %d, tick %d)\n", #a, #b, va_, vb_, __LINE__, g_tick); ++g_failures; } \
} while (0)

/* Tally of decrypted host traffic. */
static struct
{
    int status, status_marked, status_cleared, property, kind5, kind2, kind9, kind13;
    int probes, replies, wa, wt, wt_idle, wg, wd, acks, retransmits;
    uint32_t wg_values[4];
    uint16_t wa_id;
    uint8_t wa_cid[2], wd_cid[2];
    uint32_t status_seq_first, status_seq_last, last_stamp;
    bool status_checked, kind5_checked, kind2_checked, property_checked, wa_checked, layout_ok;
    int wt_before_uni_wg;
    uint16_t seen[65536 / 16];
} g_h;

static void check_status(const uint8_t *p, size_t n, uint16_t src, bool est, uint16_t dst, uint16_t pkt, int footer, bool zip)
{
    CHECK_EQ(n, 162);
    CHECK(est && dst == 0 && pkt == 0 && footer == 0 && zip);
    CHECK(p[0] == 1 && p[1] == 0x11 && bin_b16(p + 2) == 132);
    uint32_t seq = bin_b32(p + 4);
    if (!g_h.status) g_h.status_seq_first = seq;
    g_h.status_seq_last = seq;
    CHECK_EQ(bin_b16(p + 8), g_host.our_id);
    CHECK(memcmp(p + 10, kHostMac, 6) == 0);
    static const uint8_t zero6[6];
    CHECK(memcmp(p + 16, zero6, 6) == 0);
    CHECK_EQ(bin_b32(p + 22), g_link.crypto.net_id);
    CHECK(p[26] == 1 && p[27] == 0 && p[28] == 6);
    const uint8_t *e0 = p + 30, *e1 = p + 52;
    static const uint8_t host_entry[22] = {0, 0, 0, 0, 169, 254, 47, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0x39};
    CHECK(memcmp(e0, host_entry, 22) == 0);
    if (p[29] == 0)
    {
        CHECK_EQ(src, g_host.our_id);
        static const uint8_t joiner_entry[22] = {0, 1, 0, 0, 169, 254, 47, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0x39};
        CHECK(memcmp(e1, joiner_entry, 22) == 0);
        ++g_h.status;
    }
    else
    {
        CHECK_EQ(src, 0);
        if (e1[1] == 0xff)
        {
            static const uint8_t empty[22] = {0, 0xff};
            CHECK(memcmp(e1, empty, 22) == 0);
            ++g_h.status_cleared;
        }
        else
        {
            static const uint8_t marked[22] = {0, 1, 0, 1, 169, 254, 47, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0x39};
            CHECK(memcmp(e1, marked, 22) == 0);
            ++g_h.status_marked;
        }
    }
    for (int i = 2; i < 6; ++i)
    {
        static const uint8_t empty[22] = {0, 0xff};
        CHECK(memcmp(p + 30 + 22 * i, empty, 22) == 0);
    }
    g_h.status_checked = true;
}

static void check_kind5(const uint8_t *p, size_t n, uint16_t dst, int footer_id, bool zip)
{
    ++g_h.kind5;
    CHECK_EQ(n, 192);
    CHECK(dst == 1 && footer_id == g_link.conn.our_id && zip);
    static const uint8_t head[7] = {5, 0, 0, 1, 0, 0, 3};
    CHECK(memcmp(p, head, 7) == 0);
    CHECK(memcmp(p + 7, kHostMac, 6) == 0 && p[13] == 0 && p[14] == 0);
    CHECK_EQ(bin_b16(p + 15), g_host.our_id);
    static const uint8_t mid[10] = {2, 0, 0, 1, 0, 0, 0, 0, 0, 0};
    CHECK(memcmp(p + 17, mid, 10) == 0);
    CHECK(memcmp(p + 27, kHostMac, 6) == 0);
    CHECK_EQ(bin_b16(p + 35), g_host.our_id);
    static const uint8_t host_addr[6] = {169, 254, 47, 1, 0x30, 0x39};
    CHECK(memcmp(p + 37, host_addr, 6) == 0);
    static const uint8_t host_tail[31] = {1, 1, 0, 0, 0x10, 0, 0, 0, 0, 0x38, 0xf9, 0x50, 0x7e, 0x55, 0xd0, 0x40,
                                          0x19, 0x13, 0xc5, 0x89, 0, 0, 0, 6, 1, 'A', 's', 'h', 't', 'o', 'n'};
    bool zeros = true;
    for (int i = 43; i < 80; ++i) zeros = zeros && p[i] == 0;
    CHECK(zeros);
    CHECK(memcmp(p + 80, host_tail, 31) == 0);
    CHECK(memcmp(p + 111, kJoinerMac, 6) == 0 && p[117] == 0 && p[118] == 0);
    CHECK_EQ(bin_b16(p + 119), g_link.conn.our_id);
    static const uint8_t joiner_addr[10] = {169, 254, 47, 2, 0x30, 0x39, 1, 0, 1, 0};
    CHECK(memcmp(p + 121, joiner_addr, 10) == 0);
    zeros = true;
    for (int i = 131; i < 164; ++i) zeros = zeros && p[i] == 0;
    CHECK(zeros);
    static const uint8_t joiner_tail[28] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 'E', 'M', 'U'};
    CHECK(memcmp(p + 164, joiner_tail, 28) == 0);
    g_h.kind5_checked = true;
}

static void check_kind2(const uint8_t *p, size_t n, uint16_t dst, int footer_id, bool zip)
{
    ++g_h.kind2;
    CHECK_EQ(n, 37);
    CHECK(dst == g_link.conn.our_id && footer_id == g_link.conn.our_id && !zip);
    static const uint8_t head[8] = {2, 0x0d, 7, 1, 0, 0, 0, 0};
    CHECK(memcmp(p, head, 8) == 0);
    CHECK(memcmp(p + 12, kHostMac, 6) == 0 && p[18] == 0 && p[19] == 0);
    CHECK_EQ(bin_b16(p + 20), g_host.our_id);
    CHECK(memcmp(p + 22, kJoinerMac, 6) == 0 && p[28] == 0 && p[29] == 0);
    CHECK_EQ(bin_b16(p + 30), g_link.conn.our_id);
    static const uint8_t tail[5] = {1, 0, 1, 0, 0};
    CHECK(memcmp(p + 32, tail, 5) == 0);
    g_h.kind2_checked = true;
}

static void check_property(const uint8_t *p, size_t n, bool est, uint16_t dst, uint16_t pkt, bool zip)
{
    ++g_h.property;
    CHECK_EQ(n, 160);
    CHECK(est && dst == 0 && pkt == 0 && zip);
    CHECK(p[0] == 1 && p[1] == 0x50 && bin_b16(p + 2) == 122 && bin_b32(p + 4) == 1 && bin_b32(p + 8) == 0);
    CHECK_EQ(bin_b32(p + 12), g_link.crypto.net_id);
    static const uint8_t words[22] = {0, 2, 0, 6, 0, 0, 0, 0, 0, 0, 0x57, 0x0f, 1, 1, 0, 0, 0, 0x5c, 0, 0, 0, 0x1e};
    CHECK(memcmp(p + 16, words, 22) == 0);
    CHECK(memcmp(p + 38, g_app_data, 122) == 0);
    g_h.property_checked = true;
}

static void check_reliable(const uint8_t *p, size_t n)
{
    CHECK(n >= 8 && bin_b16(p + 1) == n - 8 && p[7] == 0);
    if (n < 8) return;
    uint8_t flags = p[0];
    uint16_t seq = bin_b16(p + 3);
    const uint8_t *w = p + 8;
    size_t len = n - 8;
    if ((flags & 1) == 0) { ++g_h.acks; CHECK(len == 20 && w[0] == 0 && w[1] == 1); return; }
    CHECK(flags == 7 || flags == 15);
    /* Repeated seq = retransmission. */
    if (g_h.seen[seq >> 4] & (1u << (seq & 15))) { ++g_h.retransmits; return; }
    g_h.seen[seq >> 4] |= (uint16_t)(1u << (seq & 15));
    CHECK(len >= 4 && w[0] == 'W' && (size_t)bin_u16(w + 2) + 4 == len);
    if (len < 4) return;
    if (w[1] == 'A')
    {
        ++g_h.wa;
        CHECK(flags == 15 && seq == 0xfff0 && len == 10 && w[8] == 0 && w[9] == 0);
        g_h.wa_id = bin_u16(w + 4);
        memcpy(g_h.wa_cid, w + 6, 2);
        g_h.wa_checked = true;
    }
    else if (w[1] == 'T')
    {
        ++g_h.wt;
        CHECK(flags == 7 && len >= 12 && w[9] == 0 && w[10] == 0 && w[11] == 0);
        size_t data = w[8];
        if (data == 1) { ++g_h.wt_idle; CHECK_EQ(len, 12); }
        else CHECK_EQ(len, 12 + ((data + 3) & ~(size_t)3));
        uint32_t stamp = bin_u32(w + 4);
        if (g_h.wt > 1) CHECK(stamp > g_h.last_stamp);
        g_h.last_stamp = stamp;
        if (g_h.wg < 2) ++g_h.wt_before_uni_wg;
    }
    else if (w[1] == 'G')
    {
        CHECK(flags == 7 && len == 8);
        if (g_h.wg < 4) g_h.wg_values[g_h.wg] = bin_u32(w + 4);
        ++g_h.wg;
    }
    else if (w[1] == 'D')
    {
        ++g_h.wd;
        CHECK(flags == 7 && len == 6);
        memcpy(g_h.wd_cid, w + 4, 2);
    }
    else CHECK(!"unknown W frame from the host");
}

/* Decrypts each host datagram with the joiner's keys and checks its layout. */
static void inspect_host_datagram(const uint8_t *d, size_t n)
{
    static uint8_t plain[PIA_MAX_BODY], app[PIA_MAX_INFLATE];
    CHECK(n >= 29 && d[4] == 0x90 && (d[5] & 0x0c) == 0);
    int len = pia_decrypt(&g_link.crypto, d, n, HOST_IP, plain, sizeof(plain));
    CHECK(len >= 0);
    if (len < 0) return;
    int padding = d[5] >> 4, footer = d[12];
    bool zip = d[5] & 1, est = (d[5] & 2) != 0;
    uint16_t dst = bin_b16(d + 6), src = bin_b16(d + 8), pkt = bin_b16(d + 10);
    CHECK((len & 15) == 0 && padding + footer <= len && (footer == 0 || footer == 2));
    for (int i = 0; i < padding; ++i) CHECK(plain[len - 1 - i] == 0xff);
    len -= padding + footer;
    int footer_id = footer == 2 ? bin_b16(plain + len) : -1;
    CHECK(est == (dst == 0) && (footer == 2) == !est);
    if (footer == 2) CHECK_EQ(footer_id, g_link.conn.our_id);
    const uint8_t *body = plain;
    int size = len;
    if (zip)
    {
        size = pia_decompress(plain, (size_t)len, app, sizeof(app));
        CHECK(size >= 0);
        if (size < 0) return;
        body = app;
    }
    pia_message_iter_t it;
    pia_message_t m;
    pia_message_iter_init(&it, body, (size_t)size);
    int count = 0;
    while (pia_message_next(&it, &m))
    {
        ++count;
        const uint8_t *p = m.payload;
        if (m.protocol == 1)
        {
            CHECK(m.length >= 8 && p[0] == 1);
            if (p[1] == 0x11) check_status(p, m.length, src, est, dst, pkt, footer, zip);
            else if (p[1] == 0x50) check_property(p, m.length, est, dst, pkt, zip);
            else CHECK(!"unexpected net message type");
        }
        else if (m.protocol == 13)
        {
            CHECK(m.length >= 1);
            if (p[0] == 5) check_kind5(p, m.length, dst, footer_id, zip);
            else if (p[0] == 2) check_kind2(p, m.length, dst, footer_id, zip);
            else if (p[0] == 9)
            {
                ++g_h.kind9;
                CHECK(m.length == 30 && dst == g_link.conn.our_id && !est);
                CHECK(memcmp(p + 1, kHostMac, 6) == 0 && bin_b16(p + 9) == g_host.our_id && p[11] == 0);
                static const uint8_t addr[6] = {169, 254, 47, 1, 0x30, 0x39};
                CHECK(memcmp(p + 12, addr, 6) == 0);
                CHECK(memcmp(p + 18, kJoinerMac, 6) == 0 && bin_b16(p + 26) == g_link.conn.our_id && p[28] == 0 && p[29] == 0);
            }
            else if (p[0] == 13)
            {
                ++g_h.kind13;
                CHECK(m.length == 10 && dst == 0 && est && src == g_host.our_id);
                CHECK(memcmp(p + 1, kHostMac, 6) == 0 && p[7] == 0 && p[8] == 0 && p[9] == 2);
            }
            else CHECK(!"unexpected session kind");
        }
        else if (m.protocol == 3)
        {
            CHECK(m.length == 21 && dst == 1 && !est);
            CHECK_EQ(bin_b16(p + 19), g_host.our_id);
            if (p[0] == 0) { ++g_h.probes; CHECK(bin_b16(p + 17) == 0); }
            else { ++g_h.replies; CHECK(p[0] == 1 && bin_b16(p + 17) == g_link.conn.our_id); }
        }
        else if (m.protocol == 10)
        {
            CHECK(dst == g_link.conn.our_id && !est);
            check_reliable(p, m.length);
        }
        else CHECK(!"unexpected protocol");
    }
    CHECK(count > 0);
}

/* ---- callbacks --------------------------------------------------------------------- */

static int g_child_expected = 1, g_child_delivered, g_child_bad_length, g_byte8_length = -1;
static int g_parent_last, g_parent_delivered, g_parent_state_only;

static void host_send(const uint8_t *d, size_t n, const char *dest, void *user)
{
    (void)user;
    CHECK(strcmp(dest, JOINER_IP) == 0);
    inspect_host_datagram(d, n);
    pipe_put(&g_to_joiner, d, n);
}

/* Child frame: 2-byte LLSF header + 14-byte slot, command word = counter. */
static void host_deliver(const uint8_t *payload, size_t n, int station, void *user)
{
    (void)user;
    CHECK_EQ(station, 0);
    if (n == 20) { g_byte8_length = (int)n; return; }   /* frame from inject_byte8_wt */
    if (n != 16) { ++g_child_bad_length; return; }
    CHECK(payload[0] == 0x0e && payload[1] == 0x10);
    CHECK_EQ(bin_u16(payload + 2), g_child_expected);
    ++g_child_expected;
    ++g_child_delivered;
}

static void host_log(const char *message, void *user) { (void)user; printf("  host: %s\n", message); }

static void joiner_send(const uint8_t *d, size_t n, const char *dest, void *user)
{
    (void)user;
    CHECK(strcmp(dest, HOST_IP) == 0);
    pipe_put(&g_to_host, d, n);
}

/* Joiner delivers the whole WT frame: length at byte 8, payload from byte 12. */
static void joiner_deliver(const uint8_t *frame, size_t length, void *user)
{
    (void)user;
    CHECK(length >= 12 && frame[0] == 'W' && frame[1] == 'T');
    size_t n = frame[8] & 0x7f;
    if (n == 1) { CHECK_EQ(length, 12); return; }
    CHECK(12 + n <= length);
    const uint8_t *payload = frame + 12;
    if (n == 73)
    {
        CHECK(payload[0] == 0x46 && payload[1] == 0x00 && payload[2] == 0x05);
        uint16_t counter = bin_u16(payload + 3);
        if (counter == 0) { ++g_parent_state_only; return; }
        /* In order, no duplicates. Frames the child cannot take in time are dropped
           (and counted) by the host. */
        CHECK(counter > g_parent_last);
        g_parent_last = counter;
        ++g_parent_delivered;
    }
}

static void joiner_log(const char *message, void *user) { (void)user; printf("  joiner: %s\n", message); }

/* ---- scenarios ------------------------------------------------------------------ */

static void step(void)
{
    ++g_tick;
    g_now_us = (int64_t)g_tick * 16743;
    datagram_t *d;
    while ((d = pipe_take(&g_to_host)) != NULL) pia_host_receive(&g_host, d->data, d->len, JOINER_IP);
    while ((d = pipe_take(&g_to_joiner)) != NULL) pia_link_receive(&g_link, d->data, d->len, HOST_IP);
    pia_host_tick(&g_host, g_now_us / 1000);
    pia_link_tick(&g_link);
}

static void parent_command_frame(uint16_t counter)
{
    uint8_t f[73] = {0x46, 0x00, 0x05};
    bin_w16(f + 3, counter);
    bin_w16(f + 5, (uint16_t)(counter * 3));
    pia_host_parent_frame(&g_host, f, sizeof(f));
}

static void child_frame(uint16_t counter)
{
    uint8_t f[16] = {0x0e, 0x10};
    bin_w16(f + 2, counter);
    bin_w16(f + 4, (uint16_t)(counter ^ 0x5a5a));
    pia_link_enqueue(&g_link, f, sizeof(f));
}

static void reset_all(void)
{
    memset(&g_to_joiner, 0, sizeof(g_to_joiner));
    memset(&g_to_host, 0, sizeof(g_to_host));
    memset(&g_h, 0, sizeof(g_h));
    g_tick = 0;
    g_child_expected = 1; g_parent_last = 0;
    g_child_delivered = g_parent_delivered = g_child_bad_length = g_parent_state_only = 0;
    g_byte8_length = -1;
    pia_link_init(&g_link, kSsid, kJoinerMac, kHostMac, JOINER_IP, HOST_IP, joiner_send, joiner_deliver, joiner_log, NULL);
    pia_link_set_connect(&g_link, false);
    pia_host_init(&g_host, kSsid, kHostMac, HOST_IP, host_send, host_deliver, host_log, NULL);
    pia_host_set_app_data(&g_host, g_app_data, sizeof(g_app_data));
}

/* Child WT as a Switch builds it: length at byte 8, byte 9 zero. */
static void inject_byte8_wt(void)
{
    uint8_t wt[12 + 20] = {'W', 'T', 28, 0};
    bin_w32(wt + 4, 0x5000);
    wt[8] = 20;
    for (int i = 0; i < 20; ++i) wt[12 + i] = (uint8_t)(0x40 + i);
    uint8_t frame[8 + sizeof(wt)];
    frame[0] = 7;
    bin_wb16(frame + 1, sizeof(wt));
    uint16_t seq = g_link.reliable.next;
    bin_wb16(frame + 3, seq);
    bin_wb16(frame + 5, pia_reliable_send_low(&g_link.reliable));
    frame[7] = 0;
    memcpy(frame + 8, wt, sizeof(wt));
    g_link.reliable.next = (uint16_t)(seq + 1);
    pia_message_t m = {.protocol = 10, .payload = frame, .length = sizeof(frame), .has_flags = false};
    uint8_t body[128], datagram[256];
    int n = pia_message_encode(&m, body, sizeof(body));
    bin_wb16(body + n, g_host.our_id);
    n += 2;
    int padding = (16 - (n & 15)) & 15;
    memset(body + n, 0xff, padding);
    n += padding;
    int len = pia_encrypt(&g_link.crypto, body, (size_t)n, JOINER_IP, g_host.our_id, g_link.conn.our_id, 999,
                          0x123456789abcdefULL, (uint8_t)(padding << 4), 2, datagram, sizeof(datagram));
    CHECK(len > 0);
    pia_host_receive(&g_host, datagram, (size_t)len, JOINER_IP);
}

static void run_session(uint32_t seed)
{
    printf("session with seed %u\n", (unsigned)seed);
    g_rng = seed;
    reset_all();

    /* Station handshake: status, join, station list, probes. */
    CHECK_EQ(pia_host_station_joined(&g_host, kJoinerMac, JOINER_IP, 1), 0);
    CHECK_EQ(g_host.st[0].state, 1);
    int t0 = g_tick;
    while (!(pia_conn_connected(&g_link.conn) && pia_host_session_up(&g_host, 0)) && g_tick - t0 < 300) step();
    CHECK(pia_conn_connected(&g_link.conn));
    CHECK(pia_host_session_up(&g_host, 0));
    CHECK(g_host.st[0].status_acked);
    CHECK_EQ(g_host.st[0].id, g_link.conn.our_id);
    CHECK_EQ(g_link.conn.host_id, g_host.our_id);
    CHECK_EQ(g_h.status, 1);
    CHECK_EQ(g_h.status_seq_first, 2);
    CHECK_EQ(g_h.kind5, 1);
    CHECK_EQ(g_h.kind2, 1);
    CHECK(g_h.status_checked && g_h.kind5_checked && g_h.kind2_checked);
    CHECK(g_h.probes >= 1);
    CHECK_EQ(g_h.wa, 0);
    CHECK_EQ(g_h.wt, 0);

    /* Probes both ways to fill the host's RTT window. */
    for (int i = 0; i < 60; ++i) step();
    CHECK(g_h.replies >= 2);
    CHECK(g_host.st[0].reliable.sample_count >= 1);

    /* Joiner WC -> bridge -> WA. */
    pia_link_set_connect(&g_link, true);
    t0 = g_tick;
    while (pia_host_connect_pending(&g_host) < 0 && g_tick - t0 < 60) step();
    CHECK_EQ(pia_host_connect_pending(&g_host), 0);
    CHECK(memcmp(g_host.st[0].connect_id, g_link.connect_id, 2) == 0);
    for (int i = 0; i < 3; ++i) step();       /* bridge delay asking the GBA */
    CHECK_EQ(g_h.wa, 0);
    pia_host_accept(&g_host, 0, true, 0);
    CHECK(g_host.st[0].child_accepted);
    CHECK_EQ(pia_host_connect_pending(&g_host), -1);
    t0 = g_tick;
    while (!g_link.accepted && g_tick - t0 < 60) step();
    CHECK(g_link.accepted);
    CHECK(g_h.wa_checked);
    CHECK_EQ(g_h.wa_id, 1);
    CHECK(memcmp(g_h.wa_cid, g_link.connect_id, 2) == 0);

    /* No parent frames yet: idle WT every 16 ticks, WG 0, and the property update
       repeated until the joiner's 0x51. */
    for (int i = 0; i < 90; ++i) step();
    CHECK(g_h.wt >= 4 && g_h.wt <= 6);
    CHECK_EQ(g_h.wt_idle, g_h.wt);
    CHECK_EQ(g_h.wg, 1);
    CHECK_EQ(g_h.wg_values[0], 0);
    CHECK(g_h.property_checked);
    CHECK_EQ(g_h.property, 1);
    CHECK(g_host.st[0].property_acked);

    /* Join phase: parent NI frames, sent as WT with padded data. */
    static const uint8_t ni[8] = {0x05, 0x48, 0x04, 0x00, 0x05, 0x00, 0x01, 0x00};
    pia_host_parent_frame(&g_host, ni, 8);
    pia_host_parent_frame(&g_host, ni, 5);
    step(); step();
    CHECK_EQ(g_host.st[0].parent_frames, 2);
    CHECK_EQ(g_h.wg, 1);

    /* 2000 ticks: parent command frame every tick, child frame every other tick,
       5% loss and 2% reordering each way. */
    g_to_joiner.loss = g_to_host.loss = 0.05;
    g_to_joiner.reorder = g_to_host.reorder = 0.02;
    uint16_t parent_counter = 1, child_counter = 1;
    int worst_lag = 0;
    for (int i = 0; i < 2000; ++i)
    {
        parent_command_frame(parent_counter++);
        if (i % 2 == 0) child_frame(child_counter++);
        step();
        int lag = (parent_counter - 1) - g_parent_delivered - g_host.st[0].parent_dropped;
        if (lag > worst_lag) worst_lag = lag;
        CHECK(lag < 90);
        if (g_failures > 20) { printf("  too many failures, stopping\n"); break; }
    }
    g_to_joiner.loss = g_to_host.loss = 0;
    g_to_joiner.reorder = g_to_host.reorder = 0;
    /* Parent keeps streaming like a GBA; child sends on its credits. */
    for (int i = 0; i < 120; ++i) { parent_command_frame(parent_counter++); step(); }
    for (int i = 0; i < 30; ++i) step();
    printf("  exchange: parent %d/%d child %d/%d worst lag %d, host resends %d (%d copies on the wire),"
           " host holds dropped %d, joiner reordered %d overflow %d, loss %d/%d and %d/%d\n",
           g_parent_delivered, parent_counter - 1, g_child_delivered, child_counter - 1, worst_lag,
           g_host.st[0].resends, g_h.retransmits, g_host.st[0].hold_dropped, g_link.reordered, g_link.overflow,
           g_to_joiner.dropped, g_to_joiner.sent, g_to_host.dropped, g_to_host.sent);
    /* Each parent frame is delivered or dropped by host pacing. Pacing follows the
       child's datagram cadence, so the drop ratio here does not model a real child. */
    CHECK_EQ(g_parent_delivered + g_host.st[0].parent_dropped, parent_counter - 1);
    CHECK_EQ(g_child_delivered, child_counter - 1);
    CHECK_EQ(g_child_bad_length, 0);
    CHECK_EQ(g_host.st[0].child_frames, child_counter - 1);
    CHECK_EQ(g_host.st[0].hold_dropped, 0);
    CHECK_EQ(g_host.st[0].decrypt_failures, 0);
    CHECK_EQ(g_host.st[0].rx_bad_frame, 0);
    CHECK_EQ(g_host.st[0].rx_unzip_fail, 0);
    CHECK_EQ(g_link.overflow, 0);
    CHECK_EQ(g_link.out_count, 0);
    CHECK_EQ(g_link.hold_dropped, 0);
    CHECK_EQ(g_link.decrypt_failures, 0);
    CHECK_EQ(g_link.rx_bad_frame, 0);
    CHECK(g_host.st[0].resends > 0);
    CHECK(g_host.st[0].wk_acks > 0);
    CHECK_EQ(pia_reliable_pending(&g_host.st[0].reliable), 0);
    CHECK_EQ(pia_reliable_pending(&g_link.reliable), 0);
    CHECK_EQ(g_h.wg, 2);
    CHECK_EQ(g_h.wg_values[1], 1);
    CHECK(g_h.wt_before_uni_wg <= 8);      /* NI-phase only; WG 1 precedes the first UNI frame */
    CHECK(g_host.st[0].reliable.sample_count == PIA_RTT_SAMPLES);
    double rto = pia_reliable_rto(&g_host.st[0].reliable);
    CHECK(rto >= 40 && rto <= 250);
    CHECK(g_h.property == 1);

    /* Command-less frames replace each other in the queue and repeat when idle. */
    uint8_t plain_frame[73] = {0x46, 0x00, 0x05};
    for (int i = 0; i < 5; ++i) pia_host_parent_frame(&g_host, plain_frame, sizeof(plain_frame));
    CHECK_EQ(g_host.st[0].parent_count, 1);
    int state_only = g_parent_state_only;
    for (int i = 0; i < 40; ++i) step();
    CHECK(g_parent_state_only - state_only >= 2);
    CHECK_EQ(g_host.st[0].parent_count, 0);

    /* Child WT with length at byte 8, as a Switch child sends it. */
    inject_byte8_wt();
    CHECK_EQ(g_byte8_length, 20);

    /* GBA drops the child: WD, then notices, broadcasts and two status updates for the
       removal. The station is then forgotten. */
    pia_host_disconnect_child(&g_host, 0);
    CHECK(g_host.st[0].child_disconnected);
    t0 = g_tick;
    while (!g_link.host_disconnected && g_tick - t0 < 60) step();
    CHECK(g_link.host_disconnected);
    CHECK_EQ(g_h.wd, 1);
    CHECK(memcmp(g_h.wd_cid, g_link.connect_id, 2) == 0);
    int wt_at_wd = g_h.wt;
    t0 = g_tick;
    while (g_host.st[0].state != 0 && g_tick - t0 < 1200) step();
    CHECK_EQ(g_host.st[0].state, 0);
    CHECK_EQ(g_h.wt, wt_at_wd);
    CHECK_EQ(g_h.kind9, 3);
    CHECK_EQ(g_h.kind13, 5);
    CHECK(g_h.status_marked >= 1 && g_h.status_marked <= 2);
    CHECK(g_h.status_cleared >= 1 && g_h.status_cleared <= 2);
    CHECK_EQ(g_h.status_seq_last, 4);
    CHECK_EQ(g_host.st[0].decrypt_failures, 0);
    printf("  teardown took %d ticks; %d probes, %d replies, %d acks, %d WT (%d idle), %d WG\n",
           g_tick - t0, g_h.probes, g_h.replies, g_h.acks, g_h.wt, g_h.wt_idle, g_h.wg);
}

/* Handshake with the joiner's early datagrams lost. Status update repeats until acked;
   station list repeats until the joiner answers. */
static void run_lossy_handshake(uint32_t seed)
{
    printf("handshake with early loss, seed %u\n", (unsigned)seed);
    g_rng = seed;
    reset_all();
    g_to_host.loss = 1.0;          /* all joiner traffic lost for 40 ticks */
    CHECK_EQ(pia_host_station_joined(&g_host, kJoinerMac, JOINER_IP, 1), 0);
    for (int i = 0; i < 40; ++i) step();
    CHECK(g_h.status >= 2);
    CHECK(!g_host.st[0].status_acked);
    g_to_host.loss = 0.3;
    g_to_joiner.loss = 0.3;
    int t0 = g_tick;
    while (!(pia_conn_connected(&g_link.conn) && pia_host_session_up(&g_host, 0)) && g_tick - t0 < 600) step();
    CHECK(pia_conn_connected(&g_link.conn));
    CHECK(pia_host_session_up(&g_host, 0));
    g_to_host.loss = g_to_joiner.loss = 0;
    for (int i = 0; i < 40; ++i) step();
    CHECK(g_host.st[0].status_acked);
    CHECK(g_host.st[0].child_alive);
    printf("  status sent %d times, station list %d times, joined after %d ticks\n", g_h.status, g_h.kind5, g_tick - t0);
}

/* Parent queue rules, one station joined. */
static void test_parent_queue(void)
{
    printf("parent queue\n");
    reset_all();
    CHECK_EQ(pia_host_station_joined(&g_host, kJoinerMac, JOINER_IP, 1), 0);
    uint8_t plain_frame[73] = {0x46, 0x00, 0x05};
    uint8_t carrier[73] = {0};
    uint8_t command[73] = {0x46, 0x00, 0x05, 0x00, 0x66};
    uint8_t ni[4] = {0x01, 0x88, 0x04, 0x05};
    pia_host_parent_frame(&g_host, plain_frame, 73);
    pia_host_parent_frame(&g_host, carrier, 73);
    pia_host_parent_frame(&g_host, plain_frame, 73);
    CHECK_EQ(g_host.st[0].parent_count, 1);
    CHECK(g_host.has_idle && g_host.idle_len == 73);
    pia_host_parent_frame(&g_host, command, 73);
    CHECK_EQ(g_host.st[0].parent_count, 2);
    pia_host_parent_frame(&g_host, plain_frame, 73);
    pia_host_parent_frame(&g_host, plain_frame, 73);
    CHECK_EQ(g_host.st[0].parent_count, 3);
    pia_host_parent_frame(&g_host, ni, 4);
    pia_host_parent_frame(&g_host, ni, 4);
    CHECK_EQ(g_host.st[0].parent_count, 5);
    while (g_host.st[0].parent_count < PIA_HOST_PARENT_SLOTS) pia_host_parent_frame(&g_host, command, 73);
    pia_host_parent_frame(&g_host, plain_frame, 73);
    CHECK_EQ(g_host.st[0].parent_count, PIA_HOST_PARENT_SLOTS);
    CHECK_EQ(g_host.st[0].parent_dropped, 1);
    /* Queue full: oldest command-less frame is evicted, new command is kept. */
    pia_host_parent_frame(&g_host, command, 73);
    CHECK_EQ(g_host.st[0].parent_count, PIA_HOST_PARENT_SLOTS);
    CHECK_EQ(g_host.st[0].parent_dropped, 2);
    int commands = 0;
    for (int i = 0; i < g_host.st[0].parent_count; ++i)
    {
        const uint8_t *f = g_host.st[0].parent[(g_host.st[0].parent_head + i) % PIA_HOST_PARENT_SLOTS].data;
        if (f[3] || f[4]) ++commands;
    }
    CHECK_EQ(commands, PIA_HOST_PARENT_SLOTS - 1);
}

/* ---- group: one host, three joiners ---------------------------------------------- */

#define GROUP 3
static const char *const kGroupIp[GROUP] = {"169.254.47.2", "169.254.47.3", "169.254.47.4"};
static const uint8_t kGroupMac[GROUP][6] = {
    {0x8a, 0xca, 0xba, 0x4f, 0x18, 0x8f}, {0x6a, 0x16, 0x0c, 0x22, 0x7c, 0xda}, {0xaa, 0x23, 0x81, 0xca, 0x25, 0xfa}};
static pia_link_t g_links[GROUP];
static pipe_t g_to_member[GROUP];
static struct
{
    int parent_last, parent_delivered, child_expected, child_delivered, child_misrouted;
    int wa, wa_own, wd, wd_own, lists, list_stations, list_left_max, status_entries;
} g_m[GROUP];
static int g_relayed[GROUP][GROUP];

static int member_of_ip(const char *ip)
{
    for (int j = 0; j < GROUP; ++j) if (strcmp(ip, kGroupIp[j]) == 0) return j;
    return -1;
}

/* What the host sends joiner j: WA and WD (whose and to whom), station lists, statuses. */
static void inspect_group_datagram(const uint8_t *d, size_t n, int j)
{
    static uint8_t plain[PIA_MAX_BODY], app[PIA_MAX_INFLATE];
    int len = pia_decrypt(&g_links[j].crypto, d, n, HOST_IP, plain, sizeof(plain));
    CHECK(len >= 0);
    if (len < 0) return;
    int padding = d[5] >> 4, footer = d[12];
    len -= padding + footer;
    if (footer == 2) CHECK_EQ(bin_b16(plain + len), g_links[j].conn.our_id);
    const uint8_t *body = plain;
    int size = len;
    if (d[5] & 1)
    {
        size = pia_decompress(plain, (size_t)len, app, sizeof(app));
        CHECK(size >= 0);
        if (size < 0) return;
        body = app;
    }
    pia_message_iter_t it;
    pia_message_t m;
    pia_message_iter_init(&it, body, (size_t)size);
    while (pia_message_next(&it, &m))
    {
        const uint8_t *q = m.payload;
        if (m.protocol == 13 && m.length >= 21 && q[0] == 5)
        {
            ++g_m[j].lists;
            g_m[j].list_stations = q[17];
            if (q[18] > g_m[j].list_left_max) g_m[j].list_left_max = q[18];
        }
        else if (m.protocol == 1 && m.length >= 162 && q[1] == 0x11 && q[29] == 0)
        {
            int entries = 0;
            for (int i = 0; i < 6; ++i)
                if (q[30 + 22 * i + 1] != 0xff) { CHECK_EQ(q[30 + 22 * i + 1], i); ++entries; }
            g_m[j].status_entries = entries;
        }
        else if (m.protocol == 10 && m.length >= 12 && (q[0] & 1) && q[8] == 'W')
        {
            const uint8_t *w = q + 8;
            const bool own = memcmp(w + 4 + (w[1] == 'A' ? 2 : 0), g_links[j].connect_id, 2) == 0;
            if (w[1] == 'A') { ++g_m[j].wa; g_m[j].wa_own += own; if (own) CHECK_EQ(w[8], j); }
            else if (w[1] == 'D') { ++g_m[j].wd; g_m[j].wd_own += own; }
        }
    }
}

static void group_host_send(const uint8_t *d, size_t n, const char *dest, void *user)
{
    (void)user;
    const int j = member_of_ip(dest);
    CHECK(j >= 0);
    if (j < 0) return;
    inspect_group_datagram(d, n, j);
    pipe_put_from(&g_to_member[j], d, n, -1);
}

/* Child frame from joiner j: its index in the value's top bits. */
static void group_host_deliver(const uint8_t *payload, size_t n, int station, void *user)
{
    (void)user;
    CHECK(n == 16 && station >= 0 && station < GROUP);
    if (n != 16 || station < 0 || station >= GROUP) return;
    const int j = bin_u16(payload + 4) >> 12;
    if (j != station) { ++g_m[station].child_misrouted; return; }
    CHECK_EQ(bin_u16(payload + 2), g_m[j].child_expected);
    ++g_m[j].child_expected;
    ++g_m[j].child_delivered;
}

static void group_joiner_send(const uint8_t *d, size_t n, const char *dest, void *user)
{
    const int j = (int)(intptr_t)user;
    if (strcmp(dest, HOST_IP) == 0) { pipe_put_from(&g_to_host, d, n, j); return; }
    const int k = member_of_ip(dest);
    CHECK(k >= 0 && k != j);
    if (k < 0 || k == j) return;
    ++g_relayed[j][k];
    pipe_put_from(&g_to_member[k], d, n, j);
}

static void group_joiner_deliver(const uint8_t *frame, size_t length, void *user)
{
    const int j = (int)(intptr_t)user;
    CHECK(length >= 12 && frame[0] == 'W' && frame[1] == 'T');
    const size_t n = frame[8] & 0x7f;
    if (n != 73 || length < 12 + n) return;
    const uint16_t counter = bin_u16(frame + 12 + 3);
    if (counter == 0) return;
    CHECK(counter > g_m[j].parent_last);
    g_m[j].parent_last = counter;
    ++g_m[j].parent_delivered;
}

static void group_step(void)
{
    ++g_tick;
    g_now_us = (int64_t)g_tick * 16743;
    datagram_t *d;
    while ((d = pipe_take(&g_to_host)) != NULL) pia_host_receive(&g_host, d->data, d->len, kGroupIp[d->from]);
    for (int j = 0; j < GROUP; ++j)
        while ((d = pipe_take(&g_to_member[j])) != NULL)
            pia_link_receive(&g_links[j], d->data, d->len, d->from < 0 ? HOST_IP : kGroupIp[d->from]);
    pia_host_tick(&g_host, g_now_us / 1000);
    for (int j = 0; j < GROUP; ++j) pia_link_tick(&g_links[j]);
}

static void group_loss(double loss, double reorder)
{
    g_to_host.loss = loss;
    g_to_host.reorder = reorder;
    for (int j = 0; j < GROUP; ++j) { g_to_member[j].loss = loss; g_to_member[j].reorder = reorder; }
}

static void group_child_frame(int j, uint16_t counter)
{
    uint8_t f[16] = {0x0e, 0x10};
    bin_w16(f + 2, counter);
    bin_w16(f + 4, (uint16_t)(j << 12 | (counter & 0xfff)));
    pia_link_enqueue(&g_links[j], f, sizeof(f));
}

/* A child frame from joiner j as a Switch child sends it: the send header has the length
   only at bit 8 + 5 x its slot. Uses the joiner's next stream sequence, so it goes last. */
static void inject_switch_wt(int j, uint16_t counter)
{
    uint8_t wt[12 + 16] = {'W', 'T', 24, 0};
    bin_w32(wt + 4, 0x6000 + counter);
    bin_w32(wt + 8, 16u << (8 + 5 * g_links[j].client_slot));
    wt[12] = 0x0e; wt[13] = 0x10;
    bin_w16(wt + 14, counter);
    bin_w16(wt + 16, (uint16_t)(j << 12 | (counter & 0xfff)));
    uint8_t frame[8 + sizeof(wt)];
    frame[0] = 7;
    bin_wb16(frame + 1, sizeof(wt));
    const uint16_t seq = g_links[j].reliable.next;
    bin_wb16(frame + 3, seq);
    bin_wb16(frame + 5, pia_reliable_send_low(&g_links[j].reliable));
    frame[7] = 0;
    memcpy(frame + 8, wt, sizeof(wt));
    g_links[j].reliable.next = (uint16_t)(seq + 1);
    pia_message_t m = {.protocol = 10, .payload = frame, .length = sizeof(frame), .has_flags = false};
    uint8_t body[128], datagram[256];
    int n = pia_message_encode(&m, body, sizeof(body));
    bin_wb16(body + n, g_host.our_id);
    n += 2;
    int padding = (16 - (n & 15)) & 15;
    memset(body + n, 0xff, padding);
    n += padding;
    int len = pia_encrypt(&g_links[j].crypto, body, (size_t)n, kGroupIp[j], g_host.our_id, g_links[j].conn.our_id,
                          900 + j, 0x2468ace013579bdULL + (uint64_t)j, (uint8_t)(padding << 4), 2, datagram, sizeof(datagram));
    CHECK(len > 0);
    pia_host_receive(&g_host, datagram, (size_t)len, kGroupIp[j]);
}

static void run_group(uint32_t seed)
{
    printf("group of %d joiners, seed %u\n", GROUP, (unsigned)seed);
    g_rng = seed;
    memset(&g_to_host, 0, sizeof(g_to_host));
    memset(g_to_member, 0, sizeof(g_to_member));
    memset(g_m, 0, sizeof(g_m));
    memset(g_relayed, 0, sizeof(g_relayed));
    g_tick = 0;
    pia_host_init(&g_host, kSsid, kHostMac, HOST_IP, group_host_send, group_host_deliver, host_log, NULL);
    pia_host_set_app_data(&g_host, g_app_data, sizeof(g_app_data));
    for (int j = 0; j < GROUP; ++j)
    {
        pia_link_init(&g_links[j], kSsid, kGroupMac[j], kHostMac, kGroupIp[j], HOST_IP, group_joiner_send,
                      group_joiner_deliver, joiner_log, (void *)(intptr_t)j);
        pia_link_set_connect(&g_links[j], false);
        g_m[j].child_expected = 1;
    }

    /* Joiners arrive one after another; each list names everyone so far. */
    for (int j = 0; j < GROUP; ++j)
    {
        CHECK_EQ(pia_host_station_joined(&g_host, kGroupMac[j], kGroupIp[j], (uint8_t)(j + 1)), j);
        for (int i = 0; i < 40; ++i) group_step();
    }
    int t0 = g_tick;
    bool all = false;
    while (!all && g_tick - t0 < 600)
    {
        group_step();
        all = true;
        for (int j = 0; j < GROUP; ++j) all = all && pia_conn_connected(&g_links[j].conn) && pia_host_session_up(&g_host, j);
    }
    CHECK(all);
    for (int i = 0; i < 200; ++i) group_step();
    CHECK_EQ(g_host.list_seq, GROUP);
    for (int j = 0; j < GROUP; ++j)
    {
        CHECK_EQ(g_host.st[j].update_acked, g_host.list_seq);
        CHECK(g_host.st[j].status_acked);
        CHECK_EQ(g_m[j].status_entries, GROUP + 1);
        CHECK_EQ(g_m[j].list_stations, GROUP + 1);
        CHECK_EQ(g_links[j].conn.peer_count, GROUP - 1);
        for (int k = 0; k < g_links[j].conn.peer_count; ++k)
        {
            const int o = member_of_ip(g_links[j].conn.peers[k].ip);
            CHECK(o >= 0 && o != j);
            if (o >= 0) CHECK_EQ(g_links[j].conn.peers[k].id, g_links[o].conn.our_id);
        }
        for (int k = 0; k < GROUP; ++k) if (k != j) CHECK(g_relayed[j][k] > 0);
    }

    /* Connects one at a time, as the bridge passes them to the adapter; slots in order. */
    for (int j = 0; j < GROUP; ++j) pia_link_set_connect(&g_links[j], true);
    int accepted = 0;
    t0 = g_tick;
    while (accepted < GROUP && g_tick - t0 < 600)
    {
        const int w = pia_host_connect_pending(&g_host);
        if (w >= 0)
        {
            CHECK_EQ(w, accepted);
            pia_host_accept(&g_host, w, true, (uint8_t)accepted);
            ++accepted;
        }
        group_step();
    }
    for (int i = 0; i < 60; ++i) group_step();
    for (int j = 0; j < GROUP; ++j)
    {
        CHECK(g_links[j].accepted);
        CHECK_EQ(g_links[j].client_slot, j);
        CHECK_EQ(g_m[j].wa_own, 1);
    }
    CHECK(g_m[0].wa > 1);                   /* the first joiner also saw the later WAs */

    /* The GBA's frames reach every child in order; each child's frames reach the GBA as
       its own, under 3% loss and 1% reordering. */
    group_loss(0.03, 0.01);
    uint16_t parent_counter = 1, child_counter[GROUP] = {1, 1, 1};
    for (int i = 0; i < 1500; ++i)
    {
        parent_command_frame(parent_counter++);
        for (int j = 0; j < GROUP; ++j) if ((i + j) % 2 == 0) group_child_frame(j, child_counter[j]++);
        group_step();
        if (g_failures > 20) { printf("  too many failures, stopping\n"); return; }
    }
    group_loss(0, 0);
    for (int i = 0; i < 120; ++i) { parent_command_frame(parent_counter++); group_step(); }
    for (int i = 0; i < 30; ++i) group_step();
    for (int j = 0; j < GROUP; ++j)
    {
        const pia_host_station_t *st = &g_host.st[j];
        printf("  joiner %d: parent %d (+%d dropped) of %d, child %d of %d, host resends %d holds dropped %d\n", j,
               g_m[j].parent_delivered, st->parent_dropped, parent_counter - 1, g_m[j].child_delivered,
               child_counter[j] - 1, st->resends, st->hold_dropped);
        CHECK_EQ(g_m[j].parent_delivered + st->parent_dropped, parent_counter - 1);
        CHECK_EQ(g_m[j].child_delivered, child_counter[j] - 1);
        CHECK_EQ(g_m[j].child_misrouted, 0);
        CHECK_EQ(st->decrypt_failures, 0);
        CHECK_EQ(g_links[j].overflow, 0);
        CHECK_EQ(g_links[j].out_count, 0);
    }

    /* The GBA drops the second child: its WD reaches the group, only it leaves; the other
       two get a list naming it as left and keep going. */
    pia_host_disconnect_child(&g_host, 1);
    t0 = g_tick;
    while (!g_links[1].host_disconnected && g_tick - t0 < 60) group_step();
    CHECK(g_links[1].host_disconnected);
    CHECK(!g_links[0].host_disconnected && !g_links[2].host_disconnected);
    CHECK(g_m[0].wd == 1 && g_m[0].wd_own == 0 && g_m[1].wd_own == 1);
    t0 = g_tick;
    while (g_host.st[1].state != 0 && g_tick - t0 < 1200) { parent_command_frame(parent_counter++); group_step(); }
    CHECK_EQ(g_host.st[1].state, 0);
    for (int i = 0; i < 200; ++i) { parent_command_frame(parent_counter++); group_step(); }
    for (int j = 0; j < GROUP; j += 2)
    {
        CHECK_EQ(g_links[j].conn.peer_count, 1);
        CHECK_EQ(g_m[j].list_stations, GROUP);
        CHECK_EQ(g_m[j].list_left_max, 1);
        CHECK_EQ(g_m[j].status_entries, GROUP);
        CHECK_EQ(g_host.st[j].update_acked, g_host.list_seq);
    }
    CHECK_EQ(g_host.left_count, 0);           /* acknowledged by both: no longer listed */
    const int before0 = g_m[0].parent_delivered, before2 = g_m[2].parent_delivered;
    for (int i = 0; i < 300; ++i)
    {
        parent_command_frame(parent_counter++);
        if (i % 2 == 0) { group_child_frame(0, child_counter[0]++); group_child_frame(2, child_counter[2]++); }
        group_step();
    }
    for (int i = 0; i < 30; ++i) group_step();
    CHECK(g_m[0].parent_delivered - before0 > 200 && g_m[2].parent_delivered - before2 > 200);
    CHECK_EQ(g_m[0].child_delivered, child_counter[0] - 1);
    CHECK_EQ(g_m[2].child_delivered, child_counter[2] - 1);
    printf("  after the drop: list version %d, joiner 0 parent %d child %d, joiner 2 parent %d child %d\n",
           g_host.list_seq, g_m[0].parent_delivered, g_m[0].child_delivered, g_m[2].parent_delivered,
           g_m[2].child_delivered);

    /* Switch-style child frames, slot 0 and slot 2, reach the GBA whole and in order. */
    for (int j = 0; j < GROUP; j += 2)
    {
        const int before = g_m[j].child_delivered;
        inject_switch_wt(j, child_counter[j]++);
        CHECK_EQ(g_m[j].child_delivered, before + 1);
        CHECK_EQ(g_m[j].child_misrouted, 0);
    }
}

int main(void)

{
    test_parent_queue();
    run_session(1);
    run_session(7);
    run_session(42);
    run_lossy_handshake(3);
    run_group(5);
    run_group(11);
    if (g_failures) { printf("%d FAILURES\n", g_failures); return 1; }
    printf("all checks passed\n");
    return 0;
}
