
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRCS
    src/init.S
    src/main.c
    src/sha.c
)

add_cherios_executable(sha ADD_TO_FILESYSTEM LINKER_SCRIPT sha.ld SOURCES ${SRCS})
set_target_properties(sha PROPERTIES COMPILE_FLAGS -mxgot)
