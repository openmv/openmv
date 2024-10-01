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

#ifndef VL53L5CX_API_H_
#define VL53L5CX_API_H_

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION < 6010050)
#pragma anon_unions
#endif



#include "platform.h"

/**
 * @brief Current driver version.
 */

#define VL53L5CX_API_REVISION			"VL53L5CX_2.0.0"

/**
 * @brief Default I2C address of VL53L5CX sensor. Can be changed using function
 * vl53l5cx_set_i2c_address() function is called.
 */

#define VL53L5CX_DEFAULT_I2C_ADDRESS	        ((uint16_t)0x52)

/**
 * @brief Macro VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8 allows
 * setting sensor in 4x4 mode or 8x8 mode, using function
 * vl53l5cx_set_resolution().
 */

#define VL53L5CX_RESOLUTION_4X4			((uint8_t) 16U)
#define VL53L5CX_RESOLUTION_8X8			((uint8_t) 64U)


/**
 * @brief Macro VL53L5CX_TARGET_ORDER_STRONGEST or VL53L5CX_TARGET_ORDER_CLOSEST
 *	are used to select the target order for data output.
 */

#define VL53L5CX_TARGET_ORDER_CLOSEST		((uint8_t) 1U)
#define VL53L5CX_TARGET_ORDER_STRONGEST		((uint8_t) 2U)

/**
 * @brief Macro VL53L5CX_RANGING_MODE_CONTINUOUS and
 * VL53L5CX_RANGING_MODE_AUTONOMOUS are used to change the ranging mode.
 * Autonomous mode can be used to set a precise integration time, whereas
 * continuous is always maximum.
 */

#define VL53L5CX_RANGING_MODE_CONTINUOUS	((uint8_t) 1U)
#define VL53L5CX_RANGING_MODE_AUTONOMOUS	((uint8_t) 3U)

/**
 * @brief The default power mode is VL53L5CX_POWER_MODE_WAKEUP. User can choose
 * the mode VL53L5CX_POWER_MODE_SLEEP to save power consumption is the device
 * is not used. The low power mode retains the firmware and the configuration.
 * Both modes can be changed using function vl53l5cx_set_power_mode().
 */

#define VL53L5CX_POWER_MODE_SLEEP		((uint8_t) 0U)
#define VL53L5CX_POWER_MODE_WAKEUP		((uint8_t) 1U)

/**
 * @brief Macro VL53L5CX_STATUS_OK indicates that VL53L5 sensor has no error.
 * Macro VL53L5CX_STATUS_ERROR indicates that something is wrong (value,
 * I2C access, ...). Macro VL53L5CX_MCU_ERROR is used to indicate a MCU issue.
 */

#define VL53L5CX_STATUS_OK			((uint8_t) 0U)
#define VL53L5CX_STATUS_TIMEOUT_ERROR		((uint8_t) 1U)
#define VL53L5CX_STATUS_CORRUPTED_FRAME		((uint8_t) 2U)
#define VL53L5CX_STATUS_CRC_CSUM_FAILED		((uint8_t) 3U)
#define VL53L5CX_STATUS_XTALK_FAILED		((uint8_t) 4U)
#define VL53L5CX_MCU_ERROR			((uint8_t) 66U)
#define VL53L5CX_STATUS_INVALID_PARAM		((uint8_t) 127U)
#define VL53L5CX_STATUS_ERROR			((uint8_t) 255U)

/**
 * @brief Definitions for Range results block headers
 */

#if VL53L5CX_NB_TARGET_PER_ZONE == 1

#define VL53L5CX_START_BH				((uint32_t)0x0000000DU)
#define VL53L5CX_METADATA_BH			((uint32_t)0x54B400C0U)
#define VL53L5CX_COMMONDATA_BH			((uint32_t)0x54C00040U)
#define VL53L5CX_AMBIENT_RATE_BH		((uint32_t)0x54D00104U)
#define VL53L5CX_SPAD_COUNT_BH			((uint32_t)0x55D00404U)
#define VL53L5CX_NB_TARGET_DETECTED_BH	((uint32_t)0xDB840401U)
#define VL53L5CX_SIGNAL_RATE_BH			((uint32_t)0xDBC40404U)
#define VL53L5CX_RANGE_SIGMA_MM_BH		((uint32_t)0xDEC40402U)
#define VL53L5CX_DISTANCE_BH			((uint32_t)0xDF440402U)
#define VL53L5CX_REFLECTANCE_BH			((uint32_t)0xE0440401U)
#define VL53L5CX_TARGET_STATUS_BH		((uint32_t)0xE0840401U)
#define VL53L5CX_MOTION_DETECT_BH		((uint32_t)0xD85808C0U)

#define VL53L5CX_METADATA_IDX			((uint16_t)0x54B4U)
#define VL53L5CX_SPAD_COUNT_IDX			((uint16_t)0x55D0U)
#define VL53L5CX_AMBIENT_RATE_IDX		((uint16_t)0x54D0U)
#define VL53L5CX_NB_TARGET_DETECTED_IDX	((uint16_t)0xDB84U)
#define VL53L5CX_SIGNAL_RATE_IDX		((uint16_t)0xDBC4U)
#define VL53L5CX_RANGE_SIGMA_MM_IDX		((uint16_t)0xDEC4U)
#define VL53L5CX_DISTANCE_IDX			((uint16_t)0xDF44U)
#define VL53L5CX_REFLECTANCE_EST_PC_IDX	((uint16_t)0xE044U)
#define VL53L5CX_TARGET_STATUS_IDX		((uint16_t)0xE084U)
#define VL53L5CX_MOTION_DETEC_IDX		((uint16_t)0xD858U)

#else
#define VL53L5CX_START_BH				((uint32_t)0x0000000DU)
#define VL53L5CX_METADATA_BH			((uint32_t)0x54B400C0U)
#define VL53L5CX_COMMONDATA_BH			((uint32_t)0x54C00040U)
#define VL53L5CX_AMBIENT_RATE_BH		((uint32_t)0x54D00104U)
#define VL53L5CX_NB_TARGET_DETECTED_BH	((uint32_t)0x57D00401U)
#define VL53L5CX_SPAD_COUNT_BH			((uint32_t)0x55D00404U)
#define VL53L5CX_SIGNAL_RATE_BH			((uint32_t)0x58900404U)
#define VL53L5CX_RANGE_SIGMA_MM_BH		((uint32_t)0x64900402U)
#define VL53L5CX_DISTANCE_BH			((uint32_t)0x66900402U)
#define VL53L5CX_REFLECTANCE_BH			((uint32_t)0x6A900401U)
#define VL53L5CX_TARGET_STATUS_BH		((uint32_t)0x6B900401U)
#define VL53L5CX_MOTION_DETECT_BH		((uint32_t)0xCC5008C0U)

#define VL53L5CX_METADATA_IDX			((uint16_t)0x54B4U)
#define VL53L5CX_SPAD_COUNT_IDX			((uint16_t)0x55D0U)
#define VL53L5CX_AMBIENT_RATE_IDX		((uint16_t)0x54D0U)
#define VL53L5CX_NB_TARGET_DETECTED_IDX	((uint16_t)0x57D0U)
#define VL53L5CX_SIGNAL_RATE_IDX		((uint16_t)0x5890U)
#define VL53L5CX_RANGE_SIGMA_MM_IDX		((uint16_t)0x6490U)
#define VL53L5CX_DISTANCE_IDX			((uint16_t)0x6690U)
#define VL53L5CX_REFLECTANCE_EST_PC_IDX	((uint16_t)0x6A90U)
#define VL53L5CX_TARGET_STATUS_IDX		((uint16_t)0x6B90U)
#define VL53L5CX_MOTION_DETEC_IDX		((uint16_t)0xCC50U)
#endif


/**
 * @brief Inner Macro for API. Not for user, only for development.
 */

#define VL53L5CX_NVM_DATA_SIZE			((uint16_t)492U)
#define VL53L5CX_CONFIGURATION_SIZE		((uint16_t)972U)
#define VL53L5CX_OFFSET_BUFFER_SIZE		((uint16_t)488U)
#define VL53L5CX_XTALK_BUFFER_SIZE		((uint16_t)776U)

#define VL53L5CX_DCI_ZONE_CONFIG		((uint16_t)0x5450U)
#define VL53L5CX_DCI_FREQ_HZ			((uint16_t)0x5458U)
#define VL53L5CX_DCI_INT_TIME			((uint16_t)0x545CU)
#define VL53L5CX_DCI_FW_NB_TARGET		((uint16_t)0x5478)
#define VL53L5CX_DCI_RANGING_MODE		((uint16_t)0xAD30U)
#define VL53L5CX_DCI_DSS_CONFIG			((uint16_t)0xAD38U)
#define VL53L5CX_DCI_VHV_CONFIG			((uint16_t)0xAD60U)
#define VL53L5CX_DCI_TARGET_ORDER		((uint16_t)0xAE64U)
#define VL53L5CX_DCI_SHARPENER			((uint16_t)0xAED8U)
#define VL53L5CX_DCI_INTERNAL_CP		((uint16_t)0xB39CU)
#define VL53L5CX_DCI_SYNC_PIN			((uint16_t)0xB5F0U)
#define VL53L5CX_DCI_MOTION_DETECTOR_CFG ((uint16_t)0xBFACU)
#define VL53L5CX_DCI_SINGLE_RANGE		((uint16_t)0xD964U)
#define VL53L5CX_DCI_OUTPUT_CONFIG		((uint16_t)0xD968U)
#define VL53L5CX_DCI_OUTPUT_ENABLES		((uint16_t)0xD970U)
#define VL53L5CX_DCI_OUTPUT_LIST		((uint16_t)0xD980U)
#define VL53L5CX_DCI_PIPE_CONTROL		((uint16_t)0xDB80U)
#define VL53L5CX_GLARE_FILTER			((uint16_t)0xE108U)


#define VL53L5CX_UI_CMD_STATUS			((uint16_t)0x2C00U)
#define VL53L5CX_UI_CMD_START			((uint16_t)0x2C04U)
#define VL53L5CX_UI_CMD_END				((uint16_t)0x2FFFU)

/**
 * @brief Inner values for API. Max buffer size depends of the selected output.
 */

#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
#define L5CX_AMB_SIZE	260U
#else
#define L5CX_AMB_SIZE	0U
#endif

#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
#define L5CX_SPAD_SIZE	260U
#else
#define L5CX_SPAD_SIZE	0U
#endif

#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
#define L5CX_NTAR_SIZE	68U
#else
#define L5CX_NTAR_SIZE	0U
#endif

#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
#define L5CX_SPS_SIZE ((256U * VL53L5CX_NB_TARGET_PER_ZONE) + 4U)
#else
#define L5CX_SPS_SIZE	0U
#endif

#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
#define L5CX_SIGR_SIZE ((128U * VL53L5CX_NB_TARGET_PER_ZONE) + 4U)
#else
#define L5CX_SIGR_SIZE	0U
#endif

#ifndef VL53L5CX_DISABLE_DISTANCE_MM
#define L5CX_DIST_SIZE ((128U * VL53L5CX_NB_TARGET_PER_ZONE) + 4U)
#else
#define L5CX_DIST_SIZE	0U
#endif

#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
#define L5CX_RFLEST_SIZE ((64U *VL53L5CX_NB_TARGET_PER_ZONE) + 4U)
#else
#define L5CX_RFLEST_SIZE	0U
#endif

#ifndef VL53L5CX_DISABLE_TARGET_STATUS
#define L5CX_STA_SIZE ((64U  *VL53L5CX_NB_TARGET_PER_ZONE) + 4U)
#else
#define L5CX_STA_SIZE	0U
#endif

#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
#define L5CX_MOT_SIZE	144U
#else
#define L5CX_MOT_SIZE	0U
#endif

/**
 * @brief Macro VL53L5CX_MAX_RESULTS_SIZE indicates the maximum size used by
 * output through I2C. Value 40 corresponds to headers + meta-data + common-data
 * and 20 corresponds to the footer.
 */

#define VL53L5CX_MAX_RESULTS_SIZE ( 40U \
	+ L5CX_AMB_SIZE + L5CX_SPAD_SIZE + L5CX_NTAR_SIZE + L5CX_SPS_SIZE \
	+ L5CX_SIGR_SIZE + L5CX_DIST_SIZE + L5CX_RFLEST_SIZE + L5CX_STA_SIZE \
	+ L5CX_MOT_SIZE + 20U)

/**
 * @brief Macro VL53L5CX_TEMPORARY_BUFFER_SIZE can be used to know the size of
 * the temporary buffer. The minimum size is 1024, and the maximum depends of
 * the output configuration.
 */

#if VL53L5CX_MAX_RESULTS_SIZE < 1024U
#define VL53L5CX_TEMPORARY_BUFFER_SIZE ((uint32_t) 1024U)
#else
#define VL53L5CX_TEMPORARY_BUFFER_SIZE ((uint32_t) VL53L5CX_MAX_RESULTS_SIZE)
#endif


/**
 * @brief Structure VL53L5CX_Configuration contains the sensor configuration.
 * User MUST not manually change these field, except for the sensor address.
 */

typedef struct
{
	/* Platform, filled by customer into the 'platform.h' file */
	VL53L5CX_Platform	platform;
	/* Results streamcount, value auto-incremented at each range */
	uint8_t		        streamcount;
	/* Size of data read though I2C */
	uint32_t	        data_read_size;
	/* Address of default configuration buffer */
	uint8_t		        *default_configuration;
	/* Address of default Xtalk buffer */
	uint8_t		        *default_xtalk;
	/* Offset buffer */
	uint8_t		        offset_data[VL53L5CX_OFFSET_BUFFER_SIZE];
	/* Xtalk buffer */
	uint8_t		        xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE];
	/* Temporary buffer used for internal driver processing */
	 uint8_t	        temp_buffer[VL53L5CX_TEMPORARY_BUFFER_SIZE];
	/* Auto-stop flag for stopping the sensor */
	uint8_t				is_auto_stop_enabled;
} VL53L5CX_Configuration;


/**
 * @brief Structure VL53L5CX_ResultsData contains the ranging results of
 * VL53L5CX. If user wants more than 1 target per zone, the results can be split
 * into 2 sub-groups :
 * - Per zone results. These results are common to all targets (ambient_per_spad
 * , nb_target_detected and nb_spads_enabled).
 * - Per target results : These results are different relative to the detected
 * target (signal_per_spad, range_sigma_mm, distance_mm, reflectance,
 * target_status).
 */

typedef struct
{
	/* Internal sensor silicon temperature */
	int8_t silicon_temp_degc;

	/* Ambient noise in kcps/spads */
#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
	uint32_t ambient_per_spad[VL53L5CX_RESOLUTION_8X8];
#endif

	/* Number of valid target detected for 1 zone */
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
	uint8_t nb_target_detected[VL53L5CX_RESOLUTION_8X8];
#endif

	/* Number of spads enabled for this ranging */
#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
	uint32_t nb_spads_enabled[VL53L5CX_RESOLUTION_8X8];
#endif

	/* Signal returned to the sensor in kcps/spads */
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
	uint32_t signal_per_spad[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Sigma of the current distance in mm */
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
	uint16_t range_sigma_mm[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Measured distance in mm */
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
	int16_t distance_mm[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Estimated reflectance in percent */
#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
	uint8_t reflectance[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Status indicating the measurement validity (5 & 9 means ranging OK)*/
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
	uint8_t target_status[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Motion detector results */
#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
	struct
	{
		uint32_t global_indicator_1;
		uint32_t global_indicator_2;
		uint8_t	 status;
		uint8_t	 nb_of_detected_aggregates;
		uint8_t	 nb_of_aggregates;
		uint8_t	 spare;
		uint32_t motion[32];
	} motion_indicator;
#endif

} VL53L5CX_ResultsData;


union Block_header {
	uint32_t bytes;
	struct {
		uint32_t type : 4;
		uint32_t size : 12;
		uint32_t idx : 16;
	};
};

uint8_t vl53l5cx_is_alive(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_is_alive);

/**
 * @brief Mandatory function used to initialize the sensor. This function must
 * be called after a power on, to load the firmware into the VL53L5CX. It takes
 * a few hundred milliseconds.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @return (uint8_t) status : 0 if initialization is OK.
 */

uint8_t vl53l5cx_init(
		VL53L5CX_Configuration		*p_dev);

/**
 * @brief This function is used to change the I2C address of the sensor. If
 * multiple VL53L5 sensors are connected to the same I2C line, all other LPn
 * pins needs to be set to Low. The default sensor address is 0x52.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint16_t) i2c_address : New I2C address.
 * @return (uint8_t) status : 0 if new address is OK
 */

uint8_t vl53l5cx_set_i2c_address(
		VL53L5CX_Configuration		*p_dev,
		uint16_t			i2c_address);

/**
 * @brief This function is used to get the current sensor power mode.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *p_power_mode : Current power mode. The value of this
 * pointer is equal to 0 if the sensor is in low power,
 * (VL53L5CX_POWER_MODE_SLEEP), or 1 if sensor is in standard mode
 * (VL53L5CX_POWER_MODE_WAKEUP).
 * @return (uint8_t) status : 0 if power mode is OK
 */

uint8_t vl53l5cx_get_power_mode(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_power_mode);

/**
 * @brief This function is used to set the sensor in Low Power mode, for
 * example if the sensor is not used during a long time. The macro
 * VL53L5CX_POWER_MODE_SLEEP can be used to enable the low power mode. When user
 * want to restart the sensor, he can use macro VL53L5CX_POWER_MODE_WAKEUP.
 * Please ensure that the device is not streaming before calling the function.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) power_mode : Selected power mode (VL53L5CX_POWER_MODE_SLEEP
 * or VL53L5CX_POWER_MODE_WAKEUP)
 * @return (uint8_t) status : 0 if power mode is OK, or 127 if power mode
 * requested by user is not valid.
 */

uint8_t vl53l5cx_set_power_mode(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				power_mode);

/**
 * @brief This function starts a ranging session. When the sensor streams, host
 * cannot change settings 'on-the-fly'.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @return (uint8_t) status : 0 if start is OK.
 */

uint8_t vl53l5cx_start_ranging(
		VL53L5CX_Configuration		*p_dev);

/**
 * @brief This function stops the ranging session. It must be used when the
 * sensor streams, after calling vl53l5cx_start_ranging().
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @return (uint8_t) status : 0 if stop is OK
 */

uint8_t vl53l5cx_stop_ranging(
		VL53L5CX_Configuration		*p_dev);

/**
 * @brief This function checks if a new data is ready by polling I2C. If a new
 * data is ready, a flag will be raised.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *p_isReady : Value of this pointer be updated to 0 if data
 * is not ready, or 1 if a new data is ready.
 * @return (uint8_t) status : 0 if I2C reading is OK
 */

uint8_t vl53l5cx_check_data_ready(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_isReady);

/**
 * @brief This function gets the ranging data, using the selected output and the
 * resolution.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (VL53L5CX_ResultsData) *p_results : VL53L5 results structure.
 * @return (uint8_t) status : 0 data are successfully get.
 */

uint8_t vl53l5cx_get_ranging_data(
		VL53L5CX_Configuration		*p_dev,
		VL53L5CX_ResultsData		*p_results);

/**
 * @brief This function gets the current resolution (4x4 or 8x8).
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *p_resolution : Value of this pointer will be equal to 16
 * for 4x4 mode, and 64 for 8x8 mode.
 * @return (uint8_t) status : 0 if resolution is OK.
 */

uint8_t vl53l5cx_get_resolution(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_resolution);

/**
 * @brief This function sets a new resolution (4x4 or 8x8).
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) resolution : Use macro VL53L5CX_RESOLUTION_4X4 or
 * VL53L5CX_RESOLUTION_8X8 to set the resolution.
 * @return (uint8_t) status : 0 if set resolution is OK.
 */

uint8_t vl53l5cx_set_resolution(
		VL53L5CX_Configuration		 *p_dev,
		uint8_t                         resolution);

/**
 * @brief This function gets the current ranging frequency in Hz. Ranging
 * frequency corresponds to the time between each measurement.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *p_frequency_hz: Contains the ranging frequency in Hz.
 * @return (uint8_t) status : 0 if ranging frequency is OK.
 */

uint8_t vl53l5cx_get_ranging_frequency_hz(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_frequency_hz);

/**
 * @brief This function sets a new ranging frequency in Hz. Ranging frequency
 * corresponds to the measurements frequency. This setting depends of
 * the resolution, so please select your resolution before using this function.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) frequency_hz : Contains the ranging frequency in Hz.
 * - For 4x4, min and max allowed values are : [1;60]
 * - For 8x8, min and max allowed values are : [1;15]
 * @return (uint8_t) status : 0 if ranging frequency is OK, or 127 if the value
 * is not correct.
 */

uint8_t vl53l5cx_set_ranging_frequency_hz(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				frequency_hz);

/**
 * @brief This function gets the current integration time in ms.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint32_t) *p_time_ms: Contains integration time in ms.
 * @return (uint8_t) status : 0 if integration time is OK.
 */

uint8_t vl53l5cx_get_integration_time_ms(
		VL53L5CX_Configuration		*p_dev,
		uint32_t			*p_time_ms);

/**
 * @brief This function sets a new integration time in ms. Integration time must
 * be computed to be lower than the ranging period, for a selected resolution.
 * Please note that this function has no impact on ranging mode continous.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint32_t) time_ms : Contains the integration time in ms. For all
 * resolutions and frequency, the minimum value is 2ms, and the maximum is
 * 1000ms.
 * @return (uint8_t) status : 0 if set integration time is OK.
 */

uint8_t vl53l5cx_set_integration_time_ms(
		VL53L5CX_Configuration		*p_dev,
		uint32_t			integration_time_ms);

/**
 * @brief This function gets the current sharpener in percent. Sharpener can be
 * changed to blur more or less zones depending of the application.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint32_t) *p_sharpener_percent: Contains the sharpener in percent.
 * @return (uint8_t) status : 0 if get sharpener is OK.
 */

uint8_t vl53l5cx_get_sharpener_percent(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_sharpener_percent);

/**
 * @brief This function sets a new sharpener value in percent. Sharpener can be
 * changed to blur more or less zones depending of the application. Min value is
 * 0 (disabled), and max is 99.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint32_t) sharpener_percent : Value between 0 (disabled) and 99%.
 * @return (uint8_t) status : 0 if set sharpener is OK.
 */

uint8_t vl53l5cx_set_sharpener_percent(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				sharpener_percent);

/**
 * @brief This function gets the current target order (closest or strongest).
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *p_target_order: Contains the target order.
 * @return (uint8_t) status : 0 if get target order is OK.
 */

uint8_t vl53l5cx_get_target_order(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_target_order);

/**
 * @brief This function sets a new target order. Please use macros
 * VL53L5CX_TARGET_ORDER_STRONGEST and VL53L5CX_TARGET_ORDER_CLOSEST to define
 * the new output order. By default, the sensor is configured with the strongest
 * output.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) target_order : Required target order.
 * @return (uint8_t) status : 0 if set target order is OK, or 127 if target
 * order is unknown.
 */

uint8_t vl53l5cx_set_target_order(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				target_order);

/**
 * @brief This function is used to get the ranging mode. Two modes are
 * available using ULD : Continuous and autonomous. The default
 * mode is Autonomous.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *p_ranging_mode : current ranging mode
 * @return (uint8_t) status : 0 if get ranging mode is OK.
 */

uint8_t vl53l5cx_get_ranging_mode(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*p_ranging_mode);

/**
 * @brief This function is used to set the ranging mode. Two modes are
 * available using ULD : Continuous and autonomous. The default
 * mode is Autonomous.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) ranging_mode : Use macros VL53L5CX_RANGING_MODE_CONTINUOUS,
 * VL53L5CX_RANGING_MODE_CONTINUOUS.
 * @return (uint8_t) status : 0 if set ranging mode is OK.
 */

uint8_t vl53l5cx_set_ranging_mode(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				ranging_mode);

/**
 * @brief This function is used to disable the VCSEL charge pump
 * This optimizes the power consumption of the device
 * To be used only if AVDD = 3.3V
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 */
uint8_t vl53l5cx_enable_internal_cp(
		VL53L5CX_Configuration          *p_dev);


/**
 * @brief This function is used to disable the VCSEL charge pump
 * This optimizes the power consumption of the device
 * To be used only if AVDD = 3.3V
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 */
uint8_t vl53l5cx_disable_internal_cp(
 	      VL53L5CX_Configuration          *p_dev);

/**
 * @brief This function is used to get the number of frames between 2 temperature
 * compensation.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint32_t) *p_repeat_count : Number of frames before next temperature
 * compensation. Set to 0 to disable the feature (default configuration).
 */
uint8_t vl53l5cx_get_VHV_repeat_count(
		VL53L5CX_Configuration *p_dev,
		uint32_t *p_repeat_count);

/**
 * @brief This function is used to set a periodic temperature compensation. By
 * setting a repeat count different to 0 the firmware automatically runs a
 * temperature calibration every N frames.
 * default the repeat count is set to 0
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint32_t) repeat_count : Number of frames between temperature
 * compensation. Set to 0 to disable the feature (default configuration).
 */
uint8_t vl53l5cx_set_VHV_repeat_count(
		VL53L5CX_Configuration *p_dev,
		uint32_t repeat_count);

/**
 * @brief This function can be used to read 'extra data' from DCI. Using a known
 * index, the function fills the casted structure passed in argument.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *data : This field can be a casted structure, or a simple
 * array. Please note that the FW only accept data of 32 bits. So field data can
 * only have a size of 32, 64, 96, 128, bits ....
 * @param (uint32_t) index : Index of required value.
 * @param (uint16_t)*data_size : This field must be the structure or array size
 * (using sizeof() function).
 * @return (uint8_t) status : 0 if OK
 */

uint8_t vl53l5cx_dci_read_data(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size);

/**
 * @brief This function can be used to write 'extra data' to DCI. The data can
 * be simple data, or casted structure.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *data : This field can be a casted structure, or a simple
 * array. Please note that the FW only accept data of 32 bits. So field data can
 * only have a size of 32, 64, 96, 128, bits ..
 * @param (uint32_t) index : Index of required value.
 * @param (uint16_t)*data_size : This field must be the structure or array size
 * (using sizeof() function).
 * @return (uint8_t) status : 0 if OK
 */

uint8_t vl53l5cx_dci_write_data(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size);

/**
 * @brief This function can be used to replace 'extra data' in DCI. The data can
 * be simple data, or casted structure.
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (uint8_t) *data : This field can be a casted structure, or a simple
 * array. Please note that the FW only accept data of 32 bits. So field data can
 * only have a size of 32, 64, 96, 128, bits ..
 * @param (uint32_t) index : Index of required value.
 * @param (uint16_t)*data_size : This field must be the structure or array size
 * (using sizeof() function).
 * @param (uint8_t) *new_data : Contains the new fields.
 * @param (uint16_t) new_data_size : New data size.
 * @param (uint16_t) new_data_pos : New data position into the buffer.
 * @return (uint8_t) status : 0 if OK
 */

uint8_t vl53l5cx_dci_replace_data(
		VL53L5CX_Configuration		*p_dev,
		uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size,
		uint8_t				*new_data,
		uint16_t			new_data_size,
		uint16_t			new_data_pos);

#endif //VL53L5CX_API_H_
