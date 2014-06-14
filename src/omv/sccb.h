#ifndef __SCCB_H__
#define __SCCB_H__
#include <stdint.h>
void SCCB_Init();
uint8_t SCCB_Read(uint8_t addr);
uint8_t SCCB_Write(uint8_t addr, uint8_t data);
#endif /* __SCCB_H__ */ 
