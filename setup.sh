#!/bin/sh
# Builds app/FRLG_Trade_Center-x86_64.AppImage (self-contained). Needs the .NET 10 SDK, curl and network access.
set -eu
root=$(cd "$(dirname "$0")" && pwd)
desktop="$root/host/desktop"
project="$desktop/Frlg.Trade.Desktop.csproj"
appdir="$desktop/obj/appimage/AppDir"
tools="$root/local/tools"
output="$root/app/FRLG_Trade_Center-x86_64.AppImage"

# appimagetool and the AppImage runtime, pinned by SHA-256.
fetch() {
    [ -f "$tools/$1" ] || curl --fail --location --silent --show-error --output "$tools/$1" "$2"
    echo "$3  $tools/$1" | sha256sum --check --quiet || { rm -f "$tools/$1"; echo "$1 is not the expected file; removed it, run again" >&2; exit 1; }
}
mkdir -p "$tools" "$root/app"
fetch appimagetool-1.9.1-x86_64.AppImage https://github.com/AppImage/appimagetool/releases/download/1.9.1/appimagetool-x86_64.AppImage \
    ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0
fetch runtime-20251108-x86_64 https://github.com/AppImage/type2-runtime/releases/download/20251108/runtime-x86_64 \
    2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d
chmod +x "$tools/appimagetool-1.9.1-x86_64.AppImage"

rm -rf "$appdir"
dotnet restore "$project" --locked-mode -p:PublishProfile=LinuxProfile
dotnet publish "$project" -c Release --no-restore -p:PublishProfile=LinuxProfile

cp "$desktop/linux/AppRun" "$desktop/linux/frlg-trade-center.desktop" "$appdir/"
cp "$desktop/Assets/icon.png" "$appdir/frlg-trade-center.png"
ln -s frlg-trade-center.png "$appdir/.DirIcon"

# --appimage-extract-and-run: the build machine needs no FUSE.
ARCH=x86_64 "$tools/appimagetool-1.9.1-x86_64.AppImage" --appimage-extract-and-run --no-appstream \
    --runtime-file "$tools/runtime-20251108-x86_64" "$appdir" "$output"
echo "Ready: app/$(basename "$output")"
