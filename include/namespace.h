#ifndef _NAMESPACE_H
#define _NAMESPACE_H

#include"mips.h"

#define PORT_UART 1
#define PORT_MEMMGT 3
#define PORT_AES 5
#define PORT_SHA 6
#define PORT_CCALL 7
#define PORT_QSORT 10

void namespace_init(aid_t ns_aid);
int	namespace_register(int nb, aid_t aid, void * __capability PCC,
        void * __capability IDC);
aid_t namespace_get_aid(int nb);
void * __capability	namespace_get_PCC(int nb);
void * __capability	namespace_get_IDC(int nb);

extern void * namespace_ref;
#endif // _NAMESPACE_H
