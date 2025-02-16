/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*******************************************************************************
 * @file     clock_runtime.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     29-Nov-2023
 * @brief    Wrapper to update the system clocks Information
 ******************************************************************************/
#include "clock_runtime.h"
#include "services_lib_api.h"
#include "se_services_port.h"
#include <RTE_Components.h>
#include CMSIS_device_header

#ifndef MHZ
#define  MHZ            ( 1000000UL)
#endif

static uint32_t clock_frequency_enum_to_value(clock_frequency_t freq_t)
{
    uint32_t clk = 0;

    switch (freq_t)
    {
    case CLOCK_FREQUENCY_800MHZ:
        clk = 800*MHZ;
        break;
    case CLOCK_FREQUENCY_400MHZ:
        clk = 400*MHZ;
        break;
    case CLOCK_FREQUENCY_300MHZ:
        clk = 300*MHZ;
        break;
    case CLOCK_FREQUENCY_200MHZ:
        clk = 200*MHZ;
        break;
    case CLOCK_FREQUENCY_160MHZ:
        clk = 160*MHZ;
        break;
    case CLOCK_FREQUENCY_120MHZ:
        clk = 120*MHZ;
        break;
    case CLOCK_FREQUENCY_80MHZ:
        clk = 80*MHZ;
        break;
    case CLOCK_FREQUENCY_60MHZ:
        clk = 60*MHZ;
        break;
    case CLOCK_FREQUENCY_100MHZ:
        clk = 100*MHZ;
        break;
    case CLOCK_FREQUENCY_50MHZ:
        clk = 50*MHZ;
        break;
    case CLOCK_FREQUENCY_20MHZ:
        clk = 20*MHZ;
        break;
    case CLOCK_FREQUENCY_10MHZ:
        clk = 10*MHZ;
        break;
    case CLOCK_FREQUENCY_76_8_RC_MHZ:
    case CLOCK_FREQUENCY_76_8_XO_MHZ:
        clk = 76800000;
        break;
    case CLOCK_FREQUENCY_38_4_RC_MHZ:
    case CLOCK_FREQUENCY_38_4_XO_MHZ:
        clk = 38400000;
        break;
    default:
        break;
    }

    return clk;
}

/**
  \fn          int32_t system_update_clock_values(void)
  \brief       Update system clock values retrieved from SE services
  \return      0 for SUCCESS
*/
int32_t system_update_clock_values(void)
{
    uint32_t        service_error_code;
    uint32_t        error_code = SERVICES_REQ_SUCCESS;
    run_profile_t   runp = {0};
    uint32_t        frequency = 0;

    /* Get the current run configuration from SE */
    error_code = SERVICES_get_run_cfg(se_services_s_handle,
                                      &runp,
                                      &service_error_code);
    if(error_code)
    {
        return -1;
    }
    SystemCoreClock = clock_frequency_enum_to_value(runp.cpu_clk_freq);

    error_code = SERVICES_clocks_get_apb_frequency(se_services_s_handle,
                                                   &frequency,
                                                   &service_error_code);
    if(error_code)
    {
        return -1;
    }
    SystemAPBClock = frequency;
    SystemAHBClock = frequency * 2;
    SystemAXIClock = frequency * 4;

    error_code = SERVICES_clocks_get_refclk_frequency(se_services_s_handle,
                                                      &frequency,
                                                      &service_error_code);
    if(error_code)
    {
        return -1;
    }
    SystemREFClock = frequency;

    return 0;
}
