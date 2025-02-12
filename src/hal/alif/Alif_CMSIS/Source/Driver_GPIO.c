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
 * @file     Driver_GPIO.c
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     29-March-2023
 * @brief    CMSIS Driver for GPIO.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "Driver_GPIO.h"
#include "Driver_GPIO_Private.h"
#include "gpio.h"
#include "sys_ctrl_gpio.h"

#if !(RTE_GPIO0 || RTE_GPIO1 || RTE_GPIO2 || RTE_GPIO3 || RTE_GPIO4  || RTE_GPIO5 || RTE_GPIO6 || RTE_GPIO7 || \
      RTE_GPIO8 || RTE_GPIO9 || RTE_GPIO10 || RTE_GPIO11 || RTE_GPIO12  || RTE_GPI13 || RTE_GPI14 || RTE_LPGPIO)
#error "GPIO is not enabled in the RTE_Device.h"
#endif

#if !defined(RTE_Drivers_GPIO)
#error "GPIO is not enabled in the RTE_Components.h"
#endif

/**
  \fn      int32_t GPIO_Initialize (GPIO_RESOURCES *GPIO, ARM_GPIO_SignalEvent_t cb_event, uint8_t pin_no)
  \brief   Initialize the gpio pin
  \param   cb_event : Pointer to gpio Event \ref GPIO_SignalEvent
  \param   GPIO     : Pointer to gpio device resources
  \param   pin_no   : pin to be configured.
  \return  execution_status
*/
static int32_t GPIO_Initialize (GPIO_RESOURCES *GPIO, ARM_GPIO_SignalEvent_t cb_event, uint8_t pin_no)
{
    if (pin_no >= GPIO_PORT_MAX_PIN_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* mask the interrupt */
    gpio_mask_interrupt (GPIO->reg_base, pin_no);

    GPIO->cb_event[pin_no] = cb_event;

    GPIO->state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t GPIO_PowerControl(GPIO_RESOURCES *GPIO, uint8_t pin_no, ARM_POWER_STATE state).
 * @brief   Handles the gpio power.
 * @note    none.
 * @param   GPIO   : Pointer to gpio resources structure.
 * @param   pin_no : pin to be configured.
 * @param   state  : power state.
 * @retval  \ref execution_status
 */
static int32_t GPIO_PowerControl (GPIO_RESOURCES *GPIO, uint8_t pin_no, ARM_POWER_STATE state)
{
    if (pin_no >= GPIO_PORT_MAX_PIN_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            if (GPIO->state.powered == 0)
            {
                return ARM_DRIVER_OK;
            }

            GPIO->state.powered = 0;
            break;
        }
        case ARM_POWER_FULL:
        {
            if (GPIO->state.initialized == 0)
            {
                return ARM_DRIVER_ERROR;
            }

            GPIO->state.powered = 1;
            break;
        }
        case ARM_POWER_LOW:
        default:
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }
    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t GPIO_SetDirection (GPIO_RESOURCES *GPIO, uint8_t pin_no, GPIO_PIN_DIRECTION dir)
 * @brief   Set the direction for gpio pin.
 * @note    none.
 * @param   GPIO   : Pointer to gpio resources structure.
 * @param   pin_no : pin to be configured.
 * @param   dir    : direction to be set.
 * @retval  \ref execution_status
 */
static int32_t GPIO_SetDirection (GPIO_RESOURCES *GPIO, uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    if (GPIO->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }
    if (pin_no >= GPIO_PORT_MAX_PIN_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    switch (dir)
    {
        case GPIO_PIN_DIRECTION_INPUT:
        {
            /**< Configuring Pin's to Input Direction >*/
            gpio_set_direction_input(GPIO->reg_base, pin_no);
            break;
        }
        case GPIO_PIN_DIRECTION_OUTPUT:
        {
            /**< Configuring Pin's to Output Direction >*/
            gpio_set_direction_output(GPIO->reg_base, pin_no);
            break;
        }
        default:
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
    }
    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t GPIO_GetDirection (GPIO_RESOURCES *GPIO, uint8_t pin_no, uint32_t *dir)
 * @brief   Read gpio pin direction.
 * @note    none.
 * @param   GPIO   : Pointer to gpio resources structure.
 * @param   pin_no : pin to be configured.
 * @param   dir    : Pointer to the variable where direction should be stored.
 * @retval  \ref execution_status
 */
static int32_t GPIO_GetDirection (GPIO_RESOURCES *GPIO, uint8_t pin_no, uint32_t *dir)
{
    if (GPIO->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }
    if (pin_no >= GPIO_PORT_MAX_PIN_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (dir == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (gpio_get_direction (GPIO->reg_base, pin_no))
    {
        *dir = GPIO_PIN_DIRECTION_OUTPUT;
    }
    else
    {
        *dir = GPIO_PIN_DIRECTION_INPUT;
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t GPIO_SetValue (GPIO_RESOURCES *GPIO, uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
 * @brief   Set value for gpio pin.
 * @note    none.
 * @param   GPIO   : Pointer to gpio resources structure.
 * @param   pin_no : pin to be configured.
 * @param   value  : value to be set.
 * @retval  \ref execution_status
 */
static int32_t GPIO_SetValue (GPIO_RESOURCES *GPIO, uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    if (GPIO->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }
    if (pin_no >= GPIO_PORT_MAX_PIN_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    switch (value)
    {
        case GPIO_PIN_OUTPUT_STATE_LOW:
        {
            /**< Set LOW(0) to the pin >*/
            gpio_set_value_low (GPIO->reg_base, pin_no);
            break;
        }
        case GPIO_PIN_OUTPUT_STATE_HIGH:
        {
            /**< Set HIGH(1) to the pin >*/
            gpio_set_value_high (GPIO->reg_base, pin_no);
            break;
        }
        case GPIO_PIN_OUTPUT_STATE_TOGGLE:
        {
            /**< Toggle pin value >*/
            gpio_toggle_value (GPIO->reg_base, pin_no);
            break;
        }
        default:
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
     }
     return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t GPIO_GetValue (GPIO_RESOURCES *GPIO, uint8_t pin_no, uint32_t *value)
 * @brief   Read gpio pin data value.
 * @note    none.
 * @param   GPIO  : Pointer to gpio resources structure.
 * @param   pin_no: pin to be configured.
 * @param   value : Pointer to the variable in which gpio data should be stored.
 * @retval  \ref execution_status
 */
static int32_t GPIO_GetValue (GPIO_RESOURCES *GPIO, uint8_t pin_no, uint32_t *value)
{
    if (GPIO->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }
    if (pin_no >= GPIO_PORT_MAX_PIN_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (gpio_get_value (GPIO->reg_base, pin_no))
    {
        *value = GPIO_PIN_STATE_HIGH;
    }
    else
    {
        *value = GPIO_PIN_STATE_LOW;
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t GPIO_Control (GPIO_RESOURCES *GPIO, uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
 * @brief   Used to configure gpio.
 * @note    none.
 * @param   GPIO         : Pointer to gpio resources structure.
 * @param   pin_no       : pin to be configured.
 * @param   control_code : control code to configure.
 * @param   arg          : Pointer to an argument.
 * @retval  \ref execution_status
 */
static int32_t GPIO_Control (GPIO_RESOURCES *GPIO, uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    if (GPIO->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }
    if (pin_no >= GPIO_PORT_MAX_PIN_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    switch (control_code)
    {
        case ARM_GPIO_CONFIG_DEBOUNCE :
        {
            if (!arg)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            if (GPIO->gpio_id == LPGPIO_INSTANCE)
            {
                return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            if(*arg)
            {
                /* Enable Debounce clock from EXPMST0 */
                enable_gpio_debounce_clk (GPIO->gpio_id);

                /* Set Debounce clock divider from EXPMST0 */
                set_gpio_debounce_clkdiv (GPIO->db_clkdiv, GPIO->gpio_id);

                /* Enable De-bounce operation */
                gpio_enable_debounce (GPIO->reg_base, pin_no);
            }
            else
            {
                /* Disable Debounce clock from EXPMST0 */
                disable_gpio_debounce_clk (GPIO->gpio_id);

                /* Disable De-bounce operation */
                gpio_disable_debounce (GPIO->reg_base, pin_no);
            }

            break;
        }
        case ARM_GPIO_ENABLE_INTERRUPT :
        {
            if (!arg)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            /**< Enable interrupt >*/
            gpio_enable_interrupt (GPIO->reg_base, pin_no);

            /**< Unmask interrupt >*/
            gpio_unmask_interrupt (GPIO->reg_base, pin_no);

            if (ARM_GPIO_BIT_IRQ_BOTH_EDGE(*arg))
            {
                /**< Configure interrupt for both edge >*/
                gpio_interrupt_set_both_edge_trigger (GPIO->reg_base, pin_no);
            }
            else
            {
                if (ARM_GPIO_BIT_IRQ_SENSITIVE(*arg))
                {
                    /**< Configure interrupt as edge sensitive >*/
                    gpio_interrupt_set_edge_trigger (GPIO->reg_base, pin_no);
                }
                else
                {
                    /**< Configure interrupt as level sensitive >*/
                    gpio_interrupt_set_level_trigger (GPIO->reg_base, pin_no);
                }

                if (ARM_GPIO_BIT_IRQ_POLARITY(*arg))
                {
                    /**< Configure interrupt for high edge >*/
                    gpio_interrupt_set_polarity_high (GPIO->reg_base, pin_no);
                }
                else
                {
                    /**< Configure interrupt for low edge >*/
                    gpio_interrupt_set_polarity_low (GPIO->reg_base, pin_no);
                }
            }

            gpio_interrupt_eoi(GPIO->reg_base, pin_no);

            NVIC_ClearPendingIRQ (GPIO->IRQ_base_num + pin_no);
            NVIC_SetPriority ((GPIO->IRQ_base_num + pin_no), GPIO->IRQ_priority[pin_no]);
            NVIC_EnableIRQ (GPIO->IRQ_base_num + pin_no);

            break;
        }
        case ARM_GPIO_DISABLE_INTERRUPT :
        {
            /* setting to default values */
            gpio_disable_interrupt (GPIO->reg_base, pin_no);
            gpio_mask_interrupt (GPIO->reg_base, pin_no);
            gpio_interrupt_set_polarity_low (GPIO->reg_base, pin_no);
            gpio_interrupt_set_level_trigger (GPIO->reg_base, pin_no);

            NVIC_ClearPendingIRQ (GPIO->IRQ_base_num + pin_no);
            NVIC_DisableIRQ (GPIO->IRQ_base_num + pin_no);
            break;
        }
        case ARM_GPIO_GET_CONFIG_VALUE1 :
        {
            if (!arg)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            *arg = gpio_read_config1 (GPIO->reg_base);
            break;
        }
        case ARM_GPIO_GET_CONFIG_VALUE2 :
        {
            if (!arg)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            *arg = gpio_read_config2 (GPIO->reg_base);
            break;
        }
        case ARM_GPIO_CONFIG_FLEXIO :
        {
            if (!((GPIO->gpio_id == GPIO7_INSTANCE) || (GPIO->gpio_id == LPGPIO_INSTANCE)))
            {
                return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            if (GPIO->gpio_id == LPGPIO_INSTANCE)
            {
                if (!(lpgpio_is_flexio (pin_no)))
                {
                    return ARM_DRIVER_ERROR_UNSUPPORTED;
                }
            }
            else
            {
                if (!(gpio_is_flexio (pin_no)))
                {
                    return ARM_DRIVER_ERROR_UNSUPPORTED;
                }
            }

            if (!arg)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            if (*arg)
            {
                set_flexio_gpio_voltage_1v8();
            }
            else
            {
                set_flexio_gpio_voltage_3v3();
            }
            break;
        }
        case ARM_GPIO_CONFIG_MODE :
        {
            if (GPIO->gpio_id != LPGPIO_INSTANCE)
            {
                return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            if (*arg)
            {
                gpio_set_hardware_mode(GPIO->reg_base, pin_no);
            }
            else
            {
                gpio_set_software_mode(GPIO->reg_base, pin_no);
            }
            break;
        }
        default:
        {
            return ARM_DRIVER_ERROR;
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t GPIO_Uninitialize (GPIO_RESOURCES *GPIO, uint8_t pin_no)
 * @brief   Un-Initialize the gpio.
 * @note    none.
 * @param   GPIO   : Pointer to gpio resources structure.
 * @param   pin_no : pin to be configured.
 * @retval  \ref execution_status
 */
static int32_t GPIO_Uninitialize (GPIO_RESOURCES *GPIO, uint8_t pin_no)
{
    if (GPIO->state.initialized == 0)
    {
        return ARM_DRIVER_OK;
    }
    if (pin_no >= GPIO_PORT_MAX_PIN_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    GPIO->cb_event[pin_no] = NULL;

    GPIO->state.initialized = 0;

    return ARM_DRIVER_OK;
}

/**
  \fn      void GPIO_IRQ_Handler (GPIO_RESOURCES *GPIO, uint8_t pin_no)
  \brief   Initialize the gpio pin
  \param   cb_event : Pointer to gpio Event \ref GPIO_SignalEvent
  \param   GPIO     : Pointer to gpio device resources
  @param   pin_no   : pin to be configured.
  \return  execution_status
*/
static void GPIO_IRQ_Handler (GPIO_RESOURCES *GPIO, uint8_t pin_no)
{
    /* clear pin interrupt */
    gpio_interrupt_eoi(GPIO->reg_base, pin_no);

    if (GPIO->cb_event[pin_no] != NULL)
    {
        GPIO->cb_event[pin_no](ARM_GPIO_IRQ_EVENT_EXTERNAL);
    }
}

/**<GPIO Instance 0>*/
#if RTE_GPIO0
static GPIO_RESOURCES GPIO0_RES = {
    .reg_base = (GPIO_Type*) GPIO0_BASE,
    .IRQ_base_num = GPIO0_IRQ0_IRQn,
    .gpio_id = GPIO0_INSTANCE,
    .db_clkdiv = RTE_GPIO0_DB_CLK_DIV,
    .IRQ_priority = {
        RTE_GPIO0_PIN0_IRQ_PRIORITY,
        RTE_GPIO0_PIN1_IRQ_PRIORITY,
        RTE_GPIO0_PIN2_IRQ_PRIORITY,
        RTE_GPIO0_PIN3_IRQ_PRIORITY,
        RTE_GPIO0_PIN4_IRQ_PRIORITY,
        RTE_GPIO0_PIN5_IRQ_PRIORITY,
        RTE_GPIO0_PIN6_IRQ_PRIORITY,
        RTE_GPIO0_PIN7_IRQ_PRIORITY
    }
};

void GPIO0_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO0_RES, 0);    }

void GPIO0_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO0_RES, 1);    }

void GPIO0_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO0_RES, 2);    }

void GPIO0_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO0_RES, 3);    }

void GPIO0_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO0_RES, 4);    }

void GPIO0_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO0_RES, 5);    }

void GPIO0_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO0_RES, 6);    }

void GPIO0_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO0_RES, 7);    }


static int32_t ARM_GPIO0_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO0_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO0_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO0_RES, pin_no, state);
}

static int32_t ARM_GPIO0_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO0_RES, pin_no, dir);
}

static int32_t ARM_GPIO0_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO0_RES, pin_no, dir);
}

static int32_t ARM_GPIO0_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO0_RES, pin_no, value);
}

static int32_t ARM_GPIO0_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO0_RES, pin_no, value);
}

static int32_t ARM_GPIO0_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO0_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO0_Uninitialize (uint8_t pin_no) {

    return GPIO_Uninitialize (&GPIO0_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO0;
ARM_DRIVER_GPIO Driver_GPIO0 = {
    ARM_GPIO0_Initialize,
    ARM_GPIO0_PowerControl,
    ARM_GPIO0_SetDirection,
    ARM_GPIO0_GetDirection,
    ARM_GPIO0_SetValue,
    ARM_GPIO0_GetValue,
    ARM_GPIO0_Control,
    ARM_GPIO0_Uninitialize
};
#endif   /* RTE_GPIO0 */


/**<GPIO Instance 1>*/
#if RTE_GPIO1
static GPIO_RESOURCES GPIO1_RES = {
    .reg_base = (GPIO_Type*) GPIO1_BASE,
    .IRQ_base_num = GPIO1_IRQ0_IRQn,
    .gpio_id = GPIO1_INSTANCE,
    .db_clkdiv = RTE_GPIO1_DB_CLK_DIV,
    .IRQ_priority = {
        RTE_GPIO1_PIN0_IRQ_PRIORITY,
        RTE_GPIO1_PIN1_IRQ_PRIORITY,
        RTE_GPIO1_PIN2_IRQ_PRIORITY,
        RTE_GPIO1_PIN3_IRQ_PRIORITY,
        RTE_GPIO1_PIN4_IRQ_PRIORITY,
        RTE_GPIO1_PIN5_IRQ_PRIORITY,
        RTE_GPIO1_PIN6_IRQ_PRIORITY,
        RTE_GPIO1_PIN7_IRQ_PRIORITY
    }
};

void GPIO1_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO1_RES, 0);    }

void GPIO1_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO1_RES, 1);    }

void GPIO1_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO1_RES, 2);    }

void GPIO1_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO1_RES, 3);    }

void GPIO1_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO1_RES, 4);    }

void GPIO1_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO1_RES, 5);    }

void GPIO1_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO1_RES, 6);    }

void GPIO1_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO1_RES, 7);    }

static int32_t ARM_GPIO1_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO1_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO1_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO1_RES, pin_no, state);
}

static int32_t ARM_GPIO1_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO1_RES, pin_no, dir);
}

static int32_t ARM_GPIO1_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO1_RES, pin_no, dir);
}

static int32_t ARM_GPIO1_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO1_RES, pin_no, value);
}

static int32_t ARM_GPIO1_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO1_RES, pin_no, value);
}

static int32_t ARM_GPIO1_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO1_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO1_Uninitialize (uint8_t pin_no) {

    return GPIO_Uninitialize (&GPIO1_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO1;
ARM_DRIVER_GPIO Driver_GPIO1 = {
    ARM_GPIO1_Initialize,
    ARM_GPIO1_PowerControl,
    ARM_GPIO1_SetDirection,
    ARM_GPIO1_GetDirection,
    ARM_GPIO1_SetValue,
    ARM_GPIO1_GetValue,
    ARM_GPIO1_Control,
    ARM_GPIO1_Uninitialize
};
#endif   /* RTE_GPIO1 */


/**<GPIO Instance 2>*/
#if RTE_GPIO2
static GPIO_RESOURCES GPIO2_RES = {
    .reg_base = (GPIO_Type*) GPIO2_BASE,
    .IRQ_base_num = GPIO2_IRQ0_IRQn,
    .gpio_id = GPIO2_INSTANCE,
    .db_clkdiv = RTE_GPIO2_DB_CLK_DIV,
    .IRQ_priority = {
        RTE_GPIO2_PIN0_IRQ_PRIORITY,
        RTE_GPIO2_PIN1_IRQ_PRIORITY,
        RTE_GPIO2_PIN2_IRQ_PRIORITY,
        RTE_GPIO2_PIN3_IRQ_PRIORITY,
        RTE_GPIO2_PIN4_IRQ_PRIORITY,
        RTE_GPIO2_PIN5_IRQ_PRIORITY,
        RTE_GPIO2_PIN6_IRQ_PRIORITY,
        RTE_GPIO2_PIN7_IRQ_PRIORITY
    }
};

void GPIO2_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO2_RES, 0);    }

void GPIO2_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO2_RES, 1);    }

void GPIO2_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO2_RES, 2);    }

void GPIO2_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO2_RES, 3);    }

void GPIO2_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO2_RES, 4);    }

void GPIO2_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO2_RES, 5);    }

void GPIO2_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO2_RES, 6);    }

void GPIO2_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO2_RES, 7);    }

static int32_t ARM_GPIO2_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO2_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO2_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO2_RES, pin_no, state);
}

static int32_t ARM_GPIO2_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO2_RES, pin_no, dir);
}

static int32_t ARM_GPIO2_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO2_RES, pin_no, dir);
}

static int32_t ARM_GPIO2_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO2_RES, pin_no, value);
}

static int32_t ARM_GPIO2_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO2_RES, pin_no, value);
}

static int32_t ARM_GPIO2_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO2_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO2_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO2_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO2;
ARM_DRIVER_GPIO Driver_GPIO2 = {
    ARM_GPIO2_Initialize,
    ARM_GPIO2_PowerControl,
    ARM_GPIO2_SetDirection,
    ARM_GPIO2_GetDirection,
    ARM_GPIO2_SetValue,
    ARM_GPIO2_GetValue,
    ARM_GPIO2_Control,
    ARM_GPIO2_Uninitialize
};
#endif   /* RTE_GPIO2 */


/**<GPIO Instance 3>*/
#if RTE_GPIO3
static GPIO_RESOURCES GPIO3_RES = {
    .reg_base = (GPIO_Type*) GPIO3_BASE,
    .IRQ_base_num = GPIO3_IRQ0_IRQn,
    .gpio_id = GPIO3_INSTANCE,
    .db_clkdiv = RTE_GPIO3_DB_CLK_DIV,
    .IRQ_priority = {
        RTE_GPIO3_PIN0_IRQ_PRIORITY,
        RTE_GPIO3_PIN1_IRQ_PRIORITY,
        RTE_GPIO3_PIN2_IRQ_PRIORITY,
        RTE_GPIO3_PIN3_IRQ_PRIORITY,
        RTE_GPIO3_PIN4_IRQ_PRIORITY,
        RTE_GPIO3_PIN5_IRQ_PRIORITY,
        RTE_GPIO3_PIN6_IRQ_PRIORITY,
        RTE_GPIO3_PIN7_IRQ_PRIORITY
    }
};

void GPIO3_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO3_RES, 0);    }

void GPIO3_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO3_RES, 1);    }

void GPIO3_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO3_RES, 2);    }

void GPIO3_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO3_RES, 3);    }

void GPIO3_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO3_RES, 4);    }

void GPIO3_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO3_RES, 5);    }

void GPIO3_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO3_RES, 6);    }

void GPIO3_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO3_RES, 7);    }

static int32_t ARM_GPIO3_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO3_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO3_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO3_RES, pin_no, state);
}

static int32_t ARM_GPIO3_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO3_RES, pin_no, dir);
}

static int32_t ARM_GPIO3_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO3_RES, pin_no, dir);
}

static int32_t ARM_GPIO3_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO3_RES, pin_no, value);
}

static int32_t ARM_GPIO3_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO3_RES, pin_no, value);
}

static int32_t ARM_GPIO3_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO3_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO3_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO3_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO3;
ARM_DRIVER_GPIO Driver_GPIO3 = {
    ARM_GPIO3_Initialize,
    ARM_GPIO3_PowerControl,
    ARM_GPIO3_SetDirection,
    ARM_GPIO3_GetDirection,
    ARM_GPIO3_SetValue,
    ARM_GPIO3_GetValue,
    ARM_GPIO3_Control,
    ARM_GPIO3_Uninitialize
};
#endif   /* RTE_GPIO3 */


/**<GPIO Instance 4>*/
#if RTE_GPIO4
static GPIO_RESOURCES GPIO4_RES = {
    .reg_base = (GPIO_Type*) GPIO4_BASE,
    .IRQ_base_num = GPIO4_IRQ0_IRQn,
    .gpio_id = GPIO4_INSTANCE,
    .db_clkdiv = RTE_GPIO4_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO4_PIN0_IRQ_PRIORITY,
            RTE_GPIO4_PIN1_IRQ_PRIORITY,
            RTE_GPIO4_PIN2_IRQ_PRIORITY,
            RTE_GPIO4_PIN3_IRQ_PRIORITY,
            RTE_GPIO4_PIN4_IRQ_PRIORITY,
            RTE_GPIO4_PIN5_IRQ_PRIORITY,
            RTE_GPIO4_PIN6_IRQ_PRIORITY,
            RTE_GPIO4_PIN7_IRQ_PRIORITY,
    }
};

void GPIO4_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO4_RES, 0);    }

void GPIO4_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO4_RES, 1);    }

void GPIO4_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO4_RES, 2);    }

void GPIO4_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO4_RES, 3);    }

void GPIO4_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO4_RES, 4);    }

void GPIO4_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO4_RES, 5);    }

void GPIO4_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO4_RES, 6);    }

void GPIO4_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO4_RES, 7);    }

static int32_t ARM_GPIO4_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO4_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO4_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO4_RES, pin_no, state);
}

static int32_t ARM_GPIO4_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO4_RES, pin_no, dir);
}

static int32_t ARM_GPIO4_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    int32_t ret =  GPIO_GetDirection (&GPIO4_RES, pin_no, dir);
    return ret;
}

static int32_t ARM_GPIO4_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO4_RES, pin_no, value);
}

static int32_t ARM_GPIO4_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO4_RES, pin_no, value);
}

static int32_t ARM_GPIO4_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO4_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO4_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO4_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO4;
ARM_DRIVER_GPIO Driver_GPIO4 = {
    ARM_GPIO4_Initialize,
    ARM_GPIO4_PowerControl,
    ARM_GPIO4_SetDirection,
    ARM_GPIO4_GetDirection,
    ARM_GPIO4_SetValue,
    ARM_GPIO4_GetValue,
    ARM_GPIO4_Control,
    ARM_GPIO4_Uninitialize
};
#endif   /* RTE_GPIO4 */


/**<GPIO Instance 5>*/
#if RTE_GPIO5
static GPIO_RESOURCES GPIO5_RES = {
    .reg_base = (GPIO_Type*) GPIO5_BASE,
    .IRQ_base_num = GPIO5_IRQ0_IRQn,
    .gpio_id = GPIO5_INSTANCE,
    .db_clkdiv = RTE_GPIO5_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO5_PIN0_IRQ_PRIORITY,
            RTE_GPIO5_PIN1_IRQ_PRIORITY,
            RTE_GPIO5_PIN2_IRQ_PRIORITY,
            RTE_GPIO5_PIN3_IRQ_PRIORITY,
            RTE_GPIO5_PIN4_IRQ_PRIORITY,
            RTE_GPIO5_PIN5_IRQ_PRIORITY,
            RTE_GPIO5_PIN6_IRQ_PRIORITY,
            RTE_GPIO5_PIN7_IRQ_PRIORITY,
    }
};

void GPIO5_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO5_RES, 0);    }

void GPIO5_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO5_RES, 1);    }

void GPIO5_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO5_RES, 2);    }

void GPIO5_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO5_RES, 3);    }

void GPIO5_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO5_RES, 4);    }

void GPIO5_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO5_RES, 5);    }

void GPIO5_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO5_RES, 6);    }

void GPIO5_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO5_RES, 7);    }

static int32_t ARM_GPIO5_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO5_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO5_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO5_RES, pin_no, state);
}

static int32_t ARM_GPIO5_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO5_RES, pin_no, dir);
}

static int32_t ARM_GPIO5_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    int32_t ret =  GPIO_GetDirection (&GPIO5_RES, pin_no, dir);
    return ret;
}

static int32_t ARM_GPIO5_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO5_RES, pin_no, value);
}

static int32_t ARM_GPIO5_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO5_RES, pin_no, value);
}

static int32_t ARM_GPIO5_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO5_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO5_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO5_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO5;
ARM_DRIVER_GPIO Driver_GPIO5 = {
    ARM_GPIO5_Initialize,
    ARM_GPIO5_PowerControl,
    ARM_GPIO5_SetDirection,
    ARM_GPIO5_GetDirection,
    ARM_GPIO5_SetValue,
    ARM_GPIO5_GetValue,
    ARM_GPIO5_Control,
    ARM_GPIO5_Uninitialize
};
#endif   /* RTE_GPIO5 */


/**<GPIO Instance 6>*/
#if RTE_GPIO6
static GPIO_RESOURCES GPIO6_RES = {
    .reg_base = (GPIO_Type*) GPIO6_BASE,
    .IRQ_base_num = GPIO6_IRQ0_IRQn,
    .gpio_id = GPIO6_INSTANCE,
    .db_clkdiv = RTE_GPIO6_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO6_PIN0_IRQ_PRIORITY,
            RTE_GPIO6_PIN1_IRQ_PRIORITY,
            RTE_GPIO6_PIN2_IRQ_PRIORITY,
            RTE_GPIO6_PIN3_IRQ_PRIORITY,
            RTE_GPIO6_PIN4_IRQ_PRIORITY,
            RTE_GPIO6_PIN5_IRQ_PRIORITY,
            RTE_GPIO6_PIN6_IRQ_PRIORITY,
            RTE_GPIO6_PIN7_IRQ_PRIORITY,
    }
};

void GPIO6_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO6_RES, 0);    }

void GPIO6_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO6_RES, 1);    }

void GPIO6_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO6_RES, 2);    }

void GPIO6_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO6_RES, 3);    }

void GPIO6_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO6_RES, 4);    }

void GPIO6_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO6_RES, 5);    }

void GPIO6_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO6_RES, 6);    }

void GPIO6_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO6_RES, 7);    }

static int32_t ARM_GPIO6_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO6_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO6_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO6_RES, pin_no, state);
}

static int32_t ARM_GPIO6_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO6_RES, pin_no, dir);
}

static int32_t ARM_GPIO6_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO6_RES, pin_no, dir);

}

static int32_t ARM_GPIO6_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO6_RES, pin_no, value);
}

static int32_t ARM_GPIO6_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO6_RES, pin_no, value);
}

static int32_t ARM_GPIO6_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO6_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO6_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO6_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO6;
ARM_DRIVER_GPIO Driver_GPIO6 = {
    ARM_GPIO6_Initialize,
    ARM_GPIO6_PowerControl,
    ARM_GPIO6_SetDirection,
    ARM_GPIO6_GetDirection,
    ARM_GPIO6_SetValue,
    ARM_GPIO6_GetValue,
    ARM_GPIO6_Control,
    ARM_GPIO6_Uninitialize
};
#endif   /* RTE_GPIO6 */


/**<GPIO Instance 7>*/
#if RTE_GPIO7
static GPIO_RESOURCES GPIO7_RES = {
    .reg_base = (GPIO_Type*) GPIO7_BASE,
    .IRQ_base_num = GPIO7_IRQ0_IRQn,
    .gpio_id = GPIO7_INSTANCE,
    .db_clkdiv = RTE_GPIO7_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO7_PIN0_IRQ_PRIORITY,
            RTE_GPIO7_PIN1_IRQ_PRIORITY,
            RTE_GPIO7_PIN2_IRQ_PRIORITY,
            RTE_GPIO7_PIN3_IRQ_PRIORITY,
            RTE_GPIO7_PIN4_IRQ_PRIORITY,
            RTE_GPIO7_PIN5_IRQ_PRIORITY,
            RTE_GPIO7_PIN6_IRQ_PRIORITY,
            RTE_GPIO7_PIN7_IRQ_PRIORITY,
    }
};

void GPIO7_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO7_RES, 0);    }

void GPIO7_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO7_RES, 1);    }

void GPIO7_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO7_RES, 2);    }

void GPIO7_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO7_RES, 3);    }

void GPIO7_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO7_RES, 4);    }

void GPIO7_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO7_RES, 5);    }

void GPIO7_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO7_RES, 6);    }

void GPIO7_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO7_RES, 7);    }

static int32_t ARM_GPIO7_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO7_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO7_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO7_RES, pin_no, state);
}

static int32_t ARM_GPIO7_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO7_RES, pin_no, dir);
}

static int32_t ARM_GPIO7_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO7_RES, pin_no, dir);
}

static int32_t ARM_GPIO7_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO7_RES, pin_no, value);
}

static int32_t ARM_GPIO7_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO7_RES, pin_no, value);
}

static int32_t ARM_GPIO7_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO7_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO7_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO7_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO7;
ARM_DRIVER_GPIO Driver_GPIO7 = {
    ARM_GPIO7_Initialize,
    ARM_GPIO7_PowerControl,
    ARM_GPIO7_SetDirection,
    ARM_GPIO7_GetDirection,
    ARM_GPIO7_SetValue,
    ARM_GPIO7_GetValue,
    ARM_GPIO7_Control,
    ARM_GPIO7_Uninitialize
};
#endif   /* RTE_GPIO7 */


/**<GPIO Instance 8>*/
#if RTE_GPIO8
static GPIO_RESOURCES GPIO8_RES = {
    .reg_base = (GPIO_Type*) GPIO8_BASE,
    .IRQ_base_num = GPIO8_IRQ0_IRQn,
    .gpio_id = GPIO8_INSTANCE,
    .db_clkdiv = RTE_GPIO8_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO8_PIN0_IRQ_PRIORITY,
            RTE_GPIO8_PIN1_IRQ_PRIORITY,
            RTE_GPIO8_PIN2_IRQ_PRIORITY,
            RTE_GPIO8_PIN3_IRQ_PRIORITY,
            RTE_GPIO8_PIN4_IRQ_PRIORITY,
            RTE_GPIO8_PIN5_IRQ_PRIORITY,
            RTE_GPIO8_PIN6_IRQ_PRIORITY,
            RTE_GPIO8_PIN7_IRQ_PRIORITY,
    }
};

void GPIO8_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO8_RES, 0);    }

void GPIO8_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO8_RES, 1);    }

void GPIO8_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO8_RES, 2);    }

void GPIO8_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO8_RES, 3);    }

void GPIO8_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO8_RES, 4);    }

void GPIO8_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO8_RES, 5);    }

void GPIO8_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO8_RES, 6);    }

void GPIO8_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO8_RES, 7);    }

static int32_t ARM_GPIO8_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO8_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO8_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO8_RES, pin_no, state);
}

static int32_t ARM_GPIO8_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO8_RES, pin_no, dir);
}

static int32_t ARM_GPIO8_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO8_RES, pin_no, dir);
}

static int32_t ARM_GPIO8_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO8_RES, pin_no, value);
}

static int32_t ARM_GPIO8_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO8_RES, pin_no, value);
}

static int32_t ARM_GPIO8_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO8_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO8_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO8_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO8;
ARM_DRIVER_GPIO Driver_GPIO8 = {
    ARM_GPIO8_Initialize,
    ARM_GPIO8_PowerControl,
    ARM_GPIO8_SetDirection,
    ARM_GPIO8_GetDirection,
    ARM_GPIO8_SetValue,
    ARM_GPIO8_GetValue,
    ARM_GPIO8_Control,
    ARM_GPIO8_Uninitialize
};
#endif   /* RTE_GPIO8 */


/**<GPIO Instance 9>*/
#if RTE_GPIO9
static GPIO_RESOURCES GPIO9_RES = {
    .reg_base = (GPIO_Type*) GPIO9_BASE,
    .IRQ_base_num = GPIO9_IRQ0_IRQn,
    .gpio_id = GPIO9_INSTANCE,
    .db_clkdiv = RTE_GPIO9_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO9_PIN0_IRQ_PRIORITY,
            RTE_GPIO9_PIN1_IRQ_PRIORITY,
            RTE_GPIO9_PIN2_IRQ_PRIORITY,
            RTE_GPIO9_PIN3_IRQ_PRIORITY,
            RTE_GPIO9_PIN4_IRQ_PRIORITY,
            RTE_GPIO9_PIN5_IRQ_PRIORITY,
            RTE_GPIO9_PIN6_IRQ_PRIORITY,
            RTE_GPIO9_PIN7_IRQ_PRIORITY,
    }
};

void GPIO9_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO9_RES, 0);    }

void GPIO9_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO9_RES, 1);    }

void GPIO9_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO9_RES, 2);    }

void GPIO9_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO9_RES, 3);    }

void GPIO9_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO9_RES, 4);    }

void GPIO9_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO9_RES, 5);    }

void GPIO9_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO9_RES, 6);    }

void GPIO9_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO9_RES, 7);    }

static int32_t ARM_GPIO9_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO9_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO9_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO9_RES, pin_no, state);
}

static int32_t ARM_GPIO9_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO9_RES, pin_no, dir);
}

static int32_t ARM_GPIO9_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO9_RES, pin_no, dir);
}

static int32_t ARM_GPIO9_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO9_RES, pin_no, value);
}

static int32_t ARM_GPIO9_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO9_RES, pin_no, value);
}

static int32_t ARM_GPIO9_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO9_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO9_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO9_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO9;
ARM_DRIVER_GPIO Driver_GPIO9 = {
    ARM_GPIO9_Initialize,
    ARM_GPIO9_PowerControl,
    ARM_GPIO9_SetDirection,
    ARM_GPIO9_GetDirection,
    ARM_GPIO9_SetValue,
    ARM_GPIO9_GetValue,
    ARM_GPIO9_Control,
    ARM_GPIO9_Uninitialize
};
#endif   /* RTE_GPIO9 */


/**<GPIO Instance 10>*/
#if RTE_GPIO10
static GPIO_RESOURCES GPIO10_RES = {
    .reg_base = (GPIO_Type*) GPIO10_BASE,
    .IRQ_base_num = GPIO10_IRQ0_IRQn,
    .gpio_id = GPIO10_INSTANCE,
    .db_clkdiv = RTE_GPIO10_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO10_PIN0_IRQ_PRIORITY,
            RTE_GPIO10_PIN1_IRQ_PRIORITY,
            RTE_GPIO10_PIN2_IRQ_PRIORITY,
            RTE_GPIO10_PIN3_IRQ_PRIORITY,
            RTE_GPIO10_PIN4_IRQ_PRIORITY,
            RTE_GPIO10_PIN5_IRQ_PRIORITY,
            RTE_GPIO10_PIN6_IRQ_PRIORITY,
            RTE_GPIO10_PIN7_IRQ_PRIORITY,
    }
};

void GPIO10_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO10_RES, 0);    }

void GPIO10_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO10_RES, 1);    }

void GPIO10_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO10_RES, 2);    }

void GPIO10_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO10_RES, 3);    }

void GPIO10_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO10_RES, 4);    }

void GPIO10_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO10_RES, 5);    }

void GPIO10_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO10_RES, 6);    }

void GPIO10_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO10_RES, 7);    }

static int32_t ARM_GPIO10_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO10_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO10_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO10_RES, pin_no, state);
}

static int32_t ARM_GPIO10_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO10_RES, pin_no, dir);
}

static int32_t ARM_GPIO10_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO10_RES, pin_no, dir);
}

static int32_t ARM_GPIO10_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO10_RES, pin_no, value);
}

static int32_t ARM_GPIO10_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO10_RES, pin_no, value);
}

static int32_t ARM_GPIO10_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO10_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO10_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO10_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO10;
ARM_DRIVER_GPIO Driver_GPIO10 = {
    ARM_GPIO10_Initialize,
    ARM_GPIO10_PowerControl,
    ARM_GPIO10_SetDirection,
    ARM_GPIO10_GetDirection,
    ARM_GPIO10_SetValue,
    ARM_GPIO10_GetValue,
    ARM_GPIO10_Control,
    ARM_GPIO10_Uninitialize
};
#endif   /* RTE_GPIO10 */


/**<GPIO Instance 11>*/
#if RTE_GPIO11
static GPIO_RESOURCES GPIO11_RES = {
    .reg_base = (GPIO_Type*) GPIO11_BASE,
    .IRQ_base_num = GPIO11_IRQ0_IRQn,
    .gpio_id = GPIO11_INSTANCE,
    .db_clkdiv = RTE_GPIO11_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO11_PIN0_IRQ_PRIORITY,
            RTE_GPIO11_PIN1_IRQ_PRIORITY,
            RTE_GPIO11_PIN2_IRQ_PRIORITY,
            RTE_GPIO11_PIN3_IRQ_PRIORITY,
            RTE_GPIO11_PIN4_IRQ_PRIORITY,
            RTE_GPIO11_PIN5_IRQ_PRIORITY,
            RTE_GPIO11_PIN6_IRQ_PRIORITY,
            RTE_GPIO11_PIN7_IRQ_PRIORITY,
    }
};

void GPIO11_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO11_RES, 0);    }

void GPIO11_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO11_RES, 1);    }

void GPIO11_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO11_RES, 2);    }

void GPIO11_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO11_RES, 3);    }

void GPIO11_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO11_RES, 4);    }

void GPIO11_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO11_RES, 5);    }

void GPIO11_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO11_RES, 6);    }

void GPIO11_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO11_RES, 7);    }

static int32_t ARM_GPIO11_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO11_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO11_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO11_RES, pin_no, state);
}

static int32_t ARM_GPIO11_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO11_RES, pin_no, dir);
}

static int32_t ARM_GPIO11_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO11_RES, pin_no, dir);
}

static int32_t ARM_GPIO11_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO11_RES, pin_no, value);
}

static int32_t ARM_GPIO11_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO11_RES, pin_no, value);
}

static int32_t ARM_GPIO11_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO11_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO11_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO11_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO11;
ARM_DRIVER_GPIO Driver_GPIO11 = {
    ARM_GPIO11_Initialize,
    ARM_GPIO11_PowerControl,
    ARM_GPIO11_SetDirection,
    ARM_GPIO11_GetDirection,
    ARM_GPIO11_SetValue,
    ARM_GPIO11_GetValue,
    ARM_GPIO11_Control,
    ARM_GPIO11_Uninitialize
};
#endif   /* RTE_GPIO11 */


/**<GPIO Instance 12>*/
#if RTE_GPIO12
static GPIO_RESOURCES GPIO12_RES = {
    .reg_base = (GPIO_Type*) GPIO12_BASE,
    .IRQ_base_num = GPIO12_IRQ0_IRQn,
    .gpio_id = GPIO12_INSTANCE,
    .db_clkdiv = RTE_GPIO12_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO12_PIN0_IRQ_PRIORITY,
            RTE_GPIO12_PIN1_IRQ_PRIORITY,
            RTE_GPIO12_PIN2_IRQ_PRIORITY,
            RTE_GPIO12_PIN3_IRQ_PRIORITY,
            RTE_GPIO12_PIN4_IRQ_PRIORITY,
            RTE_GPIO12_PIN5_IRQ_PRIORITY,
            RTE_GPIO12_PIN6_IRQ_PRIORITY,
            RTE_GPIO12_PIN7_IRQ_PRIORITY,
    }
};

void GPIO12_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO12_RES, 0);    }

void GPIO12_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO12_RES, 1);    }

void GPIO12_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO12_RES, 2);    }

void GPIO12_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO12_RES, 3);    }

void GPIO12_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO12_RES, 4);    }

void GPIO12_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO12_RES, 5);    }

void GPIO12_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO12_RES, 6);    }

void GPIO12_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO12_RES, 7);    }

static int32_t ARM_GPIO12_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO12_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO12_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO12_RES, pin_no, state);
}

static int32_t ARM_GPIO12_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO12_RES, pin_no, dir);
}

static int32_t ARM_GPIO12_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO12_RES, pin_no, dir);
}

static int32_t ARM_GPIO12_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO12_RES, pin_no, value);
}

static int32_t ARM_GPIO12_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO12_RES, pin_no, value);
}

static int32_t ARM_GPIO12_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO12_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO12_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO12_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO12;
ARM_DRIVER_GPIO Driver_GPIO12 = {
    ARM_GPIO12_Initialize,
    ARM_GPIO12_PowerControl,
    ARM_GPIO12_SetDirection,
    ARM_GPIO12_GetDirection,
    ARM_GPIO12_SetValue,
    ARM_GPIO12_GetValue,
    ARM_GPIO12_Control,
    ARM_GPIO12_Uninitialize
};
#endif   /* RTE_GPIO12 */


/**<GPIO Instance 13>*/
#if RTE_GPIO13
static GPIO_RESOURCES GPIO13_RES = {
    .reg_base = (GPIO_Type*) GPIO13_BASE,
    .IRQ_base_num = GPIO13_IRQ0_IRQn,
    .gpio_id = GPIO13_INSTANCE,
    .db_clkdiv = RTE_GPIO13_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO13_PIN0_IRQ_PRIORITY,
            RTE_GPIO13_PIN1_IRQ_PRIORITY,
            RTE_GPIO13_PIN2_IRQ_PRIORITY,
            RTE_GPIO13_PIN3_IRQ_PRIORITY,
            RTE_GPIO13_PIN4_IRQ_PRIORITY,
            RTE_GPIO13_PIN5_IRQ_PRIORITY,
            RTE_GPIO13_PIN6_IRQ_PRIORITY,
            RTE_GPIO13_PIN7_IRQ_PRIORITY,
    }
};

void GPIO13_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO13_RES, 0);    }

void GPIO13_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO13_RES, 1);    }

void GPIO13_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO13_RES, 2);    }

void GPIO13_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO13_RES, 3);    }

void GPIO13_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO13_RES, 4);    }

void GPIO13_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO13_RES, 5);    }

void GPIO13_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO13_RES, 6);    }

void GPIO13_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO13_RES, 7);    }

static int32_t ARM_GPIO13_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO13_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO13_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO13_RES, pin_no, state);
}

static int32_t ARM_GPIO13_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO13_RES, pin_no, dir);
}

static int32_t ARM_GPIO13_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO13_RES, pin_no, dir);
}

static int32_t ARM_GPIO13_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO13_RES, pin_no, value);
}

static int32_t ARM_GPIO13_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO13_RES, pin_no, value);
}

static int32_t ARM_GPIO13_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO13_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO13_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO13_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO13;
ARM_DRIVER_GPIO Driver_GPIO13 = {
    ARM_GPIO13_Initialize,
    ARM_GPIO13_PowerControl,
    ARM_GPIO13_SetDirection,
    ARM_GPIO13_GetDirection,
    ARM_GPIO13_SetValue,
    ARM_GPIO13_GetValue,
    ARM_GPIO13_Control,
    ARM_GPIO13_Uninitialize
};
#endif   /* RTE_GPIO13 */


/**<GPIO Instance 14>*/
#if RTE_GPIO14
static GPIO_RESOURCES GPIO14_RES = {
    .reg_base = (GPIO_Type*) GPIO14_BASE,
    .IRQ_base_num = GPIO14_IRQ0_IRQn,
    .gpio_id = GPIO14_INSTANCE,
    .db_clkdiv = RTE_GPIO14_DB_CLK_DIV,
    .IRQ_priority = {
            RTE_GPIO14_PIN0_IRQ_PRIORITY,
            RTE_GPIO14_PIN1_IRQ_PRIORITY,
            RTE_GPIO14_PIN2_IRQ_PRIORITY,
            RTE_GPIO14_PIN3_IRQ_PRIORITY,
            RTE_GPIO14_PIN4_IRQ_PRIORITY,
            RTE_GPIO14_PIN5_IRQ_PRIORITY,
            RTE_GPIO14_PIN6_IRQ_PRIORITY,
            RTE_GPIO14_PIN7_IRQ_PRIORITY,
    }
};

void GPIO14_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&GPIO14_RES, 0);    }

void GPIO14_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&GPIO14_RES, 1);    }

void GPIO14_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&GPIO14_RES, 2);    }

void GPIO14_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&GPIO14_RES, 3);    }

void GPIO14_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&GPIO14_RES, 4);    }

void GPIO14_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&GPIO14_RES, 5);    }

void GPIO14_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&GPIO14_RES, 6);    }

void GPIO14_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&GPIO14_RES, 7);    }

static int32_t ARM_GPIO14_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&GPIO14_RES, cb_event, pin_no);
}

static int32_t ARM_GPIO14_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&GPIO14_RES, pin_no, state);
}

static int32_t ARM_GPIO14_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&GPIO14_RES, pin_no, dir);
}

static int32_t ARM_GPIO14_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&GPIO14_RES, pin_no, dir);
}

static int32_t ARM_GPIO14_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&GPIO14_RES, pin_no, value);
}

static int32_t ARM_GPIO14_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&GPIO14_RES, pin_no, value);
}

static int32_t ARM_GPIO14_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&GPIO14_RES, pin_no, control_code, arg);
}

static int32_t ARM_GPIO14_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&GPIO14_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIO14;
ARM_DRIVER_GPIO Driver_GPIO14 = {
    ARM_GPIO14_Initialize,
    ARM_GPIO14_PowerControl,
    ARM_GPIO14_SetDirection,
    ARM_GPIO14_GetDirection,
    ARM_GPIO14_SetValue,
    ARM_GPIO14_GetValue,
    ARM_GPIO14_Control,
    ARM_GPIO14_Uninitialize
};
#endif   /* RTE_GPIO14 */


/**< Low Power GPIO >*/
#if RTE_LPGPIO
static GPIO_RESOURCES LPGPIO_RES = {
    .reg_base = (GPIO_Type*) LPGPIO_BASE,
    .gpio_id = LPGPIO_INSTANCE,
    .IRQ_base_num = LPGPIO_IRQ0_IRQn,
    .IRQ_priority = {
            RTE_LPGPIO_PIN0_IRQ_PRIORITY,
            RTE_LPGPIO_PIN1_IRQ_PRIORITY,
            RTE_LPGPIO_PIN2_IRQ_PRIORITY,
            RTE_LPGPIO_PIN3_IRQ_PRIORITY,
            RTE_LPGPIO_PIN4_IRQ_PRIORITY,
            RTE_LPGPIO_PIN5_IRQ_PRIORITY,
            RTE_LPGPIO_PIN6_IRQ_PRIORITY,
            RTE_LPGPIO_PIN7_IRQ_PRIORITY,
    }
};
void LPGPIO_IRQ0Handler  (void) {   GPIO_IRQ_Handler (&LPGPIO_RES, 0);    }

void LPGPIO_IRQ1Handler  (void) {   GPIO_IRQ_Handler (&LPGPIO_RES, 1);    }

void LPGPIO_IRQ2Handler  (void) {   GPIO_IRQ_Handler (&LPGPIO_RES, 2);    }

void LPGPIO_IRQ3Handler  (void) {   GPIO_IRQ_Handler (&LPGPIO_RES, 3);    }

void LPGPIO_IRQ4Handler  (void) {   GPIO_IRQ_Handler (&LPGPIO_RES, 4);    }

void LPGPIO_IRQ5Handler  (void) {   GPIO_IRQ_Handler (&LPGPIO_RES, 5);    }

void LPGPIO_IRQ6Handler  (void) {   GPIO_IRQ_Handler (&LPGPIO_RES, 6);    }

void LPGPIO_IRQ7Handler  (void) {   GPIO_IRQ_Handler (&LPGPIO_RES, 7);    }

static int32_t ARM_LPGPIO_Initialize (uint8_t pin_no, ARM_GPIO_SignalEvent_t cb_event)
{
    return GPIO_Initialize (&LPGPIO_RES, cb_event, pin_no);
}

static int32_t ARM_LPGPIO_PowerControl (uint8_t pin_no, ARM_POWER_STATE state)
{
    return GPIO_PowerControl (&LPGPIO_RES, pin_no, state);
}

static int32_t ARM_LPGPIO_SetDirection (uint8_t pin_no, GPIO_PIN_DIRECTION dir)
{
    return GPIO_SetDirection (&LPGPIO_RES, pin_no, dir);
}

static int32_t ARM_LPGPIO_GetDirection (uint8_t pin_no, uint32_t *dir)
{
    return GPIO_GetDirection (&LPGPIO_RES, pin_no, dir);
}

static int32_t ARM_LPGPIO_SetValue (uint8_t pin_no, GPIO_PIN_OUTPUT_STATE value)
{
    return GPIO_SetValue (&LPGPIO_RES, pin_no, value);
}

static int32_t ARM_LPGPIO_GetValue (uint8_t pin_no, uint32_t *value)
{
    return GPIO_GetValue (&LPGPIO_RES, pin_no, value);
}

static int32_t ARM_LPGPIO_Control (uint8_t pin_no, GPIO_OPERATION control_code, uint32_t *arg)
{
    return GPIO_Control (&LPGPIO_RES, pin_no, control_code, arg);
}

static int32_t ARM_LPGPIO_Uninitialize (uint8_t pin_no)
{
    return GPIO_Uninitialize (&LPGPIO_RES, pin_no);
}

extern ARM_DRIVER_GPIO Driver_GPIOLP;
ARM_DRIVER_GPIO Driver_GPIOLP = {
    ARM_LPGPIO_Initialize,
    ARM_LPGPIO_PowerControl,
    ARM_LPGPIO_SetDirection,
    ARM_LPGPIO_GetDirection,
    ARM_LPGPIO_SetValue,
    ARM_LPGPIO_GetValue,
    ARM_LPGPIO_Control,
    ARM_LPGPIO_Uninitialize
};

/* ---------- LPGPIO instance name Aliases ---------- */
extern ARM_DRIVER_GPIO __attribute__((alias("Driver_GPIOLP"))) Driver_GPIO15;
#endif   /* RTE_LPGPIO */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
