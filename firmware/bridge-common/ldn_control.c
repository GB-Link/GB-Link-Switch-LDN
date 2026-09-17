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
#include "usb_transport.h"
#include "ldn_control.h"
#include "ldn_udp.h"
#include "ldn_session.h"
#include "ldn_wire.h"
#include "pico_link.h"
#include "ldn_keys.h"
#include "pia_bridge.h"
#include "trade_shim.h"
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

/* Host frames reach the Pico verbatim: both ends speak the same 'GB' framing,
   so nothing here needs to understand the adapter protocol. */
static void host_to_pico(const uint8_t *frame, size_t length)
{
    pico_link_write(frame, length);
}

void ldn_control_init(esp_netif_t *netif, const unsigned char host[6])
{
    s_netif = netif;
    memcpy(s_host, host, 6);
    ldn_udp_init(netif, host);
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, s_mac));
    s_rx = xQueueCreate(8, sizeof(control_frame_t));
    ESP_ERROR_CHECK(s_rx ? ESP_OK : ESP_ERR_NO_MEM);
    usb_transport_init();
    ESP_ERROR_CHECK(esp_wifi_set_tx_done_cb(tx_done));
    printf("LDN_S3_READY transport=usb-serial-jtag heap=%" PRIu32 "\n", esp_get_free_heap_size());
    /* Bring the Pico link up automatically: a host closing the port resets the
       chip, and a manual start does not survive that -- the GBA then sees its
       adapter disappear. Harmless when no Pico is attached. */
    ldn_wire_set_rfu_handler(host_to_pico);
    pico_link_start();
    trade_shim_boot((int)esp_reset_reason());
    /* Standalone by default: a host closing the USB port resets this chip, and a
       manually started bridge does not survive that -- the GBA then sees the room
       disappear. LDN_BRIDGE_STOP hands control back for the PC-relay path. */
    pia_bridge_start();
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

static void emit_line(const char *line) { printf("%s\n", line); }
static bool s_dumping_log;

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
    /* Pico link: the companion board owns the GBA cable and speaks the same
       framed transport, so this side only starts, stops and counts it. */
    if (!strcmp(line, "LDN_PICO_START")) {
        ldn_wire_set_rfu_handler(host_to_pico);
        pico_link_start();
        printf("LDN_PICO_STARTED\n"); return;
    }
    /* Key provisioning. Values are written straight to NVS and never echoed, logged
       or returned -- only which names are present is observable. */
    if (!strncmp(line, "LDN_KEY ", 8)) {
        char name[40]; char hex[40];
        if (sscanf(line + 8, "%39s %39s", name, hex) != 2 || strlen(hex) != 32) {
            printf("LDN_KEY_BAD format\n"); return;
        }
        uint8_t value[16];
        for (int i = 0; i < 16; ++i) {
            int hi = nibble(hex[2 * i]), lo = nibble(hex[2 * i + 1]);
            if (hi < 0 || lo < 0) { printf("LDN_KEY_BAD hex\n"); return; }
            value[i] = (uint8_t)(hi << 4 | lo);
        }
        const bool ok = ldn_keys_store(name, value);
        memset(value, 0, sizeof(value));
        memset(hex, 0, sizeof(hex));
        printf(ok ? "LDN_KEY_OK %s\n" : "LDN_KEY_BAD name\n", name);
        return;
    }
    if (!strcmp(line, "LDN_KEYS")) {
        bool kek = false, gen = false, m00 = false, m12 = false;
        ldn_keys_status(&kek, &gen, &m00, &m12);
        printf("LDN_KEYS kek=%d gen=%d master00=%d master12=%d protocol1=%d protocol3=%d\n",
               kek, gen, m00, m12, ldn_keys_supports(1), ldn_keys_supports(3));
        return;
    }
    if (!strcmp(line, "LDN_KEYS_ERASE")) { ldn_keys_erase(); printf("LDN_KEYS_ERASED\n"); return; }
    if (!strcmp(line, "LDN_BRIDGE_START")) { pia_bridge_start(); printf("LDN_BRIDGE_STARTED\n"); return; }
    if (!strcmp(line, "LDN_BRIDGE_STOP")) { pia_bridge_stop(); printf("LDN_BRIDGE_STOPPED\n"); return; }
    if (!strcmp(line, "LDN_SHIM_LOG")) { trade_shim_dump_begin(); s_dumping_log = true; return; }
    if (!strcmp(line, "LDN_BRIDGE_STATUS")) {
        /* Long enough for the link counters and the shim's own after them: the shim's
           half is appended only if it fits whole, so a buffer that merely overflows
           drops the trade state silently. */
        static char report[768];
        pia_bridge_status(report, sizeof(report));
        printf("LDN_BRIDGE_STATUS %s\n", report);
        return;
    }
    if (!strcmp(line, "LDN_PIA_TEST")) {
        extern int pia_selftest(char *out, size_t cap);
        static char report[192];
        pia_selftest(report, sizeof(report));
        printf("LDN_PIA_TEST %s\n", report);
        return;
    }
    if (!strcmp(line, "LDN_PICO_BOOTSEL")) { pico_link_bootsel(); printf("LDN_PICO_BOOTSEL_SENT\n"); return; }
    if (!strcmp(line, "LDN_PICO_DATA")) {
        for (uint8_t slot = 0; slot < 4; ++slot) {
            uint8_t frame[24];
            const uint8_t n = pico_link_frame_log(slot, frame, sizeof(frame));
            if (!n) continue;
            printf("LDN_PICO_DATA slot=%u", slot);
            for (uint8_t i = 0; i < n; ++i) printf(" %02x", frame[i]);
            printf("\n");
        }
        return;
    }
    if (!strcmp(line, "LDN_PICO_STATUS")) {
        uint16_t codes[8];
        const uint8_t n = pico_link_status_log(codes, 8);
        printf("LDN_PICO_STATUS count=%u", n);
        for (uint8_t i = 0; i < n; ++i) printf(" %04x", codes[i]);
        printf("\n");
        return;
    }
    if (!strcmp(line, "LDN_PICO_SWAP")) {
        pico_link_swap();
        printf("LDN_PICO_SWAPPED tx=%d rx=%d\n",
               pico_link_swapped() ? PICO_LINK_PIN_RX : PICO_LINK_PIN_TX,
               pico_link_swapped() ? PICO_LINK_PIN_TX : PICO_LINK_PIN_RX);
        return;
    }
    if (!strcmp(line, "LDN_PICO_PROBE")) {
        bool tx = false, rx = false;
        pico_link_probe(&tx, &rx);
        printf("LDN_PICO_PROBE pin%d_driven=%d pin%d_driven=%d\n",
               PICO_LINK_PIN_TX, tx ? 1 : 0, PICO_LINK_PIN_RX, rx ? 1 : 0);
        return;
    }
    if (!strcmp(line, "LDN_PICO_MODE")) { pico_link_set_mode(); printf("LDN_PICO_MODE_SENT\n"); return; }
    if (!strncmp(line, "LDN_ANTENNA ", 12)) {
        const bool external = line[12] == '1';
        const bool took = bridge_board_antenna(external);
        printf("LDN_ANTENNA %s %s rssi=%d\n", external ? "external" : "onboard",
               took ? "set" : "unsupported", ldn_probe_last_rssi());
        return;
    }
    if (!strcmp(line, "LDN_RF")) {
        int count, average, low, high;
        ldn_probe_rssi_window(&count, &average, &low, &high);
        printf("LDN_RF frames=%d avg=%d min=%d max=%d last=%d\n",
               count, average, low, high, ldn_probe_last_rssi());
        return;
    }
    if (!strcmp(line, "LDN_PICO_STOP")) { pico_link_stop(); printf("LDN_PICO_STOPPED\n"); return; }
    if (!strcmp(line, "LDN_PICO_STATS")) {
        uint32_t rx = 0, tx = 0, bytes = 0, resync = 0, dropped = 0;
        pico_link_stats(&rx, &tx, &bytes, &resync, &dropped);
        printf("LDN_PICO_STATS running=%d rx_frames=%" PRIu32 " tx_frames=%" PRIu32
               " rx_bytes=%" PRIu32 " resync=%" PRIu32 " dropped=%" PRIu32 "\n",
               pico_link_running() ? 1 : 0, rx, tx, bytes, resync, dropped);
        printf("LDN_PICO_MODE set_mode_sent=%" PRIu32 " await_mode=%d wrong_cable=%d\n",
               pico_link_mode_sent(), pico_link_await_mode() ? 1 : 0, pico_link_wrong_cable() ? 1 : 0);
        printf("LDN_PICO_GBA active=%d\n", pico_link_gba_active() ? 1 : 0);
        uint8_t first[21];
        const uint8_t n = pico_link_first_frame(first, sizeof(first));
        if (n) {
            printf("LDN_PICO_FIRST");
            for (uint8_t i = 0; i < n; ++i) printf(" %02x", first[i]);
            printf("\n");
        }
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
            atomic_load(&s_dropped), usb_transport_dropped(), esp_get_free_heap_size());
        return;
    }
    if (strncmp(line, "LDN_TX ", 7)) { printf("LDN_ERROR UNKNOWN_COMMAND\n"); return; }
    static uint8_t body[FRAME_MAX];
    const char *hex = line + 7;
    size_t n = strlen(hex);
    int result = ESP_ERR_INVALID_ARG;
    if (n < 10 || n > 3000 || n % 2) goto done;
    for (size_t i = 0; i < n / 2; ++i) {
        int hi = nibble(hex[2 * i]), lo = nibble(hex[2 * i + 1]);
        if (hi < 0 || lo < 0) goto done;
        body[i] = (uint8_t)((hi << 4) | lo);
    }
    result = ldn_control_tx_action(body, n / 2);
done:
    printf("LDN_TX_RESULT %d\n", result);
}

int ldn_control_tx_action(const uint8_t *body, size_t length)
{
    static const uint8_t prefix[] = {0x00, 0x22, 0xaa, 0x01, 0x02};
    if (length < 5 || length + 14 > FRAME_MAX || memcmp(body, prefix, sizeof(prefix))) return ESP_ERR_INVALID_ARG;
    if (!atomic_load(&s_connected)) return ESP_ERR_INVALID_STATE;
    static uint8_t frame[FRAME_MAX];
    memcpy(frame, s_host, 6);
    memcpy(frame + 6, s_mac, 6);
    frame[12] = 0x88; frame[13] = 0xb7;
    memcpy(frame + 14, body, length);
    return esp_wifi_internal_tx(WIFI_IF_STA, frame, 14 + length);
}

bool ldn_control_connected(void) { return atomic_load(&s_connected); }
void ldn_control_mac(uint8_t out[6]) { memcpy(out, s_mac, 6); }

static void (*s_action_handler)(const uint8_t source[6], const uint8_t *body, size_t length);
void ldn_control_set_action_handler(void (*handler)(const uint8_t source[6], const uint8_t *body, size_t length))
{
    s_action_handler = handler;
}

void ldn_control_poll(void)
{
    /* A few lines per pass: printing hundreds at once holds this loop, and the bridge
       with it, for as long as the console takes to drain. */
    if (s_dumping_log && !trade_shim_dump_step(emit_line, 8)) s_dumping_log = false;
    /* Hand queued adapter frames to the host from THIS task: the wire layer
       serialises through static buffers and the adapter runs on another core. */
    if (ldn_wire_active() && !pia_bridge_running()) {
        /* Anything queued before the host attached describes a state that has since
           moved on; a replayed WrongCable would abort a session that is now fine. */
        static bool flushed;
        if (!flushed) { pico_link_reset_inbound(); flushed = true; }
        static uint8_t pico_frame[5 + PICO_LINK_MAX_PAYLOAD];
        size_t pico_len = 0;
        for (int i = 0; i < 32 && pico_link_poll_inbound(pico_frame, sizeof(pico_frame), &pico_len); ++i)
            ldn_wire_send_rfu(pico_frame, pico_len);
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
        if (s_action_handler) s_action_handler(frame.bytes + 6, frame.bytes + 14, n);
        else printf("LDN_RX " MACSTR " %s\n", MACARGS(frame.bytes + 6), hex);
    }
    static char line[3016];
    static size_t used;
    static bool overflow;
    static uint8_t bytes[8192];
    int n = usb_transport_read(bytes, sizeof(bytes));
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
