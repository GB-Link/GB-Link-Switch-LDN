/* Host test of the one-board link (bridge/main/pico_link_local.c) against a scripted
   adapter: the bridge must see what a GB-Link on the UART would give it. Checks when
   SetMode reaches the adapter, the frames both ways, and the host port. Run with
   test/link/run.sh.

   Adapter model, per the rfu_adapter component: it starts on its own task, announces
   AwaitMode, then reports every 500 ms (tag 0x0E, byte 1 = a GBA is clocking it). A mode
   entry is answered with AwaitMode on its next pass. It can stall (no passes). */

#include "../../bridge/main/pico_link_local.c"

#include <stdio.h>
#include <stdlib.h>

static TickType_t g_now, g_until;
static int g_failures;

static void check(bool ok, const char *what)
{
    if (ok) return;
    ++g_failures;
    fprintf(stderr, "FAIL at %u ms: %s\n", (unsigned)g_now, what);
}

/* ---- the adapter */

static struct
{
    bool started, running, stalled, gba, entry_pending;
    wa_transport_t transport;
    unsigned starts, entries;
    uint8_t role;
    TickType_t next_report;
    uint8_t fed[1024];
    size_t fed_length;
    uint32_t search_age;
} g_wa;

bool wa_adapter_start(const wa_config_t *config, const wa_transport_t *transport)
{
    (void)config;
    ++g_wa.starts;
    if (g_wa.started) return true;
    g_wa.started = true;
    g_wa.transport = *transport;
    return true;
}

bool wa_adapter_running(void) { return g_wa.running; }

void wa_adapter_enter_mode(uint8_t role)
{
    ++g_wa.entries;
    g_wa.role = role;
    g_wa.entry_pending = true;
}

void wa_adapter_feed(const uint8_t *bytes, size_t length)
{
    if (g_wa.fed_length + length > sizeof(g_wa.fed)) g_wa.fed_length = 0;
    memcpy(g_wa.fed + g_wa.fed_length, bytes, length);
    g_wa.fed_length += length;
}

void wa_adapter_recalibrate(void) { }

uint32_t wa_adapter_search_age_ms(void) { return g_wa.search_age; }

void wa_adapter_stats(wa_stats_t *out)
{
    memset(out, 0, sizeof(*out));
    out->cal.ok = true;
    snprintf(out->cal.detail, sizeof(out->cal.detail), "scripted");
    out->any_rx = g_wa.gba;
}

static void adapter_status_out(uint16_t code) { g_wa.transport.status(code, g_wa.transport.ctx); }

static void adapter_runs(void)
{
    if (!g_wa.started || g_wa.stalled) return;
    if (!g_wa.running)
    {
        g_wa.running = true;
        adapter_status_out(0xFF02);
        g_wa.next_report = g_now + 500;
    }
    if (g_wa.entry_pending)
    {
        g_wa.entry_pending = false;
        adapter_status_out(0xFF02);
    }
    if ((int32_t)(g_now - g_wa.next_report) < 0) return;
    g_wa.next_report += 500;
    uint8_t report[16] = {0x0E, g_wa.gba ? 1 : 0};
    g_wa.transport.data(report, sizeof(report), g_wa.transport.ctx);
}

/* ---- timeline */

typedef enum { GBA_ON, STALL, RESUME, DATA_CHUNK, CHECK_LINKED, CHECK_ACTIVE, CHECK_QUIET_SINCE } action_t;
typedef struct { TickType_t at; action_t action; TickType_t since; const char *what; } event_t;

static unsigned g_entries_at[8];
static TickType_t g_marks[8];
static unsigned g_mark_count;

static void mark(void)
{
    if (g_mark_count >= 8) return;
    g_marks[g_mark_count] = g_now;
    g_entries_at[g_mark_count++] = g_wa.entries;
}

static unsigned entries_since(TickType_t since)
{
    for (unsigned i = 0; i < g_mark_count; ++i)
        if (g_marks[i] == since) return g_wa.entries - g_entries_at[i];
    return g_wa.entries;
}

static const event_t g_events[] = {
    {1000, CHECK_LINKED, 0, "the adapter is in the mode from power-up"},
    {1000, CHECK_QUIET_SINCE, 0, "an adapter that announced itself is not sent SetMode"},
    {2000, GBA_ON, 0, NULL},
    {3000, CHECK_ACTIVE, 0, "a GBA clocking the adapter shows in its reports"},
    {3100, DATA_CHUNK, 0, NULL},
    {3100, CHECK_ACTIVE, 0, "an RFU1 chunk that starts like a report is not read as one"},
    {20000, CHECK_QUIET_SINCE, 1000, "a working link is left alone"},
    {20000, STALL, 0, NULL},
    {27000, CHECK_QUIET_SINCE, 20000, "a silent adapter is not sent SetMode, which would wipe its link"},
    {27000, RESUME, 0, NULL},
    {28000, CHECK_LINKED, 0, "an adapter that answers again is still in the mode"},
    {40000, CHECK_QUIET_SINCE, 28000, "and then left alone"},
};
static size_t g_next_event;

static void time_passes(TickType_t ticks)
{
    for (TickType_t t = 0; t < (ticks ? ticks : 1); ++t)
    {
        ++g_now;
        adapter_runs();
    }
    while (g_next_event < sizeof(g_events) / sizeof(g_events[0]) && g_events[g_next_event].at <= g_now)
    {
        const event_t *event = &g_events[g_next_event++];
        switch (event->action)
        {
        case GBA_ON: g_wa.gba = true; break;
        case STALL: g_wa.stalled = true; break;
        case RESUME: g_wa.stalled = false; g_wa.next_report = g_now; break;
        case DATA_CHUNK:
        {
            uint8_t chunk[64] = {0x0E, 0x00};
            g_wa.transport.data(chunk, sizeof(chunk), g_wa.transport.ctx);
            break;
        }
        case CHECK_LINKED: check(pico_link_await_mode(), event->what); mark(); break;
        case CHECK_ACTIVE: check(pico_link_gba_active(), event->what); break;
        case CHECK_QUIET_SINCE: check(entries_since(event->since) == 0, event->what); break;
        }
        if (event->action == CHECK_QUIET_SINCE && event->since == 0) mark();
        if (event->action == GBA_ON || event->action == STALL) mark();
    }
    if ((int32_t)(g_now - g_until) >= 0) s_running = false;
}

/* ---- FreeRTOS stubs */

struct host_queue { uint32_t length, item_size, count, head; uint8_t *items; };

QueueHandle_t xQueueCreate(uint32_t length, uint32_t item_size)
{
    QueueHandle_t queue = calloc(1, sizeof(*queue));
    queue->length = length;
    queue->item_size = item_size;
    queue->items = calloc(length, item_size);
    return queue;
}

BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t wait)
{
    (void)wait;
    if (queue->count == queue->length) return pdFALSE;
    memcpy(queue->items + ((queue->head + queue->count++) % queue->length) * queue->item_size, item, queue->item_size);
    return pdTRUE;
}

BaseType_t xQueueReceive(QueueHandle_t queue, void *item, TickType_t wait)
{
    (void)wait;
    if (!queue->count) return pdFALSE;
    memcpy(item, queue->items + queue->head * queue->item_size, queue->item_size);
    queue->head = (queue->head + 1) % queue->length;
    --queue->count;
    return pdTRUE;
}

static int g_mutex;
SemaphoreHandle_t xSemaphoreCreateMutex(void) { return &g_mutex; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t wait)
{
    (void)wait;
    check(*mutex == 0, "the adapter lock is free when taken");
    ++*mutex;
    return pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex)
{
    --*mutex;
    return pdTRUE;
}

TickType_t xTaskGetTickCount(void) { return g_now; }
void vTaskDelay(TickType_t ticks) { time_passes(ticks); }
void vTaskDelete(TaskHandle_t task) { (void)task; }

static void (*g_task)(void *);
BaseType_t xTaskCreate(void (*task)(void *), const char *name, uint32_t stack, void *arg, int priority,
                       TaskHandle_t *handle)
{
    (void)name; (void)stack; (void)arg; (void)priority;
    g_task = task;
    *handle = (TaskHandle_t)&g_task;
    return pdTRUE;
}

/* ---- frames */

static void drain_inbound(void)
{
    uint8_t frame[HEADER_LENGTH + PICO_LINK_MAX_PAYLOAD];
    size_t length;
    while (pico_link_poll_inbound(frame, sizeof(frame), &length)) { }
}

static bool next_inbound(uint8_t *frame, size_t *length)
{
    return pico_link_poll_inbound(frame, HEADER_LENGTH + PICO_LINK_MAX_PAYLOAD, length);
}

static void check_frames(void)
{
    uint8_t frame[HEADER_LENGTH + PICO_LINK_MAX_PAYLOAD];
    size_t length = 0;

    /* Bridge to adapter: a 64-byte RFU1 chunk is fed as it is. */
    uint8_t chunk[64];
    for (int i = 0; i < 64; ++i) chunk[i] = (uint8_t)(i * 7 + 3);
    g_wa.fed_length = 0;
    check(pico_link_send(PICO_LINK_CHANNEL_DATA, chunk, sizeof(chunk)), "a chunk is sent");
    check(g_wa.fed_length == 64 && memcmp(g_wa.fed, chunk, 64) == 0, "the chunk reaches the adapter whole");

    /* Raw frames written in odd pieces: two data frames and a SetMode with a variant. */
    uint8_t stream[2 * (HEADER_LENGTH + 64) + HEADER_LENGTH + 3];
    size_t at = 0;
    for (int f = 0; f < 2; ++f)
    {
        const uint8_t header[HEADER_LENGTH] = {SYNC0, SYNC1, PICO_LINK_CHANNEL_DATA, 64, 0};
        memcpy(stream + at, header, HEADER_LENGTH);
        at += HEADER_LENGTH;
        for (int i = 0; i < 64; ++i) stream[at++] = (uint8_t)(f * 100 + i);
    }
    const uint8_t set_mode[] = {SYNC0, SYNC1, PICO_LINK_CHANNEL_COMMAND, 3, 0, 0x00, 0x07, 0x02};
    memcpy(stream + at, set_mode, sizeof(set_mode));
    at += sizeof(set_mode);
    g_wa.fed_length = 0;
    const unsigned entries = g_wa.entries;
    for (size_t o = 0; o < at; o += 7) check(pico_link_write(stream + o, at - o < 7 ? at - o : 7), "a piece is written");
    bool same = g_wa.fed_length == 128;
    for (int i = 0; same && i < 128; ++i) same = g_wa.fed[i] == (uint8_t)((i / 64) * 100 + i % 64);
    check(same, "written frames reach the adapter in order, without their headers");
    check(g_wa.entries == entries + 1 && g_wa.role == 2, "a written SetMode enters the mode with its variant");

    /* Adapter to bridge: the frames a GB-Link would send. */
    drain_inbound();
    g_wa.transport.data(chunk, sizeof(chunk), g_wa.transport.ctx);
    check(next_inbound(frame, &length) && length == HEADER_LENGTH + 64 && frame[0] == SYNC0 && frame[1] == SYNC1 &&
              frame[2] == PICO_LINK_CHANNEL_DATA && frame[3] == 64 && frame[4] == 0 &&
              memcmp(frame + HEADER_LENGTH, chunk, 64) == 0,
          "an adapter chunk arrives as a data frame");
    adapter_status_out(0xFF05);
    static const uint8_t connected[] = {SYNC0, SYNC1, PICO_LINK_CHANNEL_STATUS, 2, 0, 0x05, 0xFF};
    check(next_inbound(frame, &length) && length == sizeof(connected) && memcmp(frame, connected, length) == 0,
          "a status code arrives as a status frame");

    /* The bridge's own SetMode after a session: a mode entry, answered on the next pass. */
    const unsigned before = g_wa.entries;
    pico_link_set_mode();
    check(g_wa.entries == before + 1 && g_wa.role == 0, "the bridge's SetMode enters the mode");
    drain_inbound();
    adapter_runs();
    static const uint8_t await_mode[] = {SYNC0, SYNC1, PICO_LINK_CHANNEL_STATUS, 2, 0, 0x02, 0xFF};
    check(next_inbound(frame, &length) && length == sizeof(await_mode) && memcmp(frame, await_mode, length) == 0,
          "and is answered with AwaitMode");

    /* The GBA's group searches, as this chip's adapter reports them. */
    g_wa.search_age = UINT32_MAX;
    check(pico_link_gba_search_age_ms() == INT64_MAX, "no search before the GBA's first");
    g_wa.search_age = 1234;
    check(pico_link_gba_search_age_ms() == 1234, "the adapter's search age is passed on");

    /* Host port: the client's adapter. This chip's adapter is neither fed nor heard. */
    check(pico_link_set_port(PICO_PORT_HOST), "the host port opens");
    check(!pico_link_await_mode() && !pico_link_gba_active(), "adapter state starts over on the host port");
    check(pico_link_gba_search_age_ms() == -1, "the client's adapter does not report searches");
    g_wa.fed_length = 0;
    check(pico_link_send(PICO_LINK_CHANNEL_DATA, chunk, sizeof(chunk)), "a chunk is sent to the client");
    check(g_wa.fed_length == 0, "this chip's adapter is not fed on the host port");
    check(pico_link_poll_host_out(frame, sizeof(frame), &length) && length == HEADER_LENGTH + 64 &&
              frame[2] == PICO_LINK_CHANNEL_DATA && memcmp(frame + HEADER_LENGTH, chunk, 64) == 0,
          "the chunk goes to the client as a frame");
    g_wa.transport.data(chunk, sizeof(chunk), g_wa.transport.ctx);
    check(!next_inbound(frame, &length), "this chip's adapter is not heard on the host port");
    static const uint8_t client_report[] = {SYNC0, SYNC1, PICO_LINK_CHANNEL_DATA, 4, 0, 0x0E, 0x01, 0x00, 0x04};
    pico_link_feed_host(await_mode, 3);
    pico_link_feed_host(await_mode + 3, sizeof(await_mode) - 3);
    pico_link_feed_host(client_report, sizeof(client_report));
    check(pico_link_await_mode() && pico_link_gba_active(), "the client's adapter reports are read");
    check(next_inbound(frame, &length) && memcmp(frame, await_mode, sizeof(await_mode)) == 0,
          "the client's frames reach the bridge");

    /* Back on the pins. */
    check(pico_link_set_port(PICO_PORT_UART), "back to this chip's adapter");
    g_wa.transport.data(chunk, sizeof(chunk), g_wa.transport.ctx);
    check(next_inbound(frame, &length) && length == HEADER_LENGTH + 64, "this chip's adapter is heard again");
    check(g_mutex == 0, "the adapter lock is balanced");
}

int main(void)
{
    g_until = 40000;
    pico_link_start();
    check(g_wa.starts == 1, "the adapter is started with the link");
    g_task(NULL);
    check(g_next_event == sizeof(g_events) / sizeof(g_events[0]), "the whole timeline ran");
    printf("timeline: %u mode entries\n", g_wa.entries);

    s_running = true;
    check_frames();
    uint32_t rx, tx, bytes, resync, dropped;
    pico_link_stats(&rx, &tx, &bytes, &resync, &dropped);
    check(resync == 0, "no bytes were skipped");
    printf("frames: %u in, %u out, %u dropped\n", (unsigned)rx, (unsigned)tx, (unsigned)dropped);

    printf(g_failures ? "%d FAILED\n" : "all checks pass\n", g_failures);
    return g_failures ? 1 : 0;
}
