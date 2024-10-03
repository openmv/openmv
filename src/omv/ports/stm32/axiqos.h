/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
