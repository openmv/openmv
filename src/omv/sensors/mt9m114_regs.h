/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MT9M114 driver.
 */

#define MT9M114_REG_PAD_SLEW                        (0x001E)

#define MT9M114_REG_SYSCTL                          (0x001A)
#define MT9M114_SYSCTL_SOFT_RESET                   (1 << 0)

#define MT9M114_REG_HOST_COMMAND                    (0x0080)
#define MT9M114_HC_APPLY_PATCH                      (0x0001)
#define MT9M114_HC_SET_STATE                        (0x0002)
#define MT9M114_HC_REFRESH                          (0x0004)
#define MT9M114_HC_WAIT_FOR_EVENT                   (0x0008)
#define MT9M114_HC_OK                               (0x8000)

#define MT9M114_HC_DELAY                            (1)
#define MT9M114_HC_TIMEOUT                          (100)

#define MT9M114_SS_ENTER_CONFIG_CHANGE              (0x28)
#define MT9M114_SS_STREAMING                        (0x31)
#define MT9M114_SS_START_STREAMING                  (0x34)
#define MT9M114_SS_ENTER_SUSPEND                    (0x40)
#define MT9M114_SS_SUSPENDED                        (0x41)
#define MT9M114_SS_ENTER_STANDBY                    (0x50)
#define MT9M114_SS_STANDBY                          (0x52)
#define MT9M114_SS_LEAVE_STANDBY                    (0x54)

#define MT9M114_REG_XMDA_ACCESS_CTL_STAT            (0x0982)
#define MT9M114_REG_XMDA_PHYSICAL_ADDRESS_ACCESS    (0x098A)
#define MT9M114_REG_XMDA_LOGIC_ADDRESS_ACCESS       (0x098E)

#define MT9M114_REG_SEQ_ERROR_CODE                  (0x8406)

#define MT9M114_REG_CAM_OUTPUT_FORMAT               (0xC86C)
#define MT9M114_OUTPUT_FORMAT_SWAP_RB               (1 << 0)
#define MT9M114_OUTPUT_FORMAT_SWAP_BYTES            (1 << 1)
#define MT9M114_OUTPUT_FORMAT_MONO                  (1 << 2)
#define MT9M114_OUTPUT_FORMAT_BT656                 (1 << 3)
#define MT9M114_OUTPUT_FORMAT_BT656_FIXED           (1 << 4)
#define MT9M114_OUTPUT_FORMAT_YUV                   (0 << 8)
#define MT9M114_OUTPUT_FORMAT_RGB                   (1 << 8)
#define MT9M114_OUTPUT_FORMAT_BAYER                 (2 << 8)
#define MT9M114_OUTPUT_FORMAT_RAW_BAYER_10          (0 << 10)
#define MT9M114_OUTPUT_FORMAT_RAW_BAYER_10_PRE      (1 << 10)
#define MT9M114_OUTPUT_FORMAT_RAW_BAYER_10_POST     (2 << 10)
#define MT9M114_OUTPUT_FORMAT_PROCESSED_BAYER       (3 << 10)
#define MT9M114_OUTPUT_FORMAT_RGB565                (0 << 12)
#define MT9M114_OUTPUT_FORMAT_RGB555                (1 << 12)
#define MT9M114_OUTPUT_FORMAT_XRGB444               (2 << 12)
#define MT9M114_OUTPUT_FORMAT_RGB444X               (3 << 12)

#define MT9M114_REG_LL_ALGO                         (0xBC04)

#define MT9M114_REG_SENSOR_CFG_Y_ADDR_START         (0xC800)
#define MT9M114_REG_SENSOR_CFG_X_ADDR_START         (0xC802)
#define MT9M114_REG_SENSOR_CFG_Y_ADDR_END           (0xC804)
#define MT9M114_REG_SENSOR_CFG_X_ADDR_END           (0xC806)

#define MT9M114_REG_SENSOR_CFG_FINE_INTEG_TIME_MIN  (0xC80E)
#define MT9M114_REG_SENSOR_CFG_FINE_INTEG_TIME_MAX  (0xC810)
#define MT9M114_REG_SENSOR_CFG_FRAME_LENGTH_LINES   (0xC812)
#define MT9M114_REG_SENSOR_CFG_LINE_LENGTH_PCK      (0xC814)
#define MT9M114_REG_SENSOR_CFG_FINE_CORRECTION      (0xC816)
#define MT9M114_REG_SENSOR_CFG_CPIPE_LAST_ROW       (0xC818)

#define MT9M114_REG_SENSOR_CONTROL_READ_MODE        (0xC834)
#define MT9M114_SENSOR_CONTROL_READ_MODE_HMIRROR    (0x1)
#define MT9M114_SENSOR_CONTROL_READ_MODE_VFLIP      (0x2)
#define MT9M114_SENSOR_CONTROL_READ_MODE_HBIN_MASK  (0x30)
#define MT9M114_SENSOR_CONTROL_READ_MODE_HBIN       (0x30)
#define MT9M114_SENSOR_CONTROL_READ_MODE_VBIN_MASK  (0x300)
#define MT9M114_SENSOR_CONTROL_READ_MODE_VBIN       (0x300)

#define MT9M114_REG_CAM_MODE_SELECT                 (0xC84C)

#define MT9M114_REG_CROP_WINDOW_X_OFFSET            (0xC854)
#define MT9M114_REG_CROP_WINDOW_Y_OFFSET            (0xC856)
#define MT9M114_REG_CROP_WINDOW_WIDTH               (0xC858)
#define MT9M114_REG_CROP_WINDOW_HEIGHT              (0xC85A)

#define MT9M114_REG_CAM_OUTPUT_WIDTH                (0xC868)
#define MT9M114_REG_CAM_OUTPUT_HEIGHT               (0xC86A)

#define MT9M114_REG_CAM_SFX_CONTROL                 (0xC874)

#define MT9M114_REG_CAM_AET_MAX_FRAME_RATE          (0xC88C)
#define MT9M114_REG_CAM_AET_MIN_FRAME_RATE          (0xC88E)

#define MT9M114_REG_CAM_AWB_XSCALE                  (0xC8F2)
#define MT9M114_REG_CAM_AWB_YSCALE                  (0xC8F3)

#define MT9M114_REG_CAM_AWB_X_SHIFT_PRE_ADJ         (0xC904)
#define MT9M114_REG_CAM_AWB_Y_SHIFT_PRE_ADJ         (0xC906)

#define MT9M114_REG_AWB_CLIP_WINDOW_X_START         (0xC914)
#define MT9M114_REG_AWB_CLIP_WINDOW_Y_START         (0xC916)
#define MT9M114_REG_AWB_CLIP_WINDOW_X_END           (0xC918)
#define MT9M114_REG_AWB_CLIP_WINDOW_Y_END           (0xC91A)

#define MT9M114_REG_CAM_LL_START_SATURATION         (0xC92A)

#define MT9M114_REG_AE_INITIAL_WINDOW_X_START       (0xC91C)
#define MT9M114_REG_AE_INITIAL_WINDOW_Y_START       (0xC91E)
#define MT9M114_REG_AE_INITIAL_WINDOW_X_END         (0xC920)
#define MT9M114_REG_AE_INITIAL_WINDOW_Y_END         (0xC922)

#define MT9M114_REG_CAM_SYSCTL_PLL_DIVIDER_M_N      (0xC980)
#define MT9M114_REG_CAM_PORT_OUTPUT_CONTROL         (0xC984)

#define MT9M114_REG_SYSMGR_NEXT_STATE               (0xDC00)
#define MT9M114_REG_SYSMGR_CURRENT_STATE            (0xDC01)
#define MT9M114_REG_SYSMGR_CMD_STATUS               (0xDC02)

#define MT9M114_REG_PATCHLDR_LOADER_ADDRESS         (0xE000)
#define MT9M114_REG_PATCHLDR_PATCH_ID               (0xE002)
#define MT9M114_REG_PATCHLDR_FIRMWARE_ID_HI         (0xE004)
#define MT9M114_REG_PATCHLDR_FIRMWARE_ID_LO         (0xE006)
#define MT9M114_REG_PATCHLDR_APPLY_STATUS           (0xE008)

#define MT9M114_REG_AUTO_BINNING_MODE               (0xE801)

#define MT9M114_REG_CMD_HANDLE_WAIT_EVENT_ID        (0xFC00)
#define MT9M114_REG_CMD_HANDLE_NUM_EVENTS           (0xFC02)
