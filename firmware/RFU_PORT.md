# Wireless-adapter (RFU) port to the ESP32

Built for both `firmware/esp32-s3` (AtomS3) and `firmware/esp32` (classic). The
sources are identical; only the pin set differs, selected in `rfu_link.h`.

Goal: the ESP32-S3 speaks to the GBA over a link cable *and* to the Switch over
LDN, so the GB-Link Pico is no longer in the path. This removes a device, a USB
hop, and the latency that goes with them.

## Status

Builds clean against the pinned ESP-IDF v6.1. **Nothing here has run on
hardware** — it was written while the bench was unattended. Treat the wire
timing as the part most likely to need rework.

| Piece | File | State |
|---|---|---|
| Adapter state machine | `main/rfu_protocol.hpp` | Vendored verbatim, 1953 lines, no changes needed |
| GBA link layer | `main/rfu_link.c/.h` | Written for the S3, untested |
| Glue / delivery handshake | `main/rfu_section.cpp/.h` | Written, untested |
| Host transport wiring | `ldn_wire.c`, `ldn_control.c` | Written, untested |

The protocol core is pure C++ (`<cstdint>`, `<cstring>`, `<cstddef>` only), so
all of the hardware-proven behaviour — the ID dance, command handling, block
sends, the queue rules from fw 2.4.18/2.4.20 — came across unmodified. Re-vendor
from upstream rather than editing the copy.

## Wiring

All four pins are in the 0-31 GPIO bank so the hot loop can use single-register
access, and SC must be output-capable because we drive it in the adapter-master
role.

Wire signal-to-signal against the GBA's EXT connector. There is no crossover:
that exists only inside a link cable, between its two ends. (The Pico firmware's
"needs the grey GBC cable" rule was a quirk of the GB-Link board's cable
detection, not of the GBA protocol, and does not apply here.)

| GBA EXT pin | Signal | Direction |
|---|---|---|
| 1 | VCC 3.3 V | **leave unconnected** -- do not tie the supplies |
| 2 | SO, the GBA's output | GBA -> us |
| 3 | SI, the GBA's input | us -> GBA |
| 4 | SD, carries the soft-reset pulse | GBA -> us |
| 5 | SC, clock | GBA drives it first; we drive it as master |
| 6 | GND | required |

Both sides are 3.3 V logic, so no level shifting. Getting SO and SI the wrong way
round puts two outputs against each other -- verify with a continuity meter
rather than by wire colour, especially with a cut-open cable. `LDN_RFU_STATS`
reports the live line levels, and shows `si=0` once started because we hold our
output low as a real adapter does, so the output stage can be confirmed before
anything is connected.

| Signal | AtomS3 | Classic ESP32 | Direction |
|---|---|---|---|
| SC (clock) | 5 | 18 | input while the GBA clocks; output when we do |
| SO (GBA's data out) | 6 | 19 | input |
| SI (GBA's data in) | 7 | 21 | output |
| SD (soft-reset pulse) | 8 | 22 | input, pulled down |

The classic set avoids the strapping pins (0/2/5/12/15), the flash pins (6-11),
the console UART (1/3), and the input-only range (34-39). On the AtomS3, 38/39
are left spare.

**Prefer the classic ESP32 for bring-up**: far more pins are exposed, so a scope
can reach the link lines.

## How it works

Ported from the Pico's PIO implementation, whose comments record what was proven
on hardware:

* 32-bit words, **MSB first**. SC idles high; the bus master sets up on the
  falling edge and samples on the rising edge. Both roles use that geometry,
  mirrored.
* **GBA-master role** is edge-driven, so one path serves both 256 kHz and 2 MHz.
* **Idle-gap mirror**: while SC rests high, SI must continuously equal NOT SO,
  or librfu's `handshake_wait` busy-waits never release.
* **Role swaps must not glitch SC** — an armed GBA-as-slave counts any edge as a
  data bit, so the output latch is seeded high before the pin is driven.
* The final-word handshake waits 60 ms, carrying the fw 2.4.19 fix: a 4 ms
  budget expired during the post-trade save and stranded librfu mid-delivery.

Everything is bit-banged from one task pinned to a core, rather than using SPI.
The master side is slow (~154 kHz) and the slave side follows edges, which suits
a tight poll loop and avoids peripheral-handoff glitches on the shared SC pin.

## Host protocol

RFU1 frames ride the existing COBS/CRC32 serial protocol the same way UDP
datagrams do (kinds 4/5):

* **kind 6** — adapter to host
* **kind 7** — host to adapter

Adapter mode is opt-in so the LDN-only path is untouched:

| Command | Effect |
|---|---|
| `LDN_RFU_START` | claim the link pins, start the adapter task on core 1 |
| `LDN_RFU_STOP` | stop it |
| `LDN_RFU_STATS` | `slave_words`, `master_words`, `partial`, `net_in`, `net_out` |

Outbound frames are queued by the adapter task and drained by the task that
owns the wire layer, which serialises through static buffers.

The PC side still needs pointing at this: `GbLinkDevice` currently speaks the
Pico's `'G','B'` CDC framing, so it needs a variant that reads kind 6/7 frames
from the ESP port, after which `--bridge` takes one port instead of two.

## What is left

1. **PC-side transport.** See above.
2. **Core pinning.** `rfu_section_start(core_id)` must get the core Wi-Fi is not
   on (normally core 1). The loop spins to keep up with SC and yields only every
   200 ms; the task watchdog for that core likely needs adjusting.
3. **Flash writes stall the link.** All timing now uses the CPU cycle counter
   rather than `esp_timer`, so a disabled cache cannot skew a budget. But a flash
   write on the *other* core stalls this one outright, which would corrupt a word
   in flight. Avoid NVS/flash writes while a session is running; Wi-Fi does write
   calibration data, mostly at startup.
4. **Timing validation.** The 2 MHz slave case is the risk: 250 ns per half-bit
   is ~60 CPU cycles at 240 MHz. Interrupts are disabled for a whole word (~16 us
   at 2 MHz) and the hot path is in IRAM, but this needs a scope or the
   `rfu_link_stats()` `partial` counter to confirm. If it proves too tight,
   the fallback is the S3's SPI slave peripheral with CS tied low, re-armed in
   the >=40 us inter-word gap.
5. **Cable detection.** The Pico refuses the mode without the GBC cable. There is
   no equivalent check here.
