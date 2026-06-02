/****************************************************************************
* SPDX-License-Identifier: Apache-2.0
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.  The
* ASF licenses this file to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance with the
* License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
* License for the specific language governing permissions and limitations
* under the License.
*
****************************************************************************/

 /*******************************************************************//*
 * This file is derived from the NUTTX project with modifications.
 * /nuttx/sched/misc/coredump.c
 ********************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <elf32.h>
#include <elf.h>
#include "board_memory.h"
#include "generic_timer.h"

#define ALIGN_UP(x, align_to)    (((x) + ((align_to)-1)) & ~((align_to)-1))
#define ALIGN_DOWN(x, align_to) ((x) & ~((align_to)-1))

extern char __supervisor_stack_bottom[];// bottom 
extern char __supervisor_stack_top[];// top
#define COREDUMP_INFONAME_SIZE  ALIGN_UP(SUPERVISOR_STACK_SIZE, 8)

#define MIN(a,b)      (((a) < (b)) ? (a) : (b))
#define MAX(a,b)      (((a) > (b)) ? (a) : (b))
#define nitems(_a)    (sizeof(_a) / sizeof(0[(_a)]))
#define ELF_PRARGSZ    (80)
#define PROGRAM_ALIGNMENT 64
#define ELF_PAGESIZE    4096
#define COREDUMP_MAGIC  0x434f5245
#define EF_ARM_ABI_FLOAT_SOFT    0x00000200
#define EF_ARM_ABI_FLOAT_HARD    0x00000400
#define EF_ARM_EABI_VER5         0x05000000
#ifdef __VFP_FP__
#  ifdef __ARM_PCS_VFP
#    define EF_ARM_ABI_FLOAT     EF_ARM_ABI_FLOAT_HARD
#  else
#    define EF_ARM_ABI_FLOAT     EF_ARM_ABI_FLOAT_SOFT
#  endif
#else
#  define EF_ARM_ABI_FLOAT       0
#endif

#define EF_FLAG                  (EF_ARM_EABI_VER5 | EF_ARM_ABI_FLOAT)

typedef unsigned long elf_gregset_t[18];

typedef struct elf_prpsinfo_s
{
  char           pr_state;    /* Numeric process state */
  char           pr_sname;    /* Char for pr_state */
  char           pr_zomb;     /* Zombie */
  char           pr_nice;     /* Nice val */
  unsigned long  pr_flag;     /* Flags */
  unsigned short pr_uid;
  unsigned short pr_gid;
  int            pr_pid;
  int            pr_ppid;
  int            pr_pgrp;
  int            pr_sid;
  char           pr_fname[16];           /* Filename of executable */
  char           pr_psargs[ELF_PRARGSZ]; /* Initial part of arg list */
} elf_prpsinfo_t;

typedef struct elf_siginfo_s
{
  int            si_signo;    /* Signal number */
  int            si_code;     /* Extra code */
  int            si_errno;    /* Errno */
} elf_siginfo_t;

typedef struct elf_timeval_s
{
  long           tv_sec;      /* Seconds */
  long           tv_usec;     /* Microseconds */
} elf_timeval_t;

typedef struct elf_prstatus_s
{
  elf_siginfo_t  pr_info;     /* Info associated with signal */
  short          pr_cursig;   /* Current signal */
  short          pr_padding;  /* Padding align */
  unsigned long  pr_sigpend;  /* Set of pending signals */
  unsigned long  pr_sighold;  /* Set of held signals */
  int            pr_pid;
  int            pr_ppid;
  int            pr_pgrp;
  int            pr_sid;
  elf_timeval_t  pr_utime;    /* User time */
  elf_timeval_t  pr_stime;    /* System time */
  elf_timeval_t  pr_cutime;   /* Cumulative user time */
  elf_timeval_t  pr_cstime;   /* Cumulative system time */
  elf_gregset_t  pr_regs;
  int            pr_fpvalid;  /* True if math co-processor being used */
} elf_prstatus_t;

struct memory_region_s
{
  unsigned long int start;   /* Start address of this region */
  unsigned long int end;     /* End address of this region */
  unsigned int  flags;   /* Figure 5-3: Segment Flag Bits: PF_[X|W|R] */
};

struct dump_save_region
{
  unsigned long int start;   /* Start address of this region */
  unsigned long int end;     /* End address of this region */
  unsigned int offset;
};

struct elf_dumpinfo_s
{
  const struct memory_region_s *regions;
  struct dump_save_region *dump_regions;
};
 
struct coredump_info_s
{
  struct timespec time;
  size_t          size;
};

static struct memory_region_s g_memory_region[] =
{
  {DDR_ORG,DDR_LEN,PF_R|PF_W|PF_X},
};

static const struct memory_region_s *g_regions;

static int elf_emit(struct elf_dumpinfo_s *cinfo,const void *buf, size_t len)
{
  const uint8_t *ptr = buf;
  size_t total = len;
  memcpy((void*)cinfo->dump_regions->start + cinfo->dump_regions->offset,ptr,total);
  cinfo->dump_regions->offset = total;
  return 0;
}

static int elf_get_note_size(int stksegs)
{
  int total;
  total  = stksegs * (sizeof(Elf_Nhdr) + COREDUMP_INFONAME_SIZE + sizeof(elf_prstatus_t));
  total += stksegs * (sizeof(Elf_Nhdr) + COREDUMP_INFONAME_SIZE + sizeof(elf_prpsinfo_t));
  return total;
}

static int elf_get_info_note_size(void)
{
  return sizeof(Elf_Nhdr) + COREDUMP_INFONAME_SIZE + sizeof(struct coredump_info_s);
}

static void elf_emit_tcb_phdr(struct elf_dumpinfo_s *cinfo,Elf_Phdr *phdr, off_t *offset)
{
  unsigned long int sp = 0;
  phdr->p_vaddr = 0;

  //sp = up_getusrsp(tcb->xcp.regs);  // to do  
  if (sp > (unsigned long int)__supervisor_stack_top &&
      sp < (unsigned long int)__supervisor_stack_bottom)
  {
    phdr->p_filesz = (unsigned long int)__supervisor_stack_bottom - sp;
    phdr->p_vaddr  = sp;
  }

  if (phdr->p_vaddr == 0)
  {
    phdr->p_vaddr  = (unsigned long int)__supervisor_stack_top;
    phdr->p_filesz = (unsigned long int)__supervisor_stack_bottom - (unsigned long int)__supervisor_stack_top;
  }

  sp = ALIGN_DOWN(phdr->p_vaddr, PROGRAM_ALIGNMENT);
  phdr->p_filesz = ALIGN_UP(phdr->p_filesz + (phdr->p_vaddr - sp), PROGRAM_ALIGNMENT);
  phdr->p_vaddr  = sp;

  phdr->p_type   = PT_LOAD;
  phdr->p_offset = ALIGN_UP(*offset, ELF_PAGESIZE);
  phdr->p_paddr  = phdr->p_vaddr;
  phdr->p_memsz  = phdr->p_filesz;
  phdr->p_flags  = PF_X | PF_W | PF_R;
  *offset       += ALIGN_UP(phdr->p_memsz, ELF_PAGESIZE);

  elf_emit(cinfo, phdr, sizeof(*phdr));
}

static int elf_emit_align(struct elf_dumpinfo_s *cinfo)
{
  off_t align = ALIGN_UP(cinfo->dump_regions->offset,ELF_PAGESIZE) - cinfo->dump_regions->offset;
  unsigned char null[256];
  off_t total = align;
  off_t ret = 0;

  memset(null, 0, sizeof(null));

  while (total > 0)
    {
      ret = elf_emit(cinfo, null, total > sizeof(null) ? sizeof(null) : total);
      if (ret <= 0)
        {
          break;
        }

      total -= ret;
    }

  return ret < 0 ? ret : align;
}

static int elf_emit_hdr(struct elf_dumpinfo_s *cinfo,int segs)
{
  Elf_Ehdr ehdr;

  memset(&ehdr, 0, sizeof(ehdr));
  memcpy(ehdr.e_ident, ELFMAG, EI_MAGIC_SIZE);

  ehdr.e_ident[EI_CLASS]   = ELF_CLASS;
  ehdr.e_ident[EI_DATA]    = ELF_DATA;
  ehdr.e_ident[EI_VERSION] = EV_CURRENT;
  ehdr.e_ident[EI_OSABI]   = ELF_OSABI;

  ehdr.e_type              = ET_CORE;
  ehdr.e_machine           = EM_ARM;
  ehdr.e_version           = EV_CURRENT;
  ehdr.e_phoff             = sizeof(Elf_Ehdr);
  ehdr.e_flags             = EF_FLAG;
  ehdr.e_ehsize            = sizeof(Elf_Ehdr);
  ehdr.e_phentsize         = sizeof(Elf_Phdr);
  ehdr.e_phnum             = segs;

  return elf_emit(cinfo, &ehdr, sizeof(ehdr));
}

static void elf_emit_phdr(struct elf_dumpinfo_s *cinfo,int stksegs, int memsegs)
{
  off_t offset = cinfo->dump_regions->offset + (stksegs + memsegs + 1 + 1) * sizeof(Elf_Phdr);
  Elf_Phdr phdr;
  int i;

  memset(&phdr, 0, sizeof(Elf_Phdr));

  phdr.p_type   = PT_NOTE;
  phdr.p_offset = offset;
  phdr.p_filesz = elf_get_note_size(stksegs);
  offset       += phdr.p_filesz;

  elf_emit(cinfo, &phdr, sizeof(phdr));

  phdr.p_align  = ELF_PAGESIZE;
  elf_emit_tcb_phdr(cinfo,&phdr, &offset);

  /* Write program headers for segments dump */

  for (i = 0; i < memsegs; i++)
  {
    phdr.p_type   = PT_LOAD;
    phdr.p_offset = ALIGN_UP(offset, ELF_PAGESIZE);
    phdr.p_vaddr  = cinfo->regions[i].start;
    phdr.p_paddr  = phdr.p_vaddr;
    phdr.p_filesz = cinfo->regions[i].end - cinfo->regions[i].start;
    phdr.p_memsz  = phdr.p_filesz;
    phdr.p_flags  = cinfo->regions[i].flags;
    offset       += ALIGN_UP(phdr.p_memsz, ELF_PAGESIZE);
    elf_emit(cinfo, &phdr, sizeof(phdr));
  }

  memset(&phdr, 0, sizeof(Elf_Phdr));
  phdr.p_type   = PT_NOTE;
  phdr.p_offset = ALIGN_UP(offset, ELF_PAGESIZE);
  phdr.p_filesz = elf_get_info_note_size();
  offset       += phdr.p_filesz;

  elf_emit(cinfo, &phdr, sizeof(phdr));
}

static void elf_emit_tcb_note(struct elf_dumpinfo_s *cinfo)
{
  char name[16];
  elf_prstatus_t status;
  elf_prpsinfo_t info;
  unsigned long int *regs;
  Elf_Nhdr nhdr;
  int i;
  size_t pad;

  memset(name,   0x0, sizeof(name));
  memset(&info,   0x0, sizeof(info));
  memset(&status, 0x0, sizeof(status));

  /* Fill Process info */

  nhdr.n_namesz = 5; // "core" + 1
  nhdr.n_descsz = sizeof(info);
  nhdr.n_type   = NT_PRPSINFO;

  elf_emit(cinfo, &nhdr, sizeof(nhdr));

  strlcpy(name,"CORE", sizeof(name));
  elf_emit(cinfo, name,nhdr.n_namesz);
  pad = ALIGN_UP(nhdr.n_namesz, 4) - nhdr.n_namesz;
  if (pad > 0)
  {
    static const uint8_t zero[3] = {0};
    elf_emit(cinfo, zero, pad);
  }

  info.pr_pid   = 0;
  strlcpy(info.pr_fname,"i.mx6q:", sizeof(info.pr_fname));
  elf_emit(cinfo, &info, sizeof(info));

  /* Fill Process status */
  nhdr.n_descsz = sizeof(status);
  nhdr.n_type   = NT_PRSTATUS;
  elf_emit(cinfo, &nhdr, sizeof(nhdr));
  elf_emit(cinfo, name, sizeof(name));
  status.pr_pid = 0;
/*
  up_saveusercontext(g_running_regs);
  regs = (unsigned long int *)g_running_regs;
  if (regs != NULL)
  {
    for (i = 0; i < MIN(nitems(status.pr_regs), 17); i++)// R0 - R15,CPSR
    {
      status.pr_regs[i] = *(unsigned long int *)((unsigned char *)regs + g_tcbinfo.reg_off.p[i]);
    }
  }
*/
  elf_emit(cinfo, &status, sizeof(status));
}

static void elf_emit_tcb_stack(struct elf_dumpinfo_s *cinfo)
{
  unsigned long int buf = 0;
  unsigned long int sp;
  size_t len;

  //sp = up_getusrsp(tcb->xcp.regs);  // to do  
  if (sp > (unsigned long int)__supervisor_stack_top &&
      sp < (unsigned long int)__supervisor_stack_bottom)
  {
    len = (unsigned long int)__supervisor_stack_bottom - sp;
    buf = sp;
  }

  if (buf == 0)
  {
    buf = (unsigned long int)__supervisor_stack_top;
    len = (unsigned long int)__supervisor_stack_bottom - (unsigned long int)__supervisor_stack_top;
  }

  sp  = ALIGN_DOWN(buf, PROGRAM_ALIGNMENT);
  len = ALIGN_UP(len + (buf - sp), PROGRAM_ALIGNMENT);
  buf = sp;

  elf_emit(cinfo, (void *)buf, len);
  /* Align to page */
  elf_emit_align(cinfo);
}

static void elf_emit_note(struct elf_dumpinfo_s *cinfo)
{
  elf_emit_tcb_note(cinfo);
}

static void elf_emit_stack(struct elf_dumpinfo_s *cinfo)
{
  elf_emit_tcb_stack(cinfo);
}

 
static void elf_emit_memory(struct elf_dumpinfo_s *cinfo, int memsegs)
{
  int i;
  for (i = 0; i < memsegs; i++)
  {
    if (cinfo->regions[i].flags & PF_REGISTER)
    {
      unsigned long int *start = (unsigned long int *)cinfo->regions[i].start;
      unsigned long int *end = (unsigned long int *)cinfo->regions[i].end;
      unsigned long int buf[64];
      size_t offset = 0;
      while (start < end)
      {
        buf[offset++] = *start++;
        if (offset % (sizeof(buf) / sizeof(unsigned long int)) == 0)
        {
          elf_emit(cinfo, buf, sizeof(buf));
          offset = 0;
        }
      }
      if (offset != 0)
      {
        elf_emit(cinfo, buf, offset * sizeof(unsigned long int));
      }
    }
    else
    {
      elf_emit(cinfo, ( void *)cinfo->regions[i].start,cinfo->regions[i].end - cinfo->regions[i].start);
    }
    /* Align to page */
    elf_emit_align(cinfo);
  }
}

static void elf_emit_info_note( struct elf_dumpinfo_s *cinfo)
{
  struct coredump_info_s info;
  Elf_Nhdr nhdr;
  char name[16];
  int pad = 0;
  memset(&info, 0x0, sizeof(info));
  memset(name, 0x0, sizeof(name));

  nhdr.n_namesz = 5;
  nhdr.n_descsz = sizeof(info);
  nhdr.n_type   = COREDUMP_MAGIC;

  elf_emit(cinfo, &nhdr, sizeof(nhdr));

  strlcpy(name, "CORE", sizeof(name));
  elf_emit(cinfo, name,nhdr.n_namesz);
  pad = ALIGN_UP(nhdr.n_namesz, 4) - nhdr.n_namesz;
  if (pad > 0)
  {
    static const uint8_t zero[3] = {0};
    elf_emit(cinfo, zero, pad);
  }

  info.size = cinfo->dump_regions->offset + sizeof(info);
  clock_gettime(&info.time);
  elf_emit(cinfo, &info, sizeof(info));
}

int coredump(const struct memory_region_s *regions)
{
    struct elf_dumpinfo_s cinfo;
    int memsegs = 0;
    int stksegs = 1; // NO RTOS now 
    cinfo.regions = regions;
    if (cinfo.regions != NULL)
    {
      for (; cinfo.regions[memsegs].start <
             cinfo.regions[memsegs].end; memsegs++);
    }
    /* 
    * Fill notes section, with additional one for program header,
    * and one for the core file info defined by NuttX.
    */
    elf_emit_hdr(&cinfo, stksegs + memsegs + 1 + 1);

    /* 
    * Fill all the program information about the process for the
    * notes.  This also sets up the file header.
    */
    elf_emit_phdr(&cinfo, stksegs, memsegs);
   /* Fill note information */

    elf_emit_note(&cinfo);

    /* Align to page */

    elf_emit_align(&cinfo);

    /* Dump stack */

    elf_emit_stack(&cinfo);

    /* Dump memory segments */

    if (memsegs > 0)
    {
        elf_emit_memory(&cinfo, memsegs);
    }

    /* Emit core info note */

    elf_emit_info_note(&cinfo);

    /* Flush the dump */

    //elf_flush(&cinfo);

    return 0;
}
