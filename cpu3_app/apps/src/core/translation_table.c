/************************************************************************************************************************
----------------------------------System memory map----------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------   
Start Address       End Address  |  SIZE     |   Description                              |   Definition in Translation Table   
------------------------------------------------------------------------------------------------------------------------
0x1000_0000---------0xFFFF_FFFF  |  3840 MB  |   MMDC—DDR Controller.                     | Normal write-back Cacheable 
0x0800_0000---------0x0FFF_FFFF  |  128 MB   |   EIM—(NOR/SRAM)                           | Normal write-back Cacheable
0x02A0_0000---------0x02DF_FFFF  |  4 MB     |   IPU-2                                    |
0x0260_0000---------0x029F_FFFF  |  4 MB     |   IPU-1                                    |
0x0220_C000---------0x023F_FFFF  |  2 MB     |   Reserved                                 |
0x0220_8000---------0x0220_BFFF  |  16 KB    |   MIPI_HSI                                 |
0x0220_4000---------0x0220_7FFF  |  16 KB    |   OpenVG (GC355)                           |
0x0220_0000---------0x0220_3FFF  |  16 KB    |   SATA                                     |
0x0210_0000---------0x021F_FFFF  |  1 MB     |   Peripheral IPs via AIPS-2                |
0x0200_0000---------0x020F_FFFF  |  1 MB     |   Peripheral IPs via AIPS-1                |
0x01FF_C000---------0x01FF_FFFF  |  16 KB    |   PCIe registers                           |
0x0100_0000---------0x01FF_BFFF  |  16368 KB |   PCIe                                     |
0x00D0_0000---------0x00FF_FFFF  |  3072 KB  |   Reserved                                 |
0x00C0_0000---------0x00CF_FFFF  |  1 MB     |   GPV_1 PL301 (fast1)                      |
0x00B0_0000---------0x00BF_FFFF  |  1 MB     |   GPV_0 PL301 (fast2) configuration port   |
0x00A0_3000---------0x00AF_FFFF  |  1012 KB  |   Reserved                                 |
0x00A0_2000---------0x00A0_2FFF  |  4 KB     |   PL310 (L2 cache controller)              |
0x00A0_0000---------0x00A0_1FFF  |  8 KB     |   ARM MP                                   |
0x0094_0000---------0x009F_FFFF  |  0.75 MB  |   OCRAM aliased                            |
0x0090_0000---------0x0093_FFFF  |  0.25 MB  |   OCRAM 256 KB                             |
0x0080_0000---------0x008F_FFFF  |  1 MB     |   GPV_4 PL301 (fast3) configuration port   |
0x0040_0000---------0x007F_FFFF  |  4 MB     |   Reserved                                 |
0x0030_0000---------0x003F_FFFF  |  1 MB     |   GPV_3 PL301 (per2) configuration port    |
0x0020_0000---------0x002F_FFFF  |  1 MB     |   GPV_2 PL301 (per1) configuration port    |
0x0013_C000---------0x001F_FFFF  |  784 KB   |   Reserved                                 |
0x0013_8000---------0x0013_BFFF  |  16 KB    |   DTCP                                     |
0x0013_4000---------0x0013_7FFF  |  16 KB    |   GPU 2D (GC320)                           |
0x0013_0000---------0x0013_3FFF  |  16 KB    |   GPU 3D                                   |
0x0012_9000---------0x0012_FFFF  |  28 KB    |   Reserved                                 |
0x0012_0000---------0x0012_8FFF  |  36 KB    |   HDMI                                     |
0x0011_8000---------0x0011_FFFF  |  32 KB    |   Reserved                                 |
0x0011_4000---------0x0011_7FFF  |  16 KB    |   BCH                                      |
0x0011_2000---------0x0011_3FFF  |  8 KB     |   GPMI                                     |
0x0011_0000---------0x0011_1FFF  |  8 KB     |   APBH-Bridge-DMA                          |
0x0010_4000---------0x0010_FFFF  |  48 KB    |   Reserved                                 |
0x0010_0000---------0x0010_3FFF  |  16 KB    |   CAAM (16 KB secure RAM)                  |
0x0001_8000---------0x000F_FFFF  |  928 KB   |   Reserved                                 |
0x0000_0000---------0x0001_7FFF  |  96 KB    |   Boot ROM (ROMCP)                         |
-----------------------------------------------------------------------------------------------------------------------------
*****************************************************************************************************************************/
#include "public.h"

#define MMU_L1_TABLE_ITEM_NUM  (4096)       // L1 size is 4096 index * 4 bytes 
#define MMU_L2_TABLE_ITEM_NUM  (4096*256)   // each L2 size is 256 index * 4 bytes = 1k;

typedef union mmu_l1_entry
{
    uint32_t u;
    struct {
        //uint32_t id:2;           //!< ID translation table entry format
        //uint32_t sbz:3;
        //uint32_t domain:4;
        //uint32_t p:1;       
        uint32_t l1_attr:10;
        uint32_t l2_address:22;// Level 2 Descriptor Base Address
    };
} mmu_l1_entry_t;

typedef union mmu_l2_entry
{
    uint32_t u;
    struct {
        //uint32_t id:2;           //!< ID translation table entry format
        //uint32_t b:1;
        //uint32_t c:1;
        //uint32_t ap:2;
        //uint32_t tex:3;
        //uint32_t apx:1;
        //uint32_t s:1;
        //uint32_t ng:1;
        uint32_t l2_attr:12;
        uint32_t address:20;// Small Page Base Address
    };
} mmu_l2_entry_t;


extern char __mmu_l1_tbl_start;
extern char __mmu_l2_tbl_start;

//uint32_t *l1_table = (uint32_t *)&__mmu_l1_tbl_start;
//uint32_t *l2_table = (uint32_t *)&__mmu_l2_tbl_start;

__attribute__ ((section (".mmu_l1_tbl"))) unsigned int l1_table[MMU_L1_TABLE_ITEM_NUM] = {[0 ... (MMU_L1_TABLE_ITEM_NUM - 1)] = 0x0};
__attribute__ ((section (".mmu_l2_tbl"))) unsigned int l2_table[MMU_L2_TABLE_ITEM_NUM] = {[0 ... (MMU_L1_TABLE_ITEM_NUM - 1)] = 0x0};

mmu_l1_entry_t l1_entry;
mmu_l2_entry_t l2_entry;


static void mmu_l1_l2_map(unsigned int pa_start, unsigned int va_start,unsigned int length,unsigned int l1_attr, unsigned int l2_attr)
{
    unsigned int i = 0;
    unsigned int l1_index = 0;
    unsigned int l2_index = 0;
    unsigned int va = 0;
    unsigned int pa = 0;
    unsigned int num_pages = 0,num_sections = 0;

    l1_entry.u = 0;
    l2_entry.u = 0;
    va = va_start;
    pa = pa_start;

    num_pages = length >> 12;// length / 4k 
    num_sections = num_pages >> 8; // pages / 256;
    l1_entry.l1_attr = (l1_attr & 0x3ff);
    l2_entry.l2_attr = (l2_attr & 0xfff);

    for(i = 0;i < num_pages;i++)
    {
        l1_index = (va & 0xfff00000) >> 20;
        l2_index = (va & 0xff000) >> 12;
        
        l1_entry.l2_address = (unsigned int)(&__mmu_l2_tbl_start) + l1_index * 256 * 4;//bytes
        l1_table[l1_index] = l1_entry.u;
        
        l2_entry.address = (pa_start & 0xffffc000) >> 12;
        l2_table[l2_index + l1_index * 256] = l2_entry.u;
        
        pa = pa + 4*1024;//up 4k;     
        va = va + 4*1024;//up 4k;
    }
    
    return ;
}


void mmu_table_init(void)
{
    /* 0x0000_0000---------0x0001_7FFF  |  96 KB    |   Boot ROM (ROMCP)                         |                            */
    mmu_l1_l2_map(0x00000000, 0x00000000,0x1800,0,0);
    /* 0x0001_8000---------0x000F_FFFF  |  928 KB   |   Reserved                                 |                            */
    mmu_l1_l2_map(0x00018000, 0x00018000,0xe8000,0x0,0x0);
    /* 0x0010_0000---------0x0010_3FFF  |  16 KB    |   CAAM (16 KB secure RAM)                  | */
    mmu_l1_l2_map(0x00100000, 0x00100000,0x4000,0x0,0x0);
    /* 0x0010_4000---------0x0010_FFFF  |  48 KB    |   Reserved                                 |  */
    mmu_l1_l2_map(0x00104000, 0x00104000,0xc000,0x0,0x0);
    /* 0x0011_0000---------0x0011_1FFF  |  8 KB     |   APBH-Bridge-DMA                          | */
    mmu_l1_l2_map(0x00110000, 0x00110000,0x2000,0x0,0x0);
    /* 0x0011_2000---------0x0011_3FFF  |  8 KB     |   GPMI                                     | */
    mmu_l1_l2_map(0x00112000, 0x00112000,0x2000,0x0,0x0);
    /* 0x0011_4000---------0x0011_7FFF  |  16 KB    |   BCH                                      | */
    mmu_l1_l2_map(0x00114000, 0x00114000,0x4000,0x0,0x0);
    /* 0x0011_8000---------0x0011_FFFF  |  32 KB    |   Reserved                                 | */
    mmu_l1_l2_map(0x00118000, 0x00118000,0x8000,0x0,0x0);
    /* 0x0012_0000---------0x0012_8FFF  |  36 KB    |   HDMI                                     |  */
    mmu_l1_l2_map(0x00120000, 0x00120000,0x9000,0x0,0x0);
    /* 0x0012_9000---------0x0012_FFFF  |  28 KB    |   Reserved                                 |  */
    mmu_l1_l2_map(0x00129000, 0x00129000,0x7000,0x0,0x0);
    /* 0x0013_0000---------0x0013_3FFF  |  16 KB    |   GPU 3D                                   |  */
    mmu_l1_l2_map(0x00130000, 0x00130000,0x4000,0x0,0x0);
    /* 0x0013_4000---------0x0013_7FFF  |  16 KB    |   GPU 2D (GC320)                           |  */
    mmu_l1_l2_map(0x00134000, 0x00134000,0x4000,0x0,0x0);
    /* 0x0013_8000---------0x0013_BFFF  |  16 KB    |   DTCP                                     | */
    mmu_l1_l2_map(0x00138000, 0x00138000,0x4000,0x0,0x0);
    /* 0x0013_C000---------0x001F_FFFF  |  784 KB   |   Reserved                                 |  */
    mmu_l1_l2_map(0x0013c000, 0x0013c000,0xc4000,0x0,0x0);
    /* 0x0020_0000---------0x002F_FFFF  |  1 MB     |   GPV_2 PL301 (per1) configuration port    |  */
    mmu_l1_l2_map(0x00200000, 0x00200000,0x100000,0x0,0x0);
    /* 0x0030_0000---------0x003F_FFFF  |  1 MB     |   GPV_3 PL301 (per2) configuration port    |  */
    mmu_l1_l2_map(0x00300000, 0x00300000,0x100000,0x0,0x0);
    /* 0x0040_0000---------0x007F_FFFF  |  4 MB     |   Reserved                                 | */
    mmu_l1_l2_map(0x00400000, 0x00400000,0x400000,0x0,0x0);
    /* 0x0080_0000---------0x008F_FFFF  |  1 MB     |   GPV_4 PL301 (fast3) configuration port   |*/
    mmu_l1_l2_map(0x00800000, 0x00800000,0x100000,0x0,0x0);
    /* 0x0090_0000---------0x0093_FFFF  |  0.25 MB  |   OCRAM 256 KB                             | */
    mmu_l1_l2_map(0x00900000, 0x00900000,0x40000,0x0,0x0);
    /* 0x0094_0000---------0x009F_FFFF  |  0.75 MB  |   OCRAM aliased                            |  */
    mmu_l1_l2_map(0x00940000, 0x00940000,0xc0000,0x0,0x0);
    /* 0x00A0_0000---------0x00A0_1FFF  |  8 KB     |   ARM MP                                   |  */
    mmu_l1_l2_map(0x00a00000, 0x00a00000,0x2000,0x0,0x0);
    /* 0x00A0_2000---------0x00A0_2FFF  |  4 KB     |   PL310 (L2 cache controller)              | */
    mmu_l1_l2_map(0x00a02000, 0x00a02000,0x1000,0x0,0x0);
    /* 0x00A0_3000---------0x00AF_FFFF  |  1012 KB  |   Reserved                                 |  */
    mmu_l1_l2_map(0x00a03000, 0x00a03000,0xfd000,0x0,0x0);
    /* 0x00B0_0000---------0x00BF_FFFF  |  1 MB     |   GPV_0 PL301 (fast2) configuration port   |  */
    mmu_l1_l2_map(0x00b00000, 0x00b00000,0x100000,0x0,0x0);
    /* 0x00C0_0000---------0x00CF_FFFF  |  1 MB     |   GPV_1 PL301 (fast1)                      |  */
    mmu_l1_l2_map(0x00c00000, 0x00c00000,0x100000,0x0,0x0);
    /* 0x00D0_0000---------0x00FF_FFFF  |  3072 KB  |   Reserved                                 |   */
    mmu_l1_l2_map(0x00d00000, 0x00d00000,0x300000,0x0,0x0);
    /* 0x0100_0000---------0x01FF_BFFF  |  16368 KB |   PCIe                                     |  */
    mmu_l1_l2_map(0x01000000, 0x01000000,0xffc000,0x0,0x0);
    /* 0x01FF_C000---------0x01FF_FFFF  |  16 KB    |   PCIe registers                           | */
    mmu_l1_l2_map(0x01ffc000, 0x01ffc000,0x4000,0x0,0x0);
    /* 0x0200_0000---------0x020F_FFFF  |  1 MB     |   Peripheral IPs via AIPS-1                | */
    mmu_l1_l2_map(0x02000000, 0x02000000,0x100000,0x0,0x0);
    /* 0x0210_0000---------0x021F_FFFF  |  1 MB     |   Peripheral IPs via AIPS-2                |  */
    mmu_l1_l2_map(0x02100000, 0x02100000,0x100000,0x0,0x0);
    /* 0x0220_0000---------0x0220_3FFF  |  16 KB    |   SATA                                     |  */
    mmu_l1_l2_map(0x02200000, 0x02200000,0x4000,0x0,0x0);
    /* 0x0220_4000---------0x0220_7FFF  |  16 KB    |   OpenVG (GC355)                           |  */
    mmu_l1_l2_map(0x02204000, 0x02204000,0x4000,0x0,0x0);
    /* 0x0220_8000---------0x0220_BFFF  |  16 KB    |   MIPI_HSI                                 |  */
    mmu_l1_l2_map(0x02208000, 0x02208000,0x4000,0x0,0x0);
    /* 0x0220_C000---------0x023F_FFFF  |  2 MB     |   Reserved                                 | */
    mmu_l1_l2_map(0x0220c000, 0x0220c000,0x200000,0x0,0x0);
    /* 0x0260_0000---------0x029F_FFFF  |  4 MB     |   IPU-1                                    |  */
    mmu_l1_l2_map(0x02600000, 0x02600000,0x400000,0x0,0x0);
    /* 0x02A0_0000---------0x02DF_FFFF  |  4 MB     |   IPU-2                                    |   */
    mmu_l1_l2_map(0x02a00000, 0x02a00000,0x400000,0x0,0x0);
    /* 0x0800_0000---------0x0FFF_FFFF  |  128 MB   |   EIM—(NOR/SRAM)                           |   */
    mmu_l1_l2_map(0x08000000, 0x0800000,0x8000000,0x0,0x0);
    /* 0x1000_0000---------0xFFFF_FFFF  |  3840 MB  |   MMDC—DDR Controller.                     |  */
    mmu_l1_l2_map(0x10000000, 0x10000000,0xf0000000,0x0,0x0);
    disp("translation table init done \n");   
    return;
}










