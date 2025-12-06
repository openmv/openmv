/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     FLASH_ISSI_Baremetal.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     26-Oct-2022
 * @brief    Baremetal Application to demo the APIs of FLASH Driver.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include "pinconf.h"
#include "Driver_Flash.h"
#include "Driver_GPIO.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


#define FLASH_NUM 1

extern ARM_DRIVER_FLASH ARM_Driver_Flash_(FLASH_NUM);
#define ptrFLASH (&ARM_Driver_Flash_(FLASH_NUM))

#define OSPI_RESET_PORT     15
#define OSPI_RESET_PIN      7

extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(OSPI_RESET_PORT);
ARM_DRIVER_GPIO *GPIODrv = &ARM_Driver_GPIO_(OSPI_RESET_PORT);

#define FLASH_ADDR  0x00
#define BUFFER_SIZE 1024

/**
 * @fn      static int32_t setup_PinMUX(void)
 * @brief   Set up PinMUX and PinPAD
 * @note    none
 * @param   none
 * @retval  -1 : On Error
 *           0 : On Success
 */
static int32_t setup_PinMUX(void)
{
    int32_t ret;

    ret = pinconf_set(PORT_9, PIN_5, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_9, PIN_6, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_9, PIN_7, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |  PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_10, PIN_0, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_10, PIN_1, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_10, PIN_2, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_10, PIN_3, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_10, PIN_4, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |  PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_10, PIN_7, PINMUX_ALTERNATE_FUNCTION_7,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_5, PIN_5, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_8, PIN_0, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA);
    if (ret)
        return -1;

    ret = pinconf_set(PORT_5, PIN_7, PINMUX_ALTERNATE_FUNCTION_1,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST);
    if (ret)
        return -1;

    ret = GPIODrv->Initialize(OSPI_RESET_PIN, NULL);
    if (ret != ARM_DRIVER_OK)
        return -1;

    ret = GPIODrv->PowerControl(OSPI_RESET_PIN, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
        return -1;

    ret = GPIODrv->SetDirection(OSPI_RESET_PIN, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK)
        return -1;

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK)
        return -1;

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (ret != ARM_DRIVER_OK)
        return -1;

    return 0;
}

/* Buffers for reading and writing data */
uint16_t read_buff[BUFFER_SIZE];
uint16_t write_buff[BUFFER_SIZE];

/**
 * @fn      int main ()
 * @brief   Main Function
 * @note    none
 * @param   none
 * @retval  0 : Success
 */
int main ()
{
    uint32_t ret, index, iter = 0, count = 0;
    int32_t status;
    ARM_DRIVER_VERSION version;
    ARM_FLASH_INFO *flash_info;

    #if defined(RTE_Compiler_IO_STDOUT_User)
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif

    /* Prepare the data for writing to flash */
    for (index = 0; index < BUFFER_SIZE; index++)
    {
        write_buff[index] = index % 65536;
    }

    printf("OSPI Flash Initialization\n");

    ret = setup_PinMUX();

    if (ret != ARM_DRIVER_OK)
    {
        printf("Set up pinmux failed\n");
        goto error_pinmux;
    }

    /* Get version of the flash */
    version = ptrFLASH->GetVersion();

    printf("\r\n FLASH version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize the flash */
    status = ptrFLASH->Initialize(NULL);

    if (status != ARM_DRIVER_OK)
    {
        printf("Flash initialization failed\n");
        goto error_uninitialize;
    }

    status = ptrFLASH->PowerControl(ARM_POWER_FULL);

    if (status != ARM_DRIVER_OK)
    {
        printf("Flash Power control failed\n");
        goto error_poweroff;
    }

    /* Get Flash Info.*/
    flash_info = ptrFLASH->GetInfo();

    printf("\r\n FLASH Info : \n Sector Count : %d\n Sector Size : %d Bytes\n Page Size : %d\n Program Unit : %d\n "
             "Erased Value : 0x%X \r\n",flash_info->sector_count, flash_info->sector_size, flash_info->page_size,
             flash_info->program_unit, flash_info->erased_value);

    printf("\nErasing the chip\n");

    /* Erase the chip */
    status = ptrFLASH->EraseChip();

    if (status != ARM_DRIVER_OK)
    {
        printf("Chip erase failed\n");
        goto error_poweroff;
    }

    printf("starting reading erased data\n");

    iter = 0;

    /* Read 2KB data after erase and check if it is erased completely */
    status = ptrFLASH->ReadData(FLASH_ADDR, read_buff, BUFFER_SIZE);

    if (status != BUFFER_SIZE)
    {
        printf("Data not read completely\n");
        goto error_poweroff;
    }

    /* Verify the read data */
    while (iter < BUFFER_SIZE)
    {
        if (read_buff[iter] != (flash_info->erased_value << 8 | flash_info->erased_value))
            count++;
        iter++;
    }

    printf("Total errors after reading erased chip = %d\n", count);

    printf("Starting writing\n");

    /* Write 2 KB data to the flash */
    status = ptrFLASH->ProgramData(FLASH_ADDR, write_buff, BUFFER_SIZE);
    if (status != BUFFER_SIZE)
    {
        printf("Data not written completely\n");
        goto error_poweroff;
    }

    printf("Finished writing\n");

    iter = 0;
    count = 0;

    printf("Starting reading after writing\n");

    /* Read 2 KB data after writing to flash */
    status = ptrFLASH->ReadData(FLASH_ADDR, read_buff, BUFFER_SIZE);

    if (status != BUFFER_SIZE)
    {
        printf("Data not read completely\n");
        goto error_poweroff;
    }

    while (iter < BUFFER_SIZE)
    {
        if (read_buff[iter] != write_buff[iter])
            count++;
        iter++;
    }

    printf("Total errors after reading data written to flash = %d\n", count);

    iter = 0;
    count = 0;

    /* Erase 4KB sector */
    status = ptrFLASH->EraseSector(FLASH_ADDR);

    if (status != ARM_DRIVER_OK)
    {
        printf("Sector erase failed\n");
        goto error_poweroff;
    }

    printf("starting reading after erasing a sector\n");

    /* Read 2KB data after erasing a sector */
    status = ptrFLASH->ReadData(FLASH_ADDR, read_buff, BUFFER_SIZE);

    if (status != BUFFER_SIZE)
    {
        printf("Data not read completely\n");
        goto error_poweroff;
    }

    while (iter < BUFFER_SIZE)
    {
        if (read_buff[iter] != (flash_info->erased_value << 8 | flash_info->erased_value))
            count++;
        iter++;
    }

    printf("Total errors after erasing a sector = %d\n", count);

    while (1);

error_poweroff :
    status = ptrFLASH->PowerControl(ARM_POWER_OFF);
    if (status != ARM_DRIVER_OK)
    {
        printf("Flash Power control failed\n");
    }

error_uninitialize :
    status = ptrFLASH->Uninitialize();
    if (status != ARM_DRIVER_OK)
    {
        printf("Flash un-initialization failed\n");
    }

error_pinmux :
    return 0;
}
