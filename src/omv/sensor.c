#include <stdlib.h>
#include <string.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_dcmi.h>
#include "sccb.h"
#include "ov9650.h"
#include "ov2640.h"
#include "systick.h"
#include "sensor.h"
#include "framebuffer.h"

#define REG_PID        0x0A
#define REG_VER        0x0B

#define REG_MIDH       0x1C
#define REG_MIDL       0x1D

#define OV9650_PID     0x96
#define OV2640_PID     0x26
#define BREAK()     __asm__ volatile ("BKPT")
#define DCMI_DR_ADDRESS     (DCMI_BASE + 0x28)

struct sensor_dev sensor;
static volatile int frame_ready = 0;

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

/* IRQ Handlers */
void DCMI_IRQHandler(void)
{
    if (DCMI_GetITStatus(DCMI_IT_VSYNC)) {
        DCMI_ClearITPendingBit(DCMI_IT_VSYNC);
    } else if (DCMI_GetITStatus(DCMI_IT_LINE)) {
        DCMI_ClearITPendingBit(DCMI_IT_LINE);
    } else if (DCMI_GetITStatus(DCMI_IT_FRAME)) {
        DCMI_ClearITPendingBit(DCMI_IT_FRAME);
        BREAK();
    } else if (DCMI_GetITStatus(DCMI_IT_OVF)) {
        DCMI_ClearITPendingBit(DCMI_IT_OVF);
        BREAK();
    } else if (DCMI_GetITStatus(DCMI_IT_ERR)) {
        DCMI_ClearITPendingBit(DCMI_IT_ERR);
        BREAK();
    }
}

void DMA2_Stream1_IRQHandler(void)
{
    /* DMA Transfer Complete Interrupt */
    if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1)) {
        /* clear DMA TCIF pending interrupt */
        DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
        /* set frame ready flag */
        frame_ready = 1;
    } else if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_HTIF1)) {
        DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_HTIF1);
    } else if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_TEIF1)) {
        DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TEIF1);
        BREAK();
    } else if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_DMEIF1)) {
        DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_DMEIF1);
        BREAK();
    } else if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_FEIF1)) {
        DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_FEIF1);
        BREAK();
    }
}

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
    GPIO_InitTypeDef    GPIO_InitStructure;
    TIM_OCInitTypeDef   TIM_OCInitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef   RCC_Clocks;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    /* TIM channel GPIO configuration */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* Connect TIM pins to AF */
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);

    /* Read Core Clocks */
    RCC_GetClocksFreq(&RCC_Clocks);

    /* set TCLK to HCLK (PCLK2 * 2) */
    int tclk  = RCC_Clocks.PCLK2_Frequency * 2;

    /* No prescalar */
    int prescaler = (uint16_t) (RCC_Clocks.SYSCLK_Frequency / tclk) - 1;

    /* Period should be even */
    int period = (tclk / frequency)-1;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = period;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel2 */
    TIM_OCInitStructure.TIM_Pulse = period/2;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    /* TIM1 enable counter */
    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

static int dcmi_config()
{
    DCMI_InitTypeDef DCMI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /*** DCMI GPIO configuration ***/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE |
                           RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOA, ENABLE);
    /* Connect DCMI pins to AF13 */
    /* D0..D7 */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_DCMI);

    /* VSYNC, HSYNC, PCLK */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_DCMI);

    /* DCMI GPIO configuration */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    //GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

    /* D0,D1 (PC6/7) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* D2,D3,D4,D6,D7 (E0/1/4/5/6) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |
                                  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* D5,VSYNC (PB6/7) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* HSYNC,PCLK (PA4/6) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*** DCMI Configuration ***/
    DCMI_DeInit();
    DCMI_Cmd(DISABLE);

    /* Enable DCMI clock */
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);

    /* Configure capture mode SnapShot/Continuous */
    DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_SnapShot;

    /* Hardware synchronization via VSYNC/HSYNC/PCLK lines */
    DCMI_InitStructure.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;

    /* Active VS/HS clocks*/
    DCMI_InitStructure.DCMI_VSPolarity  = sensor.vsync_pol;
    DCMI_InitStructure.DCMI_HSPolarity  = sensor.hsync_pol;

    /* Sample data on rising edge of PCK */
    DCMI_InitStructure.DCMI_PCKPolarity = sensor.pixck_pol;
    DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;

    /* Capture 8 bits on every pixel clock */
    DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
    /* Init DCMI */
    DCMI_Init(&DCMI_InitStructure);

#if 0
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure DCMI Interrupts */
    DCMI_ITConfig(DCMI_IT_OVF, ENABLE);
    DCMI_ITConfig(DCMI_IT_ERR, ENABLE);
    //DCMI_ITConfig(DCMI_IT_FRAME, ENABLE);
    //DCMI_ITConfig(DCMI_IT_LINE, ENABLE);
    //DCMI_ITConfig(DCMI_IT_VSYNC, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

    /* Enable DCMI Perphieral */
    DCMI_Cmd(ENABLE);
    return 0;
}

static int dma_config(uint8_t *buffer, uint32_t size)
{
    DMA_InitTypeDef  DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable DMA2 clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_DeInit(DMA2_Stream1);
    DMA_Cmd(DMA2_Stream1, DISABLE);

    /* DMA2 Stream1 Configuration */
    DMA_InitStructure.DMA_Channel = DMA_Channel_1;

    /* DMA direction peripheral to memory */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;

    /* Number of data items to be transferred in multiples of (Mburst beat*(Msize)/(Psize))*/
    DMA_InitStructure.DMA_BufferSize = size/4;

    /* Base memory and peripheral addresses */
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) buffer;
    DMA_InitStructure.DMA_PeripheralBaseAddr = DCMI_DR_ADDRESS;

    /* Memory and peripheral address increments */
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

    /* Set Msize and Psize to one word */
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;

    /* Configure circular mode for DMA buffer */
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;

    /* Enable FIFO with threshold of 16 bytes */
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;

    /* Set burst mode, Mburst is 4 beats (16 bytes each) */
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    DMA_Init(DMA2_Stream1, &DMA_InitStructure);

    /* Enable DMA interrupts */
    DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE);
//    DMA_ITConfig(DMA2_Stream1, DMA_IT_HT, ENABLE);
//    DMA_ITConfig(DMA2_Stream1, DMA_IT_TE, ENABLE);
//    DMA_ITConfig(DMA2_Stream1, DMA_IT_FE, ENABLE);

    /* DMA Stream enable */
    DMA_Cmd(DMA2_Stream1, ENABLE);

    int dma_timeout = 10000;
    while ((DMA_GetCmdStatus(DMA2_Stream1) != ENABLE) && (--dma_timeout > 0));

    if (dma_timeout == 0) {
        return -1;
    }

    /* Enable the DMA Stream IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    return 0;
}

int sensor_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /* RESET/PWDN GPIO configuration */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

    /* Configure the RESET GPIO pin */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure the PWDN GPIO pin */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Do a power cycle */
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    systick_sleep(100);

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
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

    GPIO_SetBits(GPIOA, GPIO_Pin_10);
    systick_sleep(10);

    GPIO_ResetBits(GPIOA, GPIO_Pin_10);
    systick_sleep(10);

    /* Check if we can read PID */
    if (SCCB_Read(REG_PID) == 255) {
        /* Sensor is held in reset, so reset is active high */
        sensor.reset_pol = ACTIVE_LOW;

        GPIO_ResetBits(GPIOA, GPIO_Pin_10);
        systick_sleep(10);

        GPIO_SetBits(GPIOA, GPIO_Pin_10);
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

    /* Configure the DCMI interface. This should be called
       after ovxxx_init to set VSYNC/HSYNC/PCLK polarities */
    dcmi_config();
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
            GPIO_SetBits(GPIOA, GPIO_Pin_10);
            systick_sleep(10);

            GPIO_ResetBits(GPIOA, GPIO_Pin_10);
            systick_sleep(10);
           break;
       case ACTIVE_LOW:
            GPIO_ResetBits(GPIOA, GPIO_Pin_10);
            systick_sleep(10);

            GPIO_SetBits(GPIOA, GPIO_Pin_10);
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
    /* clear frame_ready flag */
    frame_ready = 0;

    /* re-enable DCMI interface */
    DCMI_CaptureCmd(ENABLE);

    /* wait for dma transfer to finish */
    while (!frame_ready);

    /* wait for DCMI to be disabled */
    while (DCMI->CR & DCMI_CR_CAPTURE);

    if (sensor.pixformat == PIXFORMAT_GRAYSCALE) {
        int i;
        /* extract Y channel */
        for (i=0; i<(fb->w * fb->h); i++) {
            fb->pixels[i] = fb->pixels[i*2+(fb->w * fb->h)];
        }
    }

    image->w = fb->w;
    image->h = fb->h;
    image->bpp = fb->bpp;
    image->pixels = fb->pixels;
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

    if (pixformat==PIXFORMAT_GRAYSCALE) {
        dma_config(fb->pixels+(fb->w * fb->h), fb->w * fb->h * 2);
    } else {
        dma_config(fb->pixels, fb->w * fb->h * 2);
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

    /* Reconfigure the DMA stream */
    dma_config(fb->pixels, fb->w * fb->h * 2);

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
    return DMA_GetCurrDataCounter(DMA2_Stream1);
}
