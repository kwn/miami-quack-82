#!/usr/bin/env bash
# run.sh – build, deploy to ~/Amiga/work, launch FS-UAE
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BARTMAN="$HOME/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin"
export PATH="$PATH:$BARTMAN/opt/bin:$BARTMAN"

echo "--- Compiling ---"
make -C "$SCRIPT_DIR" -j4

echo "--- Deploying to ~/Amiga/work ---"
mkdir -p "$HOME/Amiga/work/S"
cp "$SCRIPT_DIR/build/game.exe"         "$HOME/Amiga/work/"
cp "$SCRIPT_DIR/conf/startup-sequence"  "$HOME/Amiga/work/S/"

echo "--- Starting FS-UAE ---"
"/Applications/FS-UAE.app/Contents/MacOS/fs-uae" "$SCRIPT_DIR/conf/game.fs-uae"
