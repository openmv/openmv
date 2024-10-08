/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2020-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Audio Python module.
 */
#include <stdio.h>
#include "py/obj.h"
#include "py/objarray.h"
#include "py/nlr.h"
#include "py/mphal.h"
#include "py/binary.h"
#include "runtime.h"

#include "py_audio.h"
#include "py_assert.h"
#include "py_helper.h"
#include "fb_alloc.h"
#include "omv_boardconfig.h"
#include "omv_common.h"
#include "nrfx_pdm.h"
#include "hal/nrf_gpio.h"

#if MICROPY_PY_AUDIO
#define PDM_DEFAULT_GAIN           (20)
#define PDM_IRQ_PRIORITY           (7)
#define PDM_BUFFER_SIZE            (256)

#define NRF_PDM_FREQ_1280K         (nrf_pdm_freq_t) (0x0A000000UL) // PDM_CLK= 1.280 MHz (32 MHz / 25) => Fs= 20000 Hz
#define NRF_PDM_FREQ_2000K         (nrf_pdm_freq_t) (0x10000000UL) // PDM_CLK= 2.000 MHz (32 MHz / 16) => Fs= 31250 Hz
#define NRF_PDM_FREQ_2667K         (nrf_pdm_freq_t) (0x15000000UL) // PDM_CLK= 2.667 MHz (32 MHz / 12) => Fs= 41667 Hz
#define NRF_PDM_FREQ_3200K         (nrf_pdm_freq_t) (0x19000000UL) // PDM_CLK= 3.200 MHz (32 MHz / 10) => Fs= 50000 Hz
#define NRF_PDM_FREQ_4000K         (nrf_pdm_freq_t) (0x20000000UL) // PDM_CLK= 4.000 MHz (32 MHz /  8) => Fs= 62500 Hz
#define RAISE_OS_EXCEPTION(msg)    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(msg))

static mp_obj_array_t *g_pcmbuf = NULL;
static mp_obj_t g_audio_callback = mp_const_none;
static uint32_t g_channels = 1;
static uint32_t g_buf_index = 0;
static int16_t PDM_BUFFERS[2][PDM_BUFFER_SIZE];
static mp_sched_node_t audio_task_sched_node;
static void audio_task_callback(mp_sched_node_t *node);

static void nrfx_pdm_event_handler(nrfx_pdm_evt_t const *evt) {
    if (evt->error == NRFX_PDM_NO_ERROR) {
        if (evt->buffer_requested) {
            if (evt->buffer_released == PDM_BUFFERS[0]) {
                nrfx_pdm_buffer_set(PDM_BUFFERS[1], PDM_BUFFER_SIZE);
            } else {
                // NULL (first buffer request) or second buffer is full.
                nrfx_pdm_buffer_set(PDM_BUFFERS[0], PDM_BUFFER_SIZE);
            }
        }
        if (evt->buffer_released && g_audio_callback != mp_const_none) {
            g_buf_index = (evt->buffer_released == PDM_BUFFERS[0]) ?  0 : 1;
            mp_sched_schedule_node(&audio_task_sched_node, audio_task_callback);
        }
    } else {
        // TODO
    }
}

static mp_obj_t py_audio_init(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_channels, ARG_frequency, ARG_gain_db };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_channels, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 2 } },
        { MP_QSTR_frequency, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 16000 } },
        { MP_QSTR_gain_db, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = PDM_DEFAULT_GAIN } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    g_channels = args[ARG_channels].u_int;
    uint32_t frequency = args[ARG_frequency].u_int;
    int gain_db = args[ARG_gain_db].u_int;

    nrfx_pdm_config_t nrfx_pdm_config = {
        .pin_clk = OMV_PDM_CLK_PIN,
        .pin_din = OMV_PDM_DIN_PIN,
        .gain_l = gain_db,
        .gain_r = gain_db,
        .interrupt_priority = PDM_IRQ_PRIORITY,
    };

    // Enable high frequency oscillator if not already enabled
    if (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
        NRF_CLOCK->TASKS_HFCLKSTART = 1;
        while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
        }
    }

    // configure the sample rate and channels
    switch (frequency) {
        case 16000:
            NRF_PDM->RATIO = ((PDM_RATIO_RATIO_Ratio80 << PDM_RATIO_RATIO_Pos) & PDM_RATIO_RATIO_Msk);
            nrfx_pdm_config.clock_freq = NRF_PDM_FREQ_1280K;
            break;
        case 41667:
            nrfx_pdm_config.clock_freq = NRF_PDM_FREQ_2667K;
            break;
        default:
            RAISE_OS_EXCEPTION("Invalid frequency!");
    }

    switch (g_channels) {
        case 1:
            nrfx_pdm_config.mode = NRF_PDM_MODE_MONO;
            nrfx_pdm_config.edge = NRF_PDM_EDGE_LEFTFALLING;
            break;
        case 2:
            #if 0 // Disable 2 channels for now.
            nrfx_pdm_config.mode = NRF_PDM_MODE_STEREO;
            nrfx_pdm_config.edge = NRF_PDM_EDGE_LEFTFALLING;
            break;
            #endif
        default:
            RAISE_OS_EXCEPTION("Invalid number of channels! Expected 1 or 2.");
    }

    // Power the mic on.
    nrf_gpio_cfg_output(OMV_PDM_PWR_PIN);
    nrf_gpio_pin_set(OMV_PDM_PWR_PIN);

    // Allocate global PCM buffer.
    g_pcmbuf = mp_obj_new_bytearray_by_ref(PDM_BUFFER_SIZE * sizeof(int16_t), m_new(int16_t, PDM_BUFFER_SIZE));

    nrfx_pdm_init(&nrfx_pdm_config, nrfx_pdm_event_handler);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_init_obj, 0, py_audio_init);

void py_audio_deinit() {
    // Disable PDM and IRQ
    nrfx_pdm_uninit();

    // Power the mic off
    nrf_gpio_cfg_output(OMV_PDM_PWR_PIN);
    nrf_gpio_pin_clear(OMV_PDM_PWR_PIN);
    nrf_gpio_cfg_input(OMV_PDM_PWR_PIN, NRF_GPIO_PIN_PULLDOWN);

    g_channels = 0;
    g_pcmbuf = NULL;
    g_buf_index = 0;
    g_audio_callback = mp_const_none;
}

static void audio_task_callback(mp_sched_node_t *node) {
    memcpy(g_pcmbuf->items, PDM_BUFFERS[g_buf_index], PDM_BUFFER_SIZE * sizeof(int16_t));
    // Call user callback
    mp_call_function_1(g_audio_callback, MP_OBJ_FROM_PTR(g_pcmbuf));
}

static mp_obj_t py_audio_start_streaming(mp_obj_t callback_obj) {
    g_audio_callback = callback_obj;

    if (!mp_obj_is_callable(g_audio_callback)) {
        g_audio_callback = mp_const_none;
        RAISE_OS_EXCEPTION("Invalid callback object!");
    }

    // Start PDM.
    if (nrfx_pdm_start() != NRFX_SUCCESS) {
        g_audio_callback = mp_const_none;
        RAISE_OS_EXCEPTION("PDM start failed!");
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_audio_start_streaming_obj, py_audio_start_streaming);

static mp_obj_t py_audio_stop_streaming() {
    // Stop PDM.
    nrfx_pdm_stop();
    g_audio_callback = mp_const_none;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_stop_streaming_obj, py_audio_stop_streaming);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_audio)               },
    { MP_ROM_QSTR(MP_QSTR_init),            MP_ROM_PTR(&py_audio_init_obj)           },
    { MP_ROM_QSTR(MP_QSTR_start_streaming), MP_ROM_PTR(&py_audio_start_streaming_obj)},
    { MP_ROM_QSTR(MP_QSTR_stop_streaming),  MP_ROM_PTR(&py_audio_stop_streaming_obj) },
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t audio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_audio, audio_module);
#endif //MICROPY_PY_AUDIO
