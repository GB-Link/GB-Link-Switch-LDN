#include <inttypes.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_private/wifi.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "s3_transport.h"
#include "ldn_control.h"
#include "ldn_udp.h"
#include "ldn_session.h"
#include "ldn_wire.h"
#include "rfu_link.h"
#include "rfu_section.h"
#define printf ldn_wire_printf
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACARGS(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define FRAME_MAX 1514

typedef struct { uint16_t length; uint8_t bytes[FRAME_MAX]; } control_frame_t;
static esp_netif_t *s_netif;
static QueueHandle_t s_rx;
static uint8_t s_host[6], s_mac[6];
static atomic_bool s_connected;
static atomic_uint s_dropped, s_rx_count, s_tx_ok, s_tx_failed, s_air_data;

static esp_err_t control_rx(void *buffer, uint16_t length, void *eb)
{
    const uint8_t *bytes = buffer;
    if (length >= 14 && bytes[12] == 0x88 && bytes[13] == 0xb7) {
        control_frame_t frame;
        if (length <= FRAME_MAX && memcmp(bytes + 6, s_host, 6) == 0) {
            frame.length = length;
            memcpy(frame.bytes, bytes, length);
            atomic_fetch_add(&s_rx_count, 1);
            if (xQueueSend(s_rx, &frame, 0) != pdTRUE) atomic_fetch_add(&s_dropped, 1);
        }
        esp_wifi_internal_free_rx_buffer(eb);
        return ESP_OK;
    }
    return esp_netif_receive(s_netif, buffer, length, eb);
}

static void tx_done(uint8_t ifidx, uint8_t *data, uint16_t *length, bool success)
{
    if (ifidx == WIFI_IF_STA) atomic_fetch_add(success ? &s_tx_ok : &s_tx_failed, 1);
}

void ldn_control_sniff(const unsigned char *frame, size_t length)
{
    if (length >= 28 && (frame[0] & 0x0c) == 8 && !memcmp(frame + 10, s_host, 6))
        atomic_fetch_add(&s_air_data, 1);
}

void ldn_control_init(esp_netif_t *netif, const unsigned char host[6])
{
    s_netif = netif;
    memcpy(s_host, host, 6);
    ldn_udp_init(netif, host);
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, s_mac));
    s_rx = xQueueCreate(8, sizeof(control_frame_t));
    ESP_ERROR_CHECK(s_rx ? ESP_OK : ESP_ERR_NO_MEM);
    s3_transport_init();
    ESP_ERROR_CHECK(esp_wifi_set_tx_done_cb(tx_done));
    printf("LDN_S3_READY transport=usb-serial-jtag heap=%" PRIu32 "\n", esp_get_free_heap_size());
    /* Bring the adapter up automatically: a host closing the port resets the
       chip, and a manual LDN_RFU_START does not survive that -- the GBA then
       sees its adapter disappear. Harmless when no cable is attached. */
    ldn_wire_set_rfu_handler(rfu_section_feed_bytes);
    rfu_section_start(1);
}

void ldn_control_link(bool connected)
{
    if (connected) ESP_ERROR_CHECK(esp_wifi_internal_reg_rxcb(WIFI_IF_STA, control_rx));
    atomic_store(&s_connected, connected);
}

void ldn_control_target(const unsigned char host[6])
{
    memcpy(s_host, host, 6);
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, s_mac));
    ldn_udp_init(s_netif, host);
    xQueueReset(s_rx);
    atomic_store(&s_connected, false);
}

static int nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void command(const char *line)
{
    if (!strcmp(line, "LDN_BINARY") || !strcmp(line, "LDN_HELLO")) { ldn_wire_enable(); return; }
    if (!strncmp(line, "LDN_BEGIN ", 10)) {
        char *end; unsigned long id = strtoul(line + 10, &end, 16);
        if (*end || !id || strlen(line + 10) != 8) { printf("LDN_ERROR INVALID_SESSION\n"); return; }
        ldn_session_stop(); ldn_wire_session((uint32_t)id); printf("LDN_BEGUN\n"); return;
    }
    if (!strncmp(line, "LDN_SCAN ", 9)) {
        unsigned channel; char extra;
        int result = sscanf(line + 9, "%u %c", &channel, &extra) == 1 ? ldn_session_scan(channel) : ESP_ERR_INVALID_ARG;
        printf("LDN_SCAN_RESULT %d\n", result); return;
    }
    if (!strncmp(line, "LDN_CONFIG ", 11)) {
        unsigned channel; char ssid[33], bssid[18], key[33], extra;
        int result = sscanf(line + 11, "%u %32s %17s %32s %c", &channel, ssid, bssid, key, &extra) == 4
            ? ldn_session_configure(ssid, bssid, key, channel) : ESP_ERR_INVALID_ARG;
        memset(key, 0, sizeof(key));
        printf("LDN_CONFIG_RESULT %d\n", result); return;
    }
    if (!strcmp(line, "LDN_STOP")) { ldn_session_stop(); printf("LDN_STOPPED\n"); return; }
    if (!strcmp(line, "LDN_QUIET")) { printf("LDN_QUIET_OK\n"); return; }
    /* Wireless-adapter (RFU) mode: claims the GBA link pins and pins a spinning
       task to the core Wi-Fi is not on. Opt-in, so the LDN-only path is unchanged. */
    if (!strcmp(line, "LDN_RFU_START")) {
        ldn_wire_set_rfu_handler(rfu_section_feed_bytes);
        rfu_section_start(1);
        printf("LDN_RFU_STARTED\n"); return;
    }
    if (!strcmp(line, "LDN_RFU_CSTEST")) {
        /* Drives CS: only safe when the GBA is idle, never while traffic flows. */
        bool lo = false, hi = false;
        const bool ok = rfu_link_cs_controllable(&lo, &hi);
        printf("LDN_RFU_CS controllable=%d low=%d high=%d\n", ok ? 1 : 0, lo ? 1 : 0, hi ? 1 : 0);
        return;
    }
    if (!strcmp(line, "LDN_RFU_LOOPBACK")) {
        uint16_t bits = 0; uint32_t word = 0;
        const bool ok = rfu_link_loopback_test(&bits, &word);
        printf("LDN_RFU_LOOPBACK ok=%d bits=%u word=%08" PRIx32 "\n", ok ? 1 : 0, bits, word);
        return;
    }
    if (!strcmp(line, "LDN_RFU_STOP")) { rfu_section_stop(); printf("LDN_RFU_STOPPED\n"); return; }
    if (!strcmp(line, "LDN_RFU_STATS")) {
        uint32_t sw = 0, mw = 0, partial = 0, in = 0, out = 0;
        rfu_section_stats(&sw, &mw, &partial, &in, &out);
        bool enabled = false; int role = 0; uint8_t levels = 0, comstate = 0;
        rfu_section_state(&enabled, &role, &levels, &comstate);
        uint32_t at0 = 0, late = 0; uint8_t lastbit = 0;
        rfu_link_partial_detail(&at0, &late, &lastbit);
        printf("LDN_RFU_PARTIAL false_start=%" PRIu32 " too_slow=%" PRIu32 " last_bit=%u glitches=%" PRIu32 "\n",
               at0, late, lastbit, rfu_link_glitches());
        uint32_t words[6]; const uint8_t nw = rfu_link_word_log(words, 6);
        if (nw) {
            printf("LDN_RFU_WORDS");
            for (uint8_t i = 0; i < nw; ++i) printf(" %08" PRIx32, words[i]);
            printf("\n");
        }
        printf("LDN_RFU_RATE polls=%" PRIu32 " empty_frames=%" PRIu32 " last_bits=%u\n",
               rfu_link_polls(), rfu_link_empty_frames(), rfu_link_last_trans_bits());
        printf("LDN_RFU_STATE enabled=%d role=%s comstate=%u sc=%d so=%d si=%d sd=%d\n",
               enabled ? 1 : 0, role ? "adapter-master" : "gba-master", comstate,
               (levels & 1) ? 1 : 0, (levels & 2) ? 1 : 0, (levels & 4) ? 1 : 0, (levels & 8) ? 1 : 0);
        printf("LDN_RFU_STATS slave_words=%" PRIu32 " master_words=%" PRIu32 " partial=%" PRIu32
               " net_in=%" PRIu32 " net_out=%" PRIu32 "\n", sw, mw, partial, in, out);
        return;
    }
    if (!strcmp(line, "LDN_BAUD 921600") || !strcmp(line, "LDN_BAUD 115200")) {
        /* USB line coding does not change physical throughput. Keep host v1 compatibility. */
        printf("LDN_BAUD_READY %s\n", line + 9); return;
    }
    if (ldn_udp_command(line, atomic_load(&s_connected))) return;
    if (!strcmp(line, "LDN_STATUS")) {
        printf("LDN_LINK %u " MACSTR "\n", atomic_load(&s_connected), MACARGS(s_mac));
        printf("LDN_S3_STATS tx_ok=%u tx_failed=%u ethernet_rx=%u air_rx=%u dropped=%u usb_dropped=%" PRIu32 " heap=%" PRIu32 "\n",
            atomic_load(&s_tx_ok), atomic_load(&s_tx_failed), atomic_load(&s_rx_count), atomic_load(&s_air_data),
            atomic_load(&s_dropped), s3_transport_dropped(), esp_get_free_heap_size());
        return;
    }
    if (strncmp(line, "LDN_TX ", 7)) { printf("LDN_ERROR UNKNOWN_COMMAND\n"); return; }
    static uint8_t frame[FRAME_MAX];
    const char *hex = line + 7;
    size_t n = strlen(hex);
    int result = ESP_ERR_INVALID_ARG;
    if (n < 10 || n > 3000 || n % 2) goto done;
    for (size_t i = 0; i < n / 2; ++i) {
        int hi = nibble(hex[2 * i]), lo = nibble(hex[2 * i + 1]);
        if (hi < 0 || lo < 0) goto done;
        frame[14 + i] = (hi << 4) | lo;
    }
    static const uint8_t prefix[] = {0x00, 0x22, 0xaa, 0x01, 0x02};
    if (memcmp(frame + 14, prefix, sizeof(prefix))) goto done;
    result = ESP_ERR_INVALID_STATE;
    if (!atomic_load(&s_connected)) goto done;
    memcpy(frame, s_host, 6); memcpy(frame + 6, s_mac, 6);
    frame[12] = 0x88; frame[13] = 0xb7;
    result = esp_wifi_internal_tx(WIFI_IF_STA, frame, 14 + n / 2);
done:
    printf("LDN_TX_RESULT %d\n", result);
}

void ldn_control_poll(void)
{
    /* Hand queued adapter frames to the host from THIS task: the wire layer
       serialises through static buffers and the adapter runs on another core. */
    if (ldn_wire_active()) {
        static uint8_t rfu_frame[104];
        size_t rfu_len = 0;
        for (int i = 0; i < 8 && rfu_section_poll_outbound(rfu_frame, sizeof(rfu_frame), &rfu_len); ++i)
            ldn_wire_send_rfu(rfu_frame, rfu_len);
    }

    static bool previous;
    bool connected = atomic_load(&s_connected);
    if (previous != connected) {
        printf("LDN_LINK %u " MACSTR "\n", connected, MACARGS(s_mac)); previous = connected;
    }
    static control_frame_t frame;
    static char hex[FRAME_MAX * 2 + 1];
    static const char digits[] = "0123456789abcdef";
    while (xQueueReceive(s_rx, &frame, 0) == pdTRUE) {
        size_t n = frame.length - 14;
        for (size_t i = 0; i < n; ++i) { hex[2*i] = digits[frame.bytes[14+i] >> 4]; hex[2*i+1] = digits[frame.bytes[14+i] & 15]; }
        hex[2*n] = 0;
        printf("LDN_RX " MACSTR " %s\n", MACARGS(frame.bytes + 6), hex);
    }
    static char line[3016];
    static size_t used;
    static bool overflow;
    static uint8_t bytes[8192];
    int n = s3_transport_read(bytes, sizeof(bytes));
    for (int i = 0; i < n; ++i) {
        if (ldn_wire_active()) { ldn_wire_feed(bytes[i], command); continue; }
        if (bytes[i] == '\r') continue;
        if (bytes[i] == '\n') {
            line[used] = 0;
            if (!overflow) command(line); else printf("LDN_ERROR LINE_TOO_LONG\n");
            used = 0; overflow = false;
        } else if (used < sizeof(line) - 1) line[used++] = bytes[i];
        else overflow = true;
    }
    ldn_udp_poll(connected);
}
