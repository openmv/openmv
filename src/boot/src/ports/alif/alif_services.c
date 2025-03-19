/*
 * Copyright (C) 2023-2025 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * SE services helper functions.
 */
#include <stdio.h>
#include <string.h>

#include ALIF_CMSIS_H
#include "alif_services.h"
#include "services_lib_bare_metal.h"

// MHU indices.
#define MHU_M55_SE_MHU0 0
#define MAX_MHU         1

extern void __fatal_error(const char *msg);

static uint32_t se_handle;
static uint32_t se_buffer[SERVICES_MAX_PACKET_BUFFER_SIZE / sizeof(uint32_t)];
static mhu_driver_out_t mhu_driver_out;

static const uint32_t mhu_sender_base_address_list[MAX_MHU] = {
    MHU_SESS_S_TX_BASE,
};

static const uint32_t mhu_receiver_base_address_list[MAX_MHU] = {
    MHU_SESS_S_RX_BASE,
};

void MHU_SESS_S_TX_IRQHandler(void) {
    mhu_driver_out.sender_irq_handler(MHU_M55_SE_MHU0);
}

void MHU_SESS_S_RX_IRQHandler(void) {
    mhu_driver_out.receiver_irq_handler(MHU_M55_SE_MHU0);
}

static int dummy_printf(const char *fmt, ...) {
    (void) fmt;
    return 0;
}

void alif_services_init(void) {
    // Initialize MHU.
    mhu_driver_in_t mhu_driver_in = {
        .sender_base_address_list = (uint32_t *) mhu_sender_base_address_list,
        .receiver_base_address_list = (uint32_t *) mhu_receiver_base_address_list,
        .mhu_count = MAX_MHU,
        .send_msg_acked_callback = SERVICES_send_msg_acked_callback,
        .rx_msg_callback = SERVICES_rx_msg_callback,
        .debug_print = NULL, // not currently used by MHU_driver_initialize
    };
    MHU_driver_initialize(&mhu_driver_in, &mhu_driver_out);

    // Initialize SE services.
    services_lib_t services_init_params = {
        .packet_buffer_address = (uint32_t) se_buffer,
        .fn_send_mhu_message = mhu_driver_out.send_message,
        .fn_wait_ms = NULL, // not currently used by services_host_handler.c
        .wait_timeout = 1000000,
        .fn_print_msg = dummy_printf,
    };
    SERVICES_initialize(&services_init_params);

    // Create SE services channel for sending requests.
    se_handle = SERVICES_register_channel(MHU_M55_SE_MHU0, 0);

    // Enable MHU RX IRQ.
    NVIC_SetPriority(MHU_SESS_S_RX_IRQ_IRQn, 0);
    NVIC_ClearPendingIRQ(MHU_SESS_S_RX_IRQ_IRQn);
    NVIC_EnableIRQ(MHU_SESS_S_RX_IRQ_IRQn);

    // Enable MHU TX IRQ.
    NVIC_SetPriority(MHU_SESS_S_TX_IRQ_IRQn, 0);
    NVIC_ClearPendingIRQ(MHU_SESS_S_TX_IRQ_IRQn);
    NVIC_EnableIRQ(MHU_SESS_S_TX_IRQ_IRQn);

    // Send heartbeat services requests until one succeeds.
    SERVICES_synchronize_with_se(se_handle);

    // Set default run profile
    run_profile_t run_profile = {
        .dcdc_mode = DCDC_MODE_PWM,
        .dcdc_voltage = DCDC_VOUT_0825,
        .aon_clk_src = CLK_SRC_LFXO,
        .run_clk_src = CLK_SRC_PLL,
        #if CORE_M55_HP
        .cpu_clk_freq = CLOCK_FREQUENCY_400MHZ,
        #else
        .cpu_clk_freq = CLOCK_FREQUENCY_160MHZ,
        #endif
        .scaled_clk_freq = SCALED_FREQ_XO_HIGH_DIV_38_4_MHZ,
        .power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK | PD_SYST_MASK |
                         PD_DBSS_MASK | PD_SESS_MASK | PD_SRAMS_MASK | PD_SRAM_CTRL_AON_MASK,
        .memory_blocks = SERAM_MASK | SRAM0_MASK | SRAM1_MASK | MRAM_MASK | BACKUP4K_MASK,
        .ip_clock_gating = OSPI_1_MASK | USB_MASK | LP_PERIPH_MASK,
        .phy_pwr_gating = LDO_PHY_MASK | USB_PHY_MASK,
        .vdd_ioflex_3V3 = IOFLEX_LEVEL_3V3,
    };

    uint32_t error;
    SERVICES_set_run_cfg(se_handle, &run_profile, &error);
    if (error) {
        __fatal_error("SERVICES_set_run_cfg");
    }
}

void alif_services_deinit(void) {
    // Disable MHU RX IRQ.
    NVIC_DisableIRQ(MHU_SESS_S_RX_IRQ_IRQn);
    NVIC_ClearPendingIRQ(MHU_SESS_S_RX_IRQ_IRQn);

    // Disable MHU TX IRQ.
    NVIC_DisableIRQ(MHU_SESS_S_TX_IRQ_IRQn);
    NVIC_ClearPendingIRQ(MHU_SESS_S_TX_IRQ_IRQn);
}

void alif_services_get_unique_id(uint8_t *id, size_t len) {
    uint32_t error;
    SERVICES_version_data_t data = { 0 };
    SERVICES_system_get_device_data(se_handle, &data, &error);
    if (error == 0) {
        memcpy(id, data.MfgData, len);
    }
}
