/**
 * @file services_host_power.c
 *
 * @brief Low Power STOP / Global Standby services service source file
 * @ingroup host_services
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

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "services_lib_api.h"
#include "services_lib_protocol.h"
#include "services_lib_ids.h"
#include "aipm.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/
#define UNUSED(x) (void)(x)

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/*******************************************************************************
 *  G L O B A L   V A R I A B L E S
 ******************************************************************************/

/*******************************************************************************
 *  C O D E
 ******************************************************************************/

/**
 * @brief aiPM RUN
 *
 * @param services_handle
 * @param pp
 * @param error_code
 * @return
 */
uint32_t SERVICES_get_run_cfg(uint32_t services_handle, run_profile_t *pp,
                              uint32_t *error_code)
{
  uint32_t srv_error_code = 0; /* Service function call return */

  aipm_get_run_profile_svc_t * p_svc = (aipm_get_run_profile_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(aipm_get_run_profile_svc_t));

  srv_error_code = SERVICES_send_request(services_handle,
                                         SERVICE_POWER_GET_RUN_REQ_ID,
                                         DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code; /* return actual call error */

  pp->aon_clk_src       = p_svc->resp_aon_clk_src;
  pp->run_clk_src       = p_svc->resp_run_clk_src;
  pp->cpu_clk_freq      = p_svc->resp_cpu_clk_freq;
  pp->scaled_clk_freq   = p_svc->resp_scaled_clk_freq;
  pp->dcdc_mode         = p_svc->resp_dcdc_mode;
  pp->dcdc_voltage      = p_svc->resp_dcdc_voltage;
  pp->memory_blocks     = p_svc->resp_memory_blocks;
  pp->ip_clock_gating   = p_svc->resp_ip_clock_gating;
  pp->phy_pwr_gating    = p_svc->resp_phy_pwr_gating;
  pp->power_domains     = p_svc->resp_power_domains;
  pp->vdd_ioflex_3V3    = p_svc->resp_vdd_ioflex_3V3;

  return srv_error_code;  /* Return */
}

/**
 * @brief aiPM RUN
 *
 * @param services_handle
 * @param pp
 * @param error_code
 * @return
 */
uint32_t SERVICES_set_run_cfg(uint32_t services_handle, run_profile_t *pp,
                              uint32_t *error_code)
{
  uint32_t srv_error_code = 0; /* Service function call return */

  aipm_set_run_profile_svc_t * p_svc = (aipm_set_run_profile_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(aipm_set_run_profile_svc_t));

  /**
   * pack ready to send
   * @todo use memcpy()
   */
  p_svc->send_aon_clk_src      = pp->aon_clk_src; /*typedef */
  p_svc->send_run_clk_src      = pp->run_clk_src; /*typedef */
  p_svc->send_cpu_clk_freq     = pp->cpu_clk_freq;
  p_svc->send_scaled_clk_freq  = pp->scaled_clk_freq;
  p_svc->send_dcdc_mode        = pp->dcdc_mode; /* typedef */
  p_svc->send_dcdc_voltage     = pp->dcdc_voltage;
  p_svc->send_memory_blocks    = pp->memory_blocks;
  p_svc->send_ip_clock_gating  = pp->ip_clock_gating;
  p_svc->send_phy_pwr_gating   = pp->phy_pwr_gating; /*typedef */
  p_svc->send_power_domains    = pp->power_domains;
  p_svc->send_vdd_ioflex_3V3   = pp->vdd_ioflex_3V3;

  srv_error_code = SERVICES_send_request(services_handle,
                                         SERVICE_POWER_SET_RUN_REQ_ID,
                                         DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code; /* return actual call error */

   return srv_error_code;  /* Return error */
}

/**
 * @brief aiPM OFF
 *
 * @param services_handle
 * @param wp
 * @param error_code
 * @return
 */
uint32_t SERVICES_get_off_cfg(uint32_t services_handle, off_profile_t *wp,
                              uint32_t *error_code)
{
  uint32_t srv_error_code = 0; /* Service function call return */

  aipm_get_off_profile_svc_t * p_svc = (aipm_get_off_profile_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(aipm_get_off_profile_svc_t));

  srv_error_code = SERVICES_send_request(services_handle,
                                         SERVICE_POWER_GET_OFF_REQ_ID,
                                         DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code; /* return actual call error */

  wp->dcdc_voltage      = p_svc->resp_dcdc_voltage;
  wp->memory_blocks     = p_svc->resp_memory_blocks;
  wp->power_domains     = p_svc->resp_power_domains;
  wp->aon_clk_src       = p_svc->resp_aon_clk_src;
  wp->stby_clk_src      = p_svc->resp_stby_clk_src;
  wp->stby_clk_freq     = p_svc->resp_stby_clk_freq;
  wp->ip_clock_gating   = p_svc->resp_ip_clock_gating;
  wp->phy_pwr_gating    = p_svc->resp_phy_pwr_gating;
  wp->vdd_ioflex_3V3    = p_svc->resp_vdd_ioflex_3V3;
  wp->vtor_address      = p_svc->resp_vtor_address;
  wp->vtor_address_ns   = p_svc->resp_vtor_address_ns;
  wp->wakeup_events     = p_svc->resp_wakeup_events;
  wp->ewic_cfg          = p_svc->resp_ewic_cfg;

  return srv_error_code;  /* Return error */
}

/**
 * @brief aiPM RUN
 *
 * @param services_handle
 * @param wp
 * @param error_code
 * @return
 */
uint32_t SERVICES_set_off_cfg(uint32_t services_handle, off_profile_t *wp,
                              uint32_t *error_code)
{
  uint32_t srv_error_code = 0; /* Service function call return */

  aipm_set_off_profile_svc_t * p_svc = (aipm_set_off_profile_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(aipm_get_off_profile_svc_t));

  /**
   * pack ready to send
   * @todo use memcpy()
   */
  p_svc->send_dcdc_voltage     = wp->dcdc_voltage;
  p_svc->send_memory_blocks    = wp->memory_blocks;
  p_svc->send_power_domains    = wp->power_domains;
  p_svc->send_aon_clk_src      = wp->aon_clk_src;
  p_svc->send_stby_clk_src     = wp->stby_clk_src;
  p_svc->send_stby_clk_freq    = wp->stby_clk_freq;
  p_svc->send_ip_clock_gating  = wp->ip_clock_gating;
  p_svc->send_phy_pwr_gating   = wp->phy_pwr_gating; /*typedef */
  p_svc->send_vdd_ioflex_3V3   = wp->vdd_ioflex_3V3;
  p_svc->send_vtor_address     = wp->vtor_address;
  p_svc->send_vtor_address_ns  = wp->vtor_address_ns;
  p_svc->send_wakeup_events    = wp->wakeup_events;
  p_svc->send_ewic_cfg         = wp->ewic_cfg;

  srv_error_code = SERVICES_send_request(services_handle,
                                         SERVICE_POWER_SET_OFF_REQ_ID,
                                         DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code; /* return actual call error */

  return srv_error_code;  /* Return error */
}

/**
 * @brief Function to request stop mode
 * @param services_handle
 * @param power_profile
 * @param override
 * @return
 */
uint32_t SERVICES_power_stop_mode_req(uint32_t services_handle,
                                      services_power_profile_t power_profile,
                                      bool override)
{
  stop_mode_request_svc_t * p_svc = (stop_mode_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(stop_mode_request_svc_t));

  p_svc->power_profile = power_profile;
  p_svc->override      = override;
  p_svc->pad           = 0x0;

  return SERVICES_send_request(services_handle, 
                               SERVICE_POWER_STOP_MODE_REQ_ID,
                               DEFAULT_TIMEOUT);
}

/**
 * @brief Function to disable SERAM 0, 1 or MRAM
 * @param services_handle
 * @param memory_request
 * @param error_code
 * @return
 */
uint32_t SERVICES_power_memory_req(uint32_t services_handle,
                                   uint32_t memory_request,
                                   uint32_t *error_code)
{
  mem_power_config_request_svc_t * p_svc = (mem_power_config_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(mem_power_config_request_svc_t));

  p_svc->send_memory_power = memory_request;

  UNUSED(error_code);

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_MEMORY_POWER_REQ_ID,
                               DEFAULT_TIMEOUT);
}


/**
 * @brief Function to configure EWIC source
 * @param services_handle
 * @param ewic_source
 * @param power_profile
 * @return
 */
uint32_t SERVICES_power_ewic_config(uint32_t services_handle,
                                    uint32_t ewic_source,
                                    services_power_profile_t power_profile)
{
  ewic_config_request_svc_t * p_svc = (ewic_config_request_svc_t *)
    SERVICES_prepare_packet_buffer(sizeof(ewic_config_request_svc_t));

  p_svc->send_ewic_source = ewic_source;
  p_svc->power_profile    = power_profile;

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_EWIC_CONFIG_REQ_ID,
                               DEFAULT_TIMEOUT);
}

/**
 * @brief Function to configure VBAT wake up source
 * @param services_handle
 * @param vbat_wakeup_source
 * @param power_profile
 * @return
 */
uint32_t SERVICES_power_wakeup_config(uint32_t services_handle,
                                      uint32_t vbat_wakeup_source,
                                      services_power_profile_t power_profile)
{
  vbat_wakeup_config_request_svc_t * p_svc =
    (vbat_wakeup_config_request_svc_t *)
    SERVICES_prepare_packet_buffer(sizeof(vbat_wakeup_config_request_svc_t));

  p_svc->send_vbat_wakeup_source = vbat_wakeup_source;
  p_svc->power_profile           = power_profile;

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_VBAT_WAKEUP_CONFIG_REQ_ID,
                               DEFAULT_TIMEOUT);
}

/**
 * @brief Function to configure memory retention
 * @param services_handle
 * @param mem_retention
 * @param power_profile
 * @return
 */
uint32_t
SERVICES_power_mem_retention_config(uint32_t services_handle,
                                    uint32_t mem_retention,
                                    services_power_profile_t power_profile)
{
  mem_ret_config_request_svc_t * p_svc =
      (mem_ret_config_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(mem_ret_config_request_svc_t));

  p_svc->send_mem_retention = mem_retention;
  p_svc->power_profile      = power_profile;

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_MEM_RETENTION_CONFIG_REQ_ID,
                               DEFAULT_TIMEOUT);
}

/**
 * @brief Function to request m55-he VTOR value save for wake up
 * @param services_handle
 * @param ns_vtor_addr
 * @param se_vtor_addr
 * @param power_profile
 * @return
 */
uint32_t
SERVICES_power_m55_he_vtor_save(uint32_t services_handle,
                                uint32_t ns_vtor_addr,
                                uint32_t se_vtor_addr,
                                services_power_profile_t power_profile)
{
  m55_vtor_save_request_svc_t * p_svc =
      (m55_vtor_save_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(m55_vtor_save_request_svc_t));

  p_svc->ns_vtor_addr       = ns_vtor_addr;
  p_svc->se_vtor_addr       = se_vtor_addr;
  p_svc->power_profile      = power_profile;

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_M55_HE_VTOR_SAVE_REQ_ID,
                               DEFAULT_TIMEOUT);
}

/**
 * @brief Function to request m55-hp VTOR value save for wake up
 * @param services_handle
 * @param ns_vtor_addr
 * @param se_vtor_addr
 * @param power_profile
 * @return
 */
uint32_t
SERVICES_power_m55_hp_vtor_save(uint32_t services_handle,
                                uint32_t ns_vtor_addr,
                                uint32_t se_vtor_addr,
                                services_power_profile_t power_profile)
{
  m55_vtor_save_request_svc_t * p_svc =
      (m55_vtor_save_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(m55_vtor_save_request_svc_t));

  p_svc->ns_vtor_addr       = ns_vtor_addr;
  p_svc->se_vtor_addr       = se_vtor_addr;
  p_svc->power_profile      = power_profile;

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_M55_HP_VTOR_SAVE_REQ_ID,
                               DEFAULT_TIMEOUT);
}

/**
 * @brief Function to request standby mode
 * @param services_handle
 * @param host_cpu_clus_pwr_req
 * @param bsys_pwr_req
 * @param error_code
 * @return
 */
uint32_t
SERVICES_corstone_standby_mode(uint32_t services_handle,
                               host_cpu_clus_pwr_req_t host_cpu_clus_pwr_req,
                               bsys_pwr_req_t bsys_pwr_req,
                               uint32_t *error_code)
{
  uint32_t srv_error_code = 0; /* Service function call return */

  global_standby_request_svc_t *p_svc =
      (global_standby_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(global_standby_request_svc_t));

  p_svc->host_cpu_clus_pwr_req.word = host_cpu_clus_pwr_req.word;
  p_svc->bsys_pwr_req.word          = bsys_pwr_req.word;

  srv_error_code = SERVICES_send_request(services_handle,
                                         SERVICE_POWER_GLOBAL_STANDBY_REQ_ID,
                                         DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code; /* return actual call error */

  return srv_error_code;  /* Return SERVICES error code */
}

/**
 * @brief SE to deep sleep
 *
 * @param services_handle
 * @param se_param
 * @param error_code
 * @return
 */
uint32_t SERVICES_power_se_sleep_req(uint32_t services_handle,
                                     uint32_t se_param,
                                     uint32_t *error_code)
{
  uint32_t srv_error_code = 0; /* Service function call return */
  se_sleep_svc_t *p_svc =
      (se_sleep_svc_t *)SERVICES_prepare_packet_buffer(sizeof(se_sleep_svc_t));

  p_svc->send_param = se_param;

  srv_error_code = SERVICES_send_request(services_handle,
                                         SERVICE_POWER_SE_SLEEP_REQ_ID,
                                         DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code; /* return actual call error */

  return srv_error_code;  /* Return SERVICES error code */
}

/**
 * @brief Function for DCDC voltage control
 * @param services_handle
 * @param dcdc_vout_sel
 * @param dcdc_vout_trim
 * @param error_code
 * @return
 */
uint32_t
SERVICES_power_dcdc_voltage_control(uint32_t services_handle,
                                    uint32_t dcdc_vout_sel,
                                    uint32_t dcdc_vout_trim,
                                    uint32_t *error_code)
{
  dcdc_voltage_request_svc_t * p_svc =
      (dcdc_voltage_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(dcdc_voltage_request_svc_t));

  p_svc->dcdc_vout_sel     = dcdc_vout_sel;
  p_svc->dcdc_vout_trim    = dcdc_vout_trim;

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_POWER_DCDC_VOLTAGE_REQ_ID,
                                       DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;

  return ret;
}

/**
 * @brief Function for LDO voltage control
 * @param services_handle
 * @param ret_ldo_voltage
 * @param aon_ldo_voltage
 * @param error_code
 * @return
 */
uint32_t
SERVICES_power_ldo_voltage_control(uint32_t services_handle,
                                   uint32_t ret_ldo_voltage,
                                   uint32_t aon_ldo_voltage,
                                   uint32_t *error_code)
{
  ldo_voltage_request_svc_t * p_svc =
      (ldo_voltage_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(ldo_voltage_request_svc_t));

  p_svc->ret_ldo_voltage    = ret_ldo_voltage;
  p_svc->aon_ldo_voltage    = aon_ldo_voltage;

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_POWER_LDO_VOLTAGE_REQ_ID,
                                       DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;

  return ret;
}

/**
 * @fn  uint32_t SERVICES_power_setting_configure(uint32_t services_handle,
 *                                                power_setting_t setting_type,
 *                                                uint32_t value,
 *                                                uint32_t *error_code)
 * @brief Configure a power-related setting
 * @param services_handle
 * @param setting_type      Setting type
 * @param value             Setting value
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_power_setting_configure(uint32_t services_handle,
                                          power_setting_t setting_type,
                                          uint32_t value,
                                          uint32_t *error_code)
{
  power_setting_svc_t * p_svc =
      (power_setting_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(power_setting_svc_t));

  p_svc->send_setting_type = setting_type;
  p_svc->value = value;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_POWER_SETTING_CONFIG_REQ_ID, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_power_setting_get(uint32_t services_handle,
 *                                          power_setting_t setting_type,
 *                                          uint32_t *value,
 *                                          uint32_t *error_code)
 * @brief Get a power-related setting
 * @param services_handle
 * @param setting_type      Setting type
 * @param value             Setting value
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_power_setting_get(uint32_t services_handle,
                                    power_setting_t setting_type,
                                    uint32_t *value,
                                    uint32_t *error_code)
{
  power_setting_svc_t * p_svc =
      (power_setting_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(power_setting_svc_t));

  p_svc->send_setting_type = setting_type;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_POWER_SETTING_GET_REQ_ID, DEFAULT_TIMEOUT);

  if (SERVICES_REQ_SUCCESS == ret)
  {
    *value = p_svc->value;
  }

  *error_code = p_svc->resp_error_code;
  return ret;
}
