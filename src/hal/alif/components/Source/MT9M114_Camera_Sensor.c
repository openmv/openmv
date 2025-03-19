/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/**************************************************************************//**
 * @file     MT9M114_Camera_Sensor.c
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     29-Sep-2021
 * @brief    ONsemi MT9M114 Camera Sensor driver.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/
/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include "Camera_Sensor.h"
#include "Camera_Sensor_i2c.h"
#include "Driver_Common.h"
#include "Driver_CPI.h"

#if ((RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE || RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE \
      || RTE_MT9M114_CAMERA_SENSOR_LPCPI_ENABLE) && !(RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE \
      && RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE))

/* MT9M114 Camera Sensor Slave Address. */
#define MT9M114_PARALLEL_CAMERA_SENSOR_SLAVE_ADDR             0x48
#define MT9M114_CAMERA_SENSOR_MIPI_SLAVE_ADDR                 0x5D

/* MT9M114 Camera Sensor CHIP-ID registers */
#define MT9M114_CHIP_ID_REGISTER                              0x0000
#define MT9M114_CHIP_ID_REGISTER_VALUE                        0x2481

/* MT9M114 Camera Sensor Command registers */
#define MT9M114_COMMAND_REGISTER                              0x0080
#define MT9M114_COMMAND_REGISTER_APPLY_PATCH                 (1)
#define MT9M114_COMMAND_REGISTER_SET_STATE                   (1 << 1)
#define MT9M114_COMMAND_REGISTER_OK                          (1 << 15)

/* MT9M114 Camera Sensor Sysctl registers */
#define MT9M114_SYSCTL_REGISTER_RESET_AND_MISC_CONTROL        0x001A
#define MT9M114_SYSCTL_REGISTER_SLEW_RATE_CONTROL             0x001E

/* MT9M114 MIPI Control register */
#define MT9M114_MIPI_CONTROL_REGISTER                         0x3C40
#define MT9M114_MIPI_CONTROL_REGISTER_CONTINUOUS_CLOCK_MODE   (1 << 2)

/* MT9M114 Camera Sensor Camera Output Format Control registers */
#define MT9M114_CAM_OUTPUT_FORMAT_REGISTER                           0xC86C
#define MT9M114_CAM_OUTPUT_FORMAT_REGISTER_CROP_SCALE_DISABLE        (1 << 4)
#define MT9M114_CAM_OUTPUT_FORMAT_REGISTER_FORMAT_RGB                (1 << 8)
#define MT9M114_CAM_OUTPUT_FORMAT_REGISTER_FORMAT_BAYER              (2 << 8)
#define MT9M114_CAM_OUTPUT_FORMAT_REGISTER_BAYER_FORMAT_RAWR10       (0 << 10)
#define MT9M114_CAM_OUTPUT_FORMAT_REGISTER_BAYER_FORMAT_RAWR8        (3 << 10)
#define MT9M114_CAM_OUTPUT_FORMAT_REGISTER_FORMAT_RGB_565RGB         (0 << 12)

/*MT9M114 camera sensor AE control register*/
#define MT9M114_AE_TRACK_ALGO_REGISTER                               0xA804
#define MT9M114_AE_ENABLE                                            0x00FF
#define MT9M114_AE_DISABLE                                           0x0000

/* MT9M114 Camera Sensor System Manager registers */
#define MT9M114_SYSMGR_NEXT_STATE                             0xDC00
#define MT9M114_PATCH_APPLY_STATUS                            0xE008

/* MT9M114 Camera Sensor System States */
#define MT9M114_SYS_STATE_ENTER_CONFIG_CHANGE                 0x28

#define MT9M114_SYS_STATE_STREAMING                           0x31
#define MT9M114_SYS_STATE_START_STREAMING                     0x34

#define MT9M114_SYS_STATE_ENTER_SUSPEND                       0x40
#define MT9M114_SYS_STATE_SUSPENDED                           0x41

#define MT9M114_SYS_STATE_ENTER_STANDBY                       0x50
#define MT9M114_SYS_STATE_STANDBY                             0x52
#define MT9M114_SYS_STATE_LEAVE_STANDBY                       0x54


/* Wrapper function for Delay
 * Delay for millisecond:
 *  Provide busy loop delay
 */
#define MT9M114_DELAY_mSEC(msec)       sys_busy_loop_us(msec * 1000)

/**
  \brief MT9M114 Camera Sensor Register Array Structure
  used for Camera Resolution Configuration.
  */
typedef struct _MT9M114_REG {
    uint16_t reg_addr;             /* MT9M114 Camera Sensor Register Address                     */
    uint32_t reg_value;            /* MT9M114 Camera Sensor Register Value                       */
    uint8_t  reg_size;             /* MT9M114 Camera Sensor Register Size: only valid 1/2/4 Byte */
} MT9M114_REG;

#if (RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE || RTE_MT9M114_CAMERA_SENSOR_LPCPI_ENABLE)
/**
  \brief MT9M114 Camera Sensor Resolution VGA 640x480
  \note  Register Values are generated using
  MT9M114(SOC1040) Register Wizard Tool with
  below settings.
  - Image Timing :
  - Image                : VGA Binning
  - Frame Rate           : 5 Frame Per Second
  - Horizontal Mirror    : Enable
  - Vertical Flip        : Enable
  - PLL Setting :
  - Input Frequency      : 24 MHz
  - Target PLL Frequency : 96 MHz
  - Output mode          : Parallel
  */
static const MT9M114_REG mt9m114_cam_resolution_VGA_640x480[] =
{
    { 0xC97E, 0x01      , 2 }, // cam_sysctl_pll_enable = 1
    { 0xC980, 0x0120    , 2 }, // cam_sysctl_pll_divider_m_n = 288
    { 0xC982, 0x0700    , 2 }, // cam_sysctl_pll_divider_p = 1792
    { 0xC984, 0x8000    , 2 }, // cam_port_output_control = 32768 (No pixel clock slow down)
    { 0xC800, 0x0000    , 2 }, // cam_sensor_cfg_y_addr_start = 0
    { 0xC802, 0x0000    , 2 }, // cam_sensor_cfg_x_addr_start = 0
    { 0xC804, 0x03CD    , 2 }, // cam_sensor_cfg_y_addr_end = 973
    { 0xC806, 0x050D    , 2 }, // cam_sensor_cfg_x_addr_end = 1293
    { 0xC808, 0x2DC6C00 , 4 }, // cam_sensor_cfg_pixclk = 48000000
    { 0xC80C, 0x0001    , 2 }, // cam_sensor_cfg_row_speed = 1
    { 0xC80E, 0x01C3    , 2 }, // cam_sensor_cfg_fine_integ_min = 451
    { 0xC810, 0x28F8    , 2 }, // cam_sensor_cfg_fine_integ_max = 10488
    { 0xC812, 0x036C    , 2 }, // cam_sensor_cfg_frame_length_lines = 876
    { 0xC814, 0x29E3    , 2 }, // cam_sensor_cfg_line_length_pck = 10723
    { 0xC816, 0x00E0    , 2 }, // cam_sensor_cfg_fine_correction = 224
    { 0xC818, 0x01E3    , 2 }, // cam_sensor_cfg_cpipe_last_row = 483
    { 0xC826, 0x0020    , 2 }, // cam_sensor_cfg_reg_0_data = 32
    { 0xC834, 0x0333    , 2 }, // cam_sensor_control_read_mode = 819, H and V flip
    { 0xC854, 0x0000    , 2 }, // cam_crop_window_xoffset = 0
    { 0xC856, 0x0000    , 2 }, // cam_crop_window_yoffset = 0
    { 0xC858, 0x0280    , 2 }, // cam_crop_window_width = 640
    { 0xC85A, 0x01E0    , 2 }, // cam_crop_window_height = 480
    { 0xC85C, 0x03      , 1 }, // cam_crop_cropmode = 3
    { 0xC868, 0x0280    , 2 }, // cam_output_width = 640
    { 0xC86A, 0x01E0    , 2 }, // cam_output_height = 480
    { 0xC878, 0x00      , 1 }, // cam_aet_aemode = 0
    { 0xC88C, 0x051C    , 2 }, // cam_aet_max_frame_rate = 1308, (5 fps)
    { 0xC88E, 0x051C    , 2 }, // cam_aet_min_frame_rate = 1308, (5 fps)
    { 0xC914, 0x0000    , 2 }, // cam_stat_awb_clip_window_xstart = 0
    { 0xC916, 0x0000    , 2 }, // cam_stat_awb_clip_window_ystart = 0
    { 0xC918, 0x027F    , 2 }, // cam_stat_awb_clip_window_xend = 639
    { 0xC91A, 0x01DF    , 2 }, // cam_stat_awb_clip_window_yend = 479
    { 0xC91C, 0x0000    , 2 }, // cam_stat_ae_initial_window_xstart = 0
    { 0xC91E, 0x0000    , 2 }, // cam_stat_ae_initial_window_ystart = 0
    { 0xC920, 0x007F    , 2 }, // cam_stat_ae_initial_window_xend = 127
    { 0xC922, 0x005F    , 2 }, // cam_stat_ae_initial_window_yend = 95
    { 0xA404, 0x0003    , 2 }, // Adaptive Weighted AE for lowlights
};
#endif

#if(RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)

#if (RTE_MT9M114_CAMERA_SENSOR_MIPI_IMAGE_CONFIG == 0)
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT 728
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_WIDTH  1288
#define MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE 0x2B
#elif (RTE_MT9M114_CAMERA_SENSOR_MIPI_IMAGE_CONFIG == 1)
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT 720
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_WIDTH  1280
#define MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE 0x2A
#elif (RTE_MT9M114_CAMERA_SENSOR_MIPI_IMAGE_CONFIG == 2)
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT 720
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_WIDTH  1280
#define MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE 0x22
#elif (RTE_MT9M114_CAMERA_SENSOR_MIPI_IMAGE_CONFIG == 3)
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT 480
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_WIDTH  640
#define MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE 0x22
#elif (RTE_MT9M114_CAMERA_SENSOR_MIPI_IMAGE_CONFIG == 4)
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT 240
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_WIDTH  320
#define MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE 0x22
#elif (RTE_MT9M114_CAMERA_SENSOR_MIPI_IMAGE_CONFIG == 5)
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT 320
#define MT9M114_CAMERA_SENSOR_MIPI_FRAME_WIDTH  320
#define MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE 0x22
#else
#error Unsupported resolution
#endif

/**
  \brief MT9M114 Camera Sensor Resolution VGA 1280x720
  - Image Timing :
  - Frame Rate           : 30 Frame Per Second
  - PLL Setting :
  - Input Frequency      : 24 MHz
  - Target PLL Frequency : 296 MHz
  - Output mode          : MIPI
  */

/*Patch for camera sensor*/
static const uint16_t sensor_patch_reg_burst[][24] =
{
    {0x70CF, 0xFFFF, 0xC5D4, 0x903A, 0x2144,
     0x0C00, 0x2186, 0x0FF3, 0xB844, 0xB948,
     0xE082, 0x20CC, 0x80E2, 0x21CC, 0x80A2,
     0x21CC, 0x80E2, 0xF404, 0xD801, 0xF003,
     0xD800, 0x7EE0, 0xC0F1, 0x08BA},

    {0x0600, 0xC1A1, 0x76CF, 0xFFFF, 0xC130,
     0x6E04, 0xC040, 0x71CF, 0xFFFF, 0xC790,
     0x8103, 0x77CF, 0xFFFF, 0xC7C0, 0xE001,
     0xA103, 0xD800, 0x0C6A, 0x04E0, 0xB89E,
     0x7508, 0x8E1C, 0x0809, 0x0191},

    {0xD801, 0xAE1D, 0xE580, 0x20CA, 0x0022,
     0x20CF, 0x0522, 0x0C5C, 0x04E2, 0x21CA,
     0x0062, 0xE580, 0xD901, 0x79C0, 0xD800,
     0x0BE6, 0x04E0, 0xB89E, 0x70CF, 0xFFFF,
     0xC8D4, 0x9002, 0x0857, 0x025E},

    {0xFFDC, 0xE080, 0x25CC, 0x9022, 0xF225,
     0x1700, 0x108A, 0x73CF, 0xFF00, 0x3174,
     0x9307, 0x2A04, 0x103E, 0x9328, 0x2942,
     0x7140, 0x2A04, 0x107E, 0x9349, 0x2942,
     0x7141, 0x2A04, 0x10BE, 0x934A},

    {0x2942, 0x714B, 0x2A04, 0x10BE, 0x130C,
     0x010A, 0x2942, 0x7142, 0x2250, 0x13CA,
     0x1B0C, 0x0284, 0xB307, 0xB328, 0x1B12,
     0x02C4, 0xB34A, 0xED88, 0x71CF, 0xFF00,
     0x3174, 0x9106, 0xB88F, 0xB106},

    {0x210A, 0x8340, 0xC000, 0x21CA, 0x0062,
     0x20F0, 0x0040, 0x0B02, 0x0320, 0xD901,
     0x07F1, 0x05E0, 0xC0A1, 0x78E0, 0xC0F1,
     0x71CF, 0xFFFF, 0xC7C0, 0xD840, 0xA900,
     0x71CF, 0xFFFF, 0xD02C, 0xD81E},

    {0x0A5A, 0x04E0, 0xDA00, 0xD800, 0xC0D1,
     0x7EE0},

    {0x70CF, 0xFFFF, 0xC5D4, 0x903A, 0x2144,
     0x0C00, 0x2186, 0x0FF3, 0xB844, 0x262F,
     0xF008, 0xB948, 0x21CC, 0x8021, 0xD801,
     0xF203, 0xD800, 0x7EE0, 0xC0F1, 0x71CF,
     0xFFFF, 0xC610, 0x910E, 0x208C},

    {0x8014, 0xF418, 0x910F, 0x208C, 0x800F,
     0xF414, 0x9116, 0x208C, 0x800A, 0xF410,
     0x9117, 0x208C, 0x8807, 0xF40C, 0x9118,
     0x2086, 0x0FF3, 0xB848, 0x080D, 0x0090,
     0xFFEA, 0xE081, 0xD801, 0xF203},

    {0xD800, 0xC0D1, 0x7EE0, 0x78E0, 0xC0F1,
     0x71CF, 0xFFFF, 0xC610, 0x910E, 0x208C,
     0x800A, 0xF418, 0x910F, 0x208C, 0x8807,
     0xF414, 0x9116, 0x208C, 0x800A, 0xF410,
     0x9117, 0x208C, 0x8807, 0xF40C},

    {0x9118, 0x2086, 0x0FF3, 0xB848, 0x080D,
     0x0090, 0xFFD9, 0xE080, 0xD801, 0xF203,
     0xD800, 0xF1DF, 0x9040, 0x71CF, 0xFFFF,
     0xC5D4, 0xB15A, 0x9041, 0x73CF, 0xFFFF,
     0xC7D0, 0xB140, 0x9042, 0xB141},

    {0x9043, 0xB142, 0x9044, 0xB143, 0x9045,
     0xB147, 0x9046, 0xB148, 0x9047, 0xB14B,
     0x9048, 0xB14C, 0x9049, 0x1958, 0x0084,
     0x904A, 0x195A, 0x0084, 0x8856, 0x1B36,
     0x8082, 0x8857, 0x1B37, 0x8082},

    {0x904C, 0x19A7, 0x009C, 0x881A, 0x7FE0,
     0x1B54, 0x8002, 0x78E0, 0x71CF, 0xFFFF,
     0xC350, 0xD828, 0xA90B, 0x8100, 0x01C5,
     0x0320, 0xD900, 0x78E0, 0x220A, 0x1F80,
     0xFFFF, 0xD4E0, 0xC0F1, 0x0811},

    {0x0051, 0x2240, 0x1200, 0xFFE1, 0xD801,
     0xF006, 0x2240, 0x1900, 0xFFDE, 0xD802,
     0x1A05, 0x1002, 0xFFF2, 0xF195, 0xC0F1,
     0x0E7E, 0x05C0, 0x75CF, 0xFFFF, 0xC84C,
     0x9502, 0x77CF, 0xFFFF, 0xC344},

    {0x2044, 0x008E, 0xB8A1, 0x0926, 0x03E0,
     0xB502, 0x9502, 0x952E, 0x7E05, 0xB5C2,
     0x70CF, 0xFFFF, 0xC610, 0x099A, 0x04A0,
     0xB026, 0x0E02, 0x0560, 0xDE00, 0x0A12,
     0x0320, 0xB7C4, 0x0B36, 0x03A0},

    {0x70C9, 0x9502, 0x7608, 0xB8A8, 0xB502,
     0x70CF, 0x0000, 0x5536, 0x7860, 0x2686,
     0x1FFB, 0x9502, 0x78C5, 0x0631, 0x05E0,
     0xB502, 0x72CF, 0xFFFF, 0xC5D4, 0x923A,
     0x73CF, 0xFFFF, 0xC7D0, 0xB020},

    {0x9220, 0xB021, 0x9221, 0xB022, 0x9222,
     0xB023, 0x9223, 0xB024, 0x9227, 0xB025,
     0x9228, 0xB026, 0x922B, 0xB027, 0x922C,
     0xB028, 0x1258, 0x0101, 0xB029, 0x125A,
     0x0101, 0xB02A, 0x1336, 0x8081},

    {0xA836, 0x1337, 0x8081, 0xA837, 0x12A7,
     0x0701, 0xB02C, 0x1354, 0x8081, 0x7FE0,
     0xA83A, 0x78E0, 0xC0F1, 0x0DC2, 0x05C0,
     0x7608, 0x09BB, 0x0010, 0x75CF, 0xFFFF,
     0xD4E0, 0x8D21, 0x8D00, 0x2153},

    {0x0003, 0xB8C0, 0x8D45, 0x0B23, 0x0000,
     0xEA8F, 0x0915, 0x001E, 0xFF81, 0xE808,
     0x2540, 0x1900, 0xFFDE, 0x8D00, 0xB880,
     0xF004, 0x8D00, 0xB8A0, 0xAD00, 0x8D05,
     0xE081, 0x20CC, 0x80A2, 0xDF00},

    {0xF40A, 0x71CF, 0xFFFF, 0xC84C, 0x9102,
     0x7708, 0xB8A6, 0x2786, 0x1FFE, 0xB102,
     0x0B42, 0x0180, 0x0E3E, 0x0180, 0x0F4A,
     0x0160, 0x70C9, 0x8D05, 0xE081, 0x20CC,
     0x80A2, 0xF429, 0x76CF, 0xFFFF},

    {0xC84C, 0x082D, 0x0051, 0x70CF, 0xFFFF,
     0xC90C, 0x8805, 0x09B6, 0x0360, 0xD908,
     0x2099, 0x0802, 0x9634, 0xB503, 0x7902,
     0x1523, 0x1080, 0xB634, 0xE001, 0x1D23,
     0x1002, 0xF00B, 0x9634, 0x9503},

    {0x6038, 0xB614, 0x153F, 0x1080, 0xE001,
     0x1D3F, 0x1002, 0xFFA4, 0x9602, 0x7F05,
     0xD800, 0xB6E2, 0xAD05, 0x0511, 0x05E0,
     0xD800, 0xC0F1, 0x0CFE, 0x05C0, 0x0A96,
     0x05A0, 0x7608, 0x0C22, 0x0240},

    {0xE080, 0x20CA, 0x0F82, 0x0000, 0x190B,
     0x0C60, 0x05A2, 0x21CA, 0x0022, 0x0C56,
     0x0240, 0xE806, 0x0E0E, 0x0220, 0x70C9,
     0xF048, 0x0896, 0x0440, 0x0E96, 0x0400,
     0x0966, 0x0380, 0x75CF, 0xFFFF},

    {0xD4E0, 0x8D00, 0x084D, 0x001E, 0xFF47,
     0x080D, 0x0050, 0xFF57, 0x0841, 0x0051,
     0x8D04, 0x9521, 0xE064, 0x790C, 0x702F,
     0x0CE2, 0x05E0, 0xD964, 0x72CF, 0xFFFF,
     0xC700, 0x9235, 0x0811, 0x0043},

    {0xFF3D, 0x080D, 0x0051, 0xD801, 0xFF77,
     0xF025, 0x9501, 0x9235, 0x0911, 0x0003,
     0xFF49, 0x080D, 0x0051, 0xD800, 0xFF72,
     0xF01B, 0x0886, 0x03E0, 0xD801, 0x0EF6,
     0x03C0, 0x0F52, 0x0340, 0x0DBA},

    {0x0200, 0x0AF6, 0x0440, 0x0C22, 0x0400,
     0x0D72, 0x0440, 0x0DC2, 0x0200, 0x0972,
     0x0440, 0x0D3A, 0x0220, 0xD820, 0x0BFA,
     0x0260, 0x70C9, 0x0451, 0x05C0, 0x78E0,
     0xD900, 0xF00A, 0x70CF, 0xFFFF},

    {0xD520, 0x7835, 0x8041, 0x8000, 0xE102,
     0xA040, 0x09F1, 0x8114, 0x71CF, 0xFFFF,
     0xD4E0, 0x70CF, 0xFFFF, 0xC594, 0xB03A,
     0x7FE0, 0xD800, 0x0000, 0x0000, 0x0500,
     0x0500, 0x0200, 0x0330, 0x0000},

    {0x0000, 0x03CD, 0x050D, 0x01C5, 0x03B3,
     0x00E0, 0x01E3, 0x0280, 0x01E0, 0x0109,
     0x0080, 0x0500, 0x0000, 0x0000, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000},

    {0x0000, 0x0000, 0xFFFF, 0xC9B4, 0xFFFF,
     0xD324, 0xFFFF, 0xCA34, 0xFFFF, 0xD3EC},

};

static const MT9M114_REG mt9m114_cfg_resolution[] = {

#if ((MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT == 720) || (MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT == 728))
    { 0x301A, 0x0230,    2 },    // RESET_REGISTER
    { 0x98E, 0x1000,     2 },    // LOGICAL_ADDRESS_ACCESS
    { 0xC97E, 0x01,      1 },    // cam_sysctl_pll_enable = 1
    { 0xC980, 0x0120,    2 },    // cam_sysctl_pll_divider_m_n = 288
    { 0xC982, 0x0700,    2 },    // cam_sysctl_pll_divider_p = 1792
    { 0xC984, 0x8000,    2 },    // cam_port_output_control = 32768
    { 0xC800, 0x007C,    2 },    // cam_sensor_cfg_y_addr_start = 124
    { 0xC802, 0x0004,    2 },    // cam_sensor_cfg_x_addr_start = 4
    { 0xC804, 0x0353,    2 },    // cam_sensor_cfg_y_addr_end = 851
    { 0xC806, 0x050B,    2 },    // cam_sensor_cfg_x_addr_end = 1291
    { 0xC808, 0x2DC6C00, 4 },    // cam_sensor_cfg_pixclk = 48000000
    { 0xC80C, 0x0001,    2 },    // cam_sensor_cfg_row_speed = 1
    { 0xC80E, 0x00DB,    2 },    // cam_sensor_cfg_fine_integ_time_min = 219
    { 0xC810, 0x05BD,    2 },    // cam_sensor_cfg_fine_integ_time_max = 1469
    { 0xC812, 0x03E8,    2 },    // cam_sensor_cfg_frame_length_lines = 1000
    { 0xC814, 0x0640,    2 },    // cam_sensor_cfg_line_length_pck = 1600
    { 0xC816, 0x0060,    2 },    // cam_sensor_cfg_fine_correction = 96
    { 0xC818, 0x02D3,    2 },    // cam_sensor_cfg_cpipe_last_row = 723
    { 0xC826, 0x0020,    2 },    // cam_sensor_cfg_reg_0_data = 32
    { 0xC834, 0x0000,    2 },    // cam_sensor_control_read_mode = 0
    { 0xC854, 0x0000,    2 },    // cam_crop_window_xoffset = 0
    { 0xC856, 0x0000,    2 },    // cam_crop_window_yoffset = 0
    { 0xC858, 0x0500,    2 },    // cam_crop_window_width = 1280
    { 0xC85A, 0x02D0,    2 },    // cam_crop_window_height = 720
    { 0xC85C, 0x03,      1 },    // cam_crop_cropmode = 3
    { 0xC868, 0x0500,    2 },    // cam_output_width = 1280
    { 0xC86A, 0x02D0,    2 },    // cam_output_height = 720
    { 0xC878, 0x0C,      1 },    // cam_aet_aemode = C
    { 0xC88C, 0x1E00,    2 },    // cam_aet_max_frame_rate = 7680
    { 0xC88E, 0x1E00,    2 },    // cam_aet_min_frame_rate = 7680
    { 0xC914, 0x0000,    2 },    // cam_stat_awb_clip_window_xstart = 0
    { 0xC916, 0x0000,    2 },    // cam_stat_awb_clip_window_ystart = 0
    { 0xC918, 0x04FF,    2 },    // cam_stat_awb_clip_window_xend = 1279
    { 0xC91A, 0x02CF,    2 },    // cam_stat_awb_clip_window_yend = 719
    { 0xC91C, 0x0000,    2 },    // cam_stat_ae_initial_window_xstart = 0
    { 0xC91E, 0x0000,    2 },    // cam_stat_ae_initial_window_ystart = 0
    { 0xC920, 0x00FF,    2 },    // cam_stat_ae_initial_window_xend = 255
    { 0xC922, 0x008F,    2 },    // cam_stat_ae_initial_window_yend = 143
    { 0xE801, 0x00,      1 },    // AUTO_BINNING_MODE

#elif (MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT == 480)
    { 0x301A, 0x0230,    2 },    // RESET_REGISTER
    { 0x98E, 0x1000,     2 },    // LOGICAL_ADDRESS_ACCESS
    { 0xC97E, 0x01,      1 },    // cam_sysctl_pll_enable = 1
    { 0xC980, 0x0120,    2 },    // cam_sysctl_pll_divider_m_n = 288
    { 0xC982, 0x0700,    2 },    // cam_sysctl_pll_divider_p = 1792
    { 0xC984, 0x8000,    2 },    // cam_port_output_control = 32768
    { 0xC800, 0x0004,    2 },    // cam_sensor_cfg_y_addr_start = 4
    { 0xC802, 0x0004,    2 },    // cam_sensor_cfg_x_addr_start = 4
    { 0xC804, 0x03CB,    2 },    // cam_sensor_cfg_y_addr_end = 971
    { 0xC806, 0x050B,    2 },    // cam_sensor_cfg_x_addr_end = 1291
    { 0xC808, 0x2DC6C00, 4 },    // cam_sensor_cfg_pixclk = 48000000
    { 0xC80C, 0x0001,    2 },    // cam_sensor_cfg_row_speed = 1
    { 0xC80E, 0x00DB,    2 },    // cam_sensor_cfg_fine_integ_time_min = 219
    { 0xC810, 0x05B3,    2 },    // cam_sensor_cfg_fine_integ_time_max = 1459
    { 0xC812, 0x03EE,    2 },    // cam_sensor_cfg_frame_length_lines = 1006
    { 0xC814, 0x0636,    2 },    // cam_sensor_cfg_line_length_pck = 1590
    { 0xC816, 0x0060,    2 },    // cam_sensor_cfg_fine_correction = 96
    { 0xC818, 0x03C3,    2 },    // cam_sensor_cfg_cpipe_last_row = 963
    { 0xC826, 0x0020,    2 },    // cam_sensor_cfg_reg_0_data = 32
    { 0xC834, 0x0000,    2 },    // cam_sensor_control_read_mode = 0
    { 0xC854, 0x0000,    2 },    // cam_crop_window_xoffset = 0
    { 0xC856, 0x0000,    2 },    // cam_crop_window_yoffset = 0
    { 0xC858, 0x0500,    2 },    // cam_crop_window_width = 1280
    { 0xC85A, 0x03C0,    2 },    // cam_crop_window_height = 960
    { 0xC85C, 0x03,      1 },    // cam_crop_cropmode = 3
    { 0xC868, 0x0280,    2 },    // cam_output_width = 640
    { 0xC86A, 0x01E0,    2 },    // cam_output_height = 480
    { 0xC878, 0x0C,      1 },    // cam_aet_aemode = C
    { 0xC88C, 0x1E02,    2 },    // cam_aet_max_frame_rate = 7682
    { 0xC88E, 0x1E02,    2 },    // cam_aet_min_frame_rate = 7682
    { 0xC914, 0x0000,    2 },    // cam_stat_awb_clip_window_xstart = 0
    { 0xC916, 0x0000,    2 },    // cam_stat_awb_clip_window_ystart = 0
    { 0xC918, 0x027F,    2 },    // cam_stat_awb_clip_window_xend = 639
    { 0xC91A, 0x01DF,    2 },    // cam_stat_awb_clip_window_yend = 479
    { 0xC91C, 0x0000,    2 },    // cam_stat_ae_initial_window_xstart = 0
    { 0xC91E, 0x0000,    2 },    // cam_stat_ae_initial_window_ystart = 0
    { 0xC920, 0x007F,    2 },    // cam_stat_ae_initial_window_xend = 127
    { 0xC922, 0x005F,    2 },    // cam_stat_ae_initial_window_yend = 95

#elif (MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT == 240)
    { 0x301A, 0x0230,    2 },    // RESET_REGISTER
    { 0x98E, 0x1000,     2 },    // LOGICAL_ADDRESS_ACCESS
    { 0xC97E, 0x01,      1 },    // cam_sysctl_pll_enable = 1
    { 0xC980, 0x0120,    2 },    // cam_sysctl_pll_divider_m_n = 288
    { 0xC982, 0x0700,    2 },    // cam_sysctl_pll_divider_p = 1792
    { 0xC984, 0x8000,    2 },    // cam_port_output_control = 32768
    { 0xC800, 0x0004,    2 },    // cam_sensor_cfg_y_addr_start = 4
    { 0xC802, 0x0004,    2 },    // cam_sensor_cfg_x_addr_start = 4
    { 0xC804, 0x03CB,    2 },    // cam_sensor_cfg_y_addr_end = 971
    { 0xC806, 0x050B,    2 },    // cam_sensor_cfg_x_addr_end = 1291
    { 0xC808, 0x2DC6C00, 4 },    // cam_sensor_cfg_pixclk = 48000000
    { 0xC80C, 0x0001,    2 },    // cam_sensor_cfg_row_speed = 1
    { 0xC80E, 0x00DB,    2 },    // cam_sensor_cfg_fine_integ_time_min = 219
    { 0xC810, 0x05B3,    2 },    // cam_sensor_cfg_fine_integ_time_max = 1459
    { 0xC812, 0x03EE,    2 },    // cam_sensor_cfg_frame_length_lines = 1006
    { 0xC814, 0x0636,    2 },    // cam_sensor_cfg_line_length_pck = 1590
    { 0xC816, 0x0060,    2 },    // cam_sensor_cfg_fine_correction = 96
    { 0xC818, 0x03C3,    2 },    // cam_sensor_cfg_cpipe_last_row = 963
    { 0xC826, 0x0020,    2 },    // cam_sensor_cfg_reg_0_data = 32
    { 0xC834, 0x0000,    2 },    // cam_sensor_control_read_mode = 0
    { 0xC854, 0x0000,    2 },    // cam_crop_window_xoffset = 0
    { 0xC856, 0x0000,    2 },    // cam_crop_window_yoffset = 0
    { 0xC858, 0x0500,    2 },    // cam_crop_window_width = 1280
    { 0xC85A, 0x03C0,    2 },    // cam_crop_window_height = 960
    { 0xC85C, 0x03,      1 },    // cam_crop_cropmode = 3
    { 0xC868, 0x0140,    2 },    // cam_output_width = 320
    { 0xC86A, 0x00F0,    2 },    // cam_output_height = 240
    { 0xC878, 0x0C,      1 },    // cam_aet_aemode = C
    { 0xC88C, 0x1E02,    2 },    // cam_aet_max_frame_rate = 7682
    { 0xC88E, 0x1E02,    2 },    // cam_aet_min_frame_rate = 7682
    { 0xC914, 0x0000,    2 },    // cam_stat_awb_clip_window_xstart = 0
    { 0xC916, 0x0000,    2 },    // cam_stat_awb_clip_window_ystart = 0
    { 0xC918, 0x013F,    2 },    // cam_stat_awb_clip_window_xend = 319
    { 0xC91A, 0x00EF,    2 },    // cam_stat_awb_clip_window_yend = 239
    { 0xC91C, 0x0000,    2 },    // cam_stat_ae_initial_window_xstart = 0
    { 0xC91E, 0x0000,    2 },    // cam_stat_ae_initial_window_ystart = 0
    { 0xC920, 0x003F,    2 },    // cam_stat_ae_initial_window_xend = 63
    { 0xC922, 0x002F,    2 },    // cam_stat_ae_initial_window_yend = 47

#elif (MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT == 320)
    { 0x301A, 0x0230,    2 },    // RESET_REGISTER
    { 0x98E, 0x1000,     2 },    // LOGICAL_ADDRESS_ACCESS
    { 0xC97E, 0x01,      1 },    // cam_sysctl_pll_enable = 1
    { 0xC980, 0x0120,    2 },    // cam_sysctl_pll_divider_m_n = 288
    { 0xC982, 0x0700,    2 },    // cam_sysctl_pll_divider_p = 1792
    { 0xC984, 0x8000,    2 },    // cam_port_output_control = 32768
    { 0xC800, 0x0004,    2 },    // cam_sensor_cfg_y_addr_start = 4
    { 0xC802, 0x0004,    2 },    // cam_sensor_cfg_x_addr_start = 4
    { 0xC804, 0x03CB,    2 },    // cam_sensor_cfg_y_addr_end = 971
    { 0xC806, 0x050B,    2 },    // cam_sensor_cfg_x_addr_end = 1291
    { 0xC808, 0x2DC6C00, 4 },    // cam_sensor_cfg_pixclk = 48000000
    { 0xC80C, 0x0001,    2 },    // cam_sensor_cfg_row_speed = 1
    { 0xC80E, 0x00DB,    2 },    // cam_sensor_cfg_fine_integ_time_min = 219
    { 0xC810, 0x05B3,    2 },    // cam_sensor_cfg_fine_integ_time_max = 1459
    { 0xC812, 0x03EE,    2 },    // cam_sensor_cfg_frame_length_lines = 1006
    { 0xC814, 0x0636,    2 },    // cam_sensor_cfg_line_length_pck = 1590
    { 0xC816, 0x0060,    2 },    // cam_sensor_cfg_fine_correction = 96
    { 0xC818, 0x03C3,    2 },    // cam_sensor_cfg_cpipe_last_row = 963
    { 0xC826, 0x0020,    2 },    // cam_sensor_cfg_reg_0_data = 32
    { 0xC834, 0x0000,    2 },    // cam_sensor_control_read_mode = 0
    { 0xC854, 0x0000,    2 },    // cam_crop_window_xoffset = 0
    { 0xC856, 0x0000,    2 },    // cam_crop_window_yoffset = 0
    { 0xC858, 0x0500,    2 },    // cam_crop_window_width = 1280
    { 0xC85A, 0x03C0,    2 },    // cam_crop_window_height = 960
    { 0xC85C, 0x03,      1 },    // cam_crop_cropmode = 3
    { 0xC868, 0x0140,    2 },    // cam_output_width = 320
    { 0xC86A, 0x0140,    2 },    // cam_output_height = 320
    { 0xC878, 0x0C,      1 },    // cam_aet_aemode = C
    { 0xC88C, 0x1E02,    2 },    // cam_aet_max_frame_rate = 7682
    { 0xC88E, 0x1E02,    2 },    // cam_aet_min_frame_rate = 7682
    { 0xC914, 0x0000,    2 },    // cam_stat_awb_clip_window_xstart = 0
    { 0xC916, 0x0000,    2 },    // cam_stat_awb_clip_window_ystart = 0
    { 0xC918, 0x013F,    2 },    // cam_stat_awb_clip_window_xend = 319
    { 0xC91A, 0x013F,    2 },    // cam_stat_awb_clip_window_yend = 319
    { 0xC91C, 0x0000,    2 },    // cam_stat_ae_initial_window_xstart = 0
    { 0xC91E, 0x0000,    2 },    // cam_stat_ae_initial_window_ystart = 0
    { 0xC920, 0x003F,    2 },    // cam_stat_ae_initial_window_xend = 63
    { 0xC922, 0x003F,    2 },    // cam_stat_ae_initial_window_yend = 63
#endif

};

static const MT9M114_REG mt9m114_init_sensor_optimization[] = {
    { 0x316A, 0x8270, 2 }, // RESERVED_CORE_316A
    { 0x316C, 0x8270, 2 }, // RESERVED_CORE_316C
    { 0x3ED0, 0x2305, 2 }, // RESERVED_CORE_3ED0
    { 0x3ED2, 0x77CF, 2 }, // RESERVED_CORE_3ED2
    { 0x316E, 0x8202, 2 }, // RESERVED_CORE_316E
    { 0x3180, 0x87FF, 2 }, // RESERVED_CORE_3180
    { 0x30D4, 0x6080, 2 }, // RESERVED_CORE_30D4
    { 0xA802, 0x0008, 2 }, // RESERVED_AE_TRACK_02
    { 0x3E14, 0xFF39, 2 }, // RESERVED_CORE_3E14
};

static const MT9M114_REG mt9m114_load_patch_black_level_correction_fix[] = {
    { 0x0982, 0x0001, 2 }, // ACCESS_CTL_STAT
    { 0x098A, 0x5000, 2 }, // PHYSICAL_ADDRESS_ACCESS
};

static const MT9M114_REG mt9m114_apply_patch_black_level_correction_fix[] = {
    { 0x098E, 0x0000,     2 }, // LOGICAL_ADDRESS_ACCESS
    { 0xE000, 0x010C,     2 }, // patchldr_loader_address
    { 0xE002, 0x0202,     2 }, // patchldr_patch_id
    { 0xE004, 0x41030202, 4 }, // patchldr_firmware_id
};

static const MT9M114_REG mt9m114_load_patch_adaptive_sensitivity[] = {
    { 0x0982, 0x0001, 2 }, // ACCESS_CTL_STAT
    { 0x098A, 0x512C, 2 }, // PHYSICAL_ADDRESS_ACCESS
};

static const MT9M114_REG mt9m114_apply_patch_adaptive_sensitivity[] = {
    { 0x098E, 0x0000,     2 }, // LOGICAL_ADDRESS_ACCESS
    { 0xE000, 0x04B4,     2 }, // patchldr_loader_address
    { 0xE002, 0x0302,     2 }, // patchldr_patch_id
    { 0xE004, 0x41030202, 4 }, // patchldr_firmware_id
};

static const MT9M114_REG mt9m114_init_awb_ae[] = {
    { 0x098E, 0x0000, 2 }, // LOGICAL_ADDRESS_ACCESS
    { 0xC95E, 0x0003, 2 }, // CAM_PGA_PGA_CONTROL
    { 0x098E, 0x0000, 2 }, // LOGICAL_ADDRESS_ACCESS
    { 0xC95E, 0x0000, 2 }, // CAM_PGA_PGA_CONTROL
    { 0xc892, 0x0267, 2 }, // CAM_AWB_CCM_L_0
    { 0xc894, 0xff1a, 2 }, // CAM_AWB_CCM_L_1
    { 0xc896, 0xffb3, 2 }, // CAM_AWB_CCM_L_2
    { 0xc898, 0xff80, 2 }, // CAM_AWB_CCM_L_3
    { 0xc89a, 0x0166, 2 }, // CAM_AWB_CCM_L_4
    { 0xc89c, 0x0003, 2 }, // CAM_AWB_CCM_L_5
    { 0xc89e, 0xff9a, 2 }, // CAM_AWB_CCM_L_6
    { 0xc8a0, 0xfeb4, 2 }, // CAM_AWB_CCM_L_7
    { 0xc8a2, 0x024d, 2 }, // CAM_AWB_CCM_L_8
    { 0xc8a4, 0x01bf, 2 }, // CAM_AWB_CCM_M_0
    { 0xc8a6, 0xff01, 2 }, // CAM_AWB_CCM_M_1
    { 0xc8a8, 0xfff3, 2 }, // CAM_AWB_CCM_M_2
    { 0xc8aa, 0xff75, 2 }, // CAM_AWB_CCM_M_3
    { 0xc8ac, 0x0198, 2 }, // CAM_AWB_CCM_M_4
    { 0xc8ae, 0xfffd, 2 }, // CAM_AWB_CCM_M_5
    { 0xc8b0, 0xff9a, 2 }, // CAM_AWB_CCM_M_6
    { 0xc8b2, 0xfee7, 2 }, // CAM_AWB_CCM_M_7
    { 0xc8b4, 0x02a8, 2 }, // CAM_AWB_CCM_M_8
    { 0xc8b6, 0x01d9, 2 }, // CAM_AWB_CCM_R_0
    { 0xc8b8, 0xff26, 2 }, // CAM_AWB_CCM_R_1
    { 0xc8ba, 0xfff3, 2 }, // CAM_AWB_CCM_R_2
    { 0xc8bc, 0xffb3, 2 }, // CAM_AWB_CCM_R_3
    { 0xc8be, 0x0132, 2 }, // CAM_AWB_CCM_R_4
    { 0xc8c0, 0xffe8, 2 }, // CAM_AWB_CCM_R_5
    { 0xc8c2, 0xffda, 2 }, // CAM_AWB_CCM_R_6
    { 0xc8c4, 0xfecd, 2 }, // CAM_AWB_CCM_R_7
    { 0xc8c6, 0x02c2, 2 }, // CAM_AWB_CCM_R_8
    { 0xc8c8, 0x0075, 2 }, // CAM_AWB_CCM_L_RG_GAIN
    { 0xc8ca, 0x011c, 2 }, // CAM_AWB_CCM_L_BG_GAIN
    { 0xc8cc, 0x009a, 2 }, // CAM_AWB_CCM_M_RG_GAIN
    { 0xc8ce, 0x0105, 2 }, // CAM_AWB_CCM_M_BG_GAIN
    { 0xc8d0, 0x00a4, 2 }, // CAM_AWB_CCM_R_RG_GAIN
    { 0xc8d2, 0x00ac, 2 }, // CAM_AWB_CCM_R_BG_GAIN
    { 0xc8d4, 0x0a8c, 2 }, // CAM_AWB_CCM_L_CTEMP
    { 0xc8d6, 0x0f0a, 2 }, // CAM_AWB_CCM_M_CTEMP
    { 0xc8d8, 0x1964, 2 }, // CAM_AWB_CCM_R_CTEMP
    { 0xC914, 0x0000, 2 }, // CAM_STAT_AWB_CLIP_WINDOW_XSTART
    { 0xC916, 0x0000, 2 }, // CAM_STAT_AWB_CLIP_WINDOW_YSTART
    { 0xC918, 0x04FF, 2 }, // CAM_STAT_AWB_CLIP_WINDOW_XEND
    { 0xC91A, 0x02CF, 2 }, // CAM_STAT_AWB_CLIP_WINDOW_YEND
    { 0xc904, 0x0033, 2 }, // CAM_AWB_AWB_XSHIFT_PRE_ADJ
    { 0xc906, 0x0040, 2 }, // CAM_AWB_AWB_YSHIFT_PRE_ADJ
    { 0xc8f2, 0x03,   1 }, // CAM_AWB_AWB_XSCALE
    { 0xc8f3, 0x02,   1 }, // CAM_AWB_AWB_YSCALE
    { 0xc906, 0x003C, 2 }, // CAM_AWB_AWB_YSHIFT_PRE_ADJ
    { 0xc8f4, 0x0000, 2 }, // CAM_AWB_AWB_WEIGHTS_0
    { 0xc8f6, 0x0000, 2 }, // CAM_AWB_AWB_WEIGHTS_1
    { 0xc8f8, 0x0000, 2 }, // CAM_AWB_AWB_WEIGHTS_2
    { 0xc8fa, 0xe724, 2 }, // CAM_AWB_AWB_WEIGHTS_3
    { 0xc8fc, 0x1583, 2 }, // CAM_AWB_AWB_WEIGHTS_4
    { 0xc8fe, 0x2045, 2 }, // CAM_AWB_AWB_WEIGHTS_5
    { 0xc900, 0x03ff, 2 }, // CAM_AWB_AWB_WEIGHTS_6
    { 0xc902, 0x007c, 2 }, // CAM_AWB_AWB_WEIGHTS_7
    { 0xc90c, 0x80,   1 }, // CAM_AWB_K_R_L
    { 0xc90d, 0x80,   1 }, // CAM_AWB_K_G_L
    { 0xc90e, 0x80,   1 }, // CAM_AWB_K_B_L
    { 0xc90f, 0x88,   1 }, // CAM_AWB_K_R_R
    { 0xc910, 0x80,   1 }, // CAM_AWB_K_G_R
    { 0xc911, 0x80,   1 }, // CAM_AWB_K_B_R
    { 0xc926, 0x0020, 2 }, // CAM_LL_START_BRIGHTNESS
    { 0xc928, 0x009a, 2 }, // CAM_LL_STOP_BRIGHTNESS
    { 0xc946, 0x0070, 2 }, // CAM_LL_START_GAIN_METRIC
    { 0xc948, 0x00f3, 2 }, // CAM_LL_STOP_GAIN_METRIC
    { 0xc952, 0x0020, 2 }, // CAM_LL_START_TARGET_LUMA_BM
    { 0xc954, 0x009a, 2 }, // CAM_LL_STOP_TARGET_LUMA_BM
    { 0xc92a, 0x80,   1 }, // CAM_LL_START_SATURATION
    { 0xc92b, 0x4b,   1 }, // CAM_LL_END_SATURATION
    { 0xc92c, 0x00,   1 }, // CAM_LL_START_DESATURATION
    { 0xc92d, 0xff,   1 }, // CAM_LL_END_DESATURATION
    { 0xc92e, 0x3c,   1 }, // CAM_LL_START_DEMOSAIC
    { 0xc92f, 0x02,   1 }, // CAM_LL_START_AP_GAIN
    { 0xc930, 0x06,   1 }, // CAM_LL_START_AP_THRESH
    { 0xc931, 0x64,   1 }, // CAM_LL_STOP_DEMOSAIC
    { 0xc932, 0x01,   1 }, // CAM_LL_STOP_AP_GAIN
    { 0xc933, 0x0c,   1 }, // CAM_LL_STOP_AP_THRESH
    { 0xc934, 0x3c,   1 }, // CAM_LL_START_NR_RED
    { 0xc935, 0x3c,   1 }, // CAM_LL_START_NR_GREEN
    { 0xc936, 0x3c,   1 }, // CAM_LL_START_NR_BLUE
    { 0xc937, 0x0f,   1 }, // CAM_LL_START_NR_THRESH
    { 0xc938, 0x64,   1 }, // CAM_LL_STOP_NR_RED
    { 0xc939, 0x64,   1 }, // CAM_LL_STOP_NR_GREEN
    { 0xc93a, 0x64,   1 }, // CAM_LL_STOP_NR_BLUE
    { 0xc93b, 0x32,   1 }, // CAM_LL_STOP_NR_THRESH
    { 0xc93c, 0x0020, 2 }, // CAM_LL_START_CONTRAST_BM
    { 0xc93e, 0x009a, 2 }, // CAM_LL_STOP_CONTRAST_BM
    { 0xc940, 0x00dc, 2 }, // CAM_LL_GAMMA
    { 0xc942, 0x38,   1 }, // CAM_LL_START_CONTRAST_GRADIENT
    { 0xc943, 0x30,   1 }, // CAM_LL_STOP_CONTRAST_GRADIENT
    { 0xc944, 0x50,   1 }, // CAM_LL_START_CONTRAST_LUMA_PERCENTAGE
    { 0xc945, 0x19,   1 }, // CAM_LL_STOP_CONTRAST_LUMA_PERCENTAGE
    { 0xc94a, 0x0230, 2 }, // CAM_LL_START_FADE_TO_BLACK_LUMA
    { 0xc94c, 0x0010, 2 }, // CAM_LL_STOP_FADE_TO_BLACK_LUMA
    { 0xc94e, 0x01cd, 2 }, // CAM_LL_CLUSTER_DC_TH_BM
    { 0xc950, 0x05,   1 }, // CAM_LL_CLUSTER_DC_GATE_PERCENTAGE
    { 0xc951, 0x40,   1 }, // CAM_LL_SUMMING_SENSITIVITY_FACTOR
    { 0xc87b, 0x1b,   1 }, // CAM_AET_TARGET_AVERAGE_LUMA_DARK
    { 0xc878, 0x0E,   1 }, // CAM_AET_AEMODE
    { 0xc890, 0x0080, 2 }, // CAM_AET_TARGET_GAIN
    { 0xc886, 0x0100, 2 }, // CAM_AET_AE_MAX_VIRT_AGAIN
    { 0xc87c, 0x005a, 2 }, // CAM_AET_BLACK_CLIPPING_TARGET
    { 0xb42a, 0x05,   1 }, // CCM_DELTA_GAIN
    { 0xa80a, 0x20,   1 }, // AE_TRACK_AE_TRACKING_DAMPENING_SPEED
};

static const MT9M114_REG mt9m114_init_mipi[] = {
    { 0x098E, 0x0000, 2 }, // LOGICAL_ADDRESS_ACCESS
    { 0xC984, 0x8000, 2 }, // CAM_PORT_OUTPUT_CONTROL
    { 0x001E, 0x0777, 2 }, // PAD_SLEW
    { 0x098E, 0x0000, 2 }, // LOGICAL_ADDRESS_ACCESS
    { 0xC984, 0x8001, 2 }, // CAM_PORT_OUTPUT_CONTROL
    { 0xC988, 0x0F00, 2 }, // CAM_PORT_MIPI_TIMING_T_HS_ZERO
    { 0xC98A, 0x0B07, 2 }, // CAM_PORT_MIPI_TIMING_T_HS_EXIT_HS_TRAIL
    { 0xC98C, 0x0D01, 2 }, // CAM_PORT_MIPI_TIMING_T_CLK_POST_CLK_PRE
    { 0xC98E, 0x071D, 2 }, // CAM_PORT_MIPI_TIMING_T_CLK_TRAIL_CLK_ZERO
    { 0xC990, 0x0006, 2 }, // CAM_PORT_MIPI_TIMING_T_LPX
    { 0xC992, 0x0A0C, 2 }, // CAM_PORT_MIPI_TIMING_INIT_TIMING
    { 0x3C5A, 0x0009, 2 }, // RESERVED_TX_SS_3C5A
};

static const MT9M114_REG mt9m114_speedup_awb_ae[] = {
    { 0x098E, 0x2802, 2}, // LOGICAL_ADDRESS_ACCESS
    { 0xA802, 0x0008, 2}, // RESERVED_AE_TRACK_02
    { 0xC908, 0x01,   1}, // RESERVED_CAM_108
    { 0xC878, 0x01,   1}, // CAM_AET_SKIP_FRAMES
    { 0xC909, 0x02 ,  1}, // CAM_AWB_AWBMODE
    { 0xA80A, 0x18,   1}, // AE_TRACK_AE_TRACKING_DAMPENING_SPEED
    { 0xA80B, 0x18,   1}, // AE_TRACK_AE_DAMPENING_SPEED
    { 0xAC16, 0x18,   1}, // AWB_PRE_AWB_RATIOS_TRACKING_SPEED
    { 0xC878, 0x0E,   1}, // CAM_AET_AEMODE
};
#endif

#if(RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)

#include "Driver_GPIO.h"
#include "sys_ctrl_cpi.h"
/* MT9M114 Camera reset GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_RST =
                        &ARM_Driver_GPIO_(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_GPIO_PORT);

/* MT9M114 Camera power GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_PWR =
                        &ARM_Driver_GPIO_(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_GPIO_PORT);

/**
  \fn           void MT9M114_Sensor_Enable_Clk_Src(void)
  \brief        Enable MT9M114 Camera Sensor external clock source configuration.
  \param[in]    none
  \return       none
  */
static void MT9M114_Sensor_Enable_Clk_Src(void)
{
    set_cpi_pixel_clk(CPI_PIX_CLKSEL_400MZ,
              RTE_MT9M114_CAMERA_SENSOR_MIPI_CSI_CLK_SCR_DIV);
}

/**
  \fn           int32_t mt9m114_power_on(void)
  \brief        Power On MT9M114 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t mt9m114_power_on(void)
{
    int32_t  ret = 0;

    ret = GPIO_Driver_CAM_PWR->Initialize(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->PowerControl(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_PIN_NO,
                                            ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetDirection(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_PIN_NO,
                                            GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->Initialize(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO,
                                            ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetDirection(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO,
                                            GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_PIN_NO,
                                        GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO,
                                        GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_PIN_NO,
                                        GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /*Enable camera sensor clock source config*/
    MT9M114_Sensor_Enable_Clk_Src();

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO,
                                        GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    MT9M114_DELAY_mSEC(50);

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO,
                                        GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_power_off(void)
  \brief        Power Off MT9M114 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t mt9m114_power_off(void)
{
    int32_t  ret = 0;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO,
                                        GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_PIN_NO,
                                        GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO,
                                            ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->Uninitialize(RTE_MT9M114_CAMERA_SENSOR_MIPI_RESET_PIN_NO);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->PowerControl(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_PIN_NO,
                                            ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->Uninitialize(RTE_MT9M114_CAMERA_SENSOR_MIPI_POWER_PIN_NO);
    if(ret != ARM_DRIVER_OK)
        return ret;

    clear_cpi_pixel_clk();

    return ARM_DRIVER_OK;
}
#endif
/**
  \fn           int32_t mt9m114_bulk_write_reg(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg,
                                               const MT9M114_REG mt9m114_reg[],
                                               uint32_t total_num)
  \brief        write array of registers value to MT9M114 Camera Sensor registers.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \param[in]    mt9m114_reg : MT9M114 Camera Sensor Register Array Structure
  \param[in]    total_num   : total number of registers(size of array)
  \return       \ref execution_status
  */
static int32_t mt9m114_bulk_write_reg(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg,
                                      const MT9M114_REG mt9m114_reg[],
                                      uint32_t total_num)
{
    uint32_t i  = 0;
    int32_t ret = 0;

    for(i = 0; i < total_num; i++)
    {
        ret = camera_sensor_i2c_write(i2c_cfg, mt9m114_reg[i].reg_addr,
                                      mt9m114_reg[i].reg_value,
                                      mt9m114_reg[i].reg_size);
        if(ret != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_soft_reset(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Software Reset MT9M114 Camera Sensor
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \return       \ref execution_status
  */
static int32_t mt9m114_soft_reset(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    int32_t ret = 0;

    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_SYSCTL_REGISTER_RESET_AND_MISC_CONTROL,
                                  0x0001,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    MT9M114_DELAY_mSEC(10);

    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_SYSCTL_REGISTER_RESET_AND_MISC_CONTROL,
                                  0x0000,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    /* @Observation: more delay is required for Camera Sensor
     *               to setup after Soft Reset.
     */
    MT9M114_DELAY_mSEC(100);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_wait_for_command(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg,
                                                 uint32_t command)
  \brief        wait for System State command to complete.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \param[in]    command  : MT9M114 Camera Sensor command
  \return       \ref execution_status
  */
static int32_t mt9m114_wait_for_command(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg,
                                        uint32_t command)
{
    uint32_t i = 0;
    uint32_t reg_value = 0;
    int32_t  ret = 0;

    /* wait for System State command to complete. */
    for(i = 0; i < 2000; ++i)
    {
        ret = camera_sensor_i2c_read(i2c_cfg,
                                     MT9M114_COMMAND_REGISTER,
                                     &reg_value,2);
        if(ret != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;

        if(!(reg_value & command))
            break;

        MT9M114_DELAY_mSEC(1);
    }

    if(reg_value & command)
        return ARM_DRIVER_ERROR;

    if(!(reg_value & MT9M114_COMMAND_REGISTER_OK))
        return ARM_DRIVER_ERROR;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_set_system_state(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg,
                                                 uint8_t next_state)
  \brief        Set the desired next System State.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \param[in]    next_state  : System State which needs to be set.
  - \ref Valid MT9M114 Camera Sensor System States:
  - MT9M114_SYS_STATE_ENTER_CONFIG_CHANGE
  - MT9M114_SYS_STATE_START_STREAMING
  - MT9M114_SYS_STATE_ENTER_SUSPEND
  - MT9M114_SYS_STATE_ENTER_STANDBY
  - MT9M114_SYS_STATE_LEAVE_STANDBY
  \return       \ref execution_status
  */
static int32_t mt9m114_set_system_state(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg,
                                        uint8_t next_state)
{
    int32_t ret = 0;

    /* Set the desired next System State */
    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_SYSMGR_NEXT_STATE,
                                  next_state,
                                  1);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    /* Issue the Set State Command */
    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_COMMAND_REGISTER,
                                  MT9M114_COMMAND_REGISTER_OK |
                                  MT9M114_COMMAND_REGISTER_SET_STATE,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    /* Wait for the FW to complete the command. */
    ret = mt9m114_wait_for_command(i2c_cfg, MT9M114_COMMAND_REGISTER_SET_STATE);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_system_change_config(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Issue Change-Config Command.
  This command must be issued after any change in
  sensor sub-system registers to take effect,
  for detail refer data-sheet.
  Change system state to MT9M114_SYS_STATE_ENTER_CONFIG_CHANGE.
  The Change Config performs the following operations:
  1. Requests the sensor to stop STREAMING
  2. Waits until the sensor stops STREAMING (this
  can take an entire frame time depending on when
  the command was issued)
  3. When the sensor stops streaming, reconfigures all
  subsystems including the sensor
  4. Restarts the sensor
  5. Command completes
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \return       \ref execution_status
  */
static __inline int32_t mt9m114_system_change_config(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    return mt9m114_set_system_state(i2c_cfg, MT9M114_SYS_STATE_ENTER_CONFIG_CHANGE);
}

/**
  \fn           int32_t mt9m114_stream_start(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Start MT9M114 Camera Sensor Streaming,
  change system state to MT9M114_SYS_STATE_START_STREAMING.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration
  \return       \ref execution_status
  */
static __inline int32_t mt9m114_stream_start(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    return mt9m114_set_system_state(i2c_cfg, MT9M114_SYS_STATE_START_STREAMING);
}

/**
  \fn           int32_t mt9m114_stream_stop(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Stop MT9M114 Camera Sensor Streaming,
  change system state to MT9M114_SYS_STATE_ENTER_SUSPEND.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration
  \return       \ref execution_status
  */
static __inline int32_t mt9m114_stream_stop(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    return mt9m114_set_system_state(i2c_cfg, MT9M114_SYS_STATE_ENTER_SUSPEND);
}

#if (RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE || RTE_MT9M114_CAMERA_SENSOR_LPCPI_ENABLE)
/**
  \fn           int32_t mt9m114_parallel_camera_init(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Initialize MT9M114 Parallel Camera Sensor.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration
  \return       \ref execution_status
  */
static int32_t mt9m114_parallel_camera_init(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    uint32_t total_num     = 0;
    uint16_t output_format = 0;
    int32_t  ret = 0;

    total_num = (sizeof(mt9m114_cam_resolution_VGA_640x480) / sizeof(MT9M114_REG));
    ret = mt9m114_bulk_write_reg(i2c_cfg, mt9m114_cam_resolution_VGA_640x480, total_num);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    /* Configure Camera Sensor slew rate */
    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_SYSCTL_REGISTER_SLEW_RATE_CONTROL,
                                  0x0,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    output_format =  MT9M114_CAM_OUTPUT_FORMAT_REGISTER_FORMAT_BAYER |
                     MT9M114_CAM_OUTPUT_FORMAT_REGISTER_BAYER_FORMAT_RAWR10;


    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_CAM_OUTPUT_FORMAT_REGISTER,
                                  output_format,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    /* @NOTE: Issue Change-Config Command to re-configure
     *        all the MT9M114 Camera Sensor sub-system and registers.
     *
     *        This command must be issued after any change in
     *        sensor sub-system registers to take effect,
     *        for detail refer data-sheet.
     */
    ret = mt9m114_system_change_config(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;


    return ARM_DRIVER_OK;
}
#endif

#if (RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)
/**
  \fn           int32_t mt9m114_mipi_camera_init(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Initialize MT9M114 MIPI Camera Sensor.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration
  \return       \ref execution_status
  */
static int32_t mt9m114_mipi_camera_init(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    int32_t  ret = 0;
    uint32_t reg_value = 0;

    /*Camera sensor initialization */
    ret = mt9m114_wait_for_command(i2c_cfg, MT9M114_COMMAND_REGISTER_SET_STATE);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_cfg_resolution,
                                 sizeof(mt9m114_cfg_resolution) /
                                 sizeof(mt9m114_cfg_resolution[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    //Change-Config
    ret = mt9m114_system_change_config(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_init_sensor_optimization,
                                 sizeof(mt9m114_init_sensor_optimization) /
                                 sizeof(mt9m114_init_sensor_optimization[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_load_patch_black_level_correction_fix,
                                 sizeof(mt9m114_load_patch_black_level_correction_fix) /
                                 sizeof(mt9m114_load_patch_black_level_correction_fix[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD000,
                                        (void *)sensor_patch_reg_burst[0],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD030,
                                        (void *)sensor_patch_reg_burst[1],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD060,
                                        (void *)sensor_patch_reg_burst[2],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD090,
                                        (void *)sensor_patch_reg_burst[3],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD0C0,
                                        (void *)sensor_patch_reg_burst[4],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD0F0,
                                        (void *)sensor_patch_reg_burst[5],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD120,
                                        (void *)sensor_patch_reg_burst[6],
                                        2,
                                        6);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_apply_patch_black_level_correction_fix,
                                 sizeof(mt9m114_apply_patch_black_level_correction_fix) /
                                 sizeof(mt9m114_apply_patch_black_level_correction_fix[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_COMMAND_REGISTER,
                                  MT9M114_COMMAND_REGISTER_OK,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_COMMAND_REGISTER,
                                  MT9M114_COMMAND_REGISTER_OK |
                                  MT9M114_COMMAND_REGISTER_APPLY_PATCH,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_wait_for_command(i2c_cfg, MT9M114_COMMAND_REGISTER_APPLY_PATCH);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;


    ret = camera_sensor_i2c_read(i2c_cfg,
                                 MT9M114_COMMAND_REGISTER,
                                 &reg_value,
                                 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    if(!(reg_value & MT9M114_COMMAND_REGISTER_OK))
        return ARM_DRIVER_ERROR;

    /* Check the patch status */
    ret = camera_sensor_i2c_read(i2c_cfg,
                                 MT9M114_PATCH_APPLY_STATUS,
                                 &reg_value,
                                 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    if(reg_value)
        return ARM_DRIVER_ERROR;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_load_patch_adaptive_sensitivity,
                                 sizeof(mt9m114_load_patch_adaptive_sensitivity) /
                                 sizeof(mt9m114_load_patch_adaptive_sensitivity[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD12C,
                                        (void *)sensor_patch_reg_burst[7],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD15C,
                                        (void *)sensor_patch_reg_burst[8],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD18C,
                                        (void *)sensor_patch_reg_burst[9],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD1BC,
                                        (void *)sensor_patch_reg_burst[10],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD1EC,
                                        (void *)sensor_patch_reg_burst[11],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD21C,
                                        (void *)sensor_patch_reg_burst[12],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD24C,
                                        (void *)sensor_patch_reg_burst[13],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD27C,
                                        (void *)sensor_patch_reg_burst[14],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD2AC,
                                        (void *)sensor_patch_reg_burst[15],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD2DC,
                                        (void *)sensor_patch_reg_burst[16],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD30C,
                                        (void *)sensor_patch_reg_burst[17],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD33C,
                                        (void *)sensor_patch_reg_burst[18],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD36C,
                                        (void *)sensor_patch_reg_burst[19],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD39C,
                                        (void *)sensor_patch_reg_burst[20],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD3CC,
                                        (void *)sensor_patch_reg_burst[21],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD3FC,
                                        (void *)sensor_patch_reg_burst[22],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD42C,
                                        (void *)sensor_patch_reg_burst[23],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD45C,
                                        (void *)sensor_patch_reg_burst[24],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD48C,
                                        (void *)sensor_patch_reg_burst[25],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD4BC,
                                        (void *)sensor_patch_reg_burst[26],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD4EC,
                                        (void *)sensor_patch_reg_burst[27],
                                        2,
                                        24);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write_burst(i2c_cfg,
                                        0xD51C,
                                        (void *)sensor_patch_reg_burst[28],
                                        2,
                                        10);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_apply_patch_adaptive_sensitivity,
                                 sizeof(mt9m114_apply_patch_adaptive_sensitivity) /
                                 sizeof(mt9m114_apply_patch_adaptive_sensitivity[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_COMMAND_REGISTER,
                                  MT9M114_COMMAND_REGISTER_OK,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_COMMAND_REGISTER,
                                  MT9M114_COMMAND_REGISTER_OK |
                                  MT9M114_COMMAND_REGISTER_APPLY_PATCH,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_wait_for_command(i2c_cfg, MT9M114_COMMAND_REGISTER_APPLY_PATCH);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;


    ret = camera_sensor_i2c_read(i2c_cfg,
                                 MT9M114_COMMAND_REGISTER,
                                 &reg_value,
                                 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    if(!(reg_value & MT9M114_COMMAND_REGISTER_OK))
        return ARM_DRIVER_ERROR;

    /* Check the patch status */
    ret = camera_sensor_i2c_read(i2c_cfg,
                                 MT9M114_PATCH_APPLY_STATUS,
                                 &reg_value,
                                 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    if(reg_value)
        return ARM_DRIVER_ERROR;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_init_awb_ae,
                                 sizeof(mt9m114_init_awb_ae) /
                                 sizeof(mt9m114_init_awb_ae[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_init_mipi,
                                 sizeof(mt9m114_init_mipi) /
                                 sizeof(mt9m114_init_mipi[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_stream_stop(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_stream_start(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_bulk_write_reg(i2c_cfg,
                                 mt9m114_speedup_awb_ae,
                                 sizeof(mt9m114_speedup_awb_ae) /
                                 sizeof(mt9m114_speedup_awb_ae[0]));
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_system_change_config(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_CAM_OUTPUT_FORMAT_REGISTER,
#if (MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE == 0x2A)
                                  MT9M114_CAM_OUTPUT_FORMAT_REGISTER_FORMAT_BAYER |
                                  MT9M114_CAM_OUTPUT_FORMAT_REGISTER_BAYER_FORMAT_RAWR8,
#elif (MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE == 0x2B)
                                  MT9M114_CAM_OUTPUT_FORMAT_REGISTER_FORMAT_BAYER |
                                  MT9M114_CAM_OUTPUT_FORMAT_REGISTER_BAYER_FORMAT_RAWR10,
#elif (MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE == 0x22)
                                  MT9M114_CAM_OUTPUT_FORMAT_REGISTER_FORMAT_RGB |
                                  MT9M114_CAM_OUTPUT_FORMAT_REGISTER_FORMAT_RGB_565RGB,
#endif
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = camera_sensor_i2c_read(i2c_cfg,
                                 MT9M114_MIPI_CONTROL_REGISTER,
                                 &reg_value,
                                 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    reg_value &= ~MT9M114_MIPI_CONTROL_REGISTER_CONTINUOUS_CLOCK_MODE;

    ret = camera_sensor_i2c_write(i2c_cfg,
                                  MT9M114_MIPI_CONTROL_REGISTER,
                                  reg_value,
                                  2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_system_change_config(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = mt9m114_stream_stop(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t MT9M114_Camera_AE(const uint32_t enable, CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Set camera Auto Exposure
  \param[in]    enable: 0=disable, 1=enable
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \return       \ref execution_statusn
  */
static int32_t MT9M114_Camera_AE(const uint32_t enable, CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    int32_t  ret = 0;
    if(enable)
    {
        ret = camera_sensor_i2c_write(i2c_cfg, MT9M114_AE_TRACK_ALGO_REGISTER, MT9M114_AE_ENABLE, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }
    else
    {
        ret = camera_sensor_i2c_write(i2c_cfg, MT9M114_AE_TRACK_ALGO_REGISTER, MT9M114_AE_DISABLE, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }

    return ARM_DRIVER_OK;
}
#endif

/**
  \fn           int32_t mt9m114_Init(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                                     CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)

  \brief        Initialize MT9M114 Camera Sensor
  \param[in]    cpi_mt9m114_camera_sensor  : Poter to camera sensor.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \return       \ref execution_status
  */
static int32_t mt9m114_Init(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                            CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    int32_t  ret = 0;
    uint32_t rcv_data = 0;

    /* Initialize i2c driver instance depending on
     *  MT9M114 Camera Sensor specific i2c configurations
     *   \ref mt9m114_camera_sensor_i2c_cnfg
     */
    ret = camera_sensor_i2c_init(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

#if (RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)
    if(cpi_mt9m114_camera_sensor->interface == CAMERA_SENSOR_INTERFACE_MIPI)
    {
        ret = mt9m114_power_on();
        if(ret != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;

        MT9M114_DELAY_mSEC(50);
    }
#endif

    /* Soft Reset MT9M114 Camera Sensor */
    ret = mt9m114_soft_reset(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    /* Read MT9M114 Camera Sensor CHIP ID */
    ret = camera_sensor_i2c_read(i2c_cfg,
                                 MT9M114_CHIP_ID_REGISTER,
                                 &rcv_data,
                                 2);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    /* Proceed only if CHIP ID is correct. */
    if(rcv_data != MT9M114_CHIP_ID_REGISTER_VALUE)
        return ARM_DRIVER_ERROR;

#if (RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE || RTE_MT9M114_CAMERA_SENSOR_LPCPI_ENABLE)
    if(cpi_mt9m114_camera_sensor->interface == CAMERA_SENSOR_INTERFACE_PARALLEL)
    {
        ret = mt9m114_parallel_camera_init(i2c_cfg);
        if(ret != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

#if (RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)
    if(cpi_mt9m114_camera_sensor->interface == CAMERA_SENSOR_INTERFACE_MIPI)
    {
        ret = mt9m114_mipi_camera_init(i2c_cfg);
        if(ret != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_Start(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                                      CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Start MT9M114 Camera Sensor Streaming.
  \param[in]    cpi_mt9m114_camera_sensor  : Poter to camera sensor.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \return       \ref execution_status
  */
static int32_t mt9m114_Start(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                             CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    int32_t ret = 0;

    ARG_UNUSED(cpi_mt9m114_camera_sensor);
    ARG_UNUSED(i2c_cfg);

    /* Start streaming */
    ret = mt9m114_stream_start(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_Stop(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                                     CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Stop MT9M114 Camera Sensor Streaming.
  \param[in]    cpi_mt9m114_camera_sensor  : Poter to camera sensor.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \return       \ref execution_status
  */
static int32_t mt9m114_Stop(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                            CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{
    int32_t ret = 0;

    ARG_UNUSED(cpi_mt9m114_camera_sensor);
    ARG_UNUSED(i2c_cfg);

    /* Suspend any stream */
    ret = mt9m114_stream_stop(i2c_cfg);
    if(ret != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_Control(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                                        CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg,
                                        uint32_t control,
                                        uint32_t arg)
  \brief        Control MT9M114 Camera Sensor.
  \param[in]    cpi_mt9m114_camera_sensor  : Poter to camera sensor.
  \param[in]    i2c_cfg  : Pointer to Camera Sensor i2c configuration.
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
  */
static int32_t mt9m114_Control(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                               CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg,
                               uint32_t control,
                               uint32_t arg)
{
    switch (control)
    {
        case CPI_CAMERA_SENSOR_AE:
#if (RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)
            if(cpi_mt9m114_camera_sensor->interface == CAMERA_SENSOR_INTERFACE_MIPI)
            {
                return MT9M114_Camera_AE(arg, i2c_cfg);
            }
#endif
        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t mt9m114_Uninit(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                                       CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
  \brief        Un-initialize MT9M114 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t mt9m114_Uninit(CAMERA_SENSOR_DEVICE *cpi_mt9m114_camera_sensor,
                              CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c_cfg)
{

    ARG_UNUSED(cpi_mt9m114_camera_sensor);
    ARG_UNUSED(i2c_cfg);

#if (RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)
    int32_t ret = 0;

    if(cpi_mt9m114_camera_sensor->interface == CAMERA_SENSOR_INTERFACE_MIPI)
    {
        ret = mt9m114_power_off();
        if(ret != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

#if (RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE || RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE)

/* MT9M114 CPI Initialize */
static int32_t mt9m114_CPI_Init(void);

/* MT9M114 CPI Un-initialize */
static int32_t mt9m114_CPI_Uninit(void);

/* MT9M114 CPI Start */
static int32_t mt9m114_CPI_Start(void);

/* MT9M114 CPI Stop */
static int32_t mt9m114_CPI_Stop(void);

/* MT9M114 CPI Control */
static int32_t mt9m114_CPI_Control(uint32_t control, uint32_t arg);

/**
  \brief MT9M114 Camera Sensor slave i2c Configuration
  \ref CAMERA_SENSOR_SLAVE_I2C_CONFIG
  */
#if (RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)

/* I2C Instance */
#if(RTE_MT9M114_CAMERA_SENSOR_MIPI_I2C_INSTANCE == 4)
#define CAMERA_SENSOR_I2C_CPI_INSTANCE                       I3C
#else
#define CAMERA_SENSOR_I2C_CPI_INSTANCE                       RTE_MT9M114_CAMERA_SENSOR_MIPI_I2C_INSTANCE
#endif

/* I2C Driver Instance */
extern ARM_DRIVER_I2C ARM_Driver_I2C_(CAMERA_SENSOR_I2C_CPI_INSTANCE);

CAMERA_SENSOR_SLAVE_I2C_CONFIG cpi_mt9m114_camera_sensor_i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(CAMERA_SENSOR_I2C_CPI_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_STANDARD,
    .cam_sensor_slave_addr          = MT9M114_CAMERA_SENSOR_MIPI_SLAVE_ADDR,
    .cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

/**
  \brief MT9M114 Camera Sensor CSi informations
  \ref CSI_INFO
  */
static CSI_INFO csi_mt9m114_config =
{
    .frequency              = RTE_MT9M114_CAMERA_SENSOR_MIPI_CSI_FREQ,
    .dt                     = MT9M114_CAMERA_SENSOR_MIPI_CSI_DATA_TYPE,
    .n_lanes                = RTE_MT9M114_CAMERA_SENSOR_MIPI_CSI_N_LANES,
    .vc_id                  = RTE_MT9M114_CAMERA_SENSOR_MIPI_CSI_VC_ID,
    .cpi_cfg.override       = RTE_MT9M114_CAMERA_SENSOR_MIPI_OVERRIDE_CPI_COLOR_MODE,
    .cpi_cfg.cpi_color_mode = RTE_MT9M114_CAMERA_SENSOR_MIPI_CPI_COLOR_MODE
};
#endif

#if (RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE)

/* I2C Instance */
#if(RTE_MT9M114_CAMERA_SENSOR_CPI_I2C_INSTANCE == 4)
#define CAMERA_SENSOR_I2C_CPI_INSTANCE                       I3C
#else
#define CAMERA_SENSOR_I2C_CPI_INSTANCE                       RTE_MT9M114_CAMERA_SENSOR_CPI_I2C_INSTANCE
#endif

/* I2C Driver Instance */
extern ARM_DRIVER_I2C ARM_Driver_I2C_(CAMERA_SENSOR_I2C_CPI_INSTANCE);

CAMERA_SENSOR_SLAVE_I2C_CONFIG cpi_mt9m114_camera_sensor_i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(CAMERA_SENSOR_I2C_CPI_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_STANDARD,
    .cam_sensor_slave_addr          = MT9M114_PARALLEL_CAMERA_SENSOR_SLAVE_ADDR,
    .cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

/**
  \brief CPI MT9M114 Camera Sensor Configurations
  \ref CPI_INFO
  */
static CPI_INFO cpi_mt9m114_config =
{
    .vsync_wait      = RTE_MT9M114_CAMERA_SENSOR_CPI_VSYNC_WAIT,
    .vsync_mode      = RTE_MT9M114_CAMERA_SENSOR_CPI_VSYNC_MODE,
    .pixelclk_pol    = RTE_MT9M114_CAMERA_SENSOR_CPI_PIXEL_CLK_POL,
    .hsync_pol       = RTE_MT9M114_CAMERA_SENSOR_CPI_HSYNC_POL,
    .vsync_pol       = RTE_MT9M114_CAMERA_SENSOR_CPI_VSYNC_POL,
    .data_mode       = RTE_MT9M114_CAMERA_SENSOR_CPI_DATA_MODE,
    .data_endianness = RTE_MT9M114_CAMERA_SENSOR_CPI_DATA_ENDIANNESS,
    .code10on8       = RTE_MT9M114_CAMERA_SENSOR_CPI_CODE10ON8,
    .data_mask       = RTE_MT9M114_CAMERA_SENSOR_CPI_DATA_MASK,
};
#endif

/**
  \brief MT9M114 Camera Sensor Operations
  \ref CAMERA_SENSOR_OPERATIONS
  */
static CAMERA_SENSOR_OPERATIONS cpi_mt9m114_ops =
{
    .Init    = mt9m114_CPI_Init,
    .Uninit  = mt9m114_CPI_Uninit,
    .Start   = mt9m114_CPI_Start,
    .Stop    = mt9m114_CPI_Stop,
    .Control = mt9m114_CPI_Control,
};

/**
  \brief CPI MT9M114 Camera Sensor Device Structure
Contains:
- CPI MT9M114 Camera Sensor Configurations
- MT9M114 Camera Sensor Operations
\ref CAMERA_SENSOR_DEVICE
*/
#if(RTE_MT9M114_CAMERA_SENSOR_MIPI_ENABLE)
static CAMERA_SENSOR_DEVICE cpi_mt9m114_camera_sensor =
{
    .interface   = CAMERA_SENSOR_INTERFACE_MIPI,
    .width       = MT9M114_CAMERA_SENSOR_MIPI_FRAME_WIDTH,
    .height      = MT9M114_CAMERA_SENSOR_MIPI_FRAME_HEIGHT,
    .csi_info    = &csi_mt9m114_config,
    .ops         = &cpi_mt9m114_ops,
};
#endif

#if (RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE)
static CAMERA_SENSOR_DEVICE cpi_mt9m114_camera_sensor =
{
    .interface   = CAMERA_SENSOR_INTERFACE_PARALLEL,
    .width       = RTE_MT9M114_CAMERA_SENSOR_CPI_FRAME_WIDTH,
    .height      = RTE_MT9M114_CAMERA_SENSOR_CPI_FRAME_HEIGHT,
    .cpi_info    = &cpi_mt9m114_config,
    .ops         = &cpi_mt9m114_ops,
};
#endif

static int32_t mt9m114_CPI_Init(void)
{
    return mt9m114_Init(&cpi_mt9m114_camera_sensor, &cpi_mt9m114_camera_sensor_i2c_cnfg);
}

static int32_t mt9m114_CPI_Uninit(void)
{
    return mt9m114_Uninit(&cpi_mt9m114_camera_sensor, &cpi_mt9m114_camera_sensor_i2c_cnfg);
}

static int32_t mt9m114_CPI_Start(void)
{
    return mt9m114_Start(&cpi_mt9m114_camera_sensor, &cpi_mt9m114_camera_sensor_i2c_cnfg);
}

static int32_t mt9m114_CPI_Stop(void)
{
    return mt9m114_Stop(&cpi_mt9m114_camera_sensor, &cpi_mt9m114_camera_sensor_i2c_cnfg);
}

static int32_t mt9m114_CPI_Control(uint32_t control, uint32_t arg)
{
    return mt9m114_Control(&cpi_mt9m114_camera_sensor, &cpi_mt9m114_camera_sensor_i2c_cnfg, control, arg);
}

/* Registering CPI sensor */
CAMERA_SENSOR(cpi_mt9m114_camera_sensor)
#endif

#if RTE_MT9M114_CAMERA_SENSOR_LPCPI_ENABLE

/* MT9M114 LPCPI Initialize */
static int32_t mt9m114_LPCPI_Init(void);

/* MT9M114 LPCPI Un-initialize */
static int32_t mt9m114_LPCPI_Uninit(void);

/* MT9M114 LPCPI Start */
static int32_t mt9m114_LPCPI_Start(void);

/* MT9M114 LPCPI Stop */
static int32_t mt9m114_LPCPI_Stop(void);

/* MT9M114 LPCPI Control */
static int32_t mt9m114_LPCPI_Control(uint32_t control, uint32_t arg);

/* I2C Instance */
#if(RTE_MT9M114_CAMERA_SENSOR_LPCPI_I2C_INSTANCE == 4)
#define CAMERA_SENSOR_I2C_LPCPI_INSTANCE                     I3C
#else
#define CAMERA_SENSOR_I2C_LPCPI_INSTANCE                     RTE_MT9M114_CAMERA_SENSOR_LPCPI_I2C_INSTANCE
#endif

/* I2C Driver Instance */
extern ARM_DRIVER_I2C ARM_Driver_I2C_(CAMERA_SENSOR_I2C_LPCPI_INSTANCE);

/**
  \brief MT9M114 Camera Sensor slave i2c Configuration
  \ref CAMERA_SENSOR_SLAVE_I2C_CONFIG
  */
CAMERA_SENSOR_SLAVE_I2C_CONFIG lpcpi_mt9m114_camera_sensor_i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(CAMERA_SENSOR_I2C_LPCPI_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_STANDARD,
    .cam_sensor_slave_addr          = MT9M114_PARALLEL_CAMERA_SENSOR_SLAVE_ADDR,
    .cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

/**
  \brief LPCPI MT9M114 Camera Sensor Configurations
  \ref CPI_INFO
  */
static CPI_INFO lpcpi_mt9m114_config =
{
    .pixelclk_pol    = RTE_MT9M114_CAMERA_SENSOR_LPCPI_PIXEL_CLK_POL,
    .hsync_pol       = RTE_MT9M114_CAMERA_SENSOR_LPCPI_HSYNC_POL,
    .vsync_pol       = RTE_MT9M114_CAMERA_SENSOR_LPCPI_VSYNC_POL,
    .vsync_wait      = RTE_MT9M114_CAMERA_SENSOR_LPCPI_VSYNC_WAIT,
    .vsync_mode      = RTE_MT9M114_CAMERA_SENSOR_LPCPI_VSYNC_MODE,
    .data_mode       = RTE_MT9M114_CAMERA_SENSOR_LPCPI_DATA_MODE,
    .data_endianness = RTE_MT9M114_CAMERA_SENSOR_LPCPI_DATA_ENDIANNESS,
    .code10on8       = RTE_MT9M114_CAMERA_SENSOR_LPCPI_CODE10ON8,
};

/**
  \brief MT9M114 Camera Sensor Operations
  \ref CAMERA_SENSOR_OPERATIONS
  */
static CAMERA_SENSOR_OPERATIONS lpcpi_mt9m114_ops =
{
    .Init    = mt9m114_LPCPI_Init,
    .Uninit  = mt9m114_LPCPI_Uninit,
    .Start   = mt9m114_LPCPI_Start,
    .Stop    = mt9m114_LPCPI_Stop,
    .Control = mt9m114_LPCPI_Control,
};

/**
  \brief LPCPI MT9M114 Camera Sensor Device Structure
Contains:
- LPCPI MT9M114 Camera Sensor Configurations
- MT9M114 Camera Sensor Operations
\ref CAMERA_SENSOR_DEVICE
*/
static CAMERA_SENSOR_DEVICE lpcpi_mt9m114_camera_sensor =
{
    .interface   = CAMERA_SENSOR_INTERFACE_PARALLEL,
    .width       = RTE_MT9M114_CAMERA_SENSOR_LPCPI_FRAME_WIDTH,
    .height      = RTE_MT9M114_CAMERA_SENSOR_LPCPI_FRAME_HEIGHT,
    .cpi_info    = &lpcpi_mt9m114_config,
    .ops         = &lpcpi_mt9m114_ops,
};

static int32_t mt9m114_LPCPI_Init(void)
{
    return mt9m114_Init(&lpcpi_mt9m114_camera_sensor, &lpcpi_mt9m114_camera_sensor_i2c_cnfg);
}

static int32_t mt9m114_LPCPI_Uninit(void)
{
    return mt9m114_Uninit(&lpcpi_mt9m114_camera_sensor, &lpcpi_mt9m114_camera_sensor_i2c_cnfg);
}

static int32_t mt9m114_LPCPI_Start(void)
{
    return mt9m114_Start(&lpcpi_mt9m114_camera_sensor, &lpcpi_mt9m114_camera_sensor_i2c_cnfg);
}

static int32_t mt9m114_LPCPI_Stop(void)
{
    return mt9m114_Stop(&lpcpi_mt9m114_camera_sensor, &lpcpi_mt9m114_camera_sensor_i2c_cnfg);
}

static int32_t mt9m114_LPCPI_Control(uint32_t control, uint32_t arg)
{
    return mt9m114_Control(&lpcpi_mt9m114_camera_sensor, &lpcpi_mt9m114_camera_sensor_i2c_cnfg, control, arg);
}

/* Registering CPI sensor */
LPCAMERA_SENSOR(lpcpi_mt9m114_camera_sensor)
#endif

#else
#error " Check the MT9M114 Sensor Selection "
#endif /* RTE_MT9M114_CAMERA_SENSOR_ENABLE */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
