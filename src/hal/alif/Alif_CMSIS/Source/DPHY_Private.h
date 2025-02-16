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
 * @file     DPHY_Private.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     24-Feb-2022
 * @brief    Driver Specific Header file for DPHY Driver.
 ******************************************************************************/

#ifndef DPHY_PRIVATE_H_
#define DPHY_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/*Helper macro*/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * enum DPHY_STOPSTATE
 * DPHY physical lanes stop state status
 */
typedef enum _DPHY_STOPSTATE {
	DPHY_STOPSTATE_LANE0 = (1U << 0),   /**< DPHY lane 0 stopstate status     */
	DPHY_STOPSTATE_LANE1 = (1U << 1),   /**< DPHY lane 1 stopstate status     */
	DPHY_STOPSTATE_CLOCK = (1U << 2),   /**< DPHY lane clock stopstate status */
} DPHY_STOPSTATE;

/**
 * enum DPHY_PLL_STATUS
 * DPHY PLL lock status
 */
typedef enum _DPHY_PLL_STATUS {
	DPHY_PLL_STATUS_NO_PLL_LOCK,        /**< DPHY status locked      */
	DPHY_PLL_STATUS_PLL_LOCK,           /**< DPHY status not locked  */
} DPHY_PLL_STATUS;

/**
 * enum DPHY_INIT_STATUS
 * DPHY initialization status
 */
typedef enum _DPHY_INIT_STATUS {
	DPHY_INIT_STATUS_UNINITIALIZED,     /**< DPHY driver uinitialized    */
	DPHY_INIT_STATUS_INITIALIZED,       /**< DPHY driver initialized     */
}DPHY_INIT_STATUS;

/**
 * enum DPHY_MODE_CFG
 * DPHY mode(CIS2/DSI)
 */
typedef enum _DPHY_MODE_CFG {
	DPHY_MODE_CFG_DSI,                  /**< DPHY mode DSI    */
	DPHY_MODE_CFG_CSI2,                 /**< DPHY mode CSI2   */
}DPHY_MODE_CFG;

/** \brief hsfreqrange and osc_freq_target range */
typedef struct _DPHY_FREQ_RANGE {
	uint16_t bitrate_in_mbps;           /**< DPHY data rate in mbps            */
	uint8_t  hsfreqrange;               /**< DPHY HS frequency range           */
	uint16_t osc_freq_target;           /**< DPHY oscillator frequency target  */
}DPHY_FREQ_RANGE;

/** \brief PLL vco_cntrl range */
typedef struct _DPHY_PLL_VCO_CTRL {
	float frequency_mhz;                /**< DPHY frequency in MHZ   */
	int8_t   vco_ctrl;                  /**< DPHY VCO control        */
}DPHY_PLL_VCO_CTRL;

/**
\brief PLL Output division factor range
*/
typedef struct _DPHY_PLL_OUTPUT_DIVISION_FACTOR {
	float frequency_mhz;                /**< DPHY frequency in MHZ  */
	int8_t   p;                         /**< DPHY output division factor*/
}DPHY_PLL_OUTPUT_DIVISION_FACTOR;

#ifdef  __cplusplus
}
#endif

#endif /* DPHY_PRIVATE_H_ */
