/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * LTDC display Python module.
 */
#include "omv_boardconfig.h"

#if MICROPY_PY_DISPLAY && defined(OMV_RGB_DISPLAY_CONTROLLER)

#include "py/obj.h"
#include "py/runtime.h"
#include "mphal.h"

#include "py_helper.h"
#include "py_image.h"
#include "omv_gpio.h"
#include "py_display.h"

#if defined(OMV_RGB_DISPLAY_BL_PIN)
#define OMV_DISPLAY_BL_PIN OMV_RGB_DISPLAY_BL_PIN
#endif

#if defined(OMV_RGB_DISPLAY_DISP_PIN)
#define OMV_DISPLAY_DISP_PIN OMV_RGB_DISPLAY_DISP_PIN
#endif

typedef struct display_mode {
    uint32_t hactive;
    uint32_t vactive;
    uint32_t pixel_clock;
    uint32_t hsync_len;
    uint32_t hback_porch;
    uint32_t hfront_porch;
    uint32_t vsync_len;
    uint32_t vback_porch;
    uint32_t vfront_porch;
    uint32_t hpol : 1;
    uint32_t vpol : 1;
} display_mode_t;

typedef struct _display_state {
    py_display_obj_t *self;
    LTDC_HandleTypeDef hltdc;
    #ifdef OMV_DISPLAY_BL_TIM
    TIM_HandleTypeDef htim;
    #endif
    LTDC_LayerCfgTypeDef framebuffer_layers[FRAMEBUFFER_COUNT];
} display_state_t;

static display_state_t display;

static const display_mode_t display_modes[] = {
    { // QVGA
        .hactive=320, .vactive=240, .pixel_clock=6144,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // TQVGA
        .hactive=240, .vactive=320, .pixel_clock=6426,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // FHVGA
        .hactive=480, .vactive=272, .pixel_clock=9633,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // FHVGA2
        .hactive=480, .vactive=128, .pixel_clock=4799,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // VGA
        .hactive=640, .vactive=480, .pixel_clock=21363,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // THVGA
        .hactive=320, .vactive=480, .pixel_clock=11868,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // FWVGA
        .hactive=800, .vactive=480, .pixel_clock=26110,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // FWVGA2
        .hactive=800, .vactive=320, .pixel_clock=17670,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // TFWVGA
        .hactive=480, .vactive=800, .pixel_clock=27624,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=9
    },
    { // TFWVGA2
        .hactive=480, .vactive=480, .pixel_clock=16615,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // SVGA
        .hactive=800, .vactive=600, .pixel_clock=32597,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=4
    },
    { // WSVGA
        .hactive=1024, .vactive=600, .pixel_clock=40895,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=4
    },
    { // XGA
        .hactive=1024, .vactive=768, .pixel_clock=52277,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=8
    },
    { // SXGA
        .hactive=1280, .vactive=1024, .pixel_clock=85920,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=16
    },
    { // SXGA2
        .hactive=1280, .vactive=400, .pixel_clock=33830,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=1
    },
    { // UXGA
        .hactive=1600, .vactive=1200, .pixel_clock=124364,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=21
    },
    { // HD
        .hactive=1280, .vactive=720, .pixel_clock=60405,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=7
    },
    { // FHD
        .hactive=1920, .vactive=1080, .pixel_clock=133187,
        .hsync_len=32, .hback_porch=40, .hfront_porch=8,
        .vsync_len=8, .vback_porch=6, .vfront_porch=17
    },
};

void LTDC_IRQHandler() {
    HAL_LTDC_IRQHandler(&display.hltdc);
}

static void pll_config(int framesize, int refresh) {
    uint32_t pixel_clock = (display_modes[framesize].pixel_clock * refresh) / 60;

    for (uint32_t divm = 1; divm <= 63; divm++) {
        for (uint32_t divr = 1; divr <= 128; divr++) {
            uint32_t vci = 0;
            uint32_t ref_clk = (HSE_VALUE / 1000) / divm;

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

            uint32_t vco = 0;
            uint32_t pll_clk = pixel_clock * divr;

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

static void ltdc_init(py_display_obj_t *self) {
    const display_mode_t *dm = &display_modes[self->framesize];
    uint32_t fb_size = self->width * self->height * sizeof(uint16_t);

    fb_alloc_mark();
    for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
        self->framebuffers[i] = (uint16_t *) fb_alloc0(fb_size, FB_ALLOC_CACHE_ALIGN);
        display.framebuffer_layers[i].WindowX0 = 0;
        display.framebuffer_layers[i].WindowX1 = self->width;
        display.framebuffer_layers[i].WindowY0 = 0;
        display.framebuffer_layers[i].WindowY1 = self->height;
        display.framebuffer_layers[i].PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
        display.framebuffer_layers[i].Alpha = 0;
        display.framebuffer_layers[i].Alpha0 = 0;
        display.framebuffer_layers[i].BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
        display.framebuffer_layers[i].BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
        display.framebuffer_layers[i].FBStartAdress = (uint32_t) self->framebuffers[i];
        display.framebuffer_layers[i].ImageWidth = self->width;
        display.framebuffer_layers[i].ImageHeight = self->height;
        display.framebuffer_layers[i].Backcolor.Blue = 0;
        display.framebuffer_layers[i].Backcolor.Green = 0;
        display.framebuffer_layers[i].Backcolor.Red = 0;
    }
    fb_alloc_mark_permanent();

    display.hltdc.Instance = LTDC;
    display.hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AH,
    display.hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL,
    display.hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL,
    display.hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC,

    display.hltdc.Init.HorizontalSync = dm->hsync_len -1;
    display.hltdc.Init.VerticalSync = dm->vsync_len - 1;
    display.hltdc.Init.AccumulatedHBP = dm->hsync_len + dm->hback_porch -1;
    display.hltdc.Init.AccumulatedVBP = dm->vsync_len + dm->vback_porch -1;
    display.hltdc.Init.AccumulatedActiveW = dm->hsync_len + dm->hback_porch + self->width -1;
    display.hltdc.Init.AccumulatedActiveH = dm->vsync_len + dm->vback_porch + self->height -1;
    display.hltdc.Init.TotalWidth = dm->hsync_len + dm->hback_porch + self->width + dm->hfront_porch - 1;
    display.hltdc.Init.TotalHeigh = dm->vsync_len + dm->vback_porch + self->height + dm->vfront_porch - 1;

    display.hltdc.Init.Backcolor.Blue = 0;
    display.hltdc.Init.Backcolor.Green = 0;
    display.hltdc.Init.Backcolor.Red = 0;
    HAL_LTDC_Init(&display.hltdc);

    NVIC_SetPriority(LTDC_IRQn, IRQ_PRI_LTDC);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);

    // Start interrupt chain.
    HAL_LTDC_ProgramLineEvent(&display.hltdc, 13); // AccumulatedVBP
}

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc) {
    py_display_obj_t *self = display.self;

    HAL_LTDC_ConfigLayer_NoReload(&display.hltdc,
                                  &display.framebuffer_layers[self->framebuffer_tail], LTDC_LAYER_1);
    HAL_LTDC_Reload(&display.hltdc, LTDC_RELOAD_VERTICAL_BLANKING);

    #if defined(OMV_DISPLAY_DISP_PIN)
    if (self->display_on && (self->framebuffer_tail != self->framebuffer_head)) {
        // Turn display on if there is a new command.
        omv_gpio_write(OMV_DISPLAY_DISP_PIN, 1);
    }
    #endif
    self->framebuffer_head = self->framebuffer_tail;

    // Continue chain...
    HAL_LTDC_ProgramLineEvent(&display.hltdc, 13); // AccumulatedVBP
}

static void display_write(py_display_obj_t *self, image_t *src_img, int dst_x_start, int dst_y_start,
                              float x_scale, float y_scale, rectangle_t *roi, int rgb_channel, int alpha,
                              const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint) {
    image_t dst_img;
    dst_img.w = self->width;
    dst_img.h = self->height;
    dst_img.pixfmt = PIXFORMAT_RGB565;

    int x0, x1, y0, y1;
    bool black = !imlib_draw_image_rectangle(&dst_img, src_img, dst_x_start, dst_y_start, x_scale, y_scale,
                                             roi, alpha, alpha_palette, hint, &x0, &x1, &y0, &y1);

    // For triple buffering we are never drawing where tail or head (which may instantly update to
    // to be equal to tail) is.
    int tail = (self->framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
    if (tail == self->framebuffer_head) {
        tail = (tail + 1) % FRAMEBUFFER_COUNT;
    }
    dst_img.data = (uint8_t *) self->framebuffers[tail];

    // Set default values for the layer to display the whole framebuffer.
    display.framebuffer_layers[tail].WindowX0 = black ? 0 : x0;
    display.framebuffer_layers[tail].WindowX1 = black ? self->width : x1;
    display.framebuffer_layers[tail].WindowY0 = black ? 0 : y0;
    display.framebuffer_layers[tail].WindowY1 = black ? self->height : y1;
    display.framebuffer_layers[tail].Alpha = black ? 0 : fast_roundf((alpha * 255) / 256.f);
    display.framebuffer_layers[tail].FBStartAdress =
        black ? ((uint32_t) dst_img.data) : ((uint32_t) (IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&dst_img, y0) + x0));
    display.framebuffer_layers[tail].ImageWidth = black ? self->width : dst_img.w;
    display.framebuffer_layers[tail].ImageHeight = black ? self->height : (y1 - y0);

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
    self->framebuffer_tail = tail;
}

static void display_clear(py_display_obj_t *self, bool off) {
    #if defined(OMV_DISPLAY_DISP_PIN)
    if (self->display_on && off) {
        // turns the display off (may not be black)
        omv_gpio_write(OMV_DISPLAY_DISP_PIN, 0);
        return;
    }
    #endif

    // For triple buffering we are never drawing where tail or head (which may instantly update to
    // to be equal to tail) is.
    int tail = (self->framebuffer_tail + 1) % FRAMEBUFFER_COUNT;
    if (tail == self->framebuffer_head) {
        tail = (tail + 1) % FRAMEBUFFER_COUNT;
    }

    // Set default values for the layer to display the whole framebuffer.
    display.framebuffer_layers[tail].WindowX0 = 0;
    display.framebuffer_layers[tail].WindowX1 = self->width;
    display.framebuffer_layers[tail].WindowY0 = 0;
    display.framebuffer_layers[tail].WindowY1 = self->height;
    display.framebuffer_layers[tail].Alpha = 0;
    display.framebuffer_layers[tail].FBStartAdress = (uint32_t) self->framebuffers[tail];
    display.framebuffer_layers[tail].ImageWidth = self->width;
    display.framebuffer_layers[tail].ImageHeight = self->height;

    // Update tail which means a new image is ready.
    self->framebuffer_tail = tail;
}

#ifdef OMV_DISPLAY_BL_PIN
static void display_set_backlight(py_display_obj_t *self, uint32_t intensity) {
    #ifdef OMV_DISPLAY_BL_TIM
    if ((self->intensity < 255) && (255 <= intensity)) {
    #else
    if ((self->intensity < 1) && (1 <= intensity)) {
    #endif
        omv_gpio_config(OMV_DISPLAY_BL_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_write(OMV_DISPLAY_BL_PIN, 1);
    } else if ((0 < self->intensity) && (intensity <= 0)) {
        omv_gpio_write(OMV_DISPLAY_BL_PIN, 0);
        omv_gpio_deinit(OMV_DISPLAY_BL_PIN);
    }

    #ifdef OMV_DISPLAY_BL_TIM
    int tclk = OMV_DISPLAY_BL_TIM_PCLK_FREQ() * 2;
    int period = (tclk / OMV_DISPLAY_BL_TIM_FREQ) - 1;

    if (((self->intensity <= 0) || (255 <= self->intensity)) && (0 < intensity) && (intensity < 255)) {
        omv_gpio_config(OMV_DISPLAY_BL_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

        display.htim.Instance = OMV_DISPLAY_BL_TIM;
        display.htim.Init.Prescaler = 0;
        display.htim.Init.CounterMode = TIM_COUNTERMODE_UP;
        display.htim.Init.Period = period;
        display.htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        display.htim.Init.RepetitionCounter = 0;
        display.htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        TIM_OC_InitTypeDef lcd_tim_oc_handle;
        lcd_tim_oc_handle.Pulse = (period * intensity) / 255;
        lcd_tim_oc_handle.OCMode = TIM_OCMODE_PWM1;
        lcd_tim_oc_handle.OCPolarity = TIM_OCPOLARITY_HIGH;
        lcd_tim_oc_handle.OCNPolarity = TIM_OCNPOLARITY_HIGH;
        lcd_tim_oc_handle.OCFastMode = TIM_OCFAST_DISABLE;
        lcd_tim_oc_handle.OCIdleState = TIM_OCIDLESTATE_RESET;
        lcd_tim_oc_handle.OCNIdleState = TIM_OCNIDLESTATE_RESET;

        HAL_TIM_PWM_Init(&display.htim);
        HAL_TIM_PWM_ConfigChannel(&display.htim, &lcd_tim_oc_handle, OMV_DISPLAY_BL_TIM_CHANNEL);
        HAL_TIM_PWM_Start(&display.htim, OMV_DISPLAY_BL_TIM_CHANNEL);
    } else if ((0 < self->intensity) && (self->intensity < 255) && ((intensity <= 0) || (255 <= intensity))) {
        HAL_TIM_PWM_Stop(&display.htim, OMV_DISPLAY_BL_TIM_CHANNEL);
        HAL_TIM_PWM_DeInit(&display.htim);
    } else if ((0 < self->intensity) && (self->intensity < 255) && (0 < intensity) && (intensity < 255)) {
        __HAL_TIM_SET_COMPARE(&display.htim, OMV_DISPLAY_BL_TIM_CHANNEL, (period * intensity) / 255);
    }
    #endif

    self->intensity = intensity;
}
#endif // OMV_DISPLAY_BL_PIN

static void display_deinit(py_display_obj_t *self) {
    HAL_LTDC_DeInit(&display.hltdc);
    HAL_NVIC_DisableIRQ(LTDC_IRQn);

    __HAL_RCC_PLL3_DISABLE();
    uint32_t tickstart = mp_hal_ticks_ms();
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLL3RDY)) {
        if ((mp_hal_ticks_ms() - tickstart) > PLL_TIMEOUT_VALUE) {
            break;
        }
    }

    fb_alloc_free_till_mark_past_mark_permanent();

    #ifdef OMV_DISPLAY_BL_PIN
    if (self->display_on) {
        // back to default state
        display_set_backlight(self, 0);
    }
    #endif // OMV_DISPLAY_BL_PIN
}

mp_obj_t display_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_framesize, ARG_refresh, ARG_display_on, ARG_triple_buffer, ARG_portrait };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_framesize,     MP_ARG_INT,  {.u_int = DISPLAY_RESOLUTION_FWVGA  } },
        { MP_QSTR_refresh,       MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = 60  } },
        { MP_QSTR_display_on,    MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = true} },
        { MP_QSTR_triple_buffer, MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = true} },
        { MP_QSTR_portrait,      MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = false} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if ((args[ARG_framesize].u_int < 0) || (args[ARG_framesize].u_int >= DISPLAY_RESOLUTION_MAX)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid Frame Size!"));
    }
    if ((args[ARG_refresh].u_int < 30) || (args[ARG_refresh].u_int > 120)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid Refresh Rate!"));
    }

    py_display_obj_t *self = (py_display_obj_t *) m_new_obj_with_finaliser(py_display_obj_t);
    self->base.type = &py_rgb_display_type;
    self->refresh = args[ARG_refresh].u_int;
    self->display_on = args[ARG_display_on].u_bool;
    self->bgr = false;
    self->triple_buffer = args[ARG_triple_buffer].u_bool;
    self->framebuffer_tail = 0;
    self->framebuffer_head = 0;
    self->framesize = args[ARG_framesize].u_int;
    if (args[ARG_portrait].u_bool) {
        self->width = display_modes[self->framesize].vactive;
        self->height = display_modes[self->framesize].hactive;
    } else {
        self->width = display_modes[self->framesize].hactive;
        self->height = display_modes[self->framesize].vactive;
    }

    // Store state to access it from IRQ handlers or callbacks
    display.self = self;
    
    // Configure PLL3 for the selected mode clock.
    pll_config(self->framesize, self->refresh);

    // Init LTDC controller
    ltdc_init(self);

    #ifdef OMV_DISPLAY_BL_PIN
    if (self->display_on) {
        display_set_backlight(self, 255); // to on state
    }
    #endif // OMV_DISPLAY_BL_PIN

    return MP_OBJ_FROM_PTR(self);
}

STATIC const py_display_p_t py_display_p = {
    .deinit = display_deinit,
    .clear = display_clear,
    .write = display_write,
    #ifdef OMV_DISPLAY_BL_PIN
    .set_backlight = display_set_backlight,
    #endif
};

MP_DEFINE_CONST_OBJ_TYPE(
    py_rgb_display_type,
    MP_QSTR_RGBDisplay,
    MP_TYPE_FLAG_NONE,
    make_new, display_make_new,
    protocol, &py_display_p,
    locals_dict, &py_display_locals_dict
    );
#endif // MICROPY_PY_DISPLAY
