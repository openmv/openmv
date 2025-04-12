# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# STHAL Makefile
ifeq ($(MCU_SERIES),f4)
HAL_SRC_C += $(addprefix stm32/f4/src/,\
    stm32f4xx_hal_adc.c \
    stm32f4xx_hal_adc_ex.c \
    stm32f4xx_hal_dac.c \
    stm32f4xx_hal_dac_ex.c \
    stm32f4xx_hal.c \
    stm32f4xx_hal_cortex.c \
    stm32f4xx_hal_dcmi.c \
    stm32f4xx_hal_dma.c \
    stm32f4xx_hal_dma_ex.c \
    stm32f4xx_hal_dma2d.c \
    stm32f4xx_hal_flash.c \
    stm32f4xx_hal_flash_ex.c \
    stm32f4xx_hal_gpio.c \
    stm32f4xx_hal_hcd.c \
    stm32f4xx_hal_i2c.c \
    stm32f4xx_hal_i2c_ex.c \
    stm32f4xx_hal_pcd.c \
    stm32f4xx_hal_pcd_ex.c \
    stm32f4xx_hal_pwr.c \
    stm32f4xx_hal_pwr_ex.c \
    stm32f4xx_hal_rcc.c \
    stm32f4xx_hal_rcc_ex.c \
    stm32f4xx_hal_rng.c \
    stm32f4xx_hal_rtc.c \
    stm32f4xx_hal_rtc_ex.c \
    stm32f4xx_hal_sd.c \
    stm32f4xx_hal_sdram.c \
    stm32f4xx_hal_spi.c \
    stm32f4xx_hal_can.c \
    stm32f4xx_hal_tim.c \
    stm32f4xx_hal_tim_ex.c \
    stm32f4xx_hal_uart.c \
    stm32f4xx_hal_usart.c \
    stm32f4xx_ll_usb.c \
    stm32f4xx_ll_sdmmc.c \
    stm32f4xx_ll_fmc.c \
)
endif

ifeq ($(MCU_SERIES),f7)
HAL_SRC_C += $(addprefix stm32/f7/src/,\
    stm32f7xx_hal_adc.c \
    stm32f7xx_hal_adc_ex.c \
    stm32f7xx_hal_dac.c \
    stm32f7xx_hal_dac_ex.c \
    stm32f7xx_hal.c \
    stm32f7xx_hal_cortex.c \
    stm32f7xx_hal_crc.c \
    stm32f7xx_hal_crc_ex.c \
    stm32f7xx_hal_dcmi.c \
    stm32f7xx_hal_dma.c \
    stm32f7xx_hal_dma_ex.c \
    stm32f7xx_hal_dma2d.c \
    stm32f7xx_hal_flash.c \
    stm32f7xx_hal_flash_ex.c \
    stm32f7xx_hal_gpio.c \
    stm32f7xx_hal_hcd.c \
    stm32f7xx_hal_i2c.c \
    stm32f7xx_hal_i2c_ex.c \
    stm32f7xx_hal_pcd.c \
    stm32f7xx_hal_pcd_ex.c \
    stm32f7xx_hal_pwr.c \
    stm32f7xx_hal_pwr_ex.c \
    stm32f7xx_hal_rcc.c \
    stm32f7xx_hal_rcc_ex.c \
    stm32f7xx_hal_rng.c \
    stm32f7xx_hal_rtc.c \
    stm32f7xx_hal_rtc_ex.c \
    stm32f7xx_hal_sd.c \
    stm32f7xx_hal_sdram.c \
    stm32f7xx_hal_spi.c \
    stm32f7xx_hal_can.c \
    stm32f7xx_hal_tim.c \
    stm32f7xx_hal_tim_ex.c \
    stm32f7xx_hal_uart.c \
    stm32f7xx_hal_usart.c \
    stm32f7xx_hal_jpeg.c \
    stm32f7xx_ll_usb.c \
    stm32f7xx_ll_sdmmc.c \
    stm32f7xx_ll_fmc.c \
)
endif

ifeq ($(MCU_SERIES),h7)
HAL_SRC_C += $(addprefix stm32/h7/src/,\
    stm32h7xx_hal_adc.c \
    stm32h7xx_hal_adc_ex.c \
    stm32h7xx_hal_dac.c \
    stm32h7xx_hal_dac_ex.c \
    stm32h7xx_hal_dsi.c \
    stm32h7xx_hal.c \
    stm32h7xx_hal_cortex.c \
    stm32h7xx_hal_crc.c \
    stm32h7xx_hal_crc_ex.c \
    stm32h7xx_hal_dcmi.c \
    stm32h7xx_hal_dfsdm.c \
    stm32h7xx_hal_dfsdm_ex.c \
    stm32h7xx_hal_dma.c \
    stm32h7xx_hal_dma_ex.c \
    stm32h7xx_hal_dma2d.c \
    stm32h7xx_hal_mdma.c \
    stm32h7xx_hal_flash.c \
    stm32h7xx_hal_flash_ex.c \
    stm32h7xx_hal_gpio.c \
    stm32h7xx_hal_hcd.c \
    stm32h7xx_hal_hsem.c \
    stm32h7xx_hal_i2c.c \
    stm32h7xx_hal_i2c_ex.c \
    stm32h7xx_hal_ltdc.c \
    stm32h7xx_hal_ltdc_ex.c \
    stm32h7xx_hal_pcd.c \
    stm32h7xx_hal_pcd_ex.c \
    stm32h7xx_hal_pwr.c \
    stm32h7xx_hal_pwr_ex.c \
    stm32h7xx_hal_rcc.c \
    stm32h7xx_hal_rcc_ex.c \
    stm32h7xx_hal_rng.c \
    stm32h7xx_hal_rtc.c \
    stm32h7xx_hal_rtc_ex.c \
    stm32h7xx_hal_sai.c \
    stm32h7xx_hal_sai_ex.c \
    stm32h7xx_hal_sd.c \
    stm32h7xx_hal_sdram.c \
    stm32h7xx_hal_spi.c \
    stm32h7xx_hal_fdcan.c \
    stm32h7xx_hal_tim.c \
    stm32h7xx_hal_tim_ex.c \
    stm32h7xx_hal_uart.c \
    stm32h7xx_hal_usart.c \
    stm32h7xx_hal_jpeg.c \
    stm32h7xx_hal_qspi.c \
    stm32h7xx_ll_rcc.c \
    stm32h7xx_ll_usb.c \
    stm32h7xx_ll_sdmmc.c \
    stm32h7xx_ll_fmc.c \
    stm32h7xx_ll_delayblock.c \
)
endif

ifeq ($(MCU_SERIES),n6)
HAL_SRC_C += $(addprefix stm32/n6/src/,\
    stm32n6xx_hal_adc.c \
    stm32n6xx_hal_adc_ex.c \
    stm32n6xx_hal_bsec.c \
    stm32n6xx_hal.c \
    stm32n6xx_hal_cacheaxi.c \
    stm32n6xx_hal_cortex.c \
    stm32n6xx_hal_crc.c \
    stm32n6xx_hal_crc_ex.c \
    stm32n6xx_hal_cryp.c \
    stm32n6xx_hal_cryp_ex.c \
    stm32n6xx_hal_dcmi.c \
    stm32n6xx_hal_dcmipp.c \
    stm32n6xx_hal_dma2d.c \
    stm32n6xx_hal_dma.c \
    stm32n6xx_hal_dma_ex.c \
    stm32n6xx_hal_dts.c \
    stm32n6xx_hal_eth.c \
    stm32n6xx_hal_eth_ex.c \
    stm32n6xx_hal_exti.c \
    stm32n6xx_hal_fdcan.c \
    stm32n6xx_hal_gfxmmu.c \
    stm32n6xx_hal_gfxtim.c \
    stm32n6xx_hal_gpio.c \
    stm32n6xx_hal_gpu2d.c \
    stm32n6xx_hal_hash.c \
    stm32n6xx_hal_hcd.c \
    stm32n6xx_hal_i2c.c \
    stm32n6xx_hal_i2c_ex.c \
    stm32n6xx_hal_i2s.c \
    stm32n6xx_hal_i2s_ex.c \
    stm32n6xx_hal_i3c.c \
    stm32n6xx_hal_icache.c \
    stm32n6xx_hal_irda.c \
    stm32n6xx_hal_iwdg.c \
    stm32n6xx_hal_jpeg.c \
    stm32n6xx_hal_lptim.c \
    stm32n6xx_hal_ltdc.c \
    stm32n6xx_hal_ltdc_ex.c \
    stm32n6xx_hal_mce.c \
    stm32n6xx_hal_mdf.c \
    stm32n6xx_hal_mdios.c \
    stm32n6xx_hal_mmc.c \
    stm32n6xx_hal_mmc_ex.c \
    stm32n6xx_hal_nand.c \
    stm32n6xx_hal_nor.c \
    stm32n6xx_hal_pcd.c \
    stm32n6xx_hal_pcd_ex.c \
    stm32n6xx_hal_pka.c \
    stm32n6xx_hal_pssi.c \
    stm32n6xx_hal_pwr.c \
    stm32n6xx_hal_pwr_ex.c \
    stm32n6xx_hal_ramcfg.c \
    stm32n6xx_hal_rcc.c \
    stm32n6xx_hal_rcc_ex.c \
    stm32n6xx_hal_rif.c \
    stm32n6xx_hal_rng.c \
    stm32n6xx_hal_rng_ex.c \
    stm32n6xx_hal_rtc.c \
    stm32n6xx_hal_rtc_ex.c \
    stm32n6xx_hal_sai.c \
    stm32n6xx_hal_sai_ex.c \
    stm32n6xx_hal_sd.c \
    stm32n6xx_hal_sd_ex.c \
    stm32n6xx_hal_sdram.c \
    stm32n6xx_hal_smartcard.c \
    stm32n6xx_hal_smartcard_ex.c \
    stm32n6xx_hal_smbus.c \
    stm32n6xx_hal_smbus_ex.c \
    stm32n6xx_hal_spdifrx.c \
    stm32n6xx_hal_spi.c \
    stm32n6xx_hal_spi_ex.c \
    stm32n6xx_hal_sram.c \
    stm32n6xx_hal_tim.c \
    stm32n6xx_hal_tim_ex.c \
    stm32n6xx_hal_uart.c \
    stm32n6xx_hal_uart_ex.c \
    stm32n6xx_hal_usart.c \
    stm32n6xx_hal_usart_ex.c \
    stm32n6xx_hal_wwdg.c \
    stm32n6xx_hal_xspi.c \
    stm32n6xx_ll_adc.c \
    stm32n6xx_ll_crc.c \
    stm32n6xx_ll_dma2d.c \
    stm32n6xx_ll_dma.c \
    stm32n6xx_ll_exti.c \
    stm32n6xx_ll_fmc.c \
    stm32n6xx_ll_gpio.c \
    stm32n6xx_ll_i2c.c \
    stm32n6xx_ll_i3c.c \
    stm32n6xx_ll_lptim.c \
    stm32n6xx_ll_lpuart.c \
    stm32n6xx_ll_pka.c \
    stm32n6xx_ll_pwr.c \
    stm32n6xx_ll_rcc.c \
    stm32n6xx_ll_rng.c \
    stm32n6xx_ll_rtc.c \
    stm32n6xx_ll_sdmmc.c \
    stm32n6xx_ll_spi.c \
    stm32n6xx_ll_tim.c \
    stm32n6xx_ll_ucpd.c \
    stm32n6xx_ll_usart.c \
    stm32n6xx_ll_usb.c \
    stm32n6xx_ll_utils.c \
    stm32n6xx_ll_venc.c \
    stm32n6xx_util_i3c.c \
)
endif

HAL_CFLAGS += -I$(TOP_DIR)/lib/stm32/$(MCU_SERIES)/include
HAL_CFLAGS += -I$(TOP_DIR)/lib/stm32/$(MCU_SERIES)/include/Legacy

OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/, $(HAL_SRC_C:.c=.o))
