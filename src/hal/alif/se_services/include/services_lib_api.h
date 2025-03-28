/**
 * @file services_lib_api.h
 *
 * @brief Services library public API header file
 * @defgroup host_services host_services
 * @par
 *
 * Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#ifndef __SERVICES_LIB_API_H__
#define __SERVICES_LIB_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include "services_lib_protocol.h"
#include "aipm.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

/**
 * Default service call timeout
 */
#define DEFAULT_TIMEOUT                            (0)

/**
 * Common Service error codes - follow the pattern from the PLL services
 */
#define SERVICE_SUCCESS                            0x0
#define SERVICE_FAIL                               0x200
#define SERVICE_INVALID_PARAMETER                  0x201

/**
 * Pin muxing/pad control error codes
 */
#define PINMUX_SUCCESS                             0x0
#define PINMUX_ERROR_INVALID_PARAMETER             0x200

/**
 * OSPI Write Key error codes
 */
#define OSPI_WRITE_KEY_SUCCESS                     0x0
#define OSPI_WRITE_KEY_ERROR_INVALID_PARAMETER     0x200
#define OSPI_WRITE_KEY_ERROR_OTP_READ_FAILED       0x201

/**
 * Crypto services error codes - use values not used by MbedTLS
 */
#define CRYPTOCELL_SUCCESS                         0x0
#define CRYPTOCELL_ERROR_INVALID_CRYPT_TYPE        0xFFFFFFFFul
#define CRYPTOCELL_ERROR_INVALID_SHA_TYPE          0xFFFFFFFEul
#define CRYPTOCELL_ERROR_INVALID_KEY_TYPE          0xFFFFFFFDul
#define CRYPTOCELL_ERROR_INVALID_SEND_DIRECTION    0xFFFFFFFCul

/**
 * Clocks services error codes
 */
#define PLL_SUCCESS                                0x0
#define PLL_ERROR_INVALID_PARAMETER                0x200
#define PLL_ERROR_PLL_NOT_RUNNING                  0x201
#define PLL_ERROR_PLL_ALREADY_RUNNING              0x202
#define PLL_ERROR_XTAL_NOT_RUNNING                 0x203
#define PLL_ERROR_XTAL_ALREADY_RUNNING             0x204

/**
 * Boot services error codes
 */
#define BL_STATUS_OK                                  0x00
#define BL_ERROR_APP_INVALID_TOC_ADDRESS              0x01
#define BL_ERROR_APP_INVALID_TOC_OFFSET               0x02
#define BL_ERROR_UNALIGNED_ADDRESS                    0x03
#define BL_ERROR_INVALID_TOC_ADDRESS_RANGE            0x04
#define BL_ERROR_INVALID_TOC_FLAGS                    0x05
#define BL_ERROR_INVALID_ADDRESS                      0x06
#define BL_ERROR_CERTIFICATE_NO_VERIFY_IN_MEMORY      0x07
#define BL_ERROR_CERTIFICATE_NO_VERIFY_IN_FLASH       0x08
#define BL_ERROR_CERTIFICATE_INVALID_LOAD_ADDRESS     0x09
#define BL_ERROR_CERTIFICATE_INVALID_CHAIN            0x0A
#define BL_ERROR_CERTIFICATE_STORAGE_ADDRESS_INVALID  0x0B
#define BL_ERROR_DEVICE_ADDRESS_INVALID               0x0C
#define BL_ERROR_UNCOMPRESS_FAILED                    0x0D
#define BL_ERROR_SIGNATURE_VERIFY_FAILED              0x0E
#define BL_ERROR_APP_ACCESSING_PROTECTED_AREA         0x0F
#define BL_ERROR_ICV_ACCESSING_PROTECTED_AREA         0x10
#define BL_ERROR_FAILED_TOC_CRC32                     0x11
#define BL_ERROR_INVALID_TOC                          0x12
#define BL_ERROR_EXCEED_MAXIMUM_TOC_ENTRIES           0x13
#define BL_ERROR_NOT_IMAGE_NOT_DEVICE_CFG             0x14
#define BL_ERROR_INVALID_TOC_ENTRY_ID                 0x15
#define BL_ERROR_INVALID_CPU_ID                       0x16
#define BL_ERROR_ENTRY_NOT_SIGNED                     0x17
#define BL_ERROR_LOAD_TO_MRAM_NOT_ALLOWED             0x18
#define BL_ERROR_NO_FREE_SLOTS                        0x19
#define BL_ERROR_INVALID_M55_BOOT_ADDRESS             0x1A
#define BL_TOC_OBJECT_NOT_FOUND                       0x1B
#define BL_TOC_OBJECT_FOR_CPU_NOT_FOUND               0x1C
#define BL_TOC_IMAGE_NOT_BOOTABLE                     0x1D
#define BL_TOC_IMAGE_NOT_FOUND                        0x1E
#define BL_ERROR_COMPRESSION_NOT_SUPPORTED            0x1F
#define BL_TOC_IMAGE_DEVICE_MISMATCH                  0x20
#define BL_ERROR_UPD_SIGNATURE_INCORRECT              0x21
#define BL_ERROR_UPD_IMG_IN_MRAM_NOT_SUPPORTED        0x22

/**
 * OTP Offsets
 */
#define OTP_MANUFACTURE_INFO_DATA_START            0x51
#define OTP_MANUFACTURE_INFO_DATA_END              0x58
#define OTP_MANUFACTURE_INFO_SERIAL_NUMBER_START   0x59
#define OTP_MANUFACTURE_INFO_SERIAL_NUMBER_END     0x5A
#define OTP_MANUFACTURE_INFO_PART_NUMBER_START     0x5B
#define OTP_MANUFACTURE_INFO_PART_NUMBER_END       0x5E
#define OTP_CUSTOMER_SECURITY_FLAGS_START          0x5F

#define OTP_ALIF_MANUFACTURE_INFO_PART_NUMBER_LENGTH_BYTES    16
#define OTP_ALIF_MANUFACTURE_INFO_OPTIONAL_LENGTH_BYTES       32
#define OTP_ALIF_MANUFACTURE_INFO_SERIAL_NUMBER_LENGTH_BYTES  8
#define OTP_ALIF_HBK_LENGTH_BYTES                             16

#define OTP_FIRMWARE_VERSION_DCU_LENGTH_BYTES                 40

#define OTP_OSPI_KEY_OSPI0                         0x60
#define OTP_OSPI_KEY_OSPI1                         0x64
#define OSPI_KEY_LENGTH_BYTES                      16

#define SERVICES_MAX_PACKET_BUFFER_SIZE            600

/**
 * MBED TLS
 */
#define MBEDTLS_OP_DECRYPT                         0
#define MBEDTLS_OP_ENCRYPT                         1

#define MBEDTLS_AES_BLOCK_SIZE                     16

#define MBEDTLS_AES_CRYPT_ECB                      0
#define MBEDTLS_AES_CRYPT_CBC                      1
#define MBEDTLS_AES_CRYPT_CTR                      2
#define MBEDTLS_AES_CRYPT_OFB                      3

#define MBEDTLS_AES_KEY_128                        128
#define MBEDTLS_AES_KEY_192                        192
#define MBEDTLS_AES_KEY_256                        256

#define MBEDTLS_HASH_SHA1                          0
#define MBEDTLS_HASH_SHA224                        1
#define MBEDTLS_HASH_SHA256                        2

#define MBEDTLS_CCM_KEY                            0
#define MBEDTLS_GCM_KEY                            1

#define MBEDTLS_CCM_ENCRYPT_AND_TAG                0
#define MBEDTLS_CCM_AUTH_DECRYPT                   1
#define MBEDTLS_CCM_STAR_ENCRYPT_AND_TAG           2
#define MBEDTLS_CCM_STAR_AUTH_DECRYPT              3
#define MBEDTLS_GCM_ENCRYPT_AND_TAG                4
#define MBEDTLS_GCM_DECRYPT_AND_TAG                5
#define MBEDTLS_GCM_AUTH_DECRYPT                   6

#define MBEDTLS_CHACHAPOLY_ENCRYPT_AND_TAG         0
#define MBEDTLS_CHACHAPOLY_AUTH_DECRYPT            1

#define SERVICES_NUMBER_OF_TOC_ENTRIES             15

/**
 * OSPI write key commands
 */
#define OSPI_WRITE_OTP_KEY_OSPI0                   0
#define OSPI_WRITE_OTP_KEY_OSPI1                   1
#define OSPI_WRITE_EXTERNAL_KEY_OSPI0              2
#define OSPI_WRITE_EXTERNAL_KEY_OSPI1              3

/**
 * TOC related
 */
#define TOC_NAME_LENGTH                            8

/**
 * Global standby configuration macros
 */

/*
 * Host CPU Cluster Power Request HOST_CPU_CLUS_PWR_REQ
 */
// MEM_RET_REQ
#define MEM_RET_REQ_LAST_LEVEL_CACHE_RET_OFF   0x0
#define MEM_RET_REQ_LAST_LEVEL_CACHE_RET_ON    0x1
// PWR_REQ
#define PWR_REQ_CLUSTOP_LOW_POWER_ON           0x0
#define PWR_REQ_CLUSTOP_FUNC_RET_ON            0x1

/*
 * Base System Power Request BSYS_PWR_REQ
 */
// SYSTOP_PWR_REQ
#define SYSTOP_PWR_REQ_LOGIC_OFF_MEM_OFF       0x0
#define SYSTOP_PWR_REQ_LOGIC_OFF_MEM_RET       0x1
#define SYSTOP_PWR_REQ_LOGIC_ON_MEM_ON_OR_RET  0x2
#define SYSTOP_PWR_REQ_LOGIC_ON_MEM_ON         0x4
// DBGTOP_PWR_REQ
#define DBGTOP_PWR_REQ_OFF                     0x0
#define DBGTOP_PWR_REQ_ON                      0x1
// REFCLK_REQ
#define REFCLK_REQ_OFF                         0x0
#define REFCLK_REQ_ON                          0x1
// WAKEUP_EN
#define WAKEUP_EN_SE_OFF                       0x0
#define WAKEUP_EN_SE_ON                        0x1

/**
 * @brief Power / retention error codes
 */
#define ERROR_POWER_SRAM_RETENTION_INVALID     0x100

/**
 * Memory SRAM 0 1 MRAM Power configuration bit encoding
 */
#define POWER_MEM_SRAM_0_ENABLE                (1 << 0)
#define POWER_MEM_SRAM_1_ENABLE                (1 << 1)
#define POWER_MEM_SRAM_0_ISOLATION_ENABLE      (1 << 2)
#define POWER_MEM_SRAM_1_ISOLATION_ENABLE      (1 << 3)
#define POWER_MEM_MRAM_ENABLE                  (1 << 4)

  /**
   * SYSTOP power configuration
   */
#define SYSTOP_LOGIC_OFF_POWER_OFF             0x0
#define SYSTOP_LOGIC_OFF_RETENTION_ON          0x1
#define SYSTOP_LOGIC_ON_POWER_X_RET_X          0x2 // can be powered/retained
#define SYSTOP_LOGIC_ON_POWER_ON               0x4

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

typedef int32_t (*wait_ms_t)(uint32_t wait_time_ms);
typedef int (*print_msg_t)(const char *fmt, ...);

/**
 * @enum SERVICES_cpuid_t
 * @brief
 */
typedef enum {
	HOST_CPU_0   = 0,                /*!< A32_0 CPU               HOST_CPU_0 */
	HOST_CPU_1   = 1,                /*!< A32_1 CPU               HOST_CPU_1 */
	EXTSYS_0     = 2,                /*!< M55 HP CPU or other CPU EXTSYS_0   */
	EXTSYS_1     = 3,                /*!< M55 HE CPU              EXTSYS_1   */
} SERVICES_cpuid_t;

/**
 * @struct SERVICES_toc_info_t
 * @brief user facing TOC information
 */
typedef struct {
	uint8_t   image_identifier[TOC_NAME_LENGTH]; /*!< TOC name         */
	uint32_t  version;                           /*!< TOC Version      */
	uint32_t  cpu;                               /*!< TOC Cpu ID       */
	uint32_t  store_address;                     /*!< TOC MRAM address */
	uint32_t  load_address;                      /*!< TOC load         */
	uint32_t  boot_address;                      /*!< TOC boot address */
	uint32_t  image_size;                        /*!< TOC image size   */
	uint32_t  processing_time;                   /*!< TOC process time */
	uint32_t  flags;                             /*!< TOC flag state   */
	uint8_t   flags_string[FLAG_STRING_SIZE];    /*!< TOC flag string  */
} SERVICES_toc_info_t;

/**
 * @struct SERVICES_version_data_t
 * @brief  user facing device details, including internal OTP
 */
typedef struct {
	uint32_t revision_id; /*!< SoC revision          */
	uint8_t version[4];   /*!< @todo deprecate       */
	uint8_t ALIF_PN[16];  /*!< SoC part number       */
	uint8_t HBK0[16];     /*!< ALIF Key              */
	uint8_t HBK1[16];     /*!< ALIF Key              */
	uint8_t HBK_FW[20];   /*!< ALIF Firmware version */
	uint8_t config[4];    /*!< Wounding data         */
	uint8_t DCU[16];      /*!< DCU settings          */
	uint8_t MfgData[32];  /*!< Manufacturing data    */
	uint8_t SerialN[8];   /*!< SoC Serial number     */
	uint8_t LCS;          /*!< SoC lifecycle state   */
} SERVICES_version_data_t;

/**
 * @struct SERVICES_toc_data_t
 */
typedef struct {
	uint32_t number_of_toc_entries;     /*!< Number of real TOC objects */
	SERVICES_toc_info_t toc_entry[SERVICES_NUMBER_OF_TOC_ENTRIES]; /* TOC details */
} SERVICES_toc_data_t;

/**
 * @enum services_power_profile_t
 */
typedef enum {
	OFF_PROFILE = 0,               /*!< OFF_PROFILE                    */
	RUN_PROFILE,                   /*!< HIGH_PERFORMANCE_POWER_PROFILE */
	NUMBER_OF_POWER_PROFILES       /*!< NUMBER_OF_POWER_PROFILES       */
} services_power_profile_t;

/**
 * Clocks Services definitions
 */

// Oscillator clock selectors
typedef enum {
	OSCILLATOR_SOURCE_RC,    // use RC as oscillator clock
	OSCILLATOR_SOURCE_XTAL   // use XTAL  as oscillator clock
} oscillator_source_t;

typedef enum {
	OSCILLATOR_TARGET_SYS_CLOCKS,    // various system clocks
	OSCILLATOR_TARGET_PERIPH_CLOCKS, // clock for peripherrals
	OSCILLATOR_TARGET_S32K_CLOCK     // 32K low frequency clock
} oscillator_target_t;

// PLL clock selectors
typedef enum {
	PLL_SOURCE_PLL,  // use the PLL clocks
	PLL_SOURCE_OSC   // use the OCS clocks (can be RC or XTAL)
} pll_source_t;

// ES0 CPU clock frequencies
#define ES0_CLOCK_16MHZ   0
#define ES0_CLOCK_24MHZ   4
#define ES0_CLOCK_48MHZ   0xC

// ExtSys0 Boot arguments
typedef struct {
	uint32_t nvds_src_addr;
	uint32_t nvds_dst_addr;
	uint32_t nvds_copy_len;
	uint32_t trng_dst_addr;
	uint32_t trng_len;
	uint32_t es0_clock_select;
} net_proc_boot_args_t;

typedef enum {
	PLL_TARGET_SYSREFCLK,
	PLL_TARGET_SYSCLK,
	PLL_TARGET_UART,
	PLL_TARGET_ES0,
	PLL_TARGET_ES1,
	PLL_TARGET_SECENC,
	PLL_TARGET_PD4_SRAM
} pll_target_t;

typedef enum {
	CLKEN_SYSPLL,
	CLKEN_CPUPLL,
	CLKEN_ES0,
	CLKEN_ES1,
	CLKEN_HFXO_OUT,
	CLKEN_CLK_160M,
	CLKEN_CLK_100M,
	CLKEN_USB,
	CLKEN_HFOSC,
	CLKEN_SRAM0,
	CLKEN_SRAM1
} clock_enable_t;

typedef enum {
	A32_CLOCK_GATE = 0,
	A32_REFCLK = 1,
	A32_SYSPLL = 2,
	A32_CPUPLL = 4
} a32_source_t;

typedef enum {
	ACLK_CLOCK_GATE = 0,
	ACLK_REFCLK = 1,
	ACLK_SYSPLL = 2
} aclk_source_t;

typedef enum {
	DIVIDER_CPUPLL,
	DIVIDER_SYSPLL,
	DIVIDER_ACLK,
	DIVIDER_HCLK,
	DIVIDER_PCLK
} clock_divider_t;

typedef enum {
	POWER_SETTING_BOR_EN,
	POWER_SETTING_SCALED_CLK_FREQ
} power_setting_t;

typedef enum {
    CLOCK_SETTING_HFOSC_FREQ,
    CLOCK_SETTING_EXTSYS0_FREQ,
    CLOCK_SETTING_EXTSYS1_FREQ,
    CLOCK_SETTING_AXI_FREQ,
    CLOCK_SETTING_AHB_FREQ,
    CLOCK_SETTING_APB_FREQ,
    CLOCK_SETTING_SYSREF_FREQ,
} clock_setting_t;

/*******************************************************************************
 *  G L O B A L   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  F U N C T I O N   P R O T O T Y P E S
 ******************************************************************************/

// Services infrastructure APIs
uint32_t SERVICES_register_channel(uint32_t mhu_id, uint32_t channel_number);
void SERVICES_unregister_channel(uint32_t mhu_id, uint32_t channel_number);

const char *SERVICES_version(void);
char *SERVICES_error_to_string(uint32_t error_code);

// Services functional APIs
uint32_t SERVICES_heartbeat(uint32_t services_handle);
uint32_t SERVICES_uart_write(uint32_t services_handle, size_t size, const uint8_t *uart_data);
uint32_t SERVICES_pinmux(uint32_t services_handle, uint8_t port_number, uint8_t pin_number,
			 uint8_t config_data,
			 uint32_t *error_code);
uint32_t SERVICES_padcontrol(uint32_t services_handle,
			     uint8_t port_number,
			     uint8_t pin_number,
			     uint8_t configuration_value,
			     uint32_t *error_code);
uint32_t SERVICES_application_ospi_write_key(uint32_t services_handle,
					     uint32_t command,
					     uint8_t *key,
					     uint32_t *error_code);
uint32_t SERVICES_cryptocell_get_rnd(uint32_t services_handle,
				     uint16_t rnd_len,
				     void *rnd_value,
				     int32_t *error_code);

uint32_t SERVICES_cryptocell_get_lcs(uint32_t services_handle,
					uint32_t *lcs_state,
					int32_t *error_code);
// MbedTLS macros and APIs
uint32_t SERVICES_cryptocell_mbedtls_hardware_poll(uint32_t services_handle,
						   uint32_t *error_code,
						   uint32_t data,
						   uint32_t output,
						   uint32_t len,
						   uint32_t olen);
uint32_t SERVICES_cryptocell_mbedtls_aes_init(uint32_t services_handle,
					      uint32_t *error_code,
					      uint32_t ctx);
uint32_t SERVICES_cryptocell_mbedtls_aes_set_key(uint32_t services_handle,
						 uint32_t *error_code,
						 uint32_t ctx,
						 uint32_t key,
						 uint32_t keybits,
						 uint32_t dir);
uint32_t SERVICES_cryptocell_mbedtls_aes_crypt(uint32_t services_handle,
					       uint32_t *error_code,
					       uint32_t ctx,
					       uint32_t crypt_type,
					       uint32_t mode,
					       uint32_t length,
					       uint32_t iv,
					       uint32_t input,
					       uint32_t output);
uint32_t SERVICES_cryptocell_mbedtls_aes(uint32_t services_handle,
                           uint32_t *error_code,
                           uint32_t key,
                           uint32_t keybits,
                           uint32_t direction,
                           uint32_t crypt_type,
                           uint32_t iv,
                           uint32_t length,
                           uint32_t input,
                           uint32_t output);

uint32_t SERVICES_cryptocell_mbedtls_sha_starts(uint32_t services_handle,
						uint32_t *error_code,
						uint32_t ctx,
						uint32_t sha_type);
uint32_t SERVICES_cryptocell_mbedtls_sha_process(uint32_t services_handle,
						uint32_t *error_code,
						uint32_t ctx,
						uint32_t sha_type,
						uint32_t data);
uint32_t SERVICES_cryptocell_mbedtls_sha_update(uint32_t services_handle,
						uint32_t *error_code,
						uint32_t ctx,
						uint32_t sha_type,
						uint32_t data,
						uint32_t data_length);
uint32_t SERVICES_cryptocell_mbedtls_sha_finish(uint32_t services_handle,
						uint32_t *error_code,
						uint32_t ctx,
						uint32_t sha_type,
						uint32_t data);
uint32_t SERVICES_cryptocell_mbedtls_sha(uint32_t services_handle,
                        uint32_t *error_code,
                        uint32_t sha_type,
                        uint32_t data,
                        uint32_t data_length,
                        uint32_t sha_sum);

uint32_t SERVICES_cryptocell_mbedtls_ccm_gcm_set_key(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t context_addr,
	uint32_t key_type,
	uint32_t cipher,
	uint32_t key_addr,
	uint32_t key_bits);
uint32_t SERVICES_cryptocell_mbedtls_ccm_gcm_crypt(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t context_addr,
	uint32_t crypt_type,
	uint32_t length,
	uint32_t iv_addr,
	uint32_t iv_length,
	uint32_t add_addr,
	uint32_t add_length,
	uint32_t input_addr,
	uint32_t output_addr,
	uint32_t tag_addr,
	uint32_t tag_length);
uint32_t SERVICES_cryptocell_mbedtls_ccm_gcm(uint32_t services_handle,
    uint32_t *error_code,
    uint32_t crypt_type,
    uint32_t key_addr,
    uint32_t key_bits,
    uint32_t length,
    uint32_t iv_addr,
    uint32_t iv_length,
    uint32_t add_addr,
    uint32_t add_length,
    uint32_t input_addr,
    uint32_t output_addr,
    uint32_t tag_addr,
    uint32_t tag_length);

uint32_t SERVICES_cryptocell_mbedtls_chacha20_crypt(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t key_addr,
	uint32_t nonce_addr,
	uint32_t counter,
	uint32_t data_len,
	uint32_t input_addr,
	uint32_t output_addr);
uint32_t SERVICES_cryptocell_mbedtls_chachapoly_crypt(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t context_addr,
	uint32_t crypt_type,
	uint32_t length,
	uint32_t nonce_addr,
	uint32_t aad_addr,
	uint32_t aad_len,
	uint32_t tag_addr,
	uint32_t input_addr,
	uint32_t output_addr);
uint32_t SERVICES_cryptocell_mbedtls_poly1305_crypt(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t key_addr,
	uint32_t input_addr,
	uint32_t ilen,
	uint32_t mac_addr);
uint32_t SERVICES_cryptocell_mbedtls_cmac_init_setkey(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t ctx,
	uint32_t key,
	uint32_t keybits);
uint32_t SERVICES_cryptocell_mbedtls_cmac_update(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t ctx,
	uint32_t input,
	uint32_t length);
uint32_t SERVICES_cryptocell_mbedtls_cmac_finish(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t ctx,
	uint32_t output);
uint32_t SERVICES_cryptocell_mbedtls_cmac_reset(uint32_t services_handle,
	uint32_t *error_code,
	uint32_t ctx);
uint32_t SERVICES_cryptocell_mbedtls_cmac(uint32_t services_handle,
    uint32_t *error_code,
    uint32_t key,
    uint32_t keybits,
    uint32_t input,
    uint32_t length,
    uint32_t output);

uint32_t SERVICES_system_get_toc_version(uint32_t services_handle,
	uint32_t *toc_version,
	uint32_t *error_code);
uint32_t SERVICES_system_get_toc_number(uint32_t services_handle,
	uint32_t *toc_number,
					uint32_t *error_code);
uint32_t SERVICES_system_get_toc_via_name(uint32_t services_handle,
					  const uint8_t *cpu_name,
					  uint32_t *error_code);
uint32_t SERVICES_system_get_toc_via_cpuid(uint32_t services_handle,
					   SERVICES_cpuid_t cpuid,
					   SERVICES_toc_data_t *toc_info,
					   uint32_t *error_code);
uint32_t SERVICES_system_get_toc_data(uint32_t services_handle,
					SERVICES_toc_data_t *toc_data,
					uint32_t *error_code);
uint32_t SERVICES_system_get_device_part_number(uint32_t services_handle,
						uint32_t *device_part_number,
						uint32_t *error_code);
uint32_t SERVICES_system_set_services_debug(uint32_t services_handle,
					     bool debug_enable,
					     uint32_t *error_code);
uint32_t SERVICES_system_get_device_data(uint32_t services_handle,
					 SERVICES_version_data_t *device_info,
					 uint32_t *error_code);
uint32_t SERVICES_get_se_revision(uint32_t services_handle,
				  uint8_t *revision_data, uint32_t *error_code);
uint32_t SERVICES_system_read_otp(uint32_t services_handle,
				  uint32_t otp_offset,
				  uint32_t *otp_value_word,
				  uint32_t *error_code);
uint32_t SERVICES_system_write_otp(uint32_t services_handle,
				   uint32_t otp_offset,
				   uint32_t otp_value_word,
				   uint32_t *error_code);

uint32_t SERVICES_system_get_eui_extension(uint32_t services_handle,
					bool is_eui48,
					uint8_t *eui_extension,
					uint32_t *error_code);

uint32_t SERVICES_boot_process_toc_entry(uint32_t services_handle,
					 const uint8_t *image_id,
					 uint32_t *error_code);
uint32_t SERVICES_boot_cpu(uint32_t services_handle,
			   uint32_t cpu_id,
			   uint32_t address,
			   uint32_t *error_code);
uint32_t SERVICES_boot_set_vtor(uint32_t services_handle,
				uint32_t cpu_id,
				uint32_t address,
				uint32_t *error_code);
uint32_t SERVICES_boot_reset_cpu(uint32_t services_handle,
				 uint32_t cpu_id,
				 uint32_t *error_code);
uint32_t SERVICES_boot_release_cpu(uint32_t services_handle,
				   uint32_t cpu_id,
				   uint32_t *error_code);
uint32_t SERVICES_boot_reset_soc(uint32_t services_handle);

uint32_t SERVICES_power_stop_mode_req(uint32_t services_handle,
				      services_power_profile_t power_profile,
				      bool override);
uint32_t SERVICES_power_ewic_config(uint32_t services_handle,
				    uint32_t ewic_source,
				    services_power_profile_t power_profile);
uint32_t SERVICES_power_wakeup_config(uint32_t services_handle,
				      uint32_t vbat_wakeup_source,
				      services_power_profile_t power_profile);
uint32_t SERVICES_power_memory_req(uint32_t services_handle,
                                   uint32_t memory_request,
                                   uint32_t *error_code);
uint32_t SERVICES_power_se_sleep_req(uint32_t services_handle,
                                     uint32_t se_param,
                                     uint32_t *error_code);
uint32_t
SERVICES_power_mem_retention_config(uint32_t services_handle,
				    uint32_t mem_retention,
				    services_power_profile_t power_profile);
uint32_t
SERVICES_corstone_standby_mode(uint32_t services_handle,
			       host_cpu_clus_pwr_req_t host_cpu_clus_pwr_req,
			       bsys_pwr_req_t bsys_pwr_req,
			       uint32_t *error_code);
uint32_t
SERVICES_power_m55_he_vtor_save(uint32_t services_handle,
				uint32_t ns_vtor_addr,
				uint32_t se_vtor_addr,
				services_power_profile_t power_profile);
uint32_t
SERVICES_power_m55_hp_vtor_save(uint32_t services_handle,
				uint32_t ns_vtor_addr,
				uint32_t se_vtor_addr,
				services_power_profile_t power_profile);

uint32_t
SERVICES_power_dcdc_voltage_control(uint32_t services_handle,
				    uint32_t dcdc_vout_sel,
				    uint32_t dcdc_vout_trim,
				    uint32_t *error_code);

uint32_t
SERVICES_power_ldo_voltage_control(uint32_t services_handle,
				   uint32_t ret_ldo_voltage,
				   uint32_t aon_ldo_voltage,
				   uint32_t *error_code);

uint32_t SERVICES_power_setting_configure(uint32_t services_handle,
					  power_setting_t setting_type,
					  uint32_t value,
					  uint32_t *error_code);
uint32_t SERVICES_power_setting_get(uint32_t services_handle,
				    power_setting_t setting_type,
				    uint32_t *value,
				    uint32_t *error_code);

// Clocks services
uint32_t SERVICES_clocks_select_osc_source(uint32_t services_handle, oscillator_source_t source, oscillator_target_t target, uint32_t *error_code);
uint32_t SERVICES_clocks_select_pll_source(uint32_t services_handle, pll_source_t source, pll_target_t target, uint32_t *error_code);
uint32_t SERVICES_clocks_enable_clock(uint32_t services_handle, clock_enable_t clock, bool enable, uint32_t *error_code);
uint32_t SERVICES_clocks_set_ES0_frequency(uint32_t services_handle, clock_frequency_t frequency, uint32_t *error_code);
uint32_t SERVICES_clocks_set_ES1_frequency(uint32_t services_handle, clock_frequency_t frequency, uint32_t *error_code);
uint32_t SERVICES_clocks_select_a32_source(uint32_t services_handle, a32_source_t source, uint32_t *error_code);
uint32_t SERVICES_clocks_select_aclk_source(uint32_t services_handle, aclk_source_t source, uint32_t *error_code);
uint32_t SERVICES_clocks_set_divider(uint32_t services_handle, clock_divider_t divider, uint32_t value, uint32_t *error_code);
uint32_t SERVICES_clocks_setting_get(uint32_t services_handle,
                                     clock_setting_t setting_type,
                                     uint32_t *value,
                                     uint32_t *error_code);

// PLL services
uint32_t SERVICES_pll_initialize(uint32_t services_handle, uint32_t *error_code);
uint32_t SERVICES_pll_deinit(uint32_t services_handle, uint32_t *error_code);
uint32_t SERVICES_pll_xtal_start(uint32_t services_handle, bool faststart, bool boost, uint32_t delay_count, uint32_t *error_code);
uint32_t SERVICES_pll_xtal_stop(uint32_t services_handle, uint32_t *error_code);
uint32_t SERVICES_pll_xtal_is_started(uint32_t services_handle, bool *is_started, uint32_t *error_code);
uint32_t SERVICES_pll_clkpll_start(uint32_t services_handle, bool faststart, uint32_t delay_count, uint32_t *error_code);
uint32_t SERVICES_pll_clkpll_stop(uint32_t services_handle, uint32_t *error_code);
uint32_t SERVICES_pll_clkpll_is_locked(uint32_t services_handle, bool *is_locked, uint32_t *error_code);

// External System 0 Services
uint32_t SERVICES_Boot_Net_Proc(uint32_t services_handle, net_proc_boot_args_t *boot_args, uint32_t *error_code);
uint32_t SERVICES_Shutdown_Net_Proc(uint32_t services_handle, uint32_t *error_code);

// Update services
uint32_t SERVICES_update_stoc(uint32_t services_handle,
							  uint32_t image_address,
							  uint32_t image_size,
							  uint32_t *error_code);


#ifdef __cplusplus
}
#endif
#endif /* __SERVICES_LIB_API_H__ */
