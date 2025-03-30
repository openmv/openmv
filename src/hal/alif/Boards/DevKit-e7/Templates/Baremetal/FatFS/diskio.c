/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"            /* Obtains integer types */
#include "diskio.h"        /* Declarations of disk functions */
#include "string.h"
#include "stdio.h"

/* Definitions of physical drive number for each drive */
#define DEV_MMC        0    /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB        1    /* Example: Map USB MSD to physical drive 2 */

/* SD Card Instance */
extern sd_handle_t Hsd;
const diskio_t  *p_SD_Driver = &SD_Driver;
static sd_handle_t *pHsd = &Hsd;

/* Interrupt Handler callback */
volatile uint32_t dma_done_irq;
void sd_cb(uint32_t status)
{
    if(status == RES_OK)
        dma_done_irq = 1;
    else
        printf("Invalid Xfer status...:%d\n",status);
}
extern unsigned char media_memory;
volatile uint8_t *dma_buff = &media_memory;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv        /* Physical drive number to identify the drive */
)
{

    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE drivenum)//FATFS *p_sd_card, char *MEDIA_NAME, void * media_memory, uint32_t media_size)
{
    DSTATUS stat;
    int status;
    sd_param_t sd_param;

    sd_param.dev_id         = SDMMC_DEV_ID;
    sd_param.clock_id       = RTE_SDC_CLOCK_SELECT;
    sd_param.bus_width      = RTE_SDC_BUS_WIDTH;
    sd_param.dma_mode       = RTE_SDC_DMA_SELECT;
    sd_param.app_callback   = sd_cb;

    status = p_SD_Driver->disk_initialize(&sd_param);

    if(status)
        return STA_NOINIT;

    /*dummy read */
    memset((void *)dma_buff, '\0', sizeof(media_memory));
    p_SD_Driver->disk_read(0, 1, dma_buff);

    sys_busy_loop_us(1000);

    return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive number to identify the drive */
    BYTE *buff,        /* Data buffer to store read data */
    LBA_t sector,    /* Start sector in LBA */
    UINT count        /* Number of sectors to read */
)
{
    DRESULT res;
    int result;

    dma_done_irq = 0;

    result = p_SD_Driver->disk_read(sector, count, dma_buff);

    while(!dma_done_irq);

    RTSS_InvalidateDCache_by_Addr((volatile void *)dma_buff, count * SDMMC_BLK_SIZE_512_Msk);

    memcpy(buff, (const void *)dma_buff, count * SDMMC_BLK_SIZE_512_Msk);

    return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,          /* Physical drive number to identify the drive */
    const BYTE *buff,   /* Data to be written */
    LBA_t sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    DRESULT res = RES_OK;

    dma_done_irq = 0;
    RTSS_CleanDCache_by_Addr((volatile void *)buff, count * SDMMC_BLK_SIZE_512_Msk);

    memcpy((void*)dma_buff, buff, count * SDMMC_BLK_SIZE_512_Msk);

    if( p_SD_Driver->disk_write(sector, count, (volatile uint8_t *)dma_buff) != SD_DRV_STATUS_OK )
        res = RES_ERROR;

    while(!dma_done_irq);

    return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive number (0..) */
    BYTE cmd,         /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    DRESULT res = 0;

    switch (pdrv) {
    case DEV_MMC :

        // Process of the command for the MMC/SD card

        return res;

    case DEV_USB :

        // Process of the command the USB drive

        return res;
    }

    return RES_PARERR;
}

