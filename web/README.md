# Web client

One page that sets up the boards and, if you want, carries the link between them or
plays the second game itself. It is a static site with no build step and no server side:
everything runs in the browser over Web Serial and WebUSB.

The page has two trees, chosen at the top and remembered. `#gba` and `#switch` in the
address open one directly.

**GBA to Switch** – a real GBA joins the Switch's room. Needs the ESP32 board and a
GB-Link adapter.

1. **ESP32 board** – connects to the ESP32, installs or updates its firmware (the chip
   is detected, so the same button serves the ESP32-S3, C6, C3 and the original ESP32),
   and stores the four console keys from a `prod.keys` you drop on the page.
2. **GB-Link adapter** – connects over WebUSB or serial, reports whether its firmware
   has the wireless adapter mode, and installs the firmware that does.
3. **Play** – shows what the ESP32 board is doing (looking for a room, joining, linked),
   checks the wiring for standalone use, or stands in for the wires: with both boards
   on USB, the page passes the adapter's traffic between them.

**Just the Switch** – the page plays the second game itself, in place of the GBA. Needs
only the ESP32 board.

1. **ESP32 board** – the same card, set up the same way.
2. **Trade** – with one of two things:
   - *Wonder Trade*: a trade with the online pool of <https://pokemon.gblink.io>
     (<https://pokemon.gblink.io/pool> shows what is in it), on the same server
     (`wss://pokemon-gb-online-trades.herokuapp.com`, path `/pool3`; the address can be
     changed under *More options*). The pool offers one of its Pokémon, and the one the
     Switch gives for it goes into the pool. The pool is reached when you connect, but
     no Pokémon is taken from it until the player on the Switch has let the page into
     the group, because a Pokémon on offer to one connection is kept from everyone else.
     It comes into view on the page together with the Switch's team, as it does on the
     Switch. The pool is asked before the trade is confirmed, so a Pokémon it will not
     take never leaves the Switch.
   - *PK3 files*: a party of your own, kept in the browser. Pokémon go in and out as
     `.pk3` files (↓ and ↑ on each one, between visits), and what the Switch sends takes
     the place of what you gave. `assets/party.json` here is the party a first visit
     starts from, a copy of the one in the repository root; replace both before
     publishing if you would rather not ship your own Pokémon.

   Click one of your Pokémon to offer it, as the player on the Switch chooses one there;
   nothing is offered until you do, because the game's leader waits for both players and
   will not let its own player leave the menu while a partner's offer is standing. The
   leader also never sends its own player's pick (only its cancel goes out on the link),
   it judges that cancel the moment its own broadcast completes, and an offer that is in
   cannot be taken back. So the page cannot wait for a pick and answer it: an offer made
   ahead of the pick costs the Switch a second CANCEL, and one held back makes the pick
   wait for it. The pool's Pokémon is therefore held back too: the page waits for *Accept
   trade* or *Cancel trade* under it, pressed before or after choosing on the Switch, and
   neither can be changed once the trade is under way. CANCEL in
   the trade menu brings both players back to the room, and sitting down at the table
   again opens a new menu: with the pool, that is how to get a different Pokémon. Keep
   the tab in view: a browser slows a tab it is not showing, and a game that stops
   answering is dropped. The pictures come from
   [PokeAPI](https://github.com/PokeAPI/sprites).

The first two cards each show one line of status and at most one button, for whatever
comes next on that board: connect, install firmware, add keys. A board that is ready
shrinks to a single line. Everything else (installing again, erasing, a `.uf2` of your
own, connecting over serial, disconnecting) is under *More options*.

**The board does the wireless, trading included.** It finds the room, joins it,
decrypts everything and runs the session, exactly as it does for a real Game Boy
Advance. The page asks to stand in for the adapter (`LDN_ADAPTER host`) and answers the
plain frames the board passes on. So the page holds no key, decrypts nothing, and needs
no firmware beyond the 2.0 the rest of the page already installs.

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
| `js/app.js` | the page: the two trees, and each card drawn from one view of its state (status line, hint, one button) |
| `js/trade/` | trading without a Game Boy Advance: `adapter.js` (the wireless adapter's frames, as the board relays them), `link.js` (joining the room and answering each frame), `rfu.js` and `engine.js` (the games' link protocol and the trade itself), `pk3.js` with the generated `pk3-data.js` (Pokémon data), `pool.js` (the trade pool's server), `party.js`, `sprites.js`, `session.js` (a visit to the room, with either) and `bytes.js` |
| `js/launcher-return.js` | the "Launcher" button shown when the page is opened from the [GB-Link launcher](https://launcher.gblink.io) (`?from=gblink-launcher`); the same file the other GB-Link web clients carry |

`firmware/tools/host_bridge.py` does the same relay from a terminal and is the reference
the JavaScript was checked against. Three suites run without hardware:

```
node web/tests/run.mjs            # framing, and a session against a stand-in console
node web/tests/trade.mjs          # the trade code against the reference host
node web/tests/session-test.mjs   # a whole trade against a stand-in Switch
```

`run.mjs` checks the framing against vectors from that Python (`tests/make_vectors.py`
regenerates them) and the session logic against stand-ins for the firmware's console:
restarts, a board behind a USB-UART chip whose reset hangs off DTR and RTS, and what a
blank, foreign or crashing chip prints.

`trade.mjs` checks the trading code against the C# host in `host/`, which has made real
trades: the Pokémon data against PKHeX through 93 generated Pokémon, the adapter's
frames through a torn and padded stream, and the trade engine by replaying recorded
scenarios of the host's own engine call by call. It also checks the trade pool's client
against a stand-in for the pool's server (`tests/fake-pool.mjs`, which follows
`serving.py` of [PokemonGB_Online_Trades_and_Battles](https://github.com/Lorenzooone/PokemonGB_Online_Trades_and_Battles)
message for message).
`dotnet run --project host/tests -c Release -- --web-vectors` regenerates
`web/tests/trade-vectors.json` and `web/js/trade/pk3-data.js` from that host.

`session-test.mjs` runs the page's session code against a stand-in board that relays the
Switch's game (`tests/fake-port.mjs`, with the leader's side in `tests/fake-switch.mjs`):
one trade, two trades in a row, leaving without trading, changing which Pokémon is
offered while connected, and leaving the menu and sitting down again. With the stand-in
pool it runs a pool trade, the different Pokémon that follows a trade or a return to the
table, a Pokémon taken from the pool only once the Switch has let the page in, an offer
held back until the page makes it, an empty pool, mail travelling both ways, a Pokémon the pool refuses, and a swap the pool never confirms. It goes through the same `EspDevice` and framing the page uses.
That stand-in board also runs in the browser, so the page itself can be driven without
hardware:

```js
const { FakeBoardPort, installFakeSerial } = await import('/tests/fake-port.mjs');
const party = (await (await fetch('/tests/trade-vectors.json')).json()).engine.hostParty
    .map((hex) => Uint8Array.from(hex.match(/../g), (b) => parseInt(b, 16)));
const port = new FakeBoardPort({ hostParty: party });
port.onLinkUp = (leader) => leader.greet().sit().open(party);
installFakeSerial(port);
```

then use the page as usual: connect in step 1, then trade under *Just the Switch*.
`port.leader` takes the Switch's next moves, such as `command(0xdddd, 0)` to choose its
first Pokémon (the values are `LINK` in `js/trade/engine.js`), and `port.leaveRoom()`
ends the visit. The stand-in keeps its pace in a tab that is not on show, which the page
it drives does not.

## Third-party code

- `vendor/esptool-js` – [esptool-js](https://github.com/espressif/esptool-js) 0.6.1,
  Apache-2.0
- `vendor/picoflash` – [picoflash](https://github.com/picoflash/picoflash), MIT,
  © Piers Finlayson
