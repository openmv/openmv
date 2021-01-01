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
#include "MLX90621_I2C_Driver.h"

static cambus_t *bus;

void MLX90621_I2CInit(cambus_t *hbus)
{   
    bus = hbus;
}

int MLX90621_I2CReadEEPROM(uint8_t slaveAddr, uint8_t startAddress, uint16_t nMemAddressRead, uint8_t *data)
{
    if (cambus_write_bytes(bus, (slaveAddr << 1), &startAddress, 1, CAMBUS_XFER_NO_STOP) != 0) {
        return -1;
    }
             
    if (cambus_read_bytes(bus, (slaveAddr << 1), data, nMemAddressRead, CAMBUS_XFER_NO_FLAGS) != 0) {
        return -1; 
    }          
    
    return 0;   
} 

int MLX90621_I2CRead(uint8_t slaveAddr,uint8_t command,
        uint8_t startAddress, uint8_t addressStep, uint8_t nMemAddressRead, uint16_t *data)
{
    uint8_t cmd[4] = {
        command,
        startAddress,
        addressStep,
        nMemAddressRead
    };
    
    if (cambus_write_bytes(bus, (slaveAddr << 1), cmd, 4, CAMBUS_XFER_NO_STOP) != 0) {
        return -1;
    }

    if (cambus_read_bytes(bus, (slaveAddr << 1), (uint8_t *) data, nMemAddressRead * 2, CAMBUS_XFER_NO_FLAGS) != 0) {
        return -1; 
    }          

    return 0;   
} 

int MLX90621_I2CWrite(uint8_t slaveAddr, uint8_t command, uint8_t checkValue, uint16_t data)
{
    uint8_t cmd[5] = { 
        command,
        (data & 0x00FF) - checkValue,
        (data & 0x00FF),
        (data >> 8) - checkValue,
        (data >> 8)
    };

    if (cambus_write_bytes(bus, (slaveAddr << 1), cmd, 5, CAMBUS_XFER_NO_FLAGS) != 0) {
        return -1;
    }
    return 0;
}

