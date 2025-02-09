/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#if defined (M55_HP)
  #include "M55_HP.h"
#elif defined (M55_HE)
  #include "M55_HE.h"
#else
  #error device not specified!
#endif

#include "tgu_M55.h"
#include "tcm_partition.h"

/*
 * Array of NS memory regions. Add new regions to this list.
 * The start and end address of the non-secure region in updated
 * from the linker scripts.
 */
static const struct mem_region ns_regions[] __STARTUP_RO_DATA_ATTRIBUTE = {
    {(uint32_t)&ns_region_0_start, (uint32_t)&ns_region_0_end - 1, DTCM},
    };

/*----------------------------------------------------------------------------
  TGU setup function. Marks single memory region starting at NS_REGION_1_BASE
  and ending at NS_REGION_1_END as Non Secure in the TGU look up tables. The
  region should be in the DTCM area. Can be extended to include the ITCM region
  and multiple discontigous regions if needed.
 *----------------------------------------------------------------------------*/
void TGU_Setup(void)
{
    uint32_t start_block, end_block, start_lut, end_lut;
    uint32_t start_offset, end_offset, lut_val_l, lut_val_h;
    uint32_t base, itcm_blksize, dtcm_blksize, blksize;
    uint32_t i;

    /* Find out the TGU block size for ITCM */
    blksize = *((volatile unsigned int *) ITGU_CFG) & ITGU_CFG_BLKSZ;
    /* ITCM blksize is 2^(blksize + 5)*/
    itcm_blksize = 1 << (blksize + 5);

    /* Find out the TGU block size for DTCM */
    blksize = *((volatile unsigned int *) DTGU_CFG) & DTGU_CFG_BLKSZ;
    /* DTCM block size is 2^(blksize + 5) */
    dtcm_blksize = 1 << (blksize + 5);

    for (i = 0; i < (sizeof(ns_regions) / sizeof(ns_regions[0])); i++) {
        if (ns_regions[i].type == DTCM) {
            start_block =  (ns_regions[i].start - DTCM_BASE) / dtcm_blksize;
            end_block = (ns_regions[i].end - DTCM_BASE) / dtcm_blksize;
            base = DTGU_BASE;
        } else {
            start_block =  (ns_regions[i].start - ITCM_BASE) / itcm_blksize;
            end_block = (ns_regions[i].end - ITCM_BASE) / itcm_blksize;
            base = ITGU_BASE;
        }

        start_lut = start_block / 32;
        end_lut = end_block / 32;

        start_offset = start_block % 32;
        end_offset = end_block % 32;

        if (start_lut == end_lut) {
            /* same LUT register */
            lut_val_l = SET_BIT_RANGE(start_offset, end_offset);
            *((volatile uint32_t *) TGU_LUT(base, start_lut)) |= lut_val_l;
        } else {
            /* the range spans multiple LUT registers */
            lut_val_l = SET_BIT_RANGE(start_offset, 31);
            lut_val_h = SET_BIT_RANGE(0, end_offset);

            /* Write into the first LUT register */
            *((volatile uint32_t *) TGU_LUT(base, start_lut)) = lut_val_l;

            /* Now write to all the intermediate LUT registers */
            while (start_lut != (end_lut - 1)) {
                start_lut++;
                *((volatile uint32_t *) TGU_LUT(base, start_lut)) = 0xFFFFFFFFU;
            }

            /* Write into the last LUT register*/
            *((volatile uint32_t *) TGU_LUT(base, end_lut)) |= lut_val_h;
        }
    }
}
