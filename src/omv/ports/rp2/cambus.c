#include <stdio.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "omv_boardconfig.h"
#include "cambus.h"

#define I2C_TIMEOUT             (100*1000)
#define I2C_SCAN_TIMEOUT        (1*1000)

int cambus_init(cambus_t *bus, uint32_t bus_id, uint32_t speed)
{
    bus->id = bus_id;
    bus->initialized = false;

    switch (speed) {
        case CAMBUS_SPEED_STANDARD:
            bus->speed = 100 * 1000; ///< 100 kbps
            break;
        case CAMBUS_SPEED_FULL:
            bus->speed = 250 * 1000; ///< 250 kbps
            break;
        case CAMBUS_SPEED_FAST:
            bus->speed = 400 * 1000;  ///< 400 kbps
            break;
        default:
            return -1;
    }

    switch (bus_id) {
        case 0: {
            bus->i2c = i2c0;
            bus->scl_pin = I2C0_SCL_PIN;
            bus->sda_pin = I2C0_SDA_PIN;
            break;
        }
        case 1: {
            bus->i2c = i2c1;
            bus->scl_pin = I2C1_SCL_PIN;
            bus->sda_pin = I2C1_SDA_PIN;
            break;
        }
        default:
            return -1;
    }

    i2c_init(bus->i2c, bus->speed);
    gpio_set_function(bus->scl_pin, GPIO_FUNC_I2C);
    gpio_set_function(bus->sda_pin, GPIO_FUNC_I2C);

    bus->initialized = true;
    return 0;
}

int cambus_deinit(cambus_t *bus)
{
    if (bus->initialized) {
        i2c_deinit(bus->i2c);
        bus->initialized = false;
    }
    return 0;
}

int cambus_scan(cambus_t *bus)
{
    for (uint8_t addr=0x20, rxdata; addr<=0x48; addr++) {
        if (i2c_read_timeout_us(bus->i2c, addr, &rxdata, 1, false, I2C_SCAN_TIMEOUT) >= 0) {
            return (addr << 1);
        }
    }
    return 0;
}

int cambus_readb(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data)
{
    int bytes = 0;
    slv_addr = slv_addr >> 1;

    bytes += i2c_write_timeout_us(bus->i2c, slv_addr, &reg_addr, 1, false, I2C_TIMEOUT);
    bytes += i2c_read_timeout_us(bus->i2c, slv_addr, reg_data, 1, false, I2C_TIMEOUT);

    return (bytes == 2) ? 0 : -1;
}

int cambus_writeb(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    int bytes = 0;
    slv_addr = slv_addr >> 1;

    uint8_t buf[] = {reg_addr, reg_data};
    bytes = i2c_write_timeout_us(bus->i2c, slv_addr, buf, 2, false, I2C_TIMEOUT);

    return (bytes == 2) ? 0 : -1;
}

int cambus_read_bytes(cambus_t *bus, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags)
{
    int bytes = 0;
    slv_addr = slv_addr >> 1;
    bool nostop = false;

    if (flags & CAMBUS_XFER_NO_STOP) {
        nostop = true;
    }

    bytes = i2c_read_timeout_us(bus->i2c, slv_addr, buf, len, nostop, I2C_TIMEOUT);

    return (bytes == len) ? 0 : -1;
}

int cambus_write_bytes(cambus_t *bus, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags)
{
    int bytes = 0;
    slv_addr = slv_addr >> 1;
    bool nostop = false;

    if (flags & CAMBUS_XFER_NO_STOP) {
        nostop = true;
    }

    bytes = i2c_write_timeout_us(bus->i2c, slv_addr, buf, len, nostop, I2C_TIMEOUT);

    return (bytes == len) ? 0 : -1;
}

int cambus_pulse_scl(cambus_t *bus)
{
    cambus_deinit(bus);

    // Configure SCL as GPIO
    gpio_init(bus->scl_pin);
    gpio_set_dir(bus->scl_pin, GPIO_OUT);

    // Pulse SCL to recover stuck device.
    for (int i=0; i<10000; i++) {
        gpio_put(bus->scl_pin, 1);
        mp_hal_delay_us(10);
        gpio_put(bus->scl_pin, 0);
        mp_hal_delay_us(10);
    }
    return 0;
}
