/*******************************************************************************
  WINC1500 Wireless Driver

  File Name:
    inet_ntop.c

  Summary:
    Implementation of standard inet_ntop function.

  Description:
    Implementation of standard inet_ntop function.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2022, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/

#include <stdint.h>
#include <stddef.h>
#include "socket.h"

const char *inet_ntop(int af, const void *src, char *dst, size_t size)
{
    uint8_t i, v, t, c, n;
    char *rp = dst;
    uint32_t ip = ((struct in_addr*)src)->s_addr;

    if ((NULL == src) || (NULL == dst) || (size < 16))
    {
        return NULL;
    }

    for (i=0; i<4; i++)
    {
        t = ip;
        v = 100;

        // Check for zero

        if (t > 0)
        {
            n = 0;

            do
            {
                c = '0';
                while (t >= v)
                {
                    c++;
                    t -= v;
                }
                v /= 10;

                if (('0' != c) || (n > 0))
                {
                    *dst++ = c;

                    n++;
                }
            }
            while (v > 0);
        }
        else
        {
            *dst++ = '0';
        }

        if (3 == i)
        {
            *dst++ = '\0';
        }
        else
        {
            *dst++ = '.';
        }

        ip >>= 8;
    }

    return rp;
}

//DOM-IGNORE-END
