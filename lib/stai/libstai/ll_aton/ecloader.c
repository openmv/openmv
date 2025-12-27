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
 * Get the pointer to the blob contained in an Epoch Controller binary.
 *
 * \param[in] binary_ptr is the pointer to the memory area containing the whole Epoch Controller binary
 *
 * \return the pointer to the blob contained in the Epoch Controller binary pointed by \e binary_ptr, or \e NULL on
 * errors
 */

const uint64_t *ec_get_blob_ptr(const uint8_t *binary_ptr)
{
  const uint8_t *ptr = binary_ptr;

  // read the binary header

  ECFileEntry magic = 0;
  ECFileEntry blob_offset = 0;

  magic = *((ECFileEntry *)ptr);

  ptr += sizeof(ECFileEntry);

  if (magic != ECASM_BINARY_MAGIC)
  {
    LL_ATON_PRINTF("Error: Epoch Controller binary is invalid\n");

    return NULL;
  }

  ptr = binary_ptr + sizeof(ECFileEntry) + 3 * sizeof(ECFileEntry);

  blob_offset = *((ECFileEntry *)ptr);

  ptr += sizeof(ECFileEntry);

  if (blob_offset == 0)
  {
    LL_ATON_PRINTF("Error: Blob section offset in blob binary is invalid\n");

    return NULL;
  }

  return (const uint64_t *)(binary_ptr + blob_offset);
}

/**
 * Copy to memory an Epoch Controller blob contained in an Epoch Controller binary.
 * The magic number and the length of the blob will be copied as well.
 *
 * \param[out]    blob      is the pointer to the memory area (which must be already allocated and large enough) that
 * will contain the Epoch Controller blob
 * \param[in]     binary_ptr   is the pointer to the memory area containing the whole Epoch Controller binary
 * \param[in,out] blob_size is the pointer to the variable that, if \e
 * blob is not \e NULL, contains the size in terms of 32-bit words of the memory area pointed by \e blob or, if \e
 * blob is \e NULL, will contain the size in terms of 32-bit words of the Epoch Controller blob (including magic
 * number and blob length):
 *                             - <em>blob == NULL && blob_size != NULL</em> ==> the size of the blob section
 * must be retrieved
 *                             - <em>blob != NULL && blob_size != NULL</em> ==> the blob section must be copied
 * (\e blob_size is used for checking that the allocated space is sufficient)
 *                             - <em>blob != NULL && blob_size == NULL</em> ==> not allowed
 *                             - <em>blob == NULL && blob_size == NULL</em> ==> not allowed
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_copy_blob(ECInstr *blob, const uint8_t *binary_ptr, unsigned int *blob_size)
{
  const uint8_t *ptr = (const uint8_t *)ec_get_blob_ptr(binary_ptr);

  if (ptr == NULL)
    return false;

  // read the Epoch Controller blob section

  ECInstr blob_magic = 0;

  blob_magic = *((ECInstr *)ptr);

  ptr += sizeof(ECInstr);

  if (blob_magic != ECASM_BLOB_MAGIC)
  {
    LL_ATON_PRINTF("Error: Invalid magic number of Epoch Controller blob\n");

    return false;
  }

  ECInstr size = 0;

  size = *((ECInstr *)ptr);

  ptr += sizeof(ECInstr);

  if (blob == NULL)
  {
    if (blob_size != NULL)
      *blob_size = size + 2;
  }
  else
  {
    if ((((intptr_t)blob) % 8) != 0)
    {
      LL_ATON_PRINTF("Error: Memory allocated for the Epoch Controller blob must be 8-byte aligned\n");

      return false;
    }

    if (blob_size == NULL)
    {
      LL_ATON_PRINTF("Error: Size of memory allocated for the Epoch Controller blob has not been specified\n");

      return false;
    }

    if (*blob_size < (size + 2))
    {
      LL_ATON_PRINTF(
          "Error: Memory allocated for the Epoch Controller blob is not sufficient (at least space for %" PRIu32
          " 32-bit words must be allocated, but only %u 32-bit words were allocated)\n",
          size + 2, *blob_size);

      return false;
    }

    blob[0] = blob_magic;
    blob[1] = size;

    memcpy((uint8_t *)(blob + 2), ptr, size * sizeof(ECInstr));
  }

  return true;
}

/**
 * Copy to memory the relocation table contained in an Epoch Controller binary.
 *
 * \param[out]    reloc_table     is the pointer to the memory area (which must be already allocated and large enough)
 * that will contain the Epoch Controller blob
 * \param[in]     binary_ptr      is the pointer to the memory area containing the whole Epoch Controller binary
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

bool ec_copy_reloc_table(ECFileEntry *reloc_table, const uint8_t *binary_ptr, unsigned int *reloc_table_size)
{
  const ECFileEntry *ptr = (const ECFileEntry *)binary_ptr;

  // read the binary header

  ECFileEntry magic = *ptr++;

  if (magic != ECASM_BINARY_MAGIC)
  {
    LL_ATON_PRINTF("Error: Epoch Controller binary is invalid\n");

    return false;
  }

  ECFileEntry reloc_table_offset = *((ECFileEntry *)ptr++);
  ECFileEntry patch_offset = *((ECFileEntry *)ptr++);
  ECFileEntry debug_offset = *((ECFileEntry *)ptr++);
  ECFileEntry blob_offset = *((ECFileEntry *)ptr++);

  if (reloc_table_offset == 0)
  {
    LL_ATON_PRINTF("Error: Relocation table is not present\n");

    return false;
  }

  ECFileEntry end = blob_offset;

  if (debug_offset != 0)
    end = debug_offset;

  if (patch_offset != 0)
    end = patch_offset;

  ECFileEntry size = end - reloc_table_offset;

  if ((size % 4) != 0)
  {
    LL_ATON_PRINTF("Error: Size of the relocation table of an Epoch Controller blob must be 4-byte aligned\n");

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
          "Error: Memory allocated for the relocation table of the Epoch Controller blob must be 4-byte aligned\n");

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
      LL_ATON_PRINTF("Error: Memory allocated for the relocation table of the Epoch Controller blob is not sufficient "
                     "(at least space for %" PRIu32 " 32-bit words must be allocated)\n",
                     size);

      return false;
    }

    memcpy(reloc_table, binary_ptr + reloc_table_offset, size * 4);
  }

  return true;
}

/**
 * Get the pointer to the relocation table contained in an Epoch Controller binary.
 *
 * \param[in] binary_ptr is the pointer to the memory area containing the whole Epoch Controller binary
 *
 * \return the pointer to the relocation table contained in the Epoch Controller binary pointed by \e binary_ptr, or \e
 * NULL if the Epoch Controller binary does not contain any relocation table or on errors
 */

const ECFileEntry *ec_get_reloc_table_ptr(const uint8_t *binary_ptr)
{
  const uint8_t *ptr = binary_ptr;

  // read the binary header

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

  return (reloc_offset == 0) ? NULL : (const ECFileEntry *)(binary_ptr + reloc_offset);
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
 * Relocate the value associated with a relocation specified by using an index.
 *
 * \param[out]    blob         is the pointer to the memory area containing the Epoch Controller
 * blob (that will be patched)
 * \param[in]     reloc_table_ptr is the pointer to the relocation table (contained in an Epoch Controller binary or
 * copied from it)
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

bool ec_reloc(ECInstr *blob, const ECFileEntry *reloc_table_ptr, unsigned int idx, ECAddr base, ECAddr *prev_base)
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

      // offset is from the real beginning of the EC blob, that is, from the first real instruction (the one
      // following the magic number of the EC blob and its size)
      blob[offset + 2] += base - *prev_base;
    }
  }

  *prev_base = base;

  return true;
}

/**
 * Relocate the value associated with a relocation specified by using an identifier.
 *
 * \param[out]    blob         is the pointer to the memory area containing the Epoch Controller
 * blob (that will be patched)
 * \param[in]     reloc_table_ptr is the pointer to the relocation table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[in]     id       is the identifier of the relocation whose values must be relocated
 * \param[in]     base            is the offset that must be added to the values to be relocated
 * \param[in,out] prev_base       is the pointer to a memory location containing the previous value of the base address
 * associated with this relocation (this memory location will be updated with \e base if this function completes
 * successfully)
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_reloc_by_id(ECInstr *blob, const ECFileEntry *reloc_table_ptr, const char *id, ECAddr base, ECAddr *prev_base)
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

        // offset is from the real beginning of the EC blob, that is, from the first real instruction (the one
        // following the magic number of the EC blob and its size)
        blob[offset + 2] += base - *prev_base;
      }

      *prev_base = base;

      return true;
    }
  }

  LL_ATON_PRINTF("Error: Relocation symbol '%s' not found in Epoch Controller relocation table\n", id);

  return false;
}

/**
 * Copy to memory the patch table contained in an Epoch Controller binary.
 *
 * \param[out]    patch_table     is the pointer to the memory area (which must be already allocated and large enough)
 * that will contain the Epoch Controller blob
 * \param[in]     binary_ptr      is the pointer to the memory area containing the whole Epoch Controller binary
 * \param[in,out] patch_table_size is the pointer to the variable that,
 * if \e patch_table is not \e NULL, contains the size in terms of 32-bit words of the memory area pointed by \e
 * patch_table or, if \e patch_table is \e NULL, will contain the size in terms of 32-bit words of the patch table:
 *                             - <em>patch_table == NULL && patch_table_size != NULL</em> ==> the size of the patch
 * table must be retrieved
 *                             - <em>patch_table != NULL && patch_table_size != NULL</em> ==> the patch table must
 * be copied
 * (\e patch_table_size is used for checking that the allocated space is sufficient)
 *                             - <em>patch_table != NULL && patch_table_size == NULL</em> ==> not allowed
 *                             - <em>patch_table == NULL && patch_table_size == NULL</em> ==> not allowed
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_copy_patch_table(ECFileEntry *patch_table, const uint8_t *binary_ptr, unsigned int *patch_table_size)
{
  const ECFileEntry *ptr = (const ECFileEntry *)binary_ptr;

  // read the binary header

  ECFileEntry magic = *ptr++;

  if (magic != ECASM_BINARY_MAGIC)
  {
    LL_ATON_PRINTF("Error: Epoch Controller binary is invalid\n");

    return false;
  }

  ptr++;

  ECFileEntry patch_table_offset = *(ptr++);
  ECFileEntry debug_offset = *(ptr++);
  ECFileEntry blob_offset = *(ptr++);

  if (patch_table_offset == 0)
  {
    LL_ATON_PRINTF("Error: Patch table is not present\n");

    return false;
  }

  ECFileEntry end = blob_offset;

  if (debug_offset != 0)
    end = debug_offset;

  ECFileEntry size = end - patch_table_offset;

  if ((size % 4) != 0)
  {
    LL_ATON_PRINTF("Error: Size of the patch table of an Epoch Controller blob must be 4-byte aligned\n");

    return false;
  }

  size /= 4;

  if (patch_table == NULL)
  {
    if (patch_table_size != NULL)
      *patch_table_size = size;
  }
  else
  {
    if ((((intptr_t)patch_table) % 4) != 0)
    {
      LL_ATON_PRINTF(
          "Error: Memory allocated for the patch table of the Epoch Controller blob must be 4-byte aligned\n");

      return false;
    }

    if (patch_table_size == NULL)
    {
      LL_ATON_PRINTF(
          "Error: Size of memory allocated for the patch table of the Epoch Controller has not been specified\n");

      return false;
    }

    if (*patch_table_size < size)
    {
      LL_ATON_PRINTF("Error: Memory allocated for the patch table of the Epoch Controller blob is not sufficient (at "
                     "least space for %u "
                     "32-bit words must be allocated)\n",
                     size);

      return false;
    }

    memcpy(patch_table, binary_ptr + patch_table_offset, size * 4);
  }

  return true;
}

/**
 * Get the pointer to the patch table contained in an Epoch Controller binary.
 *
 * \param[in] binary_ptr is the pointer to the memory area containing the whole Epoch Controller binary
 *
 * \return the pointer to the patch table contained in the Epoch Controller binary pointed by \e binary_ptr, or \e
 * NULL if the Epoch Controller binary does not contain any patch table or on errors
 */

const ECFileEntry *ec_get_patch_table_ptr(const uint8_t *binary_ptr)
{
  const uint8_t *ptr = binary_ptr;

  // read the binary header

  ECFileEntry magic = 0;
  ECFileEntry patch_offset = 0;

  magic = *((ECFileEntry *)ptr);

  ptr += sizeof(ECFileEntry);

  if (magic != ECASM_BINARY_MAGIC)
  {
    LL_ATON_PRINTF("Error: Epoch Controller binary is invalid\n");

    return NULL;
  }

  ptr += sizeof(ECFileEntry);

  patch_offset = *((ECFileEntry *)ptr);

  return (patch_offset == 0) ? NULL : (const ECFileEntry *)(binary_ptr + patch_offset);
}

/**
 * Return the number of different patches contained in an Epoch Controller binary.
 *
 * \param[in] patch_table_ptr is the pointer to the patch table (contained in an Epoch Controller binary or
 * copied from it)
 *
 * \return the number of different patches contained in the patch table contained in the memory area starting
 * at \e patch_table_ptr
 */

unsigned int ec_get_num_patches(const ECFileEntry *patch_table_ptr)
{
  ECFileEntry size = 0;

  // read the patch table, if any
  if (patch_table_ptr != NULL)
    size = *patch_table_ptr;

  return size;
}

/**
 * Return the identifier of a patch contained in an Epoch Controller binary.
 *
 * \param[in] patch_table_ptr is the pointer to the patch table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[in] idx      is the index of the patch whose identifier must be retrieved
 *
 * \return the identifier of patch having index \e idx among the patches contained in the patch table
 * contained in the memory area starting at \e patch_table_ptr, or \e NULL if that patch does not exist
 */

const char *ec_get_patch_id(const ECFileEntry *patch_table_ptr, unsigned int idx)
{
  // read the patch table, if any
  if (patch_table_ptr != NULL)
  {
    const ECFileEntry *ptr = patch_table_ptr;

    ECFileEntry size = *ptr;

    if (idx < size)
    {
      ptr = patch_table_ptr + 5 * idx + 1;

      ECFileEntry offset = *ptr;

      return (const char *)((const uint8_t *)patch_table_ptr + offset);
    }
  }

  return NULL;
}

/**
 * Get the number of bits for which the value to apply to the patch contained in an Epoch Controller binary must be
 * shifted right or left.
 *
 * \param[in] patch_table_ptr is the pointer to the patch table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[in] idx      is the index of the patch whose least significant bit must be retrieved
 *
 * \return the number of bits for which the value to apply to the patch having index \e idx among the patches contained
 * in the patch table contained in the memory area starting at \e patch_table_ptr must be shifted right or left, or a
 * value greater than 63 if that patch does not exist
 */

int32_t ec_get_patch_shr(const ECFileEntry *patch_table_ptr, unsigned int idx)
{
  // read the patch table, if any
  if (patch_table_ptr != NULL)
  {
    const ECFileEntry *ptr = patch_table_ptr;

    ECFileEntry size = *ptr;

    if (idx < size)
    {
      ptr = patch_table_ptr + 5 * idx + 2;

      return (int32_t)*ptr;
    }
  }

  return 0;
}

/**
 * Return the mask associated with a patch contained in an Epoch Controller binary.
 *
 * \param[in] patch_table_ptr is the pointer to the patch table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[in] idx      is the index of the patch whose mask must be retrieved
 *
 * \return the mask associated with patch having index \e idx among the patches contained in the patch table
 * contained in the memory area starting at \e patch_table_ptr, or 0 if that patch does not exist
 */

uint32_t ec_get_patch_mask(const ECFileEntry *patch_table_ptr, unsigned int idx)
{
  // read the patch table, if any
  if (patch_table_ptr != NULL)
  {
    const ECFileEntry *ptr = patch_table_ptr;

    ECFileEntry size = *ptr;

    if (idx < size)
    {
      ptr = patch_table_ptr + 5 * idx + 3;

      return *ptr;
    }
  }

  return 0;
}

/**
 * Patch the value associated with a patch specified by using an index.
 *
 * \param[out]    blob         is the pointer to the memory area containing the Epoch Controller
 * blob (that will be patched)
 * \param[in]     patch_table_ptr is the pointer to the patch table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[in]     idx             is the index of the patch whose values must be
 * patched
 * \param[in]     value           is the 64-bit value that must be applied to the patch
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_patch(ECInstr *blob, const ECFileEntry *patch_table_ptr, unsigned int idx, uint64_t value)
{
  if (patch_table_ptr == NULL)
  {
    LL_ATON_PRINTF("Error: Cannot patch because the pointer to the Epoch Controller patch table is invalid\n");

    return false;
  }

  const ECFileEntry *ptr = patch_table_ptr;

  ECFileEntry size = *ptr;

  if (idx < size)
  {
    ptr = patch_table_ptr + 5 * idx + 2;

    int32_t shr = *ptr++;
    uint32_t mask = *ptr++;
    ECFileEntry num = *ptr++;

    ECFileEntry offset = *ptr;

    if ((offset % sizeof(ECFileEntry)) != 0)
    {
      LL_ATON_PRINTF("Error: Offset %lu in Epoch Controller binary is invalid\n", (unsigned long)offset);

      return false;
    }

    if (shr >= 0)
      value >>= shr;
    else
    {
      mask <<= (-shr);
      value <<= (-shr);
    }

    value &= mask;

    ptr = (const ECFileEntry *)((const uint8_t *)patch_table_ptr + offset);

    for (unsigned int i = 0; i < num; i++)
    {
      // offset is from the real beginning of the EC blob, that is, from the first real instruction (the one
      // following the magic number of the EC blob and its size)
      ECFileEntry offset = *ptr++;

      blob[offset + 2] &= ~mask;
      blob[offset + 2] |= value;
    }
  }

  return true;
}

/**
 * Patch all the values associated with a patch specified by using an identifier.
 *
 * \param[out]    blob         is the pointer to the memory area containing the Epoch Controller
 * blob (that will be patched)
 * \param[in]     patch_table_ptr is the pointer to the patch table (contained in an Epoch Controller binary or
 * copied from it)
 * \param[in]     id       is the identifier of the patch whose values must be patched
 * \param[in]     value           is the 64-bit value that must be applied to the patch
 *
 * \retval \e true  on success
 * \retval \e false otherwise
 */

bool ec_patch_by_id(ECInstr *blob, const ECFileEntry *patch_table_ptr, const char *id, uint64_t value)
{
  if (patch_table_ptr == NULL)
  {
    LL_ATON_PRINTF("Error: Cannot patch because the pointer to the Epoch Controller patch table is invalid\n");

    return false;
  }

  const ECFileEntry *ptr = patch_table_ptr;

  ECFileEntry size = *ptr;

  bool found = false;

  uint64_t orig_value = value;

  for (unsigned int n = 0; n < size; n++)
  {
    ptr = patch_table_ptr + 5 * n + 1;

    ECFileEntry offset = *ptr;

    const char *tmp_id = (const char *)((const uint8_t *)patch_table_ptr + offset);

    if (strcmp(id, tmp_id) == 0)
    {
      ptr++;

      int32_t shr = *ptr++;
      uint32_t mask = *ptr++;
      ECFileEntry num = *ptr++;

      ECFileEntry offset = *ptr;

      if ((offset % sizeof(ECFileEntry)) != 0)
      {
        LL_ATON_PRINTF("Error: Offset %lu in Epoch Controller binary is invalid\n", (unsigned long)offset);

        return false;
      }

      if (shr >= 0)
        value = orig_value >> shr;
      else
      {
        mask <<= (-shr);
        value = orig_value << (-shr);
      }

      value &= mask;

      ptr = (const ECFileEntry *)((const uint8_t *)patch_table_ptr + offset);

      for (unsigned int i = 0; i < num; i++)
      {
        // offset is from the real beginning of the EC blob, that is, from the first real instruction (the one
        // following the magic number of the EC blob and its size)
        ECFileEntry offset = *ptr++;

        blob[offset + 2] &= ~mask;
        blob[offset + 2] |= value;
      }

      found = true;
    }
  }

  if (found)
    return true;

  LL_ATON_PRINTF("Error: Patch '%s' not found in Epoch Controller patch table\n", id);

  return false;
}
