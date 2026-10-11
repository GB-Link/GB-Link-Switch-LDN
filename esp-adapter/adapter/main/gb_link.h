#pragma once
#include <stddef.h>
#include <stdint.h>

/* The GB-Link's framed serial link, as the bridge board's pico_link.c speaks it over
   its UART:

       | 0x47 0x42 | channel:1 | len:2 LE | payload[len] |

   Channel 0 carries commands to the adapter (SetMode), 1 data both ways (RFU1 frames
   in 64-byte chunks, telemetry in shorter ones), 2 status codes from the adapter. */

#define GB_LINK_COMMAND 0x00
#define GB_LINK_DATA 0x01
#define GB_LINK_STATUS 0x02

typedef void (*gb_link_handler_t)(uint8_t channel, const uint8_t *payload, size_t length);

void gb_link_start(gb_link_handler_t handler);
void gb_link_send(uint8_t channel, const uint8_t *payload, size_t length);
void gb_link_send_status(uint16_t code);
void gb_link_stats(uint32_t *rx_frames, uint32_t *tx_frames, uint32_t *resync);
