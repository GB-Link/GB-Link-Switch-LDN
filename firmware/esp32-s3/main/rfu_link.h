#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

// 32-bit SIO NORMAL-mode link layer for the GBA wireless adapter (RFU) role,
// bit-banged on a core pinned to this work. Ported from the GB-Link Pico
// firmware's PIO implementation (src/layers/linkLayer_rfu.c, fw 2.4.20); the
// wire semantics below are its hardware-proven findings, not guesses:
//
//   * MSB first, 32 bits per word.
//   * SC idles HIGH. The bus master sets up on the falling edge and samples on
//     the rising edge; both roles follow that same geometry, mirrored.
//   * While SC rests high between words in the GBA-master role, SI must
//     continuously mirror NOT SO. librfu's command-phase handshake_wait(1)/
//     handshake_wait(0) busy-waits poll SI between every word and never
//     release without it.
//   * A role swap must not glitch SC: an armed GBA-as-slave counts ANY SC edge
//     as a data bit, so the output latch is seeded HIGH before the pin is ever
//     driven.
//
// Wiring (GBC-style cable; the GBA's SI and SO cross over):
//   SC  -> CONFIG pin rfu, clock, input while the GBA drives it
//   SO  -> the GBA's SO, our data input
//   SI  -> the GBA's SI, our data output
//   SD  -> carries the game's AgbRFU_SoftReset pulse (input, pulled down)
// Ground must be common. All four default pins are in the 0-31 bank so the hot
// loop can use single-register GPIO access.

// Pin choice per target. Every link pin must be in the 0-31 bank so the hot
// loop can use single-register access, and SC must be output-capable because we
// drive it in the adapter-master role.
#if CONFIG_IDF_TARGET_ESP32S3
// AtomS3 / AtomS3 Lite expose 1, 2, 5-8 and 38/39. These four avoid the S3's
// strapping pins (0/3/45/46), its SPI flash pins (26-32) and its native
// USB-JTAG pins (19/20), and the S3 has no input-only range, so all four can
// drive. NOTE: 38/39 are unusable for link pins regardless of availability --
// they are in the upper GPIO bank, and the hot loop reads and writes a single
// 32-bit register.
#define RFU_PIN_SC 5
#define RFU_PIN_SO 6
#define RFU_PIN_SI 7
#define RFU_PIN_SD 8
// The GBA provides no chip-select, so one is synthesised on a spare pin and
// pulsed inside the inter-word gap to frame each word for the SPI slave.
#define RFU_PIN_CS 1
#else
// Classic ESP32: avoids the strapping pins (0/2/5/12/15), the flash pins
// (6-11), the console UART (1/3), and the input-only range (34-39).
#define RFU_PIN_SC 18
#define RFU_PIN_SO 19
#define RFU_PIN_SI 21
#define RFU_PIN_SD 22
#define RFU_PIN_CS 23
#endif

typedef enum
{
    RFU_ROLE_GBA_MASTER,     // the GBA clocks (command phase)
    RFU_ROLE_ADAPTER_MASTER  // we clock (wait-response delivery)
} rfu_role_t;

// Invoked from the link task with each 32-bit word received in GBA-master role.
typedef void (*rfu_transfer_done_t)(uint32_t rx, void *user);

void rfu_link_set_done_callback(rfu_transfer_done_t cb, void *user);

// Claim the pins, bias them like a real adapter, and start in GBA-master role
// with the RESET-state response (0) staged.
void rfu_link_enable(void);
void rfu_link_disable(void);

// Service the link. In GBA-master role this runs the idle-gap SI mirror and,
// on an SC falling edge, clocks one whole 32-bit word (invoking the done
// callback). Must be called from a tightly spinning task; it is the equivalent
// of the Pico's always-running state machine.
void rfu_link_poll(void);

void rfu_link_set_role(rfu_role_t role);
rfu_role_t rfu_link_get_role(void);

// Adapter-master role only: clock one 32-bit word into the GBA-as-slave and
// return what it shifted out. Synchronous. The inter-word ready handshake is
// the caller's job (rfu_link_master_drive_si + polling rfu_link_gba_line_high).
bool rfu_link_master_exchange(uint32_t word, uint32_t *rx, uint32_t timeout_us);

// Adapter-master role only: drive SI for the inter-word handshake.
void rfu_link_master_drive_si(bool high);

// Stage the next transmit word for the GBA-master role. Exactly one word is
// kept staged at all times.
void rfu_link_push_tx(uint32_t word);

// Raw level of the GBA's SO line.
bool rfu_link_gba_line_high(void);

// True once per soft reset: a rising edge was latched on SD since the last
// call, meaning the game ran AgbRFU_SoftReset and all link state must drop.
bool rfu_link_sd_reset_seen(void);

// Counters for bring-up: words exchanged in each role, and words abandoned
// because the GBA stopped clocking mid-word.
void rfu_link_stats(uint32_t *slave_words, uint32_t *master_words, uint32_t *partial);

// Breakdown of partial words: how many died at bit 0 (a false start -- no second
// edge ever arrived) versus later in the word (genuinely too slow), plus the bit
// index of the most recent failure.
void rfu_link_partial_detail(uint32_t *at_bit0, uint32_t *late, uint8_t *last_bit);

// Single-sample lows on SC rejected by the word-start debounce -- i.e. noise on
// the clock line that did not become a phantom word.
uint32_t rfu_link_glitches(void);

// True when the clock has rested high long enough that the GBA is provably
// between words, so the caller may safely do other work. Its inter-word gap is
// at least 40 us; anything shorter is inter-bit and must not be interrupted.
bool rfu_link_idle_gap(void);

// Total wire samples taken. Divided by elapsed time this is the sampling rate,
// which must comfortably exceed the 2 MHz clock's 250 ns half-bit to be viable.
uint32_t rfu_link_polls(void);

// SPI framing diagnostics: how many armed words completed with no clocks at all
// (a framing pulse the GBA never used), and the bit count of the most recent
// completion. 32 means a real word.
uint32_t rfu_link_empty_frames(void);
uint16_t rfu_link_last_trans_bits(void);

// Self-test: drive CS each way and read the pin back. Returns true only if the
// pin follows. bit4 of rfu_link_levels() is the CS level.
bool rfu_link_cs_controllable(bool *reads_low, bool *reads_high);

// Framing self-test needing no GBA: clocks 32 pulses into our own slave and
// reports the bit count the transaction closed with (32 = framing correct) plus
// the word received. Only meaningful while the GBA is idle or unplugged.
bool rfu_link_loopback_test(uint16_t *bits_seen, uint32_t *word_seen);

// The first words as received, for checking bit order and alignment against
// what the protocol expects (the GBA opens checkID with 0x0000494E).
uint8_t rfu_link_word_log(uint32_t *out, uint8_t max);

// True once rfu_link_enable() has claimed the pins.
bool rfu_link_enabled(void);

// Live levels of the four link lines, for bring-up: bit0 SC, bit1 SO, bit2 SI,
// bit3 SD. With nothing connected SC and SO read high (pulled up) and SD low.
uint8_t rfu_link_levels(void);

#ifdef __cplusplus
}
#endif
