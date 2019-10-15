/**
  ******************************************************************************
  * @file    ai_network_inspector.h
  * @author  AST Embedded Analytics Research Platform
  * @date    6-Aug-2018
  * @brief   header file of the network inspector wrapper plugin
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

#ifndef __AI_NETWORK_INSPECTOR_H_
#define __AI_NETWORK_INSPECTOR_H_
#pragma once

#include "ai_platform.h"
#include "ai_platform_interface.h"
#include "core_net_inspect_interface.h"

#define AI_INSPECTOR_API_MAJOR            1
#define AI_INSPECTOR_API_MINOR            1
#define AI_INSPECTOR_API_MICRO            0

/*!
 * @defgroup ai_network_inspector AI Network Inspector Module Tool
 * @brief header with datatypes and APIs for inspector module 
 */

#define AI_INSPECTOR_NETWORK_BIND_FAILED            (0x0)


AI_API_DECLARE_BEGIN

/*!
 * @typedef ai_inspector_entry_id
 * @ingroup ai_network_inspector
 * @brief network inspector  bind network index id number 
 * (actually the inspector could bind up to 65534 networks)
 */
typedef ai_u16 ai_inspector_entry_id;

/*!
 * @typedef ai_inspector_net_info
 * @ingroup ai_network_inspector
 * @brief wrapper struct for @ref ai_network_report
 */
typedef ai_network_report ai_inspector_net_info;

/*!
 * @typedef ai_inspector_config
 * @ingroup ai_network_inspector
 * @brief wrapper struct for @ref ai_inspect_config
 */
typedef ai_inspect_config ai_inspector_config;

/*!
 * @typedef ai_inspector_node_info
 * @ingroup ai_network_inspector
 * @brief wrapper struct for @ref ai_inspect_node_info
 */
typedef ai_inspect_node_info ai_inspector_node_info;

/*!
 * @typedef ai_inspector_net_report
 * @ingroup ai_network_inspector
 * @brief wrapper struct for @ref ai_inspect_net_report
 */
typedef ai_inspect_net_report ai_inspector_net_report;

/*!
 * @struct ai_inspector_net_entry
 * @ingroup ai_network_inspector
 * @brief struct with info related to the bound network. It has the network 
 * handle, the network params, and the error (see @ref ai_error definition)
 */
typedef struct ai_inspector_net_entry_ {
  ai_handle                   handle;  /*!< bound network context handle */
  ai_network_params           params;  /*!< bound network context params */
  ai_error                    error;   /*!< bound network context error */
} ai_inspector_net_entry;

/*!
 * @brief Return default context config.
 * @ingroup ai_network_inspector
 * @param[out] inspector config datastructure
 * @return the default inspector configuration
 */
AI_API_ENTRY
ai_inspector_config ai_inspector_default_config(void);

/*!
 * @brief Create a network inspector plugin module.
 * @ingroup ai_network_inspector
 * @param[out] handle a pointer to an opaque handle that points to the inspector
 * context created
 * @param[in] cfg a pointer to the inspector config. if NULL a default config is
 * used by the inspector instance
 * @return true if initialization was fine, false otherwise 
 */
AI_API_ENTRY
ai_bool ai_inspector_create(
  ai_handle* handle, const ai_inspector_config* cfg);

/*!
 * @brief Destroy a network inspector plugin module.
 * @ingroup ai_network_inspector
 * @param[in/out] handle an opaque handle to the inspector context to destroy
 * context
 * @return true if destroy was fine, false otherwise 
 */
AI_API_ENTRY
ai_bool ai_inspector_destroy(ai_handle handle);

/*!
 * @brief Bind a network instance with the inspector plugin
 * @ingroup ai_network_inspector
 * @param[in/out] handle an opaque handle to the inspector context
 * @param[in] entry a pointer to the info about the network to be bound
 * @return a network id > 0 that is an index used to refer to the bound 
 * network instance. if the returned index is 0 an error occurred during binding 
 */
AI_API_ENTRY
ai_inspector_entry_id ai_inspector_bind_network(
  ai_handle handle, const ai_inspector_net_entry* entry);

/*!
 * @brief Unbind a network instance from the inspector plugin
 * @ingroup ai_network_inspector
 * @param[in/out] handle an opaque handle to the inspector context
 * @param[in] net_id: a network id provided by @ref ai_inspector_bind_network API 
 * @return true if the unbind was successful, false otherwise
 */
AI_API_ENTRY
ai_bool ai_inspector_unbind_network(
  ai_handle handle, const ai_inspector_entry_id net_id);

/*!
 * @brief Get inspection report on a bind network
 * @ingroup ai_network_inspector
 * @param[in/out] handle an opaque handle to the inspector context
 * @param[in] net_id: a network id provided by @ref ai_inspector_bind_network API 
 * @param[out] report a pointer to the required report @ref ai_inspector_net_report
 * data struct 
 * @return true if the query was successful, false otherwise
 */
AI_API_ENTRY
ai_bool ai_inspector_get_report(
  ai_handle handle, const ai_inspector_entry_id net_id,
  ai_inspector_net_report* report);

/*!
 * @brief Run a network instance bind to the inspector
 * @ingroup ai_network_inspector
 * @param[in/out] handle an opaque handle to the inspector context
 * @param[in] net_id: a network id provided by @ref ai_inspector_bind_network API 
 * @param[in] input a pointer to the input data buffer
 * @param[out] outbut a pointer to the output data buffer
 * @return the number of batches processed. a value <=0 indicates an error
 */
AI_API_ENTRY
ai_i32 ai_inspector_run(
  ai_handle handle, const ai_inspector_entry_id net_id, 
  const ai_buffer* input, ai_buffer* output);

AI_API_DECLARE_END

#endif  /* __AI_NETWORK_INSPECTOR_H_ */
