# ESP32-C6 UART LDN firmware

A standalone ESP32-C6 project pinned to ESP-IDF v6.1 that talks to the host over UART.
The sources are in `main/`; the C6 build, flash, raw-frame adapter and link-audit tools are in `tools/`.
SDK installation and environment activation are shared through `../tools/setup.ps1` and `../tools/environment.ps1`.

## Build and flash

Run from the repository root:

```powershell
.\firmware\tools\setup.ps1
.\firmware\esp32-c6\tools\probe.ps1 -Action doctor
.\firmware\esp32-c6\tools\probe.ps1 -Action build
.\firmware\esp32-c6\tools\probe.ps1 -Action flash -Port COM6
```

Replace the port with the actual device port. The default configuration is C6, UART, 16 MB flash,
`serial` mode; the flash size can be set with `-FlashSize 4MB`, `8MB` or `16MB`.
Close the host connection and any serial monitor holding the port before flashing.

Build directories live inside this project and are named `build-c6-<mode>-<interface and size>-<channel>/`.
The default configuration's application image is `build-c6-serial-uart16-1/ldn_wifi_probe.bin`;
the complete list of files to flash and their addresses is in `flash_args` in the same directory.

## Status LED

The on-board WS2812 RGB LED (GPIO8) breathes to show the current state; color and speed follow the state:

| Color | State |
| --- | --- |
| White | Booting |
| Blue (slow) | Standby, waiting for the host to send a room configuration |
| Amber (fast) | Joining a room |
| Green | Room authenticated, ready to trade |
| Red (fast) | Join failed (association timeout or key verification failed); returns to standby after about 3 seconds |

The LED driver is the official `espressif/led_strip` component, fetched automatically on the first build.
`LDN_PROBE_STATUS_LED` / `LDN_PROBE_STATUS_LED_GPIO` can be disabled or changed in menuconfig;
if the LED fails to initialize, the firmware skips it and keeps working.

## Communication and debugging

The complete trade bridge uses `-Mode serial -Console uart` and connects to the PC through the on-board
USB-to-UART bridge or an external 3.3 V USB serial adapter. With an external adapter, cross TX/RX and
share ground. The C6's native USB Serial/JTAG configuration is only for wireless diagnostics.
The `public`, `discovery` and `send` modes do not provide the trade bridge.

This project contains a raw-frame transmit adapter and hardware transmit tracing tied to the pinned C6 driver.
The build script checks the SDK version, the driver library hashes and the final link result; these
implementations must not be reused directly on other chips. Host usage is described in the
[project README](../../README.md); the protocol is in the [serial protocol document](../../docs/SERIAL_PROTOCOL.md).
