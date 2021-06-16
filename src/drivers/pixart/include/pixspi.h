#ifndef __PIXSPI__
#define __PIXSPI__
#include <stdbool.h>
#include <stdint.h>
bool pixspi_init();
void pixspi_release();
int pixspi_regs_read(uint8_t addr, uint8_t * data, uint16_t length);
int pixspi_regs_write(uint8_t addr, const uint8_t * data, uint16_t length);
#endif