#include "wa_adapter.h"

#include <cstring>
#include <new>

#include "rfuProtocol.hpp"

// The glue around rfuproto::RfuCore, after the GB-Link's rfuProtocolSection.cpp. The
// GBA side runs on the ESP32's SPI slave and GPIO (wa_port.h) instead of PIO.
//
// Words the GBA clocks. After each one the GBA's librfu handshake (librfu_intr.c,
// sio32intr_clock_master) waits for SI high, raises SO, waits for SI low and only then
// clocks the next word. The word interrupt holds SI high, runs the core, and spins a
// few microseconds for that SO. Once it rises the next word is armed with lead bit 0,
// which pulls SI low and lets the GBA go: the reply is in the shifter before the GBA
// can clock it, so no deadline is involved. When SO does not rise (the ID exchange,
// a restarted command) the word is armed with lead bit 1 and an SO-rise interrupt
// does the release later. The first word of each command needs no handshake: the arm
// made at the end of the previous command is waiting for it.
//
// Words the adapter clocks (event delivery after a WAIT-class command) are bit-banged
// with the same handshake the GB-Link runs. The last one ends by arming the next
// GBA-clocked word with lead bit 1: that SI high is what lets the GBA leave slave
// mode, so its next command always finds the shifter armed.
//
// Context map, all core access under the port lock:
//   word / SO-rise interrupts   FSM step, stage and arm the next word
//   SD interrupt                latch only
//   feed (network side)         parse RFU1, park frames for the adapter task
//   adapter task                apply parked frames, waits and delivery, upkeep,
//                               outbound chunks, telemetry

namespace
{

using namespace rfuproto;

// The core and the two frame queues (~13 KB) come from the heap when the adapter starts:
// a board whose GB-Link is the adapter never starts this one.
RfuCore* g_core = nullptr;
Rfu1StreamParser g_parser;
wa_transport_t g_transport;
wa_config_t g_config;
wa_cal_t g_cal;
bool g_started = false;
volatile bool g_running = false;
volatile bool g_recalibrate = false;
volatile bool g_enterMode = false;
volatile uint8_t g_enterRole = 0;

enum Wire : uint8_t
{
    wireOff,
    wireHeld,       // armed, lead 1: SI high until the GBA clocks
    wireReleased,   // armed, lead 0: SI low, the GBA may clock
    wireMaster      // the adapter drives SC
};
volatile Wire g_wire = wireOff;
volatile uint32_t g_armedReply = 0;

// AgbRFU_SoftReset drives SD low, high for ~1.1 ms, then low until its ID exchange
// starts a few microseconds later; the rest of the time the GBA holds SD high. So a reset
// is two falls about 1.1 ms apart. The lows are too short to read SD's level from the
// interrupt, only the falls are timed, and an unqualified reset would wipe live link
// state.
constexpr uint32_t kSdPulseMinUs = 500;
constexpr uint32_t kSdPulseMaxUs = 5000;
volatile bool g_sdLatched = false;
// Any fall of SD while the adapter drives SC: the GBA has begun a reset, and once it
// starts its ID exchange it drives SC itself, so the clock is handed back at once.
volatile bool g_sdFell = false;
volatile uint32_t g_sdFallUs = 0;
// SD the GBA is not holding can chatter at hundreds of kilohertz, most of all while the
// self-test moves SI, and an interrupt per fall would take the whole core. Falls this
// close together are never a reset's, so a run of them mutes the interrupt until the
// line has had time to settle.
constexpr uint32_t kSdChatterUs = 100;
constexpr uint8_t kSdChatterRun = 8;
constexpr uint32_t kSdMuteMs = 50;
volatile uint8_t g_sdChatter = 0;
volatile bool g_sdMuted = false;

// How long the GBA may take to service one delivery word. Its slave ISR normally
// answers in microseconds, but a game writing its save holds the handler off for a
// whole flash sector erase. librfu waits ~6 s.
constexpr uint32_t kGbaHandshakeUs = 60000;
constexpr uint32_t kMaxRetriesPerWait = 400;
constexpr uint8_t kMaxEventWords = 24;
uint32_t g_retriesThisWait = 0;

// Where the last unsuccessful delivery failed: low nibble stage, high nibble word.
enum DelivFail : uint8_t
{
    dfNone = 0,
    dfHandshakeWait = 1,   // the GBA never finished its WAIT handshake
    dfDanceSoHigh = 3,     // the GBA never raised SO after a word
    dfFinalSoLow = 4,      // the GBA never dropped SO after the last word
    dfErrResponse = 5,     // the GBA rejected the event (0x996601EE)
    dfReset = 6,           // the GBA reset the adapter mid-delivery
};
volatile uint8_t g_dbgDelivFail = dfNone;
volatile uint8_t g_dbgDelivSo = 0;
volatile uint32_t g_revRx0 = 0, g_revRx1 = 0;

struct Counters
{
    uint32_t words, badBits, realigns, releasedSpin, releasedIrq, held;
    uint32_t firstWords[8];
    uint8_t firstCount;
    uint32_t deliveries, deliveryFails, sdResets, finalSoStuck;
    uint32_t framesIn, framesOut;
    uint32_t onesMismatches;
};
Counters g_ct = {};

// ---- outbound: whole RFU1 frames, appended under the lock ----------------------

constexpr size_t kOutRing = 2048;
uint8_t* g_out = nullptr;
size_t g_outHead = 0, g_outCount = 0;
uint16_t g_outRingDrops = 0;
uint8_t g_chunk[64];
size_t g_chunkLen = 0;

void emitFrame(void*, const uint8_t* frame, size_t len)
{
    // Called by the core, so under the lock. All or nothing: a partial frame would
    // corrupt the stream.
    if (kOutRing - g_outCount < len)
    {
        g_outRingDrops = g_outRingDrops + 1;
        return;
    }
    for (size_t i = 0; i < len; i++)
        g_out[(g_outHead + g_outCount + i) % kOutRing] = frame[i];
    g_outCount += len;
    g_ct.framesOut++;
}

uint16_t rand16(void*) { return wa_port_random16(); }

// ---- inbound: parked until the adapter task can apply them ---------------------

struct ParkedFrame
{
    uint32_t ptype;
    uint32_t hdata;
    uint16_t payloadLen;
    uint8_t payload[92];
};
constexpr uint8_t kParkedMax = 32;
ParkedFrame* g_parked = nullptr;
volatile uint8_t g_parkedCount = 0;
uint16_t g_parkedDrops = 0;       // command-carrying frames lost
uint16_t g_parkedIdleDrops = 0;   // idle state repeats not kept
uint8_t g_parkedHigh = 0;         // per-report high-water
uint8_t g_parkedHighEver = 0;
volatile uint32_t g_lastNetRxMs = 0;

// Frames are applied while the GBA is between commands or is the bus slave, as on
// the GB-Link, so a command's words and its reply see one consistent state.
bool safeToApplyInbound(uint8_t comstate)
{
    return comstate == comWaitCmd || comstate == comWaitEvent || comstate == comWaitResp;
}

// An idle parent UNI frame is a "current state" repeat: only the newest is kept, and
// a run of them must not crowd out a frame carrying a command, which the parent sends
// only once.
bool idleParentState(uint32_t ptype, const uint8_t* payload, uint16_t len)
{
    return ptype == NET_HOST_SEND && RfuCore::isParentUni(payload, len) &&
           !RfuCore::parentUniHasCommand(payload, len);
}

void frameIn(void*, uint32_t ptype, uint32_t hdata, const uint8_t* payload, uint16_t payloadLen)
{
    if (payloadLen > sizeof(g_parked[0].payload)) return;
    wa_port_lock();
    g_ct.framesIn++;
    // Beacons repeat; one that cannot be applied at once is dropped rather than parked.
    if (ptype == NET_BROADCAST && (g_parkedCount > 0 || !safeToApplyInbound(g_core->comstate)))
    {
        wa_port_unlock();
        return;
    }
    const bool idle = idleParentState(ptype, payload, payloadLen);
    uint8_t slot = g_parkedCount;
    if (idle && slot > 0 &&
        idleParentState(g_parked[slot - 1].ptype, g_parked[slot - 1].payload, g_parked[slot - 1].payloadLen))
    {
        slot = slot - 1;   // the newest idle state wins, in place
    }
    else if (slot == kParkedMax)
    {
        if (idle)
        {
            g_parkedIdleDrops = g_parkedIdleDrops + 1;
            wa_port_unlock();
            wa_port_wake();
            return;
        }
        // Full: a command frame takes the place of the newest idle repeat. Order among
        // command frames is untouched.
        int victim = -1;
        for (int i = kParkedMax - 1; i >= 0; i--)
            if (idleParentState(g_parked[i].ptype, g_parked[i].payload, g_parked[i].payloadLen))
            {
                victim = i;
                break;
            }
        if (victim < 0)
        {
            g_parkedDrops = g_parkedDrops + 1;   // a sequence gap for the games: counted
            wa_port_unlock();
            wa_port_wake();
            return;
        }
        for (int i = victim; i + 1 < kParkedMax; i++) g_parked[i] = g_parked[i + 1];
        g_parkedIdleDrops = g_parkedIdleDrops + 1;
        slot = kParkedMax - 1;
    }
    ParkedFrame& p = g_parked[slot];
    p.ptype = ptype;
    p.hdata = hdata;
    p.payloadLen = payloadLen;
    std::memcpy(p.payload, payload, payloadLen);
    if (slot == g_parkedCount) g_parkedCount = g_parkedCount + 1;
    if (g_parkedCount > g_parkedHigh) g_parkedHigh = g_parkedCount;
    if (g_parkedCount > g_parkedHighEver) g_parkedHighEver = g_parkedCount;
    wa_port_unlock();
    wa_port_wake();
}

// ---- the GBA-clocked word path (under the lock) --------------------------------

void noteWord(uint32_t rx, uint16_t bits)
{
    g_ct.words++;
    if (g_ct.firstCount < 8) g_ct.firstWords[g_ct.firstCount++] = rx;
    if (g_cal.word_bits && bits != g_cal.word_bits) g_ct.badBits++;
}

// The caller has set SI's GPIO level for the moment before the switch to the slave.
void armWord(uint32_t reply, bool lead)
{
    wa_port_arm(reply, lead);
    wa_port_si_spi();
    g_armedReply = reply;
    g_wire = lead ? wireHeld : wireReleased;
}

// The GBA in handshake_wait(0): it raised SO and clocks nothing until SI drops.
bool handshakeWaiting()
{
    return wa_port_so() && wa_port_sc() && wa_port_edges() == 0;
}

void release()
{
    wa_port_so_irq(false);
    wa_port_si_drive(true);   // SI stays high while the slave is re-armed
    armWord(g_armedReply, false);
}

// Held, with the SO-rise interrupt watching for the GBA's handshake. The rise may
// already have happened, so the level is checked once the interrupt is on.
void watchHandshake()
{
    wa_port_so_irq(true);
    if (handshakeWaiting())
    {
        release();
        g_ct.releasedIrq++;
    }
}

void onWord()
{
    wa_port_lock_isr();
    if (g_wire != wireHeld && g_wire != wireReleased)
    {
        wa_port_unlock_isr();
        return;
    }
    wa_port_so_irq(false);
    // librfu's handshake_wait(1) follows every command-phase word: hold SI high.
    wa_port_si_drive(true);
    uint16_t bits = 0;
    uint32_t rx = wa_port_collect(&bits);
    // The word's last bit is taken from the count of ones read at SC's edges: the GBA
    // drops SO to its idle level as the word ends, before the slave has the bit.
    const uint32_t ones = wa_port_ones();
    const uint32_t upper = static_cast<uint32_t>(__builtin_popcount(rx & ~1u));
    if (ones == upper || ones == upper + 1)
        rx = (rx & ~1u) | (ones - upper);
    else
        g_ct.onesMismatches++;
    noteWord(rx, bits);
    if ((g_cal.word_bits && bits != g_cal.word_bits) || wa_port_edges() > 32)
    {
        // The GBA always clocks 32 bits, so the shifter was off the word boundary
        // (armed inside a word, or noise on SC) and this is two words' pieces. Let
        // the GBA's word finish and start over at the boundary. The core never sees
        // the pieces; the GBA's own checks catch the word it lost.
        const uint32_t start = wa_port_us();
        uint32_t quietSince = start;
        while (wa_port_us() - start < 300)
        {
            if (!wa_port_sc()) quietSince = wa_port_us();
            else if (wa_port_us() - quietSince >= 8) break;
        }
        armWord(g_armedReply, true);
        g_ct.realigns++;
        watchHandshake();
        wa_port_unlock_isr();
        return;
    }
    g_core->onSlaveTransfer(rx, wa_port_ms());
    const uint32_t reply = g_core->stagedTx;
    const bool waitEvent = g_core->comstate == comWaitEvent;

    // The GBA answers SI high by raising SO within a couple of microseconds when it
    // runs the handshake. It cannot clock again before SI drops, so SO here is the
    // handshake, not data.
    bool answered = false;
    const uint32_t start = wa_port_us();
    do
    {
        if (wa_port_so() && wa_port_sc())
        {
            answered = true;
            break;
        }
    } while (wa_port_us() - start < g_config.spin_us);

    if (answered)
    {
        armWord(reply, false);
        g_ct.releasedSpin++;
    }
    else
    {
        armWord(reply, true);
        g_ct.held++;
        watchHandshake();
    }
    wa_port_unlock_isr();
    // The WAIT-class acknowledgement has been read: the GBA turns slave next.
    if (waitEvent) wa_port_wake_isr();
}

void onSoRise()
{
    wa_port_lock_isr();
    if (g_wire == wireHeld)
    {
        if (wa_port_edges() != 0 || !wa_port_sc())
            wa_port_so_irq(false);   // a word's data; its end re-evaluates
        else if (wa_port_so())
        {
            release();
            g_ct.releasedIrq++;
        }
    }
    wa_port_unlock_isr();
}

void onSdFall()
{
    const uint32_t now = wa_port_us() | 1;
    const uint32_t previous = g_sdFallUs;
    g_sdFallUs = now;
    g_sdFell = true;
    if (previous != 0 && now - previous < kSdChatterUs)
    {
        if (++g_sdChatter < kSdChatterRun) return;
        wa_port_sd_irq(false);
        g_sdChatter = 0;
        g_sdFallUs = 0;
        g_sdMuted = true;
        return;
    }
    g_sdChatter = 0;
    if (previous == 0 || now - previous < kSdPulseMinUs || now - previous > kSdPulseMaxUs) return;
    g_sdFallUs = 0;   // a reset's falls are not the start of another
    g_sdLatched = true;
    wa_port_wake_isr();
}

// ---- trace snapshot (tag 0x2D) --------------------------------------------------

uint8_t g_traceSnap[4 + RfuCore::kTraceLen * 4];
uint8_t g_traceSnapSeq = 0;
uint8_t g_traceSnapSends = 0;

void captureTraceSnapshot()
{
    const uint8_t n = g_core->dbgTraceCount;
    std::memset(g_traceSnap, 0, sizeof(g_traceSnap));
    g_traceSnap[0] = 0x2D;
    g_traceSnap[1] = ++g_traceSnapSeq;
    g_traceSnap[2] = n;
    g_traceSnap[3] = static_cast<uint8_t>(g_core->counters.commandRestarts);
    for (uint8_t i = 0; i < n; i++)   // oldest first
    {
        const auto& e = g_core->dbgTrace[(g_core->dbgTraceHead + RfuCore::kTraceLen - n + i) % RfuCore::kTraceLen];
        uint8_t* o = g_traceSnap + 4 + i * 4;
        o[0] = e.kind;
        o[1] = e.code;
        o[2] = static_cast<uint8_t>(e.dtMs);
        o[3] = static_cast<uint8_t>(e.dtMs >> 8);
    }
    g_traceSnapSends = 3;
}

// ---- adapter task ----------------------------------------------------------------

void sendData(const uint8_t* bytes, size_t len)
{
    if (g_transport.data) g_transport.data(bytes, len, g_transport.ctx);
}

void sendStatus(uint16_t code)
{
    if (g_transport.status) g_transport.status(code, g_transport.ctx);
}

void pumpOutbound()
{
    for (;;)
    {
        if (g_chunkLen < sizeof(g_chunk))
        {
            wa_port_lock();
            while (g_outCount > 0 && g_chunkLen < sizeof(g_chunk))
            {
                g_chunk[g_chunkLen++] = g_out[g_outHead];
                g_outHead = (g_outHead + 1) % kOutRing;
                g_outCount--;
            }
            wa_port_unlock();
        }
        if (g_chunkLen == sizeof(g_chunk))
        {
            sendData(g_chunk, sizeof(g_chunk));
            g_chunkLen = 0;
            continue;
        }
        if (g_chunkLen > 0)
        {
            // The ring holds whole frames only: pad to the chunk and send now.
            std::memset(&g_chunk[g_chunkLen], 0, sizeof(g_chunk) - g_chunkLen);
            sendData(g_chunk, sizeof(g_chunk));
            g_chunkLen = 0;
        }
        return;
    }
}

void applyParked(uint32_t now)
{
    if (g_parkedCount == 0 || !safeToApplyInbound(g_core->comstate)) return;
    const uint8_t cnt = g_parkedCount;
    g_lastNetRxMs = now ? now : 1;
    for (uint8_t i = 0; i < cnt; i++)
    {
        wa_port_lock();
        g_core->applyNetPacket(g_parked[i].ptype, g_parked[i].hdata, g_parked[i].payload,
                              g_parked[i].payloadLen, now);
        wa_port_unlock();
    }
    // A frame can park while the loop runs: compact rather than zero the count.
    wa_port_lock();
    for (uint8_t i = cnt; i < g_parkedCount; i++) g_parked[i - cnt] = g_parked[i];
    g_parkedCount = g_parkedCount - cnt;
    wa_port_unlock();
}

// Waits for SO to reach level. Leaves early when the GBA resets the adapter.
bool waitSo(bool level, uint32_t timeoutUs)
{
    const uint32_t start = wa_port_us();
    while (wa_port_so() != level)
    {
        if (g_sdLatched || g_sdFell || wa_port_us() - start >= timeoutUs) return wa_port_so() == level;
    }
    return true;
}

// Clock one wait-event frame into the GBA-as-slave word by word, running librfu's
// slave-side handshake (sio32intr_clock_slave) between words: after each word the GBA
// waits for SI low, raises SO, takes the word, waits for SI high, then drops SO and
// re-arms, or after the last word leaves slave mode. Word 1 needs no handshake: the
// GBA armed itself after the WAIT-class acknowledgement and counts clocks.
void runDelivery()
{
    uint32_t words[kMaxEventWords];
    uint8_t n;
    wa_port_lock();
    if (g_core->comstate != comWaitResp || g_core->eventWordCount() > kMaxEventWords)
    {
        wa_port_unlock();
        g_retriesThisWait = 0;
        return;
    }
    n = g_core->eventWordCount();
    for (uint8_t i = 0; i < n; i++) words[i] = g_core->eventWord(i);
    wa_port_unlock();

    // The GBA turns slave once its handshake for the acknowledgement ends: that word's
    // release is armed and SO has dropped.
    const uint32_t start = wa_port_us();
    while (!(g_wire == wireReleased && !wa_port_so()))
    {
        if (g_sdLatched || wa_port_us() - start >= kGbaHandshakeUs)
        {
            wa_port_lock();
            if (g_core->comstate == comWaitResp) g_core->retryDelivery(wa_port_ms());
            wa_port_unlock();
            g_dbgDelivFail = dfHandshakeWait;
            g_dbgDelivSo = wa_port_so() ? 1 : 0;
            return;
        }
    }
    // It loads its reply word and re-enables its shifter as an external-clock slave.
    wa_port_delay_us(150);

    wa_port_lock();
    if (g_core->comstate != comWaitResp || g_wire != wireReleased)
    {
        // A soft reset (0x494E) came through meanwhile: the GBA is bus master again.
        wa_port_unlock();
        g_retriesThisWait = 0;
        return;
    }
    wa_port_word_irq(false);
    wa_port_so_irq(false);
    wa_port_si_drive(false);   // the inverted-ACK lead-in starts with SI low
    wa_port_disarm();
    g_sdFell = false;
    wa_port_sc_drive(true);
    g_wire = wireMaster;
    wa_port_unlock();

    // Word-1 inverted-ACK lead-in (afska wireless_adapter.md "Inverted ACKs"): the GBA
    // may answer SI low with an SO-high pulse, but usually does not before word 1.
    if (waitSo(true, 120))
    {
        wa_port_si_drive(true);
        waitSo(false, 4000);
        wa_port_si_drive(false);
    }
    wa_port_delay_us(45);

    bool released = false;
    for (uint8_t i = 0; i < n && !g_sdFell; i++)
    {
        uint32_t rx = 0;
        if (!wa_port_master_word(words[i], &rx, &g_sdFell))
        {
            g_dbgDelivFail = static_cast<uint8_t>(dfReset | (i << 4));
            break;
        }
        g_revRx0 = g_revRx1;
        g_revRx1 = rx;
        g_core->counters.masterTransfers++;

        if (i > 0 && rx == 0x996601EE)
        {
            // The GBA rejected the event and staged an error reply in its place; it
            // recovers through its own timer and checkID.
            g_core->counters.errResponses++;
            g_dbgDelivFail = static_cast<uint8_t>(dfErrResponse | (i << 4));
            break;
        }

        wa_port_si_drive(false);
        if (!waitSo(true, kGbaHandshakeUs))   // SO high: word received, processing
        {
            g_dbgDelivFail = static_cast<uint8_t>(dfDanceSoHigh | (i << 4));
            g_dbgDelivSo = wa_port_so() ? 1 : 0;
            break;
        }

        if (i + 1 < n)
        {
            wa_port_si_drive(true);   // release its handshake_wait(1)
            // It re-arms with two SIOCNT writes under IME=0 (librfu_intr.c:311-328): no
            // level to wait on. Bounded by its TM0-proximity spin, worst ~125 us.
            wa_port_delay_us(250);
            continue;
        }

        // Last word: arming the next GBA-clocked word is the release. Its lead bit
        // takes SI high, the GBA leaves slave mode, and the command it sends next
        // finds the shifter waiting.
        wa_port_lock();
        g_core->finishDelivery(wa_port_ms());
        wa_port_sc_drive(false);
        wa_port_word_irq(true);
        armWord(g_core->stagedTx, true);
        wa_port_unlock();
        released = true;
    }

    if (released)
    {
        g_dbgDelivFail = dfNone;
        g_retriesThisWait = 0;
        g_ct.deliveries++;
        // It drops SO as it leaves slave mode.
        if (!waitSo(false, kGbaHandshakeUs))
        {
            g_ct.finalSoStuck++;
            g_dbgDelivFail = static_cast<uint8_t>(dfFinalSoLow | ((n - 1) << 4));
        }
        return;
    }

    g_ct.deliveryFails++;
    wa_port_lock();
    g_core->abortDelivery(wa_port_ms());
    g_retriesThisWait = 0;
    wa_port_sc_drive(false);
    wa_port_si_drive(true);
    wa_port_word_irq(true);
    armWord(g_core->stagedTx, true);
    wa_port_unlock();
}

void stopLink()
{
    if (g_wire == wireMaster) wa_port_sc_drive(false);
    wa_port_word_irq(false);
    wa_port_so_irq(false);
    wa_port_disarm();
    wa_port_si_drive(true);
    g_wire = wireOff;
}

void startLink()
{
    wa_port_si_drive(true);
    wa_port_word_irq(true);
    armWord(g_core->stagedTx, true);
    watchHandshake();
}

// The flag is cleared before the interrupt comes back, so a fall that mutes it again at
// once is not lost.
void listenToSd()
{
    g_sdFallUs = 0;
    g_sdChatter = 0;
    g_sdMuted = false;
    wa_port_sd_irq(true);
}

void unmuteSd(uint32_t now)
{
    static uint32_t mutedSince = 0;
    if (!g_sdMuted)
        mutedSince = 0;
    else if (!mutedSince)
        mutedSince = now | 1;
    else if (now - mutedSince >= kSdMuteMs)
    {
        mutedSince = 0;
        listenToSd();
    }
}

// The reset is latched when the pulse ends.
void handleSdReset(uint32_t now)
{
    if (!g_sdLatched) return;
    g_sdLatched = false;
    wa_port_lock();
    if (g_core->state != stIdle) captureTraceSnapshot();
    g_core->onSdReset(now);
    stopLink();
    wa_port_unlock();
    g_retriesThisWait = 0;
    g_ct.sdResets++;

    // A real adapter's shift register resets with the pulse. During it the game's SC
    // floats on the pull-ups and the pulse's own edges couple onto it, so the arm
    // follows the falling edge: the ID exchange that comes next must find the
    // shifter at bit 0.
    wa_port_lock();
    startLink();
    wa_port_unlock();
}

// A word that started and stopped part-way (the shifter armed in the middle of one, or
// noise on SC) leaves the count short of 32, and every word after would arrive
// shifted. The GBA never pauses inside a word: its 32 clocks take at most 125 us. A
// short count that has not moved since the last pass is stale: start over.
void watchPartialWord()
{
    static uint32_t lastEdges = 0, lastUs = 0;
    const Wire w = g_wire;
    const uint32_t e = (w == wireHeld || w == wireReleased) ? wa_port_edges() : 0;
    const uint32_t now = wa_port_us();
    const bool stale = e > 0 && e < 32 && e == lastEdges && now - lastUs >= 300;
    if (!stale)
    {
        if (e != lastEdges)
        {
            lastEdges = e;
            lastUs = now;
        }
        return;
    }
    wa_port_lock();
    if (g_wire == w && wa_port_edges() == e)
    {
        wa_port_so_irq(false);
        wa_port_si_drive(true);
        armWord(g_armedReply, w == wireHeld);
        if (w == wireHeld) watchHandshake();
        g_ct.realigns++;
    }
    wa_port_unlock();
    lastEdges = 0;
}

void recalibrate()
{
    g_recalibrate = false;
    wa_port_lock();
    stopLink();
    wa_port_unlock();
    wa_port_sd_irq(false);   // the self-test drives SI, which can set SD chattering
    wa_port_calibrate(&g_cal);
    listenToSd();
    wa_port_lock();
    startLink();
    wa_port_unlock();
}

bool g_reportedReconnecting = false;

void superviseLink(uint32_t nowMs)
{
    // Status only: the adapter keeps answering the GBA regardless. Skipped until the
    // network side has been heard once.
    const uint32_t lastHeard = g_lastNetRxMs;
    if (lastHeard == 0) return;
    const bool quiet = static_cast<int32_t>(nowMs - lastHeard) > 4000;
    if (quiet && !g_reportedReconnecting)
    {
        sendStatus(0xFF06);   // LinkReconnecting
        g_reportedReconnecting = true;
    }
    else if (!quiet && g_reportedReconnecting)
    {
        sendStatus(0xFF05);   // LinkConnected
        g_reportedReconnecting = false;
    }
}

// SetMode(wireless) while running gives what the GB-Link's mode entry gives: a fresh
// adapter, and AwaitMode. The bridge relies on it after a session to clear the old
// room from the GBA's list. The GBA has to log in again.
void enterMode()
{
    g_enterMode = false;
    wa_port_lock();
    stopLink();
    g_core->reset();
    g_core->roleLock = g_enterRole;
    g_parkedCount = 0;
    g_outHead = 0;
    g_outCount = 0;
    wa_port_unlock();
    g_chunkLen = 0;
    g_retriesThisWait = 0;
    g_lastNetRxMs = 0;
    g_reportedReconnecting = false;
    sendStatus(0xFF02);   // AwaitMode
    wa_port_lock();
    startLink();
    wa_port_unlock();
}

uint8_t linesNow()
{
    return static_cast<uint8_t>((wa_port_sc() ? 1 : 0) | (wa_port_so() ? 2 : 0) |
                                (wa_port_si() ? 4 : 0) | (wa_port_sd() ? 8 : 0));
}

void put32(uint8_t* o, uint32_t v)
{
    o[0] = static_cast<uint8_t>(v);
    o[1] = static_cast<uint8_t>(v >> 8);
    o[2] = static_cast<uint8_t>(v >> 16);
    o[3] = static_cast<uint8_t>(v >> 24);
}

void put16(uint8_t* o, uint16_t v)
{
    o[0] = static_cast<uint8_t>(v);
    o[1] = static_cast<uint8_t>(v >> 8);
}

uint16_t sat16(uint32_t v) { return v > 65535 ? 65535 : static_cast<uint16_t>(v); }
uint8_t sat4(uint8_t v) { return v > 15 ? 15 : v; }

// The GB-Link's telemetry reports, same tags and layouts: the bridge logs them.
void reportDiagnostics(uint32_t nowMs)
{
    static uint32_t lastMs = 0;
    if (nowMs - lastMs < 500) return;
    lastMs = nowMs;

    // Tag 0x0E: detection (the bridge reads byte 1 as "a GBA is clocking").
    uint8_t diag[16] = {0x0E,
                        static_cast<uint8_t>(g_core->dbgAnyRx ? 1 : 0),
                        static_cast<uint8_t>(g_core->dbgNintendo ? 1 : 0),
                        static_cast<uint8_t>(g_core->comstate)};
    put32(diag + 4, g_core->dbgFirstRx);
    put32(diag + 8, g_core->dbgLastRx);
    put32(diag + 12, g_core->counters.slaveTransfers);
    sendData(diag, sizeof(diag));

    // Tag 0x2E: core state, the last eight commands, counters, delivery readback.
    uint8_t f[62] = {};
    f[0] = 0x2E;
    f[1] = static_cast<uint8_t>(g_core->comstate);
    f[2] = static_cast<uint8_t>(g_core->state);
    f[3] = g_wire == wireMaster ? 1 : 0;
    f[4] = static_cast<uint8_t>((g_core->dbgAnyRx ? 0x01 : 0) | (g_core->dbgNintendo ? 0x02 : 0) |
                                (g_core->dbgWaitAck() ? 0x04 : 0));
    f[5] = g_core->dbgLastCmd();
    f[6] = g_core->dbgLastPlen();
    const uint8_t cnt = g_core->dbgCmdRingCount;
    const uint8_t head = g_core->dbgCmdRingHead;
    f[7] = cnt;
    for (uint8_t i = 0; i < 8; i++)
        f[8 + i] = (i < cnt) ? g_core->dbgCmdRing[static_cast<uint8_t>(head - cnt + i) & 7] : 0;
    const uint32_t c32[8] = {
        g_core->counters.slaveTransfers, g_core->counters.masterTransfers,
        g_core->counters.waitEventTransfers, g_core->counters.commands,
        g_core->counters.deliveryAborts, g_core->counters.loginRestarts,
        g_core->counters.errResponses, g_core->dbgLastRx,
    };
    for (int i = 0; i < 8; i++) put32(f + 16 + i * 4, c32[i]);
    put32(f + 48, g_revRx0);
    put32(f + 52, g_revRx1);
    f[56] = g_dbgDelivFail;
    f[57] = g_dbgDelivSo;
    put32(f + 58, g_core->counters.deliveryRetries);
    sendData(f, sizeof(f));

    // Tag 0x1D: how waits resolve, slot wipes, inbound loss, flow control.
    const uint32_t lev = g_core->dbgLastEvent;
    const uint32_t lpw = g_core->dbgLastLinkPwr;
    const uint8_t fifoHigh = g_core->dbgFifoHigh;
    g_core->dbgFifoHigh = 0;
    uint8_t ev[25] = {0x1D, g_core->dbgEvData, g_core->dbgEvRtx, g_core->dbgEvTimeo, g_core->dbgEvDisc};
    put32(ev + 5, lev);
    ev[9] = g_core->dbgLinkPwrZero;
    put32(ev + 10, lpw);
    ev[14] = static_cast<uint8_t>(g_ct.sdResets > 255 ? 255 : g_ct.sdResets);
    ev[15] = static_cast<uint8_t>((sat4(g_core->dbgWipeEvict) << 4) | sat4(g_core->dbgWipeNetDisc));
    ev[16] = static_cast<uint8_t>((sat4(g_core->dbgWipeHostStart) << 4) | sat4(g_core->dbgWipeCmdDisc));
    ev[17] = static_cast<uint8_t>((sat4(g_core->dbgWipeReset) << 4) | g_core->dbgSlotOccupancy());
    ev[18] = static_cast<uint8_t>(g_parkedDrops > 255 ? 255 : g_parkedDrops);
    ev[19] = static_cast<uint8_t>(g_core->counters.rxDropQueueFull > 255 ? 255 : g_core->counters.rxDropQueueFull);
    ev[20] = fifoHigh;
    ev[21] = static_cast<uint8_t>((sat4(g_core->dbgFlowHints) << 4) | sat4(g_core->dbgFlowHolds));
    ev[22] = g_core->dbgSheds;
    ev[23] = g_core->dbgIdleRetx;
    ev[24] = static_cast<uint8_t>(g_core->counters.commands);
    sendData(ev, sizeof(ev));

    // Tag 0x1E: where inbound frames can be lost, and delivery outcomes.
    uint8_t tr[19] = {0x1E};
    put16(tr + 1, 0);   // no adapter-side UART on this chip
    tr[3] = g_parkedHigh;
    put16(tr + 4, g_parkedDrops);
    put16(tr + 6, sat16(g_core->counters.rxDropQueueFull));
    tr[8] = fifoHigh;
    put16(tr + 9, sat16(g_core->counters.deliveryAborts));
    put16(tr + 11, sat16(g_core->counters.deliveryRetries));
    put16(tr + 13, g_parkedIdleDrops);
    put16(tr + 15, g_outRingDrops);
    put16(tr + 17, sat16(g_core->counters.commandRestarts));
    g_parkedHigh = 0;
    sendData(tr, sizeof(tr));

    if (g_traceSnapSends > 0)
    {
        g_traceSnapSends--;
        sendData(g_traceSnap, sizeof(g_traceSnap));
    }
}

void adapterTask(void*)
{
    static const wa_isr_t isr = {onWord, onSoRise, onSdFall};
    wa_port_init(&g_config.pins, &isr, &g_cal);
    listenToSd();

    wa_port_lock();
    new (g_core) RfuCore();
    g_core->emit = &emitFrame;
    g_core->rand16 = &rand16;
    g_core->reset();
    g_core->roleLock = g_config.role;   // after reset(), which must not clobber it
    startLink();
    wa_port_unlock();
    g_running = true;
    sendStatus(0xFF02);   // in the mode from power-up

    for (;;)
    {
        const uint32_t now = wa_port_ms();
        if (g_recalibrate) recalibrate();
        if (g_enterMode) enterMode();
        pumpOutbound();
        unmuteSd(now);
        handleSdReset(now);
        applyParked(now);
        watchPartialWord();

        wa_port_lock();
        const bool deliver = g_core->pollWaitEvent(now);
        g_core->tick(now);
        wa_port_unlock();
        if (deliver) runDelivery();

        superviseLink(now);
        reportDiagnostics(now);
        wa_port_wait(1);
    }
}

}  // namespace

extern "C" bool wa_adapter_start(const wa_config_t* config, const wa_transport_t* transport)
{
    if (__atomic_exchange_n(&g_started, true, __ATOMIC_ACQ_REL)) return true;
    g_core = new (std::nothrow) RfuCore();
    g_parked = new (std::nothrow) ParkedFrame[kParkedMax];
    g_out = new (std::nothrow) uint8_t[kOutRing];
    if (!g_core || !g_parked || !g_out)
    {
        delete g_core;
        delete[] g_parked;
        delete[] g_out;
        g_core = nullptr;
        g_parked = nullptr;
        g_out = nullptr;
        g_started = false;
        return false;
    }
    g_config = *config;
    if (g_config.spin_us == 0) g_config.spin_us = 15;
    g_transport = *transport;
    new (&g_parser) Rfu1StreamParser();
    g_parser.setCallback(&frameIn, nullptr);
    return wa_port_spawn(&adapterTask, nullptr);
}

extern "C" bool wa_adapter_running(void) { return g_running; }

extern "C" void wa_adapter_enter_mode(uint8_t role)
{
    g_enterRole = role;
    g_enterMode = true;
    wa_port_wake();
}

extern "C" void wa_adapter_feed(const uint8_t* bytes, size_t length)
{
    if (!g_running) return;
    g_parser.push(bytes, length);
}

extern "C" void wa_adapter_recalibrate(void)
{
    g_recalibrate = true;
    wa_port_wake();
}

extern "C" uint32_t wa_adapter_search_age_ms(void)
{
    if (!g_core) return UINT32_MAX;
    wa_port_lock();
    const bool searched = g_core->searched;
    const uint32_t at = g_core->lastSearchMs;
    wa_port_unlock();
    return searched ? wa_port_ms() - at : UINT32_MAX;
}

extern "C" void wa_adapter_stats(wa_stats_t* out)
{
    std::memset(out, 0, sizeof(*out));
    wa_port_lock();
    out->cal = g_cal;
    out->words = g_ct.words;
    out->bad_bits = g_ct.badBits;
    out->realigns = g_ct.realigns;
    out->released_spin = g_ct.releasedSpin;
    out->released_irq = g_ct.releasedIrq;
    out->held = g_ct.held;
    out->first_count = g_ct.firstCount;
    std::memcpy(out->first_words, g_ct.firstWords, sizeof(out->first_words));
    if (!g_core)
    {
        wa_port_unlock();
        return;
    }
    out->master_words = g_core->counters.masterTransfers;
    out->deliveries = g_ct.deliveries;
    out->delivery_fails = g_ct.deliveryFails;
    out->delivery_retries = g_core->counters.deliveryRetries;
    out->delivery_aborts = g_core->counters.deliveryAborts;
    out->last_fail = g_dbgDelivFail;
    out->sd_resets = g_ct.sdResets;
    out->comstate = static_cast<uint8_t>(g_core->comstate);
    out->state = static_cast<uint8_t>(g_core->state);
    out->role = g_core->roleLock;
    out->last_cmd = g_core->dbgLastCmd();
    out->any_rx = g_core->dbgAnyRx;
    out->nintendo = g_core->dbgNintendo;
    out->commands = g_core->counters.commands;
    out->err_responses = g_core->counters.errResponses;
    out->login_restarts = g_core->counters.loginRestarts;
    out->command_restarts = g_core->counters.commandRestarts;
    out->ones_mismatches = g_ct.onesMismatches;
    out->frames_in = g_ct.framesIn;
    out->frames_out = g_ct.framesOut;
    out->parked_high = g_parkedHighEver;
    out->parked_drops = g_parkedDrops;
    out->out_drops = g_outRingDrops;
    wa_port_unlock();
    out->lines = linesNow();
}
