#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "wa_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GBA Wireless Adapter (AGB-015) emulation on the ESP32's own pins, in place of a
   GB-Link running the wireless-adapter mode. The GBA side is the protocol core the
   GB-Link uses (rfuProtocol.hpp); the network side is the same RFU1 frame stream. */

typedef struct {
    /* A 64-byte chunk of RFU1 frames (zero padded) or a telemetry report (shorter),
       for the data channel. Called from the adapter task. */
    void (*data)(const uint8_t *bytes, size_t length, void *ctx);
    /* A link status code (0xFF02 AwaitMode, ...), for the status channel. */
    void (*status)(uint16_t code, void *ctx);
    void *ctx;
} wa_transport_t;

typedef struct {
    wa_pins_t pins;
    uint8_t role;            /* SetMode variant: 0 symmetric, 1 host, 2 client */
    uint32_t spin_us;        /* how long a word's interrupt waits for the GBA's handshake */
} wa_config_t;

/* Starts the adapter task: pins, self-test, then the GBA link and AwaitMode on the
   status channel. Returns at once; later calls do nothing. */
bool wa_adapter_start(const wa_config_t *config, const wa_transport_t *transport);
/* The task is up and the GBA link armed. */
bool wa_adapter_running(void);

/* Stops a started adapter and gives its memory back: the GBA's lines are let go, the
   task ends, and the core and its queues are freed. wa_adapter_start starts it again,
   with the self-test. Waits for the task; not from the adapter task. */
void wa_adapter_stop(void);

/* SetMode(wireless) for a started adapter: a fresh adapter with this role, then
   AwaitMode, as the GB-Link's mode entry. A GBA using it has to log in again. */
void wa_adapter_enter_mode(uint8_t role);

/* RFU1 bytes from the network side, in any chunking. One caller at a time. */
void wa_adapter_feed(const uint8_t *bytes, size_t length);

/* Runs the self-test again on the adapter task, then resumes the link. */
void wa_adapter_recalibrate(void);

/* How long ago the GBA last looked for a group to join (started or polled a search),
   in ms; UINT32_MAX before its first search. */
uint32_t wa_adapter_search_age_ms(void);

typedef struct {
    /* self-test */
    wa_cal_t cal;
    /* GBA-clocked words */
    uint32_t words, bad_bits, realigns, released_spin, released_irq, held;
    uint32_t ones_mismatches; /* words whose count of ones disagreed with the slave
                                 beyond the last bit */
    uint32_t first_words[8];
    uint8_t first_count;
    /* adapter-clocked delivery */
    uint32_t master_words, deliveries, delivery_fails, delivery_retries, delivery_aborts;
    uint8_t last_fail;
    uint32_t sd_resets;
    /* protocol core */
    uint8_t comstate, state, role, last_cmd;
    bool any_rx, nintendo;
    uint32_t commands, err_responses, login_restarts, command_restarts;
    /* network side */
    uint32_t frames_in, frames_out, parked_high, parked_drops, out_drops;
    uint8_t lines;           /* bit 0 SC, 1 SO, 2 SI, 3 SD */
} wa_stats_t;

void wa_adapter_stats(wa_stats_t *out);

#ifdef __cplusplus
}
#endif
