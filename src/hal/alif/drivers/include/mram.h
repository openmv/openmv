/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef MRAM_H_
#define MRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
\brief MRAM(On-Chip NVM) Sector Size: 16-Byte(128-bit)
*/
#define MRAM_SECTOR_SIZE         (0x10)
#define MRAM_ADDR_ALIGN_MASK     (uint32_t)(0xFFFFFFF0)

/**
  \fn          void mram_read(void *p_dst, const void *p_src, uint32_t cnt)
  \brief       Read data from source(MRAM) to destination.
  \param[out]  p_dst  Pointer to destination address.
  \param[in]   p_src  Pointer to source address
  \param[in]   cnt    Number of data items to read.
  \return      none
*/
void mram_read(void *p_dst, const void *p_src, uint32_t cnt);

/**
  \fn          void mram_write_128bit(uint8_t *p_dst, const uint8_t *p_src)
  \brief       write 128-bit data from source to destination(MRAM).
  \param[out]  p_dst   Pointer to destination address.
  \param[in]   p_src   Pointer to source address.
  \return      none
*/
void mram_write_128bit(uint8_t *p_dst, const uint8_t *p_src);

#ifdef __cplusplus
}
#endif

#endif /* MRAM_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
