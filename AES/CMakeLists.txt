
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRCS
    src/init.S
    src/main.c
    src/aes.c
)

add_cherios_executable(AES ADD_TO_FILESYSTEM LINKER_SCRIPT AES.ld SOURCES ${SRCS})
