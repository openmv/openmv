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
 * @file     tgu_M55_HP.h
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     3-Jun-2021
 * @brief    TGU initialization code.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef TGU_M55_H
#define TGU_M55_H
#include <stdint.h>
#include <limits.h>

#if defined (M55_HP)
  #include "M55_HP_map.h"
#elif defined (M55_HE)
  #include "M55_HE_map.h"
#else
  #error device not specified!
#endif



#define ITGU_BASE                 (0xE001E500UL)
#define ITGU_CTRL                 (ITGU_BASE + 0x0)
#define ITGU_CFG                  (ITGU_BASE + 0x4)
#define ITGU_LUT(n)               (ITGU_BASE + 0x10 + 4 * n)

#define DTGU_BASE                 (0xE001E600UL)
#define DTGU_CTRL                 (DTGU_BASE + 0x0)
#define DTGU_CFG                  (DTGU_BASE + 0x4)
#define DTGU_LUT(n)               (DTGU_BASE + 0x10 + 4 * n)
#define TGU_LUT(base, n)          ((base) + 0x10 + 4 * (n))

#define DTGU_CFG_BLKSZ            (0xFU << 0)
#define ITGU_CFG_BLKSZ            (0xFU << 0)

#define SET_BIT_RANGE(from, to) (~0U >> ((CHAR_BIT * sizeof(unsigned int)) - (to) - 1)) \
                                    & (~0U << (from))

enum tcm_type {ITCM, DTCM};

struct mem_region {
    uint32_t start;
    uint32_t end;
    enum tcm_type type;
};

/*
 * TGU_Setup()
 * Set up the TGU look up tables as per the information provided by
 * the platform in ns_regions array.
 */
void TGU_Setup(void);
#endif /* TGU_M55_H */
