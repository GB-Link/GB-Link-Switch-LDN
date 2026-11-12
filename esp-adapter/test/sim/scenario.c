/* The GBA's side of the test: librfu's SIO layer, driven the way a game drives it,
   against the adapter on the simulated wire. Each step checks what librfu reports
   (command callbacks, its restart count, the reply words) rather than anything about
   the adapter, so it checks what the game would see. */
#include <stdio.h>
#include <string.h>

#include "librfu.h"
#include "sim_api.h"
#include "wa_adapter.h"

int g_failures;
int g_rounds = 40;
int g_allow_restarts;   /* a case that breaks a word on purpose */

static struct RfuIntrStruct s_intr;
static volatile int s_cmd = -1, s_err = -1, s_event = -1;
static unsigned s_restarts;

static void cb_m(int cmd, int err)
{
    s_cmd = cmd;
    s_err = err;
}

static void cb_s(u16 value) { s_event = value; }

#define CHECK(cond, ...)                                   \
    do {                                                   \
        if (!(cond)) {                                     \
            g_failures++;                                  \
            printf("  FAIL line %d: ", __LINE__);          \
            printf(__VA_ARGS__);                           \
            printf("\n");                                  \
        }                                                  \
    } while (0)

static u32 reply(int i) { return ((u32 *)gSTWIStatus->rxPacket)[i]; }

static void wait_us(uint64_t us)
{
    const uint64_t end = sim_now_us() + us;
    while (sim_now_us() < end) sim_gba_cycles(1000);
}

/* The REQ callback of the command just started, as a game's polling loop waits for
   it. A restarted command (librfu's recovery) still has to complete. */
static int finish(const char *name, int id)
{
    const uint64_t start = sim_now_us();
    while (s_cmd < 0 && sim_now_us() - start < 2000000) sim_gba_cycles(300);
    s_restarts += gSTWIStatus->recoveryCount;
    if (gSTWIStatus->recoveryCount) sim_log("%s restarted %u times", name, gSTWIStatus->recoveryCount);
    if (s_cmd != id || s_err != 0)
    {
        CHECK(0, "%s: callback for %02x error %d after %llu us", name, s_cmd, s_err,
              (unsigned long long)(sim_now_us() - start));
        return 0;
    }
    sim_log("%s done in %llu us", name, (unsigned long long)(sim_now_us() - start));
    return 1;
}

#define RUN(name, id, call) (s_cmd = -1, s_err = -1, call, finish(name, id))

static void put_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static void net_command(uint32_t type, uint32_t hdata)
{
    uint8_t f[16] = {'R', 'F', 'U', '1'};
    put_be32(f + 4, type);
    put_be32(f + 8, hdata);
    sim_net_feed(f, sizeof(f));
}

static const uint32_t kBeacon[6] = {0x00000002, 0x12340000, 0x00000000, 0x00000004, 0x41424344, 0x45464748};

static void net_broadcast(uint16_t devid)
{
    uint8_t f[36] = {'R', 'F', 'U', '1'};
    put_be32(f + 4, 0);
    put_be32(f + 8, devid);
    for (int i = 0; i < 6; i++) put_be32(f + 12 + 4 * i, kBeacon[i]);
    sim_net_feed(f, sizeof(f));
}

static void host_data(const uint8_t *bytes, uint8_t length)
{
    uint8_t f[104] = {'R', 'F', 'U', '1'};
    put_be32(f + 4, 5);
    put_be32(f + 8, length);
    memcpy(f + 12, bytes, length);
    sim_net_feed(f, sizeof(f));
}

static int checkid(void)
{
    AgbRFU_SoftReset();
    const s32 id = AgbRFU_checkID(8);
    CHECK(id == RFU_ID, "checkID returned %lx", (long)id);
    sim_log("checkID %lx", (long)id);
    return id == RFU_ID;
}

/* MS change: the GBA hands the clock to the adapter and waits for its event. */
static int wait_event(const char *name, int expected, uint32_t limit_us)
{
    s_event = -1;
    if (!RUN(name, ID_MS_CHANGE_REQ, STWI_send_MS_ChangeREQ())) return -1;
    const uint64_t start = sim_now_us();
    while (s_event < 0 && sim_now_us() - start < limit_us) sim_gba_cycles(300);
    const uint64_t took = sim_now_us() - start;
    CHECK(s_event == expected, "%s: event %x, expected %x, after %llu us", name, s_event, expected,
          (unsigned long long)took);
    CHECK(gSTWIStatus->msMode == AGB_CLK_MASTER, "%s: the GBA stayed the clock slave", name);
    return (int)took;
}

/* Up to a joined room: detection, configuration, a scan and a connect. */
static int bring_up(void)
{
    STWI_init_all(&s_intr, sim_vector_slot(7), FALSE);
    STWI_init_timer(sim_vector_slot(6), 3);
    STWI_set_Callback_M(cb_m);
    STWI_set_Callback_S(cb_s);
    REG_IME = 1;

    if (!checkid()) return 0;

    RUN("reset", ID_RESET_REQ, STWI_send_ResetREQ());
    if (RUN("version", ID_VERSION_STATUS_REQ, STWI_send_VersionStatusREQ()))
        CHECK(reply(1) == 0x00830117, "version %08lx", (unsigned long)reply(1));
    RUN("link status", ID_LINK_STATUS_REQ, STWI_send_LinkStatusREQ());
    RUN("system status", ID_SYSTEM_STATUS_REQ, STWI_send_SystemStatusREQ());
    RUN("system config", ID_SYSTEM_CONFIG_REQ, STWI_send_SystemConfigREQ(0x003C, 0x08, 0x20));
    static const u8 serial[16] = {0x02, 0x00, 'F', 'I', 'R', 'E', 'R', 'E', 'D', 0, 0, 0, 0, 0, 0, 0};
    static const u8 uname[8] = {'R', 'E', 'D', 0, 0, 0, 0, 0};
    RUN("game config", ID_GAME_CONFIG_REQ, STWI_send_GameConfigREQ(serial, uname));

    /* Scan: a room appears on the network side and the broadcast read returns it. The
       bridge is told the GBA is looking for a group. */
    static int first_scan = 1;
    if (first_scan) CHECK(wa_adapter_search_age_ms() == UINT32_MAX, "a search reported before the first");
    first_scan = 0;
    RUN("scan start", ID_SP_START_REQ, STWI_send_SP_StartREQ());
    net_broadcast(0x1234);
    wait_us(5000);
    if (RUN("scan poll", ID_SP_POLL_REQ, STWI_send_SP_PollingREQ()))
    {
        CHECK(gSTWIStatus->ackLength >= 7, "scan poll returned %u words", gSTWIStatus->ackLength);
        CHECK((reply(1) & 0xFFFF) == 0x1234, "scan poll devid %08lx", (unsigned long)reply(1));
        CHECK(reply(2) == kBeacon[0] && reply(7) == kBeacon[5], "scan poll data %08lx..%08lx",
              (unsigned long)reply(2), (unsigned long)reply(7));
    }
    const uint64_t polled_us = sim_now_us();
    CHECK(wa_adapter_search_age_ms() < 50, "the scan poll was reported %lu ms ago",
          (unsigned long)wa_adapter_search_age_ms());
    wait_us(20000);   /* the player picks the room from the list */
    RUN("scan end", ID_SP_END_REQ, STWI_send_SP_EndREQ());

    /* Join it. */
    RUN("connect", ID_CP_START_REQ, STWI_send_CP_StartREQ(0x1234));
    wait_us(2000);
    CHECK(sim_net_count(1) >= 1, "no CONNECT_REQ went out");
    net_command(2, 0x0042 | (1 << 16));
    int connected = 0;
    for (int i = 0; i < 20 && !connected; i++)
    {
        wait_us(3000);
        if (RUN("connect poll", ID_CP_POLL_REQ, STWI_send_CP_PollingREQ())) connected = reply(1) == 0x10042;
    }
    CHECK(connected, "never connected");
    if (RUN("connect end", ID_CP_END_REQ, STWI_send_CP_EndREQ()))
        CHECK(reply(1) == 0x10042, "connect end %08lx", (unsigned long)reply(1));
    /* Ending the scan and connecting are not searches. */
    const uint32_t since_poll = (uint32_t)((sim_now_us() - polled_us) / 1000);
    CHECK(wa_adapter_search_age_ms() + 1 >= since_poll, "a search reported %lu ms ago, the last poll was %lu ms ago",
          (unsigned long)wa_adapter_search_age_ms(), (unsigned long)since_poll);
    return connected;
}

/* Data from the host: each wait ends in a data event, the read returns it, and the
   game sends its own frame back. */
static void data_rounds(int first, int count)
{
    const int sent_before = sim_net_count(6);
    for (int round = first; round < first + count; round++)
    {
        uint8_t payload[14];
        for (int i = 0; i < 14; i++) payload[i] = (uint8_t)(round * 17 + i * 3 + 1);
        payload[0] = 0x55;
        host_data(payload, sizeof(payload));
        wait_us(1000);
        wait_event("data wait", ID_DATA_READY_AND_CHANGE_REQ, 200000);
        if (RUN("data read", ID_DATA_RX_REQ, STWI_send_DataRxREQ()))
        {
            CHECK(reply(1) == 14, "data read length %lu", (unsigned long)reply(1));
            uint8_t got[16];
            memcpy(got, &((u32 *)gSTWIStatus->rxPacket)[2], 16);
            CHECK(memcmp(got, payload, 14) == 0, "data read bytes differ in round %d", round);
        }
        /* The byte count sits in the header word at slot 1's position, then the data. */
        const u32 mine[5] = {14u << (8 + 1 * 5), 0x11223344u + round, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
        RUN("data send", ID_DATA_TX_REQ, STWI_send_DataTxREQ(mine, sizeof(mine)));
    }
    wait_us(3000);   /* the adapter task forwards the last frame */
    CHECK(sim_net_count(6) - sent_before >= count, "%d CLIENT_SEND frames for %d rounds",
          sim_net_count(6) - sent_before, count);
}

static void finish_run(void)
{
    CHECK(sim_contention() == 0, "SC was driven from both ends %u times", sim_contention());
    CHECK(g_allow_restarts || s_restarts == 0, "librfu restarted commands %u times", s_restarts);
    printf("  librfu restarts %u\n", s_restarts);
}

void gba_session(void)
{
    if (!bring_up()) return;
    /* A wait with nothing to receive ends in the adapter's timeout event (32 frames). */
    const int took = wait_event("empty wait", ID_MS_CHANGE_REQ, 2000000);
    CHECK(took > 450000 && took < 700000, "the empty wait took %d us", took);
    data_rounds(0, g_rounds);
    /* The game resets its adapter and starts over. */
    if (checkid()) RUN("reset after soft reset", ID_RESET_REQ, STWI_send_ResetREQ());
    finish_run();
}

/* The game resets the adapter while the adapter is clocking an event into it. */
void gba_reset_during_delivery(void)
{
    if (!bring_up()) return;
    data_rounds(0, 3);
    uint8_t payload[14] = {0x55, 1, 2, 3};
    host_data(payload, sizeof(payload));
    wait_us(1000);
    s_event = -1;
    RUN("wait", ID_MS_CHANGE_REQ, STWI_send_MS_ChangeREQ());
    const uint64_t start = sim_now_us();
    while (!sim_adapter_clocking() && sim_now_us() - start < 200000) sim_gba_cycles(30);
    CHECK(sim_adapter_clocking(), "the adapter never started the delivery");
    if (checkid())
    {
        RUN("reset after the interrupted event", ID_RESET_REQ, STWI_send_ResetREQ());
        RUN("system status", ID_SYSTEM_STATUS_REQ, STWI_send_SystemStatusREQ());
    }
    finish_run();
}

/* The bridge re-enters the mode after a session: a fresh adapter, as the GB-Link's
   mode entry gives. The game's next command fails and it logs in again. */
void gba_mode_entry(void)
{
    if (!bring_up()) return;
    data_rounds(0, 3);
    const int announced = sim_net_status_count(0xFF02);
    CHECK(announced == 1, "AwaitMode %d times at power-up", announced);
    sim_net_enter_mode(0);
    wait_us(3000);
    CHECK(sim_net_status_count(0xFF02) == announced + 1, "no AwaitMode for the mode entry");
    s_cmd = -1;
    s_err = -1;
    STWI_send_SystemStatusREQ();
    const uint64_t start = sim_now_us();
    while (s_cmd < 0 && sim_now_us() - start < 2000000) sim_gba_cycles(300);
    CHECK(s_cmd == ID_SYSTEM_STATUS_REQ && s_err != 0, "the command after the mode entry: callback %02x error %d",
          s_cmd, s_err);
    if (!bring_up()) return;
    data_rounds(10, 3);
    finish_run();
}

/* SD chatters at about a megahertz before the game starts, as it can while nothing drives
   it. The adapter has to stop listening rather than take an interrupt per fall, and
   still see the game's reset once the line is quiet. */
void gba_sd_chatter(void)
{
    const uint32_t before = sim_sd_interrupts();
    const uint64_t end = sim_now_us() + 20000;
    while (sim_now_us() < end)
    {
        sim_sd_force(0);
        sim_gba_cycles(8);
        sim_sd_force(1);
        sim_gba_cycles(8);
    }
    sim_sd_force(-1);
    const uint32_t taken = sim_sd_interrupts() - before;
    CHECK(taken < 100, "%u SD interrupts for 20 ms of chatter", taken);
    wait_us(60000);
    if (!bring_up()) return;
    data_rounds(0, 3);
    finish_run();
}

static void client_data(uint16_t devid, unsigned slot, const uint8_t *bytes, uint8_t length)
{
    uint8_t f[104] = {'R', 'F', 'U', '1'};
    put_be32(f + 4, 6);
    put_be32(f + 8, (uint32_t)devid | (uint32_t)slot << 16 | (uint32_t)length << 24);
    memcpy(f + 12, bytes, length);
    sim_net_feed(f, sizeof(f));
}

/* The GBA leads a group. Each wait for its client ends as soon as the client's frame for
   the round is in, an all-zero one included, which is not delivered; the read then takes
   the client's next queued frame. */
void gba_host_rounds(void)
{
    STWI_init_all(&s_intr, sim_vector_slot(7), FALSE);
    STWI_init_timer(sim_vector_slot(6), 3);
    STWI_set_Callback_M(cb_m);
    STWI_set_Callback_S(cb_s);
    REG_IME = 1;
    if (!checkid()) return;
    RUN("reset", ID_RESET_REQ, STWI_send_ResetREQ());
    RUN("system config", ID_SYSTEM_CONFIG_REQ, STWI_send_SystemConfigREQ(0x003C, 0x08, 0x20));
    static const u8 serial[16] = {0x02, 0x00, 'F', 'I', 'R', 'E', 'R', 'E', 'D', 0, 0, 0, 0, 0, 0, 0};
    static const u8 uname[8] = {'R', 'E', 'D', 0, 0, 0, 0, 0};
    RUN("game config", ID_GAME_CONFIG_REQ, STWI_send_GameConfigREQ(serial, uname));
    RUN("host start", ID_SC_START_REQ, STWI_send_SC_StartREQ());

    /* A client's connect, routed by the relay; the adapter answers with its slot. */
    net_command(1, 0);
    wait_us(2000);
    uint8_t ack[16] = {0};
    CHECK(sim_net_last(2, ack, sizeof(ack)) == 16, "no CONNECT_ACK for the client");
    const uint32_t h = (uint32_t)ack[8] << 24 | (uint32_t)ack[9] << 16 | (uint32_t)ack[10] << 8 | ack[11];
    const uint16_t devid = (uint16_t)(h & 0xFFFF);
    const unsigned slot = (h >> 16) & 3;
    if (RUN("host poll", ID_SC_POLL_REQ, STWI_send_SC_PollingREQ()))
        CHECK((reply(1) & 0xFFFF) == devid, "host poll lists %08lx, the client is %04x", (unsigned long)reply(1), devid);

    for (int round = 0; round < 8; round++)
    {
        const u32 mine[5] = {14u, 0x01020304u + (u32)round, 0x05060708, 0x090A0B0C, 0x0D0E0000};
        RUN("host send", ID_DATA_TX_REQ, STWI_send_DataTxREQ(mine, sizeof(mine)));
        const int idle = round % 3 != 2;
        uint8_t child[14] = {0};
        if (!idle)
        {
            child[0] = 0x55;
            for (int i = 1; i < 14; i++) child[i] = (uint8_t)(round * 7 + i);
        }
        client_data(devid, slot, child, sizeof(child));
        wait_us(500);
        const int took = wait_event("host wait", ID_DATA_READY_AND_CHANGE_REQ, 200000);
        CHECK(took >= 0 && took < 6000, "round %d (%s): the wait took %d us", round, idle ? "idle" : "data", took);
        if (RUN("host read", ID_DATA_RX_REQ, STWI_send_DataRxREQ()))
        {
            const unsigned got = (reply(1) >> (8 + slot * 5)) & 0x1F;
            CHECK(got == (idle ? 0u : 14u), "round %d (%s): read %u bytes", round, idle ? "idle" : "data", got);
            if (!idle && got == 14)
            {
                uint8_t back[16];
                memcpy(back, &((u32 *)gSTWIStatus->rxPacket)[2], 16);
                CHECK(memcmp(back, child, 14) == 0, "round %d: the client's bytes differ", round);
            }
        }
    }
    /* No frame from the client: the wait ends with the retransmission window. */
    const u32 last[5] = {14u, 0xAABBCCDD, 0, 0, 0};
    RUN("host send", ID_DATA_TX_REQ, STWI_send_DataTxREQ(last, sizeof(last)));
    s_event = -1;
    if (RUN("host wait, client silent", ID_MS_CHANGE_REQ, STWI_send_MS_ChangeREQ()))
    {
        const uint64_t start = sim_now_us();
        while (s_event < 0 && sim_now_us() - start < 200000) sim_gba_cycles(300);
        const int took = (int)(sim_now_us() - start);
        CHECK(s_event >= 0 && took >= 9000, "a wait with nothing from the client: event %x after %d us", s_event, took);
    }
    finish_run();
}

/* The game keeps interrupts off for 5 ms (a flash save) while words are in flight, in
   both directions. */
void gba_interrupts_held(void)
{
    if (!bring_up()) return;
    data_rounds(0, 2);
    for (int i = 0; i < 4; i++)
    {
        s_cmd = -1;
        s_err = -1;
        STWI_send_SystemStatusREQ();
        REG_IME = 0;
        wait_us(5000);
        REG_IME = 1;
        finish("status with interrupts held", ID_SYSTEM_STATUS_REQ);
    }
    uint8_t payload[14] = {0x55, 9, 8, 7};
    host_data(payload, sizeof(payload));
    wait_us(1000);
    s_event = -1;
    if (RUN("wait", ID_MS_CHANGE_REQ, STWI_send_MS_ChangeREQ()))
    {
        const uint64_t start = sim_now_us();
        while (!sim_adapter_clocking() && sim_now_us() - start < 200000) sim_gba_cycles(30);
        REG_IME = 0;
        wait_us(5000);
        REG_IME = 1;
        while (s_event < 0 && sim_now_us() - start < 400000) sim_gba_cycles(300);
        CHECK(s_event == ID_DATA_READY_AND_CHANGE_REQ, "event %x after interrupts were held", s_event);
        RUN("data read", ID_DATA_RX_REQ, STWI_send_DataRxREQ());
    }
    data_rounds(100, 3);
    finish_run();
}
