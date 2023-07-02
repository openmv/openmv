/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * AXI QoS Setup
 */
#ifndef __AXIQOS_H__
#define __AXIQOS_H__

#define OMV_AXI_GPV_BASE                0x51000000
#define OMV_AXI_GPV_QOS_BASE            ((OMV_AXI_GPV_BASE) +0x41100)
#define OMV_AXI_GPV_QOS_R_BASE(x)       ((OMV_AXI_GPV_QOS_BASE) +(0x1000 * (x)) + 0x0)
#define OMV_AXI_GPV_QOS_W_BASE(x)       ((OMV_AXI_GPV_QOS_BASE) +(0x1000 * (x)) + 0x4)

#define OMV_AXI_QOS_D2_AHB_R_ADDRESS    OMV_AXI_GPV_QOS_R_BASE(1)
#define OMV_AXI_QOS_D2_AHB_R_SET(x)     *((volatile uint32_t *) (OMV_AXI_QOS_D2_AHB_R_ADDRESS)) = (x)

#define OMV_AXI_QOS_D2_AHB_W_ADDRESS    OMV_AXI_GPV_QOS_W_BASE(1)
#define OMV_AXI_QOS_D2_AHB_W_SET(x)     *((volatile uint32_t *) (OMV_AXI_QOS_D2_AHB_W_ADDRESS)) = (x)

#define OMV_AXI_QOS_C_M7_R_ADDRESS      OMV_AXI_GPV_QOS_R_BASE(2)
#define OMV_AXI_QOS_C_M7_R_SET(x)       *((volatile uint32_t *) (OMV_AXI_QOS_C_M7_R_ADDRESS)) = (x)

#define OMV_AXI_QOS_C_M7_W_ADDRESS      OMV_AXI_GPV_QOS_W_BASE(2)
#define OMV_AXI_QOS_C_M7_W_SET(x)       *((volatile uint32_t *) (OMV_AXI_QOS_C_M7_W_ADDRESS)) = (x)

#define OMV_AXI_QOS_SDMMC1_R_ADDRESS    OMV_AXI_GPV_QOS_R_BASE(3)
#define OMV_AXI_QOS_SDMMC1_R_SET(x)     *((volatile uint32_t *) (OMV_AXI_QOS_SDMMC1_R_ADDRESS)) = (x)

#define OMV_AXI_QOS_SDMMC1_W_ADDRESS    OMV_AXI_GPV_QOS_W_BASE(3)
#define OMV_AXI_QOS_SDMMC1_W_SET(x)     *((volatile uint32_t *) (OMV_AXI_QOS_SDMMC1_W_ADDRESS)) = (x)

#define OMV_AXI_QOS_MDMA_R_ADDRESS      OMV_AXI_GPV_QOS_R_BASE(4)
#define OMV_AXI_QOS_MDMA_R_SET(x)       *((volatile uint32_t *) (OMV_AXI_QOS_MDMA_R_ADDRESS)) = (x)

#define OMV_AXI_QOS_MDMA_W_ADDRESS      OMV_AXI_GPV_QOS_W_BASE(4)
#define OMV_AXI_QOS_MDMA_W_SET(x)       *((volatile uint32_t *) (OMV_AXI_QOS_MDMA_W_ADDRESS)) = (x)

#define OMV_AXI_QOS_DMA2D_R_ADDRESS     OMV_AXI_GPV_QOS_R_BASE(5)
#define OMV_AXI_QOS_DMA2D_R_SET(x)      *((volatile uint32_t *) (OMV_AXI_QOS_DMA2D_R_ADDRESS)) = (x)

#define OMV_AXI_QOS_DMA2D_W_ADDRESS     OMV_AXI_GPV_QOS_W_BASE(5)
#define OMV_AXI_QOS_DMA2D_W_SET(x)      *((volatile uint32_t *) (OMV_AXI_QOS_DMA2D_W_ADDRESS)) = (x)

#define OMV_AXI_QOS_LTDC_R_ADDRESS      OMV_AXI_GPV_QOS_R_BASE(6)
#define OMV_AXI_QOS_LTDC_R_SET(x)       *((volatile uint32_t *) (OMV_AXI_QOS_LTDC_R_ADDRESS)) = (x)

#define OMV_AXI_QOS_LTDC_W_ADDRESS      OMV_AXI_GPV_QOS_W_BASE(6)
#define OMV_AXI_QOS_LTDC_W_SET(x)       *((volatile uint32_t *) (OMV_AXI_QOS_LTDC_W_ADDRESS)) = (x)

#endif // __AXIQOS_H__
