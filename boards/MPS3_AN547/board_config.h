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
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

// Architecture info
#define OMV_BOARD_ARCH                        "MPS3 AN547"  // 33 chars max
#define OMV_BOARD_TYPE                        "M55"

// JPEG configuration.
#define OMV_JPEG_CODEC_ENABLE                 (0)
#define OMV_JPEG_QUALITY_LOW                  (35)
#define OMV_JPEG_QUALITY_HIGH                 (60)
#define OMV_JPEG_QUALITY_THRESHOLD            (160 * 120 * 2)


#define OMV_MAIN_MEMORY                       SRAM1     // Data/BSS memory
#define OMV_STACK_MEMORY                      SRAM1     // stack memory
#define OMV_STACK_SIZE                        (64K)
#define OMV_HEAP_MEMORY                       DDR
#define OMV_HEAP_SIZE                         (4M)
#define OMV_GC_BLOCK0_MEMORY                  DDR       // Main GC block
#define OMV_GC_BLOCK0_SIZE                    (8M)
#define OMV_UMA_BLOCK0_MEMORY                 DDR       // Default UMA pool.
#define OMV_UMA_BLOCK0_SIZE                   (10M)
#define OMV_UMA_BLOCK0_FLAGS                  (0)
#define OMV_SB_SIZE                           (128K)    // IDE JPEG buffer size (header + data).

// Memory map.
#define OMV_SRAM1_ORIGIN                      0x21000000
#define OMV_SRAM1_LENGTH                      8M

#define OMV_DDR_ORIGIN                        0x60000000
#define OMV_DDR_LENGTH                        32M

#define OMV_FLASH_ISR_ORIGIN                  0x00000000
#define OMV_FLASH_ISR_LENGTH                  4K

#define OMV_FLASH_TXT_ORIGIN                  0x01000000
#define OMV_FLASH_TXT_LENGTH                  2M

#define OMV_ROMFS_PART0_ORIGIN                0x62000000
#define OMV_ROMFS_PART0_LENGTH                0x02000000

// CSI configuration.
#define OMV_CSI_I2C_ID                        (0)
#define OMV_CSI_I2C_SPEED                     (OMV_I2C_SPEED_STANDARD)
#define OMV_CSI_CLK_SOURCE                    (OMV_CSI_CLK_SOURCE_TIM)
#define OMV_CSI_CLK_FREQUENCY                 (24000000)
#define OMV_CSI_HW_CROP_ENABLE                (0)
#define OMV_CSI_MAX_DEVICES                   (3)

#define OMV_SOFTCSI_ENABLE                    (1)

#endif //__BOARD_CONFIG_H__
