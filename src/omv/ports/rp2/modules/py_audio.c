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
#include <stdint.h>
#include "py/obj.h"
#include "py/objarray.h"
#include "py/nlr.h"
#include "py/mphal.h"
#include "py/binary.h"
#include "pendsv.h"
#include "runtime.h"

#include "omv_boardconfig.h"
#if MICROPY_PY_AUDIO

#include "py_audio.h"
#include "py_assert.h"
#include "py_helper.h"

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "pdm.pio.h"
#include "OpenPDMFilter.h"

#define PDM_DEFAULT_GAIN        (16)
#define PDM_BUFFER_SIZE         (512)
#define RAISE_OS_EXCEPTION(msg) mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(msg))

static mp_obj_array_t *g_pcmbuf = NULL;
static mp_obj_t g_audio_callback = mp_const_none;
static uint32_t g_buf_index = 0;
static uint32_t g_decimation = 64;
static uint8_t PDM_BUFFERS[2][PDM_BUFFER_SIZE];

// OpenPDM filter used to convert PDM into PCM
static TPDMFilter_InitStruct pdm_filter;

// Pendsv dispatch callback.
static void audio_pendsv_callback(void)
{
    // Call user callback
    mp_call_function_1(g_audio_callback, MP_OBJ_FROM_PTR(g_pcmbuf));
}

static void dma_irq_handler()
{
    // Clear the interrupt request.
    dma_irqn_acknowledge_channel(PDM_DMA, PDM_DMA_CHANNEL);

    // Set next buffer and retrigger the DMA channel.
    dma_channel_set_write_addr(PDM_DMA_CHANNEL, PDM_BUFFERS[g_buf_index ^ 1], true);

    // Convert PDM to PCM samples.
    // TODO: should run this in PendSV context instead.
    if (g_decimation == 64) {
        Open_PDM_Filter_64(PDM_BUFFERS[g_buf_index], g_pcmbuf->items, 1, &pdm_filter);
    } else {
        Open_PDM_Filter_128(PDM_BUFFERS[g_buf_index], g_pcmbuf->items, 1, &pdm_filter);
    }

    // Schedule audio callback.
    pendsv_schedule_dispatch(PENDSV_DISPATCH_AUDIO, audio_pendsv_callback);

    // Swap buffers
    g_buf_index ^= 1;
}

static mp_obj_t py_audio_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    // Read Args.
    uint32_t channels = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_channels), 1);
    uint32_t frequency = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_frequency), 16000);
    int gain = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_gain_db), PDM_DEFAULT_GAIN);

    // Sanity checks
    if (channels != 1) {
        RAISE_OS_EXCEPTION("Invalid number of channels! Expected 1.");
    }

    if (!(frequency == 16000 || frequency == 32000 ||  frequency == 48000)) {
        RAISE_OS_EXCEPTION("Invalid number of channels! Expected 16K, 32K or 48K.");
    }

    uint32_t g_decimation = (frequency == 16000) ? 128 : 64;
    uint32_t n_samples = (PDM_BUFFER_SIZE * 8) / g_decimation;

    // Initialize OpenPDM filter.
    pdm_filter.Fs               = frequency;
    pdm_filter.nSamples         = n_samples;
    pdm_filter.LP_HZ            = frequency / 2;
    pdm_filter.HP_HZ            = 10;
    pdm_filter.In_MicChannels   = channels;
    pdm_filter.Out_MicChannels  = channels;
    pdm_filter.Decimation       = g_decimation;
    pdm_filter.filterGain       = gain;
    Open_PDM_Filter_Init(&pdm_filter);

    // Configure PIO state machine
    float clkDiv = (float) clock_get_hz(clk_sys) / frequency / g_decimation / 2; 
    uint offset = pio_add_program(PDM_PIO, &pdm_pio_program);
    pdm_pio_program_init(PDM_PIO, PDM_SM, offset, PDM_CLK_PIN, PDM_DIN_PIN, clkDiv);

    // Wait for microphone 
    mp_hal_delay_ms(100);

    // Configure DMA for transferring PIO rx buffer to raw buffers
    dma_channel_config c = dma_channel_get_default_config(PDM_DMA_CHANNEL);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(PDM_PIO, PDM_SM, false));
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);

    // Configure DMA channel without starting.
    dma_channel_configure(PDM_DMA_CHANNEL, &c,
        PDM_BUFFERS[0],         // Destinatinon pointer
        &PDM_PIO->rxf[PDM_SM],  // Source pointer
        sizeof(PDM_BUFFERS[0]), // Number of transfers
        false                   // Don't start immediately
    );

    // Set new DMA IRQ handler.
    // Disable IRQs.
    irq_set_enabled(PDM_DMA_IRQ, false);

    // Clear DMA interrupts.
    dma_irqn_acknowledge_channel(PDM_DMA, PDM_DMA_CHANNEL);

    // Remove current handler if any.
    irq_handler_t irq_handler = irq_get_exclusive_handler(PDM_DMA_IRQ);
    if (irq_handler != NULL) {
        irq_remove_handler(PDM_DMA_IRQ, irq_handler);
    }

    // Set new exclusive IRQ handler.
    irq_set_exclusive_handler(PDM_DMA_IRQ, dma_irq_handler);
    // Or set shared IRQ handler, but this needs to be called once.
    // irq_add_shared_handler(PDM_DMA_IRQ, dma_irq_handler, PICO_DEFAULT_IRQ_PRIORITY);

    // Re-enable IRQs.
    irq_set_enabled(PDM_DMA_IRQ, true);
    dma_irqn_set_channel_enabled(PDM_DMA, PDM_DMA_CHANNEL, true);

    // Allocate global PCM buffer.
    g_pcmbuf = mp_obj_new_bytearray_by_ref(n_samples * sizeof(int16_t), m_new(int16_t, n_samples));

    return mp_const_none;
}

void py_audio_deinit()
{
    // Disable PDM and IRQ
    dma_channel_abort(PDM_DMA_CHANNEL);
    dma_irqn_set_channel_enabled(PDM_DMA, PDM_DMA_CHANNEL, false);

    g_pcmbuf = NULL;
    g_buf_index = 0;
    g_audio_callback = mp_const_none;
}

static mp_obj_t py_audio_start_streaming(mp_obj_t callback_obj)
{
    g_audio_callback = callback_obj;

    if (!mp_obj_is_callable(g_audio_callback)) {
        g_audio_callback = mp_const_none;
        RAISE_OS_EXCEPTION("Invalid callback object!");
    }

    dma_channel_start(PDM_DMA_CHANNEL);
    return mp_const_none;
}

static mp_obj_t py_audio_stop_streaming()
{
    py_audio_deinit();
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

MP_REGISTER_MODULE(MP_QSTR_audio, audio_module, MICROPY_PY_AUDIO);
#endif //MICROPY_PY_AUDIO
