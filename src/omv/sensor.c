/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer.
 *
 */
#include <stdlib.h>
#include <string.h>
#include "mp.h"
#include "irq.h"
#include "cambus.h"
#include "ov9650.h"
#include "ov2640.h"
#include "ov7725.h"
#include "mt9v034.h"
#include "sensor.h"
#include "systick.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

#define OV_CHIP_ID      (0x0A)
#define ON_CHIP_ID      (0x00)
#define MAX_XFER_SIZE (0xFFFC)

sensor_t sensor;
TIM_HandleTypeDef  TIMHandle;
DMA_HandleTypeDef  DMAHandle;
DCMI_HandleTypeDef DCMIHandle;

static int line = 0;
extern uint8_t _line_buf;

const int resolution[][2] = {
    // C/SIF Resolutions
    {88,   72  },    /* QQCIF     */
    {176,  144 },    /* QCIF      */
    {352,  288 },    /* CIF       */
    {88,   60  },    /* QQSIF     */
    {176,  120 },    /* QSIF      */
    {352,  240 },    /* SIF       */
    // VGA Resolutions
    {40,   30  },    /* QQQQVGA   */
    {80,   60  },    /* QQQVGA    */
    {160,  120 },    /* QQVGA     */
    {320,  240 },    /* QVGA      */
    {640,  480 },    /* VGA       */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x64    */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {800,  600 },    /* SVGA      */
    {1280, 1024},    /* SXGA      */
    {1600, 1200},    /* UXGA      */
};

#if (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
static int extclk_config(int frequency)
{
    // Doubles PCLK
    //__HAL_RCC_TIMCLKPRESCALER(RCC_TIMPRES_ACTIVATED);

    /* TCLK (PCLK * 2) */
    int tclk  = DCMI_TIM_PCLK_FREQ() * 2;

    /* Period should be even */
    int period = (tclk / frequency) - 1;

    /* Timer base configuration */
    TIMHandle.Instance          = DCMI_TIM;
    TIMHandle.Init.Period       = period;
    TIMHandle.Init.Prescaler    = 0;
    TIMHandle.Init.ClockDivision = 0;
    TIMHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;

    /* Timer channel configuration */
    TIM_OC_InitTypeDef TIMOCHandle;
    TIMOCHandle.Pulse       = period/2;
    TIMOCHandle.OCMode      = TIM_OCMODE_PWM1;
    TIMOCHandle.OCPolarity  = TIM_OCPOLARITY_HIGH;
    TIMOCHandle.OCFastMode  = TIM_OCFAST_DISABLE;
    TIMOCHandle.OCIdleState = TIM_OCIDLESTATE_RESET;

    if (HAL_TIM_PWM_Init(&TIMHandle) != HAL_OK
            || HAL_TIM_PWM_ConfigChannel(&TIMHandle, &TIMOCHandle, DCMI_TIM_CHANNEL) != HAL_OK
            || HAL_TIM_PWM_Start(&TIMHandle, DCMI_TIM_CHANNEL) != HAL_OK) {
        // Initialization Error
        return -1;
    }

    return 0;
}
#endif // (OMV_XCLK_SOURCE == OMV_XCLK_TIM)

static int dcmi_config(uint32_t jpeg_mode)
{
    // DCMI configuration
    DCMIHandle.Instance         = DCMI;
    // VSYNC clock polarity
    DCMIHandle.Init.VSPolarity  = SENSOR_HW_FLAGS_GET(&sensor, SENSOR_HW_FLAGS_VSYNC) ?
                                    DCMI_VSPOLARITY_HIGH : DCMI_VSPOLARITY_LOW;
    // HSYNC clock polarity
    DCMIHandle.Init.HSPolarity  = SENSOR_HW_FLAGS_GET(&sensor, SENSOR_HW_FLAGS_HSYNC) ?
                                    DCMI_HSPOLARITY_HIGH : DCMI_HSPOLARITY_LOW;
    // PXCLK clock polarity
    DCMIHandle.Init.PCKPolarity = SENSOR_HW_FLAGS_GET(&sensor, SENSOR_HW_FLAGS_PIXCK) ?
                                    DCMI_PCKPOLARITY_RISING : DCMI_PCKPOLARITY_FALLING;

    DCMIHandle.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;    // Enable Hardware synchronization
    DCMIHandle.Init.CaptureRate = DCMI_CR_ALL_FRAME;        // Capture rate all frames
    DCMIHandle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B; // Capture 8 bits on every pixel clock
    DCMIHandle.Init.JPEGMode = jpeg_mode;                   // Set JPEG Mode
    #if defined(STM32F765xx) || defined(STM32F769xx)
    DCMIHandle.Init.ByteSelectMode  = DCMI_BSM_ALL;         // Capture all received bytes
    DCMIHandle.Init.ByteSelectStart = DCMI_OEBS_ODD;        // Ignored
    DCMIHandle.Init.LineSelectMode  = DCMI_LSM_ALL;         // Capture all received lines
    DCMIHandle.Init.LineSelectStart = DCMI_OELS_ODD;        // Ignored
    #endif

    // Associate the DMA handle to the DCMI handle
    __HAL_LINKDMA(&DCMIHandle, DMA_Handle, DMAHandle);

   // Initialize the DCMI
    HAL_DCMI_DeInit(&DCMIHandle);
    if (HAL_DCMI_Init(&DCMIHandle) != HAL_OK) {
        // Initialization Error
        return -1;
    }

    // Configure and enable DCMI IRQ Channel
    HAL_NVIC_SetPriority(DCMI_IRQn, IRQ_PRI_DCMI, IRQ_SUBPRI_DCMI);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);
    return 0;
}

static int dma_config()
{
    // DMA Stream configuration
    DMAHandle.Instance              = DMA2_Stream1;             /* Select the DMA instance          */
    DMAHandle.Init.Channel          = DMA_CHANNEL_1;            /* DMA Channel                      */
    DMAHandle.Init.Direction        = DMA_PERIPH_TO_MEMORY;     /* Peripheral to memory transfer    */
    DMAHandle.Init.MemInc           = DMA_MINC_ENABLE;          /* Memory increment mode Enable     */
    DMAHandle.Init.PeriphInc        = DMA_PINC_DISABLE;         /* Peripheral increment mode Enable */
    DMAHandle.Init.PeriphDataAlignment  = DMA_PDATAALIGN_WORD;  /* Peripheral data alignment : Word */
    DMAHandle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;      /* Memory data alignment : Word     */
    DMAHandle.Init.Mode             = DMA_NORMAL;               /* Normal DMA mode                  */
    DMAHandle.Init.Priority         = DMA_PRIORITY_HIGH;        /* Priority level : high            */
    DMAHandle.Init.FIFOMode         = DMA_FIFOMODE_ENABLE;      /* FIFO mode enabled                */
    DMAHandle.Init.FIFOThreshold    = DMA_FIFO_THRESHOLD_FULL;  /* FIFO threshold full              */
    DMAHandle.Init.MemBurst         = DMA_MBURST_INC4;          /* Memory burst                     */
    DMAHandle.Init.PeriphBurst      = DMA_PBURST_SINGLE;        /* Peripheral burst                 */

    // Configure and disable DMA IRQ Channel
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, IRQ_PRI_DMA21, IRQ_SUBPRI_DMA21);
    HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);

    // Initialize the DMA stream
    HAL_DMA_DeInit(&DMAHandle);
    if (HAL_DMA_Init(&DMAHandle) != HAL_OK) {
        // Initialization Error
        return -1;
    }

    return 0;
}

void sensor_init0()
{
    // Init FB mutex
    mutex_init(&JPEG_FB()->lock);

    // Save fb_enabled flag state
    int fb_enabled = JPEG_FB()->enabled;

    // Clear framebuffers
    memset(MAIN_FB(), 0, sizeof(*MAIN_FB()));
    memset(JPEG_FB(), 0, sizeof(*JPEG_FB()));

    // Set default quality
    JPEG_FB()->quality = 35;

    // Set fb_enabled
    JPEG_FB()->enabled = fb_enabled;
}

int sensor_init()
{
    /* Do a power cycle */
    DCMI_PWDN_HIGH();
    systick_sleep(10);

    DCMI_PWDN_LOW();
    systick_sleep(10);

    // Initialize the camera bus.
    cambus_init();
    systick_sleep(10);

    // Configure the sensor external clock (XCLK) to XCLK_FREQ.
    //
    // Max pixclk is 2.5 * HCLK:
    //  STM32F427@180MHz PCLK = 71.9999MHz
    //  STM32F769@216MHz PCLK = 86.4000MHz
    //
    // OV2640:
    //  The sensor's internal PLL (when CLKRC=0x80) doubles the XCLK_FREQ
    //  (XCLK=XCLK_FREQ*2), and the unscaled PIXCLK output is XCLK_FREQ*4
    //
    // OV7725 PCLK when prescalar is enabled (CLKRC[6]=0):
    //  Internal clock = Input clock × PLL multiplier / [(CLKRC[5:0] + 1) × 2]
    //
    // OV7725 PCLK when prescalar is disabled (CLKRC[6]=1):
    //  Internal clock = Input clock × PLL multiplier
    //
    #if (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
    // Configure external clock timer.
    if (extclk_config(OMV_XCLK_FREQUENCY) != 0) {
        // Timer problem
        return -1;
    }
    #elif (OMV_XCLK_SOURCE == OMV_XCLK_MCO)
    // Pass through the MCO1 clock with source input set to HSE (12MHz).
    // Note MCO1 is multiplexed on OPENMV2/TIM1 only.
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
    #else
    #error "OMV_XCLK_SOURCE is not set!"
    #endif

    /* Reset the sesnor state */
    memset(&sensor, 0, sizeof(sensor_t));

    /* Some sensors have different reset polarities, and we can't know which sensor
       is connected before initializing cambus and probing the sensor, which in turn
       requires pulling the sensor out of the reset state. So we try to probe the
       sensor with both polarities to determine line state. */
    sensor.reset_pol = ACTIVE_HIGH;

    /* Reset the sensor */
    DCMI_RESET_HIGH();
    systick_sleep(10);

    DCMI_RESET_LOW();
    systick_sleep(10);

    /* Probe the sensor */
    sensor.slv_addr = cambus_scan();
    if (sensor.slv_addr == 0) {
        /* Sensor has been held in reset,
           so the reset line is active low */
        sensor.reset_pol = ACTIVE_LOW;

        /* Pull the sensor out of the reset state */
        DCMI_RESET_HIGH();
        systick_sleep(10);

        /* Probe again to set the slave addr */
        sensor.slv_addr = cambus_scan();
        if (sensor.slv_addr == 0)  {
            // Probe failed
            return -2;
        }
    }

    // Clear sensor chip ID.
    sensor.chip_id = 0;

    // Read ON semi sensor ID.
    cambus_readb(sensor.slv_addr, ON_CHIP_ID, &sensor.chip_id);
    if (sensor.chip_id == MT9V034_ID) {
        // On/Aptina MT requires 13-27MHz clock.
        extclk_config(27000000);
        // Only the MT9V034 is currently supported.
        mt9v034_init(&sensor);
    } else { // Read OV sensor ID.
        cambus_readb(sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
        // Initialize sensor struct.
        switch (sensor.chip_id) {
            case OV9650_ID:
                ov9650_init(&sensor);
                break;
            case OV2640_ID:
                ov2640_init(&sensor);
                break;
            case OV7725_ID:
                ov7725_init(&sensor);
                break;
            default:
                // Sensor is not supported.
                return -3;
        }
    }

    /* Configure the DCMI DMA Stream */
    if (dma_config() != 0) {
        // DMA problem
        return -4;
    }

    /* Configure the DCMI interface. This should be called
       after ovxxx_init to set VSYNC/HSYNC/PCLK polarities */
    if (dcmi_config(DCMI_JPEG_DISABLE) != 0){
        // DCMI config failed
        return -5;
    }

    // Disable VSYNC EXTI IRQ
    HAL_NVIC_DisableIRQ(DCMI_VSYNC_IRQN);

    // Clear fb_enabled flag
    // This is executed only once to initialize the FB enabled flag.
    JPEG_FB()->enabled = 0;

    /* All good! */
    return 0;
}

int sensor_reset()
{
    // Reset the sesnor state
    sensor.sde          = 0xFF;
    sensor.pixformat    = 0xFF;
    sensor.framesize    = 0xFF;
    sensor.framerate    = 0xFF;
    sensor.gainceiling  = 0xFF;
    sensor.vsync_gpio   = NULL;

    // Reset image filter
    sensor_set_line_filter(NULL, NULL);

    // Call sensor-specific reset function
    sensor.reset(&sensor);

    // Just in case there's a running DMA request.
    HAL_DMA_Abort(&DMAHandle);

    // Disable VSYNC EXTI IRQ
    HAL_NVIC_DisableIRQ(DCMI_VSYNC_IRQN);
    return 0;
}

int sensor_get_id()
{
    return sensor.chip_id;
}

int sensor_sleep(int enable)
{
    if (sensor.sleep == NULL
        || sensor.sleep(&sensor, enable) != 0) {
        // Operation not supported
        return -1;
    }
    return 0;
}

int sensor_read_reg(uint8_t reg_addr)
{
    if (sensor.read_reg == NULL) {
        // Operation not supported
        return -1;
    }
    return sensor.read_reg(&sensor, reg_addr);
}

int sensor_write_reg(uint8_t reg_addr, uint16_t reg_data)
{
    if (sensor.write_reg == NULL) {
        // Operation not supported
        return -1;
    }
    return sensor.write_reg(&sensor, reg_addr, reg_data);
}

int sensor_set_pixformat(pixformat_t pixformat)
{
    uint32_t jpeg_mode = DCMI_JPEG_DISABLE;

    if (sensor.pixformat == pixformat) {
        // No change
        return 0;
    }

    if (sensor.set_pixformat == NULL
        || sensor.set_pixformat(&sensor, pixformat) != 0) {
        // Operation not supported
        return -1;
    }

    // Set pixel format
    sensor.pixformat = pixformat;

    // Set JPEG mode
    if (pixformat == PIXFORMAT_JPEG) {
        jpeg_mode = DCMI_JPEG_ENABLE;
    }

    // Skip the first frame.
    MAIN_FB()->bpp = 0;

    return dcmi_config(jpeg_mode);
}

int sensor_set_framesize(framesize_t framesize)
{
    if (sensor.framesize == framesize) {
        // No change
        return 0;
    }

    // Call the sensor specific function
    if (sensor.set_framesize == NULL
        || sensor.set_framesize(&sensor, framesize) != 0) {
        // Operation not supported
        return -1;
    }

    // Set framebuffer size
    sensor.framesize = framesize;

    // Skip the first frame.
    MAIN_FB()->bpp = 0;
    MAIN_FB()->w = sensor.fb_w = resolution[framesize][0];
    MAIN_FB()->h = sensor.fb_h = resolution[framesize][1];
    HAL_DCMI_DisableCROP(&DCMIHandle);

    return 0;
}

int sensor_set_framerate(framerate_t framerate)
{
    if (sensor.framerate == framerate) {
       /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_framerate == NULL
        || sensor.set_framerate(&sensor, framerate) != 0) {
        /* operation not supported */
        return -1;
    }

    /* set the frame rate */
    sensor.framerate = framerate;

    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h)
{
    MAIN_FB()->w = sensor.fb_w = w;
    MAIN_FB()->h = sensor.fb_h = h;
    HAL_DCMI_ConfigCROP(&DCMIHandle, x*2, y, w*2-1, h-1);
    HAL_DCMI_EnableCROP(&DCMIHandle);
    return 0;
}

int sensor_set_contrast(int level)
{
    if (sensor.set_contrast != NULL) {
        return sensor.set_contrast(&sensor, level);
    }
    return -1;
}

int sensor_set_brightness(int level)
{
    if (sensor.set_brightness != NULL) {
        return sensor.set_brightness(&sensor, level);
    }
    return -1;
}

int sensor_set_saturation(int level)
{
    if (sensor.set_saturation != NULL) {
        return sensor.set_saturation(&sensor, level);
    }
    return -1;
}

int sensor_set_gainceiling(gainceiling_t gainceiling)
{
    if (sensor.gainceiling == gainceiling) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_gainceiling == NULL
        || sensor.set_gainceiling(&sensor, gainceiling) != 0) {
        /* operation not supported */
        return -1;
    }

    sensor.gainceiling = gainceiling;
    return 0;
}

int sensor_set_quality(int qs)
{
    /* call the sensor specific function */
    if (sensor.set_quality == NULL
        || sensor.set_quality(&sensor, qs) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_colorbar(int enable)
{
    /* call the sensor specific function */
    if (sensor.set_colorbar == NULL
        || sensor.set_colorbar(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_gain(int enable, float gain_db, float gain_db_ceiling)
{
    /* call the sensor specific function */
    if (sensor.set_auto_gain == NULL
        || sensor.set_auto_gain(&sensor, enable, gain_db, gain_db_ceiling) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_gain_db(float *gain_db)
{
    /* call the sensor specific function */
    if (sensor.get_gain_db == NULL
        || sensor.get_gain_db(&sensor, gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_exposure(int enable, int exposure_us)
{
    /* call the sensor specific function */
    if (sensor.set_auto_exposure == NULL
        || sensor.set_auto_exposure(&sensor, enable, exposure_us) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_exposure_us(int *exposure_us)
{
    /* call the sensor specific function */
    if (sensor.get_exposure_us == NULL
        || sensor.get_exposure_us(&sensor, exposure_us) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    /* call the sensor specific function */
    if (sensor.set_auto_whitebal == NULL
        || sensor.set_auto_whitebal(&sensor, enable, r_gain_db, g_gain_db, b_gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    /* call the sensor specific function */
    if (sensor.get_rgb_gain_db == NULL
        || sensor.get_rgb_gain_db(&sensor, r_gain_db, g_gain_db, b_gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_hmirror(int enable)
{
    /* call the sensor specific function */
    if (sensor.set_hmirror == NULL
        || sensor.set_hmirror(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_vflip(int enable)
{
    /* call the sensor specific function */
    if (sensor.set_vflip == NULL
        || sensor.set_vflip(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_special_effect(sde_t sde)
{
    if (sensor.sde == sde) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_special_effect == NULL
        || sensor.set_special_effect(&sensor, sde) != 0) {
        /* operation not supported */
        return -1;
    }

    sensor.sde = sde;
    return 0;
}

int sensor_set_lens_correction(int enable, int radi, int coef)
{
    /* call the sensor specific function */
    if (sensor.set_lens_correction == NULL
        || sensor.set_lens_correction(&sensor, enable, radi, coef) != 0) {
        /* operation not supported */
        return -1;
    }

    return 0;
}

int sensor_set_line_filter(line_filter_t line_filter_func, void *line_filter_args)
{
    // Set line pre-processing function and args
    sensor.line_filter_func = line_filter_func;
    sensor.line_filter_args = line_filter_args;
    return 0;
}

int sensor_set_vsync_output(GPIO_TypeDef *gpio, uint32_t pin)
{
    sensor.vsync_pin  = pin;
    sensor.vsync_gpio = gpio;
    // Enable VSYNC EXTI IRQ
    HAL_NVIC_SetPriority(DCMI_VSYNC_IRQN, IRQ_PRI_EXTINT, IRQ_SUBPRI_EXTINT);
    HAL_NVIC_EnableIRQ(DCMI_VSYNC_IRQN);
    return 0;
}

void DCMI_VsyncExtiCallback()
{
    __HAL_GPIO_EXTI_CLEAR_FLAG(1 << DCMI_VSYNC_IRQ_LINE);
    if (sensor.vsync_gpio != NULL) {
        HAL_GPIO_WritePin(sensor.vsync_gpio, sensor.vsync_pin,
                !HAL_GPIO_ReadPin(DCMI_VSYNC_PORT, DCMI_VSYNC_PIN));
    }
}

// This function is called back after each line transfer is complete,
// with a pointer to the line buffer that was used. At this point the
// DMA transfers the next line to the other half of the line buffer.
// Note:  For JPEG this function is called once (and ignored) at the end of the transfer.
void DCMI_DMAConvCpltUser(uint32_t addr)
{
    uint8_t *src = (uint8_t*) addr;
    uint8_t *dst = MAIN_FB()->pixels;

    if (sensor.line_filter_func && sensor.line_filter_args) {
        int bpp = ((sensor.pixformat == PIXFORMAT_GRAYSCALE) ? 1:2);
        dst += line++ * MAIN_FB()->w * bpp;
        // If there's an image filter installed call it.
        // Note: BPP is the target BPP, not the line bpp (the line is always 2 bytes per pixel) if the target BPP is 1
        // it means the image currently being read is going to be Grayscale, and the function needs to output w * 1BPP.
        sensor.line_filter_func(src, MAIN_FB()->w * 2 , dst, MAIN_FB()->w * bpp, sensor.line_filter_args);
    } else {
        switch (sensor.pixformat) {
            case PIXFORMAT_BAYER:
                dst += line++ * MAIN_FB()->w;
                for (int i=0; i<MAIN_FB()->w; i++) {
                    dst[i] = src[i];
                }
                break;
            case PIXFORMAT_GRAYSCALE:
                dst += line++ * MAIN_FB()->w;
                if (sensor.gs_bpp == 1) {
                    // 1BPP GRAYSCALE.
                    for (int i=0; i<MAIN_FB()->w; i++) {
                        dst[i] = src[i];
                    }
                } else {
                    // Extract Y channel from YUV.
                    for (int i=0; i<MAIN_FB()->w; i++) {
                        dst[i] = src[i<<1];
                    }
                }
                break;
            case PIXFORMAT_YUV422:
            case PIXFORMAT_RGB565:
                dst += line++ * MAIN_FB()->w * 2;
                for (int i=0; i<MAIN_FB()->w * 2; i++) {
                    dst[i] = src[i];
                }
                break;
            case PIXFORMAT_JPEG:
                break;
            default:
                break;
        }
    }
}

static void sensor_check_bufsize()
{
    int bpp=0;
    switch (sensor.pixformat) {
        case PIXFORMAT_BAYER:
        case PIXFORMAT_GRAYSCALE:
            bpp = 1;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_RGB565:
            bpp = 2;
            break;
        default:
            break;
    }

    if ((MAIN_FB()->w * MAIN_FB()->h * bpp) > OMV_RAW_BUF_SIZE) {
        if (sensor.pixformat == PIXFORMAT_GRAYSCALE) {
            // Crop higher GS resolutions to QVGA
            sensor_set_windowing(190, 120, 320, 240);
        } else if (sensor.pixformat == PIXFORMAT_RGB565) {
            // Switch to BAYER if the frame is too big to fit in RAM.
            sensor_set_pixformat(PIXFORMAT_BAYER);
        }
    }

}

// The JPEG offset allows JPEG compression of the framebuffer without overwriting the pixels.
// The offset size may need to be adjusted depending on the quality, otherwise JPEG data may
// overwrite image pixels before they are compressed.
int sensor_snapshot(image_t *image, line_filter_t line_filter_func, void *line_filter_args)
{
    uint32_t addr, length, tick_start;

    // Set line filter
    sensor_set_line_filter(line_filter_func, line_filter_args);

    // Compress the framebuffer for the IDE preview, only if it's not the first frame,
    // the framebuffer is enabled and the image sensor does not support JPEG encoding.
    // Note: This doesn't run unless the IDE is connected and the framebuffer is enabled.
    fb_update_jpeg_buffer();

    // The user may have changed the MAIN_FB width or height on the last image so we need
    // to restore that here. We don't have to restore bpp because that's taken care of
    // already in the code below. Note that we do the JPEG compression above first to save
    // the FB of whatever the user set it to and now we restore.
    MAIN_FB()->w = sensor.fb_w;
    MAIN_FB()->h = sensor.fb_h;

    // Make sure the raw frame fits FB. If it doesn't it will be cropped
    // for GS, or the sensor pixel format will be swicthed to bayer for RGB.
    sensor_check_bufsize();

    // Setup the size and address of the transfer
    switch (sensor.pixformat) {
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            // RGB/YUV read 2 bytes per pixel.
            length = (MAIN_FB()->w * MAIN_FB()->h * 2)/4;
            addr = (uint32_t) &_line_buf;
            break;
        case PIXFORMAT_BAYER:
            // BAYER/RAW: 1 byte per pixel
            length = (MAIN_FB()->w * MAIN_FB()->h * 1)/4;
            addr = (uint32_t) &_line_buf;
            break;
        case PIXFORMAT_GRAYSCALE:
            // 1/2BPP Grayscale.
            length = (MAIN_FB()->w * MAIN_FB()->h * sensor.gs_bpp)/4;
            addr = (uint32_t) &_line_buf;
            break;
        case PIXFORMAT_JPEG:
            // Sensor has hardware JPEG set max frame size.
            length = MAX_XFER_SIZE;
            addr = (uint32_t) (MAIN_FB()->pixels);
            break;
        default:
            return -1;
    }

    // Clear line counter
    line = 0;

    // Snapshot start tick
    tick_start = HAL_GetTick();

    // Enable DMA IRQ
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        // Start a regular transfer
        HAL_DCMI_Start_DMA(&DCMIHandle,
                DCMI_MODE_SNAPSHOT, addr, length);
    } else {
        // Start a multibuffer transfer (line by line)
        HAL_DCMI_Start_DMA_MB(&DCMIHandle,
                DCMI_MODE_SNAPSHOT, addr, length, MAIN_FB()->h);
    }

    // Wait for frame
    while ((DCMI->CR & DCMI_CR_CAPTURE) != 0) {
        // Wait for interrupt
        __WFI();

        if ((HAL_GetTick() - tick_start) >= 3000) {
            // Sensor timeout, most likely a HW issue.
            // Abort the DMA request.
            HAL_DMA_Abort(&DMAHandle);
            return -1;
        }
    }

    // Abort DMA transfer.
    // Note: In JPEG mode the DMA will still be waiting for data since
    // the max frame size is set, so we need to abort the DMA transfer.
    HAL_DMA_Abort(&DMAHandle);

    // Disable DMA IRQ
    HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);

    // Fix the BPP
    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
            MAIN_FB()->bpp = 1;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_RGB565:
            MAIN_FB()->bpp = 2;
            break;
        case PIXFORMAT_BAYER:
            MAIN_FB()->bpp = 3;
            break;
        case PIXFORMAT_JPEG:
            // Read the number of data items transferred
            MAIN_FB()->bpp = (MAX_XFER_SIZE - DMAHandle.Instance->NDTR)*4;
            break;
    }

    // Set the user image.
    if (image != NULL) {
        image->w = MAIN_FB()->w;
        image->h = MAIN_FB()->h;
        image->bpp = MAIN_FB()->bpp;
        image->pixels = MAIN_FB()->pixels;
    }

    return 0;
}
