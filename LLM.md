# LLM context

## Naming
No Hungarian notation. Plain names: `view`, `tileBuf`, `dx`, `stateMgr`.
ACE API unchanged: `tView`, `UWORD`, `TAG_*`, `KEY_*`, `BMF_*` etc.

## Non-obvious build gotchas
- `-DTOOLCHAIN_PREFIX=m68k-amiga-elf` is required – without it CMake defaults to `m68k-generic` and can't find the compiler.
- ACE CMake options (`ACE_SCROLLBUFFER_*` etc.) must be set **before** `add_subdirectory(deps/ace)`.
- `BARTMAN_GCC` is defined automatically by the toolchain file – don't add it manually.
- `-nostdlib` is already in `CMAKE_EXE_LINKER_FLAGS` in the toolchain file – don't add it again in `CMakeLists.txt`.

## Non-obvious ACE gotchas
- `systemUnuse()` must be called **after** all allocations (bitmaps, buffers, views).
- `systemUse()` must be called **first** in destroy, before freeing anything.
- `viewDestroy(view)` recursively frees vPort, tileBuffer, scrollBuffer, and camera – don't free them manually.
- State callbacks in `tState` can be NULL (`0`) if not needed.
