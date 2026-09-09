# Full flash images

This directory holds complete images (bootloader, partition table and application) for first-time flashing.
Every full BIN file is written at `0x0`.

| File | Chip | Flash | Mode / frequency | Link to the host |
| --- | --- | --- | --- | --- |
| `frlg-trade-esp32c6-uart-16mb-full.bin` | ESP32-C6 | 16 MB | DIO / 80 MHz | UART (USB-to-UART bridge) |
| `frlg-trade-esp32c3-usb-4mb-full.bin` | ESP32-C3 | 4 MB | DIO / 80 MHz | Native USB Serial/JTAG |
| `frlg-trade-esp32s3-usb-8mb-full.bin` | ESP32-S3 | 8 MB | DIO / 80 MHz | Native USB Serial/JTAG |
| `frlg-trade-esp32-uart-4mb-full.bin` | ESP32 (original) | 4 MB | DIO / 40 MHz | UART (USB-to-UART bridge) |

Choose by chip, flash size and interface; do not mix them up. Each image's partitions end below 1.1 MB,
so a board with a smaller flash than listed also works: pass `--flash-size detect` to esptool (or the
board's real size) so that the image header is rewritten to match. esptool refuses to write an image
built for a different chip.

Verify the files with `sha256sum -c SHA256SUMS.txt` (Linux/macOS) or
`Get-FileHash <file> -Algorithm SHA256` (Windows). Close the host and any serial monitor holding the port
before flashing, then run the host; it must keep running during the trade. Running the host and
configuring keys are described in the [project README](../../README.md).

## ESP32-C6

| Item | Value |
| --- | --- |
| Chip | ESP32-C6 |
| Flash | 16 MB, DIO, 80 MHz |
| Link | UART, `serial` full trade bridge mode |
| Write address | `0x0` |
| Build date | 2026-09-08 |
| Source | `7f992ba` |

Use the board's USB-to-UART port; the native USB Serial/JTAG port cannot replace this firmware's UART link.
In a graphical flasher select ESP32-C6, add only this file at `0x0`, DIO, 80 MHz, 16 MB. With esptool:

```bash
esptool --chip esp32c6 --port /dev/ttyUSB0 --baud 460800 write-flash --flash-mode dio --flash-freq 80m --flash-size detect 0x0 frlg-trade-esp32c6-uart-16mb-full.bin
```

Regenerate from the repository root in a configured ESP-IDF environment:

```powershell
.\firmware\esp32-c6\tools\probe.ps1 -Action build
python -m esptool --chip esp32c6 merge-bin --output firmware/release/frlg-trade-esp32c6-uart-16mb-full.bin --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 firmware/esp32-c6/build-c6-serial-uart16-1/bootloader/bootloader.bin 0x8000 firmware/esp32-c6/build-c6-serial-uart16-1/partition_table/partition-table.bin 0x10000 firmware/esp32-c6/build-c6-serial-uart16-1/ldn_wifi_probe.bin
```

The merged file is only padded to the end of the application; "16 MB" is the target flash configuration.
This release passed the build, the private wireless link audit and the merged-data check; it was not
flashed to a device or given a new on-board trade verification in that release.

## ESP32-C3

`frlg-trade-esp32c3-usb-4mb-full.bin`: 4 MB flash, DIO, 80 MHz, written at `0x0`. Build date 2026-09-08,
source `7f992ba`. The host must use the USB port connected to the C3's native USB Serial/JTAG, not a
USB-to-UART bridge; see the [C3 project notes](../esp32-c3/README.md).

```bash
esptool --chip esp32c3 --port /dev/ttyACM0 --baud 460800 write-flash --flash-mode dio --flash-freq 80m --flash-size detect 0x0 frlg-trade-esp32c3-usb-4mb-full.bin
```

Regenerate:

```powershell
.\firmware\esp32-c3\tools\probe.ps1 -Action build
python -m esptool --chip esp32c3 merge-bin --output firmware/release/frlg-trade-esp32c3-usb-4mb-full.bin --flash-mode dio --flash-freq 80m --flash-size 4MB 0x0 firmware/esp32-c3/build/bootloader/bootloader.bin 0x8000 firmware/esp32-c3/build/partition_table/partition-table.bin 0x10000 firmware/esp32-c3/build/ldn_c3_bridge.bin
```

That release passed the build and merged-data check without a new on-board trade verification;
the existing hardware verification record is in the C3 project notes.

## ESP32-S3

`frlg-trade-esp32s3-usb-8mb-full.bin`: 8 MB flash, DIO, 80 MHz, written at `0x0`. Built 2026-09-08 from the
`firmware/esp32-s3` sources of this fork. The host uses the S3's native USB Serial/JTAG port; see the
[S3 project notes](../esp32-s3/README.md).

```bash
esptool --chip esp32s3 --port /dev/ttyACM0 --baud 460800 write-flash --flash-mode dio --flash-freq 80m --flash-size detect 0x0 frlg-trade-esp32s3-usb-8mb-full.bin
```

Regenerate:

```bash
firmware/esp32-s3/tools/probe.sh build
esptool --chip esp32s3 merge-bin --output firmware/release/frlg-trade-esp32s3-usb-8mb-full.bin --flash-mode dio --flash-freq 80m --flash-size 8MB 0x0 firmware/esp32-s3/build/bootloader/bootloader.bin 0x8000 firmware/esp32-s3/build/partition_table/partition-table.bin 0x10000 firmware/esp32-s3/build/ldn_s3_bridge.bin
```

This image is the build that was flashed to an M5Stack AtomS3 and verified with the staged diagnostics
and a complete trade against a Switch.

## ESP32 (original)

`frlg-trade-esp32-uart-4mb-full.bin`: 4 MB flash, DIO, 40 MHz. The original ESP32's bootloader lives at
`0x1000`, so the merged image starts with 4 KiB of padding and is still written at `0x0`. Built 2026-09-08
from the `firmware/esp32` sources of this fork. The host uses the board's USB-to-UART bridge (CP2102 on the
common devkits); see the [ESP32 project notes](../esp32/README.md).

```bash
esptool --chip esp32 --port /dev/ttyUSB0 --baud 460800 write-flash --flash-mode dio --flash-freq 40m --flash-size detect 0x0 frlg-trade-esp32-uart-4mb-full.bin
```

Regenerate:

```bash
firmware/esp32/tools/probe.sh build
esptool --chip esp32 merge-bin --output firmware/release/frlg-trade-esp32-uart-4mb-full.bin --flash-mode dio --flash-freq 40m --flash-size 4MB 0x1000 firmware/esp32/build/bootloader/bootloader.bin 0x8000 firmware/esp32/build/partition_table/partition-table.bin 0x10000 firmware/esp32/build/ldn_esp32_bridge.bin
```

This image is the build that was flashed to an ESP32-D0WD-V3 devkit and verified with the staged
diagnostics and complete trades against a Switch.

## Updating the checksums

From the repository root, keeping one line per image:

```bash
(cd firmware/release && for f in $(ls *.bin | sort); do printf '%s  %s\n' "$(sha256sum "$f" | cut -c1-64)" "$f"; done > SHA256SUMS.txt)
```

```powershell
Get-ChildItem firmware/release -Filter '*.bin' | Sort-Object Name | ForEach-Object {
    $firmwareHash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    "$firmwareHash  $($_.Name)"
} | Set-Content firmware/release/SHA256SUMS.txt -Encoding ascii
```
