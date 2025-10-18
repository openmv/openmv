/*
 * Copyright (C) 2023-2025 OpenMV, LLC.
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
 * OpenMV QEMU port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__
#include <stdlib.h>

// omv_gpio_t definitions
typedef void *omv_gpio_t;

// GPIO modes, speeds, and pull configurations (dummy values for QEMU)
#define OMV_GPIO_MODE_INPUT         (0)
#define OMV_GPIO_MODE_OUTPUT        (1)
#define OMV_GPIO_MODE_OUTPUT_OD     (2)
#define OMV_GPIO_MODE_ALT           (3)
#define OMV_GPIO_MODE_ALT_OD        (4)

#define OMV_GPIO_PULL_NONE          (0)
#define OMV_GPIO_PULL_UP            (1)
#define OMV_GPIO_PULL_DOWN          (2)

#define OMV_GPIO_SPEED_LOW          (0)
#define OMV_GPIO_SPEED_MED          (1)
#define OMV_GPIO_SPEED_HIGH         (2)
#define OMV_GPIO_SPEED_MAX          (3)

// omv_i2c_t definitions
typedef void *omv_i2c_dev_t;

#endif // __OMV_PORTCONFIG_H__
