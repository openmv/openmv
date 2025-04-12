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
 * @file     DPHY_Loopback_test.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     10-Feb-2023
 * @brief    DPHY loopback test specific Header File.
 ******************************************************************************/

#ifndef DPHY_LOOPBACK_TEST_H_
#define DPHY_LOOPBACK_TEST_H_

#include <stdint.h>

/* Return status for PHY2PHY loopback test*/
#define TPASS 0
#define TFAIL -1

/**
  \fn          int32_t DPHY_External_Loopback_Test (uint32_t frequency, uint32_t loopback_test_run_time_us)
  \brief       External Loopback test.
  \param[in]   frequency to configure DPHY PLL.
  \param[in]   time in microseconds for which loopback test should run.
  \return      return test status TPASS or TFAIL.
  */
int32_t DPHY_External_Loopback_Test (uint32_t frequency, uint32_t loopback_test_run_time_us);

#endif /* DPHY_LOOPBACK_TEST_H_ */
