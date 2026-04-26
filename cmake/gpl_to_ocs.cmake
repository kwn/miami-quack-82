if(NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "INPUT and OUTPUT must be set")
endif()

file(READ "${INPUT}" GPL_CONTENT)
string(REPLACE "\r\n" "\n" GPL_CONTENT "${GPL_CONTENT}")
string(REPLACE "\r" "\n" GPL_CONTENT "${GPL_CONTENT}")
string(REPLACE "\n" ";" GPL_LINES "${GPL_CONTENT}")

set(OUT_CONTENT "")

foreach(LINE IN LISTS GPL_LINES)
    if(LINE MATCHES "^([ \t]*)([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)(.*)$")
        set(PREFIX "${CMAKE_MATCH_1}")
        set(R "${CMAKE_MATCH_2}")
        set(G "${CMAKE_MATCH_3}")
        set(B "${CMAKE_MATCH_4}")
        set(SUFFIX "${CMAKE_MATCH_5}")

        math(EXPR R4 "(${R} + 16) / 17")
        math(EXPR G4 "(${G} + 16) / 17")
        math(EXPR B4 "(${B} + 16) / 17")
        math(EXPR R8 "${R4} * 17")
        math(EXPR G8 "${G4} * 17")
        math(EXPR B8 "${B4} * 17")

        string(APPEND OUT_CONTENT "${PREFIX}${R8} ${G8} ${B8}${SUFFIX}\n")
    else()
        string(APPEND OUT_CONTENT "${LINE}\n")
    endif()
endforeach()

file(WRITE "${OUTPUT}" "${OUT_CONTENT}")
