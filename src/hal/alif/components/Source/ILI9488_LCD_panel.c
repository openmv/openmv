/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#if RTE_MIPI_DSI_ILI9488_PANEL

#include "DSI_DCS.h"
#include "Driver_GPIO.h"
#include "display.h"

/* ILI9488 panel reset GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_ILI9488_PANEL_RESET_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_Rst = &ARM_Driver_GPIO_(RTE_ILI9488_PANEL_RESET_GPIO_PORT);

/* ILI9488 panel black light LED GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_ILI9488_PANEL_BL_LED_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_BLED = &ARM_Driver_GPIO_(RTE_ILI9488_PANEL_BL_LED_GPIO_PORT);

#define ILI9488_PANEL_MIPI_DATA_LANES       1

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Panel long packet configurations */

uint8_t pgam_ctrl[] = {0xE0,0x00,0x10,0x14,0x01,0x0E,0x04,0x33,0x56,0x48,0x03,0x0C,0x0B,0x2B,0x34,0x0F};

uint8_t ngam_ctrl[] = {0xE1,0x00,0x12,0x18,0x05,0x12,0x06,0x40,0x34,0x57,0x06,0x10,0x0C,0x3B,0x3F,0x0F};

uint8_t power_ctrl1[] = {0xC0,0x0F,0x0C};

uint8_t vcom_ctrl[] = {0xC5,0x00,0x25,0x80};

uint8_t display_func_ctrl[] = {0xB6,0x02,0x02};

uint8_t adjust_ctrl[] = {0xF7,0xA9,0x51,0x2C,0x82};

/**
  \fn           int32_t ILI9488_Display_Reset (void)
  \brief        Reset ILI9488 Display Panel
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t ILI9488_Display_Reset (void)
{
    int32_t ret = 0;

    if(GPIO_Driver_Rst == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    ret = GPIO_Driver_Rst->Initialize(RTE_ILI9488_PANEL_RESET_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_Rst->PowerControl(RTE_ILI9488_PANEL_RESET_PIN_NO,
                                        ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_Rst->SetDirection(RTE_ILI9488_PANEL_RESET_PIN_NO,
                                        GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_Rst->SetValue(RTE_ILI9488_PANEL_RESET_PIN_NO,
                                    GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    sys_busy_loop_us(5000);

    ret = GPIO_Driver_Rst->SetValue(RTE_ILI9488_PANEL_RESET_PIN_NO,
                                    GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    sys_busy_loop_us(20000);

    ret = GPIO_Driver_Rst->SetValue(RTE_ILI9488_PANEL_RESET_PIN_NO,
                                    GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    sys_busy_loop_us(100000);
    sys_busy_loop_us(50000);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t  ILI9488_BL_LED_Init (void)
  \brief        Initialize GPIO for BL LED of ILI9488 Display Panel.
  \return       \ref execution_status
  */
static int32_t ILI9488_BL_LED_Init(void)
{
    int32_t ret = 0;

    if(GPIO_Driver_BLED == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    ret = GPIO_Driver_BLED->Initialize(RTE_ILI9488_PANEL_BL_LED_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_BLED->PowerControl(RTE_ILI9488_PANEL_BL_LED_PIN_NO,
                                         ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    return GPIO_Driver_BLED->SetDirection(RTE_ILI9488_PANEL_BL_LED_PIN_NO,
                                          GPIO_PIN_DIRECTION_OUTPUT);
}

/**
  \fn           int32_t  ILI9488_BL_LED_Control (uint8_t state)
  \brief        Control BL LED of ILI9488 Display Panel.
  \param[in]    state ENABLE/DISABLE
  \return       \ref execution_status
  */
static int32_t ILI9488_BL_LED_Control(uint8_t state)
{
    int32_t ret = 0;

    if(state == ENABLE)
    {
        ret = GPIO_Driver_BLED->SetValue(RTE_ILI9488_PANEL_BL_LED_PIN_NO,
                                         GPIO_PIN_OUTPUT_STATE_HIGH);
        if(ret != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
    else
    {
        ret = GPIO_Driver_BLED->SetValue(RTE_ILI9488_PANEL_BL_LED_PIN_NO,
                                         GPIO_PIN_OUTPUT_STATE_LOW);
        if(ret != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           void  ILI9488_Configure (void)
  \brief        Configure ILI9488 Display Panel
  */
static void ILI9488_Configure(void)
{
    DSI_DCS_Long_Write(pgam_ctrl, ARRAY_SIZE(pgam_ctrl));

    sys_busy_loop_us(10);

    DSI_DCS_Long_Write(ngam_ctrl, ARRAY_SIZE(ngam_ctrl));

    sys_busy_loop_us(10);

    DSI_DCS_Long_Write(power_ctrl1, ARRAY_SIZE(power_ctrl1));

    sys_busy_loop_us(10);

    DSI_DCS_Short_Write(0xC1, 0x41);

    sys_busy_loop_us(10);

    DSI_DCS_Long_Write(vcom_ctrl, ARRAY_SIZE(vcom_ctrl));

    DSI_DCS_Short_Write(0x36, 0x48);

    DSI_DCS_Short_Write(0x3A, 0x55);

    DSI_DCS_Short_Write(0xB0, 0x00);

    DSI_DCS_Short_Write(0xB1, 0xA0);

    DSI_DCS_Short_Write(0xB4, 0x02);

    DSI_DCS_Long_Write(display_func_ctrl, ARRAY_SIZE(display_func_ctrl));

    DSI_DCS_Short_Write(0xE9, 0x00);

    DSI_DCS_Long_Write(adjust_ctrl, ARRAY_SIZE(adjust_ctrl));

    DSI_DCS_CMD_Short_Write(0x21);

    DSI_DCS_CMD_Short_Write(0x11);

    sys_busy_loop_us(100000);
    sys_busy_loop_us(20000);

    DSI_DCS_CMD_Short_Write(0x29);

    sys_busy_loop_us(20000);
}

/**
  \fn           int32_t ILI9488_Init(void)
  \brief        Initialize ILI9488 Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9488_Init(void)
{
    int32_t ret = ARM_DRIVER_OK;

    ret = ILI9488_Display_Reset();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ILI9488_BL_LED_Init();
}

/**
  \fn           int32_t ILI9488_Uninit(void)
  \brief        Un-Initialize ILI9488 Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9488_Uninit(void)
{
    return GPIO_Driver_Rst->SetValue(RTE_ILI9488_PANEL_RESET_PIN_NO,
                                     GPIO_PIN_OUTPUT_STATE_LOW);
}

/**
  \fn           int32_t ILI9488_Control(uint32_t control)
  \brief        Control ILI9488 Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9488_Control(uint32_t control)
{
    switch(control)
    {
        case DISPALY_PANEL_CONFIG:
        {
            ILI9488_Configure();
            break;
        }
        default:
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t ILI9488_start(void)
  \brief        start ILI9488 Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9488_Start(void)
{
    return ILI9488_BL_LED_Control(ENABLE);
}

/**
  \fn           int32_t ILI9488_stop(void)
  \brief        stop ILI9488 Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9488_Stop(void)
{
    return ILI9488_BL_LED_Control(DISABLE);
}

static DISPLAY_PANEL_OPERATIONS ILI9488_display_ops =
{
    .Init    = ILI9488_Init,
    .Uninit  = ILI9488_Uninit,
    .Control = ILI9488_Control,
    .Start   = ILI9488_Start,
    .Stop    = ILI9488_Stop
};

static DSI_INFO ILI9488_dsi_info =
{
    .max_bitrate      = RTE_ILI9488_PANEL_MAX_BITRATE_MBPS,
    .n_lanes          = ILI9488_PANEL_MIPI_DATA_LANES,
    .vc_id            = RTE_ILI9488_PANEL_DSI_VC_ID,
    .color_coding     = RTE_ILI9488_PANEL_DSI_COLOR_MODE,
};

static DISPLAY_PANEL_DEVICE ILI9488_display_panel =
{
    .hsync_time   = RTE_ILI9488_PANEL_HSYNC_TIME,
    .hbp_time     = RTE_ILI9488_PANEL_HBP_TIME,
    .hfp_time     = RTE_ILI9488_PANEL_HFP_TIME,
    .hactive_time = RTE_ILI9488_PANEL_HACTIVE_TIME,
    .vsync_line   = RTE_ILI9488_PANEL_VSYNC_LINE,
    .vbp_line     = RTE_ILI9488_PANEL_VBP_LINE,
    .vfp_line     = RTE_ILI9488_PANEL_VFP_LINE,
    .vactive_line = RTE_ILI9488_PANEL_VACTIVE_LINE,
    .dsi_info     = &ILI9488_dsi_info,
    .ops          = &ILI9488_display_ops,
};

/* Registering Display Panel */
DISPLAY_PANEL(ILI9488_display_panel)
#endif
