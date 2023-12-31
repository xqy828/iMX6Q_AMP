/*
 * Copyright (c) 2010-2012, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "hab_defines.h"
#include "soc_memory_map.h"

//! @brief dcd data, list of (register, value) pairs to initialize ddr
uint8_t input_dcd[] __attribute__ ((section (".dcd_data")))= {
#if 0    
    /*Use default DDR frequency: 528MHz*/
// To allow DDR re-initialization if it was already set up:
// reset the MMDC and disable the chip selects.
    DCD_DATA(MMDC_P0_BASE_ADDR + MDMISC_OFFSET, 0x00001602),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDCTL_OFFSET, 0x03110000),
    /* configure the IOMUX for the DDR3 interface */
//DDR IO TYPE:
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_DDR_TYPE, 0x000c0000),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_DDRPKE, 0x00000000),
//CLOCK:
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_0, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_1, 0x00000030),
//ADDRESS:
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_CAS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_RAS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_ADDDS, 0x00000030),
//CONTROL:
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_RESET, 0x000c0030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCKE0, 0x00003000),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCKE1, 0x00003000),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDBA2, 0x00000000),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT0, 0x00003030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT1, 0x00003030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_CTLDS, 0x00000030),
//DATA STROBE:
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_DDRMODE_CTL, 0x00020000),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS0, 0x00000038),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS1, 0x00000038),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS2, 0x00000038),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS3, 0x00000038),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS4, 0x00000038),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS5, 0x00000038),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS6, 0x00000038),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS7, 0x00000038),
//DATA:
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_DDRMODE, 0x00020000),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_B0DS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_B1DS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_B2DS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_B3DS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_B4DS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_B5DS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_B6DS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_GRP_B7DS, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM0, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM1, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM2, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM3, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM4, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM5, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM6, 0x00000030),
    DCD_DATA(IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM7, 0x00000030),
    /* DDR3 calibrations */
//ZQ:
    DCD_DATA(MMDC_P0_BASE_ADDR + MPZQHWCTRL_OFFSET, 0xa1390003),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPZQHWCTRL_OFFSET, 0xa1390003),
//write leveling:
    DCD_DATA(MMDC_P0_BASE_ADDR + MPWLDECTRL0_OFFSET, 0x001F001F),
    DCD_DATA(MMDC_P0_BASE_ADDR + MPWLDECTRL1_OFFSET, 0x001F001F),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPWLDECTRL0_OFFSET, 0x00440044),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPWLDECTRL1_OFFSET, 0x00440044),
//Read DQS Gating calibration:
    DCD_DATA(MMDC_P0_BASE_ADDR + MPDGCTRL0_OFFSET, 0x4333033F),
    DCD_DATA(MMDC_P0_BASE_ADDR + MPDGCTRL1_OFFSET, 0x0339033E),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPDGCTRL0_OFFSET, 0x433F0343),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPDGCTRL1_OFFSET, 0x03490320),
//Read calibration:
    DCD_DATA(MMDC_P0_BASE_ADDR + MPRDDLCTL_OFFSET, 0x42363838),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPRDDLCTL_OFFSET, 0x3F343242),
//Write calibration:
    DCD_DATA(MMDC_P0_BASE_ADDR + MPWRDLCTL_OFFSET, 0x35373933),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPWRDLCTL_OFFSET, 0x48254a36),
//read data bit delay: (3 is the reccommended default value)
    DCD_DATA(MMDC_P0_BASE_ADDR + MPRDDQBY0DL_OFFSET, 0x33333333),
    DCD_DATA(MMDC_P0_BASE_ADDR + MPRDDQBY1DL_OFFSET, 0x33333333),
    DCD_DATA(MMDC_P0_BASE_ADDR + MPRDDQBY2DL_OFFSET, 0x33333333),
    DCD_DATA(MMDC_P0_BASE_ADDR + MPRDDQBY3DL_OFFSET, 0x33333333),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPRDDQBY0DL_OFFSET, 0x33333333),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPRDDQBY1DL_OFFSET, 0x33333333),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPRDDQBY2DL_OFFSET, 0x33333333),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPRDDQBY3DL_OFFSET, 0x33333333),
//complete calibrations by forcing measurement
    DCD_DATA(MMDC_P0_BASE_ADDR + MPMUR0_OFFSET, 0x00000800),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPMUR0_OFFSET, 0x00000800),
    /* configure the MMDC */
//DDR3, 528MHz, 64-bit mode, only MMDC0 is initiated:
    DCD_DATA(MMDC_P0_BASE_ADDR + MDPDC_OFFSET, 0x00020036),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDOTC_OFFSET, 0x09444040),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDCFG0_OFFSET, 0x555A7975),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDCFG1_OFFSET, 0xFF538F64),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDCFG2_OFFSET, 0x01ff00db),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDMISC_OFFSET, 0x00081740),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x00008000),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDRWD_OFFSET, 0x000026d2),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDOR_OFFSET, 0x005b0e21),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDASP_OFFSET, 0x00000027),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDCTL_OFFSET, 0xc31a0000),
    /* Initialize 2GB DDR3 - Micron MT41J128M  */
//MR2:
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x04088032),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x0408803a),
//MR3:
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x00008033),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x0000803b),
//MR1:
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x00048031),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x00048039),
//MR0:
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x09408030),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x09408038),
//DDR device ZQ calibration:
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x04008040),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x04008048),
//final DDR setup, before operation start:
    DCD_DATA(MMDC_P0_BASE_ADDR + MDREF_OFFSET, 0x00005800),
    DCD_DATA(MMDC_P0_BASE_ADDR + MPODTCTRL_OFFSET, 0x00022227),
    DCD_DATA(MMDC_P1_BASE_ADDR + MPODTCTRL_OFFSET, 0x00022227),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDPDC_OFFSET, 0x00025576),
    DCD_DATA(MMDC_P0_BASE_ADDR + MAPSR_OFFSET, 0x00011006),
    DCD_DATA(MMDC_P0_BASE_ADDR + MDSCR_OFFSET, 0x00000000),
#endif
};

//! @brief HAB command write data header, with tag, 
//!        size of dcd data with hdr, 
//!        parameter field (size of register value and flag)
uint8_t input_dcd_wrt_cmd[] __attribute__ ((section (".dcd_wrt_cmd")))= {
    HAB_CMD_WRT_DAT, 
    EXPAND_UINT16(sizeof(input_dcd) + HDR_BYTES),
    WRT_DAT_PAR(0, HAB_DATA_WIDTH_WORD) //!< flag 0, width 4
};

//! @brief HAB dcd header with dcd tag, size of entire dcd and version.
uint8_t input_dcd_hdr[] __attribute__ ((section (".dcd_hdr")))= {
    HAB_TAG_DCD, 
    EXPAND_UINT16(sizeof(input_dcd) + sizeof(input_dcd_wrt_cmd) + HDR_BYTES),
    HAB_VER(4,0)
};
