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

#if defined(AUDIO_SAI)
static CRC_HandleTypeDef            hcrc;
static SAI_HandleTypeDef            hsai;
static DMA_HandleTypeDef            hdma_sai_rx;
static PDM_Filter_Config_t          PDM_FilterConfig[AUDIO_MAX_CHANNELS];
static PDM_Filter_Handler_t         PDM_FilterHandler[AUDIO_MAX_CHANNELS];
// NOTE: BDMA can only access D3 SRAM4 memory.
#define PDM_BUFFER_SIZE             (16384)
uint8_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(PDM_BUFFER[PDM_BUFFER_SIZE], 32), ".d3_dma_buffer");
#elif defined(AUDIO_DFSDM)
static DFSDM_Channel_HandleTypeDef  hdfsdm;
// NOTE: Only 1 filter is supported right now.
static DFSDM_Filter_HandleTypeDef   hdfsdm_filter[AUDIO_MAX_CHANNELS];
static DMA_HandleTypeDef            hdma_filter[AUDIO_MAX_CHANNELS];
// NOTE: placed in D2 memory.
#define PDM_BUFFER_SIZE             (512 * 2)
int32_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(PDM_BUFFER[PDM_BUFFER_SIZE], 32), ".d2_dma_buffer");
#define SaturaLH(N, L, H)           (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#else
#error "No audio driver defined for this board"
#endif

static volatile uint32_t xfer_status = 0;
static mp_obj_array_t *g_pcmbuf = NULL;
static mp_obj_t g_audio_callback = mp_const_none;
static int g_channels = AUDIO_MAX_CHANNELS;

#define DMA_XFER_NONE           (0x00U)
#define DMA_XFER_HALF           (0x01U)
#define DMA_XFER_FULL           (0x04U)
#define RAISE_OS_EXCEPTION(msg) mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(msg))

// Pendsv dispatch callback.
static void audio_pendsv_callback(void);

#if defined(AUDIO_SAI)
void AUDIO_SAI_DMA_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hsai.hdmarx);
}
#elif defined(AUDIO_DFSDM)
void AUDIO_DFSDM_FLT0_IRQHandler()
{
    HAL_DFSDM_IRQHandler(&hdfsdm_filter[0]);
}

void AUDIO_DFSDM_FLT0_DMA_IRQHandler()
{
    HAL_DMA_IRQHandler(&hdma_filter[0]);
}
#endif  // defined(AUDIO_SAI)

#if defined(AUDIO_SAI)
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
#elif defined(AUDIO_DFSDM)
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
#endif
{
    xfer_status |= DMA_XFER_HALF;
    SCB_InvalidateDCache_by_Addr((uint32_t *)(&PDM_BUFFER[0]), sizeof(PDM_BUFFER) / 2);
    if (g_audio_callback != mp_const_none) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_AUDIO, audio_pendsv_callback);
    }
}

#if defined(AUDIO_SAI)
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
#elif defined(AUDIO_DFSDM)
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
#endif
{
    xfer_status |= DMA_XFER_FULL;
    SCB_InvalidateDCache_by_Addr((uint32_t *)(&PDM_BUFFER[PDM_BUFFER_SIZE / 2]), sizeof(PDM_BUFFER) / 2);
    if (g_audio_callback != mp_const_none) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_AUDIO, audio_pendsv_callback);
    }
}

#if defined(AUDIO_SAI)
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
#endif

static mp_obj_t py_audio_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    // Read Args.
    g_channels = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_channels), AUDIO_MAX_CHANNELS);
    uint32_t frequency = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_frequency), 16000);
    #if defined(AUDIO_SAI)
    int gain_db = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_gain_db), 24);
    float highpass = py_helper_keyword_float(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_highpass), 0.9883f);
    #endif

    // Sanity checks
    if (frequency < 16000 || frequency > 128000) {
        RAISE_OS_EXCEPTION("Invalid frequency!");
    }

    if (g_channels != 1 && g_channels > AUDIO_MAX_CHANNELS) {
        RAISE_OS_EXCEPTION("Invalid number of channels!");
    }

    #if defined(AUDIO_SAI)
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
    #elif defined(AUDIO_DFSDM)
    hdfsdm.Instance                      = AUDIO_DFSDM;
    hdfsdm.Init.OutputClock.Activation   = ENABLE;
    hdfsdm.Init.OutputClock.Selection    = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
    hdfsdm.Init.OutputClock.Divider      = AUDIO_DFSDM_FREQMHZ / 2;  /* Divider = Aclk / 2MHz*/
    hdfsdm.Init.Input.Multiplexer        = DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hdfsdm.Init.Input.DataPacking        = DFSDM_CHANNEL_STANDARD_MODE;
    hdfsdm.Init.Input.Pins               = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
    hdfsdm.Init.SerialInterface.Type     = DFSDM_CHANNEL_SPI_RISING;
    hdfsdm.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
    hdfsdm.Init.Awd.FilterOrder          = DFSDM_CHANNEL_FASTSINC_ORDER;
    hdfsdm.Init.Awd.Oversampling         = 125; /* 2MHz/125 = 16kHz */
    hdfsdm.Init.Offset                   = 0;
    hdfsdm.Init.RightBitShift            = 0x02;

    __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(&hdfsdm);
    if (HAL_DFSDM_ChannelInit(&hdfsdm) != HAL_OK) {
        RAISE_OS_EXCEPTION("Failed to init DFSDM");
    }

    hdfsdm_filter[0].Instance                          = AUDIO_DFSDM_FLT0;
    hdfsdm_filter[0].Init.RegularParam.Trigger         = DFSDM_FILTER_SW_TRIGGER;
    hdfsdm_filter[0].Init.RegularParam.FastMode        = ENABLE;
    hdfsdm_filter[0].Init.RegularParam.DmaMode         = ENABLE;
    hdfsdm_filter[0].Init.InjectedParam.Trigger        = DFSDM_FILTER_SINC3_ORDER;
    hdfsdm_filter[0].Init.InjectedParam.ScanMode       = ENABLE;
    hdfsdm_filter[0].Init.InjectedParam.DmaMode        = ENABLE;
    hdfsdm_filter[0].Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
    hdfsdm_filter[0].Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
    hdfsdm_filter[0].Init.FilterParam.SincOrder        = DFSDM_FILTER_FASTSINC_ORDER;
    hdfsdm_filter[0].Init.FilterParam.Oversampling     = 125; /* 2MHz/125 = 16kHz */
    hdfsdm_filter[0].Init.FilterParam.IntOversampling  = 1;

    __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&hdfsdm_filter[0]);
    if (HAL_DFSDM_FilterInit(&hdfsdm_filter[0]) != HAL_OK ||
            HAL_DFSDM_FilterConfigRegChannel(&hdfsdm_filter[0],
                AUDIO_DFSDM_CHANNEL, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK) {
        RAISE_OS_EXCEPTION("Failed to init DFSDM filter");
        return 0;
    }

    // Enable the DMA clock
    AUDIO_DFSDM_DMA_CLK_ENABLE();

    // Configure the DFSDM Filter 0 DMA/IRQ
    hdma_filter[0].Instance                 = AUDIO_DFSDM_FLT0_DMA_STREAM;
    hdma_filter[0].Init.Request             = AUDIO_DFSDM_FLT0_DMA_REQUEST;
    hdma_filter[0].Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_filter[0].Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_filter[0].Init.MemInc              = DMA_MINC_ENABLE;
    hdma_filter[0].Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_filter[0].Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma_filter[0].Init.Mode                = DMA_CIRCULAR;
    hdma_filter[0].Init.Priority            = DMA_PRIORITY_HIGH;

    __HAL_LINKDMA(&hdfsdm_filter[0], hdmaInj, hdma_filter[0]);
    __HAL_LINKDMA(&hdfsdm_filter[0], hdmaReg, hdma_filter[0]);

    // Initialize the DMA stream
    HAL_DMA_DeInit(&hdma_filter[0]);
    if (HAL_DMA_Init(&hdma_filter[0]) != HAL_OK) {
        RAISE_OS_EXCEPTION("SAI DFSDM init failed!");
    }

    // Configure and enable DFSDM Filter 0 DMA IRQ.
    NVIC_SetPriority(AUDIO_DFSDM_FLT0_DMA_IRQ, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(AUDIO_DFSDM_FLT0_DMA_IRQ);

    NVIC_SetPriority(AUDIO_DFSDM_FLT0_IRQ, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(AUDIO_DFSDM_FLT0_IRQ);

    uint32_t samples_per_channel = PDM_BUFFER_SIZE / 2; // Half a transfer
    #endif  // defined(AUDIO_SAI)

    // Allocate global PCM buffer.
    g_pcmbuf = mp_obj_new_bytearray_by_ref(
            samples_per_channel * g_channels * sizeof(int16_t),
            m_new(int16_t, samples_per_channel * g_channels));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_init_obj, 0, py_audio_init);

void py_audio_deinit()
{
    #if defined(AUDIO_SAI)
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
    #elif defined(AUDIO_DFSDM)
    if (hdma_filter[0].Instance != NULL) {
        HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm_filter[0]);
    }

    // Disable IRQs
    HAL_NVIC_DisableIRQ(AUDIO_DFSDM_FLT0_DMA_IRQ);
    HAL_NVIC_DisableIRQ(AUDIO_DFSDM_FLT0_IRQ);

    if (hdfsdm.Instance != NULL) {
        HAL_DFSDM_ChannelDeInit(&hdfsdm);
        hdfsdm.Instance = NULL;
    }

    if (hdma_filter[0].Instance != NULL) {
        HAL_DMA_DeInit(&hdma_filter[0]);
        hdma_filter[0].Instance = NULL;
    }
    #endif

    g_channels = 0;
    g_pcmbuf = NULL;
    g_audio_callback = mp_const_none;
}

static void audio_pendsv_callback(void)
{
    // Check for half transfer complete.
    if ((xfer_status & DMA_XFER_HALF)) {
        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_HALF);

        #if defined(AUDIO_SAI)
        // Convert PDM samples to PCM.
        for (int i=0; i<g_channels; i++) {
            PDM_Filter(&((uint8_t*)PDM_BUFFER)[i], &((int16_t*)g_pcmbuf->items)[i], &PDM_FilterHandler[i]);
        }
        #elif defined(AUDIO_DFSDM)
        int16_t *pcmbuf = (int16_t*) g_pcmbuf->items;
        for (int i=0; i<PDM_BUFFER_SIZE / 2; i++) {
            pcmbuf[i] = SaturaLH((PDM_BUFFER[i] >> 8), -32768, 32767);
        }
        #endif
    } else if ((xfer_status & DMA_XFER_FULL)) { // Check for transfer complete.
        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_FULL);

        #if defined(AUDIO_SAI)
        // Convert PDM samples to PCM.
        for (int i=0; i<g_channels; i++) {
            PDM_Filter(&((uint8_t*)PDM_BUFFER)[PDM_BUFFER_SIZE / 2 + i], &((int16_t*)g_pcmbuf->items)[i], &PDM_FilterHandler[i]);
        }
        #elif defined(AUDIO_DFSDM)
        int16_t *pcmbuf = (int16_t*) g_pcmbuf->items;
        for(int i = 0; i < PDM_BUFFER_SIZE / 2; i++) {
            pcmbuf[i] = SaturaLH((PDM_BUFFER[PDM_BUFFER_SIZE / 2 + i] >> 8), -32768, 32767);
        }
        #endif
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

    #if defined(AUDIO_SAI)
    // Start DMA transfer
    if (HAL_SAI_Receive_DMA(&hsai, (uint8_t*) PDM_BUFFER, PDM_BUFFER_SIZE / g_channels) != HAL_OK) {
        g_audio_callback = mp_const_none;
        RAISE_OS_EXCEPTION("SAI DMA transfer failed!");
    }
    #elif defined(AUDIO_DFSDM)
    // Start DMA transfer
    if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm_filter[0], PDM_BUFFER, PDM_BUFFER_SIZE) != HAL_OK) {
        RAISE_OS_EXCEPTION("DFSDM DMA transfer failed!");
    }
    #endif

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_audio_start_streaming_obj, py_audio_start_streaming);

static mp_obj_t py_audio_stop_streaming()
{
    #if defined(AUDIO_SAI)
    // Stop SAI DMA.
    if (hdma_sai_rx.Instance != NULL) {
        HAL_SAI_DMAStop(&hsai);
    }
    #elif defined(AUDIO_DFSDM)
    if (hdma_filter[0].Instance != NULL) {
        HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm_filter[0]);
    }
    #endif
    g_audio_callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_audio_stop_streaming_obj, py_audio_stop_streaming);

#if defined(AUDIO_SAI)
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
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_audio_read_pdm_obj, py_audio_read_pdm);
#endif

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_audio)               },
    { MP_ROM_QSTR(MP_QSTR_init),            MP_ROM_PTR(&py_audio_init_obj)           },
    { MP_ROM_QSTR(MP_QSTR_start_streaming), MP_ROM_PTR(&py_audio_start_streaming_obj)},
    { MP_ROM_QSTR(MP_QSTR_stop_streaming),  MP_ROM_PTR(&py_audio_stop_streaming_obj) },
    #if defined(AUDIO_SAI)
    { MP_ROM_QSTR(MP_QSTR_read_pdm),        MP_ROM_PTR(&py_audio_read_pdm_obj)       },
    #endif
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t audio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_audio, audio_module);
#endif //MICROPY_PY_AUDIO
