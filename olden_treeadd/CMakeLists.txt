
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRCS
    src/init.S
    src/main.c
    src/args.c
    src/par-alloc.c
)

#execute_process(COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/large.pcm" "large.pcm")

add_cherios_executable(olden_treeadd ADD_TO_FILESYSTEM LINKER_SCRIPT sandbox.ld SOURCES ${SRCS})
