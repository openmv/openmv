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
 * @file     Driver_DAC.c
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     22-Feb-2022
 * @brief    DAC(Digital to Analog Converter) driver definitions.DAC0 connected
 *           to P2_2 and DAC1 connected to P2_3.
 ******************************************************************************/

/* Project Includes */
#include "Driver_DAC_Private.h"
#include "analog_config.h"

#if !(RTE_DAC0 || RTE_DAC1)
#error "DAC is not enabled in the RTE_device.h"
#endif

#if (defined(RTE_Drivers_DAC0) && !RTE_DAC0)
#error "DAC0 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_DAC1) && !RTE_DAC1)
#error "DAC1 not configured in RTE_Device.h!"
#endif

#define ARM_DAC_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /*  Driver version */

/*Driver version*/
static const ARM_DRIVER_VERSION DriverVersion = {
        ARM_DAC_API_VERSION,
        ARM_DAC_DRV_VERSION
};

/*Driver Capabilities   */
static const ARM_DAC_CAPABILITIES DriverCapabilities = {
    1,/* 12 bit DAC resolution */
    0 /* Reserved ( must be ZERO) */
};

/**
 @fn           void analog_config(void)
 @brief        Analog configuration register includes Vbat and comparator
 @param[in]    none
 @return       none
 */
static void Analog_Config(void)
{
    /* Analog configuration Vbat register2 */
    analog_config_vbat_reg2();

    /* Analog configuration comparator register2 */
    analog_config_cmp_reg2();
}

/**
 @fn           ARM_DRIVER_VERSION DAC_GetVersion(void)
 @brief        get DAC version
 @param        none
 @return       driver version
 */
static ARM_DRIVER_VERSION DAC_GetVersion(void)
{
    return DriverVersion;
}

/**
 @fn           ARM_RTC_CAPABILITIES DAC_GetCapabilities(void)
 @brief        get DAC Capabilites
 @param        none
 @return       driver Capabilites
 */
static ARM_DAC_CAPABILITIES DAC_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
 @fn           int32_t DAC_Initialize(DAC_RESOURCES *DAC)
 @brief        Initialize the DAC
 @param[in]    DAC : Pointer to DAC resources
 @return       ARM_DRIVER_ERROR_PARAMETER : if dac parameter is invalid
               ARM_DRIVER_OK              : if dac successfully initialized
 */
static int32_t DAC_Initialize(DAC_RESOURCES *DAC)
{
    int32_t ret = ARM_DRIVER_OK;

    /* Setting the flag */
    DAC->flags.initialized = 0x1U;

    return ret;
}

/**
 @fn           int32_t DAC_Uninitialize(DAC_RESOURCES *DAC)
 @brief        Un-Initialize the DAC
 @param[in]    DAC  : Pointer to DAC resources
 @return       ARM_DRIVER_OK : if dac successfully initialized
 */
static int32_t DAC_Uninitialize(DAC_RESOURCES *DAC)
{
      int32_t ret = ARM_DRIVER_OK;

      /* Reset the flag */
      DAC->flags.initialized = 0x0U;

     return ret;
}

/**
 @fn           int32_t DAC_PowerControl(ARM_POWER_STATE state,
                                        DAC_RESOURCES *DAC)
 @brief        CMSIS-DRIVER DAC power control
 @param[in]    state : Power state
 @param[in]    DAC   : Pointer to DAC resources
 @return       ARM_DRIVER_ERROR_PARAMETER  : if initialize is not done
               ARM_DRIVER_OK               : if power done successful
 */
static int32_t DAC_PowerControl(ARM_POWER_STATE state,
                                DAC_RESOURCES *DAC)
{
    if(DAC->flags.initialized == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    switch((int32_t)state)
    {
          case ARM_POWER_OFF:

              /* If already powered OFF returns OK*/
              if(DAC->flags.powered == 0x0U)
              {
                 return ARM_DRIVER_OK;
              }

               /* Clear the DAC configuration */
               dac_clear_config(DAC->regs);

               disable_dac_periph_clk(DAC->instance);

               disable_cmp_periph_clk();

               DAC->flags.powered = 0x0U;
               break;

          case ARM_POWER_FULL:

              if(DAC->flags.powered == 0x1U)
              {
                 return ARM_DRIVER_OK;
              }

               enable_cmp_periph_clk();

               enable_dac_periph_clk(DAC->instance);

               /* Initialization for Analog configuration register */
               Analog_Config();

               /* DAC Disable */
               dac_disable(DAC->regs);

               /* DAC reset released */
               dac_reset_deassert(DAC->regs);

               /* Initialize DAC configuration */
               dac_set_config(DAC->regs, DAC->input_mux_val, DAC->dac_twoscomp_in);

               DAC->flags.powered = 0x1U;

              break;

          case ARM_POWER_LOW:
          default:
              return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
 @fn           int32_t DAC_Control(DAC_RESOURCES *DAC, uint32_t control, uint32_t arg)
 @brief        CMSIS-Driver dac control.
               Control DAC Interface.
 @param[in]    DAC     : Pointer to DAC resources
 @param[in]    control : Operation \ref Driver_DAC.h : DAC control codes
 @param[in]    arg     : Argument of operation (optional)
 @return       ARM_DRIVER_ERROR_PARAMETER  : if dac parameter is invalid
               ARM_DRIVER_OK               : if dac successfully initialized
 */
static int32_t DAC_Control(DAC_RESOURCES *DAC, uint32_t control, uint32_t arg)
{
    ARG_UNUSED(arg);
    int32_t ret = ARM_DRIVER_OK;

    if(DAC->flags.powered == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (control)
    {
        case ARM_DAC_RESET:

            if (arg)
            {
                /* DAC reset asserted */
                dac_reset_assert(DAC->regs);
            }
            else
            {
                /* DAC reset released */
                dac_reset_deassert(DAC->regs);
            }
            break;

        case ARM_DAC_INPUT_BYPASS_MODE:

            /* Set DAC input through bypass mode */
            dac_set_bypass_input(DAC->regs, arg);
            break;

        case ARM_DAC_SELECT_IBIAS_OUTPUT:

            /* Set DAC output current */
            dac_set_output_current(DAC->regs, arg);
            break;

        case ARM_DAC_CAPACITANCE_HP_MODE:

            /* Set capacitance for DAC signal */
            dac_set_capacitance(DAC->regs, arg);
            break;

        default:
            ret = ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
    }
    return ret;
}

/**
 * @fn         DAC_Start (DAC_RESOURCES *DAC)
 * @brief      CMSIS-Driver DAC Start
 *             Enable the DAC.
 @param[in]    DAC  : Pointer to DAC resources
 @return       ARM_DRIVER_OK  : if dac successfully uninitialized or already not initialized
 */
static int32_t DAC_Start (DAC_RESOURCES *DAC)
{
    int32_t ret = ARM_DRIVER_OK;

    if(DAC->flags.powered == 0x0U)
    {
         return ARM_DRIVER_ERROR;
    }

    if(DAC->flags.dac_drv_start == 0x1U)
    {
         return ARM_DRIVER_OK;
    }

    /* Enable the DAC */
    dac_enable(DAC->regs);

    DAC->flags.dac_drv_start = 0x1U;

    return ret;
}

/**
 *@fn          DAC_Stop (DAC_RESOURCES *DAC)
 *@brief       CMSIS-Driver DAC Stop
               Disable the DAC
 *@param[in]   DAC     : Pointer to DAC resources
 *@return      ARM_DRIVER_OK : if function return successfully
 */
static int32_t DAC_Stop (DAC_RESOURCES *DAC)
{
    int32_t ret = ARM_DRIVER_OK;

    if(DAC->flags.powered == 0x0U)
    {
         return ARM_DRIVER_ERROR;
    }

    /* Disable the DAC */
    dac_disable(DAC->regs);

    DAC->flags.dac_drv_start = 0x0U;

    return ret;
}

/**
 @fn           DAC_SetInput(DAC_RESOURCES *DAC, uint32_t value)
 @brief        CMSIS-Driver to set the DAC input.
 @param[in]    Input : Operation
 @param[in]    value  : DAC input
 @param[in]    DAC  : Pointer to dac resources
 @return       ARM_DRIVER_ERROR_PARAMETER : if parameter are invalid
               ARM_DRIVER_OK              : if the function return successful
 */
static int32_t DAC_SetInput(DAC_RESOURCES *DAC, uint32_t value)
{
    int32_t ret = ARM_DRIVER_OK;

    if(DAC->flags.dac_drv_start == 0x0U)
    {
         /* error:Driver is not started */
         return ARM_DRIVER_ERROR;
    }

    /* If bypass mode is not enabled then pass
       the input through the DAC_IN reg */
    if(!(dac_input_mux_enabled(DAC->regs)))
    {
        /* Set dac input */
        dac_input(DAC->regs, value);
    }

    return ret;
}

/* DAC0 driver instance */
#if(RTE_DAC0)

/* DAC configuration */
static DAC_RESOURCES DAC0 = {
    .regs            = (DAC_Type *)DAC120_BASE,
    .flags           = {0},
    .input_mux_val   = (RTE_DAC0_INPUT_BYP_MUX_EN),
    .dac_twoscomp_in = (RTE_DAC0_TWOSCOMP_EN),
    .instance        = DAC_INSTANCE_0
};

/* Function Name: DAC0_Initialize */
static int32_t DAC0_Initialize(void)
{
    return (DAC_Initialize(&DAC0));
}

/* Function Name: DAC0_uninitialize */
static int32_t DAC0_Uninitialize(void)
{
    return (DAC_Uninitialize(&DAC0));
}

/* Function Name: DAC0_PowerControl */
static int32_t DAC0_PowerControl(ARM_POWER_STATE state)
{
  return (DAC_PowerControl(state, &DAC0));
}

/* Function Name: DAC0_Control */
static int32_t DAC0_Control(uint32_t control, uint32_t arg)
{
    return (DAC_Control(&DAC0, control, arg));
}

/* Function Name: DAC0_Start */
static int32_t DAC0_Start(void)
{
    return (DAC_Start(&DAC0));
}

/* Function Name: DAC0_Stop */
static int32_t DAC0_Stop(void)
{
    return (DAC_Stop(&DAC0));
}

/* Function Name: DAC0_SetInput */
static int32_t DAC0_SetInput(uint32_t value)
{
    return (DAC_SetInput(&DAC0, value));
}

extern ARM_DRIVER_DAC Driver_DAC0;
ARM_DRIVER_DAC Driver_DAC0 =
{
    DAC_GetVersion,
    DAC_GetCapabilities,
    DAC0_Initialize,
    DAC0_Uninitialize,
    DAC0_PowerControl,
    DAC0_Control,
    DAC0_Start,
    DAC0_Stop,
    DAC0_SetInput
};

#endif /*RTE_DAC0 */

/* DAC1 driver instance */
#if(RTE_DAC1)

/* DAC1 configuration */
static DAC_RESOURCES DAC1 = {
    .regs            = (DAC_Type *)DAC121_BASE,
    .flags           = {0},
    .input_mux_val   = (RTE_DAC1_INPUT_BYP_MUX_EN),
    .dac_twoscomp_in = (RTE_DAC1_TWOSCOMP_EN),
    .instance        = DAC_INSTANCE_1
};
/* Function Name: DAC1_Initialize */
static int32_t DAC1_Initialize(void)
{
    return (DAC_Initialize(&DAC1));
}

/* Function Name: DAC1_uninitialize */
static int32_t DAC1_Uninitialize(void)
{
    return (DAC_Uninitialize(&DAC1));
}

/* Function Name: DAC1_PowerControl */
static int32_t DAC1_PowerControl(ARM_POWER_STATE state)
{
    return (DAC_PowerControl(state, &DAC1));
}

/* Function Name: DAC1_Control */
static int32_t DAC1_Control(uint32_t control, uint32_t arg)
{
    return (DAC_Control(&DAC1, control, arg));
}

/* Function Name: DAC1_Start */
static int32_t DAC1_Start(void)
{
    return (DAC_Start(&DAC1));
}

/* Function Name: DAC1_Stop */
static int32_t DAC1_Stop(void)
{
    return (DAC_Stop(&DAC1));
}

/* Function Name: DAC1_SetInput */
static int32_t DAC1_SetInput(uint32_t value)
{
    return (DAC_SetInput(&DAC1, value));
}

extern ARM_DRIVER_DAC Driver_DAC1;
ARM_DRIVER_DAC Driver_DAC1 =
{
    DAC_GetVersion,
    DAC_GetCapabilities,
    DAC1_Initialize,
    DAC1_Uninitialize,
    DAC1_PowerControl,
    DAC1_Control,
    DAC1_Start,
    DAC1_Stop,
    DAC1_SetInput
};

#endif /* RTE_DAC1 */
