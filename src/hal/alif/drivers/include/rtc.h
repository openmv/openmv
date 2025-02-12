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
 * @file     rtc.h
 * @author   Tanay Rami, Manoj A Murudi
 * @email    tanay@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     23-March-2023
 * @brief    Device Specific Low Level Header file for RTC Driver.
 ******************************************************************************/

#ifndef RTC_H_
#define RTC_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef struct {
    volatile const uint32_t LPRTC_CCVR;           /*!< (@ 0x00000000) Current Counter Value Register                             */
    volatile uint32_t       LPRTC_CMR;            /*!< (@ 0x00000004) Counter Match Register                                     */
    volatile uint32_t       LPRTC_CLR;            /*!< (@ 0x00000008) Counter Load Register                                      */
    volatile uint32_t       LPRTC_CCR;            /*!< (@ 0x0000000C) Counter Control Register                                   */
    volatile const uint32_t LPRTC_STAT;           /*!< (@ 0x00000010) Interrupt Status Register                                  */
    volatile const uint32_t LPRTC_RSTAT;          /*!< (@ 0x00000014) Interrupt Raw Status Register                              */
    volatile const uint32_t LPRTC_EOI;            /*!< (@ 0x00000018) End-of-Interrupt Register                                  */
    volatile const uint32_t LPRTC_COMP_VERSION;   /*!< (@ 0x0000001C) Component Version Register                                 */
    volatile uint32_t       LPRTC_CPSR;           /*!< (@ 0x00000020) Counter Prescaler Register                                 */
    volatile const uint32_t LPRTC_CPCVR;          /*!< (@ 0x00000024) Current Prescaler Counter Value Register                   */
} LPRTC_Type;                                     /*!< Size = 40 (0x28)                                                          */

/* CCR register fields */
#define CCR_LPRTC_PSCLR_EN        (1U << 4U)       /* Enable prescaler  */
#define CCR_LPRTC_WEN             (1U << 3U)       /* Wrap enable       */
#define CCR_LPRTC_EN              (1U << 2U)       /* Enable counter    */
#define CCR_LPRTC_MASK            (1U << 1U)       /* Mask interrupts   */
#define CCR_LPRTC_IEN             (1U << 0U)       /* Enable interrupts */

/**
  \fn           static inline void lprtc_counter_enable (LPRTC_Type *lprtc)
  \brief        Enable lprtc counter
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_counter_enable (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR |= (CCR_LPRTC_EN);
}

/**
  \fn           static inline void lprtc_counter_disable (LPRTC_Type *lprtc)
  \brief        Disable lprtc counter
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_counter_disable (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR &= ~(CCR_LPRTC_EN);
}

/**
  \fn           static inline void lprtc_prescaler_enable (LPRTC_Type *lprtc)
  \brief        Enable lprtc prescaler counter
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_prescaler_enable (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR |= (CCR_LPRTC_PSCLR_EN);
}

/**
  \fn           static inline void lprtc_prescaler_disable (LPRTC_Type *lprtc)
  \brief        Disable lprtc prescaler counter
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_prescaler_disable (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR &= ~(CCR_LPRTC_PSCLR_EN);
}

/**
  \fn           static inline void lprtc_counter_wrap_enable (LPRTC_Type *lprtc)
  \brief        Enable lprtc counter wrap
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_counter_wrap_enable (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR |= (CCR_LPRTC_WEN);
}

/**
  \fn           static inline void lprtc_counter_wrap_disable (LPRTC_Type *lprtc)
  \brief        Disable lprtc counter wrap
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_counter_wrap_disable (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR &= ~(CCR_LPRTC_WEN);
}

/**
  \fn           static inline void lprtc_interrupt_enable (LPRTC_Type *lprtc)
  \brief        Enable lprtc interrupt generation
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_interrupt_enable (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR |= (CCR_LPRTC_IEN);
}

/**
  \fn           static inline void lprtc_interrupt_control_disable (LPRTC_Type *lprtc)
  \brief        Disable lprtc interrupt generation
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_interrupt_disable (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR &= ~(CCR_LPRTC_IEN);
}

/**
  \fn           static inline void lprtc_interrupt_mask (LPRTC_Type *lprtc)
  \brief        Mask lprtc interrupt generation
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_interrupt_mask (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR |= (CCR_LPRTC_MASK);
}

/**
  \fn           static inline void lprtc_interrupt_unmask (LPRTC_Type *lprtc)
  \brief        Unmask lprtc interrupt generation
  \param[in]    lprtc  : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_interrupt_unmask (LPRTC_Type *lprtc)
{
    lprtc->LPRTC_CCR &= ~(CCR_LPRTC_MASK);
}

/**
  \fn           static inline void lprtc_interrupt_ack (LPRTC_Type *lprtc)
  \brief        Acknowledge lprtc interrupt
  \param[in]    lprtc   : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_interrupt_ack (LPRTC_Type *lprtc)
{
    /* read to clear match interrupt. */
    (void) (lprtc->LPRTC_EOI);
}

/**
  \fn           static inline void lprtc_load_prescaler (LPRTC_Type *lprtc, uint32_t value)
  \brief        Load lprtc prescaler value
  \param[in]    value        : lprtc prescaler value
  \param[in]    lplprtc      : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_load_prescaler (LPRTC_Type *lprtc, uint32_t value)
{
    lprtc->LPRTC_CPSR = value;
}

/**
  \fn           static inline void lprtc_load_count (LPRTC_Type *lprtc, uint32_t value)
  \brief        Load lprtc counter value
  \param[in]    value     : lprtc prescaler value
  \param[in]    lplprtc   : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_load_count (LPRTC_Type *lprtc, uint32_t value)
{
    lprtc->LPRTC_CLR = value;
}

/**
  \fn           static inline void lprtc_load_counter_match_register (LPRTC_Type *lprtc, uint32_t value)
  \brief        Interrupt match register,When the internal counter matches this register,
                an interrupt is generated, provided interrupt generation is enabled.
  \param[in]    value : lprtc counter match register value
  \param[in]    lprtc : Pointer to lprtc register block
  \return       none
*/
static inline void lprtc_load_counter_match_register (LPRTC_Type *lprtc, uint32_t value)
{
    lprtc->LPRTC_CMR = value;
}

/**
  \fn           static inline uint32_t lprtc_get_count (LPRTC_Type *lprtc)
  \brief        Read lprtc current counter value
  \param[in]    lprtc   : Pointer to lprtc register block
  \return       lprtc current counter value
*/
static inline uint32_t lprtc_get_count (LPRTC_Type *lprtc)
{
    return lprtc->LPRTC_CCVR;
}

#ifdef  __cplusplus
}
#endif

#endif /* RTC_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
