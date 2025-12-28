/**
 ******************************************************************************
 * @file    ll_aton_util.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   ATON LL utility functions
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "ll_aton_util.h"

/**
 * @brief  Helper function. Extracts bitfield from a buffer
 * @param  bits Pointer of the buffer to extract bits from
 * @param  pos Global bit position of the bitfield to extract
 * @param  nbits Number of bits to extract
 * @retval extracted bitfield
 */
uint32_t LL_ATON_getbits(uint32_t *bits, unsigned int pos, int nbits)
{
  unsigned int mask = (1ULL << nbits) - 1;
  uint32_t value;
  const int bitsperint = (sizeof(unsigned) * 8);
  int index0 = pos / bitsperint;
  int index1 = (pos + nbits) / bitsperint;
  // Do the bits to extract cross a boundary?
  if (index0 == index1)
  {
    value = (bits[index0] >> (pos % bitsperint)) & mask;
  }
  else
  {
    unsigned int lshift = ((pos + nbits) % bitsperint);
    value =
        (((bits[index0] >> (pos % bitsperint)) & mask) | ((bits[index1] & ((1ULL << lshift) - 1)) << (nbits - lshift)));
  }

  // handle sign extension (not sure if this must be done in every use case)
  if (nbits > 0)
  {
    unsigned int signed_mask = (1UL << (nbits - 1));
    if (value & signed_mask)
    {
      value = value | ~mask;
    }
  }

  return value;
}

/**
 * @brief  Helper function. Packs bitfield into a buffer
 * @param  bits Pointer of the buffer to pack bits into
 * @param  pos Global position of the bitfield to pack
 * @retval nbits Number of bits to extract
 */
void LL_ATON_setbits(uint32_t *bits, unsigned int pos, int nbits, unsigned int val)
{
  const int bitsperint = sizeof(unsigned int) * 8;
  unsigned int smask = ((1ULL << nbits) - 1) << (pos % bitsperint);
  unsigned int mask = ((1ULL << nbits) - 1);
  int index0 = pos / bitsperint;
  int index1 = (pos + nbits - 1) / bitsperint;
  if (index0 == index1)
  {
    uint32_t tmp = (bits[index0] & ~smask) | ((val & mask) << (pos % bitsperint));
    bits[index0] = tmp;
  }
  else
  {
    unsigned int lshift = ((pos + nbits) % bitsperint);
    uint32_t tmp = (bits[index0] & ~smask) | ((val & mask) << (pos % bitsperint));
    bits[index0] = tmp;
    tmp = (bits[index1] & ((1ULL << lshift) - 1)) | ((val & mask) >> (nbits - lshift));
    bits[index1] = tmp;
  }
}
