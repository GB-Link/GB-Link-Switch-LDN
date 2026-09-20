# Web client

One page that sets up both boards and, if you want, carries the link between them. It
is a static site with no build step and no server side: everything runs in the browser
over Web Serial and WebUSB.

1. **ESP32 board** – connects to the ESP32, installs or updates its firmware (the chip
   is detected, so the same button serves the ESP32-S3, C6, C3 and the original ESP32),
   and stores the four console keys from a `prod.keys` you drop on the page.
2. **GB-Link adapter** – connects over WebUSB or serial, reports whether its firmware
   has the wireless adapter mode, and installs the firmware that does.
3. **Play** – shows what the ESP32 board is doing (looking for a room, joining, linked),
   checks the wiring for standalone use, or stands in for the wires: with both boards
   on USB, the page passes the adapter's traffic between them.

The first two cards each show one line of status and at most one button, for whatever
comes next on that board: connect, install firmware, add keys. A board that is ready
shrinks to a single line. Everything else (installing again, erasing, a `.uf2` of your
own, connecting over serial, disconnecting) is under *More options*.

Playing without a Game Boy Advance, with the computer standing in for the GBA, is not
part of this page yet. The desktop and console hosts in `host/` do that, on the same
firmware.

## Running it

Browsers only expose USB devices to pages served over `https://` or from `localhost`.
Any static host works (GitHub Pages included). Locally:

```
python3 -m http.server -d web 8000
```

then open <http://localhost:8000> in Chrome, Edge or another Chromium browser on a
computer.

| Browser | ESP32 board | Adapter | Installing adapter firmware |
| --- | --- | --- | --- |
| Chrome, Edge, other Chromium (desktop) | yes | WebUSB or serial | yes |
| Firefox 151 and later | yes | serial only | by hand (`.uf2` onto the `RPI-RP2` drive) |
| Safari, mobile browsers | no | no | no |

### Linux

- The serial ports need the usual group membership (`dialout` on Fedora, Debian and
  Ubuntu; `uucp` on Arch).
- WebUSB access to the adapter and to its bootloader needs udev rules; the GB-Link
  firmware repository installs them with `scripts/setup-linux-permissions.sh`.
  Connecting the adapter over serial works without them.
- A serial port keeps its line settings between programs. After a tool built on pyserial
  has used a port (`esptool.py`, `idf.py monitor`, the scripts in `firmware/tools/`),
  Chrome reports the port as lost the moment it opens it. The page recognises this and
  says so; unplugging the board and plugging it back in clears it, as does
  `stty -F /dev/ttyACM0 min 1`.

## What the steps do

### ESP32 board

*Connect* uses a port the page was given before without asking again, and otherwise
opens the browser's list. Running firmware answers within milliseconds, so the page
only waits on a board that is visibly starting up; a chip that prints its ROM's
"invalid header" or "waiting for download", or the boot log of some other project, is
reported as having no firmware after about two seconds, with *Install firmware* as the
one button. On a board with a USB-UART chip the page lets RTS go before DTR after
opening the port: reset is RTS without DTR, and a CP2102 moves the two pins one after
the other, so releasing both at once restarts the board.

*Install firmware* puts the chip in its bootloader, identifies it, and writes the three
images listed for it in `firmware/manifest.json` with
[esptool-js](https://github.com/espressif/esptool-js), verifying each by MD5. The flash
size in the bootloader header is set to what the chip reports. The images are written
separately rather than as one merged file, so the key store between them survives an
update; *Erase everything and install*, under *More options*, wipes it as well. A board
that does not enter its bootloader on its own needs BOOT held while RESET is pressed.
The page resets the chip itself afterwards, because the hard reset in esptool-js 0.6.1
never asserts RTS, and then reconnects.

The keys: the page reads the `prod.keys` you give it, takes
`aes_kek_generation_source`, `aes_key_generation_source`, `master_key_00` and
`master_key_12`, and sends those four values to the board over USB. Nothing is
uploaded, stored by the page or written to the log, and the board never sends a key
back: it only reports which of the four it holds.

### GB-Link adapter

The adapter answers a statistics command (`0x4c`) only when its firmware has the
wireless adapter mode, which is how the page tells. *Install firmware* walks through
four steps on the card: the page restarts the adapter in its USB bootloader, you pick
"RP2 Boot" in the browser's list (the bootloader is a different USB device, so the
browser has to be given access once more; that click is the only one needed), the page
writes the bundled `.uf2` with [picoflash](https://github.com/picoflash/picoflash), and
it reconnects to the restarted adapter. An adapter that is already in its bootloader
can be picked with *Connect* and goes straight to writing. A `.uf2` of your own can be
installed instead, and the manual route (BOOTSEL, then copy the file to the `RPI-RP2`
drive) always works; both are under *More options*.

### Play

Standalone, the two boards are joined by three wires and need no computer: the page only
shows which pins to join and checks whether the board hears the adapter. Through the
computer, the page asks the ESP32 board for its adapter port (`LDN_ADAPTER host`) and
relays GB-Link frames both ways (message kinds 6 and 7 of [the serial
protocol](../docs/SERIAL_PROTOCOL.md)). The board restarts after every session, which
drops that setting; the page notices the boot banner, repeats the handshake and takes
the port again, so consecutive sessions need no clicks. Keep the tab open and visible
while playing. While it carries the link the page also watches how often the game resets
the adapter: a game that cannot get a command through does so about four times a second
and blocks meanwhile, which on the GBA is a freeze with no message, so the page says
what is happening and to try another link cable.

A chip's own USB port ignores the baud rate. A board behind a USB-UART bridge (the
original ESP32) runs its console at 921600 from the first line it prints, because a
browser cannot change the rate of a port it has open and the adapter's traffic needs
more than 115200 gives; the page opens such ports at 921600 and falls back to 115200,
where it finds firmware from before that (which can be updated but cannot carry the
link) and can read what a chip's bootloader prints.

## Firmware images

`firmware/` holds what the page installs: per-chip bootloader, partition table and
application for the ESP32 board, and the adapter's `.uf2`, listed in
`firmware/manifest.json`. After building, refresh them with

```
firmware/tools/package_web.py
firmware/tools/package_web.py --adapter-uf2 path/to/zephyr.uf2 --adapter-version 2.2.5
```

The offsets come from each build's `flasher_args.json`.

## Code

| File | |
| --- | --- |
| `js/wire.js` | COBS framing, CRC-32 and the message layout of the console protocol; GB-Link frames |
| `js/esp.js` | a session with the bridge firmware: handshake, commands, adapter frames, re-attaching after a restart |
| `js/gblink.js` | the adapter over WebUSB (endpoints framed and unframed here) or serial |
| `js/bridge.js` | relays frames between the two |
| `js/flash-esp.js`, `js/md5.js` | installing the bridge firmware |
| `js/flash-pico.js` | installing the adapter firmware |
| `js/keys.js` | picking the four values out of `prod.keys` |
| `js/manifest.js` | the bundled firmware list |
| `js/app.js` | the page: each card is drawn from one view of its state (status line, hint, one button) |

`firmware/tools/host_bridge.py` does the same relay from a terminal and is the reference
the JavaScript was checked against. `node web/tests/run.mjs` tests the protocol code
without hardware: the framing against vectors from that Python (`tests/make_vectors.py`
regenerates them), and the session logic against stand-ins for the firmware's console:
restarts, a board behind a USB-UART chip whose reset hangs off DTR and RTS, and what a
blank, foreign or crashing chip prints.

## Third-party code

- `vendor/esptool-js` – [esptool-js](https://github.com/espressif/esptool-js) 0.6.1,
  Apache-2.0
- `vendor/picoflash` – [picoflash](https://github.com/picoflash/picoflash), MIT,
  © Piers Finlayson
