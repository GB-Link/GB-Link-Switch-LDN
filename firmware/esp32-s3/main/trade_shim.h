#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Reconciles the link-standby barrier between a retail FireRed/LeafGreen cartridge
   (the GBA, link child) and the Switch's NSO build of the same game (link parent).

   After a completed trade both games run a series of RFU "ready to exit standby"
   rounds around their saves before reopening the trade menu. Each round is a
   numbered barrier: the child sends READY_EXIT_STANDBY with its round counter,
   the parent answers with the same number once it is at that barrier too, and a
   command whose number does not match the receiver's own counter is ignored.
   The NSO build (pokefirered's REVISION 0xA, trade_scene.c CB2_SaveAndEndTrade
   cases 43/44) runs one round more than a retail cartridge. The parent therefore
   ends its post-trade sequence waiting for a round the child never sends, while
   the child waits for the parent's party request that only comes after it: both
   sit at "Communication standby" forever.

   The shim supplies that round on the child's behalf once per trade, after the
   cartridge has finished its own rounds and the wire has gone quiet, and from
   then on maps round numbers between the two sides. Because the parent also
   validates a mod-8 sequence tag on every child command, the injected frame
   consumes a tag and the child's later tags are shifted to stay consecutive.

   Two safeguards cover the Switch emulator dropping adapter frames around its
   own stalls: an injected round the parent never answers is re-sent, and a
   parent answer the child never saw (the child keeps retrying a barrier the
   parent has already left) is re-sent to the child on the parent's behalf.

   Three more cover the block exchange. The parent echoes each child block
   fragment it accepts exactly once; an echo that never arrives makes the
   child's link layer resend the fragment without a sequence tag, forever, and a
   block request that reaches the child during that loop is refused and never
   repeated. So the missing echoes are supplied before the one the child acts on,
   the child's command tags are re-sequenced here so that its untagged resends
   (and any frame lost before this point) do not break the parent's +1 check,
   and a block request the child did not answer is repeated to it.

   Every decision is recorded in a log that survives a chip reset, so a run can
   be examined afterwards even though opening the console resets the chip. */

void trade_shim_boot(int reset_reason);
void trade_shim_reset(void);

/* Child -> parent UNI frame (2-byte LLSF header + 14-byte command slot). Rewritten
   in place. When the child is retrying a barrier the parent already answered, one
   or two parent UNI frames (73 bytes each) for the child are written to `reply`
   and their total length returned. */
size_t trade_shim_child(uint8_t *payload, size_t length, int64_t now_ms, uint8_t *reply, size_t reply_capacity, bool *forward);

/* A child frame about to be sent to the parent (called for exactly the frames that are
   sent, in order): stamps the next consecutive sequence tag on a command frame. */
void trade_shim_stamp(uint8_t *payload, size_t length);

/* Periodic checks (the parent no longer echoing the child's commands). */
void trade_shim_poll(int64_t now_ms);

/* The reliable stream to the Switch has stopped advancing: the state for the log. */
typedef struct
{
    uint16_t our_low, our_next, our_pending;
    uint16_t peer_low, peer_ack_next, receive_next;
    int64_t stalled_ms, peer_low_age_ms, peer_ack_age_ms, peer_ack_seen_age_ms;
    uint16_t out_of_order, credits, k_queued, k_inflight, out_queued;
    int overflow, idle_evicted;
} trade_shim_pia_t;
void trade_shim_pia_stall(int64_t now_ms, const trade_shim_pia_t *p);
/* The stream moved again after a reported stall. */
void trade_shim_pia_recovered(int64_t now_ms);

/* Parent -> child UNI frame (3-byte LLSF header + five 14-byte slots). Rewritten in
   place. When the parent's echo of the child's last block fragment arrives while
   earlier echoes never did, the missing echo frames (73 bytes each) are written to
   `pre` and their total length returned: they must reach the child BEFORE this frame. */
size_t trade_shim_host(uint8_t *payload, size_t length, int64_t now_ms, uint8_t *pre, size_t pre_capacity);

/* The extra child round when it is due (or a re-send of one the parent has not
   answered): writes a child UNI frame and returns its length, or returns 0. */
size_t trade_shim_inject(int64_t now_ms, uint8_t *out, size_t capacity);

/* A repeat of the parent's block request when the child has not started answering
   it: writes a parent UNI frame (73 bytes) for the child and returns its length, or
   returns 0. Poll it alongside trade_shim_inject. */
size_t trade_shim_host_inject(int64_t now_ms, uint8_t *out, size_t capacity);

/* Context from the bridge for the log. */
enum { TRADE_SHIM_NOTE_BRIDGE_START = 1, TRADE_SHIM_NOTE_CHILD_CONNECT, TRADE_SHIM_NOTE_CHILD_DISCONNECT,
       TRADE_SHIM_NOTE_HOST_DISCONNECT, TRADE_SHIM_NOTE_BRIDGE_STOP, TRADE_SHIM_NOTE_HOST_SILENCE,
       TRADE_SHIM_NOTE_PARENT_DISCONNECT_CMD, TRADE_SHIM_NOTE_CHILD_TAG_GAP, TRADE_SHIM_NOTE_SWITCH_CLOCK_SKIP,
       TRADE_SHIM_NOTE_ECHO_SYNTHESIZED, TRADE_SHIM_NOTE_PARENT_FRAGMENT_GAP, TRADE_SHIM_NOTE_ECHO_GAP,
       TRADE_SHIM_NOTE_RESTAMPED_RESEND, TRADE_SHIM_NOTE_ADAPTER_TRACE, TRADE_SHIM_NOTE_UNZIP_FAILURE,
       TRADE_SHIM_NOTE_ECHO_STALL, TRADE_SHIM_NOTE_PIA_STALL, TRADE_SHIM_NOTE_STALL_RECOVERED };
void trade_shim_note(int64_t now_ms, uint8_t kind, uint16_t b, uint16_t c);

/* Adapter telemetry frame from the Pico (a data-channel payload shorter than 64
   bytes). Loss counters that change are written to the log. */
void trade_shim_adapter(int64_t now_ms, const uint8_t *frame, size_t length);

/* The bridge's own loss counters, sampled on every poll; logged on change and
   periodically. */
void trade_shim_bridge_counters(int64_t now_ms, uint16_t reordered, uint16_t hold_dropped, uint16_t overflow, uint16_t queued,
                                uint16_t bad_frames, uint16_t decrypt_failures, uint16_t unzip_failures,
                                uint16_t host_skips, uint16_t host_long_skips,
                                uint16_t rx_body_max, uint16_t unzip_last_error);

void trade_shim_status(char *out, size_t capacity);
/* The log, a few lines per call so a long dump never holds up the bridge: begin, then
   step until it returns false. An incident record, if any, comes first and is cleared
   once the whole log has been emitted. */
void trade_shim_dump_begin(void);
bool trade_shim_dump_step(void (*emit)(const char *line), int max_lines);
