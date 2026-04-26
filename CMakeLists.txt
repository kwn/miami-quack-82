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
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.c src/*.h)

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

# ---- Asset conversion -------------------------------------------------------
set(RES_DIR ${CMAKE_CURRENT_LIST_DIR}/res)
set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)

file(MAKE_DIRECTORY
    ${DATA_DIR}/title
    ${DATA_DIR}/fonts
)

convertPalette(
    ${GAME_TARGET}
    ${RES_DIR}/title/title.gpl
    ${DATA_DIR}/title/title.plt
    CONVERT_COLORS
)

convertBitmaps(
    TARGET ${GAME_TARGET}
    PALETTE ${RES_DIR}/title/title.gpl
    SOURCES ${RES_DIR}/title/title.png
    DESTINATIONS ${DATA_DIR}/title/title.bm
)

getToolPath(font_conv TOOL_FONT_CONV)
add_custom_command(
    OUTPUT ${DATA_DIR}/fonts/quaver.fnt
    COMMAND ${TOOL_FONT_CONV}
        ${RES_DIR}/fonts/quaver.ttf
        fnt
        -out ${DATA_DIR}/fonts/quaver.fnt
        -size 8
        -charFile ${RES_DIR}/fonts/printable_ascii.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        ${RES_DIR}/fonts/quaver.ttf
        ${RES_DIR}/fonts/printable_ascii.txt
)
target_sources(${GAME_TARGET} PUBLIC ${DATA_DIR}/fonts/quaver.fnt)
