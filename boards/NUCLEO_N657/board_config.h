/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Board configuration and pin definitions for the ST NUCLEO-N657X0-Q.
 *
 * Derived from boards/OPENMV_N6. Differences from the OpenMV N6:
 *  - No external RAM: all memory pools live in the 4.2MB of internal SRAM.
 *  - Camera: 22-pin MIPI CSI-2 FFC connector (I2C2 on PB10/PB11, enable PA0,
 *    reset PO5). The camera module supplies its own clock (no XCLK pin).
 *  - VDDIO2 is 3.3V on the Nucleo: OMV_VDDIO2_1V8 = 0 (never burn the VDDIO2 HSLV fuse).
 *  - No WiFi/BT, SD card, IMU, SPI display, microphone or FIR/TOF connectors.
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

// Architecture info
#define OMV_BOARD_ARCH                      "NUCLEO-N657X0-Q" // 33 chars max
#define OMV_BOARD_TYPE                      "NUCLEO_N6"
#define OMV_BOARD_UID_ADDR                  0x46009014    // Unique ID address.
#define OMV_BOARD_UID_SIZE                  3             // Unique ID size in words.
#define OMV_BOARD_UID_OFFSET                4             // Bytes offset for multi-word UIDs.

// VDDIO2 is powered at 3.3V on the Nucleo (see lib/cmsis/src/st/system_stm32n6.c).
#define OMV_VDDIO2_1V8                      (0)

// JPEG compression settings.
#define OMV_JPEG_CODEC_ENABLE               (1)
#define OMV_JPEG_QUALITY_LOW                (50)
#define OMV_JPEG_QUALITY_HIGH               (90)
#define OMV_JPEG_QUALITY_THRESHOLD          (800 * 600 * 2)

// Enable RAW preview.
#define OMV_RAW_PREVIEW_ENABLE              (1)
#define OMV_RAW_PREVIEW_WIDTH               (512)
#define OMV_RAW_PREVIEW_HEIGHT              (512)

// GPU Configuration
#define OMV_GPU_ENABLE                      (1)
#define OMV_GPU_NEMA                        (1)
#define OMV_GPU_NEMA_BUFFER_SIZE            (32 * 1024)

// VENC Configuration
#define OMV_VENC_CODEC_ENABLE               (1)

// Image sensors reachable through the MIPI CSI-2 connector.
#define OMV_VD66GY_ENABLE                   (1)
#define OMV_VD66GY_CLK_FREQ                 (12000000) // Oscillator on the STEVAL-66GYMAI module.
#define OMV_PAG7936_ENABLE                  (1)        // PAG7936 in MIPI mode (I2C 0x2A).
#define OMV_PS5520_ENABLE                   (1)
#define OMV_SOFTCSI_ENABLE                  (1)

// USB IRQn.
#define OMV_USB_IRQN                        (USB1_OTG_HS_IRQn)

// OpenMV protocol configuration.
#define OMV_PROTOCOL_MAX_BUFFER_SIZE        (8192)
#define OMV_PROTOCOL_STDIO_BUFFER_SIZE      (2048)
#define OMV_PROTOCOL_HW_CAPS                OMV_PROTOCOL_HW_CAPS_MAKE( \
        HAS_GPU, HAS_ISP, HAS_VENC, HAS_JPEG, HAS_CRC, HAS_PMU,        \
        HAS_ETH, HAS_USB_HS)

// HSE is 48MHz on the Nucleo, same as the OpenMV N6.
//PLL1 800MHz
#define OMV_OSC_PLL1M                       (3)
#define OMV_OSC_PLL1N                       (50)
#define OMV_OSC_PLL1P1                      (1)
#define OMV_OSC_PLL1P2                      (1)
#define OMV_OSC_PLL1FRAC                    (0)
#define OMV_OSC_PLL1SOURCE                  RCC_PLLSOURCE_HSE

//PLL2 1000MHz
#define OMV_OSC_PLL2M                       (6)
#define OMV_OSC_PLL2N                       (125)
#define OMV_OSC_PLL2P1                      (1)
#define OMV_OSC_PLL2P2                      (1)
#define OMV_OSC_PLL2FRAC                    (0)
#define OMV_OSC_PLL2SOURCE                  RCC_PLLSOURCE_HSE

//PLL3  1200MHz
#define OMV_OSC_PLL3M                       (1)
#define OMV_OSC_PLL3N                       (25)
#define OMV_OSC_PLL3P1                      (1)
#define OMV_OSC_PLL3P2                      (1)
#define OMV_OSC_PLL3FRAC                    (0)
#define OMV_OSC_PLL3SOURCE                  RCC_PLLSOURCE_HSE

//PLL4  1200MHz
#define OMV_OSC_PLL4M                       (1)
#define OMV_OSC_PLL4N                       (25)
#define OMV_OSC_PLL4P1                      (1)
#define OMV_OSC_PLL4P2                      (1)
#define OMV_OSC_PLL4FRAC                    (0)
#define OMV_OSC_PLL4SOURCE                  RCC_PLLSOURCE_HSE

// Clock Sources
#define OMV_RCC_IC8_SOURCE                  (RCC_ICCLKSOURCE_PLL3)
#define OMV_RCC_IC8_CLKDIV                  (25)

#define OMV_RCC_IC10_SOURCE                 (RCC_ICCLKSOURCE_PLL1)
#define OMV_RCC_IC10_CLKDIV                 (8)

// Used by MicroPython for ethernet clocks.
#define OMV_RCC_IC12_SOURCE                 (RCC_ICCLKSOURCE_PLL1)
#define OMV_RCC_IC12_CLKDIV                 (8)

// Used by MicroPython for slow peripherals.
#define OMV_RCC_IC14_SOURCE                 (RCC_ICCLKSOURCE_PLL1)
#define OMV_RCC_IC14_CLKDIV                 (8)

#define OMV_RCC_IC15_SOURCE                 (RCC_ICCLKSOURCE_PLL1)
#define OMV_RCC_IC15_CLKDIV                 (16)

#define OMV_RCC_IC17_SOURCE                 (RCC_ICCLKSOURCE_PLL3)
#define OMV_RCC_IC17_CLKDIV                 (4)

#define OMV_RCC_IC18_SOURCE                 (RCC_ICCLKSOURCE_PLL3)
#define OMV_RCC_IC18_CLKDIV                 (60)

// I2C kernel clocks from IC10 (100MHz), which the STM32N6 I2C timing values assume.
#define OMV_OSC_I2C1_SOURCE                 (RCC_I2C1CLKSOURCE_IC10)
#define OMV_OSC_I2C2_SOURCE                 (RCC_I2C2CLKSOURCE_IC10)
#define OMV_OSC_I2C3_SOURCE                 (RCC_I2C3CLKSOURCE_IC10)
#define OMV_OSC_I2C4_SOURCE                 (RCC_I2C4CLKSOURCE_IC10)
#define OMV_OSC_SPI5_SOURCE                 (RCC_SPI5CLKSOURCE_IC14)

#define OMV_OSC_DCMIPP_SOURCE               (RCC_DCMIPPCLKSOURCE_IC17)
#define OMV_OSC_CSI_SOURCE                  (0) // has one clock source IC18

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE                   (RCC_HSE_ON)
#define OMV_OSC_HSI_STATE                   (RCC_HSI_ON)
#define OMV_OSC_HSI_DIV                     (RCC_HSI_DIV1)
#define OMV_OSC_HSI_CAL                     (RCC_HSICALIBRATION_DEFAULT)

// Power supply configuration
#define OMV_PWR_SUPPLY                      (PWR_SMPS_SUPPLY)

// Linker script constants (see common.ld.S).
//
// Internal SRAM only (no external RAM on the Nucleo):
//   ITCM 64KB    : cashew detector code (cashew_core.o) + flash driver ramfuncs
//   DTCM 128KB   : 32KB stack + 96KB pool for the detector's per-row work buffers
//   SRAM1 1MB    : data/bss, libc heap, DMA buffers, fast UMA pool
//   SRAM2 1MB    : MicroPython GC heap (768K) + IDE streaming buffer (256K)
//   SRAM3 1.75MB : default UMA pool (frame buffers, cashew workspace).
//   The NPU/ML module is disabled on this board: SRAM3 is the NPU's working memory.
#define OMV_MAIN_MEMORY                     SRAM1  // Data/BSS memory
#define OMV_STACK_MEMORY                    DTCM   // stack memory (zero wait state)
#define OMV_RAMFUNC_MEMORY                  ITCM
#define OMV_STACK_SIZE                      (32K)
#define OMV_HEAP_MEMORY                     SRAM1  // libc/sbrk heap memory
#define OMV_HEAP_SIZE                       (128K)
#define OMV_SB_MEMORY                       SRAM2  // Streaming buffer memory.
#define OMV_SB_SIZE                         (256K) // Streaming buffer size.
#define OMV_DMA_MEMORY                      SRAM1  // Misc DMA buffers memory.
#define OMV_DMA_MEMORY_D2                   SRAM7  // Domain 2 DMA buffers.
#define OMV_GC_BLOCK0_MEMORY                SRAM2  // Main GC block
#define OMV_GC_BLOCK0_SIZE                  (768K)
#define OMV_UMA_BLOCK0_MEMORY               SRAM3  // Default UMA pool.
#define OMV_UMA_BLOCK0_SIZE                 (1792K)
#define OMV_UMA_BLOCK0_FLAGS                (UMA_FAST | UMA_DTCM | UMA_DEFAULT)
#define OMV_UMA_BLOCK1_MEMORY               SRAM1  // Fast UMA pool.
#define OMV_UMA_BLOCK1_SIZE                 (648K) // +64K freed by moving the stack to DTCM
#define OMV_UMA_BLOCK1_FLAGS                (UMA_FAST | UMA_DTCM)
#define OMV_UMA_BLOCK2_MEMORY               DTCM   // DTCM UMA pool (cashew hot work buffers).
#define OMV_UMA_BLOCK2_SIZE                 (96K)
#define OMV_UMA_BLOCK2_FLAGS                (UMA_DTCM)
#define OMV_MSC_BUF_SIZE                    (4K)   // USB MSC bot data
#define OMV_VOSPI_DMA_BUFFER                ".d2_dma_buffer"

// Memory map.
#define OMV_DTCM_ORIGIN                     0x30000000
#define OMV_DTCM_LENGTH                     128K
#define OMV_ITCM_ORIGIN                     0x10000000
#define OMV_ITCM_LENGTH                     64K
#define OMV_SRAM1_ORIGIN                    0x34000000  // AXISRAM1
#define OMV_SRAM1_LENGTH                    1M          // 1MB
#define OMV_SRAM2_ORIGIN                    0x34100000  // AXISRAM2
#define OMV_SRAM2_LENGTH                    1M          // 1MB
#define OMV_SRAM3_ORIGIN                    0x34200000  // AXISRAM3-6 (NPU AXI SRAMs)
#define OMV_SRAM3_LENGTH                    1792K       // 4 x 448KB
#define OMV_SRAM7_ORIGIN                    0x38000000  // AHBSRAM1 + AHBSRAM2 combined
#define OMV_SRAM7_LENGTH                    32K         // 16KB + 16KB = 32KB

// Flash configuration (MX25UM51245G on XSPI2, same layout as the OpenMV N6).
#define OMV_FLASH_BOOT_ORIGIN               0x34180400
#define OMV_FLASH_BOOT_LENGTH               512K
#define OMV_FLASH_TXT_ORIGIN                0x70080000
#define OMV_FLASH_TXT_LENGTH                3584K
#define OMV_ROMFS_PART0_ORIGIN              0x70800000
#define OMV_ROMFS_PART0_LENGTH              0x01800000

// Enable additional GPIO ports.
#define OMV_GPIO_PORT_F_ENABLE              (1)
#define OMV_GPIO_PORT_G_ENABLE              (1)
#define OMV_GPIO_PORT_H_ENABLE              (1)
#define OMV_GPIO_PORT_N_ENABLE              (1)
#define OMV_GPIO_PORT_O_ENABLE              (1)
#define OMV_GPIO_PORT_P_ENABLE              (1)
#define OMV_GPIO_PORT_Q_ENABLE              (1)

// Physical I2C buses.

// I2C bus 1 (Arduino D14/D15)
#define OMV_I2C1_ID                         (1)
#define OMV_I2C1_SCL_PIN                    (&omv_pin_H9_I2C1)
#define OMV_I2C1_SDA_PIN                    (&omv_pin_C1_I2C1)

// I2C bus 2 (camera connector)
#define OMV_I2C2_ID                         (2)
#define OMV_I2C2_SCL_PIN                    (&omv_pin_B10_I2C2)
#define OMV_I2C2_SDA_PIN                    (&omv_pin_B11_I2C2)

// I2C bus 4
#define OMV_I2C4_ID                         (4)
#define OMV_I2C4_SCL_PIN                    (&omv_pin_E13_I2C4)
#define OMV_I2C4_SDA_PIN                    (&omv_pin_E14_I2C4)

// Physical SPI buses.

// SPI bus 5 (Arduino D10-D13)
#define OMV_SPI5_ID                         (5)
#define OMV_SPI5_SCLK_PIN                   (&omv_pin_E15_SPI5)
#define OMV_SPI5_MISO_PIN                   (&omv_pin_G1_SPI5)
#define OMV_SPI5_MOSI_PIN                   (&omv_pin_G2_SPI5)
#define OMV_SPI5_SSEL_PIN                   (&omv_pin_A3_SPI5)
#define OMV_SPI5_DMA_TX_CHANNEL             (GPDMA1_Channel13)
#define OMV_SPI5_DMA_TX_REQUEST             (GPDMA1_REQUEST_SPI5_TX)
#define OMV_SPI5_DMA_RX_CHANNEL             (GPDMA1_Channel14)
#define OMV_SPI5_DMA_RX_REQUEST             (GPDMA1_REQUEST_SPI5_RX)
#define OMV_SPI_DMA_LIST_PORTS              (DMA_LINK_ALLOCATED_PORT0)
#define OMV_SPI_DMA_XFER_PORTS              (DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1)

// CSI SPI bus
#define OMV_CSI_SPI_ID                      (OMV_SPI5_ID)

// CSI I2C bus (camera connector)
#define OMV_CSI_I2C_ID                      (OMV_I2C2_ID)
#define OMV_CSI_I2C_SPEED                   (OMV_I2C_SPEED_FULL)

// Camera Interface (MIPI CSI-2 only; the module has its own oscillator).
#define OMV_CSI_CLK_SOURCE                  (OMV_CSI_CLK_SOURCE_OSC)
#define OMV_CSI_CLK_FREQUENCY               (12000000)
#define OMV_CSI_DMA_CHANNEL                 (HPDMA1_Channel12)
#define OMV_CSI_DMA_REQUEST                 (HPDMA1_REQUEST_DCMI_PSSI)
#define OMV_CSI_DMA_MEMCPY_ENABLE           (0)
#define OMV_CSI_DMA_LIST_PORTS              (DMA_LINK_ALLOCATED_PORT0)
#define OMV_CSI_DMA_XFER_PORTS              (DMA_SRC_ALLOCATED_PORT1 | DMA_DEST_ALLOCATED_PORT0)
#define OMV_CSI_HW_CROP_ENABLE              (1)
#define OMV_CSI_MAX_DEVICES                 (3)
#define OMV_CSI_STATS_ENABLE                (1)
#define OMV_CSI_HW_SCALE_ENABLE             (1)

// Camera connector control lines: EN (power enable, high = on) and NRST (low = reset).
#define OMV_CSI_RESET_PIN                   (&omv_pin_O5_GPIO)
#define OMV_CSI_POWER_PIN                   (&omv_pin_A0_GPIO)
#define OMV_CSI_RESET_DELAY                 (50)
#define OMV_CSI_POWER_DELAY                 (20)
// { power-down level, reset level }: EN low = off, NRST low = reset.
#define OMV_CSI_POLARITY_CONFIG             { OMV_CSI_ACTIVE_LOW, OMV_CSI_ACTIVE_LOW },
#endif //__BOARD_CONFIG_H__
