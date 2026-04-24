# LLM context – ACE Amiga boilerplate

## Stack
- **Target**: Amiga OCS/ECS, CPU 68000, PAL 320×256, 4 bpp
- **Engine**: [ACE (Amiga C Engine)](https://github.com/AmigaPorts/ACE) – manages copper, blitter, viewport, tilebuffer, keyboard
- **Compiler**: Bartman's `m68k-amiga-elf-gcc` (GCC 14, bare-metal, ships with VS Code extension `bartmanabyss.amiga-debug`)
- **Build**: CMake + `make`. Output: `build/game.elf` (debug symbols) + `build/game.exe` (Amiga HUNK via elf2hunk)

## Build
```sh
make          # configure + build (first run runs cmake automatically)
make clean    # wipe build/
bash run.sh   # build → deploy to ~/Amiga/work → launch FS-UAE
```
F5 in VS Code triggers the `compile` task (same `make`) then launches Bartman's debugger/profiler.

CMake flags that matter:
- `-DTOOLCHAIN_PATH=…/bartmanabyss.amiga-debug-1.7.9/bin/darwin/opt`
- `-DTOOLCHAIN_PREFIX=m68k-amiga-elf`
- `-DM68K_CPU=68000`
- Toolchain file: `deps/AmigaCMakeCrossToolchains/m68k-bartman.cmake`

ACE is added via `add_subdirectory(deps/ace ace)`. Both `deps/ace` and `deps/AmigaCMakeCrossToolchains` are git submodules.

## Project structure
```
src/main.c        – ACE entrypoint (genericCreate/Process/Destroy) + state manager
src/scroller.c/.h – self-contained scroller game state (delete to start a new game)
CMakeLists.txt    – project "game", sources from src/*.c
deps/ace/         – ACE submodule
deps/AmigaCMakeCrossToolchains/ – CMake toolchain submodule
conf/game.fs-uae  – FS-UAE config (A1200, 2 MB chip)
conf/startup-sequence – AmigaDOS boot script: Work:game.exe
```

## Naming conventions
No Hungarian notation. Plain descriptive names:
- local variables: `dx`, `x`, `base`
- file-scope statics: `view`, `vport`, `tileBuf`, `tileSet`, `palette`
- globals: `stateMgr`, `scrollerState`

ACE API names are kept as-is: `tView`, `tVPort`, `UWORD`, `UBYTE`, `TAG_*`, `KEY_*`, `BMF_*` etc.

## ACE patterns used
```c
// main loop (generic/main.h provides amigaMain, calls these three)
void genericCreate(void)  { keyCreate(); stateMgr = stateManagerCreate(); stateChange(…); }
void genericProcess(void) { keyProcess(); stateProcess(stateMgr); }
void genericDestroy(void) { stateManagerDestroy(stateMgr); keyDestroy(); }

// game state struct
tState scrollerState = { .cbCreate=…, .cbLoop=…, .cbDestroy=…, .cbSuspend=0, .cbResume=0 };

// display teardown order
systemUse(); viewLoad(0); viewDestroy(view); bitmapDestroy(tileSet);
// viewDestroy frees vPort, tileBuffer, scrollBuffer, camera automatically
```

## Key ACE facts
- `systemUnuse()` suspends AmigaOS (call after all allocations, before main loop)
- `systemUse()` restores OS (call first in destroy, before freeing anything)
- Tilebuffer has no double buffering – single large scrolling bitmap, copper window moves
- Add BOBs → enable `SBF_DOUBLE_BUFFER` on the scrollBuffer
- `ACE_SCROLLBUFFER_ENABLE_SCROLL_X/Y` must be set ON in CMake before `add_subdirectory(deps/ace)`
- `BARTMAN_GCC` is defined automatically by the toolchain file
- `-nostdlib` is NOT needed – the toolchain file already sets it via `CMAKE_EXE_LINKER_FLAGS`

## Memory (A1200 / 2 MB chip)
- Scrollbuffer for 320×256 + margins: ~88 KB chip RAM
- Tile map 128×128 bytes: 16 KB
- 512 KB chip (A500) is too tight – use A1200 config or add slow RAM
