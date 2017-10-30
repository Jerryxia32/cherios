
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRCS
    src/init.S
    src/main.c
)

add_cherios_executable(qsort ADD_TO_FILESYSTEM LINKER_SCRIPT qsort.ld SOURCES ${SRCS})
