#!/bin/bash
# Linux counterpart of diagnose.ps1. Usage: diagnose.sh <serial|auth|pia|room> <port>
set -euo pipefail
MODE=${1:-serial}; PORT=${2:-}
case $MODE in serial|auth|pia|room) ;; *) echo "Usage: $0 <serial|auth|pia|room> <port>"; exit 1 ;; esac
[ -n "$PORT" ] || { echo "Specify the ESP32 port, for example /dev/ttyUSB0"; exit 1; }
HERE=$(cd "$(dirname "$0")" && pwd)
dotnet build "$HERE/host-diagnostic/Esp32.Diagnostic.csproj" -c Release --nologo
dotnet "$HERE/host-diagnostic/bin/Release/net10.0/Esp32.Diagnostic.dll" "$MODE" "$PORT"
