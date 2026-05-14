# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Language

Always communicate with the user in Polish. Code, comments, commit messages, and identifiers stay in English.

## What this is

A top-down shooter for Amiga (AGA/68020), rebuilt on top of the [ACE engine](https://github.com/AmigaPorts/ACE). C11, cross-compiled from macOS using the Bartman m68k-amiga-elf toolchain. This is a step-by-step reimplementation of the original game from the sibling `game/` folder — use ACE managers and tooling wherever possible instead of porting old custom low-level code.

## Build and run

```bash
./run.sh            # configure + build + package dist/ + launch FS-UAE
./run.sh build      # configure + build only
./run.sh clean      # wipe build/ and dist/
```

`run.sh` is the real entry point. The top-level `Makefile` is a thin wrapper. Key env overrides: `ACE_DIR`, `M68K_CPU` (default 68020), `GAME_USE_AGA` (default ON), `BUILD_TYPE` (default Debug).

There are no tests or linting tools — verification is done by building and running in the emulator.

## Build system details

- CMake cross-compiles game code with `m68k-amiga-elf-gcc` via Bartman toolchain (1.7.9). ACE host tools (`palette_conv`, `bitmap_conv`, `font_conv`) are built with the native compiler.
- ACE lives outside this repo (default `~/Amiga/libs/ACE`). `ACE_DIR` must never be hardcoded in `CMakeLists.txt`.
- ACE CMake options (`ACE_SCROLLBUFFER_*`, `ACE_BOB_*`) must be set **before** `add_subdirectory("${ACE_DIR}" ace)`.
- Switching `M68K_CPU`, `GAME_USE_AGA`, or `ACE_DIR` requires a clean build; `run.sh` handles this automatically.
- `BARTMAN_GCC` is defined by the toolchain file — don't add it manually. `-nostdlib` is already in `CMAKE_EXE_LINKER_FLAGS` — don't duplicate it in `CMakeLists.txt`.
- Python 3 scripts in `cmake/` transform assets at build time: `prepare_palette.py`, `prepare_tileset_column.py`, `prepare_sprite_png.py`, `prepare_masked_png.py`, `prepare_game_palette_png.py`.

## Asset pipeline

- Source art: `res/` (PNG, Aseprite, GPL palettes, TTF font). Build outputs go to `build/data/`.
- Bitmaps use per-asset palettes from `res/palettes/<name>.gpl`; fall back to `res/palettes/game.gpl`.
- Views use `GAME_BPP` = 6 (64 colors). Palettes must have exactly 64 entries.
- Player, weapons, and bullets use `#FF00FF` mask color and interleaved bitmaps.
- Tilesets are converted from a grid PNG (columns x rows) into a single interleaved column bitmap via `prepare_tileset_column.py`. Grid dimensions are defined per-world in `CMakeLists.txt` (`WORLD_TILESET_COLUMNS_*`, `WORLD_TILESET_ROWS_*`).
- For AGA builds, palettes keep full 24-bit color. For OCS, `prepare_palette.py` reduces to 12-bit.

## Architecture

### State machine

ACE `tStateManager` drives the game loop. States are `tState` structs with `cbCreate`/`cbLoop`/`cbDestroy` callbacks.

- **`main.c`** — ACE entry point (`genericCreate`/`genericProcess`/`genericDestroy`). Starts in `titleState`.
- **`title.c`** — Title screen + menu. On "START GAME", calls `campaignStartNewGame()` and transitions to `campaignState`.
- **`campaign.c`** — Routes between states based on `CampaignRoute`/`LevelResult`. Manages world/level progression, lives, money. World and level definitions are static data arrays here.
- **`get_ready.c`** — Inter-level screen. Loads level bitmap, shows level name, transitions to `gameState` via `campaignSetRoute(CAMPAIGN_ROUTE_GAMEPLAY)`.
- **`game.c`** — Gameplay state. Creates/destroys all gameplay subsystems. `gamePrepare()`/`gameDiscardPrepared()` allow reuse across level transitions without re-allocating.

### Gameplay subsystems (all created/destroyed by `game.c`)

Each subsystem follows the `create`/`process`/`render`/`destroy` pattern:

- **`scroller.c`** — Wraps ACE `tTileBufferManager`. Loads the current world's tileset, populates tile data from `mapData[]`, manages double-buffered scrolling with a pristine buffer for BOB restore.
- **`player*.c`** — Player split across files: `player.c` (BOB, position), `player_input.c`, `player_physics.c`, `player_anim.c`, `player_dodge.c`.
- **`aim.c`** — Hardware sprite cursor for aiming.
- **`weapon.c`** — Weapon BOB rendering and firing logic. 8 weapon types.
- **`bullet.c`** — Bullet pool with BOB rendering. Two bullet types (small, flame).
- **`game_camera.c`** — Smooth camera tracking of the player.
- **`map.c` / `map_generator*.c`** — `mapData[128*128]` flat array. Procedural generators per world type (outdoor, lake).
- **`hud.c`** — Top 16px HUD bar.

### AGA / ECS conditional compilation

All AGA-specific code is guarded with `#ifdef ACE_USE_AGA_FEATURES`. Key differences: palette storage (`ULONG[]` vs `UWORD[]`), fade functions (`paletteDimAga` vs `paletteDimOcs`), viewport tags (`TAG_VPORT_USES_AGA`, `TAG_VPORT_FMODE`), `KILLEHB` in `bplcon2`.

## Conventions

- No Hungarian notation for game code. Plain names: `view`, `tileBuf`, `dx`. ACE API types/macros unchanged: `tView`, `UWORD`, `TAG_*`, `KEY_*`, `BMF_*`.
- `systemUnuse()` after all allocations. `systemUse()` first in destroy, before any frees.
- `viewDestroy()` recursively frees vPort, tileBuffer, scrollBuffer, camera — don't free them manually.
- Prefer small incremental changes that keep the playable prototype working.
