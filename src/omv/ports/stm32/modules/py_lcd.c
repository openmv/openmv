/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * LCD Python module.
 */
#include "omv_boardconfig.h"

#if MICROPY_PY_LCD
#include STM32_HAL_H
#include "py/obj.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "pendsv.h"
#include "spi.h"

#include "py_lcd_cec.h"
#include "py_lcd_touch.h"
#include "py_helper.h"
#include "py_image.h"
#include "extmod/machine_i2c.h"
#include "omv_gpio.h"

#define FRAMEBUFFER_COUNT    3
static int framebuffer_tail = 0;
static volatile int framebuffer_head = 0;
static uint16_t *framebuffers[FRAMEBUFFER_COUNT] = {};

static int lcd_width = 0;
static int lcd_height = 0;

static enum {
    LCD_NONE,
    LCD_SHIELD,
    LCD_DISPLAY,
    LCD_DISPLAY_WITH_HDMI,
    LCD_DISPLAY_ONLY_HDMI
}
lcd_type = LCD_NONE;

static bool lcd_triple_buffer = false;
static bool lcd_bgr = false;
static bool lcd_byte_reverse = false;

static enum {
    LCD_DISPLAY_QVGA,
    LCD_DISPLAY_TQVGA,
    LCD_DISPLAY_FHVGA,
    LCD_DISPLAY_FHVGA2,
    LCD_DISPLAY_VGA,
    LCD_DISPLAY_THVGA,
    LCD_DISPLAY_FWVGA,
    LCD_DISPLAY_FWVGA2,
    LCD_DISPLAY_TFWVGA,
    LCD_DISPLAY_TFWVGA2,
    LCD_DISPLAY_SVGA,
    LCD_DISPLAY_WSVGA,
    LCD_DISPLAY_XGA,
    LCD_DISPLAY_SXGA,
    LCD_DISPLAY_SXGA2,
    LCD_DISPLAY_UXGA,
    LCD_DISPLAY_HD,
    LCD_DISPLAY_FHD,
    LCD_DISPLAY_MAX
}
lcd_resolution = LCD_DISPLAY_QVGA;

static int lcd_refresh = 0;
static int lcd_intensity = 0;

#ifdef OMV_SPI_LCD_CONTROLLER
static DMA_HandleTypeDef spi_tx_dma = {};

static volatile enum {
    SPI_TX_CB_IDLE,
    SPI_TX_CB_MEMORY_WRITE_CMD,
    SPI_TX_CB_MEMORY_WRITE,
    SPI_TX_CB_DISPLAY_ON,
    SPI_TX_CB_DISPLAY_OFF
}
spi_tx_cb_state = SPI_TX_CB_IDLE;

static void spi_config_deinit() {
    if (lcd_triple_buffer) {
        HAL_SPI_Abort(OMV_SPI_LCD_CONTROLLER->spi);
        spi_tx_cb_state = SPI_TX_CB_IDLE;
        fb_alloc_free_till_mark_past_mark_permanent();
    }

    spi_deinit(OMV_SPI_LCD_CONTROLLER);

    // Do not put in HAL_SPI_MspDeinit as other modules share the SPI2 bus.
    omv_gpio_deinit(OMV_SPI_LCD_MOSI_PIN);
    omv_gpio_deinit(OMV_SPI_LCD_SCLK_PIN);
    omv_gpio_deinit(OMV_SPI_LCD_RST_PIN);
    omv_gpio_deinit(OMV_SPI_LCD_RS_PIN);
    omv_gpio_deinit(OMV_SPI_LCD_SSEL_PIN);
}

static void spi_lcd_callback(SPI_HandleTypeDef *hspi);

static void spi_config_init(int w, int h, int refresh_rate, bool triple_buffer, bool bgr) {
    SPI_HandleTypeDef *hspi = OMV_SPI_LCD_CONTROLLER->spi;

    hspi->Init.Mode = SPI_MODE_MASTER;
    hspi->Init.Direction = SPI_DIRECTION_1LINE;
    hspi->Init.NSS = SPI_NSS_SOFT;
    hspi->Init.TIMode = SPI_TIMODE_DISABLE;
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_set_params(OMV_SPI_LCD_CONTROLLER, 0xffffffff, w * h * refresh_rate * 16, 0, 0, 8, 0);
    spi_init(OMV_SPI_LCD_CONTROLLER, true);
    HAL_SPI_RegisterCallback(hspi, HAL_SPI_TX_COMPLETE_CB_ID, spi_lcd_callback);

    // Do not put in HAL_SPI_MspInit as other modules share the SPI2 bus.
    omv_gpio_config(OMV_SPI_LCD_MOSI_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_SPI_LCD_SCLK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MED, -1);
    omv_gpio_config(OMV_SPI_LCD_RST_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_SPI_LCD_RST_PIN, 1);
    omv_gpio_config(OMV_SPI_LCD_RS_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 1);
    omv_gpio_config(OMV_SPI_LCD_SSEL_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);

    omv_gpio_write(OMV_SPI_LCD_RST_PIN, 0);
    mp_hal_delay_ms(100);
    omv_gpio_write(OMV_SPI_LCD_RST_PIN, 1);
    mp_hal_delay_ms(100);

    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
    HAL_SPI_Transmit(hspi, (uint8_t []) {0x11}, 1, HAL_MAX_DELAY); // sleep out
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 1);
    mp_hal_delay_ms(120);

    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
    HAL_SPI_Transmit(hspi, (uint8_t []) {0x36}, 1, HAL_MAX_DELAY); // memory data access control
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 1);
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
    HAL_SPI_Transmit(hspi, (uint8_t []) {bgr ? 0xC8 : 0xC0}, 1, HAL_MAX_DELAY); // argument
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);

    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
    HAL_SPI_Transmit(hspi, (uint8_t []) {0x3A}, 1, HAL_MAX_DELAY); // interface pixel format
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 1);
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
    HAL_SPI_Transmit(hspi, (uint8_t []) {0x05}, 1, HAL_MAX_DELAY); // argument
    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);

    if (triple_buffer) {
        fb_alloc_mark();

        framebuffer_tail = 0;
        framebuffer_head = 0;

        for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
            framebuffers[i] = (uint16_t *) fb_alloc0(w * h * sizeof(uint16_t), FB_ALLOC_CACHE_ALIGN);
        }

        dma_init(&spi_tx_dma, OMV_SPI_LCD_CONTROLLER->tx_dma_descr, DMA_MEMORY_TO_PERIPH, hspi);
        hspi->hdmatx = &spi_tx_dma;
        hspi->hdmarx = NULL;
        #if defined(MCU_SERIES_H7)
        spi_tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        #else
        spi_tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        #endif
        spi_tx_dma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        spi_tx_dma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
        spi_tx_dma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        spi_tx_dma.Init.MemBurst = DMA_MBURST_INC4;
        #if defined(MCU_SERIES_H7)
        spi_tx_dma.Init.PeriphBurst = DMA_PBURST_INC4;
        #else
        spi_tx_dma.Init.PeriphBurst = DMA_PBURST_SINGLE;
        #endif
        DMA_Stream_TypeDef *dma_chan = spi_tx_dma.Instance;
        #if defined(MCU_SERIES_H7)
        dma_chan->CR = (dma_chan->CR & ~DMA_SxCR_PSIZE_Msk) | DMA_PDATAALIGN_WORD;
        #else
        dma_chan->CR = (dma_chan->CR & ~DMA_SxCR_PSIZE_Msk) | DMA_PDATAALIGN_HALFWORD;
        #endif
        dma_chan->CR = (dma_chan->CR & ~DMA_SxCR_MSIZE_Msk) | DMA_MDATAALIGN_WORD;
        dma_chan->FCR = (dma_chan->FCR & ~DMA_SxFCR_DMDIS_Msk) | DMA_FIFOMODE_ENABLE;
        dma_chan->FCR = (dma_chan->FCR & ~DMA_SxFCR_FTH_Msk) | DMA_FIFO_THRESHOLD_FULL;
        dma_chan->CR = (dma_chan->CR & ~DMA_SxCR_MBURST_Msk) | DMA_MBURST_INC4;
        #if defined(MCU_SERIES_H7)
        dma_chan->CR = (dma_chan->CR & ~DMA_SxCR_PBURST_Msk) | DMA_PBURST_INC4;
        #else
        dma_chan->CR = (dma_chan->CR & ~DMA_SxCR_PBURST_Msk) | DMA_PBURST_SINGLE;
        #endif
        fb_alloc_mark_permanent();
    }
}

static bool spi_tx_cb_state_on[FRAMEBUFFER_COUNT] = {};

static const uint8_t display_off[] = {0x28};
static const uint8_t display_on[] = {0x29};
static const uint8_t memory_write[] = {0x2C};

static void spi_lcd_callback(SPI_HandleTypeDef *hspi) {
    if (lcd_type == LCD_SHIELD) {
        static uint16_t *spi_tx_cb_state_memory_write_addr = NULL;
        static size_t spi_tx_cb_state_memory_write_count = 0;
        static bool spi_tx_cb_state_memory_write_first = false;

        switch (spi_tx_cb_state) {
            case SPI_TX_CB_MEMORY_WRITE_CMD: {
                if (!spi_tx_cb_state_on[framebuffer_tail]) {
                    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
                    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
                    spi_tx_cb_state = SPI_TX_CB_DISPLAY_OFF;
                    framebuffer_head = framebuffer_tail;
                    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
                    HAL_SPI_Transmit_IT(hspi, (uint8_t *) display_off, sizeof(display_off));
                } else {
                    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
                    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
                    spi_tx_cb_state = SPI_TX_CB_MEMORY_WRITE;
                    spi_tx_cb_state_memory_write_addr = framebuffers[framebuffer_tail];
                    spi_tx_cb_state_memory_write_count = lcd_width * lcd_height;
                    spi_tx_cb_state_memory_write_first = true;
                    framebuffer_head = framebuffer_tail;
                    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
                    // When starting the interrupt chain the first HAL_SPI_Transmit_IT is not executed
                    // in interrupt context. So, disable interrupts for the first HAL_SPI_Transmit_IT so
                    // that it completes first and unlocks the SPI bus before allowing the interrupt
                    // it causes to trigger starting the interrupt chain.
                    uint32_t irq_state = disable_irq();
                    HAL_SPI_Transmit_IT(hspi, (uint8_t *) memory_write, sizeof(memory_write));
                    enable_irq(irq_state);
                }
                break;
            }
            case SPI_TX_CB_MEMORY_WRITE: {
                uint16_t *addr = spi_tx_cb_state_memory_write_addr;
                size_t count = IM_MIN(spi_tx_cb_state_memory_write_count, (65536 - 8u));
                spi_tx_cb_state =
                    (spi_tx_cb_state_memory_write_count > (65536 - 8u)) ? SPI_TX_CB_MEMORY_WRITE : SPI_TX_CB_DISPLAY_ON;
                spi_tx_cb_state_memory_write_addr += count;
                spi_tx_cb_state_memory_write_count -= count;
                if (spi_tx_cb_state_memory_write_first) {
                    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
                    omv_gpio_write(OMV_SPI_LCD_RS_PIN, 1);
                    spi_tx_cb_state_memory_write_first = false;
                    if (!lcd_byte_reverse) {
                        hspi->Init.DataSize = SPI_DATASIZE_16BIT;
                        #if defined(MCU_SERIES_H7)
                        hspi->Instance->CFG1 = (hspi->Instance->CFG1 & ~SPI_CFG1_DSIZE_Msk) | SPI_DATASIZE_16BIT;
                        hspi->Instance->CFG1 = (hspi->Instance->CFG1 & ~SPI_CFG1_FTHLV_Msk) | SPI_FIFO_THRESHOLD_08DATA;
                        #elif defined(MCU_SERIES_F7)
                        hspi->Instance->CR2 = (hspi->Instance->CR2 & ~SPI_CR2_DS_Msk) | SPI_DATASIZE_16BIT;
                        #elif defined(MCU_SERIES_F4)
                        hspi->Instance->CR1 = (hspi->Instance->CR1 & ~SPI_CR1_DFF_Msk) | SPI_DATASIZE_16BIT;
                        #endif
                    }
                    omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
                }
                HAL_SPI_Transmit_DMA(hspi, (uint8_t *) addr, count);
                break;
            }
            case SPI_TX_CB_DISPLAY_ON: {
                omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
                omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
                spi_tx_cb_state = SPI_TX_CB_MEMORY_WRITE_CMD;
                hspi->Init.DataSize = SPI_DATASIZE_8BIT;
                #if defined(MCU_SERIES_H7)
                hspi->Instance->CFG1 = (hspi->Instance->CFG1 & ~SPI_CFG1_DSIZE_Msk) | SPI_DATASIZE_8BIT;
                hspi->Instance->CFG1 = (hspi->Instance->CFG1 & ~SPI_CFG1_FTHLV_Msk) | SPI_FIFO_THRESHOLD_01DATA;
                #elif defined(MCU_SERIES_F7)
                hspi->Instance->CR2 = (hspi->Instance->CR2 & ~SPI_CR2_DS_Msk) | SPI_DATASIZE_8BIT;
                #elif defined(MCU_SERIES_F4)
                hspi->Instance->CR1 = (hspi->Instance->CR1 & ~SPI_CR1_DFF_Msk) | SPI_DATASIZE_8BIT;
                #endif
                omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
                HAL_SPI_Transmit_IT(hspi, (uint8_t *) display_on, sizeof(display_on));
                break;
            }
            case SPI_TX_CB_DISPLAY_OFF: {
                omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
                spi_tx_cb_state = SPI_TX_CB_IDLE;
                break;
            }
            default: {
                break;
            }
        }
    }
}

// If the callback chain is not running restart it. Display off may have been called so we need wait
// for that operation to complete before restarting the process.
static void spi_lcd_kick() {
    int spi_tx_cb_state_sampled = spi_tx_cb_state; // volatile

    if ((spi_tx_cb_state_sampled == SPI_TX_CB_IDLE)
        || (spi_tx_cb_state_sampled == SPI_TX_CB_DISPLAY_OFF)) {
        uint32_t tick = mp_hal_ticks_ms();

        while (spi_tx_cb_state != SPI_TX_CB_IDLE) {
            // volatile
            if ((mp_hal_ticks_ms() - tick) > 1000) {
                return; // give up (should not happen)
            }
        }

        spi_tx_cb_state = SPI_TX_CB_MEMORY_WRITE_CMD;
        spi_lcd_callback(OMV_SPI_LCD_CONTROLLER->spi);
    }
}

static void spi_lcd_draw_image_cb(int x_start, int x_end, int y_row, imlib_draw_row_data_t *data) {
    HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, data->dst_row_override, lcd_width, HAL_MAX_DELAY);
}

static void spi_lcd_display(image_t *src_img, int dst_x_start, int dst_y_start,
                            float x_scale, float y_scale, rectangle_t *roi, int rgb_channel, int alpha,
                            const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint) {
    image_t dst_img;
    dst_img.w = lcd_width;
    dst_img.h = lcd_height;
    dst_img.pixfmt = PIXFORMAT_RGB565;

    int x0, x1, y0, y1;
    bool black = !imlib_draw_image_rectangle(&dst_img, src_img, dst_x_start, dst_y_start,
                                             x_scale, y_scale, roi, alpha, alpha_palette, hint, &x0, &x1, &y0, &y1);

    if (!lcd_triple_buffer) {
        SPI_HandleTypeDef *hspi = OMV_SPI_LCD_CONTROLLER->spi;
        dst_img.data = fb_alloc0(lcd_width * sizeof(uint16_t), FB_ALLOC_NO_HINT);

        omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
        omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
        // memory write
        HAL_SPI_Transmit(hspi, (uint8_t *) memory_write, sizeof(memory_write), HAL_MAX_DELAY);
        omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
        omv_gpio_write(OMV_SPI_LCD_RS_PIN, 1);

        omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
        hspi->Init.DataSize = SPI_DATASIZE_16BIT;
        #if defined(MCU_SERIES_H7)
        hspi->Instance->CFG1 = (hspi->Instance->CFG1 & ~SPI_CFG1_DSIZE_Msk) | SPI_DATASIZE_16BIT;
        #elif defined(MCU_SERIES_F7)
        hspi->Instance->CR2 = (hspi->Instance->CR2 & ~SPI_CR2_DS_Msk) | SPI_DATASIZE_16BIT;
        #elif defined(MCU_SERIES_F4)
        hspi->Instance->CR1 = (hspi->Instance->CR1 & ~SPI_CR1_DFF_Msk) | SPI_DATASIZE_16BIT;
        #endif

        if (black) {
            // zero the whole image
            for (int i = 0; i < lcd_height; i++) {
                HAL_SPI_Transmit(hspi, dst_img.data, lcd_width, HAL_MAX_DELAY);
            }
        } else {
            // Zero the top rows
            for (int i = 0; i < y0; i++) {
                HAL_SPI_Transmit(hspi, dst_img.data, lcd_width, HAL_MAX_DELAY);
            }

            // Transmits left/right parts already zeroed...
            imlib_draw_image(&dst_img, src_img, dst_x_start, dst_y_start,
                             x_scale, y_scale, roi, rgb_channel, alpha, color_palette,
                             alpha_palette, hint | IMAGE_HINT_BLACK_BACKGROUND, spi_lcd_draw_image_cb, dst_img.data);

            // Zero the bottom rows
            if (y1 < lcd_height) {
                memset(dst_img.data, 0, lcd_width * sizeof(uint16_t));
            }
            for (int i = y1; i < lcd_height; i++) {
                HAL_SPI_Transmit(hspi, dst_img.data, lcd_width, HAL_MAX_DELAY);
            }
        }

        hspi->Init.DataSize = SPI_DATASIZE_8BIT;
        #if defined(MCU_SERIES_H7)
        hspi->Instance->CFG1 = (hspi->Instance->CFG1 & ~SPI_CFG1_DSIZE_Msk) | SPI_DATASIZE_8BIT;
        #elif defined(MCU_SERIES_F7)
        hspi->Instance->CR2 = (hspi->Instance->CR2 & ~SPI_CR2_DS_Msk) | SPI_DATASIZE_8BIT;
        #elif defined(MCU_SERIES_F4)
        hspi->Instance->CR1 = (hspi->Instance->CR1 & ~SPI_CR1_DFF_Msk) | SPI_DATASIZE_8BIT;
        #endif
        omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);

        omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
        omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
        HAL_SPI_Transmit(hspi, (uint8_t *) display_on, sizeof(display_on), HAL_MAX_DELAY);
        omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
        omv_gpio_write(OMV_SPI_LCD_RS_PIN, 1);

        fb_free();
    } else {
        // For triple buffering we are never drawing where tail or head (which may instantly update to
        // to be equal to tail) is.
        int new_framebuffer_tail = (framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
        if (new_framebuffer_tail == framebuffer_head) {
            new_framebuffer_tail = (new_framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
        }
        dst_img.data = (uint8_t *) framebuffers[new_framebuffer_tail];

        if (black) {
            // zero the whole image
            memset(dst_img.data, 0, lcd_width * lcd_height * sizeof(uint16_t));
        } else {
            // Zero the top rows
            if (y0) {
                memset(dst_img.data, 0, lcd_width * y0 * sizeof(uint16_t));
            }

            if (x0) {
                for (int i = y0; i < y1; i++) {
                    // Zero left
                    memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, i), 0, x0 * sizeof(uint16_t));
                }
            }

            imlib_draw_image(&dst_img, src_img, dst_x_start, dst_y_start,
                             x_scale, y_scale, roi, rgb_channel, alpha, color_palette,
                             alpha_palette, hint | IMAGE_HINT_BLACK_BACKGROUND, NULL, NULL);

            if (lcd_width - x1) {
                for (int i = y0; i < y1; i++) {
                    // Zero right
                    memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, i) + x1, 0, (lcd_width - x1) * sizeof(uint16_t));
                }
            }

            // Zero the bottom rows
            if (lcd_height - y1) {
                memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, y1),
                       0, lcd_width * (lcd_height - y1) * sizeof(uint16_t));
            }
        }

        // Tell the call back FSM that we want to turn the display on.
        spi_tx_cb_state_on[new_framebuffer_tail] = true;

        #ifdef __DCACHE_PRESENT
        // Flush data for DMA
        SCB_CleanDCache_by_Addr((uint32_t *) dst_img.data, image_size(&dst_img));
        #endif

        // Update tail which means a new image is ready.
        framebuffer_tail = new_framebuffer_tail;

        // Kick off an update of the display.
        spi_lcd_kick();
    }
}

static void spi_lcd_clear() {
    if (!lcd_triple_buffer) {
        omv_gpio_write(OMV_SPI_LCD_RS_PIN, 0);
        omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 0);
        HAL_SPI_Transmit(OMV_SPI_LCD_CONTROLLER->spi, (uint8_t *) display_off, sizeof(display_off), HAL_MAX_DELAY);
        omv_gpio_write(OMV_SPI_LCD_SSEL_PIN, 1);
        omv_gpio_write(OMV_SPI_LCD_RS_PIN, 1);
    } else {
        // For triple buffering we are never drawing where tail or head (which may instantly update to
        // to be equal to tail) is.
        int new_framebuffer_tail = (framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
        if (new_framebuffer_tail == framebuffer_head) {
            new_framebuffer_tail = (new_framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
        }

        // Tell the call back FSM that we want to turn the display off.
        spi_tx_cb_state_on[new_framebuffer_tail] = false;

        // Update tail which means a new image is ready.
        framebuffer_tail = new_framebuffer_tail;

        // Kick off an update of the display.
        spi_lcd_kick();
    }
}

#ifdef OMV_SPI_LCD_BL_DAC
static DAC_HandleTypeDef lcd_dac_handle = {};
#endif

#ifdef OMV_SPI_LCD_BL_PIN
static void spi_lcd_set_backlight(int intensity) {
    #ifdef OMV_SPI_LCD_BL_DAC
    if ((lcd_intensity < 255) && (255 <= intensity)) {
    #else
    if ((lcd_intensity < 1) && (1 <= intensity)) {
    #endif
        omv_gpio_write(OMV_SPI_LCD_BL_PIN, 1);
        omv_gpio_deinit(OMV_SPI_LCD_BL_PIN);
    } else if ((0 < lcd_intensity) && (intensity <= 0)) {
        omv_gpio_config(OMV_SPI_LCD_BL_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_write(OMV_SPI_LCD_BL_PIN, 0);
    }

    #ifdef OMV_SPI_LCD_BL_DAC
    if (((lcd_intensity <= 0) || (255 <= lcd_intensity)) && (0 < intensity) && (intensity < 255)) {
        omv_gpio_config(OMV_SPI_LCD_BL_PIN, OMV_GPIO_MODE_ANALOG, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

        DAC_ChannelConfTypeDef lcd_dac_channel_handle;
        lcd_dac_handle.Instance = OMV_SPI_LCD_BL_DAC;
        lcd_dac_channel_handle.DAC_Trigger = DAC_TRIGGER_NONE;
        lcd_dac_channel_handle.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
        #if defined(MCU_SERIES_H7)
        lcd_dac_channel_handle.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
        lcd_dac_channel_handle.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
        lcd_dac_channel_handle.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
        #endif

        HAL_DAC_Init(&lcd_dac_handle);
        HAL_DAC_ConfigChannel(&lcd_dac_handle, &lcd_dac_channel_handle, OMV_SPI_LCD_BL_DAC_CHANNEL);
        HAL_DAC_Start(&lcd_dac_handle, OMV_SPI_LCD_BL_DAC_CHANNEL);
        HAL_DAC_SetValue(&lcd_dac_handle, OMV_SPI_LCD_BL_DAC_CHANNEL, DAC_ALIGN_8B_R, intensity);
    } else if ((0 < lcd_intensity) && (lcd_intensity < 255) && ((intensity <= 0) || (255 <= intensity))) {
        HAL_DAC_Stop(&lcd_dac_handle, OMV_SPI_LCD_BL_DAC_CHANNEL);
        HAL_DAC_DeInit(&lcd_dac_handle);
    } else if ((0 < lcd_intensity) && (lcd_intensity < 255) && (0 < intensity) && (intensity < 255)) {
        HAL_DAC_SetValue(&lcd_dac_handle, OMV_SPI_LCD_BL_DAC_CHANNEL, DAC_ALIGN_8B_R, intensity);
    }
    #endif

    lcd_intensity = intensity;
}
#endif // OMV_SPI_LCD_BL_PIN
#endif // OMV_SPI_LCD_CONTROLLER

#ifdef OMV_LCD_CONTROLLER
static const uint32_t resolution_clock[] = {
    // CVT-RB ver 2 @ 60 FPS
    6144, // QVGA
    6426, // TQVGA
    9633, // FHVGA
    4799, // FHVGA2
    21363, // VGA
    11868, // THVGA
    26110, // FWVGA
    17670, // FWVGA2
    27624, // TFWVGA
    16615, // TFWVGA2
    32597, // SVGA
    40895, // WSVGA
    52277, // XGA
    85920, // SXGA
    33830, // SXGA2
    124364, // UXGA
    60405, // HD
    133187 // FHD
};

static const uint16_t resolution_w_h[][2] = {
    {320,  240}, // QVGA
    {240,  320}, // TQVGA
    {480,  272}, // FHVGA
    {480,  128}, // FHVGA2
    {640,  480}, // VGA
    {320,  480}, // THVGA
    {800,  480}, // FWVGA
    {800,  320}, // FWVGA2
    {480,  800}, // TFWVGA
    {480,  480}, // TFWVGA2
    {800,  600}, // SVGA
    {1024, 600}, // WSVGA
    {1024, 768}, // XGA
    {1280, 1024}, // SXGA
    {1280, 400}, // SXGA2
    {1600, 1200}, // UXGA
    {1280, 720}, // HD
    {1920, 1080} // FHD
};

static const LTDC_InitTypeDef resolution_cfg[] = {
    // CVT-RB ver 2
    { // QVGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 320 - 1,
        .AccumulatedActiveH = 8 + 6 + 240 - 1,
        .TotalWidth = 32 + 40 + 320 + 8 - 1,
        .TotalHeigh = 8 + 6 + 240 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // TQVGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 240 - 1,
        .AccumulatedActiveH = 8 + 6 + 320 - 1,
        .TotalWidth = 32 + 40 + 240 + 8 - 1,
        .TotalHeigh = 8 + 6 + 320 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // FHVGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 480 - 1,
        .AccumulatedActiveH = 8 + 6 + 272 - 1,
        .TotalWidth = 32 + 40 + 480 + 8 - 1,
        .TotalHeigh = 8 + 6 + 272 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // FHVGA2
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 480 - 1,
        .AccumulatedActiveH = 8 + 6 + 128 - 1,
        .TotalWidth = 32 + 40 + 480 + 8 - 1,
        .TotalHeigh = 8 + 6 + 128 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // VGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 640 - 1,
        .AccumulatedActiveH = 8 + 6 + 480 - 1,
        .TotalWidth = 32 + 40 + 640 + 8 - 1,
        .TotalHeigh = 8 + 6 + 480 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // THVGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 320 - 1,
        .AccumulatedActiveH = 8 + 6 + 480 - 1,
        .TotalWidth = 32 + 40 + 320 + 8 - 1,
        .TotalHeigh = 8 + 6 + 480 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // FWVGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 800 - 1,
        .AccumulatedActiveH = 8 + 6 + 480 - 1,
        .TotalWidth = 32 + 40 + 800 + 8 - 1,
        .TotalHeigh = 8 + 6 + 480 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // FWVGA2
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 800 - 1,
        .AccumulatedActiveH = 8 + 6 + 320 - 1,
        .TotalWidth = 32 + 40 + 800 + 8 - 1,
        .TotalHeigh = 8 + 6 + 320 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // TFWVGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 480 - 1,
        .AccumulatedActiveH = 8 + 6 + 800 - 1,
        .TotalWidth = 32 + 40 + 480 + 8 - 1,
        .TotalHeigh = 8 + 6 + 800 + 9 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // TFWVGA2
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 480 - 1,
        .AccumulatedActiveH = 8 + 6 + 480 - 1,
        .TotalWidth = 32 + 40 + 480 + 8 - 1,
        .TotalHeigh = 8 + 6 + 480 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // SVGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 800 - 1,
        .AccumulatedActiveH = 8 + 6 + 600 - 1,
        .TotalWidth = 32 + 40 + 800 + 8 - 1,
        .TotalHeigh = 8 + 6 + 600 + 4 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // WSVGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 1024 - 1,
        .AccumulatedActiveH = 8 + 6 + 600 - 1,
        .TotalWidth = 32 + 40 + 1024 + 8 - 1,
        .TotalHeigh = 8 + 6 + 600 + 4 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // XGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 1024 - 1,
        .AccumulatedActiveH = 8 + 6 + 768 - 1,
        .TotalWidth = 32 + 40 + 1024 + 8 - 1,
        .TotalHeigh = 8 + 6 + 768 + 8 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // SXGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 1280 - 1,
        .AccumulatedActiveH = 8 + 6 + 1024 - 1,
        .TotalWidth = 32 + 40 + 1280 + 8 - 1,
        .TotalHeigh = 8 + 6 + 1024 + 16 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // SXGA2
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 1280 - 1,
        .AccumulatedActiveH = 8 + 6 + 400 - 1,
        .TotalWidth = 32 + 40 + 1280 + 8 - 1,
        .TotalHeigh = 8 + 6 + 400 + 1 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // UXGA
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 1600 - 1,
        .AccumulatedActiveH = 8 + 6 + 1200 - 1,
        .TotalWidth = 32 + 40 + 1600 + 8 - 1,
        .TotalHeigh = 8 + 6 + 1200 + 21 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // HD
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 1280 - 1,
        .AccumulatedActiveH = 8 + 6 + 720 - 1,
        .TotalWidth = 32 + 40 + 1280 + 8 - 1,
        .TotalHeigh = 8 + 6 + 720 + 7 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    },
    { // FHD
        .HSPolarity = LTDC_HSPOLARITY_AH,
        .VSPolarity = LTDC_VSPOLARITY_AL,
        .DEPolarity = LTDC_DEPOLARITY_AL,
        .PCPolarity = LTDC_PCPOLARITY_IPC,
        .HorizontalSync = 32 - 1,
        .VerticalSync = 8 - 1,
        .AccumulatedHBP = 32 + 40 - 1,
        .AccumulatedVBP = 8 + 6 - 1,
        .AccumulatedActiveW = 32 + 40 + 1920 - 1,
        .AccumulatedActiveH = 8 + 6 + 1080 - 1,
        .TotalWidth = 32 + 40 + 1920 + 8 - 1,
        .TotalHeigh = 8 + 6 + 1080 + 17 - 1,
        .Backcolor = {.Blue = 0, .Green = 0, .Red = 0}
    }
};

static LTDC_HandleTypeDef ltdc_handle = {};
static LTDC_LayerCfgTypeDef ltdc_framebuffer_layers[FRAMEBUFFER_COUNT] = {};

static void ltdc_pll_config_deinit() {
    __HAL_RCC_PLL3_DISABLE();

    uint32_t tickstart = mp_hal_ticks_ms();

    while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLL3RDY)) {
        if ((mp_hal_ticks_ms() - tickstart) > PLL_TIMEOUT_VALUE) {
            break;
        }
    }
}

static void ltdc_pll_config_init(int frame_size, int refresh_rate) {
    uint32_t pixel_clock = (resolution_clock[frame_size] * refresh_rate) / 60;

    for (uint32_t divm = 1; divm <= 63; divm++) {
        for (uint32_t divr = 1; divr <= 128; divr++) {

            uint32_t ref_clk = (HSE_VALUE / 1000) / divm;

            uint32_t vci = 0;
            if (1000 <= ref_clk && ref_clk <= 2000) {
                vci = RCC_PLL3VCIRANGE_0;
            } else if (2000 <= ref_clk && ref_clk <= 4000) {
                vci = RCC_PLL3VCIRANGE_1;
            } else if (4000 <= ref_clk && ref_clk <= 8000) {
                vci = RCC_PLL3VCIRANGE_2;
            } else if (8000 <= ref_clk && ref_clk <= 16000) {
                vci = RCC_PLL3VCIRANGE_3;
            } else {
                continue;
            }

            uint32_t pll_clk = pixel_clock * divr;

            uint32_t vco = 0;
            if (150000 <= pll_clk && pll_clk <= 420000) {
                vco = RCC_PLL3VCOMEDIUM;
            } else if (192000 <= pll_clk && pll_clk <= 836000) {
                vco = RCC_PLL3VCOWIDE;
            } else {
                continue;
            }

            uint32_t divn = pll_clk / ref_clk;
            if (divn < 4 || 512 < divn) {
                continue;
            }

            uint32_t frac = ((pll_clk % ref_clk) * 8192) / ref_clk;

            RCC_PeriphCLKInitTypeDef init;
            init.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
            init.PLL3.PLL3M = divm;
            init.PLL3.PLL3N = divn;
            init.PLL3.PLL3P = 128;
            init.PLL3.PLL3Q = 128;
            init.PLL3.PLL3R = divr;
            init.PLL3.PLL3RGE = vci;
            init.PLL3.PLL3VCOSEL = vco;
            init.PLL3.PLL3FRACN = frac;

            if (HAL_RCCEx_PeriphCLKConfig(&init) == HAL_OK) {
                return;
            }
        }
    }

    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Unable to initialize LTDC PLL!"));
}

static void ltdc_config_deinit() {
    HAL_LTDC_DeInit(&ltdc_handle);
    ltdc_pll_config_deinit();
    fb_alloc_free_till_mark_past_mark_permanent();
}

static void ltdc_config_init(int frame_size, int refresh_rate) {
    int w = resolution_w_h[frame_size][0];
    int h = resolution_w_h[frame_size][1];

    fb_alloc_mark();

    framebuffer_tail = 0;
    framebuffer_head = 0;

    for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
        framebuffers[i] = (uint16_t *) fb_alloc0(w * h * sizeof(uint16_t), FB_ALLOC_CACHE_ALIGN);
        ltdc_framebuffer_layers[i].WindowX0 = 0;
        ltdc_framebuffer_layers[i].WindowX1 = w;
        ltdc_framebuffer_layers[i].WindowY0 = 0;
        ltdc_framebuffer_layers[i].WindowY1 = h;
        ltdc_framebuffer_layers[i].PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
        ltdc_framebuffer_layers[i].Alpha = 0;
        ltdc_framebuffer_layers[i].Alpha0 = 0;
        ltdc_framebuffer_layers[i].BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
        ltdc_framebuffer_layers[i].BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
        ltdc_framebuffer_layers[i].FBStartAdress = (uint32_t) framebuffers[i];
        ltdc_framebuffer_layers[i].ImageWidth = w;
        ltdc_framebuffer_layers[i].ImageHeight = h;
        ltdc_framebuffer_layers[i].Backcolor.Blue = 0;
        ltdc_framebuffer_layers[i].Backcolor.Green = 0;
        ltdc_framebuffer_layers[i].Backcolor.Red = 0;
    }

    ltdc_pll_config_init(frame_size, refresh_rate);

    ltdc_handle.Instance = LTDC;
    memcpy(&ltdc_handle.Init, &resolution_cfg[frame_size], sizeof(LTDC_InitTypeDef));

    HAL_LTDC_Init(&ltdc_handle);

    NVIC_SetPriority(LTDC_IRQn, IRQ_PRI_LTDC);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);

    fb_alloc_mark_permanent();

    // Start interrupt chain.
    HAL_LTDC_ProgramLineEvent(&ltdc_handle, 13); // AccumulatedVBP
}

void LTDC_IRQHandler() {
    IRQ_ENTER(LTDC_IRQn);
    HAL_LTDC_IRQHandler(&ltdc_handle);
    IRQ_EXIT(LTDC_IRQn);
}

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc) {
    HAL_LTDC_ConfigLayer_NoReload(&ltdc_handle, &ltdc_framebuffer_layers[framebuffer_tail], LTDC_LAYER_1);
    HAL_LTDC_Reload(&ltdc_handle, LTDC_RELOAD_VERTICAL_BLANKING);

    #if defined(OMV_LCD_DISP_PIN)
    if (((lcd_type == LCD_DISPLAY) || (lcd_type == LCD_DISPLAY_WITH_HDMI))
        && (framebuffer_tail != framebuffer_head)) {
        // Turn display on if there is a new command.
        omv_gpio_write(OMV_LCD_DISP_PIN, 1);
    }
    #endif
    framebuffer_head = framebuffer_tail;

    // Continue chain...
    HAL_LTDC_ProgramLineEvent(&ltdc_handle, 13); // AccumulatedVBP
}

static void ltdc_display(image_t *src_img, int dst_x_start, int dst_y_start,
                         float x_scale, float y_scale, rectangle_t *roi, int rgb_channel, int alpha,
                         const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint) {
    image_t dst_img;
    dst_img.w = lcd_width;
    dst_img.h = lcd_height;
    dst_img.pixfmt = PIXFORMAT_RGB565;

    int x0, x1, y0, y1;
    bool black = !imlib_draw_image_rectangle(&dst_img,
                                             src_img,
                                             dst_x_start,
                                             dst_y_start,
                                             x_scale,
                                             y_scale,
                                             roi,
                                             alpha,
                                             alpha_palette,
                                             hint,
                                             &x0,
                                             &x1,
                                             &y0,
                                             &y1);

    // For triple buffering we are never drawing where tail or head (which may instantly update to
    // to be equal to tail) is.
    int new_framebuffer_tail = (framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
    if (new_framebuffer_tail == framebuffer_head) {
        new_framebuffer_tail = (new_framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
    }
    dst_img.data = (uint8_t *) framebuffers[new_framebuffer_tail];

    // Set default values for the layer to display the whole framebuffer.
    ltdc_framebuffer_layers[new_framebuffer_tail].WindowX0 = black ? 0 : x0;
    ltdc_framebuffer_layers[new_framebuffer_tail].WindowX1 = black ? lcd_width : x1;
    ltdc_framebuffer_layers[new_framebuffer_tail].WindowY0 = black ? 0 : y0;
    ltdc_framebuffer_layers[new_framebuffer_tail].WindowY1 = black ? lcd_height : y1;
    ltdc_framebuffer_layers[new_framebuffer_tail].Alpha = black ? 0 : fast_roundf((alpha * 255) / 256.f);
    ltdc_framebuffer_layers[new_framebuffer_tail].FBStartAdress =
        black ? ((uint32_t) dst_img.data) : ((uint32_t) (IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, y0) + x0));
    ltdc_framebuffer_layers[new_framebuffer_tail].ImageWidth = black ? lcd_width : dst_img.w;
    ltdc_framebuffer_layers[new_framebuffer_tail].ImageHeight = black ? lcd_height : (y1 - y0);

    // Set alpha to 256 here as we will use the layer alpha to blend the image into the background color of black for free.
    if (!black) {
        imlib_draw_image(&dst_img, src_img, dst_x_start, dst_y_start,
                         x_scale, y_scale, roi, rgb_channel, 256, color_palette,
                         alpha_palette, hint | IMAGE_HINT_BLACK_BACKGROUND, NULL, NULL);
    }

    #ifdef __DCACHE_PRESENT
    // Flush data for DMA
    if (!black) {
        SCB_CleanDCache_by_Addr((uint32_t *) dst_img.data, image_size(&dst_img));
    }
    #endif

    // Update tail which means a new image is ready.
    framebuffer_tail = new_framebuffer_tail;
}

static void ltdc_clear() {
    // For triple buffering we are never drawing where tail or head (which may instantly update to
    // to be equal to tail) is.
    int new_framebuffer_tail = (framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
    if (new_framebuffer_tail == framebuffer_head) {
        new_framebuffer_tail = (new_framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
    }

    // Set default values for the layer to display the whole framebuffer.
    ltdc_framebuffer_layers[new_framebuffer_tail].WindowX0 = 0;
    ltdc_framebuffer_layers[new_framebuffer_tail].WindowX1 = lcd_width;
    ltdc_framebuffer_layers[new_framebuffer_tail].WindowY0 = 0;
    ltdc_framebuffer_layers[new_framebuffer_tail].WindowY1 = lcd_height;
    ltdc_framebuffer_layers[new_framebuffer_tail].Alpha = 0;
    ltdc_framebuffer_layers[new_framebuffer_tail].FBStartAdress = (uint32_t) framebuffers[new_framebuffer_tail];
    ltdc_framebuffer_layers[new_framebuffer_tail].ImageWidth = lcd_width;
    ltdc_framebuffer_layers[new_framebuffer_tail].ImageHeight = lcd_height;

    // Update tail which means a new image is ready.
    framebuffer_tail = new_framebuffer_tail;
}

#ifdef OMV_LCD_BL_TIM
static TIM_HandleTypeDef lcd_tim_handle = {};
#endif

#ifdef OMV_LCD_BL_PIN
static void ltdc_set_backlight(int intensity) {
    #ifdef OMV_LCD_BL_TIM
    if ((lcd_intensity < 255) && (255 <= intensity)) {
    #else
    if ((lcd_intensity < 1) && (1 <= intensity)) {
    #endif
        omv_gpio_config(OMV_LCD_BL_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_write(OMV_LCD_BL_PIN, 1);
    } else if ((0 < lcd_intensity) && (intensity <= 0)) {
        omv_gpio_write(OMV_LCD_BL_PIN, 0);
        omv_gpio_deinit(OMV_LCD_BL_PIN);
    }

    #ifdef OMV_LCD_BL_TIM
    int tclk = OMV_LCD_BL_TIM_PCLK_FREQ() * 2;
    int period = (tclk / OMV_LCD_BL_FREQ) - 1;

    if (((lcd_intensity <= 0) || (255 <= lcd_intensity)) && (0 < intensity) && (intensity < 255)) {
        omv_gpio_config(OMV_LCD_BL_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

        lcd_tim_handle.Instance = OMV_LCD_BL_TIM;
        lcd_tim_handle.Init.Prescaler = 0;
        lcd_tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
        lcd_tim_handle.Init.Period = period;
        lcd_tim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        lcd_tim_handle.Init.RepetitionCounter = 0;
        lcd_tim_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        TIM_OC_InitTypeDef lcd_tim_oc_handle;
        lcd_tim_oc_handle.Pulse = (period * intensity) / 255;
        lcd_tim_oc_handle.OCMode = TIM_OCMODE_PWM1;
        lcd_tim_oc_handle.OCPolarity = TIM_OCPOLARITY_HIGH;
        lcd_tim_oc_handle.OCNPolarity = TIM_OCNPOLARITY_HIGH;
        lcd_tim_oc_handle.OCFastMode = TIM_OCFAST_DISABLE;
        lcd_tim_oc_handle.OCIdleState = TIM_OCIDLESTATE_RESET;
        lcd_tim_oc_handle.OCNIdleState = TIM_OCNIDLESTATE_RESET;

        HAL_TIM_PWM_Init(&lcd_tim_handle);
        HAL_TIM_PWM_ConfigChannel(&lcd_tim_handle, &lcd_tim_oc_handle, OMV_LCD_BL_TIM_CHANNEL);
        HAL_TIM_PWM_Start(&lcd_tim_handle, OMV_LCD_BL_TIM_CHANNEL);
    } else if ((0 < lcd_intensity) && (lcd_intensity < 255) && ((intensity <= 0) || (255 <= intensity))) {
        HAL_TIM_PWM_Stop(&lcd_tim_handle, OMV_LCD_BL_TIM_CHANNEL);
        HAL_TIM_PWM_DeInit(&lcd_tim_handle);
    } else if ((0 < lcd_intensity) && (lcd_intensity < 255) && (0 < intensity) && (intensity < 255)) {
        __HAL_TIM_SET_COMPARE(&lcd_tim_handle, OMV_LCD_BL_TIM_CHANNEL, (period * intensity) / 255);
    }
    #endif

    lcd_intensity = intensity;
}
#endif // OMV_LCD_BL_PIN
#endif // OMV_LCD_CONTROLLER

#ifdef OMV_DVI_PRESENT
#define TFP410_I2C_ADDR    0x3F
mp_obj_base_t *ltdc_dvi_bus = NULL;
#ifdef OMV_DDC_PRESENT
#define EEPROM_I2C_ADDR    0x50
mp_obj_base_t *ltdc_ddc_bus = NULL;
#endif // OMV_DDC_PRESENT
mp_obj_t ltdc_dvi_user_cb = NULL;

static mp_obj_t ltdc_dvi_get_display_connected() {
    mp_obj_base_t *bus = ltdc_dvi_bus ? ltdc_dvi_bus : ((mp_obj_base_t *) MP_OBJ_TYPE_GET_SLOT(
                                                            &mp_machine_soft_i2c_type, make_new) (&mp_machine_soft_i2c_type, 2,
                                                                                                  0, (const mp_obj_t []) {
        (mp_obj_t) OMV_DVI_SCL_PIN, (mp_obj_t) OMV_DVI_SDA_PIN
    }));

    if (mp_machine_soft_i2c_transfer(bus, TFP410_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
        .len = 1, .buf = (uint8_t []) {0x09} // addr
    }), 0) == 1) {
        uint8_t reg;

        if ((mp_machine_soft_i2c_transfer(bus, TFP410_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
            .len = 1, .buf = &reg
        }), MP_MACHINE_I2C_FLAG_READ | MP_MACHINE_I2C_FLAG_STOP) == 0)
            && (mp_machine_soft_i2c_transfer(bus, TFP410_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
            .len = 2, .buf = (uint8_t []) {0x09, 0x19} // clear interrupt flag
        }), MP_MACHINE_I2C_FLAG_STOP) == 2)) {
            return mp_obj_new_bool(reg & 2);
        }
    } else {
        // generate stop on error...
        mp_machine_soft_i2c_transfer(bus, TFP410_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
            .len = 0, .buf = NULL
        }), MP_MACHINE_I2C_FLAG_STOP);
    }

    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to get display connected!"));
}

#ifdef OMV_DDC_PRESENT
static bool ltdc_dvi_checksum(uint8_t *data, int long_count) {
    uint32_t *data32 = (uint32_t *) data;
    uint32_t sum = 0;

    for (int i = 0; i < long_count; i++) {
        sum = __USADA8(data32[i], 0, sum);
    }

    return !(sum & 0xFF);
}

static mp_obj_t ltdc_dvi_get_display_id_data() {
    mp_obj_base_t *bus = ltdc_ddc_bus ? ltdc_ddc_bus :
                         ((mp_obj_base_t *) MP_OBJ_TYPE_GET_SLOT(
                              &mp_machine_soft_i2c_type, make_new) (&mp_machine_soft_i2c_type, 2, 1, (const mp_obj_t []) {
        (mp_obj_t) OMV_DDC_SCL_PIN, (mp_obj_t) OMV_DDC_SDA_PIN,
        MP_OBJ_NEW_QSTR(MP_QSTR_freq),
        MP_OBJ_NEW_SMALL_INT(100000)
    }));

    if (mp_machine_soft_i2c_transfer(bus, EEPROM_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
        .len = 1, .buf = (uint8_t []) {0x00} // addr
    }), MP_MACHINE_I2C_FLAG_STOP) == 1) {
        fb_alloc_mark();
        uint8_t *data = fb_alloc(128, FB_ALLOC_NO_HINT);

        if (mp_machine_soft_i2c_transfer(bus, EEPROM_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
            .len = 128, .buf = data
        }), MP_MACHINE_I2C_FLAG_READ | MP_MACHINE_I2C_FLAG_STOP) == 0) {
            uint32_t *data32 = (uint32_t *) data;

            if ((data32[0] == 0xFFFFFF00) && (data32[1] == 0x00FFFFFF) && ltdc_dvi_checksum(data, 32)
                && (mp_machine_soft_i2c_transfer(bus, EEPROM_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
                .len = 1, .buf = (uint8_t []) {0x80} // addr
            }), MP_MACHINE_I2C_FLAG_STOP) == 1)) {
                int extensions = data[126];
                int extensions_byte_size = extensions * 128;
                int total_data_byte_size = extensions_byte_size + 128;
                uint8_t *data2 = fb_alloc(total_data_byte_size, FB_ALLOC_NO_HINT), *data2_ext = data2 + 128;
                memcpy(data2, data, 128);

                if ((mp_machine_soft_i2c_transfer(bus, EEPROM_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
                    .len = extensions_byte_size, .buf = data2_ext
                }), MP_MACHINE_I2C_FLAG_READ | MP_MACHINE_I2C_FLAG_STOP) == 0)
                    && ltdc_dvi_checksum(data2_ext, extensions_byte_size / 4)) {
                    mp_obj_t result = mp_obj_new_bytes(data2, total_data_byte_size);
                    fb_alloc_free_till_mark();
                    return result;
                }
            }
        }
    }

    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to get display id data!"));
}
#endif // OMV_DDC_PRESENT

static void ltdc_dvi_extint_callback(void *data) {
    if (ltdc_dvi_user_cb) {
        mp_call_function_1(ltdc_dvi_user_cb, ltdc_dvi_get_display_connected());
    }
}

static void ltdc_dvi_deinit() {
    omv_gpio_irq_enable(OMV_DVI_INT_PIN, false);

    ltdc_dvi_user_cb = NULL;
    #ifdef OMV_DDC_PRESENT
    ltdc_ddc_bus = NULL;
    #endif // OMV_DDC_PRESENT
    ltdc_dvi_bus = NULL;

    omv_gpio_write(OMV_DVI_RESET_PIN, 0);
    mp_hal_delay_ms(1);

    omv_gpio_write(OMV_DVI_RESET_PIN, 1);
    mp_hal_delay_ms(1);

    omv_gpio_deinit(OMV_DVI_INT_PIN);
    omv_gpio_deinit(OMV_DVI_RESET_PIN);
    HAL_GPIO_DeInit(OMV_TOUCH_SDA_PIN->gpio, OMV_TOUCH_SDA_PIN->pin_mask);
    HAL_GPIO_DeInit(OMV_TOUCH_SCL_PIN->gpio, OMV_TOUCH_SCL_PIN->pin_mask);
}

static void ltdc_dvi_init() {
    omv_gpio_config(OMV_DVI_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_DVI_RESET_PIN, 0);
    mp_hal_delay_ms(1);
    omv_gpio_write(OMV_DVI_RESET_PIN, 1);
    mp_hal_delay_ms(1);

    ltdc_dvi_bus = (mp_obj_base_t *) MP_OBJ_TYPE_GET_SLOT(
        &mp_machine_soft_i2c_type, make_new) (&mp_machine_soft_i2c_type, 2, 0, (const mp_obj_t []) {
        (mp_obj_t) OMV_DVI_SCL_PIN, (mp_obj_t) OMV_DVI_SDA_PIN
    });

    #ifdef OMV_DDC_PRESENT
    ltdc_ddc_bus = (mp_obj_base_t *) MP_OBJ_TYPE_GET_SLOT(
        &mp_machine_soft_i2c_type, make_new) (&mp_machine_soft_i2c_type, 2, 1, (const mp_obj_t []) {
        (mp_obj_t) OMV_DDC_SCL_PIN, (mp_obj_t) OMV_DDC_SDA_PIN,
        MP_OBJ_NEW_QSTR(MP_QSTR_freq),
        MP_OBJ_NEW_SMALL_INT(100000)
    });
    #endif // OMV_DDC_PRESENT

    if (mp_machine_soft_i2c_transfer(ltdc_dvi_bus, TFP410_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
        .len = 4, .buf = (uint8_t []) {0x08, 0xB7, 0x19, 0x80} // addr, CTL_1, CTL_2, CTL_3
    }), MP_MACHINE_I2C_FLAG_STOP) == 4) {
        omv_gpio_config(OMV_DVI_INT_PIN, OMV_GPIO_MODE_IT_FALL, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_irq_register(OMV_DVI_INT_PIN, ltdc_dvi_extint_callback, NULL);
    } else {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Display init failed!"));
    }
}

static void ltdc_dvi_register_hotplug_cb(mp_obj_t cb) {
    omv_gpio_irq_enable(OMV_DVI_INT_PIN, false);
    ltdc_dvi_user_cb = cb;
    if (cb != mp_const_none) {
        omv_gpio_irq_enable(OMV_DVI_INT_PIN, true);
    }
}
#endif // OMV_DVI_PRESENT

STATIC mp_obj_t py_lcd_deinit() {
    switch (lcd_type) {
        #ifdef OMV_SPI_LCD_CONTROLLER
        case LCD_SHIELD: {
            spi_config_deinit();
            #ifdef OMV_SPI_LCD_BL_PIN
            // back to default state
            spi_lcd_set_backlight(255);
            #endif
            break;
        }
        #endif
        #ifdef OMV_LCD_CONTROLLER
        case LCD_DISPLAY: case LCD_DISPLAY_WITH_HDMI: case LCD_DISPLAY_ONLY_HDMI: {
            ltdc_config_deinit();
            #ifdef OMV_LCD_BL_PIN
            if ((lcd_type == LCD_DISPLAY) || (lcd_type == LCD_DISPLAY_WITH_HDMI)) {
                // back to default state
                ltdc_set_backlight(0);
            }
            #endif // OMV_LCD_BL_PIN
            #ifdef OMV_DVI_PRESENT
            if ((lcd_type == LCD_DISPLAY_WITH_HDMI) || (lcd_type == LCD_DISPLAY_ONLY_HDMI)) {
                ltdc_dvi_deinit();
                #ifdef OMV_CEC_PRESENT
                lcd_cec_deinit();
                #endif
            }
            #endif // OMV_DVI_PRESENT
            #ifdef OMV_TOUCH_PRESENT
            if ((lcd_type == LCD_DISPLAY) || (lcd_type == LCD_DISPLAY_WITH_HDMI)) {
                lcd_touch_deinit();
            }
            #endif // OMV_TOUCH_PRESENT
            break;
        }
        #endif
        default: {
            break;
        }
    }

    lcd_width = 0;
    lcd_height = 0;
    lcd_type = LCD_NONE;
    lcd_triple_buffer = false;
    lcd_bgr = false;
    lcd_resolution = 0;
    lcd_refresh = 0;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_deinit_obj, py_lcd_deinit);

STATIC mp_obj_t py_lcd_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    py_lcd_deinit();

    int type = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_type), LCD_SHIELD);

    switch (type) {
        #ifdef OMV_SPI_LCD_CONTROLLER
        case LCD_SHIELD: {
            int w = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_width), 128);
            if ((w <= 0) || (32767 < w)) {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid Width!"));
            }
            int h = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_height), 160);
            if ((h <= 0) || (32767 < h)) {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid Height!"));
            }
            int refresh_rate = py_helper_keyword_int(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_refresh), 60);
            if ((refresh_rate < 30) || (120 < refresh_rate)) {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid Refresh Rate!"));
            }
            bool triple_buffer = py_helper_keyword_int(n_args, args, 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_triple_buffer), false);
            bool bgr = py_helper_keyword_int(n_args, args, 5, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bgr), false);
            bool byte_reverse = py_helper_keyword_int(n_args, args, 6, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_byte_reverse), false);
            spi_config_init(w, h, refresh_rate, triple_buffer, bgr);
            #ifdef OMV_SPI_LCD_BL_PIN
            spi_lcd_set_backlight(255); // to on state
            #endif
            lcd_width = w;
            lcd_height = h;
            lcd_type = LCD_SHIELD;
            lcd_triple_buffer = triple_buffer;
            lcd_bgr = bgr;
            lcd_byte_reverse = byte_reverse;
            lcd_resolution = 0;
            lcd_refresh = refresh_rate;
            break;
        }
        #endif
        #ifdef OMV_LCD_CONTROLLER
        case LCD_DISPLAY: case LCD_DISPLAY_WITH_HDMI: case LCD_DISPLAY_ONLY_HDMI: {
            int frame_size = py_helper_keyword_int(n_args,
                                                   args,
                                                   1,
                                                   kw_args,
                                                   MP_OBJ_NEW_QSTR(MP_QSTR_framesize),
                                                   LCD_DISPLAY_FWVGA);
            if ((frame_size < 0) || (LCD_DISPLAY_MAX <= frame_size)) {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid Frame Size!"));
            }
            int refresh_rate = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_refresh), 60);
            if ((refresh_rate < 30) || (120 < refresh_rate)) {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid Refresh Rate!"));
            }
            ltdc_config_init(frame_size, refresh_rate);
            #ifdef OMV_LCD_BL_PIN
            if ((type == LCD_DISPLAY) || (type == LCD_DISPLAY_WITH_HDMI)) {
                ltdc_set_backlight(255); // to on state
            }
            #endif // OMV_LCD_BL_PIN
            #ifdef OMV_DVI_PRESENT
            if ((type == LCD_DISPLAY_WITH_HDMI) || (type == LCD_DISPLAY_ONLY_HDMI)) {
                ltdc_dvi_init();
                #ifdef OMV_CEC_PRESENT
                lcd_cec_init();
                #endif
            }
            #endif // OMV_DVI_PRESENT
            #ifdef OMV_TOUCH_PRESENT
            if ((type == LCD_DISPLAY) || (type == LCD_DISPLAY_WITH_HDMI)) {
                lcd_touch_init();
            }
            #endif // OMV_TOUCH_PRESENT
            lcd_width = resolution_w_h[frame_size][0];
            lcd_height = resolution_w_h[frame_size][1];
            lcd_type = LCD_DISPLAY;
            lcd_triple_buffer = true;
            lcd_bgr = false;
            lcd_resolution = frame_size;
            lcd_refresh = refresh_rate;
            break;
        }
        #endif
        default: {
            break;
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_lcd_init_obj, 0, py_lcd_init);

STATIC mp_obj_t py_lcd_width() {
    if (lcd_type == LCD_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_width_obj, py_lcd_width);

STATIC mp_obj_t py_lcd_height() {
    if (lcd_type == LCD_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_height_obj, py_lcd_height);

STATIC mp_obj_t py_lcd_type() {
    if (lcd_type == LCD_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_type);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_type_obj, py_lcd_type);

STATIC mp_obj_t py_lcd_triple_buffer() {
    if (lcd_type == LCD_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_triple_buffer);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_triple_buffer_obj, py_lcd_triple_buffer);

STATIC mp_obj_t py_lcd_bgr() {
    if (lcd_type == LCD_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_bgr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_bgr_obj, py_lcd_bgr);

STATIC mp_obj_t py_lcd_byte_reverse() {
    if (lcd_type == LCD_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_byte_reverse);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_byte_reverse_obj, py_lcd_byte_reverse);

STATIC mp_obj_t py_lcd_framesize() {
    if ((lcd_type == LCD_NONE) || (lcd_type == LCD_SHIELD)) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_resolution);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_framesize_obj, py_lcd_framesize);

STATIC mp_obj_t py_lcd_refresh() {
    if (lcd_type == LCD_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_refresh);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_refresh_obj, py_lcd_refresh);

STATIC mp_obj_t py_lcd_set_backlight(mp_obj_t intensity_obj) {
    int intensity = mp_obj_get_int(intensity_obj);
    if ((intensity < 0) || (255 < intensity)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= intensity <= 255!"));
    }

    switch (lcd_type) {
        #if defined(OMV_SPI_LCD_CONTROLLER) && defined(OMV_SPI_LCD_BL_PIN)
        case LCD_SHIELD: {
            spi_lcd_set_backlight(intensity);
            break;
        }
        #endif
        #if defined(OMV_LCD_CONTROLLER) && defined(OMV_LCD_BL_PIN)
        case LCD_DISPLAY: case LCD_DISPLAY_WITH_HDMI: {
            ltdc_set_backlight(intensity);
            break;
        }
        #endif
        default: {
            break;
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_set_backlight_obj, py_lcd_set_backlight);

STATIC mp_obj_t py_lcd_get_backlight() {
    if ((lcd_type == LCD_NONE) || (lcd_type == LCD_DISPLAY_ONLY_HDMI)) {
        return mp_const_none;
    }
    return mp_obj_new_int(lcd_intensity);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_get_backlight_obj, py_lcd_get_backlight);

#ifdef OMV_DVI_PRESENT
STATIC mp_obj_t py_lcd_get_display_connected() {
    return ltdc_dvi_get_display_connected();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_get_display_connected_obj, py_lcd_get_display_connected);

STATIC mp_obj_t py_lcd_register_hotplug_cb(mp_obj_t cb) {
    ltdc_dvi_register_hotplug_cb(cb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_register_hotplug_cb_obj, py_lcd_register_hotplug_cb);
#endif

#if defined(OMV_DVI_PRESENT) && defined(OMV_DDC_PRESENT)
STATIC mp_obj_t py_lcd_get_display_id_data() {
    return ltdc_dvi_get_display_id_data();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_get_display_id_data_obj, py_lcd_get_display_id_data);
#endif

#if defined(OMV_DVI_PRESENT) && defined(OMV_CEC_PRESENT)
STATIC mp_obj_t py_lcd_send_frame(mp_obj_t dst_addr, mp_obj_t src_addr, mp_obj_t bytes) {
    lcd_cec_send_frame(dst_addr, src_addr, bytes);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_lcd_send_frame_obj, py_lcd_send_frame);

STATIC mp_obj_t py_lcd_receive_frame(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return lcd_cec_receive_frame(n_args, args, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_lcd_receive_frame_obj, 1, py_lcd_receive_frame);

STATIC mp_obj_t py_lcd_register_cec_receive_cb(mp_obj_t cb, mp_obj_t dst_addr) {
    lcd_cec_register_cec_receive_cb(cb, dst_addr);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_lcd_register_cec_receive_cb_obj, py_lcd_register_cec_receive_cb);

STATIC mp_obj_t py_lcd_received_frame_src_addr() {
    return lcd_cec_received_frame_src_addr();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_received_frame_src_addr_obj, py_lcd_received_frame_src_addr);

STATIC mp_obj_t py_lcd_received_frame_bytes() {
    return lcd_cec_received_frame_bytes();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_received_frame_bytes_obj, py_lcd_received_frame_bytes);
#endif

#ifdef OMV_TOUCH_PRESENT
STATIC mp_obj_t py_lcd_update_touch_points() {
    return lcd_touch_update_touch_points();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_update_touch_points_obj, py_lcd_update_touch_points);

STATIC mp_obj_t py_lcd_register_touch_cb(mp_obj_t cb) {
    lcd_touch_register_touch_cb(cb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_register_touch_cb_obj, py_lcd_register_touch_cb);

STATIC mp_obj_t py_lcd_get_gesture() {
    return lcd_touch_get_gesture();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_get_gesture_obj, py_lcd_get_gesture);

STATIC mp_obj_t py_lcd_get_points() {
    return lcd_touch_get_points();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_get_points_obj, py_lcd_get_points);

STATIC mp_obj_t py_lcd_get_point_flag(mp_obj_t index) {
    return lcd_touch_get_point_flag(index);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_get_point_flag_obj, py_lcd_get_point_flag);

STATIC mp_obj_t py_lcd_get_point_id(mp_obj_t index) {
    return lcd_touch_get_point_id(index);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_get_point_id_obj, py_lcd_get_point_id);

STATIC mp_obj_t py_lcd_get_point_x_position(mp_obj_t index) {
    return lcd_touch_get_point_x_position(index);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_get_point_x_position_obj, py_lcd_get_point_x_position);

STATIC mp_obj_t py_lcd_get_point_y_position(mp_obj_t index) {
    return lcd_touch_get_point_y_position(index);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_get_point_y_position_obj, py_lcd_get_point_y_position);
#endif

STATIC mp_obj_t py_lcd_display(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    image_t *arg_img = py_image_cobj(args[0]);

    int arg_x_off = 0;
    int arg_y_off = 0;
    uint offset = 1;
    if (n_args > 1) {
        if (MP_OBJ_IS_TYPE(args[1], &mp_type_tuple) || MP_OBJ_IS_TYPE(args[1], &mp_type_list)) {
            mp_obj_t *arg_vec;
            mp_obj_get_array_fixed_n(args[1], 2, &arg_vec);
            arg_x_off = mp_obj_get_int(arg_vec[0]);
            arg_y_off = mp_obj_get_int(arg_vec[1]);
            offset = 2;
        } else if (n_args > 2) {
            arg_x_off = mp_obj_get_int(args[1]);
            arg_y_off = mp_obj_get_int(args[2]);
            offset = 3;
        } else if (n_args > 1) {
            mp_raise_msg(&mp_type_TypeError, MP_ERROR_TEXT("Expected x and y offset!"));
        }
    }

    float arg_x_scale = 1.f;
    bool got_x_scale = py_helper_keyword_float_maybe(n_args,
                                                     args,
                                                     offset + 0,
                                                     kw_args,
                                                     MP_OBJ_NEW_QSTR(MP_QSTR_x_scale),
                                                     &arg_x_scale);

    float arg_y_scale = 1.f;
    bool got_y_scale = py_helper_keyword_float_maybe(n_args,
                                                     args,
                                                     offset + 1,
                                                     kw_args,
                                                     MP_OBJ_NEW_QSTR(MP_QSTR_y_scale),
                                                     &arg_y_scale);

    rectangle_t arg_roi;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, offset + 2, kw_args, &arg_roi);

    int arg_rgb_channel = py_helper_keyword_int(n_args, args, offset + 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_rgb_channel), -1);
    if ((arg_rgb_channel < -1) || (2 < arg_rgb_channel)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("-1 <= rgb_channel <= 2!"));
    }

    int arg_alpha = py_helper_keyword_int(n_args, args, offset + 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 256);
    if ((arg_alpha < 0) || (256 < arg_alpha)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= alpha <= 256!"));
    }

    const uint16_t *color_palette = py_helper_keyword_color_palette(n_args, args, offset + 5, kw_args, NULL);
    const uint8_t *alpha_palette = py_helper_keyword_alpha_palette(n_args, args, offset + 6, kw_args, NULL);

    image_hint_t hint = py_helper_keyword_int(n_args, args, offset + 7, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_hint), 0);

    float arg_x_size;
    bool got_x_size = py_helper_keyword_float_maybe(n_args,
                                                    args,
                                                    offset + 8,
                                                    kw_args,
                                                    MP_OBJ_NEW_QSTR(MP_QSTR_x_size),
                                                    &arg_x_size);

    float arg_y_size;
    bool got_y_size = py_helper_keyword_float_maybe(n_args,
                                                    args,
                                                    offset + 9,
                                                    kw_args,
                                                    MP_OBJ_NEW_QSTR(MP_QSTR_y_size),
                                                    &arg_y_size);

    if (got_x_scale && got_x_size) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Choose either x_scale or x_size not both!"));
    }
    if (got_y_scale && got_y_size) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Choose either y_scale or y_size not both!"));
    }

    if (got_x_size) {
        arg_x_scale = arg_x_size / arg_roi.w;
    }
    if (got_y_size) {
        arg_y_scale = arg_y_size / arg_roi.h;
    }

    if ((!got_x_scale) && (!got_x_size) && got_y_size) {
        arg_x_scale = arg_y_scale;
    }
    if ((!got_y_scale) && (!got_y_size) && got_x_size) {
        arg_y_scale = arg_x_scale;
    }

    if ((!lcd_triple_buffer) && (arg_y_scale < 0)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Vertical flip requires triple buffering!"));
    }

    switch (lcd_type) {
        #ifdef OMV_SPI_LCD_CONTROLLER
        case LCD_SHIELD: {
            fb_alloc_mark();
            spi_lcd_display(arg_img, arg_x_off, arg_y_off, arg_x_scale, arg_y_scale, &arg_roi,
                            arg_rgb_channel, arg_alpha, color_palette, alpha_palette, hint);
            fb_alloc_free_till_mark();
            break;
        }
        #endif
        #ifdef OMV_LCD_CONTROLLER
        case LCD_DISPLAY: case LCD_DISPLAY_WITH_HDMI: case LCD_DISPLAY_ONLY_HDMI: {
            fb_alloc_mark();
            ltdc_display(arg_img, arg_x_off, arg_y_off, arg_x_scale, arg_y_scale, &arg_roi,
                         arg_rgb_channel, arg_alpha, color_palette, alpha_palette, hint);
            fb_alloc_free_till_mark();
            break;
        }
        #endif
        default: {
            break;
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_lcd_display_obj, 1, py_lcd_display);

STATIC mp_obj_t py_lcd_clear(uint n_args, const mp_obj_t *args) {
    switch (lcd_type) {
        #ifdef OMV_SPI_LCD_CONTROLLER
        case LCD_SHIELD: {
            if (n_args && mp_obj_get_int(*args)) {
                // turns the display off (may not be black)
                spi_lcd_clear();
            } else {
                // sets the display to black (not off)
                fb_alloc_mark();
                spi_lcd_display(NULL, 0, 0, 1.f, 1.f, NULL,
                                0, 0, NULL, NULL, 0);
                fb_alloc_free_till_mark();
            }
            break;
        }
        #endif
        #ifdef OMV_LCD_CONTROLLER
        case LCD_DISPLAY: case LCD_DISPLAY_WITH_HDMI: case LCD_DISPLAY_ONLY_HDMI: {
            #if defined(OMV_LCD_DISP_PIN)
            if ((lcd_type == LCD_DISPLAY) && n_args && mp_obj_get_int(*args)) {
                // turns the display off (may not be black)
                omv_gpio_write(OMV_LCD_DISP_PIN, 0);
            } else {
                // sets the display to black (not off)
            #else
            {
            #endif
                ltdc_clear();
            }
            break;
        }
        #endif
        default: {
            break;
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_lcd_clear_obj, 0, 1, py_lcd_clear);

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),                MP_OBJ_NEW_QSTR(MP_QSTR_lcd)                    },
    { MP_ROM_QSTR(MP_QSTR_LCD_NONE),                MP_ROM_INT(LCD_NONE)                            },
    { MP_ROM_QSTR(MP_QSTR_LCD_SHIELD),              MP_ROM_INT(LCD_SHIELD)                          },
    #ifdef OMV_DVI_PRESENT
    { MP_ROM_QSTR(MP_QSTR_LCD_DISPLAY),             MP_ROM_INT(LCD_DISPLAY)                         },
    { MP_ROM_QSTR(MP_QSTR_LCD_DISPLAY_WITH_HDMI),   MP_ROM_INT(LCD_DISPLAY_WITH_HDMI)               },
    { MP_ROM_QSTR(MP_QSTR_LCD_DISPLAY_ONLY_HDMI),   MP_ROM_INT(LCD_DISPLAY_ONLY_HDMI)               },
    { MP_ROM_QSTR(MP_QSTR_QVGA),                    MP_ROM_INT(LCD_DISPLAY_QVGA)                    },
    { MP_ROM_QSTR(MP_QSTR_TQVGA),                   MP_ROM_INT(LCD_DISPLAY_TQVGA)                   },
    { MP_ROM_QSTR(MP_QSTR_FHVGA),                   MP_ROM_INT(LCD_DISPLAY_FHVGA)                   },
    { MP_ROM_QSTR(MP_QSTR_FHVGA2),                  MP_ROM_INT(LCD_DISPLAY_FHVGA2)                  },
    { MP_ROM_QSTR(MP_QSTR_VGA),                     MP_ROM_INT(LCD_DISPLAY_VGA)                     },
    { MP_ROM_QSTR(MP_QSTR_THVGA),                   MP_ROM_INT(LCD_DISPLAY_THVGA)                   },
    { MP_ROM_QSTR(MP_QSTR_FWVGA),                   MP_ROM_INT(LCD_DISPLAY_FWVGA)                   },
    { MP_ROM_QSTR(MP_QSTR_FWVGA2),                  MP_ROM_INT(LCD_DISPLAY_FWVGA2)                  },
    { MP_ROM_QSTR(MP_QSTR_TFWVGA),                  MP_ROM_INT(LCD_DISPLAY_TFWVGA)                  },
    { MP_ROM_QSTR(MP_QSTR_TFWVGA2),                 MP_ROM_INT(LCD_DISPLAY_TFWVGA2)                 },
    { MP_ROM_QSTR(MP_QSTR_SVGA),                    MP_ROM_INT(LCD_DISPLAY_SVGA)                    },
    { MP_ROM_QSTR(MP_QSTR_WSVGA),                   MP_ROM_INT(LCD_DISPLAY_WSVGA)                   },
    { MP_ROM_QSTR(MP_QSTR_XGA),                     MP_ROM_INT(LCD_DISPLAY_XGA)                     },
    { MP_ROM_QSTR(MP_QSTR_SXGA),                    MP_ROM_INT(LCD_DISPLAY_SXGA)                    },
    { MP_ROM_QSTR(MP_QSTR_SXGA2),                   MP_ROM_INT(LCD_DISPLAY_SXGA2)                   },
    { MP_ROM_QSTR(MP_QSTR_UXGA),                    MP_ROM_INT(LCD_DISPLAY_UXGA)                    },
    { MP_ROM_QSTR(MP_QSTR_HD),                      MP_ROM_INT(LCD_DISPLAY_HD)                      },
    { MP_ROM_QSTR(MP_QSTR_FHD),                     MP_ROM_INT(LCD_DISPLAY_FHD)                     },
    #endif
    #ifdef OMV_TOUCH_PRESENT
    { MP_ROM_QSTR(MP_QSTR_LCD_GESTURE_MOVE_UP),     MP_ROM_INT(PY_LCD_TOUCH_GESTURE_MOVE_UP)        },
    { MP_ROM_QSTR(MP_QSTR_LCD_GESTURE_MOVE_LEFT),   MP_ROM_INT(PY_LCD_TOUCH_GESTURE_MOVE_LEFT)      },
    { MP_ROM_QSTR(MP_QSTR_LCD_GESTURE_MOVE_DOWN),   MP_ROM_INT(PY_LCD_TOUCH_GESTURE_MOVE_DOWN)      },
    { MP_ROM_QSTR(MP_QSTR_LCD_GESTURE_MOVE_RIGHT),  MP_ROM_INT(PY_LCD_TOUCH_GESTURE_MOVE_RIGHT)     },
    { MP_ROM_QSTR(MP_QSTR_LCD_GESTURE_ZOOM_IN),     MP_ROM_INT(PY_LCD_TOUCH_GESTURE_ZOOM_IN)        },
    { MP_ROM_QSTR(MP_QSTR_LCD_GESTURE_ZOOM_OUT),    MP_ROM_INT(PY_LCD_TOUCH_GESTURE_ZOOM_OUT)       },
    { MP_ROM_QSTR(MP_QSTR_LCD_GESTURE_NONE),        MP_ROM_INT(PY_LCD_TOUCH_GESTURE_NONE)           },
    { MP_ROM_QSTR(MP_QSTR_LCD_FLAG_PRESSED),        MP_ROM_INT(PY_LCD_TOUCH_EVENT_PUT_DOWN)         },
    { MP_ROM_QSTR(MP_QSTR_LCD_FLAG_RELEASED),       MP_ROM_INT(PY_LCD_TOUCH_EVENT_PUT_UP)           },
    { MP_ROM_QSTR(MP_QSTR_LCD_FLAG_MOVED),          MP_ROM_INT(PY_LCD_TOUCH_EVENT_CONTACT)          },
    #endif
    { MP_ROM_QSTR(MP_QSTR_init),                    MP_ROM_PTR(&py_lcd_init_obj)                    },
    { MP_ROM_QSTR(MP_QSTR_deinit),                  MP_ROM_PTR(&py_lcd_deinit_obj)                  },
    { MP_ROM_QSTR(MP_QSTR_width),                   MP_ROM_PTR(&py_lcd_width_obj)                   },
    { MP_ROM_QSTR(MP_QSTR_height),                  MP_ROM_PTR(&py_lcd_height_obj)                  },
    { MP_ROM_QSTR(MP_QSTR_type),                    MP_ROM_PTR(&py_lcd_type_obj)                    },
    { MP_ROM_QSTR(MP_QSTR_triple_buffer),           MP_ROM_PTR(&py_lcd_triple_buffer_obj)           },
    { MP_ROM_QSTR(MP_QSTR_bgr),                     MP_ROM_PTR(&py_lcd_bgr_obj)                     },
    { MP_ROM_QSTR(MP_QSTR_byte_reverse),            MP_ROM_PTR(&py_lcd_byte_reverse_obj)            },
    { MP_ROM_QSTR(MP_QSTR_framesize),               MP_ROM_PTR(&py_lcd_framesize_obj)               },
    { MP_ROM_QSTR(MP_QSTR_refresh),                 MP_ROM_PTR(&py_lcd_refresh_obj)                 },
    { MP_ROM_QSTR(MP_QSTR_get_backlight),           MP_ROM_PTR(&py_lcd_get_backlight_obj)           },
    { MP_ROM_QSTR(MP_QSTR_set_backlight),           MP_ROM_PTR(&py_lcd_set_backlight_obj)           },
    #ifdef OMV_DVI_PRESENT
    { MP_ROM_QSTR(MP_QSTR_get_display_connected),   MP_ROM_PTR(&py_lcd_get_display_connected_obj)   },
    { MP_ROM_QSTR(MP_QSTR_register_hotplug_cb),     MP_ROM_PTR(&py_lcd_register_hotplug_cb_obj)     },
    #else
    { MP_ROM_QSTR(MP_QSTR_get_display_connected),   MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_register_hotplug_cb),     MP_ROM_PTR(&py_func_unavailable_obj)            },
    #endif
    #if defined(OMV_DVI_PRESENT) && defined(OMV_DDC_PRESENT)
    { MP_ROM_QSTR(MP_QSTR_get_display_id_data),     MP_ROM_PTR(&py_lcd_get_display_id_data_obj)     },
    #else
    { MP_ROM_QSTR(MP_QSTR_get_display_id_data),     MP_ROM_PTR(&py_func_unavailable_obj)            },
    #endif
    #if defined(OMV_DVI_PRESENT) && defined(OMV_CEC_PRESENT)
    { MP_ROM_QSTR(MP_QSTR_send_frame),              MP_ROM_PTR(&py_lcd_send_frame_obj)              },
    { MP_ROM_QSTR(MP_QSTR_receive_frame),           MP_ROM_PTR(&py_lcd_receive_frame_obj)           },
    { MP_ROM_QSTR(MP_QSTR_register_receive_cb),     MP_ROM_PTR(&py_lcd_register_cec_receive_cb_obj) },
    { MP_ROM_QSTR(MP_QSTR_received_frame_src_addr), MP_ROM_PTR(&py_lcd_received_frame_src_addr_obj) },
    { MP_ROM_QSTR(MP_QSTR_received_frame_bytes),    MP_ROM_PTR(&py_lcd_received_frame_bytes_obj)    },
    #else
    { MP_ROM_QSTR(MP_QSTR_send_frame),              MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_receive_frame),           MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_register_receive_cb),     MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_received_frame_src_addr), MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_received_frame_bytes),    MP_ROM_PTR(&py_func_unavailable_obj)            },
    #endif
    #ifdef OMV_TOUCH_PRESENT
    { MP_ROM_QSTR(MP_QSTR_update_touch_points),     MP_ROM_PTR(&py_lcd_update_touch_points_obj)     },
    { MP_ROM_QSTR(MP_QSTR_register_touch_cb),       MP_ROM_PTR(&py_lcd_register_touch_cb_obj)       },
    { MP_ROM_QSTR(MP_QSTR_get_gesture),             MP_ROM_PTR(&py_lcd_get_gesture_obj)             },
    { MP_ROM_QSTR(MP_QSTR_get_points),              MP_ROM_PTR(&py_lcd_get_points_obj)              },
    { MP_ROM_QSTR(MP_QSTR_get_point_flag),          MP_ROM_PTR(&py_lcd_get_point_flag_obj)          },
    { MP_ROM_QSTR(MP_QSTR_get_point_id),            MP_ROM_PTR(&py_lcd_get_point_id_obj)            },
    { MP_ROM_QSTR(MP_QSTR_get_point_x_position),    MP_ROM_PTR(&py_lcd_get_point_x_position_obj)    },
    { MP_ROM_QSTR(MP_QSTR_get_point_y_position),    MP_ROM_PTR(&py_lcd_get_point_y_position_obj)    },
    #else
    { MP_ROM_QSTR(MP_QSTR_update_touch_points),     MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_register_touch_cb),       MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_get_gesture),             MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_get_points),              MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_get_point_flag),          MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_get_point_id),            MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_get_point_x_position),    MP_ROM_PTR(&py_func_unavailable_obj)            },
    { MP_ROM_QSTR(MP_QSTR_get_point_y_position),    MP_ROM_PTR(&py_func_unavailable_obj)            },
    #endif
    { MP_ROM_QSTR(MP_QSTR_display),                 MP_ROM_PTR(&py_lcd_display_obj)                 },
    { MP_ROM_QSTR(MP_QSTR_clear),                   MP_ROM_PTR(&py_lcd_clear_obj)                   },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t lcd_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

void py_lcd_init0() {
    py_lcd_deinit();
}

MP_REGISTER_MODULE(MP_QSTR_lcd, lcd_module);
#endif // MICROPY_PY_LCD
