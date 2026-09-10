#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Link to a GB-Link Pico running the wireless-adapter firmware.

   The Pico owns the GBA cable: its PIO is the only thing in this stack that
   meets the 2 MHz SIO32 word timing (the S3's GPIO sits behind the 80 MHz APB
   bus, ~403 ns per access against a 250 ns half-bit). It hands over the same
   'GB' framed transport it would otherwise send a host PC over USB CDC:

       | 0x47 0x42 | channel:1 | len:2 LE | payload[len] |

   Frames are forwarded whole, header included, so anything downstream sees the
   byte stream unchanged from what the Pico's CDC port would have produced. */

/* UART signals are routed through the GPIO matrix, so these are free choices;
   swapping TX/RX here is all that a reversed pair needs. The Pico side is fixed
   by its mux: GP8 can only be uart1 TX, GP9 only uart1 RX. */
#define PICO_LINK_PIN_TX 2 /* -> Pico GP9, the Pico's uart1 RX */
#define PICO_LINK_PIN_RX 1 /* <- Pico GP8, the Pico's uart1 TX */
#define PICO_LINK_BAUD 921600

#define PICO_LINK_MAX_PAYLOAD 128
#define PICO_LINK_CHANNEL_COMMAND 0x00
#define PICO_LINK_CHANNEL_DATA 0x01
#define PICO_LINK_CHANNEL_STATUS 0x02

void pico_link_start(int core_id);
void pico_link_stop(void);
bool pico_link_running(void);

/* Send raw pre-framed bytes to the Pico (what the host emits verbatim). */
bool pico_link_write(const uint8_t *bytes, size_t length);

/* Wrap a payload in a 'GB' frame and send it. */
bool pico_link_send(uint8_t channel, const uint8_t *payload, size_t length);

/* Drain one complete frame received from the Pico, header included. Mirrors the
   old adapter's outbound accessor so the wire layer stays single-threaded: the
   UART task never touches it, the control poll does. */
bool pico_link_poll_inbound(uint8_t *frame, size_t capacity, size_t *length);

/* Re-issue SetMode(rfuWireless) to the Pico. Sent automatically on start and
   retried until the Pico answers; exposed for a manual retry after that. */
void pico_link_set_mode(void);
uint32_t pico_link_mode_sent(void);
bool pico_link_await_mode(void);
bool pico_link_wrong_cable(void);
bool pico_link_gba_active(void);

/* Discard frames buffered before the host was listening. */
void pico_link_reset_inbound(void);

/* Reboot the Pico into its USB bootloader so it can be reflashed. */
void pico_link_bootsel(void);

/* Swap which physical pin is TX and which is RX, at runtime. */
void pico_link_swap(void);
bool pico_link_swapped(void);

/* Report whether each pin has a driver on the far end (an idle UART line is
   held high, a floating pin follows whichever pull is applied). */
void pico_link_probe(bool *tx_driven, bool *rx_driven);

void pico_link_stats(uint32_t *rx_frames, uint32_t *tx_frames, uint32_t *rx_bytes,
                     uint32_t *resync, uint32_t *dropped);

/* Hex of the first frame seen since boot, for confirming the pair is not
   reversed and the baud matches. Returns bytes written. */
uint8_t pico_link_first_frame(uint8_t *out, uint8_t capacity);

/* Recent data-channel frames, header included, for decoding the adapter's own
   diagnostics (tag 0x0E carries dbgAnyRx / dbgNintendo / comstate). */
uint8_t pico_link_frame_log(uint8_t slot, uint8_t *out, uint8_t capacity);

/* Most recent status-channel codes from the Pico (0xFF02 AwaitMode means the
   adapter section is running, 0xFF0D WrongCable means it sampled the wrong one). */
uint8_t pico_link_status_log(uint16_t *out, uint8_t capacity);
