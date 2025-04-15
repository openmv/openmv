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
 * @file     GT911_Baremetal.c
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     08-August-2023
 * @brief    Baremetal demo application to verify GT911 touch screen.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include "stdio.h"
#include "string.h"

/* PINMUX Driver */
#include "pinconf.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/*touch screen driver */
#include "Driver_Touch_Screen.h"

/* Touch screen driver instance */
extern ARM_DRIVER_TOUCH_SCREEN GT911;
static ARM_DRIVER_TOUCH_SCREEN *Drv_Touchscreen = &GT911;

void touchscreen_demo();

#define GT911_TOUCH_INT_GPIO_PORT        PORT_9
#define GT911_TOUCH_INT_PIN_NO           PIN_4
#define GT911_TOUCH_I2C_SDA_PORT         PORT_7
#define GT911_TOUCH_I2C_SDA_PIN_NO       PIN_2
#define GT911_TOUCH_I2C_SCL_PORT         PORT_7
#define GT911_TOUCH_I2C_SCL_PIN_NO       PIN_3
#define ACTIVE_TOUCH_POINTS              5

/**
  \fn          int32_t hardware_cfg(void)
  \brief       i2c hardware pin initialization:
                   -  PIN-MUX configuration
                   -  PIN-PAD configuration
                 -  GPIO9 initialization:
                   -  PIN-MUX configuration
                   -  PIN-PAD configuration
                 -  UART hardware pin initialization (if printf redirection to UART is chosen):
                   -  PIN-MUX configuration for UART receiver
                   -  PIN-MUX configuration for UART transmitter
  \param[in]   none
  \return      ARM_DRIVER_OK: success; 0: failure
  */
int32_t hardware_cfg(void)
{
    int32_t ret = -1;

    /* gpio9 config for interrupt
     * Pad function: PADCTRL_READ_ENABLE |
     *               PADCTRL_DRIVER_DISABLED_PULL_UP |
     *               PADCTRL_SCHMITT_TRIGGER_ENABLE
     */
    ret = pinconf_set(GT911_TOUCH_INT_GPIO_PORT, GT911_TOUCH_INT_PIN_NO, PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE |\
                     PADCTRL_SCHMITT_TRIGGER_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: GPIO PINMUX failed.\r\n");
        return ret;
    }

    /* Configure GPIO Pin : P7_2 as i2c1_sda_c
     * Pad function: PADCTRL_READ_ENABLE |
     *               PADCTRL_DRIVER_DISABLED_PULL_UP
     */
    ret = pinconf_set(GT911_TOUCH_I2C_SDA_PORT, GT911_TOUCH_I2C_SDA_PIN_NO, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | \
                     PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I2C SDA PINMUX failed.\r\n");
        return ret;
    }

    /* Configure GPIO Pin : P7_3 as i2c1_scl_c
     * Pad function: PADCTRL_READ_ENABLE |
     *               PADCTRL_DRIVER_DISABLED_PULL_UP
     */
    ret = pinconf_set(GT911_TOUCH_I2C_SCL_PORT, GT911_TOUCH_I2C_SCL_PIN_NO, PINMUX_ALTERNATE_FUNCTION_5,PADCTRL_READ_ENABLE | \
                     PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I2C SCL PINMUX failed.\r\n");
        return ret;
    }

    return ARM_DRIVER_OK;
}

/**
 * @function    void touchscreen_demo()
\brief          Bare Metal TestApp to verify GT911 touch screen.
                This demo application does:
                    - initialize i2c AND gpio9 port hardware pins
                    - initialize UART hardware pins if print redirection to UART is chosen
                    - initialize USART driver if printf redirection to UART is chosen.
                    - initialize GT911 Touch screen driver with call back function.
                    - check if touch screen is pressed or not
                    - if pressed then print up to 5 coordinate positions where display touch screen was touched.
  \param[in]   none
  \return      none
  */
void touchscreen_demo()
{
    int32_t ret;
    int32_t count = 0;
    ARM_DRIVER_VERSION version;
    ARM_TOUCH_STATE state;

    /* Initialize i2c and GPIO9 hardware pins using PinMux Driver. */
    /* Initialize UART4 hardware pins using PinMux driver if printf redirection to UART is selected */
    ret = hardware_cfg();
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in hardware configuration */
        printf("\r\n Error: Hardware configuration failed.\r\n");
    }

    /* Touch screen version */
    version = Drv_Touchscreen->GetVersion();
    printf("\r\n Touchscreen driver version api:0x%X driver:0x%X \r\n",version.api, version.drv);

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

    while(1)
    {
        /* Reading GT911 touch screen press status */
        ret =Drv_Touchscreen->GetState(&state);
        if(ret != ARM_DRIVER_OK)
        {
            /* Error in GT911 touch screen read status */
            printf("\r\n Error: GT911 touch screen read  status failed.\r\n");
            goto error_GT911_poweroff;
        }

        if(state.numtouches){
            for(count = 1; count <= ACTIVE_TOUCH_POINTS; count++)
            {
                /* Print coordinates positions of pressing on GT911 touch screen up to max touch points set */
                printf("x%d: %d y%d: %d \r\n",count, state.coordinates[count - 1].x, count, state.coordinates[count - 1].y);
            }
            memset(state.coordinates,0,sizeof(state.coordinates));
        }
    }

error_GT911_poweroff:
    /* Received error Power off GT911 touch screen peripheral */
    ret = Drv_Touchscreen->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in GT911 Touch screen Power OFF. */
        printf("ERROR: Could not power OFF touch screen\n");
        return;
    }

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

/* Define main entry point.  */
int main()
{
    #if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif

    touchscreen_demo();
    return 0;
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
