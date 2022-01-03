/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MT9V0XX driver.
 */

#define MT9V0XX_CHIP_VERSION                    (0x00)

#define MT9V0XX_COL_START                       (0x01)
#define MT9V0X4_COL_START_B                     (0xC9)
#define MT9V0XX_COL_START_MIN                   (1)
#define MT9V0XX_COL_START_MAX                   (752)

#define MT9V0XX_ROW_START                       (0x02)
#define MT9V0X4_ROW_START_B                     (0xCA)
#define MT9V0XX_ROW_START_MIN                   (4)
#define MT9V0XX_ROW_START_MAX                   (482)

#define MT9V0XX_WINDOW_HEIGHT                   (0x03)
#define MT9V0X4_WINDOW_HEIGHT_B                 (0xCB)
#define MT9V0XX_WINDOW_HEIGHT_MIN               (1)
#define MT9V0XX_WINDOW_HEIGHT_MAX               (480)

#define MT9V0XX_WINDOW_WIDTH                    (0x04)
#define MT9V0X4_WINDOW_WIDTH_B                  (0xCC)
#define MT9V0XX_WINDOW_WIDTH_MIN                (1)
#define MT9V0XX_WINDOW_WIDTH_MAX                (752)

#define MT9V0XX_HORIZONTAL_BLANKING             (0x05)
#define MT9V0X4_HORIZONTAL_BLANKING_B           (0xCD)
#define MT9V0X2_HORIZONTAL_BLANKING_MIN         (43)
#define MT9V0XX_HORIZONTAL_BLANKING_MIN_1       (61)
#define MT9V0XX_HORIZONTAL_BLANKING_MIN_2       (71)
#define MT9V0XX_HORIZONTAL_BLANKING_MIN_4       (91)
#define MT9V0XX_HORIZONTAL_BLANKING_MAX         (1023)
#define MT9V0XX_HORIZONTAL_BLANKING_DEF         (94)

#define MT9V0XX_VERTICAL_BLANKING               (0x06)
#define MT9V0X4_VERTICAL_BLANKING_B             (0xCE)
#define MT9V0XX_VERTICAL_BLANKING_MIN           (4)
#define MT9V0XX_VERTICAL_BLANKING_MAX           (3000)
#define MT9V0XX_VERTICAL_BLANKING_DEF           (45)

#define MT9V0XX_CHIP_CONTROL                    (0x07)
#define MT9V0XX_CHIP_CONTROL_MASTER_MODE        (1 << 3)
#define MT9V0XX_CHIP_CONTROL_SNAP_MODE          (3 << 3)
#define MT9V0XX_CHIP_CONTROL_MODE_MASK          (3 << 3)
#define MT9V0XX_CHIP_CONTROL_DOUT_ENABLE        (1 << 7)
#define MT9V0XX_CHIP_CONTROL_SEQUENTIAL         (1 << 8)
#define MT9V0X4_CHIP_CONTROL_RESERVED           (1 << 9)
#define MT9V0X4_CHIP_CONTROL_CONTEXT            (1 << 15)

#define MT9V0XX_SHUTTER_WIDTH1                  (0x08)
#define MT9V0X4_SHUTTER_WIDTH1_B                (0xCF)

#define MT9V0XX_SHUTTER_WIDTH2                  (0x09)
#define MT9V0X4_SHUTTER_WIDTH2_B                (0xD0)

#define MT9V0XX_SHUTTER_WIDTH_CONTROL           (0x0A)
#define MT9V0X4_SHUTTER_WIDTH_CONTROL_B         (0xD1)

#define MT9V0XX_TOTAL_SHUTTER_WIDTH             (0x0B)
#define MT9V0X4_TOTAL_SHUTTER_WIDTH_B           (0xD2)
#define MT9V0XX_TOTAL_SHUTTER_WIDTH_MIN         (1)
#define MT9V0XX_TOTAL_SHUTTER_WIDTH_MAX         (32767)

#define MT9V0XX_RESET                           (0x0C)
#define MT9V0XX_RESET_SOFT_RESET                (1 << 0)

#define MT9V0XX_READ_MODE                       (0x0D)
#define MT9V0X4_READ_MODE_B                     (0x0E)
#define MT9V0XX_READ_MODE_ROW_BIN_2             (1 << 0)
#define MT9V0XX_READ_MODE_ROW_BIN_4             (1 << 1)
#define MT9V0XX_READ_MODE_COL_BIN_2             (1 << 2)
#define MT9V0XX_READ_MODE_COL_BIN_4             (1 << 3)
#define MT9V0XX_READ_MODE_ROW_FLIP              (1 << 4)
#define MT9V0XX_READ_MODE_COL_FLIP              (1 << 5)
#define MT9V0XX_READ_MODE_DARK_COLS             (1 << 6)
#define MT9V0XX_READ_MODE_DARK_ROWS             (1 << 7)

#define MT9V0XX_PIXEL_OPERATION_MODE            (0x0F)
#define MT9V0X4_PIXEL_OPERATION_MODE_HDR        (1 << 0)
#define MT9V0X4_PIXEL_OPERATION_MODE_COLOR      (1 << 1)
#define MT9V0X4_PIXEL_OPERATION_MODE_HDR_B      (1 << 8)

#define MT9V0XX_ADC_COMPANDING_MODE             (0x1C)
#define MT9V0XX_ADC_COMPANDING_MODE_LINEAR      (2 << 0)
#define MT9V0X4_ADC_COMPANDING_MODE_LINEAR_B    (2 << 8)

#define MT9V0XX_ANALOG_GAIN                     (0x35)
#define MT9V0X4_ANALOG_GAIN_B                   (0x36)
#define MT9V0XX_ANALOG_GAIN_MIN                 (16)
#define MT9V0XX_ANALOG_GAIN_MAX                 (64)

#define MT9V0XX_V1_CONTROL                      (0x31)
#define MT9V0X4_V1_CONTROL_B                    (0x39)

#define MT9V0XX_V2_CONTROL                      (0x32)
#define MT9V0X4_V2_CONTROL_B                    (0x3A)

#define MT9V0XX_V3_CONTROL                      (0x33)
#define MT9V0X4_V3_CONTROL_B                    (0x3B)

#define MT9V0XX_V4_CONTROL                      (0x34)
#define MT9V0X4_V4_CONTROL_B                    (0x3C)

#define MT9V0XX_FRAME_DARK_AVERAGE              (0x42)

#define MT9V0XX_DARK_AVG_THRESH                 (0x46)
#define MT9V0XX_DARK_AVG_LOW_THRESH_MASK        (255 << 0)
#define MT9V0XX_DARK_AVG_LOW_THRESH_SHIFT       (0)
#define MT9V0XX_DARK_AVG_HIGH_THRESH_MASK       (255 << 8)
#define MT9V0XX_DARK_AVG_HIGH_THRESH_SHIFT      (8)

#define MT9V0XX_ROW_NOISE_CORR_CONTROL          (0x70)
#define MT9V0X2_ROW_NOISE_CORR_ENABLE           (1 << 5)
#define MT9V0X4_ROW_NOISE_CORR_ENABLE           (1 << 0)
#define MT9V0X4_ROW_NOISE_CORR_ENABLE_B         (1 << 8)
#define MT9V0X2_ROW_NOISE_CORR_USE_BLK_AVG      (1 << 11)
#define MT9V0X4_ROW_NOISE_CORR_USE_BLK_AVG      (1 << 1)
#define MT9V0X4_ROW_NOISE_CORR_USE_BLK_AVG_B    (1 << 9)

#define MT9V0X2_PIXEL_CLOCK                     (0x74)
#define MT9V0X4_PIXEL_CLOCK                     (0x72)
#define MT9V0XX_PIXEL_CLOCK_INV_LINE            (1 << 0)
#define MT9V0XX_PIXEL_CLOCK_INV_FRAME           (1 << 1)
#define MT9V0XX_PIXEL_CLOCK_XOR_LINE            (1 << 2)
#define MT9V0XX_PIXEL_CLOCK_CONT_LINE           (1 << 3)
#define MT9V0XX_PIXEL_CLOCK_INV_PXL_CLK         (1 << 4)

#define MT9V0XX_TEST_PATTERN                    (0x7F)
#define MT9V0XX_TEST_PATTERN_DATA_MASK          (1023 << 0)
#define MT9V0XX_TEST_PATTERN_DATA_SHIFT         (0)
#define MT9V0XX_TEST_PATTERN_USE_DATA           (1 << 10)
#define MT9V0XX_TEST_PATTERN_GRAY_MASK          (3 << 11)
#define MT9V0XX_TEST_PATTERN_GRAY_NONE          (0 << 11)
#define MT9V0XX_TEST_PATTERN_GRAY_VERTICAL      (1 << 11)
#define MT9V0XX_TEST_PATTERN_GRAY_HORIZONTAL    (2 << 11)
#define MT9V0XX_TEST_PATTERN_GRAY_DIAGONAL      (3 << 11)
#define MT9V0XX_TEST_PATTERN_ENABLE             (1 << 13)
#define MT9V0XX_TEST_PATTERN_FLIP               (1 << 14)

#define MT9V0XX_AEC_AGC_ENABLE                  (0xAF)
#define MT9V0XX_AEC_ENABLE                      (1 << 0)
#define MT9V0X4_AEC_ENABLE_B                    (1 << 8)
#define MT9V0XX_AGC_ENABLE                      (1 << 1)
#define MT9V0X4_AGC_ENABLE_B                    (1 << 9)

#define MT9V0XX_THERMAL_INFO                    (0xC1)

#define MT9V0XX_ID_REG                          (0x6B)

#define MT9V0X2_MAX_GAIN                        (0x36)
#define MT9V0X4_MAX_GAIN                        (0xAB)

#define MT9V0X2_MAX_EXPOSE                      (0xBD)
#define MT9V0X4_MAX_EXPOSE                      (0xAD)

#define MT9V0XX_PIXEL_COUNT                     (0xB0)
#define MT9V0XX_AGC_GAIN_OUTPUT                 (0xBA)
#define MT9V0XX_AEC_EXPOSURE_OUTPUT             (0xBB)

#define MT9V0X4_FINE_SHUTTER_WIDTH_TOTAL        (0xD5)
#define MT9V0X4_FINE_SHUTTER_WIDTH_TOTAL_B      (0xD8)

#define MICROSECOND_CLKS                        (1000000)
