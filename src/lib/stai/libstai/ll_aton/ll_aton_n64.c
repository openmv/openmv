/**
 ******************************************************************************
 * @file    ll_aton.c
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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "ll_aton.h"
#include "ll_aton_NN_interface.h"
#include "ll_aton_util.h"

#ifdef ATON_STRENG64_NUM

extern int startWatchdog(uint32_t timeout);
extern int checkWatchdog(void);

/**
 * @brief  Waits for 64-bits streaming engine(s) to become idle
 * @param  id Array of DMA identifier each entry in [0-5]
 * @param  n  Number of entries in id array in [1-6]
 * @retval Error code
 */
int LL_Streng64_Wait(uint32_t mask)
{
  int i;
  uint32_t enableFlags;

  startWatchdog(ATON_EPOCH_TIMEOUT);

  do
  {
    enableFlags = 0;
    for (i = 0; i < ATON_STRENG64_NUM; i++)
    {
      if (mask & (1 << i))
      {
        enableFlags |= (ATON_STRENG64_CTRL_GET(i) & (1U << ATON_STRENG64_CTRL_RUNNING_LSB));
      }
    }

    LL_ATON_ASSERT(checkWatchdog() == 0);

  } while (enableFlags);

  return LL_ATON_OK;
}

/**
 * @brief  Configures 64-bits streaming engine
 * @param  id Streaming engine identifier [0..ATON_STRENG64_NUM-1]
 * @param  conf Pointer to structure(s) describing initialization parameters
 * @param  n Number of elements in initialization structure array
 * @retval error code. E.g.: Invalid ID, invalid parameters, not idle,..
 */
int LL_Streng64_TensorInit(int id, const LL_Streng_TensorInitTypeDef *conf, int n)
{
  uint32_t t;

  /* deferred register values */
  uint32_t t_streng_strd = ATON_STRENG64_STRD_DT;
  uint32_t t_streng_cid_cache = ATON_STRENG64_CID_CACHE_DT;
  uint32_t t_streng_event = ATON_STRENG64_EVENT_DT;

  if (id >= ATON_STRENG64_NUM)
    return LL_ATON_INVALID_ID;

#define _LL_min(x, y) ((x) > (y) ? (y) : (x))

  if (n != 1)
    return -1;
    // if (conf->dir == 0 && (conf->nbits_in > conf->nbits_out)) return -1;
#ifndef ATON_IMC_NUM
  if (conf->nbits_in > 64 || conf->nbits_out > 64)
    return -1;
#endif

  t = ATON_STRENG64_CTRL_DT;
  t = ATON_STRENG64_CTRL_SET_DIR(t, (conf->dir != 0));
  t = ATON_STRENG64_CTRL_SET_RAW(t, (conf->raw != 0));
#ifdef ATON_STRENG64_CTRL_SET_RAW_OUT
  t = ATON_STRENG64_CTRL_SET_RAW_OUT(t, conf->raw_out);
#else
  /* Hardware feature not supported */
  LL_ATON_ASSERT(conf->raw_out == 0);
#endif
  t = ATON_STRENG64_CTRL_SET_NOBLK(t, (conf->noblk != 0));
  t = ATON_STRENG64_CTRL_SET_NOINC(t, (conf->noinc == 1));
  t = ATON_STRENG64_CTRL_SET_SINGLE(t, conf->frame_tot_cnt == 1);
  t = ATON_STRENG64_CTRL_SET_CONT(t, conf->continuous == 1);
  t = ATON_STRENG64_CTRL_SET_LSBMODE(t, conf->align_right == 1);

  // FIXME remove this LL_ATON_ASSERT when this function supports unsigned types
  // LL_ATON_ASSERT(conf->nbits_unsigned == 0 || conf->nbits_out == conf->nbits_in);
  // t = ATON_STRENG64_CTRL_SET_SIGNEXT(t, conf->nbits_unsigned == 0 && conf->align_right == 1);
  t = ATON_STRENG64_CTRL_SET_SIGNEXT(t, conf->align_right == 1 && conf->nbits_unsigned == 0);

  int ch_bits[3] = {0, 0, 0};
  int in_bits[3];
  int out_bits[3];
  int nbits_out = conf->nbits_out;
  int nbits_in = conf->nbits_in;

  // case 0: bus -> stream --> in_bits > out_bits -> use FRONT_GAP
  //                      |
  // case 1:              --> in_bits <= out_bits -> use lanes but valid for out_bits=8,16,24 for other bit length out
  // is right shifted by (8,16,24) - out_bits
  //
  // case 2: stream->bus  --> in_bits >= out_bits -> use lanes but valid for in_bits=8,16,24 for other bit length out is
  // right shifted by (8,16,24) - out_bits
  //                      |
  // case 3:              --> in_bits < out_bits -> use FRONT_GAP

  int io_case = ((conf->dir != 0) << 1);
  io_case += (conf->dir == 0 ? (conf->nbits_in <= conf->nbits_out) : (conf->nbits_in < conf->nbits_out));

  switch (io_case)
  {
  case 0: // in_bits > out_bits && bus->stream
          // must use FRONT_GAP
  {
    uint32_t tgap = 0;
    if (conf->mem_lsb)
      tgap = ATON_STRENG64_STRD_SET_FGAP(0, (nbits_in - nbits_out));
    else
      tgap = ATON_STRENG64_STRD_SET_BGAP(0, (nbits_in - nbits_out));
    t_streng_strd = tgap;
    nbits_in = nbits_out;
  }
  // intentional fall trough !!!
  case 1: // in_bits <= out_bits && bus->stream
    in_bits[0] = _LL_min(8, nbits_in);
    in_bits[1] = nbits_in > 8 ? _LL_min(8, nbits_in - 8) : 0;
    in_bits[2] = nbits_in > 16 ? _LL_min(8, nbits_in - 16) : 0;
    if (conf->align_right)
    {
      ch_bits[0] = in_bits[0];
      ch_bits[1] = in_bits[1];
      ch_bits[2] = in_bits[2];
    }
    else
    {
      if (nbits_out > 16)
      {
        ch_bits[2] = in_bits[0];
        ch_bits[1] = in_bits[1];
        ch_bits[0] = in_bits[2];
      }
      else if (nbits_out > 8)
      {
        ch_bits[1] = in_bits[0];
        ch_bits[0] = in_bits[1];
      } // N.B. the DMA stuffs the bits to the left of the channel
      else
        ch_bits[0] = in_bits[0];
    }
    break;
  case 3: // in_bits < out_bits && stream->bus
  {
    uint32_t tgap = 0;
    if (conf->mem_lsb)
      tgap = ATON_STRENG64_STRD_SET_FGAP(0, (nbits_out - nbits_in));
    else
      tgap = ATON_STRENG64_STRD_SET_BGAP(0, (nbits_out - nbits_in));
    t_streng_strd = tgap;
    nbits_out = nbits_in;
  }
  // intentional fall trough !!!
  case 2: // in_bits >= out_bits && stream->bus
    out_bits[0] = _LL_min(8, nbits_out);
    out_bits[1] = nbits_out > 8 ? _LL_min(8, nbits_out - 8) : 0;
    out_bits[2] = nbits_out > 16 ? _LL_min(8, nbits_out - 16) : 0;
    if (conf->align_right)
    {
      ch_bits[0] = out_bits[0];
      ch_bits[1] = out_bits[1];
      ch_bits[2] = out_bits[2];
    }
    else
    {
      if (nbits_in > 16)
      {
        ch_bits[2] = out_bits[0];
        ch_bits[1] = out_bits[1];
        ch_bits[0] = out_bits[2];
      }
      else if (nbits_in > 8)
      {
        ch_bits[1] = out_bits[0];
        ch_bits[0] = out_bits[1];
      } // N.B. the DMA stuffs the bits to the left of the channel
      else
        ch_bits[0] = out_bits[0];
    }
    break;
  }

/* Take care of N64 single size register. TODO: double check this  */
#ifdef ATON_STRENG64_CTRL_SET_SIZE1
  t = ATON_STRENG64_CTRL_SET_SIZE0(t, ch_bits[0]);
  t = ATON_STRENG64_CTRL_SET_SIZE1(t, ch_bits[1]);
  t = ATON_STRENG64_CTRL_SET_SIZE2(t, ch_bits[2]);
#else
  if (conf->dir)
    t = ATON_STRENG64_CTRL_SET_SIZE0(t, conf->nbits_out);
  else
    t = ATON_STRENG64_CTRL_SET_SIZE0(t, conf->nbits_in);
#endif

  ATON_STRENG64_CTRL_SET(id, t);

  // ATON_STRENG64_ADDR_SET(id, conf->addr_start.i);
  LL_ATON_REG_WRITE_RELOC(((volatile uint32_t *)(uintptr_t)ATON_STRENG64_ADDR_ADDR(id)), conf->addr_base.i,
                          conf->offset_start);

  if (conf->raw)
  {
    if (conf->frame_count)
      t = conf->frame_count;
    else
    {
      // N.B. end - start must contain padding if nbits_in is not power of two
      t = (LL_Streng_len(conf) * 8) / (conf->dir == 0 ? conf->nbits_in : conf->nbits_out);
    }
    ATON_STRENG64_FSIZE_SET(id, t);
  }
  else
  {
    t = ATON_STRENG64_FSIZE_SET_WIDTH(0, conf->fwidth);
    t = ATON_STRENG64_FSIZE_SET_HEIGHT(t, conf->fheight);
    ATON_STRENG64_FSIZE_SET(id, t);

    uint32_t line_offset = conf->line_offset == 0 ? conf->fwidth * conf->batch_offset : conf->line_offset;
    // if line_offset is left=0 then it's computed from the standard geometry of lines and batch
    t = t_streng_strd;
    t = ATON_STRENG64_STRD_SET_LOFF(t, line_offset);
    t_streng_strd = t;

#if defined(ATON_STRENG64_CID_CACHE_SET_LOFF_MSB)
    t = t_streng_cid_cache;
    t = ATON_STRENG64_CID_CACHE_SET_LOFF_MSB(t, (line_offset >> ATON_STRENG64_STRD_LOFF_W));
    t_streng_cid_cache = t;
#endif

    t = ATON_STRENG64_DEPTH_SET_SIZE(0, conf->batch_depth);
    t = ATON_STRENG64_DEPTH_SET_OFFSET(t, conf->batch_offset);
    ATON_STRENG64_DEPTH_SET(id, t);
  }

  ATON_STRENG64_FRPTOFF_SET(id, conf->loop_offset);
  ATON_STRENG64_FRAME_RPT_SET(id, conf->frame_loop_cnt);
  ATON_STRENG64_FOFFSET_SET(id, conf->frame_offset);

  t = ATON_STRENG64_LIMITEN_SET_FRAMELIMIT(0, 1); // all other fields set to zero
#if defined(ATON_STRENG64_LIMITEN_SET_DOFF_MSB)
  t = ATON_STRENG64_LIMITEN_SET_DOFF_MSB(t, conf->batch_offset >> ATON_STRENG64_DEPTH_OFFSET_W);
#endif
  ATON_STRENG64_LIMITEN_SET(id, t);

  if (/*(conf->dir == 0) && */ (conf->offset_limit != 0x0))
  {
    t = ATON_STRENG64_LIMITEN_SET_ADDRLIMIT(t, 1);
    t = ATON_STRENG64_LIMITEN_SET_STOPPREFTC(t, 1);
    ATON_STRENG64_LIMITEN_SET(id, t);
    // NOTE: limiter is to be set to last accessible byte address
    // ATON_STRENG64_LIMITADDR_SET(id, (conf->addr_limit.i - 1));
    LL_ATON_REG_WRITE_RELOC(((volatile uint32_t *)(uintptr_t)ATON_STRENG64_LIMITADDR_ADDR(id)), conf->addr_base.i,
                            conf->offset_limit - 1);
  }

  t = ATON_STRENG64_LIMIT_SET_CNT(0, conf->frame_tot_cnt);
  ATON_STRENG64_LIMIT_SET(id, t);
  // LL_ATON_PRINTF("frame_tot_cnt=%d\n", conf->frame_tot_cnt);

#if defined(ATON_STRENG64_CID_CACHE_SET_CID)
  t = t_streng_cid_cache;
  t = ATON_STRENG64_CID_CACHE_SET_CID(t, conf->bus_cid);
  t = ATON_STRENG64_CID_CACHE_SET_CACHEABLE(t, conf->cacheable);
  t = ATON_STRENG64_CID_CACHE_SET_ALLOC(t, conf->cache_allocate);
  t = ATON_STRENG64_CID_CACHE_SET_PFETCH(t, conf->bus_pfetch);
  t = ATON_STRENG64_CID_CACHE_SET_LINESIZE(t, conf->cache_linesize);
  t_streng_cid_cache = t;
#endif

  /* Enable event interrupts */
  if (conf->dir == 1)
  {
#if LL_ATON_EN_EVENT_IRQ
    t = ATON_STRENG64_EVENT_SET_EN_OFLOW_FRM(0, 1); // enable frame overflow interrupt
    t_streng_event = t;
#if 0
    t = ATON_STRENG64_EVENT_SET_EN_OFLOW_ADD(0,1); // enable address limiter interrupt
    t_streng_event = t;
#endif
#endif // LL_ATON_EN_EVENT_IRQ
  }

  /* Enable illegal configuration interrupts */
#if LL_ATON_EN_ERROR_IRQ
  t = t_streng_event;
  t = ATON_STRENG64_EVENT_SET_EN_ILLCFG(t, 1); // Enable Illegal Configuration interrupt
  // t = ATON_STRENG64_EVENT_SET_EN_FMTMM(t, 1); // Enable Format Mismatch interrupt (not enabled intentionally)
  t_streng_event = t;
#endif // LL_ATON_EN_ERROR_IRQ

  if ((conf->dir == 0) && conf->sync_with_other)
  {
    t = t_streng_event;
    t = ATON_STRENG64_EVENT_SET_FRMTRG_EN(t, 1);               // Enable synchronizations of frames with other dma
    t = ATON_STRENG64_EVENT_SET_FRMTRG_SRC(t, conf->sync_dma); // Enable synchronizations of frames with other dma
    t_streng_event = t;
  }

#if 1
  t = ATON_STRENG64_POS_DT;
  t = ATON_STRENG64_POS_SET_GAPCYCLES(t, 0); // set interline gap cycle to 0, as it should be safe to do so
  ATON_STRENG64_POS_SET(id, t);
#endif

  /* deferred register setting */
  ATON_STRENG64_STRD_SET(id, t_streng_strd);
  ATON_STRENG64_CID_CACHE_SET(id, t_streng_cid_cache);
  ATON_STRENG64_EVENT_SET(id, t_streng_event);

  return 0;
}
#endif // ifdef ATON_STRENG64_NUM

#ifdef ATON_STRSWITCH64_NUM
/**
 * @brief  Connects input and output port(s) on the 64-bits streaming switch (without clearing configuration)
 * @param  LL_Switch_InitStruct Pointer to structure(s) describing ports to be connected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_Switch64_Init_NoReset(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n)
{
  int i;
  volatile uint32_t *reg;
  uint32_t t;
  unsigned int en_shift[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH64_DST_EN0_LSB, ATON_STRSWITCH64_DST_EN1_LSB};
  unsigned int link_shift[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH64_DST_LINK0_LSB, ATON_STRSWITCH64_DST_LINK1_LSB};
  unsigned int fnr_shift[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH64_DST_FNR0_LSB, ATON_STRSWITCH64_DST_FNR1_LSB};
  unsigned int fnr_mask[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH64_DST_FNR0_MASK, ATON_STRSWITCH64_DST_FNR1_MASK};

  /* Enable Switch */
  t = ATON_STRSWITCH64_CTRL_DT;
  t = ATON_STRSWITCH64_CTRL_SET_EN(t, 1);
  ATON_STRSWITCH64_CTRL_SET(0, t);

  for (i = 0; i < n; i++)
  {
    /* Compute target destination configuration register. Todo: use ATON.h macros */
    reg = (uint32_t *)(ATON_STRSWITCH64_BASE(0) + ATONN_DSTPORT_ID(LL_Switch_InitStruct[i].dest));
    t = 0;
    /* Enable Context and create link */
#if ATON_SWITCH_CONTEXT_NUM == 2
    t |= ((LL_Switch_InitStruct[i].context0 != 0) << en_shift[0]);
    t |= (ATONN_SRCPORT_ID(LL_Switch_InitStruct[i].source0) << link_shift[0]);
    t |= ((LL_Switch_InitStruct[i].frames0 << fnr_shift[0]) & fnr_mask[0]);
    t |= ((LL_Switch_InitStruct[i].context1 != 0) << en_shift[1]);
    t |= (ATONN_SRCPORT_ID(LL_Switch_InitStruct[i].source1) << link_shift[1]);
    t |= ((LL_Switch_InitStruct[i].frames1 << fnr_shift[1]) & fnr_mask[1]);
#else
    int c;
    for (c = 0; c < ATON_SWITCH_CONTEXT_NUM; c++)
    {
      t |= ((LL_Switch_InitStruct[i].context[c] != 0) << en_shift[c]);
      t |= (ATONN_SRCPORT_ID(LL_Switch_InitStruct[i].source[c]) << link_shift[c]);
      t |= ((LL_Switch_InitStruct[i].frames[c] << fnr_shift[c]) & fnr_mask[c]);
    }
#endif

    *reg = t;
  }

  return 0;
}

/**
 * @brief  Connects input and output port(s) on the 64-bits streaming switch (clearing configuration at the beginning)
 * @param  LL_Switch_InitStruct Pointer to structure(s) describing ports to be connected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_Switch64_Init(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n)
{
  uint32_t t;

  /* Clear Configuration */
  ATON_DISABLE_CLR_CONFCLR(STRSWITCH64, 0);

  return LL_Switch64_Init_NoReset(LL_Switch_InitStruct, n);
}

/**
 * @brief  Fully disconnect a destination port from source ports on the 64-bits streaming switch
 * @param  LL_Switch_InitStruct Pointer to structure(s) describing ports to be disconnected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_Switch64_Deinit(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n)
{
  int i;
  volatile uint32_t *reg;

  for (i = 0; i < n; i++)
  {
    /* Compute target destination configuration register */
    reg = (uint32_t *)(ATON_STRSWITCH64_BASE(0) + ATONN_DSTPORT_ID(LL_Switch_InitStruct[i].dest));

    /* Disable contexts */
    *reg = 0;
  }

  return 0;
}

#endif // ifdef ATON_STRSWITCH64_NUM
