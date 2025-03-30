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
 * Alif HAL.
 */
#include <stdio.h>
#include <stdint.h>

#include "irq.h"
#include CMSIS_MCU_H

#include "py/mphal.h"
#include "alif_hal.h"
#include "sys_ctrl_i3c.h"
#include "sys_ctrl_spi.h"
#include "sys_ctrl_cpi.h"
#include "sys_ctrl_pdm.h"
#include "se_services.h"
#include "omv_boardconfig.h"
// Define pin objects in this file.
#define OMV_GPIO_DEFINE_PINS    (1)
#include "omv_gpio.h"

#define MPU_ATTR_NORMAL_WT_RA_TRANSIENT (0)
#define MPU_ATTR_DEVICE_nGnRE           (1)
#define MPU_ATTR_NORMAL_WB_RA_WA        (2)
#define MPU_ATTR_NORMAL_WT_RA           (3)
#define MPU_ATTR_NORMAL_NON_CACHEABLE   (4)

#define MPU_REGION_SRAM0                (0)
#define MPU_REGION_SRAM1                (1)
#define MPU_REGION_HOST_PERIPHERALS     (2)
#define MPU_REGION_MRAM                 (3)
#define MPU_REGION_OSPI_REGISTERS       (4)
#define MPU_REGION_OSPI0_XIP            (5)
#define MPU_REGION_OPENAMP              (6)
#define MPU_REGION_FIRST_FREE           (7) // Reserve the first 8 regions.

uint8_t OMV_BOARD_UID_ADDR[12];

void alif_hal_mpu_init(void);
extern int alif_npu_init(void);
extern void __fatal_error(const char *msg);

void alif_hal_init(void) {
    // Disable BOR.
    ANA_REG->VBAT_ANA_REG1 |= (1 << 25);

    // Disable USB mux.
    #if CORE_M55_HP && defined(OMV_USB_SWITCH_PIN)
    omv_gpio_config(OMV_USB_SWITCH_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_USB_SWITCH_PIN, 1);
    #endif

    // Init services.
    se_services_init();

    // Configures run/off profiles and SRAM clock source.
    board_early_init();

    // Set board unique ID.
    se_services_get_unique_id(OMV_BOARD_UID_ADDR);
    // To match the USB serial.
    for (size_t i = 0, j = sizeof(OMV_BOARD_UID_ADDR) - 1; i < j; i++, j--) {
        OMV_BOARD_UID_ADDR[i] ^= OMV_BOARD_UID_ADDR[j];
        OMV_BOARD_UID_ADDR[j] ^= OMV_BOARD_UID_ADDR[i];
        OMV_BOARD_UID_ADDR[i] ^= OMV_BOARD_UID_ADDR[j];
    }

    // load MPU regions and enable the MPU.
    alif_hal_mpu_init();

    // Configure and initialize the NPU.
    alif_npu_init();

    // Configure and enable USB IRQs.
    NVIC_ClearPendingIRQ(USB_IRQ_IRQn);
    NVIC_SetPriority(USB_IRQ_IRQn, IRQ_PRI_USB);
}

int alif_hal_i2c_init(uint32_t bus_id) {
    omv_gpio_t scl_pin = NULL;
    omv_gpio_t sda_pin = NULL;

    switch (bus_id) {
        #if defined(OMV_I3C0_ID)
        case OMV_I3C0_ID: {
            enable_i3c_clock();
            scl_pin = OMV_I3C0_SCL_PIN;
            sda_pin = OMV_I3C0_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C0_ID)
        case OMV_I2C0_ID: {
            scl_pin = OMV_I2C0_SCL_PIN;
            sda_pin = OMV_I2C0_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C1_ID)
        case OMV_I2C1_ID: {
            scl_pin = OMV_I2C1_SCL_PIN;
            sda_pin = OMV_I2C1_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C2_ID)
        case OMV_I2C2_ID: {
            scl_pin = OMV_I2C2_SCL_PIN;
            sda_pin = OMV_I2C2_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C3_ID)
        case OMV_I2C3_ID: {
            scl_pin = OMV_I2C3_SCL_PIN;
            sda_pin = OMV_I2C3_SDA_PIN;
            break;
        }
        #endif
        default:
            return -1;
    }

    omv_gpio_config(scl_pin, OMV_GPIO_MODE_ALT_OD, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_config(sda_pin, OMV_GPIO_MODE_ALT_OD, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);

    return 0;
}

int alif_hal_spi_init(uint32_t bus_id, bool nss_enable, uint32_t nss_pol) {
    typedef struct {
        omv_gpio_t sclk_pin;
        omv_gpio_t miso_pin;
        omv_gpio_t mosi_pin;
        omv_gpio_t ssel_pin;
    } spi_pins_t;

    spi_pins_t spi_pins = { NULL, NULL, NULL, NULL };

    switch (bus_id) {
        #if defined(OMV_SPI0_ID)
        case OMV_SPI0_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI0_SCLK_PIN, OMV_SPI0_MISO_PIN, OMV_SPI0_MOSI_PIN, OMV_SPI0_SSEL_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_SPI1_ID)
        case OMV_SPI1_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI1_SCLK_PIN, OMV_SPI1_MISO_PIN, OMV_SPI1_MOSI_PIN, OMV_SPI1_SSEL_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_SPI2_ID)
        case OMV_SPI2_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI2_SCLK_PIN, OMV_SPI2_MISO_PIN, OMV_SPI2_MOSI_PIN, OMV_SPI2_SSEL_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_SPI3_ID)
        case OMV_SPI3_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI3_SCLK_PIN, OMV_SPI3_MISO_PIN, OMV_SPI3_MOSI_PIN, OMV_SPI3_SSEL_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_SPI4_ID)
        case OMV_SPI4_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI4_SCLK_PIN, OMV_SPI4_MISO_PIN, OMV_SPI4_MOSI_PIN, OMV_SPI4_SSEL_PIN
            };
            enable_lpspi_clk();
            break;
        }
        #endif
        default:
            return -1;
    }

    omv_gpio_config(spi_pins.sclk_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(spi_pins.miso_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(spi_pins.mosi_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    if (nss_enable) {
        omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MED, -1);
    } else {
        omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MED, 0);
        if (nss_pol == OMV_SPI_NSS_LOW) {
            omv_gpio_write(spi_pins.ssel_pin, 1);
        } else {
            omv_gpio_write(spi_pins.ssel_pin, 0);
        }
    }
    return 0;
}

int alif_hal_spi_deinit(uint32_t bus_id) {
    typedef struct {
        omv_gpio_t sclk_pin;
        omv_gpio_t miso_pin;
        omv_gpio_t mosi_pin;
        omv_gpio_t ssel_pin;
    } spi_pins_t;

    spi_pins_t spi_pins = { NULL, NULL, NULL, NULL };

    switch (bus_id) {
        #if defined(OMV_SPI0_ID)
        case OMV_SPI0_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI0_SCLK_PIN, OMV_SPI0_MISO_PIN, OMV_SPI0_MOSI_PIN, OMV_SPI0_SSEL_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_SPI1_ID)
        case OMV_SPI1_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI1_SCLK_PIN, OMV_SPI1_MISO_PIN, OMV_SPI1_MOSI_PIN, OMV_SPI1_SSEL_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_SPI2_ID)
        case OMV_SPI2_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI2_SCLK_PIN, OMV_SPI2_MISO_PIN, OMV_SPI2_MOSI_PIN, OMV_SPI2_SSEL_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_SPI3_ID)
        case OMV_SPI3_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI3_SCLK_PIN, OMV_SPI3_MISO_PIN, OMV_SPI3_MOSI_PIN, OMV_SPI3_SSEL_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_SPI4_ID)
        case OMV_SPI4_ID: {
            spi_pins = (spi_pins_t) {
                OMV_SPI4_SCLK_PIN, OMV_SPI4_MISO_PIN, OMV_SPI4_MOSI_PIN, OMV_SPI4_SSEL_PIN
            };
            disable_lpspi_clk();
            break;
        }
        #endif
        default:
            return -1;
    }

    omv_gpio_deinit(spi_pins.sclk_pin);
    omv_gpio_deinit(spi_pins.miso_pin);
    omv_gpio_deinit(spi_pins.mosi_pin);
    omv_gpio_deinit(spi_pins.ssel_pin);

    return 0;
}

int alif_hal_pdm_init(uint32_t pdm_id) {
    typedef struct {
        omv_gpio_t clk_pin;
        omv_gpio_t dat_pin;
    } pdm_pins_t;

    pdm_pins_t pdm_pins = { NULL, NULL };

    switch (pdm_id) {
        #if defined(OMV_PDM0_ID)
        case OMV_PDM0_ID: {
            enable_pdm_periph_clk();
            pdm_pins = (pdm_pins_t) {
                OMV_PDM0_C0_PIN, OMV_PDM0_D0_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_PDM1_ID)
        case OMV_PDM1_ID: {
            enable_lppdm_periph_clk();
            pdm_pins = (pdm_pins_t) {
                OMV_PDM1_C0_PIN, OMV_PDM1_D0_PIN
            };
            break;
        }
        #endif
        default:
            return -1;
    }

    omv_gpio_config(pdm_pins.clk_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(pdm_pins.dat_pin, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    return 0;
}

int alif_hal_pdm_deinit(uint32_t pdm_id) {
    typedef struct {
        omv_gpio_t clk_pin;
        omv_gpio_t dat_pin;
    } pdm_pins_t;

    pdm_pins_t pdm_pins = { NULL, NULL };

    switch (pdm_id) {
        #if defined(OMV_PDM0_ID)
        case OMV_PDM0_ID: {
            disable_pdm_periph_clk();
            pdm_pins = (pdm_pins_t) {
                OMV_PDM0_C0_PIN, OMV_PDM0_D0_PIN
            };
            break;
        }
        #endif
        #if defined(OMV_PDM1_ID)
        case OMV_PDM1_ID: {
            disable_lppdm_periph_clk();
            pdm_pins = (pdm_pins_t) {
                OMV_PDM1_C0_PIN, OMV_PDM1_D0_PIN
            };
            break;
        }
        #endif
        default:
            return -1;
    }

    omv_gpio_deinit(pdm_pins.clk_pin);
    omv_gpio_deinit(pdm_pins.dat_pin);
    return 0;
}

int alif_hal_csi_init(CPI_Type *cpi, uint32_t mode) {
    if (mode == 0) {
        enable_cpi_periph_clk();
    } else {
        enable_lpcpi_periph_clk();
    }

    // Configure camera sensor interface pins
    omv_gpio_config(OMV_CSI_D0_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_D1_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_D2_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_D3_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_D4_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_D5_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_D6_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_D7_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);

    omv_gpio_config(OMV_CSI_HSYNC_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_VSYNC_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_PXCLK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(OMV_CSI_MXCLK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);

    // Configure DCMI GPIOs
    #if defined(OMV_CSI_RESET_PIN)
    omv_gpio_config(OMV_CSI_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_DOWN, OMV_GPIO_SPEED_LOW, -1);
    #endif
    #if defined(OMV_CSI_FSYNC_PIN)
    omv_gpio_config(OMV_CSI_FSYNC_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_DOWN, OMV_GPIO_SPEED_LOW, -1);
    #endif
    #if defined(OMV_CSI_POWER_PIN)
    omv_gpio_config(OMV_CSI_POWER_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    #endif

    NVIC_SetPriority(CAM_IRQ_IRQn, IRQ_PRI_CSI);
    return 0;
}

void alif_hal_mpu_init(void) {
    static const ARM_MPU_Region_t MPU_CONFIG_TABLE[] __STARTUP_RO_DATA_ATTRIBUTE = {
        [MPU_REGION_SRAM0] = {   /* SRAM0 - 4MB : RO-0, NP-1, XN-0 */
            .RBAR = ARM_MPU_RBAR(0x02000000, ARM_MPU_SH_NON, 0, 1, 0),
            .RLAR = ARM_MPU_RLAR(0x023FFFFF, MPU_ATTR_NORMAL_WT_RA_TRANSIENT)
        },
        [MPU_REGION_SRAM1] = {   /* SRAM1 - 2.5MB : RO-0, NP-1, XN-0 */
            .RBAR = ARM_MPU_RBAR(0x08000000, ARM_MPU_SH_NON, 0, 1, 0),
            .RLAR = ARM_MPU_RLAR(0x0827FFFF, MPU_ATTR_NORMAL_WB_RA_WA)
        },
        [MPU_REGION_HOST_PERIPHERALS] = {   /* Host Peripherals - 16MB : RO-0, NP-1, XN-1 */
            .RBAR = ARM_MPU_RBAR(0x1A000000, ARM_MPU_SH_NON, 0, 1, 1),
            .RLAR = ARM_MPU_RLAR(0x1AFFFFFF, MPU_ATTR_DEVICE_nGnRE)
        },
        [MPU_REGION_MRAM] = {   /* MRAM - 5.5MB : RO-1, NP-1, XN-0  */
            .RBAR = ARM_MPU_RBAR(0x80000000, ARM_MPU_SH_NON, 1, 1, 0),
            .RLAR = ARM_MPU_RLAR(0x8057FFFF, MPU_ATTR_NORMAL_WT_RA)
        },
        [MPU_REGION_OSPI_REGISTERS] = {   /* OSPI Regs - 16MB : RO-0, NP-1, XN-1  */
            .RBAR = ARM_MPU_RBAR(0x83000000, ARM_MPU_SH_NON, 0, 1, 1),
            .RLAR = ARM_MPU_RLAR(0x83FFFFFF, MPU_ATTR_DEVICE_nGnRE)
        },
        [MPU_REGION_OSPI0_XIP] = {   /* OSPI0 XIP flash - 512MB : RO-1, NP-1, XN-0  */
            .RBAR = ARM_MPU_RBAR(0xA0000000, ARM_MPU_SH_NON, 1, 1, 0),
            .RLAR = ARM_MPU_RLAR(0xBFFFFFFF, MPU_ATTR_NORMAL_NON_CACHEABLE)
        },
    };

    // Disable IRQs.
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    // Disable MPU
    ARM_MPU_Disable();

    // Clear all regions.
    for (size_t i = 0; i < (MPU->TYPE >> 8); i++) {
        ARM_MPU_ClrRegion(i);
    }

    // Mem Attribute for 0th index
    ARM_MPU_SetMemAttr(MPU_ATTR_NORMAL_WT_RA_TRANSIENT, ARM_MPU_ATTR(
                           /* NT=0, WB=0, RA=1, WA=0 */
                           ARM_MPU_ATTR_MEMORY_(0, 0, 1, 0),
                           ARM_MPU_ATTR_MEMORY_(0, 0, 1, 0)));

    // Mem Attribute for 1st index
    ARM_MPU_SetMemAttr(MPU_ATTR_DEVICE_nGnRE, ARM_MPU_ATTR(
                           /* Device Memory */
                           ARM_MPU_ATTR_DEVICE,
                           ARM_MPU_ATTR_DEVICE_nGnRE));

    // Mem Attribute for 2nd index
    ARM_MPU_SetMemAttr(MPU_ATTR_NORMAL_WB_RA_WA, ARM_MPU_ATTR(
                           /* NT=1, WB=1, RA=1, WA=1 */
                           ARM_MPU_ATTR_MEMORY_(1, 1, 1, 1),
                           ARM_MPU_ATTR_MEMORY_(1, 1, 1, 1)));

    // Mem Attribute for 3th index
    ARM_MPU_SetMemAttr(MPU_ATTR_NORMAL_WT_RA, ARM_MPU_ATTR(
                           /* NT=1, WB=0, RA=1, WA=0 */
                           ARM_MPU_ATTR_MEMORY_(1, 0, 1, 0),
                           ARM_MPU_ATTR_MEMORY_(1, 0, 1, 0)));

    // Mem Attribute for 4th index
    ARM_MPU_SetMemAttr(MPU_ATTR_NORMAL_NON_CACHEABLE, ARM_MPU_ATTR(
                           ARM_MPU_ATTR_NON_CACHEABLE,
                           ARM_MPU_ATTR_NON_CACHEABLE));

    // Load the MPU regions.
    ARM_MPU_Load(0, MPU_CONFIG_TABLE, sizeof(MPU_CONFIG_TABLE) / sizeof(ARM_MPU_Region_t));

    // Configure MPU regions.
    typedef struct {
        uint32_t addr;
        uint32_t size;
    } dma_memory_table_t;

    // Start after the last region used by the startup code.
    uint8_t region_number = MPU_REGION_FIRST_FREE;
    extern const dma_memory_table_t _dma_memory_table_start;
    extern const dma_memory_table_t _dma_memory_table_end;

    for (dma_memory_table_t const *buf = &_dma_memory_table_start; buf < &_dma_memory_table_end; buf++) {
        uint32_t region_base = buf->addr;
        uint32_t region_size = buf->size;
        if (region_size) {
            MPU->RNR = region_number++;
            MPU->RBAR = ARM_MPU_RBAR(region_base, ARM_MPU_SH_NON, 0, 1, 0); // RO-0, NP-1, XN-0
            MPU->RLAR = ARM_MPU_RLAR(region_base + region_size - 1, MPU_ATTR_NORMAL_NON_CACHEABLE);
        }
    }

    // Re-enable MPU
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);

    // Re-enable IRQs.
    __set_PRIMASK(primask);
}

// Bypass startup MPU defaults.
void MPU_Setup(void) {

}

// Bypass startup MPU defaults.
void MPU_Load_Regions(void) {

}

// Used by MicroPython VFS.
void mpu_config_mram(bool read_only) {
    // Disable IRQs.
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    // Disable MPU
    ARM_MPU_Disable();

    MPU->RNR = MPU_REGION_MRAM;
    MPU->RBAR = ARM_MPU_RBAR(MRAM_BASE, ARM_MPU_SH_NON, read_only, 1, 0);
    MPU->RLAR = ARM_MPU_RLAR(MRAM_BASE + MRAM_SIZE - 1, MPU_ATTR_NORMAL_WT_RA);

    // Re-enable MPU
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);

    // Re-enable IRQs.
    __set_PRIMASK(primask);
}
