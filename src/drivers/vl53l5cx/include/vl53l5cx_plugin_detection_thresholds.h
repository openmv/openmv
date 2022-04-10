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

#ifndef VL53L5CX_PLUGIN_DETECTION_THRESHOLDS_H_
#define VL53L5CX_PLUGIN_DETECTION_THRESHOLDS_H_

#include "vl53l5cx_api.h"

/**
 * @brief Macro VL53L5CX_NB_THRESHOLDS indicates the number of checkers. This
 * value cannot be changed.
 */

#define VL53L5CX_NB_THRESHOLDS				((uint8_t)64U)

/**
 * @brief Inner Macro for API. Not for user, only for development.
 */

#define VL53L5CX_DCI_DET_THRESH_CONFIG			((uint16_t)0x5488U)
#define VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG		((uint16_t)0xB6E0U)
#define VL53L5CX_DCI_DET_THRESH_START			((uint16_t)0xB6E8U)
#define VL53L5CX_DCI_DET_THRESH_VALID_STATUS		((uint16_t)0xB9F0U)

/**
 * @brief Macro VL53L5CX_LAST_THRESHOLD is used to indicate the end of checkers
 * programming.
 */

#define VL53L5CX_LAST_THRESHOLD				((uint8_t)128U)

/**
 * @brief The following macro are used to define the 'param_type' of a checker.
 * They indicate what is the measurement to catch.
 */

#define VL53L5CX_DISTANCE_MM				((uint8_t)1U)
#define VL53L5CX_SIGNAL_PER_SPAD_KCPS 		        ((uint8_t)2U)
#define VL53L5CX_RANGE_SIGMA_MM		 		((uint8_t)4U)
#define VL53L5CX_AMBIENT_PER_SPAD_KCPS 			((uint8_t)8U)
#define VL53L5CX_NB_TARGET_DETECTED	 		((uint8_t)9U)
#define VL53L5CX_TARGET_STATUS				((uint8_t)12U)
#define VL53L5CX_NB_SPADS_ENABLED			((uint8_t)13U)
#define VL53L5CX_MOTION_INDICATOR          		((uint8_t)19U)

/**
 * @brief The following macro are used to define the 'type' of a checker.
 * They indicate the window of measurements, defined by low and a high
 * thresholds.
 */

#define VL53L5CX_IN_WINDOW				((uint8_t)0U)
#define VL53L5CX_OUT_OF_WINDOW				((uint8_t)1U)
#define VL53L5CX_LESS_THAN_EQUAL_MIN_CHECKER		((uint8_t)2U)
#define VL53L5CX_GREATER_THAN_MAX_CHECKER		((uint8_t)3U)
#define VL53L5CX_EQUAL_MIN_CHECKER			((uint8_t)4U)
#define VL53L5CX_NOT_EQUAL_MIN_CHECKER			((uint8_t)5U)

/**
 * @brief The following macro are used to define multiple checkers in the same
 * zone, using operators. Please note that the first checker MUST always be a OR
 * operation.
 */

#define VL53L5CX_OPERATION_NONE				((uint8_t)0U)
#define VL53L5CX_OPERATION_OR				((uint8_t)0U)
#define VL53L5CX_OPERATION_AND				((uint8_t)2U)

/**
 * @brief Structure VL53L5CX_DetectionThresholds contains a single threshold.
 * This structure  is never used alone, it must be used as an array of 64
 * thresholds (defined by macro VL53L5CX_NB_THRESHOLDS).
 */

typedef struct {

	/* Low threshold */
	int32_t 	param_low_thresh;
	/* High threshold */
	int32_t 	param_high_thresh;
	/* Measurement to catch (VL53L5CX_MEDIAN_RANGE_MM,...)*/
	uint8_t 	measurement;
	/* Windows type (VL53L5CX_IN_WINDOW, VL53L5CX_OUT_WINDOW, ...) */
	uint8_t 	type;
	/* Zone id. Please read VL53L5 user manual to find the zone id.Set macro
	 * VL53L5CX_LAST_THRESHOLD to indicates the end of checkers */
	uint8_t 	zone_num;
	/* Mathematics operation (AND/OR). The first threshold is always OR.*/
	uint8_t		mathematic_operation;
}VL53L5CX_DetectionThresholds;

/**
 * @brief This function allows indicating if the detection thresholds are
 * enabled.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *p_enabled : Set to 1 if enabled, or 0 if disable.
 * @return (uint8_t) status : 0 if OK
 */

uint8_t vl53l5cx_get_detection_thresholds_enable(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_enabled);

/**
 * @brief This function allows enable the detection thresholds.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) enabled : Set to 1 to enable, or 0 to disable thresholds.
 * @return (uint8_t) status : 0 if programming is OK
 */

uint8_t vl53l5cx_set_detection_thresholds_enable(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				enabled);

/**
 * @brief This function allows getting the detection thresholds.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (VL53L5CX_DetectionThresholds) *p_thresholds : Array of 64 thresholds.
 * @return (uint8_t) status : 0 if programming is OK
 */

uint8_t vl53l5cx_get_detection_thresholds(
		VL53L5CX_Configuration		*p_dev,
		VL53L5CX_DetectionThresholds	*p_thresholds);

/**
 * @brief This function allows programming the detection thresholds.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (VL53L5CX_DetectionThresholds) *p_thresholds :  Array of 64 thresholds.
 * @return (uint8_t) status : 0 if programming is OK
 */

uint8_t vl53l5cx_set_detection_thresholds(
		VL53L5CX_Configuration		*p_dev,
		VL53L5CX_DetectionThresholds	*p_thresholds);

#endif /* VL53L5CX_PLUGIN_DETECTION_THRESHOLDS_H_ */
