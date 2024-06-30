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
 * @file     Driver_GPIO_Private.h
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     29-March-2023
 * @brief    Header file for GPIO.
 * @bug      None.
 * @Note	 None
 ******************************************************************************/

#ifndef DRIVER_GPIO_PRIVATE_H_

#define DRIVER_GPIO_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_GPIO.h"
#include "gpio.h"

#define GPIO_PORT_MAX_PIN_NUMBER                0x8U     /* Number of pins in each port */

#define ARM_GPIO_BIT_IRQ_POLARITY_Pos           0U       ///< bits - 0
#define ARM_GPIO_BIT_IRQ_POLARITY_Msk           (1U << ARM_GPIO_BIT_IRQ_POLARITY_Pos)
#define ARM_GPIO_BIT_IRQ_POLARITY(x)            (((x)& ARM_GPIO_BIT_IRQ_POLARITY_Msk) >> ARM_GPIO_BIT_IRQ_POLARITY_Pos)

#define ARM_GPIO_BIT_IRQ_BOTH_EDGE_Pos          1U       ///< bits - 1
#define ARM_GPIO_BIT_IRQ_BOTH_EDGE_Msk          (1U << ARM_GPIO_BIT_IRQ_BOTH_EDGE_Pos)
#define ARM_GPIO_BIT_IRQ_BOTH_EDGE(x)           (((x)& ARM_GPIO_BIT_IRQ_BOTH_EDGE_Msk) >> ARM_GPIO_BIT_IRQ_BOTH_EDGE_Pos)

#define ARM_GPIO_BIT_IRQ_SENSITIVE_Pos          2U       ///< bits - 2
#define ARM_GPIO_BIT_IRQ_SENSITIVE_Msk          (1U << ARM_GPIO_BIT_IRQ_SENSITIVE_Pos)
#define ARM_GPIO_BIT_IRQ_SENSITIVE(x)           (((x)& ARM_GPIO_BIT_IRQ_SENSITIVE_Msk) >> ARM_GPIO_BIT_IRQ_SENSITIVE_Pos)

/**
 * enum GPIO_INSTANCE.
 * GPIO instances.
 */
typedef enum _GPIO_INSTANCE
{
    GPIO0_INSTANCE,
    GPIO1_INSTANCE,
    GPIO2_INSTANCE,
    GPIO3_INSTANCE,
    GPIO4_INSTANCE,
    GPIO5_INSTANCE,
    GPIO6_INSTANCE,
    GPIO7_INSTANCE,
    GPIO8_INSTANCE,
    GPIO9_INSTANCE,
    GPIO10_INSTANCE,
    GPIO11_INSTANCE,
    GPIO12_INSTANCE,
    GPIO13_INSTANCE,
    GPIO14_INSTANCE,
    LPGPIO_INSTANCE
} GPIO_INSTANCE;

typedef struct _GPIO_DRV_STATE {
    uint32_t initialized : 1; /* Driver Initialized*/
    uint32_t powered     : 1; /* Driver powered */
    uint32_t reserved    : 30;/* Reserved */
} GPIO_DRV_STATE;

/**
  * @brief GPIO Resources
  */
typedef struct _GPIO_RESOURCES {
    GPIO_Type           *reg_base;                               /**< GPIO PORT Base Address>**/
    IRQn_Type           IRQ_base_num;                            /**< GPIO PORT IRQ base Num>**/
    uint16_t            db_clkdiv;                               /**< GPIO PORT debounce clk divisor: only for GPIO 0-14 >**/
    GPIO_DRV_STATE      state;                                   /**< GPIO PORT status flag >**/
    uint8_t             IRQ_priority[GPIO_PORT_MAX_PIN_NUMBER];  /**< GPIO PIN IRQ priority >**/
    GPIO_INSTANCE       gpio_id;                                 /**< GPIO instance >*/
    ARM_GPIO_SignalEvent_t cb_event[GPIO_PORT_MAX_PIN_NUMBER];   /**< GPIO Call back function >*/
} GPIO_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_GPIO_PRIVATE_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
