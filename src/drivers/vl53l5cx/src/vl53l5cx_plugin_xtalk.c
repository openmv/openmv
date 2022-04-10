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

#include "vl53l5cx_plugin_xtalk.h"

/*
 * Inner function, not available outside this file. This function is used to
 * wait for an answer from VL53L5 sensor.
 */

static uint8_t _vl53l5cx_poll_for_answer(
		VL53L5CX_Configuration   *p_dev,
		uint16_t 				address,
		uint8_t 				expected_value)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	uint8_t timeout = 0;

	do {
		status |= RdMulti(&(p_dev->platform), 
                                  address, p_dev->temp_buffer, 4);
		status |= WaitMs(&(p_dev->platform), 10);
		
                /* 2s timeout or FW error*/
		if((timeout >= (uint8_t)200) 
                   || (p_dev->temp_buffer[2] >= (uint8_t) 0x7f))
		{
			status |= VL53L5CX_MCU_ERROR;		
			break;
		}
                else
                {
                  timeout++;
                }
	}while ((p_dev->temp_buffer[0x1]) != expected_value);
        
	return status;
}

/*
 * Inner function, not available outside this file. This function is used to
 * program the output using the macro defined into the 'platform.h' file.
 */

static uint8_t _vl53l5cx_program_output_config(
		VL53L5CX_Configuration 		 *p_dev)
{
	uint8_t resolution, status = VL53L5CX_STATUS_OK;
	uint32_t i;
	uint64_t header_config;
	union Block_header *bh_ptr;

	status |= vl53l5cx_get_resolution(p_dev, &resolution);
	p_dev->data_read_size = 0;

	/* Enable mandatory output (meta and common data) */
	uint32_t output_bh_enable[] = {
			0x0001FFFFU,
			0x00000000U,
			0x00000000U,
			0xC0000000U};

	/* Send addresses of possible output */
	uint32_t output[] ={
			0x0000000DU,
			0x54000040U,
			0x9FD800C0U,
			0x9FE40140U,
			0x9FF80040U,
			0x9FFC0404U,
			0xA0FC0100U,
			0xA10C0100U,
			0xA11C00C0U,
			0xA1280902U,
			0xA2480040U,
			0xA24C0081U,
			0xA2540081U,
			0xA25C0081U,
			0xA2640081U,
			0xA26C0084U,
			0xA28C0082U};

	/* Update data size */
	for (i = 0; i < (uint32_t)(sizeof(output)/sizeof(uint32_t)); i++)
	{
		if ((output[i] == (uint8_t)0) 
                    || ((output_bh_enable[i/(uint32_t)32]
                         &((uint32_t)1 << (i%(uint32_t)32))) == (uint32_t)0))
		{
			continue;
		}

		bh_ptr = (union Block_header *)&(output[i]);
		if (((uint8_t)bh_ptr->type >= (uint8_t)0x1) 
                    && ((uint8_t)bh_ptr->type < (uint8_t)0x0d))
		{
			if ((bh_ptr->idx >= (uint16_t)0x54d0) 
                            && (bh_ptr->idx < (uint16_t)(0x54d0 + 960)))
			{
				bh_ptr->size = resolution;
			}	
			else 
			{
				bh_ptr->size = (uint8_t)(resolution 
                                  * (uint8_t)VL53L5CX_NB_TARGET_PER_ZONE);
			}

                        
			p_dev->data_read_size += bh_ptr->type * bh_ptr->size;
		}
		else
		{
			p_dev->data_read_size += bh_ptr->size;
		}

		p_dev->data_read_size += (uint32_t)4;
	}
	p_dev->data_read_size += (uint32_t)20;

	status |= vl53l5cx_dci_write_data(p_dev,
			(uint8_t*)&(output), 
                        VL53L5CX_DCI_OUTPUT_LIST, (uint16_t)sizeof(output));
        
        header_config = (uint64_t)i + (uint64_t)1;
	header_config = header_config << 32;
	header_config += (uint64_t)p_dev->data_read_size;

	status |= vl53l5cx_dci_write_data(p_dev, (uint8_t*)&(header_config),
			VL53L5CX_DCI_OUTPUT_CONFIG, 
                        (uint16_t)sizeof(header_config));
	status |= vl53l5cx_dci_write_data(p_dev, (uint8_t*)&(output_bh_enable),
			VL53L5CX_DCI_OUTPUT_ENABLES, 
                        (uint16_t)sizeof(output_bh_enable));

	return status;
}

uint8_t vl53l5cx_calibrate_xtalk(
		VL53L5CX_Configuration		*p_dev,
		uint16_t			reflectance_percent,
		uint8_t				nb_samples,
		uint16_t			distance_mm)
{
	uint16_t timeout = 0;
	uint8_t cmd[] = {0x00, 0x03, 0x00, 0x00};
	uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};
	uint8_t continue_loop = 1, status = VL53L5CX_STATUS_OK;

	uint8_t resolution, frequency, target_order, sharp_prct, ranging_mode;
	uint32_t integration_time_ms, xtalk_margin;
        
	uint16_t reflectance = reflectance_percent;
	uint8_t	samples = nb_samples;
	uint16_t distance = distance_mm;

	/* Get initial configuration */
	status |= vl53l5cx_get_resolution(p_dev, &resolution);
	status |= vl53l5cx_get_ranging_frequency_hz(p_dev, &frequency);
	status |= vl53l5cx_get_integration_time_ms(p_dev, &integration_time_ms);
	status |= vl53l5cx_get_sharpener_percent(p_dev, &sharp_prct);
	status |= vl53l5cx_get_target_order(p_dev, &target_order);
	status |= vl53l5cx_get_xtalk_margin(p_dev, &xtalk_margin);
	status |= vl53l5cx_get_ranging_mode(p_dev, &ranging_mode);

	/* Check input arguments validity */
	if(((reflectance < (uint16_t)1) || (reflectance > (uint16_t)99))
		|| ((distance < (uint16_t)600) || (distance > (uint16_t)3000))
		|| ((samples < (uint8_t)1) || (samples > (uint8_t)16)))
	{
		status |= VL53L5CX_STATUS_INVALID_PARAM;
	}
	else
	{
		status |= vl53l5cx_set_resolution(p_dev, 
				VL53L5CX_RESOLUTION_8X8);

		/* Send Xtalk calibration buffer */
                (void)memcpy(p_dev->temp_buffer, VL53L5CX_CALIBRATE_XTALK, 
                       sizeof(VL53L5CX_CALIBRATE_XTALK));
		status |= WrMulti(&(p_dev->platform), 0x2c28,
				p_dev->temp_buffer, 
                       (uint16_t)sizeof(VL53L5CX_CALIBRATE_XTALK));
		status |= _vl53l5cx_poll_for_answer(p_dev, 
				VL53L5CX_UI_CMD_STATUS, 0x3);

		/* Format input argument */
		reflectance = reflectance * (uint16_t)16;
		distance = distance * (uint16_t)4;

		/* Update required fields */
		status |= vl53l5cx_dci_replace_data(p_dev, p_dev->temp_buffer,
				VL53L5CX_DCI_CAL_CFG, 8, 
                                (uint8_t*)&distance, 2, 0x00);

		status |= vl53l5cx_dci_replace_data(p_dev, p_dev->temp_buffer,
				VL53L5CX_DCI_CAL_CFG, 8,
                                (uint8_t*)&reflectance, 2, 0x02);

		status |= vl53l5cx_dci_replace_data(p_dev, p_dev->temp_buffer,
				VL53L5CX_DCI_CAL_CFG, 8, 
                                (uint8_t*)&samples, 1, 0x04);

		/* Program output for Xtalk calibration */
		status |= _vl53l5cx_program_output_config(p_dev);

		/* Start ranging session */
		status |= WrMulti(&(p_dev->platform),
				VL53L5CX_UI_CMD_END - (uint16_t)(4 - 1),
				(uint8_t*)cmd, sizeof(cmd));
		status |= _vl53l5cx_poll_for_answer(p_dev, 
				VL53L5CX_UI_CMD_STATUS, 0x3);

		/* Wait for end of calibration */
		do {
			status |= RdMulti(&(p_dev->platform), 
                                          0x0, p_dev->temp_buffer, 4);
			if(p_dev->temp_buffer[0] != VL53L5CX_STATUS_ERROR)
			{
				/* Coverglass too good for Xtalk calibration */
				if((p_dev->temp_buffer[2] >= (uint8_t)0x7f) &&
				(((uint16_t)(p_dev->temp_buffer[3] & 
                                 (uint16_t)0x80) >> 7) == (uint16_t)1))
				{
					(void)memcpy(p_dev->xtalk_data, 
                                               p_dev->default_xtalk,
                                               VL53L5CX_XTALK_BUFFER_SIZE);
				}
				continue_loop = (uint8_t)0;
			}
			else if(timeout >= (uint16_t)400)
			{
				status |= VL53L5CX_STATUS_ERROR;
				continue_loop = (uint8_t)0;
			}
			else
			{
				timeout++;
				status |= WaitMs(&(p_dev->platform), 50);
			}

		}while (continue_loop == (uint8_t)1);
	}

	/* Save Xtalk data into the Xtalk buffer */
        (void)memcpy(p_dev->temp_buffer, VL53L5CX_GET_XTALK_CMD, 
               sizeof(VL53L5CX_GET_XTALK_CMD));
	status |= WrMulti(&(p_dev->platform), 0x2fb8,
			p_dev->temp_buffer, 
                        (uint16_t)sizeof(VL53L5CX_GET_XTALK_CMD));
	status |= _vl53l5cx_poll_for_answer(p_dev,VL53L5CX_UI_CMD_STATUS, 0x03);
	status |= RdMulti(&(p_dev->platform), VL53L5CX_UI_CMD_START,
			p_dev->temp_buffer, 
                        VL53L5CX_XTALK_BUFFER_SIZE + (uint16_t)4);

	(void)memcpy(&(p_dev->xtalk_data[0]), &(p_dev->temp_buffer[8]),
			VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8);
	(void)memcpy(&(p_dev->xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE 
                       - (uint16_t)8]), footer, sizeof(footer));

	/* Reset default buffer */
	status |= WrMulti(&(p_dev->platform), 0x2c34,
			p_dev->default_configuration,
			VL53L5CX_CONFIGURATION_SIZE);
	status |= _vl53l5cx_poll_for_answer(p_dev,VL53L5CX_UI_CMD_STATUS, 0x03);

	/* Reset initial configuration */
	status |= vl53l5cx_set_resolution(p_dev, resolution);
	status |= vl53l5cx_set_ranging_frequency_hz(p_dev, frequency);
	status |= vl53l5cx_set_integration_time_ms(p_dev, integration_time_ms);
	status |= vl53l5cx_set_sharpener_percent(p_dev, sharp_prct);
	status |= vl53l5cx_set_target_order(p_dev, target_order);
	status |= vl53l5cx_set_xtalk_margin(p_dev, xtalk_margin);
	status |= vl53l5cx_set_ranging_mode(p_dev, ranging_mode);

	return status;
}

uint8_t vl53l5cx_get_caldata_xtalk(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_xtalk_data)
{
	uint8_t status = VL53L5CX_STATUS_OK, resolution;
	uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};

	status |= vl53l5cx_get_resolution(p_dev, &resolution);
	status |= vl53l5cx_set_resolution(p_dev, VL53L5CX_RESOLUTION_8X8);

        (void)memcpy(p_dev->temp_buffer, VL53L5CX_GET_XTALK_CMD, 
               sizeof(VL53L5CX_GET_XTALK_CMD));
	status |= WrMulti(&(p_dev->platform), 0x2fb8,
			p_dev->temp_buffer,  sizeof(VL53L5CX_GET_XTALK_CMD));
	status |= _vl53l5cx_poll_for_answer(p_dev,VL53L5CX_UI_CMD_STATUS, 0x03);
	status |= RdMulti(&(p_dev->platform), VL53L5CX_UI_CMD_START,
			p_dev->temp_buffer, 
                        VL53L5CX_XTALK_BUFFER_SIZE + (uint16_t)4);

	(void)memcpy(&(p_xtalk_data[0]), &(p_dev->temp_buffer[8]),
			VL53L5CX_XTALK_BUFFER_SIZE-(uint16_t)8);
	(void)memcpy(&(p_xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8]),
			footer, sizeof(footer));

	status |= vl53l5cx_set_resolution(p_dev, resolution);

	return status;
}

uint8_t vl53l5cx_set_caldata_xtalk(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_xtalk_data)
{
	uint8_t resolution, status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_get_resolution(p_dev, &resolution);
	(void)memcpy(p_dev->xtalk_data, p_xtalk_data, VL53L5CX_XTALK_BUFFER_SIZE);
	status |= vl53l5cx_set_resolution(p_dev, resolution);

	return status;
}

uint8_t vl53l5cx_get_xtalk_margin(
		VL53L5CX_Configuration		*p_dev,
		uint32_t			*p_xtalk_margin)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(p_dev, (uint8_t*)p_dev->temp_buffer,
			VL53L5CX_DCI_XTALK_CFG, 16);

	(void)memcpy(p_xtalk_margin, p_dev->temp_buffer, 4);
	*p_xtalk_margin = *p_xtalk_margin/(uint32_t)2048;

	return status;
}

uint8_t vl53l5cx_set_xtalk_margin(
		VL53L5CX_Configuration		*p_dev,
		uint32_t			xtalk_margin)
{
	uint8_t status = VL53L5CX_STATUS_OK;
        uint32_t margin_kcps = xtalk_margin;

	if(margin_kcps > (uint32_t)10000)
	{
		status |= VL53L5CX_STATUS_INVALID_PARAM;
	}
	else
	{
		margin_kcps = margin_kcps*(uint32_t)2048;
		status |= vl53l5cx_dci_replace_data(p_dev, p_dev->temp_buffer,
				VL53L5CX_DCI_XTALK_CFG, 16, 
                                (uint8_t*)&margin_kcps, 4, 0x00);
	}

	return status;
}
