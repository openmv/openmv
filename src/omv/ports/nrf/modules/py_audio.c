/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Audio Python module.
 */
#include <stdio.h>
#include "py/obj.h"
#include "py/objarray.h"
#include "py/nlr.h"
#include "py/mphal.h"
#include "py/binary.h"
#include "pendsv.h"
#include "runtime.h"

#include "py_audio.h"
#include "py_assert.h"
#include "py_helper.h"
#include "fb_alloc.h"
#include "omv_boardconfig.h"
#include "common.h"
#include "nrfx_pdm.h"
#include "hal/nrf_gpio.h"

#if MICROPY_PY_AUDIO
#define PDM_DEFAULT_GAIN     20
#define PDM_IRQ_PRIORITY     7
#define PDM_BUFFER_SIZE     (256)

#define NRF_PDM_FREQ_1280K  (nrf_pdm_freq_t)(0x0A000000UL)               ///< PDM_CLK= 1.280 MHz (32 MHz / 25) => Fs= 20000 Hz
#define NRF_PDM_FREQ_2000K  (nrf_pdm_freq_t)(0x10000000UL)               ///< PDM_CLK= 2.000 MHz (32 MHz / 16) => Fs= 31250 Hz
#define NRF_PDM_FREQ_2667K  (nrf_pdm_freq_t)(0x15000000UL)               ///< PDM_CLK= 2.667 MHz (32 MHz / 12) => Fs= 41667 Hz
#define NRF_PDM_FREQ_3200K  (nrf_pdm_freq_t)(0x19000000UL)               ///< PDM_CLK= 3.200 MHz (32 MHz / 10) => Fs= 50000 Hz
#define NRF_PDM_FREQ_4000K  (nrf_pdm_freq_t)(0x20000000UL)               ///< PDM_CLK= 4.000 MHz (32 MHz /  8) => Fs= 62500 Hz
#define RAISE_OS_EXCEPTION(msg)     mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(msg))

static mp_obj_array_t *g_pcmbuf = NULL;
static mp_obj_t g_audio_callback = mp_const_none;
static uint32_t g_channels = 1;
static uint32_t g_buf_index = 0;
static int16_t PDM_BUFFERS[2][PDM_BUFFER_SIZE];

// Pendsv dispatch callback.
static void audio_pendsv_callback(void);

static void nrfx_pdm_event_handler(nrfx_pdm_evt_t const *evt)
{
    if (evt->error == NRFX_PDM_NO_ERROR) {
        if (evt->buffer_requested) {
            if (evt->buffer_released == PDM_BUFFERS[0]) {
                nrfx_pdm_buffer_set(PDM_BUFFERS[1], PDM_BUFFER_SIZE);
            } else { // NULL (first buffer request) or second buffer is full.
                nrfx_pdm_buffer_set(PDM_BUFFERS[0], PDM_BUFFER_SIZE);
            }
        }
        if (evt->buffer_released && g_audio_callback != mp_const_none) {
            g_buf_index = (evt->buffer_released == PDM_BUFFERS[0]) ?  0 : 1;
            pendsv_schedule_dispatch(PENDSV_DISPATCH_AUDIO, audio_pendsv_callback);
        }
    } else {
        // TODO
    }
}

static mp_obj_t py_audio_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    // Read Args.
    g_channels = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_channels), 2);
    uint32_t frequency = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_frequency), 16000);
    int gain_db = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_gain_db), PDM_DEFAULT_GAIN);

    nrfx_pdm_config_t nrfx_pdm_config = {
        .pin_clk            = PDM_CLK_PIN,
        .pin_din            = PDM_DIN_PIN,
        .gain_l             = gain_db,
        .gain_r             = gain_db,
        .interrupt_priority = PDM_IRQ_PRIORITY,
    };

    // Enable high frequency oscillator if not already enabled
    if (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
        NRF_CLOCK->TASKS_HFCLKSTART    = 1;
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
    nrf_gpio_cfg_output(PDM_PWR_PIN);
    nrf_gpio_pin_set(PDM_PWR_PIN);

    // Allocate global PCM buffer.
    g_pcmbuf = mp_obj_new_bytearray_by_ref(PDM_BUFFER_SIZE * sizeof(int16_t), m_new(int16_t, PDM_BUFFER_SIZE));

    nrfx_pdm_init(&nrfx_pdm_config, nrfx_pdm_event_handler);
    return mp_const_none;
}

void py_audio_deinit()
{
    // Disable PDM and IRQ
    nrfx_pdm_uninit();

    // Power the mic off
    nrf_gpio_cfg_output(PDM_PWR_PIN);
    nrf_gpio_pin_clear(PDM_PWR_PIN);
    nrf_gpio_cfg_input(PDM_PWR_PIN, NRF_GPIO_PIN_PULLDOWN);

    g_channels = 0;
    g_pcmbuf = NULL;
    g_buf_index = 0;
    g_audio_callback = mp_const_none;
}

static void audio_pendsv_callback(void)
{
    memcpy(g_pcmbuf->items, PDM_BUFFERS[g_buf_index], PDM_BUFFER_SIZE * sizeof(int16_t));
    // Call user callback
    mp_call_function_1(g_audio_callback, MP_OBJ_FROM_PTR(g_pcmbuf));
}

static mp_obj_t py_audio_start_streaming(mp_obj_t callback_obj)
{
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

static mp_obj_t py_audio_stop_streaming()
{
    // Stop PDM.
    nrfx_pdm_stop();
    g_audio_callback = mp_const_none;
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_init_obj, 0, py_audio_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_audio_start_streaming_obj, py_audio_start_streaming);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_audio_stop_streaming_obj, py_audio_stop_streaming);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_audio)               },
    { MP_ROM_QSTR(MP_QSTR_init),            MP_ROM_PTR(&py_audio_init_obj)           },
    { MP_ROM_QSTR(MP_QSTR_start_streaming), MP_ROM_PTR(&py_audio_start_streaming_obj)},
    { MP_ROM_QSTR(MP_QSTR_stop_streaming),  MP_ROM_PTR(&py_audio_stop_streaming_obj) },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t audio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_audio, audio_module);
#endif //MICROPY_PY_AUDIO
