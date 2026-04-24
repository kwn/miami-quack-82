# Install script for directory: /Users/kwnuk/Amiga/ACE

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Users/kwnuk/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin/opt/bin/m68k-amiga-elf-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/kwnuk/Amiga/ace-scroller/build/_deps/bartman_gcc_support-build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/objects-Debug/ace" TYPE FILE FILES
    "src/ace/managers/advancedsprite.c.obj"
    "src/ace/managers/blit.c.obj"
    "src/ace/managers/bob.c.obj"
    "src/ace/managers/copper.c.obj"
    "src/ace/managers/game.c.obj"
    "src/ace/managers/joy.c.obj"
    "src/ace/managers/key.c.obj"
    "src/ace/managers/log.c.obj"
    "src/ace/managers/memory.c.obj"
    "src/ace/managers/mouse.c.obj"
    "src/ace/managers/ptplayer.c.obj"
    "src/ace/managers/rand.c.obj"
    "src/ace/managers/sprite.c.obj"
    "src/ace/managers/state.c.obj"
    "src/ace/managers/system.c.obj"
    "src/ace/managers/timer.c.obj"
    "src/ace/managers/viewport/camera.c.obj"
    "src/ace/managers/viewport/scrollbuffer.c.obj"
    "src/ace/managers/viewport/simplebuffer.c.obj"
    "src/ace/managers/viewport/tilebuffer.c.obj"
    "src/ace/utils/bitmap.c.obj"
    "src/ace/utils/bmframe.c.obj"
    "src/ace/utils/chunky.c.obj"
    "src/ace/utils/custom.c.obj"
    "src/ace/utils/dir.c.obj"
    "src/ace/utils/disk_file.c.obj"
    "src/ace/utils/extview.c.obj"
    "src/ace/utils/file.c.obj"
    "src/ace/utils/font.c.obj"
    "src/ace/utils/pak_file.c.obj"
    "src/ace/utils/palette.c.obj"
    "src/ace/utils/sprite.c.obj"
    "src/ace/utils/string.c.obj"
    "src/ace/utils/tag.c.obj"
    "src/fixmath/fix16.c.obj"
    "src/fixmath/fix16_exp.c.obj"
    "src/fixmath/fix16_sqrt.c.obj"
    "src/fixmath/fix16_str.c.obj"
    "src/fixmath/fix16_trig.c.obj"
    "src/fixmath/fract32.c.obj"
    "src/fixmath/uint32.c.obj"
    "src/mini_std/ctype.c.obj"
    "src/mini_std/errno.c.obj"
    "src/mini_std/intrin.c.obj"
    "src/mini_std/printf.c.obj"
    "src/mini_std/stdio_file.c.obj"
    "src/mini_std/stdio_putchar.c.obj"
    "src/mini_std/stdlib.c.obj"
    "src/mini_std/string.c.obj"
    "src/mini_std/strtoul.c.obj"
    FILES_FROM_DIR "/Users/kwnuk/Amiga/ace-scroller/build/ace/CMakeFiles/ace.dir/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/kwnuk/Amiga/ace-scroller/build/ace/CMakeFiles/ace.dir/install-cxx-module-bmi-Debug.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace" TYPE FILE FILES
    "/Users/kwnuk/Amiga/ACE/include/ace/macros.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/types.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace/generic" TYPE FILE FILES
    "/Users/kwnuk/Amiga/ACE/include/ace/generic/main.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/generic/screen.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace/utils" TYPE FILE FILES
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/bitmap.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/bmframe.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/chunky.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/custom.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/dir.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/disk_file.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/disk_file_private.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/endian.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/extview.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/file.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/font.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/mini_std.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/pak_file.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/palette.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/sprite.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/string.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/utils/tag.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace/managers" TYPE FILE FILES
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/advancedsprite.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/blit.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/bob.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/copper.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/game.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/joy.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/key.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/log.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/memory.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/mouse.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/ptplayer.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/rand.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/sprite.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/state.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/system.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/timer.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace/managers/viewport" TYPE FILE FILES
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/viewport/camera.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/viewport/scrollbuffer.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/viewport/simplebuffer.h"
    "/Users/kwnuk/Amiga/ACE/include/ace/managers/viewport/tilebuffer.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/fixmath" TYPE FILE FILES
    "/Users/kwnuk/Amiga/ACE/include/fixmath/fix16.h"
    "/Users/kwnuk/Amiga/ACE/include/fixmath/fix16_trig_sin_lut.h"
    "/Users/kwnuk/Amiga/ACE/include/fixmath/fixmath.h"
    "/Users/kwnuk/Amiga/ACE/include/fixmath/fract32.h"
    "/Users/kwnuk/Amiga/ACE/include/fixmath/int64.h"
    "/Users/kwnuk/Amiga/ACE/include/fixmath/uint32.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/kwnuk/Amiga/ace-scroller/build/ace/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
