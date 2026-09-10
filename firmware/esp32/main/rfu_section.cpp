#include "rfu_section.h"

#include "rfu_link.h"
#include "rfu_protocol.hpp"

#include <cstring>

#include "esp_random.h"
#include "esp_rom_sys.h"
#include "esp_cpu.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

// Glue between the vendored RFU adapter core and this firmware: it owns the
// core, drives the GBA link, and moves RFU1 frames to and from the host.
// Ported from the Pico firmware's rfuProtocolSection.cpp, keeping the parts
// that encode hardware-proven behaviour and dropping its USB/LED plumbing.

namespace
{

rfuproto::RfuCore g_core;
rfuproto::Rfu1StreamParser g_parser;
rfu_section_emit_t g_emit;
void *g_emit_user;
struct OutFrame { uint16_t len; uint8_t data[104]; };
QueueHandle_t g_outbound;
TaskHandle_t g_task;
volatile bool g_cancel;
uint32_t g_net_in, g_net_out;

// Inbound frames are parked and applied only between GBA commands: applying one
// mid-command reorders the adapter's view of the exchange.
struct Parked
{
    uint32_t ptype, hdata;
    uint8_t payload[104];
    uint32_t payloadLen;
};
constexpr uint8_t kParkMax = 8;
Parked g_parked[kParkMax];
volatile uint8_t g_parkedCount;
portMUX_TYPE g_lock = portMUX_INITIALIZER_UNLOCKED;

inline uint32_t now_ms() { return (uint32_t)(esp_timer_get_time() / 1000); }

// Poll the GBA's SO line for a level, timed off the CPU cycle counter so a
// disabled flash cache cannot skew the budget.
bool waitGbaSo(bool level, uint32_t timeout_us)
{
    const uint32_t start = esp_cpu_get_cycle_count();
    const uint32_t budget = timeout_us * (uint32_t)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
    do
    {
        if (rfu_link_gba_line_high() == level) return true;
    } while (esp_cpu_get_cycle_count() - start < budget);
    return rfu_link_gba_line_high() == level;
}

uint16_t rand16(void *) { return (uint16_t)esp_random(); }

void emitFrame(void *, const uint8_t *frame, size_t len)
{
    ++g_net_out;
    if (g_emit) g_emit(frame, len, g_emit_user);
    if (g_outbound && len <= sizeof(OutFrame::data))
    {
        OutFrame out;
        out.len = (uint16_t)len;
        std::memcpy(out.data, frame, len);
        // Never block the adapter task on a slow host; a dropped frame is
        // recoverable, a stalled link is not.
        xQueueSend(g_outbound, &out, 0);
    }
}

// A command may only be applied while the adapter is not mid-exchange.
bool safeToApplyInbound(uint8_t comstate)
{
    return comstate == rfuproto::comWaitCmd || comstate == rfuproto::comIdWait;
}

// A decoded inbound frame. Parked rather than applied here: applying one while
// the GBA is mid-command reorders the adapter's view of the exchange.
void onInboundFrame(void *, uint32_t ptype, uint32_t hdata,
                    const uint8_t *payload, uint16_t payloadLen)
{
    ++g_net_in;
    taskENTER_CRITICAL(&g_lock);
    if (g_parkedCount < kParkMax)
    {
        Parked &slot = g_parked[g_parkedCount];
        slot.ptype = ptype;
        slot.hdata = hdata;
        slot.payloadLen = payloadLen > sizeof(slot.payload) ? (uint32_t)sizeof(slot.payload) : payloadLen;
        if (slot.payloadLen) std::memcpy(slot.payload, payload, slot.payloadLen);
        g_parkedCount = (uint8_t)(g_parkedCount + 1);
    }
    taskEXIT_CRITICAL(&g_lock);
}

void onSlaveWord(uint32_t rx, void *)
{
    g_core.onSlaveTransfer(rx, now_ms());
    rfu_link_push_tx(g_core.stagedTx);
}

// Clock the staged wait-response into the GBA as bus master. This mirrors the
// Pico's runDelivery: the GBA arms itself as an external-clock slave after the
// WAIT-class ACK and counts EVERY SC edge as a data bit, so the role handover
// and the inter-word handshake have to follow librfu's slave ISR exactly.
void runDelivery()
{
    uint32_t words[8];  // the core never stages more than a few words per response
    uint8_t n;
    taskENTER_CRITICAL(&g_lock);
    if (g_core.comstate != rfuproto::comWaitResp) { taskEXIT_CRITICAL(&g_lock); return; }
    n = g_core.eventWordCount();
    if (n > (uint8_t)(sizeof(words) / sizeof(words[0]))) n = (uint8_t)(sizeof(words) / sizeof(words[0]));
    for (uint8_t i = 0; i < n; i++) words[i] = g_core.eventWord(i);
    taskEXIT_CRITICAL(&g_lock);

    // Hold the slave role briefly so the command-phase SI mirror finishes
    // serving the GBA's post-ACK handshake_wait and its arm sequence. There is
    // no observable "armed" level to gate on: the GBA's SO rests low until it
    // is clocked.
    esp_rom_delay_us(150);

    taskENTER_CRITICAL(&g_lock);
    if (g_core.comstate != rfuproto::comWaitResp) { taskEXIT_CRITICAL(&g_lock); return; }
    rfu_link_set_role(RFU_ROLE_ADAPTER_MASTER);
    taskEXIT_CRITICAL(&g_lock);

    // Word-1 inverted-ACK lead-in: the GBA may answer our SI-low with an
    // SO-high pulse. On real hardware it usually does not before word 1, so
    // this poll is bounded and timing out is the normal path.
    if (waitGbaSo(true, 120))
    {
        rfu_link_master_drive_si(true);                       // its handshake_wait(1)
        waitGbaSo(false, 4000);                               // it drops SO when ready
        rfu_link_master_drive_si(false);
    }
    esp_rom_delay_us(45);                                     // >=40us settle

    bool ok = true;
    for (uint8_t i = 0; ok && i < n; i++)
    {
        uint32_t rx = 0;
        ok = rfu_link_master_exchange(words[i], &rx, 2000);
        if (!ok) break;

        if (i + 1 < n)
        {
            // Between words the GBA re-arms with two back-to-back SIOCNT writes
            // and presents the staged word's MSB; there is no observable
            // "re-armed" level to wait on, so a fixed settle covers its bounded
            // path (worst-case TM0-proximity spin plus a few stores).
            esp_rom_delay_us(250);
        }
        else
        {
            // Final word: the GBA drops SO and keeps it low, then re-drives SC
            // as bus master within tens of microseconds. Wait out its handshake
            // generously -- a 4ms budget expired during the post-trade save and
            // stranded librfu mid-delivery (fw 2.4.19).
            ok = waitGbaSo(false, 60000);
        }
    }

    taskENTER_CRITICAL(&g_lock);
    if (ok) g_core.finishDelivery(); else g_core.abortDelivery();
    rfu_link_set_role(RFU_ROLE_GBA_MASTER);
    rfu_link_push_tx(g_core.stagedTx);
    taskEXIT_CRITICAL(&g_lock);
}

void linkTask(void *)
{
    // Reset here, not in rfu_section_start: RfuCore::reset() assigns
    // default-constructed HostState/ClientState, and ClientState alone holds a
    // PktQueue<128,32> -- a ~4 KB temporary. Doing that on the caller's stack
    // overflowed the 3584-byte main task and rebooted the chip. This task's
    // stack is sized for it.
    g_core.reset();

    rfu_link_set_done_callback(onSlaveWord, nullptr);
    rfu_link_enable();
    rfu_link_push_tx(g_core.stagedTx);

    uint32_t lastYield = 0, lastHousekeep = 0;
    while (!g_cancel)
    {
        // Service the wire relentlessly. Any gap is a blind window: a word is
        // 16 us at 2 MHz, and missing its first falling edge puts us one bit out
        // of step for the rest of it (which shows up as a timeout on bit 31).
        // Housekeeping is therefore batched as coarsely as the protocol allows,
        // and esp_timer_get_time() stays out of the inner path -- it contends a
        // cross-core lock and starved the other core's console when called hot.
        // Watch the wire until the GBA is provably between words. Housekeeping
        // anywhere else costs word starts: miss a word's first falling edge and
        // we are one bit out of step for the rest of it, which shows up as a
        // timeout near bit 31.
        //
        // But it must never be starved outright. SD-reset detection and tick()
        // live below, so skipping them indefinitely means a GBA reset is never
        // noticed and the adapter can never re-run detection -- the link just
        // stays dead. Prefer a gap; take one anyway once overdue.
        // ALWAYS watch the wire for a full batch. (An earlier version made this
        // loop conditional on not being in an idle gap, which inverted the
        // intent: at rest the gap is always true, so the poll never ran and the
        // adapter stopped seeing the GBA entirely.)
        for (int i = 0; i < 4096; ++i) rfu_link_poll();
        const uint32_t now = now_ms();
        // Then housekeep only when the GBA is provably between words -- or when
        // overdue, since SD-reset detection and tick() live below and starving
        // them leaves the adapter permanently stuck after a GBA reset.
        if (!rfu_link_idle_gap() && now - lastHousekeep < 4) continue;
        lastHousekeep = now;

        // The game pulsed SD (AgbRFU_SoftReset): drop all adapter state, the
        // same signal a real adapter resets on. This catches resets in every
        // state, including mid-payload.
        if (rfu_link_sd_reset_seen())
        {
            taskENTER_CRITICAL(&g_lock);
            g_core.onSdReset();
            taskEXIT_CRITICAL(&g_lock);
        }

        if (g_parkedCount > 0 && safeToApplyInbound(g_core.comstate))
        {
            taskENTER_CRITICAL(&g_lock);
            const uint8_t cnt = g_parkedCount;
            for (uint8_t i = 0; i < cnt; i++)
                g_core.applyNetPacket(g_parked[i].ptype, g_parked[i].hdata,
                                      g_parked[i].payload, g_parked[i].payloadLen, now);
            for (uint8_t i = cnt; i < g_parkedCount; i++) g_parked[i - cnt] = g_parked[i];
            g_parkedCount = (uint8_t)(g_parkedCount - cnt);
            taskEXIT_CRITICAL(&g_lock);
        }

        if (g_core.pollWaitEvent(now)) runDelivery();
        g_core.tick(now);

        // No vTaskDelay: this task owns the core, and one FreeRTOS tick of sleep
        // would lose ~60 words. The idle-task watchdog for this core is disabled
        // in sdkconfig.defaults to allow that.
        (void)lastYield;
    }

    rfu_link_disable();
    g_task = nullptr;
    vTaskDelete(nullptr);
}

}  // namespace

extern "C" {

void rfu_section_set_emit(rfu_section_emit_t cb, void *user)
{
    g_emit_user = user;
    g_emit = cb;
}

void rfu_section_start(int core_id)
{
    if (g_task) return;
    if (!g_outbound) g_outbound = xQueueCreate(32, sizeof(OutFrame));
    g_cancel = false;
    g_core.rand16 = rand16;
    g_core.emit = emitFrame;
    g_core.emitCtx = nullptr;
    g_parser.setCallback(onInboundFrame, nullptr);
    // 16 KB: reset() and onSdReset() build multi-kilobyte temporaries on the
    // stack, and the delivery path nests on top of them. Priority is high but
    // not maximal -- it must not outrank the Wi-Fi and timer machinery it shares
    // the chip with.
    xTaskCreatePinnedToCore(linkTask, "rfu_link", 16384, nullptr,
                            configMAX_PRIORITIES - 8, &g_task, core_id);
}

void rfu_section_stop(void) { g_cancel = true; }

void rfu_section_feed(const uint8_t *data, size_t len)
{
    g_parser.push(data, len);
}

void rfu_section_feed_bytes(const uint8_t *frame, size_t length) { rfu_section_feed(frame, length); }

void rfu_section_state(bool *enabled, int *role, uint8_t *levels, uint8_t *comstate)
{
    if (enabled) *enabled = rfu_link_enabled();
    if (role) *role = (int)rfu_link_get_role();
    if (levels) *levels = rfu_link_levels();
    if (comstate) *comstate = (uint8_t)g_core.comstate;
}

bool rfu_section_poll_outbound(uint8_t *buffer, size_t capacity, size_t *length)
{
    if (!g_outbound) return false;
    OutFrame out;
    if (xQueueReceive(g_outbound, &out, 0) != pdTRUE) return false;
    if (out.len > capacity) return false;
    std::memcpy(buffer, out.data, out.len);
    if (length) *length = out.len;
    return true;
}

void rfu_section_stats(uint32_t *slave_words, uint32_t *master_words,
                       uint32_t *partial, uint32_t *net_in, uint32_t *net_out)
{
    rfu_link_stats(slave_words, master_words, partial);
    if (net_in) *net_in = g_net_in;
    if (net_out) *net_out = g_net_out;
}

}
