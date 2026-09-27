/*
 * main.h -- cashew camera, standalone firmware for NUCLEO-N657X0-Q + STEVAL-66GYMAI (VD66GY).
 */
#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"
#include "board.h"

// Code that must run from ITCM even when its object file is not listed in the linker script.
#define ITCM_FUNC   __attribute__((section(".itcm_text"), noinline))

void Error_Handler(void) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
