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
 * @file     clk.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     29-Nov-2023
 * @brief    Clock information
 ******************************************************************************/
#include <clk.h>

#ifndef SYST_ACLK
#define SYST_ACLK   400000000
#endif

#ifndef SYST_HCLK
#define SYST_HCLK   200000000
#endif

#ifndef SYST_PCLK
#define SYST_PCLK   100000000
#endif

#ifndef SYST_REFCLK
#define SYST_REFCLK 100000000
#endif

/*----------------------------------------------------------------------------
  System AXI Clock Variable(SYST_ACLK)
 *----------------------------------------------------------------------------*/
uint32_t SystemAXIClock = SYST_ACLK;

/*----------------------------------------------------------------------------
  System AHB Clock Variable(SYST_HCLK)
 *----------------------------------------------------------------------------*/
uint32_t SystemAHBClock = SYST_HCLK;

/*----------------------------------------------------------------------------
  System APB Clock Variable(SYST_PCLK)
 *----------------------------------------------------------------------------*/
uint32_t SystemAPBClock = SYST_PCLK;

/*----------------------------------------------------------------------------
  System REF Clock Variable(SYST_REFCLK)
 *----------------------------------------------------------------------------*/
uint32_t SystemREFClock = SYST_REFCLK;

/*----------------------------------------------------------------------------
  System HFOSC Clock Variable(HFOSC_CLK)
 *----------------------------------------------------------------------------*/
uint32_t SystemHFOSCClock = HFOSC_CLK;

/*----------------------------------------------------------------------------
  Get System AXI Clock function
 *----------------------------------------------------------------------------*/
uint32_t GetSystemAXIClock (void)
{
  return SystemAXIClock;
}

/*----------------------------------------------------------------------------
  Get System AHB Clock function
 *----------------------------------------------------------------------------*/
uint32_t GetSystemAHBClock (void)
{
  return SystemAHBClock;
}

/*----------------------------------------------------------------------------
  Get System APB Clock function
 *----------------------------------------------------------------------------*/
uint32_t GetSystemAPBClock (void)
{
  return SystemAPBClock;
}

/*----------------------------------------------------------------------------
  Get System REF Clock function
 *----------------------------------------------------------------------------*/
uint32_t GetSystemREFClock (void)
{
  return SystemREFClock;
}

/*----------------------------------------------------------------------------
  Get System HFOSC Clock function
 *----------------------------------------------------------------------------*/
uint32_t GetSystemHFOSClock (void)
{
  return SystemHFOSCClock;
}