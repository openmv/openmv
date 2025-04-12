/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef WDT_H_
#define WDT_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {                                    /*!< (@ 0x40100000) WDT_HP_CTRL Structure                                      */
    volatile uint32_t       WDT_WDOGLOAD;           /*!< (@ 0x00000000) Watchdog Load Register                                     */
    const volatile uint32_t WDT_WDOGVALUE;          /*!< (@ 0x00000004) Watchdog Value Register                                    */
    volatile uint32_t       WDT_WDOGCONTROL;        /*!< (@ 0x00000008) Watchdog Control Register                                  */
    volatile  uint32_t      WDT_WDOGINTCLR;         /*!< (@ 0x0000000C) Watchdog Clear Interrupt Register                          */
    const volatile uint32_t WDT_WDOGRIS;            /*!< (@ 0x00000010) Watchdog Raw Interrupt Status Register                     */
    const volatile uint32_t WDT_WDOGMIS;            /*!< (@ 0x00000014) Watchdog Interrupt Status Register                         */
    const volatile uint32_t RESERVED[762];
    volatile uint32_t       WDT_WDOGLOCK;           /*!< (@ 0x00000C00) Watchdog Lock Register                                     */
    const volatile uint32_t RESERVED1[243];
    const volatile uint32_t WDT_WDOGPERIPHID4;      /*!< (@ 0x00000FD0) Reserved                                                   */
    const volatile uint32_t RESERVED2[3];
    const volatile uint32_t WDT_WDOGPERIPHID0;      /*!< (@ 0x00000FE0) Reserved                                                   */
    const volatile uint32_t WDT_WDOGPERIPHID1;      /*!< (@ 0x00000FE4) Reserved                                                   */
    const volatile uint32_t WDT_WDOGPERIPHID2;      /*!< (@ 0x00000FE8) Reserved                                                   */
    const volatile uint32_t WDT_WDOGPERIPHID3;      /*!< (@ 0x00000FEC) Reserved                                                   */
    const volatile uint32_t WDT_WDOGPCELLID0;       /*!< (@ 0x00000FF0) Reserved                                                   */
    const volatile uint32_t WDT_WDOGPCELLID1;       /*!< (@ 0x00000FF4) Reserved                                                   */
    const volatile uint32_t WDT_WDOGPCELLID2;       /*!< (@ 0x00000FF8) Reserved                                                   */
    const volatile uint32_t WDT_WDOGPCELLID3;       /*!< (@ 0x00000FFC) Reserved                                                   */
} WDT_CTRL_Type;

#define WATCHDOG_CTRL_INTEN        (0x1UL << 0)
#define WATCHDOG_CTRL_RESEN        (0x1UL << 1)
#define WATCHDOG_INTCLR            1UL
#define WATCHDOG_RAWINTSTAT        1UL
#define WATCHDOG_MASKINTSTAT       1UL
#define WATCHDOG_UNLOCK_VALUE      0x1ACCE551UL
#define WATCHDOG_LOCK_VALUE        0x00000001UL
#define WATCHDOG_INTEGTESTEN       1UL
#define WATCHDOG_INTEGTESTOUTSET   1UL
#define WATCHDOG_MAX_VALUE         0xFFFFFFFFUL

/**
  \fn          static inline void wdt_lock(WDT_CTRL_Type *wdt)
  \brief       Lock watchdog registers.
  \param[in]   wdt     Pointer to the watchdog register map
  \return      none
*/
static inline void wdt_lock(WDT_CTRL_Type *wdt)
{
    wdt->WDT_WDOGLOCK = WATCHDOG_LOCK_VALUE;
}

/**
  \fn          static inline void wdt_unlock(WDT_CTRL_Type *wdt)
  \brief       Unlock watchdog registers.
  \param[in]   wdt     Pointer to the watchdog register map
  \return      none
*/
static inline void wdt_unlock(WDT_CTRL_Type *wdt)
{
    wdt->WDT_WDOGLOCK = WATCHDOG_UNLOCK_VALUE;
}

/**
  \fn          static inline bool wdt_locked(WDT_CTRL_Type *wdt)
  \brief       Check the lock status of the watchdog registers.
  \param[in]   wdt     Pointer to the watchdog register map
  \return      Locked/Unlocked status
*/
static inline bool wdt_locked(WDT_CTRL_Type *wdt)
{
    return wdt->WDT_WDOGLOCK ? true : false;
}

/**
  \fn          static inline void wdt_start(WDT_CTRL_Type *wdt, uint32_t timeout)
  \brief       Start the watchdog.
  \param[in]   wdt     Pointer to the watchdog register map
  \param[in]   timeout Watchdog timeout value
  \return      none
*/
static inline void wdt_start(WDT_CTRL_Type *wdt, uint32_t timeout)
{
    wdt->WDT_WDOGLOAD = timeout;
    wdt->WDT_WDOGCONTROL = (WATCHDOG_CTRL_RESEN | WATCHDOG_CTRL_INTEN);
}

/**
  \fn          static inline void wdt_stop(WDT_CTRL_Type *wdt)
  \brief       Stop the watchdog.
  \param[in]   wdt     Pointer to the watchdog register map
  \return      none
*/
static inline void wdt_stop(WDT_CTRL_Type *wdt)
{
    wdt->WDT_WDOGCONTROL &= ~(WATCHDOG_CTRL_RESEN | WATCHDOG_CTRL_INTEN);
}

/**
  \fn          static inline void wdt_feed(WDT_CTRL_Type *wdt, uint32_t timeout)
  \brief       Feed/Kick the watchdog.
  \param[in]   wdt     Pointer to the watchdog register map
  \param[in]   timeout Watchdog timeout value
  \return      none
*/
static inline void wdt_feed(WDT_CTRL_Type *wdt, uint32_t timeout)
{
    wdt->WDT_WDOGLOAD = timeout;
    wdt->WDT_WDOGINTCLR = WATCHDOG_INTCLR;
}

/**
  \fn          static inline uint32_t wdt_get_remaining_ticks(WDT_CTRL_Type *wdt)
  \brief       Get the remaining ticks until a timeout.
  \param[in]   wdt     Pointer to the watchdog register map
  \return      remaining ticks
*/
static inline uint32_t wdt_get_remaining_ticks(WDT_CTRL_Type *wdt)
{
    return wdt->WDT_WDOGVALUE;
}

#ifdef  __cplusplus
}
#endif

#endif /* WDT__H_ */
