#!/usr/bin/env bash
# run.sh - configure/build/package dist/, then launch FS-UAE for quick testing.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/build}"
DIST_DIR="${DIST_DIR:-$SCRIPT_DIR/dist}"

AMIGA_LIBS="${AMIGA_LIBS:-$HOME/Amiga/libs}"
ACE_DIR="${ACE_DIR:-$AMIGA_LIBS/ACE}"
TOOLCHAINS_DIR="${TOOLCHAINS_DIR:-$AMIGA_LIBS/AmigaCMakeCrossToolchains}"

BARTMAN_VERSION="${BARTMAN_VERSION:-1.7.9}"
BARTMAN_DIR="${BARTMAN_DIR:-$HOME/.vscode/extensions/bartmanabyss.amiga-debug-$BARTMAN_VERSION/bin/darwin}"
TOOLCHAIN_PATH="${TOOLCHAIN_PATH:-$BARTMAN_DIR/opt}"
TOOLCHAIN_FILE="${TOOLCHAIN_FILE:-$TOOLCHAINS_DIR/m68k-bartman.cmake}"
TOOLCHAIN_PREFIX="${TOOLCHAIN_PREFIX:-m68k-amiga-elf}"

M68K_CPU="${M68K_CPU:-68020}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
GAME_USE_AGA="${GAME_USE_AGA:-ON}"
BUILD_JOBS="${BUILD_JOBS:-4}"
FS_UAE="${FS_UAE:-/Applications/FS-UAE.app/Contents/MacOS/fs-uae}"

export PATH="$TOOLCHAIN_PATH/bin:$BARTMAN_DIR:$PATH"

usage() {
    cat <<EOF
Usage: ./run.sh [configure|build|dist|run|clean]

Default command is "run".

Environment overrides:
  AMIGA_LIBS       $AMIGA_LIBS
  ACE_DIR          $ACE_DIR
  TOOLCHAINS_DIR   $TOOLCHAINS_DIR
  BARTMAN_VERSION  $BARTMAN_VERSION
  M68K_CPU         $M68K_CPU
  BUILD_TYPE       $BUILD_TYPE
  GAME_USE_AGA     $GAME_USE_AGA
EOF
}

configure() {
    if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
        cached_ace_dir="$(sed -n 's/^ACE_DIR:PATH=//p' "$BUILD_DIR/CMakeCache.txt" || true)"
        cached_cpm_dir="$(sed -n 's/^CPM_DIRECTORY:INTERNAL=//p' "$BUILD_DIR/CMakeCache.txt" || true)"

        if [ -n "$cached_ace_dir" ] && [ "$cached_ace_dir" != "$ACE_DIR" ]; then
            echo "--- Removing stale build cache for previous ACE_DIR ---"
            rm -rf "$BUILD_DIR"
        elif [ -n "$cached_cpm_dir" ] && [ "$cached_cpm_dir" != "$ACE_DIR/cmake" ]; then
            echo "--- Removing stale build cache for previous ACE CPM directory ---"
            rm -rf "$BUILD_DIR"
        fi
    fi

    echo "--- Configuring CMake ---"
    cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -DTOOLCHAIN_PATH="$TOOLCHAIN_PATH" \
        -DTOOLCHAIN_PREFIX="$TOOLCHAIN_PREFIX" \
        -DM68K_CPU="$M68K_CPU" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DACE_DIR="$ACE_DIR" \
        -DGAME_USE_AGA="$GAME_USE_AGA" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
}

build() {
    configure
    echo "--- Building ---"
    cmake --build "$BUILD_DIR" --parallel "$BUILD_JOBS"
}

package_dist() {
    build
    echo "--- Packaging dist ---"
    rm -rf "$DIST_DIR"
    mkdir -p "$DIST_DIR/s"
    cp "$BUILD_DIR/game.exe" "$DIST_DIR/"
    cp "$SCRIPT_DIR/conf/startup-sequence" "$DIST_DIR/s/"
    cp -R "$BUILD_DIR/data" "$DIST_DIR/"
}

run() {
    package_dist
    echo "--- Starting FS-UAE ---"
    "$FS_UAE" \
        "$SCRIPT_DIR/conf/game.fs-uae" \
        --hard_drive_0="$DIST_DIR" \
        --hard_drive_0_label=Work \
        --hard_drive_0_bootable=1
}

clean() {
    echo "--- Cleaning ---"
    rm -rf "$BUILD_DIR" "$DIST_DIR"
}

COMMAND="${1:-run}"
case "$COMMAND" in
    configure) configure ;;
    build) build ;;
    dist) package_dist ;;
    run) run ;;
    clean) clean ;;
    -h|--help|help) usage ;;
    *)
        usage
        exit 1
        ;;
esac
