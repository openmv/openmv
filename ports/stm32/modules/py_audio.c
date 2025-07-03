/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
#include "systick.h"
#include "runtime.h"

#include "py_audio.h"
#include "py_assert.h"
#include "py_helper.h"
#include "pdm2pcm_glo.h"
#include "fb_alloc.h"
#include "omv_boardconfig.h"
#include "omv_common.h"
#include "stm_dma.h"

#if MICROPY_PY_AUDIO

#if defined(OMV_SAI)
static CRC_HandleTypeDef hcrc;
static SAI_HandleTypeDef hsai;
static DMA_HandleTypeDef hdma_sai_rx;

static PDM_Filter_Config_t PDM_FilterConfig[OMV_AUDIO_MAX_CHANNELS];
static PDM_Filter_Handler_t PDM_FilterHandler[OMV_AUDIO_MAX_CHANNELS];
// NOTE: BDMA can only access D3 SRAM4 memory.
#define PDM_BUFFER_SIZE      (16384)
uint8_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(PDM_BUFFER[PDM_BUFFER_SIZE], 32), ".d3_dma_buffer");

#elif defined(OMV_DFSDM)
static DFSDM_Channel_HandleTypeDef hdfsdm;

// NOTE: Only 1 filter is supported right now.
static DFSDM_Filter_HandleTypeDef hdfsdm_filter[OMV_AUDIO_MAX_CHANNELS];
static DMA_HandleTypeDef hdma_filter[OMV_AUDIO_MAX_CHANNELS];

// NOTE: placed in D2 memory.
#define PDM_BUFFER_SIZE      (512 * 2)
int32_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(PDM_BUFFER[PDM_BUFFER_SIZE], 32), ".d2_dma_buffer");

#define DFSDM_GAIN_FRAC_BITS (3)
static int32_t dfsdm_gain = 1;

#elif defined(OMV_MDF)
static MDF_HandleTypeDef hmdf;
static MDF_FilterConfigTypeDef hmdf_filter[OMV_AUDIO_MAX_CHANNELS];

// NOTE: Only 1 filter is supported right now.
static DMA_QListTypeDef hdma_queue;
static DMA_NodeTypeDef OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(hdma_node, 32), ".dma_buffer");
static DMA_HandleTypeDef hdma_filter[OMV_AUDIO_MAX_CHANNELS];

#define PDM_BUFFER_SIZE      (512 * 2)
int32_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(PDM_BUFFER[PDM_BUFFER_SIZE], 32), ".dma_buffer");
#else
#error "No audio driver defined for this board"
#endif

static volatile uint32_t xfer_status = 0;
static int g_channels = OMV_AUDIO_MAX_CHANNELS;
static uint32_t g_pdm_buffer_size = 0;
static mp_sched_node_t audio_task_sched_node;

#define DMA_XFER_NONE              (0x00U)
#define DMA_XFER_HALF              (0x01U)
#define DMA_XFER_FULL              (0x04U)
#define RAISE_OS_EXCEPTION(msg)    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(msg))

// Scheduler callback.
static void audio_task_callback(mp_sched_node_t *node);

#if defined(OMV_SAI)
void OMV_SAI_DMA_IRQHandler(void) {
    HAL_DMA_IRQHandler(hsai.hdmarx);
}
#elif defined(OMV_DFSDM)
void OMV_DFSDM_FLT0_IRQHandler(void) {
    HAL_DFSDM_IRQHandler(&hdfsdm_filter[0]);
}
#elif defined(OMV_MDF)
void OMV_MDF_FLT0_IRQHandler(void) {
    HAL_MDF_IRQHandler(&hmdf);
}
#endif  // defined(OMV_SAI)

#if defined(OMV_SAI)
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
#elif defined(OMV_DFSDM)
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
#elif defined(OMV_MDF)
void HAL_MDF_AcqHalfCpltCallback(MDF_HandleTypeDef *hmdf)
#endif
{
    xfer_status |= DMA_XFER_HALF;
    if (MP_STATE_PORT(audio_callback) != mp_const_none) {
        mp_sched_schedule_node(&audio_task_sched_node, audio_task_callback);
    }
}

#if defined(OMV_SAI)
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
#elif defined(OMV_DFSDM)
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
#elif defined(OMV_MDF)
void HAL_MDF_AcqCpltCallback(MDF_HandleTypeDef *hmdf)
#endif
{
    xfer_status |= DMA_XFER_FULL;
    if (MP_STATE_PORT(audio_callback) != mp_const_none) {
        mp_sched_schedule_node(&audio_task_sched_node, audio_task_callback);
    }
}

#if defined(OMV_SAI)
static uint32_t get_decimation_factor(uint32_t decimation) {
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

static mp_obj_t py_audio_init(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_channels, ARG_frequency, ARG_gain_db, ARG_highpass, ARG_samples };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_channels, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = OMV_AUDIO_MAX_CHANNELS } },
        { MP_QSTR_frequency, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 16000 } },
        { MP_QSTR_gain_db, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 24 } },
        { MP_QSTR_highpass, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_samples, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1 } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Read Args.
    g_channels = args[ARG_channels].u_int;
    uint32_t frequency = args[ARG_frequency].u_int;
    int gain_db = args[ARG_gain_db].u_int;
    #if defined(OMV_SAI)
    float highpass = py_helper_arg_to_float(args[ARG_highpass].u_obj, 0.9883f);
    #endif

    // Sanity checks
    if (frequency < 16000 || frequency > 128000) {
        RAISE_OS_EXCEPTION("Invalid frequency!");
    }

    if (g_channels != 1 && g_channels > OMV_AUDIO_MAX_CHANNELS) {
        RAISE_OS_EXCEPTION("Invalid number of channels!");
    }

    // Default/max PDM buffer size;
    g_pdm_buffer_size = PDM_BUFFER_SIZE;

    #if defined(OMV_MDF)
    uint32_t samples_per_channel = PDM_BUFFER_SIZE / 2; // Half a transfer
    #elif defined(OMV_DFSDM)
    uint32_t samples_per_channel = PDM_BUFFER_SIZE / 2; // Half a transfer
    dfsdm_gain = __USAT(fast_roundf(expf((gain_db / 20.0f) * M_LN10) * (1 << DFSDM_GAIN_FRAC_BITS)), 15);
    #else
    uint32_t decimation_factor = OMV_SAI_FREQKHZ / (frequency / 1000);
    uint32_t decimation_factor_const = get_decimation_factor(decimation_factor);
    if (decimation_factor_const == 0) {
        RAISE_OS_EXCEPTION("This frequency is not supported!");
    }
    uint32_t samples_per_channel = (PDM_BUFFER_SIZE * 8) / (decimation_factor * g_channels * 2); // Half a transfer
    #endif  // defined(OMV_DFSDM)

    if (args[ARG_samples].u_int > 0) {
        if (args[ARG_samples].u_int % 16 != 0 ||
            args[ARG_samples].u_int > samples_per_channel) {
            mp_raise_msg_varg(&mp_type_ValueError,
                              MP_ERROR_TEXT("Invalid number of samples."                      \
                                            "The number of samples must be a multiple of 16," \
                                            "and a maximum of %d"),
                              samples_per_channel);
        }
        samples_per_channel = args[ARG_samples].u_int;
        // Recalculate the PDM buffer size for the requested samples.
        #if defined(OMV_DFSDM) || defined(OMV_MDF)
        g_pdm_buffer_size = samples_per_channel * 2;
        #else
        g_pdm_buffer_size = (samples_per_channel * decimation_factor * g_channels) / 4;
        #endif  // defined(OMV_DFSDM)
    }

    #if defined(OMV_SAI)
    hsai.Instance = OMV_SAI;
    hsai.Init.Protocol = SAI_FREE_PROTOCOL;
    hsai.Init.AudioMode = SAI_MODEMASTER_RX;
    hsai.Init.DataSize = (g_channels == 1) ? SAI_DATASIZE_8 : SAI_DATASIZE_16;
    hsai.Init.FirstBit = SAI_FIRSTBIT_LSB;
    hsai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
    hsai.Init.Synchro = SAI_ASYNCHRONOUS;
    hsai.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
    hsai.Init.NoDivider = SAI_MASTERDIVIDER_DISABLE;
    hsai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
    hsai.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
    hsai.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_MCKDIV;
    hsai.Init.MonoStereoMode = (g_channels == 1)  ? SAI_MONOMODE: SAI_STEREOMODE;
    hsai.Init.CompandingMode = SAI_NOCOMPANDING;
    hsai.Init.TriState = SAI_OUTPUT_RELEASED;

    // The master clock output (MCLK_x) is disabled and the SAI clock
    // is passed out to SCK_x bit clock. SCKx frequency = SAI_KER_CK / MCKDIV / 2
    hsai.Init.Mckdiv = OMV_SAI_MCKDIV;                 //2.048MHz
    hsai.Init.MckOutput = SAI_MCK_OUTPUT_DISABLE;
    hsai.Init.MckOverSampling = SAI_MCK_OVERSAMPLING_DISABLE;

    // Enable and configure PDM mode.
    hsai.Init.PdmInit.Activation = ENABLE;
    hsai.Init.PdmInit.MicPairsNbr = 1;
    hsai.Init.PdmInit.ClockEnable = SAI_PDM_CLOCK1_ENABLE;

    hsai.FrameInit.FrameLength = 16;
    hsai.FrameInit.ActiveFrameLength = 1;
    hsai.FrameInit.FSDefinition = SAI_FS_STARTFRAME;
    hsai.FrameInit.FSPolarity = SAI_FS_ACTIVE_HIGH;
    hsai.FrameInit.FSOffset = SAI_FS_FIRSTBIT;

    hsai.SlotInit.FirstBitOffset = 0;
    hsai.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
    hsai.SlotInit.SlotNumber = (g_channels == 1) ? 2 : 1;
    hsai.SlotInit.SlotActive = (g_channels == 1) ? (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1) : SAI_SLOTACTIVE_0;

    // Initialize the SAI
    HAL_SAI_DeInit(&hsai);
    if (HAL_SAI_Init(&hsai) != HAL_OK) {
        RAISE_OS_EXCEPTION("Failed to init SAI");
    }

    // Enable the DMA clock
    OMV_SAI_DMA_CLK_ENABLE();

    // Configure the SAI DMA
    hdma_sai_rx.Instance = OMV_SAI_DMA_STREAM;
    hdma_sai_rx.Init.Request = OMV_SAI_DMA_REQUEST;
    hdma_sai_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sai_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sai_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sai_rx.Init.PeriphDataAlignment = (g_channels == 1) ? DMA_PDATAALIGN_BYTE : DMA_PDATAALIGN_HALFWORD;
    hdma_sai_rx.Init.MemDataAlignment = (g_channels == 1) ? DMA_MDATAALIGN_BYTE : DMA_MDATAALIGN_HALFWORD;
    hdma_sai_rx.Init.Mode = DMA_CIRCULAR;
    hdma_sai_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_sai_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sai_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_sai_rx.Init.PeriphBurst = DMA_MBURST_SINGLE;
    __HAL_LINKDMA(&hsai, hdmarx, hdma_sai_rx);

    // Initialize the DMA stream
    HAL_DMA_DeInit(&hdma_sai_rx);
    if (HAL_DMA_Init(&hdma_sai_rx) != HAL_OK) {
        RAISE_OS_EXCEPTION("SAI DMA init failed!");
    }

    // Configure and enable SAI DMA IRQ Channel
    NVIC_SetPriority(OMV_SAI_DMA_IRQ, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(OMV_SAI_DMA_IRQ);

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
    for (int i = 0; i < g_channels; i++) {
        PDM_FilterHandler[i].bit_order = PDM_FILTER_BIT_ORDER_MSB;
        PDM_FilterHandler[i].endianness = PDM_FILTER_ENDIANNESS_LE;
        PDM_FilterHandler[i].high_pass_tap = (uint32_t) (highpass * (float) 2147483647U); // coff * (2^31-1)
        PDM_FilterHandler[i].out_ptr_channels = g_channels;
        PDM_FilterHandler[i].in_ptr_channels = g_channels;
        PDM_Filter_Init(&PDM_FilterHandler[i]);

        PDM_FilterConfig[i].mic_gain = gain_db;
        PDM_FilterConfig[i].output_samples_number = samples_per_channel;
        PDM_FilterConfig[i].decimation_factor = decimation_factor_const;
        PDM_Filter_setConfig(&PDM_FilterHandler[i], &PDM_FilterConfig[i]);
    }
    #elif defined(OMV_DFSDM)
    hdfsdm.Instance = OMV_DFSDM;
    hdfsdm.Init.OutputClock.Activation = ENABLE;
    hdfsdm.Init.OutputClock.Selection = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
    hdfsdm.Init.OutputClock.Divider = OMV_DFSDM_FREQMHZ / 2;       /* Divider = Aclk / 2MHz*/
    hdfsdm.Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hdfsdm.Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
    hdfsdm.Init.Input.Pins = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
    hdfsdm.Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_RISING;
    hdfsdm.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
    hdfsdm.Init.Awd.FilterOrder = DFSDM_CHANNEL_FASTSINC_ORDER;
    hdfsdm.Init.Awd.Oversampling = 125;         /* 2MHz/125 = 16kHz */
    hdfsdm.Init.Offset = 0;
    hdfsdm.Init.RightBitShift = 0x02;

    __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(&hdfsdm);
    if (HAL_DFSDM_ChannelInit(&hdfsdm) != HAL_OK) {
        RAISE_OS_EXCEPTION("Failed to init DFSDM");
    }

    hdfsdm_filter[0].Instance = OMV_DFSDM_FLT0;
    hdfsdm_filter[0].Init.RegularParam.Trigger = DFSDM_FILTER_SW_TRIGGER;
    hdfsdm_filter[0].Init.RegularParam.FastMode = ENABLE;
    hdfsdm_filter[0].Init.RegularParam.DmaMode = ENABLE;
    hdfsdm_filter[0].Init.InjectedParam.Trigger = DFSDM_FILTER_SINC3_ORDER;
    hdfsdm_filter[0].Init.InjectedParam.ScanMode = ENABLE;
    hdfsdm_filter[0].Init.InjectedParam.DmaMode = ENABLE;
    hdfsdm_filter[0].Init.InjectedParam.ExtTrigger = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
    hdfsdm_filter[0].Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
    hdfsdm_filter[0].Init.FilterParam.SincOrder = DFSDM_FILTER_FASTSINC_ORDER;
    hdfsdm_filter[0].Init.FilterParam.Oversampling = 125;     /* 2MHz/125 = 16kHz */
    hdfsdm_filter[0].Init.FilterParam.IntOversampling = 1;

    __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&hdfsdm_filter[0]);
    if (HAL_DFSDM_FilterInit(&hdfsdm_filter[0]) != HAL_OK ||
        HAL_DFSDM_FilterConfigRegChannel(&hdfsdm_filter[0],
                                         OMV_DFSDM_CHANNEL, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK) {
        RAISE_OS_EXCEPTION("Failed to init DFSDM filter");
        return 0;
    }

    // Enable the DMA clock
    OMV_DFSDM_DMA_CLK_ENABLE();

    // Configure the DFSDM Filter 0 DMA/IRQ
    hdma_filter[0].Instance = OMV_DFSDM_FLT0_DMA_STREAM;
    hdma_filter[0].Init.Request = OMV_DFSDM_FLT0_DMA_REQUEST;
    hdma_filter[0].Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_filter[0].Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_filter[0].Init.MemInc = DMA_MINC_ENABLE;
    hdma_filter[0].Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_filter[0].Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_filter[0].Init.Mode = DMA_CIRCULAR;
    hdma_filter[0].Init.Priority = DMA_PRIORITY_HIGH;

    __HAL_LINKDMA(&hdfsdm_filter[0], hdmaInj, hdma_filter[0]);
    __HAL_LINKDMA(&hdfsdm_filter[0], hdmaReg, hdma_filter[0]);

    // Set DMA IRQ handle
    stm_dma_set_irq_descr(OMV_DFSDM_FLT0_DMA_STREAM, &hdma_filter[0]);

    // Initialize the DMA stream
    HAL_DMA_DeInit(&hdma_filter[0]);
    if (HAL_DMA_Init(&hdma_filter[0]) != HAL_OK) {
        RAISE_OS_EXCEPTION("SAI DFSDM init failed!");
    }

    // Configure and enable DFSDM Filter 0 DMA IRQ.
    NVIC_SetPriority(OMV_DFSDM_FLT0_DMA_IRQ, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(OMV_DFSDM_FLT0_DMA_IRQ);

    NVIC_SetPriority(OMV_DFSDM_FLT0_IRQ, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(OMV_DFSDM_FLT0_IRQ);
    #elif defined(OMV_MDF)
    hmdf.Instance = OMV_MDF;
    hmdf.Init.CommonParam.InterleavedFilters = 0;
    hmdf.Init.CommonParam.ProcClockDivider = OMV_MDF_PROC_CLKDIV;
    hmdf.Init.CommonParam.OutputClock.Activation = ENABLE;
    hmdf.Init.CommonParam.OutputClock.Pins = MDF_OUTPUT_CLOCK_0;
    hmdf.Init.CommonParam.OutputClock.Divider = OMV_MDF_CCKY_CLKDIV;
    hmdf.Init.CommonParam.OutputClock.Trigger.Activation = DISABLE;
    hmdf.Init.SerialInterface.Activation = ENABLE;
    hmdf.Init.SerialInterface.Mode = MDF_SITF_LF_MASTER_SPI_MODE;
    hmdf.Init.SerialInterface.ClockSource = MDF_SITF_CCK0_SOURCE;
    hmdf.Init.SerialInterface.Threshold = 31;
    hmdf.Init.FilterBistream = MDF_BITSTREAM0_FALLING;
    if (HAL_MDF_Init(&hmdf) != HAL_OK) {
        RAISE_OS_EXCEPTION("MDF init failed!");
    }

    hmdf_filter[0].DataSource = MDF_DATA_SOURCE_BSMX;
    hmdf_filter[0].Delay = 0;
    hmdf_filter[0].CicMode = MDF_ONE_FILTER_SINC4;
    hmdf_filter[0].DecimationRatio = 32;
    hmdf_filter[0].Offset = 0;
    hmdf_filter[0].Gain = gain_db / 3;  // gain in steps of 3db
    hmdf_filter[0].ReshapeFilter.Activation = ENABLE;
    hmdf_filter[0].ReshapeFilter.DecimationRatio = MDF_RSF_DECIMATION_RATIO_4;
    hmdf_filter[0].HighPassFilter.Activation = DISABLE; // Disabled for now.
    hmdf_filter[0].HighPassFilter.CutOffFrequency = MDF_HPF_CUTOFF_0_000625FPCM;
    hmdf_filter[0].Integrator.Activation = DISABLE;
    hmdf_filter[0].SoundActivity.Activation = DISABLE;
    hmdf_filter[0].AcquisitionMode = MDF_MODE_ASYNC_CONT;
    hmdf_filter[0].FifoThreshold = MDF_FIFO_THRESHOLD_NOT_EMPTY;
    hmdf_filter[0].DiscardSamples = 0;

    DMA_NodeConfTypeDef dma_ncfg;

    dma_ncfg.NodeType = DMA_GPDMA_LINEAR_NODE;
    dma_ncfg.Init.Mode = DMA_NORMAL;
    dma_ncfg.Init.Request = OMV_MDF_FLT0_DMA_REQUEST;
    dma_ncfg.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    dma_ncfg.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_ncfg.Init.SrcInc = DMA_SINC_FIXED;
    dma_ncfg.Init.DestInc = DMA_DINC_INCREMENTED;
    dma_ncfg.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
    dma_ncfg.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
    dma_ncfg.Init.SrcBurstLength = 1;
    dma_ncfg.Init.DestBurstLength = 1;
    dma_ncfg.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
    dma_ncfg.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;

    dma_ncfg.SrcSecure = DMA_CHANNEL_SRC_SEC;
    dma_ncfg.DestSecure = DMA_CHANNEL_DEST_SEC;
    dma_ncfg.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
    dma_ncfg.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    dma_ncfg.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;

    if (HAL_DMAEx_List_BuildNode(&dma_ncfg, &hdma_node) != HAL_OK ||
        HAL_DMAEx_List_InsertNode(&hdma_queue, NULL, &hdma_node) != HAL_OK ||
        HAL_DMAEx_List_SetCircularMode(&hdma_queue) != HAL_OK) {
        RAISE_OS_EXCEPTION("MDF DMA init failed!");
    }

    hdma_filter[0].Instance = OMV_MDF_FLT0_DMA_STREAM;
    hdma_filter[0].InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    hdma_filter[0].InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    hdma_filter[0].InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    hdma_filter[0].InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    hdma_filter[0].InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;

    if (HAL_DMAEx_List_Init(&hdma_filter[0]) != HAL_OK ||
        HAL_DMAEx_List_LinkQ(&hdma_filter[0], &hdma_queue) != HAL_OK) {
        RAISE_OS_EXCEPTION("MDF DMA init failed!");
    }

    __HAL_LINKDMA(&hmdf, hdma, hdma_filter[0]);

    if (HAL_DMA_ConfigChannelAttributes(&hdma_filter[0],
                                        DMA_CHANNEL_PRIV | DMA_CHANNEL_SEC |
                                        DMA_CHANNEL_SRC_SEC | DMA_CHANNEL_DEST_SEC) != HAL_OK) {
        RAISE_OS_EXCEPTION("MDF DMA init failed!");
    }

    // Set DMA IRQ handle
    stm_dma_set_irq_descr(OMV_MDF_FLT0_DMA_STREAM, &hdma_filter[0]);

    // Configure and enable MDF Filter 0 DMA IRQ.
    NVIC_SetPriority(OMV_MDF_FLT0_IRQ, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(OMV_MDF_FLT0_IRQ);

    NVIC_SetPriority(OMV_MDF_FLT0_DMA_IRQ, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(OMV_MDF_FLT0_DMA_IRQ);
    #endif  // defined(OMV_SAI)

    // Allocate global PCM buffer.
    MP_STATE_PORT(audio_pcm_buffer) = m_new(int16_t, samples_per_channel * g_channels);
    MP_STATE_PORT(audio_pcm_array) = mp_obj_new_bytearray_by_ref(samples_per_channel * g_channels * sizeof(int16_t),
                                                                 MP_STATE_PORT(audio_pcm_buffer));

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_audio_init_obj, 0, py_audio_init);

void py_audio_deinit() {
    #if defined(OMV_SAI)
    // Stop SAI DMA.
    if (hdma_sai_rx.Instance != NULL) {
        HAL_SAI_DMAStop(&hsai);
    }

    // Disable IRQs
    HAL_NVIC_DisableIRQ(OMV_SAI_DMA_IRQ);

    if (hsai.Instance != NULL) {
        HAL_SAI_DeInit(&hsai);
        hsai.Instance = NULL;
    }

    if (hdma_sai_rx.Instance != NULL) {
        HAL_DMA_DeInit(&hdma_sai_rx);
        hdma_sai_rx.Instance = NULL;
    }
    #elif defined(OMV_MDF)
    if (hmdf.Instance != NULL) {
        HAL_MDF_AcqStop_DMA(&hmdf);
        HAL_MDF_DeInit(&hmdf);
    }

    // Disable IRQs
    HAL_NVIC_DisableIRQ(OMV_MDF_FLT0_IRQ);
    HAL_NVIC_DisableIRQ(OMV_MDF_FLT0_DMA_IRQ);
    #elif defined(OMV_DFSDM)
    if (hdma_filter[0].Instance != NULL) {
        HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm_filter[0]);
    }

    // Disable IRQs
    HAL_NVIC_DisableIRQ(OMV_DFSDM_FLT0_DMA_IRQ);
    HAL_NVIC_DisableIRQ(OMV_DFSDM_FLT0_IRQ);

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
    MP_STATE_PORT(audio_pcm_buffer) = NULL;
    MP_STATE_PORT(audio_pcm_array) = mp_const_none;
    MP_STATE_PORT(audio_callback) = mp_const_none;
}

static void audio_task_callback(mp_sched_node_t *node) {
    int16_t *pcmbuf = (int16_t *) MP_STATE_PORT(audio_pcm_buffer);

    // Check for half transfer complete.
    if ((xfer_status & DMA_XFER_HALF)) {
        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_HALF);

        #if defined(OMV_SAI)
        // Convert PDM samples to PCM.
        for (int i = 0; i < g_channels; i++) {
            PDM_Filter(&((uint8_t *) PDM_BUFFER)[i], &pcmbuf[i], &PDM_FilterHandler[i]);
        }
        #elif defined(OMV_MDF)
        for (int i = 0; i < g_pdm_buffer_size / 2; i++) {
            pcmbuf[i] = PDM_BUFFER[i] >> 16;
        }
        #elif defined(OMV_DFSDM)
        for (int i = 0; i < g_pdm_buffer_size / 2; i++) {
            pcmbuf[i] = __SSAT_ASR((PDM_BUFFER[i] >> 8) * dfsdm_gain, 16, DFSDM_GAIN_FRAC_BITS);
        }
        #endif
    } else if ((xfer_status & DMA_XFER_FULL)) {
        // Check for transfer complete.
        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_FULL);

        #if defined(OMV_SAI)
        // Convert PDM samples to PCM.
        for (int i = 0; i < g_channels; i++) {
            PDM_Filter(&((uint8_t *) PDM_BUFFER)[g_pdm_buffer_size / 2 + i], &pcmbuf[i], &PDM_FilterHandler[i]);
        }
        #elif defined(OMV_MDF)
        for (int i = 0; i < g_pdm_buffer_size / 2; i++) {
            pcmbuf[i] = PDM_BUFFER[g_pdm_buffer_size / 2 + i] >> 16;
        }
        #elif defined(OMV_DFSDM)
        for (int i = 0; i < g_pdm_buffer_size / 2; i++) {
            pcmbuf[i] = __SSAT_ASR((PDM_BUFFER[g_pdm_buffer_size / 2 + i] >> 8) * dfsdm_gain, 16, DFSDM_GAIN_FRAC_BITS);
        }
        #endif
    }

    // Call user callback
    mp_call_function_1(MP_STATE_PORT(audio_callback), MP_STATE_PORT(audio_pcm_array));
}

static mp_obj_t py_audio_start_streaming(mp_obj_t callback_obj) {
    if (!mp_obj_is_callable(callback_obj)) {
        RAISE_OS_EXCEPTION("Invalid callback object!");
    }

    MP_STATE_PORT(audio_callback) = callback_obj;

    // Clear DMA buffer status
    xfer_status &= DMA_XFER_NONE;

    #if defined(OMV_SAI)
    // Start DMA transfer
    if (HAL_SAI_Receive_DMA(&hsai, (uint8_t *) PDM_BUFFER, g_pdm_buffer_size / g_channels) != HAL_OK) {
        MP_STATE_PORT(audio_callback) = mp_const_none;
        RAISE_OS_EXCEPTION("SAI DMA transfer failed!");
    }
    #elif defined(OMV_MDF)
    MDF_DmaConfigTypeDef dma_config = {
        .Address = (uint32_t) PDM_BUFFER,
        .DataLength = sizeof(PDM_BUFFER[0]) * g_pdm_buffer_size,   // in bytes
        .MsbOnly = DISABLE,
    };
    if (HAL_MDF_AcqStart_DMA(&hmdf, &hmdf_filter[0], &dma_config) != HAL_OK) {
        RAISE_OS_EXCEPTION("MDF DMA transfer failed!");
    }
    #elif defined(OMV_DFSDM)
    // Start DMA transfer
    if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm_filter[0], PDM_BUFFER, g_pdm_buffer_size) != HAL_OK) {
        RAISE_OS_EXCEPTION("DFSDM DMA transfer failed!");
    }
    #endif

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_audio_start_streaming_obj, py_audio_start_streaming);

static mp_obj_t py_audio_stop_streaming() {
    #if defined(OMV_SAI)
    // Stop SAI DMA.
    if (hdma_sai_rx.Instance != NULL) {
        HAL_SAI_DMAStop(&hsai);
    }
    #elif defined(OMV_DFSDM)
    if (hdma_filter[0].Instance != NULL) {
        HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm_filter[0]);
    }
    #endif
    MP_STATE_PORT(audio_callback) = mp_const_none;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_stop_streaming_obj, py_audio_stop_streaming);

#if defined(OMV_SAI)
static mp_obj_t py_audio_read_pdm(mp_obj_t buf_in) {
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
    if (HAL_SAI_Receive_DMA(&hsai, (uint8_t *) PDM_BUFFER, g_pdm_buffer_size / g_channels) != HAL_OK) {
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
        uint32_t samples = OMV_MIN(n_samples, g_pdm_buffer_size);
        for (int i = 0; i < samples; i++, n_samples--, xfer_samples++) {
            ((uint8_t *) pdmbuf.buf)[xfer_samples] = ((uint8_t *) PDM_BUFFER)[i];
        }

        if (xfer_status & DMA_XFER_FULL) {
            printf("Dropping samples!\n");
        }
    }

    // Stop SAI DMA.
    HAL_SAI_DMAStop(&hsai);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_audio_read_pdm_obj, py_audio_read_pdm);
#endif

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_audio)               },
    { MP_ROM_QSTR(MP_QSTR_init),            MP_ROM_PTR(&py_audio_init_obj)           },
    { MP_ROM_QSTR(MP_QSTR_start_streaming), MP_ROM_PTR(&py_audio_start_streaming_obj)},
    { MP_ROM_QSTR(MP_QSTR_stop_streaming),  MP_ROM_PTR(&py_audio_stop_streaming_obj) },
    #if defined(OMV_SAI)
    { MP_ROM_QSTR(MP_QSTR_read_pdm),        MP_ROM_PTR(&py_audio_read_pdm_obj)       },
    #endif
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t audio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_ROOT_POINTER(mp_obj_t audio_callback);
MP_REGISTER_ROOT_POINTER(mp_obj_t audio_pcm_array);
MP_REGISTER_ROOT_POINTER(int16_t * audio_pcm_buffer);
MP_REGISTER_MODULE(MP_QSTR_audio, audio_module);
#endif //MICROPY_PY_AUDIO
