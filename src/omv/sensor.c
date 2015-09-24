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
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_msp.h>

#include "sccb.h"
#include "ov9650.h"
#include "ov2640.h"
#include "sensor.h"
#include "systick.h"
#include "framebuffer.h"

#define REG_PID        0x0A
#define REG_VER        0x0B

#define REG_MIDH       0x1C
#define REG_MIDL       0x1D

#define OV9650_PID     0x96
#define OV2640_PID     0x26
#ifdef OPENMV1
#define XCLK_FREQ      (12000000)
#else
#define XCLK_FREQ      (13000000)
#endif
#define BREAK()         __asm__ volatile ("BKPT")

struct sensor_dev sensor;
TIM_HandleTypeDef  TIMHandle;
DMA_HandleTypeDef  DMAHandle;
DCMI_HandleTypeDef DCMIHandle;
int usbdbg_is_connected();

const int res_width[] = {
    88,     /* QQCIF */
    160,    /* QQVGA */
    128,    /* QQVGA2*/
    #ifdef OPENMV2
    192,    /* QCIF  */
    #else
    176,    /* QCIF  */
    #endif
    320,    /* QVGA  */
    352,    /* CIF   */
    640,    /* VGA   */
    800,    /* SVGA  */
    1280,   /* SXGA  */
    1600,   /* UXGA  */
};

const int res_height[]= {
    72,     /* QQCIF */
    120,    /* QQVGA */
    160,    /* QQVGA2*/
    144,    /* QCIF  */
    240,    /* QVGA  */
    288,    /* CIF   */
    480,    /* VGA   */
    600,    /* SVGA   */
    1024,   /* SXGA  */
    1200,   /* UXGA  */
};

static void extclk_config(int frequency)
{
    /* TCLK (PCLK2 * 2) */
    int tclk  = HAL_RCC_GetPCLK2Freq() * 2;

    /* SYSCLK/TCLK = No prescaler */
    int prescaler = (uint16_t) (HAL_RCC_GetSysClockFreq()/ tclk) - 1;

    /* Period should be even */
    int period = (tclk / frequency)-1;

    /* Timer base configuration */
    TIMHandle.Instance          = DCMI_TIM;
    TIMHandle.Init.Period       = period;
    TIMHandle.Init.Prescaler    = prescaler;
    TIMHandle.Init.ClockDivision = 0;
    TIMHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;

    /* Timer channel configuration */
    TIM_OC_InitTypeDef TIMOCHandle;
    TIMOCHandle.Pulse       = period/2;
    TIMOCHandle.OCMode      = TIM_OCMODE_PWM1;
    TIMOCHandle.OCPolarity  = TIM_OCPOLARITY_HIGH;
    TIMOCHandle.OCFastMode  = TIM_OCFAST_DISABLE;
    TIMOCHandle.OCIdleState = TIM_OCIDLESTATE_RESET;
    if (HAL_TIM_PWM_Init(&TIMHandle) != HAL_OK) {
        /* Initialization Error */
        BREAK();
    }

    if (HAL_TIM_PWM_ConfigChannel(&TIMHandle, &TIMOCHandle, DCMI_TIM_CHANNEL) != HAL_OK) {
        BREAK();
    }

    if (HAL_TIM_PWM_Start(&TIMHandle, DCMI_TIM_CHANNEL) != HAL_OK) {
        BREAK();
    }
}

static int dcmi_config(uint32_t jpeg_mode)
{
    /* DCMI configuration */
    DCMIHandle.Instance         = DCMI;
    DCMIHandle.Init.VSPolarity  = sensor.vsync_pol;         /* VSYNC clock polarity                 */
    DCMIHandle.Init.HSPolarity  = sensor.hsync_pol;         /* HSYNC clock polarity                 */
    DCMIHandle.Init.PCKPolarity = sensor.pixck_pol;         /* PXCLK clock polarity                 */
    DCMIHandle.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;    /* Enable Hardware synchronization      */
    DCMIHandle.Init.CaptureRate = DCMI_CR_ALL_FRAME;        /* Capture rate all frames              */
    DCMIHandle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B; /* Capture 8 bits on every pixel clock  */
    DCMIHandle.Init.JPEGMode = jpeg_mode;                   /* Set JPEG Mode                        */

    /* Associate the DMA handle to the DCMI handle */
    __HAL_LINKDMA(&DCMIHandle, DMA_Handle, DMAHandle);

    /* Configure and enable DCMI IRQ Channel */
    HAL_NVIC_SetPriority(DCMI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);

    /* Init DCMI */
    if (HAL_DCMI_Init(&DCMIHandle) != HAL_OK) {
        /* Initialization Error */
        return -1;
    }

    return 0;
}

static int dma_config()
{
    /* DMA Stream configuration */
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

    /* Configure and enable DMA IRQ Channel */
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

    /* Initialize the DMA stream */
    if (HAL_DMA_Init(&DMAHandle) != HAL_OK) {
        /* Initialization Error */
        return 1;
    }

    return 0;
}

int sensor_init()
{
    /* Do a power cycle */
    DCMI_PWDN_HIGH();
    systick_sleep(10);

    DCMI_PWDN_LOW();
    systick_sleep(100);

    /* Initialize the SCCB interface */
    SCCB_Init();
    systick_sleep(10);

    /* Configure the sensor external clock (XCLK) to XCLK_FREQ (13MHz).
       Note: The sensor's internal PLL (when CLKRC=0x80) doubles the XCLK_FREQ
             (XCLK=XCLK_FREQ*2), and the unscaled PIXCLK output is XCLK_FREQ*4 */
    extclk_config(XCLK_FREQ);

    /* Uncomment this to pass through the MCO1 clock (HSI=16MHz) this results in a
       64MHz PIXCLK output from the sensor.
       Note: The maximum pixel clock input on the STM32F4xx is 54MHz,
             the STM32F7 can probably handle higher input pixel clock.
       */
    //(void) extclk_config;
    //HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);

    /* Reset the sesnor state */
    memset(&sensor, 0, sizeof(struct sensor_dev));

    /* Some sensors have different reset polarities, and we can't know which sensor
       is connected before initializing SCCB and reading the PID register, which in
       turn requires pulling the sensor out of the reset state. So we try to read a
       register with both polarities to determine line state. */
    sensor.reset_pol = ACTIVE_HIGH;

    DCMI_RESET_HIGH();
    systick_sleep(10);

    DCMI_RESET_LOW();
    systick_sleep(10);

    /* Check if we can read PID */
    if (SCCB_Read(REG_PID) == 255) {
        /* Sensor is held in reset, so reset is active high */
        sensor.reset_pol = ACTIVE_LOW;

        DCMI_RESET_LOW();
        systick_sleep(10);

        DCMI_RESET_HIGH();
        systick_sleep(10);
    }

    /* Read the sensor information */
    sensor.id.MIDH = SCCB_Read(REG_MIDH);
    sensor.id.MIDL = SCCB_Read(REG_MIDL);
    sensor.id.PID  = SCCB_Read(REG_PID);
    sensor.id.VER  = SCCB_Read(REG_VER);

    /* Call the sensor-specific init function */
    switch (sensor.id.PID) {
        case OV9650_PID:
            ov9650_init(&sensor);
            break;
        case OV2640_PID:
            ov2640_init(&sensor);
            break;
        default:
            /* sensor not supported */
            return -1;
    }

    /* Configure the DCMI DMA Stream */
    if (dma_config() != 0) {
        return -2;
    }

    /* Configure the DCMI interface. This should be called
       after ovxxx_init to set VSYNC/HSYNC/PCLK polarities */
    if (dcmi_config(DCMI_JPEG_DISABLE) != 0){
        return -3;
    }

    /* init/re-init mutex */
    mutex_init(&fb->lock);

    // blocks usbdbg until the sensor is configured
    fb->ready=0;
    return 0;
}

int sensor_reset()
{
    /* Reset the sesnor state */
    sensor.pixformat=0xFF;
    sensor.framesize=0xFF;
    sensor.framerate=0xFF;
    sensor.gainceiling=0xFF;

    mutex_lock(&fb->lock);
    fb->ready=0;
    mutex_unlock(&fb->lock);

    /* Call sensor-specific reset function */
    sensor.reset();

    // just in case there's a running DMA request.
    HAL_DMA_Abort(&DMAHandle);
    return 0;
}

int sensor_read_reg(uint8_t reg)
{
    return SCCB_Read(reg);
}

int sensor_write_reg(uint8_t reg, uint8_t val)
{
    return SCCB_Write(reg, val);
}

#define MAX_XFER_SIZE (0xFFFC)

int sensor_snapshot(struct image *image)
{
    volatile uint32_t addr;
    volatile uint16_t length;
    uint32_t snapshot_start;

    addr = (uint32_t) fb->pixels;

    if (sensor.pixformat==PIXFORMAT_JPEG) {
        length = MAX_XFER_SIZE;
    } else {
        length =(fb->w * fb->h * 2)/4;
    }

    /* Lock framebuffer mutex */
    mutex_lock(&fb->lock);

    // Snapshot start tick
    snapshot_start = HAL_GetTick();

    /* Start the DCMI */
    HAL_DCMI_Start_DMA(&DCMIHandle,
            DCMI_MODE_SNAPSHOT, addr, length);

    /* Wait for frame */
    while ((DCMI->CR & DCMI_CR_CAPTURE) != 0) {
        if ((HAL_GetTick() - snapshot_start) >= 1000) {
            // Sensor timeout, most likely a HW issue.
            // unlock fb mutex and abort the DMA request
            mutex_unlock(&fb->lock);
            HAL_DMA_Abort(&DMAHandle);
            return -1;
        }
    }

    if (sensor.pixformat == PIXFORMAT_GRAYSCALE) {
        /* If GRAYSCALE extract Y channel from YUYV */
        for (int i=0; i<(fb->w * fb->h); i++) {
            fb->pixels[i] = fb->pixels[i*2];
        }
    } else if (sensor.pixformat == PIXFORMAT_JPEG) {
        /* The frame is finished, but DMA still waiting
           for data because we set max frame size
           so we need to abort the DMA transfer here */
        HAL_DMA_Abort(&DMAHandle);

        /* Read the number of data items transferred */
        fb->bpp = (MAX_XFER_SIZE - DMAHandle.Instance->NDTR)*4;
    }

    if (image != NULL) {
        image->w = fb->w;
        image->h = fb->h;
        image->bpp = fb->bpp;
        image->pixels = fb->pixels;
    }

    fb->ready = 1;

    /* unlock framebuffer mutex */
    mutex_unlock(&fb->lock);

    while (fb->lock_tried) {
        systick_sleep(2);
    }
    return 0;
}

int sensor_set_pixformat(enum sensor_pixformat pixformat)
{
    uint32_t jpeg_mode = DCMI_JPEG_DISABLE;

    if (sensor.pixformat == pixformat) {
        /* no change */
        return 0;
    }

    mutex_lock(&fb->lock);
    fb->ready = 0;
    mutex_unlock(&fb->lock);

    if (sensor.set_pixformat == NULL
        || sensor.set_pixformat(pixformat) != 0) {
        /* operation not supported */
        return -1;
    }

    /* set pixel format */
    sensor.pixformat = pixformat;

    /* set bytes per pixel */
    switch (pixformat) {
        case PIXFORMAT_GRAYSCALE:
            fb->bpp    = 1;
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            fb->bpp    = 2;
            break;
        case PIXFORMAT_JPEG:
            fb->bpp    = 0;
            break;
        default:
            return -1;
    }

    if (pixformat == PIXFORMAT_JPEG) {
        jpeg_mode = DCMI_JPEG_ENABLE;
    }

    return dcmi_config(jpeg_mode);
}

int sensor_set_framesize(enum sensor_framesize framesize)
{
    if (sensor.framesize == framesize) {
       /* no change */
        return 0;
    }

    mutex_lock(&fb->lock);
    fb->ready = 0;
    mutex_unlock(&fb->lock);

    /* call the sensor specific function */
    if (sensor.set_framesize == NULL
        || sensor.set_framesize(framesize) != 0) {
        /* operation not supported */
        return -1;
    }

    /* set framebuffer size */
    sensor.framesize = framesize;

    /* set framebuffer dimensions */
    switch (framesize) {
        case FRAMESIZE_QQCIF:
        case FRAMESIZE_QQVGA:
        case FRAMESIZE_QQVGA2:
        case FRAMESIZE_QCIF:
        case FRAMESIZE_QVGA:
        case FRAMESIZE_CIF:
        case FRAMESIZE_VGA:
        case FRAMESIZE_SVGA:
        case FRAMESIZE_SXGA:
        case FRAMESIZE_UXGA:
            fb->w =res_width[framesize];
            fb->h =res_height[framesize];
            break;
        default:
            return -1;
    }

    return 0;
}

int sensor_set_framerate(enum sensor_framerate framerate)
{
    if (sensor.framerate == framerate) {
       /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_framerate == NULL
        || sensor.set_framerate(framerate) != 0) {
        /* operation not supported */
        return -1;
    }

    /* set the frame rate */
    sensor.framerate = framerate;

    return 0;
}

int sensor_set_contrast(int level)
{
    if (sensor.set_contrast != NULL) {
        return sensor.set_contrast(level);
    }
    return -1;
}

int sensor_set_brightness(int level)
{
    if (sensor.set_brightness != NULL) {
        return sensor.set_brightness(level);
    }
    return -1;
}

int sensor_set_saturation(int level)
{
    if (sensor.set_saturation != NULL) {
        return sensor.set_saturation(level);
    }
    return -1;
}

int sensor_set_exposure(int exposure)
{
    return 0;
}

int sensor_set_gainceiling(enum sensor_gainceiling gainceiling)
{
    if (sensor.gainceiling == gainceiling) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_gainceiling == NULL
        || sensor.set_gainceiling(gainceiling) != 0) {
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
        || sensor.set_quality(qs) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_colorbar(int enable)
{
    /* call the sensor specific function */
    if (sensor.set_colorbar == NULL
        || sensor.set_colorbar(enable) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}
