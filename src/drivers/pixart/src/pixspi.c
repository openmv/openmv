#include "pixspi.h"
#include <stdbool.h>
#include STM32_HAL_H
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "omv_boardconfig.h"
#include <py/mpconfig.h>

#ifdef ISC_SPI
#define SPI_TIMEOUT         (5000)  /* in ms */

#define CS_PORT             ISC_SPI_SSEL_PORT
#define CS_PIN              ISC_SPI_SSEL_PIN
#define W_CS_LOW()    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET)
#define W_CS_HIGH()   HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET)

extern SPI_HandleTypeDef ISC_SPIHandle;

static bool spi_send(uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status = HAL_OK;
    W_CS_LOW();
    //mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    status = HAL_SPI_Transmit(
        &ISC_SPIHandle, data, len, SPI_TIMEOUT);
    //MICROPY_END_ATOMIC_SECTION(atomic_state);
    W_CS_HIGH();

    return (status == HAL_OK);
}

static bool spi_send_recv(uint8_t *txData, uint8_t *rxData, uint16_t len)
{
    W_CS_LOW();
    //mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    bool res = (HAL_SPI_Transmit(&ISC_SPIHandle, txData, 1, SPI_TIMEOUT) == HAL_OK);
    res = (HAL_SPI_Receive(&ISC_SPIHandle, rxData, len, SPI_TIMEOUT) == HAL_OK);
    //MICROPY_END_ATOMIC_SECTION(atomic_state);
    W_CS_HIGH();
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

    if (HAL_SPI_Init(&ISC_SPIHandle) != HAL_OK)
    {
        /* Initialization Error */
        return false;
    }

    GPIO_InitTypeDef GPIO_InitStructure;
    // Re-Init GPIO for SPI SS(CS) pin, software control.
    GPIO_InitStructure.Pin = ISC_SPI_SSEL_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Alternate = ISC_SPI_SSEL_AF;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ISC_SPI_SSEL_PORT, &GPIO_InitStructure);

    // Default high.
    W_CS_HIGH();
    return true;
}

void pixspi_release()
{
    W_CS_LOW();
    HAL_SPI_DeInit(&ISC_SPIHandle);
}

int pixspi_regs_read(uint8_t addr, uint8_t * data, uint16_t length)
{
    if ((addr & 0x80))
    {
#ifdef DEBUG
        printf("pixspi_regs_read() address (0x%x) overflow.\n", addr);
#endif
        return -1;
    }
    addr |= 0x80;
    if (!spi_send_recv(&addr, data, length))
    {
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
    if ((addr & 0x80) == 0x80)
    {
#ifdef DEBUG
        printf("pixspi_regs_read() address (0x%x) overflow.\n", addr);
#endif
        return -1;
    }
    int32_t remaining = length;

    const static uint16_t MAX_LENGTH = 255;
    do
    {
        uint16_t len = remaining>MAX_LENGTH?MAX_LENGTH:remaining;
        buff[0] = addr;
        memcpy(buff+1, data, len);
        bool res = spi_send(buff, len + 1);
        if (!res)
        {
#ifdef DEBUG
            printf("spi_send() failed.\n");
#endif
            return -1;
        }
        remaining = remaining - MAX_LENGTH;
    }
    while (remaining > 0);

    return 0;
}
#else
bool pixspi_init() { return false; }
void pixspi_release() {}
int pixspi_regs_read(uint8_t addr, uint8_t * data, uint16_t length) { return -1; }
int pixspi_regs_write(uint8_t addr, const uint8_t * data, uint16_t length) { return -1; }
#endif