/**
 ******************************************************************************
 * @file    ll_aton_ec_trace.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file for defining epoch controller trace methods
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

#ifndef __LL_ATON_EC_TRACE_H
#define __LL_ATON_EC_TRACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

  // MCU+NPU cache line size (power of 2 not less than 8)
  extern unsigned int cache_line_size;

  typedef struct mpool_reloc_info_t
  {
    const char *name;
    const char *base_symbol;
    uintptr_t base_address;
    bool is_absolute;
    bool is_user_io;
  } mpool_reloc_info_t;

  // array of memory pool info needed for relocation (ends when name == NULL)
  extern mpool_reloc_info_t mpool_reloc_info[];

  extern uintptr_t get_ec_aton_base(void);
  extern void initialize_ec_aton_base(void);

  extern void ec_trace_comment(const char *comment);

  extern void ec_trace_init(const char *out_filename, const char *network_name, bool encrypted);
  extern void ec_trace_start_blob(const char *blob_name);
  extern void ec_trace_end_blob(const char *blob_name);
  extern void ec_trace_start_epoch(unsigned int num);
  extern void ec_trace_end_epoch(unsigned int num);
  extern void ec_trace_all_blobs_done(void);

  extern void ec_trace_wait_epoch_end(uint32_t wait_mask);

  extern void ec_trace_unsupported(void);

  extern unsigned int ec_trace_get_IP_id(uintptr_t unitbase);
  extern unsigned int ec_trace_get_REG_id(unsigned int regoffset);

  extern void ec_trace_write(uintptr_t dstreg, unsigned int val);
  extern void ec_trace_write_reloc(uintptr_t dstreg, unsigned int base, unsigned int offset);
  extern void ec_trace_add_patch(const char *id, int32_t shr, uint32_t mask);
  extern void ec_trace_reg_write(unsigned int IP_id, unsigned int REG_id, unsigned int val);
  extern void ec_trace_reg_write_reloc(unsigned int IP_id, unsigned int REG_id, unsigned int base, unsigned int offset);
  extern void ec_trace_reg_writefield(unsigned int IP_id, unsigned int REG_id, unsigned int lsb, unsigned int num_bits,
                                      unsigned int val);
  extern void ec_trace_reg_poll(unsigned int IP_id, unsigned int REG_id, unsigned int lsb, unsigned int num_bits,
                                unsigned int val);

#ifdef __cplusplus
}
#endif

#endif // __LL_ATON_EC_TRACE_H
