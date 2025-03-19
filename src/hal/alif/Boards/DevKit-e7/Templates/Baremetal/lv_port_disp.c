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
 * @file     lv_port_disp.c
 * @author   Ahmad Rashed
 * @email    ahmad.rashed@alifsemi.com
 * @version  V1.0.0
 * @date     28-March-2022
 * @brief    for lvgl library display init with touch input
 * @bug      None.
 * @Note     None
 ******************************************************************************/

/* System Includes */
#include <stdio.h>

/*RTE configuration includes */
#include <RTE_Device.h>
#include <RTE_Components.h>
#include CMSIS_device_header

/* LVGL driver */
#include "lvgl.h"

/* CDC200 driver */
#include "Driver_CDC200.h"

/* PINMUX Driver */
#include "pinconf.h"

/* SE Services */
#include "se_services_port.h"

#define I2C_TOUCH_ENABLE         1

/* Selecting LVGL color depth in matching with CDC200 controller pixel format */
#if ((LV_COLOR_DEPTH == 16) && (RTE_CDC200_PIXEL_FORMAT != 2)) || \
    ((LV_COLOR_DEPTH == 32) && (RTE_CDC200_PIXEL_FORMAT != 0))
#error "The LV_COLOR_DEPTH and RTE_CDC200_PIXEL_FORMAT must match."
#endif

#if RTE_CDC200_PIXEL_FORMAT   == 0
#define PIXEL_BYTES    (4)
#elif RTE_CDC200_PIXEL_FORMAT == 2
#define PIXEL_BYTES    (2)
#elif RTE_CDC200_PIXEL_FORMAT == 1
#error "Using 24-bit image buffer is not supported by LVGL, use 16-bit or 32-bit only."
#endif

#define DIMAGE_X                 (RTE_PANEL_HACTIVE_TIME)
#define DIMAGE_Y                 (RTE_PANEL_VACTIVE_LINE)

static uint8_t lcd_image[DIMAGE_Y][DIMAGE_X][PIXEL_BYTES] __attribute__((section(".bss.lcd_frame_buf")));
static uint8_t lcd_image2[DIMAGE_Y][DIMAGE_X][PIXEL_BYTES] __attribute__((section(".bss.lcd_frame_buf")));

/* CDC200 driver instance */
extern ARM_DRIVER_CDC200 Driver_CDC200;
static ARM_DRIVER_CDC200 *CDCdrv = &Driver_CDC200;

volatile uint8_t line_irq_status = 0;
volatile uint8_t dsi_err = 0;

/**
  \fn          void hw_disp_cb(uint32_t event)
  \brief       Display callback
  \param[in]   event: Display Event
  \return      none
  */
void hw_disp_cb(uint32_t event)
{
    if(event & ARM_CDC_SCANLINE0_EVENT)
    {
        /* Received scan line event. */
        line_irq_status = 1;
    }

    if(event & ARM_CDC_DSI_ERROR_EVENT)
    {
        /* Transfer Error: Received Hardware error, Wake-up Thread. */
        dsi_err = 1;
    }
}

/**
 * @function    static void hw_disp_init(void)
  \brief        Initializes CDC200 controller
  \param[in]    none
  \return       none
  */
static void hw_disp_init(void)
{
    int ret = 0;
    uint32_t  service_error_code;
    uint32_t  error_code;
    run_profile_t runp = {0};

    /* Initialize the SE services */
    se_services_port_init();

    /* Enable MIPI Clocks */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, true, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
    {
        printf("SE: MIPI 100MHz clock enable = %d\n", error_code);
        return;
    }

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSC, true, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
    {
        printf("SE: MIPI 38.4Mhz(HFOSC) clock enable = %d\n", error_code);
        goto error_disable_100mhz_clk;
    }

    /* Get the current run configuration from SE */
    error_code = SERVICES_get_run_cfg(se_services_s_handle,
                                      &runp,
                                      &service_error_code);
    if(error_code)
    {
        printf("\r\nSE: get_run_cfg error = %d\n", error_code);
        goto error_disable_hfosc_clk;
    }

    /*
     * Note:
     * This demo uses a specific profile setting that only enables the
     * items it needs. For example, it only requests the RAM regions and
     * peripheral power that are relevant for this demo. If you want to adapt
     * this example for your own use case, you should adjust the profile setting
     * accordingly. You can either add any additional items that you need, or
     * remove the request altogether to use the default setting that turns on
     * almost everything.
     */

    runp.memory_blocks = MRAM_MASK | SRAM0_MASK;

    runp.phy_pwr_gating = MIPI_PLL_DPHY_MASK | MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK | LDO_PHY_MASK;

    /* Set the new run configuration */
    error_code = SERVICES_set_run_cfg(se_services_s_handle,
                                      &runp,
                                      &service_error_code);
    if(error_code)
    {
        printf("\r\nSE: set_run_cfg error = %d\n", error_code);
        goto error_disable_hfosc_clk;
    }

    /* Initialize CDC200 controller */
    ret = CDCdrv->Initialize(hw_disp_cb);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in CDC200 initialize */
        printf("\r\n Error: CDC200 initialization failed.\r\n");
        goto error_disable_hfosc_clk;
    }

    /* Power ON CDC200 controller */
    ret = CDCdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in CDC200 Power ON */
        printf("\r\n Error: CDC200 Power ON failed.\r\n");
        goto error_CDC200_uninitialize;
    }

    /* Configure CDC200 controller */
    ret = CDCdrv->Control(CDC200_CONFIGURE_DISPLAY, (uint32_t)lcd_image);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in CDC200 control configuration */
        printf("\r\n Error: CDC200 control configuration failed.\r\n");
        goto error_CDC200_poweroff;
    }

    /* Configure CDC200 controller */
    ret = CDCdrv->Control(CDC200_SCANLINE0_EVENT, ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in CDC200 control configuration */
        printf("\r\n Error: CDC200 control configuration failed.\r\n");
    }

    /* Start CDC200 controller */
    ret = CDCdrv->Start();
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in CDC200 start */
        printf("\r\n Error: CDC200 start failed.\r\n");
    }

    return;

error_CDC200_poweroff:
    /* Received error Power OFF CDC200 driver */
    ret = CDCdrv->PowerControl(ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK)
    {
        /* Error in CDC200 poweroff. */
        printf("ERROR: Could not power off CDC200\n");
        return;
    }

error_CDC200_uninitialize:
    /* Received error Un-initialize CDC200 */
    ret = CDCdrv->Uninitialize();
    if (ret != ARM_DRIVER_OK)
    {
        /* Error in CDC200 uninitialize. */
        printf("ERROR: Could not unintialize CDC200\n");
        return;
    }

error_disable_hfosc_clk:
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSC, false, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
        printf("SE: MIPI 38.4Mhz(HFOSC)  clock disable = %d\n", error_code);

error_disable_100mhz_clk:
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, false, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
        printf("SE: MIPI 100MHz clock disable = %d\n", error_code);
}

#if(I2C_TOUCH_ENABLE == 1)

/*touch screen driver */
#include "Driver_Touch_Screen.h"

/* Touch screen driver instance */
extern ARM_DRIVER_TOUCH_SCREEN GT911;
static ARM_DRIVER_TOUCH_SCREEN *Drv_Touchscreen = &GT911;

/**
  \fn          int hardware_cfg(void)
  \brief       i2c hardware pin initialization:
                   -  PIN-MUX and PIN_PAD configuration
               GPIO9 initialization:
                   -  PIN-MUX and PIN-PAD configuration
  \param[in]   none
  \return      ARM_DRIVER_OK: success; -1: failure
  */
int hardware_cfg(void)
{
    int ret = 0;

    /* Configure GPIO Pin : P9_4 as gpio pin
     *   Pad function: PAD_FUNCTION_READ_ENABLE |
     *                 PAD FUNCTION_DRIVER_DISABLE_STATE_WITH_PULL_UP |
     *                 PADCTRL_SCHMITT_TRIGGER_ENABLE
     */
    ret = pinconf_set(PORT_9, PIN_4, PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE | \
            PADCTRL_DRIVER_DISABLED_PULL_UP | PADCTRL_SCHMITT_TRIGGER_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        return -1;
    }

   /* Configure GPIO Pin : P7_2 as i2c1_sda_c
    * Pad function: PADCTRL_READ_ENABLE |
    *               PAD PADCTRL_DRIVER_DISABLED_PULL_UP
    */
    ret = pinconf_set(PORT_7, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | \
            PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    /* Configure GPIO Pin : P7_3 as i2c1_scl_c
     * Pad function: PADCTRL_READ_ENABLE |
     *               PADCTRL_DRIVER_DISABLED_PULL_UP
     */
    ret = pinconf_set(PORT_7, PIN_3, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | \
            PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    return ARM_DRIVER_OK;
}

/**
  \function     static void lv_touch_get(lv_indev_drv_t * drv, lv_indev_data_t * data)
  \brief        Check touch screen is pressed or not
  \param[in]    drv: pointer to LVGL driver
                data: data to LVGL driver
  \return       none
  */
static void lv_touch_get(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    ARM_TOUCH_STATE status;
    Drv_Touchscreen->GetState(&status);

    if(status.numtouches)
    {
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    data->point.x = status.coordinates[0].x;
    data->point.y = status.coordinates[0].y;
}

/**
  \function	   static void hw_touch_init(void)
  \brief	   This hardware initialization of GT911 touch screen does:
                 - initialize i3c AND gpio2 port hardware pins
                 - initialize GT911 Touch screen driver.
  \param[in]   none
  \return      none
  */
static void hw_touch_init(void)
{
    int ret = 0;

    /* Initialize i3c and GPIO2 hardware pins using PinMux Driver. */
    ret = hardware_cfg();
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in hardware configuration */
        printf("\r\n Error: Hardware configuration failed.\r\n");
    }

    /* Initialize GT911 touch screen */
    ret = Drv_Touchscreen->Initialize();
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in GT911 touch screen initialize */
        printf("\r\n Error: GT911 touch screen initialization failed.\r\n");
        goto error_GT911_uninitialize;
    }

    /* Power ON GT911 touch screen */
    ret = Drv_Touchscreen->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in GT911 touch screen power up */
        printf("\r\n Error: GT911 touch screen Power Up failed.\r\n");
        goto error_GT911_uninitialize;
    }

    return;

error_GT911_uninitialize:
    /* Received error Un-initialize Touch screen driver */
    ret = Drv_Touchscreen->Uninitialize();
    if (ret != ARM_DRIVER_OK)
    {
        /* Error in GT911 Touch screen uninitialize. */
        printf("ERROR: Could not unintialize touch screen\n");
        return;
    }
}
#endif

/**
  \function    void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p))
  \brief       Flush the data in buffer to display.
  \param[in]   disp_drv Pointer descriptor of a display driver.
  \param[in]   area     Area of the buffer containing data to be displayed.
  \param[in]   color_p  buffer containing the data to be flushed to display.
  \return      none
  */
static void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    int ret = 0 ;
    int lp_count = 0;

    while(dsi_err == 0 && line_irq_status == 0)
    {
        if(lp_count++ < 1000000)
        {
            sys_busy_loop_us(1);
        }
        else
        {
            printf("Error: Scan line event did not occurred.\r\n");
            return;
        }
    }

    if(dsi_err == 1)
    {
        printf("Error: DSI error occurred.\r\n");
        return;
    }

    line_irq_status = 0;

    /* Configure CDC200 controller */
    ret = CDCdrv->Control(CDC200_FRAMEBUF_UPDATE, (uint32_t) color_p);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in CDC200 control configuration */
        printf("\r\n Error: CDC200 control configuration failed.\r\n");
    }

    /* Indicating flushing is done to display */
    lv_disp_flush_ready(disp_drv);
}

/**
  \function void lv_port_disp_init(void)
  \brief    This function does:
                - storing data in temporary buffer.
                - flushing the data to display using callback function
                - enabling a touch pointer as a input device to display
  \param[in]   none
  \return      none
  */
void lv_port_disp_init(void)
{
    /* Descriptor of a display driver */
    static lv_disp_drv_t disp_drv;
    static lv_disp_draw_buf_t disp_buf;

    /* Initialize LVGL */
    lv_init();

    /*Initialize the display buffer.*/
    lv_disp_draw_buf_init(&disp_buf, lcd_image, lcd_image2, DIMAGE_Y*DIMAGE_X);

    /* Basic initialization */
    lv_disp_drv_init(&disp_drv);

    /* Assign the buffer to the display */
    disp_drv.draw_buf = &disp_buf;

    /* Set the horizontal resolution of the display */
    disp_drv.hor_res = DIMAGE_X;

    /* Set the vertical resolution of the display */
    disp_drv.ver_res = DIMAGE_Y;

    disp_drv.direct_mode = 1;

    disp_drv.full_refresh = 1;

    /* Set the driver function */
    disp_drv.flush_cb = lv_disp_flush;

    /* Finally register the driver */
    lv_disp_drv_register(&disp_drv);

    /* Display hardware initialization */
    hw_disp_init();

#if(I2C_TOUCH_ENABLE == 1)
    /* Descriptor of a input device driver */
    static lv_indev_drv_t touch_drv;

    /* Basic initialization */
    lv_indev_drv_init(&touch_drv);

    /* Touch pad is a pointer-like device */
    touch_drv.type = LV_INDEV_TYPE_POINTER;

    /* Set the driver function */
    touch_drv.read_cb = lv_touch_get;

    /*Finally register the driver*/
    lv_indev_drv_register(&touch_drv);

    /* Touch screen hardware initialization */
    hw_touch_init();
#endif
}
