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

#include "vl53l5cx_plugin_detection_thresholds.h"

uint8_t vl53l5cx_get_detection_thresholds_enable(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_enabled)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(p_dev, (uint8_t*)p_dev->temp_buffer,
			VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8);
	*p_enabled = p_dev->temp_buffer[0x1];

	return status;
}

uint8_t vl53l5cx_set_detection_thresholds_enable(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				enabled)
{
	uint8_t tmp, status = VL53L5CX_STATUS_OK;
	uint8_t grp_global_config[] = {0x01, 0x00, 0x01, 0x00};

	if(enabled == (uint8_t)1)
	{
		grp_global_config[0x01] = 0x01;
		tmp = 0x04;
	}
	else
	{
		grp_global_config[0x01] = 0x00;
		tmp = 0x0C;
	}

	/* Set global interrupt config */
	status |= vl53l5cx_dci_replace_data(p_dev, p_dev->temp_buffer,
			VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8,
			(uint8_t*)&grp_global_config, 4, 0x00);

	/* Update interrupt config */
	status |= vl53l5cx_dci_replace_data(p_dev, p_dev->temp_buffer,
			VL53L5CX_DCI_DET_THRESH_CONFIG, 20,
			(uint8_t*)&tmp, 1, 0x11);

	return status;
}

uint8_t vl53l5cx_get_detection_thresholds(
		VL53L5CX_Configuration			*p_dev,
		VL53L5CX_DetectionThresholds	*p_thresholds)
{
	uint8_t i, status = VL53L5CX_STATUS_OK;

	/* Get thresholds configuration */
	status |= vl53l5cx_dci_read_data(p_dev, (uint8_t*)p_thresholds,
			VL53L5CX_DCI_DET_THRESH_START, 
                        (uint16_t)VL53L5CX_NB_THRESHOLDS
			*(uint16_t)sizeof(VL53L5CX_DetectionThresholds));

	for(i = 0; i < (uint8_t)VL53L5CX_NB_THRESHOLDS; i++)
	{
		switch(p_thresholds[i].measurement)
		{
			case VL53L5CX_DISTANCE_MM:
				p_thresholds[i].param_low_thresh  /= 4;
				p_thresholds[i].param_high_thresh /= 4;
				break;
			case VL53L5CX_SIGNAL_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  /= 2048;
				p_thresholds[i].param_high_thresh /= 2048;
				break;
			case VL53L5CX_RANGE_SIGMA_MM:
				p_thresholds[i].param_low_thresh  /= 128;
				p_thresholds[i].param_high_thresh /= 128;
				break;
			case VL53L5CX_AMBIENT_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  /= 2048;
				p_thresholds[i].param_high_thresh /= 2048;
				break;
			case VL53L5CX_NB_SPADS_ENABLED:
				p_thresholds[i].param_low_thresh  /= 256;
				p_thresholds[i].param_high_thresh /= 256;
				break;
			case VL53L5CX_MOTION_INDICATOR:
				p_thresholds[i].param_low_thresh  /= 65535;
				p_thresholds[i].param_high_thresh /= 65535;
				break;
			default:
				break;
		}
	}

	return status;
}

uint8_t vl53l5cx_set_detection_thresholds(
		VL53L5CX_Configuration			*p_dev,
		VL53L5CX_DetectionThresholds	*p_thresholds)
{
	uint8_t i, status = VL53L5CX_STATUS_OK;
	uint8_t grp_valid_target_cfg[] = {0x05, 0x05, 0x05, 0x05,
					0x05, 0x05, 0x05, 0x05};

	for(i = 0; i < (uint8_t) VL53L5CX_NB_THRESHOLDS; i++)
	{
		switch(p_thresholds[i].measurement)
		{
			case VL53L5CX_DISTANCE_MM:
				p_thresholds[i].param_low_thresh  *= 4;
				p_thresholds[i].param_high_thresh *= 4;
				break;
			case VL53L5CX_SIGNAL_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  *= 2048;
				p_thresholds[i].param_high_thresh *= 2048;
				break;
			case VL53L5CX_RANGE_SIGMA_MM:
				p_thresholds[i].param_low_thresh  *= 128;
				p_thresholds[i].param_high_thresh *= 128;
				break;
			case VL53L5CX_AMBIENT_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  *= 2048;
				p_thresholds[i].param_high_thresh *= 2048;
				break;
			case VL53L5CX_NB_SPADS_ENABLED:
				p_thresholds[i].param_low_thresh  *= 256;
				p_thresholds[i].param_high_thresh *= 256;
				break;
			case VL53L5CX_MOTION_INDICATOR:
				p_thresholds[i].param_low_thresh  *= 65535;
				p_thresholds[i].param_high_thresh *= 65535;
				break;
			default:
				break;
		}
	}

	/* Set valid target list */
	status |= vl53l5cx_dci_write_data(p_dev, (uint8_t*)grp_valid_target_cfg,
			VL53L5CX_DCI_DET_THRESH_VALID_STATUS, 
			(uint16_t)sizeof(grp_valid_target_cfg));

	/* Set thresholds configuration */
	status |= vl53l5cx_dci_write_data(p_dev, (uint8_t*)p_thresholds,
			VL53L5CX_DCI_DET_THRESH_START, 
			(uint16_t)(VL53L5CX_NB_THRESHOLDS
			*sizeof(VL53L5CX_DetectionThresholds)));

	return status;
}
