/**
  ******************************************************************************
  * @file    stai_events.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   Definitions of ST.AI registered events
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef STAI_EVENTS_H
#define STAI_EVENTS_H
#include "stai.h"


STAI_API_DECLARE_BEGIN

/*****************************************************************************/
/** STAI event IDs definition
 * This is the list of stai supported events that could be triggered by network
 * callback. Each event has an associated datastruct that provides the payload
 * for a give event. All supported payloads are defined int the @ref stai_event union.
 * New events could be defined by:
 *    1) Add a new entry in stai_event_type with unique type id
 *    2) Add a new datastruct definition fpr the new event that specified its payload
 *    3) Add a pointer to this payload into stai_event union to make it public
 **/
#define STAI_EVENT_NODE_START               (0x01)
#define STAI_EVENT_NODE_STOP                (0x02)


/*****************************************************************************/
/** STAI events definitions
 * ADD YOUR EVENT HERE AND ADD TO STAI_EVENT if public
 **/
typedef struct {
  int32_t                   node_id;
  stai_array_const_ptr            buffers;
} stai_event_node_start_stop;


STAI_API_DECLARE_END

#endif    /* STAI_EVENTS_H */
