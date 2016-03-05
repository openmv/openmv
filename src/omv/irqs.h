/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library.
 *
 */
#ifndef __IRQS_H__
#define __IRQS_H__
#include <core_cmFunc.h>

#define IRQ_SYSTICK_PRE_PRI         (0)
#define IRQ_SYSTICK_SUB_PRI         (0)

#define IRQ_FLASH_PRE_PRI           (1)
#define IRQ_FLASH_SUB_PRI           (0)

#define IRQ_DCMI_PRE_PRI            (2)
#define IRQ_DCMI_SUB_PRI            (0)

#define IRQ_DMA21_PRE_PRI           (2)
#define IRQ_DMA21_SUB_PRI           (0)

#define IRQ_SDIO_PRE_PRI            (3)
#define IRQ_SDIO_SUB_PRI            (0)

#define IRQ_DMA23_PRE_PRI           (4)
#define IRQ_DMA23_SUB_PRI           (0)

#define IRQ_OTGFS_PRE_PRI           (5)
#define IRQ_OTGFS_SUB_PRI           (0)

#define IRQ_TIM3_PRE_PRI            (6)
#define IRQ_TIM3_SUB_PRI            (0)

#define IRQ_PENDSV_PRE_PRI          (15)
#define IRQ_PENDSV_SUB_PRI          (15)


// Disable IRQs with a priority higher than or equal to "priority"
static inline void irq_set_base_priority(uint32_t priority) {
    __set_BASEPRI(priority << (8 - __NVIC_PRIO_BITS));
    __DSB(); __ISB(); // Data/Instruction barriers
}

#endif // __IRQS_H__
