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
 * @file     ILI9806E_LCD_panel.c
 * @author   Prasanna Ravi and Chandra Bhushan Singh
 * @email    prasanna.ravi@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     28-Sep-23
 * @brief    Focus ILI9806E LCD panel driver.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#if RTE_MIPI_DSI_ILI9806E_PANEL

#include "DSI_DCS.h"
#include "Driver_GPIO.h"
#include "display.h"

#if (!defined(RTE_Drivers_MIPI_DSI_ILI9806E_PANEL))
#error "MIPI DSI ILI9806E Panel not configured in RTE_Components.h!"
#endif

/* ILI9806E panel reset GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_ILI9806E_PANEL_RESET_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_Rst = &ARM_Driver_GPIO_(RTE_ILI9806E_PANEL_RESET_GPIO_PORT);

/* ILI9806E panel black light LED GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_ILI9806E_PANEL_BL_LED_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_BLED = &ARM_Driver_GPIO_(RTE_ILI9806E_PANEL_BL_LED_GPIO_PORT);

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
  \brief structure for LCD Page commands.
  */
typedef struct
{
    uint8_t cmd;
    uint8_t data;
}LCD_Page_cmd;

/* command set selection */
uint8_t page0_cmdsel[] = {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x00};
uint8_t page1_cmdsel[] = {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x01};
uint8_t page5_cmdsel[] = {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x05};
uint8_t page6_cmdsel[] = {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x06};
uint8_t page7_cmdsel[] = {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x07};

#if RTE_ILI9806E_PANEL_E43RB_FW405_EN
/*Commands for Page 1*/
static LCD_Page_cmd page1_cfg[] =
{
    {0x08, 0x00}, {0x20, 0x00}, {0x21, 0x01}, {0x30, 0x02},
    {0x31, 0x00}, {0x40, 0x00}, {0x41, 0x33}, {0x42, 0x02},
    {0x43, 0x89}, {0x44, 0x89}, {0x46, 0x34}, {0x50, 0xA8},
    {0x51, 0xA8}, {0x52, 0x00}, {0x53, 0x78}, {0x54, 0x00},
    {0x55, 0x78}, {0x60, 0x07}, {0x61, 0x04}, {0x62, 0x08},
    {0x63, 0x04}, {0xA0, 0x00}, {0xA1, 0x0B}, {0xA2, 0x13},
    {0xA3, 0x0D}, {0xA4, 0x07}, {0xA5, 0x0B}, {0xA6, 0x07},
    {0xA7, 0x06}, {0xA8, 0x07}, {0xA9, 0x0A}, {0xAA, 0x12},
    {0xAB, 0x0D}, {0xAC, 0x11}, {0xAD, 0x0F}, {0xAE, 0x0E},
    {0xAF, 0x0B}, {0xC0, 0x00}, {0xC1, 0x0B}, {0xC2, 0x13},
    {0xC3, 0x0D}, {0xC4, 0x06}, {0xC5, 0x0B}, {0xC6, 0x07},
    {0xC7, 0x06}, {0xC8, 0x07}, {0xC9, 0x0A}, {0xCA, 0x12},
    {0xCB, 0x0D}, {0xCC, 0x11}, {0xCD, 0x0F}, {0xCE, 0x0E},
    {0xCF, 0x0B}
};

/*Commands for Page 6*/
static LCD_Page_cmd page6_cfg[] = {
    {0x00, 0x21}, {0x01, 0x09}, {0x02, 0x00}, {0x03, 0x00},
    {0x04, 0x01}, {0x05, 0x01}, {0x06, 0x98}, {0x07, 0x05},
    {0x08, 0x02}, {0x09, 0x00}, {0x0A, 0x00}, {0x0B, 0x00},
    {0x0C, 0x01}, {0x0D, 0x01}, {0x0E, 0x00}, {0x0F, 0x00},
    {0x10, 0xE0}, {0x11, 0xE0}, {0x12, 0x00}, {0x13, 0x00},
    {0x14, 0x00}, {0x15, 0x43}, {0x16, 0x08}, {0x17, 0x00},
    {0x18, 0x00}, {0x19, 0x00}, {0x1A, 0x00}, {0x1B, 0x00},
    {0x1C, 0x00}, {0x1D, 0x00}, {0x20, 0x01}, {0x21, 0x23},
    {0x22, 0x45}, {0x23, 0x67}, {0x24, 0x01}, {0x25, 0x23},
    {0x26, 0x45}, {0x27, 0x67}, {0x30, 0x01}, {0x31, 0x11},
    {0x32, 0x00}, {0x33, 0x22}, {0x34, 0x22}, {0x35, 0xCB},
    {0x36, 0xDA}, {0x37, 0xAD}, {0x38, 0xBC}, {0x39, 0x66},
    {0x3A, 0x77}, {0x3B, 0x22}, {0x3C, 0x22}, {0x3D, 0x22},
    {0x3E, 0x22}, {0x3F, 0x22}, {0x40, 0x22}
};

/*Commands for Page 7*/
static LCD_Page_cmd page7_cfg[] =
{
    {0x18, 0x1D}, {0x02, 0x77}, {0xE1, 0x79}
};

/*Commands for Page 0*/
static LCD_Page_cmd page0_cfg[] =
{
    {0x36, 0x01},
    {0x3A, 0x70}
};

#endif

#if RTE_ILI9806E_PANEL_E43GB_MW405_EN
/*Commands for Page 1*/
static LCD_Page_cmd page1_cfg[] =
{
    {0x08, 0x10}, {0x20, 0x00}, {0x21, 0x01}, {0x30, 0x02},
    {0x31, 0x00}, {0x40, 0x14}, {0x41, 0x22}, {0x42, 0x02},
    {0x43, 0x84}, {0x44, 0x8A}, {0x50, 0x78}, {0x51, 0x78},
    {0x52, 0x00}, {0x53, 0x2B}, {0x54, 0x00}, {0x55, 0x2B},
    {0x60, 0x07}, {0x61, 0x06}, {0x62, 0x06}, {0x63, 0x04},
    {0xA0, 0x00}, {0xA1, 0x0B}, {0xA2, 0x19}, {0xA3, 0x10},
    {0xA4, 0x06}, {0xA5, 0x0F}, {0xA6, 0x09}, {0xA7, 0x06},
    {0xA8, 0x0C}, {0xA9, 0x0E}, {0xAA, 0x16}, {0xAB, 0x0D},
    {0xAC, 0x15}, {0xAD, 0x0F}, {0xAE, 0x11}, {0xAF, 0x00},
    {0xC0, 0x00}, {0xC1, 0x24}, {0xC2, 0x29}, {0xC3, 0x0C},
    {0xC4, 0x07}, {0xC5, 0x03}, {0xC6, 0x03}, {0xC7, 0x03},
    {0xC8, 0x03}, {0xC9, 0x09}, {0xCA, 0x0D}, {0xCB, 0x01},
    {0xCC, 0x06}, {0xCD, 0x1B}, {0xCE, 0x08}, {0xCF, 0x00}
};

/*Commands for Page 6*/
static LCD_Page_cmd page6_cfg[] = {
    {0x00, 0x20}, {0x01, 0x04}, {0x02, 0x00}, {0x03, 0x00},
    {0x04, 0x01}, {0x05, 0x01}, {0x06, 0x88}, {0x07, 0x04},
    {0x08, 0x01}, {0x09, 0x90}, {0x0A, 0x03}, {0x0B, 0x01},
    {0x0C, 0x01}, {0x0D, 0x01}, {0x0E, 0x00}, {0x0F, 0x00},
    {0x10, 0x55}, {0x11, 0x53}, {0x12, 0x01}, {0x13, 0x0D},
    {0x14, 0x0D}, {0x15, 0x43}, {0x16, 0x0B}, {0x17, 0x00},
    {0x18, 0x00}, {0x19, 0x00}, {0x1A, 0x00}, {0x1B, 0x00},
    {0x1C, 0x00}, {0x1D, 0x00}, {0x20, 0x01}, {0x21, 0x23},
    {0x22, 0x45}, {0x23, 0x67}, {0x24, 0x01}, {0x25, 0x23},
    {0x26, 0x45}, {0x27, 0x67}, {0x30, 0x02}, {0x31, 0x22},
    {0x32, 0x11}, {0x33, 0xAA}, {0x34, 0xBB}, {0x35, 0x66},
    {0x36, 0x00}, {0x37, 0x22}, {0x38, 0x22}, {0x39, 0x22},
    {0x3A, 0x22}, {0x3B, 0x22}, {0x3C, 0x22}, {0x3D, 0x22},
    {0x3E, 0x22}, {0x3F, 0x22}, {0x40, 0x22}
};

/*Commands for Page 5*/
static LCD_Page_cmd page5_cfg[] =
{
    {0x09, 0xFC}, {0x07, 0xBC}
};

/*Commands for Page 0*/
static LCD_Page_cmd page0_cfg[] =
{
    {0x3A, 0x70}
};

#endif

#if RTE_ILI9806E_PANEL_E50RA_MW550_EN
/*Commands for Page 1*/
static LCD_Page_cmd page1_cfg[] =
{
    {0x08, 0x10}, {0x20, 0x00}, {0x21, 0x01}, {0x30, 0x01},
    {0x31, 0x00}, {0x40, 0x16}, {0x41, 0x33}, {0x42, 0x03},
    {0x43, 0x09}, {0x44, 0x06}, {0x50, 0x88}, {0x51, 0x88},
    {0x52, 0x00}, {0x53, 0x49}, {0x55, 0x49}, {0x60, 0x07},
    {0x61, 0x00}, {0x62, 0x07}, {0x63, 0x00}, {0xA0, 0x00},
    {0xA1, 0x09}, {0xA2, 0x11}, {0xA3, 0x0B}, {0xA4, 0x05},
    {0xA5, 0x08}, {0xA6, 0x06}, {0xA7, 0x04}, {0xA8, 0x09},
    {0xA9, 0x0C}, {0xAA, 0x15}, {0xAB, 0x08}, {0xAC, 0x0F},
    {0xAD, 0x12}, {0xAE, 0x09}, {0xAF, 0x00}, {0xC0, 0x00},
    {0xC1, 0x09}, {0xC2, 0x10}, {0xC3, 0x0C}, {0xC4, 0x05},
    {0xC5, 0x08}, {0xC6, 0x06}, {0xC7, 0x04}, {0xC8, 0x08},
    {0xC9, 0x0C}, {0xCA, 0x14}, {0xCB, 0x08}, {0xCC, 0x0F},
    {0xCD, 0x11}, {0xCE, 0x09}, {0xCF, 0x00}
};

/*Commands for Page 6*/
static LCD_Page_cmd page6_cfg[] = {
    {0x00, 0x20}, {0x01, 0x0A}, {0x02, 0x00}, {0x03, 0x00},
    {0x04, 0x01}, {0x05, 0x01}, {0x06, 0x98}, {0x07, 0x06},
    {0x08, 0x01}, {0x09, 0x80}, {0x0A, 0x00}, {0x0B, 0x00},
    {0x0C, 0x01}, {0x0D, 0x01}, {0x0E, 0x05}, {0x0F, 0x00},
    {0x10, 0xF0}, {0x11, 0xF4}, {0x12, 0x01}, {0x13, 0x00},
    {0x14, 0x00}, {0x15, 0xC0}, {0x16, 0x08}, {0x17, 0x00},
    {0x18, 0x00}, {0x19, 0x00}, {0x1A, 0x00}, {0x1B, 0x00},
    {0x1C, 0x00}, {0x1D, 0x00}, {0x20, 0x01}, {0x21, 0x23},
    {0x22, 0x45}, {0x23, 0x67}, {0x24, 0x01}, {0x25, 0x23},
    {0x26, 0x45}, {0x27, 0x67}, {0x30, 0x11}, {0x31, 0x11},
    {0x32, 0x00}, {0x33, 0xEE}, {0x34, 0xFF}, {0x35, 0xBB},
    {0x36, 0xAA}, {0x37, 0xDD}, {0x38, 0xCC}, {0x39, 0x66},
    {0x3A, 0x77}, {0x3B, 0x22}, {0x3C, 0x22}, {0x3D, 0x22},
    {0x3E, 0x22}, {0x3F, 0x22}, {0x40, 0x22}
};

/*Commands for Page 5*/
static LCD_Page_cmd page5_cfg[] =
{
    {0x17, 0x22}, {0x02, 0x77}, {0x26, 0xB2}
};

/*Commands for Page 0*/
static LCD_Page_cmd page0_cfg[] =
{
    {0x3A, 0x70}
};
#endif

/**
  \fn           int32_t ILI9806E_Display_Reset (void)
  \brief        Reset ILI9806E Display Panel
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t ILI9806E_Display_Reset (void)
{
    int32_t ret = 0;

    if(GPIO_Driver_Rst == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    ret = GPIO_Driver_Rst->Initialize(RTE_ILI9806E_PANEL_RESET_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_Rst->PowerControl(RTE_ILI9806E_PANEL_RESET_PIN_NO,
                                        ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_Rst->SetDirection(RTE_ILI9806E_PANEL_RESET_PIN_NO,
                                        GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_Rst->SetValue(RTE_ILI9806E_PANEL_RESET_PIN_NO,
                                    GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    sys_busy_loop_us(5000);

    ret = GPIO_Driver_Rst->SetValue(RTE_ILI9806E_PANEL_RESET_PIN_NO,
                                    GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    sys_busy_loop_us(50000);

    ret = GPIO_Driver_Rst->SetValue(RTE_ILI9806E_PANEL_RESET_PIN_NO,
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
  \fn           int32_t  ILI9806E_BL_LED_Init (void)
  \brief        Initialize GPIO for BL LED of ILI9806E Display Panel.
  \return       \ref execution_status
  */
static int32_t ILI9806E_BL_LED_Init(void)
{
    int32_t ret = 0;

    if(GPIO_Driver_BLED == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    ret = GPIO_Driver_BLED->Initialize(RTE_ILI9806E_PANEL_BL_LED_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_BLED->PowerControl(RTE_ILI9806E_PANEL_BL_LED_PIN_NO,
                                         ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ret = GPIO_Driver_BLED->SetDirection(RTE_ILI9806E_PANEL_BL_LED_PIN_NO,
                                         GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t  ILI9806E_BL_LED_Control (uint8_t state)
  \brief        Control BL LED of ILI9806E Display Panel.
  \param[in]    state ENABLE/DISABLE
  \return       \ref execution_status
  */
static int32_t ILI9806E_BL_LED_Control(uint8_t state)
{
    int32_t ret = 0;

    if(state == ENABLE)
    {
        ret = GPIO_Driver_BLED->SetValue(RTE_ILI9806E_PANEL_BL_LED_PIN_NO,
                                         GPIO_PIN_OUTPUT_STATE_HIGH);
        if(ret != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
    else
    {
        ret = GPIO_Driver_BLED->SetValue(RTE_ILI9806E_PANEL_BL_LED_PIN_NO,
                                         GPIO_PIN_OUTPUT_STATE_LOW);
        if(ret != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }

    return ARM_DRIVER_OK;
}


/**
  \fn           void  ILI9806E_Configure (void)
  \brief        Configure ILI9806E Display Panel
  */
static void ILI9806E_Configure(void)
{
    /*Change to Page 1 CMD*/
    DSI_DCS_Long_Write(page1_cmdsel, ARRAY_SIZE(page1_cmdsel));

    for(uint32_t i = 0; i < ARRAY_SIZE(page1_cfg); i++)
    {
        DSI_DCS_Short_Write(page1_cfg[i].cmd,page1_cfg[i].data);
    }

    /*Change to Page 6 CMD*/
    DSI_DCS_Long_Write(page6_cmdsel, ARRAY_SIZE(page6_cmdsel));

    for(uint32_t i = 0; i < ARRAY_SIZE(page6_cfg); i++)
    {
        DSI_DCS_Short_Write(page6_cfg[i].cmd,page6_cfg[i].data);
    }

#if RTE_ILI9806E_PANEL_E43RB_FW405_EN
    /*Change to Page 7 CMD*/
    DSI_DCS_Long_Write(page7_cmdsel, ARRAY_SIZE(page7_cmdsel));

    for(uint32_t i = 0; i < ARRAY_SIZE(page7_cfg); i++)
    {
        DSI_DCS_Short_Write(page7_cfg[i].cmd,page7_cfg[i].data);
    }
#endif

#if RTE_ILI9806E_PANEL_E43GB_MW405_EN || RTE_ILI9806E_PANEL_E50RA_MW550_EN
    /*Change to Page 5 CMD*/
    DSI_DCS_Long_Write(page5_cmdsel, ARRAY_SIZE(page5_cmdsel));

    for(uint32_t i = 0; i < ARRAY_SIZE(page5_cfg); i++)
    {
        DSI_DCS_Short_Write(page5_cfg[i].cmd,page5_cfg[i].data);
    }
#endif

    /*Change to Page 0 CMD*/
    DSI_DCS_Long_Write(page0_cmdsel, ARRAY_SIZE(page0_cmdsel));

    for(uint32_t i = 0; i < ARRAY_SIZE(page0_cfg); i++)
    {
        DSI_DCS_Short_Write(page0_cfg[i].cmd,page0_cfg[i].data);
    }

    DSI_DCS_CMD_Short_Write(0x11);

    sys_busy_loop_us(100000);
    sys_busy_loop_us(20000);

    DSI_DCS_CMD_Short_Write(0x29);

    sys_busy_loop_us(25000);

    /*Normal Display mode on*/
    DSI_DCS_Short_Write(0x05, 0x13);

    /*All Pixel On*/
    DSI_DCS_Short_Write(0x05, 0x23);

    /*Display On*/
    DSI_DCS_Short_Write(0x05, 0x29);
}

/**
  \fn           int32_t ILI9806E_Init(void)
  \brief        Initialize ILI9806E Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9806E_Init(void)
{
    int32_t ret = ARM_DRIVER_OK;

    ret = ILI9806E_Display_Reset();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    ret = ILI9806E_BL_LED_Init();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ret;
}

/**
  \fn           int32_t ILI9806E_Uninit(void)
  \brief        Un-Initialize ILI9806E Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9806E_Uninit(void)
{
    int32_t  ret = ARM_DRIVER_OK;

    ret = GPIO_Driver_Rst->SetValue(RTE_ILI9806E_PANEL_RESET_PIN_NO,
                                    GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    return ret;
}

/**
  \fn           int32_t ILI9806E_Control(uint32_t control)
  \brief        Control ILI9806E Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9806E_Control(uint32_t control)
{
    switch(control)
    {
        case DISPALY_PANEL_CONFIG:
        {
            ILI9806E_Configure();
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
  \fn           int32_t ILI9806E_start(void)
  \brief        start ILI9806E Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9806E_Start(void)
{
    int32_t  ret = ARM_DRIVER_OK;

    ret = ILI9806E_BL_LED_Control(ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ret;
}

/**
  \fn           int32_t ILI9806E_stop(void)
  \brief        stop ILI9806E Display Panel
  \return       \ref execution_status
  */
static int32_t ILI9806E_Stop(void)
{
    int32_t  ret = ARM_DRIVER_OK;

    ret = ILI9806E_BL_LED_Control(DISABLE);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ret;
}

static DISPLAY_PANEL_OPERATIONS ili9806e_display_ops =
{
    .Init    = ILI9806E_Init,
    .Uninit  = ILI9806E_Uninit,
    .Control = ILI9806E_Control,
    .Start   = ILI9806E_Start,
    .Stop    = ILI9806E_Stop
};

static DSI_INFO ili9806e_dsi_info =
{
    .max_bitrate      = RTE_ILI9806E_PANEL_MAX_BITRATE_MBPS,
    .n_lanes          = RTE_ILI9806E_PANEL_DSI_N_LANES,
    .vc_id            = RTE_ILI9806E_PANEL_DSI_VC_ID,
    .color_coding     = RTE_ILI9806E_PANEL_DSI_COLOR_MODE,
};

static DISPLAY_PANEL_DEVICE ILI9806E_display_panel =
{
    .hsync_time   = RTE_PANEL_HSYNC_TIME,
    .hbp_time     = RTE_PANEL_HBP_TIME,
    .hfp_time     = RTE_PANEL_HFP_TIME,
    .hactive_time = RTE_PANEL_HACTIVE_TIME,
    .vsync_line   = RTE_PANEL_VSYNC_LINE,
    .vbp_line     = RTE_PANEL_VBP_LINE,
    .vfp_line     = RTE_PANEL_VFP_LINE,
    .vactive_line = RTE_PANEL_VACTIVE_LINE,
    .dsi_info     = &ili9806e_dsi_info,
    .ops          = &ili9806e_display_ops,
};

/* Registering Display Panel */
DISPLAY_PANEL(ILI9806E_display_panel)

#endif /* RTE_MIPI_DSI_ILI9806E_PANEL */
