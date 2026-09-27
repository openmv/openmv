/**
  ******************************************************************************
  * @file    vd6g.c
  * @author  MDG Application Team
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

#include "vd6g.h"

#include <assert.h>
#include <stddef.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#include "vd6g_patch.c"
#include "vd6g_vtpatch.c"

#define VD6G_REG_MODEL_ID                    0x0000
  #define VD6G_MODEL_ID                      0x5603
#define VD6G_REG_REVISION                    0x0002
  #define VD6G_TOP_DIE_SLOW_BOOT             0x020
  #define VD6G_TOP_DIE_FAST_BOOT             0x031
#define VD6G_REG_ROM_REVISION                0x0014
#define VD6G_REG_UI_REVISION                 0x0018
#define VD6G_ERROR_CODE                      0x001c
#define VD6G_REG_OPTICAL_REVISION            0x001a
#define VD6G_REG_FWPATCH_REVISION            0x001e
#define VD6G_REG_VTIMING_RD_REVISION         0x0020
#define VD6G_REG_VTIMING_GR_REVISION         0x0024
#define VD6G_REG_VTIMING_GT_REVISION         0x0026
#define VD6G_REG_SYSTEM_FSM                  0x0028
  #define VD6G_SYSTEM_FSM_READY_TO_BOOT      0x01
  #define VD6G_SYSTEM_FSM_SW_STBY            0x02
  #define VD6G_SYSTEM_FSM_STREAMING          0x03
#define VD6G_REG_BOOT                        0x0200
  #define VD6G_CMD_ACK                       0
  #define VD6G_CMD_BOOT                      1
  #define VD6G_CMD_PATCH_SETUP               2
#define VD6G_REG_STBY                        0x0201
  #define VD6G_CMD_START_STREAM              1
#define VD6G_REG_STREAMING                   0x0202
  #define VD6G_CMD_STOP_STREAM               1
#define VD6G_REG_VTPATCHING                  0x0203
  #define VD6G_CMD_START_VTRAM_UPDATE        1
  #define VD6G_CMD_END_VTRAM_UPDATE          2
#define VD6G_REG_EXT_CLOCK                   0x0220
#define VD6G_REG_CLK_PLL_PREDIV              0x0224
#define VD6G_REG_CLK_SYS_PLL_MULT            0x0226
#define VD6G_LINE_LENGTH                     0x0300
#define VD6G_REG_ORIENTATION                 0x0302
#define VD6G_REG_FORMAT_CTRL                 0x030a
#define VD6G_REG_OIF_CTRL                    0x030c
#define VD6G_REG_OIF_IMG_CTRL                0x030f
  #define VD6G_RAW8_DATA_TYPE                0x2a
  #define VD6G_RAW10_DATA_TYPE               0x2b
#define VD6G_REG_OIF_CSI_BITRATE             0x0312
#define VD6G_REG_DUSTER_CTRL                 0x0318
  #define VD6G_DUSTER_CTRL_DISABLE           0
#define VD6G_REG_DARKCAL_CTRL                0x0340
  #define VD6G_DARKCAL_CTRL_DISABLE          0
  #define VD6G_DARKCAL_CTRL_ENABLE           1
  #define VD6G_DARKCAL_CTRL_DISABLE_DARKAVG  2
#define VD6G_REG_PATGEN_CTRL                 0x0400
  #define VD6G_PATGEN_CTRL_DISABLE           0x0000
  #define VD6G_PATGEN_CTRL_SOLID_COLOR       0x0011
  #define VD6G_PATGEN_CTRL_VER_COLOR_BAR     0x0021
  #define VD6G_PATGEN_CTRL_HOR_GRAY          0x0101
  #define VD6G_PATGEN_CTRL_VER_GRAY          0x0111
  #define VD6G_PATGEN_CTRL_DIAG_GRAY         0x0121
  #define VD6G_PATGEN_CTRL_PSN               0x0131
#define VD6G_REG_AE_COMPILER_CONTROL         0x0430
  #define AE_COMP_CTRL_GAIN_PRIO             0
  #define AE_COMP_CTRL_FLICKER_50_HZ         1
  #define AE_COMP_CTRL_FLICKER_60_HZ         3
#define VD6G_REG_AE_TARGET_PERCENTAGE        0x043c
#define VD6G_REG_EXP_MODE                    0x044c
  #define VD6G_EXP_AUTO                      0
  #define VD6G_EXP_FREEZE                    1
  #define VD6G_EXP_MANUAL                    2
#define VD6G_REG_MANUAL_ANALOG_GAIN          0x044d
#define VD6G_REG_MANUAL_COARSE_EXP           0x044e
#define VD6G_REG_MANUAL_CH0_DIGITAL_GAIN     0x0450
#define VD6G_REG_MANUAL_CH1_DIGITAL_GAIN     0x0452
#define VD6G_REG_MANUAL_CH2_DIGITAL_GAIN     0x0454
#define VD6G_REG_MANUAL_CH3_DIGITAL_GAIN     0x0456
#define VD6G_FRAME_LENGTH                    0x0458
#define VD6G_REG_OUT_ROI_X_START             0x045e
#define VD6G_REG_OUT_ROI_X_END               0x0460
#define VD6G_REG_OUT_ROI_Y_START             0x0462
#define VD6G_REG_OUT_ROI_Y_END               0x0464
#define VD6G_REG_GPIO_x_CTRL(_i_)            (0x0467 + _i_)
  #define VD6G_REG_GPIO_INPUT                1
#define VD6G_REG_READOUT_CTRL                0x047e
#define VD6G_REG_EXP_COARSE_INTG_MARGIN      0x0946
  #define VD6G_MIN_EXP_COARSE_INTG_MARGIN    68
  #define VD6G_MIN_MANUAL_EXP_COARSE         21
  #define VD6G_MAX_MANUAL_EXP_COARSE_OFFSET  7
#define VD6G_DPHYTX_CTRL                     0x0956
  #define DBG_CONT_MODE_DISABLED             0x0c
  #define DBG_CONT_MODE_ENABLED              0x1c
#define VD6G_REG_MAX_DG                      0x095E
#define VD6G_REG_MAX_AG_CODED                0x0960
  #define VD6G_REG_MAX_AG_CODED_MASK         0x001f
#define VD6G_REG_MIN_DG                      0x097C
#define VD6G_REG_MIN_AG_CODED                0x097E
  #define VD6G_REG_MIN_AG_CODED_MASK         0x001f

#define VD6G_TARGET_PLL                      804000000UL

#define M_HZ                                 (1000 * 1000)
#define VD6G_PIXEL_CLOCK                     160800000

#define CHECK_RANGE(x, min, max)             (((x) >= (min) && (x) <= (max)) ? 0 : -1)
#define CEIL(num)                            ((num) == (int)(num) ? (int)(num) : (num) > 0 ? (int)((num) + 1) : (int)(num))

enum vdg_bin_mode {
  VD6G_BIN_MODE_NORMAL,
  VD6G_BIN_MODE_DIGITAL_X2,
  VD6G_BIN_MODE_DIGITAL_X4,
};

struct vdg_rect {
  int32_t left;
  int32_t top;
  uint32_t width;
  uint32_t height;
};

struct vdg_mode {
  uint32_t width;
  uint32_t height;
  enum vdg_bin_mode bin_mode;
  struct vdg_rect crop;
};

static const struct vdg_mode vdg_supported_modes[] = {
  {
    .width = VD6G_MAX_WIDTH,
    .height = VD6G_MAX_HEIGHT,
    .bin_mode = VD6G_BIN_MODE_NORMAL,
    .crop = {
      .left = 2,
      .top = 0,
      .width = VD6G_MAX_WIDTH,
      .height = VD6G_MAX_HEIGHT,
    },
  },
  {
    .width = 1024,
    .height = 1280,
    .bin_mode = VD6G_BIN_MODE_NORMAL,
    .crop = {
      .left = 50,
      .top = 42,
      .width = 1024,
      .height = 1280,
    },
  },
  {
    .width = 1024,
    .height = 768,
    .bin_mode = VD6G_BIN_MODE_NORMAL,
    .crop = {
      .left = 50,
      .top = 298,
      .width = 1024,
      .height = 768,
    },
  },
  {
    .width = 768,
    .height = 1024,
    .bin_mode = VD6G_BIN_MODE_NORMAL,
    .crop = {
      .left = 178,
      .top = 170,
      .width = 768,
      .height = 1024,
    },
  },
  {
    .width = 720,
    .height = 1280,
    .bin_mode = VD6G_BIN_MODE_NORMAL,
    .crop = {
      .left = 202,
      .top = 42,
      .width = 720,
      .height = 1280,
    },
  },
  {
    .width = 640,
    .height = 480,
    .bin_mode = VD6G_BIN_MODE_NORMAL,
    .crop = {
      .left = 242,
      .top = 442,
      .width = 640,
      .height = 480,
    },
  },
  {
    .width = 480,
    .height = 640,
    .bin_mode = VD6G_BIN_MODE_DIGITAL_X2,
    .crop = {
      .left = 82,
      .top = 42,
      .width = 960,
      .height = 1280,
    },
  },
  {
    .width = 320,
    .height = 240,
    .bin_mode = VD6G_BIN_MODE_DIGITAL_X2,
    .crop = {
      .left = 242,
      .top = 442,
      .width = 640,
      .height = 480,
    },
  },
  {
    .width = VD6G_MAX_WIDTH,
    .height = 720,
    .bin_mode = VD6G_BIN_MODE_NORMAL,
    .crop = {
      .left = 2,
      .top = 322,
      .width = VD6G_MAX_WIDTH,
      .height = 720,
    },
  },
};

#define VD6G_TraceError(_ctx_,_ret_) do { \
  if (_ret_) VD6G_error(_ctx_, "Error on %s:%d : %d\n", __func__, __LINE__, _ret_); \
  if (_ret_) display_error(_ctx_); \
  if (_ret_) return _ret_; \
} while(0)

static void VD6G_dbg(VD6G_Ctx_t *ctx, int lvl, const char *format, ...)
{
  va_list ap;

  if (!ctx->log)
    return ;

  va_start(ap, format);
  ctx->log(ctx, VD6G_LVL_DBG(lvl), format, ap);
  va_end(ap);
}

static void VD6G_notice(VD6G_Ctx_t *ctx, const char *format, ...)
{
  va_list ap;

  if (!ctx->log)
    return ;

  va_start(ap, format);
  ctx->log(ctx, VD6G_LVL_NOTICE, format, ap);
  va_end(ap);
}

static void VD6G_warn(VD6G_Ctx_t *ctx, const char *format, ...)
{
  va_list ap;

  if (!ctx->log)
    return ;

  va_start(ap, format);
  ctx->log(ctx, VD6G_LVL_WARNING, format, ap);
  va_end(ap);
}

static void VD6G_error(VD6G_Ctx_t *ctx, const char *format, ...)
{
  va_list ap;

  if (!ctx->log)
    return ;

  va_start(ap, format);
  ctx->log(ctx, VD6G_LVL_ERROR, format, ap);
  va_end(ap);
}

static void display_error(VD6G_Ctx_t *ctx)
{
  uint16_t reg16;
  int ret;

  ret = ctx->read16(ctx, VD6G_ERROR_CODE, &reg16);
  assert(ret == 0);
  VD6G_error(ctx, "ERROR_CODE : 0x%04x\n", reg16);
}

static int VD6G_PollReg8(VD6G_Ctx_t *ctx, uint16_t addr, uint8_t poll_val)
{
  const unsigned int loop_delay_ms = 10;
  const unsigned int timeout_ms = 500;
  int loop_nb = timeout_ms / loop_delay_ms;
  uint8_t val;
  int ret;

  while (--loop_nb) {
    ret = ctx->read8(ctx, addr, &val);
    VD6G_TraceError(ctx, ret);
    if (val == poll_val)
      return 0;
    ctx->delay(ctx, loop_delay_ms);
  }

  return -1;
}

static int VD6G_GetLineTimeInNs(VD6G_Ctx_t *ctx, uint32_t *line_time_in_ns)
{
  uint16_t line_len;
  int ret;

  ret = ctx->read16(ctx, VD6G_LINE_LENGTH, &line_len);
  VD6G_TraceError(ctx, ret);

  /* compute line_time_in_ns */
  *line_time_in_ns = ((uint64_t)line_len * 1000000000) / VD6G_PIXEL_CLOCK;

  return 0;
}

static int VD6G_IsStreaming(VD6G_Ctx_t *ctx)
{
  uint8_t state;
  int ret;

  ret = ctx->read8(ctx, VD6G_REG_SYSTEM_FSM, &state);
  VD6G_TraceError(ctx, ret);

  return state == VD6G_SYSTEM_FSM_STREAMING;
}

static int VD6G_WaitState(VD6G_Ctx_t *ctx, int state)
{
  int ret = VD6G_PollReg8(ctx, VD6G_REG_SYSTEM_FSM, state);

  if (ret)
    VD6G_warn(ctx, "Unable to reach state %d\n", state);
  else
    VD6G_dbg(ctx, 0, "reach state %d\n", state);

  return ret;
}

static int VD6G_GetTopDie(VD6G_Ctx_t *ctx, uint8_t *top_die)
{
  uint16_t reg16;
  int ret;

  ret = ctx->read16(ctx, VD6G_REG_REVISION, &reg16);
  VD6G_TraceError(ctx, ret);
  VD6G_dbg(ctx, 0, "revision = 0x%04x\n", reg16);
  *top_die = reg16 >> 8;

  return 0;
}

static int VD6G_CheckModelId(VD6G_Ctx_t *ctx)
{
  uint8_t top_die;
  uint16_t reg16;
  int ret;

  ret = ctx->read16(ctx, VD6G_REG_MODEL_ID, &reg16);
  VD6G_TraceError(ctx, ret);
  VD6G_dbg(ctx, 0, "model_id = 0x%04x\n", reg16);
  if (reg16 != VD6G_MODEL_ID) {
    VD6G_error(ctx, "Bad model id expected 0x%04x / got 0x%04x\n", VD6G_MODEL_ID, reg16);
    return -1;
  }

  ret = VD6G_GetTopDie(ctx, &top_die);
  if (ret)
    return ret;
  if (top_die != VD6G_TOP_DIE_SLOW_BOOT && top_die != VD6G_TOP_DIE_FAST_BOOT) {
    VD6G_error(ctx, "Unsupported revision\n");
    return -1;
  }

  ret = ctx->read16(ctx, VD6G_REG_ROM_REVISION, &reg16);
  VD6G_TraceError(ctx, ret);
  VD6G_dbg(ctx, 0, "rom = 0x%04x\n", reg16);

  return 0;
}

static int VD6G_ApplyPatchSlowBoot(VD6G_Ctx_t *ctx)
{
  uint8_t patch_major;
  uint8_t patch_minor;
  uint16_t reg16;
  int ret;

  patch_major = patch_cut2[3];
  patch_minor = patch_cut2[2];

  ret = ctx->write_array(ctx, 0x2000, (uint8_t *) patch_cut2, sizeof(patch_cut2));
  VD6G_TraceError(ctx, ret);

  ret = ctx->write8(ctx, VD6G_REG_BOOT, VD6G_CMD_PATCH_SETUP);
  VD6G_TraceError(ctx, ret);

  ret = VD6G_PollReg8(ctx, VD6G_REG_BOOT, VD6G_CMD_ACK);
  if (ret)
    return ret;

  ret = ctx->read16(ctx, VD6G_REG_FWPATCH_REVISION, &reg16);
  VD6G_TraceError(ctx, ret);
  VD6G_dbg(ctx, 0, "patch = 0x%04x\n", reg16);

  if (reg16 != (patch_major << 8) + patch_minor) {
    VD6G_error(ctx, "bad patch version expected %d.%d got %d.%d\n", patch_major, patch_minor,
                 reg16 >> 8, reg16 & 0xff);
    return -1;
  }
  VD6G_notice(ctx, "patch %d.%d applied\n", reg16 >> 8, reg16 & 0xff);

  return 0;
}

static int VD6G_ApplyPatch(VD6G_Ctx_t *ctx)
{
  uint8_t top_die;
  int ret;

  ret = VD6G_GetTopDie(ctx, &top_die);
  if (ret)
    return ret;

  switch (top_die) {
  case VD6G_TOP_DIE_SLOW_BOOT:
    ret = VD6G_ApplyPatchSlowBoot(ctx);
    break;
  case VD6G_TOP_DIE_FAST_BOOT:
    ret = 0;
    break;
  default:
    assert(0);
  }

  return ret;
}

static int VD6G_BootMcu(VD6G_Ctx_t *ctx)
{
  int ret;

  ret = ctx->write8(ctx, VD6G_REG_BOOT, VD6G_CMD_BOOT);
  VD6G_TraceError(ctx, ret);

  ret = VD6G_PollReg8(ctx, VD6G_REG_BOOT, VD6G_CMD_ACK);
  if (ret)
    return ret;

  ret = VD6G_WaitState(ctx, VD6G_SYSTEM_FSM_SW_STBY);
  if (ret)
    return ret;

  VD6G_notice(ctx, "sensor boot successfully\n");

  return 0;
}

static int VD6G_ApplyVtPatchSlowBoot(VD6G_Ctx_t *ctx)
{
  uint8_t cur_vtpatch_rd_rev, cur_vtpatch_gr_rev, cur_vtpatch_gt_rev;
  int vtpatch_offset = 0;
  int ret;
  int i;

  ret = ctx->write8(ctx, VD6G_REG_VTPATCHING, VD6G_CMD_START_VTRAM_UPDATE);
  VD6G_TraceError(ctx, ret);

  ret = VD6G_PollReg8(ctx, VD6G_REG_VTPATCHING, VD6G_CMD_ACK);
  if (ret)
    return ret;

  ret = VD6G_WaitState(ctx, VD6G_SYSTEM_FSM_SW_STBY);
  if (ret)
    return ret;

  for (i = 0; i < vtpatch_area_nb; i++) {
    ret = ctx->write_array(ctx, vtpatch_desc[i].offset, (uint8_t *) (vtpatch + vtpatch_offset),
                           vtpatch_desc[i].size);
    VD6G_TraceError(ctx, ret);
    vtpatch_offset += vtpatch_desc[i].size;
  }
  ret = ctx->write8(ctx, 0xd9f8, VT_REVISION);
  VD6G_TraceError(ctx, ret);
  ret = ctx->write8(ctx, 0xaffc, VT_REVISION);
  VD6G_TraceError(ctx, ret);
  ret = ctx->write8(ctx, 0xbbb4, VT_REVISION);
  VD6G_TraceError(ctx, ret);
  ret = ctx->write8(ctx, 0xb898, VT_REVISION);
  VD6G_TraceError(ctx, ret);

  ret = ctx->write8(ctx, VD6G_REG_VTPATCHING, VD6G_CMD_END_VTRAM_UPDATE);
  VD6G_TraceError(ctx, ret);

  ret = VD6G_PollReg8(ctx, VD6G_REG_VTPATCHING, VD6G_CMD_ACK);
  if (ret)
    return ret;

  ret = VD6G_WaitState(ctx, VD6G_SYSTEM_FSM_SW_STBY);
  if (ret)
    return ret;

  ret = ctx->read8(ctx, VD6G_REG_VTIMING_RD_REVISION, &cur_vtpatch_rd_rev);
  VD6G_TraceError(ctx, ret);
  ret = ctx->read8(ctx, VD6G_REG_VTIMING_GR_REVISION, &cur_vtpatch_gr_rev);
  VD6G_TraceError(ctx, ret);
  ret = ctx->read8(ctx, VD6G_REG_VTIMING_GT_REVISION, &cur_vtpatch_gt_rev);
  VD6G_TraceError(ctx, ret);

  if (cur_vtpatch_rd_rev != VT_REVISION ||
      cur_vtpatch_gr_rev != VT_REVISION ||
      cur_vtpatch_gt_rev != VT_REVISION) {
    VD6G_error(ctx, "bad vtpatch version, expected %d got rd:%d, gr:%d gt:%d\n", VT_REVISION, cur_vtpatch_rd_rev,
                 cur_vtpatch_gr_rev, cur_vtpatch_gt_rev);
    return -1;
  }
  VD6G_notice(ctx, "VT patch %d applied\n", VT_REVISION);

  return 0;
}

static int VD6G_ApplyVtPatch(VD6G_Ctx_t *ctx)
{
  uint8_t top_die;
  int ret;

  ret = VD6G_GetTopDie(ctx, &top_die);
  if (ret)
    return ret;

  switch (top_die) {
  case VD6G_TOP_DIE_SLOW_BOOT:
    ret = VD6G_ApplyVtPatchSlowBoot(ctx);
    break;
  case VD6G_TOP_DIE_FAST_BOOT:
    ret = 0;
    break;
  default:
    assert(0);
  }

  return ret;
}

static int VD6G_SetBayerType(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  uint8_t reg8;
  int ret;

  ret = ctx->read8(ctx, VD6G_REG_OPTICAL_REVISION, &reg8);
  VD6G_TraceError(ctx, ret);

  if (!(reg8 & 1)) {
    ctx->bayer = VD6G_BAYER_NONE;
    return 0;
  }

  switch (drv_ctx->config_save.flip_mirror_mode) {
  case VD6G_MIRROR_FLIP_NONE:
    ctx->bayer = VD6G_BAYER_GRBG;
    break;
  case VD6G_FLIP:
    ctx->bayer = VD6G_BAYER_BGGR;
    break;
  case VD6G_MIRROR:
    ctx->bayer = VD6G_BAYER_RGGB;
    break;
  case VD6G_MIRROR_FLIP:
    ctx->bayer = VD6G_BAYER_GBRG;
    break;
  default:
    assert(0);
  }

  return 0;
}

static int VD6G_SetExposureModeInternal(VD6G_Ctx_t *ctx, VD6G_ExposureMode_t mode)
{
  int ret;

  if ((mode != VD6G_EXPOSURE_AUTO) &&
      (mode != VD6G_EXPOSURE_FREEZE_AEALGO) &&
      (mode != VD6G_EXPOSURE_MANUAL)) {
    return -1;
  }

  ret = ctx->write8(ctx, VD6G_REG_EXP_MODE, mode);
  VD6G_TraceError(ctx, ret);

  return 0;
}

static int VD6G_Gpios(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  int ret;
  int i;

  for (i = 0; i < VD6G_GPIO_NB; i++) {
    ret = ctx->write8(ctx, VD6G_REG_GPIO_x_CTRL(i), drv_ctx->config_save.gpio_ctrl[i]);
    VD6G_TraceError(ctx, ret);
  }

  return 0;
}

static int VD6G_Boot(VD6G_Ctx_t *ctx)
{
  int ret;

  ret = VD6G_WaitState(ctx, VD6G_SYSTEM_FSM_READY_TO_BOOT);
  if (ret)
    return ret;

  ret = VD6G_CheckModelId(ctx);
  if (ret)
    return ret;

  ret = VD6G_ApplyPatch(ctx);
  if (ret)
    return ret;

  ret = VD6G_BootMcu(ctx);
  if (ret)
    return ret;

  ret = VD6G_ApplyVtPatch(ctx);
  if (ret)
    return ret;

  ret = VD6G_SetBayerType(ctx);
  if (ret)
    return ret;

  ret = VD6G_Gpios(ctx);
  if (ret)
    return ret;
  return 0;
}

static int VD6G_ComputeClocks(VD6G_Ctx_t *ctx, uint32_t ext_clock, uint8_t *pll_prediv, uint8_t *pll_mult)
{
  const unsigned int predivs[] = { 1, 2, 4 };
  int i;

  if (ext_clock < 6 * M_HZ ||
      ext_clock > 27 * M_HZ) {
    VD6G_error(ctx, "External clock out of rangen\n");
    return -1;
  }

  /* PLL input should be in [6Mhz-12Mhz[ */
  for (i = 0; i < ARRAY_SIZE(predivs); i++) {
    *pll_prediv = predivs[i];
    if (ext_clock / *pll_prediv < 12 * M_HZ)
      break;
  }

  /* PLL output clock must be as close as possible to 804Mhz */
  *pll_mult = (VD6G_TARGET_PLL * *pll_prediv + ext_clock / 2) / ext_clock;

  VD6G_dbg(ctx, 0, "Ext Clock = %d Hz\n", ext_clock);
  VD6G_dbg(ctx, 0, "PLL prediv = %d\n", *pll_prediv);
  VD6G_dbg(ctx, 0, "PLL mult = %d\n", *pll_mult);

  return 0;
}

static int VD6G_SetupClocks(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  uint8_t pll_prediv;
  uint8_t pll_mult;
  int ret;

  ret = VD6G_ComputeClocks(ctx, drv_ctx->config_save.ext_clock_freq_in_hz, &pll_prediv, &pll_mult);
  if (ret)
    return ret;

  ret = ctx->write32(ctx, VD6G_REG_EXT_CLOCK, drv_ctx->config_save.ext_clock_freq_in_hz);
  VD6G_TraceError(ctx, ret);

  ret = ctx->write8(ctx, VD6G_REG_CLK_PLL_PREDIV, pll_prediv);
  VD6G_TraceError(ctx, ret);

  ret = ctx->write8(ctx, VD6G_REG_CLK_SYS_PLL_MULT, pll_mult);
  VD6G_TraceError(ctx, ret);

  if (drv_ctx->config_save.line_len) {
    ret = ctx->write16(ctx, VD6G_LINE_LENGTH, drv_ctx->config_save.line_len);
    VD6G_TraceError(ctx, ret);
  }

  return 0;
}

static int VD6G_SetupOutput(VD6G_Ctx_t *ctx)
{
  VD6G_OutItf_Config_t *out_itf = &ctx->ctx.config_save.out_itf;
  uint8_t pixel_depth = ctx->ctx.config_save.pixel_depth;
  uint8_t data_type;
  uint16_t oif_ctrl;
  int ret;

  if (out_itf->datalane_nb != 1 && out_itf->datalane_nb != 2)
    return -1;
  /* Be sure we got value 0 or 1 */
  out_itf->clock_lane_swap_enable = !!out_itf->clock_lane_swap_enable;
  out_itf->data_lane0_swap_enable = !!out_itf->data_lane0_swap_enable;
  out_itf->data_lane1_swap_enable = !!out_itf->data_lane1_swap_enable;
  out_itf->data_lanes_mapping_swap_enable = !!out_itf->data_lanes_mapping_swap_enable;

  if (pixel_depth == 10)
    data_type = VD6G_RAW10_DATA_TYPE;
  else
    data_type = VD6G_RAW8_DATA_TYPE;

  ret = ctx->write8(ctx, VD6G_REG_FORMAT_CTRL, pixel_depth);
  VD6G_TraceError(ctx, ret);

  /* csi lanes */
  oif_ctrl = out_itf->data_lane1_swap_enable << 9 |
             !out_itf->data_lanes_mapping_swap_enable << 7 |
             out_itf->data_lane0_swap_enable << 6 |
             out_itf->data_lanes_mapping_swap_enable << 4 |
             out_itf->clock_lane_swap_enable << 3 |
             out_itf->datalane_nb << 0;
  ret = ctx->write16(ctx, VD6G_REG_OIF_CTRL, oif_ctrl);
  VD6G_TraceError(ctx, ret);

  /* csi speed */
  ret = ctx->write16(ctx, VD6G_REG_OIF_CSI_BITRATE, 804);
  VD6G_TraceError(ctx, ret);

  /* data type */
  ret = ctx->write8(ctx, VD6G_REG_OIF_IMG_CTRL, data_type);
  VD6G_TraceError(ctx, ret);

  return 0;
}

static const struct vdg_mode *VD6G_Resolution2Mode(VD6G_Res_t resolution)
{
  switch (resolution) {
  case VD6G_RES_QVGA_320_240:
    return &vdg_supported_modes[7];
    break;
  case VD6G_RES_VGA_640_480:
    return &vdg_supported_modes[5];
    break;
  case VD6G_RES_VGA_PORTRAIT_480_640:
    return &vdg_supported_modes[6];
    break;
  case VD6G_RES_XGA_1024_768:
    return &vdg_supported_modes[2];
    break;
  case VD6G_RES_XGA_PORTRAIT_768_1024:
    return &vdg_supported_modes[3];
    break;
  case VD6G_RES_720P_PORTRAIT_720_1280:
    return &vdg_supported_modes[4];
    break;
  case VD6G_RES_SXGA_PORTRAIT_1024_1280:
    return &vdg_supported_modes[1];
    break;
  case VD6G_RES_PORTRAIT_1120_720:
    return &vdg_supported_modes[8];
    break;
  case VD6G_RES_FULL_1120_1364:
    return &vdg_supported_modes[0];
    break;
  default:
    return NULL;
  }
}

static int VD6G_SetupSize(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  const struct vdg_mode *mode;
  int ret;

  mode = VD6G_Resolution2Mode(drv_ctx->config_save.resolution);
  if (!mode)
    return -1;

  ret = ctx->write8(ctx, VD6G_REG_READOUT_CTRL, mode->bin_mode);
  VD6G_TraceError(ctx, ret);

  ret = ctx->write16(ctx, VD6G_REG_OUT_ROI_X_START, mode->crop.left);
  VD6G_TraceError(ctx, ret);
  ret = ctx->write16(ctx, VD6G_REG_OUT_ROI_X_END, mode->crop.left + mode->crop.width - 1);
  VD6G_TraceError(ctx, ret);

  ret = ctx->write16(ctx, VD6G_REG_OUT_ROI_Y_START, mode->crop.top);
  VD6G_TraceError(ctx, ret);
  ret = ctx->write16(ctx, VD6G_REG_OUT_ROI_Y_END, mode->crop.top + mode->crop.height - 1);
  VD6G_TraceError(ctx, ret);

  return 0;
}

static int VD6G_SetupFrameRate(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  uint16_t frame_length;
  uint16_t line_length;
  int ret;

  ret = ctx->read16(ctx, VD6G_LINE_LENGTH, &line_length);
  VD6G_TraceError(ctx, ret);
  VD6G_dbg(ctx, 0, "Line len = %d\n", line_length);

  frame_length = VD6G_PIXEL_CLOCK / (line_length * drv_ctx->config_save.frame_rate);
  VD6G_dbg(ctx, 0, "Set frame_length to %d to reach %d fps\n", frame_length, drv_ctx->config_save.frame_rate);
  ret = ctx->write16(ctx, VD6G_FRAME_LENGTH, frame_length);
  VD6G_TraceError(ctx, ret);

  return 0;
}

static int VD6G_SetupExposure(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;

  return VD6G_SetExposureModeInternal(ctx, drv_ctx->config_save.exposure_mode);
}

static int VD6G_SetupMirrorFlip(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  uint8_t mode;
  int ret;

  switch (drv_ctx->config_save.flip_mirror_mode) {
  case VD6G_MIRROR_FLIP_NONE:
    mode = 0;
    break;
  case VD6G_FLIP:
    mode = 2;
    break;
  case VD6G_MIRROR:
    mode = 1;
    break;
  case VD6G_MIRROR_FLIP:
    mode = 3;
    break;
  default:
    return -1;
  }

  ret = ctx->write8(ctx, VD6G_REG_ORIENTATION, mode);
  VD6G_TraceError(ctx, ret);

  return 0;
}

static int VD6G_SetupPatGen(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  uint16_t value = VD6G_PATGEN_CTRL_DISABLE;
  int ret;

  switch (drv_ctx->config_save.patgen) {
  case VD6G_PATGEN_DISABLE:
    value = VD6G_PATGEN_CTRL_DISABLE;
    break;
  case VD6G_PATGEN_SOLID_COLOR:
    value = VD6G_PATGEN_CTRL_SOLID_COLOR ;
    break;
  case VD6G_PATGEN_VERTICAL_COLOR_BARS:
    value = VD6G_PATGEN_CTRL_VER_COLOR_BAR;
    break;
  case VD6G_PATGEN_HORIZONTAL_GRAYSCALE:
    value = VD6G_PATGEN_CTRL_HOR_GRAY;
    break;
  case VD6G_PATGEN_VERTICAL_GRAYSCALE:
    value = VD6G_PATGEN_CTRL_VER_GRAY;
    break;
  case VD6G_PATGEN_DIAGONAL_GRAYSCALE:
    value = VD6G_PATGEN_CTRL_DIAG_GRAY;
    break;
  case VD6G_PATGEN_PSEUDO_RANDOM:
    value = VD6G_PATGEN_CTRL_PSN;
    break;
  default:
    return -1;
  }

  if (drv_ctx->config_save.patgen != VD6G_PATGEN_DISABLE)
  {
    ret = ctx->write8(ctx, VD6G_REG_DUSTER_CTRL, VD6G_DUSTER_CTRL_DISABLE);
    VD6G_TraceError(ctx, ret);
    ret = ctx->write8(ctx, VD6G_REG_DARKCAL_CTRL, VD6G_DARKCAL_CTRL_DISABLE_DARKAVG);
    VD6G_TraceError(ctx, ret);
  }

  ret = ctx->write16(ctx, VD6G_REG_PATGEN_CTRL, value);
  VD6G_TraceError(ctx, ret);

  return 0;
}

static int VD6G_SetFlicker(VD6G_Ctx_t *ctx, VD6G_Flicker_t flicker)
{
  uint16_t mode;
  int ret;

  switch (flicker) {
  case VD6G_FLICKER_FREE_NONE:
    mode = AE_COMP_CTRL_GAIN_PRIO;
    break;
  case VD6G_FLICKER_FREE_50HZ:
    mode = AE_COMP_CTRL_FLICKER_50_HZ;
    break;
  case VD6G_FLICKER_FREE_60HZ:
    mode = AE_COMP_CTRL_FLICKER_60_HZ;
    break;
  default:
    return -1;
  }

  ret = ctx->write16(ctx, VD6G_REG_AE_COMPILER_CONTROL, mode);
  VD6G_TraceError(ctx, ret);

  return 0;
}

static int VD6G_Flicker(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;

  return VD6G_SetFlicker(ctx, drv_ctx->config_save.flicker);
}

static int VD6G_SetupEarly(VD6G_Ctx_t *ctx)
{
  int ret;

  ret = VD6G_SetupClocks(ctx);
  if (ret)
    return ret;

  ret = VD6G_SetupFrameRate(ctx);
  if (ret)
    return ret;

  return 0;
}

static int VD6G_Setup(VD6G_Ctx_t *ctx)
{
  int ret;

  ret = VD6G_SetupOutput(ctx);
  if (ret)
    return ret;

  ret = VD6G_SetupSize(ctx);
  if (ret)
    return ret;

  ret = VD6G_SetupExposure(ctx);
  if (ret)
    return ret;

  ret = VD6G_SetupMirrorFlip(ctx);
  if (ret)
    return ret;

  ret = VD6G_SetupPatGen(ctx);
  if (ret)
    return ret;

  ret = VD6G_Flicker(ctx);
  if (ret)
    return ret;

  return 0;
}

static int VD6G_StartStreaming(VD6G_Ctx_t *ctx)
{
  int ret;

  ret = ctx->write8(ctx, VD6G_REG_STBY, VD6G_CMD_START_STREAM);
  VD6G_TraceError(ctx, ret);

  ret = VD6G_PollReg8(ctx, VD6G_REG_STBY, VD6G_CMD_ACK);
  if (ret)
    return ret;

  ret = VD6G_WaitState(ctx, VD6G_SYSTEM_FSM_STREAMING);
  if (ret)
    return ret;

  VD6G_notice(ctx, "Streaming is on\n");

  return 0;
}

static int VD6G_StopStreaming(VD6G_Ctx_t *ctx)
{
  int ret;

  ret = ctx->write8(ctx, VD6G_REG_STREAMING, VD6G_CMD_STOP_STREAM);
  VD6G_TraceError(ctx, ret);

  ret = VD6G_PollReg8(ctx, VD6G_REG_STREAMING, VD6G_CMD_ACK);
  if (ret)
    return ret;

  ret = VD6G_WaitState(ctx, VD6G_SYSTEM_FSM_SW_STBY);
  if (ret)
    return ret;

  VD6G_notice(ctx, "Streaming is off\n");

  return 0;
}

int VD6G_Init(VD6G_Ctx_t *ctx, VD6G_Config_t *config)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  int ret;

  if (config->frame_rate < VD6G_MIN_FPS)
    return -1;
  if (config->frame_rate > VD6G_MAX_FPS)
    return -1;

  if ((config->resolution != VD6G_RES_QVGA_320_240) &&
      (config->resolution != VD6G_RES_VGA_640_480) &&
      (config->resolution != VD6G_RES_VGA_PORTRAIT_480_640) &&
      (config->resolution != VD6G_RES_XGA_1024_768) &&
      (config->resolution != VD6G_RES_XGA_PORTRAIT_768_1024) &&
      (config->resolution != VD6G_RES_720P_PORTRAIT_720_1280) &&
      (config->resolution != VD6G_RES_SXGA_PORTRAIT_1024_1280) &&
      (config->resolution != VD6G_RES_PORTRAIT_1120_720) &&
      (config->resolution != VD6G_RES_FULL_1120_1364)) {
    return -1;
  }

  if ((config->flip_mirror_mode != VD6G_MIRROR_FLIP_NONE) &&
      (config->flip_mirror_mode != VD6G_FLIP) &&
      (config->flip_mirror_mode != VD6G_MIRROR) &&
      (config->flip_mirror_mode != VD6G_MIRROR_FLIP)) {
    return -1;
  }

  if ((config->pixel_depth != 8) && (config->pixel_depth != 10))
    return -1;

  ctx->shutdown_pin(ctx, 0);
  ctx->delay(ctx, 10);
  ctx->shutdown_pin(ctx, 1);
  ctx->delay(ctx, 10);

  drv_ctx->config_save = *config;
  drv_ctx->is_streaming = 0;

  ret = VD6G_Boot(ctx);
  if (ret)
    return ret;

  ret = VD6G_SetupEarly(ctx);
  if (ret)
    return ret;

  return 0;
}

int VD6G_DeInit(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;

  if (drv_ctx->is_streaming)
    return -1;

  ctx->shutdown_pin(ctx, 0);
  ctx->delay(ctx, 10);

  return 0;
}

int VD6G_Start(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  int ret;

  ret = VD6G_Setup(ctx);
  if (ret)
    return ret;

  ret = VD6G_StartStreaming(ctx);
  if (ret)
    return ret;
  drv_ctx->is_streaming = 1;

  return 0;
}

int VD6G_Stop(VD6G_Ctx_t *ctx)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  int ret;

  ret = VD6G_StopStreaming(ctx);
  if (ret)
    return ret;
  drv_ctx->is_streaming = 0;

  return 0;
}

int VD6G_SetFlipMirrorMode(VD6G_Ctx_t *ctx, VD6G_MirrorFlip_t mode)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  int is_streaming;
  int ret;

  is_streaming = VD6G_IsStreaming(ctx);
  if (is_streaming < 0)
    return is_streaming;

  if (is_streaming) {
    ret = VD6G_Stop(ctx);
    if (ret)
      return ret;
  }

  drv_ctx->config_save.flip_mirror_mode = mode;

  if (is_streaming) {
    ret = VD6G_Start(ctx);
    if (ret)
      return ret;
  }

  return 0;
}

int VD6G_GetBrightnessLevel(VD6G_Ctx_t *ctx, int *level)
{
  uint16_t value;
  int ret;

  ret = ctx->read16(ctx, VD6G_REG_AE_TARGET_PERCENTAGE, &value);
  VD6G_TraceError(ctx, ret);
  *level = value;

  return 0;
}

int VD6G_SetBrightnessLevel(VD6G_Ctx_t *ctx, int level)
{
  uint16_t value = level;
  int ret;

  if (level < VD6G_MIN_BRIGHTNESS || level > VD6G_MAX_BRIGHTNESS)
    return -1;

  ret = ctx->write16(ctx, VD6G_REG_AE_TARGET_PERCENTAGE, value);
  VD6G_TraceError(ctx, ret);

  return 0;
}

int VD6G_SetFlickerMode(VD6G_Ctx_t *ctx, VD6G_Flicker_t mode)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  int ret;

  ret = VD6G_SetFlicker(ctx, mode);
  if (ret)
    return ret;

  drv_ctx->config_save.flicker = mode;

  return 0;
}

int VD6G_SetExposureMode(VD6G_Ctx_t *ctx, VD6G_ExposureMode_t mode)
{
  struct drv_vd6g_ctx *drv_ctx = &ctx->ctx;
  int ret;

  ret = VD6G_SetExposureModeInternal(ctx, mode);
  if (ret)
    return ret;

  drv_ctx->config_save.exposure_mode = mode;

  return 0;
}

int VD6G_GetAnalogGainRegRange(VD6G_Ctx_t *ctx, uint8_t *AGmin, uint8_t *AGmax)
{
  int ret;
  uint8_t value;

  if ((AGmin == NULL) || (AGmax == NULL))
    return -1;

  ret = ctx->read8(ctx, VD6G_REG_MIN_AG_CODED, &value);
  VD6G_TraceError(ctx, ret);
  *AGmin = value & VD6G_REG_MIN_AG_CODED_MASK;

  ret = ctx->read8(ctx, VD6G_REG_MAX_AG_CODED, &value);
  VD6G_TraceError(ctx, ret);
  *AGmax = value & VD6G_REG_MAX_AG_CODED_MASK;

  return 0;
}

int VD6G_SetAnalogGain(VD6G_Ctx_t *ctx, int gain)
{
  int ret;
  uint8_t again_regmin, again_regmax;

  ret = VD6G_GetAnalogGainRegRange(ctx, &again_regmin, &again_regmax);
  if (ret)
    return ret;

  ret = CHECK_RANGE(gain, again_regmin, again_regmax);
  if (ret)
    return ret;

  ret = ctx->write8(ctx, VD6G_REG_MANUAL_ANALOG_GAIN, gain);
  VD6G_TraceError(ctx, ret);

  return 0;
}

int VD6G_GetDigitalGainRegRange(VD6G_Ctx_t *ctx, uint16_t *DGmin, uint16_t *DGmax)
{
  int ret;

  if ((DGmin == NULL) && (DGmax == NULL))
    return -1;

  ret = ctx->read16(ctx, VD6G_REG_MIN_DG, DGmin);
  VD6G_TraceError(ctx, ret);

  ret = ctx->read16(ctx, VD6G_REG_MAX_DG, DGmax);
  VD6G_TraceError(ctx, ret);

  return 0;
}

int VD6G_SetDigitalGain(VD6G_Ctx_t *ctx, int gain)
{
  int ret;
  uint16_t dgain_regmin, dgain_regmax;

  ret = VD6G_GetDigitalGainRegRange(ctx, &dgain_regmin, &dgain_regmax);
  if (ret)
    return ret;

  ret = CHECK_RANGE(gain, dgain_regmin, dgain_regmax);
  if (ret)
    return ret;

  ret = ctx->write16(ctx, VD6G_REG_MANUAL_CH0_DIGITAL_GAIN, gain);
  VD6G_TraceError(ctx, ret);

  ret = ctx->write16(ctx, VD6G_REG_MANUAL_CH1_DIGITAL_GAIN, gain);
  VD6G_TraceError(ctx, ret);

  ret = ctx->write16(ctx, VD6G_REG_MANUAL_CH2_DIGITAL_GAIN, gain);
  VD6G_TraceError(ctx, ret);

  ret = ctx->write16(ctx, VD6G_REG_MANUAL_CH3_DIGITAL_GAIN, gain);
  VD6G_TraceError(ctx, ret);

  return 0;
}

int VD6G_GetExposureRegRange(VD6G_Ctx_t *ctx, uint32_t *min_us, uint32_t *max_us)
{
  uint16_t exp_coarse_intg_margin;
  uint32_t line_time_in_ns;
  uint16_t frame_length;
  int ret;

  if ((min_us == NULL) || (max_us == NULL))
    return -1;

  ret = VD6G_GetLineTimeInNs(ctx, &line_time_in_ns);
  if (ret)
    return ret;

  *min_us = (VD6G_MIN_MANUAL_EXP_COARSE * line_time_in_ns) / 1000;

  ret = ctx->read16(ctx, VD6G_FRAME_LENGTH, &frame_length);
  VD6G_TraceError(ctx, ret);

  ret = ctx->read16(ctx, VD6G_REG_EXP_COARSE_INTG_MARGIN, &exp_coarse_intg_margin);
  VD6G_TraceError(ctx, ret);

  if (exp_coarse_intg_margin >= VD6G_MIN_EXP_COARSE_INTG_MARGIN)
    *max_us = ((frame_length - exp_coarse_intg_margin - VD6G_MAX_MANUAL_EXP_COARSE_OFFSET) * line_time_in_ns) / 1000;
  else
    return -1;

  return 0;
}

int VD6G_SetExposureTime(VD6G_Ctx_t *ctx, int exposure_us)
{
  int32_t ret;
  uint32_t exp_min, exp_max;
  uint32_t line_time_in_ns;

  ret = VD6G_GetExposureRegRange(ctx, &exp_min, &exp_max);
  if (ret)
    return ret;

  ret = CHECK_RANGE(exposure_us, exp_min, exp_max);
  if (ret)
    return ret;

  ret = VD6G_GetLineTimeInNs(ctx, &line_time_in_ns);
  if (ret)
    return ret;

  ret = ctx->write16(ctx, VD6G_REG_MANUAL_COARSE_EXP, CEIL(1000 * exposure_us / line_time_in_ns));
  VD6G_TraceError(ctx, ret);

  return 0;
}
