/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef DAC_H_
#define DAC_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/**
  * @brief DAC (DAC)
  */
typedef struct {                             /*!< DAC Structure                                          */
    volatile uint32_t  DAC_REG1;             /*!< REG1 DAC Control Register                              */
    volatile uint32_t  DAC_IN;               /*!< DAC Input Value Register                               */
} DAC_Type;                                  /*!< Size = 8 (0x8)                                         */

/* DAC  Control register */
#define DAC_EN                   (1U << 0)   /* Enable DAC                                               */
#define DAC_RESET_B              (1U << 27)  /* 0=Reset,this will reset the DAC                          */
#define DAC_HP_MODE_EN           (1U << 18)  /* To enable the dac output buffer                          */
#define DAC_MAX_INPUT            (0xFFFU)    /* Maximum input for the DAC is 4095(DAC 12 bit resolution) */
#define DAC_IN_BYP_MUX           (1U << 1U)  /* Select the DAC input data source                         */
#define DAC_MAX_BYP_VAL_Msk      (0x3FFCU)   /* DAC input data in bypass mode                            */
#define DAC_TWOSCOMP_Pos          22U        /* Converts two's complement to unsigned binary data        */
#define DAC_INPUT_BYP_MUX_Pos     1U         /* Set DAC input source in bypass mode                      */
#define DAC_BYP_VAL_Pos           2U         /* DAC input data bypass mode                               */
#define DAC_IBIAS_VAL_Pos         23U        /* DAC buffer output current                                */
#define DAC_CAP_VAL_Pos           14U        /* DAC capacitance compensation                             */

/**
 @fn           void dac_enable(DAC_Type *dac)
 @brief        Enable the DAC.
 @param[in]    dac : Pointer to dac Type
 @return       none
 */
static inline void dac_enable(DAC_Type *dac)
{
    /* Enable the DAC */
    dac->DAC_REG1 |= (DAC_EN);
}

/**
 @fn           void dac_disable(DAC_Type *dac)
 @brief        Disable the DAC.
 @param[in]    dac : Pointer to dac Type
 @return       none
 */
static inline void dac_disable(DAC_Type *dac)
{
    /* Disable the DAC */
    dac->DAC_REG1 &= ~(DAC_EN);
}

/**
 @fn           void dac_set_config(DAC_Type *dac, uint8_t input_mux_val, uint16_t bypass_val)
 @brief        Configure the DAC
 @param[in]    input_mux_val : To select the Dac input data source
               conv_input    : Converts two's complement to unsigned binary data
 @param[in]    dac   : Pointer to dac Type
 @return       none
 */
static inline void dac_set_config(DAC_Type *dac, uint8_t input_mux_val, uint8_t conv_input)
{
    /* Set dac input bypass mux and conversion to unsigned binary data */
    dac->DAC_REG1 |= ((input_mux_val << DAC_INPUT_BYP_MUX_Pos) | (conv_input << DAC_TWOSCOMP_Pos));
}

/**
 @fn           void dac_clear_config (DAC_Type *dac)
 @brief        Clear the DAC configuration.
 @param[in]    dac : Pointer to dac Type
 @return       none
 */
static inline void dac_clear_config(DAC_Type *dac)
{
    /* Clear the DAC configuration */
    dac->DAC_REG1 = 0U;
}

/**
 @fn           dac_hp_mode_enable(DAC_Type *dac)
 @brief        Enable HP mode of DAC.
 @param[in]    dac : Pointer to dac Type
 @return       none
 */
static inline void dac_hp_mode_enable(DAC_Type *dac)
{
    /* To enable the output buffer of DAC */
    dac->DAC_REG1 |= DAC_HP_MODE_EN;
}

/**
 @fn           void dac_reset_deassert(DAC_Type *dac)
 @brief        DAC reset released.
 @param[in]    dac : Pointer to dac Type
 @return       none
 */
static inline void dac_reset_deassert(DAC_Type *dac)
{
    /* DAC reset released */
    dac->DAC_REG1 |= (DAC_RESET_B);
}

/**
 @fn           void dac_reset_assert(DAC_Type *dac)
 @brief        DAC reset asserted.
 @param[in]    dac : Pointer to dac Type
 @return       none
 */
static inline void dac_reset_assert(DAC_Type *dac)
{
    /* DAC reset asserted */
    dac->DAC_REG1 &= ~(DAC_RESET_B);
}

/**
 @fn           void dac_input(DAC_Type *dac, uint32_t value)
 @brief        Set the DAC input.
 @param[in]    value : DAC input
 @param[in]    dac   : Pointer to dac Type
 @return       none
 */
static inline void dac_input(DAC_Type *dac, uint32_t value)
{
    /* set the DAC input */
    dac->DAC_IN = value;
}

/**
 @fn           void dac_set_output_current(DAC_Type *dac, uint8_t ibias_val)
 @brief        Set the DAC buffer output current.
 @param[in]    ibias_val : DAC output current value
 @param[in]    dac   : Pointer to dac Type
 @return       none
 */
static inline void dac_set_output_current(DAC_Type *dac, uint8_t ibias_val)
{
    /* Set DAC output current */
    dac->DAC_REG1 |= (ibias_val << DAC_IBIAS_VAL_Pos);
}

/**
 @fn           void dac_set_bypass_input(DAC_Type *dac, uint32_t bypass_val)
 @brief        Set the DAC input through the bypass mode.
 @param[in]    bypass_val : DAC bypass input
 @param[in]    dac   : Pointer to dac Type
 @return       none
 */
static inline void dac_set_bypass_input(DAC_Type *dac, uint32_t bypass_val)
{
    /* Clear the DAC bypass value */
    dac->DAC_REG1 &= ~(DAC_MAX_BYP_VAL_Msk);

    bypass_val = (bypass_val & DAC_MAX_INPUT);

    /* Set the DAC bypass input value */
    dac->DAC_REG1 |= (bypass_val << DAC_BYP_VAL_Pos);
}

/**
 @fn           void dac_set_capacitance(DAC_Type *dac, uint8_t cap_val)
 @brief        Set the DAC capacitance.
 @param[in]    value : DAC capacitance
 @param[in]    dac   : Pointer to dac Type
 @return       none
 */
static inline void dac_set_capacitance(DAC_Type *dac, uint8_t cap_val)
{
    /* Set DAC capacitance value */
    dac->DAC_REG1 |= (cap_val << DAC_CAP_VAL_Pos);
}

/**
 @fn           dac_input_mux_enabled(DAC_Type *dac)
 @brief        Check DAC input bypass mux is enable or disable
 @param[in]    dac   : Pointer to dac Type
 @return       DAC input bypass mux is enable or disable
 */
static inline bool dac_input_mux_enabled(DAC_Type *dac)
{
    return (dac->DAC_REG1 & DAC_IN_BYP_MUX) ? true : false;
}

#endif /* DAC_H */
