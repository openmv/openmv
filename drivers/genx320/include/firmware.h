/**
 ******************************************************************************
 * @file    firmware.h
 * @author  PSEE Applications Team
 * @brief	RISC-V Firmwares
 *
 ******************************************************************************
 * @attention
 * Copyright (c) Prophesee S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and limitations under the License.
 *
 ******************************************************************************
 */
#ifndef APPLICATION_USER_FIRMWARE_H_
#define APPLICATION_USER_FIRMWARE_H_

/* Standard Includes */
#include <stdint.h>

/* User Includes */
#include "psee_genx320.h"

#ifdef __cplusplus
 extern "C" {
#endif

 /* Low power wakeup application Firmware */
 extern const size_t fw_esp_wakeup_size;
 extern const Firmware fw_esp_wakeup[];

 /* Led tracking application Firmware */
 extern const size_t fw_led_tracking_size;
 extern const Firmware fw_led_tracking[];

#ifdef __cplusplus
 }
#endif

#endif /* APPLICATION_USER_FIRMWARE_H_ */
