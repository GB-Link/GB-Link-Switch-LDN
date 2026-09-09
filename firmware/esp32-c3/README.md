# ESP32-C3 USB LDN firmware

A standalone, experimental ESP32-C3 project modelled on the C6's LDN protocol implementation and pinned to
ESP-IDF v6.1. Default 4 MB flash, DIO, 80 MHz; it talks to the host through the C3's native USB Serial/JTAG.
This version does not use UART0: the devkit's USB connector must be wired to the C3's native USB,
not to a USB-to-UART chip.

## Differences from the C6

- `main/c3_transport.c` sends and receives binary frames with the USB Serial/JTAG driver. When the host
  sets a baud rate the device only acknowledges; the physical USB rate is unaffected.
- `main/ldn_control.c` implements authentication frame I/O, status queries and USB command handling on its own.
- The C6's raw-frame relocation patch, hardware descriptor offsets and transmit trace hooks are not included.
- The WPA callbacks and the CCMP key interface target the pinned C3 driver, and the build checks the SHA-256
  of the C3 wireless library. That driver's pairwise-key getter reports "not supported"; the group key is
  verified by reading it back, and the pairwise key by the actual encrypted authentication response.
- `LDN_HELLO` resets the protocol session id, so the host can reconnect right after closing the port.
- FreeRTOS runs a 1 ms tick so the main loop's 2 ms delay is effective and does not busy-loop.

The matching host initializes the Pia receive sequence from the peer's advertised send-window base,
which supports a non-fixed starting sequence after rejoining a room.

## Build and flash

Run from the repository root, replacing the example port with the actual C3 port:

```powershell
.\firmware\esp32-c3\tools\probe.ps1 -Action identify -Port COM5
.\firmware\esp32-c3\tools\probe.ps1 -Action build
.\firmware\esp32-c3\tools\probe.ps1 -Action flash -Port COM5
```

The script reuses `firmware/tools/environment.ps1` to activate the SDK; the sources and the build directory
are in this project. Close the host or any serial monitor holding the port before flashing.
The existing C# host can select the native USB COM port directly.

With other flashing tools select ESP32-C3, DIO, 80 MHz, 4 MB; the files are in `build/`:

| Address | File |
| --- | --- |
| `0x0` | `bootloader/bootloader.bin` |
| `0x8000` | `partition_table/partition-table.bin` |
| `0x10000` | `ldn_c3_bridge.bin` |

## Hardware diagnostics

Requires the .NET 10 SDK. The diagnostic reuses the C# protocol library and stops the device session and
releases the port when it exits.

```powershell
.\firmware\esp32-c3\tools\diagnose.ps1 -Mode serial -Port COM5
.\firmware\esp32-c3\tools\diagnose.ps1 -Mode auth -Port COM5
.\firmware\esp32-c3\tools\diagnose.ps1 -Mode pia -Port COM5
.\firmware\esp32-c3\tools\diagnose.ps1 -Mode room -Port COM5
```

- `serial`: protocol handshake, 50 channel changes, invalid-command rejection, recovery and stop.
- `auth`: additionally scans for a FireRed Leader room and verifies the advertisement, association,
  key installation, authentication response, challenge, member list and UDP interface setup.
- `pia`: additionally runs a short encrypted Pia exchange, verifying the packet counts in both directions
  and the decryption, then disconnects; this is not a complete trade test.
- `room`: runs for up to 3 minutes, checks real room entry, and records the reliable-transport sequence
  numbers and device state; operate the Switch alongside it to test a trade. Captures and received
  Pokémon are saved in `local/esp32-c3/runs/`.

`auth`, `pia` and `room` require the Switch to be hosting a Leader room and valid keys in the current user's
`.switch/prod.keys`. Keys are read at run time and never embedded in the sources or the firmware.
Board backups, room information and debug logs belong in the repository's `local/esp32-c3/`.

Verified on hardware: flashing, native USB communication, repeated reconnection, wireless association,
LDN authentication, real room entry and one complete Pokémon trade. That run had no Pia decryption
failures and no corrupt serial frames; long-term stability still needs continued testing.
