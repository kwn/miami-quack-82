#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# ---- Build ----------------------------------------------------------------
echo "--- Compiling ---"
export PATH="$PATH:$HOME/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin/opt/bin:$HOME/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin"
make -C "$SCRIPT_DIR"

# ---- Deploy to Work drive -------------------------------------------------
echo "--- Deploying to ~/Amiga/work ---"
mkdir -p "$HOME/Amiga/work/S"
cp "$SCRIPT_DIR/build/ace_scroller.exe" "$HOME/Amiga/work/"
cp "$SCRIPT_DIR/conf/startup-sequence"  "$HOME/Amiga/work/S/"

# ---- Launch FS-UAE --------------------------------------------------------
echo "--- Starting FS-UAE ---"
"/Applications/FS-UAE.app/Contents/MacOS/fs-uae" "$SCRIPT_DIR/conf/scroller.fs-uae"
