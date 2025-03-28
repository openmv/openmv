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
 * @file     DMA_Common.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     05-Aug-2022
 * @brief    Common DMA definitions.
 ******************************************************************************/

#ifndef DMA_COMMON_H_
#define DMA_COMMON_H_

#include <Driver_DMA.h>
#include <RTE_Device.h>
#include <evtrtr.h>

#ifdef  __cplusplus
extern "C"
{
#endif

#if RTE_DMA0
extern ARM_DRIVER_DMA ARM_Driver_DMA_(0);
#endif

#if RTE_DMA1
extern ARM_DRIVER_DMA ARM_Driver_DMA_(1);
#endif

#if RTE_DMA2
extern ARM_DRIVER_DMA ARM_Driver_DMA_(2);
#endif


/**
\brief DMA Peripheral Configuration
*/
typedef struct _DMA_PERIPHERAL_CONFIG {

    /*!< DMA controller driver  */
    const ARM_DRIVER_DMA *dma_drv;

    /*!< Peripheral request number */
    const uint8_t dma_periph_req;

    /*!< DMA handle */
    DMA_Handle_Type dma_handle;

    /*!< Event Router Configuration */
    EVTRTR_CONFIG evtrtr_cfg;
} DMA_PERIPHERAL_CONFIG;

#ifdef  __cplusplus
}
#endif

#endif /* DMA_COMMON_H_ */
