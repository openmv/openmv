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
 * @file     Driver_UTIMER.c
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     02-April-2023
 * @brief    CMSIS Driver for UTIMER.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "Driver_UTIMER.h"
#include "Driver_UTIMER_Private.h"
#include "utimer.h"

#if !(RTE_UTIMER)
#error "UTIMER is not enabled in RTE_Device.h"
#endif

#if !defined(RTE_Drivers_UTIMER)
#error "UTIMER not configured in RTE_Component.h"
#endif

/**
 * @fn      void UTIMER_Interrupt_Enable (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   Enable interrupt for UTIMER.
 * @note    none.
 * @param   UTIMER_RES : Pointer to utimer resources structure.
 * @param   channel    : Pointer to user callback function.
 * @retval  none
 */
static void UTIMER_Interrupt_Enable (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    switch (UTIMER_RES->ch_info[channel].channel_counter_dir_backup)
    {
        case ARM_UTIMER_COUNTER_UP:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
            {
                utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_OVER_FLOW);

                NVIC_ClearPendingIRQ (UTIMER_OVERFLOW_IRQ(channel));
                NVIC_SetPriority (UTIMER_OVERFLOW_IRQ(channel), UTIMER_RES->ch_info[channel].over_flow_irq_priority);
                NVIC_EnableIRQ (UTIMER_OVERFLOW_IRQ(channel));
            }
            break;
        }

        case ARM_UTIMER_COUNTER_DOWN:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
            {
                utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_UNDER_FLOW);

                NVIC_ClearPendingIRQ (UTIMER_UNDERFLOW_IRQ(channel));
                NVIC_SetPriority (UTIMER_UNDERFLOW_IRQ(channel), UTIMER_RES->ch_info[channel].under_flow_irq_priority);
                NVIC_EnableIRQ (UTIMER_UNDERFLOW_IRQ(channel));
            }
            break;
        }

        case ARM_UTIMER_COUNTER_TRIANGLE:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
            {
                utimer_unmask_interrupt(UTIMER_RES->regs, channel, (CHAN_INTERRUPT_OVER_FLOW|CHAN_INTERRUPT_OVER_FLOW));

                NVIC_ClearPendingIRQ (UTIMER_OVERFLOW_IRQ(channel));
                NVIC_SetPriority (UTIMER_OVERFLOW_IRQ(channel), UTIMER_RES->ch_info[channel].over_flow_irq_priority);
                NVIC_EnableIRQ (UTIMER_OVERFLOW_IRQ(channel));

                NVIC_ClearPendingIRQ (UTIMER_UNDERFLOW_IRQ(channel));
                NVIC_SetPriority (UTIMER_UNDERFLOW_IRQ(channel), UTIMER_RES->ch_info[channel].under_flow_irq_priority);
                NVIC_EnableIRQ (UTIMER_UNDERFLOW_IRQ(channel));
            }
            break;
        }
    }

    switch (UTIMER_RES->ch_info[channel].channel_mode_backup)
    {
        case ARM_UTIMER_MODE_CAPTURING:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.driver_A)
            {
                utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_A);

                if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
                {
                    NVIC_ClearPendingIRQ (UTIMER_CAPTURE_A_IRQ(channel));
                    NVIC_SetPriority (UTIMER_CAPTURE_A_IRQ(channel), UTIMER_RES->ch_info[channel].capture_A_irq_priority);
                    NVIC_EnableIRQ (UTIMER_CAPTURE_A_IRQ(channel));
                }
                else
                {
                    NVIC_ClearPendingIRQ (QEC_CAPTURE_A_IRQ(channel));
                    NVIC_SetPriority (QEC_CAPTURE_A_IRQ(channel), UTIMER_RES->ch_info[channel].capture_A_irq_priority);
                    NVIC_EnableIRQ (QEC_CAPTURE_A_IRQ(channel));
                }
            }
            if (UTIMER_RES->ch_info[channel].ch_config.driver_B)
            {
                utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_B);

                if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
                {
                    NVIC_ClearPendingIRQ (UTIMER_CAPTURE_B_IRQ(channel));
                    NVIC_SetPriority (UTIMER_CAPTURE_B_IRQ(channel), UTIMER_RES->ch_info[channel].capture_B_irq_priority);
                    NVIC_EnableIRQ (UTIMER_CAPTURE_B_IRQ(channel));
                }
                else
                {
                    NVIC_ClearPendingIRQ (QEC_CAPTURE_B_IRQ(channel));
                    NVIC_SetPriority (QEC_CAPTURE_B_IRQ(channel), UTIMER_RES->ch_info[channel].capture_B_irq_priority);
                    NVIC_EnableIRQ (QEC_CAPTURE_B_IRQ(channel));
                }
            }
            break;
        }
        case ARM_UTIMER_MODE_COMPARING:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
            {
                if (UTIMER_RES->ch_info[channel].ch_config.driver_A)
                {
                    utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_A);

                    NVIC_ClearPendingIRQ (UTIMER_CAPTURE_A_IRQ(channel));
                    NVIC_SetPriority (UTIMER_CAPTURE_A_IRQ(channel), UTIMER_RES->ch_info[channel].capture_A_irq_priority);
                    NVIC_EnableIRQ (UTIMER_CAPTURE_A_IRQ(channel));

                    if (UTIMER_RES->ch_info[channel].ch_config.buffer_operation)
                    {
                        utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_A_BUF1);

                        NVIC_ClearPendingIRQ (UTIMER_CAPTURE_C_IRQ(channel));
                        NVIC_SetPriority (UTIMER_CAPTURE_C_IRQ(channel), UTIMER_RES->ch_info[channel].capture_C_irq_priority);
                        NVIC_EnableIRQ (UTIMER_CAPTURE_C_IRQ(channel));

                        if (UTIMER_RES->ch_info[channel].ch_config.buffering_type)
                        {
                            utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_A_BUF2);

                            NVIC_ClearPendingIRQ (UTIMER_CAPTURE_D_IRQ(channel));
                            NVIC_SetPriority (UTIMER_CAPTURE_D_IRQ(channel), UTIMER_RES->ch_info[channel].capture_D_irq_priority);
                            NVIC_EnableIRQ (UTIMER_CAPTURE_D_IRQ(channel));
                        }
                    }
                }
                if (UTIMER_RES->ch_info[channel].ch_config.driver_B)
                {
                    utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_B);

                    NVIC_ClearPendingIRQ (UTIMER_CAPTURE_B_IRQ(channel));
                    NVIC_SetPriority (UTIMER_CAPTURE_B_IRQ(channel), UTIMER_RES->ch_info[channel].capture_B_irq_priority);
                    NVIC_EnableIRQ (UTIMER_CAPTURE_B_IRQ(channel));

                    if (UTIMER_RES->ch_info[channel].ch_config.buffer_operation)
                    {
                        utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_A_BUF1);

                        NVIC_ClearPendingIRQ (UTIMER_CAPTURE_E_IRQ(channel));
                        NVIC_SetPriority (UTIMER_CAPTURE_E_IRQ(channel), UTIMER_RES->ch_info[channel].capture_E_irq_priority);
                        NVIC_EnableIRQ (UTIMER_CAPTURE_E_IRQ(channel));

                        if (UTIMER_RES->ch_info[channel].ch_config.buffering_type)
                        {
                            utimer_unmask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_B_BUF2);

                            NVIC_ClearPendingIRQ (UTIMER_CAPTURE_F_IRQ(channel));
                            NVIC_SetPriority (UTIMER_CAPTURE_F_IRQ(channel), UTIMER_RES->ch_info[channel].capture_F_irq_priority);
                            NVIC_EnableIRQ (UTIMER_CAPTURE_F_IRQ(channel));
                        }
                    }
                }
            }
            break;
        }

        case ARM_UTIMER_MODE_BASIC:
        case ARM_UTIMER_MODE_BUFFERING:
        case ARM_UTIMER_MODE_TRIGGERING:
        case ARM_UTIMER_MODE_DEAD_TIME:
        break;
    }
}

/**
 * @fn      void UTIMER_Interrupt_Disable (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   Disable interrupt for UTIMER.
 * @note    none.
 * @param   UTIMER_RES : Pointer to utimer resources structure.
 * @param   channel    : Pointer to user callback function.
 * @retval  none
 */
static void UTIMER_Interrupt_Disable (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    switch (UTIMER_RES->ch_info[channel].channel_counter_dir_backup)
    {
        case ARM_UTIMER_COUNTER_UP:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
            {
                utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_OVER_FLOW);

                NVIC_ClearPendingIRQ (UTIMER_OVERFLOW_IRQ(channel));
                NVIC_DisableIRQ (UTIMER_OVERFLOW_IRQ(channel));
            }
            break;
        }

        case ARM_UTIMER_COUNTER_DOWN:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
            {
                utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_UNDER_FLOW);

                NVIC_ClearPendingIRQ (UTIMER_UNDERFLOW_IRQ(channel));
                NVIC_DisableIRQ (UTIMER_UNDERFLOW_IRQ(channel));
            }
            break;
        }

        case ARM_UTIMER_COUNTER_TRIANGLE:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
            {
                utimer_mask_interrupt(UTIMER_RES->regs, channel, (CHAN_INTERRUPT_OVER_FLOW|CHAN_INTERRUPT_UNDER_FLOW));

                NVIC_ClearPendingIRQ (UTIMER_OVERFLOW_IRQ(channel));
                NVIC_ClearPendingIRQ (UTIMER_UNDERFLOW_IRQ(channel));
                NVIC_DisableIRQ (UTIMER_OVERFLOW_IRQ(channel));
                NVIC_DisableIRQ (UTIMER_UNDERFLOW_IRQ(channel));
            }
            break;
        }
    }

    switch (UTIMER_RES->ch_info[channel].channel_mode_backup)
    {
        case ARM_UTIMER_MODE_CAPTURING:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.driver_A)
            {
                utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_A);

                if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
                {
                    NVIC_ClearPendingIRQ (UTIMER_CAPTURE_A_IRQ(channel));
                    NVIC_DisableIRQ (UTIMER_CAPTURE_A_IRQ(channel));
                }
                else
                {
                    NVIC_ClearPendingIRQ (QEC_CAPTURE_A_IRQ(channel));
                    NVIC_DisableIRQ (QEC_CAPTURE_A_IRQ(channel));
                }
            }
            if (UTIMER_RES->ch_info[channel].ch_config.driver_B)
            {
                utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_B);

                if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
                {
                    NVIC_ClearPendingIRQ (UTIMER_CAPTURE_B_IRQ(channel));
                    NVIC_DisableIRQ (UTIMER_CAPTURE_B_IRQ(channel));
                }
                else
                {
                    NVIC_ClearPendingIRQ (QEC_CAPTURE_B_IRQ(channel));
                    NVIC_DisableIRQ (QEC_CAPTURE_B_IRQ(channel));
                }
            }
            break;
        }
        case ARM_UTIMER_MODE_COMPARING:
        {
            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode)
            {
                if (UTIMER_RES->ch_info[channel].ch_config.driver_A)
                {
                    utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_A);

                    NVIC_ClearPendingIRQ (UTIMER_CAPTURE_A_IRQ(channel));
                    NVIC_DisableIRQ (UTIMER_CAPTURE_A_IRQ(channel));

                    if (UTIMER_RES->ch_info[channel].ch_config.buffer_operation)
                    {
                        utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_A_BUF1);

                        NVIC_ClearPendingIRQ (UTIMER_CAPTURE_C_IRQ(channel));
                        NVIC_DisableIRQ (UTIMER_CAPTURE_C_IRQ(channel));

                        if (UTIMER_RES->ch_info[channel].ch_config.buffering_type)
                        {
                            utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_A_BUF2);

                            NVIC_ClearPendingIRQ (UTIMER_CAPTURE_D_IRQ(channel));
                            NVIC_DisableIRQ (UTIMER_CAPTURE_D_IRQ(channel));
                        }
                    }
                }
                if (UTIMER_RES->ch_info[channel].ch_config.driver_B)
                {
                    utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_B);

                    NVIC_ClearPendingIRQ (UTIMER_CAPTURE_B_IRQ(channel));
                    NVIC_DisableIRQ (UTIMER_CAPTURE_B_IRQ(channel));

                    if (UTIMER_RES->ch_info[channel].ch_config.buffer_operation)
                    {
                        utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_B_BUF1);

                        NVIC_ClearPendingIRQ (UTIMER_CAPTURE_E_IRQ(channel));
                        NVIC_DisableIRQ (UTIMER_CAPTURE_E_IRQ(channel));

                        if (UTIMER_RES->ch_info[channel].ch_config.buffering_type)
                        {
                            utimer_mask_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_B_BUF2);

                            NVIC_ClearPendingIRQ (UTIMER_CAPTURE_F_IRQ(channel));
                            NVIC_DisableIRQ (UTIMER_CAPTURE_F_IRQ(channel));
                        }
                    }
                }
            }
            break;
        }

        case ARM_UTIMER_MODE_BASIC:
        case ARM_UTIMER_MODE_BUFFERING:
        case ARM_UTIMER_MODE_TRIGGERING:
        case ARM_UTIMER_MODE_DEAD_TIME:
        break;
    }
}

/**
 * @fn      int32_t ARM_UTIMER_Initialize (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, ARM_UTIMER_SignalEvent_t cb_unit_event)
 * @brief   Initialize the UTIMER.
 * @note    none.
 * @param   UTIMER_RES    : Pointer to utimer resources structure.
 * @param   channel       : channel number.
 * @param   cb_unit_event : Pointer to user callback function.
 * @retval  \ref execution_status
 */
static int32_t ARM_UTIMER_Initialize (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, ARM_UTIMER_SignalEvent_t cb_unit_event)
{
    if((cb_unit_event == NULL) && (channel < ARM_UTIMER_CHANNEL12))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if(channel > ARM_UTIMER_MAX_CHANNEL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if(UTIMER_RES->ch_info[channel].state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

    if (channel < ARM_UTIMER_CHANNEL12)
    {
        UTIMER_RES->ch_info[channel].ch_config.utimer_mode = UTIMER_MODE_ENABLE;
    }
    else
    {
        UTIMER_RES->ch_info[channel].ch_config.utimer_mode = QEC_MODE_ENABLE;
    }

    UTIMER_RES->ch_info[channel].CB_function_ptr = cb_unit_event;

    UTIMER_RES->ch_info[channel].state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_UTIMER_PowerControl (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, ARM_POWER_STATE state)
 * @brief   Handles the utimer power.
 * @note    none.
 * @param   UTIMER_RES : Pointer to utimer resources structure.
 * @param   channel    : channel number.
 * @param   state      : power state.
 * @retval  \ref execution_status
 */
static int32_t ARM_UTIMER_PowerControl (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, ARM_POWER_STATE state)
{
    if (channel > ARM_UTIMER_MAX_CHANNEL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            utimer_control_disable (UTIMER_RES->regs, channel);

            utimer_driver_output_disable(UTIMER_RES->regs, channel);

            if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode == 0 &&
                UTIMER_RES->ch_info[channel].channel_mode_backup == ARM_UTIMER_MODE_CAPTURING)
            {
                UTIMER_Interrupt_Disable(UTIMER_RES, channel);
            }

            utimer_reset(UTIMER_RES->regs, channel);

            /* disable channel clock */
            utimer_clock_disable(UTIMER_RES->regs, channel);

            UTIMER_RES->ch_info[channel].state.powered = 0;
            break;
        }
        case ARM_POWER_FULL:
        {
            if(UTIMER_RES->ch_info[channel].state.powered == 1)
            {
                return ARM_DRIVER_OK;
            }
            if(UTIMER_RES->ch_info[channel].state.initialized != 1)
            {
                return ARM_DRIVER_ERROR;
            }

            /* enabling channel clock */
            utimer_clock_enable (UTIMER_RES->regs, channel);

            utimer_control_enable (UTIMER_RES->regs, channel);

            UTIMER_RES->ch_info[channel].state.powered = 1;
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
 * @fn      int32_t ARM_UTIMER_ConfigCounter (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, UTIMER_MODE mode, ARM_UTIMER_COUNTER_DIR dir)
 * @brief   to configure utimer mode and type.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @param   mode         : counter mode.
 * @param   dir          : counter direction.
 * @retval  \ref execution_status
 */
static int32_t ARM_UTIMER_ConfigCounter (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, ARM_UTIMER_MODE mode, ARM_UTIMER_COUNTER_DIR dir)
{
    if (channel > ARM_UTIMER_MAX_CHANNEL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (UTIMER_RES->ch_info[channel].state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }
    if (UTIMER_RES->ch_info[channel].ch_config.utimer_mode == QEC_MODE_ENABLE)
    {
        if ((mode != ARM_UTIMER_MODE_TRIGGERING) && (mode != ARM_UTIMER_MODE_CAPTURING))
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }

    UTIMER_RES->ch_info[channel].channel_mode_backup = mode;
    UTIMER_RES->ch_info[channel].channel_counter_dir_backup= dir;

    /* utimer direction configurations */
    utimer_config_direction (UTIMER_RES->regs, channel, (UTIMER_COUNTER_DIR) dir, &(UTIMER_RES->ch_info[channel].ch_config));

    /* utimer mode configurations */
    utimer_config_mode (UTIMER_RES->regs, channel, (UTIMER_MODE) mode, &(UTIMER_RES->ch_info[channel].ch_config));

    if ((mode != ARM_UTIMER_MODE_TRIGGERING) && (mode != ARM_UTIMER_MODE_CAPTURING))
    {
        if (UTIMER_RES->ch_info[channel].dc_enable)
        {
            utimer_enable_duty_cycle (UTIMER_RES->regs, channel, &(UTIMER_RES->ch_info[channel].ch_config));
        }
    }

    UTIMER_Interrupt_Enable (UTIMER_RES, channel);

    UTIMER_RES->ch_info[channel].state.configured = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_UTIMER_SetCount (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, UTIMER_SET_OPERATION counter, uint32_t value)
 * @brief   to set counter value.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @param   counter      : counter type.
 * @param   value        : counter value.
 * @retval  \ref execution_status
 */
static int32_t ARM_UTIMER_SetCount (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, ARM_UTIMER_COUNTER counter, uint32_t value)
{
    if (channel > ARM_UTIMER_MAX_CHANNEL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (counter > (ARM_UTIMER_COUNTER) UTIMER_COMPARE_B_BUF2)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (UTIMER_RES->ch_info[channel].state.configured == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    utimer_set_count (UTIMER_RES->regs, channel, (UTIMER_COUNTER)counter, value);

    return ARM_DRIVER_OK;
}

/**
 * @fn      uint32_t ARM_UTIMER_GetCount (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, UTIMER_GET_COUNTER_TYPE counter)
 * @brief   to get counter value.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @param   counter      : counter type.
 * @retval  counter value
 */
static uint32_t ARM_UTIMER_GetCount (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, ARM_UTIMER_COUNTER counter)
{
    return (utimer_get_count (UTIMER_RES->regs, channel, (UTIMER_COUNTER)counter));
}

/**
 * @fn      int32_t ARM_UTIMER_ConfigTrigger (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, UTIMER_TRIGGER_CONFIG *arg)
 * @brief   to configure utimer external triggers.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @param   arg          : argument.
 * @retval  \ref execution_status
 */
static int32_t ARM_UTIMER_ConfigTrigger (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, ARM_UTIMER_TRIGGER_CONFIG *arg)
{
    uint32_t trigger = 0;

    if(channel > ARM_UTIMER_MAX_CHANNEL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if(UTIMER_RES->ch_info[channel].ch_config.utimer_mode == QEC_MODE_ENABLE)
    {
        if ((arg->triggerSrc != ARM_UTIMER_SRC_0) || (arg->trigger > ARM_UTIMER_SRC0_TRIG11_FALLING))
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
    }
    if((arg->triggerTarget > ARM_UTIMER_TRIGGER_DMA_CLEAR_B) || (arg->triggerSrc > ARM_UTIMER_CNTR_PAUSE_TRIGGER))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if(((arg->triggerSrc == ARM_UTIMER_SRC_0) && ((ARM_UTIMER_SRC0_TRIG0_RISING <= arg->trigger) && (arg->trigger >= ARM_UTIMER_SRC0_TRIG15_FALLING))) ||
       ((arg->triggerSrc == ARM_UTIMER_SRC_1) && ((ARM_UTIMER_SRC1_DRIVE_A_RISING_B_0 <= arg->trigger) && (arg->trigger >= ARM_UTIMER_SRC1_DRIVE_B_FALLING_A_1))) ||
       ((arg->triggerSrc == ARM_UTIMER_FAULT_TRIGGER) && ((ARM_UTIMER_FAULT_TRIG0_RISING <= arg->trigger) && (arg->trigger >= ARM_UTIMER_FAULT_TRIG3_FALLING))) ||
       ((arg->triggerSrc == ARM_UTIMER_CNTR_PAUSE_TRIGGER) && ((ARM_UTIMER_PAUSE_SRC_0_HIGH <= arg->trigger) && (arg->trigger >= ARM_UTIMER_PAUSE_SRC_1_LOW))))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if(UTIMER_RES->ch_info[channel].state.configured == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    trigger = UTIMER_Get_TriggerType (arg->trigger);

    UTIMER_TRIGGER_CONFIG trigger_arg = {
        .trigger_target = (UTIMER_TRIGGER_TARGET) arg->triggerTarget,
        .src_type = (UTIMER_TRIGGER_SRC) arg->triggerSrc,
        .trigger_type = trigger
    };

    utimer_config_trigger (UTIMER_RES->regs, channel, &trigger_arg, &(UTIMER_RES->ch_info[channel].ch_config));

    UTIMER_RES->ch_info[channel].state.triggered = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_UTIMER_Start (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   to start utimer counter.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  \ref execution_status
 */
static int32_t ARM_UTIMER_Start (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    if(channel > ARM_UTIMER_MAX_CHANNEL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if(UTIMER_RES->ch_info[channel].state.configured == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    utimer_counter_start (UTIMER_RES->regs, channel);

    if (utimer_counter_running(UTIMER_RES->regs, channel))
    {
        UTIMER_RES->ch_info[channel].state.started = 1;
        return ARM_DRIVER_OK;
    }
    else
    {
        return ARM_DRIVER_ERROR;
    }
}

/**
 * @fn      int32_t ARM_UTIMER_Stop (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, bool count_clear_option)
 * @brief   to stop utimer counter.
 * @note    none.
 * @param   UTIMER_RES         : Pointer to utimer resources structure.
 * @param   channel            : channel number.
 * @param   count_clear_option : counter clear option.
 * @retval  \ref execution_status
 */
static int32_t ARM_UTIMER_Stop (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel, bool count_clear_option)
{
    if(channel > ARM_UTIMER_MAX_CHANNEL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    utimer_counter_stop (UTIMER_RES->regs, channel, count_clear_option);

    if (utimer_counter_running(UTIMER_RES->regs, channel))
    {
        return ARM_DRIVER_ERROR;
    }
    else
    {
        UTIMER_RES->ch_info[channel].state.started = 0;
        return ARM_DRIVER_OK;
    }
}

/**
 * @fn      int32_t ARM_UTIMER_Uninitialize (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   un-initialize utimer.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  \ref execution_status
 */
static int32_t ARM_UTIMER_Uninitialize (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    if(channel > ARM_UTIMER_MAX_CHANNEL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    UTIMER_RES->ch_info[channel].CB_function_ptr = NULL;
    UTIMER_RES->ch_info[channel].channel_counter_dir_backup = 0;
    UTIMER_RES->ch_info[channel].channel_mode_backup = 0;

    UTIMER_RES->ch_info[channel].state.initialized = 0;

    return ARM_DRIVER_OK;
}

/**
 * @fn      void UTIMER_Capture_A_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   utimer interrupt handler for compare/capture A event.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  none
 */
static void UTIMER_Capture_A_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    utimer_clear_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_A);

    if (UTIMER_RES->ch_info[channel].channel_mode_backup == ARM_UTIMER_MODE_CAPTURING)
    {
        UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_CAPTURE_A);
    }
    else
    {
        UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_COMPARE_A);
    }
}

/**
 * @fn      void UTIMER_Capture_B_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   utimer interrupt handler for compare/capture B event.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  none
 */
static void UTIMER_Capture_B_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    utimer_clear_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_CAPTURE_B);

    if (UTIMER_RES->ch_info[channel].channel_mode_backup == ARM_UTIMER_MODE_CAPTURING)
    {
        UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_CAPTURE_B);
    }
    else
    {
        UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_COMPARE_B);
    }
}

/**
 * @fn      void UTIMER_Compare_A_Buf1_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   utimer interrupt handler for compare A buf1 event.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  none
 */
static void UTIMER_Compare_A_Buf1_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    utimer_clear_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_A_BUF1);

    UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_COMPARE_A_BUF1);
}

/**
 * @fn      void UTIMER_Compare_A_Buf2_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   utimer interrupt handler for compare A buf2 event.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  none
 */
static void UTIMER_Compare_A_Buf2_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    utimer_clear_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_A_BUF2);

    UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_COMPARE_A_BUF2);
}

/**
 * @fn      void UTIMER_Compare_B_Buf1_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   utimer interrupt handler for compare B buf1 event.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  none
 */
static void UTIMER_Compare_B_Buf1_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    utimer_clear_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_B_BUF1);

    UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_COMPARE_B_BUF1);
}

/**
 * @fn      void UTIMER_Compare_B_Buf2_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   utimer interrupt handler for compare B buf2 event.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  none
 */
static void UTIMER_Compare_B_Buf2_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    utimer_clear_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_COMPARE_B_BUF2);

    UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_COMPARE_B_BUF2);
}

/**
 * @fn      void UTIMER_OverFlow_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   utimer interrupt handler for overflow event.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  none
 */
static void UTIMER_OverFlow_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    utimer_clear_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_OVER_FLOW);

    UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_OVER_FLOW);
}

/**
 * @fn      void UTIMER_UnderFlow_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
 * @brief   utimer interrupt handler for underflow event.
 * @note    none.
 * @param   UTIMER_RES   : Pointer to utimer resources structure.
 * @param   channel      : channel number.
 * @retval  none
 */
static void UTIMER_UnderFlow_IRQHandler (UTIMER_RESOURCES *UTIMER_RES, uint8_t channel)
{
    utimer_clear_interrupt(UTIMER_RES->regs, channel, CHAN_INTERRUPT_UNDER_FLOW);

    UTIMER_RES->ch_info[channel].CB_function_ptr(ARM_UTIMER_EVENT_UNDER_FLOW);
}

/* UTIMER0 driver instance */
#if RTE_UTIMER
static UTIMER_RESOURCES UTIMER0 = {
    .regs        = (UTIMER_Type*) UTIMER_BASE,
    .ch_info[ARM_UTIMER_CHANNEL0]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL0_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL0_DRIVER_A,
            .driver_B = RTE_UTIMER_CHANNEL0_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL0_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL0_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL0_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL0_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL0_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL0_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL0_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL0_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL0_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL0_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL0_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL0_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL0_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL0_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL0_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL0_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL0_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL0_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL0_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL0_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL0_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL1]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL1_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL1_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL1_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL1_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL1_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL1_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL1_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL1_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL1_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL1_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL1_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL1_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL1_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL1_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL1_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL1_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL1_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL1_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL1_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL1_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL1_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL1_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL1_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL1_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL2]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL2_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL2_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL2_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL2_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL2_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL2_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL2_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL2_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL2_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL2_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL2_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL2_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL2_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL2_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL2_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL2_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL2_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL2_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL2_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL2_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL2_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL2_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL2_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL2_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL3]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL3_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL3_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL3_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL3_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL3_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL3_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL3_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL3_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL3_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL3_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL3_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL3_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL3_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL3_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL3_EVENT_AT_CREST,
            .buffering_type = RTE_UTIMER_CHANNEL3_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL3_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL3_BUFFERING_TYPE_B,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL3_EVENT_AT_TROUGH,
            .buffer_operation  = RTE_UTIMER_CHANNEL3_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL3_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL3_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL3_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL3_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL4]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL4_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL4_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL4_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL4_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL4_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL4_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL4_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL4_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL4_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL4_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL4_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL4_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL4_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL4_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL4_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL4_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL4_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL4_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL4_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL4_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL4_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL4_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL4_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL4_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL5]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL5_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL5_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL5_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL5_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL5_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL5_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL5_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL5_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL5_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL5_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL5_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL5_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL5_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL5_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL5_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL5_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL5_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL5_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL5_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL5_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL5_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL5_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL5_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL5_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL6]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL6_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL6_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL6_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL6_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL6_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL6_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL6_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL6_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL6_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL6_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL6_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL6_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL6_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL6_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL6_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL6_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL6_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL6_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL6_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL6_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL6_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL6_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL6_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL6_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL7]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL7_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL7_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL7_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL7_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL7_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL7_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL7_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL7_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL7_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL7_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL7_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL7_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL7_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL7_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL7_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL7_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL7_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL7_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL7_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL7_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL7_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL7_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL7_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL7_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL8]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL8_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL8_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL8_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL8_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL8_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL8_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL8_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL8_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL8_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL8_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL8_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL8_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL8_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL8_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL8_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL8_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL8_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL8_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL8_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL8_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL8_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL8_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL8_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL8_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL9]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL9_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL9_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL9_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL9_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL9_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL9_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL9_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL9_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL9_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL9_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL9_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL9_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL9_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL9_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL9_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL9_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL9_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL9_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL9_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL9_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL9_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL9_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL9_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL9_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL10]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL10_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL10_DRIVER_A,
            .driver_B = RTE_UTIMER_CHANNEL10_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL10_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL10_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL10_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL10_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL10_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL10_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL10_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL10_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL10_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL10_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL10_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL10_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL10_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL10_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL10_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL10_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL10_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL10_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL10_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL10_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL10_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL11]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL11_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL11_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL11_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL11_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL11_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL11_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL11_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL11_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL11_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL11_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL11_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL11_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL11_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL11_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL11_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL11_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL11_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL11_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL11_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL11_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL11_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL11_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL11_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority = RTE_UTIMER_CHANNEL11_UNDER_FLOW_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL12]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL12_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL12_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL12_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL12_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL12_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL12_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL12_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL12_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL12_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL12_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL12_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL12_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL12_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL12_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL12_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL12_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL12_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL12_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL12_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL12_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL12_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL12_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL12_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL12_CAPTURE_B_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL13]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL13_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL13_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL13_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL13_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL13_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL13_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL13_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL13_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL13_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL13_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL13_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL13_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL13_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL13_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL13_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL13_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL13_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL13_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL13_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL13_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL13_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL13_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL13_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL13_CAPTURE_B_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL14]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL14_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL14_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL14_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL14_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL14_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL14_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL14_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL14_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL14_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL14_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL14_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL14_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL14_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL14_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL14_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL14_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL14_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL14_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL14_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL14_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL14_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL14_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL14_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL14_CAPTURE_B_IRQ_PRIORITY
    },
    .ch_info[ARM_UTIMER_CHANNEL15]  = {
        .ch_config = {
            .buf_trough_n_crest = RTE_UTIMER_CHANNEL15_BUF_TROUGH_N_CREST,
            .driver_A = RTE_UTIMER_CHANNEL15_DRIVER_A,
            .driver_B =RTE_UTIMER_CHANNEL15_DRIVER_B,
            .dma_ctrl = RTE_UTIMER_CHANNEL15_DMA_CONTROL,
            .fault_type = RTE_UTIMER_CHANNEL15_FAULT_TYPE,
            .fixed_buffer = RTE_UTIMER_CHANNEL15_FIXED_BUFFER,
            .driver_a_at_comp_match = RTE_UTIMER_CHANNEL15_DRV_A_OP_AT_MATCH_COUNT,
            .driver_a_at_cycle_end = RTE_UTIMER_CHANNEL15_DRV_A_OP_AT_CYCLE_END,
            .driver_a_start_state = RTE_UTIMER_CHANNEL15_DRV_A_START_STATE,
            .driver_a_stop_state = RTE_UTIMER_CHANNEL15_DRV_A_STOP_STATE,
            .driver_b_at_comp_match = RTE_UTIMER_CHANNEL15_DRV_B_OP_AT_MATCH_COUNT,
            .driver_b_at_cycle_end = RTE_UTIMER_CHANNEL15_DRV_B_OP_AT_CYCLE_END,
            .driver_b_start_state = RTE_UTIMER_CHANNEL15_DRV_B_START_STATE,
            .driver_b_stop_state = RTE_UTIMER_CHANNEL15_DRV_B_STOP_STATE,
            .comp_buffer_at_crest = RTE_UTIMER_CHANNEL15_EVENT_AT_CREST,
            .comp_buffer_at_trough = RTE_UTIMER_CHANNEL15_EVENT_AT_TROUGH,
            .buffering_type = RTE_UTIMER_CHANNEL15_BUFFERING_TYPE,
            .capt_buffer_type_A = RTE_UTIMER_CHANNEL15_BUFFERING_TYPE_A,
            .capt_buffer_type_B = RTE_UTIMER_CHANNEL15_BUFFERING_TYPE_B,
            .buffer_operation  = RTE_UTIMER_CHANNEL15_BUFFER_OPERATION,
            .dc_value = RTE_UTIMER_CHANNEL15_DUTY_CYCLE_VALUE
        },
        .dc_enable = RTE_UTIMER_CHANNEL15_DUTY_CYCLE_ENABLE,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL15_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL15_CAPTURE_B_IRQ_PRIORITY
    }
};

static void UTIMER_IRQHandler_Capture_A(uint8_t channel)
{
    UTIMER_Capture_A_IRQHandler(&UTIMER0, channel);
}

static void UTIMER_IRQHandler_Capture_B(uint8_t channel)
{
    UTIMER_Capture_B_IRQHandler(&UTIMER0, channel);
}

static void UTIMER_IRQHandler_Compare_A_Buf1(uint8_t channel)
{
    UTIMER_Compare_A_Buf1_IRQHandler(&UTIMER0, channel);
}

static void UTIMER_IRQHandler_Compare_A_Buf2(uint8_t channel)
{
    UTIMER_Compare_A_Buf2_IRQHandler(&UTIMER0, channel);
}

static void UTIMER_IRQHandler_Compare_B_Buf1(uint8_t channel)
{
    UTIMER_Compare_B_Buf1_IRQHandler(&UTIMER0, channel);
}

static void UTIMER_IRQHandler_Compare_B_Buf2(uint8_t channel)
{
    UTIMER_Compare_B_Buf2_IRQHandler(&UTIMER0, channel);
}

static void UTIMER_IRQHandler_OverFlow(uint8_t channel)
{
    UTIMER_OverFlow_IRQHandler(&UTIMER0, channel);
}

static void UTIMER_IRQHandler_UnderFlow(uint8_t channel)
{
    UTIMER_UnderFlow_IRQHandler(&UTIMER0, channel);
}

void QEC0_CMPA_IRQHandler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL12);
}

void QEC0_CMPB_IRQHandler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL12);
}

void QEC1_CMPA_IRQHandler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL13);
}

void QEC1_CMPB_IRQHandler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL13);
}

void QEC2_CMPA_IRQHandler(void)
{
	UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL14);
}

void QEC2_CMPB_IRQHandler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL14);
}

void QEC3_CMPA_IRQHandler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL15);
}

void QEC3_CMPB_IRQHandler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL15);
}

void UTIMER_IRQ0Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL0);
}

void UTIMER_IRQ1Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL0);
}

void UTIMER_IRQ2Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL0);
}

void UTIMER_IRQ3Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL0);
}
void UTIMER_IRQ4Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL0);
}

void UTIMER_IRQ5Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL0);
}

void UTIMER_IRQ6Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL0);
}

void UTIMER_IRQ7Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL0);
}

void UTIMER_IRQ8Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL1);
}

void UTIMER_IRQ9Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL1);
}

void UTIMER_IRQ10Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL1);
}

void UTIMER_IRQ11Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL1);
}

void UTIMER_IRQ12Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL1);
}

void UTIMER_IRQ13Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL1);
}

void UTIMER_IRQ14Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL1);
}

void UTIMER_IRQ15Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL1);
}

void UTIMER_IRQ16Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL2);
}

void UTIMER_IRQ17Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL2);
}

void UTIMER_IRQ18Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL2);
}

void UTIMER_IRQ19Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL2);
}

void UTIMER_IRQ20Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL2);
}

void UTIMER_IRQ21Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL2);
}

void UTIMER_IRQ22Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL2);
}

void UTIMER_IRQ23Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL2);
}

void UTIMER_IRQ24Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL3);
}

void UTIMER_IRQ25Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL3);
}

void UTIMER_IRQ26Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL3);
}

void UTIMER_IRQ27Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL3);
}

void UTIMER_IRQ28Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL3);
}

void UTIMER_IRQ29Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL3);
}

void UTIMER_IRQ30Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL3);
}

void UTIMER_IRQ31Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL3);
}

void UTIMER_IRQ32Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL4);
}

void UTIMER_IRQ33Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL4);
}

void UTIMER_IRQ34Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL4);
}

void UTIMER_IRQ35Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL4);
}

void UTIMER_IRQ36Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL4);
}

void UTIMER_IRQ37Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL4);
}

void UTIMER_IRQ38Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL4);
}

void UTIMER_IRQ39Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL4);
}

void UTIMER_IRQ40Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL5);
}

void UTIMER_IRQ41Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL5);
}

void UTIMER_IRQ42Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL5);
}

void UTIMER_IRQ43Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL5);
}

void UTIMER_IRQ44Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL5);
}

void UTIMER_IRQ45Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL5);
}

void UTIMER_IRQ46Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL5);
}

void UTIMER_IRQ47Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL5);
}

void UTIMER_IRQ48Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL6);
}

void UTIMER_IRQ49Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL6);
}

void UTIMER_IRQ50Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL6);
}

void UTIMER_IRQ51Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL6);
}

void UTIMER_IRQ52Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL6);
}

void UTIMER_IRQ53Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL6);
}

void UTIMER_IRQ54Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL6);
}

void UTIMER_IRQ55Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL6);
}

void UTIMER_IRQ56Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL7);
}

void UTIMER_IRQ57Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL7);
}

void UTIMER_IRQ58Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL7);
}

void UTIMER_IRQ59Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL7);
}

void UTIMER_IRQ60Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL7);
}

void UTIMER_IRQ61Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL7);
}

void UTIMER_IRQ62Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL7);
}

void UTIMER_IRQ63Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL7);
}

void UTIMER_IRQ64Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL8);
}

void UTIMER_IRQ65Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL8);
}

void UTIMER_IRQ66Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL8);
}

void UTIMER_IRQ67Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL8);
}

void UTIMER_IRQ68Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL8);
}

void UTIMER_IRQ69Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL8);
}

void UTIMER_IRQ70Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL8);
}

void UTIMER_IRQ71Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL8);
}

void UTIMER_IRQ72Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL9);
}

void UTIMER_IRQ73Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL9);
}

void UTIMER_IRQ74Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL9);
}

void UTIMER_IRQ75Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL9);
}

void UTIMER_IRQ76Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL9);
}

void UTIMER_IRQ77Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL9);
}

void UTIMER_IRQ78Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL9);
}

void UTIMER_IRQ79Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL9);
}

void UTIMER_IRQ80Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL10);
}

void UTIMER_IRQ81Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL10);
}

void UTIMER_IRQ82Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL10);
}

void UTIMER_IRQ83Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL10);
}

void UTIMER_IRQ84Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL10);
}

void UTIMER_IRQ85Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL10);
}

void UTIMER_IRQ86Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL10);
}

void UTIMER_IRQ87Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL10);
}

void UTIMER_IRQ88Handler(void)
{
    UTIMER_IRQHandler_Capture_A(ARM_UTIMER_CHANNEL11);
}

void UTIMER_IRQ89Handler(void)
{
    UTIMER_IRQHandler_Capture_B(ARM_UTIMER_CHANNEL11);
}

void UTIMER_IRQ90Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf1(ARM_UTIMER_CHANNEL11);
}

void UTIMER_IRQ91Handler(void)
{
    UTIMER_IRQHandler_Compare_A_Buf2(ARM_UTIMER_CHANNEL11);
}

void UTIMER_IRQ92Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf1(ARM_UTIMER_CHANNEL11);
}

void UTIMER_IRQ93Handler(void)
{
    UTIMER_IRQHandler_Compare_B_Buf2(ARM_UTIMER_CHANNEL11);
}

void UTIMER_IRQ94Handler(void)
{
    UTIMER_IRQHandler_UnderFlow(ARM_UTIMER_CHANNEL11);
}

void UTIMER_IRQ95Handler(void)
{
    UTIMER_IRQHandler_OverFlow(ARM_UTIMER_CHANNEL11);
}

static int32_t ARM_UTIMER0_Initialize(uint8_t channel, ARM_UTIMER_SignalEvent_t cb_unit_event)
{
    return ARM_UTIMER_Initialize(&UTIMER0, channel, cb_unit_event);
}

static int32_t ARM_UTIMER0_PowerControl(uint8_t channel, ARM_POWER_STATE state)
{
    return ARM_UTIMER_PowerControl(&UTIMER0, channel, state);
}

static int32_t ARM_UTIMER0_ConfigCounter(uint8_t channel, ARM_UTIMER_MODE mode, ARM_UTIMER_COUNTER_DIR dir)
{
    return ARM_UTIMER_ConfigCounter(&UTIMER0, channel, mode, dir);
}

static int32_t ARM_UTIMER0_SetCount(uint8_t channel, ARM_UTIMER_COUNTER counter, uint32_t value)
{
    return ARM_UTIMER_SetCount(&UTIMER0, channel, counter, value);
}

static uint32_t ARM_UTIMER0_GetCount(uint8_t channel, ARM_UTIMER_COUNTER counter)
{
    return ARM_UTIMER_GetCount(&UTIMER0, channel, counter);
}

static int32_t ARM_UTIMER0_ConfigTrigger(uint8_t channel, ARM_UTIMER_TRIGGER_CONFIG *arg)
{
    return ARM_UTIMER_ConfigTrigger(&UTIMER0, channel, arg);
}

static int32_t ARM_UTIMER0_Start(uint8_t channel)
{
    return ARM_UTIMER_Start(&UTIMER0, channel);
}

static int32_t ARM_UTIMER0_Stop(uint8_t channel, bool count_clear_option)
{
    return ARM_UTIMER_Stop(&UTIMER0, channel, count_clear_option);
}

static int32_t ARM_UTIMER0_Uninitialize(uint8_t channel)
{
    return ARM_UTIMER_Uninitialize(&UTIMER0, channel);
}

/*UTIMER Control Block */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER DRIVER_UTIMER0 = {
    ARM_UTIMER0_Initialize,
    ARM_UTIMER0_PowerControl,
    ARM_UTIMER0_ConfigCounter,
    ARM_UTIMER0_SetCount,
    ARM_UTIMER0_GetCount,
    ARM_UTIMER0_ConfigTrigger,
    ARM_UTIMER0_Start,
    ARM_UTIMER0_Stop,
    ARM_UTIMER0_Uninitialize
};
#endif /* RTE_UTIMER */
