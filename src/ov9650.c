#include <stdint.h>
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
#include "systick.h"
#include "ov9650_regs.h"
#define NUM_BR_LEVELS       7
#define DCMI_DR_ADDRESS     (DCMI_BASE + 0x28)
#define BREAK() __asm__ volatile ("BKPT")
static volatile int frame_ready = 0;

static uint8_t ov9650_init_regs[][2] = {
    {REG_BLUE,   0x80},
    {REG_RED,    0x80},

    /* See Implementation Guide */
    {REG_COM2,   0x01},  /*  Output drive x2 */
    {REG_COM5,   0x00},  /*  System clock  */
    {REG_CLKRC,  0x81},  /*  Clock control 30 FPS*/
    {REG_MVFP,   0x00},  /*  Mirror/VFlip */

    /* Default QQVGA-RGB565 */
    {REG_COM7,   0x14},  /*  QVGA/RGB565 */    
    {REG_COM1,   0x24},  /*  QQVGA/Skip Option */
    {REG_COM3,   0x04},  /*  Vario Pixels */
    {REG_COM4,   0x80},  /*  Vario Pixels */
    {REG_COM15,  0xD0},  /*  Output range 0x00-0xFF/RGB565*/
 
    /* Dummy pixels settings */
    {REG_EXHCH,  0x00},  /*  Dummy Pixel Insert MSB */
    {REG_EXHCL,  0x00},  /*  Dummy Pixel Insert LSB */

    {REG_ADVFH,  0x00},  /*  Dummy Pixel Insert MSB */
    {REG_ADVFL,  0x00},  /*  Dummy Pixel Insert LSB */

    /* See Implementation Guide Section 3.4.1.2 */
    {REG_COM8,   0xA3}, /* Enable Fast Mode AEC/Enable Banding Filter/AGC/AWB/AEC */
    {0x60,       0x8C}, /* Normal AWB, 0x0C for Advanced AWB */
    {REG_AEW,    0x74}, /* AGC/AEC Threshold Upper Limit */
    {REG_AEB,    0x68}, /* AGC/AEC Threshold Lower Limit */
    {REG_VPT,    0xC3}, /* Fast AEC operating region */

    /* See OV9650 Implementation Guide */
//    {REG_CHLF,   0xE2}, /* External Regulator */
//    {REG_GRCOM,  0x3F}, /* Analog BLC/External Regulator */

    /* See OV9650 Implementation Guide */
    {REG_COM11,  0x01}, /* Automatic/Manual Banding Filter */
    {REG_MBD,    0x1a}, /* Manual banding filter LSB */
    {REG_COM12,  0x04}, /* HREF options/ UV average  */
    {REG_COM9,   0x58}, /* Gain ceiling [6:4]/Over-Exposure */
    {REG_COM16,  0x02}, /* Color matrix coeff double option */
    {REG_COM13,  0x10}, /* Gamma/Colour Matrix/UV delay */
    {REG_COM23,  0x00}, /* Disable Color bar/Analog Color Gain */
    {REG_PSHFT,  0x00}, /* Pixel delay after HREF  */
    {REG_COM10,  0x00}, /* Slave mode, HREF vs HSYNC, signals negate */
    {REG_EDGE,   0xa6}, /* Edge enhancement treshhold and factor */
    {REG_COM6,   0x43}, /* HREF & ADBLC options */
    {REG_COM22,  0x00}, /* Edge enhancement/Denoising */

//  {REG_AECH,   0x00}, /* Exposure Value MSB */
//  {REG_AECHM,  0x00}, /* Exposure Value LSB */

    #if 0
    /* Windowing Settings */
    {REG_HSTART, 0x1d},  /*  Horiz start high bits  */
    {REG_HSTOP,  0xbd},  /*  Horiz stop high bits  */
    {REG_HREF,   0xbf},  /*  HREF pieces  */
    
    {REG_VSTART, 0x00},  /*  Vert start high bits  */
    {REG_VSTOP,  0x80},  /*  Vert stop high bits  */
    {REG_VREF,   0x12},  /*  Pieces of GAIN, VSTART, VSTOP  */
    #endif 

    /* Gamma curve P */
    {0x6C,  0x40},
    {0x6d,  0x30},
    {0x6e,  0x4b},
    {0x6f,  0x60},
    {0x70,  0x70},
    {0x71,  0x70},
    {0x72,  0x70},
    {0x73,  0x70},
    {0x74,  0x60},
    {0x75,  0x60},
    {0x76,  0x50},
    {0x77,  0x48},
    {0x78,  0x3a},
    {0x79,  0x2e},
    {0x7a,  0x28},
    {0x7b,  0x22},

    /* Gamma curve T */
    {0x7c,  0x04},
    {0x7d,  0x07},
    {0x7e,  0x10},
    {0x7f,  0x28},
    {0x80,  0x36},
    {0x81,  0x44},
    {0x82,  0x52},
    {0x83,  0x60},
    {0x84,  0x6c},
    {0x85,  0x78},
    {0x86,  0x8c},
    {0x87,  0x9e},
    {0x88,  0xbb},
    {0x89,  0xd2},
    {0x8a,  0xe6},

    /* Reserved Registers, see OV965x App Note */
    {0x16,  0x06}, 
    {0x34,  0xbf}, 
    //{0xa8,  0x80},/* this doesn't work with QQCIF/QCIF */
    {0x96,  0x04}, 
    {0x8e,  0x00}, 
    {0x8b,  0x06}, 
    {0x35,  0x91}, 
    {0x94,  0x88},
    {0x95,  0x88},
    {0xa9,  0xb8},
    {0xaa,  0x92},
    {0xab,  0x0a},
    {0x5c,  0x96}, 
    {0x5d,  0x96},
    {0x5e,  0x10},
    {0x59,  0xeb},
    {0x5a,  0x9c},
    {0x5b,  0x55},

    /* NULL reg */
    {0x00,  0x00}
};

static uint8_t ov9650_rgb565_regs[][2] = {
    /* See Implementation Guide */
    {REG_COM3,   0x04},  /*  Vario Pixels */
    {REG_COM4,   0x80},  /*  Vario Pixels */
    {REG_COM15,  0xD0},  /*  Output range 0x00-0xFF/RGB565*/
      
    /* See Implementation Guide Section 3.4.1.2 */
    {REG_OFON,   0x43},  /*  Power down register  */
    {REG_ACOM38, 0x12},  /*  reserved  */
    {REG_ADC,    0x00},  /*  reserved  */
    {REG_RSVD35, 0x81},  /*  reserved  */
    
    /* YUV fmt /Special Effects Controls */
    {REG_TSLB,   0x01},  /*  YUVU/DBLC Enable */
    {REG_MANU,   0x80},  /*  Manual U */
    {REG_MANV,   0x80},  /*  Manual V */

    /* RGB color matrix */
    {REG_MTX1,   0x71},
    {REG_MTX2,   0x3e},
    {REG_MTX3,   0x0c},

    {REG_MTX4,   0x33},
    {REG_MTX5,   0x72},
    {REG_MTX6,   0x00},

    {REG_MTX7,   0x2b},
    {REG_MTX8,   0x66},
    {REG_MTX9,   0xd2},
    {REG_MTXS,   0x65},

    /* NULL reg */
    {0x00,  0x00}
};

static uint8_t ov9650_yuv422_regs[][2] = {
    /* See Implementation Guide */
    {REG_COM3,   0x04},  /*  Vario Pixels */
    {REG_COM4,   0x80},  /*  Vario Pixels */
    {REG_COM15,  0xC0},  /*  Output range 0x00-0xFF  */
 
    /* See Implementation Guide Section 3.4.1.2 */
    {REG_OFON,   0x50},  /*  Power down register  */
    {REG_ACOM38, 0x12},  /*  reserved  */
    {REG_ADC,    0x00},  /*  reserved  */
    {REG_RSVD35, 0x81},  /*  reserved  */

    /* YUV fmt /Special Effects Controls */
    {REG_TSLB,   0x01},  /*  YUVU/DBLC Enable */
    {REG_MANU,   0x80},  /*  Manual U */
    {REG_MANV,   0x80},  /*  Manual V */

    /* YUV color matrix */
    {REG_MTX1,   0x3a},
    {REG_MTX2,   0x3d},
    {REG_MTX3,   0x03},

    {REG_MTX4,   0x12},
    {REG_MTX5,   0x26},
    {REG_MTX6,   0x38},

    {REG_MTX7,   0x40},
    {REG_MTX8,   0x40},
    {REG_MTX9,   0x40},
    {REG_MTXS,   0x0d},

    /* NULL reg */
    {0x00,  0x00}
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

void delay(uint32_t ntime)
{
    uint32_t x;
    while (ntime) {
        for (x=0; x<10000; x--);        
        ntime--;
    }
}

/*
   TIM3 Configuration In this example TIM3 input clock (TIM3CLK) is 
   set to 2 * APB1 clock (PCLK1), since APB1 prescaler is different from ABP2.
     TIM3CLK = 2 * PCLK1  (PCLK1 = HCLK / 4)
     TIM3CLK = 2 * HCLK/4 = HCLK / 2 
     TIM3CLK = 168MHz / 2 = 84MHz
         
   To get TIM3 counter clock at x MHz, the prescaler is computed as follows:
      Prescaler = (TIM3CLK / TIM3 counter clock) - 1
      Prescaler = (84 MHz / x MHz) - 1
                                             
   To get TIM3 output clock at 30 KHz, the period (ARR)) is computed as follows:
      ARR = (TIM3 counter clock / TIM3 output clock) - 1
      ARR = 21 MHz/ 30KHz = 669 
                 
   TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR)* 100 = 50%
 */   
#define PWM_TIMER       TIM2
#define PWM_TIMER_CHAN  TIM2
static void extclk_config(int frequency)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /* TIM channel GPIO configuration */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 

    /* Connect TIM pins to AF */  
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_TIM2);

    /* Calculate the prescaler value */ 
//    int tclk  = 84000000;
//    int prescaler =(uint16_t) ((SystemCoreClock/2) / tclk) - 1;

    int tclk  = 168000000;
    int prescaler =(uint16_t) (SystemCoreClock / tclk) - 1;

    //period must be even
    int period = (tclk / frequency)-1;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = period;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel2 */
    TIM_OCInitStructure.TIM_Pulse = period/2;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    /* TIM3 enable counter */
    TIM_Cmd(TIM2, ENABLE);
    TIM_CtrlPWMOutputs(TIM2, ENABLE);
}

static int dcmi_config()
{
    DCMI_InitTypeDef DCMI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
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
    DCMI_InitStructure.DCMI_VSPolarity  = DCMI_VSPolarity_High;
    DCMI_InitStructure.DCMI_HSPolarity  = DCMI_HSPolarity_Low;

    /* Sample data on rising edge of PCK */
    DCMI_InitStructure.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising; 
    DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;

    /* Capture 8 bits on every pixel clock */
    DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
    /* Init DCMI */ 
    DCMI_Init(&DCMI_InitStructure);

#if 0
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

int dma_config(uint8_t *buffer, uint32_t size)
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

int ov9650_init(struct ov9650_handle *ov9650) 
{
    int i=0;
    uint8_t (*regs)[2];

    /* Initialize SCCB interface */
    SCCB_Init();
    systick_sleep(100);

    /* Configure the external clock (XCLK) */
    extclk_config(24000000);
    systick_sleep(100);

    /* Configure the DCMI interface */
    dcmi_config();
    
    //ov9650_reset(ov9650);

    /* Write initial general sensor registers */
    i=0;
    regs = ov9650_init_regs;
    while (regs[i][0]) {
        SCCB_Write(regs[i][0], regs[i][1]);
        while (SCCB_Read(regs[i][0]) != regs[i][1]) {
            SCCB_Write(regs[i][0], regs[i][1]);
        }  
        i++;
    }

    bzero(ov9650, sizeof(struct ov9650_handle));

    /* read sensor id */
    ov9650->id.MIDH = SCCB_Read(REG_MIDH);
    ov9650->id.MIDL = SCCB_Read(REG_MIDL);
    ov9650->id.PID  = SCCB_Read(REG_PID);
    ov9650->id.VER  = SCCB_Read(REG_VER);
    return 0;
}

void ov9650_reset(struct ov9650_handle *ov9650)
{
    SCCB_Write(REG_COM7, 0x80);
    systick_sleep(500);
}


int ov9650_set_pixformat(struct ov9650_handle *ov9650, enum ov9650_pixformat pixformat)
{
    int i=0;
    uint8_t (*regs)[2];
    struct frame_buffer *fb = &ov9650->frame_buffer;
    uint8_t com7=0x00; /* framesize/RGB */

    ov9650->pixformat = pixformat;
    switch (pixformat) {
        case PIXFORMAT_RGB565:
            fb->bpp    = 2;
            regs = ov9650_rgb565_regs;
            break;
        case PIXFORMAT_YUV422:
            fb->bpp    = 2;
            regs = ov9650_yuv422_regs;
            break;
        case PIXFORMAT_GRAYSCALE:
            fb->bpp    = 1;
            regs = ov9650_yuv422_regs;
            break;
        default:
            return -1;
    }

    /* set RGB output */
    com7 = SCCB_Read(REG_COM7);
    if (ov9650->pixformat == PIXFORMAT_RGB565) {
        com7 |= REG_COM7_RGB;
    } else {
        com7 &= (~REG_COM7_RGB);
    }
    
    SCCB_Write(REG_COM7, com7);

    /* Write pixel format registers */
    while (regs[i][0]) {
        SCCB_Write(regs[i][0], regs[i][1]);
        while (SCCB_Read(regs[i][0]) != regs[i][1]) {
            SCCB_Write(regs[i][0], regs[i][1]);
        }  
        i++;
    }

    return 0;
}

int ov9650_set_framesize(struct ov9650_handle *ov9650, enum ov9650_framesize framesize)
{
    ov9650->framesize = framesize;
    struct frame_buffer *fb = &ov9650->frame_buffer;

    uint8_t com7=0x00; /* framesize/RGB */
    uint8_t com1=0x00; /* Skip option */

    switch (framesize) {
        case FRAMESIZE_QQCIF:
            fb->width  = 88;
            fb->height = 72;
            com7 = REG_COM7_QCIF;
            com1 = REG_COM1_QQCIF|REG_COM1_SKIP2;
            break;
        case FRAMESIZE_QQVGA:
            fb->width  = 160;
            fb->height = 120;
            com7 = REG_COM7_QVGA;
            com1 = REG_COM1_QQVGA|REG_COM1_SKIP2;
            break;
        case FRAMESIZE_QCIF:
            fb->width  = 176;
            fb->height = 144;
            com7 = REG_COM7_QCIF;
            break;
        default:
            return -1;
    }

    if (ov9650->pixformat == PIXFORMAT_RGB565) {
        com7 |= REG_COM7_RGB;
    }

    SCCB_Write(REG_COM1, com1);
    while (SCCB_Read(REG_COM1) != com1) {
        SCCB_Write(REG_COM1, com1);
    }  

    SCCB_Write(REG_COM7, com7);
    while (SCCB_Read(REG_COM7) != com7) {
        SCCB_Write(REG_COM7, com7);
    }  
//    SCCB_Write(REG_COM1, com1);
//    SCCB_Write(REG_COM7, com7);

    /* realloc frame buffer */
    fb->pixels = realloc(fb->pixels, 
            fb->width * fb->height * 2); /* always use 2 bpp */

    if (fb->pixels == NULL) {
        return -1;
    }

    /* Reconfigure the DMA stream */
    dma_config(fb->pixels, fb->width * fb->height * 2);
    return 0;
}

int ov9650_set_framerate(struct ov9650_handle *ov9650, enum ov9650_framerate framerate)
{
    ov9650->framerate=framerate;

    /* Write framerate register */
    SCCB_Write(REG_CLKRC, framerate);
    while (SCCB_Read(REG_CLKRC) != framerate) {
        SCCB_Write(REG_CLKRC, framerate);
    }  
    return 0;
}   

int ov9650_set_brightness(struct ov9650_handle *ov9650, int level)
{
    int i;
    static uint8_t regs[NUM_BR_LEVELS + 1][3] = {
        { REG_AEW, REG_AEB, REG_VPT },
        { 0x1c, 0x12, 0x50 }, /* -3 */
        { 0x3d, 0x30, 0x71 }, /* -2 */
        { 0x50, 0x44, 0x92 }, /* -1 */
        { 0x70, 0x64, 0xc3 }, /*  0 */
        { 0x90, 0x84, 0xd4 }, /* +1 */
        { 0xc4, 0xbf, 0xf9 }, /* +2 */
        { 0xd8, 0xd0, 0xfa }, /* +3 */
    };

    level += (NUM_BR_LEVELS / 2 + 1);
    if (level < 0 || level > NUM_BR_LEVELS) {
        return -1;
    }

    for (i=0; i<3; i++) {
        SCCB_Write(regs[0][i], regs[level][i]);
    }

    return 0;
}

int ov9650_set_exposure(struct ov9650_handle *ov9650, uint16_t exposure)
{
   uint8_t val;
   val = SCCB_Read(REG_COM1);

   /* exposure [1:0] */
   SCCB_Write(REG_COM1, val | (exposure&0x03));

   /* exposure [9:2] */
   SCCB_Write(REG_AECH, ((exposure>>2)&0xFF));

   /* exposure [15:10] */
   SCCB_Write(REG_AECHM, ((exposure>>10)&0x3F));
    
   return 0;
}

int ov9650_snapshot(struct ov9650_handle *ov9650)
{
    /* clear frame_ready flag */
    frame_ready = 0;

    /* re-enable DCMI interface */
    DCMI_CaptureCmd(ENABLE);

    /* wait for dma transfer to finish */
    while (!frame_ready);

    /* wait for DCMI to be disabled */
    while (DCMI->CR & DCMI_CR_CAPTURE);

    if (ov9650->pixformat == PIXFORMAT_GRAYSCALE) {
        int i;
        struct frame_buffer *fb = &ov9650->frame_buffer;
        /* extract Y channel */
        for (i=0; i<(fb->width * fb->height); i++) {
            fb->pixels[i] = fb->pixels[i*2]; 
        }
    }
    return 0;
}


