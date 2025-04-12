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
#include "omv_i2c.h"
#include "MLX90640_I2C_Driver.h"
static omv_i2c_t *bus;

void MLX90640_I2CInit(omv_i2c_t *hbus)
{   
    bus = hbus;
}

int MLX90640_I2CGeneralReset()
{
    if (omv_i2c_gencall(bus, 0x06) != 0) {
        return -1;
	}

    return 0;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    for (uint16_t n = nMemAddressRead, *d = data; n; ) {
        uint16_t write_address = __REVSH(startAddress);
        uint16_t read_size = n > OMV_I2C_MAX_16BIT_XFER ? OMV_I2C_MAX_16BIT_XFER : n;

        if (omv_i2c_write_bytes(bus, (slaveAddr<<1), (uint8_t *) &write_address, 2, OMV_I2C_XFER_NO_STOP) != 0) {
            return -1;
        }


        if (omv_i2c_read_bytes(bus, (slaveAddr<<1), (uint8_t *) d, read_size*2, OMV_I2C_XFER_NO_FLAGS) != 0) {
            return -1;
        }

        startAddress += read_size;
        n -= read_size;
        d += read_size;
    }

    for(int i=0; i<nMemAddressRead; i++) {
        data[i] = __REVSH(data[i]);
    }

	return 0;   
} 

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    data = __REVSH(data);
    writeAddress = __REVSH(writeAddress);

	if (omv_i2c_write_bytes(bus, (slaveAddr << 1), (uint8_t*) &writeAddress, 2, OMV_I2C_XFER_SUSPEND) != 0) {
        return -1;
	}         

	if (omv_i2c_write_bytes(bus, (slaveAddr << 1), (uint8_t *) &data, 2, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return -1;
	}         
	return 0;
}
