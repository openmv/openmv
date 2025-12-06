/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     app_issi_flash.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     05-Dec-2022
 * @brief    Application to set up flash in XIP mode and jump to the flash
 *           image address.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "RTE_Components.h"
#include CMSIS_device_header
#include "setup_flash_xip.h"
#include "ospi_xip_user.h"
#include "pinconf.h"
#include "Driver_GPIO.h"
#if OSPI_XIP_ENABLE_AES_DECRYPTION
#include "se_services_port.h"
#include "services_lib_api.h"
#include "ospi_xip_aes_key.h"
#endif

#include "system_utils.h"

#define OSPI_RESET_PORT 15
#define OSPI_RESET_PIN  7

extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(OSPI_RESET_PORT);
ARM_DRIVER_GPIO *GPIODrv = &ARM_Driver_GPIO_(OSPI_RESET_PORT);

typedef void (*pfun) (void);

static pfun EntryPoint;

/**
  \fn        static int32_t setup_PinMUX(void)
  \brief     Set up PinMUX and PinPAD
  \param[in] none
  \return    -1: If any error
 */
static int32_t setup_pinmux(void)
{
    int32_t ret;

#if OSPI_XIP_INSTANCE == OSPI0
    ret = pinconf_set(OSPI0_D0_PORT, OSPI0_D0_PIN, OSPI0_D0_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_D1_PORT, OSPI0_D1_PIN, OSPI0_D1_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_D2_PORT, OSPI0_D2_PIN, OSPI0_D2_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_D3_PORT, OSPI0_D3_PIN, OSPI0_D3_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_D4_PORT, OSPI0_D4_PIN, OSPI0_D4_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_D5_PORT, OSPI0_D5_PIN, OSPI0_D5_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_D6_PORT, OSPI0_D6_PIN, OSPI0_D6_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_D7_PORT, OSPI0_D7_PIN, OSPI0_D7_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_RXDS_PORT, OSPI0_RXDS_PIN, OSPI0_RXDS_PIN_FUNCTION, PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_SCLK_PORT, OSPI0_SCLK_PIN, OSPI0_SCLK_PIN_FUNCTION, 0);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI0_CS_PORT, OSPI0_CS_PIN, OSPI0_CS_PIN_FUNCTION, 0);
    if (ret)
        return -1;
#else
    ret = pinconf_set(OSPI1_D0_PORT, OSPI1_D0_PIN, OSPI1_D0_PIN_FUNCTION,
                        PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_D1_PORT, OSPI1_D1_PIN, OSPI1_D1_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_D2_PORT, OSPI1_D2_PIN, OSPI1_D2_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |  PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_D3_PORT, OSPI1_D3_PIN, OSPI1_D3_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_D4_PORT, OSPI1_D4_PIN, OSPI1_D4_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_D5_PORT, OSPI1_D5_PIN, OSPI1_D5_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_D6_PORT, OSPI1_D6_PIN, OSPI1_D6_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_D7_PORT, OSPI1_D7_PIN, OSPI1_D7_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |  PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_RXDS_PORT, OSPI1_RXDS_PIN, OSPI1_RXDS_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_SCLK_PORT, OSPI1_SCLK_PIN, OSPI1_SCLK_PIN_FUNCTION,
                     PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_CS_PORT, OSPI1_CS_PIN, OSPI1_CS_PIN_FUNCTION, PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA);
    if (ret)
        return -1;

    ret = pinconf_set(OSPI1_SCLKN_PORT, OSPI1_SCLKN_PIN, OSPI1_SCLKN_PIN_FUNCTION,
                    PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA);
    if (ret)
        return -1;
#endif

    ret = GPIODrv->Initialize(OSPI_RESET_PIN, NULL);
    if (ret != ARM_DRIVER_OK)
        return -1;

    ret = GPIODrv->PowerControl(OSPI_RESET_PIN, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
        return -1;

    ret = GPIODrv->SetDirection(OSPI_RESET_PIN, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK)
        return -1;

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK)
        return -1;

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (ret != ARM_DRIVER_OK)
        return -1;

    return 0;
}

/**
  \fn        int main ()
  \brief     Main Function
  \param[in] none
  \return    0 : Success
 */
int main ()
{
#if OSPI_XIP_SKIP_INITIALIZATION == 0
    int32_t ret;

    ret = setup_pinmux();

    if (ret)
    {
        while(1);
    }

#if OSPI_XIP_ENABLE_AES_DECRYPTION
    uint32_t se_ret = SERVICES_REQ_SUCCESS, error_code, command;
    uint8_t key[AES_128_KEY_SIZE];

    /* Initialize the SE services */
    se_services_port_init();

    /* Prepare the user provided key for SE service request */
    for (int i = 0; i < AES_128_KEY_SIZE; i++)
    {
        key[i] = ospi_aes_user_key[AES_128_KEY_SIZE - 1 - i];
    }

#if OSPI_XIP_INSTANCE == OSPI0
    command = OSPI_WRITE_EXTERNAL_KEY_OSPI0;
#else
    command = OSPI_WRITE_EXTERNAL_KEY_OSPI1;
#endif

    se_ret = SERVICES_application_ospi_write_key(se_services_s_handle, command, key, &error_code);

    if (se_ret != SERVICES_REQ_SUCCESS || error_code != 0)
    {
        while(1);
    }
#endif

    ret = setup_flash_xip();

    if (ret)
    {
        while(1);
    }
#else
    /*
     * Wait for the other core to finish setting up OSPI XIP and AES.
     * This mechanism can be replaced by another 'wait and notify'
     * synchronization mechanism between the cores as per the application
     * requirements.
     */
    while (flash_xip_enabled() == false)
    {
        sys_busy_loop_us(1000);
    }
#endif

    /* Read the Reset_Handler address from the vector table of the image in OSPI */
    EntryPoint = (pfun)(*(__IO uint32_t *)(OSPI_XIP_IMAGE_ADDRESS + 4));

    /* Jump to the Reset_Handler */
    EntryPoint();

    return 0;
}

