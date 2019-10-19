/**
  ******************************************************************************
  * @file    core_net_inspect_interface.h
  * @author  AST Embedded Analytics Research Platform
  * @date    20-Lug-2018
  * @brief   header file of core network inspection interface APIs
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

#ifndef __CORE_NET_INSPECT_INTERFACE_H_
#define __CORE_NET_INSPECT_INTERFACE_H_
#pragma once

#include "ai_platform.h"

AI_API_DECLARE_BEGIN

/*!
 * @defgroup core_validation Validation Core
 * @brief Implementation of the validation network interface headers 
 */


/*!
 * @struct ai_inspect_node_info
 * @brief network node inspection context: there is one of this datastruct
 * for each node of the network
 */
typedef struct ai_inspect_node_info_s {
  ai_u16                type; /*!< node type info @see ai_node datastruct */
  ai_u16                id;   /*!< node id assigned by codegen tool to identify 
                              the specific node instance */
  ai_u16                batch_id; /*!< current node batch processed */
  ai_u16                n_batches; /*!< total number of node batches to process */
  ai_float              elapsed_ms; /*!< node performance analysys: time in
                                    milliseconds to execute the node forward 
                                    function */
  ai_u16                in_size; /*!< number of node's input activation buffers */
  ai_u16                out_size; /*!< number of node's output activation buffers */
  ai_buffer*            in;   /*!< input node activation buffer see @ref ai_buffer */
  ai_buffer*            out;  /*!< output node activation buffer see @ref ai_buffer */
} ai_inspect_node_info;

/*!
 * @struct ai_inspect_net_report
 * @brief network inspection report context
 */
typedef struct ai_inspect_net_report_s {
  ai_u32                id;         /*!< id of the report */
  ai_signature          signature;  /*!< network identification checksum */
  ai_u32                num_inferences; /*!< total number of inferences processed 
                                        during the inspection */
  ai_u32                n_nodes;    /*!< number of nodes in the network */
  ai_float              elapsed_ms; /*!< network total time (in ms) for processing
                                     num_inferences inferences */
  ai_inspect_node_info* node;     /*!< pointer to the array of size n_nodes where
                                    a single node report is reported. see @ref 
                                     ai_inspect_node_info datastruct */
} ai_inspect_net_report;

/*!
 * @enum net inspector inspection mode
 * @brief configuration flags to set net inspection mode
 */
typedef enum {
  VALIDATION_INSPECT          = (0x1<<0), /**< Network validation inspection mode */
  STORE_ALL_IO_ACTIVATIONS    = (0x1<<7), /**< Store all I/O activations on snapshot datastruct */
} ai_inspect_mode;

typedef enum {
  AI_NODE_EXEC_PRE_FORWARD_STAGE  = 0x0,
  AI_NODE_EXEC_POST_FORWARD_STAGE = 0x1,
} ai_node_exec_stage;

/*!
 * @brief function pointer to callback report
 */
typedef void (*ai_inspect_report_cb_func)(
  const ai_handle cookie,
  const ai_inspect_net_report* report);

/*!
 * @brief function pointer to node execute
 */
typedef void (*ai_inspect_exec_node_cb_func)(
  const ai_handle cookie,
  const ai_inspect_node_info* node_info,
  const ai_node_exec_stage stage);

/*!
 * @struct ai_inspect_config
 * @brief inspection config datastruct
 */
typedef struct ai_inspect_config_s {
  ai_u8      validation_mode; /*!< validation mode flags
                                   see @ref ai_inspect_mode */
  ai_u8      log_level; /*!< log class level see @ref LOG_SUDO */
  ai_bool    log_quiet; /*!< log class quiet mode */
  ai_inspect_report_cb_func  on_report_destroy; /*!< callback function
                                     called when a report datastruct
                                     is released from memory */
  ai_inspect_exec_node_cb_func  on_exec_node; /*!< callback function
                                called when a node is executed (pre & post) */
  ai_handle  cookie;
} ai_inspect_config;


AI_API_DECLARE_END

#endif    /*__CORE_NET_INSPECT_INTERFACE_H_*/
