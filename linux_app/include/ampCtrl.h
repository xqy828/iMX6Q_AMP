#ifndef __AMPCTRL_H__
#define __AMPCTRL_H__
#include "public.h"
#include "elf.h"

#define EI_NIDENT       (16)
typedef struct
{
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half  e_type;
    Elf32_Half  e_machine;
    Elf32_Word  e_version;
    Elf32_Addr  e_entry;
    Elf32_Off  e_phoff;
    Elf32_Off  e_shoff;
    Elf32_Word  e_flags;
    Elf32_Half  e_ehsize;
    Elf32_Half  e_phentsize;
    Elf32_Half  e_phnum;
    Elf32_Half  e_shentsize;
    Elf32_Half  e_shnum;
    Elf32_Half  e_shstrndx;
}ELF32_EHEADER_STU;

typedef struct 
{
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off   sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
}ELF_SECTIONS_HEADER_STU;


typedef struct
{
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
}ELF_PROGRAM_HEADER_STU;

#define ELF_PT_LOAD         (1)

U32 InitCpu3(void);
U32 getCpu3SectionAddr(const char* cpu3SectionName,UADDR *Cpu3SectionAddr);
U32 Cpu3PowerOn(void);
U32 Cpu3PowerOff(void);


#endif

