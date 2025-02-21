/**
 * @file services_lib_protocol.h
 *
 * @brief Private header file for services library
 *
 * @par
 * @ingroup services
 * Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#ifndef __SERVICES_LIB_PROTOCOL_H__
#define __SERVICES_LIB_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdint.h>

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/
/**
 * Version  Description
 * 0.51.0   Updated contributed power examples
 * 0.50.0   API for getting HFOSC and EXTSYS0/1 frequency
 * 0.49.0   Deprecating SERVICES_system_get_toc_via_name
 * 0.49.1   Updated examples for ALIF Update Package support.
 * 0.48.0   Switch to external CMSIS source builds
 * 0.0.47   Add Power setting Get/Configure API
 * 0.0.46   Adding UPDATE STOC Service and test
 * 0.0.45   Adding STOP, STANDBY Cycle tests
 *          Adding SES update Service
 * 0.0.44   Example test changes
 * 0.0.43   CMSIS V1.0.0
 * 0.0.42   Reduce the size of the packet buffer in the services examples
 * 0.0.41
 * 0.0.40   [aiPM] Define the parameter VDD_IOFLEX_3V3 as an enum
 * 0.0.39   SERVICES switch to CMSIS V0.9.4
 * 0.0.38   SERVICES switch to CMSIS V0.9.3
 * 0.0.37   Scalable HFRC and HXTAL frequencies
 * 0.0.36   SERVICES switch to CMSIS V0.9.1
 *          arm clang startup issues
 * 0.0.35   SERVICES build switch to CMake
 *          addition of aiPM Service API - RUN
 * 0.0.34   Addition of amPM Service API - OFF
 *          retrieve full revision info
 * 0.0.33   edits for simulation testing
 * 0.0.32   Add a new service call SERVICES_boot_set_vtor
 *          Use defines to support different power test variations
 * 0.0.31   Warnings fixes
 * 0.0.30   Fix services-he-hp-a32-xip.json HP address
 * 0.0.29   Consistent Error handling
 *          Memory Power On Off
 *          Add boot services and clock tests
 * 0.0.28   Add new warnings and do cleanup
 * 0.0.27   Add new warnings and do cleanup
 * 0.0.26   Retention fine grained control
 *          CMSIS update build options
 *          Add SERVICES_INVALID_ADDRESS error code
 * 0.0.25   Clocks apis
 *          Retention APIs
 *          TEST Services initialize polling
 * 0.0.24   GCC build for m55_power
 *          Query TOC by cpu id returns N entries
 * 0.0.23   CMSIS integration
 * 0.0.22   Simplify error code usage at the services transport layer
 * 0.0.21   Added build support for SPARK
 * 0.0.20   Updated ALIF License
 * 0.0.19   Adding CMake build
 * 0.0.18   Stop mode power profile
 * 0.0.17   Fixed unimplemented function warnings in GCC build
 * 0.0.16   Update to RTSS V0.4.1
 * 0.0.15   Added examples installation, fixed build flags
 * 0.0.14   Addition of XIP examples
 * 0.0.13   Examples common directory
 * 0.0.12   get otp data (for real!)
 * 0.0.11   get all toc info
 *          get otp data
 * 0.0.10   standardized variables for send/resp
 * 0.0.9    bounds checks for UART prints
 * 0.0.8    Added firmware version id
 * 0.0.7    mbed TLS accelerators
 * 0.0.6    Added enable / disable debug status
 * 0.0.5    Service API error code added
 * 0.0.4    Service BOOT reset added
 * 0.0.3    RPC Parameter changes
 * 0.0.2    First re-factoring
 * 0.0.1    Concept + realization - First implementation
 */
#define SE_SERVICES_VERSION_STRING                 "0.50.1"
#define SE_SERVICES_VERSION_MAJOR                  0
#define SE_SERVICES_VERSION_MINOR                  50
#define SE_SERVICES_VERSION_PATCH                  1

#define IMAGE_NAME_LENGTH                          8
#define VERSION_RESPONSE_LENGTH                    80

/**
 * Transport layer error codes
 */
#define SERVICES_REQ_SUCCESS                       0x00
#define SERVICES_REQ_NOT_ACKNOWLEDGE               0xFF
#define SERVICES_REQ_TIMEOUT                       0xFD
#define SERVICES_RESP_UNKNOWN_COMMAND              0xFC

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/**
 * Transport layer header structure, common for all services
 */

/**
 * @struct service_header_t
 */
typedef struct {
	volatile uint16_t hdr_service_id; // Requested Service ID
	volatile uint16_t hdr_flags;      // Request flags
	volatile uint16_t hdr_error_code; // Transport layer error code
	volatile uint16_t hdr_padding;
} service_header_t;

/*******************************************************************************
 *  Service format definitions
 ******************************************************************************/

// Generic APIs
typedef struct {
	service_header_t header;
	volatile int     resp_error_code;
} generic_svc_t;

// SE Deep Sleeper
typedef struct
{
  service_header_t header;
  volatile uint32_t  send_param;
  volatile uint32_t  resp_error_code;
} se_sleep_svc_t;

// AI PM APIs
typedef struct {
	service_header_t header;
	volatile uint32_t send_power_domains;
	volatile uint32_t send_dcdc_voltage;
	volatile uint32_t send_dcdc_mode;
	volatile uint32_t send_aon_clk_src;
	volatile uint32_t send_run_clk_src;
	volatile uint32_t send_cpu_clk_freq;
	volatile uint32_t send_scaled_clk_freq;
	volatile uint32_t send_memory_blocks;
	volatile uint32_t send_ip_clock_gating;
	volatile uint32_t send_phy_pwr_gating;
	volatile uint32_t send_vdd_ioflex_3V3;
	volatile int      resp_error_code;
} aipm_set_run_profile_svc_t;

typedef struct {
	service_header_t header;
	volatile uint32_t resp_power_domains;
	volatile uint32_t resp_dcdc_voltage;
	volatile uint32_t resp_dcdc_mode;
	volatile uint32_t resp_aon_clk_src;
	volatile uint32_t resp_run_clk_src;
	volatile uint32_t resp_cpu_clk_freq;
	volatile uint32_t resp_scaled_clk_freq;
	volatile uint32_t resp_memory_blocks;
	volatile uint32_t resp_ip_clock_gating;
	volatile uint32_t resp_phy_pwr_gating;
	volatile uint32_t resp_vdd_ioflex_3V3;
	volatile int      resp_error_code;
} aipm_get_run_profile_svc_t;

typedef struct {
	service_header_t header;
	volatile uint32_t send_power_domains;
	volatile uint32_t send_dcdc_voltage;
	volatile uint32_t send_dcdc_mode;
	volatile uint32_t send_aon_clk_src;
	volatile uint32_t send_stby_clk_src;
	volatile uint32_t send_stby_clk_freq;
	volatile uint32_t send_memory_blocks;
	volatile uint32_t send_ip_clock_gating;
	volatile uint32_t send_phy_pwr_gating;
	volatile uint32_t send_vdd_ioflex_3V3;
	volatile uint32_t send_wakeup_events;
	volatile uint32_t send_ewic_cfg;
	volatile uint32_t send_vtor_address;
	volatile uint32_t send_vtor_address_ns;
	volatile int      resp_error_code;
} aipm_set_off_profile_svc_t;

typedef struct {
	service_header_t header;
	volatile uint32_t resp_power_domains;
	volatile uint32_t resp_dcdc_voltage;
	volatile uint32_t resp_dcdc_mode;
	volatile uint32_t resp_aon_clk_src;
	volatile uint32_t resp_stby_clk_src;
	volatile uint32_t resp_stby_clk_freq;
	volatile uint32_t resp_memory_blocks;
	volatile uint32_t resp_ip_clock_gating;
	volatile uint32_t resp_phy_pwr_gating;
	volatile uint32_t resp_vdd_ioflex_3V3;
	volatile uint32_t resp_wakeup_events;
	volatile uint32_t resp_ewic_cfg;
	volatile uint32_t resp_vtor_address;
	volatile uint32_t resp_vtor_address_ns;
	volatile int      resp_error_code;
} aipm_get_off_profile_svc_t;

// Extsys0 services
typedef struct {
	service_header_t header;
	volatile uint32_t send_nvds_src_addr;
	volatile uint32_t send_nvds_dst_addr;
	volatile uint32_t send_nvds_copy_len;
	volatile uint32_t send_trng_dst_addr;
	volatile uint32_t send_trng_len;
	volatile int      resp_error_code;
} net_proc_boot_svc_t;

typedef struct {
	service_header_t header;
	volatile int     resp_error_code;
} net_proc_shutdown_svc_t;

typedef struct {
	service_header_t header;
	volatile int     resp_error_code;
} extsys1_wakeup_svc_t;

// Crypto Services

// Get Random Data

// MBEDTLS_CTR_DRBG_MAX_REQUEST in cryptocell-rt is 1024,
// it was decided that we will use a smaller buffer
#define MAX_RND_LENGTH 256
typedef struct {
	service_header_t header;
	volatile uint32_t  send_rnd_length;
	volatile uint8_t   resp_rnd[MAX_RND_LENGTH];
	volatile int       resp_error_code;
} get_rnd_svc_t;

// Get LCS
typedef struct {
	service_header_t header;
	volatile uint32_t  resp_lcs;
	volatile uint32_t  resp_error_code;
} get_lcs_svc_t;

// MBEDTLS TRNG HARDWARE POLL
typedef struct {
	service_header_t header;
	volatile uint32_t send_data_addr;
	volatile uint32_t send_output_addr;
	volatile uint32_t send_len;
	volatile uint32_t send_olen_addr;
	volatile uint32_t resp_error_code;
} mbedtls_trng_hardware_poll_svc_t;

// MBEDTLS AES INIT
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
} mbedtls_aes_init_svc_t;

// MBEDTLS AES SET KEY
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_key_addr;
	volatile uint32_t send_key_bits;
	volatile uint32_t send_direction;
	volatile uint32_t resp_error_code;
} mbedtls_aes_set_key_svc_t;

// MBEDTLS AES CRYPT
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_crypt_type; // ECB, CBC, CTR_OFB
	volatile uint32_t send_mode;       // Encrypt, Decrypt
	volatile uint32_t send_length;
	volatile uint32_t send_iv_addr;
	volatile uint32_t send_input_addr;
	volatile uint32_t send_output_addr;
	volatile uint32_t resp_error_code;
} mbedtls_aes_crypt_svc_t;

// MBEDTLS SHA
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_sha_type;  // SHA1, SHA224, SHA256
	volatile uint32_t send_data_addr;
	volatile uint32_t send_data_length;
	volatile uint32_t resp_error_code;
} mbedtls_sha_svc_t;

// MBEDTLS CCM/GCM SET KEY
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_key_type;
	volatile uint32_t send_cipher;
	volatile uint32_t send_key_addr;
	volatile uint32_t send_key_bits;
	volatile uint32_t resp_error_code;
} mbedtls_ccm_gcm_set_key_svc_t;

// MBEDTLS CCM/GCM CRYPT
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_crypt_type;
	volatile uint32_t send_length;
	volatile uint32_t send_iv_addr;
	volatile uint32_t send_iv_length;
	volatile uint32_t send_add_addr;
	volatile uint32_t send_add_length;
	volatile uint32_t send_input_addr;
	volatile uint32_t send_output_addr;
	volatile uint32_t send_tag_addr;
	volatile uint32_t send_tag_length;
	volatile uint32_t resp_error_code;
} mbedtls_ccm_gcm_crypt_svc_t;

// MBEDTLS CHACHA20 CRYPT
typedef struct {
	service_header_t header;
	volatile uint32_t send_key_addr;
	volatile uint32_t send_nonce_addr;
	volatile uint32_t send_counter;
	volatile uint32_t send_data_len;
	volatile uint32_t send_input_addr;
	volatile uint32_t send_output_addr;
	volatile uint32_t resp_error_code;
} mbedtls_chacha20_crypt_svc_t;

// MBEDTLS CHACHAPOLY CRYPT
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_crypt_type;
	volatile uint32_t send_length;
	volatile uint32_t send_nonce_addr;
	volatile uint32_t send_aad_addr;
	volatile uint32_t send_aad_len;
	volatile uint32_t send_tag_addr;
	volatile uint32_t send_input_addr;
	volatile uint32_t send_output_addr;
	volatile uint32_t resp_error_code;
} mbedtls_chachapoly_crypt_svc_t;

// MBEDTLS POLY1305 CRYPT
typedef struct {
	service_header_t header;
	volatile uint32_t send_key_addr;
	volatile uint32_t send_input_addr;
	volatile uint32_t send_ilen;
	volatile uint32_t send_mac_addr;
	volatile uint32_t resp_error_code;
} mbedtls_poly1305_crypt_svc_t;

// MBEDTLS CMAC INIT/SETKEY
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_key_addr;
	volatile uint32_t send_key_bits;
	volatile uint32_t resp_error_code;
} mbedtls_cmac_init_setkey_svc_t;

// MBEDTLS CMAC UPDATE
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_input_addr;
	volatile uint32_t send_input_length;
	volatile uint32_t resp_error_code;
} mbedtls_cmac_update_svc_t;

// MBEDTLS CMAC FINISH
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_output_addr;
	volatile uint32_t resp_error_code;
} mbedtls_cmac_finish_svc_t;

// MBEDTLS CMAC RESET
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t resp_error_code;
} mbedtls_cmac_reset_svc_t;


// Boot Services

// Process TOC Entry
typedef struct {
	service_header_t header;
	volatile uint8_t   send_entry_id[IMAGE_NAME_LENGTH];
	volatile uint32_t  resp_error_code;
} process_toc_entry_svc_t;

// Boot CPU
typedef struct {
	service_header_t header;
	volatile uint32_t  send_cpu_id;
	volatile uint32_t  send_address;
	volatile uint32_t  resp_error_code;
} boot_cpu_svc_t;

// Control CPU
typedef struct {
	service_header_t header;
	volatile uint32_t  send_cpu_id;
	volatile uint32_t  resp_error_code;
} control_cpu_svc_t;

/**
 * Application Services
 */

// Pimux
typedef struct {
	service_header_t header;
	volatile uint32_t send_port_num;
	volatile uint32_t send_pin_num;
	volatile uint32_t send_config_data;
	volatile uint32_t resp_error_code;
} pinmux_svc_t;

// Pad Control
typedef struct {
	service_header_t header;
	volatile uint32_t send_port_num;
	volatile uint32_t send_pin_num;
	volatile uint32_t send_config_data;
	volatile uint32_t resp_error_code;
} pad_control_svc_t;

/* UART Write */
typedef struct {
	service_header_t header;
	volatile uint32_t  send_string_length;
	volatile uint8_t   send_string_contents[256];
	volatile uint32_t  resp_error_code;
} uart_write_svc_t;

// OSPI write key
typedef struct {
	service_header_t header;
	volatile uint32_t  send_command;
	volatile uint8_t   send_key[16];
	volatile uint32_t  resp_error_code;
} ospi_write_key_svc_t;

// Perform the DMPU function, to advance from DM to SE LCS
typedef struct {
	service_header_t header;
	volatile uint32_t send_assets_addr;
	volatile uint32_t resp_error_code;
} dmpu_svc_t;

/**
 * @struct get_se_revision_t
 * Retrieve SERAM version banner
 */
typedef struct {
	service_header_t header;
	volatile uint32_t  resp_se_revision_length;
	volatile uint8_t   resp_se_revision[VERSION_RESPONSE_LENGTH];
	volatile uint32_t  resp_error_code;
} get_se_revision_t;

/**
 * System Management Services
 */

/**
 * @struct get_toc_version_svc_t
 * Get TOC Version
 */
typedef struct {
	service_header_t header;
	volatile uint32_t  resp_version;
	volatile uint32_t  resp_error_code;
} get_toc_version_svc_t;

/**
 * @struct get_toc_number_svc_t
 * Get TOC Number
 */
typedef struct {
	service_header_t header;
	volatile  uint32_t  resp_number_of_toc;
	volatile uint32_t  resp_error_code;
} get_toc_number_svc_t;

/**
 * @struct get_toc_via_name_svc_t
 * Get TOC via Name
 */
typedef struct {
  service_header_t   header;
  volatile  uint8_t  send_name[IMAGE_NAME_LENGTH];
  volatile uint32_t  resp_error_code;
} get_toc_via_name_t;

/**
 * @struct get_toc_via_cpuid_svc_t
 * Get TOC via CPU ID
 */
typedef struct {
  service_header_t   header;
  volatile uint32_t  send_cpuid;
  volatile uint32_t  resp_error_code;
} get_toc_via_cpuid_t;

/**
 * @struct get_toc_entry_t
 * @brief  one toc record
 */
typedef struct {
	volatile  uint8_t  image_identifier[IMAGE_NAME_LENGTH];
	volatile uint32_t  version;
	volatile uint32_t  cpu;
	volatile uint32_t  store_address;
	volatile uint32_t  load_address;
	volatile uint32_t  boot_address;
	volatile uint32_t  image_size;
	volatile uint32_t  flags;
} get_toc_entry_t;

/**
 * @struct get_toc_data_t
 * @brief  Get all TOC data. 15 is the Max number of SES records in SERAM
 */
typedef struct {
	service_header_t header;
	volatile uint32_t send_entry_idx;
	get_toc_entry_t resp_toc_entry;
	volatile uint32_t resp_error_code;
} get_toc_data_t;

/**
 * @struct get_otp_data_t
 * @brief  bucket to hold otp data
 * @todo   to deprecate
 */
typedef struct {
	service_header_t header;
	volatile uint32_t  otp_alif_manufacturing_data[4];
	volatile uint32_t  otp_alif_manufacturing_serial_number[2];
	volatile uint8_t   otp_alif_manufacturing_part_number[16];
	volatile uint32_t  otp_alif_hbk_0[3];
	volatile uint32_t  otp_alif_hbk_1[3];
	volatile uint32_t  otp_alif_firmware_version_dcu[10];
	volatile uint32_t resp_error_code;
} get_otp_data_t;

/**
 * @struct  otp_data_t
 * @brief   request for otp read or write
 */
typedef struct {
	service_header_t header;
	volatile uint32_t send_offset;     /**< OTP offset to read or write */
	volatile uint32_t otp_word;        /**< OTP contents       */
	volatile uint32_t resp_error_code;
} otp_data_t;

/**
 * @struct get_device_part_svc_t
 * Get Device Part
 */
typedef struct {
	service_header_t header;
	volatile uint32_t  resp_device_string;
	volatile uint32_t  resp_error_code;
} get_device_part_svc_t;

/**
 * @struct  get_device_revision_data_t
 * @brief   Get all relevant device information
 */
typedef struct {
	service_header_t header;
	volatile uint32_t revision_id;
	volatile uint8_t resp_version[4];
	volatile uint8_t ALIF_PN[16];
	volatile uint8_t HBK0[16];
	volatile uint8_t HBK1[16];
	volatile uint8_t HBK_FW[20];
	volatile uint8_t config[4];
	volatile uint8_t DCU[16];
	volatile uint8_t MfgData[32];
	volatile uint8_t SerialN[8];
	volatile uint8_t LCS;
	volatile uint32_t  resp_error_code;
} get_device_revision_data_t;

/**
 * @struct set_services_capabilities_t
 * Set capabilities
 * @note for now it is limited to debug toggle
 */
typedef struct {
	service_header_t header;
	volatile bool      send_services_debug;
	volatile uint32_t  resp_error_code;
} set_services_capabilities_t;

/**
 * Power Services
 */

/**
 * Stop Mode Request
 * enable - any bit enables
 *           bit 1 - Set M55_HE VTOR to sentinel pattern
 *           bit 2 - Enable SE TCM retention
 *           bit 3 - force SE  WDOG expiration
 *           bit 4 - force SOC WDOG expiration
 */
typedef struct {
	service_header_t header;
	uint32_t power_profile;
	bool override;
	uint16_t pad;
} stop_mode_request_svc_t;

//----------------------------------------------------------------
// EWIC configuration Request
//
typedef struct {
	service_header_t header;
	uint32_t send_ewic_source;
	uint32_t power_profile;
	uint32_t resp_error_code;
} ewic_config_request_svc_t;

//----------------------------------------------------------------
// VBAT wakeup configuration Request
//
typedef struct {
	service_header_t header;
	uint32_t send_vbat_wakeup_source;
	uint32_t power_profile;
	uint32_t resp_error_code;
} vbat_wakeup_config_request_svc_t;

//----------------------------------------------------------------
// SE memory retention configuration Request
//
typedef struct {
	service_header_t header;
	uint32_t send_mem_retention;
	uint32_t power_profile;
	uint32_t resp_error_code;
} mem_ret_config_request_svc_t;

//----------------------------------------------------------------
// SRAM 0 1 MRAM Power On Off
//
typedef struct {
	service_header_t header;
	uint32_t send_memory_power;
	uint32_t resp_error_code;
} mem_power_config_request_svc_t;

//----------------------------------------------------------------
// Global standby mode Request
//
typedef union {
	struct {
		uint16_t pwr_req :1;
		uint16_t mem_ret_req :1;
	} bits;
	uint32_t word;
} host_cpu_clus_pwr_req_t;

typedef union {
	struct {
		uint8_t wakeup_en :1;
		uint8_t refclk_req :1;
		uint8_t dbgtop_pwr_req :1;
		uint8_t systop_pwr_req :3;
	} bits;
	uint32_t word;
} bsys_pwr_req_t;

typedef union {
	struct {
		uint8_t reserved :2;
		uint8_t dbgtop_pwr_st :1;
		uint8_t systop_pwr_st :3;
	} bits;
	uint32_t word;
} bsys_pwr_st_t;

typedef struct {
	service_header_t header;
	host_cpu_clus_pwr_req_t host_cpu_clus_pwr_req;
	bsys_pwr_req_t bsys_pwr_req;
	uint32_t resp_error_code;
} global_standby_request_svc_t;

//----------------------------------------------------------------
// M55-HE VTOR save Request
//
typedef struct {
	service_header_t header;
	uint32_t ns_vtor_addr;
	uint32_t se_vtor_addr;
	uint32_t power_profile;
	uint32_t resp_error_code;
} m55_vtor_save_request_svc_t;

//----------------------------------------------------------------
// DCDC voltage control Request
//
typedef struct {
	service_header_t header;
	uint32_t dcdc_vout_sel; // bit1:0
	uint32_t dcdc_vout_trim; // bit3:0
	uint32_t resp_error_code;
} dcdc_voltage_request_svc_t;

//----------------------------------------------------------------
// LDO voltage control Request
//
typedef struct {
	service_header_t header;
	uint32_t ret_ldo_voltage; // bit3:0
	uint32_t aon_ldo_voltage; // bit3:0
	uint32_t resp_error_code;
} ldo_voltage_request_svc_t;

//----------------------------------------------------------------
// Clocks API

// struct for clock selection APIs
typedef struct {
	service_header_t header;
	uint32_t send_clock_source;
	uint32_t send_clock_target;
	uint32_t resp_error_code;
} clk_select_clock_source_svc_t;

// enable or disable a clock
typedef struct {
	service_header_t header;
	uint32_t send_clock_type;
	uint32_t send_enable;
	uint32_t resp_error_code;
} clk_set_enable_svc_t;

// struct for M55 frequency selection APIs
typedef struct {
	service_header_t header;
	uint32_t send_frequency;
	uint32_t resp_error_code;
} clk_m55_set_frequency_svc_t;

// struct for system clock source selection (A32 and AXI)
typedef struct {
	service_header_t header;
	uint32_t send_source;
	uint32_t resp_error_code;
} clk_select_sys_clk_source_svc_t;

// struct for setting clock dividers
typedef struct {
	service_header_t header;
	uint32_t send_divider;
	uint32_t send_value;
	uint32_t resp_error_code;
} clk_set_clk_divider_svc_t;

// struct for returning clock registers
typedef struct {
	service_header_t header;
	volatile uint32_t cgu_osc_ctrl;
	volatile uint32_t cgu_pll_sel;
	volatile uint32_t cgu_clk_ena;
	volatile uint32_t cgu_escclk_sel;
	volatile uint32_t systop_clk_div;
	volatile uint32_t hostcpuclk_ctrl;
	volatile uint32_t hostcpuclk_div0;
	volatile uint32_t hostcpuclk_div1;
	volatile uint32_t aclk_ctrl;
	volatile uint32_t aclk_div0;
	volatile uint32_t resp_error_code;
} clk_get_clocks_svc_t;

// struct for starting the external HF crystall
typedef struct {
	service_header_t header;
	uint32_t send_faststart;
	uint32_t send_boost;
	uint32_t send_delay_count;
	uint32_t resp_error_code;
} pll_xtal_start_svc_t;

// struct for starting the PLL
typedef struct {
	service_header_t header;
	uint32_t send_faststart;
	uint32_t send_delay_count;
	uint32_t resp_error_code;
} pll_clkpll_start_svc_t;

// struct for updating the STOC
typedef struct {
	service_header_t header;
	uint32_t send_image_address;
	uint32_t send_image_size;
	uint32_t resp_error_code;
} update_stoc_svc_t;

// Power Get/Configure API
typedef struct {
	service_header_t header;
	volatile uint32_t send_setting_type;
	volatile uint32_t value;
	volatile uint32_t resp_error_code;
} power_setting_svc_t;

// Clock Setting Get API
typedef struct {
    service_header_t header;
    volatile uint32_t send_setting_type;
    volatile uint32_t value;
    volatile uint32_t resp_error_code;
} clock_setting_svc_t;

/*******************************************************************************
 *  G L O B A L   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  F U N C T I O N   P R O T O T Y P E S
 ******************************************************************************/

uintptr_t SERVICES_prepare_packet_buffer(uint32_t size);

uint32_t SERVICES_send_msg(uint32_t services_handle, uint32_t services_data);
uint32_t SERVICES_send_request(uint32_t services_handle,
							   uint16_t service_id,
							   uint32_t service_timeout);

#ifdef __cplusplus
}
#endif
#endif /* __SERVICES_LIB_PROTOCOL_H__ */
