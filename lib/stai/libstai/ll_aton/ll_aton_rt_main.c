/**
 ******************************************************************************
 * @file    ll_aton_rt_main.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Template `main()` function (named `LL_ATON_RT_Main()`) for
 *          Cube.AI/ATON integration in a RTOS-less application.
 * @note    This file is intended to be just a template and is limited to run the
 *          ATON LL runtime with a single network instance for a single inference!
 *          Please, generate your own embodiment of this file and customize it
 *          so to fit the needs of your application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "ll_aton_runtime.h"
#include "ll_aton_util.h"

/*** Main Template ***/

/**
 * @brief Template for synchronously executing a single inference of a single network instance (e.g. regression tests)
 * @param network_instance pointer to the network instance representing the network and execution instance to execute.
 *                         The instance object MUST have already set a valid link to a network interface.
 *                         The user may declare/instantiate such an object by using either macro
 *                          `LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE()` to create both the execution instance
 *                         and the network interface, or macros
 *                         `LL_ATON_DECLARE_NAMED_NN_INTERFACE()` & `LL_ATON_DECLARE_NAMED_NN_INSTANCE()` to
 *                         create/instantiate the objects separately.
 */
void LL_ATON_RT_Main(NN_Instance_TypeDef *network_instance)
{
  LL_ATON_RT_RetValues_t ll_aton_rt_ret;

  /*** Start of user initialization code ***/

  /*** End of user initialization code ***/

  LL_ATON_ASSERT(network_instance != NULL);
  LL_ATON_ASSERT(network_instance->network != NULL);
  LL_ATON_RT_RuntimeInit();                  // Initialize runtime
  LL_ATON_RT_Init_Network(network_instance); // Initialize passed network instance object

  do
  {
    /* Execute first/next step of Cube.AI/ATON runtime */
    ll_aton_rt_ret = LL_ATON_RT_RunEpochBlock(network_instance);

    /*** Start of user event handling code ***/

    /*** End of user event handling code ***/

    /* Wait for next event */
    if (ll_aton_rt_ret == LL_ATON_RT_WFE)
    { /*** subject to change to fit also user code requirements ***/
      LL_ATON_OSAL_WFE();
    }
  } while (ll_aton_rt_ret != LL_ATON_RT_DONE); /*** subject to change to fit also user code requirements ***/

  LL_ATON_RT_DeInit_Network(network_instance); // De-initialize the network instance object
  LL_ATON_RT_RuntimeDeInit();                  // De-initialize runtime

  /*** Start of user de-initialization code ***/

  /*** End of user de-initialization code ***/
}
