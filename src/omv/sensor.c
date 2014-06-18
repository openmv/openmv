#include <stdlib.h>
#include <string.h>
#include "sccb.h"
#include "ov9650.h"
#include "ov2640.h"
#include "systick.h"
#include "sensor.h"
#include "pincfg.h"
#include "framebuffer.h"

#define REG_PID        0x0A
#define REG_VER        0x0B

#define REG_MIDH       0x1C
#define REG_MIDL       0x1D

#define OV9650_PID     0x96
#define OV2640_PID     0x26
#define BREAK()         __asm__ volatile ("BKPT")

struct sensor_dev sensor;
TIM_HandleTypeDef  TIMHandle;
DMA_HandleTypeDef  DMAHandle;
DCMI_HandleTypeDef DCMIHandle;

/* DCMI GPIOs */
static const gpio_t dcmi_pins[] = {
    {DCMI_D0_PORT, DCMI_D0_PIN},
    {DCMI_D1_PORT, DCMI_D1_PIN},
    {DCMI_D2_PORT, DCMI_D2_PIN},
    {DCMI_D3_PORT, DCMI_D3_PIN},
    {DCMI_D4_PORT, DCMI_D4_PIN},
    {DCMI_D5_PORT, DCMI_D5_PIN},
    {DCMI_D6_PORT, DCMI_D6_PIN},
    {DCMI_D7_PORT, DCMI_D7_PIN},
    {DCMI_HSYNC_PORT, DCMI_HSYNC_PIN},
    {DCMI_VSYNC_PORT, DCMI_VSYNC_PIN},
    {DCMI_PXCLK_PORT, DCMI_PXCLK_PIN},
};

#define NUM_PINS        (sizeof(dcmi_pins)/sizeof(dcmi_pins[0]))
#define RESET_LOW()     HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_RESET)
#define RESET_HIGH()    HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_SET)

#define PWDN_LOW()      HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_RESET)
#define PWDN_HIGH()     HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_SET)

const int res_width[] = {
    88,     /* QQCIF */
    160,    /* QQVGA */
    176,    /* QCIF  */
    320,    /* QVGA  */
    352,    /* CIF   */
    640,    /* VGA   */
    1280,   /* SXGA  */
};

const int res_height[]= {
    72,     /* QQCIF */
    120,    /* QQVGA */
    144,    /* QCIF  */
    240,    /* QVGA  */
    288,    /* CIF   */
    480,    /* VGA   */
    1024,   /* SXGA  */
};

/*
   TIM1 input clock (TIM1CLK) is set to 2 * APB2 clock (PCLK2)
     TIM1CLK = 2 * PCLK2 (PCLK2 = HCLK / 2)
     TIM1CLK = 2 * HCLK/2 = HCLK
     TIM1CLK = HCLK (168MHz)

   To get TIM1 counter clock at x MHz, the prescaler is computed as follows:
      Prescaler = (TIM1CLK / TIM1 counter clock) - 1
      Prescaler = (168MHz / xMHz) - 1

   To get TIM1 output clock at 30 KHz, the period (ARR)) is computed as follows:
      ARR = (TIM1 counter clock / TIM1 output clock) - 1
      ARR = 21 MHz/ 30KHz = 669

   TIM1 Channel1 duty cycle = (TIM1_CCR1/ TIM1_ARR)* 100 = 50%
 */
static void extclk_config(int frequency)
{
    /* TCLK (PCLK2 * 2) */
    int tclk  = HAL_RCC_GetPCLK2Freq() * 2;

    /* SYSCLK/TCLK = No prescaler */
    int prescaler = (uint16_t) (HAL_RCC_GetSysClockFreq()/ tclk) - 1;

    /* Period should be even */
    int period = (tclk / frequency)-1;

    //TODO move to MSP
    /* Timer GPIO configuration */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pin       = DCMI_TIM_PIN;
    GPIO_InitStructure.Pull      = GPIO_NOPULL;
    GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = DCMI_TIM_AF;
    HAL_GPIO_Init(DCMI_TIM_PORT, &GPIO_InitStructure);

    /* Enable DCMI timer clock */
    DCMI_TIM_CLK_ENABLE();

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

static int dcmi_config()
{
    /* DCMI clock enable */
    __DCMI_CLK_ENABLE();

    /* DCMI GPIOs configuration */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull      = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = GPIO_AF13_DCMI;

    for (int i=0; i<NUM_PINS; i++) {
        GPIO_InitStructure.Pin = dcmi_pins[i].pin;
        HAL_GPIO_Init(dcmi_pins[i].port, &GPIO_InitStructure);
    }

    /* DCMI configuration */
    DCMIHandle.Instance         = DCMI;
    DCMIHandle.Init.VSPolarity  = sensor.vsync_pol;         /* VSYNC clock polarity                 */
    DCMIHandle.Init.HSPolarity  = sensor.hsync_pol;         /* HSYNC clock polarity                 */
    DCMIHandle.Init.PCKPolarity = sensor.pixck_pol;         /* PXCLK clock polarity                 */
    DCMIHandle.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;    /* Enable Hardware synchronization      */
    DCMIHandle.Init.CaptureRate = DCMI_CR_ALL_FRAME;        /* Capture rate all frames              */
    DCMIHandle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B; /* Capture 8 bits on every pixel clock  */
    DCMIHandle.Init.JPEGMode = DCMI_JPEG_DISABLE;           /* Disable JPEG Mode                    */

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

    __HAL_DCMI_DISABLE_IT(&DCMIHandle, DCMI_IT_LINE);
    __HAL_DCMI_DISABLE_IT(&DCMIHandle, DCMI_IT_VSYNC);
    __HAL_DCMI_DISABLE_IT(&DCMIHandle, DCMI_IT_ERR);
    __HAL_DCMI_DISABLE_IT(&DCMIHandle, DCMI_IT_OVF);

    return 0;
}

static int dma_config()
{
    /* Enable DMA2 clock */
    __DMA2_CLK_ENABLE();

    /* DMA Stream configuration */
    DMAHandle.Instance          = DMA2_Stream1;                 /* Select the DMA instance          */
    DMAHandle.Init.Channel      = DMA_CHANNEL_1;                /* DMA Channel                      */
    DMAHandle.Init.Direction    = DMA_PERIPH_TO_MEMORY;         /* Peripheral to memory transfer    */
    DMAHandle.Init.MemInc       = DMA_MINC_ENABLE;              /* Memory increment mode Enable     */
    DMAHandle.Init.PeriphInc    = DMA_PINC_DISABLE;             /* Peripheral increment mode Enable */
    DMAHandle.Init.PeriphDataAlignment  = DMA_PDATAALIGN_WORD;  /* Peripheral data alignment : Word */
    DMAHandle.Init.MemDataAlignment     = DMA_MDATAALIGN_WORD;  /* Memory data alignment : Word     */
    DMAHandle.Init.Mode          = DMA_CIRCULAR;                /* Circular DMA mode                */
    DMAHandle.Init.Priority      = DMA_PRIORITY_HIGH;           /* Priority level : high            */
    DMAHandle.Init.FIFOMode      = DMA_FIFOMODE_DISABLE;        /* FIFO mode enabled                */
    DMAHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;     /* FIFO threshold full              */
    DMAHandle.Init.MemBurst      = DMA_MBURST_SINGLE;           /* Memory burst                     */
    DMAHandle.Init.PeriphBurst   = DMA_PBURST_SINGLE;           /* Peripheral burst                 */

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
    /* RESET/PWDN GPIO configuration */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;

    /* RESET */
    GPIO_InitStructure.Pin = DCMI_RESET_PIN;
    HAL_GPIO_Init(DCMI_RESET_PORT, &GPIO_InitStructure);

    /* PWDN */
    GPIO_InitStructure.Pin = DCMI_PWDN_PIN;
    HAL_GPIO_Init(DCMI_PWDN_PORT, &GPIO_InitStructure);

    /* Do a power cycle */
    PWDN_HIGH();
    systick_sleep(10);

    PWDN_LOW();
    systick_sleep(100);

    /* Initialize the SCCB interface */
    SCCB_Init();
    systick_sleep(10);

    /* Configure the external clock (XCLK) */
    extclk_config(24000000);
    systick_sleep(10);

    /* Reset the sesnor state */
    memset(&sensor, 0, sizeof(struct sensor_dev));

    /* Some sensors have different reset polarities, and we can't know which sensor
       is connected before initializing SCCB and reading the PID register, which in
       turn requires pulling the sensor out of the reset state. So we try to read a
       register with both polarities to determine line state. */
    sensor.reset_pol = ACTIVE_HIGH;

    RESET_HIGH();
    systick_sleep(10);

    RESET_LOW();
    systick_sleep(10);

    /* Check if we can read PID */
    if (SCCB_Read(REG_PID) == 255) {
        /* Sensor is held in reset, so reset is active high */
        sensor.reset_pol = ACTIVE_LOW;

        RESET_LOW();
        systick_sleep(10);

        RESET_HIGH();
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
        return -1;
    }

    /* Configure the DCMI interface. This should be called
       after ovxxx_init to set VSYNC/HSYNC/PCLK polarities */
    if (dcmi_config() != 0){
        return -1;
    }
    return 0;
}

int sensor_reset()
{
    /* Reset the sesnor state */
    sensor.pixformat=0xFF;
    sensor.framesize=0xFF;
    sensor.framerate=0xFF;
    sensor.gainceiling=0xFF;

    /* Hard reset the sensor */
    switch (sensor.reset_pol) {
        case ACTIVE_HIGH:
            RESET_HIGH();
            systick_sleep(10);

            RESET_LOW();
            systick_sleep(10);
           break;
       case ACTIVE_LOW:
            RESET_LOW();
            systick_sleep(10);

            RESET_HIGH();
            systick_sleep(10);
            break;
    }

    /* Call sensor-specific reset function */
    sensor.reset();
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

int sensor_snapshot(struct image *image)
{
    /* Enable the Frame capture complete interrupt */
    __HAL_DCMI_ENABLE_IT(&DCMIHandle, DCMI_IT_FRAME);

    HAL_DCMI_Start_DMA(&DCMIHandle, DCMI_MODE_SNAPSHOT, (uint32_t) fb->pixels,  (fb->w * fb->h * 2)/4);

    /* Wait for frame */
    while (HAL_DCMI_GetState(&DCMIHandle) == HAL_DCMI_STATE_BUSY);

    if (sensor.pixformat == PIXFORMAT_GRAYSCALE) {
        /* Extract Y channel from YUYV */
        for (int i=0; i<(fb->w * fb->h); i++) {
            fb->pixels[i] = fb->pixels[i*2];
        }
    }

//    image->w = fb->w;
//    image->h = fb->h;
//    image->bpp = fb->bpp;
//    image->pixels = fb->pixels;
    return 0;
}

int sensor_set_pixformat(enum sensor_pixformat pixformat)
{
    if (sensor.pixformat == pixformat) {
        /* no change */
        return 0;
    }

    if (sensor.set_pixformat == NULL
        || sensor.set_pixformat(pixformat) != 0) {
        /* operation not supported */
        return -1;
    }

    /* set pixel format */
    sensor.pixformat = pixformat;

    /* set bytes per pixel */
    switch (pixformat) {
        case PIXFORMAT_RGB565:
            fb->bpp    = 2;
            break;
        case PIXFORMAT_YUV422:
            fb->bpp    = 2;
            break;
        case PIXFORMAT_GRAYSCALE:
            fb->bpp    = 1;
            break;
        default:
            return -1;
    }
    return 0;
}

int sensor_set_framesize(enum sensor_framesize framesize)
{
    if (sensor.framesize == framesize) {
       /* no change */
        return 0;
    }

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
            fb->w = 88;
            fb->h = 72;
            break;
        case FRAMESIZE_QQVGA:
            fb->w = 160;
            fb->h = 120;
            break;
        case FRAMESIZE_QCIF:
            fb->w = 176;
            fb->h = 144;
            break;
        default:
            return -1;
    }

#if 0
    /* This enables croping use it to test bigger frames */
    DCMI_CROPCmd(DISABLE);
    DCMI_CROPInitTypeDef DCMI_CROPInitStructure= {
        .DCMI_HorizontalOffsetCount = 0,
        .DCMI_CaptureCount          = fb->w * 2,
        .DCMI_VerticalStartLine     = 0,
        .DCMI_VerticalLineCount     = fb->h
    };

    DCMI_CROPConfig(&DCMI_CROPInitStructure);
    DCMI_CROPCmd(ENABLE);
#endif
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

int sensor_set_brightness(uint8_t level)
{
    sensor.set_brightness(level);
    return 0;
}

int sensor_set_exposure(uint16_t exposure)
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

int get_bytes()
{
//    return DMA_GetCurrDataCounter(DMA2_Stream1);
    return 0;
}
