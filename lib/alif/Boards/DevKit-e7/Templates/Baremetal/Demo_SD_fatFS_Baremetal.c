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
 * @file     Demo_SD_fatFS_Baremetal.c
 * @author   Deepak Kumar
 * @email    deepak@alifsemi.com
 * @version  V0.0.1
 * @date     02-Sep-2024
 * @brief    Test App for baremetal generic fatfs.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
/* System Includes */
#include "RTE_Device.h"
#include "stdio.h"
#include "string.h"
#include "se_services_port.h"
#include "ff.h"
#include "diskio.h"

/* include for Pin Mux config */
#include "pinconf.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#include "Driver_Common.h"
#endif  /* RTE_Compiler_IO_STDOUT */

#define SDC_A 1
#define SDC_B 2
#define SDC_C 3
#define SDC_PINS SDC_A

#define MEDIA_NAME "/SD_DISK/"

#define TEST_FILE "/TestFile34.txt"
/* Define Test Requirement <Test File Name> */
//#define FILE_CREATE_TEST TEST_FILE
#define FILE_READ_TEST TEST_FILE
//#define FILE_WRITE_TEST TEST_FILE

#define K (1024)

/* Tasks Pool size, stack size, and pointers */
#define STACK_POOL_SIZE (40*K)
#define SD_STACK_SIZE (10*K)
#define SD_BLK_SIZE 512
#define NUM_BLK_TEST 1
#define SD_TEST_ITTR_CNT 10

uint32_t count1, count2, total_cnt=0;
uint32_t service_error_code;
uint32_t error_code = SERVICES_REQ_SUCCESS;

/* Buffer for FileX FF_Disk_t sector cache. This must be large enough for at least one sector , which are typically 512 bytes in size. */
unsigned char media_memory[SD_BLK_SIZE*NUM_BLK_TEST] __attribute__((section("sd_dma_buf"))) __attribute__((aligned(512)));
unsigned char filebuffer[SD_BLK_SIZE*NUM_BLK_TEST] __attribute__((section("sd_dma_buf"))) __attribute__((aligned(512)));
FATFS sd_card __attribute__((section("sd_dma_buf"))) __attribute__((aligned(512)));
FIL test_file;

/**
  \fn           mySD_Thread_entry(unsigned long int args)
  \brief        ThreadX and FileX integrated SD driver Test Function
  \param[in]    args - Thread argument
  \return       none
  */
void SD_Baremetal_fatfs_test(unsigned long int args)
{
    unsigned long int   actual;
    unsigned long int   startCnt, EndCnt;
    FRESULT fr;
    UINT br,bw;
    FILE *fp = NULL;

#if (SDC_PINS == SDC_A)
    pinconf_set(PORT_7, PIN_0, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE); //cmd
    pinconf_set(PORT_7, PIN_1, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE); //clk
    pinconf_set(PORT_5, PIN_0, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_READ_ENABLE); //d0
#if RTE_SDC_BUS_WIDTH == SDMMC_4_BIT_MODE
    pinconf_set(PORT_5, PIN_1, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_READ_ENABLE); //d1
    pinconf_set(PORT_5, PIN_2, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_READ_ENABLE); //d2
    pinconf_set(PORT_5, PIN_3, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE); //d3
#endif

#elif (SDC_PINS == SDC_B)
    pinconf_set(PORT_14, PIN_0, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //cmd
    pinconf_set(PORT_14, PIN_1, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //clk
    pinconf_set(PORT_14, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //rst
    pinconf_set(PORT_13, PIN_0, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //d0
#if RTE_SDC_BUS_WIDTH == SDMMC_4_BIT_MODE
    pinconf_set(PORT_13, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //d1
    pinconf_set(PORT_13, PIN_2, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //d2
    pinconf_set(PORT_13, PIN_3, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //d3
#endif

#elif (SDC_PINS == SDC_C)
    pinconf_set(PORT_7, PIN_0, PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_BUS_REPEATER); //cmd
    pinconf_set(PORT_7, PIN_1, PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_BUS_REPEATER); //clk
    pinconf_set(PORT_5, PIN_0, PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_BUS_REPEATER); //d0
#if RTE_SDC_BUS_WIDTH == SDMMC_4_BIT_MODE
    pinconf_set(PORT_5, PIN_1, PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_BUS_REPEATER); //d1
    pinconf_set(PORT_5, PIN_2, PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_BUS_REPEATER); //d2
    pinconf_set(PORT_5, PIN_3, PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_BUS_REPEATER); //d3
#endif
    pinconf_set(PORT_9, PIN_0, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //cmd
    pinconf_set(PORT_9, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //clk
    pinconf_set(PORT_8, PIN_0, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //d0
#if RTE_SDC_BUS_WIDTH == SDMMC_4_BIT_MODE
    pinconf_set(PORT_8, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //d1
    pinconf_set(PORT_8, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //d2
    pinconf_set(PORT_8, PIN_3, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //d3
#endif

#elif (SDC_PINS == SDC_D)
    pinconf_set(PORT_9, PIN_0, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //cmd
    pinconf_set(PORT_9, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //clk
    //pinconf_set(PORT_9, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //rst
    pinconf_set(PORT_8, PIN_0, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //d0
#if RTE_SDC_BUS_WIDTH == SDMMC_4_BIT_MODE
    pinconf_set(PORT_8, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //d1
    pinconf_set(PORT_8, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //d2
    pinconf_set(PORT_8, PIN_3, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //d3
#endif

#else
    #error "Invalid SDMMC Pins defined...\n"

#endif

    /* Open the SD disk. and initialize SD controller */
    fr = f_mount(&sd_card, MEDIA_NAME, 1);

    /* Check the media open status.  */
    if (fr)
    {
        printf("media open fail status = %d...\n",fr);
        while(1);
    }
    printf("SD Mounted Successfully...\n");


#ifdef FILE_CREATE_TEST

    /* Create a file in the root directory.  */
    fr =  f_open(&test_file, FILE_CREATE_TEST, FA_CREATE_ALWAYS | FA_WRITE);

    if (fr)
    {
        printf("File open status: %d\n",fr);
        /* Error opening file, break the loop.  */
        while(1);
    }


#elif defined(FILE_READ_TEST)

    /* Open the test file.  */
    fr = f_open(&test_file, FILE_READ_TEST, FA_READ);

    /* Check the file open status.  */
    if (fr)
    {
        printf("File open status: %d\n",fr);
        /* Error opening file, break the loop.  */
        while(1);
    }

    printf("Reading Data from File...%s\n",FILE_READ_TEST);
    memset(filebuffer,'\0',sizeof(filebuffer));

    while(1)
    {

        fr =  f_read(&test_file, filebuffer, (SD_BLK_SIZE * NUM_BLK_TEST), &br);

        /* Check the file read status.  */
        if (fr)
        {
            /* Error performing file read, break the loop.  */
            printf("File read status: %d\n",fr);
            break;
        }

        if( f_eof(&test_file) )
        {
            printf("End of File Reached...\n");
            break;
        }

        printf("size = %u\n %s\n",br, (const char *)filebuffer);

    }

#elif defined(FILE_WRITE_TEST)

    /* Open the test file.  */
    fr = f_open(&test_file, FILE_WRITE_TEST, FA_CREATE_ALWAYS | FA_WRITE);

    /* Check the file open status.  */
    if (fr)
    {
        printf("File open status: %d\n",fr);
        /* Error opening file, break the loop.  */
        while(1);
    }

    printf("Writing Data in File...%s\n",FILE_WRITE_TEST);
    memset(filebuffer, '\0', sizeof(filebuffer));

    for(int i = 0; i<SD_TEST_ITTR_CNT; i++)
    {

        memset(filebuffer, 'F', (SD_BLK_SIZE * NUM_BLK_TEST));

        /* Write a string to the test file.  */
        fr =  f_write(&test_file, (void *)filebuffer, (SD_BLK_SIZE * NUM_BLK_TEST), &bw);

        /* Check the file write status.  */
        if (fr)
        {
            printf("ittr: %d File write status: %d\n",i, fr);
            break;
        }
    }
#else
#error "No Test Defined...\n"

#endif

    printf("Closing File...%s\n",TEST_FILE);

    /* Close the test file.  */
    fr = f_close(&test_file);

    /* Check the file close status.  */
    if (fr)
    {
        printf("File close status: %d\n",fr);
        /* Error closing the file, break the loop.  */
        while(1);
    }

    printf("File R/W Test Completed!!!\n");

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, false, &service_error_code);
    if(error_code)
    {
        printf("SE: SDMMC 100MHz clock disable = %d\n", error_code);
        return;
    }

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_USB, false, &service_error_code);
    if(error_code)
    {
        printf("SE: SDMMC 20MHz clock disable = %d\n", error_code);
        return;
    }

    while(1);

}

int main()
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

    /* Initialize the SE services */
    se_services_port_init();

    /* Enable SDMMC Clocks */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, true, &service_error_code);
    if(error_code)
    {
        printf("SE: SDMMC 100MHz clock enable = %d\n", error_code);
        return 0;
    }

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_USB, true, &service_error_code);
    if(error_code)
    {
        printf("SE: SDMMC 20MHz clock enable = %d\n", error_code);
        return 0;
    }

    SD_Baremetal_fatfs_test(NULL);

    return 0;
}

