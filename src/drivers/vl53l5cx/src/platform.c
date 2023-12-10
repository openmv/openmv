/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * VL53L5CX platform implementation.
 */

#include "omv_boardconfig.h"
#if (OMV_ENABLE_TOF_VL53L5CX == 1)

#include "py/mphal.h"
#include "omv_i2c.h"
#include "platform.h"

uint8_t RdByte(VL53L5CX_Platform *platform, uint16_t regaddr, uint8_t *data)
{
    regaddr = __REVSH(regaddr);

    if (omv_i2c_write_bytes(platform->bus, platform->address, (uint8_t *) &regaddr, 2, OMV_I2C_XFER_NO_STOP) != 0) {
        return -1;
    }

    if (omv_i2c_read_bytes(platform->bus, platform->address, data, 1, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return -1;
    }
    return 0;
}

uint8_t WrByte(VL53L5CX_Platform *platform, uint16_t regaddr, uint8_t data)
{
    regaddr = __REVSH(regaddr);

    if (omv_i2c_write_bytes(platform->bus, platform->address, (uint8_t*) &regaddr, 2, OMV_I2C_XFER_SUSPEND) != 0) {
        return -1;
    }

    if (omv_i2c_write_bytes(platform->bus, platform->address, &data, 1, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return -1;
    }

    return 0;
}

uint8_t RdMulti(VL53L5CX_Platform *platform, uint16_t regaddr, uint8_t *data, uint32_t size)
{
    regaddr = __REVSH(regaddr);

    if (omv_i2c_write_bytes(platform->bus, platform->address, (uint8_t *) &regaddr, 2, OMV_I2C_XFER_NO_STOP) != 0) {
        return -1;
    }

    if (omv_i2c_read_bytes(platform->bus, platform->address, data, size, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return -1;
    }
    return 0;
}

uint8_t WrMulti(VL53L5CX_Platform *platform, uint16_t regaddr, uint8_t *data, uint32_t size)
{
    regaddr = __REVSH(regaddr);

    if (omv_i2c_write_bytes(platform->bus, platform->address, (uint8_t*) &regaddr, 2, OMV_I2C_XFER_SUSPEND) != 0) {
        return -1;
    }

    if (omv_i2c_write_bytes(platform->bus, platform->address, data, size, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return -1;
    }

    return 0;
}


uint8_t Reset_Sensor( VL53L5CX_Platform *platform)
{
    uint8_t status = 0;
    /* (Optional) Need to be implemented by customer. This function returns 0 if OK */
    /* Set pin LPN to LOW */
    /* Set pin AVDD to LOW */
    /* Set pin VDDIO  to LOW */
    WaitMs(platform, 100);

    /* Set pin LPN of to HIGH */
    /* Set pin AVDD of to HIGH */
    /* Set pin VDDIO of  to HIGH */
    WaitMs(platform, 100);

    return status;
}

void SwapBuffer(uint8_t *buffer, uint16_t size)
{
    uint32_t i, tmp;
    /* Example of possible implementation using <string.h> */
    for(i = 0; i < size; i = i + 4) {
        tmp = (
                buffer[i]<<24)
            |(buffer[i+1]<<16)
            |(buffer[i+2]<<8)
            |(buffer[i+3]);

        memcpy(&(buffer[i]), &tmp, 4);
    }
}

uint8_t WaitMs(VL53L5CX_Platform *platform, uint32_t ms)
{
    mp_hal_delay_ms(ms);
    return 0;
}
#endif // #if (OMV_ENABLE_TOF_VL53L5CX == 1)
