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
- ACE CMake options (`ACE_SCROLLBUFFER_*` etc.) must be set **before** `add_subdirectory(deps/ace)`.
- `CMakeLists.txt` auto-detects AGA-capable ACE by checking for `ACE_USE_AGA_FEATURES` in `deps/ace/CMakeLists.txt`.
- If AGA is detected, the project forces `ACE_USE_AGA_FEATURES=ON` and writes `title.plt` with `palette_conv --aga`.
- If AGA is not detected, the project writes legacy ECS `.plt` with `palette_conv --legacy` for compatibility with the old ACE palette loader.
- `cmake/gpl_to_ocs.cmake` replaces old `CONVERT_COLORS`/`-cc`: it generates `build/data/title/title_ocs.gpl` from the original 24-bit `title.gpl`.
- `bitmap_conv` must keep using original `res/title/title.gpl`; only runtime ECS `.plt` uses generated `title_ocs.gpl`.
- `BARTMAN_GCC` is defined automatically by the toolchain file – don't add it manually.
- `-nostdlib` is already in `CMAKE_EXE_LINKER_FLAGS` in the toolchain file – don't add it again in `CMakeLists.txt`.

## Non-obvious ACE gotchas
- `systemUnuse()` must be called **after** all allocations (bitmaps, buffers, views).
- `systemUse()` must be called **first** in destroy, before freeing anything.
- `viewDestroy(view)` recursively frees vPort, tileBuffer, scrollBuffer, and camera – don't free them manually.
- State callbacks in `tState` can be NULL (`0`) if not needed.
- In AGA builds, title palette storage is `ULONG[]` and fade uses `paletteDimAGA()` with 0-255 levels.
- In ECS builds, title palette storage is `UWORD[]` and fade uses `paletteDim()` with 0-15 levels.
- AGA title viewport tags are guarded with `#ifdef ACE_USE_AGA_FEATURES`; ECS builds must not reference AGA tags/functions.
