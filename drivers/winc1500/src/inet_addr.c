/*******************************************************************************
  WINC1500 Wireless Driver

  File Name:
    inet_addr.c

  Summary:
    Implementation of standard inet_addr function.

  Description:
    Implementation of standard inet_addr function.
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
#include "socket.h"

in_addr_t inet_addr(const char *cp)
{
    uint8_t i,l;
    uint16_t t;
    uint32_t ip;
    char c;

    ip = 0;

    for (i=0; i<4; i++)
    {
        t = 0;
        ip >>= 8;

        // Count non-delimiter or terminator characters

        for (l=0; l<4; l++)
        {
            c = cp[l];

            if (('.' == c) || ('\0' == c))
            {
                break;
            }
        }

        // There must be 1 to 3 characters

        if ((0 == l) || (4 == l))
        {
            return 0;
        }

        c = *cp++;

        // First digit can't be '0' unless it's the only one

        if ((l > 1) && (c == '0'))
        {
            return 0;
        }

        while(l--)
        {
            // Each digit must be decimal

            if ((c < '0') || (c > '9'))
            {
                return 0;
            }

            t = (t * 10) + (c - '0');

            c = *cp++;
        }

        // Total accumulated number must be less than 256

        if (t > 255)
        {
            return 0;
        }

        // Pack number into 32 bit IP address representation

        ip |= ((uint32_t)t << 24);

        // First three numbers must terminate with '.', last one with '\0's

        if ((('\0' == c) && (i != 3)) || (('\0' != c) && (i == 3)))
        {
            return 0;
        }
    }

    return ip;
}

//DOM-IGNORE-END
