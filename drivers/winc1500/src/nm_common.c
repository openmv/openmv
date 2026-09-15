/*******************************************************************************
  File Name:
    nm_common.c

  Summary:
    This module contains common APIs implementations.

  Description:
    This module contains common APIs implementations.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2022, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/

#include "nm_common.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_gpio.h"

/*!
 *  @fn         nm_sleep
 *  @brief      Sleep in units of mSec
 *  @param[IN]  u32TimeMsec
 *              Time in milliseconds
 */
void nm_sleep(uint32_t u32TimeMsec)
{
    WDRV_MSDelay(u32TimeMsec);
}

/*!
 *  @fn     nm_reset
 *  @brief  Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_reset(void)
{
    WDRV_WINC_GPIOChipEnableDeassert();
    WDRV_WINC_GPIOResetAssert();
    nm_sleep(100);
    WDRV_WINC_GPIOChipEnableAssert();
    nm_sleep(10);
    WDRV_WINC_GPIOResetDeassert();
    nm_sleep(10);
}

//DOM-IGNORE-END
