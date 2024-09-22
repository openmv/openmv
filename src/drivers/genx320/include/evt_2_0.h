/**
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

#ifndef INC_EVT_2_0_H_
#define INC_EVT_2_0_H_

#define __EVT20_TYPE(__VAL__) ((__VAL__ >> 28)& 0xFU)
#define __EVT20_TS(__VAL__) ((__VAL__ >> 22) & 0xF3U)
#define __EVT20_X(__VAL__) ((__VAL__ >> 11) & 0x7FFU)
#define __EVT20_Y(__VAL__) ((__VAL__ ) & 0x7FFU)
#define __EVT20_TIME_HIGH(__VAL__) ((__VAL__ ) & 0xFFFFFFFU)

#define TD_LOW              0x0U  /*!< EVT2.0 TD Event, Decrease in illumination   */
#define TD_HIGH             0x1U  /*!< EVT2.0 TD Event, Increase in illumination   */
#define EV_TIME_HIGH        0x8U  /*!< Timer High bits                             */
#define EXT_TRIGGER         0xAU  /*!< External triggers                           */
#define OTHERS              0xEU  /*!< To be used in extension in the event types  */
#define CONTINUED           0xFU  /*!< Extra data to previous events               */

#endif /* INC_EVT_2_0_H_ */
