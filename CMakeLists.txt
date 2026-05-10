cmake_minimum_required(VERSION 3.14.0)
project(game)

if(NOT AMIGA)
    message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)

# ---- ACE options (set before add_subdirectory) ----------------------------
set(ACE_DIR "" CACHE PATH "Path to ACE checkout")
option(GAME_USE_AGA "Build with ACE AGA features" ON)

if(NOT ACE_DIR)
    message(FATAL_ERROR "ACE_DIR must be provided, for example via run.sh or -DACE_DIR=/path/to/ACE")
endif()

if(NOT EXISTS "${ACE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "ACE_DIR does not point to an ACE checkout: ${ACE_DIR}")
endif()

if(GAME_USE_AGA)
    file(STRINGS
        ${ACE_DIR}/CMakeLists.txt
        ACE_AGA_FEATURE_LINES
        REGEX "ACE_USE_AGA_FEATURES"
    )
    if(NOT ACE_AGA_FEATURE_LINES)
        message(FATAL_ERROR "GAME_USE_AGA is ON, but ACE_DIR does not look like an AGA-capable ACE checkout: ${ACE_DIR}")
    endif()
    set(ACE_USE_AGA_FEATURES ON CACHE BOOL "" FORCE)
endif()

set(ACE_SCROLLBUFFER_ENABLE_SCROLL_X ON  CACHE BOOL "" FORCE)
set(ACE_SCROLLBUFFER_ENABLE_SCROLL_Y ON  CACHE BOOL "" FORCE)
set(ACE_BOB_WRAP_Y                   ON  CACHE BOOL "" FORCE)
set(ACE_BOB_PRISTINE_BUFFER          ON  CACHE BOOL "" FORCE)

add_subdirectory("${ACE_DIR}" ace)
message(STATUS "[game] GAME_USE_AGA: '${GAME_USE_AGA}'")
message(STATUS "[game] ACE_DIR: '${ACE_DIR}'")

function(ensureAceTools)
    set(ACE_TOOLS_DIR ${ACE_DIR}/tools)
    set(ACE_TOOLS_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/ace-tools)
    set(ACE_REQUIRED_TOOLS palette_conv bitmap_conv font_conv)
    find_program(HOST_C_COMPILER cc REQUIRED)
    find_program(HOST_CXX_COMPILER c++ REQUIRED)

    if(EXISTS "${ACE_TOOLS_BUILD_DIR}/CMakeCache.txt")
        file(STRINGS
            "${ACE_TOOLS_BUILD_DIR}/CMakeCache.txt"
            ACE_TOOLS_CROSS_CACHE
            REGEX "m68k-amiga-elf"
        )
        if(ACE_TOOLS_CROSS_CACHE)
            file(REMOVE_RECURSE "${ACE_TOOLS_BUILD_DIR}")
        endif()
    endif()

    message(STATUS "[game] Ensuring ACE host tools")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S ${ACE_TOOLS_DIR}
            -B ${ACE_TOOLS_BUILD_DIR}
            -DCMAKE_C_COMPILER=${HOST_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${HOST_CXX_COMPILER}
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
set(GAME_OPT_FLAGS -O2 -fno-optimize-sibling-calls)
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
target_link_libraries(${GAME_TARGET} ace)

# ---- Asset conversion -------------------------------------------------------
set(RES_DIR ${CMAKE_CURRENT_LIST_DIR}/res)
set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)
set(BITMAPS_DIR ${RES_DIR}/bitmaps)
set(PALETTES_DIR ${RES_DIR}/palettes)
set(GAME_PALETTE_SOURCE ${RES_DIR}/palettes/game.gpl)
set(AIM_SPRITE_PNG ${DATA_DIR}/sprites/aim_hw.png)
set(AIM_SPRITE_PALETTE ${DATA_DIR}/sprites/aim.gpl)
set(PLAYER_MASKED_PNG ${DATA_DIR}/player/player_masked.png)
set(OUTDOOR_TILESET_COLUMN_PNG ${DATA_DIR}/tileset/outdoor_column.png)
set(WEAPON_NAMES knife gun shotgun sniper uzi m16 flamethrower minigun)

file(MAKE_DIRECTORY
    ${DATA_DIR}/bitmaps
    ${DATA_DIR}/palettes
    ${DATA_DIR}/player
    ${DATA_DIR}/sprites
    ${DATA_DIR}/tileset
    ${DATA_DIR}/weapons
    ${DATA_DIR}/fonts
)

getToolPath(palette_conv TOOL_PALETTE_CONV)
find_program(PYTHON3_EXECUTABLE python3 REQUIRED)

if(GAME_USE_AGA)
    set(GAME_PALETTE_MODE AGA)
    set(GAME_PALETTE_CONV_FLAGS -aga)
else()
    set(GAME_PALETTE_MODE OCS)
    set(GAME_PALETTE_CONV_FLAGS)
endif()

function(addPaletteAsset PALETTE_SOURCE PALETTE_DESTINATION)
    get_filename_component(PALETTE_DESTINATION_DIR ${PALETTE_DESTINATION} DIRECTORY)
    get_filename_component(PALETTE_NAME ${PALETTE_DESTINATION} NAME_WE)
    set(PALETTE_PREPARED_GPL ${PALETTE_DESTINATION_DIR}/${PALETTE_NAME}.gpl)
    file(MAKE_DIRECTORY ${PALETTE_DESTINATION_DIR})

    add_custom_command(
        OUTPUT ${PALETTE_PREPARED_GPL}
        COMMAND ${PYTHON3_EXECUTABLE}
            ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_palette.py
            --input ${PALETTE_SOURCE}
            --output ${PALETTE_PREPARED_GPL}
            --mode ${GAME_PALETTE_MODE}
        DEPENDS
            ${PALETTE_SOURCE}
            ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_palette.py
    )

    add_custom_command(
        OUTPUT ${PALETTE_DESTINATION}
        COMMAND ${TOOL_PALETTE_CONV}
            ${PALETTE_PREPARED_GPL}
            ${PALETTE_DESTINATION}
            ${GAME_PALETTE_CONV_FLAGS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${PALETTE_PREPARED_GPL}
    )
    target_sources(${GAME_TARGET} PUBLIC ${PALETTE_DESTINATION})
endfunction()

function(addPaletteResources PALETTE_SOURCE_DIR PALETTE_DESTINATION_DIR)
    file(GLOB PALETTE_SOURCES CONFIGURE_DEPENDS ${PALETTE_SOURCE_DIR}/*.gpl)
    foreach(PALETTE_SOURCE IN LISTS PALETTE_SOURCES)
        get_filename_component(PALETTE_NAME ${PALETTE_SOURCE} NAME_WE)
        addPaletteAsset(
            ${PALETTE_SOURCE}
            ${PALETTE_DESTINATION_DIR}/${PALETTE_NAME}.plt
        )
    endforeach()
endfunction()

function(addBitmapAsset PALETTE_SOURCE BITMAP_SOURCE BITMAP_DESTINATION)
    convertBitmaps(
        TARGET ${GAME_TARGET}
        PALETTE ${PALETTE_SOURCE}
        SOURCES ${BITMAP_SOURCE}
        DESTINATIONS ${BITMAP_DESTINATION}
    )
endfunction()

function(addBitmapResources BITMAP_SOURCE_DIR BITMAP_DESTINATION_DIR)
    file(GLOB BITMAP_SOURCES CONFIGURE_DEPENDS ${BITMAP_SOURCE_DIR}/*.png)
    foreach(BITMAP_SOURCE IN LISTS BITMAP_SOURCES)
        get_filename_component(BITMAP_NAME ${BITMAP_SOURCE} NAME_WE)
        set(BITMAP_PALETTE_SOURCE ${PALETTES_DIR}/${BITMAP_NAME}.gpl)
        set(BITMAP_CONVERSION_SOURCE ${BITMAP_SOURCE})
        if(NOT EXISTS ${BITMAP_PALETTE_SOURCE})
            set(BITMAP_PALETTE_SOURCE ${GAME_PALETTE_SOURCE})
            set(BITMAP_CONVERSION_SOURCE ${BITMAP_DESTINATION_DIR}/${BITMAP_NAME}.png)
            add_custom_command(
                OUTPUT ${BITMAP_CONVERSION_SOURCE}
                COMMAND ${PYTHON3_EXECUTABLE}
                    ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_game_palette_png.py
                    --input ${BITMAP_SOURCE}
                    --output ${BITMAP_CONVERSION_SOURCE}
                    --game-palette ${GAME_PALETTE_SOURCE}
                DEPENDS
                    ${BITMAP_SOURCE}
                    ${GAME_PALETTE_SOURCE}
                    ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_game_palette_png.py
            )
        endif()

        addBitmapAsset(
            ${BITMAP_PALETTE_SOURCE}
            ${BITMAP_CONVERSION_SOURCE}
            ${BITMAP_DESTINATION_DIR}/${BITMAP_NAME}.bm
        )
    endforeach()
endfunction()

function(addInterleavedBitmapAsset PALETTE_SOURCE BITMAP_SOURCE BITMAP_DESTINATION)
    convertBitmaps(
        TARGET ${GAME_TARGET}
        INTERLEAVED
        PALETTE ${PALETTE_SOURCE}
        SOURCES ${BITMAP_SOURCE}
        DESTINATIONS ${BITMAP_DESTINATION}
    )
endfunction()

function(addMaskedInterleavedBitmapAsset PALETTE_SOURCE BITMAP_SOURCE BITMAP_DESTINATION MASK_DESTINATION)
    convertBitmaps(
        TARGET ${GAME_TARGET}
        INTERLEAVED
        PALETTE ${PALETTE_SOURCE}
        MASK_COLOR "#FF00FF"
        SOURCES ${BITMAP_SOURCE}
        DESTINATIONS ${BITMAP_DESTINATION}
        MASKS ${MASK_DESTINATION}
    )
endfunction()

# ---- Game resources ---------------------------------------------------------

addBitmapResources(${BITMAPS_DIR} ${DATA_DIR}/bitmaps)

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

addInterleavedBitmapAsset(
    ${AIM_SPRITE_PALETTE}
    ${AIM_SPRITE_PNG}
    ${DATA_DIR}/sprites/aim.bm
)

add_custom_command(
    OUTPUT ${PLAYER_MASKED_PNG}
    COMMAND ${PYTHON3_EXECUTABLE}
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_masked_png.py
        --input ${RES_DIR}/player/player.png
        --output ${PLAYER_MASKED_PNG}
        --width 16
        --height 240
        --game-palette ${GAME_PALETTE_SOURCE}
    DEPENDS
        ${RES_DIR}/player/player.png
        ${GAME_PALETTE_SOURCE}
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_masked_png.py
)

addMaskedInterleavedBitmapAsset(
    ${GAME_PALETTE_SOURCE}
    ${PLAYER_MASKED_PNG}
    ${DATA_DIR}/player/player.bm
    ${DATA_DIR}/player/player_mask.bm
)

foreach(WEAPON_NAME IN LISTS WEAPON_NAMES)
    set(WEAPON_SOURCE_PNG ${RES_DIR}/weapons/weapon_${WEAPON_NAME}.png)
    set(WEAPON_MASKED_PNG ${DATA_DIR}/weapons/weapon_${WEAPON_NAME}_masked.png)
    if(EXISTS "${WEAPON_SOURCE_PNG}")
        add_custom_command(
            OUTPUT ${WEAPON_MASKED_PNG}
            COMMAND ${PYTHON3_EXECUTABLE}
                ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_masked_png.py
                --input ${WEAPON_SOURCE_PNG}
                --output ${WEAPON_MASKED_PNG}
                --width 16
                --height 144
                --game-palette ${GAME_PALETTE_SOURCE}
            DEPENDS
                ${WEAPON_SOURCE_PNG}
                ${GAME_PALETTE_SOURCE}
                ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_masked_png.py
        )

        addMaskedInterleavedBitmapAsset(
            ${GAME_PALETTE_SOURCE}
            ${WEAPON_MASKED_PNG}
            ${DATA_DIR}/weapons/weapon_${WEAPON_NAME}.bm
            ${DATA_DIR}/weapons/weapon_${WEAPON_NAME}_mask.bm
        )
    endif()
endforeach()

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
        --game-palette ${GAME_PALETTE_SOURCE}
    DEPENDS
        ${RES_DIR}/tileset/outdoor.png
        ${GAME_PALETTE_SOURCE}
        ${CMAKE_CURRENT_LIST_DIR}/cmake/prepare_tileset_column.py
)

addInterleavedBitmapAsset(
    ${GAME_PALETTE_SOURCE}
    ${OUTDOOR_TILESET_COLUMN_PNG}
    ${DATA_DIR}/tileset/outdoor.bm
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

addPaletteResources(${PALETTES_DIR} ${DATA_DIR}/palettes)
