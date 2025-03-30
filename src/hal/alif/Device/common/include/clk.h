/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef CLK_H_
#define CLK_H_

#include <peripheral_types.h>

#define PLL_CLK1    800000000
#define PLL_CLK3    480000000

#ifndef HFOSC_CLK
#define HFOSC_CLK   38400000
#endif

static inline void enable_force_peripheral_functional_clk(void)
{
    CLKCTL_PER_SLV->EXPMST0_CTRL |= EXPMST0_CTRL_IPCLK_FORCE;
}

static inline void disable_force_peripheral_functional_clk(void)
{
    CLKCTL_PER_SLV->EXPMST0_CTRL &= ~EXPMST0_CTRL_IPCLK_FORCE;
}

static inline void enable_force_apb_interface_clk(void)
{
    CLKCTL_PER_SLV->EXPMST0_CTRL |= EXPMST0_CTRL_PCLK_FORCE;
}

static inline void disable_force_apb_interface_clk(void)
{
    CLKCTL_PER_SLV->EXPMST0_CTRL &= ~EXPMST0_CTRL_PCLK_FORCE;
}

static inline void enable_cgu_clk160m(void)
{
    CGU->CLK_ENA |= CLK_ENA_CLK160M;
}

static inline void disable_cgu_clk160m(void)
{
    CGU->CLK_ENA &= ~CLK_ENA_CLK160M;
}
static inline void enable_cgu_clk100m(void)
{
    CGU->CLK_ENA |= CLK_ENA_CLK100M;
}

static inline void disable_cgu_clk100m(void)
{
    CGU->CLK_ENA &= ~CLK_ENA_CLK100M;
}

static inline void enable_cgu_clk20m(void)
{
    CGU->CLK_ENA |= CLK_ENA_CLK20M;
}

static inline void disable_cgu_clk20m(void)
{
    CGU->CLK_ENA &= ~CLK_ENA_CLK20M;
}

static inline void enable_cgu_clk38p4m(void)
{
    CGU->CLK_ENA |= CLK_ENA_CLK38P4M;
}

static inline void disable_cgu_clk38p4m(void)
{
    CGU->CLK_ENA &= ~CLK_ENA_CLK38P4M;
}

static inline void enable_gpu_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_GPU_CKEN;
}

static inline void disable_gpu_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_GPU_CKEN;
}

static inline void enable_sdc_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_SDC_CKEN;
}

static inline void disable_sdc_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_SDC_CKEN;
}

static inline void enable_usb_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_USB_CKEN;
}

static inline void disable_usb_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_USB_CKEN;
}

/**
  \brief System AXI Clock Frequency (AXI Clock)
*/
extern uint32_t SystemAXIClock;

/**
  \brief System AHB Clock Frequency (AHB Clock)
*/
extern uint32_t SystemAHBClock;

/**
  \brief System APB Clock Frequency (APB Clock)
*/
extern uint32_t SystemAPBClock;

/**
  \brief System REF Clock Frequency (REF Clock)
*/
extern uint32_t SystemREFClock;

/**
  \brief System HFOSC Clock Frequency (HFOSC Clock)
*/
extern uint32_t SystemHFOSCClock;

/**
  \brief  Get System AXI Clock value.

   returns the currently configured AXI clock value.
 */
uint32_t GetSystemAXIClock(void);

/**
  \brief  Get System AHB Clock value.

   returns the currently configured AHB clock value.
 */
uint32_t GetSystemAHBClock(void);

/**
  \brief  Get System APB Clock value.

   returns the currently configured APB clock value.
 */
uint32_t GetSystemAPBClock(void);

/**
  \brief  Get System REF Clock value.

   returns the currently configured REF clock value.
 */
uint32_t GetSystemREFClock(void);

/**
  \brief  Get System HFOSC Clock value.

   returns the currently configured HFOSC clock value.
 */
uint32_t GetSystemHFOSClock(void);

#endif /* CLK_H_ */
