
/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#ifndef POWER_H_
#define POWER_H_

#include "peripheral_types.h"

static inline void enable_mipi_dphy_power(void)
{
    VBAT->PWR_CTRL &= ~( PWR_CTRL_TX_DPHY_PWR_MASK | PWR_CTRL_RX_DPHY_PWR_MASK | PWR_CTRL_DPHY_PLL_PWR_MASK | PWR_CTRL_DPHY_VPH_1P8_PWR_BYP_EN);
}

static inline void disable_mipi_dphy_power(void)
{
    VBAT->PWR_CTRL |= ( PWR_CTRL_TX_DPHY_PWR_MASK | PWR_CTRL_RX_DPHY_PWR_MASK | PWR_CTRL_DPHY_PLL_PWR_MASK | PWR_CTRL_DPHY_VPH_1P8_PWR_BYP_EN);
}

static inline void enable_mipi_dphy_isolation(void)
{
    VBAT->PWR_CTRL |= ( PWR_CTRL_TX_DPHY_ISO | PWR_CTRL_RX_DPHY_ISO | PWR_CTRL_DPHY_PLL_ISO );
}

static inline void disable_mipi_dphy_isolation(void)
{
    VBAT->PWR_CTRL &= ~( PWR_CTRL_TX_DPHY_ISO | PWR_CTRL_RX_DPHY_ISO | PWR_CTRL_DPHY_PLL_ISO );
}

static inline void enable_usb_phy_power(void)
{
    VBAT->PWR_CTRL &= ~PWR_CTRL_UPHY_PWR_MASK;
}

static inline void disable_usb_phy_power(void)
{
    VBAT->PWR_CTRL |= PWR_CTRL_UPHY_PWR_MASK;
}

static inline void enable_usb_phy_isolation(void)
{
    VBAT->PWR_CTRL |= PWR_CTRL_UPHY_ISO;
}

static inline void disable_usb_phy_isolation(void)
{
    VBAT->PWR_CTRL &= ~PWR_CTRL_UPHY_ISO;
}

#endif /* POWER_H_ */
