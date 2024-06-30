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
/* See SERVICES documentation for change log */
#define SE_SERVICES_VERSION_STRING                 "0.50.3"
#define SE_SERVICES_VERSION_MAJOR                  0
#define SE_SERVICES_VERSION_MINOR                  50
#define SE_SERVICES_VERSION_PATCH                  3

#define IMAGE_NAME_LENGTH                          8
#define VERSION_RESPONSE_LENGTH                    80

/**
 * @brief Flag positions with the SERVICES_toc_data_t flag string
 */
#define FLAG_STRING_COMPRESSED                     0
#define FLAG_STRING_LOAD_IMAGE                     1
#define FLAG_STRING_VERIFY                         2
#define FLAG_STRING_CPU_BOOTED                     3
#define FLAG_STRING_ENCRYPTED                      4
#define FLAG_STRING_DEFERRED                       5
#define FLAG_STRING_END                            6
#define FLAG_STRING_SIZE                           8

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
typedef struct {
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
	volatile uint32_t send_internal_clock_select;
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
	volatile uint32_t send_crypt_type; // ECB, CBC, CTR, OFB
	volatile uint32_t send_mode;       // Encrypt, Decrypt
	volatile uint32_t send_length;
	volatile uint32_t send_iv_addr;
	volatile uint32_t send_input_addr;
	volatile uint32_t send_output_addr;
	volatile uint32_t resp_error_code;
} mbedtls_aes_crypt_svc_t;

// MBEDTLS AES
typedef struct {
    service_header_t header;
    volatile uint32_t send_key_addr;
    volatile uint32_t send_key_bits;
    volatile uint32_t send_direction;  // Encrypt, Decrypt
    volatile uint32_t send_crypt_type; // ECB, CBC, CTR, OFB
    volatile uint32_t send_iv_addr;
    volatile uint32_t send_length;
    volatile uint32_t send_input_addr;
    volatile uint32_t send_output_addr;
    volatile uint32_t resp_error_code;
} mbedtls_aes_svc_t;

// MBEDTLS SHA
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_sha_type;  // SHA1, SHA224, SHA256
	volatile uint32_t send_data_addr;
	volatile uint32_t send_data_length;
	volatile uint32_t resp_error_code;
} mbedtls_sha_svc_t;

typedef struct {
    service_header_t header;
    volatile uint32_t send_sha_type;  // SHA1, SHA224, SHA256
    volatile uint32_t send_data_addr;
    volatile uint32_t send_data_length;
    volatile uint32_t send_shasum_addr;
    volatile uint32_t resp_error_code;
} mbedtls_sha_single_svc_t;

// MBEDTLS CCM/GCM SET KEY
typedef struct {
	service_header_t header;
	volatile uint32_t send_context_addr;
	volatile uint32_t send_key_type;     // CCM or GCM
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

// MBEDTLS CCM/GCM
typedef struct {
    service_header_t header;
    volatile uint32_t send_crypt_type;
    volatile uint32_t send_key_addr;
    volatile uint32_t send_key_bits;
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
} mbedtls_ccm_gcm_svc_t;

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

// MBEDTLS CMAC
typedef struct {
    service_header_t header;
    volatile uint32_t send_key_addr;
    volatile uint32_t send_key_bits;
    volatile uint32_t send_input_addr;
    volatile uint32_t send_input_length;
    volatile uint32_t send_output_addr;
    volatile uint32_t resp_error_code;
} mbedtls_cmac_svc_t;

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
 * @brief  TOC record used to convey details of each TOC entry known by SES.
 */
typedef struct {
	volatile uint8_t   resp_image_identifier[IMAGE_NAME_LENGTH];
	volatile uint32_t  resp_version;
	volatile uint32_t  resp_cpu;
	volatile uint32_t  resp_store_address;
	volatile uint32_t  resp_load_address;
	volatile uint32_t  resp_boot_address;
	volatile uint32_t  resp_image_size;
	volatile uint32_t  resp_processing_time;
	volatile uint32_t  resp_flags;
	volatile uint8_t   resp_flags_string[FLAG_STRING_SIZE];
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
