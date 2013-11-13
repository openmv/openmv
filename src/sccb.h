/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			SCCB.h
** Descriptions:		SCCB 
**
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2011-2-13
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/
#ifndef __SCCB_H__
#define __SCCB_H__
#include <stdint.h>
void SCCB_Init();
uint8_t SCCB_Read(uint8_t regID);
uint8_t SCCB_Write(uint8_t regID, uint8_t regDat);
#endif /* __SCCB_H__ */ 
