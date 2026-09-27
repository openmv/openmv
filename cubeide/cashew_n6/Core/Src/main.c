/*
 * main.c -- cashew camera, standalone bare-metal firmware (no MicroPython / OpenMV).
 *
 * Board : NUCLEO-N657X0-Q (STM32N657X0H3Q, Cortex-M55 at 600 MHz)
 * Camera: STEVAL-66GYMAI (ST VD66GY global shutter, 2-lane MIPI CSI-2 RAW10)
 * Output: ST-LINK virtual COM port (USART1, 921600 8N1), scope pins D2 (kernel) / D3 (busy)
 *
 * Runs as the FSBL image: the boot ROM copies it from external flash into AXISRAM2
 * (0x34180400) and starts it in secure privileged mode; in development boot mode the
 * debugger loads it to the same place.
 *
 * Board safety: this firmware never programs OTP fuses. The Nucleo's VDDIO2 rail (ports
 * O/P, camera reset) is 3.3 V, so its I/O range is left at 3.3 V.
 */
#include <string.h>
#include "main.h"
#include "timebase.h"
#include "uart_log.h"
#include "camera_vd66gy.h"
#include "capture.h"
#include "cashew_app.h"

static void SystemClock_Config(void);
static void GPIO_Init(void);

// ---- tightly coupled memories ------------------------------------------------------------
// Runs from .preinit_array, after the startup code has copied .data (DTCM) and zeroed .bss
// (DTCM) and before main(): copies the ITCM code from its load image and zeroes .sram_bss.
extern uint32_t _sitcm[], _eitcm[], _siitcm[], _ssram_bss[], _esram_bss[];

// memcpy/memset themselves live in ITCM, so the compiler must not turn these loops into calls.
__attribute__((optimize("no-tree-loop-distribute-patterns")))
static void tcm_init(void) {
    MEMSYSCTL->ITCMCR |= MEMSYSCTL_ITCMCR_EN_Msk;       // enabled out of reset on the N6; make sure
    MEMSYSCTL->DTCMCR |= MEMSYSCTL_DTCMCR_EN_Msk;
    const volatile uint32_t *src = _siitcm;
    for (volatile uint32_t *dst = _sitcm; dst < _eitcm;) {
        *dst++ = *src++;
    }
    for (volatile uint32_t *dst = _ssram_bss; dst < _esram_bss;) {
        *dst++ = 0;
    }
    __DSB();
    __ISB();
}

__attribute__((section(".preinit_array"), used))
static void (*const preinit_tcm_init)(void) = tcm_init;

// -------------------------------------------------------------------------------------------
int main(void) {
    SCB_EnableICache();
    SCB_EnableDCache();
    __enable_irq();                     // disabled by Reset_Handler until the ITCM handlers exist

    HAL_Init();
    SystemClock_Config();
    tb_init();
    GPIO_Init();
    ulog_init(LOG_UART_BAUD);

    ulog_puts("\r\n\r\n==== cashew camera: NUCLEO-N657X0-Q + VD66GY, standalone firmware ====\r\n");
    ulog_printf("CPU %lu MHz, build " __DATE__ " " __TIME__ "%s\r\n", (unsigned long) (SystemCoreClock / 1000000u),
                tb_cycle_counter_ok() ? "" : " (cycle counter off: timestamps from SysTick)");

    // Camera first (power, reset, boot), then the CSI-2 receiver, as in ST's examples.
    bool camera_ok = false;
    int e = cam_init();
    if (e != 0) {
        ulog_printf("camera not found (%d): check the FFC cable orientation, and that PA0 (enable) and PO5 (reset) reach the module\r\n", e);
    } else if ((e = cap_init()) != 0) {
        ulog_printf("DCMIPP init failed (%d)\r\n", e);
    } else {
        camera_ok = true;
    }
    if (!camera_ok) {
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
    }
    app_init(camera_ok);

    GPIO_PinState last = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
    uint32_t last_change = HAL_GetTick();
    for (;;) {
        app_poll();
        GPIO_PinState b = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
        if (b != last && (HAL_GetTick() - last_change) > 50) {
            last = b;
            last_change = HAL_GetTick();
            if (b == GPIO_PIN_SET) {
                app_toggle_run();
            }
        }
    }
}

// 600 MHz CPU from PLL1 (HSI 64 MHz / 4 * 75 = 1200 MHz), same as ST's Nucleo templates:
// CPU = IC1 / 2 = 600 MHz, system bus IC2 / 3 = 400 MHz (AXI), HCLK = 200 MHz.
static void SystemClock_Config(void) {
    RCC_OscInitTypeDef osc = { 0 };
    RCC_ClkInitTypeDef clk = { 0 };

    if (HAL_PWREx_ConfigSupply(PWR_EXTERNAL_SOURCE_SUPPLY) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        Error_Handler();
    }

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSIDiv = RCC_HSI_DIV1;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL1.PLLState = RCC_PLL_NONE;
    osc.PLL2.PLLState = RCC_PLL_NONE;
    osc.PLL3.PLLState = RCC_PLL_NONE;
    osc.PLL4.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }

    // If the boot ROM (or a previous run) left the CPU on PLL1, move to HSI while PLL1 changes.
    HAL_RCC_GetClockConfig(&clk);
    if (clk.CPUCLKSource == RCC_CPUCLKSOURCE_IC1 || clk.SYSCLKSource == RCC_SYSCLKSOURCE_IC2_IC6_IC11) {
        clk.ClockType = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK;
        clk.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
        clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
        if (HAL_RCC_ClockConfig(&clk) != HAL_OK) {
            Error_Handler();
        }
    }

    memset(&osc, 0, sizeof(osc));
    osc.OscillatorType = RCC_OSCILLATORTYPE_NONE;
    osc.PLL1.PLLState = RCC_PLL_ON;
    osc.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL1.PLLM = 4;
    osc.PLL1.PLLN = 75;
    osc.PLL1.PLLFractional = 0;
    osc.PLL1.PLLP1 = 1;
    osc.PLL1.PLLP2 = 1;
    osc.PLL2.PLLState = RCC_PLL_NONE;
    osc.PLL3.PLLState = RCC_PLL_NONE;
    osc.PLL4.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }

    memset(&clk, 0, sizeof(clk));
    clk.ClockType = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK5 | RCC_CLOCKTYPE_PCLK4;
    clk.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;
    clk.AHBCLKDivider = RCC_HCLK_DIV2;
    clk.APB1CLKDivider = RCC_APB1_DIV1;
    clk.APB2CLKDivider = RCC_APB2_DIV1;
    clk.APB4CLKDivider = RCC_APB4_DIV1;
    clk.APB5CLKDivider = RCC_APB5_DIV1;
    clk.IC1Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    clk.IC1Selection.ClockDivider = 2;
    clk.IC2Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    clk.IC2Selection.ClockDivider = 3;
    clk.IC6Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    clk.IC6Selection.ClockDivider = 4;
    clk.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    clk.IC11Selection.ClockDivider = 3;
    if (HAL_RCC_ClockConfig(&clk) != HAL_OK) {
        Error_Handler();
    }
}

static void GPIO_Init(void) {
    GPIO_InitTypeDef g = { 0 };

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOO_CLK_ENABLE();

    // Port O is powered from VDDIO2 (3.3 V on this board): mark the supply valid. The I/O
    // range stays at 3.3 V; the 1.8 V range / HSLV fuse must never be used on the Nucleo.
    HAL_PWREx_EnableVddIO2();
    HAL_PWREx_ConfigVddIORange(PWR_VDDIO2, PWR_VDDIO_RANGE_3V3);

    // LEDs off (active low).
    HAL_GPIO_WritePin(GPIOG, LED_GREEN_PIN | LED_BLUE_PIN | LED_RED_PIN, GPIO_PIN_SET);
    g.Pin = LED_GREEN_PIN | LED_BLUE_PIN | LED_RED_PIN;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &g);

    // Scope / ejector outputs, low.
    HAL_GPIO_WritePin(KERNEL_OUT_PORT, KERNEL_OUT_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BUSY_OUT_PORT, BUSY_OUT_PIN, GPIO_PIN_RESET);
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pin = KERNEL_OUT_PIN;
    HAL_GPIO_Init(KERNEL_OUT_PORT, &g);
    g.Pin = BUSY_OUT_PIN;
    HAL_GPIO_Init(BUSY_OUT_PORT, &g);

    // Camera module: enable low (off) and reset low until cam_init().
    HAL_GPIO_WritePin(CAM_EN_PORT, CAM_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CAM_NRST_PORT, CAM_NRST_PIN, GPIO_PIN_RESET);
    g.Speed = GPIO_SPEED_FREQ_LOW;
    g.Pin = CAM_EN_PIN;
    HAL_GPIO_Init(CAM_EN_PORT, &g);
    g.Pin = CAM_NRST_PIN;
    HAL_GPIO_Init(CAM_NRST_PORT, &g);

    // User button (high when pressed, external pull-down).
    g.Pin = BUTTON_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(BUTTON_PORT, &g);
}

void Error_Handler(void) {
    __disable_irq();
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
    for (;;) {
    }
}

// Linked with -nostartfiles (the startup file is ST's, not newlib's crt0): __libc_init_array
// still calls _init/_fini, which crti.o would normally provide.
__attribute__((weak)) void _init(void) {
}

__attribute__((weak)) void _fini(void) {
}

// ST's vd6g driver uses assert(); report instead of pulling in newlib's stdio.
void __assert_func(const char *file, int line, const char *func, const char *expr) {
    ulog_set_blocking(true);
    ulog_printf("assert failed: %s (%s:%d %s)\r\n", expr, file, line, func ? func : "");
    ulog_flush();
    Error_Handler();
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    ulog_printf("HAL assert: %s:%lu\r\n", (const char *) file, (unsigned long) line);
}
#endif
