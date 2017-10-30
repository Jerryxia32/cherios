
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRCS
    src/init.S
    src/bmhasrch.c
    src/bmhisrch.c
    src/bmhsrch.c
    src/main.c
)

add_cherios_executable(stringsearch ADD_TO_FILESYSTEM LINKER_SCRIPT stringsearch.ld SOURCES ${SRCS})
