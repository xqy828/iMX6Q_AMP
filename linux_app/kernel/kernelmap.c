#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "public.h"
#include "kernelmap.h"

#define sev()    __asm__("sev")
#define dmb()    __asm__ __volatile__ ("dmb" : : : "memory")            

#define PAGE_SIZE ((size_t)getpagesize())

KernelBusMapCtrl gKernelMmapCtrl = {0};

static void kernel_phys_to_virt(U32 ulPhyAddr,U32 ulsize,S32 slMemFd, U32 *pVirtAddr)
{
	U32 uloffset = ulPhyAddr % PAGE_SIZE;
	U32 ulPhyBase = ulPhyAddr - uloffset;
	U32 *ulVirtBase = NULL;
	ulVirtBase = mmap(NULL,ulsize,PROT_READ | PROT_WRITE,MAP_SHARED,slMemFd,ulPhyBase);
	if(ulVirtBase == NULL)
	{
		printf("phyaddr:0x%x mmap failed !\n",ulPhyAddr);
	}
	else
	{
		//debug(" phys to virt success !,offset:0x%08x , Page_Size:0x%08x ,MmapMemVirtAddr:0x%08x\n",uloffset,PAGE_SIZE,(U32)ulVirtBase + uloffset);
	}
	*pVirtAddr =((U32)ulVirtBase + uloffset);
}

void KernelMmapInit(void)
{
	S32 mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	U32 PAGE_CNT=0;
    if(mem_fd == -1)
	{
		perror("open /dev/mem failed !\n");
		return;
	}
	gKernelMmapCtrl.mem_fd = mem_fd;
    kernel_phys_to_virt(REGS_SRC_BASE,PAGE_SIZE,gKernelMmapCtrl.mem_fd,&gKernelMmapCtrl.memRegsSrcBase);
    /* MAX cpu3 app size is 10M */
    PAGE_CNT = SIZE_1M * 10 /PAGE_SIZE +1;
    kernel_phys_to_virt(CPU3APP_PHY_BASE,PAGE_SIZE*PAGE_CNT,gKernelMmapCtrl.mem_fd,&gKernelMmapCtrl.memCpu3PhyBase);

	close(mem_fd);
}

