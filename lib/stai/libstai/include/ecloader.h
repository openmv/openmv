/**
 ******************************************************************************
 * @file    ecloader.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of Epoch Controller Blobs Loader.
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

#ifndef __ECLOADER_H
#define __ECLOADER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>
#include <stdbool.h>

#include "ec.h"

#ifdef USE_FILES

  // return the size of a file.
  extern long ec_file_size(const char *path);

  // copy a file to memory
  extern bool ec_copy_file(const char *path, uint8_t *ptr);

#endif /* #ifdef USE_FILES */

  // get the pointer to the blob contained in an Epoch Controller binary
  const uint64_t *ec_get_blob_ptr(const uint8_t *binary_ptr);

  // copy to memory the Epoch Controller blob contained in an Epoch Controller binary
  extern bool ec_copy_blob(ECInstr *blob, const uint8_t *binary_ptr, unsigned int *blob_size);

  /* functions dealing with relocations */

  // copy to memory the relocation table contained in an Epoch Controller binary
  extern bool ec_copy_reloc_table(ECFileEntry *reloc_table, const uint8_t *binary_ptr, unsigned int *reloc_table_size);

  // get the pointer to the relocation table contained in an Epoch Controller binary
  extern const ECFileEntry *ec_get_reloc_table_ptr(const uint8_t *binary_ptr);

  // return the number of different relocations contained in an Epoch Controller binary
  extern unsigned int ec_get_num_relocs(const ECFileEntry *reloc_table_ptr);

  // return the identifier of a relocation contained in an Epoch Controller binary
  extern const char *ec_get_reloc_id(const ECFileEntry *reloc_table_ptr, unsigned int idx);

  // relocate all the values associated with a relocation specified by using an index
  extern bool ec_reloc(ECInstr *blob, const ECFileEntry *reloc_table_ptr, unsigned int idx, ECAddr base,
                       ECAddr *prev_base);

  // relocate all the values associated with a relocation specified by using an identifier
  extern bool ec_reloc_by_id(ECInstr *blob, const ECFileEntry *reloc_table_ptr, const char *id, ECAddr base,
                             ECAddr *prev_base);

  /* functions dealing with patches */

  // copy to memory the patch table contained in an Epoch Controller binary
  extern bool ec_copy_patch_table(ECFileEntry *patch_table, const uint8_t *binary_ptr, unsigned int *patch_table_size);

  // get the pointer to the patch table contained in an Epoch Controller binary
  extern const ECFileEntry *ec_get_patch_table_ptr(const uint8_t *binary_ptr);

  // return the number of different patches contained in an Epoch Controller binary
  extern unsigned int ec_get_num_patches(const ECFileEntry *patch_table_ptr);

  // return the identifier of a patch contained in an Epoch Controller binary
  extern const char *ec_get_patch_id(const ECFileEntry *patch_table_ptr, unsigned int idx);

  // get the number of bits for which the value to apply to the patch contained in an Epoch Controller binary must be
  // shifted right or left
  extern int32_t ec_get_patch_shr(const ECFileEntry *patch_table_ptr, unsigned int idx);

  // return the mask associated with a patch contained in an Epoch Controller binary
  extern uint32_t ec_get_patch_mask(const ECFileEntry *patch_table_ptr, unsigned int idx);

  // patch all the values associated with a patch specified by using an index
  extern bool ec_patch(ECInstr *blob, const ECFileEntry *patch_table_ptr, unsigned int idx, uint64_t value);

  // patch all the values associated with a patch specified by using an identifier
  extern bool ec_patch_by_id(ECInstr *blob, const ECFileEntry *patch_table_ptr, const char *id, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __ECLOADER_H
