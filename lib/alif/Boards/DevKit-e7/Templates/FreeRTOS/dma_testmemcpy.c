/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
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
 * @date     15-Mar-2023
 * @brief    ThreadX demo app for memcpy
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

#include <Driver_DMA.h>

#include <RTE_Components.h>
#include CMSIS_device_header

#if defined( RTE_Compiler_IO_STDOUT )
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

/*RTOS Includes */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/* Enable the DMA controller to test */
//#define TEST_DMA0
#if defined ( M55_HP )
    #define TEST_DMA1
#endif
#if defined ( M55_HE )
    #define TEST_DMA2
#endif


#define DMA0 0                  /* DMA0 */
#define DMA1 1                  /* DMA1 */
#define DMA2 2                  /* DMA2 */

#define DMA_BS       BS_BYTE_8
#define DMA_BLEN     16
#define DMA_XFER_LEN 1000


#if defined ( M55_HE )
/* TCM size is less in RTSS_HE */
#define MAX_TRANSFER_LEN ( 1024 )               /* Total Number of bytes */
#else
#define MAX_TRANSFER_LEN ( 130 * 1024 )         /* Total Number of bytes */
#endif

/*
 * Add extra space at the end of the buffer. This will help to identify if
 * the DMA copies extra bytes
 */
#define ACTUAL_BUFF_SIZE ( MAX_TRANSFER_LEN + 1024 )

static uint8_t  ucSrcBuff[ACTUAL_BUFF_SIZE];
static uint8_t  ucDstBuff[ACTUAL_BUFF_SIZE];

static TaskHandle_t         xTaskDMAMemcpy;

#define DMA_MEMCPY_TASK_STACK_SIZE     1024
#define DMA_MEMCPY_TASK_PRIORITY       3

#define DMA_SEND_COMPLETE_EVENT (1 << 0)
#define DMA_ABORT_EVENT         (1 << 1)


/**
  \fn          void prvFillBuffer( void * pvBuf, uint32_t ulSize )
  \brief       Fill src buffer
  \param[in]   pvBuf Pointer to the buffer
  \param[in]   ulSize Buffer Size
*/
static void prvFillBuffer( void * pvBuf, uint32_t ulSize )
{
    uint8_t * pucTemp = ( uint8_t * )pvBuf;
    uint32_t ulCount;

    for( ulCount = 0; ulCount < ulSize; ulCount++ )
        pucTemp[ulCount] = ( uint8_t )ulCount + 1;
}

/**
  \fn          void prvCompareBuffers( uint8_t *pucSrc, uint8_t *pucDst,
                                       uint32_t ulTransferLen, uint32_t ulDstBuffSize )
  \brief       Compare the src and dst buffers
  \param[in]   pucSrc Pointer to the src buffer
  \param[in]   pucDst Pointer to the dst buffer
  \param[in]   ulTransferLen Transfer length
  \param[in]   ulDstBuffSize Total size of the ucDstBuff
  \return      0 for Success otherwise Error
*/
static int32_t prvCompareBuffers( uint8_t *pucSrc, uint8_t *pucDst,
                                  uint32_t ulTransferLen, uint32_t ulDstBuffSize )
{
    int32_t  lRet;
    uint32_t ulCount;

    lRet = memcmp( pucSrc, pucDst, ulTransferLen );

    if( lRet == 0 )
    {
        for ( ulCount = ulTransferLen; ulCount < ulDstBuffSize; ulCount++ )
        {
            if ( pucDst[ulCount] != 0 )
            {
                printf( "DMA COPIED MORE BYTES\n" );
                return -1;
            }
        }
    }

    return lRet;
}

/**
  \fn          void prvDmacallback( uint32_t ulEvent, int8_t cPeripheralNum )
  \brief       Callback routine from the dma driver
  \param[in]   ulEvent Event for which the callback has been called
  \param[in]   cPeripheralNum Peripheral number
*/
static void prvDmacallback( uint32_t ulEvent, int8_t cPeripheralNum )
{
    ( void ) cPeripheralNum;

    BaseType_t xHigherPriorityTaskWoken, xResult;

    xHigherPriorityTaskWoken = pdFALSE;

    if( ulEvent & ARM_DMA_EVENT_COMPLETE )
    {
        /* Send Success: Wake-up Thread. */
        xResult = xTaskNotifyFromISR( xTaskDMAMemcpy,
                                      DMA_SEND_COMPLETE_EVENT,
                                      eSetBits,
                                      &xHigherPriorityTaskWoken
                                    );

        if(xResult == pdPASS)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if( ulEvent & ARM_DMA_EVENT_ABORT )
    {
        /* Send Success: Wake-up Thread. */
        xResult = xTaskNotifyFromISR( xTaskDMAMemcpy,
                                      DMA_ABORT_EVENT,
                                      eSetBits,
                                      &xHigherPriorityTaskWoken
                                    );

        if(xResult == pdPASS)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/**
  \fn          void prvDmaMemcpyTask( void * pvParameters )
  \brief       DMA Thread to handle transmission
  \param[in]   pvParameters Task Private parameters
*/
static void prvDmaMemcpyTask( void * pvParameters )
{
    ARM_DRIVER_VERSION   xVersion;
    ARM_DRIVER_DMA       *pxDMADrv;
    int32_t              lStatus;
    DMA_Handle_Type      xDMAHandle;
    ARM_DMA_PARAMS       xDMAParams;
    ARM_DMA_BS_Type      xDMABurstSize = DMA_BS;
    uint8_t              ucDMABurstLen = DMA_BLEN;
    uint32_t             ulLength      = DMA_XFER_LEN;
    uint32_t             ulNotificationValue;


    extern ARM_DRIVER_DMA ARM_Driver_DMA_( DMA1 );
    extern ARM_DRIVER_DMA ARM_Driver_DMA_( DMA2 );
    extern ARM_DRIVER_DMA ARM_Driver_DMA_( DMA0 );

    ( void ) pvParameters;

#if defined( TEST_DMA0 )
    pxDMADrv = &ARM_Driver_DMA_( DMA0 );
#elif defined( TEST_DMA1 )
    pxDMADrv = &ARM_Driver_DMA_( DMA1 );
#elif defined( TEST_DMA2 )
    pxDMADrv = &ARM_Driver_DMA_( DMA2 );
#else
    #error Select the DMA
#endif

    /* Verify the DMA API Version for compatibility*/
    xVersion = pxDMADrv->GetVersion();
    printf( "DMA API version = %d\n", xVersion.api );

    /* Initializes DMA interface */
    lStatus = pxDMADrv->Initialize();
    if( lStatus )
    {
        printf( "DMA Init FAILED = %d\n", lStatus );
        while( 1 );
    }

    /* Power control for DMA */
    lStatus = pxDMADrv->PowerControl( ARM_POWER_FULL );
    if( lStatus )
    {
        printf( "DMA Power FAILED = %d\n", lStatus );
        while( 1 );
    }

    /* Allocate handle for DMA */
    lStatus = pxDMADrv->Allocate( &xDMAHandle );
    if( lStatus )
    {
        printf( "DMA Channel Allocation FAILED = %d\n", lStatus );
        while( 1 );
    }

    xDMAParams.peri_reqno = -1;
    xDMAParams.dir        = ARM_DMA_MEM_TO_MEM;
    xDMAParams.cb_event   = prvDmacallback;
    xDMAParams.src_addr   = &ucSrcBuff;
    xDMAParams.dst_addr   = &ucDstBuff;

    xDMAParams.burst_size = xDMABurstSize;
    xDMAParams.burst_len  = ucDMABurstLen;
    xDMAParams.num_bytes  = ulLength;

    printf( "DMA MEMCPY STARTED : Burst Size = %d, Burst len = %d, Transfer Len = %d\n",
            xDMABurstSize, ucDMABurstLen, ulLength );

    /* Start transfer */
    lStatus = pxDMADrv->Start( &xDMAHandle, &xDMAParams );
    if( lStatus || (xDMAHandle < 0) )
    {
        printf( "DMA Start FAILED = %d\n", lStatus );
        while( 1 );
    }

    /* wait for the dma callback */
    if( xTaskNotifyWait( 0U, DMA_SEND_COMPLETE_EVENT | DMA_ABORT_EVENT,
                     &ulNotificationValue, portMAX_DELAY ) == pdPASS)
    {
        if(ulNotificationValue & DMA_ABORT_EVENT)
        {
            printf( "DMA ABORT OCCURRED \n" );
            while( 1 );
        }
    }

    /* Now the buffer is ready, compare it */
    lStatus = prvCompareBuffers( ucSrcBuff, ucDstBuff, ulLength, ACTUAL_BUFF_SIZE );
    if( lStatus )
    {
      printf( "DMA MEMCPY *FAILED* \n" );
    }
    else
    {
      printf( "DMA MEMCPY SUCCESS\n" );
    }

    lStatus = pxDMADrv->DeAllocate( &xDMAHandle );
    if( lStatus )
    {
        printf( "DMA DeAllocate Failed = %d\n", lStatus );
        while( 1 );
    }

    /* Power control for DMA */
    lStatus = pxDMADrv->PowerControl( ARM_POWER_OFF );
    if( lStatus )
    {
        printf( "DMA PowerOff failed = %d\n", lStatus );
        while( 1 );
    }

    printf( "DMA TEST COMPLETED \n" );
}

/**
  \fn          int main (void)
  \brief       Application Main
  \return      int application exit status
*/
int main (void)
{
    extern void SystemCoreClockUpdate ( void );

    BaseType_t xStatus;

    #if defined( RTE_Compiler_IO_STDOUT_User )
    {
        int32_t lRet;
        lRet = stdout_init();
        if( lRet != ARM_DRIVER_OK )
        {
            while( 1 )
            {
            }
        }
    }
    #endif

    memset( ucDstBuff, 0 , ACTUAL_BUFF_SIZE );

    RTSS_CleanDCache_by_Addr( ucDstBuff, ACTUAL_BUFF_SIZE );

    /* Create random data for the DMA Source data*/
    prvFillBuffer( ( void * ) ucSrcBuff, ACTUAL_BUFF_SIZE );

    xStatus = xTaskCreate( prvDmaMemcpyTask,
                           "DMA Memcpy Task",
                           DMA_MEMCPY_TASK_STACK_SIZE / sizeof(size_t),
                           NULL,
                           DMA_MEMCPY_TASK_PRIORITY,
                           &xTaskDMAMemcpy );
    if( xStatus != pdPASS )
    {
       vTaskDelete( xTaskDMAMemcpy );
       printf( "Could not create DMA Task \n" );

       return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();

    return 0;
}
