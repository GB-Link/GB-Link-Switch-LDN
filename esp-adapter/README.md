# GBA Wireless Adapter on an ESP32

The GBA Wireless Adapter (AGB-015), emulated on the ESP32's own pins, so the bridge no
longer needs a GB-Link's RP2040 to talk to the GBA. Builds for the original ESP32, the
ESP32-S3 and the ESP32-C6.

| Folder | What it is |
|---|---|
| `components/rfu_adapter` | The adapter: the GB-Link firmware's protocol core (`rfuProtocol.hpp`) and the ESP32 side of the link port. The core differs from the firmware's in one way: it resets its state in place, so the GBA's word interrupt never needs a large stack. |
| `adapter/` | A board that is only the adapter. It speaks the GB-Link's UART protocol, so it takes the GB-Link's place next to a bridge board running the normal firmware. |
| `bridge/` | The Switch bridge with the adapter on the same chip: one board between the GBA and the Switch. |
| `test/` | Tests that run on a PC. |

## How it works

The GBA clocks its words: the 256 kHz login and the 2 MHz commands. Those go through
the chip's SPI peripheral as a slave, so the hardware shifts every bit. The GBA has no
chip-select line. The slave's chip select comes from the GPIO matrix's constant inputs
instead: it is asserted when a reply is loaded and released to end the word. A pulse
counter on SC counts the GBA's 32 clock edges and raises the word interrupt.

The slave can miss a word's last bit. A second pulse counter counts the ones SO shows
at SC's rising edges, and the last bit is taken from that count.

Between words, the GBA's wireless library runs a handshake on SI and SO. The interrupt
holds SI high, waits for the GBA's SO, and then loads the next reply. The level SI
shows before the first clock edge is what lets the GBA go. Nothing has to happen within
a bit time: the GBA waits for the handshake.

SD, the GBA's reset line, rests high. The game resets the adapter by pulling SD low,
high for about a millisecond and low again, so two falls 0.5 to 5 ms apart are a reset.
Any fall during an event delivery ends the delivery and lets go of SC. When the GBA is
not holding SD, the line can chatter at hundreds of kilohertz, most of all while the
self-test moves SI. Falls that close together are never a reset's, so a run of them
turns SD's interrupt off for 50 ms instead of letting it take the core.

SI's edges couple into SC along the cable. The board drives SI and SC at the lowest
drive strength, and on the S3 and C6, SC, SO and SD pass through the GPIO glitch
filter.

When the adapter has an event for the GBA, the adapter drives the clock. It bit-banges
those words on GPIO at about 160 kHz, with the same handshake the GB-Link runs. A
slower bit only stretches the word.

The SPI slaves of the three chips differ in when they put their first bit out. At
power-up a self-test clocks test words through the slave from the GPIO matrix's
constant inputs and watches SI. It picks the setting that puts each bit out before the
GBA samples it, and finds where the received word starts.

## Wiring

Count the pins at the plug that goes into the GBA. A link cable crosses SO and SI
between its two ends.

| GBA link port | Line | ESP32 | ESP32-S3 | ESP32-C6 |
|---|---|---|---|---|
| 2 | SO, the GBA's output | GPIO19 | GPIO6 | GPIO20 |
| 3 | SI, the GBA's input | GPIO21 | GPIO7 | GPIO18 |
| 4 | SD, the GBA's reset line | GPIO22 | GPIO8 | GPIO21 |
| 5 | SC, the clock | GPIO18 | GPIO5 | GPIO19 |
| 6 | GND | GND | GND | GND |

- Do not power the board from pin 1 (the GBA's 3.3 V). The board runs from its own USB.
- The port runs at 3.3 V while a GBA game is running. A Game Boy or Game Boy Color
  cartridge switches it to 5 V, which the ESP32's pins do not tolerate. Only GBA games
  use the wireless adapter, so keep the board off the port otherwise.
- The one-board bridge can report which of the two the port is in. Wire pin 1 to an
  ADC pin through a divider, 10 kΩ from pin 1 to the pin and 10 kΩ from the pin to GND,
  and set that pin under `menuconfig`, "LDN bridge". ADC pins: GPIO32-39 on the ESP32,
  GPIO1-10 on the S3, GPIO0-6 on the C6. The console then prints `LDN_GBA_VCC` with the
  voltage and `off`, `gba` (3.3 V) or `gb` (5 V) at power-up and whenever it changes.
- A 100 to 220 Ω resistor in series with SI and with SC (the two lines the board
  drives) is optional protection.
- Other pins: `menuconfig`, "GBA wireless adapter". Any output-capable GPIO works.
  Avoid strapping pins, the flash and PSRAM pins, and the USB pins.

## Two boards: the adapter in place of a GB-Link

Flash `adapter/` to one board. The other board keeps the normal bridge firmware
(`firmware/ESP32*`) and talks to the adapter board exactly as it would to a GB-Link.
The chips can differ, for example a C6 adapter next to an S3 bridge.

| Adapter board | Bridge board |
|---|---|
| TX: GPIO17 (ESP32), GPIO2 (S3, C6) | RX: GPIO16 (ESP32), GPIO1 (S3, C6) |
| RX: GPIO16 (ESP32), GPIO1 (S3, C6) | TX: GPIO17 (ESP32), GPIO2 (S3, C6) |
| GND | GND |

The adapter runs from power-up and keeps running when the bridge board restarts.
During play the adapter board only needs power. Opening or closing its console can
reset it, and the GBA with it, so close the console before playing.

The console (USB on the S3 and C6, the UART bridge at 115200 on the ESP32) has two
commands:

- `status`: the self-test, the GBA's state, word and handshake counters, event
  deliveries, resets, the network side and the level on each line.
- `selftest`: runs the self-test again. Only with the GBA off or idle, as it drives SI.

## One board: `bridge/`

The bridge firmware with the adapter on its own pins. The page, the keys and the
console protocol are the same as for the bridge with a GB-Link. The adapter starts with
the board and reports through the same frames a GB-Link sends, so the page's wiring
check hears it. At boot the console prints the self-test:

    LDN_WA adapter on SC 5 SO 6 SI 7 SD 8, self-test passed: mode 3, ...

The board looks for a Switch's room only while the GBA's player looks for a group (JOIN
GROUP at the Direct Corner, or in the Union Room), and leaves a room again when the player
stops looking without joining it. A board whose GBA is doing something else stays out of
other rooms, so it never takes a place meant for a Switch.

After every session and every failed join the bridge starts over without restarting
the chip. The adapter keeps its state, so a GBA between links, in the Union Room or
back at the counter, carries on. A link still open at that moment ends as if the
partner had left.

The Union Room works both ways. The GBA's player shows up in the Switch's Union Room
and the Switch's player in the GBA's, and either side can start an activity: trainer
cards, trades, the trading board, battles and chat. The bridge hosts the GBA's player
while it waits, and joins the Switch's network when the GBA starts an activity with the
Switch's player. While hosting, the radio hears only the room's channel. The room opens
where a Switch's Union Room was last heard, if one was. Until a Switch is heard there, the
radio steps off the channel for a fifth of a second each second to listen on the other two,
with the room kept open, and a Switch found on another channel has the room moved to it.

Limits of one board:

- Opening or closing the USB console can reset the chip, and the adapter with it: a
  GBA using the adapter at that moment gets a link error. Keep the page connected
  through a session.
- Memory on the original ESP32: the adapter takes about 14 KB of static RAM. That
  leaves about 6 KB of the chip's static RAM free, against about 21 KB for the bridge
  with a GB-Link, and the heap is smaller by about the same amount. With a GB-Link, that
  chip had about 21 KB of heap free before hosting a room (GBA leads), so hosting there
  may not fit. The S3 and C6 have room to spare.

## Build and flash

ESP-IDF v6.1 at the same commit as the bridge firmware, in `~/esp/esp-idf-v6.1` (or set
`SDK_PATH`).

    ./build.sh adapter esp32s3                     # build
    ./build.sh bridge esp32c6 flash /dev/ttyACM0   # build and flash
    ./build.sh adapter esp32 monitor /dev/ttyUSB0  # console
    ./build.sh bridge esp32 size                   # memory use

Each app and chip builds in its own `build-<app>-<chip>` folder. For menuconfig, run
`idf.py -C adapter -B build-adapter-esp32s3 menuconfig`.

## Bring-up

1. Flash `adapter/` with no GBA connected and open the console. The first line after
   the pins is the self-test. On `FAILED` the line shows what SI did for two settings;
   that line is what to report.
2. Wire the GBA with it switched off, then start a game and open its wireless menu. The
   console shows each step as it happens:
   - `WA the GBA is clocking:` then the first words. The first should be `0000494e`,
     the GBA's login. Anything else (a shifted or inverted value) means the self-test
     chose the wrong setting: report the line with the self-test's. If the words never
     reach the login, check first that SO and SI are not swapped. Swapping `WA_PIN_SO`
     and `WA_PIN_SI` in menuconfig fixes that as well as moving the wires does.
   - `WA the GBA started the ID exchange`
   - `WA the GBA found the adapter and sends commands`
3. `status` should show no growing count of bad bit counts or realigned words, and no
   failed event deliveries.
4. Wire the bridge board (or flash `bridge/` instead) and play.

## Settings

| Setting | Default | |
|---|---|---|
| `WA_PIN_SC`, `WA_PIN_SO`, `WA_PIN_SI`, `WA_PIN_SD` | see Wiring | |
| `WA_HANDSHAKE_SPIN_US` | 15 | How long a word's interrupt waits for the GBA's handshake before leaving it to the SO interrupt. |
| `WA_TASK_PRIORITY` | 15 | |
| `WA_TASK_CORE` | 1 | Dual-core chips: the core for the adapter task and its interrupts, away from Wi-Fi. |
| `GB_LINK_TX_GPIO`, `GB_LINK_RX_GPIO` | see Two boards | `adapter/` only. |

## Tests

- `test/link/run.sh`: the one-board link against a scripted adapter. It checks when
  SetMode reaches the adapter, the frames in both directions, and the page's adapter
  port.
- `test/run.sh`: the adapter against a simulated GBA running the game's own link
  library, under a spread of timings and faults (slow interrupts, a corrupted bit, a
  spike on SC, a reset mid-delivery, interrupts held off, a mode entry after a session),
  and leading a group.
  The script's header says what it needs.
