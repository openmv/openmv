/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer.
 */
#include <stdlib.h>
#include <string.h>
#include "mp.h"
#include "irq.h"
#include "cambus.h"
#include "ov2640.h"
#include "ov5640.h"
#include "ov7725.h"
#include "ov7690.h"
#include "ov9650.h"
#include "mt9v034.h"
#include "lepton.h"
#include "hm01b0.h"
#include "sensor.h"
#include "systick.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

#define MAX_XFER_SIZE   (0xFFFF*4)

extern void __fatal_error(const char *msg);

sensor_t           sensor     = {0};
TIM_HandleTypeDef  TIMHandle  = {0};
DMA_HandleTypeDef  DMAHandle  = {0};
DCMI_HandleTypeDef DCMIHandle = {0};

extern uint8_t _line_buf;
static uint8_t *dest_fb = NULL;
static volatile int offset = 0;
static volatile bool jpeg_buffer_overflow = false;
static volatile bool waiting_for_data = false;

const int resolution[][2] = {
    {0,    0   },
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
    {128,  128 },    /* 128x128   */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {720,  480 },    /* WVGA      */
    {752,  480 },    /* WVGA2     */
    {800,  600 },    /* SVGA      */
    {1024, 768 },    /* XGA       */
    {1280, 1024},    /* SXGA      */
    {1600, 1200},    /* UXGA      */
    {1280, 720 },    /* HD        */
    {1920, 1080},    /* FHD       */
    {2560, 1440},    /* QHD       */
    {2048, 1536},    /* QXGA      */
    {2560, 1600},    /* WQXGA     */
    {2592, 1944},    /* WQXGA2    */
};

static int extclk_config(int frequency)
{
    #if (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
    /* TCLK (PCLK * 2) */
    int tclk = DCMI_TIM_PCLK_FREQ() * 2;

    /* Period should be even */
    int period = (tclk / frequency) - 1;

    if (TIMHandle.Init.Period && (TIMHandle.Init.Period != period)) {
        // __HAL_TIM_SET_AUTORELOAD sets TIMHandle.Init.Period...
        __HAL_TIM_SET_AUTORELOAD(&TIMHandle, period);
        __HAL_TIM_SET_COMPARE(&TIMHandle, DCMI_TIM_CHANNEL, period / 2);
        return 0;
    }

    /* Timer base configuration */
    TIMHandle.Instance           = DCMI_TIM;
    TIMHandle.Init.Period        = period;
    TIMHandle.Init.Prescaler     = TIM_ETRPRESCALER_DIV1;
    TIMHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    TIMHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    /* Timer channel configuration */
    TIM_OC_InitTypeDef TIMOCHandle;
    TIMOCHandle.Pulse       = period / 2;
    TIMOCHandle.OCMode      = TIM_OCMODE_PWM1;
    TIMOCHandle.OCPolarity  = TIM_OCPOLARITY_HIGH;
    TIMOCHandle.OCFastMode  = TIM_OCFAST_DISABLE;
    TIMOCHandle.OCIdleState = TIM_OCIDLESTATE_RESET;

    if ((HAL_TIM_PWM_Init(&TIMHandle) != HAL_OK)
    || (HAL_TIM_PWM_ConfigChannel(&TIMHandle, &TIMOCHandle, DCMI_TIM_CHANNEL) != HAL_OK)
    || (HAL_TIM_PWM_Start(&TIMHandle, DCMI_TIM_CHANNEL) != HAL_OK)) {
        return -1;
    }
    #endif // (OMV_XCLK_SOURCE == OMV_XCLK_TIM)

    return 0;
}

static int dma_config()
{
    // DMA Stream configuration
    DMAHandle.Instance                  = DMA2_Stream1;             /* Select the DMA instance          */
    #if defined(MCU_SERIES_H7)
    DMAHandle.Init.Request              = DMA_REQUEST_DCMI;         /* DMA Channel                      */
    #else
    DMAHandle.Init.Channel              = DMA_CHANNEL_1;            /* DMA Channel                      */
    #endif
    DMAHandle.Init.Direction            = DMA_PERIPH_TO_MEMORY;     /* Peripheral to memory transfer    */
    DMAHandle.Init.MemInc               = DMA_MINC_ENABLE;          /* Memory increment mode Enable     */
    DMAHandle.Init.PeriphInc            = DMA_PINC_DISABLE;         /* Peripheral increment mode Enable */
    DMAHandle.Init.PeriphDataAlignment  = DMA_PDATAALIGN_WORD;      /* Peripheral data alignment : Word */
    DMAHandle.Init.MemDataAlignment     = DMA_MDATAALIGN_WORD;      /* Memory data alignment : Word     */
    DMAHandle.Init.Mode                 = DMA_NORMAL;               /* Normal DMA mode                  */
    DMAHandle.Init.Priority             = DMA_PRIORITY_HIGH;        /* Priority level : high            */
    DMAHandle.Init.FIFOMode             = DMA_FIFOMODE_ENABLE;      /* FIFO mode enabled                */
    DMAHandle.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;  /* FIFO threshold full              */
    DMAHandle.Init.MemBurst             = DMA_MBURST_INC4;          /* Memory burst                     */
    DMAHandle.Init.PeriphBurst          = DMA_PBURST_SINGLE;        /* Peripheral burst                 */

    // Initialize the DMA stream
    HAL_DMA_DeInit(&DMAHandle);
    if (HAL_DMA_Init(&DMAHandle) != HAL_OK) {
        // Initialization Error
        return -1;
    }

    // Configure and enable DMA IRQ Channel
    NVIC_SetPriority(DMA2_Stream1_IRQn, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
    return 0;
}

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
    // Setup capture parameters.
    DCMIHandle.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;    // Enable Hardware synchronization
    DCMIHandle.Init.CaptureRate = DCMI_CR_ALL_FRAME;        // Capture rate all frames
    DCMIHandle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B; // Capture 8 bits on every pixel clock
    DCMIHandle.Init.JPEGMode = jpeg_mode;                   // Set JPEG Mode
    #if defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
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
    NVIC_SetPriority(DCMI_IRQn, IRQ_PRI_DCMI);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);
    return 0;
}

static void dcmi_abort()
{
    // This stops the DCMI hardware from generating DMA requests immediately and then stops the DMA
    // hardware. Note that HAL_DMA_Abort is a blocking operation. Do not use this in an interrupt.

    if (DCMI->CR & DCMI_CR_ENABLE) {
        DCMI->CR &= ~DCMI_CR_ENABLE;
        HAL_DMA_Abort(&DMAHandle);
    }
}

// Returns true if a crop is being applied to the frame buffer.
static bool cropped()
{
    return MAIN_FB()->x // needs to be zero if not being cropped.
        || MAIN_FB()->y // needs to be zero if not being cropped.
        || (MAIN_FB()->u != resolution[sensor.framesize][0])  // should be equal to the resolution if not cropped.
        || (MAIN_FB()->v != resolution[sensor.framesize][1]); // should be equal to the resolution if not cropped.
}

void sensor_init0()
{
    dcmi_abort();

    // Save fb_enabled flag state
    int fb_enabled = JPEG_FB()->enabled;

    // Clear framebuffers
    memset(MAIN_FB(), 0, sizeof(*MAIN_FB()));
    memset(JPEG_FB(), 0, sizeof(*JPEG_FB()));

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    // Enable streaming.
    MAIN_FB()->streaming_enabled = true; // controlled by the OpenMV Cam.

    // Set default quality
    JPEG_FB()->quality = ((JPEG_QUALITY_HIGH - JPEG_QUALITY_LOW) / 2) + JPEG_QUALITY_LOW;

    // Set fb_enabled
    JPEG_FB()->enabled = fb_enabled; // controlled by the IDE.
}

int sensor_init()
{
    int init_ret = 0;

    /* Do a power cycle */
    DCMI_PWDN_HIGH();
    systick_sleep(10);

    DCMI_PWDN_LOW();
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
    #elif (OMV_XCLK_SOURCE == OMV_XCLK_OSC)
    // An external oscillator is used for the sensor clock.
    // Nothing to do.
    #else
    #error "OMV_XCLK_SOURCE is not set!"
    #endif

    /* Reset the sesnor state */
    memset(&sensor, 0, sizeof(sensor_t));

    /* Some sensors have different reset polarities, and we can't know which sensor
       is connected before initializing cambus and probing the sensor, which in turn
       requires pulling the sensor out of the reset state. So we try to probe the
       sensor with both polarities to determine line state. */
    sensor.pwdn_pol = ACTIVE_HIGH;
    sensor.reset_pol = ACTIVE_HIGH;

    /* Reset the sensor */
    DCMI_RESET_HIGH();
    systick_sleep(10);

    DCMI_RESET_LOW();
    systick_sleep(10);

    // Initialize the camera bus.
    cambus_init(&sensor.i2c, SCCB_I2C, SCCB_TIMING);
    systick_sleep(10);

    /* Probe the sensor */
    sensor.slv_addr = cambus_scan(&sensor.i2c);
    if (sensor.slv_addr == 0) {
        /* Sensor has been held in reset,
           so the reset line is active low */
        sensor.reset_pol = ACTIVE_LOW;

        /* Pull the sensor out of the reset state */
        DCMI_RESET_HIGH();
        systick_sleep(10);

        /* Probe again to set the slave addr */
        sensor.slv_addr = cambus_scan(&sensor.i2c);
        if (sensor.slv_addr == 0) {
            sensor.pwdn_pol = ACTIVE_LOW;

            DCMI_PWDN_HIGH();
            systick_sleep(10);

            sensor.slv_addr = cambus_scan(&sensor.i2c);
            if (sensor.slv_addr == 0) {
                sensor.reset_pol = ACTIVE_HIGH;

                DCMI_RESET_LOW();
                systick_sleep(10);

                sensor.slv_addr = cambus_scan(&sensor.i2c);
                if (sensor.slv_addr == 0) {
                    return -2;
                }
            }
        }
    }

    // Clear sensor chip ID.
    sensor.chip_id = 0;

    // Set default snapshot function.
    sensor.snapshot = sensor_snapshot;

    switch (sensor.slv_addr) {
    case OV2640_SLV_ADDR:
        cambus_readb(&sensor.i2c, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
        break;
    case OV5640_SLV_ADDR:
        cambus_readb2(&sensor.i2c, sensor.slv_addr, OV5640_CHIP_ID, &sensor.chip_id);
        break;
    case OV7725_SLV_ADDR: // Same for OV7690.
        cambus_readb(&sensor.i2c, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
        break;
    case MT9V034_SLV_ADDR:
        cambus_readb(&sensor.i2c, sensor.slv_addr, ON_CHIP_ID, &sensor.chip_id);
        break;
    case LEPTON_SLV_ADDR:
        sensor.chip_id = LEPTON_ID;
        break;
    #if (OMV_ENABLE_HM01B0 == 1)
    case HM01B0_SLV_ADDR:
        cambus_readb2(&sensor.i2c, sensor.slv_addr, HIMAX_CHIP_ID, &sensor.chip_id);
        break;
    #endif //(OMV_ENABLE_HM01B0 == 1)
    default:
        return -3;
        break;
    }

    switch (sensor.chip_id) {
    case OV2640_ID:
        init_ret = ov2640_init(&sensor);
        break;
    case OV5640_ID:
        if (extclk_config(OV5640_XCLK_FREQ) != 0) {
            return -3;
        }
        init_ret = ov5640_init(&sensor);
        break;
    case OV7725_ID:
        init_ret = ov7725_init(&sensor);
        break;
    #if (OMV_ENABLE_OV7690 == 1)
    case OV7690_ID:
        if (extclk_config(OV7690_XCLK_FREQ) != 0) {
            return -3;
        }
        init_ret = ov7690_init(&sensor);
        break;
    #endif //(OMV_ENABLE_OV7690 == 1)
    case OV9650_ID:
        init_ret = ov9650_init(&sensor);
        break;
    case MT9V034_ID:
        if (extclk_config(MT9V034_XCLK_FREQ) != 0) {
            return -3;
        }
        init_ret = mt9v034_init(&sensor);
        break;
    case LEPTON_ID:
        if (extclk_config(LEPTON_XCLK_FREQ) != 0) {
            return -3;
        }
        init_ret = lepton_init(&sensor);
        break;
    #if (OMV_ENABLE_HM01B0 == 1)
    case HM01B0_ID:
        init_ret = hm01b0_init(&sensor);
        break;
    #endif //(OMV_ENABLE_HM01B0 == 1)
    default:
        return -3;
        break;
    }

    if (init_ret != 0 ) {
        // Sensor init failed.
        return -4;
    }

    /* Configure the DCMI DMA Stream */
    if (dma_config() != 0) {
        // DMA problem
        return -5;
    }

    /* Configure the DCMI interface. This should be called
       after ovxxx_init to set VSYNC/HSYNC/PCLK polarities */
    if (dcmi_config(DCMI_JPEG_DISABLE) != 0){
        // DCMI config failed
        return -6;
    }

    // Disable VSYNC EXTI IRQ
    HAL_NVIC_DisableIRQ(DCMI_VSYNC_IRQN);

    // Clear fb_enabled flag
    // This is executed only once to initialize the FB enabled flag.
    JPEG_FB()->enabled = 0;

    // Set default color palette.
    sensor.color_palette = rainbow_table;

    /* All good! */
    return 0;
}

int sensor_reset()
{
    dcmi_abort();

    // Reset the sensor state
    sensor.sde           = 0;
    sensor.pixformat     = 0;
    sensor.framesize     = 0;
    sensor.gainceiling   = 0;
    sensor.hmirror       = false;
    sensor.vflip         = false;
    sensor.transpose     = false;
    #if MICROPY_PY_IMU
    sensor.auto_rotation = sensor.chip_id == OV7690_ID;
    #else
    sensor.auto_rotation = false;
    #endif // MICROPY_PY_IMU
    sensor.vsync_gpio    = NULL;

    // Reset default color palette.
    sensor.color_palette = rainbow_table;

    // Restore shutdown state on reset.
    sensor_shutdown(false);

    // Call sensor-specific reset function
    if (sensor.reset(&sensor) != 0) {
        return -1;
    }

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
    dcmi_abort();

    if (sensor.sleep == NULL
        || sensor.sleep(&sensor, enable) != 0) {
        // Operation not supported
        return -1;
    }
    return 0;
}

int sensor_shutdown(int enable)
{
    dcmi_abort();

    if (enable) {
        DCMI_PWDN_HIGH();
    } else {
        DCMI_PWDN_LOW();
    }

    systick_sleep(10);
    return 0;
}

int sensor_read_reg(uint16_t reg_addr)
{
    if (sensor.read_reg == NULL) {
        // Operation not supported
        return -1;
    }
    return sensor.read_reg(&sensor, reg_addr);
}

int sensor_write_reg(uint16_t reg_addr, uint16_t reg_data)
{
    if (sensor.write_reg == NULL) {
        // Operation not supported
        return -1;
    }
    return sensor.write_reg(&sensor, reg_addr, reg_data);
}

int sensor_set_pixformat(pixformat_t pixformat)
{
    if (sensor.pixformat == pixformat) {
        // No change
        return 0;
    }

    // sensor_check_buffsize() will switch from PIXFORMAT_BAYER to PIXFORMAT_RGB565 to try to fit
    // the MAIN_FB() in RAM as a first step optimization. If the user tries to switch back to RGB565
    // and that would be bigger than the RAM buffer we would just switch back.
    //
    // So, just short-circuit doing any work.
    //
    // This code is explicitly here to allow users to set the resolution to RGB565 and have it
    // switch to BAYER only once even though they are setting the resolution to RGB565 repeatedly
    // in a loop. Only RGB565->BAYER has this problem and needs this fix because of sensor_check_buffsize().
    if ((sensor.pixformat == PIXFORMAT_BAYER)
    &&  (pixformat == PIXFORMAT_RGB565)
    &&  (MAIN_FB()->u * MAIN_FB()->v * 2 > OMV_RAW_BUF_SIZE)
    &&  (MAIN_FB()->u * MAIN_FB()->v * 1 <= OMV_RAW_BUF_SIZE)) {
        // No change
        return 0;
    }

    // Cropping and transposing (and thus auto rotation) don't work in JPEG mode.
    if ((pixformat == PIXFORMAT_JPEG) && (cropped() || sensor.transpose || sensor.auto_rotation)) {
        return -1;
    }

    dcmi_abort();

    if (sensor.set_pixformat == NULL
        || sensor.set_pixformat(&sensor, pixformat) != 0) {
        // Operation not supported
        return -1;
    }

    systick_sleep(100); // wait for the camera to settle

    // Set pixel format
    sensor.pixformat = pixformat;

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    // Change the JPEG mode.
    return dcmi_config((pixformat == PIXFORMAT_JPEG) ? DCMI_JPEG_ENABLE : DCMI_JPEG_DISABLE);
}

int sensor_set_framesize(framesize_t framesize)
{
    if (sensor.framesize == framesize) {
        // No change
        return 0;
    }

    dcmi_abort();

    // Call the sensor specific function
    if (sensor.set_framesize == NULL
        || sensor.set_framesize(&sensor, framesize) != 0) {
        // Operation not supported
        return -1;
    }

    systick_sleep(100); // wait for the camera to settle

    // Set framebuffer size
    sensor.framesize = framesize;

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    // Set MAIN FB x offset, y offset, width, height, backup width, and backup height.
    MAIN_FB()->x = 0;
    MAIN_FB()->y = 0;
    MAIN_FB()->w = MAIN_FB()->u = resolution[framesize][0];
    MAIN_FB()->h = MAIN_FB()->v = resolution[framesize][1];

    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h)
{
    // py_sensor_set_windowing ensures this the window is at least 8x8
    // and that it is fully inside the sensor output framesize window.
    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    // We force everything to be a multiple of 2 so that when you switch between
    // grayscale/rgb565/bayer/jpeg the frame doesn't need to move around for bayer to work.
    MAIN_FB()->x = (x / 2) * 2;
    MAIN_FB()->y = (y / 2) * 2;
    MAIN_FB()->w = MAIN_FB()->u = (w / 2) * 2;
    MAIN_FB()->h = MAIN_FB()->v = (h / 2) * 2;

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
    if (sensor.hmirror == ((bool) enable)) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_hmirror == NULL
        || sensor.set_hmirror(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    sensor.hmirror = enable;
    systick_sleep(100); // wait for the camera to settle
    return 0;
}

bool sensor_get_hmirror()
{
    return sensor.hmirror;
}

int sensor_set_vflip(int enable)
{
    if (sensor.vflip == ((bool) enable)) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_vflip == NULL
        || sensor.set_vflip(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    sensor.vflip = enable;
    systick_sleep(100); // wait for the camera to settle
    return 0;
}

bool sensor_get_vflip()
{
    return sensor.vflip;
}

int sensor_set_transpose(bool enable)
{
    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    sensor.transpose = enable;
    return 0;
}

bool sensor_get_transpose()
{
    return sensor.transpose;
}

int sensor_set_auto_rotation(bool enable)
{
    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    sensor.auto_rotation = enable;
    return 0;
}

bool sensor_get_auto_rotation()
{
    return sensor.auto_rotation;
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

int sensor_ioctl(int request, ... /* arg */)
{
    dcmi_abort();

    int ret = -1;

    if (sensor.ioctl != NULL) {
        va_list ap;
        va_start(ap, request);
        /* call the sensor specific function */
        ret = sensor.ioctl(&sensor, request, ap);
        va_end(ap);
    }

    return ret;
}

int sensor_set_vsync_output(GPIO_TypeDef *gpio, uint32_t pin)
{
    sensor.vsync_pin  = pin;
    sensor.vsync_gpio = gpio;
    // Enable VSYNC EXTI IRQ
    NVIC_SetPriority(DCMI_VSYNC_IRQN, IRQ_PRI_EXTINT);
    HAL_NVIC_EnableIRQ(DCMI_VSYNC_IRQN);
    return 0;
}

int sensor_set_color_palette(const uint16_t *color_palette)
{
    sensor.color_palette = color_palette;
    return 0;
}

const uint16_t *sensor_get_color_palette()
{
    return sensor.color_palette;
}

void DCMI_VsyncExtiCallback()
{
    __HAL_GPIO_EXTI_CLEAR_FLAG(1 << DCMI_VSYNC_IRQ_LINE);
    if (sensor.vsync_gpio != NULL) {
        HAL_GPIO_WritePin(sensor.vsync_gpio, sensor.vsync_pin,
                !HAL_GPIO_ReadPin(DCMI_VSYNC_PORT, DCMI_VSYNC_PIN));
    }
}

// To make the user experience better we automatically shrink the size of the MAIN_FB() to fit
// within the RAM we have onboard the system.
static void sensor_check_buffsize()
{
    uint32_t bpp;

    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER:
            bpp = 1;
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            bpp = 2;
            break;
        // If the pixformat is NULL/JPEG there we can't do anything to check if it fits before hand.
        default:
            return;
    }

    // MAIN_FB() fits, we are done.
    if ((MAIN_FB()->u * MAIN_FB()->v * bpp) <= OMV_RAW_BUF_SIZE) {
        return;
    }

    if (sensor.pixformat == PIXFORMAT_RGB565) {
        // Switch to bayer for the quick 2x savings.
        sensor_set_pixformat(PIXFORMAT_BAYER);
        bpp = 1;

        // MAIN_FB() fits, we are done (bpp is 1).
        if (MAIN_FB()->u * MAIN_FB()->v <= OMV_RAW_BUF_SIZE) {
            return;
        }
    }

    int window_w = MAIN_FB()->u;
    int window_h = MAIN_FB()->v;

    // We need to shrink the frame buffer. We can do this by cropping. So, we will subtract columns
    // and rows from the frame buffer until it fits within the frame buffer.
    int max = IM_MAX(window_w, window_h);
    int min = IM_MIN(window_w, window_h);
    float aspect_ratio = max / ((float) min);
    float r = aspect_ratio, best_r = r;
    int c = 1, best_c = c;
    float best_err = FLT_MAX;

    // Find the width/height ratio that's within 1% of the aspect ratio with a loop limit.
    for (int i = 100; i; i--) {
        float err = fast_fabsf(r - fast_roundf(r));

        if (err <= best_err) {
            best_err = err;
            best_r = r;
            best_c = c;
        }

        if (best_err <= 0.01f) {
            break;
        }

        r += aspect_ratio;
        c += 1;
    }

    // Select the larger geometry to map the aspect ratio to.
    int u_sub, v_sub;

    if (window_w > window_h) {
        u_sub = fast_roundf(best_r);
        v_sub = best_c;
    } else {
        u_sub = best_c;
        v_sub = fast_roundf(best_r);
    }

    // Crop the frame buffer while keeping the aspect ratio and keeping the width/height even.
    while (((MAIN_FB()->u * MAIN_FB()->v * bpp) > OMV_RAW_BUF_SIZE) || (MAIN_FB()->u % 2)  || (MAIN_FB()->v % 2)) {
        MAIN_FB()->u -= u_sub;
        MAIN_FB()->v -= v_sub;
    }

    // Center the new window using the previous offset and keep the offset even.
    MAIN_FB()->x += (window_w - MAIN_FB()->u) / 2;
    MAIN_FB()->y += (window_h - MAIN_FB()->v) / 2;
    if (MAIN_FB()->x % 2) MAIN_FB()->x -= 1;
    if (MAIN_FB()->y % 2) MAIN_FB()->y -= 1;
}

// ARM Cortex-M4/M7 Processors can access memory using unaligned 32-bit reads/writes.
void *unaligned_2_to_1_memcpy(void *dest, void *src, size_t n)
{
    uint32_t *dest32 = (uint32_t *) dest;
    uint32_t *src32 = (uint32_t *) src;

// TODO: Make this faster using only 32-bit aligned reads/writes with data shifting.
#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
    for (; n > 4; n -= 4) {
        uint32_t tmp1 = *src32++;
        uint32_t tmp2 = *src32++;
        *dest32++ = (tmp1 & 0xff) | ((tmp1 >> 8) & 0xff00) | ((tmp2 & 0xff) << 16) | ((tmp2 & 0xff0000) << 8);
    }
#endif

    uint8_t *dest8 = (uint8_t *) dest32;
    uint16_t *src16 = (uint16_t *) src32;

    for (; n > 0; n -= 1) {
        *dest8++ = *src16++;
    }

    return dest;
}

// ARM Cortex-M4/M7 Processors can access memory using unaligned 32-bit reads/writes.
void *unaligned_memcpy(void *dest, void *src, size_t n)
{
// TODO: Make this faster using only 32-bit aligned reads/writes with data shifting.
#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
    uint32_t *dest32 = (uint32_t *) dest;
    uint32_t *src32 = (uint32_t *) src;

    for (; n > 4; n -= 4) {
        *dest32++ = *src32++;
    }

    uint8_t *dest8 = (uint8_t *) dest32;
    uint8_t *src8 = (uint8_t *) src32;

    for (; n > 0; n -= 1) {
        *dest8++ = *src8++;
    }

    return dest;
#else
    return memcpy(dest, src, n);
#endif
}

// Stop allowing new data in on the end of the frame and let snapshot know that the frame has been
// received. Note that DCMI_DMAConvCpltUser() is called before DCMI_IT_FRAME is enabled by
// DCMI_DMAXferCplt() so this means that the last line of data is *always* transferred before
// waiting_for_data is set to false.
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    waiting_for_data = false;
}

// This function is called back after each line transfer is complete,
// with a pointer to the line buffer that was used. At this point the
// DMA transfers the next line to the other half of the line buffer.
void DCMI_DMAConvCpltUser(uint32_t addr)
{
    // If snapshot was not already waiting to receive data then we have missed this frame and have
    // to drop it. So, abort this and future transfers. Snapshot will restart the process.
    if (!waiting_for_data) {
        DCMI->CR &= ~DCMI_CR_ENABLE;
        HAL_DMA_Abort_IT(&DMAHandle); // Note: Use HAL_DMA_Abort_IT and not HAL_DMA_Abort inside an interrupt.
        return;
    }

    // We are transferring the image from the DCMI hardware to line buffers so that we have more
    // control to post process the image data before writing it to the frame buffer. This requires
    // more CPU, but, allows us to crop and rotate the image as the data is received.

    // Additionally, the line buffers act as very large fifos which hide SDRAM memory access times
    // on the OpenMV Cam H7 Plus. When SDRAM refreshes the row you are trying to write to the fifo
    // depth on the DCMI hardware and DMA hardware is not enough to prevent data loss.

    uint8_t *src = (uint8_t*) addr;
    uint8_t *dst = (uint8_t*) dest_fb;

    uint16_t *src16 = (uint16_t*) addr;
    uint16_t *dst16 = (uint16_t*) dest_fb;

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        if (sensor.chip_id == OV5640_ID) {
            // JPEG MODE 4:
            //
            // The width and height are fixed in each frame. The first two bytes are valid data
            // length in every line, followed by valid image data. Dummy data (0xFF) may be used as
            // padding at each line end if the current valid image data is less than the line width.
            //
            // In this mode `offset` holds the size of all jpeg data transferred.
            //
            // Note: We are using this mode for the OV5640 because it allows us to use the line
            // buffers to fifo the JPEG image data input so we can handle SDRAM refresh hiccups
            // that will cause data loss if we make the DMA hardware write directly to the FB.
            //
            uint16_t size = __REV16(*src16);
            // Prevent a buffer overflow when writing the jpeg data.
            if (offset + size > OMV_RAW_BUF_SIZE) {
                jpeg_buffer_overflow = true;
                return;
            }
            unaligned_memcpy(MAIN_FB()->pixels + offset, src16 + 1, size);
            offset += size;
        } else {
            // JPEG MODE 3:
            //
            // Compression data is transmitted with programmable width. The last line width maybe
            // different from the other line (there is no dummy data). In each frame, the line
            // number may be different.
            //
            // In this mode `offset` will be incremented by one after 262,140 Bytes have been
            // transferred. If 524,280 Bytes have been transferred line will be incremented again.
            // The DMA counter must be used to get the amount of data transferred between.
            //
            // Note: In this mode the JPEG image data is written directly to the frame buffer. This
            // is not optimal. However, it works okay for the OV2640 since the PCLK is much lower
            // than the OV5640 PCLK. The OV5640 drops data in this mode. Hence using mode 4 above.
            //
            offset += 1;
        }
        return;
    }

    // Implement per line, per pixel cropping, and image transposing (for image rotation) in
    // in software using the CPU to transfer the image from the line buffers to the frame buffer.
    if (offset >= MAIN_FB()->y && offset <= (MAIN_FB()->y + MAIN_FB()->h)) {
        if (!sensor.transpose) {
            switch (sensor.pixformat) {
                case PIXFORMAT_BAYER:
                    dst += (offset - MAIN_FB()->y) * MAIN_FB()->w;
                    src += MAIN_FB()->x;
                    unaligned_memcpy(dst, src, MAIN_FB()->w);
                    break;
                case PIXFORMAT_GRAYSCALE:
                    dst += (offset - MAIN_FB()->y) * MAIN_FB()->w;
                    if (sensor.gs_bpp == 1) {
                        // 1BPP GRAYSCALE.
                        src += MAIN_FB()->x;
                        unaligned_memcpy(dst, src, MAIN_FB()->w);
                    } else {
                        // Extract Y channel from YUV.
                        src16 += MAIN_FB()->x;
                        unaligned_2_to_1_memcpy(dst, src16, MAIN_FB()->w);
                    }
                    break;
                case PIXFORMAT_YUV422:
                case PIXFORMAT_RGB565:
                    dst16 += (offset - MAIN_FB()->y) * MAIN_FB()->w;
                    src16 += MAIN_FB()->x;
                    unaligned_memcpy(dst16, src16, MAIN_FB()->w * sizeof(uint16_t));
                    break;
                default:
                    break;
            }
        } else {
            switch (sensor.pixformat) {
                case PIXFORMAT_BAYER:
                    dst += offset - MAIN_FB()->y;
                    src += MAIN_FB()->x;
                    for (int i = MAIN_FB()->w, h = MAIN_FB()->h; i; i--) {
                        *dst = *src++;
                        dst += h;
                    }
                    break;
                case PIXFORMAT_GRAYSCALE:
                    dst += offset - MAIN_FB()->y;
                    if (sensor.gs_bpp == 1) {
                        src += MAIN_FB()->x;
                        // 1BPP GRAYSCALE.
                        for (int i = MAIN_FB()->w, h = MAIN_FB()->h; i; i--) {
                            *dst = *src++;
                            dst += h;
                        }
                    } else {
                        src16 += MAIN_FB()->x;
                        // Extract Y channel from YUV.
                        for (int i = MAIN_FB()->w, h = MAIN_FB()->h; i; i--) {
                            *dst = *src16++;
                            dst += h;
                        }
                    }
                    break;
                case PIXFORMAT_YUV422:
                case PIXFORMAT_RGB565:
                    dst16 += offset - MAIN_FB()->y;
                    src16 += MAIN_FB()->x;
                    for (int i = MAIN_FB()->w, h = MAIN_FB()->h; i; i--) {
                        *dst16 = *src16++;
                        dst16 += h;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    offset++;
}

// This is the default snapshot function, which can be replaced in sensor_init functions. This function
// uses the DCMI and DMA to capture frames and each line is processed in the DCMI_DMAConvCpltUser function.
int sensor_snapshot(sensor_t *sensor, image_t *image, streaming_cb_t streaming_cb)
{
    uint32_t frame = 0;
    bool streaming = (streaming_cb != NULL); // Streaming mode.
    bool doublebuf = false;
    uint32_t addr, length, tick_start;

    // In streaming mode the image pointer must be valid.
    if (streaming) {
        if (image == NULL) {
            return -1;
        }

        // Clear the first image in to not trigger the streaming_cb in double buffer mode.
        image->pixels = NULL;
    }

    // Compress the framebuffer for the IDE preview, only if it's not the first frame,
    // the framebuffer is enabled and the image sensor does not support JPEG encoding.
    // Note: This doesn't run unless the IDE is connected and the framebuffer is enabled.
    fb_update_jpeg_buffer();

    // Make sure the raw frame fits into the FB. It will be switched from RGB565 to BAYER
    // first to save space before being cropped until it fits.
    sensor_check_buffsize();

    // Set the current frame buffer target used in the DMA line callback
    // (DCMI_DMAConvCpltUser function), in both snapshot and streaming modes.
    dest_fb = MAIN_FB()->pixels;

    // The user may have changed the MAIN_FB width or height on the last image so we need
    // to restore that here. We don't have to restore bpp because that's taken care of
    // already in the code below. Note that we do the JPEG compression above first to save
    // the FB of whatever the user set it to and now we restore.
    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;

    // If an error occurs we should have a valid w/h and invalid bpp so that we leave the frame
    // buffer like how sensor_set_pixformat()/sensor_set_framesize() leave it.
    MAIN_FB()->bpp = -1;

    // We use the stored frame size to read the whole frame. Note that cropping is
    // done in the line function using the dimensions stored in MAIN_FB()->x,y,w,h.
    uint32_t w = resolution[sensor->framesize][0];
    uint32_t h = resolution[sensor->framesize][1];

    // Setup the size and address of the transfer
    switch (sensor->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            // 1/2BPP Grayscale.
            length = (w * h * sensor->gs_bpp);
            addr = (uint32_t) &_line_buf;
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            // RGB/YUV read 2 bytes per pixel.
            length = (w * h * 2);
            addr = (uint32_t) &_line_buf;
            break;
        case PIXFORMAT_BAYER:
            // BAYER/RAW: 1 byte per pixel
            length = (w * h * 1);
            addr = (uint32_t) &_line_buf;
            break;
        case PIXFORMAT_JPEG:
            if (sensor->chip_id == OV5640_ID) {
                // The JPEG image needs to be transferred to the line buffer.
                // There is no limit on the amount of data transferred.
                length = w * h;
                addr = (uint32_t) &_line_buf;
            } else {
                // The JPEG image will be directly transferred to the frame buffer.
                // The DCMI hardware can transfer up to 524,280‬ bytes.
                length = MAX_XFER_SIZE * 2;
                addr = (uint32_t) (MAIN_FB()->pixels);
            }
            break;
        default:
            return -2; // Error out if the pixformat is not set.
    }

    // Error out if the frame size wasn't set or the line width is larger than the camera line buffers.
    if ((!length) || (((length / h) > (OMV_LINE_BUF_SIZE / 2)) && (addr == ((uint32_t) &_line_buf)))) {
        return -3;
    }

    // If two frames fit in ram, use double buffering in streaming mode.
    doublebuf = ((length*2) <= OMV_RAW_BUF_SIZE);

    do {
        // Clear the offset counter variable before we allow more data to be received.
        offset = 0;

        // Clear jpeg error flag before we allow more data to be received.
        jpeg_buffer_overflow = false;

        // If DCMI_DMAConvCpltUser() happens before waiting_for_data = true; below then the
        // transfer is stopped and it will be re-enabled again right afterwards. We know the
        // transfer was stopped by checking DCMI_CR_ENABLE.

        waiting_for_data = true;

        // We will be in one of the following states now:
        // 1. No transfer is currently running right now and DCMI_CR_ENABLE is not set.
        // 2. A transfer is running and we are waiting for the data to be received.

        // We are not using DCMI_CR_CAPTURE because when this bit is cleared to stop the continuous transfer it does not actually go
        // low until the end of the frame (yes, you read that right). DCMI_CR_ENABLE stops the capture when cleared and stays low.
        //
        // When DCMI_CR_ENABLE is cleared during a DCMI transfer the hardware will automatically
        // wait for the start of the next frame when it's re-enabled again below. So, we do not
        // need to wait till there's no frame happening before enabling.
        if (!(DCMI->CR & DCMI_CR_ENABLE)) {
            // Note that HAL_DCMI_Start_DMA and HAL_DCMI_Start_DMA_MB are effectively the same
            // method. The only difference between them is how large the DMA transfer size gets
            // set at. For both of them DMA doesn't actually care how much data the DCMI hardware
            // generates. It's just trying to move fixed size DMA transfers from the DCMI hardware
            // to one memory address or another memory address. After transferring X bytes to one
            // address it will switch to the next address and transfer X bytes again. Both of these
            // methods set the addresses right after each other. So, effectively DMA is just writing
            // data to a circular buffer with an interrupt every time 1/2 of it is written.
            if ((sensor->pixformat == PIXFORMAT_JPEG) && (sensor->chip_id != OV5640_ID)) {
                // Start a transfer where the whole frame buffer is located where the DMA is writing
                // data to. We only use this for JPEG mode for the OV2640. Since we don't know the
                // line size of data being transferred we just examine how much data was transferred
                // once DMA hardware stalls waiting for data. Note that because we are writing
                // directly to the frame buffer we do not have the option of aborting the transfer
                // if we are not ready to move data from a line buffer to the frame buffer.
                HAL_DCMI_Start_DMA(&DCMIHandle,
                        DCMI_MODE_SNAPSHOT, addr, length/4);
                // In this mode the DMA hardware is just treating the frame buffer as two large
                // DMA buffers. At the end of the frame less data may be transferred than requested.
            } else {
                // Start a multibuffer transfer (line by line). The DMA hardware will ping-pong
                // transferring data between the uncached line buffers. Since data is continuously
                // being captured the ping-ponging will stop at the end of the frame and then
                // continue when the next frame starts.
                HAL_DCMI_Start_DMA_MB(&DCMIHandle,
                        DCMI_MODE_CONTINUOUS, addr, length/4, h);
            }
        }

        // Let the camera know we want to trigger it now.
        #if defined(DCMI_FSYNC_PIN)
        if (SENSOR_HW_FLAGS_GET(sensor, SENSOR_HW_FLAGS_FSYNC)) {
            DCMI_FSYNC_HIGH();
        }
        #endif

        // DCMI_DMAConvCpltUser() will start triggering now. Since waiting_for_data = true; the
        // data will be transferred to the frame buffer.

        // Before we wait for the next frame try to get some work done. If we are in double buffer
        // mode then we can start processing the previous image buffer.
        if (streaming_cb && doublebuf && image->pixels != NULL) {
            // Call streaming callback function with previous frame.
            // Note: Image pointer should Not be NULL in streaming mode.
            streaming = streaming_cb(image);
        }

        // In camera sensor JPEG mode 4 we will not necessarily see every line in the frame and
        // in camera sensor JPEG mode 3 we will definitely not see every line in the frame. Given
        // this, we need to enable the end of frame interrupt before we have necessarily
        // finished transferring all JEPG data. This works as long as the end of the frame comes
        // much later after all JPEG data has been transferred. If this is violated the JPEG image
        // will be corrupted.
        if (DCMI->CR & DCMI_JPEG_ENABLE) {
            __HAL_DCMI_ENABLE_IT(&DCMIHandle, DCMI_IT_FRAME);
        }

        // Wait for the frame data. __WFI() below will exit right on time because of DCMI_IT_FRAME.
        // While waiting SysTick will trigger allowing us to timeout.
        for (tick_start = HAL_GetTick(); waiting_for_data; ) {
            __WFI();

            // If we haven't exited this loop before the timeout then we need to abort the transfer.
            if ((HAL_GetTick() - tick_start) >= 3000) {
                waiting_for_data = false;
                dcmi_abort();

                #if defined(DCMI_FSYNC_PIN)
                if (SENSOR_HW_FLAGS_GET(sensor, SENSOR_HW_FLAGS_FSYNC)) {
                    DCMI_FSYNC_LOW();
                }
                #endif

                return -4;
            }
        }

        // We have to abort the JPEG data transfer since it will be stuck waiting for data.
        // line will contain how many transfers we completed.
        // The DMA counter must be used to get the number of remaining words to be transferred.
        if ((sensor->pixformat == PIXFORMAT_JPEG) && (sensor->chip_id != OV5640_ID)) {
            dcmi_abort();
        }

        // We're done receiving data.
        #if defined(DCMI_FSYNC_PIN)
        if (SENSOR_HW_FLAGS_GET(sensor, SENSOR_HW_FLAGS_FSYNC)) {
            DCMI_FSYNC_LOW();
        }
        #endif

        // The JPEG in the frame buffer is actually invalid.
        if (jpeg_buffer_overflow) {
            return -5;
        }

        // After the above loop we have received all data in the frame. The DCMI hardware is left
        // running to look for the start of the next frame which it needs to sync to to capture
        // data. If it misses the start of the frame then the DCMI hardware will not capture that
        // frame. Assuming our processing is fast enough to start waiting for data again before
        // DCMI_DMAConvCpltUser() is called we can receive the next frame. If we are not fast
        // enough DCMI_DMAConvCpltUser() will automatically abort the transfer on being called.
        //
        // In the case of the OV2640 in JPEG mode since we are writing to the main FB we do not
        // put the DCMI hardware into continuous mode. So, we will drop frames more easily in that
        // mode and may be able to only achieve 1/2 the max FPS.

        //
        // Next, prepare the frame buffer w/h/bpp values given the image type.
        //

        // Fix resolution if transposed.
        if (sensor->transpose) {
            MAIN_FB()->w = MAIN_FB()->v; // v==h -> w
            MAIN_FB()->h = MAIN_FB()->u; // u==w -> h
        }

        // Fix the BPP.
        switch (sensor->pixformat) {
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
                if (sensor->chip_id == OV5640_ID) {
                    // Offset contains the sum of all the bytes transferred from the offset buffers
                    // while in DCMI_DMAConvCpltUser().
                    MAIN_FB()->bpp = offset;
                } else {
                    // Offset contains the number of MAX_XFER_SIZE transfers completed. To get the number of bytes transferred
                    // within a transfer we have to look at the DMA counter and see how much data was moved.
                    MAIN_FB()->bpp = (offset * MAX_XFER_SIZE) + ((MAX_XFER_SIZE/4) - __HAL_DMA_GET_COUNTER(&DMAHandle))*4;

                    // DMA has most likely corrupted FB alloc state and or more.
                    if (MAIN_FB()->bpp > OMV_RAW_BUF_SIZE) {
                        __fatal_error("JPEG Overflow!");
                    }

                    #if defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
                    // In JPEG mode, the DMA uses the frame buffer memory directly instead of the line buffer, which is
                    // located in a cacheable region and therefore must be invalidated before the CPU can access it again.
                    // Note: The frame buffer address is 32-byte aligned, and the size is a multiple of 32-bytes for all boards.
                    SCB_InvalidateDCache_by_Addr((uint32_t*)MAIN_FB()->pixels, OMV_RAW_BUF_SIZE);
                    #endif
                }
                // Clean trailing data.
                while ((MAIN_FB()->bpp >= 2)
                   && ((MAIN_FB()->pixels[MAIN_FB()->bpp-2] != 0xFF)
                    || (MAIN_FB()->pixels[MAIN_FB()->bpp-1] != 0xD9))) {
                    MAIN_FB()->bpp -= 1;
                }
                break;
            default:
                break;
        }

        //
        // Finally, return an image object.
        //

        // Set the user image.
        if (image != NULL) {
            image->w = MAIN_FB()->w;
            image->h = MAIN_FB()->h;
            image->bpp = MAIN_FB()->bpp;
            image->pixels = MAIN_FB()->pixels;

            if (streaming_cb) {
                // In streaming mode, either switch frame buffers in double buffer mode,
                // or call the streaming callback with the main FB in single buffer mode.
                if (doublebuf == false) {
                    // In single buffer mode, call streaming callback.
                    streaming = streaming_cb(image);
                } else {
                    // In double buffer mode, switch frame buffers.
                    if (frame == 0) {
                        image->pixels = MAIN_FB()->pixels;
                        // Next frame will be transferred to the second half.
                        dest_fb = MAIN_FB()->pixels + length;
                    } else {
                        image->pixels = MAIN_FB()->pixels + length;
                        // Next frame will be transferred to the first half.
                        dest_fb = MAIN_FB()->pixels;
                    }

                    // Switch frame buffers.
                    frame ^= 1;
                }
            }
        }
    } while (streaming == true);

    return 0;
}
