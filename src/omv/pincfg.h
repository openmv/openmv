#ifndef __PINCFG_H__
#define __PINCFG_H__
#include <stm32f4xx_hal.h>
/* GPIO struct */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} gpio_t;

extern const gpio_t led_pins[];

/* LEDs */
#define LED_PORT                (GPIOD)
#define LED_RED_PIN             (GPIO_PIN_4)
#define LED_GREEN_PIN           (GPIO_PIN_6)
#define LED_BLUE_PIN            (GPIO_PIN_5)
#define LED_ON(gpio)            (gpio.port->BSRRH = gpio.pin)
#define LED_OFF(gpio)           (gpio.port->BSRRL = gpio.pin)

/* SCCB/I2C */
#define SCCB_I2C                (I2C1)
#define SCCB_AF                 (GPIO_AF4_I2C1)
#define SCCB_CLK_ENABLE()       __I2C1_CLK_ENABLE()
#define SCCB_CLK_DISABLE()      __I2C1_CLK_DISABLE()
#define SCCB_PORT               (GPIOB)
#define SCCB_SCL_PIN            (GPIO_PIN_8)
#define SCCB_SDA_PIN            (GPIO_PIN_9)

/* DCMI */
#define DCMI_TIM                (TIM1)
#define DCMI_TIM_PIN            (GPIO_PIN_9)
#define DCMI_TIM_PORT           (GPIOE)
#define DCMI_TIM_AF             (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL        (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()   __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()  __TIM1_CLK_DISABLE()

#define DCMI_RESET_PIN          (GPIO_PIN_10)
#define DCMI_RESET_PORT         (GPIOA)

#define DCMI_PWDN_PIN           (GPIO_PIN_5)
#define DCMI_PWDN_PORT          (GPIOB)

#define DCMI_D0_PIN             (GPIO_PIN_6)
#define DCMI_D1_PIN             (GPIO_PIN_7)
#define DCMI_D2_PIN             (GPIO_PIN_0)
#define DCMI_D3_PIN             (GPIO_PIN_1)
#define DCMI_D4_PIN             (GPIO_PIN_4)
#define DCMI_D5_PIN             (GPIO_PIN_5)
#define DCMI_D6_PIN             (GPIO_PIN_6)
#define DCMI_D7_PIN             (GPIO_PIN_6)

#define DCMI_D0_PORT            (GPIOC)
#define DCMI_D1_PORT            (GPIOC)
#define DCMI_D2_PORT            (GPIOE)
#define DCMI_D3_PORT            (GPIOE)
#define DCMI_D4_PORT            (GPIOE)
#define DCMI_D5_PORT            (GPIOE)
#define DCMI_D6_PORT            (GPIOE)
#define DCMI_D7_PORT            (GPIOB)

#define DCMI_HSYNC_PIN          (GPIO_PIN_7)
#define DCMI_VSYNC_PIN          (GPIO_PIN_4)
#define DCMI_PXCLK_PIN          (GPIO_PIN_6)

#define DCMI_HSYNC_PORT         (GPIOB)
#define DCMI_VSYNC_PORT         (GPIOA)
#define DCMI_PXCLK_PORT         (GPIOA)

#define DCMI_RESET_LOW()        HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_RESET)
#define DCMI_RESET_HIGH()       HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_SET)

#define DCMI_PWDN_LOW()         HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_RESET)
#define DCMI_PWDN_HIGH()        HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_SET)

#endif //__PINCFG_H__
