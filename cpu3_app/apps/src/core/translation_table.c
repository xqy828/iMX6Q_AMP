/************************************************************************************************************************
----------------------------------System memory map----------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------   
Start Address       End Address  |  SIZE     |   Description                              |   Definition in Translation Table   
------------------------------------------------------------------------------------------------------------------------
0x1000_0000---------0xFFFF_FFFF  |  3840 MB  |   MMDC—DDR Controller.                     | Normal write-back Cacheable 
0x0800_0000---------0x0FFF_FFFF  |  128 MB   |   EIM—(NOR/SRAM)                           | Normal write-back Cacheable or Device memory (nor flash)
0x02A0_0000---------0x02DF_FFFF  |  4 MB     |   IPU-2                                    | Device Memory  
0x0260_0000---------0x029F_FFFF  |  4 MB     |   IPU-1                                    | Device Memory  
0x0220_C000---------0x023F_FFFF  |  2 MB     |   Reserved                                 | Unassigned
0x0220_8000---------0x0220_BFFF  |  16 KB    |   MIPI_HSI                                 | Device Memory  
0x0220_4000---------0x0220_7FFF  |  16 KB    |   OpenVG (GC355)                           | Device Memory  
0x0220_0000---------0x0220_3FFF  |  16 KB    |   SATA                                     | Device Memory  
0x0210_0000---------0x021F_FFFF  |  1 MB     |   Peripheral IPs via AIPS-2                | Device Memory  
0x0200_0000---------0x020F_FFFF  |  1 MB     |   Peripheral IPs via AIPS-1                | Device Memory  
0x01FF_C000---------0x01FF_FFFF  |  16 KB    |   PCIe registers                           | Device Memory  
0x0100_0000---------0x01FF_BFFF  |  16368 KB |   PCIe                                     | Device Memory  
0x00D0_0000---------0x00FF_FFFF  |  3072 KB  |   Reserved                                 | Unassigned
0x00C0_0000---------0x00CF_FFFF  |  1 MB     |   GPV_1 PL301 (fast1)                      | Device Memory  
0x00B0_0000---------0x00BF_FFFF  |  1 MB     |   GPV_0 PL301 (fast2) configuration port   | Device Memory  
0x00A0_3000---------0x00AF_FFFF  |  1012 KB  |   Reserved                                 | Unassigned
0x00A0_2000---------0x00A0_2FFF  |  4 KB     |   PL310 (L2 cache controller)              | Device Memory  
0x00A0_0000---------0x00A0_1FFF  |  8 KB     |   ARM MP                                   | Device Memory  
0x0094_0000---------0x009F_FFFF  |  0.75 MB  |   OCRAM aliased                            | Normal inner write-back cacheable
0x0090_0000---------0x0093_FFFF  |  0.25 MB  |   OCRAM 256 KB                             | Normal inner write-back cacheable
0x0080_0000---------0x008F_FFFF  |  1 MB     |   GPV_4 PL301 (fast3) configuration port   | Device Memory  
0x0040_0000---------0x007F_FFFF  |  4 MB     |   Reserved                                 | Unassigned
0x0030_0000---------0x003F_FFFF  |  1 MB     |   GPV_3 PL301 (per2) configuration port    | Device Memory  
0x0020_0000---------0x002F_FFFF  |  1 MB     |   GPV_2 PL301 (per1) configuration port    | Device Memory  
0x0013_C000---------0x001F_FFFF  |  784 KB   |   Reserved                                 | Unassigned
0x0013_8000---------0x0013_BFFF  |  16 KB    |   DTCP                                     | Device Memory
0x0013_4000---------0x0013_7FFF  |  16 KB    |   GPU 2D (GC320)                           | Device Memory
0x0013_0000---------0x0013_3FFF  |  16 KB    |   GPU 3D                                   | Device Memory
0x0012_9000---------0x0012_FFFF  |  28 KB    |   Reserved                                 | Unassigned
0x0012_0000---------0x0012_8FFF  |  36 KB    |   HDMI                                     | Device Memory
0x0011_8000---------0x0011_FFFF  |  32 KB    |   Reserved                                 | Unassigned
0x0011_4000---------0x0011_7FFF  |  16 KB    |   BCH                                      | Device Memory
0x0011_2000---------0x0011_3FFF  |  8 KB     |   GPMI                                     | Device Memory
0x0011_0000---------0x0011_1FFF  |  8 KB     |   APBH-Bridge-DMA                          | Device Memory
0x0010_4000---------0x0010_FFFF  |  48 KB    |   Reserved                                 | Unassigned
0x0010_0000---------0x0010_3FFF  |  16 KB    |   CAAM (16 KB secure RAM)                  | Device Memory
0x0001_8000---------0x000F_FFFF  |  928 KB   |   Reserved                                 | Unassigned
0x0000_0000---------0x0001_7FFF  |  96 KB    |   Boot ROM (ROMCP)                         | Device Memory
-----------------------------------------------------------------------------------------------------------------------------
*****************************************************************************************************************************/
#include "public.h"
#include "cortex_a9.h"
#include "mmu.h"
#include "asm_defines.h"
#include "arm_cache.h"

#define MMU_L1_TABLE_ITEM_NUM  (4096)       // L1 size is 4096 index * 4 bytes 
#define MMU_L2_TABLE_ITEM_NUM  (4096*256)   // each L2 size is 256 index * 4 bytes = 1k;

typedef union mmu_l1_entry
{
    unsigned int u;
    struct {
        unsigned int l1_attr:10;
        unsigned int l2_address:22;// Level 2 Descriptor Base Address
    };
} mmu_l1_entry_t;

typedef union mmu_l2_entry
{
    unsigned int u;
    struct {
        unsigned int l2_attr:12;
        unsigned int address:20;// Small Page Base Address
    };
} mmu_l2_entry_t;

typedef struct 
{
    unsigned int physicalStartAddress;
    unsigned int physicalEndAddress;
    unsigned int l1_attr;
    unsigned int l2_attr;
}PlatformMemoryMap;

PlatformMemoryMap gChipMemoryMap[] = {
    {0x00000000, 0x00017FFF, 0x001, 0x636},// Boot ROM (ROMCP) 
    {0x00018000, 0x000FFFFF, 0x001, 0x000},// Reserved
    {0x00100000, 0x00103FFF, 0x001, 0x436},// CAAM (16 KB secure RAM)
    {0x00104000, 0x0010FFFF, 0x000, 0x000},// Reserved
    {0x00110000, 0x00111FFF, 0x001, 0x436},// APBH-Bridge-DMA 
    {0x00112000, 0x00113FFF, 0x001, 0x436},// GPMI
    {0x00114000, 0x00117FFF, 0x001, 0x436},// BCH
    {0x00118000, 0x0011FFFF, 0x001, 0x000},// Reserved
    {0x00120000, 0x00128FFF, 0x001, 0x436},// HDMI
    {0x00129000, 0x0012FFFF, 0x001, 0x000},// Reserved
    {0x00130000, 0x00133FFF, 0x001, 0x436},// GPU 3D 
    {0x00134000, 0x00137FFF, 0x001, 0x436},// GPU 2D (GC320)
    {0x00138000, 0x0013BFFF, 0x001, 0x436},// DTCP
    {0x0013C000, 0x001FFFFF, 0x001, 0x000},// Reserved
    {0x00200000, 0x002FFFFF, 0x001, 0x436},// GPV_2 PL301 (per1) configuration port 
    {0x00300000, 0x003FFFFF, 0x001, 0x436},// GPV_3 PL301 (per2) configuration port
    {0x00400000, 0x007FFFFF, 0x001, 0x000},// Reserved
    {0x00800000, 0x008FFFFF, 0x001, 0x436},// GPV_4 PL301 (fast3) configuration port
    {0x00900000, 0x0093FFFF, 0x1e1, 0x13e},// OCRAM 256 KB 
    {0x00940000, 0x009FFFFF, 0x1e1, 0x13e},// OCRAM aliased
    {0x00A00000, 0x00A01FFF, 0x001, 0x436},// ARM MP
    {0x00A02000, 0x00A02FFF, 0x001, 0x436},// PL310 (L2 cache controller)
    {0x00A03000, 0x00AFFFFF, 0x001, 0x000},// Reserved
    {0x00B00000, 0x00BFFFFF, 0x001, 0x436},// GPV_0 PL301 (fast2) configuration port
    {0x00C00000, 0x00CFFFFF, 0x001, 0x436},// GPV_1 PL301 (fast1)
    {0x00D00000, 0x00FFFFFF, 0x001, 0x000},// Reserved
    {0x01000000, 0x01FFBFFF, 0x001, 0x436},// PCIe
    {0x01FFC000, 0x01FFFFFF, 0x001, 0x436},// PCIe registers
    {0x02000000, 0x020FFFFF, 0x001, 0x436},// Peripheral IPs via AIPS-1
    {0x02100000, 0x021FFFFF, 0x001, 0x436},// Peripheral IPs via AIPS-2 
    {0x02200000, 0x02203FFF, 0x001, 0x436},// SATA
    {0x02204000, 0x02207FFF, 0x001, 0x436},// OpenVG (GC355)
    {0x02208000, 0x0220BFFF, 0x001, 0x436},// MIPI_HSI
    {0x0220C000, 0x023FFFFF, 0x001, 0x000},// Reserved
    {0x02600000, 0x029FFFFF, 0x001, 0x436},// IPU-1
    {0x02A00000, 0x02DFFFFF, 0x001, 0x436},// IPU-2 
    {0x08000000, 0x0FFFFFFF, 0x001, 0x436},// EIM-(NOR/SDRAM)
    {0x10000000, 0xFFFFFFFF, 0x1e1, 0x576},// MMDC-DDR Controller
};


extern char __mmu_l1_tbl_start;
extern char __mmu_l2_tbl_start;

//uint32_t *l1_table = (uint32_t *)&__mmu_l1_tbl_start;
//uint32_t *l2_table = (uint32_t *)&__mmu_l2_tbl_start;

__attribute__ ((section (".mmu_l1_tbl"))) unsigned int l1_table[MMU_L1_TABLE_ITEM_NUM] = {[0 ... (MMU_L1_TABLE_ITEM_NUM - 1)] = 0x0};
__attribute__ ((section (".mmu_l2_tbl"))) unsigned int l2_table[MMU_L2_TABLE_ITEM_NUM] = {[0 ... (MMU_L2_TABLE_ITEM_NUM - 1)] = 0x0};

/*
 * create identical mapping, va = pa
 */
static void mmu_l1_l2_map(unsigned int pa_start, unsigned int va_start,unsigned int length,unsigned int l1_attr, unsigned int l2_attr)
{
    unsigned int i = 0;
    unsigned int l1_index = 0;
    unsigned int l2_index = 0;
    unsigned int va = 0;
    unsigned int pa = 0;
    unsigned int num_pages = 0;

    register mmu_l1_entry_t l1_entry = {0};
    register mmu_l2_entry_t l2_entry = {0};

    va = va_start;
    pa = pa_start;

    num_pages = length >> 12;// length / 4k 
    
    l1_entry.l1_attr = (l1_attr & 0x3ff);
    l2_entry.l2_attr = (l2_attr & 0xfff);

    for(i = 0;i < num_pages;i++)
    {
        l1_index = (va & 0xfff00000) >> 20;
        l2_index = (va & 0xff000) >> 12;
        
        l1_entry.l2_address = ((unsigned int)(&__mmu_l2_tbl_start) + l1_index * 256 * 4) >> 10;//bytes
        l1_table[l1_index] = l1_entry.u;
        
        l2_entry.address = (pa & 0xfffff000) >> 12;
        l2_table[l2_index + l1_index * 256] = l2_entry.u;
        pa = pa + 4*1024;//up 4k;     
        va = va + 4*1024;//up 4k;
    }
    
    return ;
}


void mmu_table_init(void)
{
#if 0
    /* 0x0000_0000---------0x0001_7FFF  |  96 KB    |   Boot ROM (ROMCP)                         |*/
    mmu_l1_l2_map(0x00000000, 0x00000000,0x1800,0x001,0x636);// device APX=1 AP=11 read only 
    /* 0x0001_8000---------0x000F_FFFF  |  928 KB   |   Reserved                                 |*/
    mmu_l1_l2_map(0x00018000, 0x00018000,0xe8000,0x0,0x0);
    /* 0x0010_0000---------0x0010_3FFF  |  16 KB    |   CAAM (16 KB secure RAM)                  |*/
    mmu_l1_l2_map(0x00100000, 0x00100000,0x4000,0x001,0x436);
    /* 0x0010_4000---------0x0010_FFFF  |  48 KB    |   Reserved                                 |*/
    mmu_l1_l2_map(0x00104000, 0x00104000,0xc000,0x0,0x0);
    /* 0x0011_0000---------0x0011_1FFF  |  8 KB     |   APBH-Bridge-DMA                          |*/
    // l1 P=1'b0,Domain = 4'b0 ,SBZ = 3'b0,ID = 2'b01; l2 nG = 1'b0,S = 1'b1,APX = 1'b0,TEX = 3'b000 AP = 2'b11, C = 1'b0,B=1'b1,XN=1'b0
    mmu_l1_l2_map(0x00110000, 0x00110000,0x2000,0x001,0x436);
    /* 0x0011_2000---------0x0011_3FFF  |  8 KB     |   GPMI                                     |*/
    mmu_l1_l2_map(0x00112000, 0x00112000,0x2000,0x001,0x436);
    /* 0x0011_4000---------0x0011_7FFF  |  16 KB    |   BCH                                      |*/
    mmu_l1_l2_map(0x00114000, 0x00114000,0x4000,0x001,0x436);
    /* 0x0011_8000---------0x0011_FFFF  |  32 KB    |   Reserved                                 |*/
    mmu_l1_l2_map(0x00118000, 0x00118000,0x8000,0x0,0x0);
    /* 0x0012_0000---------0x0012_8FFF  |  36 KB    |   HDMI                                     |*/
    mmu_l1_l2_map(0x00120000, 0x00120000,0x9000,0x001,0x436);
    /* 0x0012_9000---------0x0012_FFFF  |  28 KB    |   Reserved                                 |*/
    mmu_l1_l2_map(0x00129000, 0x00129000,0x7000,0x0,0x0);
    /* 0x0013_0000---------0x0013_3FFF  |  16 KB    |   GPU 3D                                   |*/
    mmu_l1_l2_map(0x00130000, 0x00130000,0x4000,0x001,0x436);
    /* 0x0013_4000---------0x0013_7FFF  |  16 KB    |   GPU 2D (GC320)                           |*/
    mmu_l1_l2_map(0x00134000, 0x00134000,0x4000,0x001,0x436);
    /* 0x0013_8000---------0x0013_BFFF  |  16 KB    |   DTCP                                     |*/
    mmu_l1_l2_map(0x00138000, 0x00138000,0x4000,0x001,0x436);
    /* 0x0013_C000---------0x001F_FFFF  |  784 KB   |   Reserved                                 |*/
    mmu_l1_l2_map(0x0013c000, 0x0013c000,0xc4000,0x0,0x0);
    /* 0x0020_0000---------0x002F_FFFF  |  1 MB     |   GPV_2 PL301 (per1) configuration port    |*/
    mmu_l1_l2_map(0x00200000, 0x00200000,0x100000,0x001,0x436);
    /* 0x0030_0000---------0x003F_FFFF  |  1 MB     |   GPV_3 PL301 (per2) configuration port    |*/
    mmu_l1_l2_map(0x00300000, 0x00300000,0x100000,0x001,0x436);
    /* 0x0040_0000---------0x007F_FFFF  |  4 MB     |   Reserved                                 |*/
    mmu_l1_l2_map(0x00400000, 0x00400000,0x400000,0x0,0x0);
    /* 0x0080_0000---------0x008F_FFFF  |  1 MB     |   GPV_4 PL301 (fast3) configuration port   |*/
    mmu_l1_l2_map(0x00800000, 0x00800000,0x100000,0x001,0x436);
    /* 0x0090_0000---------0x0093_FFFF  |  0.25 MB  |   OCRAM 256 KB                             |*/
    mmu_l1_l2_map(0x00900000, 0x00900000,0x40000,0x1e1,0x13e);// inner-cacheable
    /* 0x0094_0000---------0x009F_FFFF  |  0.75 MB  |   OCRAM aliased                            |*/
    mmu_l1_l2_map(0x00940000, 0x00940000,0xc0000,0x1e1,0x13e);// inner-cacheable
    /* 0x00A0_0000---------0x00A0_1FFF  |  8 KB     |   ARM MP                                   |*/
    mmu_l1_l2_map(0x00a00000, 0x00a00000,0x2000,0x001,0x436);
    /* 0x00A0_2000---------0x00A0_2FFF  |  4 KB     |   PL310 (L2 cache controller)              |*/
    mmu_l1_l2_map(0x00a02000, 0x00a02000,0x1000,0x001,0x436);
    /* 0x00A0_3000---------0x00AF_FFFF  |  1012 KB  |   Reserved                                 |*/
    mmu_l1_l2_map(0x00a03000, 0x00a03000,0xfd000,0x0,0x0);
    /* 0x00B0_0000---------0x00BF_FFFF  |  1 MB     |   GPV_0 PL301 (fast2) configuration port   |*/
    mmu_l1_l2_map(0x00b00000, 0x00b00000,0x100000,0x001,0x436);
    /* 0x00C0_0000---------0x00CF_FFFF  |  1 MB     |   GPV_1 PL301 (fast1)                      |*/
    mmu_l1_l2_map(0x00c00000, 0x00c00000,0x100000,0x001,0x436);
    /* 0x00D0_0000---------0x00FF_FFFF  |  3072 KB  |   Reserved                                 |*/
    mmu_l1_l2_map(0x00d00000, 0x00d00000,0x300000,0x0,0x0);
    /* 0x0100_0000---------0x01FF_BFFF  |  16368 KB |   PCIe                                     |*/
    mmu_l1_l2_map(0x01000000, 0x01000000,0xffc000,0x001,0x436);
    /* 0x01FF_C000---------0x01FF_FFFF  |  16 KB    |   PCIe registers                           |*/
    mmu_l1_l2_map(0x01ffc000, 0x01ffc000,0x4000,0x001,0x436);
    /* 0x0200_0000---------0x020F_FFFF  |  1 MB     |   Peripheral IPs via AIPS-1                |*/
    mmu_l1_l2_map(0x02000000, 0x02000000,0x100000,0x001,0x436);
    /* 0x0210_0000---------0x021F_FFFF  |  1 MB     |   Peripheral IPs via AIPS-2                |*/
    mmu_l1_l2_map(0x02100000, 0x02100000,0x100000,0x001,0x436);
    /* 0x0220_0000---------0x0220_3FFF  |  16 KB    |   SATA                                     |*/
    mmu_l1_l2_map(0x02200000, 0x02200000,0x4000,0x001,0x436);
    /* 0x0220_4000---------0x0220_7FFF  |  16 KB    |   OpenVG (GC355)                           |*/
    mmu_l1_l2_map(0x02204000, 0x02204000,0x4000,0x001,0x436);
    /* 0x0220_8000---------0x0220_BFFF  |  16 KB    |   MIPI_HSI                                 |*/
    mmu_l1_l2_map(0x02208000, 0x02208000,0x4000,0x001,0x436);
    /* 0x0220_C000---------0x023F_FFFF  |  2 MB     |   Reserved                                 |*/
    mmu_l1_l2_map(0x0220c000, 0x0220c000,0x200000,0x0,0x0);
    /* 0x0260_0000---------0x029F_FFFF  |  4 MB     |   IPU-1                                    |*/
    mmu_l1_l2_map(0x02600000, 0x02600000,0x400000,0x001,0x436);
    /* 0x02A0_0000---------0x02DF_FFFF  |  4 MB     |   IPU-2                                    |*/
    mmu_l1_l2_map(0x02a00000, 0x02a00000,0x400000,0x001,0x436);
    /* 0x0800_0000---------0x0FFF_FFFF  |  128 MB   |   EIM—(NOR/SRAM)                           |*/
    mmu_l1_l2_map(0x08000000, 0x08000000,0x8000000,0x001,0x436);
    /* 0x1000_0000---------0xFFFF_FFFF  |  3840 MB  |   MMDC—DDR Controller.                     |*/
    mmu_l1_l2_map(0x10000000, 0x10000000,0xf0000000,0x1e1,0x576);// Shareable, Outer and Inner Write-Back, Write-Allocate cache 
    //mmu_l1_l2_map(0x10000000, 0x10000000,0xf0000000,0x1e1,0x7e);// no-Shareable, Outer and Inner Write-Back, Write-Allocate cache 
#endif
    unsigned int ItemLen = 0;
    int idx = 0;
    unsigned int virtualAddress = 0;
    unsigned int physicalAddress = 0;
    unsigned int length = 0;
    unsigned int l1_attr = 0,l2_attr = 0;
	
    ItemLen = sizeof(gChipMemoryMap)/sizeof(gChipMemoryMap[0]);
    for(idx = 0;idx < ItemLen;idx++)
    {
        length = gChipMemoryMap[idx].physicalEndAddress - gChipMemoryMap[idx].physicalStartAddress + 1;
        virtualAddress = gChipMemoryMap[idx].physicalStartAddress; 
        physicalAddress  = gChipMemoryMap[idx].physicalStartAddress;
        l1_attr = gChipMemoryMap[idx].l1_attr;
        l2_attr = gChipMemoryMap[idx].l2_attr;
        mmu_l1_l2_map(physicalAddress , virtualAddress , length ,l1_attr,l2_attr);
    }

    disp("%d Items translation table init done \n",idx);   
    return;
}

void Test_VirtualMMU(unsigned int va)
{
    unsigned int TTBAddr = 0;
    unsigned int L1_DescriptorAddr = 0;
    unsigned int L1_Descriptor = 0;
    unsigned int L2_TableBaseAddr = 0;
    unsigned int L2_DescriptorAddr = 0;
    unsigned int L2_Descriptor = 0;
    unsigned int SmallPageBaseAddr = 0;
    unsigned int PhysicalAddress = 0;

    unsigned int L1_section = 0;
    unsigned int L2_index = 0;
    unsigned int offset = 0;

    offset = va & 0xfff;

    
    TTBAddr = (unsigned int)(&__mmu_l1_tbl_start);

    disp("L1_TblAddr = 0x%x \n",TTBAddr);

    L1_section = (va & 0xfff00000) >> 20;
    L2_index = (va & 0xff000) >> 12;

    L1_DescriptorAddr = (TTBAddr & 0xffffc000) + L1_section * 4;
    
    L1_Descriptor = *(unsigned int *)L1_DescriptorAddr;
    disp("L1_DescriptorAddr = 0x%x,L1_Descriptor = 0x%x\n",L1_DescriptorAddr,L1_Descriptor);
    L1_Descriptor = l1_table[L1_section];
    disp("L1_Descriptor = 0x%x\n",L1_Descriptor);

    L2_TableBaseAddr = L1_Descriptor & 0xfffffc00; 
    L2_DescriptorAddr = L2_TableBaseAddr + L2_index * 4;
    L2_Descriptor = *(unsigned int *)L2_DescriptorAddr;
    disp("L2_TableBaseAddr = 0x%x,L2_DescriptorAddr = 0x%x,L2_Descriptor = 0x%x\n",L2_TableBaseAddr,L2_DescriptorAddr,L2_Descriptor);

    L2_Descriptor = l2_table[L1_section * 256 + L2_index];
    disp("L2_Descriptor = 0x%x\n",L2_Descriptor);
    SmallPageBaseAddr = L2_Descriptor & 0xfffff000; 

    PhysicalAddress = SmallPageBaseAddr + offset;

    disp("va = 0x%x,pa = 0x%x\n",va,PhysicalAddress);
}

void mmu_init(void)
{
    unsigned int table = (unsigned int)(&__mmu_l1_tbl_start);
    _arm_mcr(15,0,0,2,0,2);// write TTBCR reg
  
    table |= 0x58; // tlb Outer-cacheable, WB 
    _arm_mcr(15,0,table,2,0,0);  // write table address to TTBR0
    
    unsigned int dacr = 0x55555555;// set client mode for all domains
    _arm_mcr(15,0,dacr,3,0,0);
    mmu_table_init();
}

void mmu_enable(void)
{
    // invalidate all tlb 
    invalidate_tlb();
    // read SCTLR 
    uint32_t sctlr;
    _arm_mrc(15, 0, sctlr, 1, 0, 0);
    
    // set MMU enable bit,enable I-cache and D-cache 
    sctlr |= (BM_SCTLR_M | BM_SCTLR_I | BM_SCTLR_C);

    // write modified SCTLR
    _arm_mcr(15, 0, sctlr, 1, 0, 0);
    _ARM_DSB();
    _ARM_ISB();
}

void mmu_disable(void)
{
    // read current SCTLR 
    uint32_t sctlr;
    _arm_mrc(15, 0, sctlr, 1, 0, 0);
    
    // clear MMU enable bit 
    sctlr &=~ BM_SCTLR_M;

    // write modified SCTLR
    _arm_mcr(15, 0, sctlr, 1, 0, 0);
}

void SetTlbAttributes(unsigned int phy_addr,unsigned int l1_attr,unsigned int l2_attr)
{
    unsigned int L1_section = 0;
    unsigned int L2_index = 0;
    unsigned int TTBAddr = 0;
    unsigned int L1_DescriptorAddr = 0;
    unsigned int L1_Descriptor = 0;
    unsigned int L2_TableBaseAddr = 0;
    unsigned int L2_DescriptorAddr = 0;
    unsigned int L2_Descriptor = 0;
    
    L1_section = (phy_addr & 0xfff00000) >> 20;
    L2_index = (phy_addr & 0xff000) >> 12;
    
    TTBAddr = (unsigned int)(&__mmu_l1_tbl_start);

    L1_DescriptorAddr = (TTBAddr & 0xffffc000) + L1_section * 4;
    
    L1_Descriptor = *(unsigned int *)L1_DescriptorAddr;
    
    L2_TableBaseAddr = L1_Descriptor & 0xfffffc00;
    L2_DescriptorAddr = L2_TableBaseAddr + L2_index * 4;
    L2_Descriptor = *(unsigned int *)L2_DescriptorAddr;

    L1_Descriptor  =  (L1_Descriptor & 0xfffffc00) | (l1_attr & 0x3ff);
    *(unsigned int *)L1_DescriptorAddr = L1_Descriptor; 

    L2_Descriptor = (L2_Descriptor & 0xfffff000) | (l2_attr & 0xfff); 
    *(unsigned int *)L2_DescriptorAddr = L2_Descriptor; 
  
    arm_dcache_flush();
    invalidate_tlb();
    arm_branch_target_cache_invalidate();
    _ARM_DSB();
    _ARM_ISB();
}






