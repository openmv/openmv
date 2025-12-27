/**
 ******************************************************************************
 * @file    ec.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of Epoch Controller Blobs.
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

#ifndef __EC_H
#define __EC_H

#include <inttypes.h>

/** Magic number of the Epoch Controller binary file. */
#define ECASM_BINARY_MAGIC 0xECBF0050

/** Magic number of the Epoch Controller blob. */
#define ECASM_BLOB_MAGIC 0xCA057A7A

/** Type containing an Epoch Controller instruction. */
typedef uint32_t ECInstr;

/** Type containing an address of an Epoch Controller instruction. */
typedef uint32_t ECAddr;

/** Type used for each entry of the Epoch Controller binary file: magic number, number of elements, file and instruction
 * offsets. */
typedef uint32_t ECFileEntry;

#endif // #ifndef __EC_H
