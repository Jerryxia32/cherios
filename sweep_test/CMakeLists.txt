
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRCS
    src/init.S
    src/main.c
)

execute_process(COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/tags.bin" "tags.bin")

add_cherios_executable(sweep_test ADD_TO_FILESYSTEM LINKER_SCRIPT sandbox.ld SOURCES ${SRCS})
