#pragma once
#include "pia_conn.h"
#include "pia_reliable.h"

/* The Pia session as the GBA relay uses it: reliable stream plumbing, K
   acknowledgements, host polling credits, strict-order delivery, and the emulated
   wireless adapter's W frames (WC connect, WA accepted, WT data, WD disconnect). */

#define PIA_SEEN_BITS 8192
#define PIA_SEEN_RING 1024
#define PIA_HOLD_SLOTS 32
#define PIA_HOLD_BYTES 128
#define PIA_TIME_RING 256
#define PIA_K_QUEUE 32
#define PIA_OUT_SLOTS 64
#define PIA_OUT_BYTES 128

typedef void (*pia_send_fn)(const uint8_t *datagram, size_t length, const char *destination, void *user);
typedef void (*pia_gba_fn)(const uint8_t *payload, size_t length, void *user);
typedef void (*pia_log_fn)(const char *message, void *user);

typedef struct
{
    pia_crypto_t crypto;
    pia_conn_t conn;
    pia_reliable_t reliable;

    char ours[16], host[16];
    pia_send_fn send;
    pia_gba_fn deliver;
    pia_log_fn log;
    void *user;

    uint16_t packet_ids[4];
    uint8_t seen_bits[PIA_SEEN_BITS / 8];
    uint16_t seen_ring[PIA_SEEN_RING];
    int seen_head, seen_count;

    struct { uint16_t seq; uint8_t data[PIA_HOLD_BYTES]; uint16_t length; bool used; } hold[PIA_HOLD_SLOTS];
    int next_deliver;                 /* -1 until the first reliable frame arrives */

    uint32_t time_ring[PIA_TIME_RING];
    int time_head, time_count;
    struct { uint32_t sequence, time; } k_queue[PIA_K_QUEUE];
    int k_head, k_count;
    uint16_t k_inflight[3];
    int k_inflight_count;

    bool opened, connect_sent, ack_owed;
    int credits, last_ack, tick;
    uint64_t nonce;
    uint32_t timestamp, k_sequence;
    uint8_t connect_id[2];

    bool accepted, host_disconnected, connect_wanted;
    int received, decrypt_failures, sent, reordered, hold_dropped;
    int rx_seen, rx_wrong_source, rx_short, rx_bad_frame, rx_messages, rx_unzip_fail;
    uint8_t rx_first[16];
    int rx_first_len, rx_first_zipped, rx_first_pad, rx_first_footer;

    /* Child frames waiting for the Switch. Lossless: each carries a mod-8 sequence
       the parent validates, so a dropped or reordered one desyncs the trade. */
    struct { uint8_t data[PIA_OUT_BYTES]; uint16_t length; } outbound[PIA_OUT_SLOTS];
    int out_head, out_count, high_water, repeated, overflow;
    uint8_t last_enqueued[PIA_OUT_BYTES];
    uint16_t last_enqueued_len;
    uint8_t idle[PIA_OUT_BYTES];
    uint16_t idle_len;
    bool has_idle;
} pia_link_t;

void pia_link_init(pia_link_t *l, const uint8_t ssid[16], const uint8_t our_mac[6],
                   const uint8_t host_mac[6], const char *our_ip, const char *host_ip,
                   pia_send_fn send, pia_gba_fn deliver, pia_log_fn log, void *user);
void pia_link_receive(pia_link_t *l, const uint8_t *data, size_t length, const char *source);
void pia_link_tick(pia_link_t *l);
/* Queue one child payload for the Switch. */
void pia_link_enqueue(pia_link_t *l, const uint8_t *payload, size_t length);
void pia_link_set_connect(pia_link_t *l, bool wanted);
