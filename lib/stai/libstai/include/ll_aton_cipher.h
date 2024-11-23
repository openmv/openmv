/**
 ******************************************************************************
 * @file    ll_aton_cipher.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON LL module.
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

#ifndef __LL_ATON_CIPHER_H
#define __LL_ATON_CIPHER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

  /**
   * @brief Encryption configuration structure for Streaming Engines and Epoch Controller units
   */
  typedef struct
  {
    unsigned int enable;    /**< Enable/disable encryption (0, 1) */
    uint64_t encryption_id; /**< 43 bit Encryption ID */
    unsigned int rounds;    /**< Number of encryption rounds: 0->12 rounds, 1->9 rounds */
    unsigned int key_sel;   /**< Bus Interface encryption key selection (0, 1) */
    unsigned int increment; /**< Encryption ID increment rate: 0 -> no increment, <n> -> +1 every n frames */
  } LL_Streng_EncryptionTypedef;

  typedef enum
  {
    CYPHER_CACHE_NONE = 0,
    CYPHER_CACHE_SRC,
    CYPHER_CACHE_DST,
  } CypherCacheSourceMask;

  typedef enum
  {
    CYPHER_DISABLE_MASK = 0,
    CYPHER_SRC_MASK,
    CYPHER_DST_MASK,
  } CypherEnableMask;

  /**
   * @brief Cyphering configuration structure for DmaCypher function
   */

  typedef struct
  {
    uint32_t srcAdd;                       /**< Transfer source address */
    uint32_t dstAdd;                       /**< Transfer destination address */
    uint32_t len;                          /**< Transfer size */
    CypherCacheSourceMask cypherCacheMask; /**< Cache usage mask:
                                            *     0-no cache
                                            *     1-cache source
                                            *     2-cache destination */
    CypherEnableMask cypherEnableMask;     /**< Cyphering channel mask:
                                            *     0-no cypher
                                            *     1-cypher source
                                            *     2-cypher destination */
    uint64_t busIfKeyLsb;                  /**< Bus interface LSB Key */
    uint64_t busIfKeyMsb;                  /**< Bus interface MSB Key */
  } LL_Cypher_InitTypeDef;

#define CYPHER_SRC_STRENG_ID 0       /**< Stream engine used for source data in Dma/Cypher function */
#define CYPHER_DST_STRENG_ID 1       /**< Stream engine used for destination data in Dma/Cypher function */
#define CYPHER_CACHE_SIZE    0x40000 /**< N6 cache size */

  int LL_Streng_EncryptionInit(int id, LL_Streng_EncryptionTypedef *);
  int LL_Streng_WeightEncryptionInit(int id);
  int LL_EpochCtrl_EncryptionInit(int id, LL_Streng_EncryptionTypedef *conf);
  int LL_DmaCypherInit(LL_Cypher_InitTypeDef *cypherInfo);

#ifdef __cplusplus
}
#endif

#endif //__LL_ATON_CIPHER_H
