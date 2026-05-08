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
  // Find the word and bit offset
  unsigned int word_idx = pos >> 5;
  unsigned int bit_idx = pos & 0x1f;

  uint32_t value;
  if (bit_idx + nbits <= 32)
  {
    uint32_t word = bits[word_idx];
    value = (word >> bit_idx) & ((1ULL << nbits) - 1);
  }
  else
  {
    uint32_t low = bits[word_idx] >> bit_idx;
    uint32_t high = bits[word_idx + 1] & ((1ULL << (nbits - (32 - bit_idx))) - 1);
    value = low | (high << (32 - bit_idx));
  }

  // Sign extension if needed
  if (nbits < 32 && (value & (1ULL << (nbits - 1))))
  {
    // If sign bit is set, extend the sign
    value |= ~((1ULL << nbits) - 1);
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
