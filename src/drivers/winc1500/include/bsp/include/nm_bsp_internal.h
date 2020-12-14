/**
 *
 * \file
 *
 * \brief This module contains NMC1500 BSP APIs declarations.
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/**@defgroup  BSPDefine Defines
 * @ingroup nm_bsp
 * @{
 */
#ifndef _NM_BSP_INTERNAL_H_
#define _NM_BSP_INTERNAL_H_



#ifdef WIN32
#include "nm_bsp_win32.h"
#endif

#ifdef __K20D50M__
#include "nm_bsp_k20d50m.h"
#endif

#ifdef __MSP430FR5739__
#include "bsp_msp430fr5739.h"
#endif

#ifdef _FREESCALE_MCF51CN128_
#include "bsp/include/nm_bsp_mcf51cn128.h"
#endif

#ifdef __MCF964548__
#include "bsp/include/nm_bsp_mc96f4548.h"
#endif

#ifdef __APP_APS3_CORTUS__
#include "nm_bsp_aps3_cortus.h"
#endif

#if (defined __SAMR21G18A__) || (defined __SAMR21E18A__)
#include "bsp/include/nm_bsp_samr21.h"
#endif

#if (defined __SAML21J18A__) || (defined __SAML21J18B__)
#include "bsp/include/nm_bsp_saml21.h"
#endif

#if (defined __SAML22N18A__)
#include "bsp/include/nm_bsp_saml22.h"
#endif

#if (defined __SAMD21J18A__) || (defined __SAMD21G18A__)
#include "bsp/include/nm_bsp_samd21.h"
#endif

#if (defined __SAM4S16C__) || (defined __SAM4SD32C__)
#include "bsp/include/nm_bsp_sam4s.h"
#endif

#ifdef __SAMG53N19__
#include "bsp/include/nm_bsp_samg53.h"
#endif

#ifdef __SAMG55J19__
#include "bsp/include/nm_bsp_samg55.h"
#endif

#if (defined __SAME70Q21__) || (defined __SAME70Q21B__) || (defined __SAMV71Q21__)
#include "bsp/include/nm_bsp_same70.h"
#endif

#ifdef CORTUS_APP
#include "crt_iface.h"
#endif

#ifdef NRF51
#include "nm_bsp_nrf51822.h"
#endif

#ifdef _ARDUINO_UNO_
#include "bsp/include/nm_bsp_arduino_uno.h"
#endif

#include "nm_bsp_openmv.h"

#endif //_NM_BSP_INTERNAL_H_
