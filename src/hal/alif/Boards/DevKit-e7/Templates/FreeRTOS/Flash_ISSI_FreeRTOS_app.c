/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */


/**************************************************************************//**
 * @file     FLASH_ISSI_FreeRTOS_app.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @Version  V1.0.0
 * @date     28-Oct-2022
 * @brief    FreeRTOS Application to demo the APIs of FLASH Driver.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include "stdint.h"
#include "Driver_Flash.h"
#include "Driver_GPIO.h"
#include "pinconf.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/*RTOS Includes*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


#define DEMO_THREAD_STACK_SIZE                        ( 512 )

/* Thread id of thread */
TaskHandle_t xDemoTask;

#define FLASH_NUM 1

extern ARM_DRIVER_FLASH ARM_Driver_Flash_( FLASH_NUM );
#define ptrFLASH ( &ARM_Driver_Flash_( FLASH_NUM ) )

#define OSPI_RESET_PORT     15
#define OSPI_RESET_PIN      7

extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(OSPI_RESET_PORT);
ARM_DRIVER_GPIO *GPIODrv = &ARM_Driver_GPIO_(OSPI_RESET_PORT);

#define FLASH_ADDR 0x00
#define BUFFER_SIZE 1024

/* Buffers for reading and writing data */
uint16_t usReadBuff[  BUFFER_SIZE  ];
uint16_t usWriteBuff[  BUFFER_SIZE  ];

/**
 * @fn      static int32_t prvSetupPinMUX( void )
 * @brief   Set up PinMUX and PinPAD
 * @note    none
 * @param   none
 * @retval  -1 : On Error
 *           0 :  On Success
 */
static int32_t prvSetupPinMUX( void )
{
    int32_t lRet;

    lRet = pinconf_set(PORT_9, PIN_5, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_9, PIN_6, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_9, PIN_7, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |  PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_10, PIN_0, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_10, PIN_1, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_10, PIN_2, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_10, PIN_3, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_10, PIN_4, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |  PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_10, PIN_7, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_READ_ENABLE);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_5, PIN_5, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_8, PIN_0, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_5, PIN_6, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA);
    if (lRet)
        return -1;

    lRet = pinconf_set(PORT_5, PIN_7, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST);
    if (lRet)
        return -1;

    lRet = GPIODrv->Initialize(OSPI_RESET_PIN, NULL);
    if (lRet != ARM_DRIVER_OK)
        return -1;

    lRet = GPIODrv->PowerControl(OSPI_RESET_PIN, ARM_POWER_FULL);
    if (lRet != ARM_DRIVER_OK)
        return -1;

    lRet = GPIODrv->SetDirection(OSPI_RESET_PIN, GPIO_PIN_DIRECTION_OUTPUT);
    if (lRet != ARM_DRIVER_OK)
        return -1;

    lRet = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_LOW);
    if (lRet != ARM_DRIVER_OK)
        return -1;

    lRet = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (lRet != ARM_DRIVER_OK)
        return -1;

    return 0;
}

/**
 * @fn      void vFlashThread ( void *pvParameters )
 * @brief   Used to read, write and erase the flash.
 * @note    none.
 * @param   thread_input : thread input.
 * @retval  none.
 */
void vFlashThread ( void *pvParameters )
{
    uint32_t ulIndex, ulIter = 0, ulCount = 0;
    int32_t lStatus, lRet;
    ARM_DRIVER_VERSION xVersion;
    ARM_FLASH_INFO *pxFlashInfo;

    /* Prepare the data for writing to flash */
    for ( ulIndex = 0; ulIndex < BUFFER_SIZE; ulIndex++ )
    {
        usWriteBuff[ ulIndex ] = ulIndex % 65536;
    }

    printf( "OSPI Flash Initialization\n" );

    lRet = prvSetupPinMUX();

    if ( lRet != ARM_DRIVER_OK )
    {
        printf( "Set up pinmux failed\n" );
        goto error_pinmux;
    }

    /* Get xVersion of the flash */
    xVersion = ptrFLASH->GetVersion();

    printf( "\r\n FLASH xVersion api:%X driver:%X...\r\n",xVersion.api, xVersion.drv );

    /* Initialize the flash */
    lStatus = ptrFLASH->Initialize( NULL );

    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Flash initialization failed\n" );
        goto error_uninitialize;
    }

    lStatus = ptrFLASH->PowerControl( ARM_POWER_FULL );

    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Flash Power control failed\n" );
        goto error_poweroff;
    }

    /* Get Flash Info.*/
    pxFlashInfo = ptrFLASH->GetInfo();

    printf( "\r\n FLASH Info : \n Sector ulCount : %d\n Sector Size : %d Bytes\n Page Size : %d\n Program Unit : %d\n "
             "Erased Value : 0x%X \r\n",pxFlashInfo->sector_count, pxFlashInfo->sector_size, pxFlashInfo->page_size,
             pxFlashInfo->program_unit, pxFlashInfo->erased_value );

    printf( "\nErasing the chip\n" );

    /* Erase the chip */
    lStatus = ptrFLASH->EraseChip();

    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Chip erase failed\n" );
        goto error_poweroff;
    }

    printf( "starting reading erased data\n" );

    ulIter = 0;

    /* Read 2KB data after erase and check if it is erased completely */
    lStatus = ptrFLASH->ReadData( FLASH_ADDR, usReadBuff, BUFFER_SIZE );

    if ( lStatus != BUFFER_SIZE )
    {
        printf( "Data not read completely\n" );
        goto error_poweroff;
    }

    /* Verify the read data */
    while ( ulIter < BUFFER_SIZE )
    {
        if ( usReadBuff[ ulIter ] != ( pxFlashInfo->erased_value << 8 | pxFlashInfo->erased_value ))
            ulCount++;
        ulIter++;
    }

    printf( "Total errors after reading erased chip = %d\n", ulCount );

    printf( "Starting writing\n" );

    /* Write 2 KB data to the flash */
    lStatus = ptrFLASH->ProgramData( FLASH_ADDR, usWriteBuff, BUFFER_SIZE );
    if ( lStatus != BUFFER_SIZE )
    {
        printf( "Data not written completely\n" );
        goto error_poweroff;
    }

    printf( "Finished writing\n" );

    ulIter = 0;
    ulCount = 0;

    printf( "Starting reading after writing\n" );

    /* Read 2KB data after writing to flash */
    lStatus = ptrFLASH->ReadData( FLASH_ADDR, usReadBuff, BUFFER_SIZE );

    if ( lStatus != BUFFER_SIZE )
    {
        printf( "Data not read completely\n" );
        goto error_poweroff;
    }

    while ( ulIter < BUFFER_SIZE )
    {
        if ( usReadBuff[ ulIter ] != usWriteBuff[ ulIter ] )
            ulCount++;
        ulIter++;
    }

    printf( "Total errors after reading data written to flash = %d\n", ulCount );

    ulIter = 0;
    ulCount = 0;

    /* Erase 4KB sector */
    lStatus = ptrFLASH->EraseSector( FLASH_ADDR );

    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Sector erase failed\n" );
        goto error_poweroff;
    }

    printf( "starting reading after erasing a sector\n" );

    /* Read 2KB data after erasing a sector */
    lStatus = ptrFLASH->ReadData( FLASH_ADDR, usReadBuff, BUFFER_SIZE );

    if ( lStatus != BUFFER_SIZE )
    {
        printf( "Data not read completely\n" );
        goto error_poweroff;
    }

    while ( ulIter < BUFFER_SIZE )
    {
        if ( usReadBuff[ ulIter ] != ( pxFlashInfo->erased_value << 8 | pxFlashInfo->erased_value ) )
            ulCount++;
        ulIter++;
    }

    printf( "Total errors after erasing a sector = %d\n", ulCount );

    while ( 1 );

error_poweroff :
    lStatus = ptrFLASH->PowerControl( ARM_POWER_OFF );
    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Flash Power control failed\n" );
    }

error_uninitialize :
    lStatus = ptrFLASH->Uninitialize();
    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Flash un-initialization failed\n" );
    }

error_pinmux :
    printf( "Pinmux error\n" );
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main(  void  )
{
    #if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif
    /* System Initialization */
    SystemCoreClockUpdate();

    /* Create application main thread */
    BaseType_t xReturned = xTaskCreate( vFlashThread, "vFlashThread",
    DEMO_THREAD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, &xDemoTask );

    if ( xReturned != pdPASS )
    {
        vTaskDelete( xDemoTask );
        return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();

}

