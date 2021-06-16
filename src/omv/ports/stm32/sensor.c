/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "irq.h"
#include "cambus.h"
#include "sensor.h"
#include "ov2640.h"
#include "ov5640.h"
#include "ov7725.h"
#include "ov7670.h"
#include "ov7690.h"
#include "ov9650.h"
#include "mt9v034.h"
#include "mt9m114.h"
#include "lepton.h"
#include "hm01b0.h"
#include "paj6100.h"
#include "gc2145.h"
#include "systick.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"

#define MDMA_BUFFER_SIZE        (64)
#define DMA_MAX_XFER_SIZE       (0xFFFF*4)
#define DMA_MAX_XFER_SIZE_DBL   ((DMA_MAX_XFER_SIZE)*2)
#define DMA_LENGTH_ALIGNMENT    (16)
#define SENSOR_TIMEOUT_MS       (3000)

// Higher performance complete MDMA offload.
#if (OMV_ENABLE_SENSOR_MDMA == 1)
#define OMV_ENABLE_SENSOR_MDMA_TOTAL_OFFLOAD 1
#endif

#if (OMV_ENABLE_PAJ6100 == 1)
#define OMV_ENABLE_NONI2CIS
#endif

sensor_t sensor = {};
static TIM_HandleTypeDef  TIMHandle  = {.Instance = DCMI_TIM};
static DMA_HandleTypeDef  DMAHandle  = {.Instance = DMA2_Stream1};
static DCMI_HandleTypeDef DCMIHandle = {.Instance = DCMI};
#if (OMV_ENABLE_SENSOR_MDMA == 1)
static MDMA_HandleTypeDef DCMI_MDMA_Handle0 = {.Instance = MDMA_Channel0};
static MDMA_HandleTypeDef DCMI_MDMA_Handle1 = {.Instance = MDMA_Channel1};
#endif
// SPI on image sensor connector.
#ifdef ISC_SPI
SPI_HandleTypeDef ISC_SPIHandle = {.Instance = ISC_SPI};
#endif // ISC_SPI

static bool first_line = false;
static bool drop_frame = false;

extern uint8_t _line_buf;

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
    {30,   20  },    /* HQQQQVGA  */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    {480,  320 },    /* HVGA      */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x128   */
    // Himax Resolutions
    {160,  160 },    /* 160x160   */
    {320,  320 },    /* 320x320   */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {720,  480 },    /* WVGA      */
    {752,  480 },    /* WVGA2     */
    {800,  600 },    /* SVGA      */
    {1024, 768 },    /* XGA       */
    {1280, 768 },    /* WXGA      */
    {1280, 1024},    /* SXGA      */
    {1280, 960 },    /* SXGAM     */
    {1600, 1200},    /* UXGA      */
    {1280, 720 },    /* HD        */
    {1920, 1080},    /* FHD       */
    {2560, 1440},    /* QHD       */
    {2048, 1536},    /* QXGA      */
    {2560, 1600},    /* WQXGA     */
    {2592, 1944},    /* WQXGA2    */
};

void DCMI_IRQHandler(void) {
    HAL_DCMI_IRQHandler(&DCMIHandle);
}

void DMA2_Stream1_IRQHandler(void) {
    HAL_DMA_IRQHandler(DCMIHandle.DMA_Handle);
}

#ifdef ISC_SPI
void ISC_SPI_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&ISC_SPIHandle);
}

void ISC_SPI_DMA_IRQHandler(void)
{
    HAL_DMA_IRQHandler(ISC_SPIHandle.hdmarx);
}
#endif // ISC_SPI

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
    TIMOCHandle.OCNIdleState= TIM_OCIDLESTATE_RESET;

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
        HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
        #if (OMV_ENABLE_SENSOR_MDMA == 1)
        HAL_MDMA_Abort(&DCMI_MDMA_Handle0);
        HAL_MDMA_Abort(&DCMI_MDMA_Handle1);
        HAL_MDMA_DeInit(&DCMI_MDMA_Handle0);
        HAL_MDMA_DeInit(&DCMI_MDMA_Handle1);
        #endif
        __HAL_DCMI_DISABLE_IT(&DCMIHandle, DCMI_IT_FRAME);
        __HAL_DCMI_CLEAR_FLAG(&DCMIHandle, DCMI_FLAG_FRAMERI);
        first_line = false;
        drop_frame = false;
        sensor.last_frame_ms = 0;
        sensor.last_frame_ms_valid = false;
    }

    framebuffer_reset_buffers();
}

// Returns true if a crop is being applied to the frame buffer.
static bool cropped()
{
    if (sensor.framesize == FRAMESIZE_INVALID) {
        return false;
    }

    return MAIN_FB()->x // needs to be zero if not being cropped.
        || MAIN_FB()->y // needs to be zero if not being cropped.
        || (MAIN_FB()->u != resolution[sensor.framesize][0])  // should be equal to the resolution if not cropped.
        || (MAIN_FB()->v != resolution[sensor.framesize][1]); // should be equal to the resolution if not cropped.
}

void sensor_init0()
{
    dcmi_abort();

    // Always reinit cambus after soft reset which could have terminated the cambus in the middle
    // of an I2C read/write.
    cambus_init(&sensor.bus, ISC_I2C_ID, ISC_I2C_SPEED);

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    // Disable Frame callback.
    sensor_set_frame_callback(NULL);
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
    cambus_init(&sensor.bus, ISC_I2C_ID, ISC_I2C_SPEED);
    systick_sleep(10);

    /* Probe the sensor */
    sensor.slv_addr = cambus_scan(&sensor.bus);
    if (sensor.slv_addr == 0) {
        /* Sensor has been held in reset,
           so the reset line is active low */
        sensor.reset_pol = ACTIVE_LOW;

        /* Pull the sensor out of the reset state */
        DCMI_RESET_HIGH();
        systick_sleep(10);

        /* Probe again to set the slave addr */
        sensor.slv_addr = cambus_scan(&sensor.bus);
        if (sensor.slv_addr == 0) {
            sensor.pwdn_pol = ACTIVE_LOW;

            DCMI_PWDN_HIGH();
            systick_sleep(10);

            sensor.slv_addr = cambus_scan(&sensor.bus);
            if (sensor.slv_addr == 0) {
                sensor.reset_pol = ACTIVE_HIGH;

                DCMI_RESET_LOW();
                systick_sleep(10);

                sensor.slv_addr = cambus_scan(&sensor.bus);
                #ifndef OMV_ENABLE_NONI2CIS
                if (sensor.slv_addr == 0) {
                    return -2;
                }
                #endif
            }
        }
    }

    // Clear sensor chip ID.
    sensor.chip_id_w = 0;

    // Set default snapshot function.
    sensor.snapshot = sensor_snapshot;

    switch (sensor.slv_addr) {
        #if (OMV_ENABLE_PAJ6100 == 1)
        case 0:
            if (paj6100_detect(&sensor)) {
                // Found PixArt PAJ6100
                sensor.chip_id_w = PAJ6100_ID;
                sensor.pwdn_pol = ACTIVE_LOW;
                sensor.reset_pol = ACTIVE_LOW;
                break;
            }
            // Okay, there is not any sensor be detected.
            return -2;
        #endif

        #if (OMV_ENABLE_OV2640 == 1)
        case OV2640_SLV_ADDR: // Or OV9650.
            cambus_readb(&sensor.bus, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
            break;
        #endif // (OMV_ENABLE_OV2640 == 1)

        #if (OMV_ENABLE_OV5640 == 1)
        case OV5640_SLV_ADDR:
            cambus_readb2(&sensor.bus, sensor.slv_addr, OV5640_CHIP_ID, &sensor.chip_id);
            break;
        #endif // (OMV_ENABLE_OV5640 == 1)

        #if (OMV_ENABLE_OV7725 == 1) || (OMV_ENABLE_OV7670 == 1) || (OMV_ENABLE_OV7690 == 1)
        case OV7725_SLV_ADDR: // Or OV7690 or OV7670.
            cambus_readb(&sensor.bus, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
            break;
        #endif //(OMV_ENABLE_OV7725 == 1) || (OMV_ENABLE_OV7670 == 1) || (OMV_ENABLE_OV7690 == 1)

        #if (OMV_ENABLE_MT9V034 == 1)
        case MT9V034_SLV_ADDR:
            cambus_readb(&sensor.bus, sensor.slv_addr, ON_CHIP_ID, &sensor.chip_id);
            break;
        #endif //(OMV_ENABLE_MT9V034 == 1)

        #if (OMV_ENABLE_MT9M114 == 1)
        case MT9M114_SLV_ADDR:
            cambus_readw2(&sensor.bus, sensor.slv_addr, ON_CHIP_ID, &sensor.chip_id_w);
            break;
        #endif // (OMV_ENABLE_MT9M114 == 1)

        #if (OMV_ENABLE_LEPTON == 1)
        case LEPTON_SLV_ADDR:
            sensor.chip_id = LEPTON_ID;
            break;
        #endif // (OMV_ENABLE_LEPTON == 1)

        #if (OMV_ENABLE_HM01B0 == 1)
        case HM01B0_SLV_ADDR:
            cambus_readb2(&sensor.bus, sensor.slv_addr, HIMAX_CHIP_ID, &sensor.chip_id);
            break;
        #endif //(OMV_ENABLE_HM01B0 == 1)

        #if (OMV_ENABLE_GC2145 == 1)
        case GC2145_SLV_ADDR:
            cambus_readb(&sensor.bus, sensor.slv_addr, GC_CHIP_ID, &sensor.chip_id);
            break;
        #endif //(OMV_ENABLE_GC2145 == 1)

        default:
            return -3;
            break;
    }
    switch (sensor.chip_id_w) {
        #if (OMV_ENABLE_OV2640 == 1)
        case OV2640_ID:
            if (extclk_config(OV2640_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = ov2640_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV2640 == 1)

        #if (OMV_ENABLE_OV5640 == 1)
        case OV5640_ID:
            if (extclk_config(OV5640_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = ov5640_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV5640 == 1)

        #if (OMV_ENABLE_OV7670 == 1)
        case OV7670_ID:
            if (extclk_config(OV7670_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = ov7670_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV7670 == 1)

        #if (OMV_ENABLE_OV7690 == 1)
        case OV7690_ID:
            if (extclk_config(OV7690_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = ov7690_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV7690 == 1)

        #if (OMV_ENABLE_OV7725 == 1)
        case OV7725_ID:
            init_ret = ov7725_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV7725 == 1)

        #if (OMV_ENABLE_OV9650 == 1)
        case OV9650_ID:
            init_ret = ov9650_init(&sensor);
            break;
        #endif // (OMV_ENABLE_OV9650 == 1)

        #if (OMV_ENABLE_MT9V034 == 1)
        case MT9V034_ID:
            if (extclk_config(MT9V034_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = mt9v034_init(&sensor);
            break;
        #endif //(OMV_ENABLE_MT9V034 == 1)

        #if (OMV_ENABLE_MT9M114 == 1)
        case MT9M114_ID:
            if (extclk_config(MT9M114_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = mt9m114_init(&sensor);
            break;
        #endif //(OMV_ENABLE_MT9M114 == 1)

        #if (OMV_ENABLE_LEPTON == 1)
        case LEPTON_ID:
            if (extclk_config(LEPTON_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = lepton_init(&sensor);
            break;
        #endif // (OMV_ENABLE_LEPTON == 1)

        #if (OMV_ENABLE_HM01B0 == 1)
        case HM01B0_ID:
            init_ret = hm01b0_init(&sensor);
            break;
        #endif //(OMV_ENABLE_HM01B0 == 1)

        #if (OMV_ENABLE_GC2145 == 1)
        case GC2145_ID:
            if (extclk_config(GC2145_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = gc2145_init(&sensor);
            break;
        #endif //(OMV_ENABLE_GC2145 == 1)

        #if (OMV_ENABLE_PAJ6100 == 1)
        case PAJ6100_ID:
            if (extclk_config(PAJ6100_XCLK_FREQ) != 0) {
                return -3;
            }
            init_ret = paj6100_init(&sensor);
            break;
        #endif // (OMV_ENABLE_PAJ6100 == 1)      
            
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

    sensor.detected = true;

    /* All good! */
    return 0;
}

int sensor_reset()
{
    dcmi_abort();

    // Reset the sensor state
    sensor.sde                  = 0;
    sensor.pixformat            = 0;
    sensor.framesize            = 0;
    sensor.framerate            = 0;
    sensor.last_frame_ms        = 0;
    sensor.last_frame_ms_valid  = false;
    sensor.gainceiling          = 0;
    sensor.hmirror              = false;
    sensor.vflip                = false;
    sensor.transpose            = false;
    #if MICROPY_PY_IMU
    sensor.auto_rotation        = sensor.chip_id == OV7690_ID;
    #else
    sensor.auto_rotation        = false;
    #endif // MICROPY_PY_IMU
    sensor.vsync_callback       = NULL;
    sensor.frame_callback       = NULL;

    // Reset default color palette.
    sensor.color_palette        = rainbow_table;

    sensor.disable_full_flush = false;

    // Restore shutdown state on reset.
    sensor_shutdown(false);

    // Hard-reset the sensor
    if (sensor.reset_pol == ACTIVE_HIGH) {
        DCMI_RESET_HIGH();
        systick_sleep(10);
        DCMI_RESET_LOW();
    } else {
        DCMI_RESET_LOW();
        systick_sleep(10);
        DCMI_RESET_HIGH();
    }
    systick_sleep(20);

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
    return sensor.chip_id_w;
}

bool sensor_is_detected()
{
    return sensor.detected;
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
    int ret = 0;
    dcmi_abort();

    if (enable) {
        if (sensor.pwdn_pol == ACTIVE_HIGH) {
            DCMI_PWDN_HIGH();
        } else {
            DCMI_PWDN_LOW();
        }
        HAL_NVIC_DisableIRQ(DCMI_IRQn);
        HAL_DCMI_DeInit(&DCMIHandle);
    } else {
        if (sensor.pwdn_pol == ACTIVE_HIGH) {
            DCMI_PWDN_LOW();
        } else {
            DCMI_PWDN_HIGH();
        }
        ret = dcmi_config(DCMI_JPEG_DISABLE);
    }

    systick_sleep(10);
    return ret;
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
    uint32_t size = framebuffer_get_buffer_size();
    if ((sensor.pixformat == PIXFORMAT_BAYER)
    &&  (pixformat == PIXFORMAT_RGB565)
    &&  (MAIN_FB()->u * MAIN_FB()->v * 2 > size)
    &&  (MAIN_FB()->u * MAIN_FB()->v * 1 <= size)) {
        // No change
        return 0;
    }

    // Cropping and transposing (and thus auto rotation) don't work in JPEG mode.
    if ((pixformat == PIXFORMAT_JPEG) && (cropped() || sensor.transpose || sensor.auto_rotation)) {
        return -1;
    }

    dcmi_abort();

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

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

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();

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

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

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

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();

    return 0;
}

int sensor_set_framerate(int framerate)
{
    if (sensor.framerate == framerate) {
        // No change
        return 0;
    }

    if (framerate < 0) {
        return -1;
    }

    // Call the sensor specific function (does not fail if function is not set)
    if (sensor.set_framerate != NULL) {
        if (sensor.set_framerate(&sensor, framerate) != 0) {
            // Operation not supported
            return -1;
        }
    }

    // Set framerate
    sensor.framerate = framerate;
    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h)
{
    if ((MAIN_FB()->x == x) && (MAIN_FB()->y == y) && (MAIN_FB()->u == w) && (MAIN_FB()->v == h)) {
        // No change
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    dcmi_abort();

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    // Skip the first frame.
    MAIN_FB()->bpp = -1;

    MAIN_FB()->x = x;
    MAIN_FB()->y = y;
    MAIN_FB()->w = MAIN_FB()->u = w;
    MAIN_FB()->h = MAIN_FB()->v = h;

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();

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

    dcmi_abort();

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

    dcmi_abort();

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
    if (sensor.transpose == enable) {
        /* no change */
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    dcmi_abort();

    sensor.transpose = enable;
    return 0;
}

bool sensor_get_transpose()
{
    return sensor.transpose;
}

int sensor_set_auto_rotation(bool enable)
{
    if (sensor.auto_rotation == enable) {
        /* no change */
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return -1;
    }

    dcmi_abort();

    sensor.auto_rotation = enable;
    return 0;
}

bool sensor_get_auto_rotation()
{
    return sensor.auto_rotation;
}

int sensor_set_framebuffers(int count)
{
    dcmi_abort();

    // Flush previous frame.
    framebuffer_update_jpeg_buffer();

    return framebuffer_set_buffers(count);
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

int sensor_set_vsync_callback(vsync_cb_t vsync_cb)
{
    sensor.vsync_callback = vsync_cb;
    if (sensor.vsync_callback == NULL) {
        // Disable VSYNC EXTI IRQ
        HAL_NVIC_DisableIRQ(DCMI_VSYNC_IRQN);
    } else {
        // Enable VSYNC EXTI IRQ
        NVIC_SetPriority(DCMI_VSYNC_IRQN, IRQ_PRI_EXTINT);
        HAL_NVIC_EnableIRQ(DCMI_VSYNC_IRQN);
    }
    return 0;
}

int sensor_set_frame_callback(frame_cb_t vsync_cb)
{
    sensor.frame_callback = vsync_cb;
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
    if (sensor.vsync_callback != NULL) {
        sensor.vsync_callback(HAL_GPIO_ReadPin(DCMI_VSYNC_PORT, DCMI_VSYNC_PIN));
    }
}

static uint32_t get_src_bytes_per_pixel()
{
    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
            return sensor.gs_bpp;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            return 2;
        case PIXFORMAT_BAYER:
        case PIXFORMAT_JPEG:
            return 1;
        default:
            return 0;
    }
}

static uint32_t get_dst_bytes_per_pixel()
{
    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER:
            return 1;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            return 2;
        default:
            return 0;
    }
}

// If we are cropping the image by more than 1 word in width we can align the line start to
// a word address to improve copy performance. Do not crop by more than 1 word as this will
// result in less time between DMA transfers complete interrupts on 16-byte boundaries.
static uint32_t get_dcmi_hw_crop(uint32_t bytes_per_pixel)
{
    uint32_t byte_x_offset = (MAIN_FB()->x * bytes_per_pixel) % sizeof(uint32_t);
    uint32_t width_remainder = (resolution[sensor.framesize][0] - (MAIN_FB()->x + MAIN_FB()->u)) * bytes_per_pixel;
    uint32_t x_crop = 0;

    if (byte_x_offset && (width_remainder >= (sizeof(uint32_t) - byte_x_offset))) {
        x_crop = byte_x_offset;
    }

    return x_crop;
}

// To make the user experience better we automatically shrink the size of the MAIN_FB() to fit
// within the RAM we have onboard the system.
static void sensor_check_buffsize()
{
    uint32_t size = framebuffer_get_buffer_size();
    uint32_t bpp = get_dst_bytes_per_pixel();

    // If the pixformat is NULL/JPEG there we can't do anything to check if it fits before hand.
    if (!bpp) {
        return;
    }

    // MAIN_FB() fits, we are done.
    if ((MAIN_FB()->u * MAIN_FB()->v * bpp) <= size) {
        return;
    }

    if (sensor.pixformat == PIXFORMAT_RGB565) {
        // Switch to bayer for the quick 2x savings.
        sensor_set_pixformat(PIXFORMAT_BAYER);
        bpp = 1;

        // MAIN_FB() fits, we are done (bpp is 1).
        if ((MAIN_FB()->u * MAIN_FB()->v) <= size) {
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
    while (((MAIN_FB()->u * MAIN_FB()->v * bpp) > size) || (MAIN_FB()->u % 2)  || (MAIN_FB()->v % 2)) {
        MAIN_FB()->u -= u_sub;
        MAIN_FB()->v -= v_sub;
    }

    // Center the new window using the previous offset and keep the offset even.
    MAIN_FB()->x += (window_w - MAIN_FB()->u) / 2;
    MAIN_FB()->y += (window_h - MAIN_FB()->v) / 2;
    if (MAIN_FB()->x % 2) MAIN_FB()->x -= 1;
    if (MAIN_FB()->y % 2) MAIN_FB()->y -= 1;

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers();
}

// Stop allowing new data in on the end of the frame and let snapshot know that the frame has been
// received. Note that DCMI_DMAConvCpltUser() is called before DCMI_IT_FRAME is enabled by
// DCMI_DMAXferCplt() so this means that the last line of data is *always* transferred before
// moving the tail to the next buffer.
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    // This can be executed at any time since this interrupt has a higher priority than DMA2_Stream1_IRQn.
    #if (OMV_ENABLE_SENSOR_MDMA_TOTAL_OFFLOAD == 1)
    // Clear out any stale flags.
    DMA2->LIFCR = DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5;
    // Re-enable the DMA IRQ to catch the next start line.
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
    #endif

    // Reset DCMI_DMAConvCpltUser frame drop state.
    first_line = false;
    if (drop_frame) {
        drop_frame = false;
        return;
    }

    framebuffer_get_tail(FB_NO_FLAGS);

    if (sensor.frame_callback) {
        sensor.frame_callback();
    }
}

#if (OMV_ENABLE_SENSOR_MDMA == 1)
static void mdma_memcpy(vbuffer_t *buffer, void *dst, void *src, int bpp, bool transposed)
{
    // We're using two handles to give each channel the maximum amount of time possible to do the line
    // transfer. In most situations only one channel will be running at a time. However, if SDRAM is
    // backedup we don't have to disable the channel if it is flushing trailing data to SDRAM.
    MDMA_HandleTypeDef *handle = (buffer->offset % 2) ? &DCMI_MDMA_Handle1 : &DCMI_MDMA_Handle0;

    // If MDMA is still running from a previous transfer HAL_MDMA_Start() will disable that transfer
    // and start a new transfer.
    __HAL_UNLOCK(handle);
    handle->State = HAL_MDMA_STATE_READY;
    HAL_MDMA_Start(handle,
                   (uint32_t) src,
                   (uint32_t) dst,
                   transposed ? bpp : (MAIN_FB()->u * bpp),
                   transposed ? MAIN_FB()->u : 1);
}
#endif

// This function is called back after each line transfer is complete,
// with a pointer to the line buffer that was used. At this point the
// DMA transfers the next line to the other half of the line buffer.
void DCMI_DMAConvCpltUser(uint32_t addr)
{
    if (!first_line) {
        first_line = true;
        uint32_t tick = HAL_GetTick();
        uint32_t framerate_ms = IM_DIV(1000, sensor.framerate);

        // Drops frames to match the frame rate requested by the user. The frame is NOT copied to
        // SRAM/SDRAM when dropping to save CPU cycles/energy that would be wasted.
        // If framerate is zero then this does nothing...
        if (sensor.last_frame_ms_valid && ((tick - sensor.last_frame_ms) < framerate_ms)) {
            drop_frame = true;
        } else if (sensor.last_frame_ms_valid) {
            sensor.last_frame_ms += framerate_ms;
        } else {
            sensor.last_frame_ms = tick;
            sensor.last_frame_ms_valid = true;
        }
    }

    if (drop_frame) {
        // If we're dropping a frame in full offload mode it's safe to disable this interrupt saving
        // ourselves from having to service the DMA complete callback.
        #if (OMV_ENABLE_SENSOR_MDMA_TOTAL_OFFLOAD == 1)
        if (!sensor.transpose) {
            HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);
        }
        #endif
        return;
    }

    vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);

    // If snapshot was not already waiting to receive data then we have missed this frame and have
    // to drop it. So, abort this and future transfers. Snapshot will restart the process.
    if (!buffer) {
        DCMI->CR &= ~DCMI_CR_ENABLE;
        HAL_DMA_Abort_IT(&DMAHandle); // Note: Use HAL_DMA_Abort_IT and not HAL_DMA_Abort inside an interrupt.
        #if (OMV_ENABLE_SENSOR_MDMA == 1)
        HAL_MDMA_DeInit(&DCMI_MDMA_Handle0);
        HAL_MDMA_DeInit(&DCMI_MDMA_Handle1);
        #endif
        __HAL_DCMI_DISABLE_IT(&DCMIHandle, DCMI_IT_FRAME);
        __HAL_DCMI_CLEAR_FLAG(&DCMIHandle, DCMI_FLAG_FRAMERI);
        first_line = false;
        drop_frame = false;
        sensor.last_frame_ms = 0;
        sensor.last_frame_ms_valid = false;
        // Reset the queue of frames when we start dropping frames.
        if (!sensor.disable_full_flush) {
            framebuffer_flush_buffers();
        }
        return;
    }

    // We are transferring the image from the DCMI hardware to line buffers so that we have more
    // control to post process the image data before writing it to the frame buffer. This requires
    // more CPU, but, allows us to crop and rotate the image as the data is received.

    // Additionally, the line buffers act as very large fifos which hide SDRAM memory access times
    // on the OpenMV Cam H7 Plus. When SDRAM refreshes the row you are trying to write to the fifo
    // depth on the DCMI hardware and DMA hardware is not enough to prevent data loss.

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
            uint16_t size = __REV16(*((uint16_t *) addr));
            // Prevent a buffer overflow when writing the jpeg data.
            if (buffer->offset + size > framebuffer_get_buffer_size()) {
                buffer->jpeg_buffer_overflow = true;
                return;
            }
            unaligned_memcpy(buffer->data + buffer->offset, ((uint16_t *) addr) + 1, size);
            buffer->offset += size;
        } else if (sensor.chip_id == OV2640_ID) {
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
            buffer->offset += 1;
        }
        return;
    }

    // DCMI_DMAXferCplt in the HAL DCMI driver always calls DCMI_DMAConvCpltUser with the other
    // MAR register. So, we have to fix the address in full MDMA offload mode...
    #if (OMV_ENABLE_SENSOR_MDMA_TOTAL_OFFLOAD == 1)
    if (!sensor.transpose) {
        addr = (uint32_t) &_line_buf;
    }
    #endif

    uint32_t bytes_per_pixel = get_src_bytes_per_pixel();
    uint8_t *src = ((uint8_t *) addr) + (MAIN_FB()->x * bytes_per_pixel) - get_dcmi_hw_crop(bytes_per_pixel);
    uint8_t *dst = buffer->data;

    if (sensor.pixformat == PIXFORMAT_GRAYSCALE) {
        bytes_per_pixel = sizeof(uint8_t);
    }

    // For all non-JPEG and non-transposed modes we can completely offload image catpure to MDMA
    // and we do not need to receive any line interrupts for the rest of the frame until it ends.
    #if (OMV_ENABLE_SENSOR_MDMA_TOTAL_OFFLOAD == 1)
    if (!sensor.transpose) {
        // NOTE: We're starting MDMA here because it gives the maximum amount of time before we
        // have to drop the frame if there's no space. If you use the FRAME/VSYNC callbacks then
        // you will have to drop the frame earlier than necessary if there's no space resulting
        // in the apparent unloaded FPS being lower than this method gives you.
        uint32_t line_width_bytes = MAIN_FB()->u * bytes_per_pixel;
        // DMA0 will copy this line of the image to the final destination.
        __HAL_UNLOCK(&DCMI_MDMA_Handle0);
        DCMI_MDMA_Handle0.State = HAL_MDMA_STATE_READY;
        HAL_MDMA_Start(&DCMI_MDMA_Handle0, (uint32_t) src, (uint32_t) dst,
                       line_width_bytes, 1);
        // DMA1 will copy all remaining lines of the image to the final destination.
        __HAL_UNLOCK(&DCMI_MDMA_Handle1);
        DCMI_MDMA_Handle1.State = HAL_MDMA_STATE_READY;
        HAL_MDMA_Start(&DCMI_MDMA_Handle1, (uint32_t) src, (uint32_t) (dst + line_width_bytes),
                       line_width_bytes, MAIN_FB()->v - 1);
        HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);
        return;
    }
    #endif

    if (!sensor.transpose) {
        dst += MAIN_FB()->u * bytes_per_pixel * buffer->offset++;
    } else {
        dst += bytes_per_pixel * buffer->offset++;
    }

    // Implement per line, per pixel cropping, and image transposing (for image rotation) in
    // in software using the CPU to transfer the image from the line buffers to the frame buffer.
    uint16_t *src16 = (uint16_t *) src;
    uint16_t *dst16 = (uint16_t *) dst;

    switch (sensor.pixformat) {
        case PIXFORMAT_BAYER:
            #if (OMV_ENABLE_SENSOR_MDMA == 1)
            mdma_memcpy(buffer, dst, src, sizeof(uint8_t), sensor.transpose);
            #else
            if (!sensor.transpose) {
                unaligned_memcpy(dst, src, MAIN_FB()->u);
            } else {
                for (int i = MAIN_FB()->u, h = MAIN_FB()->v; i; i--) {
                    *dst = *src++;
                    dst += h;
                }
            }
            #endif
            break;
        case PIXFORMAT_GRAYSCALE:
            #if (OMV_ENABLE_SENSOR_MDMA == 1)
            mdma_memcpy(buffer, dst, src, sizeof(uint8_t), sensor.transpose);
            #else
            if (sensor.gs_bpp == sizeof(uint8_t)) {
                // 1BPP GRAYSCALE.
                if (!sensor.transpose) {
                    unaligned_memcpy(dst, src, MAIN_FB()->u);
                } else {
                    for (int i = MAIN_FB()->u, h = MAIN_FB()->v; i; i--) {
                        *dst = *src++;
                        dst += h;
                    }
                }
            } else {
                // Extract Y channel from YUV.
                if (!sensor.transpose) {
                    unaligned_2_to_1_memcpy(dst, src16, MAIN_FB()->u);
                } else {
                    for (int i = MAIN_FB()->u, h = MAIN_FB()->v; i; i--) {
                        *dst = *src16++;
                        dst += h;
                    }
                }
            }
            #endif
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            #if (OMV_ENABLE_SENSOR_MDMA == 1)
            mdma_memcpy(buffer, dst16, src16, sizeof(uint16_t), sensor.transpose);
            #else
            if (SENSOR_HW_FLAGS_GET(&sensor, SENSOR_HW_FLAGS_RGB565_REV)) {
                if (!sensor.transpose) {
                    unaligned_memcpy_rev16(dst16, src16, MAIN_FB()->u);
                } else {
                    for (int i = MAIN_FB()->u, h = MAIN_FB()->v; i; i--) {
                        *dst16 = __REV16(*src16++);
                        dst16 += h;
                    }
                }
            } else {
                if (!sensor.transpose) {
                    unaligned_memcpy(dst16, src16, MAIN_FB()->u * sizeof(uint16_t));
                } else {
                    for (int i = MAIN_FB()->u, h = MAIN_FB()->v; i; i--) {
                        *dst16 = *src16++;
                        dst16 += h;
                    }
                }
            }
            #endif
            break;
        default:
            break;
    }
}

#if (OMV_ENABLE_SENSOR_MDMA == 1)
// Configures an MDMA channel to completely offload the CPU in copying one line of pixels.
static void mdma_config(MDMA_InitTypeDef *init, sensor_t *sensor, uint32_t bytes_per_pixel)
{
    init->Request                   = MDMA_REQUEST_SW;
    init->TransferTriggerMode       = MDMA_REPEAT_BLOCK_TRANSFER;
    init->Priority                  = MDMA_PRIORITY_VERY_HIGH;
    init->DataAlignment             = MDMA_DATAALIGN_PACKENABLE;
    init->BufferTransferLength      = MDMA_BUFFER_SIZE;
    // The source address is 1KB aligned. So, a burst size of 16 beats (AHB Max) should not break.
    // Destination lines may not be aligned however so the burst size must be computed.
    init->SourceBurst               = MDMA_SOURCE_BURST_16BEATS;
    init->SourceBlockAddressOffset  = 0;
    init->DestBlockAddressOffset    = 0;

    if ((sensor->pixformat == PIXFORMAT_RGB565) && SENSOR_HW_FLAGS_GET(sensor, SENSOR_HW_FLAGS_RGB565_REV)) {
        init->Endianness = MDMA_LITTLE_BYTE_ENDIANNESS_EXCHANGE;
    } else {
        init->Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    }

    uint32_t line_offset_bytes = (MAIN_FB()->x * bytes_per_pixel) - get_dcmi_hw_crop(bytes_per_pixel);
    uint32_t line_width_bytes = MAIN_FB()->u * bytes_per_pixel;

    if (sensor->transpose) {
        line_width_bytes = bytes_per_pixel;
        init->DestBlockAddressOffset = (MAIN_FB()->v - 1) * bytes_per_pixel;
    }

    // YUV422 Source -> Y Destination
    if ((sensor->pixformat == PIXFORMAT_GRAYSCALE) && (sensor->gs_bpp == sizeof(uint16_t))) {
        line_width_bytes /= 2;
        if (sensor->transpose) {
            init->DestBlockAddressOffset /= 2;
        }
    }

    // Destination will be 32-byte aligned. So, we just need to breakup the line width into the largest
    // power of 2. Source may have an offset which further limits this to a sub power of 2.
    for (int i = 3; i >= 0; i--) {
        if (!(line_width_bytes % (1 << i))) {
            for (int j = IM_MIN(i, 2); j >= 0; j--) {
                if (!(line_offset_bytes % (1 << j))) {
                    init->SourceInc = MDMA_CTCR_SINC_1 | (j << MDMA_CTCR_SINCOS_Pos);
                    init->SourceDataSize = j << MDMA_CTCR_SSIZE_Pos;
                    break;
                }
            }

            init->DestinationInc = MDMA_CTCR_DINC_1 | (i << MDMA_CTCR_DINCOS_Pos);
            init->DestDataSize = i << MDMA_CTCR_DSIZE_Pos;

            // Find the burst size we can break the destination transfer up into.
            uint32_t count = MDMA_BUFFER_SIZE >> i;

            for (int i = 7; i >= 0; i--) {
                if (!(count % (1 << i))) {
                    init->DestBurst = i << MDMA_CTCR_DBURST_Pos;
                    break;
                }
            }

            break;
        }
    }

    // YUV422 Source -> Y Destination
    if ((sensor->pixformat == PIXFORMAT_GRAYSCALE) && (sensor->gs_bpp == sizeof(uint16_t))) {
        init->SourceInc         = MDMA_SRC_INC_HALFWORD;
        init->SourceDataSize    = MDMA_SRC_DATASIZE_BYTE;
    }
}
#endif

// This is the default snapshot function, which can be replaced in sensor_init functions. This function
// uses the DCMI and DMA to capture frames and each line is processed in the DCMI_DMAConvCpltUser function.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    uint32_t length = 0;

    // Compress the framebuffer for the IDE preview, only if it's not the first frame,
    // the framebuffer is enabled and the image sensor does not support JPEG encoding.
    // Note: This doesn't run unless the IDE is connected and the framebuffer is enabled.
    framebuffer_update_jpeg_buffer();

    // Make sure the raw frame fits into the FB. It will be switched from RGB565 to BAYER
    // first to save space before being cropped until it fits.
    sensor_check_buffsize();

    // The user may have changed the MAIN_FB width or height on the last image so we need
    // to restore that here. We don't have to restore bpp because that's taken care of
    // already in the code below. Note that we do the JPEG compression above first to save
    // the FB of whatever the user set it to and now we restore.
    uint32_t w = MAIN_FB()->u;
    uint32_t h = MAIN_FB()->v;

    // If DCMI_DMAConvCpltUser() happens before framebuffer_free_current_buffer(); below then the
    // transfer is stopped and it will be re-enabled again right afterwards in the single vbuffer
    // case. We know the transfer was stopped by checking DCMI_CR_ENABLE.
    framebuffer_free_current_buffer();

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
        // Setup the size and address of the transfer
        uint32_t bytes_per_pixel = get_src_bytes_per_pixel();

        // Error out if the pixformat is not set.
        if (!bytes_per_pixel) {
            return -1;
        }

        uint32_t x_crop = get_dcmi_hw_crop(bytes_per_pixel);
        uint32_t dma_line_width_bytes = resolution[sensor->framesize][0] * bytes_per_pixel;

        // Shrink the captured pixel count by one word to allow cropping to fix alignment.
        if (x_crop) {
            dma_line_width_bytes -= sizeof(uint32_t);
        }

        length = dma_line_width_bytes * h;

        // Error out if the transfer size is not compatible with DMA transfer restrictions.
        if ((!dma_line_width_bytes)
        || (dma_line_width_bytes % sizeof(uint32_t))
        || (dma_line_width_bytes > (OMV_LINE_BUF_SIZE / 2))
        || (!length)
        || (length % DMA_LENGTH_ALIGNMENT)) {
            return -2;
        }

        // Get the destination buffer address.
        vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);

        if ((sensor->pixformat == PIXFORMAT_JPEG) && (sensor->chip_id == OV2640_ID) && (!buffer)) {
            return -3;
        }

        #if (OMV_ENABLE_SENSOR_MDMA == 1)
        // The code below will enable MDMA data transfer from the DCMI line buffer for non-JPEG modes.
        if (sensor->pixformat != PIXFORMAT_JPEG) {
            mdma_config(&DCMI_MDMA_Handle0.Init, sensor, bytes_per_pixel);
            memcpy(&DCMI_MDMA_Handle1.Init, &DCMI_MDMA_Handle0.Init, sizeof(MDMA_InitTypeDef));
            HAL_MDMA_Init(&DCMI_MDMA_Handle0);

            #if (OMV_ENABLE_SENSOR_MDMA_TOTAL_OFFLOAD == 1)
            // If we are not transposing the image we can fully offload image capture from the CPU.
            if (!sensor->transpose) {
                // MDMA will trigger on each TC from DMA and transfer one line to the frame buffer.
                DCMI_MDMA_Handle1.Init.Request = MDMA_REQUEST_DMA2_Stream1_TC;
                DCMI_MDMA_Handle1.Init.TransferTriggerMode = MDMA_BLOCK_TRANSFER;
                // We setup MDMA to repeatedly reset itself to transfer the same line buffer.
                DCMI_MDMA_Handle1.Init.SourceBlockAddressOffset = -(MAIN_FB()->u * bytes_per_pixel);

                HAL_MDMA_Init(&DCMI_MDMA_Handle1);
                HAL_MDMA_ConfigPostRequestMask(&DCMI_MDMA_Handle1, (uint32_t) &DMA2->LIFCR, DMA_FLAG_TCIF1_5);
            } else {
                HAL_MDMA_Init(&DCMI_MDMA_Handle1);
            }
            #else
            HAL_MDMA_Init(&DCMI_MDMA_Handle1);
            #endif
        }
        #endif

        HAL_DCMI_DisableCrop(&DCMIHandle);
        if (sensor->pixformat != PIXFORMAT_JPEG) {
            // Vertically crop the image. Horizontal cropping is done in software.
            HAL_DCMI_ConfigCrop(&DCMIHandle, x_crop, MAIN_FB()->y, dma_line_width_bytes - 1, h - 1);
            HAL_DCMI_EnableCrop(&DCMIHandle);
        }

        // Reset the circular, current target, and double buffer mode flags which get set by the below calls.
        ((DMA_Stream_TypeDef *) DMAHandle.Instance)->CR &= ~(DMA_SxCR_CIRC | DMA_SxCR_CT | DMA_SxCR_DBM);

        // Note that HAL_DCMI_Start_DMA and HAL_DCMI_Start_DMA_MB are effectively the same
        // method. The only difference between them is how large the DMA transfer size gets
        // set at. For both of them DMA doesn't actually care how much data the DCMI hardware
        // generates. It's just trying to move fixed size DMA transfers from the DCMI hardware
        // to one memory address or another memory address. After transferring X bytes to one
        // address it will switch to the next address and transfer X bytes again. Both of these
        // methods set the addresses right after each other. So, effectively DMA is just writing
        // data to a circular buffer with an interrupt every time 1/2 of it is written.
        if ((sensor->pixformat == PIXFORMAT_JPEG) && (sensor->chip_id == OV2640_ID)) {
            // The JPEG image will be directly transferred to the frame buffer.
            // The DCMI hardware can transfer up to 524,280 bytes.
            length = DMA_MAX_XFER_SIZE_DBL;
            uint32_t size = framebuffer_get_buffer_size();
            length = IM_MIN(length, size);

            // Start a transfer where the whole frame buffer is located where the DMA is writing
            // data to. We only use this for JPEG mode for the OV2640. Since we don't know the
            // line size of data being transferred we just examine how much data was transferred
            // once DMA hardware stalls waiting for data. Note that because we are writing
            // directly to the frame buffer we do not have the option of aborting the transfer
            // if we are not ready to move data from a line buffer to the frame buffer.

            // In this mode the DMA hardware is just treating the frame buffer as two large
            // DMA buffers. At the end of the frame less data may be transferred than requested.
            HAL_DCMI_Start_DMA(&DCMIHandle, DCMI_MODE_SNAPSHOT,
                               (uint32_t) buffer->data, length / sizeof(uint32_t));

            // If length is greater than DMA_MAX_XFER_SIZE then HAL_DCMI_Start_DMA splits length
            // into two transfers less than DMA_MAX_XFER_SIZE.
            if (length > DMA_MAX_XFER_SIZE) {
                length /= 2;
            }
        #if (OMV_ENABLE_SENSOR_MDMA_TOTAL_OFFLOAD == 1)
        // Special transfer mode with MDMA that completely offloads the line capture load.
        } else if ((sensor->pixformat != PIXFORMAT_JPEG) && (!sensor->transpose)) {
            // DMA to circular mode writing the same line over and over again.
            ((DMA_Stream_TypeDef *) DMAHandle.Instance)->CR |= DMA_SxCR_CIRC;
            // DCMI will transfer to same line and MDMA will move to final location.
            HAL_DCMI_Start_DMA(&DCMIHandle, DCMI_MODE_CONTINUOUS,
                               (uint32_t) &_line_buf, dma_line_width_bytes / sizeof(uint32_t));
        #endif
        } else {
            // Start a multibuffer transfer (line by line). The DMA hardware will ping-pong
            // transferring data between the uncached line buffers. Since data is continuously
            // being captured the ping-ponging will stop at the end of the frame and then
            // continue when the next frame starts.
            HAL_DCMI_Start_DMA_MB(&DCMIHandle, DCMI_MODE_CONTINUOUS,
                                  (uint32_t) &_line_buf, length / sizeof(uint32_t), h);
        }
    }

    // Let the camera know we want to trigger it now.
    #if defined(DCMI_FSYNC_PIN)
    if (SENSOR_HW_FLAGS_GET(sensor, SENSOR_HW_FLAGS_FSYNC)) {
        DCMI_FSYNC_HIGH();
    }
    #endif

    // In camera sensor JPEG mode 4 we will not necessarily see every line in the frame and
    // in camera sensor JPEG mode 3 we will definitely not see every line in the frame. Given
    // this, we need to enable the end of frame interrupt before we have necessarily
    // finished transferring all JEPG data. This works as long as the end of the frame comes
    // much later after all JPEG data has been transferred. If this is violated the JPEG image
    // will be corrupted.
    if (DCMI->CR & DCMI_JPEG_ENABLE) {
        __HAL_DCMI_ENABLE_IT(&DCMIHandle, DCMI_IT_FRAME);
    }

    vbuffer_t *buffer = NULL;
    // Wait for the frame data. __WFI() below will exit right on time because of DCMI_IT_FRAME.
    // While waiting SysTick will trigger allowing us to timeout.
    for (uint32_t tick_start = HAL_GetTick(); !(buffer = framebuffer_get_head(FB_NO_FLAGS)); ) {
        __WFI();

        // If we haven't exited this loop before the timeout then we need to abort the transfer.
        if ((HAL_GetTick() - tick_start) > SENSOR_TIMEOUT_MS) {
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
    if ((sensor->pixformat == PIXFORMAT_JPEG) && (sensor->chip_id == OV2640_ID)) {
        dcmi_abort();
    }

    // We're done receiving data.
    #if defined(DCMI_FSYNC_PIN)
    if (SENSOR_HW_FLAGS_GET(sensor, SENSOR_HW_FLAGS_FSYNC)) {
        DCMI_FSYNC_LOW();
    }
    #endif

    // The JPEG in the frame buffer is actually invalid.
    if (buffer->jpeg_buffer_overflow) {
        return -5;
    }

    // Prepare the frame buffer w/h/bpp values given the image type.

    if (!sensor->transpose) {
        MAIN_FB()->w = w;
        MAIN_FB()->h = h;
    } else {
        MAIN_FB()->w = h;
        MAIN_FB()->h = w;
    }

    // Fix the BPP.
    switch (sensor->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            MAIN_FB()->bpp = IMAGE_BPP_GRAYSCALE;
            #if (OMV_ENABLE_SENSOR_MDMA == 1)
            // Flush data for MDMA
            SCB_InvalidateDCache_by_Addr(buffer->data, w * h);
            #endif
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            MAIN_FB()->bpp = IMAGE_BPP_RGB565;
            #if (OMV_ENABLE_SENSOR_MDMA == 1)
            // Flush data for MDMA
            SCB_InvalidateDCache_by_Addr(buffer->data, w * h * sizeof(uint16_t));
            #endif
            break;
        case PIXFORMAT_BAYER:
            MAIN_FB()->bpp = IMAGE_BPP_BAYER;
            #if (OMV_ENABLE_SENSOR_MDMA == 1)
            // Flush data for MDMA
            SCB_InvalidateDCache_by_Addr(buffer->data, w * h);
            #endif
            break;
        case PIXFORMAT_JPEG:
            if (sensor->chip_id == OV5640_ID) {
                // Offset contains the sum of all the bytes transferred from the offset buffers
                // while in DCMI_DMAConvCpltUser().
                MAIN_FB()->bpp = buffer->offset;
            } else {
                // Offset contains the number of length transfers completed. To get the number of bytes transferred
                // within a transfer we have to look at the DMA counter and see how much data was moved.
                int32_t size = buffer->offset * length;

                if (__HAL_DMA_GET_COUNTER(&DMAHandle)) { // Add in the uncompleted transfer length.
                    size += ((length / sizeof(uint32_t)) - __HAL_DMA_GET_COUNTER(&DMAHandle)) * sizeof(uint32_t);
                }

                #if defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
                // Flush data for DMA
                SCB_InvalidateDCache_by_Addr(buffer->data, size);
                #endif

                MAIN_FB()->bpp = size;
            }

            // Clean trailing data after 0xFFD9 at the end of the jpeg byte stream.
            MAIN_FB()->bpp = jpeg_clean_trailing_bytes(MAIN_FB()->bpp, buffer->data);
            break;
        default:
            break;
    }

    // Finally, return an image object.

    // Set the user image.
    if (image != NULL) {
        image->w = MAIN_FB()->w;
        image->h = MAIN_FB()->h;
        image->bpp = MAIN_FB()->bpp;
        image->data = buffer->data;
    }

    return 0;
}
