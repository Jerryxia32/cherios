/* NIST Secure Hash Algorithm */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<misc.h>
#include<object.h>
#include<namespace.h>
#include<cheric.h>
#include "sha.h"

void (*msg_methods[]) = {sha_stream, sha_print};
size_t msg_methods_nb = countof(msg_methods);
void (*ctrl_methods[]) = {NULL};
size_t ctrl_methods_nb = countof(ctrl_methods);

int main()
{
    printf("SHA Hello World.\n");
    act_self_PCC = cheri_seal(act_self_PCC, act_self_cap);
    act_self_IDC = cheri_seal(act_self_IDC, act_self_cap);
    int ret = namespace_register(6, act_self_ref, act_self_PCC, act_self_IDC);
    return_cap = namespace_get_IDC(7);
    if(ret!=0) {
        printf("SHA: register failed\n");
        return -1;
    }
    printf("SHA: register OK\n");
    msg_enable = 1;
    return(0);
}
