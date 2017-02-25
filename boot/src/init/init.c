/*-
 * Copyright (c) 2016 Hongyan Xia
 * Copyright (c) 2016 Robert N. M. Watson
 * Copyright (c) 2016 Hadrien Barral
 * Copyright (c) 2016 SRI International
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract FA8750-10-C-0237
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include"cheric.h"
#include "plat.h"
#include "misc.h"
#include "init.h"
#include "object.h"
#include "stdio.h"
#include"string.h"
#include"statcounters.h"

#define B_FS 1
#define B_SO 1
#define B_ZL 1
#define B_T1 0
#define B_T2 0
#define B_T3 0
#define B_MI 1

#define B_ENTRY(_type, _name, _arg, _daemon, _cond) \
	{_type,	_cond, _name, _arg, _daemon, 0, NULL},
#define B_DENTRY(_type, _name, _arg, _cond) \
	 B_ENTRY(_type, _name, _arg, 1, _cond)
#define B_PENTRY(_type, _name, _arg, _cond) \
	 B_ENTRY(_type, _name, _arg, 0, _cond)
#define B_FENCE \
	{m_fence, 1, NULL, 0, 0, 0, NULL},

init_elem_t init_list[] = {
	B_DENTRY(m_memmgt,	"memmgt.elf",		0, 	1)
	B_FENCE
	B_DENTRY(m_namespace,	"namespace.elf",	0,	1)
	B_FENCE
	//B_DENTRY(m_uart,	"uart.elf",		0,	1)
	B_DENTRY(m_core,	"AES.elf",		0, 	1)
	B_DENTRY(m_core,	"sha.elf",		0, 	1)
	B_FENCE
	B_PENTRY(m_user,	"qsort.elf",		0, 	B_MI)
	B_PENTRY(m_user,	"spam.elf",		0, 	B_MI)
	B_PENTRY(m_user,	"CRC32.elf",		0, 	B_MI)
	B_PENTRY(m_user,	"stringsearch.elf",		0, 	B_MI)
	B_PENTRY(m_user,	"bitcount.elf",		0, 	B_MI)
    B_PENTRY(m_user,	"adpcm.elf",		0, 	B_MI)
    B_PENTRY(m_user,	"dijkstra.elf",		0, 	B_MI)
	//B_DENTRY(m_core,	"sockets.elf",		0,	B_SO)
	//B_DENTRY(m_core,	"zlib.elf",		0,	B_ZL)
	//B_DENTRY(m_core,	"virtio-blk.elf",	0,	B_FS)
	//B_PENTRY(m_fs,		"fatfs.elf",		0,	B_FS)
	B_FENCE
	//B_PENTRY(m_user,	"hello.elf",		0,	1)
	//B_FENCE
	//B_PENTRY(m_user,	"prga.elf",		1,	B_SO)
	//B_PENTRY(m_user,	"prga.elf",		2,	B_SO)
	//B_PENTRY(m_user,	"zlib_test.elf",	0,	B_ZL)
	{m_fence, 0, NULL, 0, 0, 0, NULL}
};

const size_t init_list_len = countof(init_list);

extern size_t __init_fs_start, __init_fs_stop;

void print_build_date(void) {
	int filelen=0;
	char * date = load("t1", &filelen);
	if(date == NULL) {
		printf("%s failed\n", __func__);
		return;
	}
	date[filelen-1] = '\0';
	printf("%s\n", date);
}

static void load_modules(void) {
	static void * c_memmgt = NULL;

	for(size_t i=0; i<init_list_len; i++) {
		init_elem_t * be = init_list + i;
		if(be->cond == 0) {
			continue;
		}
		if(be->type == m_fence) {
			nssleep(3);
			continue;
		}
		be->ctrl = load_module(be->type, be->name, be->arg, NULL);
		printf(KWHT"Loaded module %s"KRST"\n", be->name);
		switch(init_list[i].type) {
		case m_memmgt:
			nssleep(3);
			c_memmgt = be->ctrl;
			init_alloc_enable_system(be->ctrl);
			break;
		case m_namespace:
			nssleep(3);
			/* glue memmgt to namespace */
			glue_memmgt(c_memmgt, be->ctrl);
			break;
		default:{}
		}
	}
}

int init_main() {
    act_self_cap = cheri_setoffset(cheri_getdefault(), 1);
    act_self_cap = cheri_setbounds(act_self_cap, 1);
    act_self_cap = cheri_andperm(act_self_cap, CHERI_PERM_SEAL);

	printf("Init loaded\n");
	printf("__init_fs_start: %p, __init_fs_stop: %p\n", (void *)__init_fs_start, (void *)__init_fs_stop);

	/* Initialize the memory pool. */
	init_alloc_init();

	/* Print fs build date */
	printf("Init:C\n");
	print_build_date();

	/* Load modules */
	printf("Init:F\n");
	load_modules();

	printf("Init:Z\n");

	while(acts_alive(init_list, init_list_len)) {
		ssleep(0);
	}

	printf(KBLD"Only daemons are alive. System shutown."KRST"\n");
	//stats_display();
	hw_reboot();
}
