/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2021 Lake Fu at <lake_fu@pixart.com>
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Pixart SPI driver.
 */
#include "omv_boardconfig.h"
#ifdef ISC_SPI

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include STM32_HAL_H

#include "common.h"
#include "omv_gpio.h"

#include "pixspi.h"

#define SPI_TIMEOUT     (5000)  // in ms

extern SPI_HandleTypeDef ISC_SPIHandle;

static bool spi_send(uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status = HAL_OK;
    omv_gpio_write(ISC_SPI_SSEL_PIN, 0);
    // mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    status = HAL_SPI_Transmit(
        &ISC_SPIHandle, data, len, SPI_TIMEOUT);
    // MICROPY_END_ATOMIC_SECTION(atomic_state);
    omv_gpio_write(ISC_SPI_SSEL_PIN, 1);

    return (status == HAL_OK);
}

static bool spi_send_recv(uint8_t *txData, uint8_t *rxData, uint16_t len)
{
    omv_gpio_write(ISC_SPI_SSEL_PIN, 0);
    // mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    bool res = (HAL_SPI_Transmit(&ISC_SPIHandle, txData, 1, SPI_TIMEOUT) == HAL_OK);
    res = (HAL_SPI_Receive(&ISC_SPIHandle, rxData, len, SPI_TIMEOUT) == HAL_OK);
    // MICROPY_END_ATOMIC_SECTION(atomic_state);
    omv_gpio_write(ISC_SPI_SSEL_PIN, 1);
    return res;
}

bool pixspi_init()
{
    // Init SPI
    memset(&ISC_SPIHandle, 0, sizeof(ISC_SPIHandle));
    ISC_SPIHandle.Instance               = ISC_SPI;
    ISC_SPIHandle.Init.Mode              = SPI_MODE_MASTER;
    ISC_SPIHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    ISC_SPIHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    ISC_SPIHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    ISC_SPIHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
    ISC_SPIHandle.Init.NSS               = SPI_NSS_SOFT;
    ISC_SPIHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32 ;
    ISC_SPIHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    ISC_SPIHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    ISC_SPIHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    ISC_SPIHandle.Init.CRCPolynomial     = 0;

    if (HAL_SPI_Init(&ISC_SPIHandle) != HAL_OK) {
        // Initialization Error
        return false;
    }

    // Re-Init GPIO for SPI SS(CS) pin, software control.
    omv_gpio_config(ISC_SPI_SSEL_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);

    // Default high.
    omv_gpio_write(ISC_SPI_SSEL_PIN, 1);
    return true;
}

void pixspi_release()
{
    omv_gpio_write(ISC_SPI_SSEL_PIN, 0);
    HAL_SPI_DeInit(&ISC_SPIHandle);
}

int pixspi_regs_read(uint8_t addr, uint8_t * data, uint16_t length)
{
    if ((addr & 0x80)) {
        #ifdef DEBUG
        printf("pixspi_regs_read() address (0x%x) overflow.\n", addr);
        #endif
        return -1;
    }
    addr |= 0x80;
    if (!spi_send_recv(&addr, data, length)) {
        #ifdef DEBUG
        printf("spi_send_recv() failed.\n");
        #endif
        return -1;
    }
    return 0;
}

int pixspi_regs_write(uint8_t addr, const uint8_t * data, uint16_t length)
{
    uint8_t buff[64] = {};
    if ((addr & 0x80) == 0x80) {
        #ifdef DEBUG
        printf("pixspi_regs_read() address (0x%x) overflow.\n", addr);
        #endif
        return -1;
    }
    int32_t remaining = length;

    const static uint16_t MAX_LENGTH = 255;
    do {
        uint16_t len = remaining>MAX_LENGTH?MAX_LENGTH:remaining;
        buff[0] = addr;
        memcpy(buff+1, data, len);
        bool res = spi_send(buff, len + 1);
        if (!res) {
            #ifdef DEBUG
            printf("spi_send() failed.\n");
            #endif
            return -1;
        }
        remaining = remaining - MAX_LENGTH;
    } while (remaining > 0);

    return 0;
}
#endif
