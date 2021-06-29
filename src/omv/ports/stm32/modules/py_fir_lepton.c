/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * FIR Python module.
 */
#include "py/nlr.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "py/mphal.h"

#include "extint.h"
#include "spi.h"
#include "softtimer.h"

#include "py_helper.h"
#include "omv_boardconfig.h"
#include STM32_HAL_H

#include "crc16.h"
#include "LEPTON_SDK.h"
#include "LEPTON_AGC.h"
#include "LEPTON_SYS.h"
#include "LEPTON_VID.h"
#include "LEPTON_OEM.h"
#include "LEPTON_RAD.h"
#include "LEPTON_I2C_Reg.h"

#if (OMV_ENABLE_FIR_LEPTON == 1)

#define FRAMEBUFFER_COUNT 3
static volatile int framebuffer_tail = 0;
static int framebuffer_head = 0;
static uint16_t *framebuffers[FRAMEBUFFER_COUNT] = {};

static int fir_lepton_rad_en = false;
static bool fir_lepton_3 = false;

#if defined(OMV_FIR_LEPTON_MCLK_TIM)
static TIM_HandleTypeDef fir_lepton_mclk_tim_handle = {};
#endif

static LEP_CAMERA_PORT_DESC_T fir_lepton_handle = {};
static DMA_HandleTypeDef fir_lepton_spi_rx_dma = {};

#define VOSPI_HEADER_WORDS      (2) // 16-bits
#define VOSPI_PID_SIZE_PIXELS   (80) // w, 16-bits per pixel
#define VOSPI_PIDS_PER_SEG      (60) // h
#define VOSPI_SEGS_PER_FRAME    (4)
#define VOSPI_PACKET_SIZE       (VOSPI_HEADER_WORDS + VOSPI_PID_SIZE_PIXELS) // 16-bits
#define VOSPI_SEG_SIZE_PIXELS   (VOSPI_PIDS_PER_SEG * VOSPI_PID_SIZE_PIXELS) // 16-bits

#define VOSPI_BUFFER_SIZE       (VOSPI_PACKET_SIZE * 2) // 16-bits
#define VOSPI_CLOCK_SPEED       20000000 // hz
#define VOSPI_SYNC_MS           200 // ms

static soft_timer_entry_t flir_lepton_spi_rx_timer = {};
static int fir_lepton_spi_rx_cb_tail = 0;
static int fir_lepton_spi_rx_cb_expected_pid = 0;
static int fir_lepton_spi_rx_cb_expected_seg = 0;
extern int _fir_lepton_buf[];

STATIC mp_obj_t fir_lepton_spi_resync_callback(mp_obj_t unused)
{
    // For triple buffering we are never drawing where tail or head
    // (which may instantly update to be equal to tail) is.
    fir_lepton_spi_rx_cb_tail = (framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
    if (fir_lepton_spi_rx_cb_tail == framebuffer_head) {
        fir_lepton_spi_rx_cb_tail = (fir_lepton_spi_rx_cb_tail + 1) % FRAMEBUFFER_COUNT;
    }

    OMV_FIR_LEPTON_CS_LOW();
    HAL_SPI_Receive_DMA(OMV_FIR_LEPTON_CONTROLLER->spi,
            (uint8_t *) &_fir_lepton_buf,
            VOSPI_BUFFER_SIZE);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(fir_lepton_spi_resync_callback_obj, fir_lepton_spi_resync_callback);

static void fir_lepton_spi_resync()
{
    flir_lepton_spi_rx_timer.mode = SOFT_TIMER_MODE_ONE_SHOT;
    flir_lepton_spi_rx_timer.expiry_ms = VOSPI_SYNC_MS + mp_hal_ticks_ms();
    flir_lepton_spi_rx_timer.delta_ms = VOSPI_SYNC_MS;
    flir_lepton_spi_rx_timer.callback = (mp_obj_t) &fir_lepton_spi_resync_callback_obj;
    soft_timer_insert(&flir_lepton_spi_rx_timer);
}

#if defined(OMV_FIR_LEPTON_CHECK_CRC)
static bool fir_lepton_spi_check_crc(const uint16_t *base)
{
    int id = base[0];
    int packet_crc = base[1];
    int crc = ByteCRC16((id >> 8) & 0x0F, 0);
    crc = ByteCRC16(id, crc);
    crc = ByteCRC16(0, crc);
    crc = ByteCRC16(0, crc);

    for (int i = VOSPI_HEADER_WORDS; i < VOSPI_PACKET_SIZE; i++) {
        int value = base[i];
        crc = ByteCRC16(value >> 8, crc);
        crc = ByteCRC16(value, crc);
    }

    return packet_crc == crc;
}
#endif

static mp_obj_t fir_lepton_frame_cb = mp_const_none;

void fir_lepton_spi_callback(const uint16_t *base)
{
    int id = base[0];

    // Ignore don't care packets.
    if ((id & 0x0F00) == 0x0F00) {
        return;
    }

    int pid = id & 0x0FFF;
    int seg = ((id >> 12) & 0x7) - 1;

    // Discard packets with a pid != 0 when waiting for the first packet.
    if ((fir_lepton_spi_rx_cb_expected_pid == 0) && (pid != 0)) {
        return;
    }

    // Discard segments with a seg != 0 when waiting for the first segment.
    if (fir_lepton_3 && (pid == 20) && (fir_lepton_spi_rx_cb_expected_seg == 0) && (seg != 0)) {
        fir_lepton_spi_rx_cb_expected_pid = 0;
        return;
    }

    // Are we in sync with the flir lepton?
    if ((pid != fir_lepton_spi_rx_cb_expected_pid)
    #if defined(OMV_FIR_LEPTON_CHECK_CRC)
    || (!fir_lepton_spi_check_crc(base))
    #endif
    || (fir_lepton_3 && (pid == 20) && (seg != fir_lepton_spi_rx_cb_expected_seg))) {
        fir_lepton_spi_rx_cb_expected_pid = 0;
        fir_lepton_spi_rx_cb_expected_seg = 0;
        HAL_SPI_Abort_IT(OMV_FIR_LEPTON_CONTROLLER->spi);
        OMV_FIR_LEPTON_CS_HIGH();
        fir_lepton_spi_resync();
        return;
    }

    memcpy(framebuffers[fir_lepton_spi_rx_cb_tail]
        + (fir_lepton_spi_rx_cb_expected_pid * VOSPI_PID_SIZE_PIXELS)
        + (fir_lepton_spi_rx_cb_expected_seg * VOSPI_SEG_SIZE_PIXELS),
        base + VOSPI_HEADER_WORDS, VOSPI_PID_SIZE_PIXELS * sizeof(uint16_t));

    fir_lepton_spi_rx_cb_expected_pid += 1;
    if (fir_lepton_spi_rx_cb_expected_pid == VOSPI_PIDS_PER_SEG) {
        fir_lepton_spi_rx_cb_expected_pid = 0;

        bool frame_ready = false;

        // For the FLIR Lepton 3 we have to receive all the pids in all the segments.
        if (fir_lepton_3) {
            fir_lepton_spi_rx_cb_expected_seg += 1;
            if (fir_lepton_spi_rx_cb_expected_seg == VOSPI_SEGS_PER_FRAME) {
                fir_lepton_spi_rx_cb_expected_seg = 0;
                frame_ready = true;
            }
        // For the FLIR Lepton 1/2 we just have to receive all the pids.
        } else {
            frame_ready = true;
        }

        if (frame_ready) {
            // Update tail which means a new image is ready.
            framebuffer_tail = fir_lepton_spi_rx_cb_tail;

            // For triple buffering we are never drawing where tail or head
            // (which may instantly update to be equal to tail) is.
            fir_lepton_spi_rx_cb_tail = (fir_lepton_spi_rx_cb_tail + 1) % FRAMEBUFFER_COUNT;
            if (fir_lepton_spi_rx_cb_tail == framebuffer_head) {
                fir_lepton_spi_rx_cb_tail = (fir_lepton_spi_rx_cb_tail + 1) % FRAMEBUFFER_COUNT;
            }

            // User should use micropython.schedule() in their callback to process the new frame.
            if (fir_lepton_frame_cb != mp_const_none) {
                mp_call_function_0(fir_lepton_frame_cb);
            }
        }
    }
}

static void fir_lepton_spi_callback_half(SPI_HandleTypeDef *hspi)
{
    fir_lepton_spi_callback((uint16_t *) &_fir_lepton_buf);
}

static void fir_lepton_spi_callback_full(SPI_HandleTypeDef *hspi)
{
    fir_lepton_spi_callback(((uint16_t *) &_fir_lepton_buf) + VOSPI_PACKET_SIZE);
}

#if defined(OMV_FIR_LEPTON_VSYNC_PRESENT)
static mp_obj_t fir_lepton_vsync_cb = NULL;

STATIC mp_obj_t fir_lepton_extint_callback(mp_obj_t line)
{
    if (fir_lepton_vsync_cb) {
        mp_call_function_0(fir_lepton_vsync_cb);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(fir_lepton_extint_callback_obj, fir_lepton_extint_callback);
#endif

void fir_lepton_deinit()
{
    HAL_SPI_Abort(OMV_FIR_LEPTON_CONTROLLER->spi);
    fir_lepton_spi_rx_cb_expected_pid = 0;
    fir_lepton_spi_rx_cb_expected_seg = 0;
    fb_alloc_free_till_mark_past_mark_permanent();

    ///////////////////////////////////////////////////////////////////////

    #if defined(OMV_FIR_LEPTON_MCLK)
    HAL_TIM_PWM_Stop(&fir_lepton_mclk_tim_handle, OMV_FIR_LEPTON_MCLK_TIM_CHANNEL);
    HAL_TIM_PWM_DeInit(&fir_lepton_mclk_tim_handle);
    OMV_FIR_LEPTON_MCLK_TIM_FORCE_RESET();
    OMV_FIR_LEPTON_MCLK_TIM_RELEASE_RESET();
    OMV_FIR_LEPTON_MCLK_TIM_CLK_DISABLE();
    HAL_GPIO_DeInit(OMV_FIR_LEPTON_MCLK_PORT, OMV_FIR_LEPTON_MCLK_PIN);
    #endif

    spi_deinit(OMV_FIR_LEPTON_CONTROLLER);

    // Do not put in HAL_SPI_MspDeinit as other modules may share the SPI bus.

    HAL_GPIO_DeInit(OMV_FIR_LEPTON_MOSI_PORT, OMV_FIR_LEPTON_MOSI_PIN);
    HAL_GPIO_DeInit(OMV_FIR_LEPTON_MISO_PORT, OMV_FIR_LEPTON_MISO_PIN);
    HAL_GPIO_DeInit(OMV_FIR_LEPTON_SCLK_PORT, OMV_FIR_LEPTON_SCLK_PIN);

    HAL_GPIO_DeInit(OMV_FIR_LEPTON_CS_PORT, OMV_FIR_LEPTON_CS_PIN);
#if defined(OMV_FIR_LEPTON_RST_PIN_PRESENT)
    HAL_GPIO_DeInit(OMV_FIR_LEPTON_RST_PORT, OMV_FIR_LEPTON_RST_PIN);
#endif
#if defined(OMV_FIR_LEPTON_PWDN_PIN_PRESENT)
    HAL_GPIO_DeInit(OMV_FIR_LEPTON_PWDN_PORT, OMV_FIR_LEPTON_PWDN_PIN);
#endif
}

int fir_lepton_init(cambus_t *bus, int *w, int *h, int *refresh, int *resolution)
{
    OMV_FIR_LEPTON_CONTROLLER->spi->Init.Mode = SPI_MODE_MASTER;
    OMV_FIR_LEPTON_CONTROLLER->spi->Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
    OMV_FIR_LEPTON_CONTROLLER->spi->Init.NSS = SPI_NSS_SOFT;
    OMV_FIR_LEPTON_CONTROLLER->spi->Init.TIMode = SPI_TIMODE_DISABLE;
    OMV_FIR_LEPTON_CONTROLLER->spi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    #if defined(MCU_SERIES_H7)
    OMV_FIR_LEPTON_CONTROLLER->spi->Init.FifoThreshold = SPI_FIFO_THRESHOLD_02DATA;
    OMV_FIR_LEPTON_CONTROLLER->spi->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    #endif
    spi_set_params(OMV_FIR_LEPTON_CONTROLLER, 0xffffffff, VOSPI_CLOCK_SPEED, 1, 1, 16, 0);
    spi_init(OMV_FIR_LEPTON_CONTROLLER, true);
    HAL_SPI_RegisterCallback(OMV_FIR_LEPTON_CONTROLLER->spi, HAL_SPI_RX_HALF_COMPLETE_CB_ID,
                             fir_lepton_spi_callback_half);
    HAL_SPI_RegisterCallback(OMV_FIR_LEPTON_CONTROLLER->spi, HAL_SPI_RX_COMPLETE_CB_ID,
                             fir_lepton_spi_callback_full);

    // Do not put in HAL_SPI_MspInit as other modules share the SPI2/3 bus.

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pull      = GPIO_PULLUP;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_MEDIUM;

    GPIO_InitStructure.Alternate = OMV_FIR_LEPTON_MOSI_ALT;
    GPIO_InitStructure.Pin       = OMV_FIR_LEPTON_MOSI_PIN;
    HAL_GPIO_Init(OMV_FIR_LEPTON_MOSI_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Alternate = OMV_FIR_LEPTON_MISO_ALT;
    GPIO_InitStructure.Pin       = OMV_FIR_LEPTON_MISO_PIN;
    HAL_GPIO_Init(OMV_FIR_LEPTON_MISO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Alternate = OMV_FIR_LEPTON_SCLK_ALT;
    GPIO_InitStructure.Pin       = OMV_FIR_LEPTON_SCLK_PIN;
    HAL_GPIO_Init(OMV_FIR_LEPTON_SCLK_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStructure.Pin       = OMV_FIR_LEPTON_CS_PIN;
    HAL_GPIO_Init(OMV_FIR_LEPTON_CS_PORT, &GPIO_InitStructure);
    OMV_FIR_LEPTON_CS_HIGH();

    #if defined(OMV_FIR_LEPTON_RST_PIN_PRESENT)
    GPIO_InitStructure.Pin       = OMV_FIR_LEPTON_RST_PIN;
    HAL_GPIO_Init(OMV_FIR_LEPTON_RST_PORT, &GPIO_InitStructure);
    OMV_FIR_LEPTON_RST_HIGH();
    #endif

    #if defined(OMV_FIR_LEPTON_PWDN_PIN_PRESENT)
    GPIO_InitStructure.Pin       = OMV_FIR_LEPTON_PWDN_PIN;
    HAL_GPIO_Init(OMV_FIR_LEPTON_PWDN_PORT, &GPIO_InitStructure);
    OMV_FIR_LEPTON_PWDN_HIGH();
    #endif

    #if defined(OMV_FIR_LEPTON_MCLK_TIM)
    int tclk = OMV_FIR_LEPTON_MCLK_TIM_PCLK_FREQ() * 2;
    int period = (tclk / OMV_FIR_LEPTON_MCLK_FREQ) - 1;

    // GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Alternate = OMV_FIR_LEPTON_MCLK_ALT;
    GPIO_InitStructure.Pin = OMV_FIR_LEPTON_MCLK_PIN;
    HAL_GPIO_Init(OMV_FIR_LEPTON_MCLK_PORT, &GPIO_InitStructure);

    fir_lepton_mclk_tim_handle.Instance = OMV_FIR_LEPTON_MCLK_TIM;
    fir_lepton_mclk_tim_handle.Init.Prescaler = 0;
    fir_lepton_mclk_tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    fir_lepton_mclk_tim_handle.Init.Period = period;
    fir_lepton_mclk_tim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    fir_lepton_mclk_tim_handle.Init.RepetitionCounter = 0;
    fir_lepton_mclk_tim_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    TIM_OC_InitTypeDef fir_lepton_mclk_tim_oc_handle;
    fir_lepton_mclk_tim_oc_handle.Pulse = period / 2;
    fir_lepton_mclk_tim_oc_handle.OCMode = TIM_OCMODE_PWM1;
    fir_lepton_mclk_tim_oc_handle.OCPolarity = TIM_OCPOLARITY_HIGH;
    fir_lepton_mclk_tim_oc_handle.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    fir_lepton_mclk_tim_oc_handle.OCFastMode = TIM_OCFAST_DISABLE;
    fir_lepton_mclk_tim_oc_handle.OCIdleState = TIM_OCIDLESTATE_RESET;
    fir_lepton_mclk_tim_oc_handle.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    OMV_FIR_LEPTON_MCLK_TIM_CLK_ENABLE();
    HAL_TIM_PWM_Init(&fir_lepton_mclk_tim_handle);
    HAL_TIM_PWM_ConfigChannel(&fir_lepton_mclk_tim_handle,
                              &fir_lepton_mclk_tim_oc_handle,
                              OMV_FIR_LEPTON_MCLK_TIM_CHANNEL);
    HAL_TIM_PWM_Start(&fir_lepton_mclk_tim_handle, OMV_FIR_LEPTON_MCLK_TIM_CHANNEL);
    #endif

    #if defined(OMV_FIR_LEPTON_PWDN_PIN_PRESENT)
    OMV_FIR_LEPTON_PWDN_LOW();
    mp_hal_delay_ms(10);

    OMV_FIR_LEPTON_PWDN_HIGH();
    mp_hal_delay_ms(10);
    #endif

    #if defined(OMV_FIR_LEPTON_RST_PIN_PRESENT)
    OMV_FIR_LEPTON_RST_LOW();
    mp_hal_delay_ms(10);

    OMV_FIR_LEPTON_RST_HIGH();
    mp_hal_delay_ms(1000);
    #endif

    LEP_RAD_ENABLE_E rad;
    LEP_AGC_ROI_T roi;

    for (uint32_t start = mp_hal_ticks_ms();; mp_hal_delay_ms(1)) {
        if (LEP_OpenPort(bus, LEP_CCI_TWI, 0, &fir_lepton_handle) == LEP_OK) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= 1000) {
            return -1;
        }
    }

    #if (!defined(OMV_FIR_LEPTON_PWDN_PIN_PRESENT)) && (!defined(OMV_FIR_LEPTON_RST_PIN_PRESENT))
    if (LEP_RunOemReboot(&fir_lepton_handle) != LEP_OK) {
        return -2;
    }

    mp_hal_delay_ms(1000);
    #endif

    for (uint32_t start = mp_hal_ticks_ms();; mp_hal_delay_ms(1)) {
        LEP_SDK_BOOT_STATUS_E status;

        if (LEP_GetCameraBootStatus(&fir_lepton_handle, &status) != LEP_OK) {
            return -3;
        }

        if (status == LEP_BOOT_STATUS_BOOTED) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= 1000) {
            return -4;
        }
    }

    for (uint32_t start = mp_hal_ticks_ms();; mp_hal_delay_ms(1)) {
        LEP_UINT16 status;

        if (LEP_DirectReadRegister(&fir_lepton_handle, LEP_I2C_STATUS_REG, &status) != LEP_OK) {
            return -5;
        }

        if (!(status & LEP_I2C_STATUS_BUSY_BIT_MASK)) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= 1000) {
            return -6;
        }
    }

    if (LEP_GetAgcROI(&fir_lepton_handle, &roi) != LEP_OK) {
        return -7;
    }

    if (LEP_GetRadEnableState(&fir_lepton_handle, &rad) != LEP_OK) {
        return -8;
    }

    int flir_w = roi.endCol + 1;
    int flir_h = roi.endRow + 1;
    fir_lepton_3 = flir_h > VOSPI_PIDS_PER_SEG;
    fir_lepton_rad_en = rad == LEP_RAD_ENABLE;
    *w = flir_w;
    *h = flir_h;
    *refresh = fir_lepton_3 ? 27 : 9;
    *resolution = fir_lepton_rad_en ? 16 : 14;

    #if defined(OMV_FIR_LEPTON_VSYNC_PRESENT)
    if (LEP_SetOemGpioMode(&fir_lepton_handle, LEP_OEM_GPIO_MODE_VSYNC) != LEP_OK) {
        return -9;
    }

    extint_register((mp_obj_t) OMV_FIR_LEPTON_VSYNC_PIN, GPIO_MODE_IT_FALLING, GPIO_PULLUP,
                    (mp_obj_t) &fir_lepton_extint_callback_obj, true);
    #endif

    ///////////////////////////////////////////////////////////////////////

    fb_alloc_mark();

    framebuffer_tail = 0;
    framebuffer_head = 0;

    for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
        framebuffers[i] = (uint16_t *) fb_alloc0(flir_w * flir_h * sizeof(uint16_t),
                                                 FB_ALLOC_NO_HINT);
    }

    dma_init(&fir_lepton_spi_rx_dma,
             OMV_FIR_LEPTON_CONTROLLER->rx_dma_descr,
             DMA_PERIPH_TO_MEMORY,
             OMV_FIR_LEPTON_CONTROLLER->spi);

    OMV_FIR_LEPTON_CONTROLLER->spi->hdmatx = NULL;
    OMV_FIR_LEPTON_CONTROLLER->spi->hdmarx = &fir_lepton_spi_rx_dma;

    #if defined(MCU_SERIES_H7)
    fir_lepton_spi_rx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    #else
    fir_lepton_spi_rx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    #endif

    fir_lepton_spi_rx_dma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;

    fir_lepton_spi_rx_dma.Init.Mode = DMA_CIRCULAR;
    fir_lepton_spi_rx_dma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    fir_lepton_spi_rx_dma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    fir_lepton_spi_rx_dma.Init.MemBurst = DMA_MBURST_SINGLE;
    fir_lepton_spi_rx_dma.Init.PeriphBurst = DMA_PBURST_SINGLE;

    ((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR =
        (((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR & ~DMA_SxCR_PSIZE_Msk) |
        #if defined(MCU_SERIES_H7)
        DMA_PDATAALIGN_WORD;
        #else
        DMA_PDATAALIGN_HALFWORD;
        #endif

    ((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR =
        (((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR & ~DMA_SxCR_MSIZE_Msk) |
        DMA_MDATAALIGN_WORD;

    ((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR =
        (((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR & ~DMA_SxCR_CIRC_Msk) |
        DMA_CIRCULAR;

    ((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->FCR =
        (((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->FCR & ~DMA_SxFCR_DMDIS_Msk) |
        DMA_FIFOMODE_ENABLE;

    ((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->FCR =
        (((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->FCR & ~DMA_SxFCR_FTH_Msk) |
        DMA_FIFO_THRESHOLD_1QUARTERFULL;

    ((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR =
        (((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR & ~DMA_SxCR_MBURST_Msk) |
        DMA_MBURST_SINGLE;

    ((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR =
        (((DMA_Stream_TypeDef *) fir_lepton_spi_rx_dma.Instance)->CR & ~DMA_SxCR_PBURST_Msk) |
        DMA_PBURST_SINGLE;

    fb_alloc_mark_permanent();
    fir_lepton_spi_resync();
    return 0;
}

#if defined(OMV_FIR_LEPTON_VSYNC_PRESENT)
void fir_lepton_register_vsync_cb(mp_obj_t cb)
{
    extint_disable(OMV_FIR_LEPTON_VSYNC_PIN->pin);

    fir_lepton_vsync_cb = cb;

    if (cb != mp_const_none) {
        extint_enable(OMV_FIR_LEPTON_VSYNC_PIN->pin);
    }
}
#endif

mp_obj_t fir_lepton_get_radiometry()
{
    return mp_obj_new_bool(fir_lepton_rad_en);
}

void fir_lepton_register_frame_cb(mp_obj_t cb)
{
    fir_lepton_frame_cb = cb;
}

mp_obj_t fir_lepton_get_frame_available()
{
    return mp_obj_new_bool(framebuffer_tail != framebuffer_head);
}

static const uint16_t *fir_lepton_get_frame(int timeout)
{
    int sampled_framebuffer_tail = framebuffer_tail;

    if (timeout >= 0) {
        for (uint32_t start = mp_hal_ticks_ms();;) {
            sampled_framebuffer_tail = framebuffer_tail;

            if (framebuffer_head != sampled_framebuffer_tail) {
                break;
            }

            if ((mp_hal_ticks_ms() - start) >= timeout) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Timeout!"));
            }

            __WFI();
        }
    }

    framebuffer_head = sampled_framebuffer_tail;
    return framebuffers[sampled_framebuffer_tail];
}

static int fir_lepton_get_temperature()
{
    LEP_SYS_FPA_TEMPERATURE_KELVIN_T kelvin;

    if (LEP_GetSysFpaTemperatureKelvin(&fir_lepton_handle, &kelvin) != LEP_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("FPA Error!"));
    }

    return kelvin;
}

mp_obj_t fir_lepton_read_ta()
{
    return mp_obj_new_float((fir_lepton_get_temperature() * 0.01f) - 273.15f);
}

mp_obj_t fir_lepton_read_ir(int w, int h, bool mirror, bool flip, bool transpose, int timeout)
{
    int kelvin = fir_lepton_get_temperature();
    mp_obj_list_t *list = (mp_obj_list_t *) mp_obj_new_list(w * h, NULL);
    const uint16_t *data = fir_lepton_get_frame(timeout);
    float min = +FLT_MAX;
    float max = -FLT_MAX;
    int w_1 = w - 1;
    int h_1 = h - 1;

    for (int y = 0; y < h; y++) {
        int y_dst = flip ? (h_1 - y) : y;
        const uint16_t *raw_row = data + (y * w);
        mp_obj_t *list_row = list->items + (y_dst * w);
        mp_obj_t *t_list_row = list->items + y_dst;

        for (int x = 0; x < w; x++) {
            int x_dst = mirror ? (w_1 - x) : x;
            int raw = raw_row[x];

            if (!fir_lepton_rad_en) {
                raw = (raw - 8192) + kelvin;
            }

            float celcius = (raw * 0.01f) - 273.15f;

            if (celcius < min) {
                min = celcius;
            }

            if (celcius > max) {
                max = celcius;
            }

            mp_obj_t f = mp_obj_new_float(celcius);

            if (!transpose) {
                list_row[x_dst] = f;
            } else {
                t_list_row[x_dst * h] = f;
            }
        }
    }

    mp_obj_t tuple[4];
    tuple[0] = mp_obj_new_float((kelvin * 0.01f) - 273.15f);
    tuple[1] = MP_OBJ_FROM_PTR(list);
    tuple[2] = mp_obj_new_float(min);
    tuple[3] = mp_obj_new_float(max);
    return mp_obj_new_tuple(4, tuple);
}

void fir_lepton_fill_image(image_t *img, int w, int h, bool auto_range, float min, float max,
                           bool mirror, bool flip, bool transpose, int timeout)
{
    int kelvin = fir_lepton_get_temperature();
    const uint16_t *data = fir_lepton_get_frame(timeout);
    int new_min;
    int new_max;

    if (auto_range) {
        new_min = INT_MAX;
        new_max = INT_MIN;

        for (int i = 0, ii = w * h; i < ii; i++) {
            int temp = data[i];

            if (!fir_lepton_rad_en) {
                temp = (temp - 8192) + kelvin;
            }

            if (temp < new_min) {
                new_min = temp;
            }

            if (temp > new_max) {
                new_max = temp;
            }
        }
    } else {
        float tmp = min;
        min = (min < max) ? min : max;
        max = (max > tmp) ? max : tmp;
        new_min = fast_roundf((min + 273.15f) * 100.f); // to kelvin
        new_max = fast_roundf((max + 273.15f) * 100.f); // to kelvin
    }

    float diff = 255.f / (new_max - new_min);
    int w_1 = w - 1;
    int h_1 = h - 1;

    for (int y = 0; y < h; y++) {
        int y_dst = flip ? (h_1 - y) : y;
        const uint16_t *raw_row = data + (y * w);
        uint8_t *row_pointer = ((uint8_t *) img->data) + (y_dst * w);
        uint8_t *t_row_pointer = ((uint8_t *) img->data) + y_dst;

        for (int x = 0; x < w; x++) {
            int x_dst = mirror ? (w_1 - x) : x;
            int raw = raw_row[x];

            if (!fir_lepton_rad_en) {
                raw = (raw - 8192) + kelvin;
            }

            if (raw < new_min) {
                raw = new_min;
            }

            if (raw > new_max) {
                raw = new_max;
            }

            int pixel = fast_roundf((raw - new_min) * diff);
            pixel = __USAT(pixel, 8);

            if (!transpose) {
                row_pointer[x_dst] = pixel;
            } else {
                t_row_pointer[x_dst * h] = pixel;
            }
        }
    }
}

void fir_lepton_trigger_ffc(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    if (LEP_RunSysFFCNormalization(&fir_lepton_handle) != LEP_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("FFC Error!"));
    }

    int timeout = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_timeout), -1);

    if (timeout >= 0) {
        for (uint32_t start = mp_hal_ticks_ms();;) {
            LEP_SYS_STATUS_E status;

            if (LEP_GetSysFFCStatus(&fir_lepton_handle, &status) != LEP_OK) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("SYS Error!"));
            }

            if (status == LEP_SYS_STATUS_READY) {
                break;
            }

            if ((mp_hal_ticks_ms() - start) >= timeout) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Timeout!"));
            }

            mp_hal_delay_ms(1);
        }
    }
}

#endif // OMV_FIR_LEPTON_PRESENT
