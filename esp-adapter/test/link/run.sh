#!/bin/bash
# Builds and runs the host tests of the one-board link: the adapter on this chip's pins
# (firmware/main/pico_link_local.c) and the choice of a GB-Link on the UART
# (firmware/main/pico_link_select.c).
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
OUT="$HERE/../out"
mkdir -p "$OUT"
FLAGS=(-std=gnu17 -Wall -Wextra -I "$HERE/stubs" -I "$HERE/../../../firmware/components/rfu_adapter/include"
       -I "$HERE/../../../firmware/common")
cc "${FLAGS[@]}" "$HERE/link_test.c" -o "$OUT/link_test"
cc "${FLAGS[@]}" "$HERE/select_test.c" -o "$OUT/select_test"
cc "${FLAGS[@]}" -DUART_ON_ADAPTER_PINS "$HERE/select_test.c" -o "$OUT/select_test_adapter_pins"
"$OUT/link_test"
"$OUT/select_test"
"$OUT/select_test_adapter_pins"
