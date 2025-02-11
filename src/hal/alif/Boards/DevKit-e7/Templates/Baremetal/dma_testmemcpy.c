/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     dma_testmemcpy.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     16-Sep-2023
 * @brief    Baremetal demo app for memcpy
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include <Driver_DMA.h>
#include <string.h>
#include <RTE_Components.h>
#include CMSIS_device_header

#ifdef RTE_Compiler_IO_STDOUT
#include "retarget_stdout.h"
#endif

/* Enable the DMA controller to test */
//#define TEST_DMA0
#if defined (M55_HP)
#define TEST_DMA1
#endif
#if defined (M55_HE)
#define TEST_DMA2
#endif


#define DMA0 0                  /* DMA0 */
#define DMA1 1                  /* DMA1 */
#define DMA2 2                  /* DMA2 */

#define DMA_BS       BS_BYTE_8
#define DMA_BLEN     16
#define DMA_XFER_LEN 1000


#if defined (M55_HE)
/* TCM size is less in RTSS_HE */
#define MAX_TRANSFER_LEN (1024)               /* Total Number of bytes */
#else
#define MAX_TRANSFER_LEN (130*1024)           /* Total Number of bytes */
#endif

/*
 * Add extra space at the end of the buffer. This will help to identify if
 * the DMA copies extra bytes
 */
#define ACTUAL_BUFF_SIZE (MAX_TRANSFER_LEN + 1024)

static uint8_t  src_buff[ACTUAL_BUFF_SIZE];
static uint8_t  dst_buff[ACTUAL_BUFF_SIZE];

volatile uint32_t dma_cb_event = 0;

#define DMA_SEND_COMPLETE_EVENT (1 << 0)
#define DMA_ABORT_EVENT         (1 << 1)


/**
  \fn          void fillbuffer(void *buf, uint32_t size)
  \brief       Fill src buffer
  \param[in]   buf Pointer to the buffer
  \param[in]   size Buffer Size
*/
static void fillbuffer(void *buf, uint32_t size)
{
    uint8_t *ptr = (uint8_t*)buf;
    uint32_t cnt;

    for(cnt = 0; cnt < size; cnt++)
        ptr[cnt] = (uint8_t)cnt + 1;
}

/**
  \fn          void comparebuffers(uint8_t *src, uint8_t *dst,
                                   uint32_t transfer_len, uint32_t dst_buff_size)
  \brief       Compare the src and dst buffers
  \param[in]   src Pointer to the src buffer
  \param[in]   dst Pointer to the dst buffer
  \param[in]   transfer_len Transfer length
  \param[in]   dst_buff_size Total size of the dst_buff
  \return      0 for Success otherwise Error
*/
static int32_t comparebuffers(uint8_t *src, uint8_t *dst,
                              uint32_t transfer_len, uint32_t dst_buff_size)
{
    int32_t  ret;
    uint32_t cnt;

    ret = memcmp(src, dst, transfer_len);

    if(ret == 0)
    {
        for (cnt = transfer_len; cnt < dst_buff_size; cnt++)
        {
            if (dst[cnt] != 0 )
            {
                printf(" DMA COPIED MORE BYTES\n");
                return -1;
            }
        }
    }

    return ret;


}

/**
  \fn          void dma_cb(uint32_t event, int8_t peri_num)
  \brief       Callback routine from the dma driver
  \param[in]   event Event for which the callback has been called
  \param[in]   peri_num Peripheral number
*/
void dma_cb(uint32_t event, int8_t peri_num)
{
    (void)peri_num;

    if(event & ARM_DMA_EVENT_COMPLETE)
    {
        dma_cb_event = DMA_SEND_COMPLETE_EVENT;
    }


    if(event & ARM_DMA_EVENT_ABORT)
    {
        dma_cb_event = DMA_ABORT_EVENT;
    }
}

/**
  \fn          void dma_memcpy_task(void)
  \brief       DMA task to handle transmission
*/
static void dma_memcpy_task(void)
{
    ARM_DRIVER_VERSION   version;
    ARM_DRIVER_DMA       *dma_drv;
    int32_t              status;
    DMA_Handle_Type      handle;
    ARM_DMA_PARAMS       params;
    int32_t              ret;
    ARM_DMA_BS_Type      bs   = DMA_BS;
    uint8_t              blen = DMA_BLEN;
    uint32_t             len  = DMA_XFER_LEN;


    extern ARM_DRIVER_DMA ARM_Driver_DMA_(DMA1);
    extern ARM_DRIVER_DMA ARM_Driver_DMA_(DMA2);
    extern ARM_DRIVER_DMA ARM_Driver_DMA_(DMA0);

#if defined(TEST_DMA0)
    dma_drv = &ARM_Driver_DMA_(DMA0);
#elif defined(TEST_DMA1)
    dma_drv = &ARM_Driver_DMA_(DMA1);
#elif defined(TEST_DMA2)
    dma_drv = &ARM_Driver_DMA_(DMA2);
#else
    #error Select the DMA
#endif

    /* Verify the DMA API version for compatibility*/
    version = dma_drv->GetVersion();
    printf ("DMA API version = %d\n", version.api);

    /* Initializes DMA interface */
    status = dma_drv->Initialize();
    if(status)
    {
        printf ("DMA Init FAILED = %d\n", status);
        while(1);
    }

    /* Power control for DMA */
    status = dma_drv->PowerControl(ARM_POWER_FULL);
    if(status)
    {
        printf ("DMA Power FAILED = %d\n", status);
        while(1);
    }

    /* Allocate handle for DMA */
    status = dma_drv->Allocate(&handle);
    if(status)
    {
        printf ("DMA Channel Allocation FAILED = %d\n", status);
        while(1);
    }

    params.peri_reqno = -1;
    params.dir        = ARM_DMA_MEM_TO_MEM;
    params.cb_event   = dma_cb;
    params.src_addr   = &src_buff;
    params.dst_addr   = &dst_buff;

    params.burst_size = bs;
    params.burst_len  = blen;
    params.num_bytes  = len;

    printf("DMA MEMCPY STARTED : Burst Size = %d, Burst len = %d, Transfer Len = %d\n", bs, blen, len);

    /* Start transfer */
    status = dma_drv->Start(&handle, &params);
    if(status || (handle < 0))
    {
        printf("DMA Start FAILED = %d\n", status);
        while(1);
    }

    /* wait for the dma callback */
    while(dma_cb_event == 0)
    {
        __WFE();
    }

    if(dma_cb_event == DMA_ABORT_EVENT)
    {
        printf("DMA ABORT OCCURRED \n");
        while(1);
    }

    dma_cb_event = 0;

    /* Now the buffer is ready, compare it */
    ret = comparebuffers(src_buff, dst_buff, len, ACTUAL_BUFF_SIZE);
    if(ret)
    {
      printf("DMA MEMCPY *FAILED* \n");
    }
    else
    {
      printf("DMA MEMCPY SUCCESS\n");
    }

    status = dma_drv->DeAllocate(&handle);
    if(status)
    {
        printf("DMA DeAllocate Failed = %d\n", status);
        while(1);
    }

    /* Power control for DMA */
    status = dma_drv->PowerControl(ARM_POWER_OFF);
    if(status)
    {
        printf ("DMA PowerOff failed = %d\n", status);
        while(1);
    }

    printf("DMA TEST COMPLETED \n");
}

/**
  \fn          int main (void)
  \brief       Application Main
  \return      int application exit status
*/
int main (void)
{
#if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1);
    }
#endif

    memset (dst_buff, 0 , ACTUAL_BUFF_SIZE);

    RTSS_CleanDCache_by_Addr(dst_buff, ACTUAL_BUFF_SIZE);

    /* Create random data for the DMA Source data*/
    fillbuffer((void*)src_buff, ACTUAL_BUFF_SIZE);

    dma_cb_event = 0;

    /* Enter the dma task  */
    dma_memcpy_task();
}
