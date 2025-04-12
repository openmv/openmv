/**
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef VL53L5CX_PLUGIN_MOTION_INDICATOR_H_
#define VL53L5CX_PLUGIN_MOTION_INDICATOR_H_

#include "vl53l5cx_api.h"

/**
 * @brief Motion indicator internal configuration structure.
 */

typedef struct {
	int32_t  ref_bin_offset;
	uint32_t detection_threshold;
	uint32_t extra_noise_sigma;
	uint32_t null_den_clip_value;
	uint8_t  mem_update_mode;
	uint8_t  mem_update_choice;
	uint8_t  sum_span;
	uint8_t  feature_length;
	uint8_t  nb_of_aggregates;
	uint8_t  nb_of_temporal_accumulations;
	uint8_t  min_nb_for_global_detection;
	uint8_t  global_indicator_format_1;
	uint8_t  global_indicator_format_2;
	uint8_t  spare_1;
	uint8_t  spare_2;
	uint8_t  spare_3;
	int8_t 	 map_id[64];
	uint8_t  indicator_format_1[32];
	uint8_t  indicator_format_2[32];
}VL53L5CX_Motion_Configuration;

/**
 * @brief This function is used to initialized the motion indicator. By default,
 * indicator is programmed to monitor movements between 400mm and 1500mm.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (VL53L5CX_Motion_Configuration) *p_motion_config : Structure
 * containing the initialized motion configuration.
 * @param (uint8_t) resolution : Wanted resolution, defined by macros
 * VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8.
 * @return (uint8_t) status : 0 if OK, or 127 is the resolution is unknown.
 */

uint8_t vl53l5cx_motion_indicator_init(
		VL53L5CX_Configuration		*p_dev,
		VL53L5CX_Motion_Configuration	*p_motion_config,
		uint8_t				resolution);

/**
 * @brief This function can be used to change the working distance of motion
 * indicator. By default, indicator is programmed to monitor movements between
 * 400mm and 1500mm.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (VL53L5CX_Motion_Configuration) *p_motion_config : Structure
 * containing the initialized motion configuration.
 * @param (uint16_t) distance_min_mm : Minimum distance for indicator (min value 
 * 400mm, max 4000mm).
 * @param (uint16_t) distance_max_mm : Maximum distance for indicator (min value 
 * 400mm, max 4000mm).
 * VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8.
 * @return (uint8_t) status : 0 if OK, or 127 if an argument is invalid.
 */

uint8_t vl53l5cx_motion_indicator_set_distance_motion(
		VL53L5CX_Configuration		*p_dev,
		VL53L5CX_Motion_Configuration	*p_motion_config,
		uint16_t			distance_min_mm,
		uint16_t			distance_max_mm);

/**
 * @brief This function is used to update the internal motion indicator map.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (VL53L5CX_Motion_Configuration) *p_motion_config : Structure
 * containing the initialized motion configuration.
 * @param (uint8_t) resolution : Wanted SCI resolution, defined by macros
 * VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8.
 * @return (uint8_t) status : 0 if OK, or 127 is the resolution is unknown.
 */

uint8_t vl53l5cx_motion_indicator_set_resolution(
		VL53L5CX_Configuration		*p_dev,
		VL53L5CX_Motion_Configuration	*p_motion_config,
		uint8_t				resolution);

#endif /* VL53L5CX_PLUGIN_MOTION_INDICATOR_H_ */
