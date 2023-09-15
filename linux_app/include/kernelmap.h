#ifndef __KERNEL_MAP_H__
#define __KERNEL_MAP_H__

#include "public.h"

#define REGS_SRC_BASE (0x020d8000)
#define HW_SRC_SCR_ADDR      (gKernelMmapCtrl.memRegsSrcBase + 0x0)

#define HW_SRC_SCR           (*(volatile hw_src_scr_t *) HW_SRC_SCR_ADDR)
#define HW_SRC_SCR_RD()      (HW_SRC_SCR.U)
#define HW_SRC_SCR_WR(v)     (HW_SRC_SCR.U = (v))
#define HW_SRC_SCR_SET(v)    (HW_SRC_SCR_WR(HW_SRC_SCR_RD() |  (v)))
#define HW_SRC_SCR_CLR(v)    (HW_SRC_SCR_WR(HW_SRC_SCR_RD() & ~(v)))
#define HW_SRC_SCR_TOG(v)    (HW_SRC_SCR_WR(HW_SRC_SCR_RD() ^  (v)))

#define HW_SRC_GPR7_ADDR      (gKernelMmapCtrl.memRegsSrcBase + 0x38)

#define HW_SRC_GPR7           (*(volatile hw_src_gpr7_t *) HW_SRC_GPR7_ADDR)
#define HW_SRC_GPR7_RD()      (HW_SRC_GPR7.U)
#define HW_SRC_GPR7_WR(v)     (HW_SRC_GPR7.U = (v))
#define HW_SRC_GPR7_SET(v)    (HW_SRC_GPR7_WR(HW_SRC_GPR7_RD() |  (v)))
#define HW_SRC_GPR7_CLR(v)    (HW_SRC_GPR7_WR(HW_SRC_GPR7_RD() & ~(v)))
#define HW_SRC_GPR7_TOG(v)    (HW_SRC_GPR7_WR(HW_SRC_GPR7_RD() ^  (v)))


#define HW_SRC_GPR8_ADDR      (gKernelMmapCtrl.memRegsSrcBase + 0x3c)

#define HW_SRC_GPR8           (*(volatile hw_src_gpr8_t *) HW_SRC_GPR8_ADDR)
#define HW_SRC_GPR8_RD()      (HW_SRC_GPR8.U)
#define HW_SRC_GPR8_WR(v)     (HW_SRC_GPR8.U = (v))
#define HW_SRC_GPR8_SET(v)    (HW_SRC_GPR8_WR(HW_SRC_GPR8_RD() |  (v)))
#define HW_SRC_GPR8_CLR(v)    (HW_SRC_GPR8_WR(HW_SRC_GPR8_RD() & ~(v)))
#define HW_SRC_GPR8_TOG(v)    (HW_SRC_GPR8_WR(HW_SRC_GPR8_RD() ^  (v)))


#define CPU3APP_PHY_BASE   (0x78000000)




typedef union _hw_src_scr
{
    unsigned int  U;
    struct _hw_src_scr_bitfields
    {
        unsigned WARM_RESET_ENABLE : 1; //!< [0] WARM reset enable bit.
        unsigned SW_GPU_RST : 1; //!< [1] Software reset for gpu
        unsigned SW_VPU_RST : 1; //!< [2] Software reset for vpu
        unsigned SW_IPU1_RST : 1; //!< [3] Software reset for ipu1
        unsigned SW_OPEN_VG_RST : 1; //!< [4] Software reset for open_vg
        unsigned WARM_RST_BYPASS_COUNT : 2; //!< [6:5] Defines the ckil cycles to count before bypassing the MMDC acknowledge for WARM reset.
        unsigned MASK_WDOG_RST : 4; //!< [10:7] Mask wdog_rst_b source.
        unsigned EIM_RST : 1; //!< [11] EIM reset is needed in order to reconfigure the eim chip select.
        unsigned SW_IPU2_RST : 1; //!< [12] Software reset for ipu2
        unsigned CORE0_RST : 1; //!< [13] Software reset for core0 only.
        unsigned CORE1_RST : 1; //!< [14] Software reset for core1 only.
        unsigned CORE2_RST : 1; //!< [15] Software reset for core2 only.
        unsigned CORE3_RST : 1; //!< [16] Software reset for core3 only.
        unsigned CORE0_DBG_RST : 1; //!< [17] Software reset for core0 debug only.
        unsigned CORE1_DBG_RST : 1; //!< [18] Software reset for core1 debug only.
        unsigned CORE2_DBG_RST : 1; //!< [19] Software reset for core2 debug only.
        unsigned CORE3_DBG_RST : 1; //!< [20] Software reset for core3 debug only.
        unsigned CORES_DBG_RST : 1; //!< [21] Software reset for debug of arm platform only.
        unsigned CORE1_ENABLE : 1; //!< [22] CPU core1 enable.
        unsigned CORE2_ENABLE : 1; //!< [23] CPU core2 enable.
        unsigned CORE3_ENABLE : 1; //!< [24] CPU core3 enable.
        unsigned DBG_RST_MSK_PG : 1; //!< [25] Do not assert debug resets after power gating event of cpu
        unsigned RESERVED0 : 6; //!< [31:26] Reserved
    } B;
} hw_src_scr_t;


typedef union _hw_src_gpr7
{
    unsigned U;
    struct _hw_src_gpr7_bitfields
    {
        unsigned RESERVED0 : 32; //!< [31:0] Read/write bits, for general purpose
    } B;
} hw_src_gpr7_t;

typedef union _hw_src_gpr8
{
    unsigned int U;
    struct _hw_src_gpr8_bitfields
    {
        unsigned RESERVED0 : 32; //!< [31:0] Read/write bits, for general purpose
    } B;
} hw_src_gpr8_t;
 

typedef struct
{
    U32 memRegsSrcBase;
    U32 memCpu3PhyBase;
    U32 memOffset;
    U32 mem_fd;
}KernelBusMapCtrl;

extern KernelBusMapCtrl gKernelMmapCtrl;

void KernelMmapInit(void);

U32 Xil_Out32(U32 MemVirtAddr,U32 ulData);
U32 Xil_In32(U32 MemVirtAddr, U32 *ulData);
U32 Xil_OutStr(U32 MemVirtAddr, char *cStr);

#endif


