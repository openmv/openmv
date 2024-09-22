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

#ifndef INC_AER_H_
#define INC_AER_H_

#define __AER_Y(__VAL__) ((__VAL__ ) & 0x1FFU)
#define __AER_X(__VAL__) ((__VAL__ >> 9) & 0x1FFU)
#define __AER_P(__VAL__) ((__VAL__ >> 18 ) & 0x1U)

#endif /* INC_AER_H_ */
