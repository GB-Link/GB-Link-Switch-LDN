/* A GBA Wireless Adapter for the emulated GBA (rfu_emu.cpp): the GB-Link's adapter core with
 * the GB-Link's network side, framed for the ESP32 board's adapter port. Times are the
 * game's, in milliseconds. */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The GBA side, called by the link-port driver from inside the game's run. */

/* A fresh adapter, as at power-up: in the wireless mode, reporting it to the board. */
void rfu_power_on(uint32_t now_ms);
/* The word the adapter shifts back in the next transfer the game clocks. */
uint32_t rfu_reply(void);
/* A transfer the game clocked has ended: its word. */
void rfu_word(uint32_t rx, uint32_t now_ms);
/* Every millisecond or so: frames from the board, the adapter's upkeep and reports.
 * Returns the number of words of an event to clock into the game, 0 for none. */
int rfu_upkeep(uint32_t now_ms);
uint32_t rfu_event_word(int index);
/* An event is being clocked into the game. */
bool rfu_delivering(void);
/* All of its words have been clocked. */
void rfu_delivered(uint32_t now_ms);
/* The game was not ready for it: before its first word (started false) the event is put
 * back for the next try. */
void rfu_delivery_failed(uint32_t now_ms, bool started);
/* The game reset the adapter (its SD pulse). */
void rfu_sd_reset(uint32_t now_ms);

/* The network side, called by the page between runs. */

/* The board's SetMode(wireless): a fresh adapter in that role (0 any). */
void rfu_enter_mode(int role);
/* Bytes from the board's data channel: RFU1 frames in 64-byte chunks. */
void rfu_feed(const uint8_t* bytes, int length);
/* Takes up to capacity bytes of GB frames for the board; returns how many. */
int rfu_take(uint8_t* out, int capacity);

/* Supplied by the build: 16 random bits, for the adapter's device id. */
uint16_t rfu_random16(void);

#ifdef __cplusplus
}
#endif
