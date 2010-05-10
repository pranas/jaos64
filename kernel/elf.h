#pragma once
#ifndef ELF_H
#define ELF_H

#define	ELF64_FSZ_ADDR	8
#define	ELF64_FSZ_HALF	2
#define	ELF64_FSZ_OFF	8
#define	ELF64_FSZ_SWORD	4
#define	ELF64_FSZ_WORD	4
#define	ELF64_FSZ_SXWORD 8
#define	ELF64_FSZ_XWORD	8

#include <stdint.h>
#include "memman.h"
#include "kheap.h"
#include "fat32.h"


typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef	uint32_t Elf64_Word;
typedef int32_t	Elf64_Sword;
typedef	uint64_t Elf64_Xword;
typedef int64_t	Elf64_Sxword;

#define	EI_NIDENT	16			/* Size of e_ident[] */

// elf header
typedef struct {
	unsigned char e_ident[EI_NIDENT];	/* ELF identification */
	Elf64_Half	e_type;			/* Object file type */
	Elf64_Half	e_machine;		/* Machine type */
	Elf64_Word	e_version;		/* Object file version */
	Elf64_Addr	e_entry;		/* Entry point address */
	Elf64_Off	e_phoff;		/* Program header offset */
	Elf64_Off	e_shoff;		/* Section header offset */
	Elf64_Word	e_flags;		/* Processor-specific flags */
	Elf64_Half	e_ehsize;		/* ELF header size */
	Elf64_Half	e_phentsize;	/* Size of program header entry */
	Elf64_Half	e_phnum;		/* Number of program header entries */
	Elf64_Half	e_shentsize;	/* Size of section header entry */
	Elf64_Half	e_shnum;		/* Number of section header entries */
	Elf64_Half	e_shstrndx;		/* Section name string table index */
} __attribute__((packed)) Elf64_Ehdr;

#define	EI_MAG0		0
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4			/* File class */
#define	EI_DATA		5			/* Data encoding */
#define	EI_VERSION	6			/* File version */
#define	EI_OSABI	7			/* Operating system/ABI identification */
#define	EI_ABIVERSION	8		/* ABI version */
#define	EI_PAD		9			/* Start of padding bytes */

// program header
typedef struct {
	Elf64_Word	p_type;		/* Type of segment */
	Elf64_Word	p_flags;	/* Segment attributes */
	Elf64_Off	p_offset;	/* Offset in file */
	Elf64_Addr	p_vaddr;	/* Virtual address in memory */
	Elf64_Addr	p_paddr;	/* Reserved */
	Elf64_Xword	p_filesz;	/* Size of segment in file */
	Elf64_Xword	p_memsz;	/* Size of segment in memory */
	Elf64_Xword	p_align;	/* Alignment of segment */
} __attribute__((packed)) Elf64_Phdr;

// section header
typedef struct {
	Elf64_Word	sh_name;	/* Section name */
	Elf64_Word	sh_type;	/* Section type */
	Elf64_Xword	sh_flags;	/* Section attributes */
	Elf64_Addr	sh_vaddr;	/* Virtual address in memory */
	Elf64_Off	sh_offset;	/* Offset in file */
	Elf64_Xword	sh_size;	/* Size of section */
	Elf64_Word	sh_link;	/* Link to other section */
	Elf64_Word	sh_info;	/* Misc info */
	Elf64_Xword	sh_addralign;	/* Address alignment boundary */
	Elf64_Xword	sh_entsize;	/* Size of entries, if section has table */
} __attribute__((packed)) Elf64_Shdr;

int elf_check(void* *src);

#endif