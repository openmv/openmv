/**
 ******************************************************************************
 * @file    vl53l9.h
 * @author  IMD Software Team
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 * @warning
 *
 * Use of control adjustments, or procedures other than those specified in
 * product datasheet may result in hazardous product behavior.
 *
 */

#ifndef VL53L9_H
#define VL53L9_H

#ifdef __cplusplus
extern "C" {
#endif

/* exported headers **********************************************************/

#include <stdbool.h>
#include <stdint.h>

/* exported symbols **********************************************************/

/**
 * @defgroup VL53L9_VERSION
 * @{
 */
#define VL53L9_CORE_MAJOR (1U)
#define VL53L9_CORE_MINOR (0U)
#define VL53L9_CORE_PATCH (0U)
/** @} */

#define VL53L9_DEFAULT_ADDRESS (0x52)

/**
 * @defgroup VL53L9_ERROR
 * @{
 */
#define VL53L9_ERROR_NONE              (0)
#define VL53L9_ERROR_PLATFORM          (-1)
#define VL53L9_ERROR_INVALID_PARAM     (-2)
#define VL53L9_ERROR_INVALID_STATE     (-3)
#define VL53L9_ERROR_INVALID_OPERATION (-4)
#define VL53L9_ERROR_TIMEOUT           (-5)
#define VL53L9_ERROR_INTERNAL          (-6)
/** @} */

// output interface protocol
#define VL53L9_OUTPUT_CSI2 (0U)
#define VL53L9_OUTPUT_I3C  (1U)

// memory page size
#define VL53L9_STATUS_SIZE     (100U)
#define VL53L9_CALIB_DATA_SIZE (2332U)

/* exported types ************************************************************/

/**
 * @enum vl53l9_vdda_t
 */
typedef enum {
    VDDA_2V8 = 0U,
    VDDA_3V3 = 1U,
} vl53l9_vdda_t;

/**
 * @enum vl53l9_vddio_t
 */
typedef enum {
    VDDIO_1V2 = 0U,
    VDDIO_1V8 = 1U,
} vl53l9_vddio_t;

/**
 * @enum vl53l9_power_mode_t
 */
typedef enum {
    VL53L9_POWER_REGULAR = 0U,
    VL53L9_POWER_LOW = 1U,
    VL53L9_POWER_ULTRA_LOW = 2U,
} vl53l9_power_mode_t;

/**
 * @enum vl53l9_sync_mode_t
 */
typedef enum {
    VL53L9_SYNC_SLAVE = 0U,
    VL53L9_SYNC_MANUAL = 1U,
    VL53L9_SYNC_AUTONOMOUS = 2U,
} vl53l9_sync_mode_t;

/**
 * @enum vl53l9_context_t
 */
typedef enum {
    VL53L9_CONTEXT_SHORT = 0U,
    VL53L9_CONTEXT_LONG = 1U,
} vl53l9_context_t;

/**
 * @struct vl53l9_hw_config_t
 */
typedef struct {
    bool output_interface;   /**< false = CSI2 - true = I3C */
    bool signaling_mode;     /**< false = in-band interrupt (I3C) - true = interrupt pad */
    bool interrupt_pad_mode; /**< false = cmos - true = open drain */
    bool csi_status_line_force_width;
    uint32_t csi_data_rate;
    uint8_t csi_virtual_channel;
    uint8_t csi_status_line_datatype;
    uint8_t csi_frame_datatype;
    uint16_t csi_frame_height;
    uint16_t csi_frame_width;
} vl53l9_hw_config_t;

typedef struct {
    uint8_t fsm;
    uint8_t command;
    uint16_t firmware;
    struct {
        uint8_t vhv_overvoltage : 1;
        uint8_t vhv_undervoltage : 1;
        uint8_t spad_supply_overload : 1;
        uint8_t hvboost_limit : 1;
        uint8_t sof_outside_blanking : 1;
        uint8_t pll_lock : 1;
        uint8_t ref_array : 1;
        uint8_t internal_fw : 1;
    } error;
    uint8_t laser_driver[5];
} vl53l9_status_t;

/* exported functions ********************************************************/

/**
 * @brief Boot the device and initialize it with default settings
 * @param[in] p_dev Opaque pointer used for register level operations
 * @note This method is intended to be called after a hardware reset
 * @return See @ref VL53L9_ERROR
 *
 */
int vl53l9_init(void *const p_dev);

/**
 * @brief Update the device's communication configuration
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] address 7-bit static I2C address
 * @param[in] instance_id  I3C instance ID. This is a 4-bit value
 * @note The instance ID is used to differentiate between multiple instances of the device in a system.
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_set_com_config(void *const p_dev, uint8_t address, uint8_t instance_id);

/**
 * @brief Retrieve the device's communication configuration
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_address Pointer to the 7-bit static I2C address
 * @param[out] p_instance_id  Pointer to the I3C instance ID (encoded over 4-bit)
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_com_config(void *const p_dev, uint8_t *p_address, uint8_t *p_instance_id);

/**
 * @brief Retrieve the device's model identifier
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_id Pointer to the variable where the data will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_device_id(void *const p_dev, uint32_t *p_id);

/**
 * @brief Retrieve calibration data (meant to be fed to the postprocessing pipeline)
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] otp Array where the raw calibration data will be stored
 *
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_calib_data(void *const p_dev, uint8_t *p_buffer);

/**
 * @brief Update hardware-related parameters
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_conf Pointer to the variable where the data will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_hw_config(void *const p_dev, vl53l9_hw_config_t *p_config);

/**
 * @brief Retrieve the current hardware-related parameters
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] conf Hardware configuration to be applied
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_set_hw_config(void *const p_dev, vl53l9_hw_config_t config);

/**
 * @brief Retrieve the current power mode
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_mode Pointer to the variable where the data will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_power_mode(void *const p_dev, vl53l9_power_mode_t *p_mode);

/**
 * @brief Update the device's power mode
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] mode Power mode to be applied
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_set_power_mode(void *const p_dev, vl53l9_power_mode_t mode);

/**
 * @brief Retrieve the current synchronization mode
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_mode Pointer to the variable where the data will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_sync_mode(void *const p_dev, vl53l9_sync_mode_t *p_mode);

/**
 * @brief Update the device's synchronization mode
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] mode Synchronization mode to be applied
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_set_sync_mode(void *const p_dev, vl53l9_sync_mode_t mode);

/**
 * @brief Retrieve the current frame period in microseconds
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_frame_period_us Pointer to the variable where the data will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_frame_period(void *const p_dev, uint32_t *p_frame_period_us);

/**
 * @brief Update the device's frame period
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] frame_period_us Frame period in microseconds
 * @note This setting is relevant only in autonomous synchronization mode
 * @note The provided frame period value must be between 10 msec and 1 sec
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_set_frame_period(void *const p_dev, uint32_t frame_period_us);

/**
 * @brief Retrieve the current active context
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_context Pointer to the variable where the data will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_context(void *const p_dev, vl53l9_context_t *p_context);

/**
 * @brief Update the device's active context
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] context Context to be selected
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_set_context(void *const p_dev, vl53l9_context_t context);

/**
 * @brief Retrieve the current configuration of a given context
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] context Selected context
 * @param[out] p_binning Pointer to the variable where the data will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_binning(void *const p_dev, vl53l9_context_t context, uint8_t *p_binning);

/**
 * @brief Update the binning factor for a given context
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] context Selected context
 * @param[in] binning Binning factor, accepted values are: 2, 3, 4, 6, 8, 12, 24
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_set_binning(void *const p_dev, vl53l9_context_t context, uint8_t binning);

/**
 * @brief Retrieve the current exposure time (in milliseconds) for a given context
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] context Selected context
 * @param[out] p_exposure_ms Pointer to the variable where the current exposure time will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_exposure(void *const p_dev, vl53l9_context_t context, uint16_t *p_exposure_ms);

/**
 * @brief Update the exposure time for a given context
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[in] context Selected context
 * @param[in] exposure_ms Exposure time to be applied (in milliseconds)
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_set_exposure(void *const p_dev, vl53l9_context_t context, uint16_t exposure_ms);

/**
 * @brief Start streaming
 * @param[in] p_dev Opaque pointer used for register level operations
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_start(void *const p_dev);

/**
 * @brief Stop streaming
 * @param[in] p_dev Opaque pointer used for register level operations
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_stop(void *const p_dev);

/**
 * @brief Triggers a new frame when the devices is streaming in manual synchronization mode
 * @param[in] p_dev Opaque pointer used for register level operations
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_trigger_frame(void *const p_dev);

/**
 * @brief Check whether a frame is ready to be retrieved
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_is_ready Pointer to the variable where the data will be stored
 * @note This function is intended to be used when data is output through the I3C interface
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_poll_frame(void *const p_dev, uint8_t *p_is_ready);

/**
 * @brief Retrieve frame data
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_buffer Pointer to the buffer where the raw data will be stored
 * @param[in] size Size of the buffer in bytes
 * @note This function is intended to be used when data is output through the I3C interface
 * @note The buffer size must match one of the values returned by @ref vl53l9_get_raw_buffer_size
 * @note If the provided size is 0, the function will acknowledge the frame without reading the data
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_frame(void *const p_dev, uint8_t *p_buffer, uint16_t size);

/**
 * @brief Asynchronously retrieves a frame from the VL53L9 sensor
 *
 * This function initiates an asynchronous operation to get a frame of data
 * from the VL53L9 sensor. The data will be stored in the provided buffer.
 *
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] p_buffer Pointer to the buffer where the frame data will be stored
 * @param[in] size Size of the buffer in bytes
 * @note This function is intended to be used when data is output through the I3C interface
 * @note The buffer size can be computed using the @ref vl53l9_get_raw_buffer_size function
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_frame_async(void *const p_dev, uint8_t *p_buffer, uint16_t size); // part 1 of 2

/**
 * @brief Asynchronously acknowledges the reception of a frame
 *
 * This function retrieves the last part of the frame (contaning the DSS and the status line)
 * and acknowledges the reception of it.
 *
 * @param p_dev Opaque pointer used for register level operations
 * @param p_buffer Pointer to the buffer where the frame data will be stored
 * @param size Size of the buffer
 * @note This function is intended to be used when data is output through the I3C interface
 * @note The buffer size can be computed using the @ref vl53l9_get_raw_buffer_size function
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_frame_async_ack(void *const p_dev, uint8_t *p_buffer, uint16_t size); // part 2 of 2

/**
 * @brief Retrieve the error status of the device
 * @param[in] p_dev Opaque pointer used for register level operations
 * @param[out] status Pointer to the variable where the status data will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_status(void *const p_dev, vl53l9_status_t *status);

/**
 * @brief Get the size required to store raw buffer data given the binning factor
 * @param[in] binning Binning factor
 * @param[out] p_size Pointer to the variable where the buffer size will be stored
 * @return See @ref VL53L9_ERROR
 */
int vl53l9_get_raw_buffer_size(uint8_t binning, uint16_t *p_size);

#ifdef __cplusplus
}
#endif

#endif // VL53L9_H
