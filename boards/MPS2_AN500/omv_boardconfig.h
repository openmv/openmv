/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * Board configuration and pin definitions.
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// Architecture info
#define OMV_BOARD_ARCH                        "MPS2 AN500"  // 33 chars max
#define OMV_BOARD_TYPE                        "M7"

// JPEG configuration.
#define OMV_JPEG_CODEC_ENABLE                 (0)
#define OMV_JPEG_QUALITY_LOW                  (35)
#define OMV_JPEG_QUALITY_HIGH                 (60)
#define OMV_JPEG_QUALITY_THRESHOLD            (160 * 120 * 2)

// UMM heap block size
#define OMV_UMM_BLOCK_SIZE                    16

#define OMV_MAIN_MEMORY                       SRAM1     // Data/BSS memory
#define OMV_STACK_MEMORY                      SRAM1     // stack memory
#define OMV_STACK_SIZE                        (64K)
#define OMV_FB_MEMORY                         SRAM1     // Framebuffer, fb_alloc
#define OMV_FB_SIZE                           (1M)      // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE                     (1850K)   // minimum fb alloc size
#define OMV_GC_BLOCK0_MEMORY                  SRAM1     // Main GC block
#define OMV_GC_BLOCK0_SIZE                    (1M)
#define OMV_SB_SIZE                           (128K)    // IDE JPEG buffer size (header + data).

// Memory map.
#define OMV_SRAM1_ORIGIN                      0x20000000
#define OMV_SRAM1_LENGTH                      4M

#define OMV_FLASH_ISR_ORIGIN                  0x00000000
#define OMV_FLASH_ISR_LENGTH                  4K

#define OMV_FLASH_TXT_ORIGIN                  0x00001000
#define OMV_FLASH_TXT_LENGTH                  4092M

// CSI configuration.
#define OMV_CSI_I2C_ID                        (0)
#define OMV_CSI_I2C_SPEED                     (OMV_I2C_SPEED_STANDARD)
#define OMV_CSI_CLK_SOURCE                    (OMV_CSI_CLK_SOURCE_TIM)
#define OMV_CSI_CLK_FREQUENCY                 (24000000)
#define OMV_CSI_HW_CROP_ENABLE                (0)
#define OMV_CSI_MAX_DEVICES                   (3)

#define OMV_SOFTCSI_ENABLE                    (1)
#endif //__OMV_BOARDCONFIG_H__
