/**
 * @file services_lib_ids.h
 *
 * @brief Private header file for services library
 *
 * @par
 * @ingroup services
 * Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#ifndef __SERVICES_LIB_IDS_H__
#define __SERVICES_LIB_IDS_H__

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/**
 * @enum SERVICE_ID_t Service Identifiers (SID)
 */
enum SERVICE_ID_t {
  /**
   * Maintenance Services
   */
	SERVICE_MAINTENANCE_START  = 0,                                     /**< SERVICE_MAINTENANCE_START                  */
	SERVICE_MAINTENANCE_HEARTBEAT_ID = SERVICE_MAINTENANCE_START,       /**< SERVICE_MAINTENANCE_HEARTBEAT_ID           */
	SERVICE_MAINTENANCE_RTC_ID,                                         /**< SERVICE_MAINTENANCE_RTC_ID                 */
	SERVICE_MAINTENANCE_END    = 99,                                    /**< SERVICE_MAINTENANCE_END                    */

  /**
   * Application Services
   */
	SERVICE_APPLICATION_START  = 100,                                   /**< SERVICE_APPLICATION_START                  */
	SERVICE_APPLICATION_CLOCK_MANAGEMENT_ID = SERVICE_APPLICATION_START,/**< SERVICE_APPLICATION_CLOCK_MANAGEMENT_ID    */
	SERVICE_APPLICATION_PINMUX_ID,                                      /**< SERVICE_APPLICATION_PINMUX_ID              */
	SERVICE_APPLICATION_PAD_CONTROL_ID,                                 /**< SERVICE_APPLICATION_PAD_CONTROL            */
	SERVICE_APPLICATION_FIRMWARE_VERSION_ID,                            /**< SERVICE_APPLICATION_FIRMWARE_VERSION_ID    */
	SERVICE_APPLICATION_UART_WRITE_ID,                                  /**< SERVICE_APPLICATION_UART_WRITE_ID          */
	SERVICE_APPLICATION_OSPI_WRITE_KEY_ID,                              /**< SERVICE_APPLICATION_OSPI_WRITE_KEY_ID      */
	SERVICE_APPLICATION_DMPU_ID,                                        /**< SERVICE_APPLICATION_DMPU_ID                */
	SERVICE_APPLICATION_END    = 199,                                   /**< SERVICE_APPLICATION_END                    */

  /**
   * System Management Services
   */
	SERVICE_SYSTEM_MGMT_START  = 200,                                   /**< SERVICE_SYSTEM_MGMT_START                  */
	SERVICE_SYSTEM_MGMT_GET_TOC_VERSION = SERVICE_SYSTEM_MGMT_START,    /**< SERVICE_SYSTEM_MGMT_GET_TOC_VERSION        */
	SERVICE_SYSTEM_MGMT_GET_TOC_NUMBER,                                 /**< SERVICE_SYSTEM_MGMT_GET_TOC_NUMBER         */
	SERVICE_SYSTEM_MGMT_GET_TOC_FLAGS,                                  /**< SERVICE_SYSTEM_MGMT_GET_TOC_FLAGS          */
	SERVICE_SYSTEM_MGMT_GET_TOC_VIA_CPU_ID,                             /**< SERVICE_SYSTEM_MGMT_GET_TOC_VIA_CPU_ID     */
	SERVICE_SYSTEM_MGMT_GET_TOC_VIA_CPU_NAME,                           /**< SERVICE_SYSTEM_MGMT_GET_TOC_VIA_CPU_NAME   */
	SERVICE_SYSTEM_MGMT_GET_TOC_INFO,                                   /**< SERVICE_SYSTEM_MGMT_GET_TOC_INFO           */
	SERVICE_SYSTEM_MGMT_GET_OTP_INFO,                                   /**< SERVICE_SYSTEM_MGMT_GET_OTP_INFO           */
	SERVICE_SYSTEM_MGMT_GET_DEVICE_PART_NUMBER,                         /**< SERVICE_SYSTEM_MGMT_GET_DEVICE_PART_NUMBER */
	SERVICE_SYSTEM_MGMT_GET_DEVICE_REVISION_DATA,                       /**< SERVICE_SYSTEM_MGMT_GET_DEVICE_REVISION_DATA*/
	SERVICE_SYSTEM_MGMT_SET_CAPABILITIES_DEBUG,                         /**< SERVICE_SYSTEM_MGMT_SET_CAPABILITIES_DEBUG */
	SERVICE_SYSTEM_MGMT_READ_OTP,                                       /**< SERVICE_SYSTEM_MGMT_OTP_READ               */
	SERVICE_SYSTEM_MGMT_WRITE_OTP,                                      /**< SERVICE_SYSTEM_MGMT_OTP_WRITE              */
	SERVICE_SYSTEM_MGMT_END    = 299,                                   /**< SERVICE_SYSTEM_MGMT_END                    */

  /**
   * Power Services
   */
	SERVICE_POWER_START        = 300,                                   /**< SERVICE_POWER_START                        */
	SERVICE_POWER_STOP_MODE_REQ_ID = SERVICE_POWER_START,               /**< SERVICE_POWER_STOP_MODE_REQ_ID             */
	SERVICE_POWER_EWIC_CONFIG_REQ_ID,                                   /**< SERVICE_POWER_EWIC_CONFIG_REQ_ID           */
	SERVICE_POWER_VBAT_WAKEUP_CONFIG_REQ_ID,                            /**< SERVICE_POWER_VBAT_WAKEUP_CONFIG_REQ_ID    */
	SERVICE_POWER_MEM_RETENTION_CONFIG_REQ_ID,                          /**< SERVICE_POWER_MEM_RETENTION_CONFIG_REQ_ID  */
	SERVICE_POWER_M55_HE_VTOR_SAVE_REQ_ID,                              /**< SERVICE_POWER_M55_HE_VTOR_SAVE_REQ_ID      */
	SERVICE_POWER_M55_HP_VTOR_SAVE_REQ_ID,                              /**< SERVICE_POWER_M55_HP_VTOR_SAVE_REQ_ID      */
	SERVICE_POWER_GLOBAL_STANDBY_REQ_ID,                                /**< SERVICE_POWER_GLOBAL_STANDBY_REQ_ID        */
	SERVICE_POWER_MEMORY_POWER_REQ_ID,                                  /**< SERVICE_POWER_MEMORY_POWER_REQ_ID          */
	SERVICE_POWER_DCDC_VOLTAGE_REQ_ID,                                  /**< SERVICE_POWER_DCDC_VOLTAGE_REQ_ID          */
	SERVICE_POWER_LDO_VOLTAGE_REQ_ID,                                   /**< SERVICE_POWER_LDO_VOLTAGE_REQ_ID           */
	SERVICE_POWER_GET_RUN_REQ_ID,                                       /**< SERVICE_POWER_GET_RUN_REQ_ID               */
	SERVICE_POWER_SET_RUN_REQ_ID,                                       /**< SERVICE_POWER_SET_RUN_REQ_ID               */
	SERVICE_POWER_GET_OFF_REQ_ID,                                       /**< SERVICE_POWER_GET_OFF_REQ_ID               */
	SERVICE_POWER_SET_OFF_REQ_ID,                                       /**< SERVICE_POWER_SET_OFF_REQ_ID               */
	SERVICE_POWER_SETTING_CONFIG_REQ_ID,                                /**< SERVICE_POWER_SETTING_CONFIG_REQ_ID        */
	SERVICE_POWER_SETTING_GET_REQ_ID,                                   /**< SERVICE_POWER_SETTING_GET_REQ_ID           */
	SERVICE_POWER_SE_SLEEP_REQ_ID,                                      /**< SERVICE_POWER_SE_SLEEP_REQ_ID              */
	SERVICE_POWER_END          = 399,                                   /**< SERVICE_POWER_END                          */

  /**
   * Cryptocell / Security Services
   */
	SERVICE_CRYPTOCELL_START   = 400,                                   /**< SERVICE_CRYPTOCELL_START                   */
	SERVICE_CRYPTOCELL_GET_RND =  SERVICE_CRYPTOCELL_START,             /**< SERVICE_CRYPTOCELL_GET_RND                 */
	SERVICE_CRYPTOCELL_GET_LCS,                                         /**< SERVICE_CRYPTOCELL_GET_LCS                 */
	SERVICE_CRYPTOCELL_MBEDTLS_AES_INIT,                                /**< SERVICE_CRYPTOCELL_MBEDTLS_AES_INIT        */
	SERVICE_CRYPTOCELL_MBEDTLS_AES_SET_KEY,                             /**< SERVICE_CRYPTOCELL_MBEDTLS_AES_SET_KEY     */
	SERVICE_CRYPTOCELL_MBEDTLS_AES_CRYPT,                               /**< SERVICE_CRYPTOCELL_MBEDTLS_AES_CRYPT_ECB   */
	SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM_SET_KEY,                         /**< SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM_SET_KEY */
	SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM_CRYPT,                           /**< SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM_CRYPT   */
	SERVICE_CRYPTOCELL_MBEDTLS_CHACHA20_CRYPT,                          /**< SERVICE_CRYPTOCELL_MBEDTLS_CHACHA20_CRYPT  */
	SERVICE_CRYPTOCELL_MBEDTLS_CHACHAPOLY_CRYPT,                        /**< SERVICE_CRYPTOCELL_MBEDTLS_CHACHAPOLY_CRYPT*/
	SERVICE_CRYPTOCELL_MBEDTLS_POLY1305_CRYPT,                          /**< SERVICE_CRYPTOCELL_MBEDTLS_POLY1305_CRYPT  */
	SERVICE_CRYPTOCELL_MBEDTLS_SHA_STARTS,                              /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA_STARTS      */
	SERVICE_CRYPTOCELL_MBEDTLS_SHA_PROCESS,                             /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA_PROCESS     */
	SERVICE_CRYPTOCELL_MBEDTLS_SHA_UPDATE,                              /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA_UPDATE      */
	SERVICE_CRYPTOCELL_MBEDTLS_SHA_FINISH,                              /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA_FINISH      */
	SERVICE_CRYPTOCELL_MBEDTLS_TRNG_HARDWARE_POLL,                      /**< SERVICE_CRYPTOCELL_MBEDTLS_TRNG_HARDWARE_POLL */
	SERVICE_CRYPTOCELL_MBEDTLS_CMAC_INIT_SETKEY,                        /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC_INIT_SETKEY*/
	SERVICE_CRYPTOCELL_MBEDTLS_CMAC_UPDATE,                             /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC_UPDATE     */
	SERVICE_CRYPTOCELL_MBEDTLS_CMAC_FINISH,                             /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC_FINISH     */
	SERVICE_CRYPTOCELL_MBEDTLS_CMAC_RESET,                              /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC_RESET      */
	SERVICE_CRYPTOCELL_MBEDTLS_AES,                                     /**< SERVICE_CRYPTOCELL_MBEDTLS_AES             */
	SERVICE_CRYPTOCELL_MBEDTLS_SHA,                                     /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA             */
	SERVICE_CRYPTOCELL_MBEDTLS_CMAC,                                    /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC            */
	SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM,                                 /**< SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM         */
	SERVICE_CRYPTOCELL_END     = 499,                                   /**< SERVICE_CRYPTOCELL_END                     */

  /**
   * Boot Services
   */
	SERVICE_BOOT_START         = 500,                                   /**< SERVICE_BOOT_START                         */
	SERVICE_BOOT_PROCESS_TOC_ENTRY = SERVICE_BOOT_START,                /**< SERVICE_BOOT_PROCESS_TOC_ENTRY             */
	SERVICE_BOOT_CPU,                                                   /**< SERVICE_BOOT_CPU                           */
	SERVICE_BOOT_RELEASE_CPU,                                           /**< SERVICE_BOOT_RESEASE_CPU                   */
	SERVICE_BOOT_RESET_CPU,                                             /**< SERVICE_BOOT_RESET_CPU                     */
	SERVICE_BOOT_RESET_SOC,                                             /**< SERVICE_BOOT_RESET_SOC                     */
	SERVICE_BOOT_SET_VTOR,                                              /**< SERVICE_BOOT_SET_VTOR                      */
	SERVICE_BOOT_SET_ARGS,                                              /**< SERVICE_BOOT_SET_ARGS                      */
	SERVICE_BOOT_END           = 599,                                   /**< SERVICE_BOOT_END                           */

  /**
   * Update Services
   */
	SERVICE_UPDATE_START       = 600,                                   /**< SERVICE_UPDATE_START                       */
	SERVICE_UPDATE_STOC        = SERVICE_UPDATE_START,                  /**< SERVICE_UPDATE_STOC                        */
	SERVICE_UPDATE_END         = 699,                                   /**< SERVICE_UPDATE_END                         */

  /**
   * Clocks Services
   */
	SERVICE_CLOCK_START        = 700,                                    /**< SERVICE_CLOCK_START                       */
	SERVICE_CLOCK_SELECT_OSC_SOURCE = SERVICE_CLOCK_START,
	SERVICE_CLOCK_SELECT_PLL_SOURCE,
	SERVICE_CLOCK_SET_ENABLE,
	SERVICE_CLOCK_ES0_SET_FREQ,
	SERVICE_CLOCK_ES1_SET_FREQ,
	SERVICE_CLOCK_SELECT_A32_SOURCE,
	SERVICE_CLOCK_SELECT_ACLK_SOURCE,
	SERVICE_CLOCK_SET_DIVIDER,
	SERVICE_PLL_INITIALIZE,
	SERVICE_PLL_DEINIT,
	SERVICE_PLL_XTAL_START,
	SERVICE_PLL_XTAL_STOP,
	SERVICE_PLL_XTAL_IS_STARTED,
	SERVICE_PLL_CLKPLL_START,
	SERVICE_PLL_CLKPLL_STOP,
	SERVICE_PLL_CLKPLL_IS_LOCKED,
	SERVICE_CLOCK_GET_CLOCKS,
	SERVICE_CLOCK_SETTING_GET_REQ_ID,
	SERVICE_CLOCK_END = 799,                                             /**< SERVICE_CLOCK_END                       */

  /**
   * ExtSys0 Services
   */
	SERVICE_EXTSYS0_START         = 800,                                 /**< SERVICE_EXTSYS0_START                    */
	SERVICE_EXTSYS0_BOOT_SET_ARGS = SERVICE_EXTSYS0_START,
	SERVICE_EXTSYS0_EXTSYS1_WAKEUP,
	SERVICE_EXTSYS0_SHUTDOWN,
	SERVICE_EXTSYS0_END           = 899                                  /**< SERVICE_EXTSYS0_END                      */

};

#endif /* __SERVICES_LIB_IDS_H__ */
