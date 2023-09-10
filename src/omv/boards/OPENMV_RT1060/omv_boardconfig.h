/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Board configuration and pin definitions.
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// Architecture info
#define OMV_ARCH_STR                    "OMVRT1060 32768 SDRAM"    // 33 chars max
#define OMV_BOARD_TYPE                  "IMXRT1060"
#define OMV_UNIQUE_ID_ADDR              0x401f4410   // Unique ID address.
#define OMV_UNIQUE_ID_SIZE              3            // Unique ID size in words.
#define OMV_UNIQUE_ID_OFFSET            12           // Bytes offset for multi-word UIDs.

// Needed by the SWD JTAG testrig - located at the bottom of the frame buffer stack.
#define OMV_SELF_TEST_SWD_ADDR          MAIN_FB()->pixfmt

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY              (12000000)

// OV767x sensor configuration.
#define OMV_OV7670_VERSION              (70)
#define OMV_OV7670_CLKRC                (0)

// OV7725 sensor configuration.
#define OMV_OV7725_PLL_CONFIG           (0x41)   // x4
#define OMV_OV7725_BANDING              (0x7F)

// OV5640 Sensor Settings
#define OMV_ENABLE_OV5640_AF            (0)
#define OMV_OV5640_XCLK_FREQ            (24000000)
#define OMV_OV5640_PLL_CTRL2            (0x64)
#define OMV_OV5640_PLL_CTRL3            (0x13)
#define OMV_OV5640_REV_Y_CHECK          (0)
#define OMV_OV5640_REV_Y_FREQ           (25000000)
#define OMV_OV5640_REV_Y_CTRL2          (0x54)
#define OMV_OV5640_REV_Y_CTRL3          (0x13)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG               (0)

// Enable sensor drivers
#define OMV_ENABLE_OV2640               (0)
#define OMV_ENABLE_OV5640               (1)
#define OMV_ENABLE_OV7670               (0)
#define OMV_ENABLE_OV7690               (0)
#define OMV_ENABLE_OV7725               (1)
#define OMV_ENABLE_OV9650               (0)
#define OMV_ENABLE_MT9M114              (1)
#define OMV_ENABLE_MT9V0XX              (1)
#define OMV_ENABLE_LEPTON               (1)
#define OMV_ENABLE_HM01B0               (0)
#define OMV_ENABLE_PAJ6100              (1)
#define OMV_ENABLE_FROGEYE2020          (1)
#define OMV_ENABLE_FIR_MLX90621         (1)
#define OMV_ENABLE_FIR_MLX90640         (1)
#define OMV_ENABLE_FIR_MLX90641         (1)
#define OMV_ENABLE_FIR_AMG8833          (1)
#define OMV_ENABLE_FIR_LEPTON           (1)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG              (0)
#define OMV_ENABLE_TUSBDBG              (1)
#define OMV_TUSBDBG_PACKET              (512)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST             (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH             (320 * 240 * 2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW                50
#define JPEG_QUALITY_HIGH               90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE              16

// Core VBAT for selftests
#define OMV_CORE_VBAT                   "3.3"

// USB config.
#define OMV_USB_IRQN                    (USB_OTG1_IRQn)
#define OMV_USB1_IRQ_HANDLER            (USB_OTG1_IRQHandler)
#define OMV_USB2_IRQ_HANDLER            (USB_OTG2_IRQHandler)

#define OMV_USB_PHY_ID                  (kUSB_ControllerEhci0)
#define OMV_USB_PHY_D_CAL               (0x0CU)
#define OMV_USB_PHY_TXCAL45DP           (0x06U)
#define OMV_USB_PHY_TXCAL45DM           (0x06U)

// Linker script constants (see the linker script template mimxrt10xx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).

//#define OMV_FFS_MEMORY                  DTCM        // Flash filesystem cache memory
#define OMV_MAIN_MEMORY                 DTCM         // data, bss and heap memory
#define OMV_STACK_MEMORY                ITCM1        // stack memory
#define OMV_RAMFUNC_MEMORY              ITCM2        // RAM code memory.
#define OMV_FB_MEMORY                   DRAM         // Framebuffer, fb_alloc
#define OMV_DMA_MEMORY                  DTCM         // DMA buffers memory.
#define OMV_JPEG_MEMORY                 DRAM         // JPEG buffer memory buffer.
#define OMV_JPEG_MEMORY_OFFSET          (31M)        // JPEG buffer is placed after FB/fballoc memory.
#define OMV_VOSPI_MEMORY                OCRM2        // VoSPI buffer memory.
#define OMV_FB_OVERLAY_MEMORY           OCRM1        // Fast fb_alloc memory.

#define OMV_FB_SIZE                     (10M)        // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE               (2M)         // minimum fb alloc size
#define OMV_FB_OVERLAY_SIZE             (512K)
#define OMV_STACK_SIZE                  (32K)
#define OMV_HEAP_SIZE                   (280K)
#define OMV_SDRAM_SIZE                  (32 * 1024 * 1024)  // This needs to be here for UVC firmware.

#define OMV_LINE_BUF_SIZE               (10 * 1024)  // Image line buffer.
// TODO remove
#define OMV_MSC_BUF_SIZE                (2K)         // USB MSC bot data
#define OMV_VFS_BUF_SIZE                (1K)         // VFS struct + FATFS file buffer (624 bytes)
#define OMV_JPEG_BUF_SIZE               (1024 * 1024)  // IDE JPEG buffer (header + data).

// Memory configuration.
#define OMV_DTCM_ORIGIN                 0x20000000
#define OMV_DTCM_LENGTH                 384K
#define OMV_ITCM1_ORIGIN                0x00000000
#define OMV_ITCM1_LENGTH                32K
#define OMV_ITCM2_ORIGIN                0x00008000
#define OMV_ITCM2_LENGTH                32K
#define OMV_OCRM1_ORIGIN                0x20200000
#define OMV_OCRM1_LENGTH                512K
#define OMV_OCRM2_ORIGIN                0x20280000   // Allocated from FlexRAM.
#define OMV_OCRM2_LENGTH                64K
#define OMV_DRAM_ORIGIN                 0x80000000
#define OMV_DRAM_LENGTH                 32M
#define OMV_FLEXRAM_CONFIG              (0x5AAAAAAF)  // OCRAM: 64K DTCM: 384K ITCM: 64K

// Flash configuration.
#define OMV_FLASH_ORIGIN                0x60000000
#define OMV_FLASH_LENGTH                0x00800000

#define OMV_FLASH_APP_ORIGIN            0x60040000   // 256K reserved for bootloader
#define OMV_FLASH_APP_LENGTH            0x007C0000

#define OMV_FLASH_TXT_ORIGIN            0x60043000
#define OMV_FLASH_TXT_LENGTH            0x00380000

#define OMV_FLASH_FFS_ORIGIN            0x60400000
#define OMV_FLASH_FFS_LENGTH            0x00400000

// Main image sensor I2C bus
#define ISC_I2C_ID                      (1)
#define ISC_I2C_SPEED                   (OMV_I2C_SPEED_STANDARD)

// Thermal image sensor I2C bus
#define FIR_I2C_ID                      (4)
#define FIR_I2C_SPEED                   (OMV_I2C_SPEED_FULL)

// Main image sensor SPI bus
#define ISC_SPI_ID                      (4)

// Physical I2C buses.

// LPI2C1
#define LPI2C1_ID                       (1)
#define LPI2C1_SCL_PIN                  (&omv_pin_LPI2C1_SCL)
#define LPI2C1_SDA_PIN                  (&omv_pin_LPI2C1_SDA)

// LPI2C2
#define LPI2C2_ID                       (2)
#define LPI2C2_SCL_PIN                  (&omv_pin_LPI2C2_SCL)
#define LPI2C2_SDA_PIN                  (&omv_pin_LPI2C2_SDA)

// LPI2C4
#define LPI2C4_ID                       (4)
#define LPI2C4_SCL_PIN                  (&omv_pin_LPI2C4_SCL)
#define LPI2C4_SDA_PIN                  (&omv_pin_LPI2C4_SDA)

// Physical SPI buses.

// LPSPI3
#define LPSPI3_ID                       (3)
#define LPSPI3_SCLK_PIN                 (&omv_pin_LPSPI3_SCLK)
#define LPSPI3_MISO_PIN                 (&omv_pin_LPSPI3_MISO)
#define LPSPI3_MOSI_PIN                 (&omv_pin_LPSPI3_MOSI)
#define LPSPI3_SSEL_PIN                 (&omv_pin_LPSPI3_SSEL)
#define LPSPI3_DMA                      (DMA0)
#define LPSPI3_DMA_MUX                  (DMAMUX)
#define LPSPI3_DMA_TX_CHANNEL           (3U)
#define LPSPI3_DMA_RX_CHANNEL           (2U)

// LPSPI4
#define LPSPI4_ID                       (4)
#define LPSPI4_SCLK_PIN                 (&omv_pin_LPSPI4_SCLK)
#define LPSPI4_MISO_PIN                 (&omv_pin_LPSPI4_MISO)
#define LPSPI4_MOSI_PIN                 (&omv_pin_LPSPI4_MOSI)
#define LPSPI4_SSEL_PIN                 (&omv_pin_LPSPI4_SSEL)
#define LPSPI4_DMA                      (DMA0)
#define LPSPI4_DMA_MUX                  (DMAMUX)
#define LPSPI4_DMA_TX_CHANNEL           (1U)
#define LPSPI4_DMA_RX_CHANNEL           (0U)

// SPI LCD Interface
#define OMV_SPI_LCD_SPI_BUS             (LPSPI3_ID)
#define OMV_SPI_LCD_MOSI_PIN            (&omv_pin_LPSPI3_MOSI)
#define OMV_SPI_LCD_MISO_PIN            (&omv_pin_LPSPI3_MISO)
#define OMV_SPI_LCD_SCLK_PIN            (&omv_pin_LPSPI3_SCLK)
#define OMV_SPI_LCD_SSEL_PIN            (&omv_pin_LPSPI3_GPIO)

#define OMV_SPI_LCD_RS_PIN              (&omv_pin_P8_GPIO)
#define OMV_SPI_LCD_BL_PIN              (&omv_pin_P6_GPIO)
#define OMV_SPI_LCD_RST_PIN             (&omv_pin_P7_GPIO)

#define OMV_SPI_LCD_DEF_TRIPLE_BUF      (true)

#define OMV_SPI_LCD_RX_CLK_DIV          (8)

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS          (FIR_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED    (FIR_I2C_SPEED)

#define OMV_FIR_LEPTON_SPI_BUS          (LPSPI3_ID)
#define OMV_FIR_LEPTON_MOSI_PIN         (&omv_pin_LPSPI3_MOSI)
#define OMV_FIR_LEPTON_MISO_PIN         (&omv_pin_LPSPI3_MISO)
#define OMV_FIR_LEPTON_SCLK_PIN         (&omv_pin_LPSPI3_SCLK)
#define OMV_FIR_LEPTON_SSEL_PIN         (&omv_pin_LPSPI3_GPIO)

#define OMV_FIR_LEPTON_RX_CLK_DIV       (8)

// Camera interface configuration.
#define OMV_CSI_BASE                    (CSI)

#define DCMI_RESET_PIN                  (&omv_pin_DCMI_RESET)
#define DCMI_POWER_PIN                  (&omv_pin_DCMI_POWER)
//#define DCMI_FSYNC_PIN                  (&omv_pin_DCMI_FSYNC)

#define DCMI_D0_PIN                     (&omv_pin_DCMI_D0)
#define DCMI_D1_PIN                     (&omv_pin_DCMI_D1)
#define DCMI_D2_PIN                     (&omv_pin_DCMI_D2)
#define DCMI_D3_PIN                     (&omv_pin_DCMI_D3)
#define DCMI_D4_PIN                     (&omv_pin_DCMI_D4)
#define DCMI_D5_PIN                     (&omv_pin_DCMI_D5)
#define DCMI_D6_PIN                     (&omv_pin_DCMI_D6)
#define DCMI_D7_PIN                     (&omv_pin_DCMI_D7)
#define DCMI_MCLK_PIN                   (&omv_pin_DCMI_MCLK)
#define DCMI_HSYNC_PIN                  (&omv_pin_DCMI_HSYNC)
#define DCMI_VSYNC_PIN                  (&omv_pin_DCMI_VSYNC)
#define DCMI_PXCLK_PIN                  (&omv_pin_DCMI_PXCLK)
#endif //__OMV_BOARDCONFIG_H__
