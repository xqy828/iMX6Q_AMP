#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include "elf_loader.h"
#include "ampCtrl.h"
#include "kernelmap.h"

#define PAGE_SIZE ((size_t)getpagesize())
#define MAX_CPU3_APP_SIZE  (2*1024*1024) /*  2M bytes */ 
#define Cpu3AppBinName  "/usr/local/app/cpu3_app.bin"
#define Cpu3AppElfName  "/usr/local/app/cpu3_app.elf"
#define MAX_CORE_COUNT (4)

unsigned char gCpu3AppBinData[MAX_CPU3_APP_SIZE] = {0};
unsigned char gCpu3AppElfData[MAX_CPU3_APP_SIZE] = {0};

char Cpu3DumpData[SIZE_1M] = {0};

struct elf32_info *gelf32_info = NULL;
struct elf64_info *gelf64_info = NULL;


typedef void (*cpu_entry_point_t)(void * arg);

typedef struct _core_startup_info {
    cpu_entry_point_t entry;    //!< Function to call after starting a core.
    void * arg;                 //!< Argument to pass core entry point.
} core_startup_info_t;

static int FileSize(char* filename)
{
    struct stat status;
    int filesize = 0;
    int rc = -1;
    if(filename == NULL)
    {
        printf("invalid param \n");
        return -1;
    }
    rc = stat(filename,&status);
    if(rc == -1)
    {
        perror("stat");
        return rc;
    }
    filesize = status.st_size;
    return filesize;
}

static int isFileExist(char *filename)
{
    int rc =-1;
    if(filename == NULL)
    {
        printf("invalid param \n");
        return -1;
    }
    rc = access(filename,F_OK);
    if(rc != 0)
    {
        perror("access");
    }
    return rc;
}

U32 Cpu3Elf_Load(void)
{
    int rc=0;
    int slFileSize = 0;
    U32 ulRdCnt  =0;
    FILE *pCpu3App = NULL;
    rc = isFileExist(Cpu3AppElfName);
    if(rc != 0)
    {
        printf("%s not found \r\n",Cpu3AppElfName);
        return RET_NOK;
    }
    if((slFileSize = FileSize(Cpu3AppElfName)) <= 0)
    {
        printf("%s file size: %d abnormal !\n",Cpu3AppElfName,slFileSize);
        return RET_NOK;
    }

    if(slFileSize > MAX_CPU3_APP_SIZE)
    {
        printf("%s file size %d is over %d limit !\n",Cpu3AppElfName,slFileSize,MAX_CPU3_APP_SIZE);
        return RET_NOK;
    }

    if((pCpu3App = fopen(Cpu3AppElfName,"r")) == NULL)
    {
        printf("open cpu3 elf app failed \r\n");
        return RET_NOK;
    }
    ulRdCnt = fread(gCpu3AppElfData,1,slFileSize,pCpu3App);
    printf("load %s to Stream buf , size %d. \r\n",Cpu3AppElfName,ulRdCnt);
    rc = elf_identify(gCpu3AppElfData,slFileSize);
    if(rc != RET_OK)
    {
        printf("cpu3 elf load failed \r\n");
        return RET_NOK;
    }
    if (elf_is_64(gCpu3AppElfData) == 0)
    {
        rc = elf_load(gCpu3AppElfData,(void*)&gelf32_info);
    }
    else
    {
        rc = elf_load(gCpu3AppElfData,(void*)&gelf64_info);
    }
    return RET_OK;
}


static void getElfEntry(UADDR *pElfEntryPoint)
{
    if (elf_is_64(gCpu3AppElfData) == 0)
    {
        *pElfEntryPoint = gelf32_info->ehdr.e_entry;
    }
    else
    {
        *pElfEntryPoint = gelf64_info->ehdr.e_entry;
    }
}

static void  showElf32HeaderProcess(void *elf_info)
{
    U32 i = 0;
    struct elf32_info *elfinfo = NULL;
    Elf_32_Shdr * section = NULL;
    Elf_32_Phdr * segment = NULL;
    if(elf_info != NULL)    
    {
        elfinfo = (struct elf32_info*)elf_info;
    }
    else
    {
        printf("input parameter is null \r\n");
        return;
    }
    printf (("ELF Header:\n"));
    printf (("  Magic:   "));
    for(i = 0; i < EI_NIDENT; ++i) 
    {
        printf("%2.2x ", elfinfo->ehdr.e_ident[i]);
    }
    printf("\n");
    printf (("  Class:                             %s\n"),get_elf_class (elfinfo->ehdr.e_ident[EI_CLASS]));
    
    printf (("  Data:                              %s\n"),get_data_encoding (elfinfo->ehdr.e_ident[EI_DATA]));
    printf (("  Version:                           %d%s\n"),elfinfo->ehdr.e_ident[EI_VERSION],\
	      (elfinfo->ehdr.e_ident[EI_VERSION] == EV_CURRENT ? (" (current)") : \
              (elfinfo->ehdr.e_ident[EI_VERSION] != EV_NONE ? (" <unknown>") : "")));
    printf (("  OS/ABI:                            %s\n"),get_osabi_name (elfinfo->ehdr.e_machine,elfinfo->ehdr.e_ident[EI_OSABI]));
    printf (("  ABI Version:                       %d\n"),elfinfo->ehdr.e_ident[EI_ABIVERSION]);
    printf (("  Type:                              %s\n"),get_file_type (elfinfo->ehdr.e_type));
    printf (("  Machine:                           %s\n"),get_machine_name (elfinfo->ehdr.e_machine));
    printf (("  Version:                           0x%x\n"),elfinfo->ehdr.e_version);
    printf (("  Entry point address:               0x%x\n"),elfinfo->ehdr.e_entry);
    printf (("  Start of program headers:          "));
    printf ("%u" ,elfinfo->ehdr.e_phoff);
    printf ((" (bytes into file)\n  Start of section headers:          "));
    printf ("%u" ,elfinfo->ehdr.e_shoff);
    printf ((" (bytes into file)\n"));
    printf (("  Size of this header:               %u (bytes)\n"),elfinfo->ehdr.e_ehsize);
    printf (("  Size of program headers:           %u (bytes)\n"),elfinfo->ehdr.e_phentsize);
    printf (("  Number of program headers:         %u"),elfinfo->ehdr.e_phnum);
    printf (("  Size of section headers:           %u (bytes)\n"),elfinfo->ehdr.e_shentsize);
    printf (("  Number of section headers:         %u\n"),elfinfo->ehdr.e_shnum);
    printf (("  Section header string table index: %u"),elfinfo->ehdr.e_shstrndx);
    printf (("\nSection Headers:\n"));

    printf(("  [Nr]  Name             Type             Addr       Off        Size       ES   Flg Lk Inf Al\n"));
    for (i = 0, section = elfinfo->shdrs;i < elfinfo->ehdr.e_shnum;i++, section++)
    {
        printf ("  [%2u] ", i);
        printf("%-17s",&((char*)elfinfo->shstrtab)[section->sh_name]);
        printf (" %-15.15s ",get_section_type_name(section->sh_type));
        printf ( " 0x%8.8lx 0x%8.8lx 0x%8.8lx 0x%2.2lx",(unsigned long) section->sh_addr,(unsigned long) section->sh_offset,\
                  (unsigned long) section->sh_size,\
		   (unsigned long) section->sh_entsize);
        printf("\n");
    }
    printf (("\nProgram Headers:\n"));
    printf(("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n"));
    
    for (i = 0, segment = elfinfo->phdrs;i < elfinfo->ehdr.e_phnum;i++, segment++)
    {
        printf ("  %-14.14s ", get_segment_type (elfinfo->ehdr.e_machine, segment->p_type));
        printf ("0x%6.6lx ", (unsigned long) segment->p_offset);
        printf ("0x%8.8lx ", (unsigned long) segment->p_vaddr);
        printf ("0x%8.8lx ", (unsigned long) segment->p_paddr);
        printf ("0x%5.5lx ", (unsigned long) segment->p_filesz);
        printf ("0x%5.5lx ", (unsigned long) segment->p_memsz);
        printf ("%c%c%c ",(segment->p_flags & PF_R ? 'R' : ' '),(segment->p_flags & PF_W ? 'W' : ' '),\
		      (segment->p_flags & PF_X ? 'E' : ' '));
        printf ("%#lx", (unsigned long) segment->p_align);
        printf("\n");
    }
}

static void  showElf64HeaderProcess(void *elf_info)
{
    U32 i = 0;
    struct elf64_info *elfinfo = NULL;
    Elf_64_Shdr * section = NULL;
    Elf_64_Phdr * segment = NULL;
    if(elf_info != NULL)    
    {
        elfinfo = (struct elf64_info*)elf_info;
    }
    else
    {
        printf("input parameter is null \r\n");
        return;
    }
    printf (("ELF Header:\n"));
    printf (("  Magic:   "));
    for(i = 0; i < EI_NIDENT; ++i) 
    {
        printf("%2.2x ", elfinfo->ehdr.e_ident[i]);
    }
    printf("\n");
    printf (("  Class:                             %s\n"),get_elf_class (elfinfo->ehdr.e_ident[EI_CLASS]));
    
    printf (("  Data:                              %s\n"),get_data_encoding (elfinfo->ehdr.e_ident[EI_DATA]));
    printf (("  Version:                           %d%s\n"),elfinfo->ehdr.e_ident[EI_VERSION],\
	      (elfinfo->ehdr.e_ident[EI_VERSION] == EV_CURRENT ? (" (current)") : \
              (elfinfo->ehdr.e_ident[EI_VERSION] != EV_NONE ? (" <unknown>") : "")));
    printf (("  OS/ABI:                            %s\n"),get_osabi_name (elfinfo->ehdr.e_machine,elfinfo->ehdr.e_ident[EI_OSABI]));
    printf (("  ABI Version:                       %d\n"),elfinfo->ehdr.e_ident[EI_ABIVERSION]);
    printf (("  Type:                              %s\n"),get_file_type (elfinfo->ehdr.e_type));
    printf (("  Machine:                           %s\n"),get_machine_name (elfinfo->ehdr.e_machine));
    printf (("  Version:                           0x%x\n"),elfinfo->ehdr.e_version);
    printf (("  Entry point address:               0x%llx\n"),elfinfo->ehdr.e_entry);
    printf (("\n  Start of program headers:          "));
    printf ("%llu" ,elfinfo->ehdr.e_phoff);
    printf ((" (bytes into file)\n  Start of section headers:          "));
    printf ("%llu" ,elfinfo->ehdr.e_shoff);
    printf ((" (bytes into file)\n"));
    printf (("  Size of this header:               %u (bytes)\n"),elfinfo->ehdr.e_ehsize);
    printf (("  Size of program headers:           %u (bytes)\n"),elfinfo->ehdr.e_phentsize);
    printf (("  Number of program headers:         %u"),elfinfo->ehdr.e_phnum);
    printf (("  Size of section headers:           %u (bytes)\n"),elfinfo->ehdr.e_shentsize);
    printf (("  Number of section headers:         %u\n"),elfinfo->ehdr.e_shnum);
    printf (("  Section header string table index: %u"),elfinfo->ehdr.e_shstrndx);
    printf (("\nSection Headers:\n"));

    printf(("  [Nr] Name              Type            Addr     Off    Size   \n"));
    for (i = 0, section = elfinfo->shdrs;i < elfinfo->ehdr.e_shnum;i++, section++)
    {
        printf ("  [%2u] ", i);
        printf("%s",&((char*)elfinfo->shstrtab)[section->sh_name]);
        printf (" %-15.15s ",get_section_type_name(section->sh_type));
        printf ( " %6.6lx %6.6lx %2.2lx",(unsigned long) section->sh_offset,(unsigned long) section->sh_size,\
		   (unsigned long) section->sh_entsize);
    }
    printf("\n");
    printf (("\nProgram Headers:\n"));
    printf(("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n"));
    
    for (i = 0, segment = elfinfo->phdrs;i < elfinfo->ehdr.e_phnum;i++, segment++)
    {
        printf ("  %-14.14s ", get_segment_type (elfinfo->ehdr.e_machine, segment->p_type));
        printf ("0x%6.6lx ", (unsigned long) segment->p_offset);
        printf ("0x%8.8lx ", (unsigned long) segment->p_vaddr);
        printf ("0x%8.8lx ", (unsigned long) segment->p_paddr);
        printf ("0x%5.5lx ", (unsigned long) segment->p_filesz);
        printf ("0x%5.5lx ", (unsigned long) segment->p_memsz);
        printf ("%c%c%c ",(segment->p_flags & PF_R ? 'R' : ' '),(segment->p_flags & PF_W ? 'W' : ' '),\
		      (segment->p_flags & PF_X ? 'E' : ' '));
        printf ("%#lx", (unsigned long) segment->p_align);
        printf("\n");
    }
}

U32 Cpu3ElfHeader_Show(void)
{    
    if (elf_is_64(gCpu3AppElfData) == 0)
    {
        showElf32HeaderProcess(gelf32_info);
    }
    else
    {
         showElf64HeaderProcess(gelf64_info);
    }
    return RET_OK;
}

U32 getCpu3SectionAddr(const char* cpu3SectionName,UADDR *Cpu3SectionAddr)
{
    Elf_32_Shdr *shdr_32= NULL;
    Elf_64_Shdr *shdr_64 = NULL;
    if(cpu3SectionName == NULL)
    {
        printf("input parameter is null \r\n");
        return RET_NOK;
    }
    if (elf_is_64(gCpu3AppElfData) == 0)
    {
        shdr_32 = gelf32_info->shdrs;
        shdr_32 = elf_get_section_from_name(gelf32_info,cpu3SectionName);
        if(shdr_32 == NULL)
        {
            printf("can't get cpu3 section:%s. \r\n",cpu3SectionName);
            return RET_NOK;
        }
        *Cpu3SectionAddr = (UADDR)shdr_32->sh_addr;
    }
    else
    {
        shdr_64 = gelf64_info->shdrs;
        shdr_64 = elf_get_section_from_name(gelf64_info,cpu3SectionName);
        if(shdr_64 == NULL)
        {
            printf("can't get cpu3 section:%s. \r\n",cpu3SectionName);
            return RET_NOK;
        }
        *Cpu3SectionAddr = (UADDR)shdr_64->sh_addr;
    }

    return RET_OK;
}

U32 getCpu3DumpData(void)
{
    UADDR Cpu3DumpDataAddr = 0;
    volatile uint8_t *mm;
    S32 rc = 0;
    U32 PAGE_CNT = 0;
    U32 uloffset = 0 ; 
    U32 ulPhyBase = 0; 
    VU32 VirtAddr = 0;
    S32 mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(mem_fd < 0)
    {
    	fprintf(stderr, "open(/dev/mem) failed (%d)\n", errno);
    	return RET_NOK;
    }
    rc = getCpu3SectionAddr(".cpu3ddrdump",&Cpu3DumpDataAddr);
    if(rc != RET_OK)
    {
        printf("get cpu3 Dump Addr failed \r\n");
        return RET_NOK;
    }
    printf("cpu3 data dump addr:0x%lx\r\n",Cpu3DumpDataAddr);
    
    uloffset = Cpu3DumpDataAddr % PAGE_SIZE;
    ulPhyBase  = Cpu3DumpDataAddr - uloffset;

    PAGE_CNT = SIZE_1M / PAGE_SIZE + 1; 

    mm = mmap(NULL, PAGE_SIZE*PAGE_CNT, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, ulPhyBase);
    if (mm == MAP_FAILED) {
        fprintf(stderr, "mmap64(0x%x@0x%x) failed (%d:%s)\n",
                PAGE_SIZE*PAGE_CNT, (uint32_t)(Cpu3DumpDataAddr), errno,strerror(errno));
        return RET_NOK;
    }
    VirtAddr = (U32)mm + uloffset;
    printf("CPU3 Data Dump VirtAddr:0x%x \n", VirtAddr);
    memcpy(&Cpu3DumpData[0],(char*)VirtAddr,SIZE_1M*sizeof(char));
    msleep(1);
    munmap((void *)mm, PAGE_SIZE * PAGE_CNT);
    close(mem_fd);
    return RET_OK;
}


U32 Cpu3Elf2Bin(void)
{
    U32 ulCnt = 0;
    U32 ulRdCnt = 0;
    struct elf64_info *elf64info = NULL;
    struct elf32_info *elf32info = NULL;
    
    Elf_64_Phdr * segment_64 = NULL;
    Elf_32_Phdr * segment_32 = NULL;

    if (elf_is_64(gCpu3AppElfData) == 0)
    {
        elf32info = gelf32_info;
        for(ulCnt = 0,segment_32 = elf32info->phdrs ;ulCnt < elf32info->ehdr.e_phnum;ulCnt++,segment_32++)
        {
            if(segment_32->p_type == PT_LOAD)
            {
                memcpy(gCpu3AppBinData + ulRdCnt,gCpu3AppElfData + segment_32->p_offset,segment_32->p_filesz);
                ulRdCnt +=segment_32->p_filesz;
            }
        }
    }
    else
    {
        elf64info = gelf64_info;
        for(ulCnt = 0,segment_64 = elf64info->phdrs ;ulCnt < elf64info->ehdr.e_phnum;ulCnt++,segment_64++)
        {
            if(segment_64->p_type == PT_LOAD)
            {
                memcpy(gCpu3AppBinData + ulRdCnt,gCpu3AppElfData + segment_64->p_offset,segment_64->p_filesz);
                ulRdCnt +=segment_64->p_filesz;
            }
        }
    }

    return ulRdCnt;
}

U32 Cpu3_Load(void)
{
    int rc=0;
    U32 ulRdCnt  =0;
    rc = Cpu3Elf_Load();
    if(rc != RET_OK)
    {
        printf("%s file load abnormal !\n",Cpu3AppElfName);
        return RET_NOK;
    }

    ulRdCnt = Cpu3Elf2Bin();
    if(ulRdCnt == 0)
    {
        printf("%s convert abnormal !\n",Cpu3AppElfName);
        return RET_NOK;
    }
    printf("%s convert to bin success, bin size %d. \r\n",Cpu3AppElfName,ulRdCnt);
/*
    if((slFileSize = FileSize(Cpu3AppBinName)) <= 0)
    {
        printf("%s file size: %d abnormal !\n",Cpu3AppBinName,slFileSize);
        return RET_NOK;
    }

    if(slFileSize > MAX_CPU3_APP_SIZE)
    {
        printf("%s file size %d is over %d limit !\n",Cpu3AppBinName,slFileSize,MAX_CPU3_APP_SIZE);
        return RET_NOK;
    }

    if((pCpu3App = fopen(Cpu3AppBinName,"r")) == NULL)
    {
        printf("open cpu3 app failed \r\n");
        return RET_NOK;
    }

    ulRdCnt = fread((char*)gKernelMmapCtrl.memCpu3PhyBase,1,slFileSize,pCpu3App);
    printf("load cpu3 app to Stream buf , size %d. \r\n",ulRdCnt);
    printf("load %s success .\r\n",Cpu3AppBinName);
*/
    memcpy((char*)gKernelMmapCtrl.memCpu3PhyBase,&gCpu3AppBinData,ulRdCnt);
    printf("load cpu3 elf app success , size %d. \r\n",ulRdCnt);
    return RET_OK;
}


U32 Cpu3PowerOn(void)
{  
    HW_SRC_SCR.B.CORE3_ENABLE = 1;
    return RET_OK;
}

U32 Cpu3PowerOff(void)
{
    HW_SRC_SCR.B.CORE3_ENABLE = 0;
    return RET_OK;
}

U32 Cpu3StartRun(void)
{
    UADDR ullElfEntry = 0;
    UADDR ullElfMain = 0;
    getElfEntry(&ullElfEntry);
    getCpu3SectionAddr(".cpu3main",&ullElfMain);
    printf("cpu3 entry addr:0x%lx,entry arg:0x%lx \r\n",ullElfEntry,ullElfMain);
    HW_SRC_GPR7_WR((uint32_t)ullElfEntry);//CPU3 _start
    HW_SRC_GPR8_WR((uint32_t)ullElfMain);//CPU3 app main 
    Cpu3PowerOn();
    printf("cpu3 power on \r\n");
    return RET_OK;
}


U32 InitCpu3(void)
{
    int rc = RET_NOK;
    rc = Cpu3_Load();
    if(rc != RET_OK)
    {
        printf("CPU3 load failed \n");
        return RET_NOK;
    }
    usleep(200);
    Cpu3PowerOff();
    usleep(200);
    Cpu3StartRun();
    return RET_OK;
}
