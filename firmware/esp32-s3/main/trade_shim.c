#include "trade_shim.h"

#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_attr.h"
#else
#define RTC_NOINIT_ATTR
#endif

#define RFUCMD_READY_EXIT_STANDBY 0x6600
#define RFUCMD_SEND_BLOCK_INIT 0x8800
#define RFUCMD_SEND_BLOCK 0x8900
#define RFUCMD_SEND_BLOCK_REQ 0xa100
#define LINKCMD_CONFIRM_FINISH_TRADE 0xdcba

/* Post-trade standby rounds a retail FireRed/LeafGreen cartridge runs: trade_scene.c
   CB2_SaveAndEndTrade cases 1, 41, 5 and 8, then trade.c CB2_CreateTradeMenu case 4.
   The parent answers each; after the fifth answer it is parked in its sixth barrier. */
#define TRADE_SHIM_CHILD_ROUNDS 5
/* Quiet time after that fifth answer before the extra round goes in: long enough for the
   parent to have processed its own answer (a frame or two) and for a parent that did NOT
   need a sixth round to have sent its party request instead (~12 frames). */
#define TRADE_SHIM_SETTLE_MS 750
/* For a cartridge with a different round count: any post-trade answer followed by this
   much silence on both sides also counts as the deadlock. Longer than the cartridge's
   save, which is the longest legitimate gap between its rounds. */
#define TRADE_SHIM_FALLBACK_MS 8000
/* The parent parked in its barrier answers the extra round within a frame or two. */
#define TRADE_SHIM_ACK_MS 2000
#define TRADE_SHIM_MAX_ATTEMPTS 4
#define TRADE_SHIM_MAX_FAKES 16
/* A child that accepted a block request starts sending within a couple of frames.
   One that has sent nothing at all for this long since the request refused it
   (Rfu_InitBlockSend returns FALSE while a previous block send or another command
   is pending), and the parent never repeats it. Requiring silence, not just time,
   keeps a slow but accepted request from being answered twice. */
#define TRADE_SHIM_REREQUEST_MS 700
#define TRADE_SHIM_MAX_REREQUESTS 3
/* Fragments per block request type, sBlockRequests[] in link_rfu_2.c: 200, 200,
   100, 220 and 40 bytes in 12-byte fragments. */
static const uint8_t kRequestFragments[] = {17, 17, 9, 19, 4};

/* ---- reset-surviving event log ---------------------------------------------- */

enum { EV_BOOT = 1, EV_RESET, EV_CONFIRM, EV_PARENT_STANDBY, EV_CHILD_STANDBY, EV_REQUEST, EV_INJECT,
       EV_ACK, EV_REANSWER, EV_REREQUEST, EV_NOTE, EV_HOST_BLOCK, EV_GIVE_UP, EV_ADAPTER, EV_BRIDGE, EV_TRACE };

#define LOG_MAGIC 0x53484d32u   /* "SHM2" */
#define LOG_SLOTS 256

typedef struct { uint32_t ms; uint8_t type, a; uint16_t b, c; } log_entry_t;

static RTC_NOINIT_ATTR struct
{
    uint32_t magic;
    uint16_t head, count;
    uint16_t boots;
    log_entry_t entries[LOG_SLOTS];
} rtc_log;

static void log_event(int64_t now_ms, uint8_t type, uint8_t a, uint16_t b, uint16_t c)
{
    if (rtc_log.magic != LOG_MAGIC) return;
    log_entry_t *e = &rtc_log.entries[(rtc_log.head + rtc_log.count) % LOG_SLOTS];
    if (rtc_log.count == LOG_SLOTS) rtc_log.head = (uint16_t)((rtc_log.head + 1) % LOG_SLOTS);
    else ++rtc_log.count;
    e->ms = (uint32_t)now_ms; e->type = type; e->a = a; e->b = b; e->c = c;
}

void trade_shim_boot(int reset_reason)
{
    if (rtc_log.magic != LOG_MAGIC || rtc_log.head >= LOG_SLOTS || rtc_log.count > LOG_SLOTS)
    {
        memset(&rtc_log, 0, sizeof(rtc_log));
        rtc_log.magic = LOG_MAGIC;
    }
    ++rtc_log.boots;
    log_event(0, EV_BOOT, (uint8_t)reset_reason, rtc_log.boots, 0);
}

/* ---- state ------------------------------------------------------------------- */

static struct
{
    uint16_t fakes[TRADE_SHIM_MAX_FAKES];   /* injected rounds, parent numbering, ascending */
    int fake_count;
    int base;                 /* fakes older than the list, folded into a plain offset */
    uint8_t last_tag;         /* newest tag forwarded to the parent */
    bool have_tag;
    bool post_trade;          /* between the parent's trade confirmation and its next block request */
    bool injected;            /* this trade's extra round has been supplied */
    int last_round;           /* parent numbering of the parent's newest standby answer, -1 none */
    int last_child_round;     /* parent numbering of the newest forwarded child standby command */
    int answers_since_confirm;
    int fake_round;           /* this trade's extra round, parent numbering, -1 none */
    uint8_t fake_tag;
    bool fake_acked;
    int attempts;
    int64_t injected_at;
    int64_t quiet_since;
    int host_block_count;     /* fragment count announced by the parent's last block */
    int request_type;         /* parent's newest block request, -1 none */
    bool request_served;      /* the child has started answering it */
    int64_t request_at;       /* when it (or its latest repeat) went to the child */
    int request_attempts;
    int injections, reanswers, rerequests, trades, give_ups;
    uint16_t child_commands;  /* command frames forwarded to the parent (injected ones included) */
    uint16_t echoes;          /* host frames that echoed a child command back */
    bool raw_tag_valid;
    uint8_t last_raw_tag;
    int tag_gaps;
    /* The child's block in flight: the parent echoes each accepted fragment once, in
       slot 1, and the child's link layer treats a missing echo as a send failure it
       cannot actually repair (its resends carry no sequence tag). */
    struct { bool active; uint8_t count; uint32_t sent, echoed; uint8_t data[32][12]; } cb;
    /* The parent's block in flight, for gap forensics only. */
    struct { bool active; uint8_t count, expect; } pb;
    int echoes_synthesized, resends_restamped;
    /* The child's newest command frame exactly as the GBA sent it, and the tag it
       was forwarded with. */
    uint8_t last_frame[16];
    bool have_last_frame;
    uint8_t last_frame_tag;
    int64_t last_command_ms;
    uint32_t request_frags;   /* fragment indices the child has sent since the request */
    int duplicates;
} s;

static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static void wr16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

void trade_shim_reset(void)
{
    memset(&s, 0, sizeof(s));
    s.last_round = -1;
    s.fake_round = -1;
    s.request_type = -1;
    log_event(0, EV_RESET, 0, 0, 0);
}

void trade_shim_note(int64_t now_ms, uint8_t kind, uint16_t b, uint16_t c) { log_event(now_ms, EV_NOTE, kind, b, c); }

/* ---- loss counters ------------------------------------------------------------ */

#define TELEMETRY_PERIOD_MS 30000

static struct
{
    /* Pico, from tags 0x1D / 0x2E / 0x1E */
    uint16_t park_drops, park_idle_drops, uart_overflows, fifo_drops, aborts, retries;
    uint8_t park_high, fifo_high, sd_resets, deliv_fail, comstate;
    uint16_t err_responses, login_restarts;
    uint8_t ev_data, ev_rtx, ev_timeo, ev_disc, link_pwr_zero, wipe15, wipe16, wipe17;
    uint8_t last_cmd, last_plen, ring[8];
    uint16_t commands, out_ring_drops;
    uint16_t gba_restarts;
    uint8_t last_trace_seq;
    uint16_t trace_snapshots;
    bool seen;
    /* bridge */
    uint16_t reordered, hold_dropped, overflow, queued_high, bad_frames, decrypt_failures, unzip_failures;
    uint16_t host_skips, host_long_skips;   /* host frames whose adapter clock jumped by 2 / by 3 or more */
    int64_t last_logged, last_change_logged;
    uint32_t last_hash;
} t;

static uint32_t loss_hash(void)
{
    return (uint32_t)t.park_drops * 0x9e3779b1u ^ (uint32_t)t.uart_overflows * 0x85ebca6bu ^ (uint32_t)t.fifo_drops * 0xc2b2ae35u
         ^ (uint32_t)t.aborts * 0x27d4eb2fu ^ (uint32_t)t.retries * 0x165667b1u ^ (uint32_t)t.sd_resets * 0xd3a2646cu
         ^ (uint32_t)t.err_responses * 0xfd7046c5u ^ (uint32_t)t.login_restarts * 0xb55a4f09u
         ^ (uint32_t)t.hold_dropped * 0x1b873593u ^ (uint32_t)t.overflow * 0xcc9e2d51u
         ^ (uint32_t)t.ev_disc * 0x2545f491u ^ (uint32_t)t.ev_timeo * 0x9e3779b9u ^ (uint32_t)t.link_pwr_zero * 0x7feb352du
         ^ (uint32_t)(t.wipe15 | (t.wipe16 << 8) | ((t.wipe17 & 0xf0) << 16)) * 0x846ca68bu
         ^ (uint32_t)t.out_ring_drops * 0x4cf5ad43u
         ^ (uint32_t)t.gba_restarts * 0x9e3779b7u
         ^ (uint32_t)(t.bad_frames | (t.decrypt_failures << 8) | (t.unzip_failures << 16)) * 0x3c6ef372u;
}

static void log_counters(int64_t now_ms, bool changed)
{
    uint8_t why = changed ? 1 : 0;
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x10 | why), t.park_drops, t.uart_overflows);
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x20 | why), t.fifo_drops, (uint16_t)(t.fifo_high | (t.park_high << 8)));
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x30 | why), t.aborts, t.retries);
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x40 | why), (uint16_t)(t.sd_resets | (t.deliv_fail << 8)),
              (uint16_t)((t.err_responses > 255 ? 255 : t.err_responses) | ((t.login_restarts > 255 ? 255 : t.login_restarts) << 8)));
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x50 | why), t.park_idle_drops, t.comstate);
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x60 | why), (uint16_t)(t.ev_timeo | (t.ev_disc << 8)), (uint16_t)(t.ev_data | (t.ev_rtx << 8)));
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x70 | why), (uint16_t)(t.link_pwr_zero | (t.wipe15 << 8)), (uint16_t)(t.wipe16 | (t.wipe17 << 8)));
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x80 | why), (uint16_t)(t.ring[0] | (t.ring[1] << 8)), (uint16_t)(t.ring[2] | (t.ring[3] << 8)));
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0x90 | why), (uint16_t)(t.ring[4] | (t.ring[5] << 8)), (uint16_t)(t.ring[6] | (t.ring[7] << 8)));
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0xa0 | why), (uint16_t)(t.last_cmd | (t.last_plen << 8)), t.commands);
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0xb0 | why), t.out_ring_drops, t.queued_high);
    log_event(now_ms, EV_ADAPTER, (uint8_t)(0xc0 | why), t.gba_restarts, t.trace_snapshots);
    log_event(now_ms, EV_BRIDGE, why, t.hold_dropped, (uint16_t)(t.reordered | (t.overflow << 8)));
    log_event(now_ms, EV_BRIDGE, (uint8_t)(2 | why), t.bad_frames, (uint16_t)(t.decrypt_failures | (t.unzip_failures << 8)));
    log_event(now_ms, EV_BRIDGE, (uint8_t)(4 | why), t.host_skips, t.host_long_skips);
    log_event(now_ms, EV_BRIDGE, (uint8_t)(6 | why), s.child_commands, s.echoes);
    t.last_logged = now_ms;
    t.last_hash = loss_hash();
}

static void consider_logging(int64_t now_ms)
{
    if (!t.seen) return;
    bool changed = loss_hash() != t.last_hash;
    if (changed && now_ms - t.last_change_logged >= 1000) { t.last_change_logged = now_ms; log_counters(now_ms, true); }
    else if (now_ms - t.last_logged >= TELEMETRY_PERIOD_MS) log_counters(now_ms, false);
}

void trade_shim_adapter(int64_t now_ms, const uint8_t *f, size_t n)
{
    if (n == 25 && f[0] == 0x1d)
    {
        t.ev_data = f[1]; t.ev_rtx = f[2]; t.ev_timeo = f[3]; t.ev_disc = f[4];
        t.link_pwr_zero = f[9];
        t.wipe15 = f[15]; t.wipe16 = f[16]; t.wipe17 = f[17];
        t.sd_resets = f[14];
        if (f[18] > t.park_drops) t.park_drops = f[18];   /* the 0x1E copy is wider */
        if (f[19] > t.fifo_drops) t.fifo_drops = f[19];
        if (f[20] > t.fifo_high) t.fifo_high = f[20];
    }
    else if (n == 62 && f[0] == 0x2e)
    {
        t.comstate = f[1];
        t.last_cmd = f[5]; t.last_plen = f[6];
        memset(t.ring, 0, sizeof(t.ring));
        for (int i = 0; i < 8 && i < f[7]; ++i) t.ring[i] = f[8 + i];
        t.commands = rd16(f + 28);
        uint32_t aborts = rd16(f + 32) | ((uint32_t)rd16(f + 34) << 16);
        uint32_t restarts = rd16(f + 36) | ((uint32_t)rd16(f + 38) << 16);
        uint32_t err = rd16(f + 40) | ((uint32_t)rd16(f + 42) << 16);
        uint32_t retries = rd16(f + 58) | ((uint32_t)rd16(f + 60) << 16);
        t.aborts = aborts > 65535 ? 65535 : (uint16_t)aborts;
        t.login_restarts = restarts > 65535 ? 65535 : (uint16_t)restarts;
        t.err_responses = err > 65535 ? 65535 : (uint16_t)err;
        t.retries = retries > 65535 ? 65535 : (uint16_t)retries;
        t.deliv_fail = f[56];
    }
    else if (n >= 15 && f[0] == 0x1e)
    {
        if (n >= 17) t.out_ring_drops = rd16(f + 15);
        if (n >= 19) t.gba_restarts = rd16(f + 17);
        t.uart_overflows = rd16(f + 1);
        if (f[3] > t.park_high) t.park_high = f[3];
        t.park_drops = rd16(f + 4);
        t.fifo_drops = rd16(f + 6);
        if (f[8] > t.fifo_high) t.fifo_high = f[8];
        t.aborts = rd16(f + 9);
        t.retries = rd16(f + 11);
        t.park_idle_drops = rd16(f + 13);
    }
    else if (n >= 4 && f[0] == 0x2d && f[1] != t.last_trace_seq)
    {
        /* The adapter's exchange trace at the moment the game reset it mid-link:
           the commands, events and restarts that led up to a game-side link error. */
        t.last_trace_seq = f[1];
        ++t.trace_snapshots;
        uint8_t count = f[2];
        log_event(now_ms, EV_NOTE, TRADE_SHIM_NOTE_ADAPTER_TRACE, f[1], (uint16_t)((count << 8) | f[3]));
        for (uint8_t i = 0; i < count && 4 + 4 * (size_t)i + 4 <= n; ++i)
        {
            const uint8_t *e = f + 4 + 4 * i;
            log_event(now_ms, EV_TRACE, e[0], e[1], rd16(e + 2));
        }
        printf("LDN_BRIDGE adapter was reset by the GBA mid-link; its exchange trace is in LDN_SHIM_LOG\n");
    }
    else return;
    if (!t.seen) { t.seen = true; log_counters(now_ms, false); return; }
    consider_logging(now_ms);
}

void trade_shim_bridge_counters(int64_t now_ms, uint16_t reordered, uint16_t hold_dropped, uint16_t overflow, uint16_t queued,
                                uint16_t bad_frames, uint16_t decrypt_failures, uint16_t unzip_failures,
                                uint16_t host_skips, uint16_t host_long_skips)
{
    t.host_skips = host_skips;
    t.host_long_skips = host_long_skips;
    t.reordered = reordered;
    t.hold_dropped = hold_dropped;
    t.overflow = overflow;
    t.bad_frames = bad_frames;
    t.decrypt_failures = decrypt_failures;
    t.unzip_failures = unzip_failures;
    if (queued > t.queued_high) t.queued_high = queued;
    consider_logging(now_ms);
}

/* Round-number mapping. A child round c is the parent's round c plus every injected
   round at or below it; an injected round has no child counterpart. */
static int to_parent(int c)
{
    int p = c + s.base;
    for (int i = 0; i < s.fake_count; ++i) if (s.fakes[i] <= p) ++p;
    return p;
}
static int to_child(int p)
{
    int c = p - s.base;
    for (int i = 0; i < s.fake_count; ++i) if (s.fakes[i] < p) --c;
    return c;
}
static bool is_fake(int p)
{
    for (int i = 0; i < s.fake_count; ++i) if (s.fakes[i] == p) return true;
    return false;
}
static void add_fake(int p)
{
    if (s.fake_count == TRADE_SHIM_MAX_FAKES)
    {
        /* Rounds only move forward, so the oldest one is far behind every number
           still in play and can become part of the plain offset. */
        memmove(s.fakes, s.fakes + 1, sizeof(s.fakes) - sizeof(s.fakes[0]));
        --s.fake_count;
        ++s.base;
    }
    s.fakes[s.fake_count++] = (uint16_t)p;
}

/* A parent UNI frame for the child: 3-byte LLSF header (one child slot, UNI state,
   70 data bytes) and five 14-byte slots. */
static size_t build_host_frame(uint8_t *out, uint16_t cmd0, uint16_t v0, uint16_t cmd1, uint16_t v1)
{
    memset(out, 0, 73);
    out[0] = 0x46; out[1] = 0x00; out[2] = 0x05;
    wr16(out + 3, cmd0); wr16(out + 5, v0);
    wr16(out + 17, cmd1); wr16(out + 19, v1);
    return 73;
}

static size_t build_child_frame(uint8_t *out, uint8_t tag, uint16_t round)
{
    memset(out, 0, 16);
    out[0] = 0x0e; out[1] = 0x10;                    /* child UNI frame, 14-byte slot */
    out[2] = (uint8_t)(tag << 5);
    out[3] = RFUCMD_READY_EXIT_STANDBY >> 8;
    wr16(out + 4, round);
    return 16;
}

size_t trade_shim_child(uint8_t *payload, size_t length, int64_t now_ms, uint8_t *reply, size_t reply_capacity)
{
    if (length < 16) return 0;
    if (((rd16(payload) >> 10) & 15) != 4) return 0;  /* not a UNI frame */
    uint8_t *slot = payload + 2;
    if (slot[1] == 0) return 0;                        /* idle: no command, no tag */
    s.quiet_since = now_ms;
    uint16_t command = (uint16_t)((slot[1] << 8) | (slot[0] & 0x1f));
    uint8_t raw_tag = (uint8_t)(slot[0] >> 5);
    uint8_t index = (uint8_t)(slot[0] & 0x1f);
    bool fragment = (command & 0xff00) == RFUCMD_SEND_BLOCK;
    s.last_command_ms = now_ms;
    if (s.have_last_frame && memcmp(payload, s.last_frame, 16) == 0)
    {
        /* The same frame twice in a row: the adapter handed one GBA transfer over again
           (librfu re-runs an exchange whose acknowledgement it misread). The game's own
           commands always advance the tag, so this is not a new command. Forward it as
           it went the first time, so the bridge's repeat filter drops it and the parent
           never counts it; renumbering it would make the parent act on it twice. */
        slot[0] = (uint8_t)((s.last_frame_tag << 5) | index);
        if (command == RFUCMD_READY_EXIT_STANDBY) wr16(slot + 2, (uint16_t)to_parent(rd16(slot + 2)));
        ++s.duplicates;
        return 0;
    }
    memcpy(s.last_frame, payload, 16);
    s.have_last_frame = true;
    /* Every command frame leaves here with the tag that follows the last one forwarded.
       The parent accepts a command only when its tag is the previous one plus one and
       gives up on the child after five misses in a row, so a gap in the child's own
       stamps must not reach it: a frame lost between the GBA and this point (the
       block-level resend below repairs that) or the game's block-resend path, which
       queues fragments without a stamp (HandleSendFailure), so they arrive as tag 0.
       The raw stamps are still checked, for the log. */
    uint8_t tag = s.have_tag ? (uint8_t)((s.last_tag + 1) & 7) : raw_tag;
    bool resend = false;
    if (s.raw_tag_valid && raw_tag != ((s.last_raw_tag + 1) & 7))
    {
        resend = raw_tag == 0 && fragment && s.cb.active && index + 1 < s.cb.count;
        if (resend)
        {
            ++s.resends_restamped;
            log_event(now_ms, EV_NOTE, TRADE_SHIM_NOTE_RESTAMPED_RESEND, index, tag);
        }
        else
        {
            ++s.tag_gaps;
            log_event(now_ms, EV_NOTE, TRADE_SHIM_NOTE_CHILD_TAG_GAP, s.last_raw_tag, raw_tag);
        }
    }
    if (!resend)                                       /* a resend sits between two stamped frames */
    {
        s.last_raw_tag = raw_tag;
        s.raw_tag_valid = true;
    }
    slot[0] = (uint8_t)((tag << 5) | index);
    s.last_tag = tag;
    s.last_frame_tag = tag;
    s.have_tag = true;
    ++s.child_commands;
    if (command == RFUCMD_SEND_BLOCK_INIT)
    {
        uint16_t count = rd16(slot + 2);
        if (!s.cb.active || count != s.cb.count || s.cb.sent == 0)
        {
            s.cb.active = count >= 1 && count <= 32;
            s.cb.count = (uint8_t)count;
            s.cb.sent = s.cb.echoed = 0;
        }
    }
    else if (fragment && s.cb.active && index < s.cb.count)
    {
        memcpy(s.cb.data[index], slot + 2, 12);
        s.cb.sent |= 1u << index;
    }
    if (s.request_type >= 0 && !s.request_served)
    {
        /* The answer is a block of the requested size: its INIT, or -- should every INIT
           copy have been lost -- all of its fragments. A block the child sends on its own
           (its 20-byte trade menu messages) or the resend loop of an earlier block is not. */
        size_t type = (size_t)s.request_type;
        if (type >= sizeof(kRequestFragments)) s.request_served = true;
        else if (command == RFUCMD_SEND_BLOCK_INIT) s.request_served = rd16(slot + 2) == kRequestFragments[type];
        else if (fragment && index < 32)
        {
            uint32_t all = (1u << kRequestFragments[type]) - 1;
            s.request_frags |= 1u << index;
            s.request_served = (s.request_frags & all) == all;
        }
    }
    if (command != RFUCMD_READY_EXIT_STANDBY) return 0;

    int c = rd16(slot + 2), p = to_parent(c);
    wr16(slot + 2, (uint16_t)p);
    if (p > s.last_child_round) s.last_child_round = p;
    log_event(now_ms, EV_CHILD_STANDBY, (uint8_t)((raw_tag << 4) | tag), (uint16_t)c, (uint16_t)p);

    /* The parent answered this barrier already and has left it; it will never answer
       again, so the child's retry means the answer was lost on its way down. Complete
       the child's barrier on the parent's behalf. A late genuine copy is ignored by the
       child, whose counter has moved on. */
    if (p > s.last_round || reply_capacity < 146) return 0;
    size_t n = build_host_frame(reply, RFUCMD_READY_EXIT_STANDBY, (uint16_t)c, RFUCMD_READY_EXIT_STANDBY, (uint16_t)c);
    ++s.reanswers;
    log_event(now_ms, EV_REANSWER, 0, (uint16_t)c, (uint16_t)p);
    printf("LDN_BRIDGE GBA is retrying standby round %d that the Switch already answered (as %d), answering for it\n", c, p);
    /* A party request that arrived while the child was still inside that barrier was
       refused by its link layer and the parent does not repeat it. */
    if (s.request_type >= 0 && !s.request_served && s.request_attempts < TRADE_SHIM_MAX_REREQUESTS)
    {
        n += build_host_frame(reply + n, RFUCMD_SEND_BLOCK_REQ, (uint16_t)s.request_type, 0, 0);
        ++s.request_attempts;
        ++s.rerequests;
        s.request_at = now_ms;
        log_event(now_ms, EV_REREQUEST, 0, (uint16_t)s.request_type, 0);
        printf("LDN_BRIDGE repeating the Switch's party request %d to the GBA\n", s.request_type);
    }
    return n;
}

size_t trade_shim_host(uint8_t *payload, size_t length, int64_t now_ms, uint8_t *pre, size_t pre_capacity)
{
    size_t pre_len = 0;
    if (length < 3) return 0;
    uint32_t header = payload[0] | (payload[1] << 8) | ((uint32_t)payload[2] << 16);
    if (((header >> 14) & 15) != 4) return 0;           /* not a UNI frame */
    size_t size = header & 0x7f;
    if (size < 28 || 3 + size > length) return 0;
    for (int i = 0; i < 2; ++i)                       /* slot 0: the parent, slot 1: this child's echo */
    {
        uint8_t *slot = payload + 3 + 14 * i;
        uint16_t command = rd16(slot), value = rd16(slot + 2);
        if (command == 0) continue;
        s.quiet_since = now_ms;
        if (i == 1) ++s.echoes;
        if (i == 1 && (command & 0xff00) == RFUCMD_SEND_BLOCK && s.cb.active)
        {
            uint8_t index = (uint8_t)(command & 0x1f);
            if (index < s.cb.count) s.cb.echoed |= 1u << index;
            if (index + 1 == s.cb.count)
            {
                /* The child acts on this echo: any earlier fragment it sent that was
                   never echoed becomes a "send failure". Supply those echoes first. */
                uint32_t below = (1u << index) - 1;
                uint32_t missing = s.cb.sent & ~s.cb.echoed & below;
                if (missing) log_event(now_ms, EV_NOTE, TRADE_SHIM_NOTE_ECHO_GAP, (uint16_t)missing, s.cb.count);
                for (uint8_t k = 0; k < index && pre_len + 73 <= pre_capacity; ++k)
                {
                    if (!(missing & (1u << k))) continue;
                    uint8_t *f = pre + pre_len;
                    memset(f, 0, 73);
                    memcpy(f, payload, 3);                     /* same LLSF header as the real frame */
                    f[17] = k; f[18] = RFUCMD_SEND_BLOCK >> 8; /* slot 1: the echo, tag already stripped */
                    memcpy(f + 19, s.cb.data[k], 12);
                    pre_len += 73;
                    s.cb.echoed |= 1u << k;
                    ++s.echoes_synthesized;
                    log_event(now_ms, EV_NOTE, TRADE_SHIM_NOTE_ECHO_SYNTHESIZED, k, s.cb.count);
                    printf("LDN_BRIDGE echo of the GBA's fragment %u never came back from the Switch, supplying it\n", k);
                }
            }
        }
        if (i == 0 && (command & 0xff00) == RFUCMD_SEND_BLOCK && s.pb.active)
        {
            /* Forensics only: the parent sends each fragment once, so a gap here is a
               fragment the child can never receive. */
            uint8_t index = (uint8_t)(command & 0x1f);
            if (index != s.pb.expect && index + 1 != s.pb.expect)
                log_event(now_ms, EV_NOTE, TRADE_SHIM_NOTE_PARENT_FRAGMENT_GAP, s.pb.expect, index);
            if (index >= s.pb.expect) s.pb.expect = (uint8_t)(index + 1);
        }
        if (i == 0)
        {
            if (command == RFUCMD_SEND_BLOCK_INIT)
            {
                if (value != s.host_block_count) log_event(now_ms, EV_HOST_BLOCK, 0, value, 0);
                s.host_block_count = value;
                if (!s.pb.active || (uint8_t)value != s.pb.count || s.pb.expect == s.pb.count)
                { s.pb.active = value >= 1 && value <= 32; s.pb.count = (uint8_t)value; s.pb.expect = 0; }
            }

            else if (command == RFUCMD_SEND_BLOCK && s.host_block_count == 2 && value == LINKCMD_CONFIRM_FINISH_TRADE)
            {
                s.post_trade = true;
                s.injected = false;
                s.answers_since_confirm = 0;
                s.fake_round = -1;
                s.fake_acked = false;
                s.attempts = 0;
                ++s.trades;
                log_event(now_ms, EV_CONFIRM, 0, (uint16_t)s.trades, (uint16_t)(s.last_round + 1));
                printf("LDN_BRIDGE trade %d confirmed, watching the post-trade standby rounds\n", s.trades);
            }
            else if (command == 0xed00 || command == 0xee00)
            {
                log_event(now_ms, EV_NOTE, TRADE_SHIM_NOTE_PARENT_DISCONNECT_CMD, command, value);
                printf("LDN_BRIDGE Switch sent a disconnect command %04x\n", command);
            }
            else if (command == RFUCMD_SEND_BLOCK_REQ)
            {
                s.request_type = value;
                s.request_served = false;
                s.request_at = now_ms;
                s.request_attempts = 0;
                s.request_frags = 0;
                log_event(now_ms, EV_REQUEST, s.post_trade ? 1 : 0, value, 0);
                if (s.post_trade)
                {
                    s.post_trade = false;
                    s.fake_acked = true;              /* the parent has moved on, whatever it saw */
                    printf("LDN_BRIDGE Switch is requesting party data again\n");
                }
            }
        }
        if (command != RFUCMD_READY_EXIT_STANDBY) continue;
        if (is_fake(value))
        {
            /* The parent's half of an injected round. The child is not in that round,
               so this must not reach it: a stray matching command would pre-arm its
               next barrier. */
            if (i == 0 && value == s.fake_round && !s.fake_acked)
            {
                s.fake_acked = true;
                log_event(now_ms, EV_ACK, 0, value, 0);
                printf("LDN_BRIDGE Switch answered the supplied round %d\n", value);
            }
            log_event(now_ms, EV_PARENT_STANDBY, (uint8_t)i, value, 0xffff);
            memset(slot, 0, 4);
            continue;
        }
        if (i == 0 && (int)value > s.last_round)
        {
            s.last_round = value;
            if (s.post_trade) ++s.answers_since_confirm;
        }
        uint16_t child_value = (uint16_t)to_child(value);
        wr16(slot + 2, child_value);
        log_event(now_ms, EV_PARENT_STANDBY, (uint8_t)i, value, child_value);
    }
    return pre_len;
}

size_t trade_shim_inject(int64_t now_ms, uint8_t *out, size_t capacity)
{
    if (capacity < 16 || !s.post_trade || !s.have_tag) return 0;
    int64_t quiet = now_ms - s.quiet_since;
    if (!s.injected)
    {
        bool due = (s.answers_since_confirm >= TRADE_SHIM_CHILD_ROUNDS && quiet >= TRADE_SHIM_SETTLE_MS)
                || (s.answers_since_confirm >= 1 && quiet >= TRADE_SHIM_FALLBACK_MS);
        if (!due) return 0;
        int next = s.last_round > s.last_child_round ? s.last_round : s.last_child_round;
        s.fake_round = next + 1;
        s.fake_tag = (uint8_t)((s.last_tag + 1) & 7);
        add_fake(s.fake_round);
        s.last_tag = s.fake_tag;
        ++s.child_commands;
        s.injected = true;
        s.fake_acked = false;
        s.attempts = 1;
        s.injected_at = now_ms;
        s.quiet_since = now_ms;
        ++s.injections;
        log_event(now_ms, EV_INJECT, s.fake_tag, (uint16_t)s.fake_round, (uint16_t)s.answers_since_confirm);
        printf("LDN_BRIDGE supplied standby round %d on the GBA's behalf after %d answered rounds\n",
               s.fake_round, s.answers_since_confirm);
        return build_child_frame(out, s.fake_tag, (uint16_t)s.fake_round);
    }
    if (s.fake_acked || now_ms - s.injected_at < TRADE_SHIM_ACK_MS) return 0;
    if (s.attempts >= TRADE_SHIM_MAX_ATTEMPTS)
    {
        if (s.give_ups == s.injections - 1) { ++s.give_ups; log_event(now_ms, EV_GIVE_UP, 0, (uint16_t)s.fake_round, 0); }
        return 0;
    }
    /* Same round and the same tag: if the first copy never reached the parent's game its
       tag is still the one expected; if it did and the answer is merely late, this copy
       is a harmless duplicate. */
    ++s.attempts;
    ++s.child_commands;
    s.injected_at = now_ms;
    log_event(now_ms, EV_INJECT, s.fake_tag, (uint16_t)s.fake_round, (uint16_t)(0x100 | s.attempts));
    printf("LDN_BRIDGE no answer to the supplied round %d, sending it again (attempt %d)\n", s.fake_round, s.attempts);
    return build_child_frame(out, s.fake_tag, (uint16_t)s.fake_round);
}

size_t trade_shim_host_inject(int64_t now_ms, uint8_t *out, size_t capacity)
{
    if (capacity < 73 || s.request_type < 0 || s.request_served) return 0;
    if (s.request_attempts >= TRADE_SHIM_MAX_REREQUESTS || now_ms - s.request_at < TRADE_SHIM_REREQUEST_MS
        || now_ms - s.last_command_ms < TRADE_SHIM_REREQUEST_MS) return 0;
    /* The child's link layer refused the parent's block request -- it was still inside
       its previous block send (a lost echo keeps it there) or another command was
       pending -- and the parent asks only once. The child accepts a repeat once it is
       free; while it is still busy the repeat is refused the same way. */
    ++s.request_attempts;
    ++s.rerequests;
    s.request_at = now_ms;
    log_event(now_ms, EV_REREQUEST, (uint8_t)s.request_attempts, (uint16_t)s.request_type, 0);
    printf("LDN_BRIDGE GBA did not answer the Switch's block request %d, repeating it (attempt %d)\n",
           s.request_type, s.request_attempts);
    return build_host_frame(out, RFUCMD_SEND_BLOCK_REQ, (uint16_t)s.request_type, 0, 0);
}

void trade_shim_status(char *out, size_t capacity)
{
    snprintf(out, capacity, "shim_trades=%d shim_injected=%d shim_reanswers=%d shim_rerequests=%d shim_offset=%d shim_post_trade=%d shim_last_round=%d shim_answers=%d shim_acked=%d shim_echoes=%d shim_restamps=%d shim_tag_gaps=%d shim_commands=%u shim_echoed=%u shim_dups=%d",
             s.trades, s.injections, s.reanswers, s.rerequests, s.base + s.fake_count, s.post_trade ? 1 : 0, s.last_round,
             s.answers_since_confirm, s.fake_acked ? 1 : 0, s.echoes_synthesized, s.resends_restamped, s.tag_gaps,
             s.child_commands, s.echoes, s.duplicates);
}

static const char *const kEventNames[] = {
    "?", "BOOT", "RESET", "CONFIRM", "PARENT_STANDBY", "CHILD_STANDBY", "REQUEST", "INJECT",
    "ACK", "REANSWER", "REREQUEST", "NOTE", "HOST_BLOCK", "GIVE_UP", "ADAPTER", "BRIDGE", "TRACE" };
static const char *const kNoteNames[] = { "?", "bridge start", "child connect", "child disconnect", "host disconnect", "bridge stop", "host silence ms", "parent disconnect cmd", "child tag gap prev/now", "switch clock skip frames/ms", "echo synthesized idx/count", "parent fragment gap expect/got", "echo gap mask/count", "restamped resend idx/tag", "adapter trace seq count<<8|restarts" };
static const char *const kTraceKinds[] = { "?", "cmd", "event", "restart_in_state", "delivery", "sd_reset_in_state", "login" };

void trade_shim_dump(void (*emit)(const char *line))
{
    char line[128];
    if (rtc_log.magic != LOG_MAGIC) { emit("LDN_SHIM_LOG empty"); return; }
    snprintf(line, sizeof(line), "LDN_SHIM_LOG entries=%u boots=%u", rtc_log.count, rtc_log.boots);
    emit(line);
    for (int i = 0; i < rtc_log.count; ++i)
    {
        const log_entry_t *e = &rtc_log.entries[(rtc_log.head + i) % LOG_SLOTS];
        const char *name = e->type < sizeof(kEventNames) / sizeof(kEventNames[0]) ? kEventNames[e->type] : "?";
        double t = e->ms / 1000.0;
        switch (e->type)
        {
        case EV_BOOT: snprintf(line, sizeof(line), "  %9.3f %s reset_reason=%u boot=%u", t, name, e->a, e->b); break;
        case EV_CONFIRM: snprintf(line, sizeof(line), "  %9.3f %s trade=%u parent_count=%u", t, name, e->b, e->c); break;
        case EV_PARENT_STANDBY:
            if (e->c == 0xffff) snprintf(line, sizeof(line), "  %9.3f %s slot=%u round=%u suppressed", t, name, e->a, e->b);
            else snprintf(line, sizeof(line), "  %9.3f %s slot=%u round=%u to_gba=%u", t, name, e->a, e->b, e->c);
            break;
        case EV_CHILD_STANDBY: snprintf(line, sizeof(line), "  %9.3f %s round=%u to_switch=%u tag=%u->%u", t, name, e->b, e->c, e->a >> 4, e->a & 15); break;
        case EV_REQUEST: snprintf(line, sizeof(line), "  %9.3f %s type=%u%s", t, name, e->b, e->a ? " (post-trade watch ends)" : ""); break;
        case EV_INJECT: snprintf(line, sizeof(line), "  %9.3f %s round=%u tag=%u %s%u", t, name, e->b, e->a, (e->c & 0x100) ? "attempt=" : "after_answers=", e->c & 0xff); break;
        case EV_ACK: case EV_GIVE_UP: snprintf(line, sizeof(line), "  %9.3f %s round=%u", t, name, e->b); break;
        case EV_REANSWER: snprintf(line, sizeof(line), "  %9.3f %s gba_round=%u switch_round=%u", t, name, e->b, e->c); break;
        case EV_REREQUEST:
            if (e->a) snprintf(line, sizeof(line), "  %9.3f %s type=%u attempt=%u", t, name, e->b, e->a);
            else snprintf(line, sizeof(line), "  %9.3f %s type=%u (with a re-answer)", t, name, e->b);
            break;
        case EV_NOTE: snprintf(line, sizeof(line), "  %9.3f %s %s %u %u", t, name,
                               e->a < sizeof(kNoteNames) / sizeof(kNoteNames[0]) ? kNoteNames[e->a] : "?", e->b, e->c); break;
        case EV_HOST_BLOCK: snprintf(line, sizeof(line), "  %9.3f %s fragments=%u", t, name, e->b); break;
        case EV_ADAPTER:
        {
            const char *why = (e->a & 1) ? "changed" : "periodic";
            switch (e->a & 0xf0)
            {
            case 0x10: snprintf(line, sizeof(line), "  %9.3f %s %s park_drops=%u uart_overflows=%u", t, name, why, e->b, e->c); break;
            case 0x20: snprintf(line, sizeof(line), "  %9.3f %s %s fifo_drops=%u fifo_high=%u park_high=%u", t, name, why, e->b, e->c & 0xff, e->c >> 8); break;
            case 0x30: snprintf(line, sizeof(line), "  %9.3f %s %s delivery_aborts=%u delivery_retries=%u", t, name, why, e->b, e->c); break;
            case 0x40: snprintf(line, sizeof(line), "  %9.3f %s %s sd_resets=%u deliv_fail=%02x err_responses=%u login_restarts=%u", t, name, why, e->b & 0xff, e->b >> 8, e->c & 0xff, e->c >> 8); break;
            case 0x50: snprintf(line, sizeof(line), "  %9.3f %s %s park_idle_drops=%u comstate=%u", t, name, why, e->b, e->c); break;
            case 0x60: snprintf(line, sizeof(line), "  %9.3f %s %s events timeo=%u disc=%u data=%u rtx=%u", t, name, why, e->b & 0xff, e->b >> 8, e->c & 0xff, e->c >> 8); break;
            case 0x70: snprintf(line, sizeof(line), "  %9.3f %s %s link_pwr_zero=%u wipes evict/netdisc=%02x hoststart/cmddisc=%02x reset/occupancy=%02x", t, name, why, e->b & 0xff, e->b >> 8, e->c & 0xff, e->c >> 8); break;
            case 0x80: snprintf(line, sizeof(line), "  %9.3f %s %s gba_cmd_ring[0..3]=%02x %02x %02x %02x", t, name, why, e->b & 0xff, e->b >> 8, e->c & 0xff, e->c >> 8); break;
            case 0x90: snprintf(line, sizeof(line), "  %9.3f %s %s gba_cmd_ring[4..7]=%02x %02x %02x %02x", t, name, why, e->b & 0xff, e->b >> 8, e->c & 0xff, e->c >> 8); break;
            case 0xa0: snprintf(line, sizeof(line), "  %9.3f %s %s last_cmd=%02x plen=%u commands=%u", t, name, why, e->b & 0xff, e->b >> 8, e->c); break;
            case 0xc0: snprintf(line, sizeof(line), "  %9.3f %s %s gba_restarts=%u trace_snapshots=%u", t, name, why, e->b, e->c); break;
            default:   snprintf(line, sizeof(line), "  %9.3f %s %s out_ring_drops=%u bridge_queue_high=%u", t, name, why, e->b, e->c); break;
            }
            break;
        }
        case EV_TRACE:
            snprintf(line, sizeof(line), "  %9.3f %s +%ums %s %02x", t, name, e->c,
                     e->a < sizeof(kTraceKinds) / sizeof(kTraceKinds[0]) ? kTraceKinds[e->a] : "?", e->b);
            break;
        case EV_BRIDGE:
        {
            const char *why = (e->a & 1) ? "changed" : "periodic";
            switch (e->a & 0x0e)
            {
            case 2: snprintf(line, sizeof(line), "  %9.3f %s %s bad_frames=%u decrypt_failures=%u unzip_failures=%u", t, name, why, e->b, e->c & 0xff, e->c >> 8); break;
            case 4: snprintf(line, sizeof(line), "  %9.3f %s %s host_clock_skips single=%u multi=%u", t, name, why, e->b, e->c); break;
            case 6: snprintf(line, sizeof(line), "  %9.3f %s %s child_commands=%u echoed=%u", t, name, why, e->b, e->c); break;
            default: snprintf(line, sizeof(line), "  %9.3f %s %s hold_dropped=%u reordered=%u outbound_overflow=%u", t, name, why, e->b, e->c & 0xff, e->c >> 8); break;
            }
            break;
        }
        default: snprintf(line, sizeof(line), "  %9.3f %s %u %u %u", t, name, e->a, e->b, e->c); break;
        }
        emit(line);
    }
}
