# Miami Quack 82 ACE Port

Experimental ACE-based port of the Amiga game. The project is currently focused on reproducing the game framework setup, menu flow, tilebuffer scrolling, BOB rendering and the scrolling glitch investigation.

## Requirements

- macOS or another host capable of running the Bartman Amiga GCC toolchain.
- [FS-UAE](https://fs-uae.net/) for running the game.
- A valid Amiga Kickstart ROM for the configured model.
- Git submodules initialized.

The current helper scripts contain local paths for the toolchain, FS-UAE and Kickstart ROM. Adjust these paths locally before running if your setup differs.

## Clone

```sh
git clone <repo-url>
cd miami-quack-82
git submodule update --init --recursive
```

## Local Paths To Check

Check and adjust these files for your machine:

- `Makefile`
  - `BARTMAN_BIN` points to the Bartman VSCode/Cursor extension toolchain.
- `run.sh`
  - FS-UAE is launched from `/Applications/FS-UAE.app/Contents/MacOS/fs-uae`.
- `conf/game.fs-uae`
  - `kickstart_file` must point to your local Kickstart ROM.

## Build

```sh
make
```

This configures CMake on first run and builds `build/game.exe`.

To force a clean rebuild:

```sh
make clean
make
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

## Runtime Notes

- `conf/startup-sequence` runs `Work:game.exe`.
- `dist/` and `build/` are intentionally ignored by git.
- The game currently targets `A1200` in `conf/game.fs-uae`; `run.sh` defaults the build to `M68K_CPU=68020`.
- ACE is built from `ACE_DIR`, passed by `run.sh` and defaulting there to `~/Amiga/libs/ACE`.
