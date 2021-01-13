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
#include "cambus.h"
#include "MLX90641_I2C_Driver.h"
static cambus_t *bus;

void MLX90641_I2CInit(cambus_t *hbus)
{   
    bus = hbus;
}

int MLX90641_I2CGeneralReset(void)
{    
   if (cambus_gencall(bus, 0x06) != 0) {
        return -1;
	}

    return 0;
}

int MLX90641_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    startAddress = __REVSH(startAddress);

    if (cambus_write_bytes(bus, (slaveAddr<<1), (uint8_t *) &startAddress, 2, CAMBUS_XFER_NO_STOP) != 0) {
        return -1;
	}


    if (cambus_read_bytes(bus, (slaveAddr<<1), (uint8_t *) data, nMemAddressRead*2, CAMBUS_XFER_NO_FLAGS) != 0) {
        return -1;
	}

    for(int i=0; i<nMemAddressRead; i++) {
        data[i] = __REVSH(data[i]);
    }

	return 0;   
} 

int MLX90641_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    data = __REVSH(data);
    writeAddress = __REVSH(writeAddress);

	if (cambus_write_bytes(bus, (slaveAddr << 1), (uint8_t*) &writeAddress, 2, CAMBUS_XFER_SUSPEND) != 0) {
        return -1;
	}         

	if (cambus_write_bytes(bus, (slaveAddr << 1), (uint8_t *) &data, 2, CAMBUS_XFER_NO_FLAGS) != 0) {
        return -1;
	}         
	return 0;
}
