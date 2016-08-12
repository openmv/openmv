/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SD card SDIO driver.
 *
 */
#include <stdbool.h>
#include STM32_HAL_H
#include <core_cmInstr.h>

#include "mp.h"
#include "irq.h"
#include "ffconf.h"
#include "diskio.h"
#include "systick.h"
#include "sdcard.h"
#include "omv_boardconfig.h"

#define SDIO_TIMEOUT            (0xFFFFFFFF) // Delay counter
#define SDIO_TXRX_STREAM        DMA2_Stream3
#define SDIO_TXRX_CHANNEL       DMA_CHANNEL_4

#define UNALIGNED_BUFFER(p)     ((uint32_t)p & 3)
#define CCM_BUFFER(p)           (!(((uint32_t) p) & (1u<<29)))

SD_HandleTypeDef  SDHandle;
static DMA_HandleTypeDef DMAHandle;

static void dma_init(DMA_Stream_TypeDef *dma_stream, uint32_t dma_channel, uint32_t direction)
{
    DMAHandle.Instance                 = dma_stream;
    DMAHandle.Init.Channel             = dma_channel;
    DMAHandle.Init.Direction           = direction;
    DMAHandle.Init.MemInc              = DMA_MINC_ENABLE;
    DMAHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
    DMAHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    DMAHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    DMAHandle.Init.Mode                = DMA_PFCTRL;
    DMAHandle.Init.Priority            = DMA_PRIORITY_LOW;
    DMAHandle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    DMAHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    DMAHandle.Init.MemBurst            = DMA_MBURST_INC4;
    DMAHandle.Init.PeriphBurst         = DMA_PBURST_INC4;

    // Associate the DMA handle with the SDIO handle
    // Note: The DMA handle is linked to both SDIO TX/RX handles
    // since we're using a single DMA channel/IRQHandler for TX/RX.
    __HAL_LINKDMA(&SDHandle, hdmatx, DMAHandle);
    __HAL_LINKDMA(&SDHandle, hdmarx, DMAHandle);

    HAL_DMA_DeInit(&DMAHandle);
    if (HAL_DMA_Init(&DMAHandle) != HAL_OK) {
        // Initialization Error
        __BKPT(0);
    }

    // Configure and enable SD IRQ Channel
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, IRQ_PRI_DMA, IRQ_SUBPRI_DMA);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}

static void dma_deinit()
{
    // Just disable the DMA IRQ
    HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
}

bool sdcard_is_present(void)
{
    return (HAL_GPIO_ReadPin(SD_CD_PORT, SD_CD_PIN)==GPIO_PIN_RESET);
}

extern void __fatal_error(const char *msg);

void sdcard_init(void)
{
    volatile int retry=10;
    HAL_SD_CardInfoTypedef cardinfo;

    // SDIO configuration
    SDHandle.Instance                 = SDIO;
    SDHandle.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
    SDHandle.Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
    SDHandle.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
    SDHandle.Init.BusWide             = SDIO_BUS_WIDE_1B;
    SDHandle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    SDHandle.Init.ClockDiv            = SDIO_TRANSFER_CLK_DIV; //INIT_CLK_DIV will be used first

    // Deinit SD
    HAL_SD_DeInit(&SDHandle);
    // Init SD interface
    while(HAL_SD_Init(&SDHandle, &cardinfo) != SD_OK && retry--) {
        if (retry == 0) {
            __fatal_error("Failed to init sdcard: init timeout");
        }
        systick_sleep(100);
    }

    /* Configure the SD Card in wide bus mode. */
    if (HAL_SD_WideBusOperation_Config(&SDHandle, SDIO_BUS_WIDE_4B) != SD_OK) {
        __fatal_error("Failed to init sensor, sdcard: config wide bus");
    }

    // Configure and enable DMA IRQ Channel
    // SDIO IRQ should have a higher priority than DMA IRQ because it needs to
    // preempt the DMA irq handler to set a flag indicating the end of transfer.
    HAL_NVIC_SetPriority(SDIO_IRQn, IRQ_PRI_SDIO, IRQ_SUBPRI_SDIO);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);
}

bool sdcard_power_on(void)
{
    if (!sdcard_is_present()) {
        return false;
    }

    return true;
}

void sdcard_power_off(void)
{

}

mp_uint_t sdcard_read_blocks(uint8_t *buff, uint32_t sector, uint32_t count)
{
    HAL_SD_ErrorTypedef err;

    // If buffer is unaligned or located in CCM don't use DMA.
    if (CCM_BUFFER(buff) || UNALIGNED_BUFFER(buff)) {
        if (UNALIGNED_BUFFER(buff)) {
            printf("unaligned read buf:%p  count%lu \n", buff, count);
        }
        // This transfer has to be done in an atomic section. 
        mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
        err = HAL_SD_ReadBlocks(&SDHandle, (uint32_t*)buff,
                sector * SDCARD_BLOCK_SIZE, SDCARD_BLOCK_SIZE, count);
        MICROPY_END_ATOMIC_SECTION(atomic_state);
    } else {
        // Disable USB IRQ to prevent FatFS/MSC contention
        HAL_NVIC_DisableIRQ(OTG_FS_IRQn); __DSB(); __ISB();

        dma_init(SDIO_TXRX_STREAM, SDIO_TXRX_CHANNEL, DMA_PERIPH_TO_MEMORY);

        err = HAL_SD_ReadBlocks_DMA(&SDHandle, (uint32_t*)buff,
                sector * SDCARD_BLOCK_SIZE, SDCARD_BLOCK_SIZE, count);

        if (err == SD_OK) {
            err = HAL_SD_CheckReadOperation(&SDHandle, SDIO_TIMEOUT);
        }

        if (err != SD_OK) {
            printf("read buf:%p  addr:%lu count%lu error:%d\n", buff, sector, count, err);
        }
        dma_deinit();
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    }
    return (err != SD_OK);
}

mp_uint_t sdcard_write_blocks(const uint8_t *buff, uint32_t sector, uint32_t count)
{
    HAL_SD_ErrorTypedef err;

    // If buffer is unaligned or located in CCM don't use DMA.
    if (CCM_BUFFER(buff) || UNALIGNED_BUFFER(buff)) {
        if (UNALIGNED_BUFFER(buff)) {
            printf("unaligned write buf:%p  count%lu \n", buff, count);
        }

        // This transfer has to be done in an atomic section. 
        mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
        err = HAL_SD_WriteBlocks(&SDHandle, (uint32_t*)buff,
                sector * SDCARD_BLOCK_SIZE, SDCARD_BLOCK_SIZE, count);
        MICROPY_END_ATOMIC_SECTION(atomic_state);
    } else {
        // Disable USB IRQ to prevent FatFS/MSC contention
        HAL_NVIC_DisableIRQ(OTG_FS_IRQn); __DSB(); __ISB();

        dma_init(SDIO_TXRX_STREAM, SDIO_TXRX_CHANNEL, DMA_MEMORY_TO_PERIPH);

        err = HAL_SD_WriteBlocks_DMA(&SDHandle, (uint32_t*)buff,
                sector * SDCARD_BLOCK_SIZE, SDCARD_BLOCK_SIZE, count);

        if (err == SD_OK) {
            err = HAL_SD_CheckWriteOperation(&SDHandle, SDIO_TIMEOUT);
        }

        if (err != SD_OK) {
            printf("write buf:%p  addr:%lu count%lu error:%d\n", buff, sector, count, err);
        }
        dma_deinit();
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    }
    return (err != SD_OK);
}

uint64_t sdcard_get_capacity_in_bytes(void)
{
    HAL_SD_CardInfoTypedef cardinfo;
    HAL_SD_Get_CardInfo(&SDHandle, &cardinfo);
    return cardinfo.CardCapacity;
}
