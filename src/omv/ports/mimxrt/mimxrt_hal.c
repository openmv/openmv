/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MIMXRT HAL.
 */

#include "fsl_gpio.h"
#include "fsl_csi.h"
#include "fsl_iomuxc.h"
#include "fsl_clock.h"
#include "fsl_lpuart.h"
#include "fsl_lpi2c.h"
#include "fsl_romapi.h"
#include "fsl_dmamux.h"
#include "fsl_usb_phy.h"
#include "fsl_device_registers.h"
#include CLOCK_CONFIG_H
#include "irq.h"
#include CMSIS_MCU_H

#include "omv_boardconfig.h"
// Define pin objects in this file.
#define OMV_GPIO_DEFINE_PINS    (1)
#include "omv_gpio.h"
#include "mimxrt_hal.h"

const uint8_t dcd_data[] = {0};

void mimxrt_hal_init() {
    // Configure and enable clocks.
    BOARD_BootClockRUN();
    SystemCoreClockUpdate();

    // Default priority grouping.
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // Configure Systick at 1Hz.
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_SetPriority(SysTick_IRQn, IRQ_PRI_SYSTICK);

    // Enable I/O clocks.
    CLOCK_EnableClock(kCLOCK_Iomuxc);
    CLOCK_EnableClock(kCLOCK_IomuxcSnvs);

    // Enable I cache and D cache
    SCB_EnableDCache();
    SCB_EnableICache();

    // SDRAM
    #ifdef OMV_SDRAM_SIZE
    extern void mimxrt_sdram_init(void);
    mimxrt_sdram_init();
    #endif

    // Configure USB physical
    usb_phy_config_struct_t phyConfig = {
        OMV_USB_PHY_D_CAL,
        OMV_USB_PHY_TXCAL45DP,
        OMV_USB_PHY_TXCAL45DM,
    };

    if (OMV_USB_PHY_ID == kUSB_ControllerEhci0) {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    } else {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    USB_EhciPhyInit(OMV_USB_PHY_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
    NVIC_SetPriority(USB_OTG1_IRQn, IRQ_PRI_OTG_HS);

    // Configure shared IO multiplexers.
    #if defined(OMV_IOMUXC_GPR26_CONFIG)
    IOMUXC_GPR->GPR26 = ((IOMUXC_GPR->GPR26 &
                          (~(OMV_IOMUXC_GPR26_CONFIG)))
                         | IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL(0x00U)
                         );
    #endif
    #if defined(OMV_IOMUXC_GPR27_CONFIG)
    IOMUXC_GPR->GPR27 = ((IOMUXC_GPR->GPR27 &
                          (~(OMV_IOMUXC_GPR27_CONFIG)))
                         | IOMUXC_GPR_GPR27_GPIO_MUX2_GPIO_SEL(0x00U)
                         );
    #endif

    // Configure and enable EDMA
    edma_config_t edma_config = {0};
    EDMA_GetDefaultConfig(&edma_config);
    EDMA_Init(DMA0, &edma_config);
}

void mimxrt_hal_bootloader() {
    #if FSL_ROM_HAS_RUNBOOTLOADER_API
    // Enter ROM bootloader, with primary image active.
    uint32_t arg = 0xEB000000;
    ROM_RunBootloader(&arg);
    #endif
}

int mimxrt_hal_csi_init(CSI_Type *inst) {
    CLOCK_EnableClock(kCLOCK_Csi);

    // Configure DCMI pins.
    omv_gpio_config(OMV_CSI_D0_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_D1_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_D2_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_D3_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_D4_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_D5_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_D6_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_D7_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);

    omv_gpio_config(OMV_CSI_MXCLK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_HSYNC_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_VSYNC_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_CSI_PXCLK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);

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

    // Configure IRQ priority.
    NVIC_SetPriority(CSI_IRQn, IRQ_PRI_CSI);

    return 0;
}

int mimxrt_hal_i2c_init(uint32_t bus_id) {
    omv_gpio_t scl_pin = NULL;
    omv_gpio_t sda_pin = NULL;

    switch (bus_id) {
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
        #if defined(OMV_I2C4_ID)
        case OMV_I2C4_ID: {
            scl_pin = OMV_I2C4_SCL_PIN;
            sda_pin = OMV_I2C4_SDA_PIN;
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

int mimxrt_hal_spi_init(uint32_t bus_id, bool nss_enable, uint32_t nss_pol) {
    typedef struct {
        omv_gpio_t sclk_pin;
        omv_gpio_t miso_pin;
        omv_gpio_t mosi_pin;
        omv_gpio_t ssel_pin;
    } spi_pins_t;

    spi_pins_t spi_pins = { NULL, NULL, NULL, NULL };

    switch (bus_id) {
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
        omv_gpio_config(spi_pins.ssel_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_MED, 5);
        if (nss_pol == OMV_SPI_NSS_LOW) {
            omv_gpio_write(spi_pins.ssel_pin, 1);
        } else {
            omv_gpio_write(spi_pins.ssel_pin, 0);
        }
    }
    return 0;
}

int mimxrt_hal_spi_deinit(uint32_t bus_id) {
    typedef struct {
        omv_gpio_t sclk_pin;
        omv_gpio_t miso_pin;
        omv_gpio_t mosi_pin;
        omv_gpio_t ssel_pin;
    } spi_pins_t;

    spi_pins_t spi_pins = { NULL, NULL, NULL, NULL };

    switch (bus_id) {
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

void CSI_IRQHandler(void) {
    uint32_t csisr = CSI_REG_SR(CSI);
    extern void sensor_sof_callback();
    extern void sensor_line_callback(uint32_t);

    // Clear interrupt flags.
    CSI_REG_SR(CSI) = csisr;

    if (csisr & CSI_SR_SOF_INT_MASK) {
        sensor_sof_callback();
    } else if (csisr & CSI_SR_DMA_TSF_DONE_FB1_MASK) {
        sensor_line_callback(CSI_REG_DMASA_FB1(CSI));
    } else if (csisr & CSI_SR_DMA_TSF_DONE_FB2_MASK) {
        sensor_line_callback(CSI_REG_DMASA_FB2(CSI));
    }

    // Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate
    // overlapping exception return operation might vector to incorrect interrupt
    #if defined __CORTEX_M && (__CORTEX_M >= 4U)
    __DSB();
    #endif
}
