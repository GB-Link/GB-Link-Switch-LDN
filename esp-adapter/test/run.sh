#!/bin/bash
# Builds and runs the host simulation: librfu's SIO layer from a pokefirered checkout
# against the adapter's logic on a simulated wire.
# Usage: run.sh [pokefirered checkout] [case name filter] [-v]
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
DECOMP=${1:-$HOME/github/pokefirered}
shift || true
[ -f "$DECOMP/src/librfu_intr.c" ] || { echo "No pokefirered checkout at $DECOMP"; exit 1; }
OUT="$HERE/out"
python3 "$HERE/patch_librfu.py" "$DECOMP" "$OUT"
COMPONENT="$HERE/../../firmware/components/rfu_adapter"
CFLAGS=(-O1 -g -Wall -Wno-unused -I "$HERE/sim" -I "$OUT" -I "$COMPONENT/include")
for f in librfu_stwi librfu_intr librfu_sio32id; do
    gcc -std=gnu17 "${CFLAGS[@]}" -Wno-maybe-uninitialized -c "$OUT/$f.c" -o "$OUT/$f.o"
done
gcc -std=gnu17 "${CFLAGS[@]}" -c "$HERE/sim/scenario.c" -o "$OUT/scenario.o"
gcc -std=gnu17 "${CFLAGS[@]}" -c "$HERE/sim/test_main.c" -o "$OUT/test_main.o"
g++ -std=gnu++20 -O1 -g -Wall -Wno-volatile -Wno-maybe-uninitialized -I "$HERE/sim" -I "$COMPONENT/include" \
    -I "$COMPONENT" -c "$COMPONENT/wa_adapter.cpp" -o "$OUT/wa_adapter.o"
g++ -std=gnu++20 -O1 -g -Wall -I "$HERE/sim" -I "$COMPONENT/include" -c "$HERE/sim/sim.cpp" -o "$OUT/sim.o"
g++ -o "$OUT/adapter_sim" "$OUT"/*.o
"$OUT/adapter_sim" "$@"
