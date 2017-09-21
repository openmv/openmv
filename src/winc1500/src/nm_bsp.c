#include STM32_HAL_H
#include "conf_winc.h"
#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "extint.h"
#include "systick.h"

static tpfNmBspIsr gpfIsr;
static const pin_obj_t *PIN_CS  = &(WINC_PIN_CS);
static const pin_obj_t *PIN_EN  = &(WINC_PIN_EN);
static const pin_obj_t *PIN_RST = &(WINC_PIN_RST);
static const pin_obj_t *PIN_IRQ = &(WINC_PIN_IRQ);
static const mp_obj_fun_builtin_fixed_t irq_callback_obj;

/*
*	@fn		nm_bsp_init
*	@brief	Initialize BSP
*	@return	0 in case of success and -1 in case of failure
*/
sint8 nm_bsp_init(void)
{
	gpfIsr = NULL;

    // configure wlan CS and EN pins
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Alternate = 0;

    GPIO_InitStructure.Pin = PIN_CS->pin_mask;
    HAL_GPIO_Init(PIN_CS->gpio, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = PIN_EN->pin_mask;
    HAL_GPIO_Init(PIN_EN->gpio, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = PIN_RST->pin_mask;
    HAL_GPIO_Init(PIN_RST->gpio, &GPIO_InitStructure);

    HAL_GPIO_WritePin(PIN_CS->gpio, PIN_CS->pin_mask, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_EN->gpio, PIN_EN->pin_mask, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_RST->gpio, PIN_RST->pin_mask, GPIO_PIN_SET);

	/* Perform chip reset. */
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
    HAL_GPIO_WritePin(PIN_EN->gpio, PIN_EN->pin_mask, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PIN_RST->gpio, PIN_RST->pin_mask, GPIO_PIN_RESET);
	nm_bsp_sleep(100);
    HAL_GPIO_WritePin(PIN_EN->gpio, PIN_EN->pin_mask, GPIO_PIN_SET);
	nm_bsp_sleep(100);
    HAL_GPIO_WritePin(PIN_RST->gpio, PIN_RST->pin_mask, GPIO_PIN_SET);
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

/*
*	@fn		nm_bsp_register_isr
*	@brief	Register interrupt service routine
*	@param[IN]	pfIsr
*				Pointer to ISR handler
*/
void nm_bsp_register_isr(tpfNmBspIsr pfIsr)
{
	gpfIsr = pfIsr;
    // register EXTI
    extint_register((mp_obj_t)PIN_IRQ, GPIO_MODE_IT_FALLING, GPIO_PULLUP, (mp_obj_t)&irq_callback_obj, true);
    extint_enable(PIN_IRQ->pin);
}

/*
*	@fn		nm_bsp_interrupt_ctrl
*	@brief	Enable/Disable interrupts
*	@param[IN]	u8Enable
*				'0' disable interrupts. '1' enable interrupts
*/
void nm_bsp_interrupt_ctrl(uint8 u8Enable)
{
	if (u8Enable) {
        extint_enable(PIN_IRQ->pin);
	} else {
        extint_disable(PIN_IRQ->pin);
	}
}

static mp_obj_t irq_callback(mp_obj_t line) {
    if (gpfIsr) {
        gpfIsr();
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(irq_callback_obj, irq_callback);
