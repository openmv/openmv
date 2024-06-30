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
 * @file     lptimer.h
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     27-March-2023
 * @brief    Low Level header file for LPTIMER.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef LPTIMER_H_
#define LPTIMER_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* LPTIMER Control Register bit Definition */
#define LPTIMER_CONTROL_REG_TIMER_ENABLE_BIT                0x01U
#define LPTIMER_CONTROL_REG_TIMER_MODE_BIT                  0x02U
#define LPTIMER_CONTROL_REG_TIMER_INTERRUPT_MASK_BIT        0x04U
#define LPTIMER_CONTROL_REG_TIMER_PWM_BIT                   0x08U
#define LPTIMER_CONTROL_REG_TIMER_ON_100PWM_BIT             0x10U

/**
  * @brief LPTIMER_LPTIMER_CHANNEL_CFG [LPTIMER_CHANNEL_CFG]
  */
typedef struct {
    volatile        uint32_t  LPTIMER_LOADCOUNT;                             /*!< (@ 0x00000000) Timer (n) Load Count Register         */
    volatile const  uint32_t  LPTIMER_CURRENTVAL;                            /*!< (@ 0x00000004) Timer (n) Current Value Register      */
    volatile        uint32_t  LPTIMER_CONTROLREG;                            /*!< (@ 0x00000008) Timer (n) Control Register            */
    volatile const  uint32_t  LPTIMER_EOI;                                   /*!< (@ 0x0000000C) Timer (n) End-of-Interrupt Register   */
    volatile const  uint32_t  LPTIMER_INTSTAT;                               /*!< (@ 0x00000010) Timer (n) Interrupt Status Register   */
} LPTIMER_LPTIMER_CHANNEL_CFG_Type;                                          /*!< Size = 20 (0x14)                                     */

/**
  * @brief LPTIMER (LPTIMER)
  */
typedef struct {
    volatile        LPTIMER_LPTIMER_CHANNEL_CFG_Type LPTIMER_CHANNEL_CFG[4]; /*!< (@ 0x00000000) [0..3]                                */
    volatile const  uint32_t  RESERVED[20];
    volatile const  uint32_t  LPTIMERS_INTSTATUS;                            /*!< (@ 0x000000A0) Timers Interrupt Status Register      */
    volatile const  uint32_t  LPTIMERS_EOI;                                  /*!< (@ 0x000000A4) Timers End-of-Interrupt Register      */
    volatile const  uint32_t  LPTIMERS_RAWINTSTATUS;                         /*!< (@ 0x000000A8) Timers Raw Interrupt Status Register  */
    volatile const  uint32_t  LPTIMERS_COMP_VERSION;                         /*!< (@ 0x000000AC) Reserved                              */
} LPTIMER_Type;                                                              /*!< Size = 176 (0xb0)                                    */

/**
  \fn          static inline void lptimer_set_mode_userdefined (LPTIMER_Type *lptimer, uint8_t channel)
  \brief       Set user-defined mode for specified LPTIMER channel
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      none
*/
static inline void lptimer_set_mode_userdefined (LPTIMER_Type *lptimer, uint8_t channel)
{
    lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_CONTROLREG |=  LPTIMER_CONTROL_REG_TIMER_MODE_BIT;
}

/**
  \fn          static inline void lptimer_set_mode_freerunning (LPTIMER_Type *lptimer, uint8_t channel)
  \brief       Set free run mode for specified LPTIMER channel
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      none
*/
static inline void lptimer_set_mode_freerunning (LPTIMER_Type *lptimer, uint8_t channel)
{
    lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_CONTROLREG &= ~LPTIMER_CONTROL_REG_TIMER_MODE_BIT;
}

/**
  \fn          static inline void lptimer_load_count (LPTIMER_Type *lptimer, uint8_t channel, uint32_t value)
  \brief       Load counter value
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \param[in]   value     Pointer to variable which stores value to be assigned to counter
  \return      none
*/
static inline void lptimer_load_count (LPTIMER_Type *lptimer, uint8_t channel, uint32_t *value)
{
    lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_LOADCOUNT = *value;
}

/**
  \fn          static inline void lptimer_load_max_count (LPTIMER_Type *lptimer, uint8_t channel)
  \brief       Load maximum counter value
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      none
*/
static inline void lptimer_load_max_count (LPTIMER_Type *lptimer, uint8_t channel)
{
    lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_LOADCOUNT = 0xFFFFFFFF;
}

/**
  \fn          static inline uint32_t lptimer_get_count (LPTIMER_Type *lptimer, uint8_t channel, uint32_t *value)
  \brief       Get current counter value
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      counter value
*/
static inline uint32_t lptimer_get_count (LPTIMER_Type *lptimer, uint8_t channel)
{
    return lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_CURRENTVAL;
}

/**
  \fn          static inline void lptimer_enable_counter (LPTIMER_Type *lptimer, uint8_t channel)
  \brief       Enable timer
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      none
*/
static inline void lptimer_enable_counter (LPTIMER_Type *lptimer, uint8_t channel)
{
    lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_CONTROLREG |= LPTIMER_CONTROL_REG_TIMER_ENABLE_BIT;
}

/**
  \fn          static inline void lptimer_disable_counter (LPTIMER_Type *lptimer, uint8_t channel)
  \brief       Disable timer
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      none
*/
static inline void lptimer_disable_counter (LPTIMER_Type *lptimer, uint8_t channel)
{
    lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_CONTROLREG &= ~LPTIMER_CONTROL_REG_TIMER_ENABLE_BIT;
}

/**
  \fn          static inline void lptimer_clear_interrupt (LPTIMER_Type *lptimer, uint8_t channel)
  \brief       Clear pending cahnnel interrupt
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      none
*/
static inline void lptimer_clear_interrupt (LPTIMER_Type *lptimer, uint8_t channel)
{
    (void) (lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_EOI);
}

/**
  \fn          static inline void lptimer_mask_interrupt (LPTIMER_Type *lptimer, uint8_t channel)
  \brief       Mask channel interrupt
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      none
*/
static inline void lptimer_mask_interrupt (LPTIMER_Type *lptimer, uint8_t channel)
{
    lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_CONTROLREG |= LPTIMER_CONTROL_REG_TIMER_INTERRUPT_MASK_BIT;
}

/**
  \fn          static inline void lptimer_unmask_interrupt (LPTIMER_Type *lptimer, uint8_t channel)
  \brief       Unmask channel interrupt
  \param[in]   lptimer   Pointer to the LPTIMER register map
  \param[in]   channel   lptimer channel
  \return      none
*/
static inline void lptimer_unmask_interrupt (LPTIMER_Type *lptimer, uint8_t channel)
{
    lptimer->LPTIMER_CHANNEL_CFG[channel].LPTIMER_CONTROLREG &= ~LPTIMER_CONTROL_REG_TIMER_INTERRUPT_MASK_BIT;
}

#ifdef __cplusplus
}
#endif

#endif /* LPTIMER_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
