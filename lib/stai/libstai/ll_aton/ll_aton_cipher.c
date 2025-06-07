/**
 ******************************************************************************
 * @file    ll_aton_cipher.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   ATON LL module driver.
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

#include "ll_aton_cipher.h"
#include "ll_aton.h"
#include "ll_aton_platform.h"

#if !defined(DEFAULT_WEIGHT_ENCRYPTION_ID)
#define DEFAULT_WEIGHT_ENCRYPTION_ID 0
#endif
#if !defined(DEFAULT_WEIGHT_ENCRYPTION_ROUNDS)
#define STRENG_WEIGHT_ENCRYPTION_ROUNDS_12 0
#define STRENG_WEIGHT_ENCRYPTION_ROUNDS_9  1
#define DEFAULT_WEIGHT_ENCRYPTION_ROUNDS   STRENG_WEIGHT_ENCRYPTION_ROUNDS_12
#endif
#if !defined(DEFAULT_WEIGHT_KEY_SEL)
#define DEFAULT_WEIGHT_KEY_SEL 0
#endif

#if (ATON_STRENG_VERSION_ENCR_DT == 1)
/**
 * @brief Configures the Streaming Engine Encryption support
 * @param id Streaming engine identifier [0..ATON_STRENG_NUM-1]
 * @param LL_Streng_EncryptionStruct Pointer to structure describing encryption parameters
 * @note Encryption keys are set in the Bus Interfaces using LL_Busif_SetKeys function
 * @note Each Stream Engine is associated to a Bus Interface. Machine descriptor json file describes how they are
 * grouped
 */
int LL_Streng_EncryptionInit(int id, LL_Streng_EncryptionTypedef *LL_Streng_EncryptionStruct)
{
  uint32_t t;

  if (id >= ATON_STRENG_NUM)
    return LL_ATON_INVALID_ID;

  t = LL_Streng_EncryptionStruct->encryption_id & 0xffffffff;
  ATON_STRENG_ENCR_LSB_SET(id, t);

  t = ATON_STRENG_ENCR_MSB_DT;
  t = ATON_STRENG_ENCR_MSB_SET_ID_MSB(t, (LL_Streng_EncryptionStruct->encryption_id >> 32) & 0x7ff);
  t = ATON_STRENG_ENCR_MSB_SET_EN(t, LL_Streng_EncryptionStruct->enable);
  t = ATON_STRENG_ENCR_MSB_SET_ROUNDS(t, LL_Streng_EncryptionStruct->rounds);
  t = ATON_STRENG_ENCR_MSB_SET_KEY_SEL(t, LL_Streng_EncryptionStruct->key_sel);
  t = ATON_STRENG_ENCR_MSB_SET_INC(t, LL_Streng_EncryptionStruct->increment);
  ATON_STRENG_ENCR_MSB_SET(id, t);

  return 0;
}

int LL_Streng_WeightEncryptionInit(int id)
{
  LL_Streng_EncryptionTypedef LL_Streng_EncryptionStruct = {
      .enable = 1,
      .encryption_id = DEFAULT_WEIGHT_ENCRYPTION_ID,
      .rounds = DEFAULT_WEIGHT_ENCRYPTION_ROUNDS,
      .key_sel = DEFAULT_WEIGHT_KEY_SEL,
      .increment = 0,
  };
  return LL_Streng_EncryptionInit(id, &LL_Streng_EncryptionStruct);
}

/**
 * @brief Configures Bus Interface encryption keys
 * @param id, Bus Interface identifier [0..ATON_BUSIF_NUM-1]
 * @param key Selects key to configure [0..1]
 * @param key_low lowest 64 bits of the key
 * @param key_hi highest 64 bits of the key
 */
int LL_Busif_SetKeys(int id, int key, uint64_t key_low, uint64_t key_hi)
{
  if (id >= ATON_BUSIF_NUM)
    return LL_ATON_INVALID_ID;

  if (key == 0)
  {
    ATON_BUSIF_KEY_SET(id, ATON_BUSIF_0_KEY0_31_0_IDX, ATON_BUSIF_0_KEY0_31_0_S, key_low & 0xffffffff);
    ATON_BUSIF_KEY_SET(id, ATON_BUSIF_0_KEY0_63_32_IDX, ATON_BUSIF_0_KEY0_63_32_S, key_low >> 32);
    ATON_BUSIF_KEY_SET(id, ATON_BUSIF_0_KEY0_95_64_IDX, ATON_BUSIF_0_KEY0_95_64_S, key_hi & 0xffffffff);
    ATON_BUSIF_KEY_SET(id, ATON_BUSIF_0_KEY0_127_96_IDX, ATON_BUSIF_0_KEY0_127_96_S, key_hi >> 32);
  }
  else if (key == 1)
  {
    ATON_BUSIF_KEY_SET(id, ATON_BUSIF_0_KEY1_31_0_IDX, ATON_BUSIF_0_KEY1_31_0_S, key_low & 0xffffffff);
    ATON_BUSIF_KEY_SET(id, ATON_BUSIF_0_KEY1_63_32_IDX, ATON_BUSIF_0_KEY1_63_32_S, key_low >> 32);
    ATON_BUSIF_KEY_SET(id, ATON_BUSIF_0_KEY1_95_64_IDX, ATON_BUSIF_0_KEY1_95_64_S, key_hi & 0xffffffff);
    ATON_BUSIF_KEY_SET(id, ATON_BUSIF_0_KEY1_127_96_IDX, ATON_BUSIF_0_KEY1_127_96_S, key_hi >> 32);
  }
  else
  {
    return LL_ATON_INVALID_ID;
  }

  return 0;
}

/**
 * @brief Configures Epoch Controller Encryption support
 * @param id Epoch Controller identifier [0..ATON_EPOCHCTRL_NUM-1]
 * @param conf Pointer to LL_Streng_EncryptionTypedef structure describing Encryption parameters
 * @retval Error code
 * @note Encryption configuration structure is the same as the Stream Engine one
 * @note Encryption keys are set in the Epoch Controller's Bus Interface using LL_Busif_SetKeys function
 * @note Epoch Controller Bus Interface ID is ATON_BUSIF_NUM-1
 */
int LL_EpochCtrl_EncryptionInit(int id, LL_Streng_EncryptionTypedef *conf)
{
  uint32_t t;

  if (id >= ATON_EPOCHCTRL_NUM)
    return LL_ATON_INVALID_ID;

  t = conf->encryption_id & 0xffffffff;
  ATON_EPOCHCTRL_ENCR_LSB_SET(id, t);

  t = ATON_EPOCHCTRL_ENCR_MSB_DT;
  t = ATON_EPOCHCTRL_ENCR_MSB_SET_ID_MSB(t, (conf->encryption_id >> 32) & 0x7ff);
  t = ATON_EPOCHCTRL_ENCR_MSB_SET_EN(t, conf->enable);
  t = ATON_EPOCHCTRL_ENCR_MSB_SET_ROUNDS(t, conf->rounds);
  t = ATON_EPOCHCTRL_ENCR_MSB_SET_KEY_SEL(t, conf->key_sel);
  ATON_EPOCHCTRL_ENCR_MSB_SET(id, t);

  return 0;
}

/**
 * @brief  Handles Dma/Cypher data transfer
 * @param  cypherInfo: transfer parameters (addresses, keys, ... )
 * @retval Error code (0: og - -1: ko)
 * @note   The function uses stream engines 0 and 1
 * @note   It uses default (0) values for encryption id and round (12)
 */

int LL_DmaCypherInit(LL_Cypher_InitTypeDef *cypherInfo)
{
  uint32_t limit;
  uint32_t lastAdd;

  if (cypherInfo->cypherCacheMask != 0 && cypherInfo->len > CYPHER_CACHE_SIZE)
  {
    return (-1);
  }

  LL_ATON_Init();

  /* Use stream engine 0 as source channel */

  lastAdd = cypherInfo->srcAdd + cypherInfo->len;
  limit = (uint32_t)cypherInfo->len + (8 - (lastAdd & 7));
  limit += (8 - (limit & 7));

  LL_Streng_TensorInitTypeDef dma_in = {.dir = 0,
                                        .addr_base.i = (uint32_t)cypherInfo->srcAdd,
                                        .offset_start = 0,
                                        .offset_end = (uint32_t)cypherInfo->len,
                                        .offset_limit = limit,
                                        .raw = 1,
                                        .frame_tot_cnt = 1,
                                        .nbits_in = 8,
                                        .nbits_out = 8,
                                        .nbits_unsigned = 0};

  /* Use stream engine 0 as destination channel */

  lastAdd = cypherInfo->dstAdd + cypherInfo->len;
  limit = (uint32_t)cypherInfo->len + (8 - (lastAdd & 7));
  limit += (8 - (limit & 7));

  LL_Streng_TensorInitTypeDef dma_out = {.dir = 1,
                                         .addr_base.i = (uint32_t)cypherInfo->dstAdd,
                                         .offset_start = 0,
                                         .offset_end = (uint32_t)cypherInfo->len,
                                         .offset_limit = limit,
                                         .raw = 1,
                                         .frame_tot_cnt = 1,
                                         .nbits_in = 8,
                                         .nbits_out = 8,
                                         .nbits_unsigned = 0};

  if (0 != (cypherInfo->cypherCacheMask & CYPHER_CACHE_SRC))
  {
    dma_in.cacheable = 1;
    dma_in.cache_allocate = 1;
  }
  else
  {
    dma_in.cacheable = 0;
    dma_in.cache_allocate = 0;
  }

  if (0 != (cypherInfo->cypherCacheMask & CYPHER_CACHE_DST))
  {
    dma_out.cacheable = 1;
    dma_out.cache_allocate = 1;
  }
  else
  {
    dma_out.cacheable = 0;
    dma_out.cache_allocate = 0;
  }

  /* Configure stream switch */

  const LL_Switch_InitTypeDef switch_init = {LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0),
                                             LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0),
                                             LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0};

  /* Initialize stream engines */

  const LL_ATON_EnableUnits_InitTypeDef dma_units[] = {{{STRENG, 1}}, {{STRENG, 0}}};

  if (cypherInfo->cypherEnableMask != 0)
  {
    if ((cypherInfo->cypherEnableMask & CYPHER_SRC_MASK) != 0)
    {
      /* Enable encryption on input stream engine */
      dma_in.cipher_en = 1;
    }
    else if ((cypherInfo->cypherEnableMask & CYPHER_DST_MASK) != 0)
    {
      /* Enable encryption on output stream engine */

      dma_out.cipher_en = 1;
    }
    else
    {
      return (-1);
    }

    /* Set encryption keys into Bus Interafce */

    LL_Busif_SetKeys(0, 0, cypherInfo->busIfKeyLsb, cypherInfo->busIfKeyMsb);
  }

  LL_Streng_TensorInit(CYPHER_SRC_STRENG_ID, &dma_in, 1);
  LL_Streng_TensorInit(CYPHER_DST_STRENG_ID, &dma_out, 1);

  /* Enable stream switch */

  LL_Switch_Init(&switch_init, 1);

  /* Start */

  LL_ATON_EnableUnits_Init(dma_units, 2);

  /* Wait for end of transfer */

  const uint32_t dma_wait_mask = 1 << CYPHER_DST_STRENG_ID; // Wait for destination stream engine

  LL_Streng_Wait(dma_wait_mask);

  /* Disabkle and deinit */

  LL_ATON_DisableUnits_Init(dma_units, 1);

  LL_Switch_Deinit(&switch_init, 1);

  return (0);
}

#endif //(ATON_STRENG_VERSION_ENCR_DT == 1)
