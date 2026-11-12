# Web client

The page at <https://switch.gblink.io>. It installs the firmware on both boards, stores
the Switch keys on the ESP32, and then either carries the link between the boards, plays
a game in its own emulator, trades with the Switch by itself, or sends it Mystery Gifts.
It is a plain static site with no build step and no server.

## Modes

Pick one at the top of the page, or open `#gba`, `#emu`, `#switch`, `#celio` or `#gift`
directly.

**GBA to Switch** (ESP32 board + GB-Link adapter)

1. **ESP32 board:** install the firmware and drop your `prod.keys` on the page.
2. **GB-Link adapter:** install the wireless adapter firmware.
3. **Play:** wire the boards together, or leave both on USB and let the page pass the
   traffic between them. For Ruby and Sapphire, pick them here: the page links the game
   by cable (the adapter's mode `0x01`) and translates that link into the Switch's
   wireless, so both boards must be on USB. For Emerald and Ruby/Sapphire the page's
   National Dex bypass, when turned on, lets trades go ahead whatever either game's
   progress.

**Play in your browser** (ESP32 board only)

1. **ESP32 board:** the same as above.
2. **Game:** drop the ROM of Ruby, Sapphire, Emerald, FireRed or LeafGreen, with its
   `.sav` if you have one. Only clean dumps of the cartridges are taken, in any language
   and revision. The game and its save are kept in the browser; *Save file* downloads the
   save.
3. **Link with the Switch:** press *Start*. FireRed, LeafGreen and Emerald play with a
   Wireless Adapter plugged in, which the page carries to the board as it would a
   GB-Link's. Ruby and Sapphire play on a link cable whose other end is the page, which
   translates the link into the Switch's wireless as for a real GBA.

**PC to Switch** (ESP32 board only)

1. **ESP32 board:** the same as above.
2. **Trade:** *Wonder Trade* with the pool at <https://pokemon.gblink.io>, or *PK3
   files* from a party kept in your browser. `assets/party.json` is the party a first
   visit starts with.

The page never offers a Pokémon on its own. The Switch never tells the other side what
its player picked, so the page waits for you to choose or press *Accept trade*.

**Mystery Gift** (ESP32 board only)

1. **ESP32 board:** the same as above, with firmware 2.1.0 or later.
2. **Mystery Gift:** pick a Wonder Card, or drop a `.wc3` file of your own on the card,
   and press *Start*. The page leads a group as a FireRed sharing a card; on the Switch,
   MYSTERY GIFT, WONDER CARDS, FRIEND, then GBLINK. The card for the next Switch can
   change while the group is open.

A Switch that already has the card is sent it again only if you say so on the page. The
group stays open for one Switch after another; after a link the page had to end itself,
the board restarts and the group opens again.

Keep the tab visible while playing. Browsers slow down hidden tabs, and the game drops a
partner that stops answering.

The keys stay on your computer and the board. The page reads the four values it needs
from `prod.keys`, sends them to the board over USB, and does not keep or upload them.
The board never sends them back.

## Running it locally

USB access only works over `https://` or from `localhost`:

```
python3 -m http.server -d web 8000
```

Then open <http://localhost:8000> in Chrome or Edge on a computer.

| Browser | ESP32 board | GB-Link adapter |
| --- | --- | --- |
| Chrome, Edge (desktop) | yes | yes |
| Firefox 151 and later | yes | serial only; install its firmware by hand |
| Safari, phones | no | no |

On Linux:

- Add yourself to the serial group (`dialout`, or `uucp` on Arch).
- The adapter needs udev rules for WebUSB. Run `scripts/setup-linux-permissions.sh`
  from the GB-Link firmware repository.
- If Chrome says the port was lost right after you used `esptool.py` or `idf.py`,
  unplug the board and plug it back in.

## Firmware images

`firmware/` holds the images the page installs, listed in `firmware/manifest.json`.
Refresh them after a build with:

```
firmware/tools/package_web.py
```

## Code

| File | |
| --- | --- |
| `js/app.js` | the page |
| `js/esp.js`, `js/wire.js` | talking to the ESP32 firmware |
| `js/gblink.js` | talking to the GB-Link adapter |
| `js/bridge.js` | passing traffic between the two boards |
| `js/flash-esp.js`, `js/flash-pico.js` | installing firmware |
| `js/keys.js` | reading `prod.keys` |
| `js/trade/` | trading without a GBA |
| `js/cable/` | Ruby and Sapphire: the adapter's cable mode and the translation to the Switch's wireless (`translator.js` is MPL-2.0, ported from [mgba_LDN](https://github.com/Gr3nSkyDragon/mgba_LDN)) |
| `js/emu/` | The page's emulator: the game and its screen, sound and keys, the games kept in the browser, its wireless adapter (`wireless.js`) and its link cable (`cable.js`); the emulator itself is built into `emulator/` from [`../emulator`](../emulator/README.md) |
| `js/gift/` | Mystery Gift: the group the Switch joins as a friend, the gift exchange (from [gblink-wondercards](https://github.com/GB-Link/gblink-wondercards)) and the cards; `team.js` is written by `cards/build.mjs`, `wc3.js` reads `.wc3` files |

## Third-party code

- `vendor/esptool-js`: [esptool-js](https://github.com/espressif/esptool-js) 0.6.1,
  Apache-2.0
- `vendor/picoflash`: [picoflash](https://github.com/picoflash/picoflash), MIT,
  © Piers Finlayson
- `emulator`: [mGBA](https://mgba.io) 0.10.5, MPL-2.0 (`emulator/LICENSE-mGBA.txt`)
