/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "mram.h"
#include <string.h>

/**
  \fn          void mram_read(void *p_dst, const void *p_src, uint32_t cnt)
  \brief       Read data from source(MRAM) to destination.
  \param[out]  p_dst  Pointer to destination address.
  \param[in]   p_src  Pointer to source address
  \param[in]   cnt    Number of data items to read.
  \return      none
*/
void mram_read(void *p_dst, const void *p_src, uint32_t cnt)
{
    /* directly copy data from MRAM(source),
     * no need for 16-byte memory alignment. */
    memcpy(p_dst, p_src, cnt);
}

/**
  \fn          void mram_write_128bit(uint8_t *p_dst, const uint8_t *p_src)
  \brief       write 128-bit data from source to destination(MRAM).
  \param[out]  p_dst   Pointer to destination address.
  \param[in]   p_src   Pointer to source address.
  \return      none
*/
void mram_write_128bit(uint8_t *p_dst, const uint8_t *p_src)
{
    /* destination (MRAM address) must be always 16-byte aligned,
     * source may or may not be aligned.*/

    /* use temporary buffer for storing source data,
     * in case source data is not 16-bytes aligned.*/
    uint32_t temp_buf[4] = {0}; /* 128-bit.*/

    /* check source address is aligned to 16-bytes? */
    uint8_t *aligned_src = (uint8_t*)((uint32_t)p_src & MRAM_ADDR_ALIGN_MASK);

    /* is source data unaligned? */
    if(p_src != aligned_src)
    {
        /* unaligned source data,
         *  - copy source data first in temporary buffer
         *  - then copy buffer to destination/MRAM.
         */
        memcpy(temp_buf, p_src, MRAM_SECTOR_SIZE);

        ((volatile uint64_t *)p_dst)[0] = ((volatile uint64_t *)temp_buf)[0];
        ((volatile uint64_t *)p_dst)[1] = ((volatile uint64_t *)temp_buf)[1];
    }
    else
    {
        ((volatile uint64_t *)p_dst)[0] = ((volatile uint64_t *)p_src)[0];
        ((volatile uint64_t *)p_dst)[1] = ((volatile uint64_t *)p_src)[1];
    }
}
