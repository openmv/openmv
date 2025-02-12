/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef SYS_CTRL_CMP_H_
#define SYS_CTRL_CMP_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 * enum CMP_INSTANCE.
 * CMP instances.
 */
typedef enum _CMP_INSTANCE
{
    CMP_INSTANCE_0,                  /**< CMP instance - 0  */
    CMP_INSTANCE_1,                  /**< CMP instance - 1  */
    CMP_INSTANCE_2,                  /**< CMP instance - 2  */
    CMP_INSTANCE_3,                  /**< CMP instance - 3  */
    CMP_INSTANCE_LP                  /**< CMP instance - LP */
} CMP_INSTANCE;

/**
  \fn          static inline void enable_cmp_clk(uint8_t instance)
  \brief       Enable CMP input clock
  \param[in]   instance : Comparator instances
  \return      none
*/
static inline void enable_cmp_clk(uint8_t instance)
{
    switch(instance)
    {
    case CMP_INSTANCE_0:
        CLKCTL_PER_SLV->CMP_CTRL |= CMP_CTRL_CMP0_CLKEN;
        break;

    case CMP_INSTANCE_1:
        CLKCTL_PER_SLV->CMP_CTRL |= (CMP_CTRL_CMP1_CLKEN | CMP_CTRL_CMP0_CLKEN);
        break;

    case CMP_INSTANCE_2:
        CLKCTL_PER_SLV->CMP_CTRL |= (CMP_CTRL_CMP2_CLKEN | CMP_CTRL_CMP0_CLKEN);
        break;

    case CMP_INSTANCE_3:
        CLKCTL_PER_SLV->CMP_CTRL |= (CMP_CTRL_CMP3_CLKEN | CMP_CTRL_CMP0_CLKEN);
        break;

    case CMP_INSTANCE_LP:
        ANA_REG->VBAT_ANA_REG1 |= LPCMP_CTRL_CLKEN;
        break;
    }
}

/**
  \fn          static inline void disable_cmp_clk(uint8_t instance)
  \brief       Disable CMP input clock
  \param[in]   instance : Comparator instances
  \return      none
*/
static inline void disable_cmp_clk(uint8_t instance)
{
    switch(instance)
    {
    case CMP_INSTANCE_0:
        CLKCTL_PER_SLV->CMP_CTRL &= ~CMP_CTRL_CMP0_CLKEN;
        break;

    case CMP_INSTANCE_1:
        CLKCTL_PER_SLV->CMP_CTRL &= ~(CMP_CTRL_CMP1_CLKEN | CMP_CTRL_CMP0_CLKEN);
        break;

    case CMP_INSTANCE_2:
        CLKCTL_PER_SLV->CMP_CTRL &= ~(CMP_CTRL_CMP2_CLKEN | CMP_CTRL_CMP0_CLKEN);
        break;

    case CMP_INSTANCE_3:
        CLKCTL_PER_SLV->CMP_CTRL &= ~(CMP_CTRL_CMP3_CLKEN | CMP_CTRL_CMP0_CLKEN);
        break;

    case CMP_INSTANCE_LP:
        ANA_REG->VBAT_ANA_REG1 &= ~LPCMP_CTRL_CLKEN;
        break;
    }
}

/**
  \fn          static inline void enable_cmp(uint8_t instance)
  \brief       Enable the comparator module
  \param[in]   instance  : Comparator instances
  \return      None
 */
static inline void enable_cmp(uint8_t instance)
{
    /* comparator configuration register is provided on CMP0 base address */
    volatile uint32_t *cmp_reg = (volatile uint32_t *)CMP_CTRL_BASE;

    switch(instance)
    {
    case CMP_INSTANCE_0:
        /* Enable the CMP0 module */
        *cmp_reg |= CMP0_ENABLE;
        break;

    case CMP_INSTANCE_1:
        /* Enable the CMP1 module */
        *cmp_reg |= CMP1_ENABLE;
        break;

    case CMP_INSTANCE_2:
        /* Enable the CMP2 module */
        *cmp_reg |= CMP2_ENABLE;
        break;

    case CMP_INSTANCE_3:
        /* Enable the CMP3 module */
        *cmp_reg |= CMP3_ENABLE;
        break;

    case CMP_INSTANCE_LP:
        /* Enable the LPCMP module */
        ANA_REG->VBAT_ANA_REG2 |= LPCMP_ENABLE;
        break;
    }
}

/**
  \fn          static inline void disable_cmp(uint8_t instance)
  \brief       Disable the comparator module
  \param[in]   instance  : Comparator instances
  \return      None
 */
static inline void disable_cmp(uint8_t instance)
{
    /* comparator configuration register is provided on CMP0 base address */
    volatile uint32_t *cmp_reg = (volatile uint32_t *)CMP_CTRL_BASE;

    switch(instance)
    {
    case CMP_INSTANCE_0:
        /* Disable the CMP0 module */
        *cmp_reg &= ~CMP0_ENABLE;
        break;

    case CMP_INSTANCE_1:
        /* Disable the CMP1 module */
        *cmp_reg &= ~CMP1_ENABLE;
        break;

    case CMP_INSTANCE_2:
        /* Disable the CMP2 module */
        *cmp_reg &= ~CMP2_ENABLE;
        break;

    case CMP_INSTANCE_3:
        /* Disable the CMP3 module */
        *cmp_reg &= ~CMP3_ENABLE;
        break;

    case CMP_INSTANCE_LP:
        /* Disable the LPCMP module */
        ANA_REG->VBAT_ANA_REG2 &= ~LPCMP_ENABLE;
        break;
    }
}

/**
 * @fn          void cmp_set_config(uint8_t instance, uint32_t config_value)
 * @brief       Add configuration value to the reg1 of CMP0 instance which controls
 *              analog portion of all CMP instances.
 *              Add LPCMP configuration value to the Vbat reg2.
 * @param[in]   instance     : Comparator instances
 * @param[in]   config_value : To store the configuration values
 * @return      None
 */
static void cmp_set_config(uint8_t instance, uint32_t config_value)
{
    if(instance == CMP_INSTANCE_LP)
    {
        /* Add LPCMP configuration values in Vbat reg2 */
        ANA_REG->VBAT_ANA_REG2 |= config_value;
    }
    else
    {
        /* comparator configuration register is provided on CMP0 base address */
        volatile uint32_t *cmp_reg = (volatile uint32_t *)CMP_CTRL_BASE;

        /* Adding configuration values to the reg1 of Comp0 instance  */
        *cmp_reg |= config_value;
    }
}

/**
 * @fn          void lpcmp_clear_config(void)
 * @brief       Clear LPCMP configuration value in Vbat reg2.
 * @param[in]   None
 * @return      None
 */
static void lpcmp_clear_config(void)
{
    /* Clear LPCMP configuration values in Vbat reg2 */
    ANA_REG->VBAT_ANA_REG2 &= ~LPCMP_MSK_CTRL_VAL;
}

#endif /* SYS_CTRL_CMP_H_ */
