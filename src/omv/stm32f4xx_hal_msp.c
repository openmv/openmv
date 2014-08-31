#include "pincfg.h"
#include <stm32f4xx_hal.h>

/* LED GPIOs */
const gpio_t led_pins[] = {
    {LED_PORT, LED_RED_PIN},
    {LED_PORT, LED_GREEN_PIN},
    {LED_PORT, LED_BLUE_PIN},
#ifdef OPENMV2
    {LED_PORT, LED_IR_PIN},
#endif
};

/* GPIOs */
#ifdef OPENMV1
const gpio_t gpio_pins[] = {
    {GPIOA, GPIO_PIN_8 },
    {GPIOA, GPIO_PIN_15},
    {GPIOC, GPIO_PIN_9 },
    {GPIOC, GPIO_PIN_10},
    {GPIOC, GPIO_PIN_11},
    {GPIOC, GPIO_PIN_12},
    {NULL, 0}
};
#else
const gpio_t gpio_pins[] = {
    {GPIOA, GPIO_PIN_2 },
    {GPIOA, GPIO_PIN_3 },
    {GPIOC, GPIO_PIN_4 },
    {GPIOC, GPIO_PIN_5 },
    {GPIOD, GPIO_PIN_8 },
    {GPIOD, GPIO_PIN_9 },
    {GPIOD, GPIO_PIN_12},
    {GPIOD, GPIO_PIN_13},
    {GPIOE, GPIO_PIN_2 },
    {GPIOE, GPIO_PIN_3 },
    {GPIOE, GPIO_PIN_5 },
    {GPIOE, GPIO_PIN_6 },
    {NULL, 0}
};
#endif

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

#define NUM_LED_PINS    (sizeof(led_pins)/sizeof(led_pins[0]))
#define NUM_DCMI_PINS   (sizeof(dcmi_pins)/sizeof(dcmi_pins[0]))

void SystemClock_Config(void);

void HAL_MspInit(void)
{
    /* Set the system clock */
    SystemClock_Config();

    /* Config Systick */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    /* Enable GPIO clocks */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
#ifdef OPENMV2
    __GPIOF_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();
#endif

    /* Enable DMA clocks */
    __DMA2_CLK_ENABLE();

    /* Conigure DCMI GPIO */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin = DCMI_RESET_PIN;
    HAL_GPIO_Init(DCMI_RESET_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_PWDN_PIN;
    HAL_GPIO_Init(DCMI_PWDN_PORT, &GPIO_InitStructure);

    /* Configure LED GPIO */
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    for (int i=0; i<NUM_LED_PINS; i++) {
        HAL_GPIO_WritePin(led_pins[i].port,
                led_pins[i].pin, GPIO_PIN_SET);
        #ifdef OPENMV2
        if (led_pins[i].pin==LED_IR_PIN) { //IR LED is inverted
            HAL_GPIO_WritePin(led_pins[i].port,
                    led_pins[i].pin, GPIO_PIN_RESET);
        }
        #endif
        GPIO_InitStructure.Pin = led_pins[i].pin;
        HAL_GPIO_Init(led_pins[i].port, &GPIO_InitStructure);
    }

    /* Configure SD CD PIN */
    GPIO_InitStructure.Pin      = SD_CD_PIN;
    GPIO_InitStructure.Pull     = GPIO_NOPULL;
    GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode     = GPIO_MODE_INPUT;
    HAL_GPIO_Init(SD_CD_PORT, &GPIO_InitStructure);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == SCCB_I2C) {
        /* Enable I2C clock */
        SCCB_CLK_ENABLE();

        /* Configure SCCB GPIOs */
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.Pull      = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_LOW;
        GPIO_InitStructure.Mode      = GPIO_MODE_AF_OD;
        GPIO_InitStructure.Alternate = SCCB_AF;

        GPIO_InitStructure.Pin = SCCB_SCL_PIN;
        HAL_GPIO_Init(SCCB_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.Pin = SCCB_SDA_PIN;
        HAL_GPIO_Init(SCCB_PORT, &GPIO_InitStructure);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == SCCB_I2C) {
        SCCB_CLK_DISABLE();
    }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == DCMI_TIM) {
        /* Enable DCMI timer clock */
        DCMI_TIM_CLK_ENABLE();

        /* Timer GPIO configuration */
        GPIO_InitTypeDef  GPIO_InitStructure;
        GPIO_InitStructure.Pin       = DCMI_TIM_PIN;
        GPIO_InitStructure.Pull      = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_LOW;
        GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Alternate = DCMI_TIM_AF;
        HAL_GPIO_Init(DCMI_TIM_PORT, &GPIO_InitStructure);
    }
}

void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
    /* DCMI clock enable */
    __DCMI_CLK_ENABLE();

    /* DCMI GPIOs configuration */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull      = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = GPIO_AF13_DCMI;

    for (int i=0; i<NUM_DCMI_PINS; i++) {
        GPIO_InitStructure.Pin = dcmi_pins[i].pin;
        HAL_GPIO_Init(dcmi_pins[i].port, &GPIO_InitStructure);
    }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStructure;
#ifdef OPENMV1
    if (hspi->Instance == SD_SPI) {
            /* Enable clock */
            SD_SPI_CLK_ENABLE();

            /* Configure SPI GPIOs */
            GPIO_InitStructure.Pull      = GPIO_NOPULL;
            GPIO_InitStructure.Speed     = GPIO_SPEED_MEDIUM;
            GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStructure.Alternate = SD_SPI_AF;

            GPIO_InitStructure.Pin = SD_MOSI_PIN;
            HAL_GPIO_Init(SD_MOSI_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.Pin = SD_MISO_PIN;
            HAL_GPIO_Init(SD_MISO_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.Pin = SD_SCLK_PIN;
            HAL_GPIO_Init(SD_SCLK_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.Pin = SD_CS_PIN;
            GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
            HAL_GPIO_Init(SD_CS_PORT, &GPIO_InitStructure);

            /* De-select the Card: Chip Select high */
            SD_DESELECT();

    } else
#endif
    if (hspi->Instance == WLAN_SPI) {
            /* Enable clock */
            WLAN_SPI_CLK_ENABLE();

            /* Configure WLAN EN */
            GPIO_InitStructure.Pin      = WLAN_EN_PIN;
            GPIO_InitStructure.Pull     = GPIO_PULLDOWN;
            GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
            GPIO_InitStructure.Mode     = GPIO_MODE_OUTPUT_PP;
            HAL_GPIO_Init(WLAN_EN_PORT, &GPIO_InitStructure);

            /* Disable wlan */
            __WLAN_DISABLE();

            /* Configure WLAN IRQ */
            GPIO_InitStructure.Pin      = WLAN_IRQ_PIN;
            GPIO_InitStructure.Pull     = GPIO_PULLUP;
            GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
            GPIO_InitStructure.Mode     = GPIO_MODE_IT_FALLING;
            HAL_GPIO_Init(WLAN_IRQ_PORT, &GPIO_InitStructure);

            /* Configure WLAN SPI GPIOs */
            GPIO_InitStructure.Pull      = GPIO_PULLUP;
            GPIO_InitStructure.Speed     = GPIO_SPEED_MEDIUM;
            GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStructure.Alternate = WLAN_SPI_AF;

            GPIO_InitStructure.Pin = WLAN_MOSI_PIN;
            HAL_GPIO_Init(WLAN_MOSI_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.Pin = WLAN_MISO_PIN;
            HAL_GPIO_Init(WLAN_MISO_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.Pin = WLAN_SCLK_PIN;
            HAL_GPIO_Init(WLAN_SCLK_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.Pin  = WLAN_CS_PIN;
            GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
            HAL_GPIO_Init(WLAN_CS_PORT, &GPIO_InitStructure);

            /* Deselect the CC3K CS */
            WLAN_DESELECT();
    } else if (hspi->Instance == USR_SPI) {
            /* Enable clock */
            USR_SPI_CLK_ENABLE();

            /* Configure SPI GPIOs */
            GPIO_InitStructure.Pull      = GPIO_NOPULL;
            GPIO_InitStructure.Speed     = GPIO_SPEED_LOW;
            GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStructure.Alternate = USR_SPI_AF;

            GPIO_InitStructure.Pin = USR_SCLK_PIN;
            HAL_GPIO_Init(USR_SCLK_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.Pin = USR_MOSI_PIN;
            HAL_GPIO_Init(USR_MOSI_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.Pin = USR_MISO_PIN;
            HAL_GPIO_Init(USR_MISO_PORT, &GPIO_InitStructure);
    }
}

void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    /* Enable SDIO clock */
    __SDIO_CLK_ENABLE();

    /* SDIO GPIOs configuration */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull      = GPIO_NOPULL;
    GPIO_InitStructure.Speed     = GPIO_SPEED_MEDIUM;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = GPIO_AF12_SDIO;

    GPIO_InitStructure.Pin       = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.Pin       = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd)
{
    __SDIO_CLK_DISABLE();
}

#ifdef OPENMV2
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
    /* Enable FMC clock */
    __FMC_CLK_ENABLE();

    /* Enable GPIO clocks */
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();

    /* SDRAM pins assignment
      +-------------------+--------------------+--------------------+
      | FMC_D0 <-> PD14    | FMC_A0  <-> PF0   | FMC_CLK  <-> PG8  |
      | FMC_D1 <-> PD15    | FMC_A1  <-> PF1   | FMC_CLKE <-> PC3  |
      | FMC_D2 <-> PD0     | FMC_A2  <-> PF2   | FMC_WE   <-> PC0  |
      | FMC_D3 <-> PD1     | FMC_A3  <-> PF3   | FMC_CS   <-> PC2  |
      | FMC_D4 <-> PE7     | FMC_A4  <-> PF4   | FMC_DQM  <-> PE0  |
      | FMC_D5 <-> PE8     | FMC_A5  <-> PF5   | FMC_BA0  <-> PG4  |
      | FMC_D6 <-> PE9     | FMC_A6  <-> PF12  | FMC_BA1  <-> PG5  |
      | FMC_D7 <-> PE10    | FMC_A7  <-> PF13  | FMC_RAS  <-> PF11 |
      |                    | FMC_A8  <-> PF14  | FMC_CAS  <-> PG15 |
      |                    | FMC_A9  <-> PF15  |                   |
      |                    | FMC_A10 <-> PG0   |                   |
      |                    | FMC_A11 <-> PG1   |                   |
      +-------------------+--------------------+--------------------+ */
    /* SDRAM GPIO configuration */
    GPIO_InitTypeDef  GPIO_Init_Structure;
    GPIO_Init_Structure.Pull        = GPIO_NOPULL;
    GPIO_Init_Structure.Mode        = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Speed       = GPIO_SPEED_FAST;
    GPIO_Init_Structure.Alternate   = GPIO_AF12_FMC;

    /* GPIOC configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

    /* GPIOD configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

    /* GPIOE configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_8  | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

    /* GPIOF configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2  | GPIO_PIN_3  |
                              GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 |
                              GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

    /* GPIOG configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
                              GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);
}
#endif

void HAL_MspDeInit(void)
{

}
