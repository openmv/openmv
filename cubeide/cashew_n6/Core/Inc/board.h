/*
 * board.h -- NUCLEO-N657X0-Q (MB1940) pins and settings used by the cashew camera.
 */
#ifndef BOARD_H
#define BOARD_H

// ---- clocks ----------------------------------------------------------------------------
#define CPU_CLOCK_HZ            600000000UL     // PLL1 1200 MHz / IC1 2 (VOS low, Nucleo SMPS)

// ---- user LEDs (active low) and button ---------------------------------------------------
#define LED_GREEN_PORT          GPIOG           // LD6: heartbeat (toggles every 120 frames)
#define LED_GREEN_PIN           GPIO_PIN_0
#define LED_BLUE_PORT           GPIOG           // LD7: on while a kernel was accepted this frame
#define LED_BLUE_PIN            GPIO_PIN_8
#define LED_RED_PORT            GPIOG           // LD5: error (camera, capture)
#define LED_RED_PIN             GPIO_PIN_10
#define BUTTON_PORT             GPIOC           // B1 USER: start / stop the detection loop
#define BUTTON_PIN              GPIO_PIN_13

// ---- scope / ejector outputs on the Arduino header ---------------------------------------
#define KERNEL_OUT_PORT         GPIOD           // D2 = PD0: high for one frame after each
#define KERNEL_OUT_PIN          GPIO_PIN_0      //   frame with an accepted (new) kernel
#define BUSY_OUT_PORT           GPIOE           // D3 = PE9: high while a frame is processed
#define BUSY_OUT_PIN            GPIO_PIN_9

// ---- camera connector (22-pin MIPI CSI-2 FFC) ---------------------------------------
#define CAM_I2C                 I2C2            // PB10 SCL, PB11 SDA, AF4
#define CAM_I2C_SCL_PORT        GPIOB
#define CAM_I2C_SCL_PIN         GPIO_PIN_10
#define CAM_I2C_SDA_PORT        GPIOB
#define CAM_I2C_SDA_PIN         GPIO_PIN_11
#define CAM_I2C_AF              GPIO_AF4_I2C2
#define CAM_I2C_ADDR            (0x10 << 1)     // VD66GY, 8-bit HAL address
// I2C2 kernel clock = HSI 64 MHz. PRESC 1 (31.25 ns), SCLDEL 12, SDADEL 2, SCLH 25, SCLL 47:
// ~380 kHz fast mode.
#define CAM_I2C_TIMING          0x10C2192FUL
#define CAM_EN_PORT             GPIOA           // camera module enable (high = on)
#define CAM_EN_PIN              GPIO_PIN_0
#define CAM_NRST_PORT           GPIOO           // camera reset (low = reset), VDDIO2 domain
#define CAM_NRST_PIN            GPIO_PIN_5
#define CAM_EXT_CLK_HZ          12000000        // oscillator on the STEVAL-66GYMAI module

// ---- virtual COM port (ST-LINK) -----------------------------------------------------------
#define LOG_UART                USART1          // PE5 TX, PE6 RX, AF7
#define LOG_UART_IRQn           USART1_IRQn
#define LOG_UART_BAUD           921600

// ---- interrupt priorities (0 = highest) --------------------------------------------------
#define IRQ_PRI_CAMERA          2               // DCMIPP / CSI frame events (timestamps)
#define IRQ_PRI_UART            6
#define IRQ_PRI_TICK            15

#endif
