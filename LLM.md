# LLM context

## Project context
- This project is a step-by-step reimplementation of the original Amiga game from the sibling `game` folder.
- The goal is to rebuild the game on top of ACE, using ACE managers and tooling wherever possible instead of porting old custom low-level code directly.
- The game is a top-down shooter with scrolling maps, hardware-sprite aiming, a BOB-based player, HUD, campaign/stage flow, and generated or authored level maps.
- The codebase should remain ECS-compatible while also being AGA-ready when built against an AGA-capable ACE fork.
- Prefer small, incremental changes that keep the current playable prototype working.

## Naming
No Hungarian notation. Plain names: `view`, `tileBuf`, `dx`, `stateMgr`.
ACE API unchanged: `tView`, `UWORD`, `TAG_*`, `KEY_*`, `BMF_*` etc.

## Non-obvious build gotchas
- `-DTOOLCHAIN_PREFIX=m68k-amiga-elf` is required – without it CMake defaults to `m68k-generic` and can't find the compiler.
- ACE CMake options (`ACE_SCROLLBUFFER_*` etc.) must be set before `add_subdirectory("${ACE_DIR}" ace)`.
- `ACE_DIR` is passed by `run.sh`; keep local machine paths out of `CMakeLists.txt`.
- If `GAME_USE_AGA` is enabled, the project verifies that `ACE_DIR` looks AGA-capable and forces `ACE_USE_AGA_FEATURES=ON`.
- Palette outputs live in `build/data/palettes/*.plt`; `prepare_palette.py` leaves GPL colors unchanged for AGA and reduces them to OCS 12-bit precision for OCS before `palette_conv`.
- Bitmap resources live as flat `res/bitmaps/*.png`; a bitmap uses `res/palettes/<name>.gpl` when present, otherwise it falls back to `res/palettes/game.gpl`.
- `BARTMAN_GCC` is defined automatically by the toolchain file – don't add it manually.
- `-nostdlib` is already in `CMAKE_EXE_LINKER_FLAGS` in the toolchain file – don't add it again in `CMakeLists.txt`.

## Non-obvious ACE gotchas
- `systemUnuse()` must be called **after** all allocations (bitmaps, buffers, views).
- `systemUse()` must be called **first** in destroy, before freeing anything.
- `viewDestroy(view)` recursively frees vPort, tileBuffer, scrollBuffer, and camera – don't free them manually.
- State callbacks in `tState` can be NULL (`0`) if not needed.
- In AGA builds, title palette storage is `ULONG[]` and fade uses `paletteDimAga()` with 0-255 levels.
- In ECS builds, title palette storage is `UWORD[]` and fade uses `paletteDim()` with 0-15 levels.
- AGA title viewport tags are guarded with `#ifdef ACE_USE_AGA_FEATURES`; ECS builds must not reference AGA tags/functions.
