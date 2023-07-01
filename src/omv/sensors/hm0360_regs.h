/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2022 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2022 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HM0360 register definitions.
 */
#ifndef __REG_REGS_H__
#define __REG_REGS_H__

// Read only registers
#define         MODEL_ID_H                      0x0000
#define         MODEL_ID_L                      0x0001
#define         SILICON_REV                     0x0002
#define         FRAME_COUNT_H                   0x0005
#define         FRAME_COUNT_L                   0x0006
#define         PIXEL_ORDER                     0x0007
// Sensor mode control
#define         MODE_SELECT                     0x0100
#define         IMG_ORIENTATION                 0x0101
#define         EMBEDDED_LINE_EN                0x0102
#define         SW_RESET                        0x0103
#define         COMMAND_UPDATE                  0x0104
// Sensor exposure gain control
#define         INTEGRATION_H                   0x0202
#define         INTEGRATION_L                   0x0203
#define         ANALOG_GAIN                     0x0205
#define         DIGITAL_GAIN_H                  0x020E
#define         DIGITAL_GAIN_L                  0x020F
// Clock control
#define         PLL1_CONFIG                     0x0300
#define         PLL2_CONFIG                     0x0301
#define         PLL3_CONFIG                     0x0302
// Frame timing control
#define         FRAME_LEN_LINES_H               0x0340
#define         FRAME_LEN_LINES_L               0x0341
#define         LINE_LEN_PCK_H                  0x0342
#define         LINE_LEN_PCK_L                  0x0343
// Monochrome programming
#define         MONO_MODE                       0x0370
#define         MONO_MODE_ISP                   0x0371
#define         MONO_MODE_SEL                   0x0372
// Binning mode control
#define         H_SUBSAMPLE                     0x0380
#define         V_SUBSAMPLE                     0x0381
#define         BINNING_MODE                    0x0382
// Test pattern control
#define         TEST_PATTERN_MODE               0x0601
// Black level control
#define         BLC_TGT                         0x1004
#define         BLC2_TGT                        0x1009
#define         MONO_CTRL                       0x100A
// VSYNC / HSYNC / pixel shift registers
#define         OPFM_CTRL                       0x1014
// Tone mapping registers
#define         CMPRS_CTRL                      0x102F
#define         CMPRS_01                        0x1030
#define         CMPRS_02                        0x1031
#define         CMPRS_03                        0x1032
#define         CMPRS_04                        0x1033
#define         CMPRS_05                        0x1034
#define         CMPRS_06                        0x1035
#define         CMPRS_07                        0x1036
#define         CMPRS_08                        0x1037
#define         CMPRS_09                        0x1038
#define         CMPRS_10                        0x1039
#define         CMPRS_11                        0x103A
#define         CMPRS_12                        0x103B
#define         CMPRS_13                        0x103C
#define         CMPRS_14                        0x103D
#define         CMPRS_15                        0x103E
#define         CMPRS_16                        0x103F
// Automatic exposure control
#define         AE_CTRL                         0x2000
#define         AE_CTRL1                        0x2001
#define         CNT_ORGH_H                      0x2002
#define         CNT_ORGH_L                      0x2003
#define         CNT_ORGV_H                      0x2004
#define         CNT_ORGV_L                      0x2005
#define         CNT_STH_H                       0x2006
#define         CNT_STH_L                       0x2007
#define         CNT_STV_H                       0x2008
#define         CNT_STV_L                       0x2009
#define         CTRL_PG_SKIPCNT                 0x200A
#define         BV_WIN_WEIGHT_EN                0x200D
#define         MAX_INTG_H                      0x2029
#define         MAX_INTG_L                      0x202A
#define         MAX_AGAIN                       0x202B
#define         MAX_DGAIN_H                     0x202C
#define         MAX_DGAIN_L                     0x202D
#define         MIN_INTG                        0x202E
#define         MIN_AGAIN                       0x202F
#define         MIN_DGAIN                       0x2030
#define         T_DAMPING                       0x2031
#define         N_DAMPING                       0x2032
#define         ALC_TH                          0x2033
#define         AE_TARGET_MEAN                  0x2034
#define         AE_MIN_MEAN                     0x2035
#define         AE_TARGET_ZONE                  0x2036
#define         CONVERGE_IN_TH                  0x2037
#define         CONVERGE_OUT_TH                 0x2038
#define         FS_CTRL                         0x203B
#define         FS_60HZ_H                       0x203C
#define         FS_60HZ_L                       0x203D
#define         FS_50HZ_H                       0x203E
#define         FS_50HZ_L                       0x203F
#define         FRAME_CNT_TH                    0x205B
#define         AE_MEAN                         0x205D
#define         AE_CONVERGE                     0x2060
#define         AE_BLI_TGT                      0x2070
// Interrupt control
#define         PULSE_MODE                      0x2061
#define         PULSE_TH_H                      0x2062
#define         PULSE_TH_L                      0x2063
#define         INT_INDIC                       0x2064
#define         INT_CLEAR                       0x2065
// Motion detection control
#define         MD_CTRL                         0x2080
#define         ROI_START_END_V                 0x2081
#define         ROI_START_END_H                 0x2082
#define         MD_TH_MIN                       0x2083
#define         MD_TH_STR_L                     0x2084
#define         MD_TH_STR_H                     0x2085
#define         MD_LIGHT_COEF                   0x2099
#define         MD_BLOCK_NUM_TH                 0x209B
#define         MD_LATENCY                      0x209C
#define         MD_LATENCY_TH                   0x209D
#define         MD_CTRL1                        0x209E
// Context switch control registers
#define         PMU_CFG_3                       0x3024
#define         PMU_CFG_4                       0x3025
// Operation mode control
#define         WIN_MODE                        0x3030
// IO and clock control
#define         PAD_REGISTER_07                 0x3112

// Register bits/values
#define         HIMAX_RESET                     0x01
#define         HIMAX_MODE_STANDBY              0x00
#define         HIMAX_MODE_STREAMING            0x01     // I2C triggered streaming enable
#define         HIMAX_MODE_STREAMING_NFRAMES    0x03     // Output N frames
#define         HIMAX_MODE_STREAMING_TRIG       0x05     // Hardware Trigger
#define         HIMAX_SET_HMIRROR(r, x)         ((r & 0xFE) | ((x & 1) << 0))
#define         HIMAX_SET_VMIRROR(r, x)         ((r & 0xFD) | ((x & 1) << 1))

#define         PCLK_RISING_EDGE                0x00
#define         PCLK_FALLING_EDGE               0x01
#define         AE_CTRL_ENABLE                  0x00
#define         AE_CTRL_DISABLE                 0x01

#endif //__REG_REGS_H__
