/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Board configuration and pin definitions.
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

// Architecture info
#define OMV_BOARD_ARCH                  "OPENMV AE4"    // 33 chars max
#define OMV_BOARD_TYPE                  "AE4"
#ifndef LINKER_SCRIPT
void se_services_reset_soc(void);
extern unsigned char OMV_BOARD_UID_ADDR[12];    // Unique address.
#endif
#define OMV_BOARD_UID_SIZE              3       // Unique ID size in words.
#define OMV_BOARD_UID_OFFSET            4       // Bytes offset for multi-word UIDs.
#define OMV_BOARD_RESET                 se_services_reset_soc

// GPU configuration
#define OMV_GPU_ENABLE                  (CORE_M55_HP)

// JPEG configuration.
#define OMV_JPEG_CODEC_ENABLE           (0)
#define OMV_JPEG_QUALITY_LOW            (50)
#define OMV_JPEG_QUALITY_HIGH           (68)
#define OMV_JPEG_QUALITY_THRESHOLD      (320 * 240 * 2)

// Enable RAW preview.
#define OMV_RAW_PREVIEW_ENABLE          (1)
#define OMV_RAW_PREVIEW_WIDTH           (512)
#define OMV_RAW_PREVIEW_HEIGHT          (320)

// CSI drivers configuration.
#define OMV_PAG7936_ENABLE              (CORE_M55_HP)

// FIR drivers configuration.
#define OMV_FIR_MLX90621_ENABLE         (0)
#define OMV_FIR_MLX90640_ENABLE         (0)
#define OMV_FIR_MLX90641_ENABLE         (0)
#define OMV_FIR_AMG8833_ENABLE          (1)


// USB config.
#define OMV_USB_IRQN                    (USB_IRQ_IRQn)
#define OMV_USB_SWITCH_PIN              (&omv_pin_USB_SWITCH)

// OpenMV protocol configuration.
#define OMV_PROTOCOL_MAX_BUFFER_SIZE        (8192)
#define OMV_PROTOCOL_STDIO_BUFFER_SIZE      (2048)
#define OMV_PROTOCOL_HW_CAPS                OMV_PROTOCOL_HW_CAPS_MAKE( \
        HAS_GPU, HAS_NPU, HAS_CRC, HAS_PMU, HAS_WIFI,                  \
        HAS_BT, HAS_SD, HAS_ETH, HAS_USB_HS, HAS_MULTICORE)

// Linker script constants (see common.ld.S).
#if BOOTLOADER
// Bootloader config
#define OMV_MAIN_MEMORY                 DTCM    // data, bss and heap memory
#define OMV_HEAP_MEMORY                 DTCM    // heap memory
#define OMV_HEAP_SIZE                   (256K)
#define OMV_STACK_MEMORY                ITCM    // stack memory
#define OMV_STACK_SIZE                  (256K)
#elif CORE_M55_HP
// HP core config
#define OMV_MAIN_MEMORY                 DTCM    // data, bss
#define OMV_HEAP_MEMORY                 DTCM    // libc/sbrk heap memory
#define OMV_HEAP_SIZE                   (128K)
#define OMV_BSS_SRAM_MEMORY             SRAM1   // BSS memory outside DTCM.
#define OMV_STACK_MEMORY                ITCM    // stack memory
#define OMV_STACK_SIZE                  (256K)
#define OMV_SB_MEMORY                   SRAM6_A // Streaming buffer memory.
#define OMV_SB_SIZE                     (1M)    // Streaming buffer size.
#define OMV_DMA_MEMORY                  DTCM    // Misc DMA buffers memory.
#define OMV_GPU_MEMORY                  SRAM9_B // GPU heap.
#define OMV_GPU_SIZE                    (256K)
#define OMV_OPENAMP_MEMORY              SRAM9_A // Open-AMP SHM.
#define OMV_OPENAMP_SIZE                (64K)
#define OMV_GC_BLOCK0_MEMORY            SRAM0   // Main GC block
#define OMV_GC_BLOCK0_SIZE              (2M)
#define OMV_UMA_BLOCK0_MEMORY           SRAM1   // Default UMA pool.
#define OMV_UMA_BLOCK0_SIZE             (3M)
#define OMV_UMA_BLOCK0_FLAGS            (UMA_DEFAULT)
#define OMV_UMA_BLOCK1_MEMORY           DTCM    // Fast UMA pool.
#define OMV_UMA_BLOCK1_SIZE             (512K)
#define OMV_UMA_BLOCK1_FLAGS            (UMA_DTCM)
#else
// HE core config
#define OMV_MAIN_MEMORY                 DTCM    // data, bss
#define OMV_HEAP_MEMORY                 DTCM    // libc/sbrk heap memory
#define OMV_HEAP_SIZE                   (64K)
#define OMV_BSS_SRAM_MEMORY             DTCM    // BSS memory outside DTCM.
#define OMV_STACK_MEMORY                ITCM    // stack memory
#define OMV_STACK_SIZE                  (256K)
#define OMV_SB_MEMORY                   SRAM6_B // JPEG buffer.
#define OMV_SB_SIZE                     (256K)
#define OMV_DMA_MEMORY                  DTCM    // Misc DMA buffers memory.
#define OMV_OPENAMP_MEMORY              SRAM9_A // Open-AMP SHM
#define OMV_OPENAMP_SIZE                (64K)
#define OMV_GC_BLOCK0_MEMORY            SRAM8   // Main GC block
#define OMV_GC_BLOCK0_SIZE              (1M)
#define OMV_UMA_BLOCK0_MEMORY           SRAM6_B // Default UMA pool.
#define OMV_UMA_BLOCK0_SIZE             (256K)
#define OMV_UMA_BLOCK0_FLAGS            (UMA_DEFAULT)
#endif

// Flash configuration.
#define OMV_FLASH_BOOT_ORIGIN           0x80000000
#define OMV_FLASH_BOOT_LENGTH           0x00020000      /* 128K */

#if CORE_M55_HP
#define OMV_FLASH_ORIGIN                0x80020000
#define OMV_FLASH_LENGTH                0x00300000      /* 3MB */
#define OMV_ROMFS_PART0_ORIGIN          0xa0800000
#define OMV_ROMFS_PART0_LENGTH          0x01800000
#define OMV_ROMFS_PART1_ORIGIN          0x8047E000
#define OMV_ROMFS_PART1_LENGTH          0x00100000
#elif CORE_M55_HE
#define OMV_FLASH_ORIGIN                0x80320000
#define OMV_FLASH_LENGTH                0x0015E000      /* 1.4MB */
#define OMV_ROMFS_PART0_ORIGIN          0x8047E000
#define OMV_ROMFS_PART0_LENGTH          0x00100000
#else
#error "MCU core is not specified"
#endif

// Memory configuration.
// Note SRAM2, SRAM3, SRAM4, SRAM5 are TCMs. Each M55 core sees its own ITCM
// memory mapped at 0x00000000, and its DTCM mapped at 0x20000000.
// Since the TCM is accessible through the global address map it is possible
// to configure peripherals and DMAs to read/write data to/from the TCM.
#define OMV_DTCM_ORIGIN                 0x20000000
#define OMV_ITCM_ORIGIN                 0x00000000
#if CORE_M55_HP
#define OMV_DTCM_LENGTH                 1024K
#define OMV_ITCM_LENGTH                 256K
#else
#define OMV_DTCM_LENGTH                 256K
#define OMV_ITCM_LENGTH                 256K
#endif

// E4 has no SRAM6-9, so these regions are carved out of SRAM0 and SRAM1 to keep
// the same roles as on the AE3. SRAM1 excludes the Open-AMP SHM (SRAM9_A),
// which has its own MPU region.
//
// SRAM1 is at 0x02400000 in the E4 memory map (SHARED_MEM_CTRL = 0x80000000 in the ATOC),
// right after SRAM0, where the ISP, JPEG and NPU can also access it. In the legacy
// (E3) memory map it's at 0x08000000, but they can't access it.
#define OMV_SRAM0_ORIGIN                0x02000000
#define OMV_SRAM0_LENGTH                0x00400000              /* 4M */

#define OMV_SRAM1_ORIGIN                0x02400000
#define OMV_SRAM1_LENGTH                0x003F0000              /* 4032K */

#define OMV_SRAM6_A_ORIGIN              0x02200000
#define OMV_SRAM6_A_LENGTH              0x00100000              /* 1M */

#define OMV_SRAM6_B_ORIGIN              0x02770000
#define OMV_SRAM6_B_LENGTH              0x00080000              /* 512K */

#define OMV_SRAM8_ORIGIN                0x02300000
#define OMV_SRAM8_LENGTH                0x00100000              /* 1M */

#define OMV_SRAM9_A_ORIGIN              0x027F0000
#define OMV_SRAM9_A_LENGTH              0x00010000              /* 64K */

#define OMV_SRAM9_B_ORIGIN              0x02730000
#define OMV_SRAM9_B_LENGTH              0x00040000              /* 256K */

// #define SRAM2_ORIGIN                 0x50000000
// #define SRAM2_LENGTH                 0x00040000		/* 256K */

// #define SRAM3_ORIGIN                 0x50800000
// #define SRAM3_LENGTH                 0x00100000		/* 1M */

// #define SRAM4_ORIGIN                 0x58000000
// #define SRAM4_LENGTH                 0x00040000		/* 256K */

// #define SRAM5_ORIGIN                 0x58800000
// #define SRAM5_LENGTH                 0x00040000		/* 256K */

// Physical I2C/I3C buses.
// I2C0
#define OMV_I2C0_ID                     (0)
#define OMV_I2C0_SCL_PIN                (&omv_pin_I2C0_SCL)
#define OMV_I2C0_SDA_PIN                (&omv_pin_I2C0_SDA)

// I2C1
#define OMV_I2C1_ID                     (1)
#define OMV_I2C1_SCL_PIN                (&omv_pin_I2C1_SCL)
#define OMV_I2C1_SDA_PIN                (&omv_pin_I2C1_SDA)

// I2C3
#define OMV_I2C3_ID                     (3)
#define OMV_I2C3_SCL_PIN                (&omv_pin_I2C3_SCL)
#define OMV_I2C3_SDA_PIN                (&omv_pin_I2C3_SDA)

// I3C0
#define OMV_I3C0_ID                     (4)
#define OMV_I3C0_SCL_PIN                (&omv_pin_I3C_SCL)
#define OMV_I3C0_SDA_PIN                (&omv_pin_I3C_SDA)

// Physical PDM buses.
// PDM1
#define OMV_PDM1_ID                     (1)
#define OMV_PDM1_C0_PIN                 (&omv_pin_PDM_C1)
#define OMV_PDM1_D0_PIN                 (&omv_pin_PDM_D1)

// Physical SPI buses.
// SPI bus 0
#define OMV_SPI0_ID                     (0)
#define OMV_SPI0_SCLK_PIN               (&omv_pin_SPI_SCLK)
#define OMV_SPI0_MISO_PIN               (&omv_pin_SPI_MISO)
#define OMV_SPI0_MOSI_PIN               (&omv_pin_SPI_MOSI)
#define OMV_SPI0_SSEL_PIN               (&omv_pin_SPI_SSEL)

// SPI bus 4
#define OMV_SPI4_ID                     (4)
#define OMV_SPI4_SCLK_PIN               (&omv_pin_LPSPI_SCLK)
#define OMV_SPI4_MISO_PIN               (&omv_pin_LPSPI_MISO)
#define OMV_SPI4_MOSI_PIN               (&omv_pin_LPSPI_MOSI)
#define OMV_SPI4_SSEL_PIN               (&omv_pin_LPSPI_SSEL)

// CSI I2C bus
#define OMV_CSI_I2C_ID                  (0)
#define OMV_CSI_I2C_SPEED               (OMV_I2C_SPEED_FULL)
#define OMV_CSI_I2C_REINIT              (0)

// FIR I2C bus
#define OMV_FIR_I2C_ID                  (1)
#define OMV_FIR_I2C_SPEED               (OMV_I2C_SPEED_FULL)

// TOF I2C bus
#define OMV_TOF_I2C_ID                  (3)
#define OMV_TOF_I2C_SPEED               (OMV_I2C_SPEED_FULL)
#define OMV_TOF_POWER_PIN               (&omv_pin_TOF_POWER)

// IMU SPI bus
#define OMV_IMU_SPI_ID                  (OMV_SPI4_ID)
#define OMV_IMU_SPI_BAUDRATE            (500000)
#define OMV_IMU_CHIP_LSM6DSM            (1)
#define OMV_IMU_X_Y_ROTATION_DEGREES    90
#define OMV_IMU_MOUNTING_Z_DIRECTION    -1

// PDM configuration
#define OMV_PDM_ID                      (OMV_PDM1_ID)
#define OMV_PDM_CHANNELS                (1)

// Camera interface
#define OMV_CSI_BASE                    ((CPI_Type *) CPI_BASE)
#define OMV_CSI_CLK_FREQUENCY           (24000000)
#define OMV_CSI_STATS_ENABLE            (1)
// The software debayer applies the white balance gains in Q5 and limits them to 4x.
#define OMV_CSI_STATS_GAIN_MIN          (1.0f / 32.0f)
#define OMV_CSI_STATS_GAIN_MAX          (4.0f)

// The sensor is connected over MIPI CSI-2.
#define OMV_CSI_MIPI_LANES              (2)
// Process the frames with the ISP.
#define OMV_CSI_ISP_ENABLE              (1)

#define OMV_CSI_MXCLK_PIN               (&omv_pin_CSI_MXCLK)
#define OMV_CSI_RESET_PIN               (&omv_pin_CSI_RESET)
#define OMV_CSI_POWER_PIN               (&omv_pin_CSI_POWER)
#define OMV_CSI_FSYNC_PIN               (&omv_pin_CSI_FSYNC)

#define OMV_CSI_POLARITY_CONFIG         { OMV_CSI_ACTIVE_LOW, OMV_CSI_ACTIVE_LOW }

#define OMV_WL_POWER_PIN                (&omv_pin_WL_REG_ON)
#define OMV_BT_POWER_PIN                (&omv_pin_BT_REG_ON)

// SPI LCD Interface
#define OMV_SPI_DISPLAY_CONTROLLER      (OMV_SPI0_ID)
#define OMV_SPI_DISPLAY_MOSI_PIN        (&omv_pin_SPI_MOSI)
#define OMV_SPI_DISPLAY_MISO_PIN        (&omv_pin_SPI_MISO)
#define OMV_SPI_DISPLAY_SCLK_PIN        (&omv_pin_SPI_SCLK)
#define OMV_SPI_DISPLAY_SSEL_PIN        (&omv_pin_SPI_SSEL)
#define OMV_SPI_DISPLAY_RS_PIN          (&omv_pin_LCD_RS)
#define OMV_SPI_DISPLAY_RST_PIN         (&omv_pin_LCD_RST)

#endif //__BOARD_CONFIG_H__
