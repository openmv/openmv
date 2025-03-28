/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef CMP_H_
#define CMP_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 @brief struct CMP_Type:- Register map for Analog Comparator
 */
typedef struct {                                     /*!< CMP Structure                              */
    volatile uint32_t  CMP_COMP_REG1;                /*!< Comparator Register 1                      */
    volatile uint32_t  CMP_COMP_REG2;                /*!< Comparator Register 2                      */
    volatile uint32_t  CMP_POLARITY_CTRL;            /*!< CMP Polarity Control Register              */
    volatile uint32_t  CMP_WINDOW_CTRL;              /*!< CMP Window Control Register                */
    volatile uint32_t  CMP_FILTER_CTRL;              /*!< CMP Filter Control Register                */
    volatile uint32_t  CMP_PRESCALER_CTRL;           /*!< CMP Prescaler Control Register             */
    volatile const  uint32_t  RESERVED[2];
    volatile uint32_t  CMP_INTERRUPT_STATUS;         /*!< CMP Interrupt Status and Clear Register    */
    volatile uint32_t  CMP_INTERRUPT_MASK;           /*!< CMP Interrupt Mask Register                */
}CMP_Type;                                          /*!< Size = 40 (0x28)                           */

#define CMP0_ENABLE                    (1U << 28)    /* To enable the Comp0                          */
#define CMP1_ENABLE                    (1U << 29)    /* To enable the Comp1                          */
#define CMP2_ENABLE                    (1U << 30)    /* To enable the Comp2                          */
#define CMP3_ENABLE                    (1U << 31)    /* To enable the Comp3                          */
#define LPCMP_ENABLE                   (1U << 24)    /* To enable the LPCMP                          */

#define CMP_FILTER_CONTROL_ENABLE      (1U << 0)     /* To enable the filter control                 */
#define CMP_WINDOW_CONTROL_ENABLE      (3U << 0)     /* To enable the window control                 */
#define CMP_PRESCALER_MAX_VALUE        (0x3FU)       /* Maximum value of prescaler control           */
#define CMP_POLARITY_MAX_VALUE         (0x2U)        /* Maximum value of polarity control            */
#define CMP_WINDOW_MAX_VALUE           (0x3U)        /* Maximum value of window control              */
#define CMP_FILTER_MIN_VALUE           (0x2U)        /* Minimum value of filter control              */
#define CMP_FILTER_MAX_VALUE           (0x8U)        /* Maximum value of filter control              */

#define CMP_INT_MASK                   (0x01UL)      /* Mask for the interrupt                       */
#define CMP_INTERRUPT_CLEAR            (0x01UL)      /* To clear the interrupt                       */

#define LPCMP_MSK_CTRL_VAL             (0xFEU << 24) /* Mask all LPCMP configuration value           */

/**
  @fn          void cmp_enable_interrupt(CMP_Type *cmp)
  @brief       Enable the interrupts for the CMP instance.
  @param[in]   cmp     Pointer to the CMP register map
  @return      none
*/
static inline void cmp_enable_interrupt(CMP_Type *cmp)
{
    cmp->CMP_INTERRUPT_MASK = 0;
}

/**
  @fn          void cmp_disable_interrupt(CMP_Type *cmp)
  @brief       Disable the interrupts for the CMP instance.
  @param[in]   cmp     Pointer to the CMP register map
  @return      none
*/
static inline void cmp_disable_interrupt(CMP_Type *cmp)
{
    cmp->CMP_INTERRUPT_MASK = CMP_INT_MASK;
}

/**
  @fn          void cmp_get_interrupt_status(CMP_Type *cmp)
  @brief       Get the interrupt status of CMP instance.
  @param[in]   cmp     Pointer to the CMP register map
  @return      Interrupt status
*/
static inline uint32_t cmp_get_interrupt_status(CMP_Type *cmp)
{
    return cmp->CMP_INTERRUPT_STATUS;
}

/**
  @fn          void cmp_clear_interrupt(CMP_Type *cmp)
  @brief       Clear the interrupts for the CMP instance.
  @param[in]   cmp     Pointer to the CMP register map
  @return      none
*/
static inline void cmp_clear_interrupt(CMP_Type *cmp)
{
    cmp->CMP_INTERRUPT_STATUS = CMP_INTERRUPT_CLEAR;
}

/**
  @fn          void cmp_clear_config(CMP_Type *cmp)
  @brief       Clear the CMP configuration.
  @param[in]   cmp     Pointer to the CMP register map
  @return      none
*/
static inline void cmp_clear_config(CMP_Type *cmp)
{
    cmp->CMP_COMP_REG1 = 0U;
}

/**
  @fn          void cmp_set_polarity_ctrl(CMP_Type *cmp, uint32_t arg)
  @brief       If polarity control is Enable, invert result of the CMP.
  @param[in]   cmp     Pointer to the CMP register map
  @param[in]   arg     ENABLE/DISABLE the polarity control bit.
  @return      none
*/
static inline void cmp_set_polarity_ctrl(CMP_Type *cmp, uint32_t arg)
{
    cmp->CMP_POLARITY_CTRL = arg;
}

/**
  @fn          void cmp_set_window_ctrl(CMP_Type *cmp, uint32_t arg)
  @brief       Select one of the 4 inputs which will control the windowing
               (gating) function.
  @param[in]   cmp    Pointer to the CMP register map
  @param[in]   arg    4 input events to control the processing window.
  @return      none
*/
static inline void cmp_set_window_ctrl(CMP_Type *cmp, uint32_t arg)
{
    cmp->CMP_WINDOW_CTRL = (CMP_WINDOW_CONTROL_ENABLE | arg << 8);
}

/**
  @fn          void cmp_clear_window_ctrl(CMP_Type *cmp, uint32_t arg)
  @brief       Clear the windowing inputs to the comparator
  @param[in]   cmp    Pointer to the CMP register map
  @param[in]   arg    4 input events to control the processing window.
  @return      none
*/
static inline void cmp_clear_window_ctrl(CMP_Type *cmp, uint32_t arg)
{
    cmp->CMP_WINDOW_CTRL &= ~(CMP_WINDOW_CONTROL_ENABLE | arg << 8);
}

/**
  @fn          void cmp_set_filter_ctrl(CMP_Type *cmp, uint32_t arg)
  @brief       CMP Filter sets the filter taps.
  @param[in]   cmp  Pointer to the CMP register map
  @param[in]   arg  Number of filter taps, 2–8 taps.
  @return      none
*/
static inline void cmp_set_filter_ctrl(CMP_Type *cmp, uint32_t arg)
{
    cmp->CMP_FILTER_CTRL = (CMP_FILTER_CONTROL_ENABLE | arg << 8);
}

/**
  @fn          void cmp_set_prescaler_ctrl(CMP_Type *cmp, uint32_t arg)
  @brief       CMP prescaler sets the clocking rate for filtering
  @param[in]   cmp  Pointer to the CMP register map
  @param[in]   arg  Number of clocks between updating taps with new comparator input.
  @return      none
*/
static inline void cmp_set_prescaler_ctrl(CMP_Type *cmp, uint32_t arg)
{
    cmp->CMP_PRESCALER_CTRL = arg;
}

/**
  @fn          void cmp_irq_handler(CMP_Type *cmp) ;
  @brief       Handle interrupts for the CMP instance.
  @param[in]   cmp     Pointer to the CMP register map
  @return      none
*/
void cmp_irq_handler(CMP_Type *cmp);

#ifdef __cplusplus
}
#endif

#endif /* CMP_H_ */
