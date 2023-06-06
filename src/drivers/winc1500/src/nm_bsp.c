#include STM32_HAL_H
#include "stm32_hal_legacy.h"
#include "conf_winc.h"
#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "extint.h"
#include "systick.h"
#include "omv_boardconfig.h"
#include "omv_gpio.h"

static tpfNmBspIsr gpfIsr;
/*
*	@fn		nm_bsp_init
*	@brief	Initialize BSP
*	@return	0 in case of success and -1 in case of failure
*/
sint8 nm_bsp_init(void)
{
	gpfIsr = NULL;

    // Configure SPI pins
    omv_gpio_config(WINC_SPI_MISO_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(WINC_SPI_MOSI_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(WINC_SPI_SCLK_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_DOWN, OMV_GPIO_SPEED_HIGH, -1);
    omv_gpio_config(WINC_SPI_SSEL_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(WINC_SPI_SSEL_PIN, 1);

    // Configure GPIO pins
    omv_gpio_config(WINC_EN_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(WINC_EN_PIN, 1);

    omv_gpio_config(WINC_RST_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(WINC_RST_PIN, 1);

	// Perform chip reset.
	nm_bsp_reset();

	return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_reset
 *	@brief	Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_bsp_reset(void)
{
    omv_gpio_write(WINC_EN_PIN, 0);
    omv_gpio_write(WINC_RST_PIN, 0);
	nm_bsp_sleep(100);
    omv_gpio_write(WINC_EN_PIN, 1);
	nm_bsp_sleep(100);
    omv_gpio_write(WINC_RST_PIN, 1);
	nm_bsp_sleep(100);
}

extern void systick_sleep(uint32_t ms);
/*
*	@fn		nm_bsp_sleep
*	@brief	Sleep in units of mSec
*	@param[IN]	u32TimeMsec
*				Time in milliseconds
*/
void nm_bsp_sleep(uint32 u32TimeMsec)
{
    systick_sleep(u32TimeMsec);
}

// Pin IRQ handler
static void nm_bsp_extint_callback(omv_gpio_t pin, void *data)
{
    if (gpfIsr) {
        gpfIsr();
    }
}

/*
*	@fn		nm_bsp_register_isr
*	@brief	Register interrupt service routine
*	@param[IN]	pfIsr
*				Pointer to ISR handler
*/
void nm_bsp_register_isr(tpfNmBspIsr pfIsr)
{
	gpfIsr = pfIsr;
    omv_gpio_config(WINC_IRQ_PIN, OMV_GPIO_MODE_IT_FALL, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_irq_register(WINC_IRQ_PIN, nm_bsp_extint_callback, NULL);
    omv_gpio_irq_enable(WINC_IRQ_PIN, true);
}

/*
*	@fn		nm_bsp_interrupt_ctrl
*	@brief	Enable/Disable interrupts
*	@param[IN]	u8Enable
*				'0' disable interrupts. '1' enable interrupts
*/
void nm_bsp_interrupt_ctrl(uint8 enable)
{
    omv_gpio_irq_enable(WINC_IRQ_PIN, enable);
}
