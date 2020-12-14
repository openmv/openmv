/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdio.h>
#include STM32_HAL_H
#include "cambus.h"
#include "MLX90640_I2C_Driver.h"

static I2C_HandleTypeDef *hi2c;

void MLX90640_I2CInit(I2C_HandleTypeDef *i2c)
{   
    hi2c = i2c;
}

int MLX90640_I2CGeneralReset()
{
    if (cambus_gencall(hi2c, 0x06) != 0) {
        return -1;
	}

    return 0;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
	uint8_t* p = (uint8_t*) data;
    if (cambus_readw_bytes(hi2c, (slaveAddr<<1), startAddress, p, nMemAddressRead*2) != 0) {
        return -1;
	}

	for(int cnt=0; cnt < nMemAddressRead*2; cnt+=2) {
		uint8_t tempBuffer = p[cnt+1];
		p[cnt+1] = p[cnt];
		p[cnt] = tempBuffer;
	}

	return 0;   
} 

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    data = (data >> 8) | (data << 8);
	if (cambus_writew_bytes(hi2c, (slaveAddr << 1), writeAddress, (uint8_t*) &data, 2) != 0) {
        return -1;
	}         
	return 0;
}
