/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//#if __has_include(<elf.h>)
    //#include<elf.h>
    #include "elf_common.h"
    #include "public.h"

/* ELF32 base types - 32-bit. */
typedef U32 Elf32_Addr;
typedef U16 Elf32_Half;
typedef U32 Elf32_Off;
typedef S32 Elf32_Sword;
typedef U32 Elf32_Word;

/* ELF64 base types - 64-bit. */
typedef U64 Elf64_Addr;
typedef U16 Elf64_Half;
typedef U64 Elf64_Off;
typedef S32 Elf64_Sword;
typedef U32 Elf64_Word;
typedef U64 Elf64_Xword;
typedef S64 Elf64_Sxword;

/* Size of ELF identifier field in the ELF file header. */
#define     EI_NIDENT       16

/* ELF32 file header */
typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf_32_Ehdr;

/* ELF64 file header */
typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf_64_Ehdr;

/* ELF32 program header */
typedef struct elf32_phdr {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf_32_Phdr;

/* ELF64 program header */
typedef struct elf64_phdr {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf_64_Phdr;

/* ELF32 section header. */
typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf_32_Shdr;

/* ELF64 section header. */
typedef struct {
	Elf64_Word sh_name;
	Elf64_Word sh_type;
	Elf64_Xword sh_flags;
	Elf64_Addr sh_addr;
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;
	Elf64_Word sh_link;
	Elf64_Word sh_info;
	Elf64_Xword sh_addralign;
	Elf64_Xword sh_entsize;
} Elf_64_Shdr;

/* ELF decoding information */
struct elf32_info {
	Elf_32_Ehdr ehdr;
	Elf_32_Phdr *phdrs;
	Elf_32_Shdr *shdrs;
	void *shstrtab;
};

struct elf64_info {
	Elf_64_Ehdr ehdr;
	Elf_64_Phdr *phdrs;
	Elf_64_Shdr *shdrs;
	void *shstrtab;
};

int elf_identify(const void *img_data, size_t len);
int elf_load_header(const void *img_data,void **img_info);
int elf_load(const void *img_data,void **img_info);
UADDR elf_get_entry(void *img_info);
int elf_locate_rsc_table(void *img_info, UADDR *da,
			 size_t *offset, size_t *size);
int elf_is_64(const void *elf_info);
const char * get_elf_class (unsigned int elf_class);
const char * get_data_encoding (unsigned int encoding);
const char * get_osabi_name (unsigned e_machine, unsigned int osabi);
char * get_file_type (unsigned type);
char *get_machine_name (unsigned e_machine);
const char *get_section_type_name (unsigned int sh_type);
const char *get_segment_type (unsigned e_machine, unsigned long p_type);
char *get_machine_flags (unsigned e_flags, unsigned e_machine);
void *elf_get_section_from_name(void *elf_info, const char *name);



//#else
#ifndef ELF_LOADER_H_
#define ELF_LOADER_H_
#include "public.h"
#if defined __cplusplus
extern "C" {
#endif

#define     ELFMAG          "\177ELF"
#define     SELFMAG         4

/* Old ABI (ie GNU pre EABI).  These are deprecated.  */
#define EF_ARM_RELEXEC     0x01
#define EF_ARM_INTERWORK   0x04
#define EF_ARM_APCS_26     0x08
#define EF_ARM_APCS_FLOAT  0x10
#define EF_ARM_PIC         0x20
#define EF_ARM_ALIGN8	   0x40		/* 8-bit structure alignment is in use.  */
#define EF_ARM_NEW_ABI     0x80
#define EF_ARM_OLD_ABI     0x100
#define EF_ARM_SOFT_FLOAT  0x200
#define EF_ARM_VFP_FLOAT   0x400
#define EF_ARM_MAVERICK_FLOAT 0x800

/* Frame unwind information */
#define PT_ARM_EXIDX (PT_LOPROC + 1)

/* Old ARM ELF spec. version B-01.  Mostly deprecated.  */
#define EF_ARM_SYMSARESORTED 0x04	/* NB conflicts with EF_INTERWORK.  */
#define EF_ARM_DYNSYMSUSESEGIDX 0x08	/* NB conflicts with EF_APCS26.  */
#define EF_ARM_MAPSYMSFIRST 0x10	/* NB conflicts with EF_APCS_FLOAT.  */

/* New constants defined in the ARM ELF spec. version XXX (AAELF).
   Only valid in conjunction with EF_ARM_EABI_VER5. */
#define EF_ARM_ABI_FLOAT_SOFT 0x200	/* NB conflicts with EF_ARM_SOFT_FLOAT.  */
#define EF_ARM_ABI_FLOAT_HARD 0x400	/* NB conflicts with EF_ARM_VFP_FLOAT.  */

/* Constants defined in AAELF.  */
#define EF_ARM_BE8	    0x00800000
#define EF_ARM_LE8	    0x00400000

#define EF_ARM_EABIMASK      0xFF000000
#define EF_ARM_EABI_VERSION(flags) ((flags) & EF_ARM_EABIMASK)
#define EF_ARM_EABI_UNKNOWN  0x00000000
#define EF_ARM_EABI_VER1     0x01000000
#define EF_ARM_EABI_VER2     0x02000000
#define EF_ARM_EABI_VER3     0x03000000
#define EF_ARM_EABI_VER4     0x04000000
#define EF_ARM_EABI_VER5     0x05000000

/* Local aliases for some flags to match names used by COFF port.  */
#define F_INTERWORK	   EF_ARM_INTERWORK
#define F_APCS26	   EF_ARM_APCS_26
#define F_APCS_FLOAT	   EF_ARM_APCS_FLOAT
#define F_PIC              EF_ARM_PIC
#define F_SOFT_FLOAT	   EF_ARM_SOFT_FLOAT
#define F_VFP_FLOAT	   EF_ARM_VFP_FLOAT

/* Additional symbol types for Thumb.  */
#define STT_ARM_TFUNC      STT_LOPROC   /* A Thumb function.  */
#define STT_ARM_16BIT      STT_HIPROC   /* A Thumb label.  */

/* Additional section types.  */
#define SHT_ARM_EXIDX	       0x70000001	/* Section holds ARM unwind info.  */
#define SHT_ARM_PREEMPTMAP     0x70000002	/* Section pre-emption details.  */
#define SHT_ARM_ATTRIBUTES     0x70000003	/* Section holds attributes.  */
#define SHT_ARM_DEBUGOVERLAY   0x70000004	/* Section holds overlay debug info.  */
#define SHT_ARM_OVERLAYSECTION 0x70000005	/* Section holds GDB and overlay integration info.  */

/* ARM-specific values for sh_flags.  */
#define SHF_ENTRYSECT      0x10000000   /* Section contains an entry point.  */
#define SHF_ARM_PURECODE   0x20000000   /* Section contains only code and no data.  */
#define SHF_COMDEF         0x80000000   /* Section may be multiply defined in the input to a link step.  */

/* ARM-specific program header flags.  */
#define PF_ARM_SB          0x10000000   /* Segment contains the location addressed by the static base.  */
#define PF_ARM_PI          0x20000000   /* Segment is position-independent.  */
#define PF_ARM_ABS         0x40000000   /* Segment must be loaded at its base address.  */

/* Processor specific program header types.  */
#define PT_AARCH64_ARCHEXT	(PT_LOPROC + 0)

/* MTE memory tag segment type.  */
#define PT_AARCH64_MEMTAG_MTE     (PT_LOPROC + 0x2)

/* Additional section types.  */
#define SHT_AARCH64_ATTRIBUTES	0x70000003  /* Section holds attributes.  */

/* AArch64-specific values for sh_flags.  */
#define SHF_ENTRYSECT		0x10000000   /* Section contains an
						entry point.  */
#define SHF_COMDEF		0x80000000   /* Section may be multiply defined
						in the input to a link step.  */
/* Processor specific dynamic array tags.  */
#define DT_AARCH64_BTI_PLT	(DT_LOPROC + 1)
#define DT_AARCH64_PAC_PLT	(DT_LOPROC + 3)
#define DT_AARCH64_VARIANT_PCS	(DT_LOPROC + 5)

/* AArch64-specific values for st_other.  */
#define STO_AARCH64_VARIANT_PCS	0x80  /* Symbol may follow different call
					 convention from the base PCS.  */
#endif /* ELF_LOADER_H_ */
