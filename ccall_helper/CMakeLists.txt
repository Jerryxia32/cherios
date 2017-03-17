set(SRCS
    src/init.S
    src/main.c
)

add_cherios_executable(ccall_helper ADD_TO_FILESYSTEM LINKER_SCRIPT sandbox.ld SOURCES ${SRCS})
