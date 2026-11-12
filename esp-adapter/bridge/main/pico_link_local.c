#include "pico_link.h"

#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "wa_adapter.h"

/* pico_link.h with the wireless adapter on this chip's own pins (the rfu_adapter
   component) in place of a GB-Link on the UART. Both directions are still 'GB' frames:
   commands and RFU1 chunks go straight to the adapter, and its chunks, telemetry and
   status codes come back as the frames a GB-Link would send, so the bridge above is
   unchanged. The host port (the web client carrying the frames for its own adapter)
   works as in pico_link.c. */

#define SYNC0 0x47 /* 'G' */
#define SYNC1 0x42 /* 'B' */
#define HEADER_LENGTH 5
/* Data-channel frames shorter than this are adapter telemetry; RFU1 chunks are this long. */
#define CHUNK_LENGTH 64

typedef struct
{
    uint16_t length;
    uint8_t bytes[HEADER_LENGTH + PICO_LINK_MAX_PAYLOAD];
} pico_frame_t;

typedef enum { RX_SYNC0, RX_SYNC1, RX_CHANNEL, RX_LEN_LO, RX_LEN_HI, RX_PAYLOAD } rx_state_t;

/* One 'GB' byte stream being cut into frames. */
typedef struct
{
    rx_state_t state;
    pico_frame_t partial;
    uint16_t length;
    uint16_t seen;
} parser_t;

#if CONFIG_IDF_TARGET_ESP32
#define FRAME_QUEUE 32
#else
#define FRAME_QUEUE 48
#endif

static TaskHandle_t s_task;
static QueueHandle_t s_inbound;
static QueueHandle_t s_host_out;
/* The adapter takes one writer at a time: the bridge task and the console both write. */
static SemaphoreHandle_t s_adapterLock;
static volatile pico_port_t s_port = PICO_PORT_UART;
static volatile bool s_running;

static parser_t s_hostParser;    /* frames from the client's adapter */
static parser_t s_writeParser;   /* frames written for the adapter on these pins */

static uint32_t s_rxFrames, s_txFrames, s_rxBytes, s_resync, s_dropped;
static uint32_t s_modeSent;
static volatile bool s_awaitMode;
static volatile bool s_wrongCable;
static volatile bool s_gbaActive;
/* The client's adapter reports when its GBA last looked for a group, as GB-Link firmware
   does (the web client's emulator). This chip's adapter is asked directly. */
static volatile bool s_searchReports;
static volatile uint16_t s_searchAge;
static volatile TickType_t s_searchReportAt;
static uint8_t s_firstFrame[HEADER_LENGTH + 16];
static uint8_t s_firstFrameLength;
static uint16_t s_statusLog[8];
static uint8_t s_statusCount;
#define FRAME_LOG_SLOTS 4
#define FRAME_LOG_BYTES 24
static uint8_t s_frameLog[FRAME_LOG_SLOTS][FRAME_LOG_BYTES];
static uint8_t s_frameLogLength[FRAME_LOG_SLOTS];
static uint8_t s_frameLogCount;

/* A frame from the adapter, the one on these pins or the client's. */
static void frame_complete(const pico_frame_t *frame)
{
    ++s_rxFrames;
    if (s_firstFrameLength == 0)
    {
        const uint16_t copy = frame->length < sizeof(s_firstFrame) ? frame->length : sizeof(s_firstFrame);
        memcpy(s_firstFrame, frame->bytes, copy);
        s_firstFrameLength = (uint8_t)copy;
    }
    if (frame->bytes[2] == PICO_LINK_CHANNEL_STATUS && frame->length >= HEADER_LENGTH + 2)
    {
        const uint16_t code = (uint16_t)(frame->bytes[HEADER_LENGTH] |
                                         ((uint16_t)frame->bytes[HEADER_LENGTH + 1] << 8));
        const uint8_t slots = (uint8_t)(sizeof(s_statusLog) / sizeof(s_statusLog[0]));
        s_statusLog[s_statusCount % slots] = code;
        ++s_statusCount;
        if (code == 0xFF02) { s_awaitMode = true; s_wrongCable = false; }
        else if (code == 0xFF0D) { s_wrongCable = true; s_awaitMode = false; }
    }
    if (frame->bytes[2] == PICO_LINK_CHANNEL_DATA && frame->length == HEADER_LENGTH + 3 &&
        frame->bytes[HEADER_LENGTH] == 0x1C)
    {
        /* Search report. Not logged: it would push the adapter's other reports out of the
           four-frame log. */
        s_searchAge = (uint16_t)(frame->bytes[HEADER_LENGTH + 1] | frame->bytes[HEADER_LENGTH + 2] << 8);
        s_searchReportAt = xTaskGetTickCount();
        s_searchReports = true;
    }
    else if (frame->bytes[2] == PICO_LINK_CHANNEL_DATA)
    {
        if (frame->length >= HEADER_LENGTH + 4 && frame->length < HEADER_LENGTH + CHUNK_LENGTH &&
            frame->bytes[HEADER_LENGTH] == 0x0E)
            s_gbaActive = frame->bytes[HEADER_LENGTH + 1] != 0;   /* dbgAnyRx */

        const uint8_t slot = s_frameLogCount % FRAME_LOG_SLOTS;
        const uint8_t copy = frame->length < FRAME_LOG_BYTES ? (uint8_t)frame->length : FRAME_LOG_BYTES;
        memcpy(s_frameLog[slot], frame->bytes, copy);
        s_frameLogLength[slot] = copy;
        ++s_frameLogCount;
    }
    if (!s_inbound) return;
    /* Queue full: drop the oldest frame, keep the newest. */
    if (xQueueSend(s_inbound, frame, 0) != pdTRUE)
    {
        pico_frame_t discard;
        if (xQueueReceive(s_inbound, &discard, 0) == pdTRUE) ++s_dropped;
        if (xQueueSend(s_inbound, frame, 0) != pdTRUE) ++s_dropped;
    }
}

static void make_frame(pico_frame_t *frame, uint8_t channel, const uint8_t *payload, size_t length)
{
    frame->bytes[0] = SYNC0;
    frame->bytes[1] = SYNC1;
    frame->bytes[2] = channel;
    frame->bytes[3] = (uint8_t)(length & 0xff);
    frame->bytes[4] = (uint8_t)(length >> 8);
    if (length) memcpy(frame->bytes + HEADER_LENGTH, payload, length);
    frame->length = (uint16_t)(HEADER_LENGTH + length);
}

static void parse(parser_t *p, uint8_t byte, void (*done)(const pico_frame_t *))
{
    switch (p->state)
    {
    case RX_SYNC0:
        if (byte == SYNC0) { p->partial.bytes[0] = byte; p->state = RX_SYNC1; }
        else ++s_resync;
        return;
    case RX_SYNC1:
        if (byte == SYNC1) { p->partial.bytes[1] = byte; p->state = RX_CHANNEL; }
        else { ++s_resync; p->state = (byte == SYNC0) ? RX_SYNC1 : RX_SYNC0; }
        return;
    case RX_CHANNEL:
        p->partial.bytes[2] = byte; p->state = RX_LEN_LO; return;
    case RX_LEN_LO:
        p->partial.bytes[3] = byte; p->length = byte; p->state = RX_LEN_HI; return;
    case RX_LEN_HI:
        p->partial.bytes[4] = byte;
        p->length |= (uint16_t)byte << 8;
        if (p->length > PICO_LINK_MAX_PAYLOAD) { ++s_resync; p->state = RX_SYNC0; return; }
        p->seen = 0;
        if (p->length == 0) { p->partial.length = HEADER_LENGTH; done(&p->partial); p->state = RX_SYNC0; return; }
        p->state = RX_PAYLOAD;
        return;
    case RX_PAYLOAD:
        p->partial.bytes[HEADER_LENGTH + p->seen++] = byte;
        if (p->seen >= p->length)
        {
            p->partial.length = (uint16_t)(HEADER_LENGTH + p->length);
            done(&p->partial);
            p->state = RX_SYNC0;
        }
        return;
    }
}

/* ---- the adapter on these pins ---------------------------------------------- */

/* Called from the adapter task. Dropped while the client's adapter is in use, as a
   GB-Link's would be with the UART closed. */
static void adapter_frame(uint8_t channel, const uint8_t *payload, size_t length)
{
    if (!s_running || s_port != PICO_PORT_UART || length > PICO_LINK_MAX_PAYLOAD) return;
    pico_frame_t frame;
    make_frame(&frame, channel, payload, length);
    s_rxBytes += frame.length;
    frame_complete(&frame);
}

static void adapter_data(const uint8_t *bytes, size_t length, void *ctx)
{
    (void)ctx;
    adapter_frame(PICO_LINK_CHANNEL_DATA, bytes, length);
}

static void adapter_status(uint16_t code, void *ctx)
{
    (void)ctx;
    const uint8_t payload[2] = {(uint8_t)code, (uint8_t)(code >> 8)};
    adapter_frame(PICO_LINK_CHANNEL_STATUS, payload, sizeof(payload));
}

/* A frame for the adapter, taken as the GB-Link's firmware takes it. Under
   s_adapterLock. */
static void to_adapter(uint8_t channel, const uint8_t *payload, size_t length)
{
    if (channel == PICO_LINK_CHANNEL_DATA)
    {
        wa_adapter_feed(payload, length);
        return;
    }
    /* SetMode(wireless, variant). The GB-Link's other commands do not apply here. */
    if (channel == PICO_LINK_CHANNEL_COMMAND && length >= 2 && payload[0] == 0x00 && payload[1] == 0x07)
        wa_adapter_enter_mode(length >= 3 ? payload[2] : 0);
}

static void written_frame(const pico_frame_t *frame)
{
    to_adapter(frame->bytes[2], frame->bytes + HEADER_LENGTH, frame->length - HEADER_LENGTH);
}

/* ---- link task -------------------------------------------------------------- */

static void send_set_mode(void)
{
    static const uint8_t set_mode_rfu[] = {0x00, 0x07, 0x00};
    if (pico_link_send(PICO_LINK_CHANNEL_COMMAND, set_mode_rfu, sizeof(set_mode_rfu))) ++s_modeSent;
}

/* The mode schedule of pico_link.c, without its silence rule. The adapter on these pins
   is in the mode from power-up and cannot leave it, and a SetMode reaches it only through
   its own task, so going quiet is never answered with a SetMode that would wipe a live
   link. */
static void link_task(void *arg)
{
    (void)arg;
    TickType_t nextMode = xTaskGetTickCount() + pdMS_TO_TICKS(1200);
    bool reported = false;
    while (s_running)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        /* Whether the chip's SPI slave can serve the GBA: the one wiring-independent
           check, made once at power-up. */
        if (!reported && wa_adapter_running())
        {
            wa_stats_t stats;
            wa_adapter_stats(&stats);
            printf("LDN_WA adapter on SC %d SO %d SI %d SD %d, self-test %s: %s\n", CONFIG_WA_PIN_SC,
                   CONFIG_WA_PIN_SO, CONFIG_WA_PIN_SI, CONFIG_WA_PIN_SD, stats.cal.ok ? "passed" : "FAILED",
                   stats.cal.detail);
            reported = true;
        }
        if ((!s_awaitMode && !s_gbaActive) || s_wrongCable)
        {
            const TickType_t now = xTaskGetTickCount();
            if ((int32_t)(now - nextMode) >= 0)
            {
                send_set_mode();
                nextMode = now + pdMS_TO_TICKS(s_rxFrames ? 2000 : 500);
            }
        }
    }
    s_task = NULL;
    vTaskDelete(NULL);
}

void pico_link_start(void)
{
    if (s_running) return;

    if (!s_inbound) s_inbound = xQueueCreate(FRAME_QUEUE, sizeof(pico_frame_t));
    ESP_ERROR_CHECK(s_inbound ? ESP_OK : ESP_ERR_NO_MEM);
    if (!s_adapterLock) s_adapterLock = xSemaphoreCreateMutex();
    ESP_ERROR_CHECK(s_adapterLock ? ESP_OK : ESP_ERR_NO_MEM);

    s_running = true;
    /* Live from power-up, so the GBA finds its adapter before there is a room. */
    const wa_config_t config = {
        .pins = {.sc = CONFIG_WA_PIN_SC, .so = CONFIG_WA_PIN_SO, .si = CONFIG_WA_PIN_SI, .sd = CONFIG_WA_PIN_SD},
        .role = 0,
        .spin_us = CONFIG_WA_HANDSHAKE_SPIN_US,
    };
    const wa_transport_t transport = {.data = adapter_data, .status = adapter_status};
    ESP_ERROR_CHECK(wa_adapter_start(&config, &transport) ? ESP_OK : ESP_ERR_NO_MEM);
#if CONFIG_FREERTOS_NUMBER_OF_CORES > 1
    xTaskCreatePinnedToCore(link_task, "pico_link", 4096, NULL, 10, &s_task, 1);
#else
    xTaskCreate(link_task, "pico_link", 4096, NULL, 10, &s_task);
#endif
}

void pico_link_stop(void)
{
    if (!s_running) return;
    s_running = false;
    while (s_task) vTaskDelay(pdMS_TO_TICKS(2));
}

bool pico_link_running(void) { return s_running; }

bool pico_link_write(const uint8_t *bytes, size_t length)
{
    if (!s_running || !bytes || !length) return false;
    if (s_port == PICO_PORT_HOST)
    {
        /* Queued for the console task, the only wire-layer user. */
        if (length > sizeof(((pico_frame_t *)0)->bytes) || !s_host_out) return false;
        pico_frame_t frame;
        frame.length = (uint16_t)length;
        memcpy(frame.bytes, bytes, length);
        return xQueueSend(s_host_out, &frame, 0) == pdTRUE;
    }
    xSemaphoreTake(s_adapterLock, portMAX_DELAY);
    for (size_t i = 0; i < length; ++i) parse(&s_writeParser, bytes[i], written_frame);
    xSemaphoreGive(s_adapterLock);
    return true;
}

bool pico_link_set_port(pico_port_t port)
{
    if (port == s_port) return true;
    /* Outbound queue for the host port, allocated on first use. */
    if (port == PICO_PORT_HOST && !s_host_out)
    {
        s_host_out = xQueueCreate(FRAME_QUEUE, sizeof(pico_frame_t));
        if (!s_host_out) return false;
    }
    s_port = port;
    /* Adapter state is per port. */
    s_hostParser.state = RX_SYNC0;
    s_awaitMode = false;
    s_wrongCable = false;
    s_gbaActive = false;
    s_searchReports = false;
    s_firstFrameLength = 0;
    pico_link_reset_inbound();
    pico_frame_t discard;
    while (s_host_out && xQueueReceive(s_host_out, &discard, 0) == pdTRUE) { }
    return true;
}

pico_port_t pico_link_port(void) { return s_port; }

void pico_link_feed_host(const uint8_t *bytes, size_t length)
{
    if (s_port != PICO_PORT_HOST) return;
    s_rxBytes += length;
    for (size_t i = 0; i < length; ++i) parse(&s_hostParser, bytes[i], frame_complete);
}

bool pico_link_poll_host_out(uint8_t *frame, size_t capacity, size_t *length)
{
    pico_frame_t next;
    if (!s_host_out || xQueueReceive(s_host_out, &next, 0) != pdTRUE) return false;
    if (next.length > capacity) return false;
    memcpy(frame, next.bytes, next.length);
    *length = next.length;
    return true;
}

bool pico_link_send(uint8_t channel, const uint8_t *payload, size_t length)
{
    if (length > PICO_LINK_MAX_PAYLOAD || !s_running) return false;
    if (s_port == PICO_PORT_HOST)
    {
        pico_frame_t frame;
        make_frame(&frame, channel, payload, length);
        if (!pico_link_write(frame.bytes, frame.length)) return false;
    }
    else
    {
        xSemaphoreTake(s_adapterLock, portMAX_DELAY);
        to_adapter(channel, payload, length);
        xSemaphoreGive(s_adapterLock);
    }
    ++s_txFrames;
    return true;
}

bool pico_link_poll_inbound(uint8_t *frame, size_t capacity, size_t *length)
{
    if (!s_inbound || !frame || !length) return false;
    pico_frame_t held;
    if (xQueueReceive(s_inbound, &held, 0) != pdTRUE) return false;
    if (held.length > capacity) return false;
    memcpy(frame, held.bytes, held.length);
    *length = held.length;
    return true;
}

void pico_link_set_mode(void) { send_set_mode(); }

void pico_link_stats(uint32_t *rx_frames, uint32_t *tx_frames, uint32_t *rx_bytes,
                     uint32_t *resync, uint32_t *dropped)
{
    if (rx_frames) *rx_frames = s_rxFrames;
    if (tx_frames) *tx_frames = s_txFrames;
    if (rx_bytes) *rx_bytes = s_rxBytes;
    if (resync) *resync = s_resync;
    if (dropped) *dropped = s_dropped;
}

uint32_t pico_link_mode_sent(void) { return s_modeSent; }
bool pico_link_await_mode(void) { return s_awaitMode; }
bool pico_link_wrong_cable(void) { return s_wrongCable; }
bool pico_link_gba_active(void) { return s_gbaActive; }

int64_t pico_link_gba_search_age_ms(void)
{
    if (s_port == PICO_PORT_HOST)
    {
        if (!s_searchReports) return -1;
        const uint16_t reported = s_searchAge;
        if (reported == 0xFFFF) return INT64_MAX;
        return (int64_t)reported + (int64_t)pdTICKS_TO_MS(xTaskGetTickCount() - s_searchReportAt);
    }
    const uint32_t age = wa_adapter_search_age_ms();
    return age == UINT32_MAX ? INT64_MAX : (int64_t)age;
}

/* Drop frames buffered before a consumer attached. */
void pico_link_reset_inbound(void)
{
    if (!s_inbound) return;
    pico_frame_t discard;
    while (xQueueReceive(s_inbound, &discard, 0) == pdTRUE) { }
}

/* No separate board to reflash and no UART pair to swap or probe: the adapter's pins
   are wired straight to the link port. */
void pico_link_bootsel(void) { }
void pico_link_swap(void) { }
bool pico_link_swapped(void) { return false; }

void pico_link_probe(bool *tx_driven, bool *rx_driven)
{
    if (tx_driven) *tx_driven = false;
    if (rx_driven) *rx_driven = false;
}

uint8_t pico_link_frame_log(uint8_t slot, uint8_t *out, uint8_t capacity)
{
    if (slot >= FRAME_LOG_SLOTS) return 0;
    const uint8_t held = s_frameLogLength[slot];
    const uint8_t copy = held < capacity ? held : capacity;
    if (out && copy) memcpy(out, s_frameLog[slot], copy);
    return copy;
}

uint8_t pico_link_status_log(uint16_t *out, uint8_t capacity)
{
    const uint8_t slots = (uint8_t)(sizeof(s_statusLog) / sizeof(s_statusLog[0]));
    const uint8_t held = s_statusCount < slots ? s_statusCount : slots;
    const uint8_t copy = held < capacity ? held : capacity;
    for (uint8_t i = 0; i < copy; ++i)
    {
        const uint8_t oldest = (uint8_t)((s_statusCount >= slots ? s_statusCount : 0) % slots);
        out[i] = s_statusLog[(oldest + i) % slots];
    }
    return copy;
}

uint8_t pico_link_first_frame(uint8_t *out, uint8_t capacity)
{
    const uint8_t copy = s_firstFrameLength < capacity ? s_firstFrameLength : capacity;
    if (out && copy) memcpy(out, s_firstFrame, copy);
    return copy;
}
