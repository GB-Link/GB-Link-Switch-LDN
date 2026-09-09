# ESP32-S3 USB LDN bridge

Experimental ESP32-S3 port of the [ESP32-C3 project](../esp32-c3/README.md), pinned to the same
ESP-IDF v6.1 commit. Default 8 MB flash, DIO, 80 MHz. The host talks to the chip's native USB
Serial/JTAG port, so the board's USB connector must go to the S3 itself, not to a USB-to-UART bridge.
Developed on an M5Stack AtomS3 (ESP32-S3, 8 MB embedded flash, USB-C on the native USB port).

## Relation to the C3 project

The source is the C3 firmware with the target changed:

- `main/s3_transport.c` is the C3 USB Serial/JTAG transport; the S3 has the same peripheral and driver.
- `main/ldn_probe.c` keeps the C3 join sequence: private WPA callback table, pre-derived CCMP key install,
  then `esp_wifi_auth_done_internal()`. Key readback is diagnostic only on the S3: the pinned driver accepts
  both keys but its pairwise getter is unsupported and its group getter answers without the key, so the port is
  authorized once both installs succeed and the encrypted LDN authentication exchange verifies the keys
  (`LDN_KEY_STATUS` reports the install and readback results). The private ABI in `main/ldn_private_wifi.h`
  is the ESP-IDF v6.1 supplicant interface and is shared by all chips in that release, but the closed
  S3 driver has not been audited the way the C6 and C3 drivers were.
- `main/CMakeLists.txt` pins the SHA-256 of the v6.1 `esp32s3/libnet80211.a` this port was built against.
- Status lines are `LDN_S3_READY` and `LDN_S3_STATS`; `LDN_HELLO` reports the model as `esp32s3`.
  The trade host only displays the model, so it works unchanged.

## Build and flash

Install the pinned SDK once (Windows: `firmware/tools/setup.ps1`; Linux: extract the verified
`esp-idf-v6.1.zip` release archive to `~/esp/esp-idf-v6.1` and run `./install.sh esp32s3`). Then:

```bash
firmware/esp32-s3/tools/probe.sh identify /dev/ttyACM0
firmware/esp32-s3/tools/probe.sh build
firmware/esp32-s3/tools/probe.sh flash /dev/ttyACM0
```

`tools/probe.ps1` does the same on Windows with `-Action identify|build|flash -Port COMx`.
Close any program holding the port before flashing. The build directory is `build/`; for other flashing
tools select ESP32-S3, DIO, 80 MHz, 8 MB and write:

| Address | File |
| --- | --- |
| `0x0` | `bootloader/bootloader.bin` |
| `0x8000` | `partition_table/partition-table.bin` |
| `0x10000` | `ldn_s3_bridge.bin` |

## Hardware diagnostics

Requires the .NET 10 SDK; the diagnostic reuses the host protocol library and stops the device session on exit.

```bash
firmware/esp32-s3/tools/diagnose.sh serial /dev/ttyACM0
firmware/esp32-s3/tools/diagnose.sh auth /dev/ttyACM0
firmware/esp32-s3/tools/diagnose.sh pia /dev/ttyACM0
firmware/esp32-s3/tools/diagnose.sh room /dev/ttyACM0
```

- `serial`: protocol handshake, 50 channel changes, malformed-command rejection, recovery and stop.
- `auth`: also scans for a FireRed Leader room and verifies the advertisement, association, key install,
  authentication response, challenge, member list and UDP interface setup.
- `pia`: also exchanges a short burst of encrypted Pia traffic in both directions, then disconnects.
- `room`: runs for up to 3 minutes and checks actual room entry; use it while operating the Switch.
  Captures and received Pokémon are written to `local/esp32-s3/runs/`.

`auth`, `pia` and `room` need a Switch hosting a Leader room and valid keys in `~/.switch/prod.keys`.

## Status

Verified on an AtomS3 against a Switch FireRed Leader room (2026-09-08): serial protocol, wireless
association, key install, LDN authentication with challenge, room membership, UDP setup, and bidirectional
encrypted Pia traffic (`auth` and `pia` stages), and one complete trade through the console runner
(`host/tests --live`): room entry, party exchange, selection, confirmation, commit and clean room exit,
with 6553 Pia packets received and 6383 sent, no decrypt failures and no bad serial frames.
