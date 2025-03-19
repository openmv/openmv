/**
 * @file services_host_clocks.c
 *
 * @brief Clocks services service source file
 * @ingroup host_services
 * @par
 *
 * Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
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

/**
 * @fn   uint32_t SERVICES_clocks_select_osc_source(uint32_t services_handle,
 *                                                  oscillator_source_t source,
 *                                                  oscillator_target_t target,
 *                                                  uint32_t * error_code)
 * @brief Select RC or XTAL as Oscillator clock source
 * @param services_handle
 * @param source            RC or XTAL
 * @param target            SYSCLK (HF), PERIPHCLK (HF), S32K (LF)
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_select_osc_source(uint32_t services_handle,
                                           oscillator_source_t source,
                                           oscillator_target_t target,
                                           uint32_t * error_code)
{
  clk_select_clock_source_svc_t * p_svc =
      (clk_select_clock_source_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clk_select_clock_source_svc_t));

  p_svc->send_clock_source = source;
  p_svc->send_clock_target = target;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_SELECT_OSC_SOURCE, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn   uint32_t SERVICES_clocks_select_pll_source(uint32_t services_handle,
 *                                                  oscillator_source_t source,
 *                                                  oscillator_target_t target,
 *                                                  uint32_t * error_code)
 * @brief Select Oscillator or PLL clock source for various target clocks
 * @param services_handle
 * @param source            Oscillator or PLL
 * @param target            SYSREFCLK, SYSCLK, ExtSus0, ExtSys1
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_select_pll_source(uint32_t services_handle,
                                           pll_source_t source,
                                           pll_target_t target,
                                           uint32_t * error_code)
{
  clk_select_clock_source_svc_t * p_svc =
      (clk_select_clock_source_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clk_select_clock_source_svc_t));

  p_svc->send_clock_source = source;
  p_svc->send_clock_target = target;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_SELECT_PLL_SOURCE, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn   uint32_t SERVICES_clocks_enable_clock(uint32_t services_handle,
 *                                             clock_enable_t clock,
 *                                             bool enable,
 *                                             uint32_t * error_code)
 * @brief Select Oscialltor or PLL clock source for various target clocks
 * @param services_handle
 * @param clock             Clock to enable or disable
 * @param enable            Enable/Disable flag
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_enable_clock(uint32_t services_handle,
                                      clock_enable_t clock,
                                      bool enable,
                                      uint32_t * error_code)
{
  clk_set_enable_svc_t * p_svc =
      (clk_set_enable_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clk_set_enable_svc_t));

  p_svc->send_clock_type = clock;
  p_svc->send_enable = enable;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_SET_ENABLE, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_clocks_set_ES0_frequency(uint32_t services_handle,
 *                                                 clock_frequency_t frequency,
 *                                                 uint32_t * error_code)
 * @brief Set the clock frequency for External System 0 (M55-HP)
 * @param services_handle
 * @param frequency         Clock frequency
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_set_ES0_frequency(uint32_t services_handle,
                                           clock_frequency_t frequency,
                                           uint32_t * error_code)
{
  clk_m55_set_frequency_svc_t * p_svc =
      (clk_m55_set_frequency_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clk_m55_set_frequency_svc_t));

  p_svc->send_frequency = frequency;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_ES0_SET_FREQ, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_clocks_set_ES1_frequency(uint32_t services_handle,
 *                                                 clock_frequency_t frequency,
 *                                                 uint32_t * error_code)
 * @brief Set the clock frequency for External System 1 (M55-HE)
 * @param services_handle
 * @param frequency         Clock frequency
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_set_ES1_frequency(uint32_t services_handle,
                                           clock_frequency_t frequency,
                                           uint32_t * error_code)
{
  clk_m55_set_frequency_svc_t * p_svc =
      (clk_m55_set_frequency_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clk_m55_set_frequency_svc_t));

  p_svc->send_frequency = frequency;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_ES1_SET_FREQ, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_clocks_select_a32_source(uint32_t services_handle,
 *                                                 a32_source_t source,
 *                                                 uint32_t * error_code)
 * @brief Select a source clock for the A32 cores
 * @param services_handle
 * @param source            Clock source
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_select_a32_source(uint32_t services_handle,
                                           a32_source_t source,
                                           uint32_t * error_code)
{
  clk_select_sys_clk_source_svc_t * p_svc =
      (clk_select_sys_clk_source_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clk_select_sys_clk_source_svc_t));

  p_svc->send_source = source;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_SELECT_A32_SOURCE, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_clocks_select_aclk_source(uint32_t services_handle,
 *                                                  aclk_source_t source,
 *                                                  uint32_t * error_code)
 * @brief Select a source clock for system buses (AXI, AHB, APB)
 * @param services_handle
 * @param source            Clock source
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_select_aclk_source(uint32_t services_handle,
                                            aclk_source_t source,
                                            uint32_t * error_code)
{
  clk_select_sys_clk_source_svc_t * p_svc =
      (clk_select_sys_clk_source_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clk_select_sys_clk_source_svc_t));

  p_svc->send_source = source;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_SELECT_ACLK_SOURCE, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_clocks_set_divider(uint32_t services_handle,
 *                                           clock_divider_t divider,
 *                                           uint32_t value,
 *                                           uint32_t * error_code)
 * @brief Set the value of a divider
 * @param services_handle
 * @param divider           Which divider
 * @param value             Divider value
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_set_divider(uint32_t services_handle,
                                     clock_divider_t divider,
                                     uint32_t value,
                                     uint32_t * error_code)
{
  clk_set_clk_divider_svc_t * p_svc =
      (clk_set_clk_divider_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clk_set_clk_divider_svc_t));

  p_svc->send_divider = divider;
  p_svc->send_value = value;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_SET_DIVIDER, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_pll_initialize(uint32_t services_handle,
 *                                       uint32_t * error_code)
 * @brief High-level PLL initialize - startup HF XTAL and PLL, and switch to PLL
 * @param services_handle
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_pll_initialize(uint32_t services_handle,
                                 uint32_t * error_code)
{
  generic_svc_t * p_svc =
      (generic_svc_t *)SERVICES_prepare_packet_buffer(sizeof(generic_svc_t));

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_PLL_INITIALIZE, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_pll_deinit(uint32_t services_handle,
 *                                   uint32_t * error_code)
 * @brief High-level PLL deinit - switch to RC clocks, stop PLL and HF XTAL
 * @param services_handle
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_pll_deinit(uint32_t services_handle, uint32_t * error_code)
{
  generic_svc_t * p_svc =
      (generic_svc_t *)SERVICES_prepare_packet_buffer(sizeof(generic_svc_t));

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_PLL_DEINIT, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_pll_xtal_start(uint32_t services_handle,
 *                                       bool faststart,
 *                                       bool boost,
 *                                       uint32_t delay_count,
 *                                       uint32_t * error_code)
 * @brief Start the HF XTAL
 * @param services_handle
 * @param faststart         Enable faststart mode
 * @param boost             Enable boost mode
 * @param delay_count       Wait time
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_pll_xtal_start(uint32_t services_handle,
                                 bool faststart,
                                 bool boost,
                                 uint32_t delay_count,
                                 uint32_t * error_code)
{
  pll_xtal_start_svc_t * p_svc = (pll_xtal_start_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(pll_xtal_start_svc_t));

  p_svc->send_faststart = faststart;
  p_svc->send_boost = boost;
  p_svc->send_delay_count = delay_count;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_PLL_XTAL_START, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_pll_xtal_stop(uint32_t services_handle,
 *                                      uint32_t * error_code)
 * @brief Stop the HF XTAL
 * @param services_handle
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_pll_xtal_stop(uint32_t services_handle,
                                uint32_t * error_code)
{
  generic_svc_t * p_svc =
      (generic_svc_t *)SERVICES_prepare_packet_buffer(sizeof(generic_svc_t));

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_PLL_XTAL_STOP, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_pll_xtal_is_started(uint32_t services_handle,
 *                                            bool * is_started,
 *                                            uint32_t * error_code)
 * @brief Check if the HF XTAL is started
 * @param services_handle
 * @param is_started
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_pll_xtal_is_started(uint32_t services_handle,
                                      bool * is_started,
                                      uint32_t * error_code)
{
  generic_svc_t * p_svc =
      (generic_svc_t *)SERVICES_prepare_packet_buffer(sizeof(generic_svc_t));

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_PLL_XTAL_IS_STARTED, DEFAULT_TIMEOUT);

  *is_started = p_svc->resp_error_code != 0x0;
  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_pll_clkpll_start(uint32_t services_handle,
 *                                         bool faststart,
 *                                         uint32_t delay_count,
 *                                         uint32_t * error_code)
 * @brief Start the PLL
 * @param services_handle
 * @param faststart         Enable faststart mode
 * @param delay_count       Wait time
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_pll_clkpll_start(uint32_t services_handle,
                                   bool faststart,
                                   uint32_t delay_count,
                                   uint32_t * error_code)
{
  pll_clkpll_start_svc_t * p_svc = (pll_clkpll_start_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(pll_clkpll_start_svc_t));

  p_svc->send_faststart = faststart;
  p_svc->send_delay_count = delay_count;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_PLL_CLKPLL_START, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_pll_clkpll_stop(uint32_t services_handle,
 *                                        uint32_t * error_code)
 * @brief Stop the PLL
 * @param services_handle
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_pll_clkpll_stop(uint32_t services_handle,
                                  uint32_t * error_code)
{
  generic_svc_t * p_svc =
      (generic_svc_t *)SERVICES_prepare_packet_buffer(sizeof(generic_svc_t));

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_PLL_CLKPLL_STOP, DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn  uint32_t SERVICES_pll_clkpll_is_locked(uint32_t services_handle,
 *                                             bool * is_locked,
 *                                             uint32_t * error_code)
 * @brief Check if the PLL is started and locked
 * @param services_handle
 * @param is_locked         Which divider
 * @param error_code        Service error code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_pll_clkpll_is_locked(uint32_t services_handle,
                                       bool * is_locked,
                                       uint32_t * error_code)
{
  generic_svc_t * p_svc =
      (generic_svc_t *)SERVICES_prepare_packet_buffer(sizeof(generic_svc_t));

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_PLL_CLKPLL_IS_LOCKED, DEFAULT_TIMEOUT);

  *is_locked = p_svc->resp_error_code != 0x0;
  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @fn uint32_t SERVICES_clocks_setting_get(uint32_t, clock_setting_t, uint32_t*, uint32_t*)
 * @brief                   Get a clock setting from the 'clock_setting_t' enumeration
 * @brief
 * @param services_handle
 * @param setting_type
 * @param value
 * @param error_code
 * @return                  Transport layer error code
 */
uint32_t SERVICES_clocks_setting_get(uint32_t services_handle,
                                     clock_setting_t setting_type,
                                     uint32_t *value,
                                     uint32_t *error_code)
{
  clock_setting_svc_t * p_svc =
      (clock_setting_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(clock_setting_svc_t));

  p_svc->send_setting_type = setting_type;

  uint32_t ret = SERVICES_send_request(services_handle,
      SERVICE_CLOCK_SETTING_GET_REQ_ID, DEFAULT_TIMEOUT);

  if (SERVICES_REQ_SUCCESS == ret)
  {
    *value = p_svc->value;
  }

  *error_code = p_svc->resp_error_code;
  return ret;
}
