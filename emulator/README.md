# Emulator

The Game Boy Advance emulator the web page plays games in: [mGBA](https://mgba.io)'s Game
Boy Advance core, built for the browser. Its link port has a Wireless Adapter plugged in
for FireRed, LeafGreen and Emerald, whose wireless the page carries to the ESP32 board,
or a link cable for Ruby and Sapphire, whose other end is the page. The adapter is the
same one the ESP32 adapter firmware runs (`esp-adapter/components/rfu_adapter`).

The page loads the build in `web/emulator/`. Build it again after changing anything here:

```
emulator/build.sh
```

It needs podman. The first build downloads mGBA 0.10.5 and builds it in
`~/.cache/gblink-emu`.

## Licence

mGBA is MPL-2.0, see `LICENSE-mGBA.txt`. The rest is under the repository's licence.
