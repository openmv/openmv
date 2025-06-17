/**
 ******************************************************************************
 * @file    ecloader.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Loader for Epoch Controller BLobs.
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

#if defined(USE_FILES)
#include <stdio.h>
#endif

#include <inttypes.h>
#include <string.h>

#include "ecloader.h"
#include "ll_aton_util.h"

#ifdef USE_FILES

/**
 * Return the size of a file.
 *
 * \param[in] path is the path of the file
 *
 * \return the size of file having path equal to \e path, or -1 in case of errors
 */

long ec_file_size(const char *path)
{
  FILE *file = fopen(path, "r+");

  if (file == NULL)
  {
    LL_ATON_PRINTF("Error: Cannot open file '%s'\n", path);

    return -1;
  }

  fseek(file, 0, SEEK_END);

  long pos = ftell(file);

  fclose(file);

  return pos;
}

/**
 * Copy a file to memory.
 *
 * \param[in] path is the path of the file
 * \param[in] ptr  is the pointer to the memory area (which must be already allocated and large enough) where the file
 * must be copied
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_copy_file(const char *path, uint8_t *ptr)
{
  FILE *file = fopen(path, "r+");

  if (file == NULL)
  {
    LL_ATON_PRINTF("Error: Cannot open file '%s'\n", path);

    return false;
  }

  while (true)
  {
    uint8_t byte;

    if (fread(&byte, sizeof(uint8_t), 1, file) != 1)
      break;

    *ptr = byte;

    ptr++;
  }

  fclose(file);

  return true;
}

#endif /* #ifdef USE_FILES */

/**
 * Copy to memory an Epoch Controller program contained in an Epoch Controller binary.
 * The magic number and the length of the program will be copied as well.
 *
 * \param[in]     file_ptr     is the pointer to the memory area containing the whole Epoch Controller binary
 * \param[out]    program      is the pointer to the memory area (which must be already allocated and large enough) that
 * will contain the Epoch Controller program
 * \param[in,out] program_size is the pointer to the variable that, if \e
 * program is not \e NULL, contains the size in terms of 32-bit words of the memory area pointed by \e program or, if \e
 * program is \e NULL, will contain the size in terms of 32-bit words of the Epoch Controller program (including magic
 * number and program length):
 *                             - <em>program == NULL && program_size != NULL</em> ==> the size of the program section
 * must be retrieved
 *                             - <em>program != NULL && program_size != NULL</em> ==> the program section must be copied
 * (\e program_size is used for checking that the allocated space is sufficient)
 *                             - <em>program != NULL && program_size == NULL</em> ==> not allowed
 *                             - <em>program == NULL && program_size == NULL</em> ==> not allowed
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_copy_program(const uint8_t *file_ptr, ECInstr *program, unsigned int *program_size)
{
  const uint8_t *ptr = file_ptr;

  // read the file header

  ECFileEntry magic = 0;
  ECFileEntry program_offset = 0;

  magic = *((ECFileEntry *)ptr);

  ptr += sizeof(ECFileEntry);

  if (magic != ECASM_BINARY_MAGIC)
  {
    LL_ATON_PRINTF("Error: Epoch Controller binary is invalid\n");

    return false;
  }

  ptr = file_ptr + sizeof(ECFileEntry) + 2 * sizeof(ECFileEntry);

  program_offset = *((ECFileEntry *)ptr);

  ptr += sizeof(ECFileEntry);

  if (program_offset == 0)
  {
    LL_ATON_PRINTF("Error: Program section offset in binary file is invalid\n");

    return false;
  }

  // read the Epoch Controller program section

  ptr = file_ptr + program_offset;

  ECInstr program_magic = 0;

  program_magic = *((ECInstr *)ptr);

  ptr += sizeof(ECInstr);

  if (program_magic != ECASM_PROGRAM_MAGIC)
  {
    LL_ATON_PRINTF("Error: Invalid magic number of Epoch Controller program\n");

    return false;
  }

  ECInstr size = 0;

  size = *((ECInstr *)ptr);

  ptr += sizeof(ECInstr);

  if (program == NULL)
  {
    if (program_size != NULL)
      *program_size = size + 2;
  }
  else
  {
    if ((((intptr_t)program) % 8) != 0)
    {
      LL_ATON_PRINTF("Error: Memory allocated for the Epoch Controller program must be 8-byte aligned\n");

      return false;
    }

    if (program_size == NULL)
    {
      LL_ATON_PRINTF("Error: Size of memory allocated for the Epoch Controller program has not been specified\n");

      return false;
    }

    if (*program_size < (size + 2))
    {
      LL_ATON_PRINTF(
          "Error: Memory allocated for the Epoch Controller program is not sufficient (at least space for %" PRIu32
          " 32-bit words must be allocated)\n",
          size + 2);

      return false;
    }

    program[0] = program_magic;
    program[1] = size;

    for (unsigned int i = 0; i < size; i++, ptr += sizeof(ECInstr))
      program[i + 2] = *((ECInstr *)ptr);
  }

  return true;
}

/**
 * Copy to memory the relocation table contained in an Epoch Controller binary.
 *
 * \param[in]     file_ptr        is the pointer to the memory area containing the whole Epoch Controller binary
 * \param[out]    reloc_table     is the pointer to the memory area (which must be already allocated and large enough)
 * that will contain the Epoch Controller program
 * \param[in,out] reloc_table_size is the pointer to the variable that,
 * if \e reloc_table is not \e NULL, contains the size in terms of 32-bit words of the memory area pointed by \e
 * reloc_table or, if \e reloc_table is \e NULL, will contain the size in terms of 32-bit words of the relocation table:
 *                             - <em>reloc_table == NULL && reloc_table_size != NULL</em> ==> the size of the relocation
 * table must be retrieved
 *                             - <em>reloc_table != NULL && reloc_table_size != NULL</em> ==> the relocation table must
 * be copied
 * (\e reloc_table_size is used for checking that the allocated space is sufficient)
 *                             - <em>reloc_table != NULL && reloc_table_size == NULL</em> ==> not allowed
 *                             - <em>reloc_table == NULL && reloc_table_size == NULL</em> ==> not allowed
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_copy_reloc_table(const uint8_t *file_ptr, ECFileEntry *reloc_table, unsigned int *reloc_table_size)
{
  const ECFileEntry *ptr = (const ECFileEntry *)file_ptr;

  // read the file header

  ECFileEntry magic = *ptr++;

  if (magic != ECASM_BINARY_MAGIC)
  {
    LL_ATON_PRINTF("Error: Epoch Controller binary is invalid\n");

    return false;
  }

  ECFileEntry reloc_table_offset = *((ECFileEntry *)ptr++);
  ECFileEntry debug_offset = *((ECFileEntry *)ptr++);
  ECFileEntry program_offset = *((ECFileEntry *)ptr++);

  if (reloc_table_offset == 0)
  {
    LL_ATON_PRINTF("Error: Relocation table is not present\n");

    return false;
  }

  ECFileEntry size = ((debug_offset == 0) ? program_offset : debug_offset) - reloc_table_offset;

  if ((size % 4) != 0)
  {
    LL_ATON_PRINTF("Error: Size of the relocation table of an Epoch Controller program must be 4-byte aligned\n");

    return false;
  }

  size /= 4;

  if (reloc_table == NULL)
  {
    if (reloc_table_size != NULL)
      *reloc_table_size = size;
  }
  else
  {
    if ((((intptr_t)reloc_table) % 4) != 0)
    {
      LL_ATON_PRINTF(
          "Error: Memory allocated for the relocation table of the Epoch Controller program must be 4-byte aligned\n");

      return false;
    }

    if (reloc_table_size == NULL)
    {
      LL_ATON_PRINTF(
          "Error: Size of memory allocated for the relocation table of the Epoch Controller has not been specified\n");

      return false;
    }

    if (*reloc_table_size < size)
    {
      LL_ATON_PRINTF(
          "Error: Memory allocated for the relocation table of the Epoch Controller program is not sufficient (at "
          "least space for %" PRIu32 " 32-bit words must be allocated)\n",
          size);

      return false;
    }

    memcpy(reloc_table, file_ptr + reloc_table_offset, size * 4);
  }

  return true;
}

/**
 * Get the pointer to the relocation table contained in an Epoch Controller binary.
 *
 * \param[in] file_ptr is the pointer to the memory area containing the whole Epoch Controller binary
 *
 * \return the pointer to the relocation table contained in the Epoch Controller binary pointed by \e file_ptr, or \e
 * NULL if the Epoch Controller binary does not contain any relocation table or on errors
 */

const ECFileEntry *ec_get_reloc_table_ptr(const uint8_t *file_ptr)
{
  const uint8_t *ptr = file_ptr;

  // read the file header

  ECFileEntry magic = 0;
  ECFileEntry reloc_offset = 0;

  magic = *((ECFileEntry *)ptr);

  ptr += sizeof(ECFileEntry);

  if (magic != ECASM_BINARY_MAGIC)
  {
    LL_ATON_PRINTF("Error: Epoch Controller binary is invalid\n");

    return NULL;
  }

  reloc_offset = *((ECFileEntry *)ptr);

  return (reloc_offset == 0) ? NULL : (const ECFileEntry *)(file_ptr + reloc_offset);
}

/**
 * Return the number of different relocations contained in an Epoch Controller binary.
 *
 * \param[in] reloc_table_ptr is the pointer to the relocation table (contained in an Epoch Controller binary or
 * copied from it)
 *
 * \return the number of different relocations contained in the relocation table contained in the memory area starting
 * at \e reloc_table_ptr
 */

unsigned int ec_get_num_relocs(const ECFileEntry *reloc_table_ptr)
{
  ECFileEntry size = 0;

  // read the relocation table, if any
  if (reloc_table_ptr != NULL)
    size = *reloc_table_ptr;

  return size;
}

/**
 * Return the identifier of a relocation contained in an Epoch Controller binary.
 *
 * \param[in] reloc_table_ptr is the pointer to the relocation table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[in] idx      is the index of the relocation whose identifier must be retrieved
 *
 * \return the identifier of relocation having index \e idx among the relocations contained in the relocation table
 * contained in the memory area starting at \e reloc_table_ptr, or \e NULL if that relocation does not exist
 */

const char *ec_get_reloc_id(const ECFileEntry *reloc_table_ptr, unsigned int idx)
{
  // read the relocation table, if any
  if (reloc_table_ptr != NULL)
  {
    const ECFileEntry *ptr = reloc_table_ptr;

    ECFileEntry size = *ptr;

    if (idx < size)
    {
      ptr = reloc_table_ptr + 3 * idx + 1;

      ECFileEntry offset = *ptr;

      return (const char *)((const uint8_t *)reloc_table_ptr + offset);
    }
  }

  return NULL;
}

/**
 * Relocate all the values associated with a relocation specified by using an index.
 *
 * \param[in]     reloc_table_ptr is the pointer to the relocation table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[out]    program         is the pointer to the memory area containing the Epoch Controller
 * program (that will be patched)
 * \param[in]     idx             is the index of the relocation whose values must be
 * relocated
 * \param[in]     base            is the offset that must be added to the values to be relocated
 * \param[in,out] prev_base       is the pointer to a memory location containing the previous value of the base address
 * associated with this relocation (this memory location will be updated with \e base if this function completes
 * successfully)
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_reloc(const ECFileEntry *reloc_table_ptr, ECInstr *program, unsigned int idx, ECAddr base, ECAddr *prev_base)
{
  if (reloc_table_ptr == NULL)
  {
    LL_ATON_PRINTF("Error: Cannot relocate because the pointer to the Epoch Controller relocation table is invalid\n");

    return false;
  }

  if (base == *prev_base)
    return true;

  const ECFileEntry *ptr = reloc_table_ptr;

  ECFileEntry size = *ptr;

  if (idx < size)
  {
    ptr = reloc_table_ptr + 3 * idx + 2;

    ECFileEntry num = *ptr++;

    ECFileEntry offset = *ptr;

    if ((offset % sizeof(ECFileEntry)) != 0)
    {
      LL_ATON_PRINTF("Error: Offset %lu in Epoch Controller binary is invalid\n", (unsigned long)offset);

      return false;
    }

    ptr = (const ECFileEntry *)((const uint8_t *)reloc_table_ptr + offset);

    for (unsigned int i = 0; i < num; i++)
    {
      ECFileEntry offset = *ptr++;

      // offset is from the real beginning of the EC program, that is, from the first real instruction (the one
      // following the magic number of the EC program and its size)
      program[offset + 2] += base - *prev_base;
    }
  }

  *prev_base = base;

  return true;
}

/**
 * Relocate all the values associated with a relocation specified by using an identifier.
 *
 * \param[in]     reloc_table_ptr is the pointer to the relocation table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[out]    program         is the pointer to the memory area containing the Epoch Controller
 * program (that will be patched)
 * \param[in]     id       is the identifier of the relocation whose values must be relocated
 * \param[in]     base            is the offset that must be added to the values to be relocated
 * \param[in,out] prev_base       is the pointer to a memory location containing the previous value of the base address
 * associated with this relocation (this memory location will be updated with \e base if this function completes
 * successfully)
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_reloc_by_id(const ECFileEntry *reloc_table_ptr, ECInstr *program, const char *id, ECAddr base,
                    ECAddr *prev_base)
{
  if (reloc_table_ptr == NULL)
  {
    LL_ATON_PRINTF("Error: Cannot relocate because the pointer to the Epoch Controller relocation table is invalid\n");

    return false;
  }

  if (base == *prev_base)
    return true;

  const ECFileEntry *ptr = reloc_table_ptr;

  ECFileEntry size = *ptr;

  for (unsigned int n = 0; n < size; n++)
  {
    ptr = reloc_table_ptr + 3 * n + 1;

    ECFileEntry offset = *ptr;

    const char *tmp_id = (const char *)((const uint8_t *)reloc_table_ptr + offset);

    if (strcmp(id, tmp_id) == 0)
    {
      ptr++;

      ECFileEntry num = *ptr++;

      ECFileEntry offset = *ptr;

      if ((offset % sizeof(ECFileEntry)) != 0)
      {
        LL_ATON_PRINTF("Error: Offset %lu in Epoch Controller binary is invalid\n", (unsigned long)offset);

        return false;
      }

      ptr = (const ECFileEntry *)((const uint8_t *)reloc_table_ptr + offset);

      for (unsigned int i = 0; i < num; i++)
      {
        ECFileEntry offset = *ptr++;

        // offset is from the real beginning of the EC program, that is, from the first real instruction (the one
        // following the magic number of the EC program and its size)
        program[offset + 2] += base - *prev_base;
      }

      *prev_base = base;

      return true;
    }
  }

  LL_ATON_PRINTF("Error: Relocation symbol '%s' not found in Epoch Controller relocation table\n", id);

  return false;
}
