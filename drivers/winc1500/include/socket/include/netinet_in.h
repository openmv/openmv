/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    netinet_in.h

  Summary:

  Description:
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

#ifndef _NETINET_IN_H
#define _NETINET_IN_H

#include <stdint.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

typedef uint32_t in_addr_t;

struct in_addr {
    /*!<
        Network Byte Order representation of the IPv4 address. For example,
        the address "192.168.0.10" is represented as 0x0A00A8C0.
    */
    in_addr_t s_addr;
};

struct sockaddr_in{
    uint16_t        sin_family;
    /*!<
        Specifies the address family(AF).
        Members of AF_INET address family are IPv4 addresses.
        Hence,the only supported value for this is AF_INET.
    */
    uint16_t        sin_port;
    /*!<
        Port number of the socket.
        Network sockets are identified by a pair of IP addresses and port number.
        Must be set in the Network Byte Order format , @ref _htons (e.g. _htons(80)).
        Can NOT have zero value.
    */
    struct in_addr  sin_addr;
    /*!<
        IP Address of the socket.
        The IP address is of type @ref in_addr structure.
        Can be set to "0" to accept any IP address for server operation.
    */
    uint8_t         sin_zero[8];
    /*!<
        Padding to make structure the same size as @ref sockaddr.
    */
};

const char *inet_ntop(int af, const void *src, char *dst, size_t size);
in_addr_t inet_addr(const char *cp);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _NETINET_IN_H */
