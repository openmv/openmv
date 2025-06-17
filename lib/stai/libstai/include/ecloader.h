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

  // copy to memory the Epoch Controller program contained in an Epoch Controller binary
  extern bool ec_copy_program(const uint8_t *file_ptr, ECInstr *program, unsigned int *program_size);

  // copy to memory the relocation table contained in an Epoch Controller binary
  extern bool ec_copy_reloc_table(const uint8_t *file_ptr, ECFileEntry *reloc_table, unsigned int *reloc_table_size);

  // get the pointer to the relocation table contained in an Epoch Controller binary
  extern const ECFileEntry *ec_get_reloc_table_ptr(const uint8_t *file_ptr);

  // return the number of different relocations contained in an Epoch Controller binary
  extern unsigned int ec_get_num_relocs(const ECFileEntry *reloc_table_ptr);

  // return the identifier of a relocation contained in an Epoch Controller binary
  extern const char *ec_get_reloc_id(const ECFileEntry *reloc_table_ptr, unsigned int idx);

  // relocate all the values associated with a relocation specified by using an index
  extern bool ec_reloc(const ECFileEntry *reloc_table_ptr, ECInstr *program, unsigned int idx, ECAddr base,
                       ECAddr *prev_base);

  // relocate all the values associated with a relocation specified by using an identifier
  extern bool ec_reloc_by_id(const ECFileEntry *reloc_table_ptr, ECInstr *program, const char *id, ECAddr base,
                             ECAddr *prev_base);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __ECLOADER_H
