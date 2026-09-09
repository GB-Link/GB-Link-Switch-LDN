# ESP32 UART LDN bridge

Experimental port of the [ESP32-S3 project](../esp32-s3/README.md) to the original ESP32, pinned to
the same ESP-IDF v6.1 commit. Default 4 MB flash, DIO, 40 MHz. The original ESP32 has no native USB,
so the host talks to UART0 through the board's USB-to-UART bridge (CP2102 on the common devkits),
the same arrangement the validated C6 firmware uses: 115200 baud after boot, 921600 once the host
sends `LDN_BAUD`.

## Relation to the S3 project

- `main/esp32_transport.c` replaces the USB Serial/JTAG transport with the console UART driver
  (32 KiB RX and 4 KiB TX buffers) and performs the baud switch after the acknowledgement is flushed.
- `main/ldn_control.c` applies a pending baud change after the `LDN_BAUD` request's DONE frame, as the C6 does.
- Everything else, including the private WPA callback table, key install and the diagnostic-only key readback,
  is the S3 code. `main/CMakeLists.txt` pins the SHA-256 of the v6.1 `esp32/libnet80211.a`.
- Status lines are `LDN_ESP32_READY` and `LDN_ESP32_STATS`; `LDN_HELLO` reports the model as `esp32`.

## Build and flash

With the pinned SDK at `~/esp/esp-idf-v6.1` (run `./install.sh esp32` there once):

```bash
firmware/esp32/tools/probe.sh identify /dev/ttyUSB0
firmware/esp32/tools/probe.sh build
firmware/esp32/tools/probe.sh flash /dev/ttyUSB0
```

`tools/probe.ps1` does the same on Windows with `-Action identify|build|flash -Port COMx`.
For other flashing tools select ESP32, DIO, 40 MHz, 4 MB and write from `build/`:

| Address | File |
| --- | --- |
| `0x1000` | `bootloader/bootloader.bin` |
| `0x8000` | `partition_table/partition-table.bin` |
| `0x10000` | `ldn_esp32_bridge.bin` |

## Hardware diagnostics

Same staged diagnostic as the S3 project (`serial`, `auth`, `pia`, `room`), requiring the .NET 10 SDK;
`auth`, `pia` and `room` need a Switch hosting a Leader room and keys in `~/.switch/prod.keys`.

```bash
firmware/esp32/tools/diagnose.sh serial /dev/ttyUSB0
```

Captures and received Pokémon from `pia` and `room` are written to `local/esp32/runs/`.

## Status

Verified on an ESP32-D0WD-V3 devkit (CP2102, 4 MB flash) against a Switch FireRed Leader room
(2026-09-08): serial protocol including the 921600 baud switch, wireless association, key install
(this driver reads the group key back; the pairwise getter is unsupported), LDN authentication with
challenge, room membership, UDP setup, and bidirectional encrypted Pia traffic (`auth` and `pia` stages;
78 packets received, 68 sent, no decrypt failures), and one complete trade through the console runner
(`host/tests --live`): room entry, party exchange, selection, confirmation, commit and clean room exit,
with 5451 Pia packets received and 5276 sent, no decrypt failures and no bad serial frames.
