/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef SYS_CTRL_I3C_H
#define SYS_CTRL_I3C_H

#include <stdint.h>
#include "peripheral_types.h"


static inline void enable_i3c_clock(void)
{
    CLKCTL_PER_SLV->I3C_CTRL |= (1 << 0);
}

static inline void disable_i3c_clock(void)
{
    CLKCTL_PER_SLV->I3C_CTRL &= ( ~(1 << 0) );
}

static inline void select_i3c_dma2(void)
{
    CLKCTL_PER_SLV->I3C_CTRL |= (1 << 24);
}

#endif /* SYS_CTRL_I3C_H */
