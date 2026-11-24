/**
  ******************************************************************************
  * @file    vd1943.h
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

#ifndef VD1943_H
#define VD1943_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdarg.h>
#include <stdint.h>

#define VD1943_LVL_ERROR 0
#define VD1943_LVL_WARNING 1
#define VD1943_LVL_NOTICE 2
#define VD1943_LVL_DBG(l) (3 + (l))

typedef enum {
  VD1943_BAYER_NONE,
  VD1943_BAYER_RGGB,
  VD1943_BAYER_GRBG,
  VD1943_BAYER_GBRG,
  VD1943_BAYER_BGGR,
  VD1943_BAYER_RGBNIR,
  VD1943_BAYER_RGBNIR_MIRROR,
  VD1943_BAYER_RGBNIR_FLIP,
  VD1943_BAYER_RGBNIR_FLIP_MIRROR
} VD1943_BayerType_t;

/* Output image will have resolution of VD1943_Res_t / ss_ratio */
typedef enum {
  VD1943_RES_QVGA_320_240,
  VD1943_RES_VGA_640_480,
  VD1943_RES_SVGA_800_600,
  VD1943_RES_XGA_1024_768,
  VD1943_RES_720P_1280_720,
  VD1943_RES_SXGA_1280_1024,
  VD1943_RES_1080P_1920_1080,
  VD1943_RES_QXGA_2048_1536,
  VD1943_RES_2560_1920,
  VD1943_RES_FULL_2560_1984,
} VD1943_Res_t;

typedef enum {
  VD1943_MIRROR_FLIP_NONE,
  VD1943_FLIP,
  VD1943_MIRROR,
  VD1943_MIRROR_FLIP
} VD1943_MirrorFlip_t;

typedef enum {
  VD1943_PATGEN_DISABLE,
  VD1943_PATGEN_DIAGONAL_GRAYSCALE,
  VD1943_PATGEN_PSEUDO_RANDOM,
} VD1943_PatGen_t;

enum {
  VD1943_ST_IDLE,
  VD1943_ST_STREAMING,
};

enum {
  VD1943_MIN_CLOCK_FREQ = 12000000,
  VD1943_DEFAULT_CLOCK_FREQ = 12000000,
  VD1943_MAX_CLOCK_FREQ = 50000000,
};

enum {
  VD1943_MIN_DATARATE = 250000000,
  VD1943_DEFAULT_DATARATE = 1300000000,
  VD1943_MAX_DATARATE = 1500000000,
};

typedef enum {
  VD1943_GPIO_STROBE = 0,
  VD1943_GPIO_PWM_STROBE = 1,
  VD1943_GPIO_PWM = 2,
  VD1943_GPIO_IN = 3,
  VD1943_GPIO_OUT = 4,
  VD1943_GPIO_FSYNC_IN = 5,
} VD1943_GPIO_Mode_t;

typedef enum {
  VD1943_GPIO_LOW = (0 << 4),
  VD1943_GPIO_HIGH = (1 << 4),
} VD1943_GPIO_Value_t;

typedef enum {
  VD1943_GPIO_NO_INVERSION = (0 << 5),
  VD1943_GPIO_INVERTED = (1 << 5),
} VD1943_GPIO_Polarity_t;

enum {
  VD1943_GPIO_0,
  VD1943_GPIO_1,
  VD1943_GPIO_2,
  VD1943_GPIO_3,
  VD1943_GPIO_NB
};

typedef struct {
  /* VD6G_GPIO_Mode_t | VD6G_GPIO_Value_t | VD6G_GPIO_Polarity_t */
  uint8_t gpio_ctrl;
  uint8_t enable;
} VD1943_GPIO_t;

typedef enum {
  VD1943_GS_SS1_NATIVE_8 = 0x01,
  VD1943_GS_SS1_NATIVE_10 = 0x02,
  VD1943_GS_SS1_SPLIT_NATIVE_8 = 0x03,
  VD1943_GS_SS1_SPLIT_NATIVE_10 = 0x04,
  VD1943_GS_SS1_RGB_8 = 0x05,
  VD1943_GS_SS1_RGB_10 = 0x06,
  VD1943_GS_SS1_IR_8 = 0x0f,
  VD1943_GS_SS1_IR_10 = 0x10,
  VD1943_GS_SS1_SPLIT_IR_8 = 0x11,
  VD1943_GS_SS1_SPLIT_IR_10 = 0x12,
  VD1943_GS_SS2_MONO_8 = 0x09,
  VD1943_GS_SS2_MONO_10 = 0x0a,
  VD1943_GS_SS4_MONO_8 = 0x0b,
  VD1943_GS_SS4_MONO_10 = 0x0c,
  VD1943_GS_SS32_MONO_8 = 0x0d,
  VD1943_GS_SS32_MONO_10 = 0x0e,
  VD1943_RS_SDR_NATIVE_8 = 0x1a,
  VD1943_RS_SDR_NATIVE_10 = 0x1b,
  VD1943_RS_SDR_NATIVE_12 = 0x1c,
  VD1943_RS_SDR_RGB_8 = 0x1d,
  VD1943_RS_SDR_RGB_10 = 0x1e,
  VD1943_RS_SDR_RGB_12 = 0x1f,
  VD1943_RS_HDR_NATIVE_10 = 0x20,
  VD1943_RS_HDR_NATIVE_12 = 0x21,
  VD1943_RS_HDR_RGB_10 = 0x22,
  VD1943_RS_HDR_RGB_12 = 0x23,
} VD1943_MODE_t;

typedef enum {
  VD1943_MASTER,
  VD1943_SLAVE,
} VD1943_VT_Sync_T;

/* Sensor native resolution */
#define VD1943_MAX_WIDTH                                  2560
#define VD1943_MAX_HEIGHT                                 1984

/* Analog gain [1, 4] is computed with the following logic :
 * 16/(16 - again_reg), with again_reg in the range [0, 12] */
 #define VD1943_ANALOG_GAIN_MIN                           0
 #define VD1943_ANALOG_GAIN_MAX                           12
/* Digital gain [1.00, 32.00] is coded as a Fixed Point 5.8
 * which corresponds to sensor values in the range [0x0100, 0x2000] */
 #define VD1943_DIGITAL_GAIN_MIN                          0x100
 #define VD1943_DIGITAL_GAIN_MAX                          0x2000

/* Output interface configuration */
typedef struct {
  int data_rate_in_mps;
  int datalane_nb;
  /* Define lane mapping for the four lane even in 2 data lanes case */
  int logic_lane_mapping[4];
  int clock_lane_swap_enable;
  int physical_lane_swap_enable[4];
} VD1943_OutItf_Config_t;

/* VD1943 configuration */
typedef struct {
  int ext_clock_freq_in_hz;
  VD1943_Res_t resolution;
  int frame_rate;
  VD1943_MODE_t image_processing_mode;
  VD1943_MirrorFlip_t flip_mirror_mode;
  VD1943_PatGen_t patgen;
  VD1943_OutItf_Config_t out_itf;
  VD1943_VT_Sync_T sync_mode;
  VD1943_GPIO_t gpios[VD1943_GPIO_NB];
} VD1943_Config_t;

typedef struct VD1943_Ctx
{
  /* API client must set these values */
  void (*shutdown_pin)(struct VD1943_Ctx *ctx, int value);
  int (*read8)(struct VD1943_Ctx *ctx, uint16_t addr, uint8_t *value);
  int (*read16)(struct VD1943_Ctx *ctx, uint16_t addr, uint16_t *value);
  int (*read32)(struct VD1943_Ctx *ctx, uint16_t addr, uint32_t *value);
  int (*write8)(struct VD1943_Ctx *ctx, uint16_t addr, uint8_t value);
  int (*write16)(struct VD1943_Ctx *ctx, uint16_t addr, uint16_t value);
  int (*write32)(struct VD1943_Ctx *ctx, uint16_t addr, uint32_t value);
  int (*write_array)(struct VD1943_Ctx *ctx, uint16_t addr, uint8_t *data, int data_len);
  void (*delay)(struct VD1943_Ctx *ctx, uint32_t delay_in_ms);
  void (*log)(struct VD1943_Ctx *ctx, int lvl, const char *format, va_list ap);
  /* driver fill those values on VD1943_Init */
  VD1943_BayerType_t bayer;
  /* driver internals. do not touch */
  struct drv_vd1943_ctx {
    int state;
    uint8_t is_mono;
    uint8_t is_fastboot;
    unsigned int digital_gain;
    VD1943_Config_t config_save;
    int (*set_digital_gain)(struct VD1943_Ctx *, unsigned int);
    int (*set_expo)(struct VD1943_Ctx *, unsigned int);
    int (*get_expo)(struct VD1943_Ctx *, unsigned int *);
  } ctx;
} VD1943_Ctx_t;

int VD1943_Init(VD1943_Ctx_t *ctx, VD1943_Config_t *config);
int VD1943_DeInit(VD1943_Ctx_t *ctx);
int VD1943_Start(VD1943_Ctx_t *ctx);
int VD1943_Stop(VD1943_Ctx_t *ctx);
int VD1943_Hold(VD1943_Ctx_t *ctx, int is_enable);
/* Again valid in [VD1943_ANALOG_GAIN_MIN .. VD1943_ANALOG_GAIN_MAX] */
int VD1943_SetAnalogGain(VD1943_Ctx_t *ctx, unsigned int gain);
int VD1943_GetAnalogGain(VD1943_Ctx_t *ctx, unsigned int *gain);
/* Again valid in [VD1943_DIGITAL_GAIN_MIN .. VD1943_DIGITAL_GAIN_MAX] */
int VD1943_SetDigitalGain(VD1943_Ctx_t *ctx, unsigned int gain);
int VD1943_GetDigitalGain(VD1943_Ctx_t *ctx, unsigned int *gain);
int VD1943_SetExpo(VD1943_Ctx_t *ctx, unsigned int expo_in_us);
int VD1943_GetExpo(VD1943_Ctx_t *ctx, unsigned int *expo_in_us);
int VD1943_GetExposureRange(VD1943_Ctx_t *ctx, unsigned int *min_us, unsigned int *max_us);
int VD1943_GetPixelDepth(VD1943_Ctx_t *ctx, unsigned int *pixel_depth);

#ifdef __cplusplus
}
#endif

#endif
