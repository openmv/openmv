/*******************************************************************************
  File Name:
    nm_common.h

  Summary:
    This module contains WINC1500 BSP APIs declarations.

  Description:
    This module contains WINC1500 BSP APIs declarations.
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
//DOM-IGNORE-END

/** @defgroup nm_bsp BSP
    @brief
        Description of the BSP (<strong>B</strong>oard <strong>S</strong>upport <strong>P</strong>ackage) module.
    @{
        @defgroup   DataT       Data Types
        @defgroup   BSPDefine   Defines
        @defgroup   BSPAPI      Functions
        @brief
            Lists the available BSP (<strong>B</strong>oard <strong>S</strong>upport <strong>P</strong>ackage) APIs.
    @}
 */

/**@addtogroup BSPDefine
   @{
 */
#ifndef _NM_BSP_H_
#define _NM_BSP_H_

// OpenMV: the Harmony build supplies these through its own configuration header.
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// OpenMV: the interrupt plumbing is ours, the Harmony build does this in its
// WDRV layer instead.
typedef void (*tpfNmBspIsr)(void);

int8_t nm_bsp_init(void);
int8_t nm_bsp_deinit(void);
void nm_bsp_register_isr(tpfNmBspIsr pfIsr);
void nm_bsp_interrupt_ctrl(uint8_t enable);


#define BSP_MIN(x,y) ((x)>(y)?(y):(x))
/*!<
*     Computes the minimum value between \b x and \b y.
*/
/**@}*/     //BSPDefine

 //@}

/**
 * @addtogroup BSPDefine
 * @{
 */
#ifdef _NM_BSP_BIG_END
/*! Switch endianness of 32bit word (In the case that Host is BE) */
#define NM_BSP_B_L_32(x)      \
((((x) & 0x000000FF) << 24) + \
(((x) & 0x0000FF00) << 8)   + \
(((x) & 0x00FF0000) >> 8)   + \
(((x) & 0xFF000000) >> 24))

/*! Switch endianness of 16bit word (In the case that Host is BE) */
#define NM_BSP_B_L_16(x) \
((((x) & 0x00FF) << 8) + \
(((x)  & 0xFF00) >> 8))
#else
/*! Retain endianness of 32bit word (In the case that Host is LE) */
#define NM_BSP_B_L_32(x)  (x)
/*! Retain endianness of 16bit word (In the case that Host is LE) */
#define NM_BSP_B_L_16(x)  (x)
#endif
/**@}*/     //BSPDefine

#endif  /*_NM_BSP_H_*/
