// Host simulation of a GBA running librfu's SIO layer (built from a pokefirered checkout)
// against the ESP32 adapter's own logic (wa_adapter.cpp and the unmodified protocol core).
// Only wa_port.h is replaced: the ESP32's SPI slave, pulse counter, GPIO and interrupt
// latencies become a model on a shared, simulated wire.
//
// Time is simulated in nanoseconds. The GBA program and the adapter task run as two
// coroutines; each spends simulated time at every register or port access, and wire
// events (SC edges of a GBA-clocked transfer, timer overflows, interrupt latencies) sit
// in one queue. Nothing runs concurrently, so every interleaving is reproducible.

#include <ucontext.h>

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <queue>
#include <string>
#include <vector>

#include "sim_api.h"
#include "sim_gba.h"
#include "wa_adapter.h"

namespace
{

typedef uint64_t ns_t;
constexpr ns_t kNever = UINT64_MAX;
constexpr double kCycleNs = 1e9 / 16777216.0;

SimParams P;
ns_t now = 0;
uint32_t randState = 12345;

// ---- coroutines and events ------------------------------------------------------

struct Co
{
    ucontext_t ctx;
    std::vector<char> stack;
    ns_t wakeAt = kNever;
    bool done = false;
};
ucontext_t schedCtx;
Co gbaCo, adaCo;
Co* current = nullptr;

struct Event
{
    ns_t t;
    uint64_t seq;
    std::function<void()> fn;
    bool operator>(const Event& o) const { return t != o.t ? t > o.t : seq > o.seq; }
};
std::priority_queue<Event, std::vector<Event>, std::greater<Event>> events;
uint64_t eventSeq = 0;

void at(ns_t t, std::function<void()> fn) { events.push({t, eventSeq++, std::move(fn)}); }

void sleepUntil(ns_t t)
{
    Co* self = current;
    self->wakeAt = t;
    swapcontext(&self->ctx, &schedCtx);
}

void (*adaEntry)(void*) = nullptr;
void* adaArg = nullptr;
void (*gbaEntry)(void) = nullptr;

void runGba() { gbaEntry(); gbaCo.done = true; gbaCo.wakeAt = kNever; }
void runAda() { adaEntry(adaArg); adaCo.done = true; adaCo.wakeAt = kNever; }

void startCo(Co& co, void (*fn)())
{
    co.stack.assign(1 << 20, 0);
    getcontext(&co.ctx);
    co.ctx.uc_stack.ss_sp = co.stack.data();
    co.ctx.uc_stack.ss_size = co.stack.size();
    co.ctx.uc_link = &schedCtx;
    makecontext(&co.ctx, fn, 0);
    co.wakeAt = now;
    co.done = false;
}

// ---- the wire -------------------------------------------------------------------

struct Wire
{
    bool sc = true, so = true, sd = true;
    int sdForced = -1;   // a scenario's level on SD in place of the GBA's (-1: the GBA's)
} W;

void updateLines();

// ---- the GBA --------------------------------------------------------------------

uint8_t io[0x400];

struct Timer
{
    uint16_t reload = 0, ctrl = 0, held = 0;
    bool running = false;
    ns_t t0 = 0;
    uint32_t base = 0;
    uint32_t gen = 0;
};

struct Gba
{
    uint16_t siocnt = 0;         // as written (the read-only bits come from the hardware)
    uint16_t siocntShown = 0, rcntShown = 0;
    uint16_t rcnt = 0;
    uint16_t ifReal = 0;
    bool inIrq = false;
    // SIO
    bool active = false, internal = false;
    uint32_t tx = 0, rx = 0;
    int bit = 0;
    int words = 0;                // GBA-clocked words so far
    bool soOut = true;            // the bit on SO during a transfer
    bool scLevel = true;          // the clock it drives when it is master
    uint32_t gen = 0;
    Timer tm[4];
    uint16_t tmShown[4] = {}, tmCtrlShown[4] = {};
    IntrFunc vector[16] = {};
} G;

uint16_t rd16(uint32_t off) { return static_cast<uint16_t>(io[off] | (io[off + 1] << 8)); }
void wr16(uint32_t off, uint16_t v) { io[off] = static_cast<uint8_t>(v); io[off + 1] = static_cast<uint8_t>(v >> 8); }
uint32_t rd32(uint32_t off) { return rd16(off) | (static_cast<uint32_t>(rd16(off + 2)) << 16); }
void wr32(uint32_t off, uint32_t v) { wr16(off, static_cast<uint16_t>(v)); wr16(off + 2, static_cast<uint16_t>(v >> 16)); }

bool gpMode() { return (G.rcnt & 0xC000) == 0x8000; }

bool gbaDrivesSc()
{
    if (gpMode()) return G.rcnt & 0x10;
    return G.siocnt & 1;
}
bool gbaScLevel()
{
    if (gpMode()) return G.rcnt & 0x01;
    return G.active && G.internal ? G.scLevel : true;
}
bool gbaSo()
{
    if (gpMode()) return (G.rcnt & 0x80) ? (G.rcnt & 0x08) : true;   // an input floats up
    if (G.active) return G.soOut;
    return G.siocnt & 0x08;
}
bool gbaSd()
{
    if (gpMode() && (G.rcnt & 0x20)) return G.rcnt & 0x02;
    return true;   // the GBA holds SD high when it is not driving it
}

uint32_t tmPrescale(uint16_t ctrl)
{
    static const uint32_t p[4] = {1, 64, 256, 1024};
    return p[ctrl & 3];
}
double tmTickNs(int n) { return kCycleNs * tmPrescale(G.tm[n].ctrl); }

uint16_t tmCount(int n)
{
    Timer& t = G.tm[n];
    if (!t.running) return t.held;
    const uint64_t ticks = static_cast<uint64_t>((now - t.t0) / tmTickNs(n));
    return static_cast<uint16_t>(std::min<uint64_t>(t.base + ticks, 0xFFFF));
}

void tmSchedule(int n)
{
    Timer& t = G.tm[n];
    const uint32_t gen = ++t.gen;
    const ns_t when = t.t0 + static_cast<ns_t>((0x10000 - t.base) * tmTickNs(n));
    at(when, [n, gen, when] {
        Timer& t = G.tm[n];
        if (t.gen != gen || !t.running) return;
        if (t.ctrl & 0x40) G.ifReal |= static_cast<uint16_t>(8 << n);
        t.base = t.reload;
        t.t0 = when;
        tmSchedule(n);
    });
}

void tmControl(int n, uint16_t v)
{
    Timer& t = G.tm[n];
    const bool was = t.ctrl & 0x80, on = v & 0x80;
    if (was && !on)
    {
        t.held = tmCount(n);
        t.running = false;
        t.gen++;
    }
    t.ctrl = v;
    if (!was && on)
    {
        t.running = true;
        t.base = t.reload;
        t.t0 = now;
        tmSchedule(n);
    }
}

void transferComplete()
{
    sim_log("GBA %s word: sent %08x, got %08x", G.internal ? "master" : "slave", G.tx, G.rx);
    G.active = false;
    G.gen++;
    wr32(0x120, G.rx);
    if (G.siocnt & 0x4000) G.ifReal |= 0x80;
    G.siocnt &= ~0x80;
    updateLines();
}

bool siLine();

void adapterOnScRaw(bool level);
bool adapterArmedAndIdle();

// Crosstalk on SC between words: too short for the pulse counter's filter, but the
// shifter takes it as a clock. Placed while a word is armed and the line is idle.
void glitchWhenArmed(ns_t when)
{
    at(when, [when] {
        if (!adapterArmedAndIdle())
        {
            glitchWhenArmed(when + 1000);
            return;
        }
        adapterOnScRaw(false);
        adapterOnScRaw(true);
    });
}

void gbaSample()
{
    bool si = siLine();
    if (G.internal && P.corruptWord && G.words + 1 == P.corruptWord && G.bit == 5) si = !si;
    G.rx = (G.rx << 1) | (si ? 1 : 0);
    if (++G.bit == 32)
    {
        if (G.internal && ++G.words == P.glitchAfterWord) glitchWhenArmed(now + 3000);
        transferComplete();
    }
}

// GBA as master: the clock it generates, one half period per event.
void masterEdge(ns_t when, ns_t half, bool falling, uint32_t gen)
{
    at(when, [=] {
        if (gen != G.gen || !G.active) return;
        if (falling)
        {
            G.scLevel = false;
            G.soOut = (G.tx >> (31 - G.bit)) & 1;
            updateLines();
            masterEdge(when + half, half, false, gen);
        }
        else
        {
            G.scLevel = true;
            gbaSample();
            updateLines();
            if (G.active) masterEdge(when + half, half, true, gen);
        }
    });
}

ns_t halfPeriod(uint16_t siocnt)
{
    return (siocnt & 2) ? static_cast<ns_t>(4 * kCycleNs) : static_cast<ns_t>(32 * kCycleNs);
}

void siocntWrite(uint16_t v)
{
    const bool start = v & 0x80;
    G.siocnt = static_cast<uint16_t>(v & ~0x0004);
    if (gpMode()) return;
    if (G.active)
    {
        if (!start)
        {
            G.active = false;
            G.gen++;
        }
        else
        {
            G.siocnt |= 0x80;
            // A transfer armed on the external clock that switches to the internal one
            // (Sio32IDMain does this) is clocked from here on.
            if (!G.internal && (v & 1))
            {
                G.internal = true;
                G.gen++;
                masterEdge(now + halfPeriod(v), halfPeriod(v), true, G.gen);
            }
        }
    }
    else if (start)
    {
        G.active = true;
        G.internal = v & 1;
        G.tx = rd32(0x120);
        G.rx = 0;
        G.bit = 0;
        G.soOut = G.siocnt & 0x08;
        G.gen++;
        if (G.internal) masterEdge(now + halfPeriod(v), halfPeriod(v), true, G.gen);
    }
    updateLines();
}

// GBA as slave: the adapter's clock.
void gbaOnSc(bool level)
{
    if (!G.active || G.internal || gpMode()) return;
    if (!level)
    {
        G.soOut = (G.tx >> (31 - G.bit)) & 1;
    }
    else
        gbaSample();
}

void dispatchIrq();
void processWrites();

void publish()
{
    G.siocntShown = static_cast<uint16_t>((G.siocnt & ~0x0084) | (siLine() ? 4 : 0) | (G.active ? 0x80 : 0));
    wr16(0x128, G.siocntShown);
    G.rcntShown = G.rcnt;
    wr16(0x134, G.rcntShown);
    wr16(0x202, 0);
    for (int n = 0; n < 4; n++)
    {
        G.tmShown[n] = tmCount(n);
        wr16(0x100 + n * 4, G.tmShown[n]);
        G.tmCtrlShown[n] = G.tm[n].ctrl;
        wr16(0x102 + n * 4, G.tmCtrlShown[n]);
    }
}

// Writes since the last access take effect, in the order the program can make them.
void processWrites()
{
    const uint16_t rcnt = rd16(0x134);
    if (rcnt != G.rcntShown)
    {
        G.rcnt = rcnt;
        updateLines();
    }
    for (int n = 0; n < 4; n++)
    {
        const uint16_t l = rd16(0x100 + n * 4), h = rd16(0x102 + n * 4);
        if (l != G.tmShown[n]) G.tm[n].reload = l;
        if (h != G.tmCtrlShown[n]) tmControl(n, h);
    }
    const uint16_t siocnt = rd16(0x128);
    if (siocnt != G.siocntShown) siocntWrite(siocnt);
    const uint16_t ack = rd16(0x202);
    if (ack) G.ifReal &= ~ack;
}

void catchUp()
{
    processWrites();
    publish();
    dispatchIrq();
}

void gbaSpend(ns_t ns)
{
    if (current != &gbaCo) return;
    sleepUntil(now + ns);
}

void dispatchIrq()
{
    if (G.inIrq || !(rd16(0x208) & 1)) return;
    const uint16_t pending = rd16(0x200) & G.ifReal;
    if (!pending) return;
    int bit = 0;
    while (!(pending & (1 << bit))) bit++;
    G.ifReal &= ~(1 << bit);
    G.inIrq = true;
    gbaSpend(static_cast<ns_t>(P.gbaIrqEntryNs));
    if (G.vector[bit]) G.vector[bit]();
    G.inIrq = false;
    // The handler's last store has not been seen yet.
    processWrites();
    publish();
}

// ---- the adapter ----------------------------------------------------------------

struct Ada
{
    bool siSpi = false, siGpio = true;
    bool armed = false;
    uint64_t frame = 0;
    int outIdx = 0;
    bool miso = true;
    std::vector<uint8_t> rx;
    bool counting = false;
    uint32_t edges = 0;
    bool scDrive = false, scLevel = true;
    bool soIrq = false, sdIrq = false;
    uint32_t sdInterrupts = 0;
    ns_t wordDue = kNever, soDue = kNever, sdDue = kNever;
    int lock = 0;
    bool inIsr = false;
    bool notified = false, waiting = false;
    wa_isr_t isr = {};
    wa_cal_t cal = {};
    uint32_t contention = 0;
} A;

bool siLine() { return A.siSpi ? A.miso : A.siGpio; }
bool frameBit(int i) { return i < 40 && ((A.frame >> (39 - i)) & 1); }

ns_t nextIsrDue() { return std::min(A.wordDue, std::min(A.soDue, A.sdDue)); }

void wakeAdapter(ns_t t)
{
    if (current != &adaCo && !adaCo.done && t < adaCo.wakeAt) adaCo.wakeAt = t;
}

void raise(ns_t& due, double latencyNs)
{
    const ns_t t = now + static_cast<ns_t>(latencyNs);
    if (t < due) due = t;
    wakeAdapter(due);
}

void adapterOnScRaw(bool level)
{
    if (!A.armed) return;
    if (!level)
    {
        A.outIdx++;
        A.miso = frameBit(A.outIdx);
    }
    else
        A.rx.push_back(W.so ? 1 : 0);
}

bool adapterArmedAndIdle() { return A.armed && W.sc && A.edges == 0; }

void adapterOnSc(bool level)
{
    adapterOnScRaw(level);
    if (level && A.counting && ++A.edges == 32) raise(A.wordDue, P.wordIsrNs);
}

void updateLines()
{
    static bool busy = false;
    if (busy) return;
    busy = true;
    for (int pass = 0; pass < 4; pass++)
    {
        const bool gbaDrive = gbaDrivesSc();
        if (gbaDrive && A.scDrive) A.contention++;
        const bool sc = gbaDrive ? gbaScLevel() : (A.scDrive ? A.scLevel : true);
        const bool so = gbaSo();
        const bool sd = W.sdForced >= 0 ? W.sdForced != 0 : gbaSd();
        bool changed = false;
        if (sc != W.sc)
        {
            W.sc = sc;
            adapterOnSc(sc);
            gbaOnSc(sc);
            changed = true;
        }
        if (so != W.so)
        {
            W.so = so;
            if (so && A.soIrq) raise(A.soDue, P.gpioIsrNs);
            changed = true;
        }
        if (sd != W.sd)
        {
            W.sd = sd;
            if (!sd && A.sdIrq) raise(A.sdDue, P.gpioIsrNs);
            changed = true;
        }
        if (!changed) break;
    }
    busy = false;
}

void preempt()
{
    if (A.lock || A.inIsr) return;
    for (;;)
    {
        if (A.wordDue <= now)
        {
            A.wordDue = kNever;
            A.inIsr = true;
            A.isr.word();
            A.inIsr = false;
        }
        else if (A.soDue <= now)
        {
            A.soDue = kNever;
            A.inIsr = true;
            A.isr.so_rise();
            A.inIsr = false;
        }
        else if (A.sdDue <= now)
        {
            A.sdDue = kNever;
            A.inIsr = true;
            A.sdInterrupts++;
            A.isr.sd_fall();
            A.inIsr = false;
        }
        else
            return;
    }
}

void adaSpend(ns_t ns)
{
    if (current != &adaCo) return;
    const ns_t end = now + ns;
    for (;;)
    {
        ns_t wake = end;
        if (!A.lock && !A.inIsr) wake = std::min(wake, nextIsrDue());
        if (wake > now) sleepUntil(wake);
        preempt();
        if (now >= end) return;
    }
}

void call() { adaSpend(static_cast<ns_t>(P.adaCallNs)); }

// ---- the network side -------------------------------------------------------------

std::vector<uint8_t> outStream;
std::vector<std::vector<uint8_t>> outFrames;
std::vector<uint16_t> statusCodes;

int rfu1Size(uint32_t type)
{
    if (type == 0) return 36;
    if (type == 5 || type == 6) return 104;
    return type <= 8 ? 16 : -1;
}

uint32_t be32(const uint8_t* p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }

void netData(const uint8_t* bytes, size_t length, void*)
{
    if (length < 64) return;   // telemetry
    outStream.insert(outStream.end(), bytes, bytes + length);
    size_t at = 0;
    while (outStream.size() - at >= 12)
    {
        if (std::memcmp(&outStream[at], "RFU1", 4) != 0)
        {
            at++;
            continue;
        }
        const int size = rfu1Size(be32(&outStream[at + 4]));
        if (size < 0)
        {
            at++;
            continue;
        }
        if (outStream.size() - at < static_cast<size_t>(size)) break;
        outFrames.emplace_back(outStream.begin() + at, outStream.begin() + at + size);
        at += size;
    }
    outStream.erase(outStream.begin(), outStream.begin() + at);
}

void netStatus(uint16_t code, void*) { statusCodes.push_back(code); }

// The network side runs on another task: it waits for the adapter's lock like the
// transport task does on the chip.
void feedWhenUnlocked(std::vector<uint8_t> frame)
{
    if (A.lock)
    {
        at(now + 1000, [frame] { feedWhenUnlocked(frame); });
        return;
    }
    wa_adapter_feed(frame.data(), frame.size());
}

}  // namespace

// ---- wa_port.h on the simulated wire ------------------------------------------------

extern "C" {

bool wa_port_init(const wa_pins_t*, const wa_isr_t* isr, wa_cal_t* cal)
{
    A.isr = *isr;
    std::memset(&A.cal, 0, sizeof(A.cal));
    A.cal.ok = true;
    A.cal.preclock = P.preclock;
    A.cal.rx_offset = P.preclock ? 1 : 0;
    A.cal.word_bits = P.noBitCount ? 0 : static_cast<uint16_t>(32 + (P.preclock ? 1 : 0));
    std::snprintf(A.cal.detail, sizeof(A.cal.detail), "simulated slave%s", P.preclock ? ", pre-clocked" : "");
    if (cal) *cal = A.cal;
    return true;
}

bool wa_port_calibrate(wa_cal_t* cal)
{
    if (cal) *cal = A.cal;
    return true;
}

void wa_port_arm(uint32_t reply, bool lead)
{
    call();
    A.frame = (static_cast<uint64_t>(lead ? 1 : 0) << 39) | (static_cast<uint64_t>(reply) << 7);
    A.armed = true;
    A.rx.clear();
    A.outIdx = 0;
    A.miso = frameBit(0);
    if (P.preclock) A.rx.push_back(0);   // the pre-clock's own sample
    if (A.counting) A.edges = 0;
}

uint32_t wa_port_collect(uint16_t* bits)
{
    call();
    A.armed = false;
    if (bits) *bits = static_cast<uint16_t>(A.rx.size());
    if (A.rx.size() != A.cal.word_bits) sim_log("adapter collected %zu bits", A.rx.size());
    uint32_t word = 0;
    for (int i = 0; i < 32; i++)
    {
        const size_t at = A.cal.rx_offset + i;
        word = (word << 1) | (at < A.rx.size() ? A.rx[at] : 0);
    }
    return word;
}

void wa_port_disarm(void) { call(); A.armed = false; }
uint32_t wa_port_edges(void) { call(); return A.edges; }

uint32_t wa_port_ones(void)
{
    call();
    uint32_t ones = 0;
    for (size_t i = A.cal.rx_offset; i < A.rx.size(); i++) ones += A.rx[i];
    return ones;
}

void wa_port_word_irq(bool on)
{
    call();
    if (on == A.counting) return;
    A.counting = on;
    if (on) A.edges = 0;
}

void wa_port_so_irq(bool on)
{
    call();
    A.soIrq = on;
    A.soDue = kNever;
}

void wa_port_sd_irq(bool on)
{
    call();
    A.sdIrq = on;
    A.sdDue = kNever;
}

void wa_port_si_drive(bool level) { call(); A.siSpi = false; A.siGpio = level; }
void wa_port_si_spi(void) { call(); A.siSpi = true; }

void wa_port_sc_drive(bool on)
{
    call();
    A.scDrive = on;
    A.scLevel = true;
    updateLines();
}

bool wa_port_master_word(uint32_t word, uint32_t* rx, volatile const bool* stop)
{
    uint32_t in = 0;
    for (int bit = 31; bit >= 0; bit--)
    {
        if (*stop) return false;
        A.scLevel = false;
        A.siSpi = false;
        A.siGpio = (word >> bit) & 1;
        updateLines();
        adaSpend(2000);
        in = (in << 1) | (W.so ? 1 : 0);
        adaSpend(1000);
        A.scLevel = true;
        updateLines();
        adaSpend(3000);
    }
    *rx = in;
    return true;
}

bool wa_port_sc(void) { call(); return W.sc; }
bool wa_port_so(void) { call(); return W.so; }
bool wa_port_si(void) { call(); return siLine(); }
bool wa_port_sd(void) { call(); return W.sd; }

bool wa_port_spawn(void (*entry)(void*), void* arg)
{
    adaEntry = entry;
    adaArg = arg;
    startCo(adaCo, runAda);
    return true;
}

uint32_t wa_port_ms(void) { return static_cast<uint32_t>(now / 1000000); }
uint32_t wa_port_us(void) { call(); return static_cast<uint32_t>(now / 1000); }
void wa_port_delay_us(uint32_t us) { adaSpend(static_cast<ns_t>(us) * 1000); }
void wa_port_lock(void) { A.lock++; }
void wa_port_unlock(void)
{
    if (--A.lock == 0 && current == &adaCo) preempt();
}
void wa_port_lock_isr(void) { A.lock++; }
void wa_port_unlock_isr(void) { A.lock--; }

void wa_port_wake_isr(void) { wa_port_wake(); }
void wa_port_wake(void)
{
    A.notified = true;
    if (A.waiting) wakeAdapter(now);
}

void wa_port_wait(uint32_t ms)
{
    A.waiting = true;
    const ns_t end = now + static_cast<ns_t>(ms) * 1000000;
    while (!A.notified && now < end)
    {
        sleepUntil(std::min(end, nextIsrDue()));
        preempt();
    }
    A.notified = false;
    A.waiting = false;
}

uint16_t wa_port_random16(void)
{
    randState = randState * 1103515245 + 12345;
    return static_cast<uint16_t>(randState >> 16);
}

// ---- sim_gba.h ------------------------------------------------------------------------

void* sim_gba_reg(uint32_t offset)
{
    gbaSpend(static_cast<ns_t>(P.gbaRegNs));
    catchUp();
    return &io[offset];
}

void sim_gba_cycles(uint32_t cycles)
{
    gbaSpend(static_cast<ns_t>(cycles * kCycleNs));
    catchUp();
}

void sim_gba_intr_enable(uint16_t flags) { wr16(0x200, rd16(0x200) | flags); }

// ---- sim_api.h --------------------------------------------------------------------------

void sim_set_vector(int bit, IntrFunc fn) { G.vector[bit] = fn; }
IntrFunc* sim_vector_slot(int bit) { return &G.vector[bit]; }
uint64_t sim_now_us(void) { return now / 1000; }

void sim_net_feed(const uint8_t* frame, size_t length)
{
    std::vector<uint8_t> copy(frame, frame + length);
    at(now, [copy] { feedWhenUnlocked(copy); });
}

int sim_net_count(uint32_t type)
{
    int n = 0;
    for (const auto& f : outFrames)
        if (be32(&f[4]) == type) n++;
    return n;
}

int sim_net_last(uint32_t type, uint8_t* out, size_t capacity)
{
    for (auto it = outFrames.rbegin(); it != outFrames.rend(); ++it)
        if (be32(&(*it)[4]) == type)
        {
            const size_t n = std::min(capacity, it->size());
            std::memcpy(out, it->data(), n);
            return static_cast<int>(n);
        }
    return 0;
}

int sim_net_status_count(uint16_t code)
{
    return static_cast<int>(std::count(statusCodes.begin(), statusCodes.end(), code));
}

void sim_net_enter_mode(uint8_t role)
{
    at(now, [role] { wa_adapter_enter_mode(role); });
}

uint32_t sim_contention(void) { return A.contention; }
int sim_adapter_clocking(void) { return A.scDrive; }
uint32_t sim_sd_interrupts(void) { return A.sdInterrupts; }

void sim_sd_force(int level)
{
    W.sdForced = level;
    updateLines();
}

void sim_log(const char* fmt, ...)
{
    if (!P.verbose) return;
    std::printf("%10.3f ms  ", now / 1e6);
    va_list ap;
    va_start(ap, fmt);
    std::vprintf(fmt, ap);
    va_end(ap);
    std::printf("\n");
}

int sim_run(const SimParams* params, void (*gba)(void), uint64_t limit_us)
{
    P = *params;
    now = 0;
    while (!events.empty()) events.pop();
    std::memset(io, 0, sizeof(io));
    G = Gba{};
    A = Ada{};
    W = Wire{};
    outStream.clear();
    outFrames.clear();
    statusCodes.clear();
    publish();

    const wa_config_t config = {{5, 6, 7, 8}, 0, P.spinUs};
    const wa_transport_t transport = {netData, netStatus, nullptr};
    wa_adapter_start(&config, &transport);

    gbaEntry = gba;
    startCo(gbaCo, runGba);
    // The adapter powers up first.
    gbaCo.wakeAt = 50000;

    const ns_t limit = limit_us * 1000;
    while (!gbaCo.done)
    {
        const ns_t te = events.empty() ? kNever : events.top().t;
        const ns_t tg = gbaCo.done ? kNever : gbaCo.wakeAt;
        const ns_t ta = adaCo.done ? kNever : adaCo.wakeAt;
        const ns_t t = std::min(te, std::min(tg, ta));
        if (t == kNever || t > limit) return 0;
        if (t > now) now = t;
        if (te == t)
        {
            Event ev = events.top();
            events.pop();
            ev.fn();
            continue;
        }
        Co* c = (tg == t) ? &gbaCo : &adaCo;
        c->wakeAt = kNever;
        current = c;
        swapcontext(&schedCtx, &c->ctx);
        current = nullptr;
    }
    return 1;
}

}  // extern "C"
