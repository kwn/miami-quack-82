#!/usr/bin/env bash
# run.sh - build, package dist/, launch FS-UAE
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$SCRIPT_DIR/dist"

echo "--- Compiling ---"
make -C "$SCRIPT_DIR" -j4

echo "--- Packaging dist ---"
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR/s"
cp "$SCRIPT_DIR/build/game.exe"        "$DIST_DIR/"
cp "$SCRIPT_DIR/conf/startup-sequence" "$DIST_DIR/s/"
cp -R "$SCRIPT_DIR/build/data"         "$DIST_DIR/"

echo "--- Starting FS-UAE ---"
"/Applications/FS-UAE.app/Contents/MacOS/fs-uae" \
    "$SCRIPT_DIR/conf/game.fs-uae" \
    --hard_drive_0="$DIST_DIR" \
    --hard_drive_0_label=Work \
    --hard_drive_0_bootable=1
