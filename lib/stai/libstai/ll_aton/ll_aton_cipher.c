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

#if (LL_ATON_PLATFORM != LL_ATON_PLAT_EC_TRACE)

#include "ll_aton_runtime.h"

#endif

#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)

/**
 * @brief Construct the patch identifier given Stream Engine index and the information about the register (ENCR_LSB /
 * ENCR_MSB) that must be patched.
 * @param patch_id array containing the patch identifier (that must be already allocated and large enough); this array
 * will be filled by this function
 * @param streng_idx Streaming engine index [0..ATON_STRENG_NUM-1]
 * @param lsb_sb is \e true if the ENCR_LSB register must be patched, \e false if the ENCR_MSB register must be patched
 * grouped
 */
void construct_patch_id_streng(char *patch_id, int streng_idx, bool lsb_msb)
{
  patch_id[0] = 'S';
  patch_id[1] = 'E';
  patch_id[2] = '_';
  patch_id[3] = '0' + (streng_idx / 10);
  patch_id[4] = '0' + (streng_idx % 10);
  patch_id[5] = '_';
  patch_id[6] = (lsb_msb ? 'L' : 'M');
  patch_id[7] = 'S';
  patch_id[8] = 'B';
  patch_id[9] = '\0';
}

#endif // #if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)

/**
 * @brief Configures the Streaming Engine Encryption support
 * @param id Streaming engine identifier [0..ATON_STRENG_NUM-1]
 * @param LL_Streng_EncryptionStruct Pointer to structure describing encryption parameters
 * @note Encryption keys are set in the Bus Interfaces using LL_Busif_SetKeys function
 * @note Each Stream Engine is associated to a Bus Interface. Machine descriptor json file describes how they are
 * grouped
 */
int LL_Streng_EncryptionInit(int id, const LL_Streng_EncryptionTypedef *LL_Streng_EncryptionStruct)
{
  uint32_t t;

  if ((id >= ATON_STRENG_NUM) || (!ATON_STRENG_ENCRYPTION_EN(id)))
    return LL_ATON_INVALID_ID;

  t = LL_Streng_EncryptionStruct->encryption_id & 0xffffffff;

#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
  ec_trace_add_patch("SE_ENCRYPTION_ID", 0, 0xffffffff);
#endif

  ATON_STRENG_ENCR_LSB_SET(id, t);

  t = ATON_STRENG_ENCR_MSB_DT;
  t = ATON_STRENG_ENCR_MSB_SET_ID_MSB(t, (LL_Streng_EncryptionStruct->encryption_id >> 32) & 0x7ff);
  t = ATON_STRENG_ENCR_MSB_SET_EN(t, LL_Streng_EncryptionStruct->enable);
  t = ATON_STRENG_ENCR_MSB_SET_ROUNDS(t, LL_Streng_EncryptionStruct->rounds);
  t = ATON_STRENG_ENCR_MSB_SET_KEY_SEL(t, LL_Streng_EncryptionStruct->key_sel);
  t = ATON_STRENG_ENCR_MSB_SET_INC(t, LL_Streng_EncryptionStruct->increment);

#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
  ec_trace_add_patch("SE_ENCRYPTION_ID", 32, ATON_STRENG_ENCR_MSB_ID_MSB_MASK >> ATON_STRENG_ENCR_MSB_ID_MSB_LSB);
  ec_trace_add_patch("SE_ROUNDS", -(int32_t)ATON_STRENG_ENCR_MSB_ROUNDS_LSB,
                     ATON_STRENG_ENCR_MSB_ROUNDS_MASK >> ATON_STRENG_ENCR_MSB_ROUNDS_LSB);
  ec_trace_add_patch("SE_KEY_SEL", -(int32_t)ATON_STRENG_ENCR_MSB_KEY_SEL_LSB,
                     ATON_STRENG_ENCR_MSB_KEY_SEL_MASK >> ATON_STRENG_ENCR_MSB_KEY_SEL_LSB);
#endif

  ATON_STRENG_ENCR_MSB_SET(id, t);

  return 0;
}

/**
 * @brief Configures Bus Interface encryption keys
 * @param id, Bus Interface identifier [0..ATON_BUSIF_NUM-1]
 * @param key Selects key to configure [0..BUSIF_CRYPT_NR_KEYS-1]
 * @param key_low lowest 64 bits of the key
 * @param key_hi highest 64 bits of the key
 */
int LL_Busif_SetKeys(int id, int key, uint64_t key_low, uint64_t key_hi)
{
#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
  char patch_id[32];
#endif

  if ((id >= ATON_BUSIF_NUM) || (!ATON_BUSIF_ENCRYPTION_EN(id)) || (key >= BUSIF_CRYPT_NR_KEYS))
    return LL_ATON_INVALID_ID;

#if defined(ATON_BUSIF_KEY_SET)
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
#endif // defined(ATON_BUSIF_KEY_SET)

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
int LL_EpochCtrl_EncryptionInit(int id, const LL_Streng_EncryptionTypedef *conf)
{
  uint32_t t;

  if ((id >= ATON_EPOCHCTRL_NUM) || (!ATON_EPOCHCTRL_ENCRYPTION_EN(id)))
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

#if (LL_ATON_PLATFORM != LL_ATON_PLAT_EC_TRACE)

/**
 * @brief  Handles Dma/Cypher data transfer
 * @param  cypherInfo: transfer parameters (addresses, keys, ... )
 * @retval Error code (0: og - -1: ko)
 * @note   WARNING: this is an utility function and its usage is not compatible with the ATON runtime
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

    /* Set encryption keys into Bus Interface */

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

  /* Disable and deinit */

  LL_ATON_DisableUnits_Init(dma_units, 1);

  LL_Switch_Deinit(&switch_init, 1);

  return (0);
}

/**
 * @brief  Handles Dma/Cypher data transfer with extended parameters
 * @param  extCypherInfo: transfer parameters (addresses, keys, ... )
 * @retval Error code (0: ok - -1: ko)
 * @note   WARNING: this is an utility function and its usage is not compatible with the ATON runtime
 * @note   The function uses stream engines 0 and 1
 */

int LL_ExtDmaCypherInit(LL_ExtendedCypher_InitTypeDef *extCypherInfo)
{
  if (extCypherInfo->cypherCacheMask != 0 && extCypherInfo->transferSize > CYPHER_CACHE_SIZE)
  {
    return (-1);
  } /* endif */

  /* Add frames check */

  uint32_t frameNumber;
  uint32_t frameSize;
  uint32_t limit;
  uint32_t lastAdd;

  if (extCypherInfo->incrementStep != 0)
  {
    if ((extCypherInfo->transferSize % extCypherInfo->frameSize) != 0)
    {
      return (-1);
    } /* endif */

    frameSize = extCypherInfo->frameSize;
    frameNumber = extCypherInfo->transferSize / extCypherInfo->frameSize;
  }
  else
  {
    frameSize = extCypherInfo->transferSize;
    frameNumber = 1;
  } /* endif */

  lastAdd = extCypherInfo->srcAdd + extCypherInfo->transferSize;
  limit = (uint32_t)extCypherInfo->transferSize + (8 - (lastAdd & 7));
  limit += (8 - (limit & 7));

  /* Use stream engine 0 as source channel */

  LL_Streng_TensorInitTypeDef dma_in = {.dir = 0,
                                        .addr_base.i = (uint32_t)extCypherInfo->srcAdd,
                                        .offset_start = 0,
                                        .offset_end = frameSize,
                                        .offset_limit = limit,
                                        .raw = 1,
                                        .frame_tot_cnt = frameNumber,
                                        .frame_offset = frameSize,
                                        .nbits_in = 8,
                                        .nbits_out = 8,
                                        .nbits_unsigned = 0};

  /* Use stream engine 1 as destination channel:
     same configuration except the direction, the address and the limit */

  LL_Streng_TensorInitTypeDef dma_out = dma_in;

  lastAdd = extCypherInfo->dstAdd + extCypherInfo->transferSize;
  limit = (uint32_t)extCypherInfo->transferSize + (8 - (lastAdd & 7));
  limit += (8 - (limit & 7));

  dma_out.dir = 1;
  dma_out.addr_base.i = (uint32_t)extCypherInfo->dstAdd;
  dma_out.offset_limit = limit;

  if (0 != (extCypherInfo->cypherCacheMask & CYPHER_CACHE_SRC))
  {
    dma_in.cacheable = 1;
    dma_in.cache_allocate = 1;
  }
  else
  {
    dma_in.cacheable = 0;
    dma_in.cache_allocate = 0;
  } /* endif */

  if (0 != (extCypherInfo->cypherCacheMask & CYPHER_CACHE_DST))
  {
    dma_out.cacheable = 1;
    dma_out.cache_allocate = 1;
  }
  else
  {
    dma_out.cacheable = 0;
    dma_out.cache_allocate = 0;
  } /* endif */

  /* Configure stream switch */

  const LL_Switch_InitTypeDef switch_init = {LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0),
                                             LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0),
                                             LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0};

  /* Initialize stream engines */

  const LL_ATON_EnableUnits_InitTypeDef dma_units[] = {{{STRENG, 1}}, {{STRENG, 0}}};

  if (extCypherInfo->cypherEnableMask != 0)
  {
    if ((extCypherInfo->cypherEnableMask & CYPHER_SRC_MASK) != 0)
    {
      /* Enable encryption on input stream engine */
      dma_in.cipher_en = 1;
    }
    else if ((extCypherInfo->cypherEnableMask & CYPHER_DST_MASK) != 0)
    {
      /* Enable encryption on output stream engine */

      dma_out.cipher_en = 1;
    }
    else
    {
      return (-1);
    } /* endif */

    /* Check parameters */

    if (extCypherInfo->keySel > 1 || extCypherInfo->roundNumber > 1)
    {
      return (-1);
    } /* endif */

  } /* endif */

  /* Stream engines configuration */

  LL_Streng_TensorInit(CYPHER_SRC_STRENG_ID, &dma_in, 1);
  LL_Streng_TensorInit(CYPHER_DST_STRENG_ID, &dma_out, 1);

  /* Encryption setup */

  if (dma_in.cipher_en == 1 || dma_out.cipher_en == 1)
  {
    LL_Streng_EncryptionTypedef dmaCypherInfo;

    dmaCypherInfo.enable = 1;
    dmaCypherInfo.encryption_id = extCypherInfo->encryptionId;
    dmaCypherInfo.key_sel = extCypherInfo->keySel;
    dmaCypherInfo.rounds = extCypherInfo->roundNumber;
    dmaCypherInfo.increment = extCypherInfo->incrementStep;

    if (dma_in.cipher_en == 1)
    {
      LL_Streng_EncryptionInit(CYPHER_SRC_STRENG_ID, &dmaCypherInfo);
    }
    else
    {
      LL_Streng_EncryptionInit(CYPHER_DST_STRENG_ID, &dmaCypherInfo);
    } /* endif */

  } /* endif */

  /* Enable stream switch */

  LL_Switch_Init(&switch_init, 1);

  /* Start */

  LL_ATON_EnableUnits_Init(dma_units, 2);

  /* Set wait mask for destination engine */

  const uint32_t dma_wait_mask = 1 << CYPHER_DST_STRENG_ID;

  /* Wait for end of transfer */

  LL_Streng_Wait(dma_wait_mask);

  /* Clear interrupt at stream engine level */

  uint32_t strengIrqs = ATON_STRENG_IRQ_GET(CYPHER_DST_STRENG_ID);
  ATON_STRENG_IRQ_SET(
      CYPHER_DST_STRENG_ID,
      strengIrqs); /* Acknowledge ATON interrupt source (i.e. stream engine #i) - could be more fine grain */

  /* Data Synchronization Barrier */

  LL_ATON_DSB();

  /* Clear interrupt at interrupt controller level */

  ATON_INTCTRL_INTCLR_SET(0, 1 << CYPHER_DST_STRENG_ID);

  LL_ATON_DSB();

  /* Disable and deinit */

  LL_ATON_DisableUnits_Init(dma_units, 2);

  LL_Switch_Deinit(&switch_init, 1);

  return (0);
}

#define STRENG_WEIGHT_ENCRYPTION_ROUNDS_12 0
/**
 * @brief  Handles Dma/Cypher data transfer with extended parameters
 * @param  encr_params = encryption parameters
 * @param  dst = transfer destination address
 * @param  src = transfer source address
 * @param  size = bytes to transfer
 * @retval None
 * @note   WARNING: this is an utility function and its usage is not compatible with the ATON runtime
 */

void LL_ATON_DecryptAndCopy(const LL_Streng_EncryptionTypedef *encr_params, uint64_t *dst, const uint64_t *src,
                            uint32_t size)
{
  LL_ExtendedCypher_InitTypeDef extCypherInfo;

  /* Fill the data structure */

  extCypherInfo.srcAdd = (uint32_t)(uintptr_t)src;
  extCypherInfo.dstAdd = (uint32_t)(uintptr_t)dst;
  extCypherInfo.transferSize = size;
  extCypherInfo.cypherCacheMask = CYPHER_CACHE_NONE; /* Do not use cache */
  extCypherInfo.cypherEnableMask = CYPHER_SRC_MASK;  /* Decrypt using source address */

  /* Set default values */

  extCypherInfo.encryptionId = encr_params->encryption_id;
  extCypherInfo.frameSize = size;
  extCypherInfo.keySel = encr_params->key_sel;
  extCypherInfo.roundNumber = encr_params->rounds;
  extCypherInfo.incrementStep = encr_params->increment;

  /* Decrypt and copy in a fake instance environment */

  __ll_set_aton_owner((NN_Instance_TypeDef *)0xFFFFFFFF);
  LL_ExtDmaCypherInit(&extCypherInfo);
  __ll_clear_aton_owner((NN_Instance_TypeDef *)0xFFFFFFFF, false);
}

#endif // #if (LL_ATON_PLATFORM != LL_ATON_PLAT_EC_TRACE)
