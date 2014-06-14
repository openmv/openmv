#include <stdint.h>
#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_gpio.h>
#include "spi.h"

#define GPIO_SPI_AF         GPIO_AF_SPI3

#define GPIO_PIN_CS         GPIO_Pin_15
#define GPIO_PIN_SCLK       GPIO_Pin_10
#define GPIO_PIN_MISO       GPIO_Pin_11
#define GPIO_PIN_MOSI       GPIO_Pin_12

#define GPIO_SOURCE_SCLK    GPIO_PinSource10
#define GPIO_SOURCE_MISO    GPIO_PinSource11
#define GPIO_SOURCE_MOSI    GPIO_PinSource12

#define GPIO_PORT_CS        GPIOA
#define GPIO_PORT_SCLK      GPIOC
#define GPIO_PORT_MISO      GPIOC
#define GPIO_PORT_MOSI      GPIOC

#define GPIO_CLK_CS         RCC_AHB1Periph_GPIOA
#define GPIO_CLK_SCLK       RCC_AHB1Periph_GPIOC
#define GPIO_CLK_MISO       RCC_AHB1Periph_GPIOC
#define GPIO_CLK_MOSI       RCC_AHB1Periph_GPIOC

#define SPIx                SPI3
#define SPIx_CLK            RCC_APB1Periph_SPI3
#define SPIx_CLK_CMD        RCC_APB1PeriphClockCmd
#define GPIO_CLK_CMD        RCC_AHB1PeriphClockCmd

void spi_init()
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable SPI clock */
    SPIx_CLK_CMD(SPIx_CLK, ENABLE);

    /* Enable GPIO clocks */
    GPIO_CLK_CMD(GPIO_CLK_CS | GPIO_CLK_SCLK | GPIO_CLK_MISO | GPIO_CLK_MOSI, ENABLE);

    /* Connect SPI pins to AF */
    GPIO_PinAFConfig(GPIO_PORT_SCLK, GPIO_SOURCE_SCLK, GPIO_SPI_AF);
    GPIO_PinAFConfig(GPIO_PORT_MISO, GPIO_SOURCE_MISO, GPIO_SPI_AF);
    GPIO_PinAFConfig(GPIO_PORT_MOSI, GPIO_SOURCE_MOSI, GPIO_SPI_AF);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* SPI  MISO pin configuration */
    GPIO_InitStructure.GPIO_Pin =  GPIO_PIN_MISO;
    GPIO_Init(GPIO_PORT_MISO, &GPIO_InitStructure);

    /* SPI  MOSI pin configuration */
    GPIO_InitStructure.GPIO_Pin =  GPIO_PIN_MOSI;
    GPIO_Init(GPIO_PORT_MOSI, &GPIO_InitStructure);

    /* SPI SCK pin configuration */
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SCLK;
    GPIO_Init(GPIO_PORT_SCLK, &GPIO_InitStructure);

    /* SPI configuration */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SPIx, &SPI_InitStructure);
    SPI_CalculateCRC(SPIx, DISABLE);
    SPI_Cmd(SPIx, ENABLE);

    /* drain SPI */
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);

    /* dummy read */
    SPI_I2S_ReceiveData(SPIx);
}

uint8_t spi_read()
{
    /* Send byte through the SPI */
    SPI_I2S_SendData(SPIx, 0xFF);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);

    /* Read byte from the SPI */
    return SPI_I2S_ReceiveData(SPIx);
}

uint8_t spi_write(uint8_t b)
{
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);

    /* Send byte through the SPI */
    SPI_I2S_SendData(SPIx, b);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);

    /* Read byte from the SPI */
    return SPI_I2S_ReceiveData(SPIx);
}
