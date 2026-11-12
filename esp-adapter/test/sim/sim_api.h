#pragma once
#include <stddef.h>
#include <stdint.h>

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Timing of the simulated hardware. */
typedef struct {
    double gbaRegNs;      /* GBA: one register access, with the code around it */
    double gbaIrqEntryNs; /* GBA: interrupt to handler */
    double adaCallNs;     /* adapter: one port call (a register access on the chip) */
    double wordIsrNs;     /* adapter: 32nd edge to the word interrupt handler */
    double gpioIsrNs;     /* adapter: pin edge to its interrupt handler */
    uint32_t spinUs;      /* adapter: CONFIG_WA_HANDSHAKE_SPIN_US */
    int preclock;         /* the slave shows its first bit only at a clock edge */
    int verbose;
    int corruptWord;      /* GBA-clocked word whose 6th received bit is flipped (0: none) */
    int glitchAfterWord;  /* a spike on SC after this GBA-clocked word, seen by the shifter only */
    int noBitCount;       /* the slave's received-bit count is unusable (self-test result 0) */
} SimParams;

/* Runs gba() as the GBA's program until it returns. 0 if it overran limit_us. */
int sim_run(const SimParams *params, void (*gba)(void), uint64_t limit_us);

IntrFunc *sim_vector_slot(int bit);
uint64_t sim_now_us(void);
void sim_log(const char *fmt, ...);
uint32_t sim_contention(void);
int sim_adapter_clocking(void);   /* the adapter is driving SC */
void sim_sd_force(int level);     /* SD at this level whatever the GBA does; -1 gives it back */
uint32_t sim_sd_interrupts(void); /* SD fall interrupts the adapter has taken */

/* The network side of the adapter: RFU1 frames in, and what came out. */
void sim_net_feed(const uint8_t *frame, size_t length);
int sim_net_count(uint32_t type);
int sim_net_last(uint32_t type, uint8_t *out, size_t capacity);
int sim_net_status_count(uint16_t code);
void sim_net_enter_mode(uint8_t role);   /* the bridge's SetMode(wireless) */

#ifdef __cplusplus
}
#endif
