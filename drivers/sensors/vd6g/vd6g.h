/**
  ******************************************************************************
  * @file    vd6g.h
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

#ifndef VD6G_H
#define VD6G_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>

#define VD6G_LVL_ERROR 0
#define VD6G_LVL_WARNING 1
#define VD6G_LVL_NOTICE 2
#define VD6G_LVL_DBG(l) (3 + (l))

typedef enum {
  VD6G_BAYER_NONE,
  VD6G_BAYER_RGGB,
  VD6G_BAYER_GRBG,
  VD6G_BAYER_GBRG,
  VD6G_BAYER_BGGR,
} VD6G_BayerType_t;

typedef enum {
  VD6G_RES_QVGA_320_240,
  VD6G_RES_VGA_640_480,
  VD6G_RES_VGA_PORTRAIT_480_640,
  VD6G_RES_XGA_1024_768,
  VD6G_RES_XGA_PORTRAIT_768_1024,
  VD6G_RES_720P_PORTRAIT_720_1280,
  VD6G_RES_SXGA_PORTRAIT_1024_1280,
  VD6G_RES_PORTRAIT_1120_720,
  VD6G_RES_FULL_1120_1364,
} VD6G_Res_t;

typedef enum {
  VD6G_MIRROR_FLIP_NONE,
  VD6G_FLIP,
  VD6G_MIRROR,
  VD6G_MIRROR_FLIP
} VD6G_MirrorFlip_t;

typedef enum {
  VD6G_PATGEN_DISABLE,
  VD6G_PATGEN_SOLID_COLOR,
  VD6G_PATGEN_VERTICAL_COLOR_BARS,
  VD6G_PATGEN_HORIZONTAL_GRAYSCALE,
  VD6G_PATGEN_VERTICAL_GRAYSCALE,
  VD6G_PATGEN_DIAGONAL_GRAYSCALE,
  VD6G_PATGEN_PSEUDO_RANDOM,
} VD6G_PatGen_t;

typedef enum {
  VD6G_FLICKER_FREE_NONE,
  VD6G_FLICKER_FREE_50HZ,
  VD6G_FLICKER_FREE_60HZ
} VD6G_Flicker_t;

typedef enum {
  VD6G_EXPOSURE_AUTO,
  VD6G_EXPOSURE_FREEZE_AEALGO,
  VD6G_EXPOSURE_MANUAL,
} VD6G_ExposureMode_t;

enum {
  VD6G_MIN_FPS = 1,
  VD6G_MAX_FPS = 88,
};

enum {
  VD6G_MIN_BRIGHTNESS = 0,
  VD6G_MAX_BRIGHTNESS = 100,
};

enum {
  VD6G_COLOR_DEPTH_RAW8  = 0x8,
  VD6G_COLOR_DEPTH_RAW10 = 0xa,
};

typedef enum {
  VD6G_GPIO_FSYNC_OUT = 0,
  VD6G_GPIO_GPIO_IN = 1,
  VD6G_GPIO_STROBE = 2,
  VD6G_GPIO_PWM_STROBE = 3,
  VD6G_GPIO_PWM = 4,
  VD6G_GPIO_OUT = 5,
  VD6G_GPIO_VSYNC_OUT_MODE0 = 6,
  VD6G_GPIO_VSYNC_OUT_MODE1 = 7,
  VD6G_GPIO_VSYNC_OUT_MODE2 = 8,
  VD6G_GPIO_VT_SLAVE_MODE = 0xa,
} VD6G_GPIO_Mode_t;

typedef enum {
  VD6G_GPIO_LOW = (0 << 4),
  VD6G_GPIO_HIGH = (1 << 4),
} VD6G_GPIO_Value_t;

typedef enum {
  VD6G_GPIO_NO_INVERSION = (0 << 5),
  VD6G_GPIO_INVERTED = (1 << 5),
} VD6G_GPIO_Polarity_t;

typedef enum {
  VD6G_GPIO_0,
  VD6G_GPIO_1,
  VD6G_GPIO_2,
  VD6G_GPIO_3,
  VD6G_GPIO_4,
  VD6G_GPIO_5,
  VD6G_GPIO_6,
  VD6G_GPIO_7,
  VD6G_GPIO_NB
} VD6G_GPIO_t;

/* Sensor native resolution */
#define VD6G_MAX_WIDTH                       1120
#define VD6G_MAX_HEIGHT                      1364

/* Output interface configuration */
typedef struct {
  int datalane_nb;
  int clock_lane_swap_enable;
  int data_lane0_swap_enable;
  int data_lane1_swap_enable;
  int data_lanes_mapping_swap_enable;
} VD6G_OutItf_Config_t;

/* VD6G configuration */
typedef struct {
  int ext_clock_freq_in_hz;
  VD6G_Res_t resolution;
  int frame_rate;
  VD6G_MirrorFlip_t flip_mirror_mode;
  int line_len; /**< Set value to zero to let default sensor value */
  VD6G_PatGen_t patgen;
  VD6G_Flicker_t flicker;
  uint8_t pixel_depth;
  VD6G_OutItf_Config_t out_itf;
  /* VD6G_GPIO_Mode_t | VD6G_GPIO_Value_t | VD6G_GPIO_Polarity_t */
  uint8_t gpio_ctrl[VD6G_GPIO_NB];
  VD6G_ExposureMode_t exposure_mode;
} VD6G_Config_t;

typedef struct VD6G_Ctx
{
  /* API client must set these values */
  void (*shutdown_pin)(struct VD6G_Ctx *ctx, int value);
  int (*read8)(struct VD6G_Ctx *ctx, uint16_t addr, uint8_t *value);
  int (*read16)(struct VD6G_Ctx *ctx, uint16_t addr, uint16_t *value);
  int (*read32)(struct VD6G_Ctx *ctx, uint16_t addr, uint32_t *value);
  int (*write8)(struct VD6G_Ctx *ctx, uint16_t addr, uint8_t value);
  int (*write16)(struct VD6G_Ctx *ctx, uint16_t addr, uint16_t value);
  int (*write32)(struct VD6G_Ctx *ctx, uint16_t addr, uint32_t value);
  int (*write_array)(struct VD6G_Ctx *ctx, uint16_t addr, uint8_t *data, int data_len);
  void (*delay)(struct VD6G_Ctx *ctx, uint32_t delay_in_ms);
  void (*log)(struct VD6G_Ctx *ctx, int lvl, const char *format, va_list ap);
  /* driver fill those values on VD6G_Init */
  VD6G_BayerType_t bayer;
  /* driver internals */
  struct drv_vd6g_ctx {
    int is_streaming;
    VD6G_Config_t config_save;
  } ctx;
} VD6G_Ctx_t;

int VD6G_Init(VD6G_Ctx_t *ctx, VD6G_Config_t *config);
int VD6G_DeInit(VD6G_Ctx_t *ctx);
int VD6G_Start(VD6G_Ctx_t *ctx);
int VD6G_Stop(VD6G_Ctx_t *ctx);
int VD6G_SetFlipMirrorMode(VD6G_Ctx_t *ctx, VD6G_MirrorFlip_t mode);
int VD6G_GetBrightnessLevel(VD6G_Ctx_t *ctx, int *level);
int VD6G_SetBrightnessLevel(VD6G_Ctx_t *ctx, int level);
int VD6G_SetFlickerMode(VD6G_Ctx_t *ctx, VD6G_Flicker_t mode);
int VD6G_SetExposureMode(VD6G_Ctx_t *ctx, VD6G_ExposureMode_t mode);
int VD6G_SetAnalogGain(VD6G_Ctx_t *ctx, int gain);
int VD6G_SetDigitalGain(VD6G_Ctx_t *ctx, int gain);
int VD6G_SetExposureTime(VD6G_Ctx_t *ctx, int exposure_us);

int VD6G_GetAnalogGainRegRange(VD6G_Ctx_t *ctx, uint8_t *AGmin, uint8_t *AGmax);
int VD6G_GetDigitalGainRegRange(VD6G_Ctx_t *ctx, uint16_t *DGmin, uint16_t *DGmax);
int VD6G_GetExposureRegRange(VD6G_Ctx_t *ctx, uint32_t *min_us, uint32_t *max_us);

#ifdef __cplusplus
}
#endif

#endif
