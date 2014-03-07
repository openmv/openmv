#ifndef __SPI_H__
#define __SPI_H__
#include <stdint.h>
void spi_init();
uint8_t spi_read();
uint8_t spi_write(uint8_t b);
#endif /* __SPI_H__ */
