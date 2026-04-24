# CMake toolchain file for Bartman's m68k-amiga-elf GCC
# Targets Amiga OCS/ECS with 68000 CPU (safe default for A500)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR m68k)

# Tell ACE and game CMakeLists that we target Amiga
set(AMIGA 1 CACHE BOOL "Building for Amiga" FORCE)

# Tell ACE to use Bartman's compiler path and mini_std
set(M68K_COMPILER "Bartman" CACHE STRING "M68k compiler type" FORCE)

# Paths to Bartman's toolchain (shipped with the vscode-amiga-debug extension)
set(_BARTMAN_ROOT "$ENV{HOME}/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin")
set(_TOOLCHAIN_BIN "${_BARTMAN_ROOT}/opt/bin")

set(CMAKE_C_COMPILER    "${_TOOLCHAIN_BIN}/m68k-amiga-elf-gcc"    CACHE FILEPATH "C compiler")
set(CMAKE_ASM_COMPILER  "${_TOOLCHAIN_BIN}/m68k-amiga-elf-gcc"    CACHE FILEPATH "ASM compiler")
set(CMAKE_OBJDUMP       "${_TOOLCHAIN_BIN}/m68k-amiga-elf-objdump" CACHE FILEPATH "objdump")

# elf2hunk converts the ELF output into Amiga HUNK executable format
set(ELF2HUNK "${_BARTMAN_ROOT}/elf2hunk" CACHE FILEPATH "elf2hunk converter")

# Don't search host machine for libraries/headers
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Skip CMake's compiler sanity check (cross-compilation, no startup files yet)
set(CMAKE_C_COMPILER_WORKS   1 CACHE BOOL "")
set(CMAKE_CXX_COMPILER_WORKS 1 CACHE BOOL "")

# Target CPU - 68000 is the safest; change to 68020 for A1200/faster
set(M68K_CPU "68000" CACHE STRING "Target M68k CPU")
set(M68K_FPU "soft"  CACHE STRING "Target M68k FPU")

# m68k-amiga-elf-as requires --register-prefix-optional to accept the
# AT&T indirect addressing syntax (sp@(4), etc.) used in Amiga assembly files.
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Wa,--register-prefix-optional" CACHE STRING "" FORCE)

# -fomit-frame-pointer: standard for Amiga code; also prevents an ICE in GCC 14
# dwarf2cfi when compiling inline asm that modifies the stack pointer.
# -DBARTMAN_GCC: tells ACE's types.h to use the Bartman-specific macro definitions
#   (e.g. empty FAR, simplified REGARG) instead of the Bebbo/VBCC paths.
set(CMAKE_C_FLAGS_INIT   "-fomit-frame-pointer -m${M68K_CPU} -DBARTMAN_GCC")
set(CMAKE_ASM_FLAGS_INIT "-fomit-frame-pointer -m${M68K_CPU}")
