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
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "ll_aton.h"
#include "ll_aton_caches_interface.h"
#include "ll_aton_util.h"

#define ATONN_CONST_SRCPORT(S, J, U, I, P) ATON_##S##_##J##_LINK_##U##_##I##_##P
#define ATONN_CONST_DSTPORT(S, J, U, I, P) ATON_##S##_DST_OFFSET(J, ATON_##S##_##J##_DST##U##_##I##_##P##_IDX)

#ifdef ATON_CONVACC_NUM
static uint32_t Conv_ctrl_bits[ATON_CONVACC_NUM]; // this is an hack FIXME !!!
#endif

#if (ATON_POOL_VERSION_MAJOR_DT < 1) || ((ATON_POOL_VERSION_MAJOR_DT == 1) && (ATON_POOL_VERSION_MINOR_DT <= 0))
#define POOL_RC14    /* Pooling unit is not yet patched with respect to auto-clearing of `CLR` bit and consistent SW/HW \
                        reset values */
#endif

#define ASSERT_UNITS_VERS_W_MSG(unitname, rtl_unitver)                                                                 \
  do                                                                                                                   \
  {                                                                                                                    \
    rtl_unitver = ATON_##unitname##_VERSION_GET(0);                                                                    \
  } while (rtl_unitver == 0);                                                                                          \
  assertf(ATON_##unitname##_VERSION_GET_TYPE(rtl_unitver) == ATON_##unitname##_VERSION_TYPE_DT &&                      \
              ATON_##unitname##_VERSION_GET_MAJOR(rtl_unitver) == ATON_##unitname##_VERSION_MAJOR_DT &&                \
              ATON_##unitname##_VERSION_GET_MINOR(rtl_unitver) == ATON_##unitname##_VERSION_MINOR_DT,                  \
          0,                                                                                                           \
          "%s unit ver mismatch: RTL:t=%" PRIu32 ",ver=%" PRIu32 ".%" PRIu32 "  ATON.h:t=%" PRIu32 ",ver=%" PRIu32     \
          ".%" PRIu32 "\n\r",                                                                                          \
          #unitname, ATON_##unitname##_VERSION_GET_TYPE(rtl_unitver),                                                  \
          ATON_##unitname##_VERSION_GET_MAJOR(rtl_unitver), ATON_##unitname##_VERSION_GET_MINOR(rtl_unitver),          \
          (uint32_t)ATON_##unitname##_VERSION_TYPE_DT, (uint32_t)ATON_##unitname##_VERSION_MAJOR_DT,                   \
          (uint32_t)ATON_##unitname##_VERSION_MINOR_DT)

/** @defgroup Simple watchdog management functions.
 *  Used to exit from LL_Streng_Wait() in case epoch locks
 * @{
 */

static inline void ll_aton_static_checks(void)
{
  static char done = 0;

  if (done != 0)
    return;
  done = 1;

#define ASSERT_ATONN_SRCPORT(S, J, U, I, P)                                                                            \
  LL_ATON_ASSERT(ATONN_CONST_SRCPORT(S, J, U, I, P) == __atonn_getSrcPortID(S, J, U, I, P))
#define ASSERT_ATONN_DSTPORT(S, J, U, I, P)                                                                            \
  LL_ATON_ASSERT(ATONN_CONST_DSTPORT(S, J, U, I, P) == __atonn_getDstPortID(S, J, U, I, P))

#if ATON_STRENG_NUM > 1
  ASSERT_ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0);
#endif
#if ATON_STRENG_NUM > 2
  ASSERT_ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0);
#endif

#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)

#if ATON_CONVACC_NUM > 1
  ASSERT_ATONN_SRCPORT(STRSWITCH, 0, CONVACC, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, CONVACC, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, CONVACC, 1, 1);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, CONVACC, 1, 2);
#endif

#if ATON_CONVACC_NUM > 2
  ASSERT_ATONN_SRCPORT(STRSWITCH, 0, CONVACC, 2, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, CONVACC, 2, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, CONVACC, 2, 1);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, CONVACC, 2, 2);
#endif

#if ATON_POOL_NUM > 1
  ASSERT_ATONN_SRCPORT(STRSWITCH, 0, POOL, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, POOL, 1, 0);
#endif

#if ATON_ARITH_NUM > 1
  ASSERT_ATONN_SRCPORT(STRSWITCH, 0, ARITH, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, ARITH, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, ARITH, 1, 1);
#endif

#if ATON_ACTIV_NUM > 1
  ASSERT_ATONN_SRCPORT(STRSWITCH, 0, ACTIV, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, ACTIV, 1, 0);
#endif

#if ATON_DECUN_NUM > 1
  ASSERT_ATONN_SRCPORT(STRSWITCH, 0, DECUN, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, DECUN, 1, 0);
  ASSERT_ATONN_DSTPORT(STRSWITCH, 0, DECUN, 1, 1);
#endif

#endif // (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)

#undef ASSERT_ATONN_SRCPORT
#undef ASSERT_ATONN_DSTPORT
}

unsigned char *LL_Address_Physical2Virtual(unsigned char *address)
{
  return ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(address);
}

unsigned char *LL_Address_Virtual2Physical(unsigned char *address)
{
  return ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(address);
}

/**
 * @brief Starts watchdog
 * @param timeout Watchdog timeout in cycles
 * @retval ATON Error code
 */
LL_ATON_WEAK int startWatchdog(uint32_t timeout)
{
  (void)timeout;

  return 0;
}

/**
 * @brief Checkes whether watchdog has expired or not
 * @retval ATON Error code
 */
LL_ATON_WEAK int checkWatchdog(void)
{
  return 0;
}

/**
 * @}
 */

/**
 * @brief  ATON global initialization. Initializes clocks and bus interfaces. Must be called before anything else
 * @retval Always zero
 */
int LL_ATON_Init(void)
{
  uint32_t t;
  int i;

  ll_aton_static_checks();

  /* Clear pipeline */
  t = ATON_CLKCTRL_CTRL_GET(0);
  t = ATON_CLKCTRL_CTRL_SET_CLR(t, 1);
  ATON_CLKCTRL_CTRL_SET(0, t);

  /* Enable all ATON clocks */
  ATON_CLKCTRL_CTRL_SET(0, 1);
  ATON_CLKCTRL_AGATES0_SET(0, 0xffffffff);
  ATON_CLKCTRL_AGATES1_SET(0, 0xffffffff);

#if (LL_ATON_ENABLE_CLOCK_GATING == 1)
#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
  ATON_CLKCTRL_BGATES_SET(0, (1 << ATON_EPOCHCTRL_CLKB_CLK(0)));
#else
  ATON_CLKCTRL_BGATES_SET(0, 0x0);
#endif
#else
  ATON_CLKCTRL_BGATES_SET(0, 0xffffffff);
#endif

#ifdef ATON_CLKCTRL_BGATES1_OFFSET
  ATON_CLKCTRL_BGATES1_SET(0, 0xffffffff);
#endif

  /* Check that RTL and ATON.h match. Only check first unit */
  ASSERT_UNITS_VERS_W_MSG(STRENG, t);

#ifdef ATON_CONVACC_NUM
  ASSERT_UNITS_VERS_W_MSG(CONVACC, t);
#endif

#ifdef ATON_POOL_NUM
  ASSERT_UNITS_VERS_W_MSG(POOL, t);
#endif

#ifdef ATON_ARITH_NUM
  ASSERT_UNITS_VERS_W_MSG(ARITH, t);
#endif

#ifdef ATON_ACTIV_NUM
  ASSERT_UNITS_VERS_W_MSG(ACTIV, t);
#endif

#ifdef ATON_DECUN_NUM
  ASSERT_UNITS_VERS_W_MSG(DECUN, t);
#endif

#ifdef ATON_EPOCHCTRL_VERSION_TYPE_DT
  ASSERT_UNITS_VERS_W_MSG(EPOCHCTRL, t);
#endif

#ifdef ATON_RECBUF_VERSION_TYPE_DT
  ASSERT_UNITS_VERS_W_MSG(RECBUF, t);
#endif

#ifdef ATON_STRENG64_NUM
  ASSERT_UNITS_VERS_W_MSG(STRENG64, t);
#endif

#ifdef ATON_STRSWITCH64_NUM
  ASSERT_UNITS_VERS_W_MSG(STRSWITCH64, t);
#endif

#ifdef ATON_STRSWITCH_VC_NUM
  ASSERT_UNITS_VERS_W_MSG(STRSWITCH_VC, t);
#endif

#ifdef ATON_IMC_NUM
  ASSERT_UNITS_VERS_W_MSG(IMC, t);
#endif

  ASSERT_UNITS_VERS_W_MSG(CLKCTRL, t);

  ASSERT_UNITS_VERS_W_MSG(INTCTRL, t);

  ASSERT_UNITS_VERS_W_MSG(STRSWITCH, t);

  ASSERT_UNITS_VERS_W_MSG(BUSIF, t);

  /* Enable Bus Interfaces */
  for (i = 0; i < ATON_BUSIF_NUM; i++)
  {
    ATON_BUSIF_CTRL_SET(i, 1);
  }

  /* Enable Interrupt Controller */
  ATON_INTCTRL_CTRL_SET(0, 1);

  return 0;
}

/**
 * @brief  ATON global de-initialization. Must be called at the very end
 * @retval Always zero
 */
int LL_ATON_DeInit(void)
{
  int i;

  /* Disable Interrupt Controller */
  ATON_INTCTRL_CTRL_SET(0, 0);

  /* Disable Bus Interfaces */
  for (i = 0; i < ATON_BUSIF_NUM; i++)
  {
    ATON_BUSIF_CTRL_SET(i, 0);
  }

  /* Disable all ATON clocks */
  ATON_CLKCTRL_AGATES0_SET(0, 0);
  ATON_CLKCTRL_AGATES1_SET(0, 0);
  ATON_CLKCTRL_BGATES_SET(0, 0);
#ifdef ATON_CLKCTRL_BGATES1_OFFSET
  ATON_CLKCTRL_BGATES1_SET(0, 0);
#endif
  ATON_CLKCTRL_CTRL_SET(0, 0);

  return 0;
}

/**
 * @brief  Enables a set of ATON units
 * @param  LL_ATON_EnableUnits_InitStruct Array of units to enable
 * @param  n Lenght of the initialization array
 * @retval Error code
 * @todo   Add boundary checks
 */
int LL_ATON_EnableUnits_Init(const LL_ATON_EnableUnits_InitTypeDef *LL_ATON_EnableUnits_InitStruct, int n)
{
  int i;
  enum AccelUnitsType unitType;
  uint32_t unitId;

  for (i = 0; i < n; i++)
  {
    unitType = LL_ATON_EnableUnits_InitStruct[i].unit.unit_type;
    unitId = LL_ATON_EnableUnits_InitStruct[i].unit.unit_num;

    switch (unitType)
    {
    case STRENG:
      ATON_ENABLE(STRENG, unitId);
      break;

#ifdef ATON_STRENG64_NUM
    case STRENG64:
      ATON_ENABLE(STRENG64, unitId);
      break;
#endif

#ifdef ATON_CONVACC_NUM
    case CONVACC:
#if 0
      ATON_ENABLE(CONVACC, unitId);
#else
      ATON_CONVACC_CTRL_SET(unitId, ATON_CONVACC_CTRL_SET_EN(Conv_ctrl_bits[unitId], 1));
#endif
      break;
#endif

#ifdef ATON_DECUN_NUM
    case DECUN:
      ATON_ENABLE(DECUN, unitId);
      break;
#endif

#ifdef ATON_ACTIV_NUM
    case ACTIV:
      ATON_ENABLE(ACTIV, unitId);
      break;
#endif

#ifdef ATON_ARITH_NUM
    case ARITH:
      ATON_ENABLE(ARITH, unitId);
      break;
#endif

#ifdef ATON_POOL_NUM
    case POOL:
#ifdef POOL_RC14
      ATON_POOL_CTRL_SET(unitId, ATON_POOL_CTRL_SET_EN(ATON_POOL_CTRL_GET(unitId), 1));
#else  // !POOL_RC14
      ATON_ENABLE(POOL, unitId);
#endif // !POOL_RC14
      break;
#endif
#ifdef ATON_RECBUF_NUM
    case RECBUF:
      ATON_ENABLE(RECBUF, unitId);
      break;
#endif

    default:
      break;
    }
  }

  return 0;
}

/**
 * @brief  Disables a set of ATON units
 * @param  LL_ATON_DisableUnits_InitTypeDef Array of units to disable
 * @param  n Length of the initialization array
 * @retval Error code
 */
int LL_ATON_DisableUnits_Init(const LL_ATON_DisableUnits_InitTypeDef *LL_ATON_DisableUnits_InitStruct, int n)
{
  int i;
  enum AccelUnitsType unitType;
  uint32_t unitId;
  uint32_t t;

  for (i = 0; i < n; i++)
  {
    unitType = LL_ATON_DisableUnits_InitStruct[i].unit.unit_type;
    unitId = LL_ATON_DisableUnits_InitStruct[i].unit.unit_num;

    switch (unitType)
    {
#ifdef ATON_STRENG_NUM
    case STRENG:
      ATON_DISABLE_CLR_CONFCLR(STRENG, unitId);
      LL_ATON_DisableClock(ATON_STRENG_CLKB_CLK(unitId));
      break;
#endif

#ifdef ATON_CONVACC_NUM
    case CONVACC:
      ATON_DISABLE_CLR_CONFCLR(CONVACC, unitId);
      LL_ATON_DisableClock(ATON_CONVACC_CLKB_CLK(unitId));
      break;
#endif

#ifdef ATON_DECUN_NUM
    case DECUN:
      ATON_DISABLE_CLR_CONFCLR(DECUN, unitId);
      LL_ATON_DisableClock(ATON_DECUN_CLKB_CLK(unitId));
      break;
#endif

#ifdef ATON_ACTIV_NUM
    case ACTIV:
      ATON_DISABLE_CLR_CONFCLR(ACTIV, unitId);
      LL_ATON_DisableClock(ATON_ACTIV_CLKB_CLK(unitId));
      break;
#endif

#ifdef ATON_ARITH_NUM
    case ARITH:
      ATON_DISABLE_CLR_CONFCLR(ARITH, unitId);
      LL_ATON_DisableClock(ATON_ARITH_CLKB_CLK(unitId));
      break;
#endif

#ifdef ATON_POOL_NUM
    case POOL:
#ifdef POOL_RC14
      ATON_POOL_CTRL_SET(unitId, ATON_POOL_CTRL_SET_EN(ATON_POOL_CTRL_GET(unitId), 0));
#else  // !POOL_RC14
      ATON_DISABLE_CLR_CONFCLR(POOL, unitId);
#endif // !POOL_RC14

      LL_ATON_DisableClock(ATON_POOL_CLKB_CLK(unitId));
      break;
#endif

#ifdef ATON_RECBUF_NUM
    case RECBUF:
      ATON_DISABLE_CLR_CONFCLR(RECBUF, unitId);
      LL_ATON_DisableClock(ATON_RECBUF_CLKB_CLK(unitId));
      break;
#endif

    default:
      return LL_ATON_INVALID_PARAM;
    }
  }

  return LL_ATON_OK;
}

/**
 * @brief  Waits for streaming engine(s) to become idle
 * @param  mask Bitmask of DMA identifiers
 * @retval Error code
 */
int LL_Streng_Wait(uint32_t mask)
{
  int i;
  uint32_t enableFlags;

  startWatchdog(ATON_EPOCH_TIMEOUT);

  do
  {
    enableFlags = 0;
    for (i = 0; i < ATON_STRENG_NUM; i++)
    {
      if (mask & (1 << i))
      {
        enableFlags |= (ATON_STRENG_CTRL_GET(i) & (1U << ATON_STRENG_CTRL_RUNNING_LSB));
      }
    }

    LL_ATON_ASSERT(checkWatchdog() == 0);

  } while (enableFlags);

  return LL_ATON_OK;
}

/**
 * @brief  Configures streaming engine
 * @param  id Streaming engine identifier [0..ATON_STRENG_NUM-1]
 * @param  conf Pointer to structure(s) describing initialization parameters
 * @param  n Number of elements in initialization structure array
 * @retval error code. E.g.: Invalid ID, invalid parameters, not idle,..
 */
int LL_Streng_TensorInit(int id, const LL_Streng_TensorInitTypeDef *conf, int n)
{
  uint32_t t;

  /* deferred register values */
  uint32_t t_streng_strd = ATON_STRENG_STRD_DT;
  uint32_t t_streng_cid_cache = ATON_STRENG_CID_CACHE_DT;
  uint32_t t_streng_event = ATON_STRENG_EVENT_DT;

  if (id >= ATON_STRENG_NUM)
    return LL_ATON_INVALID_ID;

  LL_ATON_EnableClock(ATON_STRENG_CLKB_CLK(id));

#define _LL_min(x, y) ((x) > (y) ? (y) : (x))

  if (n != 1)
    return -1;
    // if (conf->dir == 0 && (conf->nbits_in > conf->nbits_out)) return -1;
#ifndef ATON_IMC_NUM
  if (conf->nbits_in > 24 || conf->nbits_out > 24)
    return -1;
#endif

  t = ATON_STRENG_CTRL_DT;
  t = ATON_STRENG_CTRL_SET_DIR(t, (conf->dir != 0));
  t = ATON_STRENG_CTRL_SET_RAW(t, (conf->raw != 0));
#ifdef ATON_STRENG_CTRL_SET_RAW_OUT
  t = ATON_STRENG_CTRL_SET_RAW_OUT(t, conf->raw_out);
#else
  /* Hardware feature not supported */
  LL_ATON_ASSERT(conf->raw_out == 0);
#endif
  t = ATON_STRENG_CTRL_SET_NOBLK(t, (conf->noblk != 0));
  t = ATON_STRENG_CTRL_SET_NOINC(t, (conf->noinc == 1));
  t = ATON_STRENG_CTRL_SET_SINGLE(t, conf->frame_tot_cnt == 1);
  t = ATON_STRENG_CTRL_SET_CONT(t, conf->continuous == 1);
  t = ATON_STRENG_CTRL_SET_LSBMODE(t, conf->align_right == 1);
  t = ATON_STRENG_CTRL_SET_SIGNEXT(t, conf->align_right == 1 && conf->nbits_unsigned == 0);

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
    if (conf->mem_lsb)
      t_streng_strd = ATON_STRENG_STRD_SET_FGAP(t_streng_strd, (nbits_in - nbits_out));
    else
      t_streng_strd = ATON_STRENG_STRD_SET_BGAP(t_streng_strd, (nbits_in - nbits_out));
    nbits_in = nbits_out;
  }
  // intentional fall through
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
    if (conf->mem_lsb)
      t_streng_strd = ATON_STRENG_STRD_SET_FGAP(t_streng_strd, (nbits_out - nbits_in));
    else
      t_streng_strd = ATON_STRENG_STRD_SET_BGAP(t_streng_strd, (nbits_out - nbits_in));
    nbits_out = nbits_in;
  }
  // intentional fall through
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
#ifdef ATON_STRENG_CTRL_SET_SIZE1
  t = ATON_STRENG_CTRL_SET_SIZE0(t, ch_bits[0]);
  t = ATON_STRENG_CTRL_SET_SIZE1(t, ch_bits[1]);
  t = ATON_STRENG_CTRL_SET_SIZE2(t, ch_bits[2]);
#else
  if (conf->dir)
    t = ATON_STRENG_CTRL_SET_SIZE0(t, conf->nbits_out);
  else
    t = ATON_STRENG_CTRL_SET_SIZE0(t, conf->nbits_in);
#endif

  ATON_STRENG_CTRL_SET(id, t);

  // ATON_STRENG_ADDR_SET(id, conf->addr_start.i);
  ATON_REG_WRITE_RELOC(((volatile uint32_t *)(uintptr_t)ATON_STRENG_ADDR_ADDR(id)), conf->addr_base.i,
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
    ATON_STRENG_FSIZE_SET(id, t);
  }
  else
  {
    t = ATON_STRENG_FSIZE_DT;
    t = ATON_STRENG_FSIZE_SET_WIDTH(t, conf->fwidth);
    t = ATON_STRENG_FSIZE_SET_HEIGHT(t, conf->fheight);
    ATON_STRENG_FSIZE_SET(id, t);

    uint32_t line_offset = conf->line_offset == 0 ? conf->fwidth * conf->batch_offset : conf->line_offset;
    // if line_offset is left=0 then it's computed from the standard geometry of lines and batch
    t_streng_strd = ATON_STRENG_STRD_SET_LOFF(t_streng_strd, line_offset);

#if defined(ATON_STRENG_CID_CACHE_SET_LOFF_MSB)
    t = t_streng_cid_cache;
    t = ATON_STRENG_CID_CACHE_SET_LOFF_MSB(t, (line_offset >> ATON_STRENG_STRD_LOFF_W));
    t_streng_cid_cache = t;
#endif

    t = ATON_STRENG_DEPTH_DT;
    t = ATON_STRENG_DEPTH_SET_SIZE(t, conf->batch_depth);
    t = ATON_STRENG_DEPTH_SET_OFFSET(t, conf->batch_offset);
    ATON_STRENG_DEPTH_SET(id, t);
  }

  ATON_STRENG_FRPTOFF_SET(id, conf->loop_offset);
  ATON_STRENG_FRAME_RPT_SET(id, conf->frame_loop_cnt);
  ATON_STRENG_FOFFSET_SET(id, conf->frame_offset);

  t = ATON_STRENG_LIMITEN_DT; // all other fields set to zero
  t = ATON_STRENG_LIMITEN_SET_FRAMELIMIT(t, 1);
#if defined(ATON_STRENG_LIMITEN_SET_DOFF_MSB)
  t = ATON_STRENG_LIMITEN_SET_DOFF_MSB(t, conf->batch_offset >> ATON_STRENG_DEPTH_OFFSET_W);
#endif
  ATON_STRENG_LIMITEN_SET(id, t);

  if (/*(conf->dir == 0) && */ (conf->offset_limit != 0x0))
  {
    t = ATON_STRENG_LIMITEN_SET_ADDRLIMIT(t, 1);
    t = ATON_STRENG_LIMITEN_SET_STOPPREFTC(t, 1);
    ATON_STRENG_LIMITEN_SET(id, t);
    // NOTE: limiter is to be set to last accessible byte address
    // ATON_STRENG_LIMITADDR_SET(id, (conf->addr_limit.i - 1));
    ATON_REG_WRITE_RELOC(((volatile uint32_t *)(uintptr_t)ATON_STRENG_LIMITADDR_ADDR(id)), conf->addr_base.i,
                         conf->offset_limit - 1);
  }

  ATON_STRENG_LIMIT_SET(id, conf->frame_tot_cnt);
  // LL_ATON_PRINTF("frame_tot_cnt=%d\n", conf->frame_tot_cnt);

#if defined(ATON_STRENG_CID_CACHE_SET_CID)
  t_streng_cid_cache = ATON_STRENG_CID_CACHE_SET_CID(t_streng_cid_cache, conf->bus_cid);
  t_streng_cid_cache = ATON_STRENG_CID_CACHE_SET_CACHEABLE(t_streng_cid_cache, conf->cacheable);
  t_streng_cid_cache = ATON_STRENG_CID_CACHE_SET_ALLOC(t_streng_cid_cache, conf->cache_allocate);
  t_streng_cid_cache = ATON_STRENG_CID_CACHE_SET_PFETCH(t_streng_cid_cache, conf->bus_pfetch);
  t_streng_cid_cache = ATON_STRENG_CID_CACHE_SET_LINESIZE(t_streng_cid_cache, conf->cache_linesize);
#endif

  /* Enable event interrupts */
  if (conf->dir == 1)
  {
#if LL_ATON_EN_EVENT_IRQ
    t_streng_event = 0;
    t_streng_event = ATON_STRENG_EVENT_SET_EN_OFLOW_FRM(t_streng_event, 1); // enable frame overflow interrupt
#if 0
    t_streng_event = ATON_STRENG_EVENT_SET_EN_OFLOW_ADD(t_streng_event, 1); // enable address limiter interrupt
#endif
#endif // LL_ATON_EN_EVENT_IRQ
  }

  /* Enable illegal configuration interrupts */
#if LL_ATON_EN_ERROR_IRQ
  t = t_streng_event;
  t = ATON_STRENG_EVENT_SET_EN_ILLCFG(t, 1); // Enable Illegal Configuration interrupt
  // t = ATON_STRENG_EVENT_SET_EN_FMTMM(t, 1); // Enable Format Mismatch interrupt (intentionally not enabled)
  t_streng_event = t;
#endif // LL_ATON_EN_ERROR_IRQ

  if ((conf->dir == 0) && conf->sync_with_other)
  {
    t = t_streng_event;
    t = ATON_STRENG_EVENT_SET_FRMTRG_EN(t, 1);               // Enable synchronizations of frames with other dma
    t = ATON_STRENG_EVENT_SET_FRMTRG_SRC(t, conf->sync_dma); // Enable synchronizations of frames with other dma
    t_streng_event = t;
  }

#if 1
  t = ATON_STRENG_POS_DT;
  t = ATON_STRENG_POS_SET_GAPCYCLES(t, 0); // set interline gap cycle to 0, as it should be safe to do so
  ATON_STRENG_POS_SET(id, t);
#endif

  /* deferred register setting */
  ATON_STRENG_STRD_SET(id, t_streng_strd);
  ATON_STRENG_CID_CACHE_SET(id, t_streng_cid_cache);
  ATON_STRENG_EVENT_SET(id, t_streng_event);

  /* Ciphering settings */
#if (ATON_STRENG_VERSION_ENCR_DT == 1)
  t = ATON_STRENG_ENCR_MSB_DT;
  t = ATON_STRENG_ENCR_MSB_SET_EN(t, conf->cipher_en);
  t = ATON_STRENG_ENCR_MSB_SET_KEY_SEL(t, conf->key_sel);
  ATON_STRENG_ENCR_MSB_SET(id, t);
#endif

  return 0;
}

/**
 * @brief Configures the Streaming Engine External trigger source
 *        To be used, for example, for configuring synchronization with DCMIPP
 * @param id Streaming engine identifier [0..ATON_STRENG_NUM-1]
 * @param conf Pointer to structure describing External Sync configuration
 */
int LL_Streng_ExtSyncInit(int id, LL_Streng_ExtSyncTypedef *conf)
{
#ifdef ATON_STRENG_EXTSYNC_SET
  uint32_t t;

  if (id >= ATON_STRENG_NUM)
    return LL_ATON_INVALID_ID;

  LL_ATON_EnableClock(ATON_STRENG_CLKB_CLK(id));

  t = ATON_STRENG_EXTSYNC_DT;
  t = ATON_STRENG_EXTSYNC_SET_EN(t, conf->enable);
  t = ATON_STRENG_EXTSYNC_SET_SRC(t, conf->trig_source);
  t = ATON_STRENG_EXTSYNC_SET_LINES(t, conf->lines);
  ATON_STRENG_EXTSYNC_SET(id, t);

  t = ATON_STRENG_EXTSYNC2_DT;
  t = ATON_STRENG_EXTSYNC2_SET_LINES(t, conf->lines_offset);
  t = ATON_STRENG_EXTSYNC2_SET_OFF(t, conf->offset);
  ATON_STRENG_EXTSYNC2_SET(id, t);
#endif
  return 0;
}

#if defined(ATON_EPOCHCTRL_NUM)
/**
 * @brief Sets external trigger signal to hi. Used to synchronize with external units, e.g.: HSP
 * @param irq Interrupt line used as trigger signal
 * @note uses Epoch Controller noack interrupt as irq source
 */
int LL_TriggerHigh(int irq)
{
  if (irq >= ATON_INTCTRL_INTS(0))
    return LL_ATON_INVALID_PARAM;

  ATON_INTCTRL_INTORMSK_SET(0, irq, ~(1 << ATON_INTCTRL_0_INTORMSK_EPOCHCTRL_0_1_IDX));
  ATON_INTCTRL_INTSET_SET(0, (1 << ATON_INTCTRL_0_INTSET_EPOCHCTRL_0_1_IDX));
  return 0;
}

/**
 * @brief Sets external trigger signal to low. Used to synchronize with external units, e.g.: HSP
 * @param irq Interrupt line used as trigger signal
 * @note uses Epoch Controller noack interrupt as irq source
 */
int LL_TriggerLow(int irq)
{
  if (irq >= ATON_INTCTRL_INTS(0))
    return LL_ATON_INVALID_PARAM;

  ATON_INTCTRL_INTCLR_SET(0, (1 << ATON_INTCTRL_0_INTCLR_EPOCHCTRL_0_1_IDX));
  return 0;
}
#endif

unsigned __atonn_getSrcPortID(enum SwitchUnitsType sut, unsigned char su_num, enum AccelUnitsType aut,
                              unsigned char au_num, unsigned char port)
{
  // FIXME
  LL_ATON_ASSERT(su_num == 0);
  switch (sut)
  {
  case STRSWITCH:
    switch (aut)
    {
#if defined(ATON_STRENG_NUM)
    case STRENG:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_STRENG_NUM);
      return ATONN_CONST_SRCPORT(STRSWITCH, 0, STRENG, 0, 0) + au_num;
      break;
#endif
#if defined(ATON_CONVACC_NUM)
    case CONVACC:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_CONVACC_NUM);
      return ATONN_CONST_SRCPORT(STRSWITCH, 0, CONVACC, 0, 0) + au_num;
      break;
#endif
#if defined(ATON_DECUN_NUM)
    case DECUN:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_DECUN_NUM);
      return ATONN_CONST_SRCPORT(STRSWITCH, 0, DECUN, 0, 0) + au_num;
      break;
#endif
#if defined(ATON_ACTIV_NUM)
    case ACTIV:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_ACTIV_NUM);
      return ATONN_CONST_SRCPORT(STRSWITCH, 0, ACTIV, 0, 0) + au_num;
      break;
#endif
#if defined(ATON_ARITH_NUM)
    case ARITH:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_ARITH_NUM);
      return ATONN_CONST_SRCPORT(STRSWITCH, 0, ARITH, 0, 0) + au_num;
      break;
#endif
#if defined(ATON_POOL_NUM)
    case POOL:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_POOL_NUM);
      return ATONN_CONST_SRCPORT(STRSWITCH, 0, POOL, 0, 0) + au_num;
      break;
#endif
    default:
      LL_ATON_ASSERT(0);
      break;
    }
    break;
  case STRSWITCH64:
    // TODO
    LL_ATON_ASSERT(0);
    break;
  case STRSWITCH_VC:
    // TODO
    LL_ATON_ASSERT(0);
    break;
  }
  return 0;
}

unsigned __atonn_getDstPortID(enum SwitchUnitsType sut, unsigned char su_num, enum AccelUnitsType aut,
                              unsigned char au_num, unsigned char port)
{
  // FIXME
  LL_ATON_ASSERT(su_num == 0);
  switch (sut)
  {
  case STRSWITCH:
    switch (aut)
    {
#if defined(ATON_STRENG_NUM)
    case STRENG:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_STRENG_NUM);
      return ATONN_CONST_DSTPORT(STRSWITCH, 0, STRENG, 0, 0) + (0x4 * au_num);
      break;
#endif
#if defined(ATON_CONVACC_NUM)
    case CONVACC:
      LL_ATON_ASSERT(port < 3);
      LL_ATON_ASSERT(au_num < ATON_CONVACC_NUM);
      return ATONN_CONST_DSTPORT(STRSWITCH, 0, CONVACC, 0, 0) + (0x4 * (3 * au_num + port));
      break;
#endif
#if defined(ATON_DECUN_NUM)
    case DECUN:
      LL_ATON_ASSERT(port < 2);
      LL_ATON_ASSERT(au_num < ATON_DECUN_NUM);
      return ATONN_CONST_DSTPORT(STRSWITCH, 0, DECUN, 0, 0) + (0x4 * (2 * au_num + port));
      break;
#endif
#if defined(ATON_ACTIV_NUM)
    case ACTIV:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_ACTIV_NUM);
      return ATONN_CONST_DSTPORT(STRSWITCH, 0, ACTIV, 0, 0) + (0x4 * au_num);
      break;
#endif
#if defined(ATON_ARITH_NUM)
    case ARITH:
      LL_ATON_ASSERT(port < 2);
      LL_ATON_ASSERT(au_num < ATON_ARITH_NUM);
      return ATONN_CONST_DSTPORT(STRSWITCH, 0, ARITH, 0, 0) + (0x4 * (2 * au_num + port));
      break;
#endif
#if defined(ATON_POOL_NUM)
    case POOL:
      LL_ATON_ASSERT(port == 0);
      LL_ATON_ASSERT(au_num < ATON_POOL_NUM);
      return ATONN_CONST_DSTPORT(STRSWITCH, 0, POOL, 0, 0) + (0x4 * au_num);
      break;
#endif
    default:
      LL_ATON_ASSERT(0);
      break;
    }
    break;
  case STRSWITCH64:
    // TODO
    LL_ATON_ASSERT(0);
    break;
  case STRSWITCH_VC:
    // TODO
    LL_ATON_ASSERT(0);
    break;
  }
  return 0;
}

/**
 * @brief  Connects input and output port(s) on the streaming switch (without clearing configuration)
 * @param  LL_Switch_InitStruct Pointer to structure(s) describing ports to be connected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_Switch_Init_NoReset(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n)
{
  int i;
  volatile uint32_t *reg;
  uint32_t t;
  unsigned int en_shift[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH_DST_EN0_LSB, ATON_STRSWITCH_DST_EN1_LSB};
  unsigned int link_shift[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH_DST_LINK0_LSB, ATON_STRSWITCH_DST_LINK1_LSB};
  unsigned int fnr_shift[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH_DST_FNR0_LSB, ATON_STRSWITCH_DST_FNR1_LSB};
  unsigned int fnr_mask[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH_DST_FNR0_MASK, ATON_STRSWITCH_DST_FNR1_MASK};

  /* Enable Switch */
  t = ATON_STRSWITCH_CTRL_DT;
  t = ATON_STRSWITCH_CTRL_SET_EN(t, 1);
  ATON_STRSWITCH_CTRL_SET(0, t);

  for (i = 0; i < n; i++)
  {
    /* Compute target destination configuration register */
    reg = (uint32_t *)(ATON_STRSWITCH_BASE(0) + ATONN_DSTPORT_ID(LL_Switch_InitStruct[i].dest));
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

    ATON_REG_WRITE(reg, t);
  }

  return 0;
}

/**
 * @brief  Connects input and output port(s) on the streaming switch (clearing configuration at the beginning)
 * @param  LL_Switch_InitStruct Pointer to structure(s) describing ports to be connected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_Switch_Init(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n)
{
  uint32_t t;

#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
  ll_aton_static_checks();
#endif

  /* Clear Configuration */
  ATON_DISABLE_CLR_CONFCLR(STRSWITCH, 0);

  return LL_Switch_Init_NoReset(LL_Switch_InitStruct, n);
}

/**
 * @brief  Fully disconnect a destination port from source ports on the streaming switch
 * @param  LL_Switch_InitStruct Pointer to structure(s) describing ports to be disconnected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_Switch_Deinit(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n)
{
  int i;
  volatile uint32_t *reg;

  for (i = 0; i < n; i++)
  {
    /* Compute target destination configuration register */
    reg = (uint32_t *)(ATON_STRSWITCH_BASE(0) + ATONN_DSTPORT_ID(LL_Switch_InitStruct[i].dest));

    /* Disable contexts */
    ATON_REG_WRITE(reg, 0);
  }

  return 0;
}

/**
 * @brief  Disconnects destination and source port(s) on the streaming switch
 * @param  LL_Switch_InitStruct Pointer to structure(s) describing ports to be disconnected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_Switch_Deinit_Fine_Grained(const LL_Switch_InitTypeDef *LL_Switch_InitStruct, int n)
{
  int i;
  volatile uint32_t *reg;
  unsigned int en_shift[ATON_SWITCH_CONTEXT_NUM] = {ATON_STRSWITCH_DST_EN0_LSB, ATON_STRSWITCH_DST_EN1_LSB};

  for (i = 0; i < n; i++)
  {
    /* Compute target destination configuration register */
    reg = (uint32_t *)(ATON_STRSWITCH_BASE(0) + ATONN_DSTPORT_ID(LL_Switch_InitStruct[i].dest));

    /* Disable context */
    uint32_t t = *reg;
#if ATON_SWITCH_CONTEXT_NUM == 2
    t &= ~((LL_Switch_InitStruct[i].context0 != 0) << en_shift[0]);
    t &= ~((LL_Switch_InitStruct[i].context1 != 0) << en_shift[1]);
#else
    int c;
    for (c = 0; c < ATON_SWITCH_CONTEXT_NUM; c++)
      t &= ~((LL_Switch_InitStruct[i].context[c] != 0) << en_shift[c]);
#endif
    ATON_REG_WRITE(reg, t);
  }

  return 0;
}

#ifdef ATON_STRSWITCH_VC_NUM

/**
 * @brief  Connects input and output port(s) on the streaming switch with virtual channels (without clearing
 * configuration)
 * @param  LL_SwitchVC_InitStruct Pointer to structure(s) describing ports to be connected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_SwitchVC_Init_NoReset(const LL_SwitchVC_InitTypeDef *LL_SwitchVC_InitStruct, int n)
{
  int i;
  volatile uint32_t *reg;
  uint32_t t;
  unsigned int en_shift = ATON_STRSWITCH_VC_DST_EN_LSB;
  unsigned int src_shift = ATON_STRSWITCH_VC_DST_SRC_LSB;

  /* Enable Switch */
  t = ATON_STRSWITCH_VC_CTRL_DT;
  t = ATON_STRSWITCH_VC_CTRL_SET_EN(t, 1);
  ATON_STRSWITCH_VC_CTRL_SET(0, t);

  for (i = 0; i < n; i++)
  {
    /* Compute target destination configuration register */
    reg = (uint32_t *)(ATON_STRSWITCH_VC_BASE(0) + ATONN_DSTPORT_ID(LL_SwitchVC_InitStruct[i].dest));
    t = 0;
    /* Enable Context and create link */
    t |= (1 << en_shift);
    t |= (ATONN_SRCPORT_ID(LL_SwitchVC_InitStruct[i].source) << src_shift);
    ATON_REG_WRITE(reg, t);
  }

  return 0;
}

/**
 * @brief  Connects input and output port(s) on the streaming switch with virtual channels (clearing configuration at
 * the beginning)
 * @param  LL_SwitchVC_InitStruct Pointer to structure(s) describing ports to be connected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_SwitchVC_Init(const LL_SwitchVC_InitTypeDef *LL_SwitchVC_InitStruct, int n)
{
  uint32_t t;

  /* Clear Configuration */
  ATON_DISABLE_CLR_CONFCLR(STRSWITCH_VC, 0);

  return LL_SwitchVC_Init_NoReset(LL_SwitchVC_InitStruct, n);
}

/**
 * @brief  Fully disconnect a destination port from source ports on the streaming switch with virtual channels
 * @param  LL_SwitchVC_InitStruct Pointer to structure(s) describing ports to be disconnected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_SwitchVC_Deinit(const LL_SwitchVC_InitTypeDef *LL_SwitchVC_InitStruct, int n)
{
  int i;
  volatile uint32_t *reg;

  for (i = 0; i < n; i++)
  {
    /* Compute target destination configuration register */
    reg = (uint32_t *)(ATON_STRSWITCH_VC_BASE(0) + ATONN_DSTPORT_ID(LL_SwitchVC_InitStruct[i].dest));

    /* Disable contexts */
    ATON_REG_WRITE(reg, 0);
  }

  return 0;
}

/**
 * @brief  Disconnects destination and source port(s) on the streaming switch with virtual channels
 * @param  LL_SwitchVC_InitStruct Pointer to structure(s) describing ports to be disconnected
 * @param  n Number of entries in configuration array
 * @retval Error code
 */
int LL_SwitchVC_Deinit_Fine_Grained(const LL_SwitchVC_InitTypeDef *LL_SwitchVC_InitStruct, int n)
{
  int i;
  volatile uint32_t *reg;
  unsigned int en_shift = ATON_STRSWITCH_VC_DST_EN_LSB;

  for (i = 0; i < n; i++)
  {
    /* Compute target destination configuration register */
    reg = (uint32_t *)(ATON_STRSWITCH_VC_BASE(0) + ATONN_DSTPORT_ID(LL_SwitchVC_InitStruct[i].dest));

    /* Disable context */
    uint32_t t = *reg;
    t &= ~(1 << en_shift);
    ATON_REG_WRITE(reg, t);
  }

  return 0;
}
#endif // ATON_STRSWITCH_VC_NUM

#ifdef ATON_DECUN_NUM

static inline uint32_t decun_inc_page_addr(int id, uint32_t ctrl_val)
{
  uint32_t page_addr = ATON_DECUN_CTRL_GET_PAGEADDR(ctrl_val);
  page_addr++;
  ctrl_val = ATON_DECUN_CTRL_SET_PAGEADDR(ctrl_val, page_addr);
  ATON_DECUN_CTRL_SET(id, ctrl_val);
  return ctrl_val;
}

static void LL_Decun_CB_1byte(int id, const LL_Decun_InitTypeDef *conf, uint32_t ctrl_val)
{
  unsigned CBsize = conf->CBs_size;
  unsigned i, k;
  unsigned mem_idx_num = ATON_DECUN_MEM_LOW_IDX_MAX - ATON_DECUN_MEM_LOW_IDX_MIN + 1;

  if (conf->nCWperCV == 1)
  {
    uint8_t *CBp = (uint8_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i++)
    {
      uint32_t temp = CBp[i];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 2)
  {
#if 0
    uint8_t *CBp = (uint8_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 2)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 8);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num )
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
#else
    uint16_t *CBp = (uint16_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize / 2; i++)
    {
      uint32_t temp = CBp[i];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
#endif
  }
  if (conf->nCWperCV == 3)
  {
    uint8_t *CBp = (uint8_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 3)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 8) | (CBp[i + 2] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 4 || conf->nCWperCV == 8)
  {
    uint32_t *CBp = (uint32_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize / 4; i++)
    {
      uint32_t temp = CBp[i];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 5)
  {
    uint8_t *CBp = (uint8_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 5)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 8) | (CBp[i + 2] << 16) | (CBp[i + 3] << 24);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      temp = CBp[i + 4];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 6)
  {
#if 0
    uint8_t *CBp = (uint8_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 6)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 8) | (CBp[i + 2] << 16) | (CBp[i + 3] << 24);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      temp = CBp[i + 4] | (CBp[i + 5] << 8);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num )
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
#else
    uint16_t *CBp = (uint16_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize / 2; i += 3)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      temp = CBp[i + 2];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
#endif
  }
  if (conf->nCWperCV == 7)
  {
    uint8_t *CBp = (uint8_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 7)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 8) | (CBp[i + 2] << 16) | (CBp[i + 3] << 24);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      temp = CBp[i + 4] | (CBp[i + 5] << 8) | (CBp[i + 6] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
}

static void LL_Decun_CB_2byte(int id, const LL_Decun_InitTypeDef *conf, uint32_t ctrl_val)
{
  unsigned CBsize = conf->CBs_size / 2;
  unsigned i, k;
  unsigned mem_idx_num = ATON_DECUN_MEM_LOW_IDX_MAX - ATON_DECUN_MEM_LOW_IDX_MIN + 1;

  if (conf->nCWperCV == 1)
  {
    uint16_t *CBp = (uint16_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i++)
    {
      uint32_t temp = CBp[i];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 2)
  {
#if 0
    uint16_t *CBp = (uint16_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 2)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num )
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
#else
    uint32_t *CBp = (uint32_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize / 2; i++)
    {
      uint32_t temp = CBp[i];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
#endif
  }
  if (conf->nCWperCV == 3)
  {
    uint16_t *CBp = (uint16_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 3)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      temp = CBp[i + 2];
      ATON_DECUN_MEM_HIGH_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 4)
  {
    uint32_t *CBp = (uint32_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize / 2; i += 2)
    {
      uint32_t temp = CBp[i];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      temp = CBp[i + 1];
      ATON_DECUN_MEM_HIGH_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 5)
  {
    uint16_t *CBp = (uint16_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 5)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      temp = CBp[i + 2] | (CBp[i + 3] << 16);
      ATON_DECUN_MEM_HIGH_SET(id, k, temp);
      k++;
      temp = CBp[i + 4];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 6)
  {
#if 0
    uint16_t *CBp = (uint16_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 6)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      temp = CBp[i + 2] | (CBp[i + 3] << 16);
      ATON_DECUN_MEM_HIGH_SET(id, k, temp);
      k++;
      if (k == mem_idx_num )
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
      temp = CBp[i + 4] | (CBp[i + 5] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num )
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
#else
    uint32_t *CBp = (uint32_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize / 2; i += 3)
    {
      uint32_t temp = CBp[i];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      temp = CBp[i + 1];
      ATON_DECUN_MEM_HIGH_SET(id, k, temp);
      k++;
      temp = CBp[i + 2];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
#endif
  }
  if (conf->nCWperCV == 7)
  {
    uint16_t *CBp = (uint16_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize; i += 7)
    {
      uint32_t temp = CBp[i] | (CBp[i + 1] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      temp = CBp[i + 2] | (CBp[i + 3] << 16);
      ATON_DECUN_MEM_HIGH_SET(id, k, temp);
      k++;
      temp = CBp[i + 4] | (CBp[i + 5] << 16);
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      temp = CBp[i + 6];
      ATON_DECUN_MEM_HIGH_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
  if (conf->nCWperCV == 8)
  {
    uint32_t *CBp = (uint32_t *)conf->CBs_vector.p;
    for (i = 0, k = 0; i < CBsize / 2; i += 2)
    {
      uint32_t temp = CBp[i];
      ATON_DECUN_MEM_LOW_SET(id, k, temp);
      temp = CBp[i + 1];
      ATON_DECUN_MEM_HIGH_SET(id, k, temp);
      k++;
      if (k == mem_idx_num)
      {
        ctrl_val = decun_inc_page_addr(id, ctrl_val);
        k = 0;
      }
    }
  }
}

/**
 * @brief  Configures Decompression Unit accelerator
 * @param  id Decompression Unit identifier (Always 0 fo Tiny Orlando)
 * @param  conf Pointer to structure describing decompression unit configuration
 * @retval Error code
 */
int LL_Decun_Init(int id, const LL_Decun_InitTypeDef *conf)
{
  uint32_t t;

  if (id >= ATON_DECUN_NUM)
    return LL_ATON_INVALID_ID;

  LL_ATON_EnableClock(ATON_DECUN_CLKB_CLK(id));

  t = ATON_DECUN_CTRL_DT;
  t = ATON_DECUN_CTRL_SET_DUALIN(t, (conf->noDualInput != 0));
  t = ATON_DECUN_CTRL_SET_OW(t, (conf->noOverWrite != 0));
  ATON_DECUN_CTRL_SET(id, t);

  if (conf->CBs_size != 0)
  {
    if (conf->nFormatBytes == 1)
    {
      LL_Decun_CB_1byte(id, conf, t);
    }
    else
    {
      // LL_ATON_ASSERT(conf->nFormatBytes == 2);
      LL_Decun_CB_2byte(id, conf, t);
    }
  }

  t = ATON_DECUN_BFORMAT_DT;
  t = ATON_DECUN_BFORMAT_SET_CVS(t, conf->nCVperCB - 1);
  t = ATON_DECUN_BFORMAT_SET_CWS(t, conf->nCWperCV - 1);
  t = ATON_DECUN_BFORMAT_SET_OSAM(t, conf->nRCWlastCV);
  ATON_DECUN_BFORMAT_SET(id, t);

  LL_ATON_ASSERT((conf->nFormatBytes == 1) || (conf->nFormatBytes == 2));

  t = ATON_DECUN_DFORMAT_DT;
  t = ATON_DECUN_DFORMAT_SET_CV8(t, conf->nFormatBytes & 0x1);
  ATON_DECUN_DFORMAT_SET(id, t);

  t = ATON_DECUN_FFORMAT_DT;
  t = ATON_DECUN_FFORMAT_SET_BN(t, conf->nBatches);
  ATON_DECUN_FFORMAT_SET(id, t);

  return LL_ATON_OK;
}
#endif // ATON_DECUN_NUM

#ifdef ATON_CONVACC_NUM
/**
 * @brief  Configures Convolution Accelerator
 * @param  id Convolutional Accelerator identifier [0, ATON_CONVACC_NUM-1]
 * @param  Convacc_InitStruct Structure describing initialization parameters
 * @retval Error code E.g.: Invalid ID, invalid parameters, not idle,..
 */
int LL_Convacc_Init(int id, const LL_Convacc_InitTypeDef *conf)
{
  uint32_t t;

  if (id >= ATON_CONVACC_NUM)
    return LL_ATON_INVALID_ID;

  LL_ATON_EnableClock(ATON_CONVACC_CLKB_CLK(id));

  t = ATON_CONVACC_CTRL_DT;
  t = ATON_CONVACC_CTRL_SET_NOSUM(t, (conf->accumulate == 0));
  t = ATON_CONVACC_CTRL_SET_NO1SUM(t, (conf->accumulate_first == 0));
  t = ATON_CONVACC_CTRL_SET_GEN1SUM(t, (conf->accumulate_gen_first != 0));
  t = ATON_CONVACC_CTRL_SET_AFILTMODE(t, conf->afilt_mode);
  t = ATON_CONVACC_CTRL_SET_SIMD(t, (conf->simd));
  t = ATON_CONVACC_CTRL_SET_KT1(t, (conf->kt1_mode != 0));
  t = ATON_CONVACC_CTRL_SET_KSETEN(t, conf->kseten);
  t = ATON_CONVACC_CTRL_SET_FUNSIGNED(t, conf->f_unsigned);
  t = ATON_CONVACC_CTRL_SET_KUNSIGNED(t, conf->k_unsigned);
#if defined(ATON_CONVACC_CTRL_SET_FSTAT)
  t = ATON_CONVACC_CTRL_SET_FSTAT(t, (conf->fstat != 0));
#endif

#if defined(ATON_CONVACC_CTRL_GET_DEEPMODE)
  t = ATON_CONVACC_CTRL_SET_DEEPMODE(t, (conf->deepmode != 0));
#if defined(ATON_CONVACC_CTRL_SET_FSTAT)
  // deepmode and feature stats are incompatible
  if ((conf->fstat != 0) && (conf->deepmode != 0))
  {
    return LL_ATON_INVALID_PARAM;
  }
#endif
#endif

#if defined(ATON_CONVACC_CTRL_GET_DSS2MODE)
  t = ATON_CONVACC_CTRL_SET_DSS2MODE(t, (conf->dss2mode != 0));
#if defined(ATON_CONVACC_CTRL_SET_FSTAT)
  // deepmode and feature stats are incompatible
  if ((conf->fstat != 0) && (conf->dss2mode != 0))
  {
    return LL_ATON_INVALID_PARAM;
  }
#endif
#endif

#if defined(ATON_CONVACC_CTRL_SET_DSS2MODE) && defined(ATON_CONVACC_CTRL_SET_DEEPMODE)
  // deepmode and dss2mode stats are incompatible
  if ((conf->dss2mode != 0) && (conf->deepmode != 0))
  {
    return LL_ATON_INVALID_PARAM;
  }
#endif

  ATON_CONVACC_CTRL_SET(id, t);
  Conv_ctrl_bits[id] = t;

  if (conf->afilt_mode != AFILT_MODE_NONE)
  {
    t = ATON_CONVACC_AFILT_DT;
    t = ATON_CONVACC_AFILT_SET_TOT(t, conf->afilt_tot);
    t = ATON_CONVACC_AFILT_SET_FIRST(t, conf->afilt_first);
    t = ATON_CONVACC_AFILT_SET_LAST(t, conf->afilt_last);
    ATON_CONVACC_AFILT_SET(id, t);
  }

  if (conf->kfilt_tot > 0)
  {
    t = ATON_CONVACC_KFILT_DT;
    t = ATON_CONVACC_KFILT_SET_TOT(t, conf->kfilt_tot);
    t = ATON_CONVACC_KFILT_SET_FIRST(t, conf->kfilt_first);
    t = ATON_CONVACC_KFILT_SET_LAST(t, conf->kfilt_last);
    ATON_CONVACC_KFILT_SET(id, t);
  }

  t = ATON_CONVACC_DFORMAT_DT;
  t = ATON_CONVACC_DFORMAT_SET_FROUND(t, conf->rounding_f);
  t = ATON_CONVACC_DFORMAT_SET_FSAT(t, conf->saturation_f);
  t = ATON_CONVACC_DFORMAT_SET_FRNDMODE(t, conf->round_mode_f);
  t = ATON_CONVACC_DFORMAT_SET_FBYTES(t, conf->inbytes_f);
  t = ATON_CONVACC_DFORMAT_SET_FSHIFT(t, ATON_SHIFT(conf->shift_f));

  t = ATON_CONVACC_DFORMAT_SET_ROUND(t, conf->rounding_o);
  t = ATON_CONVACC_DFORMAT_SET_SAT(t, conf->saturation_o);
  t = ATON_CONVACC_DFORMAT_SET_ORNDMODE(t, (conf->relu_mode_o << 1) | conf->round_mode_o);
  t = ATON_CONVACC_DFORMAT_SET_OBYTES(t, conf->outbytes_o);
  t = ATON_CONVACC_DFORMAT_SET_OUTSHIFT(t, conf->shift_o); // shift right only
  t = ATON_CONVACC_DFORMAT_SET_RAW(t, conf->raw_o);

  t = ATON_CONVACC_DFORMAT_SET_INSHIFT(t, conf->shift_a); // accumulator shift left really (macro name is misleading)
  ATON_CONVACC_DFORMAT_SET(id, t);

  t = ATON_CONVACC_FFORMAT_DT;
  t = ATON_CONVACC_FFORMAT_SET_WIDTH(t, conf->fWidth * conf->batchDepth);
  t = ATON_CONVACC_FFORMAT_SET_HEIGHT(t, conf->fHeight);
  ATON_CONVACC_FFORMAT_SET(id, t);

  t = ATON_CONVACC_KFORMAT_DT;
  t = ATON_CONVACC_KFORMAT_SET_WIDTH(t, conf->kernelWidth);
  t = ATON_CONVACC_KFORMAT_SET_HEIGHT(t, conf->kernelHeight);
  t = ATON_CONVACC_KFORMAT_SET_BTCDEPTH(t, conf->batchDepth);
  t = ATON_CONVACC_KFORMAT_SET_NR(t, conf->nKernels);
  ATON_CONVACC_KFORMAT_SET(id, t);

  // LL_ATON_PRINTF("depth=%d k_w=%d k_h=%d\n",conf->batchDepth,conf->kernelWidth,conf->kernelHeight);
  // LL_ATON_PRINTF("pad_t=%d pad_b=%d pad_l=%d
  // pad_r=%d\n",conf->top_padding,conf->bot_padding,conf->left_padding,conf->right_padding);

  int p_top = (conf->top_padding < conf->kernelHeight ? conf->top_padding : conf->kernelHeight - 1);
  int p_bot = (conf->bot_padding < conf->kernelHeight ? conf->bot_padding : conf->kernelHeight - 1);
  int p_left = (conf->left_padding < conf->kernelWidth ? conf->left_padding : conf->kernelWidth - 1);
  int p_right = (conf->right_padding < conf->kernelWidth ? conf->right_padding : conf->kernelWidth - 1);

  p_top = (p_top <= 2 ? p_top : 2);
  p_bot = (p_bot <= 2 ? p_bot : 2);
  p_left = (p_left <= 2 ? p_left : 2);
  p_right = (p_right <= 2 ? p_right : 2);
  // LL_ATON_PRINTF("p_t=%d p_b=%d p_l=%d p_r=%d\n",p_top,p_bot,p_left,p_right);

#if defined(ATON_CONVACC_CTRL_GET_DEEPMODE)
  // no pad mode available in 1x1 deepmode
  // will accomodate padding only with zframe below
  if (conf->deepmode != 0)
    p_top = p_bot = p_left = p_right = 0;
#endif
#if defined(ATON_CONVACC_CTRL_GET_DSS2MODE)
  // no pad mode available in dss2mode
  // will accomodate padding only with zframe below
  if (conf->dss2mode != 0)
    p_top = p_bot = p_left = p_right = 0;
#endif
#if defined(ATON_CONVACC_ZFBIAS_SET)
  // no pad mode available if zfbias is set
  // will accomodate padding only with zframe below
  if (conf->zfbias != 0)
    p_top = p_bot = p_left = p_right = 0;
#endif

  int z_top = (conf->top_padding - p_top);
  int z_bot = (conf->bot_padding - p_bot);
  int z_left = (conf->left_padding - p_left);
  int z_right = (conf->right_padding - p_right);
  // LL_ATON_PRINTF("z_t=%d z_b=%d z_l=%d z_r=%d\n",z_top,z_bot,z_left*conf->batchDepth,z_right*conf->batchDepth);

  t = ATON_CONVACC_ZFRAME_DT;
  t = ATON_CONVACC_ZFRAME_SET_TOP(t, z_top);
  t = ATON_CONVACC_ZFRAME_SET_BOTTOM(t, z_bot);
  t = ATON_CONVACC_ZFRAME_SET_LEFT(t, z_left * conf->batchDepth);
  t = ATON_CONVACC_ZFRAME_SET_RIGHT(t, z_right * conf->batchDepth);
  ATON_CONVACC_ZFRAME_SET(id, t);

  t = ATON_CONVACC_SAMPLE_DT;
  t = ATON_CONVACC_SAMPLE_SET_TPAD(t, p_top);
  t = ATON_CONVACC_SAMPLE_SET_BPAD(t, p_bot);
  t = ATON_CONVACC_SAMPLE_SET_LPAD(t, p_left);
  t = ATON_CONVACC_SAMPLE_SET_RPAD(t, p_right);
  t = ATON_CONVACC_SAMPLE_SET_HSTRD(t, conf->hstride);
  t = ATON_CONVACC_SAMPLE_SET_VSTRD(t, conf->vstride);
#if defined(ATON_CONVACC_SAMPLE_SET_FSTATCNT)
  if (conf->fstat != 0)
    t = ATON_CONVACC_SAMPLE_SET_FSTATCNT(t, conf->fstatcnt);
#endif
  ATON_CONVACC_SAMPLE_SET(id, t);

  // LL_ATON_PRINTF("crop_t=%d crop_b=%d crop_l=%d crop_r=%d\n",z_top,z_bot,z_left,z_right);
  t = ATON_CONVACC_FHCROP_DT;
  if (conf->left_crop > 0)
    t = ATON_CONVACC_FHCROP_SET_LEFT(t, conf->left_crop * conf->batchDepth);
  if (conf->right_crop > 0)
    t = ATON_CONVACC_FHCROP_SET_RIGHT(t, conf->right_crop * conf->batchDepth + (conf->batchDepth - 1));
  ATON_CONVACC_FHCROP_SET(id, t);

  t = ATON_CONVACC_FVCROP_DT;
  if (conf->top_crop > 0)
    t = ATON_CONVACC_FVCROP_SET_TOP(t, conf->top_crop);
  if (conf->bot_crop > 0)
    t = ATON_CONVACC_FVCROP_SET_BOTTOM(t, conf->bot_crop);
  ATON_CONVACC_FVCROP_SET(id, t);
  // LL_ATON_PRINTF("c_t=%d c_b=%d c_l=%d c_r=%d\n",conf->top_crop,conf->bot_crop,conf->left_crop  *
  // conf->batchDepth,conf->right_crop  * conf->batchDepth + (conf->batchDepth - 1));

#if defined(ATON_CONVACC_FSUB_SET)
  if (conf->fsub != 0)
  {
    t = ATON_CONVACC_FSUB_DT;
    t = ATON_CONVACC_FSUB_SET_FSUB(t, conf->fsub);
    ATON_CONVACC_FSUB_SET(id, t);
  }
#endif
#if defined(ATON_CONVACC_ZFBIAS_SET)
  t = ATON_CONVACC_ZFBIAS_DT;
  if (conf->zfbias != 0)
  {
    t = ATON_CONVACC_ZFBIAS_SET_ZFBIAS(t, conf->zfbias);
  }

  /* If hardware supports it (e.g.: 4CA2P), manage Batch Depths not fitting 8 bits */
#if defined(ATON_CONVACC_ZFBIAS_SET_ZFLEFTMSB)
  t = ATON_CONVACC_ZFBIAS_SET_ZFLEFTMSB(t, (z_left * conf->batchDepth) >> ATON_CONVACC_ZFRAME_LEFT_W);
  t = ATON_CONVACC_ZFBIAS_SET_ZFRIGHTMSB(t, (z_right * conf->batchDepth) >> ATON_CONVACC_ZFRAME_RIGHT_W);
#endif

  ATON_CONVACC_ZFBIAS_SET(id, t);
#endif

  return 0;
}
#endif // ATON_CONVACC_NUM

#ifdef ATON_ACTIV_NUM

static int32_t get_Activacc_type(LL_Activacc_Op op)
{
  switch (op)
  {
  case ACTIV_RELU:
    return ATON_ACTIVTYPE_RELU;
  case ACTIV_PRELU:
    return ATON_ACTIVTYPE_PRELU;
  case ACTIV_TRELU:
    return ATON_ACTIVTYPE_TRELU;
  case ACTIV_FUNC:
    return ATON_ACTIVTYPE_FUNCTION;
#if defined ATON_ACTIVTYPE_LUT
  case ACTIV_LUT:
    return ATON_ACTIVTYPE_LUT;
#endif
  default:
    break;
  }
  LL_ATON_ASSERT(0);
  return 0;
}

/**
 * @brief  Configures Activation Accelerator
 * @param  id Activation Accelerator identifier [0..ATON_ACTIV_NUM-1]
 * @param  Activacc_InitStruct Structure describing initialization parameters
 * @retval Error code E.g.: Invalid ID, invalid parameters, not idle,..
 */
int LL_Activacc_Init(int id, const LL_Activacc_InitTypeDef *conf)
{
  uint32_t t;

  if (id >= ATON_ACTIV_NUM)
    return LL_ATON_INVALID_ID;

  LL_ATON_EnableClock(ATON_ACTIV_CLKB_CLK(id));

  t = ATON_ACTIV_CTRL_DT;
  t = ATON_ACTIV_CTRL_SET_TYPE(t, get_Activacc_type(conf->operation));
  t = ATON_ACTIV_CTRL_SET_FBYTES(t, conf->inbytes_f);
  t = ATON_ACTIV_CTRL_SET_FSHIFT(t, ATON_SHIFT(conf->shift_f));
  t = ATON_ACTIV_CTRL_SET_FROUND(t, conf->rounding_f);
  t = ATON_ACTIV_CTRL_SET_FSAT(t, (conf->saturation_f != 0));
  t = ATON_ACTIV_CTRL_SET_FRNDMODE(t, conf->round_mode_f);
  t = ATON_ACTIV_CTRL_SET_FOBYTES(t, conf->outbytes_f);

  t = ATON_ACTIV_CTRL_SET_ROUND(t, (conf->rounding_o != 0));
  t = ATON_ACTIV_CTRL_SET_SAT(t, (conf->saturation_o != 0));
  t = ATON_ACTIV_CTRL_SET_OBYTES(t, conf->outbytes_o);
  t = ATON_ACTIV_CTRL_SET_ORNDMODE(t, (conf->relu_mode_o << 1) | conf->round_mode_o);
  ATON_ACTIV_CTRL_SET(id, t);

#if defined(ATON_ACTIV_FSUB_SET)
  if (conf->fsub != 0)
  {
    t = ATON_ACTIV_FSUB_DT;
    t = ATON_ACTIV_FSUB_SET_FSUB(t, conf->fsub);
    ATON_ACTIV_FSUB_SET(id, t);
  }
#endif

  switch (conf->operation)
  {
  case ACTIV_RELU:
  case ACTIV_PRELU:
  case ACTIV_TRELU:
    t = ATON_ACTIV_ACTIVPARAM_DT;
    t = ATON_ACTIV_ACTIVPARAM_SET_PARAM(t, conf->parameter);
    ATON_ACTIV_ACTIVPARAM_SET(id, t);

    t = ATON_ACTIV_ACTIVPARAM2_DT;
    t = ATON_ACTIV_ACTIVPARAM2_SET_PARAM2(t, conf->parameter_2);
    ATON_ACTIV_ACTIVPARAM2_SET(id, t);

    t = ATON_ACTIV_FUNC_DT;
    t = ATON_ACTIV_FUNC_SET_SIGNEDOP(t, (conf->signedop != 0));
    t = ATON_ACTIV_FUNC_SET_OUTSHIFT(t, conf->shift_o);
    ATON_ACTIV_FUNC_SET(id, t);
    break;
  case ACTIV_FUNC:
  {
    t = ATON_ACTIV_FUNC_DT;
    t = ATON_ACTIV_FUNC_SET_SIGNEDOP(t, (conf->signedop != 0));
    t = ATON_ACTIV_FUNC_SET_OUTSHIFT(t, conf->shift_o);
    t = ATON_ACTIV_FUNC_SET_BSHIFT(t, conf->shift_b);
    t = ATON_ACTIV_FUNC_SET_CSHIFT(t, conf->shift_c);
    t = ATON_ACTIV_FUNC_SET_BWIDTH(t, conf->bwidth);
    ATON_ACTIV_FUNC_SET(id, t);

    t = ATON_ACTIV_ACTIVPARAM_DT;
    t = ATON_ACTIV_ACTIVPARAM_SET_PARAM(t, conf->parameter);
    t = ATON_ACTIV_ACTIVPARAM_SET_FUNC(t, conf->shift_norm);
    ATON_ACTIV_ACTIVPARAM_SET(id, t);

    {
      uint8_t *R0p = (uint8_t *)conf->ROM0_vector.p;
      unsigned ROM0_rows = conf->ROM0_nbytes / (1 * 1);
      for (int i = 0; i < ROM0_rows; i++)
      {
        t = ATON_ACTIV_ROM0_DT;
        t = ATON_ACTIV_ROM0_SET_ENTRY(t, R0p[i]);
        ATON_ACTIV_ROM0_SET(id, i, t);
      }
    }
    {
      uint16_t *R1p = (uint16_t *)conf->ROM1_vector.p;
      unsigned ROM1_rows = conf->ROM1_nbytes / (3 * 2);
      for (int i = 0; i < ROM1_rows; i++)
      {
        t = ATON_ACTIV_ROM1_AB_DT;
        t = ATON_ACTIV_ROM1_AB_SET_A(t, R1p[3 * i + 0]);
        t = ATON_ACTIV_ROM1_AB_SET_B(t, R1p[3 * i + 1]);
        ATON_ACTIV_ROM1_AB_SET(id, i, t);
        t = ATON_ACTIV_ROM1_C_DT;
        t = ATON_ACTIV_ROM1_C_SET_C(t, R1p[3 * i + 2]);
        ATON_ACTIV_ROM1_C_SET(id, i, t);
      }
    }
  }
  break;
  case ACTIV_LUT:
#if defined ATON_ACTIVTYPE_LUT
  {
    uint8_t *Lutp = (uint8_t *)conf->LUT_vector.p;
    for (int i = 0; i < 64; i++)
    {
      uint32_t val = ATON_ACTIV_LUT_DT;
      val = ATON_ACTIV_LUT_SET_MOD8_1(val, Lutp[4 * i]);
      val = ATON_ACTIV_LUT_SET_MOD8_2(val, Lutp[4 * i + 1]);
      val = ATON_ACTIV_LUT_SET_MOD8_3(val, Lutp[4 * i + 2]);
      val = ATON_ACTIV_LUT_SET_MOD8_4(val, Lutp[4 * i + 3]);
      ATON_ACTIV_LUT_SET(id, i, val);
    }
  }
#else
    assert("Unsupported Activation LUT configuration");
#endif
  break;
  default:
    return -1;
  }

  return 0;
}
#endif // ATON_ACTIV_NUM

#ifdef ATON_ARITH_NUM

static int32_t get_Arithacc_type(LL_Arithacc_Op op)
{
  switch (op)
  {
  case ARITH_AFFINE:
    return ATON_ARITHOP_AX_BY_C;
  case ARITH_MIN:
    return ATON_ARITHOP_MIN_X_Y;
  case ARITH_MAX:
    return ATON_ARITHOP_MAX_X_Y;
  case ARITH_MUL:
    return ATON_ARITHOP_XY;
  case ARITH_X_AND_Y:
    return ATON_ARITHOP_X_AND_Y;
  case ARITH_X_OR_Y:
    return ATON_ARITHOP_X_OR_Y;
  case ARITH_NOT_X:
    return ATON_ARITHOP_NOT_X;
  case ARITH_X_XOR_Y:
    return ATON_ARITHOP_X_XOR_Y;
  case ARITH_X_EQ_Y:
    return ATON_ARITHOP_X_EQ_Y;
  case ARITH_X_LT_Y:
    return ATON_ARITHOP_X_LT_Y;
  case ARITH_X_LE_Y:
    return ATON_ARITHOP_X_LE_Y;
  case ARITH_X_GT_Y:
    return ATON_ARITHOP_X_GT_Y;
  case ARITH_X_GE_Y:
    return ATON_ARITHOP_X_GE_Y;
  case ARITH_ABS_X:
    return ATON_ARITHOP_ABS_X;
  case ARITH_SIGN_X:
    return ATON_ARITHOP_SIGN_X;
  case ARITH_CLIP:
    return ATON_ARITHOP_CLIP;
  default:
    break;
  }
  LL_ATON_ASSERT(0);
  return 0;
}

/**
 * @brief  Configures Arithmetic Accelerator
 * @param  id Arithmetic Accelerator identifier [0..ATON_ARITH_NUM-1]
 * @param  Arithacc_InitStruct Structure describing initialization parameters
 * @retval Error code E.g.: Invalid ID, invalid parameters, not idle,..
 */
int LL_Arithacc_Init(int id, const LL_Arithacc_InitTypeDef *conf)
{
  uint32_t t;
  char sshift;

  if (id >= ATON_ARITH_NUM)
    return LL_ATON_INVALID_ID;

  /* Deal with Arith Acc V3 shifts extensions */
  if (ATON_ARITH_VERSION_MAJOR_DT >= 3)
    sshift = 16;
  else
    sshift = 0;

  LL_ATON_EnableClock(ATON_ARITH_CLKB_CLK(id));

  t = ATON_ARITH_CTRL_DT;
  ATON_ARITH_CTRL_SET(id, t);

  t = ATON_ARITH_CTRL_DT;
  t = ATON_ARITH_CTRL_SET_CNT1(t, (conf->scalar == 0)); // these must be handled yet FIXME !!!
  t = ATON_ARITH_CTRL_SET_CNT2(t, (conf->scalar == 0) && (conf->bcast == ARITH_BCAST_CHAN));
  t = ATON_ARITH_CTRL_SET_CNT3(t, (conf->scalar == 0) && (conf->bcast == ARITH_BCAST_CHAN));
  t = ATON_ARITH_CTRL_SET_ROUND(t, (conf->rounding_o != 0));
  t = ATON_ARITH_CTRL_SET_ORNDMODE(t, (conf->relu_mode_o << 1) | conf->round_mode_o);
  // t = ATON_ARITH_CTRL_SET_OBYTES(t,((conf->dataw_o + 7) >> 3));
  t = ATON_ARITH_CTRL_SET_OBYTES(t, conf->outbytes_o);
  t = ATON_ARITH_CTRL_SET_SAT(t, (conf->saturation_o != 0));
  t = ATON_ARITH_CTRL_SET_COEFFA(t, (conf->scalar == 0));
  t = ATON_ARITH_CTRL_SET_COEFFB(t, (conf->scalar == 0));
  t = ATON_ARITH_CTRL_SET_COEFFC(t, (conf->scalar == 0));
  t = ATON_ARITH_CTRL_SET_DUALIN(t, (conf->dualinput != 0));
  t = ATON_ARITH_CTRL_SET_COMBINEBC(t, (conf->combinebc != 0));
#ifdef ATON_ARITH_CTRL_SET_CLIPOUT
  t = ATON_ARITH_CTRL_SET_CLIPOUT(t, (conf->clipout != 0));
#endif
  t = ATON_ARITH_CTRL_SET_OP(t, get_Arithacc_type(conf->operation));
  if (conf->operation == ARITH_NOT_X)
  { // WORKAROUND
    t = ATON_ARITH_CTRL_SET_LOGICALOP(t, 1);
  }
  ATON_ARITH_CTRL_SET(id, t);

  t = ATON_ARITH_INSHIFTER_DT;
  t = ATON_ARITH_INSHIFTER_SET_FBYTESX(t, conf->inbytes_x);
  t = ATON_ARITH_INSHIFTER_SET_FSHIFTX(t, ATON_SHIFT(conf->shift_x));
  t = ATON_ARITH_INSHIFTER_SET_FROUNDX(t, (conf->rounding_x != 0));
  t = ATON_ARITH_INSHIFTER_SET_FSATX(t, (conf->saturation_x != 0));
  t = ATON_ARITH_INSHIFTER_SET_FRNDMODEX(t, conf->round_mode_x);
  t = ATON_ARITH_INSHIFTER_SET_FOBYTESX(t, conf->outbytes_x);

  t = ATON_ARITH_INSHIFTER_SET_FBYTESY(t, conf->inbytes_y);
  t = ATON_ARITH_INSHIFTER_SET_FSHIFTY(t, ATON_SHIFT(conf->shift_y));
  t = ATON_ARITH_INSHIFTER_SET_FROUNDY(t, (conf->rounding_y != 0));
  t = ATON_ARITH_INSHIFTER_SET_FSATY(t, (conf->saturation_y != 0));
  t = ATON_ARITH_INSHIFTER_SET_FRNDMODEY(t, conf->round_mode_y);
  t = ATON_ARITH_INSHIFTER_SET_FOBYTESY(t, conf->outbytes_y);
  ATON_ARITH_INSHIFTER_SET(id, t);

  t = ATON_ARITH_SHIFT_DT;
  t = ATON_ARITH_SHIFT_SET_AX(t, conf->Ax_shift + sshift);
  t = ATON_ARITH_SHIFT_SET_BY(t, conf->By_shift + sshift);
  t = ATON_ARITH_SHIFT_SET_C(t, conf->C_shift + sshift);
  t = ATON_ARITH_SHIFT_SET_RES(t, conf->shift_o); // right shift only
  ATON_ARITH_SHIFT_SET(id, t);

  t = ATON_ARITH_COEFFAC_DT;
  t = ATON_ARITH_COEFFAC_SET_A(t, conf->A_scalar);
  t = ATON_ARITH_COEFFAC_SET_C(t, conf->C_scalar);
  ATON_ARITH_COEFFAC_SET(id, t);

  t = ATON_ARITH_COEFFB_DT;
  t = ATON_ARITH_COEFFB_SET_B(t, conf->B_scalar);
  ATON_ARITH_COEFFB_SET(id, t);

#ifdef ATON_ARITH_CLIPRANGE_SET
  if (conf->clipout)
  {
    t = ATON_ARITH_CLIPRANGE_DT;
    t = ATON_ARITH_CLIPRANGE_SET_CLIPMAX(t, (conf->clipmax & 0xFFFFU));
    t = ATON_ARITH_CLIPRANGE_SET_CLIPMIN(t, (conf->clipmin & 0xFFFFU));
    ATON_ARITH_CLIPRANGE_SET(id, t);

#ifdef ATON_ARITH_CLIPRANGE_MSB_SET
    t = ATON_ARITH_CLIPRANGE_MSB_DT;
    t = ATON_ARITH_CLIPRANGE_MSB_SET_CLIPMAX(t, (conf->clipmax >> 16));
    t = ATON_ARITH_CLIPRANGE_MSB_SET_CLIPMIN(t, (conf->clipmin >> 16));
    ATON_ARITH_CLIPRANGE_MSB_SET(id, t);
#endif
  }
#endif

  ATON_ARITH_COEFFADDR_SET(id, 0);

  int maxc = 0;
  if (conf->scalar == 0)
  { // vector mode
    switch (conf->bcast)
    {
    case ARITH_BCAST_CHAN:
      t = ATON_ARITH_INCCNT_SET_INCVAL(0, 0);
      ATON_ARITH_INCCNT_SET(id, t);

      t = ATON_ARITH_RSTCNT1_SET_RSTCNT(0, (conf->batchDepth - 1));
      ATON_ARITH_RSTCNT1_SET(id, t);

      t = ATON_ARITH_RSTCNT2_SET_RSTCNT(
          0, (conf->fWidth * conf->fHeight * conf->batchDepth - 1)); // must check this fits into 27 bits FIXME !!!
      ATON_ARITH_RSTCNT2_SET(id, t);

      t = ATON_ARITH_RSTCNT3_SET_RSTCNT(
          0, (conf->fWidth * conf->fHeight * conf->fChannels - 1)); // must check this fits into 27 bits FIXME !!!
      ATON_ARITH_RSTCNT3_SET(id, t);

      t = ATON_ARITH_INCOFFSET_SET_VAL(0, 1);
      ATON_ARITH_INCOFFSET_SET(id, t);

      t = ATON_ARITH_ADDROFFSET_SET_OFFSET1(0, -(conf->batchDepth - 1));
      t = ATON_ARITH_ADDROFFSET_SET_OFFSET2(t, 1);
      t = ATON_ARITH_ADDROFFSET_SET_OFFSET3(t, -(conf->fChannels - 1));
      ATON_ARITH_ADDROFFSET_SET(id, t);
      maxc = conf->fChannels;
      break;
    case ARITH_BCAST_HEIGHT:
      t = ATON_ARITH_INCCNT_SET_INCVAL(0, (conf->fWidth * conf->batchDepth - 1));
      ATON_ARITH_INCCNT_SET(id, t);

      t = ATON_ARITH_RSTCNT1_SET_RSTCNT(0, (conf->fWidth * conf->fHeight * conf->batchDepth - 1));
      ATON_ARITH_RSTCNT1_SET(id, t);

      t = ATON_ARITH_INCOFFSET_SET_VAL(0, 1);
      ATON_ARITH_INCOFFSET_SET(id, t);

      t = ATON_ARITH_ADDROFFSET_SET_OFFSET1(0, -(conf->fHeight - 1));
      ATON_ARITH_ADDROFFSET_SET(id, t);
      maxc = conf->fHeight;
      break;
    case ARITH_BCAST_WIDTH:
      t = ATON_ARITH_INCCNT_SET_INCVAL(0, (conf->batchDepth - 1));
      ATON_ARITH_INCCNT_SET(id, t);

      t = ATON_ARITH_RSTCNT1_SET_RSTCNT(0, (conf->fWidth * conf->batchDepth - 1));
      ATON_ARITH_RSTCNT1_SET(id, t);

      t = ATON_ARITH_INCOFFSET_SET_VAL(0, 1);
      ATON_ARITH_INCOFFSET_SET(id, t);

      t = ATON_ARITH_ADDROFFSET_SET_OFFSET1(0, -(conf->fWidth - 1));
      ATON_ARITH_ADDROFFSET_SET(id, t);
      maxc = conf->fWidth;
      break;
    case ARITH_BCAST_HEIGHT_WIDTH:
      t = ATON_ARITH_INCCNT_SET_INCVAL(0, (conf->batchDepth - 1));
      ATON_ARITH_INCCNT_SET(id, t);

      t = ATON_ARITH_RSTCNT1_SET_RSTCNT(0, (conf->fHeight * conf->fWidth * conf->batchDepth - 1));
      ATON_ARITH_RSTCNT1_SET(id, t);

      t = ATON_ARITH_INCOFFSET_SET_VAL(0, 1);
      ATON_ARITH_INCOFFSET_SET(id, t);

      t = ATON_ARITH_ADDROFFSET_SET_OFFSET1(0, -(conf->fHeight * conf->fWidth - 1));
      ATON_ARITH_ADDROFFSET_SET(id, t);
      maxc = conf->fHeight * conf->fWidth;
      break;
    case ARITH_BCAST_SCALAR:
      t = ATON_ARITH_INCCNT_SET_INCVAL(0, 0);
      ATON_ARITH_INCCNT_SET(id, t);

      t = ATON_ARITH_INCOFFSET_SET_VAL(0, 0);
      ATON_ARITH_INCOFFSET_SET(id, t);

      maxc = 1;
      break;
    default:
      return LL_ATON_INVALID_PARAM;
    }

    maxc = (maxc > 512 ? 512 : maxc);
    int i, k;
    uint32_t *Ap = (uint32_t *)conf->A_vector.p;
    uint32_t *Bp = (uint32_t *)conf->B_vector.p;
    uint32_t *Cp = (uint32_t *)conf->C_vector.p;
    int nbits_A = conf->vec_precision[0];
    int nbits_B = conf->vec_precision[1];
    int nbits_C = conf->vec_precision[2];
    int bitcnt_A = 0;
    int bitcnt_B = 0;
    int bitcnt_C = 0;
    int mem_idx_num = ATON_ARITH_COEFF_AB_IDX_MAX - ATON_ARITH_COEFF_AB_IDX_MIN + 1;

    int32_t c_sign = (conf->C_scalar == -1 ? -1 : 1);
#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
    ec_trace_comment("Block ECASM optimizations to move reg writes pass this point");
#endif
    ATON_ARITH_TRANSLATEADDR_SET(id, 1); // map 0x400 onto 0x0 offset for coefficients
    for (i = 0, k = 0; i < maxc; i++, k++)
    {
      uint32_t t;
      uint32_t A = Ap != NULL ? LL_ATON_getbits(Ap, bitcnt_A, nbits_A) : (uint32_t)conf->A_scalar;
      uint32_t B = Bp != NULL ? LL_ATON_getbits(Bp, bitcnt_B, nbits_B) : (uint32_t)conf->B_scalar;
      uint32_t C = Cp != NULL ? (c_sign * LL_ATON_getbits(Cp, bitcnt_C, nbits_C)) : (uint32_t)conf->C_scalar;
      if (conf->combinebc)
      {
        B = (C >> 16);
        C = (C & 0xFFFF);
      }
      if (k == mem_idx_num)
      {
#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
        ec_trace_comment("Block ECASM optimizations to move reg writes pass this point");
#endif
        ATON_ARITH_TRANSLATEADDR_SET(id, 0); // map 0xC00 onto 0xC00 offset for coefficients
        k -= (256 / 2);
      }
      t = ATON_ARITH_COEFF_AB_DT;
      t = ATON_ARITH_COEFF_AB_SET_A(t, A);
      t = ATON_ARITH_COEFF_AB_SET_B(t, B);
      ATON_ARITH_COEFF_AB_SET(id, k, t);
      t = ATON_ARITH_COEFF_C_DT;
      t = ATON_ARITH_COEFF_C_SET_C(t, C);
      ATON_ARITH_COEFF_C_SET(id, k, t);

      bitcnt_A += nbits_A;
      bitcnt_B += nbits_B;
      bitcnt_C += nbits_C;
    }
  }
  return 0;
}
#endif // ATON_ARITH_NUM

#ifdef ATON_POOL_NUM

static int32_t get_Poolacc_type(LL_Poolacc_Op op)
{
  switch (op)
  {
  case POOL_MAX:
    return ATON_POOLTYPE_MAX_POOLING;
  case POOL_MIN:
    return ATON_POOLTYPE_MIN_POOLING;
  case POOL_AVG:
    return ATON_POOLTYPE_AVG_POOLING;
  case POOL_GMAX:
    return ATON_POOLTYPE_GMAX_POOLING;
  case POOL_GMIN:
    return ATON_POOLTYPE_GMIN_POOLING;
  case POOL_GAVG:
    return ATON_POOLTYPE_GAVG_POOLING;
  default:
    break;
  }
  LL_ATON_ASSERT(0);
  return 0;
}

/**
 * @brief  Configure Pooling Accelerator
 * @param  id Pooling Accelerator identifier [0..ATON_POOL_NUM-1]
 * @param  Poolacc_InitStruct Structure describing initialization parameters
 * @retval Error code E.g.: Invalid ID, invalid parameters, not idle,..
 */
int LL_Poolacc_Init(int id, const LL_Poolacc_InitTypeDef *conf)
{
  uint32_t t;

  if (id >= ATON_POOL_NUM)
    return LL_ATON_INVALID_ID;

  LL_ATON_EnableClock(ATON_POOL_CLKB_CLK(id));

#ifdef POOL_RC14
  /* Clear pipeline */
  t = ATON_POOL_CTRL_GET(id);
  t = ATON_POOL_CTRL_SET_EN(t, 0);
  t = ATON_POOL_CTRL_SET_CLR(t, 1);
  t = ATON_POOL_CTRL_SET_CONFCLR(t, 1);
  ATON_POOL_CTRL_SET(id, t);
#endif // POOL_RC14

  /* Initialize CTRL register parameters */
  t = ATON_POOL_CTRL_DT;
  t = ATON_POOL_CTRL_SET_TYPE(t, get_Poolacc_type(conf->operation));
  t = ATON_POOL_CTRL_SET_ROUND(t, (conf->rounding_o != 0));
  t = ATON_POOL_CTRL_SET_SAT(t, (conf->saturation_o != 0));
  t = ATON_POOL_CTRL_SET_OUTSHIFT(t, conf->shift_o);
  t = ATON_POOL_CTRL_SET_FBYTES(t, conf->inbytes_f);
  t = ATON_POOL_CTRL_SET_FSHIFT(t, ATON_SHIFT(conf->shift_f));
  t = ATON_POOL_CTRL_SET_FROUND(t, (conf->rounding_f != 0));
  t = ATON_POOL_CTRL_SET_FSAT(t, (conf->saturation_f != 0));
#ifdef ATON_POOL_CTRL_SET_AVGNOPAD
  t = ATON_POOL_CTRL_SET_AVGNOPAD(t, (conf->avgnopad != 0));
#endif
  t = ATON_POOL_CTRL_SET_DUALLINE(t, conf->dualLine);
#if 0
  int crop_en = ((conf->leftCrop > 0) || (conf->rightCrop > 0) || (conf->topCrop > 0) || (conf->bottomCrop > 0));
#else
  int crop_en = 1;
#endif
  t = ATON_POOL_CTRL_SET_CROPEN(t, crop_en);
  ATON_POOL_CTRL_SET(id, t);

  /* configure remaining in/out shifters */
  t = ATON_POOL_RNDCTRL_DT;
  t = ATON_POOL_RNDCTRL_SET_FOBYTES(t, conf->outbytes_f);
  t = ATON_POOL_RNDCTRL_SET_FRNDMODE(t, conf->round_mode_f);
  t = ATON_POOL_RNDCTRL_SET_OBYTES(t, conf->outbytes_o);
  t = ATON_POOL_RNDCTRL_SET_ORNDMODE(t, (conf->relu_mode_o << 1) | conf->round_mode_o);
  ATON_POOL_RNDCTRL_SET(id, t);

  /* Configure Pooling unit dimensions register */
  t = ATON_POOL_PDIMS_DT;
  t = ATON_POOL_PDIMS_SET_WINX(t, conf->poolWinX);
  t = ATON_POOL_PDIMS_SET_WINY(t, conf->poolWinY);
  t = ATON_POOL_PDIMS_SET_STRDX(t, conf->strideX);
  t = ATON_POOL_PDIMS_SET_STRDY(t, conf->strideY);
  t = ATON_POOL_PDIMS_SET_TPAD(t, conf->topPad);
  t = ATON_POOL_PDIMS_SET_BPAD(t, conf->bottomPad);
  t = ATON_POOL_PDIMS_SET_LPAD(t, conf->leftPad);
  t = ATON_POOL_PDIMS_SET_RPAD(t, conf->rightPad);
  t = ATON_POOL_PDIMS_SET_BSIZE(t, conf->batchSize);
  ATON_POOL_PDIMS_SET(id, t);

  /* Configure Pooling unit FDIMS register */
  t = ATON_POOL_FDIMS_DT;
  t = ATON_POOL_FDIMS_SET_FEATX(t, conf->inputX * conf->batchSize);
  t = ATON_POOL_FDIMS_SET_FEATY(t, conf->inputY);
  ATON_POOL_FDIMS_SET(id, t);

  /* Configure Pooling unit OUTDIMS register */
  t = ATON_POOL_OUTDIMS_DT;
  t = ATON_POOL_OUTDIMS_SET_FEATX(t, conf->outputX);
  t = ATON_POOL_OUTDIMS_SET_FEATY(t, conf->outputY);
  ATON_POOL_OUTDIMS_SET(id, t);

  /* Configure mulval */
  t = ATON_POOL_MULVAL_DT;
  t = ATON_POOL_MULVAL_SET_MULVAL(t, conf->mulval);
  ATON_POOL_MULVAL_SET(id, t);

  /* Mulval is scattered across different registers from V3 onwards. TODO: use ATON.h macros */
#if (ATON_POOL_VERSION_MAJOR_DT >= 3)
  unsigned int wsize = conf->poolWinX * conf->poolWinY;
  if (wsize > 0 && wsize < 9)
  {
    uint32_t *mulval_addr = (uint32_t *)ATON_POOL_MULVAL_1_2_ADDR(id);
    wsize--;
    mulval_addr += (wsize / 2);
    if (wsize & 1)
      *mulval_addr |= ((conf->mulval) << 16);
    else
      *mulval_addr |= conf->mulval;
  }
#endif

  /* Configure cropping if needed */
  t = ATON_POOL_XCROP_DT;
  if (conf->leftCrop > 0)
    t = ATON_POOL_XCROP_SET_LCROP(t, conf->leftCrop * conf->batchSize);
  int r_crop = conf->rightCrop * conf->batchSize + (conf->batchSize - 1);
  if (r_crop > 0)
    t = ATON_POOL_XCROP_SET_RCROP(t, conf->rightCrop * conf->batchSize + (conf->batchSize - 1));
  ATON_POOL_XCROP_SET(id, t);

  t = ATON_POOL_YCROP_DT;
  if (conf->topCrop > 0)
    t = ATON_POOL_YCROP_SET_TCROP(t, conf->topCrop);
  if (conf->bottomCrop > 0)
    t = ATON_POOL_YCROP_SET_BCROP(t, conf->bottomCrop);
  ATON_POOL_YCROP_SET(id, t);

#ifdef ATON_POOL_USER_PAD_VALUE_SET
  if (conf->pad_val_en)
  {
    t = ATON_POOL_USER_PAD_VALUE_DT;
    t = ATON_POOL_USER_PAD_VALUE_SET_PADVAL(t, conf->pad_val);
    t = ATON_POOL_USER_PAD_VALUE_SET_PADVALEN(t, 1);
    ATON_POOL_USER_PAD_VALUE_SET(id, t);
  }
#endif

  return 0;
}
#endif // ATON_POOL_NUM

#if defined(ATON_EPOCHCTRL_NUM)
/**
 * @brief  Configure Epoch Controller
 * @param  id Epoch Controller identifier [0..ATON_EPOCHCTRL_NUM-1]
 * @param  conf Structure describing Epoch Controller initialization parameters
 * @retval Error code
 */
int LL_EpochCtrl_Init(int id, const LL_EpochCtrl_InitTypeDef *conf)
{
  uint32_t t;

  if (id >= ATON_EPOCHCTRL_NUM)
    return LL_ATON_INVALID_ID;

  LL_ATON_EnableClock(ATON_EPOCHCTRL_CLKB_CLK(id));

  /* Configure CTRL register */
  t = ATON_EPOCHCTRL_CTRL_DT;
  t = ATON_EPOCHCTRL_CTRL_SET_SM(t, conf->stepmode);
  ATON_EPOCHCTRL_CTRL_SET(id, t);

  /* Check address is 8 byte aligned */
  if (conf->blobaddr & 0x7)
    return LL_ATON_INVALID_PARAM;

  ATON_EPOCHCTRL_ADDR_SET(id, conf->blobaddr);

  return LL_ATON_OK;
}

/**
 * @brief  Trigger Epoch Controller Single Step execution
 * @param  id Epoch Controller identifier [0..ATON_EPOCHCTRL_NUM-1]
 * @retval Error code
 */
int LL_EpochCtrl_Step(int id)
{
  uint32_t t;

  t = ATON_EPOCHCTRL_IRQ_GET(id);
  t = ATON_EPOCHCTRL_IRQ_SET_SM(t, 1);
  ATON_EPOCHCTRL_IRQ_SET(id, t);

  return 0;
}

/**
 * @brief  Waits for epoch controller to become idle
 * @param  id Bitmask of EC identifiers
 * @retval Error code
 */
int LL_EpochCtrl_Wait(uint32_t mask)
{
  int i;
  uint32_t enableFlags;

  startWatchdog(ATON_EPOCH_TIMEOUT);

  do
  {
    enableFlags = 0;
    for (i = 0; i < ATON_EPOCHCTRL_NUM; i++)
    {
      if (mask & (1 << i))
      {
        enableFlags |= (ATON_EPOCHCTRL_CTRL_GET(i) & (1U << ATON_EPOCHCTRL_CTRL_RUNNING_LSB));
      }
    }

    LL_ATON_ASSERT(checkWatchdog() == 0);

  } while (enableFlags);

  return LL_ATON_OK;
}

/**
 * @brief  Returns the size of a blob
 * @param  eb_addr Pointer to blob
 * @retval Size in bytes of the blob
 */
unsigned int LL_EpochCtrl_GetBlobSize(uint32_t *eb_addr)
{
  unsigned bloblines = eb_addr[1] + 2;
  return bloblines * 4;
}
#endif // ATON_EPOCHCTRL_NUM

void LL_ATON_EnableClock(unsigned int clock)
{
#if (LL_ATON_ENABLE_CLOCK_GATING == 1)
  ATON_REG_WRITE_FIELD_RANGE(CLKCTRL, 0, BGATES, clock, 1, 1);
#endif
}

void LL_ATON_DisableClock(unsigned int clock)
{
#if (LL_ATON_ENABLE_CLOCK_GATING == 1)
  ATON_REG_WRITE_FIELD_RANGE(CLKCTRL, 0, BGATES, clock, 1, 0);
#endif
}

/**
 * @brief  DMA version of a memcpy functionality, this function could be overloaded if a system DMA could be used
 * @param  dst destination memory address
 * @param  src source memory address
 * @param  src_limit memory pool end address of `src`
 * @param  n number of bytes to be transferred
 * @param dst_cached Destination under cache flag
 * @param dst_cached Source under cache flag
 * @retval Error code E.g.: Invalid ID, invalid parameters, not idle,..
 *
 * @note:  This function completely undermines any possibility for integrating correctly
 *         SW operators (or any other functionality which calls this function) in any of the three ATON runtime
 * scheduling modes. In other words, function `LL_ATON_Dma_memcpy()` and its usage are incompatible with the ATON
 * runtime. Therefore either `memcpy()` should be used in its place or calls to `LL_ATON_Dma_memcpy()` need to be
 * transformed in a sequence of "epoch blocks" which can be integrated with the ATON runtime (as an example see the
 * ATON-accelerated implementation of operator `Concat`)!
 */
/* N.B. assumes DMA0 and 1 are free to use */
#define MIN_BUFF_LEN 40
LL_ATON_WEAK void *LL_ATON_Dma_memcpy(void *dst, void *src, void *src_limit, size_t n, int dst_cached, int src_cached)
{
  uint8_t *_dst_orig = dst;

  int prolog_len = (n % 3);
  int i;
  uint8_t *_dst = (uint8_t *)dst;
  uint8_t *_src = (uint8_t *)src;

  if (n < MIN_BUFF_LEN)
    prolog_len = n; // not worth it ...
  // LL_ATON_PRINTF("n=%d prolog=%d\n",n,prolog_len);

  for (i = 0; i < prolog_len; i++)
    *_dst++ = *_src++;
  n -= prolog_len;

  if (prolog_len > 0)
  {
    /* *** MCU cache clean & invalidate operation (SW) *** */
    LL_ATON_Cache_MCU_Clean_Invalidate_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)_dst_orig), prolog_len);
  }

  if (n > 0)
  {
    LL_Streng_TensorInitTypeDef dma_in = {
        .dir = 0,
        .addr_base = {_src},
        .offset_start = 0,
        .offset_end = n,
        .offset_limit = ((uintptr_t)src_limit - (uintptr_t)src), // awful FIXME Francesco
        .raw = 1,
        .frame_count = 0,
        .fwidth = 0,
        .fheight = 0,
        .batch_depth = 0,
        .batch_offset = 0,
        .frame_offset = n,
        .line_offset = 0,
        .loop_offset = 0,
        .frame_loop_cnt = 0,
        .frame_tot_cnt = 1,
        .nbits_in = 24,
        .nbits_out = 24,
        .nbits_unsigned = 0,
        .align_right = 0,
        .noblk = 0,
    };

    LL_Streng_TensorInitTypeDef dma_out = {
        .dir = 1,
        .addr_base = {_dst},
        .offset_start = 0,
        .offset_end = n,
        .raw = 1,
        .frame_count = 0,
        .fwidth = 0,
        .fheight = 0,
        .batch_depth = 0,
        .batch_offset = 0,
        .frame_offset = n,
        .line_offset = 0,
        .loop_offset = 0,
        .frame_loop_cnt = 0,
        .frame_tot_cnt = 1,
        .nbits_in = 24,
        .nbits_out = 24,
        .nbits_unsigned = 0,
        .align_right = 0,
        .noblk = 0,
    };

    if (src_cached)
    {
      dma_in.cacheable = 1;
      dma_in.cache_allocate = 1;
    }

    if (dst_cached)
    {
      dma_out.cacheable = 1;
      dma_out.cache_allocate = 1;
    }

    const LL_Switch_InitTypeDef switch_init = {LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0),
                                               LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0),
                                               LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0};
    const LL_ATON_EnableUnits_InitTypeDef dma_units[] = {{{STRENG, 1}}, {{STRENG, 0}}};
    const uint32_t dma_wait_mask = 0x2;

    LL_Streng_TensorInit(0, &dma_in, 1);
    LL_Streng_TensorInit(1, &dma_out, 1);
    LL_Switch_Init(&switch_init, 1);
    LL_ATON_EnableUnits_Init(dma_units, 2);
    LL_Streng_Wait(dma_wait_mask);
    LL_ATON_DisableUnits_Init(dma_units, 1);
    LL_Switch_Deinit(&switch_init, 1);
  }

  return dst;
}

#if (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
static void *ec_aton_base;

uintptr_t get_ec_aton_base(void)
{
  return (uintptr_t)ec_aton_base;
}

void initialize_ec_aton_base(void)
{
  ec_aton_base = malloc(ATON_SIZE);
}

/* Called by patched ATON.h to trace ATON register stores */
void ec_trace_write(uintptr_t dstreg, unsigned int val)
{
  uint32_t IP_id = ec_trace_get_IP_id(dstreg);
  uint32_t regaddr_offset = dstreg - (uintptr_t)ATON_BASE;
  uint32_t REG_id = ec_trace_get_REG_id(regaddr_offset);
  ec_trace_reg_write(IP_id, REG_id, val);
}

void ec_trace_write_reloc(uintptr_t dstreg, unsigned int base, unsigned int offset)
{
  uint32_t IP_id = ec_trace_get_IP_id(dstreg);
  uint32_t regaddr_offset = dstreg - (uintptr_t)ATON_BASE;
  uint32_t REG_id = ec_trace_get_REG_id(regaddr_offset);
  ec_trace_reg_write_reloc(IP_id, REG_id, base, offset);
}

unsigned int ec_trace_get_IP_id(uintptr_t dstreg)
{
  uint32_t regaddr_offset = dstreg - (uintptr_t)ATON_BASE;
  uint32_t IP_id = (uint32_t)((regaddr_offset & 0x0ffff000) >> 12);
  return IP_id;
}
unsigned int ec_trace_get_REG_id(unsigned int regoffset)
{
  return (regoffset & 0xfff) >> 2;
}

void ec_trace_wait_epoch_end(uint32_t wait_mask)
{
  // TODO: Only polling mode supported at the moment
  for (unsigned i = 0; i < 32; i++)
  {
    if (wait_mask & (1 << i))
    {
      ATON_REG_POLL(STRENG, i, CTRL, RUNNING, 0);
    }
  }
}

#endif
