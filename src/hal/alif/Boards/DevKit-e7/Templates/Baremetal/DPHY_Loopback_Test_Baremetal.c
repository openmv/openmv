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
 * @file     DPHY_Loopback_Test_Baremetal.c
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     14-Feb-2023
 * @brief    DPHY loopback test source File.
 ******************************************************************************/

/* System Includes */
#include "stdio.h"
#include "stdint.h"
#include "string.h"

/* Loopback test header */
#include "DPHY_Loopback_test.h"

/* Log Retargetting */
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */
#include "Driver_Common.h"


/* PLL Frequency */
#define DPHY_PLL_Frequency                  80000000

/* Time in microseconds for which loopback test continue running */
#define DPHY_Loopback_Test_Runtime          9000000


/* Main entry point */
int main()
{
    int ret = 0;

    #if defined(RTE_Compiler_IO_STDOUT_User)
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif

    ret = DPHY_External_Loopback_Test(DPHY_PLL_Frequency, DPHY_Loopback_Test_Runtime);
    if(ret == TPASS)
    {
        printf("loopback test passed.\r\n");
    }
    else
    {
        printf("loopback test failed.\r\n");
    }

    return 0;


    return 0;
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
