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
 * @file     Driver_LPTIMER.c
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     27-March-2023
 * @brief    CMSIS_Driver for LPTIMER.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "Driver_LPTIMER.h"
#include "Driver_LPTIMER_Private.h"

#if !(RTE_LPTIMER)
#error "LPTIMER is not enabled in RTE_Device.h"
#endif

#if !defined(RTE_Drivers_LPTIMER)
#error "LPTIMER is not enabled in the RTE_Components.h"
#endif

/**
 * @fn      int32_t ARM_LPTIMER_Initialize(LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel, ARM_LPTIMER_SignalEvent_t cb_event)
 * @brief   Initialize the LPTIMER.
 * @note    none.
 * @param   LPTIMER_RES : Pointer to LPTIMER resources structure.
 * @param   channel : Used LPTIMER channel.
 * @param   cb_event : Pointer to user callback function.
 * @retval  \ref execution_status
 */
static int32_t ARM_LPTIMER_Initialize (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel, ARM_LPTIMER_SignalEvent_t cb_event)
{
    if (cb_event == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (channel >= LPTIMER_MAX_CHANNEL_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (LPTIMER_RES->ch_info[channel].state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

    LPTIMER_RES->ch_info[channel].CB_function_ptr = cb_event;

    if (((channel == LPTIMER_CHANNEL_0) || (channel == LPTIMER_CHANNEL_2)) &&
        (LPTIMER_RES->ch_info[channel].clk_src > LPTIMER_CLK_EXT_SOURCE))
    {
        /* channel 0 & 2 does not support cascaded input */
        return ARM_DRIVER_ERROR;
    }
    else if (LPTIMER_RES->ch_info[channel].clk_src > LPTIMER_CLK_SOURCE_CASCADE)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Input Clock select for LPTIMER */
    select_lptimer_clk (LPTIMER_RES->ch_info[channel].clk_src, channel);

    if (LPTIMER_RES->ch_info[channel].mode)
    {
        /* set lptimer as freerun mode */
        lptimer_set_mode_freerunning (LPTIMER_RES->regs, channel);
    }
    else
    {
        /* set lptimer as user-defined mode */
        lptimer_set_mode_userdefined (LPTIMER_RES->regs, channel);
    }

    /* unmask channel interrupt */
    lptimer_unmask_interrupt (LPTIMER_RES->regs, channel);

    LPTIMER_RES->ch_info[channel].state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_LPTIMER_PowerControl (LPTIMER_RESOURCES *LPTIMER, uint8_t channel, ARM_POWER_STATE state).
 * @brief   Handles the LPTIMER power.
 * @note    none.
 * @param   LPTIMER_RES : Pointer to LPTIMER resources structure.
 * @param   channel : used LPTIMER channel.
 * @param   state : power state.
 * @retval  \ref execution_status
 */
static int32_t ARM_LPTIMER_PowerControl (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel, ARM_POWER_STATE state)
{
    if (channel >= LPTIMER_MAX_CHANNEL_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (state > ARM_POWER_FULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    switch (state)
    {
    case ARM_POWER_OFF:
    {
        if (LPTIMER_RES->ch_info[channel].state.powered == 0)
        {
            return ARM_DRIVER_OK;
        }

        NVIC_ClearPendingIRQ(LPTIMER_CHANNEL_IRQ(channel));
        NVIC_DisableIRQ(LPTIMER_CHANNEL_IRQ(channel));

        LPTIMER_RES->ch_info[channel].state.powered = 0;
        break;
    }
    case ARM_POWER_FULL:
    {
        if (LPTIMER_RES->ch_info[channel].state.powered == 1)
        {
            return ARM_DRIVER_OK;
        }

        if (LPTIMER_RES->ch_info[channel].state.initialized == 1)
        {
            NVIC_ClearPendingIRQ(LPTIMER_CHANNEL_IRQ(channel));
            NVIC_SetPriority(LPTIMER_CHANNEL_IRQ(channel), LPTIMER_RES->ch_info[channel].irq_priority);
            NVIC_EnableIRQ(LPTIMER_CHANNEL_IRQ(channel));

            LPTIMER_RES->ch_info[channel].state.powered = 1;
        }
        else
        {
            return ARM_DRIVER_ERROR;
        }
        break;
    }
    case ARM_POWER_LOW:
    {
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_LPTIMER_Control (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel, uint32_t control_code, void *arg)
 * @brief   Used to configure LPTIMER.
 * @note    none.
 * @param   LPTIMER_RES : Pointer to LPTIMER resources structure.
 * @param   channel : used LPTIMER channel.
 * @param   control_code : control code.
 * @param   arg : argument.
 * @retval  \ref execution_status
 */
static int32_t ARM_LPTIMER_Control (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel, uint32_t control_code, void *arg)
{
    if (arg == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (channel >= LPTIMER_MAX_CHANNEL_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (control_code > ARM_LPTIMER_GET_COUNT)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (LPTIMER_RES->ch_info[channel].state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (control_code)
    {
    case ARM_LPTIMER_SET_COUNT1:
    {
        if (LPTIMER_RES->ch_info[channel].mode == LPTIMER_FREE_RUN_MODE)
        {
            /* load maximum counter value*/
            lptimer_load_max_count(LPTIMER_RES->regs, channel);
        }
        else
        {
            /* load counter value */
            lptimer_load_count (LPTIMER_RES->regs, channel, arg);
        }

        LPTIMER_RES->ch_info[channel].state.set_count1 = 1;
        break;
    }
    case ARM_LPTIMER_SET_COUNT2:
    {
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    case ARM_LPTIMER_GET_COUNT:
    {
        if (LPTIMER_RES->ch_info[channel].state.started == 0)
        {
            return ARM_DRIVER_ERROR;
        }

        /* get current count value */
        *(uint32_t*)arg |= lptimer_get_count (LPTIMER_RES->regs, channel);
        break;
    }
    default:
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_LPTIMER_Start (LPTIMER_RESOURCES *LPTIMER, uint8_t channel)
 * @brief   Used to start LPTIMER counter.
 * @note    none.
 * @param   LPTIMER_RES : Pointer to LPTIMER resources structure.
 * @param   channel : used LPTIMER channel.
 * @retval  \ref execution_status
 */
static int32_t ARM_LPTIMER_Start (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel)
{
    if (channel >= LPTIMER_MAX_CHANNEL_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (LPTIMER_RES->ch_info[channel].state.started == 1)
    {
        return ARM_DRIVER_OK;
    }

    if (LPTIMER_RES->ch_info[channel].state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if (LPTIMER_RES->ch_info[channel].state.set_count1 == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    /* enable channel counter */
    lptimer_enable_counter (LPTIMER_RES->regs, channel);

    LPTIMER_RES->ch_info[channel].state.started = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_LPTIMER_Stop (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel)
 * @brief   Used to stop LPTIMER counter.
 * @note    none.
 * @param   LPTIMER_RES : Pointer to LPTIMER resources structure.
 * @param   channel : used LPTIMER channel.
 * @retval  \ref execution_status
 */
static int32_t ARM_LPTIMER_Stop (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel)
{
    if (channel >= LPTIMER_MAX_CHANNEL_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* disable channel counter */
    lptimer_disable_counter (LPTIMER_RES->regs, channel);

    LPTIMER_RES->ch_info[channel].state.started = 0;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_LPTIMER_Uninitialize (LPTIMER_RESOURCES *LPTIMER, uint8_t channel)
 * @brief   Un-Initialize the LPTIMER.
 * @note    none.
 * @param   LPTIMER_RES : Pointer to lptimer resources structure.
 * @param   channel : used LPTIMER channel.
 * @retval  \ref execution_status
 */
static int32_t ARM_LPTIMER_Uninitialize (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel)
{
    if (channel >= LPTIMER_MAX_CHANNEL_NUMBER)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (LPTIMER_RES->ch_info[channel].state.initialized == 0)
    {
        return ARM_DRIVER_OK;
    }

    LPTIMER_RES->ch_info[channel].CB_function_ptr = NULL;

    LPTIMER_RES->ch_info[channel].state.initialized = 0;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t LPTIMER_Irq_Handler (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel)
 * @brief   LPTIMER interrupt handler.
 * @note    none.
 * @param   LPTIMER_RES : Pointer to lptimer resources structure.
 * @param   channel : used LPTIMER channel.
 * @retval  \ref execution_status
 */
static int32_t LPTIMER_Irq_Handler (LPTIMER_RESOURCES *LPTIMER_RES, uint8_t channel)
{
    /* clear channel interrupt  */
    lptimer_clear_interrupt (LPTIMER_RES->regs, channel);

    LPTIMER_RES->ch_info[channel].CB_function_ptr(ARM_LPTIMER_EVENT_UNDERFLOW);

    return ARM_DRIVER_OK;
}

/* LPTIMER0 Driver Instance */
#if (RTE_LPTIMER)

static LPTIMER_RESOURCES LPTIMER0 = {
    .regs   = (LPTIMER_Type*) LPTIMER_BASE,
    .ch_info[0] = {
        .mode = RTE_LPTIMER_CHANNEL0_FREE_RUN_MODE,
        .clk_src = RTE_LPTIMER_CHANNEL0_CLK_SRC,
        .irq_priority = RTE_LPTIMER_CHANNEL0_IRQ_PRIORITY
    },
    .ch_info[1] = {
        .mode = RTE_LPTIMER_CHANNEL1_FREE_RUN_MODE,
        .clk_src = RTE_LPTIMER_CHANNEL1_CLK_SRC,
        .irq_priority = RTE_LPTIMER_CHANNEL1_IRQ_PRIORITY
    },
    .ch_info[2] = {
        .mode = RTE_LPTIMER_CHANNEL2_FREE_RUN_MODE,
        .clk_src = RTE_LPTIMER_CHANNEL2_CLK_SRC,
        .irq_priority = RTE_LPTIMER_CHANNEL2_IRQ_PRIORITY
    },
    .ch_info[3] = {
        .mode = RTE_LPTIMER_CHANNEL3_FREE_RUN_MODE,
        .clk_src = RTE_LPTIMER_CHANNEL3_CLK_SRC,
        .irq_priority = RTE_LPTIMER_CHANNEL3_IRQ_PRIORITY
    }
};

void LPTIMER0_IRQHandler (void)
{
    LPTIMER_Irq_Handler (&LPTIMER0, LPTIMER_CHANNEL_0);
}

void LPTIMER1_IRQHandler (void)
{
    LPTIMER_Irq_Handler (&LPTIMER0, LPTIMER_CHANNEL_1);
}

void LPTIMER2_IRQHandler (void)
{
    LPTIMER_Irq_Handler (&LPTIMER0, LPTIMER_CHANNEL_2);
}

void LPTIMER3_IRQHandler (void)
{
    LPTIMER_Irq_Handler (&LPTIMER0, LPTIMER_CHANNEL_3);
}

static int32_t ARM_LPTIMER0_Initialize (uint8_t channel, ARM_LPTIMER_SignalEvent_t cb_unit_event)
{
    return ARM_LPTIMER_Initialize (&LPTIMER0, channel, cb_unit_event);
}

static int32_t ARM_LPTIMER0_PowerControl (uint8_t channel, ARM_POWER_STATE state)
{
    return ARM_LPTIMER_PowerControl (&LPTIMER0, channel, state);
}

static int32_t ARM_LPTIMER0_Control (uint8_t channel, uint32_t control_code, void *arg)
{
    return ARM_LPTIMER_Control (&LPTIMER0, channel, control_code, arg);
}

static int32_t ARM_LPTIMER0_Start (uint8_t channel)
{
    return ARM_LPTIMER_Start (&LPTIMER0, channel);
}

static int32_t ARM_LPTIMER0_Stop (uint8_t channel)
{
    return ARM_LPTIMER_Stop (&LPTIMER0, channel);
}

static int32_t ARM_LPTIMER0_Uninitialize (uint8_t channel)
{
    return ARM_LPTIMER_Uninitialize (&LPTIMER0, channel);
}

/*LPTIMER Resources Control Block */
extern ARM_DRIVER_LPTIMER DRIVER_LPTIMER0;
ARM_DRIVER_LPTIMER  DRIVER_LPTIMER0 = {
    ARM_LPTIMER0_Initialize,
    ARM_LPTIMER0_PowerControl,
    ARM_LPTIMER0_Control,
    ARM_LPTIMER0_Start,
    ARM_LPTIMER0_Stop,
    ARM_LPTIMER0_Uninitialize
};
#endif /* RTE_LPTIMER */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
