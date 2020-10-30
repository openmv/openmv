/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Audio Python module.
 */
#include <mp.h>
#include "systick.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py/binary.h"
#include "pdm2pcm_glo.h"
#include "fb_alloc.h"
#include "omv_boardconfig.h"
#include "py/obj.h"
#include "py/objarray.h"

#if MICROPY_PY_AUDIO

#define RAISE_OS_EXCEPTION(msg)     nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, msg))
#define SAI_MIN(a,b)                ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

static CRC_HandleTypeDef hcrc;
static SAI_HandleTypeDef hsai;
static DMA_HandleTypeDef hdma_sai_rx;

static const int n_channels = 2;
static PDM_Filter_Handler_t  PDM_FilterHandler[2];
static PDM_Filter_Config_t   PDM_FilterConfig[2];

#define DMA_XFER_NONE   (0x00U)
#define DMA_XFER_HALF   (0x01U)
#define DMA_XFER_FULL   (0x04U)
static volatile uint32_t xfer_status = 0;

#define PDM_BUFFER_SIZE     (4096)
// BDMA can only access D3 SRAM4 memory.
uint8_t PDM_BUFFER[PDM_BUFFER_SIZE] __attribute__ ((aligned (32))) __attribute__((section(".d3_sram_buffer")));

void AUDIO_SAI_DMA_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hsai.hdmarx);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    xfer_status |= DMA_XFER_HALF;
    SCB_InvalidateDCache_by_Addr((uint32_t *)(&PDM_BUFFER[0]), PDM_BUFFER_SIZE / 2);
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    xfer_status |= DMA_XFER_FULL;
    SCB_InvalidateDCache_by_Addr((uint32_t *)(&PDM_BUFFER[PDM_BUFFER_SIZE / 2]), PDM_BUFFER_SIZE / 2);
}

static mp_obj_t py_audio_init()
{
    hsai.Instance                    = AUDIO_SAI;
    hsai.Init.Protocol               = SAI_FREE_PROTOCOL;
    hsai.Init.AudioMode              = SAI_MODEMASTER_RX;
    hsai.Init.DataSize               = SAI_DATASIZE_16;
    hsai.Init.FirstBit               = SAI_FIRSTBIT_LSB;
    hsai.Init.ClockStrobing          = SAI_CLOCKSTROBING_RISINGEDGE;
    hsai.Init.Synchro                = SAI_ASYNCHRONOUS;
    hsai.Init.OutputDrive            = SAI_OUTPUTDRIVE_DISABLE;
    hsai.Init.NoDivider              = SAI_MASTERDIVIDER_DISABLE;
    hsai.Init.FIFOThreshold          = SAI_FIFOTHRESHOLD_1QF;
    hsai.Init.SynchroExt             = SAI_SYNCEXT_DISABLE;
    hsai.Init.AudioFrequency         = SAI_AUDIO_FREQUENCY_MCKDIV;
    hsai.Init.MonoStereoMode         = SAI_STEREOMODE;
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
    hsai.SlotInit.SlotNumber         = 1;
    hsai.SlotInit.SlotActive         = SAI_SLOTACTIVE_0;

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
    hdma_sai_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_sai_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
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

    return mp_const_none;
}

static mp_obj_t py_audio_read_pdm(mp_obj_t buf_in)
{
    mp_buffer_info_t pdmbuf;
    mp_get_buffer_raise(buf_in, &pdmbuf, MP_BUFFER_WRITE);
    size_t typesize = mp_binary_get_size('@', pdmbuf.typecode, NULL);
    uint32_t xfer_samples = 0;
    uint32_t n_samples = pdmbuf.len / typesize;

    if (typesize != 2) {
        // Make sure the buffer is 16-Bits array.
        RAISE_OS_EXCEPTION("Wrong data type, expected 16-Bits array!");
    }

    // Clear DMA buffer status
    xfer_status &= DMA_XFER_NONE;

    // Start DMA transfer
    if (HAL_SAI_Receive_DMA(&hsai, (uint8_t*) PDM_BUFFER, PDM_BUFFER_SIZE / 2) != HAL_OK) {
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
        uint32_t samples = SAI_MIN(n_samples, PDM_BUFFER_SIZE/2);
        for (int i=0; i<samples; i++, n_samples--, xfer_samples++) {
            ((uint16_t*)pdmbuf.buf)[xfer_samples] = ((uint16_t *)PDM_BUFFER)[i];
        }

        if (xfer_status & DMA_XFER_FULL) {
            printf("Dropping samples!\n");
        }
    }

    // Stop SAI DMA.
    HAL_SAI_DMAStop(&hsai);

    return mp_const_none;
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
        default: RAISE_OS_EXCEPTION("This frequency is not supported!");
    }
}

static mp_obj_t py_audio_read_pcm(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    mp_buffer_info_t pcmbuf;
    mp_get_buffer_raise(args[0], &pcmbuf, MP_BUFFER_WRITE);
    uint32_t frequency = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_frequency), 16);
    int gain_db = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_gain_db), 24);
    float highpass = py_helper_keyword_float(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_highpass), 0.9883f);

    size_t typesize = mp_binary_get_size('@', pcmbuf.typecode, NULL);
    uint32_t n_samples = pcmbuf.len / typesize;
    int16_t *output_buffer = (int16_t *) pcmbuf.buf;
    uint32_t decimation_factor = AUDIO_SAI_FREQKHZ/frequency;
    uint32_t output_samples = ((PDM_BUFFER_SIZE / 2) * 8) / (decimation_factor * n_channels); // Half transfer

    if (typesize != 2) {
        // Make sure the buffer is 16-Bits array.
        RAISE_OS_EXCEPTION("Wrong data type, expected 16-Bits array!");
    }

    // Configure PDM library
    for (int i=0; i<n_channels; i++) {
        PDM_FilterHandler[i].bit_order  = PDM_FILTER_BIT_ORDER_MSB;
        PDM_FilterHandler[i].endianness = PDM_FILTER_ENDIANNESS_LE;
        PDM_FilterHandler[i].high_pass_tap = (uint32_t) (highpass * 2147483647U); // coff * (2^31-1)
        PDM_FilterHandler[i].out_ptr_channels = n_channels;
        PDM_FilterHandler[i].in_ptr_channels  = n_channels;
        PDM_Filter_Init(&PDM_FilterHandler[i]);

        PDM_FilterConfig[i].mic_gain = gain_db;
        PDM_FilterConfig[i].output_samples_number = output_samples;
        PDM_FilterConfig[i].decimation_factor = get_decimation_factor(decimation_factor);
        PDM_Filter_setConfig(&PDM_FilterHandler[i], &PDM_FilterConfig[i]);
    }

    // Clear DMA buffer status
    xfer_status &= DMA_XFER_NONE;

    // Start DMA transfer
    if (HAL_SAI_Receive_DMA(&hsai, (uint8_t*) PDM_BUFFER, PDM_BUFFER_SIZE / 2) != HAL_OK) {
        RAISE_OS_EXCEPTION("SAI DMA transfer failed!");
    }

    while (n_samples) {
        uint32_t start = HAL_GetTick();
        // Wait for half transfer complete.
        while ((xfer_status & DMA_XFER_HALF) == 0) {
            if ((HAL_GetTick() - start) >= 1000) {
                HAL_SAI_DMAStop(&hsai);
                RAISE_OS_EXCEPTION("SAI DMA transfer timeout!");
            }
        }

        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_HALF);

        // Convert PDM samples to PCM.
        for (int i=0; i<n_channels; i++) {
            PDM_Filter(&((uint8_t*)PDM_BUFFER)[i], &output_buffer[i], &PDM_FilterHandler[i]);
        }

        output_buffer += output_samples * 2;

        // Wait for transfer complete.
        while ((xfer_status & DMA_XFER_FULL) == 0) {
            if ((HAL_GetTick() - start) >= 1000) {
                HAL_SAI_DMAStop(&hsai);
                RAISE_OS_EXCEPTION("SAI DMA transfer timeout!");
            }
        }

        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_FULL);

        // Convert PDM samples to PCM.
        for (int i=0; i<n_channels; i++) {
            PDM_Filter(&((uint8_t*)PDM_BUFFER)[PDM_BUFFER_SIZE / 2 + i], &output_buffer[i], &PDM_FilterHandler[i]);
        }

        output_buffer += output_samples * 2;
        n_samples -= output_samples * 4;
    }

    // Stop SAI DMA.
    HAL_SAI_DMAStop(&hsai);

    return mp_const_none;
}

static mp_obj_t py_audio_start_streaming(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    mp_obj_t callback = args[0];
    mp_obj_t arg_obj = args[1];
    uint32_t time = (uint32_t) (py_helper_keyword_float(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_time), 1.0) * 1000);
    uint32_t frequency = py_helper_keyword_int(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_frequency), 16);
    int gain_db = py_helper_keyword_int(n_args, args, 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_gain_db), 24);
    float highpass = py_helper_keyword_float(n_args, args, 5, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_highpass), 0.9883f);

    uint32_t decimation_factor = AUDIO_SAI_FREQKHZ/frequency;
    uint32_t output_samples = ((PDM_BUFFER_SIZE / 2) * 8) / (decimation_factor * n_channels); // Half transfer
    mp_obj_array_t *pcmbuf = mp_obj_new_bytearray_by_ref(output_samples * 4, m_new(int16_t, output_samples * 2));

    // Configure PDM library
    for (int i=0; i<n_channels; i++) {
        PDM_FilterHandler[i].bit_order  = PDM_FILTER_BIT_ORDER_MSB;
        PDM_FilterHandler[i].endianness = PDM_FILTER_ENDIANNESS_LE;
        PDM_FilterHandler[i].high_pass_tap = (uint32_t) (highpass * 2147483647U); // coff * (2^31-1)
        PDM_FilterHandler[i].out_ptr_channels = n_channels;
        PDM_FilterHandler[i].in_ptr_channels  = n_channels;
        PDM_Filter_Init(&PDM_FilterHandler[i]);

        PDM_FilterConfig[i].mic_gain = gain_db;
        PDM_FilterConfig[i].output_samples_number = output_samples;
        PDM_FilterConfig[i].decimation_factor = get_decimation_factor(decimation_factor);
        PDM_Filter_setConfig(&PDM_FilterHandler[i], &PDM_FilterConfig[i]);
    }

    // Clear DMA buffer status
    xfer_status &= DMA_XFER_NONE;

    // Start DMA transfer
    if (HAL_SAI_Receive_DMA(&hsai, (uint8_t*) PDM_BUFFER, PDM_BUFFER_SIZE / 2) != HAL_OK) {
        RAISE_OS_EXCEPTION("SAI DMA transfer failed!");
    }

    uint32_t record_start = HAL_GetTick();
    while ((HAL_GetTick() - record_start) < time) {
        uint32_t start = HAL_GetTick();
        // Wait for half transfer complete.
        while ((xfer_status & DMA_XFER_HALF) == 0) {
            if ((HAL_GetTick() - start) >= 1000) {
                HAL_SAI_DMAStop(&hsai);
                RAISE_OS_EXCEPTION("SAI DMA transfer timeout!");
            }
        }

        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_HALF);

        // Convert PDM samples to PCM.
        for (int i=0; i<n_channels; i++) {
            PDM_Filter(&((uint8_t*)PDM_BUFFER)[i], &((int16_t*)pcmbuf->items)[i], &PDM_FilterHandler[i]);
        }

        mp_call_function_2(callback, MP_OBJ_FROM_PTR(pcmbuf), arg_obj);

        // Wait for transfer complete.
        while ((xfer_status & DMA_XFER_FULL) == 0) {
            if ((HAL_GetTick() - start) >= 1000) {
                HAL_SAI_DMAStop(&hsai);
                RAISE_OS_EXCEPTION("SAI DMA transfer timeout!");
            }
        }

        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_FULL);

        // Convert PDM samples to PCM.
        for (int i=0; i<n_channels; i++) {
            PDM_Filter(&((uint8_t*)PDM_BUFFER)[PDM_BUFFER_SIZE / 2 + i], &((int16_t*)pcmbuf->items)[i], &PDM_FilterHandler[i]);
        }

        mp_call_function_2(callback, MP_OBJ_FROM_PTR(pcmbuf), arg_obj);
    }

    // Stop SAI DMA.
    HAL_SAI_DMAStop(&hsai);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_audio_init_obj, py_audio_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_audio_read_pdm_obj, py_audio_read_pdm);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_read_pcm_obj, 1, py_audio_read_pcm);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_start_streaming_obj, 2, py_audio_start_streaming);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_audio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init),            (mp_obj_t)&py_audio_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_pcm),        (mp_obj_t)&py_audio_read_pcm_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_pdm),        (mp_obj_t)&py_audio_read_pdm_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start_streaming), (mp_obj_t)&py_audio_start_streaming_obj },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t audio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};

#endif //MICROPY_PY_AUDIO
