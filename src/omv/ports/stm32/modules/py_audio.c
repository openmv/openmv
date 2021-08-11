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
#include "systick.h"
#include "pendsv.h"
#include "runtime.h"

#include "py_audio.h"
#include "py_assert.h"
#include "py_helper.h"
#include "pdm2pcm_glo.h"
#include "fb_alloc.h"
#include "omv_boardconfig.h"
#include "common.h"

#if MICROPY_PY_AUDIO

#define RAISE_OS_EXCEPTION(msg)     mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(msg))

static CRC_HandleTypeDef hcrc;
static SAI_HandleTypeDef hsai;
static DMA_HandleTypeDef hdma_sai_rx;

static mp_obj_array_t *g_pcmbuf = NULL;
static mp_obj_t g_audio_callback = mp_const_none;

static int g_channels = AUDIO_SAI_NBR_CHANNELS;
static PDM_Filter_Handler_t  PDM_FilterHandler[2];
static PDM_Filter_Config_t   PDM_FilterConfig[2];

#define DMA_XFER_NONE   (0x00U)
#define DMA_XFER_HALF   (0x01U)
#define DMA_XFER_FULL   (0x04U)
static volatile uint32_t xfer_status = 0;

#define PDM_BUFFER_SIZE     (16384)
// BDMA can only access D3 SRAM4 memory.
uint8_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(PDM_BUFFER[PDM_BUFFER_SIZE], 32), ".d3_dma_buffer");

// Pendsv dispatch callback.
static void audio_pendsv_callback(void);

void AUDIO_SAI_DMA_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hsai.hdmarx);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    xfer_status |= DMA_XFER_HALF;
    SCB_InvalidateDCache_by_Addr((uint32_t *)(&PDM_BUFFER[0]), PDM_BUFFER_SIZE / 2);
    if (g_audio_callback != mp_const_none) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_AUDIO, audio_pendsv_callback);
    }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    xfer_status |= DMA_XFER_FULL;
    SCB_InvalidateDCache_by_Addr((uint32_t *)(&PDM_BUFFER[PDM_BUFFER_SIZE / 2]), PDM_BUFFER_SIZE / 2);
    if (g_audio_callback != mp_const_none) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_AUDIO, audio_pendsv_callback);
    }
}

static uint32_t get_decimation_factor(uint32_t decimation)
{
    switch (decimation) {
        case 16:    return PDM_FILTER_DEC_FACTOR_16;
        case 24:    return PDM_FILTER_DEC_FACTOR_24;
        case 32:    return PDM_FILTER_DEC_FACTOR_32;
        case 48:    return PDM_FILTER_DEC_FACTOR_48;
        case 64:    return PDM_FILTER_DEC_FACTOR_64;
        case 80:    return PDM_FILTER_DEC_FACTOR_80;
        case 128:   return PDM_FILTER_DEC_FACTOR_128;
        default: return 0;
    }
}

static mp_obj_t py_audio_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    // Read Args.
    g_channels = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_channels), 2);
    uint32_t frequency = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_frequency), 16000);
    int gain_db = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_gain_db), 24);
    float highpass = py_helper_keyword_float(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_highpass), 0.9883f);

    // Sanity checks
    if (frequency < 16000 || frequency > 128000) {
        RAISE_OS_EXCEPTION("Invalid frequency!");
    }

    if (g_channels != 1 && g_channels != 2) {
        RAISE_OS_EXCEPTION("Invalid number of channels! Expected 1 or 2.");
    }

    uint32_t decimation_factor = AUDIO_SAI_FREQKHZ / (frequency / 1000);
    uint32_t decimation_factor_const = get_decimation_factor(decimation_factor);
    if (decimation_factor_const == 0) {
        RAISE_OS_EXCEPTION("This frequency is not supported!");
    }
    uint32_t samples_per_channel = (PDM_BUFFER_SIZE * 8) / (decimation_factor * g_channels * 2); // Half a transfer

    hsai.Instance                    = AUDIO_SAI;
    hsai.Init.Protocol               = SAI_FREE_PROTOCOL;
    hsai.Init.AudioMode              = SAI_MODEMASTER_RX;
    hsai.Init.DataSize               = (g_channels == 1) ? SAI_DATASIZE_8 : SAI_DATASIZE_16;
    hsai.Init.FirstBit               = SAI_FIRSTBIT_LSB;
    hsai.Init.ClockStrobing          = SAI_CLOCKSTROBING_RISINGEDGE;
    hsai.Init.Synchro                = SAI_ASYNCHRONOUS;
    hsai.Init.OutputDrive            = SAI_OUTPUTDRIVE_DISABLE;
    hsai.Init.NoDivider              = SAI_MASTERDIVIDER_DISABLE;
    hsai.Init.FIFOThreshold          = SAI_FIFOTHRESHOLD_1QF;
    hsai.Init.SynchroExt             = SAI_SYNCEXT_DISABLE;
    hsai.Init.AudioFrequency         = SAI_AUDIO_FREQUENCY_MCKDIV;
    hsai.Init.MonoStereoMode         = (g_channels == 1)  ? SAI_MONOMODE: SAI_STEREOMODE;
    hsai.Init.CompandingMode         = SAI_NOCOMPANDING;
    hsai.Init.TriState               = SAI_OUTPUT_RELEASED;

    // The master clock output (MCLK_x) is disabled and the SAI clock
    // is passed out to SCK_x bit clock. SCKx frequency = SAI_KER_CK / MCKDIV / 2
    hsai.Init.Mckdiv                 = AUDIO_SAI_MCKDIV; //2.048MHz
    hsai.Init.MckOutput              = SAI_MCK_OUTPUT_DISABLE;
    hsai.Init.MckOverSampling        = SAI_MCK_OVERSAMPLING_DISABLE;

    // Enable and configure PDM mode.
    hsai.Init.PdmInit.Activation     = ENABLE;
    hsai.Init.PdmInit.MicPairsNbr    = 1;
    hsai.Init.PdmInit.ClockEnable    = SAI_PDM_CLOCK1_ENABLE;

    hsai.FrameInit.FrameLength       = 16;
    hsai.FrameInit.ActiveFrameLength = 1;
    hsai.FrameInit.FSDefinition      = SAI_FS_STARTFRAME;
    hsai.FrameInit.FSPolarity        = SAI_FS_ACTIVE_HIGH;
    hsai.FrameInit.FSOffset          = SAI_FS_FIRSTBIT;

    hsai.SlotInit.FirstBitOffset     = 0;
    hsai.SlotInit.SlotSize           = SAI_SLOTSIZE_DATASIZE;
    hsai.SlotInit.SlotNumber         = (g_channels == 1) ? 2 : 1;
    hsai.SlotInit.SlotActive         = (g_channels == 1) ? (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1) : SAI_SLOTACTIVE_0;

    // Initialize the SAI
    HAL_SAI_DeInit(&hsai);
    if (HAL_SAI_Init(&hsai) != HAL_OK) {
        RAISE_OS_EXCEPTION("Failed to init SAI");
    }

    // Enable the DMA clock
    AUDIO_SAI_DMA_CLK_ENABLE();

    // Configure the SAI DMA
    hdma_sai_rx.Instance                 = AUDIO_SAI_DMA_STREAM;
    hdma_sai_rx.Init.Request             = AUDIO_SAI_DMA_REQUEST;
    hdma_sai_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_sai_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_sai_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_sai_rx.Init.PeriphDataAlignment = (g_channels == 1) ? DMA_PDATAALIGN_BYTE : DMA_PDATAALIGN_HALFWORD;
    hdma_sai_rx.Init.MemDataAlignment    = (g_channels == 1) ? DMA_MDATAALIGN_BYTE : DMA_MDATAALIGN_HALFWORD;
    hdma_sai_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_sai_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_sai_rx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hdma_sai_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_sai_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;
    __HAL_LINKDMA(&hsai, hdmarx, hdma_sai_rx);

    // Initialize the DMA stream
    HAL_DMA_DeInit(&hdma_sai_rx);
    if (HAL_DMA_Init(&hdma_sai_rx) != HAL_OK) {
        RAISE_OS_EXCEPTION("SAI DMA init failed!");
    }

    // Configure and enable SAI DMA IRQ Channel
    NVIC_SetPriority(AUDIO_SAI_DMA_IRQ, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(AUDIO_SAI_DMA_IRQ);

    // Init CRC for the PDM library
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    if (HAL_CRC_Init(&hcrc) != HAL_OK) {
        RAISE_OS_EXCEPTION("Failed to initialize CRC!");
    }
    __HAL_CRC_DR_RESET(&hcrc);

    // Configure PDM filters
    for (int i=0; i<g_channels; i++) {
        PDM_FilterHandler[i].bit_order  = PDM_FILTER_BIT_ORDER_MSB;
        PDM_FilterHandler[i].endianness = PDM_FILTER_ENDIANNESS_LE;
        PDM_FilterHandler[i].high_pass_tap = (uint32_t) (highpass * 2147483647U); // coff * (2^31-1)
        PDM_FilterHandler[i].out_ptr_channels = g_channels;
        PDM_FilterHandler[i].in_ptr_channels  = g_channels;
        PDM_Filter_Init(&PDM_FilterHandler[i]);

        PDM_FilterConfig[i].mic_gain = gain_db;
        PDM_FilterConfig[i].output_samples_number = samples_per_channel;
        PDM_FilterConfig[i].decimation_factor = decimation_factor_const;
        PDM_Filter_setConfig(&PDM_FilterHandler[i], &PDM_FilterConfig[i]);
    }

    // Allocate global PCM buffer.
    g_pcmbuf = mp_obj_new_bytearray_by_ref(
            samples_per_channel * g_channels * sizeof(int16_t),
            m_new(int16_t, samples_per_channel * g_channels));

    return mp_const_none;
}

void py_audio_deinit()
{
    // Stop SAI DMA.
    if (hdma_sai_rx.Instance != NULL) {
        HAL_SAI_DMAStop(&hsai);
    }

    // Disable IRQs
    HAL_NVIC_DisableIRQ(AUDIO_SAI_DMA_IRQ);

    if (hsai.Instance != NULL) {
        HAL_SAI_DeInit(&hsai);
        hsai.Instance = NULL;
    }

    if (hdma_sai_rx.Instance != NULL) {
        HAL_DMA_DeInit(&hdma_sai_rx);
        hdma_sai_rx.Instance = NULL;
    }

    g_channels = 0;
    g_pcmbuf = NULL;
    g_audio_callback = mp_const_none;
}

static mp_obj_t py_audio_read_pdm(mp_obj_t buf_in)
{
    mp_buffer_info_t pdmbuf;
    mp_get_buffer_raise(buf_in, &pdmbuf, MP_BUFFER_WRITE);
    size_t typesize = mp_binary_get_size('@', pdmbuf.typecode, NULL);
    uint32_t xfer_samples = 0;
    // Note: samples are copied as bytes for 1 and 2 channels.
    uint32_t n_samples = pdmbuf.len;

    if (typesize != g_channels) {
        // Make sure the buffer type matches the number of channels.
        RAISE_OS_EXCEPTION("Buffer data type does not match the number of channels!");
    }

    // Clear DMA buffer status
    xfer_status &= DMA_XFER_NONE;

    // Start DMA transfer
    if (HAL_SAI_Receive_DMA(&hsai, (uint8_t*) PDM_BUFFER, PDM_BUFFER_SIZE / g_channels) != HAL_OK) {
        RAISE_OS_EXCEPTION("SAI DMA transfer failed!");
    }

    while (n_samples) {
        uint32_t start = HAL_GetTick();
        // Wait for transfer complete.
        while ((xfer_status & DMA_XFER_FULL) == 0) {
            if ((HAL_GetTick() - start) >= 1000) {
                HAL_SAI_DMAStop(&hsai);
                RAISE_OS_EXCEPTION("SAI DMA transfer timeout!");
            }
        }

        // Clear buffer state.
        xfer_status &= DMA_XFER_NONE;

        // Copy samples to pdm output buffer.
        // Note: samples are copied as bytes for 1 and 2 channels.
        uint32_t samples = OMV_MIN(n_samples, PDM_BUFFER_SIZE);
        for (int i=0; i<samples; i++, n_samples--, xfer_samples++) {
            ((uint8_t*)pdmbuf.buf)[xfer_samples] = ((uint8_t *)PDM_BUFFER)[i];
        }

        if (xfer_status & DMA_XFER_FULL) {
            printf("Dropping samples!\n");
        }
    }

    // Stop SAI DMA.
    HAL_SAI_DMAStop(&hsai);

    return mp_const_none;
}

static void audio_pendsv_callback(void)
{
    // Check for half transfer complete.
    if ((xfer_status & DMA_XFER_HALF)) {
        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_HALF);

        // Convert PDM samples to PCM.
        for (int i=0; i<g_channels; i++) {
            PDM_Filter(&((uint8_t*)PDM_BUFFER)[i], &((int16_t*)g_pcmbuf->items)[i], &PDM_FilterHandler[i]);
        }
    } else if ((xfer_status & DMA_XFER_FULL)) { // Check for transfer complete.
        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_FULL);

        // Convert PDM samples to PCM.
        for (int i=0; i<g_channels; i++) {
            PDM_Filter(&((uint8_t*)PDM_BUFFER)[PDM_BUFFER_SIZE / 2 + i], &((int16_t*)g_pcmbuf->items)[i], &PDM_FilterHandler[i]);
        }
    }
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

    // Clear DMA buffer status
    xfer_status &= DMA_XFER_NONE;

    // Start DMA transfer
    if (HAL_SAI_Receive_DMA(&hsai, (uint8_t*) PDM_BUFFER, PDM_BUFFER_SIZE / g_channels) != HAL_OK) {
        g_audio_callback = mp_const_none;
        RAISE_OS_EXCEPTION("SAI DMA transfer failed!");
    }

    return mp_const_none;
}

static mp_obj_t py_audio_stop_streaming()
{
    // Stop SAI DMA.
    HAL_SAI_DMAStop(&hsai);
    g_audio_callback = mp_const_none;
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_init_obj, 0, py_audio_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_audio_read_pdm_obj, py_audio_read_pdm);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_audio_start_streaming_obj, py_audio_start_streaming);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_audio_stop_streaming_obj, py_audio_stop_streaming);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_audio)               },
    { MP_ROM_QSTR(MP_QSTR_init),            MP_ROM_PTR(&py_audio_init_obj)           },
    { MP_ROM_QSTR(MP_QSTR_read_pdm),        MP_ROM_PTR(&py_audio_read_pdm_obj)       },
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
