/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * STM32 USB helper macros.
 */
#ifndef __OMV_BOOT_STM32_USB_H__
#define __OMV_BOOT_STM32_USB_H__
#include STM32_HAL_H

#if defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
#define USB1_IRQ_Handler    OTG_FS_IRQHandler
#define USB2_IRQ_Handler    OTG_HS_IRQHandler

#define USB_OTG_ENABLE()
#define USB_OTG_PHY_CLK_DISABLE()
#define USB_OTG_PHY_CLK_ENABLE(clkdiv)

#define USB_OTG_CLK_ENABLE() do {                \
        __HAL_RCC_USB_OTG_FS_CLK_ENABLE();       \
        __HAL_RCC_USB_OTG_FS_CLK_SLEEP_ENABLE(); \
} while (0)

#define USB_OTG_CLK_DISABLE() do {                \
        __HAL_RCC_USB_OTG_FS_CLK_DISABLE();       \
        __HAL_RCC_USB_OTG_FS_CLK_SLEEP_DISABLE(); \
} while (0)

#elif defined(STM32N6)
#define USB1_IRQ_Handler    USB1_OTG_HS_IRQHandler
#define USB2_IRQ_Handler    USB2_OTG_HS_IRQHandler

#define USB_OTG_CLK_ENABLE() do {                 \
        __HAL_RCC_USB1_OTG_HS_CLK_ENABLE();       \
        __HAL_RCC_USB1_OTG_HS_CLK_SLEEP_ENABLE(); \
} while (0)

#define USB_OTG_PHY_CLK_ENABLE(clkdiv) do {           \
        USB1_HS_PHYC->USBPHYC_CR &= ~(0x7U << 0x4U);  \
        USB1_HS_PHYC->USBPHYC_CR |= (clkdiv << 0x4U); \
        __HAL_RCC_USB1_OTG_HS_PHY_CLK_ENABLE();       \
        __HAL_RCC_USB1_OTG_HS_PHY_CLK_SLEEP_ENABLE(); \
} while (0)

#define USB_OTG_CLK_DISABLE() do {                 \
        __HAL_RCC_USB1_OTG_HS_CLK_DISABLE();       \
        __HAL_RCC_USB1_OTG_HS_CLK_SLEEP_DISABLE(); \
} while (0)

#define USB_OTG_PHY_CLK_DISABLE() do {                 \
        __HAL_RCC_USB1_OTG_HS_PHY_CLK_DISABLE();       \
        __HAL_RCC_USB1_OTG_HS_PHY_CLK_SLEEP_DISABLE(); \
} while (0)

#define USB_OTG_ENABLE() do {                            \
        USB1_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALEXTOEN; \
        USB1_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALOVAL;   \
} while (0)
#endif
#endif  // __OMV_BOOT_STM32_USB_H__
