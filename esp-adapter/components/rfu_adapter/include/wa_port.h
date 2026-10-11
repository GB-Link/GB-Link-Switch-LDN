#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Everything platform-specific the adapter needs: the GBA link port and a few OS
   services. wa_port_esp.c implements it on the ESP32 chips; the host test implements
   it against a simulated GBA running librfu.

   The lines are named from the GBA side, as on its link port:
     SC  clock. The GBA drives it while it is master (the ID exchange, its commands);
         the adapter drives it while delivering an event.
     SO  the GBA's output, our input.
     SI  the GBA's input, our output.
     SD  the GBA's soft reset. It rests high; before librfu starts over it goes low,
         high for about a millisecond and low again.

   A word the GBA clocks is shifted by the SPI slave as 33 bits: a lead bit, then the
   32-bit reply, MSB first. SI shows the lead bit from the moment the word is armed
   until the GBA's first falling edge, and between words librfu waits on that level:
   armed with lead 1 the GBA stays held, armed with lead 0 it is released to clock
   the word. */

typedef struct {
    int sc, so, si, sd;
} wa_pins_t;

typedef struct {
    void (*word)(void);      /* 32 rising SC edges since the last arm */
    void (*so_rise)(void);   /* SO rose while wa_port_so_irq was on */
    void (*sd_fall)(void);   /* SD fell */
} wa_isr_t;

/* What the start-up self-test chose. */
typedef struct {
    bool ok;
    uint8_t mode;            /* SPI slave mode (0-3) */
    bool flip13;             /* first-bit timing bit inverted from the mode's own (S3, C6) */
    bool sc_inverted;        /* SC reaches the SPI clock input inverted */
    bool preclock;           /* one internal clock before each word puts the lead bit out */
    uint8_t rx_offset;       /* received bit holding the GBA's bit 31 */
    uint16_t word_bits;      /* the slave's received-bit count after a clean word; 0 when
                                that count cannot tell a short word from a full one */
    char detail[160];
} wa_cal_t;

/* Sets up the pins, the SPI slave, the edge counter and the interrupts, then runs
   the self-test. Must be called from the task that later calls the other functions,
   so the interrupts land on its core. */
bool wa_port_init(const wa_pins_t *pins, const wa_isr_t *isr, wa_cal_t *cal);
/* Runs the self-test again (no word armed, interrupts off). */
bool wa_port_calibrate(wa_cal_t *cal);

/* GBA-clocked words */
void wa_port_arm(uint32_t reply, bool lead);     /* load, zero the edge count, assert CS */
uint32_t wa_port_collect(uint16_t *bits);        /* deassert CS; the word the GBA sent */
void wa_port_disarm(void);                       /* deassert CS, nothing collected */
uint32_t wa_port_edges(void);                    /* rising SC edges since the last arm */
uint32_t wa_port_ones(void);                     /* of those, the ones with SO high */
void wa_port_word_irq(bool on);                  /* edge count and word interrupt; arm zeroes the count */
void wa_port_so_irq(bool on);                    /* a pending edge is dropped first */
void wa_port_sd_irq(bool on);                    /* SD's fall interrupt, off after wa_port_init;
                                                    a pending fall is dropped first */

/* SI source */
void wa_port_si_drive(bool level);               /* SI from GPIO */
void wa_port_si_spi(void);                       /* SI from the SPI slave */

/* Adapter-clocked words */
void wa_port_sc_drive(bool on);                  /* SC ours (held high) or back to the GBA */
/* 32 bits out on SI, SO read back. Stops early (false) once *stop is set: the GBA has
   reset the adapter and its SC is no longer ours to clock. */
bool wa_port_master_word(uint32_t word, uint32_t *rx, volatile const bool *stop);

bool wa_port_sc(void);
bool wa_port_so(void);
bool wa_port_si(void);
bool wa_port_sd(void);

/* OS */
bool wa_port_spawn(void (*entry)(void *), void *arg);  /* the adapter task */
uint32_t wa_port_ms(void);
uint32_t wa_port_us(void);
void wa_port_delay_us(uint32_t us);
void wa_port_lock(void);
void wa_port_unlock(void);
void wa_port_lock_isr(void);
void wa_port_unlock_isr(void);
void wa_port_wake_isr(void);                     /* wake the adapter task from an interrupt */
void wa_port_wake(void);                         /* the same from a task */
void wa_port_wait(uint32_t ms);                  /* adapter task: sleep until woken or ms */
uint16_t wa_port_random16(void);

#ifdef __cplusplus
}
#endif
