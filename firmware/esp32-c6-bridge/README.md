# ESP32-C6 LDN bridge (Seeed XIAO ESP32C6)

The standalone bridge on a XIAO ESP32C6: it joins the Switch's FireRed room over LDN and
speaks to the Pico's wireless-adapter mode over a two-wire UART, with no PC in the path.

The firmware itself lives in `firmware/bridge-common/` and is shared with
`firmware/esp32-s3/`; this directory holds only what is specific to this board — the
target and pinned Wi-Fi driver hash, the sdkconfig, and the antenna switch setup in
`main/xiao_c6_board.c`. A fix to the bridge lands on both boards at once.

## Wiring

| XIAO C6 | Pico | direction |
| --- | --- | --- |
| D1 (GPIO1) | GP8 | Pico TX -> C6 RX |
| D2 (GPIO2) | GP9 | C6 TX -> Pico RX |
| GND | GND | |

921600 baud. The orientation is detected at runtime, so a swapped pair costs nothing.

## Build and flash

```
. $IDF_PATH/export.sh
idf.py -p /dev/serial/by-id/<xiao> flash
```

ESP-IDF v6.1 at the pinned commit, as with the other targets: the LDN join drives a
private WPA interface whose ABI is not stable across builds.

## Keys

The four LDN keys live in this board's NVS and do not travel with the firmware, so a new
board needs them once, over its console:

```
LDN_KEY aes_kek_generation_source <32 hex>
LDN_KEY aes_key_generation_source <32 hex>
LDN_KEY master_key_00 <32 hex>
LDN_KEY master_key_12 <32 hex>
LDN_KEYS
```

The values come from `prod.keys`. `LDN_KEYS` reports which slots are filled; it never
echoes a key back.
