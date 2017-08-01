/*-
 * Copyright (c) 2016 Hadrien Barral
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract (FA8750-10-C-0237)
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

#include "mips.h"
#include "stdarg.h"

typedef uint16_t Elf64_Half;	// Unsigned half int
typedef uint64_t Elf64_Off;	// Unsigned offset
typedef uint64_t Elf64_Addr;	// Unsigned address
typedef uint32_t Elf64_Word;	// Unsigned int
typedef int32_t  Elf64_Sword;	// Signed int
typedef uint64_t Elf64_Xword;	// Unsigned int
typedef int64_t  Elf64_Sxword;	// Signed int

typedef uint8_t     Elf_Byte;
typedef uint32_t    Elf32_Addr;    /* Unsigned program address */
typedef uint32_t    Elf32_Off;     /* Unsigned file offset */
typedef int32_t     Elf32_Sword;   /* Signed large integer */
typedef uint32_t    Elf32_Word;    /* Unsigned large integer */
typedef uint16_t Elf32_Half; /* Unsigned medium integer */

typedef struct {
	unsigned char	e_ident[16];	/*  ELF  identification  */
	Elf64_Half	e_type;		/*  Object  file  type  */
	Elf64_Half	e_machine;	/*  Machine  type  */
	Elf64_Word	e_version;	/*  Object  file  version  */
	Elf64_Addr	e_entry;	/*  Entry  point  address  */
	Elf64_Off	e_phoff;	/*  Program  header  offset  */
	Elf64_Off	e_shoff;	/*  Section  header  offset  */
	Elf64_Word	e_flags;	/*  Processor-specific  flags  */
	Elf64_Half	e_ehsize;	/*  ELF  header  size  */
	Elf64_Half	e_phentsize;	/*  Size  of  program  header  entry  */
	Elf64_Half	e_phnum;	/*  Number  of  program  header  entries  */
	Elf64_Half	e_shentsize;	/*  Size  of  section  header  entry  */
	Elf64_Half	e_shnum;	/*  Number  of  section  header  entries  */
	Elf64_Half	e_shstrndx;	/*  Section  name  string  table  index  */
}  Elf64_Ehdr;

typedef struct {
    unsigned char    e_ident[16];     /* ELF Identification */
    Elf32_Half       e_type;                 /* object file type */
    Elf32_Half       e_machine;              /* machine */
    Elf32_Word       e_version;              /* object file version */
    Elf32_Addr       e_entry;                /* virtual entry point */
    Elf32_Off        e_phoff;                /* program header table offset */
    Elf32_Off        e_shoff;                /* section header table offset */
    Elf32_Word       e_flags;                /* processor-specific flags */
    Elf32_Half       e_ehsize;               /* ELF header size */
    Elf32_Half       e_phentsize;            /* program header entry size */
    Elf32_Half       e_phnum;                /* number of program header entries */
    Elf32_Half       e_shentsize;            /* section header entry size */
    Elf32_Half       e_shnum;                /* number of section header entries */
    Elf32_Half       e_shstrndx;             /* section header table's "section
                                                header string table" entry offset */
} Elf32_Ehdr;

enum Elf_Ident {
	EI_MAG0		= 0, /* 0x7F */
	EI_MAG1		= 1, /* 'E' */
	EI_MAG2		= 2, /* 'L' */
	EI_MAG3		= 3, /* 'F' */
	EI_CLASS	= 4, /* Architecture (32/64) */
	EI_DATA		= 5, /* Byte Order */
	EI_VERSION	= 6, /* ELF Version */
	EI_OSABI	= 7, /* OS Specific */
	EI_ABIVERSION	= 8, /* OS Specific */
	EI_PAD		= 9, /* Padding */
	EI_NIDENT	= 16 /* Size of e_ident[] */
};

typedef struct {
	Elf64_Word	sh_name;	/*  Section  name  */
	Elf64_Word	sh_type;	/*  Section  type  */
	Elf64_Xword	sh_flags;	/*  Section  attributes  */
	Elf64_Addr	sh_addr;	/*  Virtual  address  in  memory  */
	Elf64_Off	sh_offset;	/*  Offset  in  file  */
	Elf64_Xword	sh_size;	/*  Size  of  section  */
	Elf64_Word	sh_link;	/*  Link  to  other  section  */
	Elf64_Word	sh_info;	/*  Miscellaneous  information  */
	Elf64_Xword	sh_addralign;	/*  Address  alignment  boundary  */
	Elf64_Xword	sh_entsize;	/*  Size  of  entries,  if  section  has  table  */
}  Elf64_Shdr;

typedef struct {
    Elf32_Word    sh_name;      /* name - index into section header
                                 * string table section */
    Elf32_Word    sh_type;      /* type */
    Elf32_Word    sh_flags;     /* flags */
    Elf32_Addr    sh_addr;      /* address */
    Elf32_Off     sh_offset;    /* file offset */
    Elf32_Word    sh_size;      /* section size */
    Elf32_Word    sh_link;      /* section header table index link */
    Elf32_Word    sh_info;      /* extra information */
    Elf32_Word    sh_addralign; /* address alignment */
    Elf32_Word    sh_entsize;   /* section entry size */
} Elf32_Shdr;

typedef struct {
	Elf64_Word	p_type;		/*  Type  of  segment  */
	Elf64_Word	p_flags;	/*  Segment  attributes  */
	Elf64_Off	p_offset;	/*  Offset  in  file  */
	Elf64_Addr	p_vaddr;	/*  Virtual  address  in  memory  */
	Elf64_Addr	p_paddr;	/*  Reserved  */
	Elf64_Xword	p_filesz;	/*  Size  of  segment  in  file  */
	Elf64_Xword	p_memsz;	/*  Size  of  segment  in  memory  */
	Elf64_Xword	p_align;	/*  Alignment  of  segment  */
}  Elf64_Phdr;

typedef struct {
    Elf32_Word    p_type;     /* segment type */
    Elf32_Off     p_offset;   /* segment offset */
    Elf32_Addr    p_vaddr;    /* virtual address of segment */
    Elf32_Addr    p_paddr;    /* physical address - ignored? */
    Elf32_Word    p_filesz;   /* number of bytes in file for seg. */
    Elf32_Word    p_memsz;    /* number of bytes in mem. for seg. */
    Elf32_Word    p_flags;    /* flags */
    Elf32_Word    p_align;    /* memory alignment */
} Elf32_Phdr;

/* Calling environment for loader */
typedef struct {
	void *(*alloc)(size_t size);
	void (*free)(void *addr);
	int (*printf)(const char *fmt, ...);
	int (*vprintf)(const char *fmt, va_list ap);
	void *(*memcpy)(void *dest, const void *src, size_t n);
} Elf_Env;

/* given pointer p to ELF image, returns a pointer to the loaded
   image.  if provided, it also sets the min and max addresses touched
   by the loader, and the entry point.
 */
void *elf_loader_mem(Elf_Env *env, void *p, size_t *minaddr, size_t *maxaddr, size_t *entry);
