#!/bin/bash
# Builds and runs the host test of the one-board link (bridge/main/pico_link_local.c).
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
OUT="$HERE/../out"
mkdir -p "$OUT"
cc -std=gnu17 -Wall -Wextra -I "$HERE/stubs" -I "$HERE/../../components/rfu_adapter/include" \
    -I "$HERE/../../../firmware/common" "$HERE/link_test.c" -o "$OUT/link_test"
"$OUT/link_test"
