#include "rfu_emu.h"

#include <cstring>
#include <new>

#include "rfuProtocol.hpp"

// The GB-Link's wireless adapter behind the emulated GBA's link port: its protocol core
// (rfuproto::RfuCore) with the glue the GB-Link and the ESP32's on-chip adapter wrap it in
// (esp-adapter/components/rfu_adapter/wa_adapter.cpp), on one thread: the link-port driver
// calls in from inside the game's run, the page between runs. The network side is the
// GB-Link's, so the ESP32 board takes this adapter for one: RFU1 frames in 64-byte chunks on
// the data channel, statuses on the status channel, and the same reports.

namespace
{

using namespace rfuproto;

constexpr uint8_t CHANNEL_DATA = 0x01;
constexpr uint8_t CHANNEL_STATUS = 0x02;

RfuCore g_core;
Rfu1StreamParser g_parser;
bool g_constructed = false;

// ---- GB frames for the board, taken by the page after each run --------------------

constexpr size_t kOutBytes = 16384;
uint8_t g_out[kOutBytes];
size_t g_outLen = 0;

void putFrame(uint8_t channel, const uint8_t* payload, size_t len)
{
    if (kOutBytes - g_outLen < len + 5) return;
    uint8_t* o = g_out + g_outLen;
    o[0] = 0x47;   // 'G'
    o[1] = 0x42;   // 'B'
    o[2] = channel;
    o[3] = static_cast<uint8_t>(len);
    o[4] = static_cast<uint8_t>(len >> 8);
    std::memcpy(o + 5, payload, len);
    g_outLen += len + 5;
}

void sendStatus(uint16_t code)
{
    const uint8_t payload[2] = {static_cast<uint8_t>(code), static_cast<uint8_t>(code >> 8)};
    putFrame(CHANNEL_STATUS, payload, sizeof(payload));
}

// ---- RFU1 frames from the core, whole, then in 64-byte chunks ---------------------

constexpr size_t kRing = 4096;
uint8_t g_ring[kRing];
size_t g_ringHead = 0, g_ringCount = 0;
uint16_t g_ringDrops = 0;

void emitFrame(void*, const uint8_t* frame, size_t len)
{
    // All or nothing: a partial frame would corrupt the stream.
    if (kRing - g_ringCount < len)
    {
        g_ringDrops = g_ringDrops + 1;
        return;
    }
    for (size_t i = 0; i < len; i++) g_ring[(g_ringHead + g_ringCount + i) % kRing] = frame[i];
    g_ringCount += len;
}

// Frames run on across chunks; the last chunk is padded, which the board's parser skips.
void pumpRing()
{
    while (g_ringCount > 0)
    {
        uint8_t chunk[64] = {};
        const size_t n = g_ringCount < sizeof(chunk) ? g_ringCount : sizeof(chunk);
        for (size_t i = 0; i < n; i++)
        {
            chunk[i] = g_ring[g_ringHead];
            g_ringHead = (g_ringHead + 1) % kRing;
        }
        g_ringCount -= n;
        putFrame(CHANNEL_DATA, chunk, sizeof(chunk));
    }
}

uint16_t random16(void*) { return rfu_random16(); }

// ---- frames from the board, parked until the game is between commands -------------

struct ParkedFrame
{
    uint32_t ptype;
    uint32_t hdata;
    uint16_t payloadLen;
    uint8_t payload[92];
};
constexpr uint8_t kParkedMax = 32;
ParkedFrame g_parked[kParkedMax];
uint8_t g_parkedCount = 0;
uint16_t g_parkedDrops = 0;       // command-carrying frames lost
uint16_t g_parkedIdleDrops = 0;   // idle state repeats not kept
uint8_t g_parkedHigh = 0;         // per-report high-water
uint32_t g_lastNetRxMs = 0;

// Applied while the game is between commands or is the bus slave, as on the GB-Link, so a
// command's words and its reply see one consistent state.
bool safeToApplyInbound(uint8_t comstate)
{
    return comstate == comWaitCmd || comstate == comWaitEvent || comstate == comWaitResp;
}

// An idle parent frame is a "current state" repeat: only the newest is kept, and a run of
// them must not crowd out a frame carrying a command, which the parent sends only once.
bool idleParentState(uint32_t ptype, const uint8_t* payload, uint16_t len)
{
    return ptype == NET_HOST_SEND && RfuCore::isParentUni(payload, len) && !RfuCore::parentUniHasCommand(payload, len);
}

void frameIn(void*, uint32_t ptype, uint32_t hdata, const uint8_t* payload, uint16_t payloadLen)
{
    if (payloadLen > sizeof(g_parked[0].payload)) return;
    // Beacons repeat; one that cannot be applied at once is dropped rather than parked.
    if (ptype == NET_BROADCAST && (g_parkedCount > 0 || !safeToApplyInbound(g_core.comstate))) return;
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
            g_parkedDrops = g_parkedDrops + 1;
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
}

void applyParked(uint32_t now)
{
    if (g_parkedCount == 0 || !safeToApplyInbound(g_core.comstate)) return;
    g_lastNetRxMs = now ? now : 1;
    for (uint8_t i = 0; i < g_parkedCount; i++)
        g_core.applyNetPacket(g_parked[i].ptype, g_parked[i].hdata, g_parked[i].payload, g_parked[i].payloadLen, now);
    g_parkedCount = 0;
}

// ---- statuses and reports ----------------------------------------------------------

uint16_t g_sdResets = 0;
bool g_reportedReconnecting = false;

// Status only: the adapter keeps answering the game regardless. Skipped until the network
// side has been heard once.
void superviseLink(uint32_t now)
{
    if (g_lastNetRxMs == 0) return;
    const bool quiet = static_cast<int32_t>(now - g_lastNetRxMs) > 4000;
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

// The GB-Link's reports, same tags and layouts, twice a second: the board reads 0x0E (a game
// is clocking), 0x1C (when it last looked for a group) and 0x1D (its resets of the adapter).
void report(uint32_t now)
{
    static uint32_t last = 0;
    if (now - last < 500) return;
    last = now;

    uint8_t diag[16] = {0x0E, static_cast<uint8_t>(g_core.dbgAnyRx ? 1 : 0),
                        static_cast<uint8_t>(g_core.dbgNintendo ? 1 : 0), static_cast<uint8_t>(g_core.comstate)};
    put32(diag + 4, g_core.dbgFirstRx);
    put32(diag + 8, g_core.dbgLastRx);
    put32(diag + 12, g_core.counters.slaveTransfers);
    putFrame(CHANNEL_DATA, diag, sizeof(diag));

    uint8_t f[62] = {};
    f[0] = 0x2E;
    f[1] = static_cast<uint8_t>(g_core.comstate);
    f[2] = static_cast<uint8_t>(g_core.state);
    f[3] = g_core.comstate == comWaitResp ? 1 : 0;
    f[4] = static_cast<uint8_t>((g_core.dbgAnyRx ? 0x01 : 0) | (g_core.dbgNintendo ? 0x02 : 0) |
                                (g_core.dbgWaitAck() ? 0x04 : 0));
    f[5] = g_core.dbgLastCmd();
    f[6] = g_core.dbgLastPlen();
    const uint8_t cnt = g_core.dbgCmdRingCount;
    const uint8_t head = g_core.dbgCmdRingHead;
    f[7] = cnt;
    for (uint8_t i = 0; i < 8; i++)
        f[8 + i] = (i < cnt) ? g_core.dbgCmdRing[static_cast<uint8_t>(head - cnt + i) & 7] : 0;
    const uint32_t c32[8] = {
        g_core.counters.slaveTransfers, g_core.counters.masterTransfers,
        g_core.counters.waitEventTransfers, g_core.counters.commands,
        g_core.counters.deliveryAborts, g_core.counters.loginRestarts,
        g_core.counters.errResponses, g_core.dbgLastRx,
    };
    for (int i = 0; i < 8; i++) put32(f + 16 + i * 4, c32[i]);
    put32(f + 58, g_core.counters.deliveryRetries);
    putFrame(CHANNEL_DATA, f, sizeof(f));

    const uint8_t fifoHigh = g_core.dbgFifoHigh;
    g_core.dbgFifoHigh = 0;
    uint8_t ev[25] = {0x1D, g_core.dbgEvData, g_core.dbgEvRtx, g_core.dbgEvTimeo, g_core.dbgEvDisc};
    put32(ev + 5, g_core.dbgLastEvent);
    ev[9] = g_core.dbgLinkPwrZero;
    put32(ev + 10, g_core.dbgLastLinkPwr);
    ev[14] = static_cast<uint8_t>(g_sdResets);
    ev[15] = static_cast<uint8_t>((sat4(g_core.dbgWipeEvict) << 4) | sat4(g_core.dbgWipeNetDisc));
    ev[16] = static_cast<uint8_t>((sat4(g_core.dbgWipeHostStart) << 4) | sat4(g_core.dbgWipeCmdDisc));
    ev[17] = static_cast<uint8_t>((sat4(g_core.dbgWipeReset) << 4) | g_core.dbgSlotOccupancy());
    ev[18] = static_cast<uint8_t>(g_parkedDrops > 255 ? 255 : g_parkedDrops);
    ev[19] = static_cast<uint8_t>(g_core.counters.rxDropQueueFull > 255 ? 255 : g_core.counters.rxDropQueueFull);
    ev[20] = fifoHigh;
    ev[21] = static_cast<uint8_t>((sat4(g_core.dbgFlowHints) << 4) | sat4(g_core.dbgFlowHolds));
    ev[22] = g_core.dbgSheds;
    ev[23] = g_core.dbgIdleRetx;
    ev[24] = static_cast<uint8_t>(g_core.counters.commands);
    putFrame(CHANNEL_DATA, ev, sizeof(ev));

    uint8_t tr[19] = {0x1E};
    tr[3] = g_parkedHigh;
    put16(tr + 4, g_parkedDrops);
    put16(tr + 6, sat16(g_core.counters.rxDropQueueFull));
    tr[8] = fifoHigh;
    put16(tr + 9, sat16(g_core.counters.deliveryAborts));
    put16(tr + 11, sat16(g_core.counters.deliveryRetries));
    put16(tr + 13, g_parkedIdleDrops);
    put16(tr + 15, g_ringDrops);
    put16(tr + 17, sat16(g_core.counters.commandRestarts));
    g_parkedHigh = 0;
    putFrame(CHANNEL_DATA, tr, sizeof(tr));

    // ms since the game last looked for a group, 0xFFFF before it has.
    const int32_t age = static_cast<int32_t>(now - g_core.lastSearchMs);
    const uint16_t searched = !g_core.searched ? 0xFFFF : age <= 0 ? 0 : age >= 0xFFFE ? 0xFFFE : static_cast<uint16_t>(age);
    const uint8_t search[3] = {0x1C, static_cast<uint8_t>(searched), static_cast<uint8_t>(searched >> 8)};
    putFrame(CHANNEL_DATA, search, sizeof(search));
}

// A fresh adapter in the wireless mode, as the GB-Link's mode entry gives one.
void freshAdapter(uint8_t role)
{
    if (!g_constructed)
    {
        new (&g_core) RfuCore();
        new (&g_parser) Rfu1StreamParser();
        g_parser.setCallback(&frameIn, nullptr);
        g_constructed = true;
    }
    g_core.emit = &emitFrame;
    g_core.rand16 = &random16;
    g_core.reset();
    g_core.roleLock = role;   // after reset(), which must not clobber it
    g_parkedCount = 0;
    g_ringHead = 0;
    g_ringCount = 0;
    g_lastNetRxMs = 0;
    g_reportedReconnecting = false;
    sendStatus(0xFF02);   // AwaitMode
}

}  // namespace

extern "C" void rfu_power_on(uint32_t) { freshAdapter(0); }

extern "C" uint32_t rfu_reply(void) { return g_core.stagedTx; }

extern "C" void rfu_word(uint32_t rx, uint32_t now_ms) { g_core.onSlaveTransfer(rx, now_ms); }

extern "C" int rfu_upkeep(uint32_t now_ms)
{
    applyParked(now_ms);
    const bool deliver = g_core.pollWaitEvent(now_ms);
    g_core.tick(now_ms);
    superviseLink(now_ms);
    report(now_ms);
    return deliver ? g_core.eventWordCount() : 0;
}

extern "C" uint32_t rfu_event_word(int index) { return g_core.eventWord(static_cast<uint8_t>(index)); }

extern "C" bool rfu_delivering(void) { return g_core.comstate == comWaitResp; }

extern "C" void rfu_delivered(uint32_t now_ms)
{
    g_core.finishDelivery(now_ms);
}

extern "C" void rfu_delivery_failed(uint32_t now_ms, bool started)
{
    if (started) g_core.abortDelivery(now_ms);
    else g_core.retryDelivery(now_ms);
}

extern "C" void rfu_sd_reset(uint32_t now_ms)
{
    g_core.onSdReset(now_ms);
    g_sdResets = static_cast<uint16_t>(g_sdResets + 1);
}

extern "C" void rfu_enter_mode(int role) { freshAdapter(static_cast<uint8_t>(role)); }

extern "C" void rfu_feed(const uint8_t* bytes, int length)
{
    if (g_constructed && length > 0) g_parser.push(bytes, static_cast<size_t>(length));
}

extern "C" int rfu_take(uint8_t* out, int capacity)
{
    pumpRing();
    const size_t n = capacity < 0 ? 0 : (static_cast<size_t>(capacity) < g_outLen ? static_cast<size_t>(capacity) : g_outLen);
    std::memcpy(out, g_out, n);
    std::memmove(g_out, g_out + n, g_outLen - n);
    g_outLen -= n;
    return static_cast<int>(n);
}
