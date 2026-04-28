cmake_minimum_required(VERSION 3.14.0)
project(game)

if(NOT AMIGA)
    message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)

# ---- ACE options (set before add_subdirectory) ----------------------------
file(STRINGS
    ${CMAKE_CURRENT_LIST_DIR}/deps/ace/CMakeLists.txt
    ACE_AGA_FEATURE_LINES
    REGEX "ACE_USE_AGA_FEATURES"
)
if(ACE_AGA_FEATURE_LINES)
    set(GAME_USE_AGA ON)
    set(ACE_USE_AGA_FEATURES ON CACHE BOOL "" FORCE)
else()
    set(GAME_USE_AGA OFF)
endif()

set(ACE_SCROLLBUFFER_ENABLE_SCROLL_X ON  CACHE BOOL "" FORCE)
set(ACE_SCROLLBUFFER_ENABLE_SCROLL_Y ON  CACHE BOOL "" FORCE)
set(ACE_BOB_WRAP_Y                   ON  CACHE BOOL "" FORCE)
set(ACE_BOB_PRISTINE_BUFFER          ON  CACHE BOOL "" FORCE)

add_subdirectory(deps/ace ace)
message(STATUS "[game] GAME_USE_AGA: '${GAME_USE_AGA}'")

function(ensureAceTools)
    set(ACE_TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR}/deps/ace/tools)
    set(ACE_TOOLS_BUILD_DIR ${ACE_TOOLS_DIR}/build)
    set(ACE_REQUIRED_TOOLS palette_conv bitmap_conv font_conv)

    message(STATUS "[game] Ensuring ACE host tools")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -S ${ACE_TOOLS_DIR} -B ${ACE_TOOLS_BUILD_DIR}
        RESULT_VARIABLE ACE_TOOLS_CONFIGURE_RESULT
    )
    if(NOT ACE_TOOLS_CONFIGURE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to configure ACE host tools")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --build ${ACE_TOOLS_BUILD_DIR}
            --target ${ACE_REQUIRED_TOOLS}
        RESULT_VARIABLE ACE_TOOLS_BUILD_RESULT
    )
    if(NOT ACE_TOOLS_BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build ACE host tools")
    endif()
endfunction()

ensureAceTools()

# Match the original game's optimization level while keeping the selected CPU
# from the active toolchain/CMake preset.
set(GAME_OPT_FLAGS -Ofast)
target_compile_options(ace PRIVATE ${GAME_OPT_FLAGS})

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
target_compile_options(${GAME_TARGET} PRIVATE -Wall -Wextra -Wno-unused-parameter ${GAME_OPT_FLAGS})
target_link_options(${GAME_TARGET} PRIVATE -Wl,--emit-relocs,--gc-sections,-Ttext=0)
target_link_libraries(${GAME_TARGET} ace)

# ---- Asset conversion -------------------------------------------------------
set(RES_DIR ${CMAKE_CURRENT_LIST_DIR}/res)
set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)
set(TITLE_OCS_PALETTE ${DATA_DIR}/title/title_ocs.gpl)
set(GET_READY_OCS_PALETTE ${DATA_DIR}/get_ready/get_ready_ocs.gpl)
set(GAME_PALETTE ${RES_DIR}/jehkoba32_ecs.gpl)
set(AIM_SPRITE_PNG ${DATA_DIR}/sprites/aim_hw.png)
set(AIM_SPRITE_PALETTE ${DATA_DIR}/sprites/aim.gpl)
set(PLAYER_MASKED_PNG ${DATA_DIR}/player/player_masked.png)
set(OUTDOOR_TILESET_COLUMN_PNG ${DATA_DIR}/tileset/outdoor_column.png)

file(MAKE_DIRECTORY
    ${DATA_DIR}/title
    ${DATA_DIR}/get_ready
    ${DATA_DIR}/game
    ${DATA_DIR}/hud
    ${DATA_DIR}/player
    ${DATA_DIR}/sprites
    ${DATA_DIR}/tileset
    ${DATA_DIR}/fonts
)

add_custom_command(
    OUTPUT ${TITLE_OCS_PALETTE}
    COMMAND ${CMAKE_COMMAND}
        -DINPUT=${RES_DIR}/title/title.gpl
        -DOUTPUT=${TITLE_OCS_PALETTE}
        -P ${CMAKE_CURRENT_LIST_DIR}/cmake/gpl_to_ocs.cmake
    DEPENDS
        ${RES_DIR}/title/title.gpl
        ${CMAKE_CURRENT_LIST_DIR}/cmake/gpl_to_ocs.cmake
)

getToolPath(palette_conv TOOL_PALETTE_CONV)
find_program(PYTHON3_EXECUTABLE python3 REQUIRED)

if(GAME_USE_AGA)
    add_custom_command(
        OUTPUT ${DATA_DIR}/title/title.plt
        COMMAND ${TOOL_PALETTE_CONV}
            ${RES_DIR}/title/title.gpl
            ${DATA_DIR}/title/title.plt
            --aga
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${RES_DIR}/title/title.gpl
    )
else()
    add_custom_command(
        OUTPUT ${DATA_DIR}/title/title.plt
        COMMAND ${TOOL_PALETTE_CONV}
            ${TITLE_OCS_PALETTE}
            ${DATA_DIR}/title/title.plt
            --legacy
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${TITLE_OCS_PALETTE}
    )
endif()
target_sources(${GAME_TARGET} PUBLIC ${DATA_DIR}/title/title.plt)

if(GAME_USE_AGA)
    add_custom_command(
        OUTPUT ${DATA_DIR}/game/game.plt
        COMMAND ${TOOL_PALETTE_CONV}
            ${GAME_PALETTE}
            ${DATA_DIR}/game/game.plt
            --aga
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${GAME_PALETTE}
    )
else()
    add_custom_command(
        OUTPUT ${DATA_DIR}/game/game.plt
        COMMAND ${TOOL_PALETTE_CONV}
            ${GAME_PALETTE}
            ${DATA_DIR}/game/game.plt
            --legacy
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${GAME_PALETTE}
    )
endif()
target_sources(${GAME_TARGET} PUBLIC ${DATA_DIR}/game/game.plt)

convertBitmaps(
    TARGET ${GAME_TARGET}
    PALETTE ${RES_DIR}/title/title.gpl
    SOURCES ${RES_DIR}/title/title.png
    DESTINATIONS ${DATA_DIR}/title/title.bm
)

if(GAME_USE_AGA)
    add_custom_command(
        OUTPUT ${DATA_DIR}/get_ready/get_ready.plt
        COMMAND ${TOOL_PALETTE_CONV}
            ${RES_DIR}/get_ready/get_ready.gpl
            ${DATA_DIR}/get_ready/get_ready.plt
            --aga
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${RES_DIR}/get_ready/get_ready.gpl
    )
else()
    add_custom_command(
        OUTPUT ${GET_READY_OCS_PALETTE}
        COMMAND ${CMAKE_COMMAND}
            -DINPUT=${RES_DIR}/get_ready/get_ready.gpl
            -DOUTPUT=${GET_READY_OCS_PALETTE}
            -P ${CMAKE_CURRENT_LIST_DIR}/cmake/gpl_to_ocs.cmake
        DEPENDS
            ${RES_DIR}/get_ready/get_ready.gpl
            ${CMAKE_CURRENT_LIST_DIR}/cmake/gpl_to_ocs.cmake
    )
    add_custom_command(
        OUTPUT ${DATA_DIR}/get_ready/get_ready.plt
        COMMAND ${TOOL_PALETTE_CONV}
            ${GET_READY_OCS_PALETTE}
            ${DATA_DIR}/get_ready/get_ready.plt
            --legacy
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${GET_READY_OCS_PALETTE}
    )
endif()
target_sources(${GAME_TARGET} PUBLIC ${DATA_DIR}/get_ready/get_ready.plt)

convertBitmaps(
    TARGET ${GAME_TARGET}
    PALETTE ${RES_DIR}/get_ready/get_ready.gpl
    SOURCES ${RES_DIR}/get_ready/get_ready.png
    DESTINATIONS ${DATA_DIR}/get_ready/get_ready.bm
)

convertBitmaps(
    TARGET ${GAME_TARGET}
    PALETTE ${GAME_PALETTE}
    SOURCES ${RES_DIR}/hud/hud.png
    DESTINATIONS ${DATA_DIR}/hud/hud.bm
)

add_custom_command(
    OUTPUT ${AIM_SPRITE_PNG} ${AIM_SPRITE_PALETTE}
    COMMAND ${PYTHON3_EXECUTABLE}
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_sprite_png.py
        --input ${RES_DIR}/sprites/aim.png
        --output-png ${AIM_SPRITE_PNG}
        --output-palette ${AIM_SPRITE_PALETTE}
        --width 16
        --height 16
    DEPENDS
        ${RES_DIR}/sprites/aim.png
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_sprite_png.py
)

convertBitmaps(
    TARGET ${GAME_TARGET}
    INTERLEAVED
    PALETTE ${AIM_SPRITE_PALETTE}
    SOURCES ${AIM_SPRITE_PNG}
    DESTINATIONS ${DATA_DIR}/sprites/aim.bm
)

add_custom_command(
    OUTPUT ${PLAYER_MASKED_PNG}
    COMMAND ${PYTHON3_EXECUTABLE}
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_masked_png.py
        --input ${RES_DIR}/player/player.png
        --output ${PLAYER_MASKED_PNG}
        --width 16
        --height 240
    DEPENDS
        ${RES_DIR}/player/player.png
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_masked_png.py
)

convertBitmaps(
    TARGET ${GAME_TARGET}
    INTERLEAVED
    PALETTE ${GAME_PALETTE}
    MASK_COLOR "#FF00FF"
    SOURCES ${PLAYER_MASKED_PNG}
    DESTINATIONS ${DATA_DIR}/player/player.bm
    MASKS ${DATA_DIR}/player/player_mask.bm
)

add_custom_command(
    OUTPUT ${OUTDOOR_TILESET_COLUMN_PNG}
    COMMAND ${PYTHON3_EXECUTABLE}
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_tileset_column.py
        --input ${RES_DIR}/tileset/outdoor.png
        --output ${OUTDOOR_TILESET_COLUMN_PNG}
        --tile-width 16
        --tile-height 16
        --columns 7
        --rows 5
    DEPENDS
        ${RES_DIR}/tileset/outdoor.png
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_tileset_column.py
)

convertBitmaps(
    TARGET ${GAME_TARGET}
    INTERLEAVED
    PALETTE ${GAME_PALETTE}
    SOURCES ${OUTDOOR_TILESET_COLUMN_PNG}
    DESTINATIONS ${DATA_DIR}/tileset/outdoor.bm
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
