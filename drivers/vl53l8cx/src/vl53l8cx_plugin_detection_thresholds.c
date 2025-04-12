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

#include "vl53l8cx_plugin_detection_thresholds.h"

uint8_t vl53l8cx_get_detection_thresholds_enable(
		VL53L8CX_Configuration		*p_dev,
		uint8_t				*p_enabled)
{
	uint8_t status = VL53L8CX_STATUS_OK;

	status |= vl53l8cx_dci_read_data(p_dev, (uint8_t*)p_dev->temp_buffer,
			VL53L8CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8);
	*p_enabled = p_dev->temp_buffer[0x1];

	return status;
}

uint8_t vl53l8cx_set_detection_thresholds_enable(
		VL53L8CX_Configuration		*p_dev,
		uint8_t				enabled)
{
	uint8_t tmp, status = VL53L8CX_STATUS_OK;
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
	status |= vl53l8cx_dci_replace_data(p_dev, p_dev->temp_buffer,
			VL53L8CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8,
			(uint8_t*)&grp_global_config, 4, 0x00);

	/* Update interrupt config */
	status |= vl53l8cx_dci_replace_data(p_dev, p_dev->temp_buffer,
			VL53L8CX_DCI_DET_THRESH_CONFIG, 20,
			(uint8_t*)&tmp, 1, 0x11);

	return status;
}

uint8_t vl53l8cx_get_detection_thresholds(
		VL53L8CX_Configuration			*p_dev,
		VL53L8CX_DetectionThresholds	*p_thresholds)
{
	uint8_t i, status = VL53L8CX_STATUS_OK;

	/* Get thresholds configuration */
	status |= vl53l8cx_dci_read_data(p_dev, (uint8_t*)p_thresholds,
			VL53L8CX_DCI_DET_THRESH_START,
                        (uint16_t)VL53L8CX_NB_THRESHOLDS
			*(uint16_t)sizeof(VL53L8CX_DetectionThresholds));

	for(i = 0; i < (uint8_t)VL53L8CX_NB_THRESHOLDS; i++)
	{
		switch(p_thresholds[i].measurement)
		{
			case VL53L8CX_DISTANCE_MM:
				p_thresholds[i].param_low_thresh  /= 4;
				p_thresholds[i].param_high_thresh /= 4;
				break;
			case VL53L8CX_SIGNAL_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  /= 2048;
				p_thresholds[i].param_high_thresh /= 2048;
				break;
			case VL53L8CX_RANGE_SIGMA_MM:
				p_thresholds[i].param_low_thresh  /= 128;
				p_thresholds[i].param_high_thresh /= 128;
				break;
			case VL53L8CX_AMBIENT_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  /= 2048;
				p_thresholds[i].param_high_thresh /= 2048;
				break;
			case VL53L8CX_NB_SPADS_ENABLED:
				p_thresholds[i].param_low_thresh  /= 256;
				p_thresholds[i].param_high_thresh /= 256;
				break;
			case VL53L8CX_MOTION_INDICATOR:
				p_thresholds[i].param_low_thresh  /= 65535;
				p_thresholds[i].param_high_thresh /= 65535;
				break;
			default:
				break;
		}
	}

	return status;
}

uint8_t vl53l8cx_set_detection_thresholds(
		VL53L8CX_Configuration			*p_dev,
		VL53L8CX_DetectionThresholds	*p_thresholds)
{
	uint8_t i, status = VL53L8CX_STATUS_OK;
	uint8_t grp_valid_target_cfg[] = {0x05, 0x05, 0x05, 0x05,
					0x05, 0x05, 0x05, 0x05};

	for(i = 0; i < (uint8_t) VL53L8CX_NB_THRESHOLDS; i++)
	{
		switch(p_thresholds->measurement)
		{
			case VL53L8CX_DISTANCE_MM:
				p_thresholds[i].param_low_thresh  *= 4;
				p_thresholds[i].param_high_thresh *= 4;
				break;
			case VL53L8CX_SIGNAL_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  *= 2048;
				p_thresholds[i].param_high_thresh *= 2048;
				break;
			case VL53L8CX_RANGE_SIGMA_MM:
				p_thresholds[i].param_low_thresh  *= 128;
				p_thresholds[i].param_high_thresh *= 128;
				break;
			case VL53L8CX_AMBIENT_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  *= 2048;
				p_thresholds[i].param_high_thresh *= 2048;
				break;
			case VL53L8CX_NB_SPADS_ENABLED:
				p_thresholds[i].param_low_thresh  *= 256;
				p_thresholds[i].param_high_thresh *= 256;
				break;
			case VL53L8CX_MOTION_INDICATOR:
				p_thresholds[i].param_low_thresh  *= 65535;
				p_thresholds[i].param_high_thresh *= 65535;
				break;
			default:
				break;
		}
	}

	/* Set valid target list */
	status |= vl53l8cx_dci_write_data(p_dev, (uint8_t*)grp_valid_target_cfg,
			VL53L8CX_DCI_DET_THRESH_VALID_STATUS,
			(uint16_t)sizeof(grp_valid_target_cfg));

	/* Set thresholds configuration */
	status |= vl53l8cx_dci_write_data(p_dev, (uint8_t*)p_thresholds,
			VL53L8CX_DCI_DET_THRESH_START,
			(uint16_t)(VL53L8CX_NB_THRESHOLDS
			*sizeof(VL53L8CX_DetectionThresholds)));

	return status;
}

uint8_t vl53l8cx_get_detection_thresholds_auto_stop(
		VL53L8CX_Configuration		*p_dev,
		uint8_t				*p_auto_stop)
{
	uint8_t status = VL53L8CX_STATUS_OK;

	status |= vl53l8cx_dci_read_data(p_dev, (uint8_t*)p_dev->temp_buffer,
			VL53L8CX_DCI_PIPE_CONTROL, 4);
	*p_auto_stop = p_dev->temp_buffer[0x3];

	return status;
}

uint8_t vl53l8cx_set_detection_thresholds_auto_stop(
		VL53L8CX_Configuration		*p_dev,
		uint8_t				auto_stop)
{
	uint8_t status = VL53L8CX_STATUS_OK;

	status |= vl53l8cx_dci_replace_data(p_dev, p_dev->temp_buffer,
			VL53L8CX_DCI_PIPE_CONTROL, 4,
	(uint8_t*)&auto_stop, 1, 0x03);
	p_dev->is_auto_stop_enabled = (uint8_t)auto_stop;

	return status;
}
