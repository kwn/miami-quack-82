cmake_minimum_required(VERSION 3.14.0)
project(ace_scroller)

if(NOT AMIGA)
    message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)

# ---- ACE configuration ----
# Both X and Y scroll enabled; our demo only scrolls horizontally but
# having Y available costs nothing and avoids recompiling ACE later.
set(ACE_SCROLLBUFFER_ENABLE_SCROLL_X ON  CACHE BOOL "" FORCE)
set(ACE_SCROLLBUFFER_ENABLE_SCROLL_Y ON  CACHE BOOL "" FORCE)
set(ACE_BOB_WRAP_Y                   OFF CACHE BOOL "" FORCE)

# Point at the ACE library sitting next to this project
add_subdirectory(/Users/kwnuk/Amiga/ACE ace)

# ---- Game target ----
file(GLOB SOURCES src/*.c)

set(STEM ace_scroller)

if(ELF2HUNK)
    # Bartman's GCC produces ELF; convert to Amiga HUNK with elf2hunk
    set(GAME_ELF ${STEM}.elf)
    set(GAME_EXE ${STEM}.exe)
    add_executable(${GAME_ELF} ${SOURCES})
    add_custom_command(
        TARGET ${GAME_ELF} POST_BUILD
        COMMAND ${ELF2HUNK} ${GAME_ELF} ${GAME_EXE}
        COMMENT "Converting ELF -> Amiga HUNK: ${GAME_EXE}"
    )
    set(GAME_TARGET ${GAME_ELF})
else()
    set(GAME_EXE ${STEM})
    add_executable(${GAME_EXE} ${SOURCES})
    set(GAME_TARGET ${GAME_EXE})
endif()

target_include_directories(${GAME_TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_options(${GAME_TARGET} PRIVATE -Wall -Wextra -Wno-unused-parameter)

# Bartman's toolchain is bare-metal: no standard libraries or startup files.
# -nostdlib    : don't link crt*.o / libgcc / libc
# --emit-relocs: keep relocation info so elf2hunk can produce the HUNK format
# --gc-sections: discard unreferenced sections
# -Ttext=0     : start code at address 0 (required by elf2hunk)
target_link_options(${GAME_TARGET} PRIVATE
    -nostdlib
    -Wl,--emit-relocs,--gc-sections,-Ttext=0
)

target_link_libraries(${GAME_TARGET} ace)
