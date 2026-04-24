cmake_minimum_required(VERSION 3.14.0)
project(game)

if(NOT AMIGA)
    message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)

# ---- ACE options (set before add_subdirectory) ----------------------------
set(ACE_SCROLLBUFFER_ENABLE_SCROLL_X ON  CACHE BOOL "" FORCE)
set(ACE_SCROLLBUFFER_ENABLE_SCROLL_Y ON  CACHE BOOL "" FORCE)
set(ACE_BOB_WRAP_Y                   OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/ace ace)

# ---- Game sources ----------------------------------------------------------
file(GLOB_RECURSE SOURCES src/*.c src/*.h)

set(STEM game)

if(ELF2HUNK)
    set(GAME_ELF ${STEM}.elf)
    set(GAME_EXE ${STEM}.exe)
    add_executable(${GAME_ELF} ${SOURCES})
    add_custom_command(
        TARGET ${GAME_ELF} POST_BUILD
        COMMAND ${ELF2HUNK} ${GAME_ELF} ${GAME_EXE}
        COMMENT "elf2hunk → ${GAME_EXE}"
    )
    set(GAME_TARGET ${GAME_ELF})
else()
    set(GAME_EXE ${STEM})
    add_executable(${GAME_EXE} ${SOURCES})
    set(GAME_TARGET ${GAME_EXE})
endif()

target_include_directories(${GAME_TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_options(${GAME_TARGET} PRIVATE -Wall -Wextra -Wno-unused-parameter)
target_link_options(${GAME_TARGET} PRIVATE -Wl,--emit-relocs,--gc-sections,-Ttext=0)
target_link_libraries(${GAME_TARGET} ace)
