/**
  ******************************************************************************
  * @file    core_net_inspect.h
  * @author  AST Embedded Analytics Research Platform
  * @date    20-Lug-2018
  * @brief   header file of core network inspection APIs
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __CORE_NET_INSPECT_H_
#define __CORE_NET_INSPECT_H_
#pragma once

#include "core_net_inspect_interface.h"

#include "core_common.h"
#include "layers_common.h"

/*!
 * @defgroup core_net_inspect Core Network Inspection routines
 * @brief Implementation of core network inspection routines that allows to 
 * inspect on a node basis a generated network model
 * @details A network context @ref ai_network basically contains a chained list
 * of nodes @ref ai_node that have an associated forward function.
 * Each ai)network context and ai_node datastructs have as a required member 
 * field an opaque handler (i.e. a void pointer) to a klass object.
 * This handler is intended to be used as a platform specific node context 
 * that implements specific target platform routines. 
 * The inspector module basically acts as a plugin that exploiting these features
 * by temporary creating an hidden inspection context (see 
 * @ref ai_core_inspect_net_klass) associated to the network and
 * linking it by re-routing the klass field to this inspection context. The
 * inspection context saves as part of its state (by a stack push operation), the
 * internal state of the network (all node / network klass pointers and actual
 * forward functions).
 * Thus, for each node it re-routes all node's forward functions to a dedicated
 * inspection forward function (see @ref _forward_inspect_validate() routine)
 * This routine is the core of the mechanism and it allows to inspect a network
 * node by node. Some additional inspection could thus be done inside the 
 * _forward_inspect_validate() routine before and after the actual node
 * forward function is called;
 *   
 */

AI_API_DECLARE_BEGIN

/*!
 * @defgroup core_net_inspect Network Inspection Core
 * @brief Implementation of the validation network routines 
 */

/*!
 * @brief Initialize the network inspection context on a given network
 * @ingroup core net inspect
 * @param network opaque handler to the network instance
 * @param cfg a pointer to the inspector configuration we want to use
 * @return true if execution of the API is fine, false otherwise
 */
AI_API_ENTRY
ai_bool ai_network_inspect_init(
  ai_handle network, const ai_inspect_config* cfg);

/*!
 * @brief Get a summary report from the inspected network
 * @ingroup core net inspect
 * @param network opaque handler to the network instance
 * @param report a pointer to the report provided back by the inspection
 * @return true if execution of the API is fine, false otherwise
 */
AI_API_ENTRY
ai_bool ai_network_inspect_get_report(
  ai_handle network, ai_inspect_net_report* report);

/*!
 * @brief Destroy the network inspection context on a given network
 * @ingroup core net inspect
 * @param network opaque handler to the network instance
 * @return true if execution of the API is fine, false otherwise
 */
AI_API_ENTRY
ai_bool ai_network_inspect_destroy(ai_handle network);

AI_API_DECLARE_END

#endif    /*__CORE_NET_INSPECT_H_*/
