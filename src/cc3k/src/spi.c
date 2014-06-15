/*****************************************************************************
 *
 *  spi.c - CC3000 Host Driver Implementation.
 *  Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#include "hci.h"
#include "spi.h"
#include "evnt_handler.h"
#include <stdlib.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_syscfg.h>

#define READ                3
#define WRITE               1
#define HI(value)           (((value) & 0xFF00) >> 8)
#define LO(value)           ((value) & 0x00FF)
#define HEADERS_SIZE_EVNT   (SPI_HEADER_SIZE + 5)

/* SPI bus states */
#define eSPI_STATE_POWERUP                  (0)
#define eSPI_STATE_INITIALIZED              (1)
#define eSPI_STATE_IDLE                     (2)
#define eSPI_STATE_WRITE_IRQ                (3)
#define eSPI_STATE_WRITE_FIRST_PORTION      (4)
#define eSPI_STATE_WRITE_EOT                (5)
#define eSPI_STATE_READ_IRQ                 (6)
#define eSPI_STATE_READ_FIRST_PORTION       (7)
#define eSPI_STATE_READ_EOT                 (8)

#define CC3K_SPI                SPI3
#define CC3K_SPI_CLK            RCC_APB1Periph_SPI3
#define CC3K_SPI_CLK_INIT       RCC_APB1PeriphClockCmd

#define CC3K_SPI_SCK_PIN        GPIO_Pin_10
#define CC3K_SPI_SCK_GPIO_PORT  GPIOC
#define CC3K_SPI_SCK_GPIO_CLK   RCC_AHB1Periph_GPIOC
#define CC3K_SPI_SCK_SOURCE     GPIO_PinSource10
#define CC3K_SPI_SCK_AF         GPIO_AF_SPI3

#define CC3K_SPI_MISO_PIN       GPIO_Pin_11
#define CC3K_SPI_MISO_GPIO_PORT GPIOC
#define CC3K_SPI_MISO_GPIO_CLK  RCC_AHB1Periph_GPIOC
#define CC3K_SPI_MISO_SOURCE    GPIO_PinSource11
#define CC3K_SPI_MISO_AF        GPIO_AF_SPI3

#define CC3K_SPI_MOSI_PIN       GPIO_Pin_12
#define CC3K_SPI_MOSI_GPIO_PORT GPIOC
#define CC3K_SPI_MOSI_GPIO_CLK  RCC_AHB1Periph_GPIOC
#define CC3K_SPI_MOSI_SOURCE    GPIO_PinSource12
#define CC3K_SPI_MOSI_AF        GPIO_AF_SPI3

#define CC3K_CS_PIN             GPIO_Pin_15
#define CC3K_CS_GPIO_PORT       GPIOA
#define CC3K_CS_GPIO_CLK        RCC_AHB1Periph_GPIOA

#define CC3K_EN_PIN             GPIO_Pin_10
#define CC3K_EN_GPIO_PORT       GPIOB
#define CC3K_EN_GPIO_CLK        RCC_AHB1Periph_GPIOB

#define CC3K_IRQ_PIN            GPIO_Pin_11
#define CC3K_IRQ_GPIO_PORT      GPIOB
#define CC3K_IRQ_GPIO_CLK       RCC_AHB1Periph_GPIOB

#define ASSERT_CS()             GPIO_ResetBits(CC3K_CS_GPIO_PORT, CC3K_CS_PIN);
#define DEASSERT_CS()           GPIO_SetBits(CC3K_CS_GPIO_PORT, CC3K_CS_PIN);

typedef struct {
    gcSpiHandleRx  SPIRxHandler;
    unsigned short usTxPacketLength;
    unsigned short usRxPacketLength;
    unsigned long  ulSpiState;
    unsigned char *pTxPacket;
    unsigned char *pRxPacket;
}tSpiInformation;
tSpiInformation sSpiInformation;

// The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
// for the purpose of detection of the overrun. The location of the memory where the magic number
// resides shall never be written. In case it is written - the overrun occured and either recevie function
// or send function will stuck forever.
#define CC3000_BUFFER_MAGIC_NUMBER (0xDE)

char spi_buffer[CC3000_RX_BUFFER_SIZE];
unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];

void SpiWriteDataSynchronous(unsigned char *data, unsigned short size);
void SpiReadDataSynchronous(unsigned char *data, unsigned short size);

void SpiOpen(gcSpiHandleRx pfRxHandler)
{
    /* init structs */
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /* initialize SPI state */
    sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;
    sSpiInformation.SPIRxHandler = pfRxHandler;
    sSpiInformation.usTxPacketLength = 0;
    sSpiInformation.pTxPacket = NULL;
    sSpiInformation.pRxPacket = (unsigned char *)spi_buffer;
    sSpiInformation.usRxPacketLength = 0;
    spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
    wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;

    /* Enable SPI clock */
    CC3K_SPI_CLK_INIT(CC3K_SPI_CLK, ENABLE);

    /* Enable GPIO clocks */
    RCC_AHB1PeriphClockCmd(CC3K_SPI_SCK_GPIO_CLK | CC3K_SPI_MISO_GPIO_CLK |
            CC3K_SPI_MOSI_GPIO_CLK | CC3K_CS_GPIO_CLK | CC3K_EN_GPIO_CLK, ENABLE);

    /* Connect SPI pins to AF */
    GPIO_PinAFConfig(CC3K_SPI_SCK_GPIO_PORT, CC3K_SPI_SCK_SOURCE, CC3K_SPI_SCK_AF);
    GPIO_PinAFConfig(CC3K_SPI_MISO_GPIO_PORT, CC3K_SPI_MISO_SOURCE, CC3K_SPI_MISO_AF);
    GPIO_PinAFConfig(CC3K_SPI_MOSI_GPIO_PORT, CC3K_SPI_MOSI_SOURCE, CC3K_SPI_MOSI_AF);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    /* SPI SCK pin configuration */
    GPIO_InitStructure.GPIO_Pin = CC3K_SPI_SCK_PIN;
    GPIO_Init(CC3K_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    /* SPI MOSI pin configuration */
    GPIO_InitStructure.GPIO_Pin =  CC3K_SPI_MOSI_PIN;
    GPIO_Init(CC3K_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /* SPI MISO pin configuration */
    GPIO_InitStructure.GPIO_Pin =  CC3K_SPI_MISO_PIN;
    GPIO_Init(CC3K_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

    /* CS pin configuration */
    GPIO_InitStructure.GPIO_Pin  = CC3K_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(CC3K_CS_GPIO_PORT, &GPIO_InitStructure);

    /* Configure WLAN enable pin */
    GPIO_InitStructure.GPIO_Pin  = CC3K_EN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(CC3K_EN_GPIO_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(CC3K_EN_GPIO_PORT, CC3K_EN_PIN);

    /* Deselect the CC3K CS */
    DEASSERT_CS();

    /* SPI configuration */
    SPI_I2S_DeInit(CC3K_SPI);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_CPOL  = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA  = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS   = SPI_NSS_Soft;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(CC3K_SPI, &SPI_InitStructure);

    /* Enable the CC3K_SPI  */
    SPI_Cmd(CC3K_SPI, ENABLE);

    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Configure IRQ pin */
    GPIO_InitStructure.GPIO_Pin  = CC3K_IRQ_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(CC3K_IRQ_GPIO_PORT, &GPIO_InitStructure);

    /* Connect EXTI */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource11);

    /* Configure EXTI Line0 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void SpiClose(void)
{
    if (sSpiInformation.pRxPacket) {
        sSpiInformation.pRxPacket = 0;
    }

    tSLInformation.WlanInterruptDisable();
}

void SpiPauseSpi(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void SpiResumeSpi(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

long TXBufferIsEmpty(void)
{
    return (SPI_I2S_GetFlagStatus(CC3K_SPI, SPI_I2S_FLAG_TXE) == SET);
}

long RXBufferIsEmpty(void)
{
    return (SPI_I2S_GetFlagStatus(CC3K_SPI, SPI_I2S_FLAG_RXNE) == RESET);
}

long ReadWlanInterruptPin(void)
{
    return GPIO_ReadInputDataBit(CC3K_IRQ_GPIO_PORT, CC3K_IRQ_PIN);
}

void WriteWlanPin(unsigned char val)
{
    if (val == WLAN_ENABLE) {
        /* Set WLAN EN high */
        GPIO_SetBits(CC3K_EN_GPIO_PORT, CC3K_EN_PIN);
    } else {
        /* Set WLAN EN LOW */
        GPIO_ResetBits(CC3K_EN_GPIO_PORT, CC3K_EN_PIN);
    }
}

void __delay_cycles(volatile int x)
{
    while (x--);
}

long SpiFirstWrite(unsigned char *ucBuf, unsigned short usLength)
{
    ASSERT_CS();

    // Assuming we are running on 24 MHz ~50 micro delay is 1200 cycles;
    __delay_cycles(1200);

    // SPI writes first 4 bytes of data
    SpiWriteDataSynchronous(ucBuf, 4);

    __delay_cycles(1200);

    SpiWriteDataSynchronous(ucBuf + 4, usLength - 4);

    // From this point on - operate in a regular way
    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

    DEASSERT_CS();

    return(0);
}

long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength)
{
    unsigned char ucPad = !(usLength & 0x0001);

    pUserBuffer[0] = WRITE;
    pUserBuffer[1] = HI(usLength + ucPad);
    pUserBuffer[2] = LO(usLength + ucPad);
    pUserBuffer[3] = 0;
    pUserBuffer[4] = 0;

    usLength += (SPI_HEADER_SIZE + ucPad);

    // The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
    // for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun
    // occurred - and we will stuck here forever!
    if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER) {
        while (1);
    }

    if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP) {
        while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED);
    }

    if (sSpiInformation.ulSpiState == eSPI_STATE_INITIALIZED) {
        // This is time for first TX/RX transactions over SPI:
        // the IRQ is down - so need to send read buffer size command
        SpiFirstWrite(pUserBuffer, usLength);
    } else {
        //
        // We need to prevent here race that can occur in case 2 back to back packets are sent to the
        // device, so the state will move to IDLE and once again to not IDLE due to IRQ
        //
        tSLInformation.WlanInterruptDisable();

        while (sSpiInformation.ulSpiState != eSPI_STATE_IDLE);

        sSpiInformation.ulSpiState = eSPI_STATE_WRITE_IRQ;
        sSpiInformation.pTxPacket = pUserBuffer;
        sSpiInformation.usTxPacketLength = usLength;

        // Assert the CS line and wait till SSI IRQ line is active and then initialize write operation
        ASSERT_CS();

        // Re-enable IRQ - if it was not disabled - this is not a problem...
        tSLInformation.WlanInterruptEnable();

        // check for a missing interrupt between the CS assertion and enabling back the interrupts
        if (tSLInformation.ReadWlanInterruptPin() == 0) {
            SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);

            sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

            DEASSERT_CS();
        }
    }

    // Due to the fact that we are currently implementing a blocking situation
    // here we will wait till end of transaction
    while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState);

    return(0);
}

void SpiWriteDataSynchronous(unsigned char *data, unsigned short size)
{
    while (size) {
        while (!TXBufferIsEmpty());
        /* Send byte through the SPI peripheral */
        SPI_I2S_SendData(CC3K_SPI, *data);

        /* wait to receive the byte from slave */
        while (RXBufferIsEmpty());

        /* Read byte from the SPI peripheral */
        SPI_I2S_ReceiveData(CC3K_SPI);
        size --;
        data++;
    }
}

void SpiReadDataSynchronous(unsigned char *data, unsigned short size)
{
    int i = 0;
    unsigned char data_to_send = READ;
    for (i = 0; i<size; i++) {
        while (!TXBufferIsEmpty());
        /* Send byte through the SPI peripheral */
        SPI_I2S_SendData(CC3K_SPI, data_to_send);
        /* wait to receive the byte from slave */
        while (RXBufferIsEmpty());
        /* Read byte from the SPI peripheral */
        data[i] = SPI_I2S_ReceiveData(CC3K_SPI);
    }
}

void SpiReadPacket(void)
{
    int length;

    /* read SPI header */
    SpiReadDataSynchronous(sSpiInformation.pRxPacket, SPI_HEADER_SIZE);

    /* parse data length  */
    STREAM_TO_UINT8(sSpiInformation.pRxPacket, SPI_HEADER_SIZE-1, length);

    /* read the remainder of the packet */
    SpiReadDataSynchronous(sSpiInformation.pRxPacket + SPI_HEADER_SIZE, length);

    sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
}

void SpiReadHeader(void)
{
    SpiReadDataSynchronous(sSpiInformation.pRxPacket, 10);
}

void SpiTriggerRxProcessing(void)
{
    SpiPauseSpi();
    DEASSERT_CS();

    // The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
    // for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun
    // occurred - and we will stuck here forever!
    if (sSpiInformation.pRxPacket[CC3000_RX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER) {
        while (1);
    }

    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
    sSpiInformation.SPIRxHandler(sSpiInformation.pRxPacket + SPI_HEADER_SIZE);
}


long SpiReadDataCont(void)
{
    long data_to_recv=0;
    unsigned char *evnt_buff, type;

    //determine what type of packet we have
    evnt_buff =  sSpiInformation.pRxPacket;
    STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_PACKET_TYPE_OFFSET, type);

    switch (type) {
        case HCI_TYPE_DATA:{
                // We need to read the rest of data..
                STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE),
                        HCI_DATA_LENGTH_OFFSET, data_to_recv);
                if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1)) {
                    data_to_recv++;
                }

                if (data_to_recv) {
                    SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
                }
                break;
            }
        case HCI_TYPE_EVNT: {
                // Calculate the rest length of the data
                STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE),
                        HCI_EVENT_LENGTH_OFFSET, data_to_recv);
                data_to_recv -= 1;

                // Add padding byte if needed
                if ((HEADERS_SIZE_EVNT + data_to_recv) & 1) {
                    data_to_recv++;
                }

                if (data_to_recv) {
                    SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
                }

                sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
                break;
            }
    }

    return 0;
}

void SSIContReadOperation(void)
{
    // The header was read - continue with  the payload read
    if (!SpiReadDataCont()) {
        /* All the data was read - finalize handling by switching
           to the task and calling from task Event Handler */
        SpiTriggerRxProcessing();
    }
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
        switch (sSpiInformation.ulSpiState) {
            case eSPI_STATE_POWERUP:
                /* This means IRQ line was low call a callback of HCI Layer to inform on event */
                sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
                break;
            case eSPI_STATE_IDLE:
                sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;

                /* IRQ line goes down - we are start reception */
                ASSERT_CS();

                // Wait for TX/RX Compete which will come as DMA interrupt
                SpiReadHeader();

                sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;

                SSIContReadOperation();
                break;
            case eSPI_STATE_WRITE_IRQ:
                SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);

                sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

                DEASSERT_CS();
                break;
        }

        /* Clear the EXTI line 0 pending bit */
        EXTI_ClearITPendingBit(EXTI_Line11);
    }
}
