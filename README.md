# Miami Quack 82 ACE Port

Experimental ACE-based port of the Amiga game. The project is currently focused on reproducing the game framework setup, menu flow, tilebuffer scrolling, BOB rendering and the scrolling glitch investigation.

## Requirements

- macOS or another host capable of running the Bartman Amiga GCC toolchain.
- [FS-UAE](https://fs-uae.net/) for running the game.
- A valid Amiga Kickstart ROM for the configured model.
- ACE checked out outside of this repository. By default `run.sh` expects it at `~/Amiga/libs/ACE`.
- AmigaCMakeCrossToolchains checked out outside of this repository. By default `run.sh` expects it at `~/Amiga/libs/AmigaCMakeCrossToolchains`.
- Bartman VSCode/Cursor extension 1.7.9, used for the bundled `m68k-amiga-elf` toolchain.

ACE currently comes from its `main` branch, where the AGA work has been merged. The project can still be built with AGA features disabled by overriding `GAME_USE_AGA=OFF`.

## Clone

```sh
git clone <repo-url>
cd miami-quack-82
```

There are no project submodules for ACE or the toolchain. Keep shared Amiga libraries under `~/Amiga/libs` or override the paths described below.

## Configuration

The main entry point is `run.sh`. Its defaults can be overridden with environment variables:

```sh
AMIGA_LIBS=~/Amiga/libs
ACE_DIR=$AMIGA_LIBS/ACE
TOOLCHAINS_DIR=$AMIGA_LIBS/AmigaCMakeCrossToolchains
BARTMAN_VERSION=1.7.9
M68K_CPU=68020
BUILD_TYPE=Debug
GAME_USE_AGA=ON
BUILD_JOBS=4
FS_UAE=/Applications/FS-UAE.app/Contents/MacOS/fs-uae
```

Also check:

- `conf/game.fs-uae`: `kickstart_file` must point to your local Kickstart ROM.
- `.vscode/launch.json`: F5 configurations use the Bartman debugger and `build/game`.

## Build

```sh
./run.sh build
```

This configures CMake, builds ACE from `ACE_DIR`, builds the ACE host tools (`palette_conv`, `bitmap_conv`, `font_conv`) with the host compiler, converts assets and produces:

```text
build/game.elf
build/game.exe
build/data/
```

The `Makefile` is only a compatibility wrapper around `run.sh`, so this also works:

```sh
make
```

To force a clean rebuild:

```sh
./run.sh clean
./run.sh build
```

When switching CPU targets, use a clean build:

```sh
./run.sh clean
M68K_CPU=68020 ./run.sh build
```

## Run

```sh
./run.sh
```

The script builds the game, creates a fresh `dist/` directory, copies the required runtime files there, mounts `dist/` as `Work:` in FS-UAE and boots the game through `conf/startup-sequence`.

Expected `dist/` contents:

```text
dist/
  game.exe
  data/
  s/startup-sequence
```

Useful commands:

```sh
./run.sh configure
./run.sh build
./run.sh dist
./run.sh run
./run.sh clean
```

## VSCode Debugging

F5 is handled by the Bartman extension and is separate from the quick FS-UAE path in `run.sh`.

Available launch configurations:

- `Amiga 500`
- `Amiga 1200`
- `Amiga 1200 + Fast`

The debugger expects `build/game` in configuration; the extension appends the final executable suffix itself.

## Runtime Notes

- `conf/startup-sequence` runs `Work:game.exe`.
- `dist/` and `build/` are intentionally ignored by git.
- `conf/game.fs-uae` currently targets `A1200` for quick `./run.sh run` testing.
- `run.sh` defaults to `M68K_CPU=68020` (A1200-class) and `GAME_USE_AGA=ON`.
- If your ACE checkout does not expose `ACE_USE_AGA_FEATURES`, build with `GAME_USE_AGA=OFF`.
- Changing `M68K_CPU` does not auto-invalidate the CMake cache; use `./run.sh clean` before rebuilding when switching CPU (e.g. to `68000` for A500-style builds).
