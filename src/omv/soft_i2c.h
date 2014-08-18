#ifndef __SOFT_I2C_H__
#define __SOFT_I2C_H__
#include <stdint.h>
void soft_i2c_init();
int soft_i2c_read_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop);
int soft_i2c_write_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop);
#endif //__SOFT_I2C_H__
