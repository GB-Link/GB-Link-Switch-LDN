#!/bin/bash
# Build, flash or monitor the wireless adapter alone (on the UART link a GB-Link uses) for
# one chip with the pinned ESP-IDF. The bridge itself is built with firmware/tools/build.sh.
# Usage: build.sh <esp32|esp32s3|esp32c6> [build | size | flash <port> | monitor <port>]
set -euo pipefail
APP=adapter; CHIP=${1:-}; ACTION=${2:-build}; PORT=${3:-}
SDK=${SDK_PATH:-$HOME/esp/esp-idf-v6.1}
REV=fff9895c82d744c7237be8847347bdd1b07c6643
ROOT=$(cd "$(dirname "$0")" && pwd)
USAGE="Usage: $0 <esp32|esp32s3|esp32c6> [build | size | flash <port> | monitor <port>]"
case $CHIP in
    esp32|esp32s3|esp32c6) ;;
    *) echo "$USAGE"; exit 1 ;;
esac
[ -f "$SDK/export.sh" ] || { echo "ESP-IDF is missing at $SDK (install the pinned v6.1 release there, or set SDK_PATH)"; exit 1; }
actual=$(git -C "$SDK" rev-parse HEAD)
[ "$actual" = "$REV" ] || { echo "This firmware requires ESP-IDF $REV; found $actual"; exit 1; }
case $ACTION in
    build|size) ;;
    flash|monitor) [ -n "$PORT" ] || { echo "Specify the serial port, for example /dev/ttyACM0"; exit 1; } ;;
    *) echo "Unknown action: $ACTION"; exit 1 ;;
esac
set +eu; . "$SDK/export.sh" >/dev/null; set -eu
PROJECT="$ROOT/$APP"
BUILD="$ROOT/build-$APP-$CHIP"
CONFIG="$BUILD/sdkconfig"
for defaults in "$PROJECT/sdkconfig.defaults" "$PROJECT/sdkconfig.defaults.$CHIP"; do
    [ "$defaults" -nt "$CONFIG" ] && rm -f "$CONFIG"
done
IDF=(idf.py -C "$PROJECT" -B "$BUILD" -DIDF_TARGET="$CHIP" -DSDKCONFIG="$CONFIG")
case $ACTION in
    build) "${IDF[@]}" build ;;
    size) "${IDF[@]}" size ;;
    flash) "${IDF[@]}" -p "$PORT" -b 460800 flash ;;
    monitor) "${IDF[@]}" -p "$PORT" monitor ;;
esac
