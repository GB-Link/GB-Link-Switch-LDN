#!/bin/bash
# Linux counterpart of probe.ps1. Usage: probe.sh build | flash <port> | identify <port> | monitor <port>
set -euo pipefail
ACTION=${1:-build}; PORT=${2:-}
SDK=${SDK_PATH:-$HOME/esp/esp-idf-v6.1}
REV=fff9895c82d744c7237be8847347bdd1b07c6643
[ -f "$SDK/export.sh" ] || { echo "ESP-IDF is missing at $SDK (install the pinned v6.1 release there)"; exit 1; }
actual=$(git -C "$SDK" rev-parse HEAD)
[ "$actual" = "$REV" ] || { echo "This project requires ESP-IDF $REV; found $actual"; exit 1; }
case $ACTION in
    build) ;;
    flash|identify|monitor) [ -n "$PORT" ] || { echo "Specify the ESP32 port, for example /dev/ttyUSB0"; exit 1; } ;;
    *) echo "Usage: $0 build | flash <port> | identify <port> | monitor <port>"; exit 1 ;;
esac
PROJECT=$(cd "$(dirname "$0")/.." && pwd); BUILD=$PROJECT/build
set +eu; . "$SDK/export.sh" >/dev/null; set -eu
if [ "$ACTION" = identify ]; then exec esptool --chip esp32 --port "$PORT" flash-id; fi
IDF=(idf.py --ccache -C "$PROJECT" -B "$BUILD" -DIDF_TARGET=esp32
     -DSDKCONFIG="$BUILD/sdkconfig" -DSDKCONFIG_DEFAULTS="$PROJECT/sdkconfig.defaults")
[ "$ACTION" = monitor ] || "${IDF[@]}" build
case $ACTION in
    flash) "${IDF[@]}" -p "$PORT" -b 460800 flash ;;   # esptool refuses to write an ESP32 image to another chip
    monitor) "${IDF[@]}" -p "$PORT" monitor ;;
esac
echo "ESP32 firmware: $BUILD/ldn_esp32_bridge.bin"
