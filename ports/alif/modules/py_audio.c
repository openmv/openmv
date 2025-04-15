/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
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
 * Audio Python module.
 */
#if MICROPY_PY_AUDIO

#include <stdio.h>
#include "py/obj.h"
#include "py/objarray.h"
#include "py/nlr.h"
#include "py/mphal.h"
#include "py/binary.h"
#include "runtime.h"

#include "py_assert.h"
#include "py_helper.h"
#include "fb_alloc.h"
#include "omv_boardconfig.h"
#include "omv_common.h"
#include "omv_gpio.h"

#include "sys_ctrl_pdm.h"
#include "pdm.h"
#include "alif_dma.h"
#include "alif_hal.h"

#define PCM_DEFAULT_SAMPLES     (512)
#define PDM_DEFAULT_BUFFERS     (16) // Number of PCM samples buffers.

#define PDM_FIFO_LEVEL          (5)
#define PDM_MODE_16K            (0x04UL)
#define PDM_MODE_32K            (0x05UL)
#define PDM_MODE_48K            (0x07UL)
#define PDM_MODE_96K            (0x08UL)
#define IRQ_PRI_PDM             NVIC_EncodePriority(NVIC_PRIORITYGROUP_7, 10, 0)

typedef struct _audio_state_t {
    uint32_t n_samples;
    uint32_t n_buffers;
    uint32_t t_samples;
    uint32_t buffer_size;
    int16_t *pcm_buffer;
    int16_t *dma_buffer[2];
    mp_obj_array_t *user_buffer;
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile bool streaming;
    volatile bool overflow;
    bool abort_on_overflow;
    mp_obj_t user_callback;
    PDM_Type *pdm_inst;
    dma_channel_t dma_channel;
} audio_state_t;

typedef struct pdm_descr {
    PDM_Type *pdm_inst;
    DMA_Type *dma_inst;
    uint16_t dma_request;
    bool is_lp;
} pdm_descr_t;

static const pdm_descr_t pdm_descr_all[] = {
    #if defined(OMV_PDM0_ID)
    { (PDM_Type *) PDM_BASE, (DMA_Type *) DMA0_NS_BASE, (PDM_DMA_GROUP << 8) | PDM_DMA_PERIPH_REQ, false },
    #else
    { NULL, NULL, 0, false },
    #endif
    #if defined(OMV_PDM1_ID)
    #if CORE_M55_HP
    { (PDM_Type *) LPPDM_BASE, (DMA_Type *) DMA0_NS_BASE, (LPPDM_DMA_GROUP << 8) | LPPDM_DMA_PERIPH_REQ, true },
    #else
    { (PDM_Type *) LPPDM_BASE, (DMA_Type *) DMALOCAL_NS_BASE, (LPPDM_DMA_GROUP << 8) | LPPDM_DMA_PERIPH_REQ, true },
    #endif
    #else
    { NULL, NULL, 0, false },
    #endif
};

#define audio_state     MP_STATE_PORT(_audio_state)
#define NEXT_BUFFER(x)  (((x) + 1) % (audio_state->n_buffers))

static bool audio_initialized = false;

static mp_sched_node_t audio_task_sched_node;
static volatile bool audio_task_scheduled = false;
static void audio_task_callback(mp_sched_node_t *node);

// Note two separate buffers instead of an array so that each is cache-aligned.
// An extra sample must be added due to the way DMA transfer works in mono channel mode.
// TODO we can add 4 to align the next buffer.
static int32_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(DMA_BUFFER0[PCM_DEFAULT_SAMPLES + 1], 32), ".bss.sram0");
static int32_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(DMA_BUFFER1[PCM_DEFAULT_SAMPLES + 1], 32), ".bss.sram0");

void PDM_ERROR_IRQHandler(void) {
    pdm_error_detect_irq_handler(audio_state->pdm_inst);
}

uint16_t gain_to_u84(float gain_db) {
    // Convert gain from dB to linear scale: gain = 10^(gain_db / 20)
    float gain_linear = powf(10.0f, gain_db / 20.0f);
    // Multiply by 16 to convert to 8.4 format (shifting 4 bits to the left)
    return (uint16_t) (gain_linear * 16.0f);
}

static inline int16_t *audio_get_buffer(size_t index) {
    return audio_state->pcm_buffer + (index * audio_state->n_samples);
}

static void dma_transfer_callback(dma_event_t event) {
    if (event & DMA_EVENT_ABORTED) {
        dma_abort(&audio_state->dma_channel, true);
    }

    if (event & DMA_EVENT_COMPLETE) {
        int16_t *pcm_buffer = audio_get_buffer(audio_state->head);
        int32_t *dma_buffer = dma_target_address(&audio_state->dma_channel);

        // Invalidate DMA buffer and copy to PCM buffer.
        SCB_InvalidateDCache_by_Addr(dma_buffer, audio_state->buffer_size);

        for (size_t i = 0; i < audio_state->n_samples; i++) {
            pcm_buffer[i] = dma_buffer[i] >> 16;
        }

        if (NEXT_BUFFER(audio_state->head) != audio_state->tail) {
            // Advance buffer.
            audio_state->head = NEXT_BUFFER(audio_state->head);
            audio_state->t_samples += audio_state->n_samples;
        } else {
            // Use current buffer and set overflow flag.
            audio_state->overflow = true;
        }

        // Schedule user callback
        if (audio_state->user_callback != mp_const_none) {
            mp_sched_schedule_node(&audio_task_sched_node, audio_task_callback);
        }
    }
}

static void audio_task_callback(mp_sched_node_t *node) {
    if (audio_state->streaming == false) {
        return;
    }
    if (audio_state->tail != audio_state->head) {
        audio_state->user_buffer->items = audio_get_buffer(audio_state->tail);
        // Advance tail to next buffer.
        audio_state->tail = NEXT_BUFFER(audio_state->tail);
        // Call user callback.
        mp_call_function_1(audio_state->user_callback, MP_OBJ_FROM_PTR(audio_state->user_buffer));
    } else if (audio_state->overflow == true && audio_state->abort_on_overflow) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Audio buffer overflow."));
    }
}

static mp_obj_t py_audio_init(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_channels, ARG_frequency, ARG_gain_db, ARG_buffers, ARG_samples,  ARG_overflow, ARG_highpass };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_channels, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = OMV_PDM_CHANNELS } },
        { MP_QSTR_frequency, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 16000 } },
        { MP_QSTR_gain_db, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 24 } },
        { MP_QSTR_buffers, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = PDM_DEFAULT_BUFFERS} },
        { MP_QSTR_samples, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1 } },
        { MP_QSTR_overflow, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = true} },
        { MP_QSTR_highpass, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_int = false } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Read Args.
    uint32_t n_channels = args[ARG_channels].u_int;
    uint32_t frequency = args[ARG_frequency].u_int;
    int32_t gain_db = args[ARG_gain_db].u_int;
    uint16_t gain_u84 = gain_to_u84((float) gain_db);

    // Sanity checks
    if (frequency != 16000 && frequency != 32000 &&
        frequency != 48000 && frequency != 96000) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid frequency!"));
    }

    if (n_channels != 1 && n_channels > OMV_PDM_CHANNELS) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid number of channels!"));
    }

    // Default/max PCM buffer size;
    uint32_t n_samples = PCM_DEFAULT_SAMPLES;

    if (args[ARG_samples].u_int > 0) {
        if (args[ARG_samples].u_int % 16 != 0 ||
            args[ARG_samples].u_int > n_samples) {
            mp_raise_msg_varg(&mp_type_ValueError,
                              MP_ERROR_TEXT("Invalid number of samples. The number of samples" \
                                            "must be a multiple of 16 and a maximum of %d"),
                              n_samples);
        }
        n_samples = args[ARG_samples].u_int;
    }

    audio_state = m_new_obj(audio_state_t);
    memset(audio_state, 0, sizeof(audio_state_t));

    const pdm_descr_t *pdm_descr = &pdm_descr_all[OMV_PDM_ID];
    audio_state->pdm_inst = pdm_descr->pdm_inst;
    audio_state->user_callback = mp_const_none;
    audio_state->abort_on_overflow = args[ARG_overflow].u_int;
    audio_state->n_samples = n_samples * n_channels;
    audio_state->n_buffers = args[ARG_buffers].u_int;
    audio_state->buffer_size = n_samples * sizeof(int32_t);
    audio_state->dma_buffer[0] = (void *) DMA_BUFFER0;
    audio_state->dma_buffer[1] = (void *) DMA_BUFFER1;
    audio_state->pcm_buffer = m_new(int16_t, audio_state->n_samples * audio_state->n_buffers);

    // Instead of returning a new heap object for every buffer, which will result in heap
    // fragmentation, reuse the same object and point to PCM buffers returned to the user.
    audio_state->user_buffer = mp_obj_new_bytearray_by_ref(audio_state->n_samples * sizeof(int16_t),
                                                           audio_state->pcm_buffer);

    if (audio_state->pcm_buffer == NULL || audio_state->user_buffer == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to allocate memory for PCM buffers."));
    }

    // Initialize GPIOs and clocks.
    alif_hal_pdm_init(OMV_PDM_ID);

    // Disable and reset PDM state.
    pdm_clear_modes(audio_state->pdm_inst);
    pdm_clear_channel(audio_state->pdm_inst);
    pdm_disable_error_irq(audio_state->pdm_inst);
    pdm_enable_fifo_clear(audio_state->pdm_inst);

    // Enable and configure PDM channels
    pdm_enable_multi_ch(audio_state->pdm_inst, PDM_CHANNEL_3);

    // Configure PDM mode.
    switch (frequency) {
        case 16000:
            pdm_enable_modes(audio_state->pdm_inst, PDM_MODE_16K);
            break;
        case 32000:
            pdm_enable_modes(audio_state->pdm_inst, PDM_MODE_32K);
            break;
        case 48000:
            pdm_enable_modes(audio_state->pdm_inst, PDM_MODE_48K);
            break;
        default:
            pdm_enable_modes(audio_state->pdm_inst, PDM_MODE_96K);
    }

    // Set gain
    pdm_set_ch_gain(audio_state->pdm_inst, 3, gain_u84);

    // Store the FIR/IIR coefficient if enabled.
    if (!args[ARG_highpass].u_int) {
        pdm_bypass_iir(audio_state->pdm_inst, true);
        pdm_bypass_fir(audio_state->pdm_inst, true);
    } else {
        uint32_t ch_iir_coef = 0x00000004;
        uint32_t ch_fir_coef[18] = {
            0x000FFFFA, 0x000FFFFA, 0x00000000,
            0x00000005, 0x000FFFE4, 0x000FFFA8,
            0x000FFFCC, 0x00000096, 0x0000016F,
            0x0000016F, 0x00000096, 0x000FFFCC,
            0x000FFFA8, 0x000FFFE4, 0x00000005,
            0x00000000, 0x000FFFFA, 0x000FFFFA
        };

        pdm_set_fir_coeff(audio_state->pdm_inst, 3, ch_fir_coef);
        pdm_set_ch_iir_coef(audio_state->pdm_inst, 3, ch_iir_coef);
    }

    // Enable the DMA handshake.
    pdm_dma_handshake(pdm_descr->pdm_inst, true);
    // Set FIFO watermark level.
    pdm_set_fifo_watermark(audio_state->pdm_inst, PDM_FIFO_LEVEL);

    // Allocate and configure a DMA channel for PDM
    dma_config_t dma_config;
    dma_config.inst = pdm_descr->dma_inst;
    dma_config.request = pdm_descr->dma_request;
    dma_config.priority = IRQ_PRI_PDM;
    dma_config.direction = DMA_TRANSFER_DEV_TO_MEM;
    dma_config.burst_size = DMA_BURST_SIZE_4;
    dma_config.burst_blen = 1;
    dma_config.byte_swap = DMA_BSWAP_NONE;
    dma_config.flags = DMA_FLAGS_SINGLE_CH;
    if (dma_alloc(&audio_state->dma_channel, &dma_config) != 0) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to allocate DMA channel"));
    }

    // Start the DMA channel. Transfer will not start until later.
    if (dma_start(&audio_state->dma_channel,
                  (void *) pdm_get_ch2_3_addr(audio_state->pdm_inst),
                  audio_state->dma_buffer[0], audio_state->dma_buffer[1],
                  audio_state->buffer_size, dma_transfer_callback) != 0) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to start the DMA"));
    }

    // Configure and enable PDM IRQs.
    NVIC_ClearPendingIRQ(PDM_ERROR_IRQ_IRQn);
    NVIC_EnableIRQ(PDM_ERROR_IRQ_IRQn);

    audio_initialized = true;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_init_obj, 0, py_audio_init);

void py_audio_deinit() {
    if (audio_initialized) {
        // Disable NVIC IRQs.
        NVIC_DisableIRQ(PDM_ERROR_IRQ_IRQn);
        NVIC_ClearPendingIRQ(PDM_ERROR_IRQ_IRQn);

        // Abort DMA channel.
        dma_abort(&audio_state->dma_channel, true);

        // De-initialize PDM.
        pdm_clear_modes(audio_state->pdm_inst);
        pdm_clear_channel(audio_state->pdm_inst);
        pdm_disable_error_irq(audio_state->pdm_inst);
        pdm_enable_fifo_clear(audio_state->pdm_inst);
        pdm_dma_handshake(audio_state->pdm_inst, false);

        // De-initialize clocks and GPIOs.
        alif_hal_pdm_init(OMV_PDM_ID);
        memset(audio_state, 0, sizeof(audio_state_t));
    }

    audio_state = MP_OBJ_NULL;
    audio_initialized = false;
}

static mp_obj_t py_audio_start_streaming(mp_obj_t callback_obj) {
    audio_state->head = 0;
    audio_state->tail = 0;
    audio_state->t_samples = 0;
    audio_state->overflow = false;
    audio_state->streaming = true;

    if (mp_obj_is_callable(callback_obj)) {
        audio_state->user_callback = callback_obj;
    } else {
        audio_state->user_callback = mp_const_none;
    }

    pdm_disable_fifo_clear(audio_state->pdm_inst);
    pdm_dma_enable_irq(audio_state->pdm_inst);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_audio_start_streaming_obj, py_audio_start_streaming);

static mp_obj_t py_audio_stop_streaming() {
    if (audio_state->streaming) {
        audio_state->streaming = false;
        for (mp_uint_t start = mp_hal_ticks_ms();
             (mp_hal_ticks_ms() - start) < 100;
             mp_hal_delay_ms(10)) {
            ;
        }
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_stop_streaming_obj, py_audio_stop_streaming);

static mp_obj_t py_audio_get_buffer(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum {
        ARG_timeout
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_timeout,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };

    // Parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (audio_state->streaming == false) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Audio streaming is not enabled."));
    }

    if (audio_state->overflow == true && audio_state->abort_on_overflow) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Audio buffer overflow."));
    }

    if (mp_obj_is_callable(audio_state->user_callback)) {
        mp_raise_msg(&mp_type_RuntimeError,
                     MP_ERROR_TEXT("Audio streaming with callback function is enabled."));
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); (audio_state->tail == audio_state->head);) {
        if (args[ARG_timeout].u_int && (mp_hal_ticks_ms() - start) >= args[ARG_timeout].u_int) {
            mp_raise_msg(&mp_type_RuntimeError,
                         MP_ERROR_TEXT("Timeout waiting for audio buffer."));
        }
    }

    audio_state->user_buffer->items = audio_get_buffer(audio_state->tail);;

    // Advance head to next buffer.
    audio_state->tail = NEXT_BUFFER(audio_state->tail);

    // Return PCM buffer.
    return MP_OBJ_FROM_PTR(audio_state->user_buffer);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_get_buffer_obj, 0, py_audio_get_buffer);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_audio) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&py_audio_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_start_streaming), MP_ROM_PTR(&py_audio_start_streaming_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop_streaming), MP_ROM_PTR(&py_audio_stop_streaming_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_buffer), MP_ROM_PTR(&py_audio_get_buffer_obj) },
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t audio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_audio, audio_module);
MP_REGISTER_ROOT_POINTER(struct _audio_state_t *_audio_state);
#endif //MICROPY_PY_AUDIO
