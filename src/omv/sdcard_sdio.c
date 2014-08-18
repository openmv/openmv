#include <stdbool.h>
#include <stm32f4xx_hal.h>
#include "mdefs.h"
#include "ffconf.h"
#include "diskio.h"
#include "pincfg.h"
#include "systick.h"
#include "sdcard.h"

#define SDIO_TIMEOUT         (100)  /* in ms */
static SD_HandleTypeDef SDHandle;

bool sdcard_is_present(void)
{
    // CD is not connected on R1
    return true;
}

void sdcard_init(void)
{
    volatile int retry=10;

    /* SDIO initial configuration */
    SDHandle.Instance                 = SDIO;
    SDHandle.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
    SDHandle.Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
    SDHandle.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE; //TODO
    SDHandle.Init.BusWide             = SDIO_BUS_WIDE_1B;
    SDHandle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    SDHandle.Init.ClockDiv            = SDIO_TRANSFER_CLK_DIV; //INIT_CLK_DIV will be used first

    /* DeInit in case of reboot */
    HAL_SD_DeInit(&SDHandle);

    /* Init the SD interface */
    HAL_SD_CardInfoTypedef cardinfo;
    while(HAL_SD_Init(&SDHandle, &cardinfo) != SD_OK && --retry) {
        systick_sleep(100);
    }

    if (retry == 0) {
        BREAK();
    }

    /* Configure the SD Card in wide bus mode. */
    if (HAL_SD_WideBusOperation_Config(&SDHandle, SDIO_BUS_WIDE_4B) != SD_OK) {
        BREAK();
    }
}

bool sdcard_power_on(void)
{
    return true;
}

void sdcard_power_off(void)
{

}

bool sdcard_read_blocks(uint8_t *buff, uint32_t sector, uint32_t count)
{
    __disable_irq();
    HAL_SD_ErrorTypedef err;
    err = HAL_SD_ReadBlocks(&SDHandle, (uint32_t*)buff,
            sector * SDCARD_BLOCK_SIZE, SDCARD_BLOCK_SIZE, count);
    __enable_irq();
    return (err == SD_OK);
}

bool sdcard_write_blocks(const uint8_t *buff, uint32_t sector, uint32_t count)
{
    __disable_irq();
    HAL_SD_ErrorTypedef err;
    err = HAL_SD_WriteBlocks(&SDHandle, (uint32_t*)buff,
            sector * SDCARD_BLOCK_SIZE, SDCARD_BLOCK_SIZE, count);
    __enable_irq();
    return (err == SD_OK);
}

uint64_t sdcard_get_capacity_in_bytes(void)
{
    HAL_SD_CardInfoTypedef cardinfo;
    HAL_SD_Get_CardInfo(&SDHandle, &cardinfo);
    return cardinfo.CardCapacity;
}
