set(LIB_SRCS
    ${CMAKE_SOURCE_DIR}/lib/memcpy_c.S
    ${CMAKE_SOURCE_DIR}/lib/memset.c
    ${CMAKE_SOURCE_DIR}/lib/strlen.c
    ${CMAKE_SOURCE_DIR}/lib/subr_prf.c
    ${CMAKE_SOURCE_DIR}/lib/crtbeginC.c
    ${CMAKE_SOURCE_DIR}/lib/crtendC.c
)

set(LIBUSER_ASM_SRCS
    src/init.S
    src/msg.S
    src/object.S
)

set(LIBUSER_SRCS
    src/assert.c
    src/ctype.c
    src/debug.c
    src/errno.c
    src/interrupt.c
    src/libuser.c
    src/malloc.c
    src/msg.c
    src/msg_default.c
    src/namespace.c
    src/object.c
    src/sched.c
    src/panic.c
    src/printf.c
    src/ssleep.c
    src/qsort.c
    src/statcounters.c
    src/stats.c
    src/stdio.c
    src/stdlib.c
    src/strcmp.c
)

add_library(user STATIC ${LIB_SRCS} ${LIBUSER_SRCS} ${LIBUSER_ASM_SRCS})
add_library(CheriOS::LibUser ALIAS user)
