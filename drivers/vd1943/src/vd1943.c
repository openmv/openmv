/**
  ******************************************************************************
  * @file    vd1943.c
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

#include "vd1943.h"

#include <assert.h>
#include <stdint.h>

#include "vd1943_patch_cut_13.c"
#include "vd5943_patch_cut_13.c"
#include "vd1943_vt_patch.c"

/* STATUS */
#define VD1943_REG_MODEL_ID                                   0x0000
  #define VD1943_MODEL_ID_1_3                                 0x53393430
  #define VD1943_MODEL_ID_1_4                                 0x53393431
#define VD1943_REG_ROM_REVISION                               0x000c
  #define VD1943_ROM_REVISION_13                              0x0400
  #define VD1943_ROM_REVISION_14                              0x0540
#define VD1943_REG_CFA_SELECTION                              0x000e
  #define VD1943_OPTICAL_RGBIR                                0x00
  #define VD1943_OPTICAL_MONO                                 0x01
#define VD1943_REG_SYSTEM_FSM_STATE                           0x0044
  #define VD1943_HW_STBY                                      0
  #define VD1943_SYSTEM_UP                                    1
  #define VD1943_BOOT                                         2
  #define VD1943_SW_STBY                                      3
  #define VD1943_STREAMING                                    4
  #define VD1943_HALT                                         6
#define VD1943_REG_SYSTEM_ERROR                               0x0048
#define VD1943_FWPATCH_REVISION                               0x004a
#define VD1943_REG_SYSTEM_PLL_CLK                             0x0228
#define VD1943_REG_PIXEL_CLK                                  0x022c
/* CMD */
  #define VD1943_CMD_ACK                                      0
#define VD1943_REG_SYSTEM_UP                                  0x0514
  #define VD1943_CMD_START_SENSOR                             1
#define VD1943_REG_BOOT                                       0x0515
  #define VD1943_CMD_LOAD_CERTIFICATE                         1
  #define VD1943_CMD_LOAD_FWP                                 2
  #define VD1943_CMD_END_BOOT                                 0x10
#define VD1943_REG_SW_STBY                                    0x0516
  #define VD1943_CMD_START_STREAMING                          1
  #define VD1943_CMD_UPDATE_VT_RAM_START                      3
  #define VD1943_CMD_UPDATE_VT_RAM_END                        4
#define VD1943_REG_STREAMING                                  0x0517
  #define VD1943_CMD_STOP_STREAMING                           1
/* SENSOR SETTINGS */
#define VD1943_REG_EXT_CLOCK                                  0x0734
#define VD1943_REG_MIPI_DATA_RATE                             0x0738
#define VD1943_REG_LANE_NB_SEL                                0x0743
  #define VD1943_LANE_NB_SEL_4                                0
  #define VD1943_LANE_NB_SEL_2                                1
/* DIAG SETTINGS */
#define VD1943_REG_DIAG_DISABLE_FW_0_ERR                      0x078d
  #define VD1943_DIAG_DISABLE_FW_0_ERR_UI_CRC                 1
#define VD1943_REG_DIAG_DISABLE_FW_1_ERR                      0x078e
  #define VD1943_DIAG_DISABLE_FW_1_ERR_TASK_MONITOR           4
#define VD1943_REG_DIAG_DISABLE_STREAMING_ERR                 0x078f
  #define VD1943_DIAG_DISABLE_STREAMING_ERR_ALL               0xff
/* STREAM STATICS */
#define VD1943_REG_ROI_A_WIDTH_OFFSET                         0x090c
#define VD1943_REG_ROI_A_HEIGHT_OFFSET                        0x090e
#define VD1943_REG_ROI_A_WIDTH                                0x0910
#define VD1943_REG_ROI_A_HEIGHT                               0x0912
#define VD1943_REG_ROI_A_DT                                   0x0914
#define VD1943_REG_LINE_LENGTH                                0x0934
#define VD1943_REG_ORIENTATION                                0x0937
#define VD1943_REG_PATGEN_CTRL                                0x0938
 #define VD1943_PATGEN_CTRL_DISABLE                           0x0000
 #define VD1943_PATGEN_CTRL_DGREY                             0x2201
 #define VD1943_PATGEN_CTRL_PN28                              0x2801
#define VD1943_REG_OIF_LANE_PHY_MAP                           0x093a
#define VD1943_REG_OIF_LANE_PHY_SWAP                          0x093b
#define VD1943_REG_OIF_INTERPACKET_DELAY                      0x093c
#define VD1943_REG_VT_CTRL                                    0x0ac6
  #define VD1943_VT_CTRL_MASTER                               0
  #define VD1943_VT_CTRL_SLAVE                                1
#define VD1943_REG_OIF_ISL_ENABLE                             0x0ac7
#define VD1943_REG_GPIO_x_CTRL(_i_)                           (0xad4 + (_i_))
#define VD1943_REG_CFA_BLENDER_ALPHA_CTRL                     0x0aed
  #define VD1943_CFA_BLENDER_NORMAL                           0
  #define VD1943_CFA_BLENDER_ZERO                             2
#define VD1943_REG_OIF_DT_ISL                                 0x0afc
/* STREAM_CTXx_STATIC */
#define VD1943_REG_CTXx_STATIC_OFFSET(_x_, _addr_)            (0x0b40 + (_x_) * 72 + (_addr_))
#define VD1943_REG_SENSOR_CONFIGURATION(_x_)                  VD1943_REG_CTXx_STATIC_OFFSET(_x_, 0x0000)
#define VD1943_REG_FRAME_LENGTH(_x_)                          VD1943_REG_CTXx_STATIC_OFFSET(_x_, 0x0006)
#define VD1943_REG_GPIO_CTRL(_x_)                             VD1943_REG_CTXx_STATIC_OFFSET(_x_, 0x0009)
 #define VD1943_GPIO_DISABLE                                  0
 #define VD1943_GPIO_ENABLE                                   1
/* STREAM DYNAMICS */
/* STREAM_CTXx_DYNAMIC */
#define VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, _addr_)           (0x0c78 + (_x_) * 40 + (_addr_))

#define VD1943_REG_GROUP_PARAMETER_HOLD(_x_)                  VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x0000)
 #define VD1943_GROUP_PARAMETER_HOLD_DISABLE                  0
 #define VD1943_GROUP_PARAMETER_HOLD_ENABLE                   1
#define VD1943_REG_ANALOG_GAIN(_x_)                           VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x0001)
#define VD1943_REG_INTEGRATION_TIME_PRIMARY(_x_)              VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x0002)
#define VD1943_REG_INTEGRATION_TIME_IR(_x_)                   VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x0004)
#define VD1943_REG_INTEGRATION_TIME_SHORT(_x_)                VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x0006)
#define VD1943_DIGITAL_GAIN_R(_x_)                            VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x0008)
#define VD1943_DIGITAL_GAIN_G(_x_)                            VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x000a)
#define VD1943_DIGITAL_GAIN_B(_x_)                            VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x000c)
#define VD1943_DIGITAL_GAIN_IR(_x_)                           VD1943_REG_CTXx_DYNAMIC_OFFSET(_x_, 0x000e)

/* DEBUG */
#define VD1943_REG_DPHYTX_CTRL                                0x108c
#define VD1943_REG_BOTTOM_STATUS_LINE_DISABLE                 0x110c
/* CERTIFICATE */
#define VD1943_REG_CERTIFICATE_AREA_START_ADDR                0x1aa8
/* PATCH AREA */
#define VD1943_REG_FWPATCH_START_ADDR                         0x2000

#define VD1943_MIN_VBLANK                                     60

/* Yes we are above maximum recommended line time duration. But this value allow to output a 10 bits full frame in
 * global shutter mode in a 2 lanes configuration.
 */
#define VD1943_LINE_TIME_IN_NS                                14500
#define VD1943_LINE_LENGTH_MIN                                3375

/* FIXME : Need characterization */
#define VD1943_EXPOSURE_MARGIN                                100

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define VD1943_COMPUTE_LEFT(_w_)                              ((VD1943_MAX_WIDTH - (_w_)) / 2)
#define VD1943_COMPUTE_TOP(_h_)                               ((VD1943_MAX_HEIGHT - (_h_)) / 2)

struct vd1943_rect {
  int32_t left;
  int32_t top;
  uint32_t width;
  uint32_t height;
};

static const struct vd1943_rect vd1943_supported_modes[] = {
  /* VD1943_RES_QVGA_320_240 */
  {
    .left = VD1943_COMPUTE_LEFT(320),
    .top = VD1943_COMPUTE_TOP(240),
    .width = 320,
    .height = 240,
  },
  /* VD1943_RES_VGA_640_480 */
  {
    .left = VD1943_COMPUTE_LEFT(640),
    .top = VD1943_COMPUTE_TOP(480),
    .width = 640,
    .height = 480,
  },
  /* VD1943_RES_VGA_800_600 */
  {
    .left = VD1943_COMPUTE_LEFT(800),
    .top = VD1943_COMPUTE_TOP(600),
    .width = 800,
    .height = 600,
  },
  /* VD1943_RES_XGA_1024_768 */
  {
    .left = VD1943_COMPUTE_LEFT(1024),
    .top = VD1943_COMPUTE_TOP(768),
    .width = 1024,
    .height = 768,
  },
  /* VD1943_RES_720P_1280_720 */
  {
    .left = VD1943_COMPUTE_LEFT(1280),
    .top = VD1943_COMPUTE_TOP(720),
    .width = 1280,
    .height = 720,
  },
  /* VD1943_RES_SXGA_1280_1024 */
  {
    .left = VD1943_COMPUTE_LEFT(1280),
    .top = VD1943_COMPUTE_TOP(1024),
    .width = 1280,
    .height = 1024,
  },
  /* VD1943_RES_1080P_1920_1080 */
  {
    .left = VD1943_COMPUTE_LEFT(1920),
    .top = VD1943_COMPUTE_TOP(1080),
    .width = 1920,
    .height = 1080,
  },
  /* VD1943_RES_QXGA_2048_1536 */
  {
    .left = VD1943_COMPUTE_LEFT(2048),
    .top = VD1943_COMPUTE_TOP(1536),
    .width = 2048,
    .height = 1536,
  },
  /* VD1943_RES_FULL43_2560_1920 */
  {
    .left = VD1943_COMPUTE_LEFT(2560),
    .top = VD1943_COMPUTE_TOP(1920),
    .width = 2560,
    .height = 1920,
  },
  /* VD1943_RES_FULL_2560_1984 */
  {
    .left = VD1943_COMPUTE_LEFT(2560),
    .top = VD1943_COMPUTE_TOP(1984),
    .width = 2560,
    .height = 1984,
  },
};

#define VD1943_TraceError(_ctx_,_ret_) do { \
  if (_ret_) VD1943_error(_ctx_, "Error on %s:%d : %d\n", __func__, __LINE__, _ret_); \
  if (_ret_) display_error(_ctx_); \
  if (_ret_) return _ret_; \
} while(0)

static void VD1943_log_impl(VD1943_Ctx_t *ctx, int lvl, const char *format, ...)
{
  va_list ap;

  if (!ctx->log)
    return ;

  va_start(ap, format);
  ctx->log(ctx, lvl, format, ap);
  va_end(ap);
}

#define VD1943_dbg(_ctx_, _lvl_, _fmt_, ...) do { \
  VD1943_log_impl(_ctx_, VD1943_LVL_DBG(_lvl_), "VD1943_DG%d-%d : " _fmt_, _lvl_, __LINE__, ##__VA_ARGS__); \
} while(0)

#define VD1943_notice(_ctx_, _fmt_, ...) do { \
  VD1943_log_impl(_ctx_, VD1943_LVL_NOTICE, "VD1943_NOT-%d : " _fmt_, __LINE__, ##__VA_ARGS__); \
} while(0)

#define VD1943_warn(_ctx_, _fmt_, ...) do { \
  VD1943_log_impl(_ctx_, VD1943_LVL_WARNING, "VD1943_WRN-%d : " _fmt_, __LINE__, ##__VA_ARGS__); \
} while(0)

#define VD1943_error(_ctx_, _fmt_, ...) do { \
  VD1943_log_impl(_ctx_, VD1943_LVL_ERROR, "VD1943_ERR-%d : " _fmt_, __LINE__, ##__VA_ARGS__); \
} while(0)

static void display_error(VD1943_Ctx_t *ctx)
{
  uint16_t reg16;
  int ret;

  ret = ctx->read16(ctx, VD1943_REG_SYSTEM_ERROR, &reg16);
  assert(ret == 0);
  VD1943_error(ctx, "ERROR_CODE : 0x%04x\n", reg16);
}

static int VD1943_PollReg8(VD1943_Ctx_t *ctx, uint16_t addr, uint8_t poll_val)
{
  const unsigned int loop_delay_ms = 10;
  const unsigned int timeout_ms = 500;
  int loop_nb = timeout_ms / loop_delay_ms;
  uint8_t val;
  int ret;

  while (--loop_nb) {
    ret = ctx->read8(ctx, addr, &val);
    if (ret < 0)
      return ret;
    if (val == poll_val)
      return 0;
    ctx->delay(ctx, loop_delay_ms);
  }

  VD1943_dbg(ctx, 0, "current state %d\n", val);

  return -1;
}

static int VD1943_WaitState(VD1943_Ctx_t *ctx, int state)
{
  int ret = VD1943_PollReg8(ctx, VD1943_REG_SYSTEM_FSM_STATE, state);

  if (ret)
    VD1943_warn(ctx, "Unable to reach state %d\n", state);
  else
    VD1943_dbg(ctx, 0, "reach state %d\n", state);

  return ret;
}

static int VD1943_ApplyCmdAndWait(VD1943_Ctx_t *ctx, uint16_t addr, uint8_t cmd)
{
  int ret;

  VD1943_dbg(ctx, 0, "Apply cmd %d to 0x%04x\n", cmd, addr);
  ret = ctx->write8(ctx, addr, cmd);
  VD1943_TraceError(ctx, ret);

  ret = VD1943_PollReg8(ctx, addr, VD1943_CMD_ACK);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_CheckModelId(VD1943_Ctx_t *ctx)
{
  uint32_t reg32;
  uint16_t reg16;
  int ret;

  ret = ctx->read32(ctx, VD1943_REG_MODEL_ID, &reg32);
  VD1943_TraceError(ctx, ret);
  VD1943_dbg(ctx, 0, "model_id = 0x%08x\n", reg32);
  if (reg32 != VD1943_MODEL_ID_1_3 && reg32 != VD1943_MODEL_ID_1_4) {
    VD1943_error(ctx, "Unsupported model id: 0x%08x\n", reg32);
    return -1;
  }

  ret = ctx->read16(ctx, VD1943_REG_ROM_REVISION, &reg16);
  VD1943_TraceError(ctx, ret);
  VD1943_dbg(ctx, 0, "rom revision = 0x%08x\n", reg16);
  if (reg16 == VD1943_ROM_REVISION_13) {
    ctx->ctx.is_fastboot = 0;
  } else if (reg16 == VD1943_ROM_REVISION_14) {
    ctx->ctx.is_fastboot = 1;
  } else {
    VD1943_error(ctx, "Unsupported rom revision: 0x%08x\n", reg16);
    return -1;
  }

  if (ctx->ctx.is_fastboot) {
    ret = ctx->read16(ctx, VD1943_REG_CFA_SELECTION, &reg16);
    VD1943_TraceError(ctx, ret);
    VD1943_dbg(ctx, 0, "optical revision = 0x%04x\n", reg16);
    ctx->ctx.is_mono = (reg16 == VD1943_OPTICAL_MONO);
  } else {
    /* On cut 1.3 it's not possible to differentiate versions */
#ifdef USE_VD5943_SENSOR
    ctx->ctx.is_mono = 1;
#else
    ctx->ctx.is_mono = 0;
#endif
  }

  return 0;
}

static int VD1943_GoToBootState(VD1943_Ctx_t *ctx)
{
  int ret;

  ret = ctx->write32(ctx, VD1943_REG_EXT_CLOCK, ctx->ctx.config_save.ext_clock_freq_in_hz);
  VD1943_TraceError(ctx, ret);

  ret = VD1943_ApplyCmdAndWait(ctx, VD1943_REG_SYSTEM_UP, VD1943_CMD_START_SENSOR);
  if (ret)
    return ret;

  ret = VD1943_WaitState(ctx, VD1943_BOOT);
  if (ret)
    return ret;

  return 0;
}

static int VD1943_ApplyCertificate(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  int ret;

  if (drv_vd1943_ctx->is_mono) {
      ret = ctx->write_array(ctx, VD1943_REG_CERTIFICATE_AREA_START_ADDR, (uint8_t *)fw_patch_13_mono_cert,
                             sizeof(fw_patch_13_mono_cert));
  } else {
      ret = ctx->write_array(ctx, VD1943_REG_CERTIFICATE_AREA_START_ADDR, (uint8_t *)fw_patch_13_rgbnir_cert,
                             sizeof(fw_patch_13_rgbnir_cert));
  }
  VD1943_TraceError(ctx, ret);

  ret = VD1943_ApplyCmdAndWait(ctx, VD1943_REG_BOOT, VD1943_CMD_LOAD_CERTIFICATE);
  if (ret)
    return ret;

  return 0;
}

static int VD1943_ApplyFwPatch(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  uint16_t reg16;
  int ret;

  if (drv_vd1943_ctx->is_mono) {
      ret = ctx->write_array(ctx, VD1943_REG_FWPATCH_START_ADDR, (uint8_t *)fw_patch_13_mono_array,
                             sizeof(fw_patch_13_mono_array));
  } else {
      ret = ctx->write_array(ctx, VD1943_REG_FWPATCH_START_ADDR, (uint8_t *)fw_patch_13_rgbnir_array,
                             sizeof(fw_patch_13_rgbnir_array));
  }
  VD1943_TraceError(ctx, ret);

  ret = VD1943_ApplyCmdAndWait(ctx, VD1943_REG_BOOT, VD1943_CMD_LOAD_FWP);
  if (ret)
    return ret;

  ret = ctx->read16(ctx, VD1943_FWPATCH_REVISION, &reg16);
  VD1943_TraceError(ctx, ret);
  VD1943_notice(ctx, "patch = %d.%d\n", reg16 >> 8, reg16 & 0xff);

  return 0;
}

static int VD1943_GoToStandbyState(VD1943_Ctx_t *ctx)
{
  int ret;

  ret = VD1943_ApplyCmdAndWait(ctx, VD1943_REG_BOOT, VD1943_CMD_END_BOOT);
  if (ret)
    return ret;

  ret = VD1943_WaitState(ctx, VD1943_SW_STBY);
  if (ret)
    return ret;

  return 0;
}

static int VD1943_ApplyVTPatch(VD1943_Ctx_t *ctx)
{
  int ret;

  ret = VD1943_ApplyCmdAndWait(ctx, VD1943_REG_SW_STBY, VD1943_CMD_UPDATE_VT_RAM_START);
  if (ret)
    return ret;

  ret = ctx->write_array(ctx, 0x5bc0, (uint8_t *) gt_ram_pat_content, sizeof(gt_ram_pat_content));
  VD1943_TraceError(ctx, ret);

  ret = ctx->write_array(ctx, 0x5e40, (uint8_t *) gt_ram_seq1_content, sizeof(gt_ram_seq1_content));
  VD1943_TraceError(ctx, ret);

  ret = ctx->write_array(ctx, 0x5f80, (uint8_t *) gt_ram_seq2_content, sizeof(gt_ram_seq2_content));
  VD1943_TraceError(ctx, ret);

  ret = ctx->write_array(ctx, 0x60c0, (uint8_t *) gt_ram_seq3_content, sizeof(gt_ram_seq3_content));
  VD1943_TraceError(ctx, ret);

  ret = ctx->write_array(ctx, 0x6160, (uint8_t *) gt_ram_seq4_content, sizeof(gt_ram_seq4_content));
  VD1943_TraceError(ctx, ret);

  ret = ctx->write_array(ctx, 0x6ac0, (uint8_t *) rd_ram_pat_content, sizeof(rd_ram_pat_content));
  VD1943_TraceError(ctx, ret);

  ret = ctx->write_array(ctx, 0x5000, (uint8_t *) rd_ram_seq1_content, sizeof(rd_ram_seq1_content));
  VD1943_TraceError(ctx, ret);

  ret = VD1943_ApplyCmdAndWait(ctx, VD1943_REG_SW_STBY, VD1943_CMD_UPDATE_VT_RAM_END);
  if (ret)
    return ret;

  return 0;
}

static int VD1943_SetDigitalGain_RGB(VD1943_Ctx_t *ctx, unsigned int gain)
{
  int ret;

  ret = ctx->write16(ctx, VD1943_DIGITAL_GAIN_R(0), gain);
  VD1943_TraceError(ctx, ret);
  ret = ctx->write16(ctx, VD1943_DIGITAL_GAIN_G(0), gain);
  VD1943_TraceError(ctx, ret);
  ret = ctx->write16(ctx, VD1943_DIGITAL_GAIN_B(0), gain);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_SetDigitalGain_IR(VD1943_Ctx_t *ctx, unsigned int gain)
{
  int ret;

  ret = ctx->write16(ctx, VD1943_DIGITAL_GAIN_IR(0), gain);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_SetDigitalGain_RGBNIR(VD1943_Ctx_t *ctx, unsigned int gain)
{
  int ret;

  ret = VD1943_SetDigitalGain_RGB(ctx, gain);
  if (ret)
    return ret;
  ret = VD1943_SetDigitalGain_IR(ctx, gain);
  if (ret)
    return ret;

  return 0;
}

static int VD1943_GetExpoCommon(VD1943_Ctx_t *ctx, unsigned int *expo_in_us)
{
  uint16_t reg16;
  int ret;

  ret = ctx->read16(ctx, VD1943_REG_INTEGRATION_TIME_PRIMARY(0), &reg16);
  VD1943_TraceError(ctx, ret);
  *expo_in_us = (reg16 * VD1943_LINE_TIME_IN_NS) / 1000;

  return 0;
}

static int VD1943_SetExpo_GS(VD1943_Ctx_t *ctx, unsigned int expo_in_us)
{
  unsigned int expo_in_line = (expo_in_us * 1000 + VD1943_LINE_TIME_IN_NS - 1) / VD1943_LINE_TIME_IN_NS;
  int ret;

  if (expo_in_line < 4)
    return -1;

  ret = ctx->write16(ctx, VD1943_REG_INTEGRATION_TIME_PRIMARY(0), expo_in_line);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_SetExpoSplit_GS(VD1943_Ctx_t *ctx, unsigned int expo_in_us)
{
  unsigned int expo_in_line = (expo_in_us * 1000 + VD1943_LINE_TIME_IN_NS - 1) / VD1943_LINE_TIME_IN_NS;
  int ret;

  if (expo_in_line < 4)
    return -1;

  ret = ctx->write16(ctx, VD1943_REG_INTEGRATION_TIME_PRIMARY(0), expo_in_line);
  VD1943_TraceError(ctx, ret);

  ret = ctx->write16(ctx, VD1943_REG_INTEGRATION_TIME_IR(0), expo_in_line);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_GetExpo_GS(VD1943_Ctx_t *ctx, unsigned int *expo_in_us)
{
  return VD1943_GetExpoCommon(ctx, expo_in_us);
}

static int VD1943_SetExpo_RS(VD1943_Ctx_t *ctx, unsigned int expo_in_us)
{
  unsigned int expo_in_line = (expo_in_us * 1000 + VD1943_LINE_TIME_IN_NS - 1) / VD1943_LINE_TIME_IN_NS;
  int ret;

  if (expo_in_line < 2)
    return -1;

  ret = ctx->write16(ctx, VD1943_REG_INTEGRATION_TIME_PRIMARY(0), expo_in_line);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_GetExpo_RS(VD1943_Ctx_t *ctx, unsigned int *expo_in_us)
{
  return VD1943_GetExpoCommon(ctx, expo_in_us);
}

static int VD1943_SetExpo_RS_HDR(VD1943_Ctx_t *ctx, unsigned int expo_in_us)
{
  /* we use a short expo ratio of 1/20 */
  unsigned int expo_short_in_line = (expo_in_us * 50 + VD1943_LINE_TIME_IN_NS - 1) / VD1943_LINE_TIME_IN_NS;
  int ret;

  ret = VD1943_SetExpo_RS(ctx, expo_in_us);
  if (ret)
    return ret;

  expo_short_in_line = MAX(2, expo_short_in_line);
  ret = ctx->write16(ctx, VD1943_REG_INTEGRATION_TIME_SHORT(0), expo_short_in_line);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_SetBayerType_RGB(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;

  switch (drv_vd1943_ctx->config_save.flip_mirror_mode) {
  case VD1943_MIRROR_FLIP_NONE:
    ctx->bayer = VD1943_BAYER_GBRG;
    break;
  case VD1943_FLIP:
    ctx->bayer = VD1943_BAYER_RGGB;
    break;
  case VD1943_MIRROR:
    ctx->bayer = VD1943_BAYER_BGGR;
    break;
  case VD1943_MIRROR_FLIP:
    ctx->bayer = VD1943_BAYER_GBRG;
    break;
  default:
    assert(0);
  }

  return 0;
}

static int VD1943_SetBayerType_RGBNIR(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;

  switch (drv_vd1943_ctx->config_save.flip_mirror_mode) {
  case VD1943_MIRROR_FLIP_NONE:
    ctx->bayer = VD1943_BAYER_RGBNIR;
    break;
  case VD1943_FLIP:
    ctx->bayer = VD1943_BAYER_RGBNIR_FLIP;
    break;
  case VD1943_MIRROR:
    ctx->bayer = VD1943_BAYER_RGBNIR_MIRROR;
    break;
  case VD1943_MIRROR_FLIP:
    ctx->bayer = VD1943_BAYER_RGBNIR_FLIP_MIRROR;
    break;
  default:
    assert(0);
  }

  return 0;
}

static int VD1943_SetBayerType(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;

  if (drv_vd1943_ctx->is_mono)
    // Monochrom variant, force Bayer None (= Mono)  
    return VD1943_BAYER_NONE;

  switch (drv_vd1943_ctx->config_save.image_processing_mode) {
  case VD1943_GS_SS1_RGB_8:
  case VD1943_GS_SS1_RGB_10:
  case VD1943_RS_SDR_RGB_8:
  case VD1943_RS_SDR_RGB_10:
  case VD1943_RS_SDR_RGB_12:
  case VD1943_RS_HDR_RGB_10:
  case VD1943_RS_HDR_RGB_12:
    return VD1943_SetBayerType_RGB(ctx);
    break;
  case VD1943_GS_SS1_NATIVE_8:
  case VD1943_GS_SS1_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_NATIVE_8:
  case VD1943_GS_SS1_SPLIT_NATIVE_10:
  case VD1943_RS_SDR_NATIVE_8:
  case VD1943_RS_SDR_NATIVE_10:
  case VD1943_RS_SDR_NATIVE_12:
  case VD1943_RS_HDR_NATIVE_10:
  case VD1943_RS_HDR_NATIVE_12:
    return VD1943_SetBayerType_RGBNIR(ctx);
    break;
  case VD1943_GS_SS1_IR_8:
  case VD1943_GS_SS1_IR_10:
  case VD1943_GS_SS1_SPLIT_IR_8:
  case VD1943_GS_SS1_SPLIT_IR_10:
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS2_MONO_10:
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS4_MONO_10:
  case VD1943_GS_SS32_MONO_8:
  case VD1943_GS_SS32_MONO_10:
    return VD1943_BAYER_NONE;
  default:
    assert(0);
  }

  return 0;
}

static int VD1943_GetSubSamplingFactor(VD1943_Ctx_t *ctx, int *ss_factor)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;

  switch (drv_vd1943_ctx->config_save.image_processing_mode) {
  case VD1943_GS_SS32_MONO_8:
  case VD1943_GS_SS32_MONO_10:
    *ss_factor = 32;
    break;
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS2_MONO_10:
    *ss_factor = 2;
    break;
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS4_MONO_10:
    *ss_factor = 4;
    break;
  default:
    *ss_factor = 1;
  }

  return 0;
}

static int VD1943_Boot(VD1943_Ctx_t *ctx)
{
  int ret;

  ret = VD1943_WaitState(ctx, VD1943_SYSTEM_UP);
  if (ret)
    return ret;

  ret = VD1943_CheckModelId(ctx);
  if (ret)
    return ret;

  ret = VD1943_GoToBootState(ctx);
  if (ret)
    return ret;

  if (!ctx->ctx.is_fastboot) {
    ret = VD1943_ApplyCertificate(ctx);
    if (ret)
      return ret;

    ret = VD1943_ApplyFwPatch(ctx);
    if (ret)
      return ret;
  }

  ret = VD1943_GoToStandbyState(ctx);
  if (ret)
    return ret;

  if (!ctx->ctx.is_fastboot) {
    ret = VD1943_ApplyVTPatch(ctx);
    if (ret)
      return ret;
  }

  ret = VD1943_SetBayerType(ctx);
  if (ret)
    return ret;

  return 0;
}

static uint8_t VD1943_GetLaneNb(VD1943_OutItf_Config_t *out_itf)
{
  switch (out_itf->datalane_nb) {
  case 2:
    return VD1943_LANE_NB_SEL_2;
    break;
  case 4:
    return VD1943_LANE_NB_SEL_4;
    break;
  default:
    assert(0);
  }

  return VD1943_LANE_NB_SEL_2;
}

static uint8_t VD1943_GetLogicalPhysicalMapping(VD1943_OutItf_Config_t *out_itf)
{
  return (out_itf->logic_lane_mapping[0] << 0) |
         (out_itf->logic_lane_mapping[1] << 2) |
         (out_itf->logic_lane_mapping[2] << 4) |
         (out_itf->logic_lane_mapping[3] << 6);
}

static uint8_t VD1943_GetLaneSwap(VD1943_OutItf_Config_t *out_itf)
{
  return (out_itf->physical_lane_swap_enable[0] << 0) |
         (out_itf->physical_lane_swap_enable[1] << 1) |
         (out_itf->physical_lane_swap_enable[2] << 2) |
         (out_itf->physical_lane_swap_enable[3] << 3) |
         (out_itf->clock_lane_swap_enable << 4);
}

static int VD1943_SetupOutputDataType(VD1943_Ctx_t *ctx)
{
  uint8_t dt;
  int ret;

  switch (ctx->ctx.config_save.image_processing_mode) {
  case VD1943_GS_SS1_NATIVE_8:
  case VD1943_GS_SS1_SPLIT_NATIVE_8:
  case VD1943_GS_SS1_RGB_8:
  case VD1943_GS_SS1_IR_8:
  case VD1943_GS_SS1_SPLIT_IR_8:
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS32_MONO_8:
  case VD1943_RS_SDR_NATIVE_8:
  case VD1943_RS_SDR_RGB_8:
    dt = 0x2a;
    break;
  case VD1943_GS_SS1_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_NATIVE_10:
  case VD1943_GS_SS1_RGB_10:
  case VD1943_GS_SS1_IR_10:
  case VD1943_GS_SS1_SPLIT_IR_10:
  case VD1943_GS_SS2_MONO_10:
  case VD1943_GS_SS4_MONO_10:
  case VD1943_GS_SS32_MONO_10:
  case VD1943_RS_SDR_NATIVE_10:
  case VD1943_RS_SDR_RGB_10:
  case VD1943_RS_HDR_NATIVE_10:
  case VD1943_RS_HDR_RGB_10:
    dt = 0x2b;
    break;
  case VD1943_RS_SDR_NATIVE_12:
  case VD1943_RS_SDR_RGB_12:
  case VD1943_RS_HDR_NATIVE_12:
  case VD1943_RS_HDR_RGB_12:
    dt = 0x2c;
    break;
  default:
    assert(0);
    return -1;
  }

  ret = ctx->write8(ctx, VD1943_REG_ROI_A_DT, dt);
  VD1943_TraceError(ctx, ret);

  return ret;
}

static int VD1943_SetupOutput(VD1943_Ctx_t *ctx)
{
  VD1943_OutItf_Config_t *out_itf = &ctx->ctx.config_save.out_itf;
  int ret;

  /* setup sensor mode */
  ret = ctx->write32(ctx, VD1943_REG_SENSOR_CONFIGURATION(0), ctx->ctx.config_save.image_processing_mode);
  VD1943_TraceError(ctx, ret);

  /* OpenMV: keep the CFA_BLENDER on in RGB. ST's driver turns it off, assuming a pure
     RGB lens, which leaves the NIR signal in the image with any other lens. */
  ret = ctx->write8(ctx, VD1943_REG_CFA_BLENDER_ALPHA_CTRL, VD1943_CFA_BLENDER_NORMAL);
  VD1943_TraceError(ctx, ret);

  /* csi-2 output itf */
   /* lane number */
  ret = ctx->write8(ctx, VD1943_REG_LANE_NB_SEL, VD1943_GetLaneNb(out_itf));
  VD1943_TraceError(ctx, ret);
  /* logical / physical mapping */
  ret = ctx->write8(ctx, VD1943_REG_OIF_LANE_PHY_MAP, VD1943_GetLogicalPhysicalMapping(out_itf));
  VD1943_TraceError(ctx, ret);
  /* lane swap */
  ret = ctx->write8(ctx, VD1943_REG_OIF_LANE_PHY_SWAP, VD1943_GetLaneSwap(out_itf));
  VD1943_TraceError(ctx, ret);
   /* data rate */
  ret = ctx->write32(ctx, VD1943_REG_MIPI_DATA_RATE, out_itf->data_rate_in_mps);
  VD1943_TraceError(ctx, ret);
   /* data type / RAW8*/
  ret = VD1943_SetupOutputDataType(ctx);
  if (ret)
    return ret;
   /* Force ISL Datatype to 0x12 (not the default value on Cut1.4) */
  ret = ctx->write8(ctx, VD1943_REG_OIF_DT_ISL, 0x12);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_SetupSize(VD1943_Ctx_t *ctx)
{
  const struct vd1943_rect *rect = &vd1943_supported_modes[ctx->ctx.config_save.resolution];
  int ss_factor;
  int ret;

  assert(rect->left % 4 == 0);
  assert(rect->top % 4 == 0);
  assert(rect->width % 4 == 0);
  assert(rect->height % 4 == 0);

  ret = VD1943_GetSubSamplingFactor(ctx, &ss_factor);
  if (ret)
    return ret;

  ret = ctx->write16(ctx, VD1943_REG_ROI_A_WIDTH_OFFSET, rect->left);
  VD1943_TraceError(ctx, ret);
  ret = ctx->write16(ctx, VD1943_REG_ROI_A_HEIGHT_OFFSET, rect->top);
  VD1943_TraceError(ctx, ret);
  ret = ctx->write16(ctx, VD1943_REG_ROI_A_WIDTH, rect->width / ss_factor);
  VD1943_TraceError(ctx, ret);
  ret = ctx->write16(ctx, VD1943_REG_ROI_A_HEIGHT, rect->height / ss_factor);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_SetupLineLen(VD1943_Ctx_t *ctx)
{
  uint8_t gs_mode;
  uint32_t system_pll_clk;
  uint32_t vt_pixelrate;
  unsigned int bpp;
  uint32_t mipi_bandwidth;
  uint32_t mipi_pixelrate;
  uint16_t line_length;
  uint32_t mipi_transition_cst_in_px;
  uint32_t mipi_interpacket_delay_in_px;
  uint64_t mipi_virtual_linelength = 0;
  int ret;
  int line_length_offset = 0;

  /* Shutter mode directly impact linelength */
  switch (ctx->ctx.config_save.image_processing_mode)
  {
  case VD1943_GS_SS1_NATIVE_8:
  case VD1943_GS_SS1_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_NATIVE_8:
  case VD1943_GS_SS1_SPLIT_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_IR_8:
  case VD1943_GS_SS1_SPLIT_IR_10:
  case VD1943_GS_SS1_RGB_8:
  case VD1943_GS_SS1_RGB_10:
  case VD1943_GS_SS1_IR_8:
  case VD1943_GS_SS1_IR_10:
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS2_MONO_10:
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS4_MONO_10:
  case VD1943_GS_SS32_MONO_8:
  case VD1943_GS_SS32_MONO_10:
    gs_mode = 1;
    break;
  case VD1943_RS_SDR_NATIVE_8:
  case VD1943_RS_SDR_NATIVE_10:
  case VD1943_RS_SDR_NATIVE_12:
  case VD1943_RS_SDR_RGB_8:
  case VD1943_RS_SDR_RGB_10:
  case VD1943_RS_SDR_RGB_12:
    gs_mode = 0;
    break;
  case VD1943_RS_HDR_NATIVE_10:
  case VD1943_RS_HDR_NATIVE_12:
  case VD1943_RS_HDR_RGB_10:
  case VD1943_RS_HDR_RGB_12:
    gs_mode = 0;
    line_length_offset = 10;
    break;
  default:
    return -1;
  }

  ret = ctx->read32(ctx, VD1943_REG_SYSTEM_PLL_CLK, &system_pll_clk);
  VD1943_TraceError(ctx, ret);
  VD1943_dbg(ctx, 0, "system pll clock = %d hz\n", system_pll_clk);

  /* With 2 rows of ADC used in GS mode, the VT pixel rate is doubled */
  if (gs_mode)
  {
    vt_pixelrate = system_pll_clk / 2;
  }
  else
  {
    vt_pixelrate = system_pll_clk / 4;
  }

  /* Compute the max pixelrate on the mipi link based on pixel depth */
  ret = VD1943_GetPixelDepth(ctx, &bpp);
  if (ret)
    return ret;

  mipi_bandwidth = ctx->ctx.config_save.out_itf.data_rate_in_mps * ctx->ctx.config_save.out_itf.datalane_nb;
  mipi_pixelrate = mipi_bandwidth / bpp;

  /* Compute the mins linelength given the mipi bandwidth */
  if (mipi_pixelrate < vt_pixelrate)
  {
    /*
     * Compute a virtual (mipi-equivalent) linelength that could fit
     * with MIPI limitation (MIPI link being the bottleneck here).
     */
    mipi_transition_cst_in_px = mipi_pixelrate / 1000000 / 4;
    mipi_interpacket_delay_in_px = 30 * 8 * ctx->ctx.config_save.out_itf.datalane_nb / bpp;
    mipi_virtual_linelength = mipi_transition_cst_in_px +
                              vd1943_supported_modes[ctx->ctx.config_save.resolution].width +
                              mipi_interpacket_delay_in_px;
    mipi_virtual_linelength = mipi_virtual_linelength * vt_pixelrate / mipi_pixelrate;
  }
  line_length = MAX(VD1943_LINE_LENGTH_MIN, (uint16_t)mipi_virtual_linelength);

  /* Ensure linelength is a multiple of 4 */
  line_length = ((((line_length + 3) / 4) + line_length_offset) * 4);

  VD1943_dbg(ctx, 0, "line_length = %d\n", line_length);

  ret = ctx->write16(ctx, VD1943_REG_LINE_LENGTH, line_length);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_ComputeFrameLength(VD1943_Ctx_t *ctx, int fps, uint16_t *frame_length)
{
  uint8_t gs_mode;
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  int min_frame_length;
  int req_frame_length;
  uint32_t system_pll_clk;
  uint16_t line_length;
  uint16_t height;
  int ret;

  /* Shutter mode directly impact linelength */
  switch (ctx->ctx.config_save.image_processing_mode)
  {
  case VD1943_GS_SS1_NATIVE_8:
  case VD1943_GS_SS1_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_NATIVE_8:
  case VD1943_GS_SS1_SPLIT_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_IR_8:
  case VD1943_GS_SS1_SPLIT_IR_10:
  case VD1943_GS_SS1_RGB_8:
  case VD1943_GS_SS1_RGB_10:
  case VD1943_GS_SS1_IR_8:
  case VD1943_GS_SS1_IR_10:
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS2_MONO_10:
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS4_MONO_10:
  case VD1943_GS_SS32_MONO_8:
  case VD1943_GS_SS32_MONO_10:
    gs_mode = 1;
    break;
  case VD1943_RS_SDR_NATIVE_8:
  case VD1943_RS_SDR_NATIVE_10:
  case VD1943_RS_SDR_NATIVE_12:
  case VD1943_RS_SDR_RGB_8:
  case VD1943_RS_SDR_RGB_10:
  case VD1943_RS_SDR_RGB_12:
  case VD1943_RS_HDR_NATIVE_10:
  case VD1943_RS_HDR_NATIVE_12:
  case VD1943_RS_HDR_RGB_10:
  case VD1943_RS_HDR_RGB_12:
    gs_mode = 0;
    break;
  default:
    return -1;
  }

  ret = ctx->read32(ctx, VD1943_REG_SYSTEM_PLL_CLK, &system_pll_clk);
  VD1943_TraceError(ctx, ret);
  ret = ctx->read16(ctx, VD1943_REG_LINE_LENGTH, &line_length);
  VD1943_TraceError(ctx, ret);
  ret = ctx->read16(ctx, VD1943_REG_ROI_A_HEIGHT, &height);
  VD1943_TraceError(ctx, ret);

  min_frame_length = height + VD1943_MIN_VBLANK;
  if (gs_mode)
    min_frame_length = min_frame_length / 2;
  req_frame_length =  system_pll_clk / (4 * line_length * drv_vd1943_ctx->config_save.frame_rate);
  *frame_length = MIN(MAX(min_frame_length, req_frame_length), 65535);

  VD1943_dbg(ctx, 0, "frame_length to MAX(%d, %d) = %d to reach %d fps\n", min_frame_length, req_frame_length,
             *frame_length, fps);

  return 0;
}

static int VD1943_SetupFrameRate(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  uint16_t frame_length;
  int ret;

  ret = VD1943_SetupLineLen(ctx);
  if (ret)
    return ret;

  ret = VD1943_ComputeFrameLength(ctx, drv_vd1943_ctx->config_save.frame_rate, &frame_length);
  if (ret)
    return ret;

  VD1943_dbg(ctx, 0, "Set frame_length to %d to reach %d fps\n", frame_length, drv_vd1943_ctx->config_save.frame_rate);
  ret = ctx->write16(ctx, VD1943_REG_FRAME_LENGTH(0), frame_length);
  VD1943_TraceError(ctx, ret);

 return 0;
}


int VD1943_GetExposureRange(VD1943_Ctx_t *ctx, unsigned int *min_us, unsigned int *max_us)
{
  uint16_t frame_length;
  int ret;

  /* Min Exposure is dependant of shutter mode */
  switch (ctx->ctx.config_save.image_processing_mode) {
  case VD1943_GS_SS1_NATIVE_8:
  case VD1943_GS_SS1_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_NATIVE_8:
  case VD1943_GS_SS1_SPLIT_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_IR_8:
  case VD1943_GS_SS1_SPLIT_IR_10:
  case VD1943_GS_SS1_RGB_8:
  case VD1943_GS_SS1_RGB_10:
  case VD1943_GS_SS1_IR_8:
  case VD1943_GS_SS1_IR_10:
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS2_MONO_10:
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS4_MONO_10:
  case VD1943_GS_SS32_MONO_8:
  case VD1943_GS_SS32_MONO_10:
    *min_us = (4 * VD1943_LINE_TIME_IN_NS) / 1000;
    break;
  case VD1943_RS_SDR_NATIVE_8:
  case VD1943_RS_SDR_NATIVE_10:
  case VD1943_RS_SDR_NATIVE_12:
  case VD1943_RS_SDR_RGB_8:
  case VD1943_RS_SDR_RGB_10:
  case VD1943_RS_SDR_RGB_12:
  case VD1943_RS_HDR_NATIVE_10:
  case VD1943_RS_HDR_NATIVE_12:
  case VD1943_RS_HDR_RGB_10:
  case VD1943_RS_HDR_RGB_12:
    *min_us = (2 * VD1943_LINE_TIME_IN_NS) / 1000;
    break;
  default:
    return -1;
  }

  /* Max Exposure is dependant of framelength */
  ret = VD1943_ComputeFrameLength(ctx, ctx->ctx.config_save.frame_rate, &frame_length);
  if (ret)
    return ret;
  *max_us = (frame_length - VD1943_EXPOSURE_MARGIN) * VD1943_LINE_TIME_IN_NS / 1000;

  return 0;
}

static int VD1943_SetupMirrorFlip(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  uint8_t mode;
  int ret;

  switch (drv_vd1943_ctx->config_save.flip_mirror_mode) {
  case VD1943_MIRROR_FLIP_NONE:
    mode = 0;
    break;
  case VD1943_FLIP:
    mode = 2;
    break;
  case VD1943_MIRROR:
    mode = 1;
    break;
  case VD1943_MIRROR_FLIP:
    mode = 3;
    break;
  default:
    return -1;
  }

  ret = ctx->write8(ctx, VD1943_REG_ORIENTATION, mode);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_SetupPatGen(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  uint16_t reg16;
  int ret;

  switch (drv_vd1943_ctx->config_save.patgen) {
  case VD1943_PATGEN_DISABLE:
    reg16 = VD1943_PATGEN_CTRL_DISABLE;
    break;
  case VD1943_PATGEN_DIAGONAL_GRAYSCALE:
    reg16 = VD1943_PATGEN_CTRL_DGREY;
    break;
  case VD1943_PATGEN_PSEUDO_RANDOM:
    reg16 = VD1943_PATGEN_CTRL_PN28;
    break;
  default:
    return -1;
  }

  ret = ctx->write16(ctx, VD1943_REG_PATGEN_CTRL, reg16);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_Gpios(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  uint8_t gpio_en = 0;
  int ret;
  int i;

  for (i = 0; i < VD1943_GPIO_NB; i++) {
    ret = ctx->write8(ctx, VD1943_REG_GPIO_x_CTRL(i), drv_vd1943_ctx->config_save.gpios[i].gpio_ctrl);
    VD1943_TraceError(ctx, ret);
    gpio_en |= (drv_vd1943_ctx->config_save.gpios[i].enable & 1) << i;
  }

  ret = ctx->write8(ctx, VD1943_REG_GPIO_CTRL(0), gpio_en);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_VT_Sync(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  int i;
  int ret;

  /* If Slave mode is enabled, ensure at lease one GPIO is configured accordingly */
  if (drv_vd1943_ctx->config_save.sync_mode == VD1943_SLAVE) {
    for (i = 0; i < VD1943_GPIO_NB; i++) {
      if (((drv_vd1943_ctx->config_save.gpios[i].gpio_ctrl & VD1943_GPIO_FSYNC_IN) == VD1943_GPIO_FSYNC_IN) &&
        (drv_vd1943_ctx->config_save.gpios[i].enable == VD1943_GPIO_ENABLE)) {
        break;
      }
    }

    if (i == VD1943_GPIO_NB) {
      // Slave mode is selected but no GPIO is configured
      return -1;
    }
  }

  ret = ctx->write8(ctx, VD1943_REG_VT_CTRL, drv_vd1943_ctx->config_save.sync_mode);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_SafetyDisable(VD1943_Ctx_t *ctx)
{
  int ret;

  ret = ctx->write8(ctx, VD1943_REG_DIAG_DISABLE_FW_0_ERR, VD1943_DIAG_DISABLE_FW_0_ERR_UI_CRC);
  VD1943_TraceError(ctx, ret);
  ret = ctx->write8(ctx, VD1943_REG_DIAG_DISABLE_FW_1_ERR, VD1943_DIAG_DISABLE_FW_1_ERR_TASK_MONITOR);
  VD1943_TraceError(ctx, ret);
  ret = ctx->write8(ctx, VD1943_REG_DIAG_DISABLE_STREAMING_ERR, VD1943_DIAG_DISABLE_STREAMING_ERR_ALL);
  VD1943_TraceError(ctx, ret);

  return 0;
}

static int VD1943_Setup(VD1943_Ctx_t *ctx)
{
  int ret;

  ret = VD1943_SetupOutput(ctx);
  if (ret)
    return ret;

  ret = VD1943_SetupSize(ctx);
  if (ret)
    return ret;

  ret = VD1943_SetupFrameRate(ctx);
  if (ret)
    return ret;

  ret = VD1943_SetupMirrorFlip(ctx);
  if (ret)
    return ret;

  ret = VD1943_SetupPatGen(ctx);
  if (ret)
    return ret;

  ret = VD1943_Gpios(ctx);
  if (ret)
    return ret;

  ret = VD1943_VT_Sync(ctx);
  if (ret)
    return ret;

  ret = VD1943_SafetyDisable(ctx);
  if (ret)
    return ret;

  return 0;
}

static int VD1943_StartStreaming(VD1943_Ctx_t *ctx)
{
  int ret;

  ret = VD1943_ApplyCmdAndWait(ctx, VD1943_REG_SW_STBY, VD1943_CMD_START_STREAMING);
  if (ret)
    return ret;

  ret = VD1943_WaitState(ctx, VD1943_STREAMING);
  if (ret)
    return ret;

  VD1943_notice(ctx, "Streaming is on\n");

  return 0;
}

static int VD1943_StopStreaming(VD1943_Ctx_t *ctx)
{
  int ret;

  ret = VD1943_ApplyCmdAndWait(ctx, VD1943_REG_STREAMING, VD1943_CMD_STOP_STREAMING);
  if (ret)
    return ret;

  ret = VD1943_WaitState(ctx, VD1943_SW_STBY);
  if (ret)
    return ret;

  VD1943_notice(ctx, "Streaming is on\n");

  return 0;
}

static int VD1943_CheckConfig(VD1943_Ctx_t *ctx, VD1943_Config_t *config)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;

  if (config->ext_clock_freq_in_hz < VD1943_MIN_CLOCK_FREQ || config->ext_clock_freq_in_hz > VD1943_MAX_CLOCK_FREQ) {
    VD1943_error(ctx, "ext_clock_freq_in_hz out of range");
    return -1;
  }

  if (config->out_itf.data_rate_in_mps < VD1943_MIN_DATARATE || config->out_itf.data_rate_in_mps > VD1943_MAX_DATARATE) {
    VD1943_error(ctx, "data_rate_in_mps out of range");
    return -1;
  }

  if (config->out_itf.datalane_nb != 2 && config->out_itf.datalane_nb != 4) {
    VD1943_error(ctx, "datalane_nb out of range");
    return -1;
  }

  if (config->resolution < VD1943_RES_QVGA_320_240 ||
      config->resolution > VD1943_RES_FULL_2560_1984) {
    VD1943_error(ctx, "resolution out of range");
    return -1;
  }

  switch (config->image_processing_mode) {
  case VD1943_GS_SS1_NATIVE_8:
  case VD1943_GS_SS1_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_NATIVE_8:
  case VD1943_GS_SS1_SPLIT_NATIVE_10:
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS2_MONO_10:
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS4_MONO_10:
  case VD1943_GS_SS32_MONO_8:
  case VD1943_GS_SS32_MONO_10:
  case VD1943_RS_SDR_NATIVE_8:
  case VD1943_RS_SDR_NATIVE_10:
  case VD1943_RS_SDR_NATIVE_12:
  case VD1943_RS_HDR_NATIVE_10:
  case VD1943_RS_HDR_NATIVE_12:
    break;
  case VD1943_GS_SS1_RGB_8:
  case VD1943_GS_SS1_RGB_10:
  case VD1943_GS_SS1_IR_8:
  case VD1943_GS_SS1_IR_10:
  case VD1943_GS_SS1_SPLIT_IR_8:
  case VD1943_GS_SS1_SPLIT_IR_10:
  case VD1943_RS_SDR_RGB_8:
  case VD1943_RS_SDR_RGB_10:
  case VD1943_RS_SDR_RGB_12:
  case VD1943_RS_HDR_RGB_10:
  case VD1943_RS_HDR_RGB_12:
    // Unsupported modes for VD5943 (Mono) variant
    if (drv_vd1943_ctx->is_mono)
      return -1;
    else
      break;
  default:
    return -1;
  }

  return 0;
}

static int VD1943_SetFctPtr(VD1943_Ctx_t *ctx)
{
  switch (ctx->ctx.config_save.image_processing_mode) {
  case VD1943_GS_SS1_NATIVE_8:
  case VD1943_GS_SS1_NATIVE_10:
    ctx->ctx.set_digital_gain = VD1943_SetDigitalGain_RGBNIR;
    ctx->ctx.set_expo = VD1943_SetExpo_GS;
    ctx->ctx.get_expo = VD1943_GetExpo_GS;
    break;
  case VD1943_GS_SS1_SPLIT_NATIVE_8:
  case VD1943_GS_SS1_SPLIT_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_IR_8:
  case VD1943_GS_SS1_SPLIT_IR_10:
    ctx->ctx.set_digital_gain = VD1943_SetDigitalGain_RGBNIR;
    ctx->ctx.set_expo = VD1943_SetExpoSplit_GS;
    ctx->ctx.get_expo = VD1943_GetExpo_GS;
    break;
  case VD1943_GS_SS1_RGB_8:
  case VD1943_GS_SS1_RGB_10:
    ctx->ctx.set_digital_gain = VD1943_SetDigitalGain_RGB;
    ctx->ctx.set_expo = VD1943_SetExpo_GS;
    ctx->ctx.get_expo = VD1943_GetExpo_GS;
    break;
  case VD1943_GS_SS1_IR_8:
  case VD1943_GS_SS1_IR_10:
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS2_MONO_10:
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS4_MONO_10:
  case VD1943_GS_SS32_MONO_8:
  case VD1943_GS_SS32_MONO_10:
    ctx->ctx.set_digital_gain = VD1943_SetDigitalGain_IR;
    ctx->ctx.set_expo = VD1943_SetExpo_GS;
    ctx->ctx.get_expo = VD1943_GetExpo_GS;
    break;
  case VD1943_RS_SDR_NATIVE_8:
  case VD1943_RS_SDR_NATIVE_10:
  case VD1943_RS_SDR_NATIVE_12:
    ctx->ctx.set_digital_gain = VD1943_SetDigitalGain_RGBNIR;
    ctx->ctx.set_expo = VD1943_SetExpo_RS;
    ctx->ctx.get_expo = VD1943_GetExpo_RS;
    break;
  case VD1943_RS_SDR_RGB_8:
  case VD1943_RS_SDR_RGB_10:
  case VD1943_RS_SDR_RGB_12:
    ctx->ctx.set_digital_gain = VD1943_SetDigitalGain_RGB;
    ctx->ctx.set_expo = VD1943_SetExpo_RS;
    ctx->ctx.get_expo = VD1943_GetExpo_RS;
    break;
  case VD1943_RS_HDR_NATIVE_10:
  case VD1943_RS_HDR_NATIVE_12:
  case VD1943_RS_HDR_RGB_10:
  case VD1943_RS_HDR_RGB_12:
    ctx->ctx.set_digital_gain = VD1943_SetDigitalGain_RGB;
    ctx->ctx.set_expo = VD1943_SetExpo_RS_HDR;
    ctx->ctx.get_expo = VD1943_GetExpo_RS;
    break;
  default:
    assert(0);
    return -1;
  }

  return 0;
}

int VD1943_Init(VD1943_Ctx_t *ctx, VD1943_Config_t *config)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  int ret;

  drv_vd1943_ctx->config_save = *config;
  drv_vd1943_ctx->digital_gain = 0x100;

  ctx->shutdown_pin(ctx, 0);
  ctx->delay(ctx, 2);
  ctx->shutdown_pin(ctx, 1);
  ctx->delay(ctx, 12);

  ret = VD1943_CheckConfig(ctx, config);
  if (ret)
    return ret;

  ret = VD1943_SetFctPtr(ctx);
  if (ret)
    return ret;

  ret = VD1943_Boot(ctx);
  if (ret)
    return ret;

  drv_vd1943_ctx->state = VD1943_ST_IDLE;

  return 0;
}

int VD1943_DeInit(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;

  if (drv_vd1943_ctx->state == VD1943_ST_STREAMING)
    return -1;

  ctx->shutdown_pin(ctx, 0);
  ctx->delay(ctx, 10);

  return 0;
}

//#define DUMP_REG
#ifdef DUMP_REG
static void dump_reg8(VD1943_Ctx_t *ctx, char *name, int addr)
{
  uint8_t reg;
  int ret;

  ret = ctx->read8(ctx, addr, &reg);
  assert(ret == 0);

  VD1943_notice(ctx, "Read %s - 0x%04x => 0x%02x (%d)\n", name, addr, reg, reg);
}

static void dump_reg16(VD1943_Ctx_t *ctx, char *name, int addr)
{
  uint16_t reg;
  int ret;

  ret = ctx->read16(ctx, addr, &reg);
  assert(ret == 0);

  VD1943_notice(ctx, "Read %s - 0x%04x => 0x%04x (%d)\n", name, addr, reg, reg);
}

static void dump_reg32(VD1943_Ctx_t *ctx, char *name, int addr)
{
  uint32_t reg;
  int ret;

  ret = ctx->read32(ctx, addr, &reg);
  assert(ret == 0);

  VD1943_notice(ctx, "Read %s - 0x%04x => 0x%08x (%d)\n", name, addr, reg, reg);
}

static void dump_reg(VD1943_Ctx_t *ctx, char *name, int addr, int bit_nb)
{
  if (bit_nb == 8)
    dump_reg8(ctx, name, addr);
  else if (bit_nb == 16)
    dump_reg16(ctx, name, addr);
  else if (bit_nb == 32)
    dump_reg32(ctx, name, addr);
  else
    assert(0);
}

static void VD1943_Dbg(VD1943_Ctx_t *ctx)
{
  ctx->delay(ctx, 100);
  dump_reg(ctx, "SYSTEM_FSM_STATE", 0x44, 8);
  dump_reg(ctx, "MIPI_LANE_NB", 0x45, 8);
  dump_reg(ctx, "SYSTEM_WARNING", 0x46, 16);
  dump_reg(ctx, "SYSTEM_ERROR", 0x48, 16);
  dump_reg(ctx, "FRAME_COUNTER", 0x6e, 16);
  dump_reg(ctx, "OUTPUT_FORMAT", 0x79, 8);
  dump_reg(ctx, "VIRTUAL_CHANNEL", 0x7a, 8);
  dump_reg(ctx, "ROI_SELECTION", 0x7b, 8);
  dump_reg(ctx, "ROI_A_OFFSET", 0x7c, 32);
  dump_reg(ctx, "ROI_A_SIZE", 0x80, 32);
  dump_reg(ctx, "ROI_A_DT", 0x9c, 8);
  dump_reg(ctx, "LINE_LENGTH", 0xb2, 16);
  dump_reg(ctx, "FRAME_LENGTH", 0xb4, 32);
  dump_reg(ctx, "READOUT_CTRL", 0xb8, 8);
  dump_reg(ctx, "IMAGE_CONFIG", 0xbc, 8);
  dump_reg(ctx, "SHUTTER_MODE", 0xbd, 8);
  dump_reg(ctx, "X_START", 0xc8, 16);
  dump_reg(ctx, "X_END", 0xca, 16);
  dump_reg(ctx, "Y_START", 0xcc, 16);
  dump_reg(ctx, "Y_END", 0xce, 16);
  dump_reg(ctx, "X_SIZE", 0xd0, 16);
  dump_reg(ctx, "Y_SIZE", 0xd2, 16);
  dump_reg(ctx, "OIF_LANE_PHY_MAP", 0x16c, 8);
  dump_reg(ctx, "OIF_LANE_PHY_SWAP", 0x16d, 8);
  dump_reg(ctx, "STREAMING_FSM", 0x201, 8);
  dump_reg(ctx, "SYSTEM_PLL_CLK", 0x228, 32);
  dump_reg(ctx, "PIXEL_CLK", 0x22c, 32);
  dump_reg(ctx, "MCU_CLK", 0x230, 32);
  dump_reg(ctx, "FRAME_RATE", 0x236, 16);
  dump_reg(ctx, "WAIT_DELAY", 0x2be, 16);
}
#endif

int VD1943_Start(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  int ret;

  if (drv_vd1943_ctx->state != VD1943_ST_IDLE)
    return -1;

  ret = VD1943_Setup(ctx);
  if (ret)
    return ret;

  ret = VD1943_StartStreaming(ctx);
  if (ret)
    return ret;
  drv_vd1943_ctx->state = VD1943_ST_STREAMING;

#ifdef DUMP_REG
  VD1943_Dbg(ctx);
#endif

  return 0;
}

int VD1943_Stop(VD1943_Ctx_t *ctx)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  int ret;

  if (drv_vd1943_ctx->state != VD1943_ST_STREAMING)
    return -1;

  ret = VD1943_StopStreaming(ctx);
  if (ret)
    return ret;
  drv_vd1943_ctx->state = VD1943_ST_IDLE;

  return 0;
}

int VD1943_Hold(VD1943_Ctx_t *ctx, int is_enable)
{
  int ret;

  ret = ctx->write8(ctx, VD1943_REG_GROUP_PARAMETER_HOLD(0),
                    is_enable ? VD1943_GROUP_PARAMETER_HOLD_ENABLE : VD1943_GROUP_PARAMETER_HOLD_DISABLE);
  VD1943_TraceError(ctx, ret);

  return 0;
}

int VD1943_SetAnalogGain(VD1943_Ctx_t *ctx, unsigned int gain)
{
  int ret;

  if (gain > VD1943_ANALOG_GAIN_MAX)
    return -1;

  ret = ctx->write8(ctx, VD1943_REG_ANALOG_GAIN(0), gain);
  VD1943_TraceError(ctx, ret);

  return 0;
}

int VD1943_GetAnalogGain(VD1943_Ctx_t *ctx, unsigned int *gain)
{
  uint8_t reg8;
  int ret;

  ret = ctx->read8(ctx, VD1943_REG_ANALOG_GAIN(0), &reg8);
  VD1943_TraceError(ctx, ret);
  *gain = reg8;

  return 0;
}

int VD1943_SetDigitalGain(VD1943_Ctx_t *ctx, unsigned int gain)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;
  int ret;

  if (gain > VD1943_DIGITAL_GAIN_MAX)
    return -1;

  assert(drv_vd1943_ctx->set_digital_gain);
  ret = drv_vd1943_ctx->set_digital_gain(ctx, gain);
  if (ret)
    return ret;
  ctx->ctx.digital_gain = gain;

  return 0;
}

int VD1943_GetDigitalGain(VD1943_Ctx_t *ctx, unsigned int *gain)
{
  return ctx->ctx.digital_gain;
}

int VD1943_SetExpo(VD1943_Ctx_t *ctx, unsigned int expo_in_us)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;

  assert(drv_vd1943_ctx->set_expo);
  return drv_vd1943_ctx->set_expo(ctx, expo_in_us);
}

int VD1943_GetExpo(VD1943_Ctx_t *ctx, unsigned int *expo_in_us)
{
  struct drv_vd1943_ctx *drv_vd1943_ctx = &ctx->ctx;

  assert(drv_vd1943_ctx->get_expo);
  return drv_vd1943_ctx->get_expo(ctx, expo_in_us);
}

int VD1943_GetPixelDepth(VD1943_Ctx_t *ctx, unsigned int *pixel_depth)
{
  switch (ctx->ctx.config_save.image_processing_mode) {
  case VD1943_GS_SS1_NATIVE_8:
  case VD1943_GS_SS1_SPLIT_NATIVE_8:
  case VD1943_GS_SS1_RGB_8:
  case VD1943_GS_SS1_IR_8:
  case VD1943_GS_SS1_SPLIT_IR_8:
  case VD1943_GS_SS2_MONO_8:
  case VD1943_GS_SS4_MONO_8:
  case VD1943_GS_SS32_MONO_8:
  case VD1943_RS_SDR_NATIVE_8:
  case VD1943_RS_SDR_RGB_8:
    *pixel_depth = 8;
    break;
  case VD1943_GS_SS1_NATIVE_10:
  case VD1943_GS_SS1_SPLIT_NATIVE_10:
  case VD1943_GS_SS1_RGB_10:
  case VD1943_GS_SS1_IR_10:
  case VD1943_GS_SS1_SPLIT_IR_10:
  case VD1943_GS_SS2_MONO_10:
  case VD1943_GS_SS4_MONO_10:
  case VD1943_GS_SS32_MONO_10:
  case VD1943_RS_SDR_NATIVE_10:
  case VD1943_RS_SDR_RGB_10:
  case VD1943_RS_HDR_NATIVE_10:
  case VD1943_RS_HDR_RGB_10:
    *pixel_depth = 10;
    break;
  case VD1943_RS_SDR_NATIVE_12:
  case VD1943_RS_SDR_RGB_12:
  case VD1943_RS_HDR_NATIVE_12:
  case VD1943_RS_HDR_RGB_12:
    *pixel_depth = 12;
    break;
  default:
    return -1;
  }

  return 0;
}
