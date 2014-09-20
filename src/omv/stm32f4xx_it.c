/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Interrupt handlers.
 *
 */
#include <stdio.h>
#include <usbd_core.h>
#include <stm32f4xx_exti.h>
#define UNUSED(x) x __attribute__((unused))
#define BREAK() __asm__ volatile ("BKPT");
extern USB_OTG_CORE_HANDLE USB_OTG_Core;
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    BREAK();
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
    UNUSED(volatile uint32_t r0);
    UNUSED(volatile uint32_t r1);
    UNUSED(volatile uint32_t r2);
    UNUSED(volatile uint32_t r3);
    UNUSED(volatile uint32_t r12);
    UNUSED(volatile uint32_t lr); /* Link register. */
    UNUSED(volatile uint32_t pc); /* Program counter. */
    UNUSED(volatile uint32_t psr);/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    BREAK();
}

//static void HardFault_Handler( void ) __attribute__( ( naked ) );
void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    BREAK();
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    BREAK();
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    BREAK();
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
    BREAK();
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
    extern void pendsv_isr_handler(void);
    pendsv_isr_handler();
}

void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_Core.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_Core);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
}

void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_Core);
}
