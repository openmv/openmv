/**
 *
 * \file
 *
 * \brief This module contains common APIs declarations.
 *
 * Copyright (c) 2016-2021 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
#include "common/include/nm_common.h"

void m2m_memcpy(uint8* pDst,uint8* pSrc,uint32 sz)
{
	if(sz == 0) return;
	do
	{
		*pDst = *pSrc;
		pDst++;
		pSrc++;
	}while(--sz);
}
uint8 m2m_checksum(uint8* buf, int sz)
{
	uint8 cs = 0;
	while(--sz)
	{
		cs ^= *buf;
		buf++;
	}

	return cs;
}

void m2m_memset(uint8* pBuf,uint8 val,uint32 sz)
{
	if(sz == 0) return;
	do
	{
		*pBuf = val;
		pBuf++;
	}while(--sz);
}

uint16 m2m_strlen(uint8 * pcStr)
{
	uint16	u16StrLen = 0;
	while(*pcStr)
	{
		u16StrLen ++;
		pcStr++;
	}
	return u16StrLen;
}

uint8 m2m_strncmp(uint8 *pcS1, uint8 *pcS2, uint16 u16Len)
{
    for ( ; u16Len > 0; pcS1++, pcS2++, --u16Len)
	if (*pcS1 != *pcS2)
	    return ((*(uint8 *)pcS1 < *(uint8 *)pcS2) ? -1 : +1);
	else if (*pcS1 == '\0')
	    return 0;
    return 0;
}

/* Finds the occurrence of pcStr in pcIn.
If pcStr is part of pcIn it returns a valid pointer to the start of pcStr within pcIn.
Otherwise a NULL Pointer is returned.
*/
uint8 * m2m_strstr(uint8 *pcIn, uint8 *pcStr)
{
    uint8 u8c;
    uint16 u16StrLen;

    u8c = *pcStr++;
    if (!u8c)
        return (uint8 *) pcIn;	// Trivial empty string case

    u16StrLen = m2m_strlen(pcStr);
    do {
        uint8 u8Sc;

        do {
            u8Sc = *pcIn++;
            if (!u8Sc)
                return (uint8 *) 0;
        } while (u8Sc != u8c);
    } while (m2m_strncmp(pcIn, pcStr, u16StrLen) != 0);

    return (uint8 *) (pcIn - 1);
}

sint8 m2m_memcmp(uint8 *pu8Buff1,uint8 *pu8Buff2 ,uint32 u32Size)
{
	uint32	i;
	sint8		s8Result = 0;
	for(i	 = 0 ; i < u32Size ; i++)
	{
		if(pu8Buff1[i] != pu8Buff2[i])
		{
			s8Result = 1;
			break;
		}
	}
	return s8Result;
}

/* Convert hexchar to value 0-15 */
static uint8 hexchar_2_val(uint8 ch)
{
    /* ch -= '0' */
    ch -= 0x30;
    if(ch <= 9)
        return ch;
    /* OR with 0x20 to convert upper case to lower case. */
    ch |= 0x20;
    /* ch -= ('a'-'0') */
    ch -= 0x31;
    if(ch <= 5)
        return ch + 10;
    return 0xFF;
}

/* Convert hexstring to bytes */
sint8 hexstr_2_bytes(uint8 *pu8Out, uint8 *pu8In, uint8 u8SizeOut)
{
    while(u8SizeOut--)
    {
        uint8   u8Out = hexchar_2_val(*pu8In++);
        if(u8Out > 0xF)
            return M2M_ERR_INVALID_ARG;
        *pu8Out = u8Out * 0x10;
        u8Out = hexchar_2_val(*pu8In++);
        if(u8Out > 0xF)
            return M2M_ERR_INVALID_ARG;
        *pu8Out += u8Out;
        pu8Out++;
    }
    return M2M_SUCCESS;
}
