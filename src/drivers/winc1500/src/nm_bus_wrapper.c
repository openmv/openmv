/**
 *
 * \file
 *
 * \brief This module contains NMC1000 bus wrapper APIs implementation.
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include <stdio.h>
#include <string.h>
#include STM32_HAL_H
#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"
#include "bus_wrapper/include/nm_bus_wrapper.h"
#include "conf_winc.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "extint.h"
#include "omv_boardconfig.h"
#include "omv_gpio.h"

static SPI_HandleTypeDef SPI_HANDLE;

#define NM_BUS_MAX_TRX_SZ 4096

tstrNmBusCapabilities egstrNmBusCapabilities =
{
	NM_BUS_MAX_TRX_SZ
};

#ifdef CONF_WINC_USE_I2C
#define SLAVE_ADDRESS 0x60

/** Number of times to try to send packet if failed. */
#define I2C_TIMEOUT 100

static sint8 nm_i2c_write(uint8 *b, uint16 sz)
{
	sint8 result = M2M_ERR_BUS_FAIL;
	return result;
}

static sint8 nm_i2c_read(uint8 *rb, uint16 sz)
{
	sint8 result = M2M_ERR_BUS_FAIL;
	return result;
}

static sint8 nm_i2c_write_special(uint8 *wb1, uint16 sz1, uint8 *wb2, uint16 sz2)
{
	static uint8 tmp[NM_BUS_MAX_TRX_SZ];
	m2m_memcpy(tmp, wb1, sz1);
	m2m_memcpy(&tmp[sz1], wb2, sz2);
	return nm_i2c_write(tmp, sz1+sz2);
}
#endif

#ifdef CONF_WINC_USE_SPI
static sint8 spi_rw(uint8 *tx_buf, uint8 *rx_buf, uint16 u16Sz)
{
    sint8 result = M2M_SUCCESS;
    omv_gpio_write(WINC_SPI_SSEL_PIN, 0);
    if (tx_buf != 0) {
        if (HAL_SPI_Transmit(&SPI_HANDLE, tx_buf, u16Sz, WINC_SPI_TIMEOUT) != HAL_OK) {
            result = M2M_ERR_BUS_FAIL;
        }
    } else {
        memset(rx_buf, 0, u16Sz);
        if (HAL_SPI_TransmitReceive(&SPI_HANDLE, rx_buf, rx_buf, u16Sz, WINC_SPI_TIMEOUT) != HAL_OK) {
            result = M2M_ERR_BUS_FAIL;
        }
    }
    omv_gpio_write(WINC_SPI_SSEL_PIN, 1);
    return result;
}
#endif

/*
*	@fn		nm_bus_init
*	@brief	Initialize the bus wrapper
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_bus_init(void *pvinit)
{
	sint8 result = M2M_SUCCESS;

    omv_gpio_write(WINC_SPI_SSEL_PIN, 1);

    // SPI configuration
    SPI_HANDLE.Instance               = WINC_SPI;
    SPI_HANDLE.Init.Mode              = SPI_MODE_MASTER;
    SPI_HANDLE.Init.Direction         = SPI_DIRECTION_2LINES;
    SPI_HANDLE.Init.DataSize          = SPI_DATASIZE_8BIT;
    SPI_HANDLE.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SPI_HANDLE.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SPI_HANDLE.Init.NSS               = SPI_NSS_SOFT;
    SPI_HANDLE.Init.BaudRatePrescaler = WINC_SPI_PRESCALER;
    SPI_HANDLE.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SPI_HANDLE.Init.TIMode            = SPI_TIMODE_DISABLED;
    SPI_HANDLE.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SPI_HANDLE.Init.CRCPolynomial     = 7;

    // Enable SPI clock
    WINC_SPI_CLK_ENABLE();

    // Init SPI
    HAL_SPI_DeInit(&SPI_HANDLE);
    if (HAL_SPI_Init(&SPI_HANDLE) != HAL_OK) {
        result = M2M_ERR_BUS_FAIL;
    }

    nm_bsp_reset();
    omv_gpio_write(WINC_SPI_SSEL_PIN, 1);

	return result;
}

/*
*	@fn		nm_bus_ioctl
*	@brief	send/receive from the bus
*	@param[IN]	u8Cmd
*					IOCTL command for the operation
*	@param[IN]	pvParameter
*					Arbitrary parameter depenging on IOCTL
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@note	For SPI only, it's important to be able to send/receive at the same time
*/
sint8 nm_bus_ioctl(uint8 u8Cmd, void* pvParameter)
{
	sint8 s8Ret = 0;
	switch(u8Cmd)
	{
#ifdef CONF_WINC_USE_I2C
		case NM_BUS_IOCTL_R: {
			tstrNmI2cDefault *pstrParam = (tstrNmI2cDefault *)pvParameter;
			s8Ret = nm_i2c_read(pstrParam->pu8Buf, pstrParam->u16Sz);
		}
		break;
		case NM_BUS_IOCTL_W: {
			tstrNmI2cDefault *pstrParam = (tstrNmI2cDefault *)pvParameter;
			s8Ret = nm_i2c_write(pstrParam->pu8Buf, pstrParam->u16Sz);
		}
		break;
		case NM_BUS_IOCTL_W_SPECIAL: {
			tstrNmI2cSpecial *pstrParam = (tstrNmI2cSpecial *)pvParameter;
			s8Ret = nm_i2c_write_special(pstrParam->pu8Buf1, pstrParam->u16Sz1, pstrParam->pu8Buf2, pstrParam->u16Sz2);
		}
		break;
#elif defined CONF_WINC_USE_SPI
		case NM_BUS_IOCTL_RW: {
			tstrNmSpiRw *pstrParam = (tstrNmSpiRw *)pvParameter;
			s8Ret = spi_rw(pstrParam->pu8InBuf, pstrParam->pu8OutBuf, pstrParam->u16Sz);
		}
		break;
#endif
		default:
			s8Ret = -1;
			M2M_ERR("invalide ioclt cmd\n");
			break;
	}

	return s8Ret;
}

/*
*	@fn		nm_bus_deinit
*	@brief	De-initialize the bus wrapper
*/
sint8 nm_bus_deinit(void)
{
	return M2M_SUCCESS;
}
