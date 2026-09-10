#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Outbound RFU1 frames produced by the adapter, destined for the host bridge.
typedef void (*rfu_section_emit_t)(const uint8_t *frame, size_t len, void *user);

void rfu_section_set_emit(rfu_section_emit_t cb, void *user);

// Start the adapter. Spawns the link task pinned to the core that is not
// running Wi-Fi, because the GBA-master role is bit-banged and a preemption
// mid-word slips every remaining bit.
void rfu_section_start(int core_id);
void rfu_section_stop(void);

// Feed bytes of the inbound RFU1 stream (from the host bridge). Frames are
// parsed and applied to the adapter between GBA commands.
void rfu_section_feed(const uint8_t *data, size_t len);

// Same, with the signature the wire layer's RFU handler hook expects.
void rfu_section_feed_bytes(const uint8_t *frame, size_t length);

// Drain one queued outbound frame, if any. Called from the task that owns the
// wire layer, so the adapter task never touches its buffers.
bool rfu_section_poll_outbound(uint8_t *buffer, size_t capacity, size_t *length);

// Bring-up counters.
void rfu_section_stats(uint32_t *slave_words, uint32_t *master_words,
                       uint32_t *partial, uint32_t *net_in, uint32_t *net_out);

// Bring-up: is the link claimed, what role, and the live line levels.
void rfu_section_state(bool *enabled, int *role, uint8_t *levels, uint8_t *comstate);

#ifdef __cplusplus
}
#endif
