/*******************************************************************************
* Copyright (c) 2020, STMicroelectronics - All Rights Reserved
*
* This file is part of the VL53L5CX Ultra Lite Driver and is dual licensed,
* either 'STMicroelectronics Proprietary license'
* or 'BSD 3-clause "New" or "Revised" License' , at your option.
*
********************************************************************************
*
* 'STMicroelectronics Proprietary license'
*
********************************************************************************
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0081
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*
********************************************************************************
*
* Alternatively, the VL53L5CX Ultra Lite Driver may be distributed under the
* terms of 'BSD 3-clause "New" or "Revised" License', in which case the
* following provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
*******************************************************************************/

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
