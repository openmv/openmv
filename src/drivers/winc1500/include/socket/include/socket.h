/**
 *
 * \file
 *
 * \brief WINC BSD compatible Socket Interface.
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

#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifdef  __cplusplus
extern "C" {
#endif

/** @defgroup SocketHeader Socket
 *          BSD compatible socket interface between the host layer and the network
 *          protocol stacks in the firmware.
 *          These functions are used by the host application to send or receive
 *          packets and to do other socket operations.
 */

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "common/include/nm_common.h"
#include "driver/include/m2m_types.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**@defgroup  SocketDefines Defines
 * @ingroup SocketHeader
 */

/**@defgroup  IPDefines TCP/IP Defines
 * @ingroup SocketDefines
 * The following list of macros are used to define constants used throughout the socket layer.
 * @{
 */

/*
 *  HOSTNAME_MAX_SIZE is defined here and also in host_drv/socket/include/m2m_socket_host_if.h
 *  The two definitions must match.
*/
#define HOSTNAME_MAX_SIZE                                   64
/*!<
    Maximum allowed size for a host domain name passed to the function gethostbyname @ref gethostbyname.
    command value. Used with the @ref setsockopt function.
*/

#define SOCKET_BUFFER_MAX_LENGTH                            1400
/*!<
    Maximum allowed size for a socket data buffer. Used with @ref send socket
    function to ensure that the buffer sent is within the allowed range.
*/

#define  AF_INET                                            2
/*!<
    The AF_INET is the address family used for IPv4. An IPv4 transport address is specified with the @ref sockaddr_in structure.
    (It is the only supported type for the current implementation.)
*/

#define  SOCK_STREAM                                        1
/*!<
     One of the IPv4 supported socket types for reliable connection-oriented stream connection.
     Passed to the @ref socket function for the socket creation operation.
*/

#define  SOCK_DGRAM                                         2
/*!<
     One of the IPv4 supported socket types for unreliable connectionless datagram connection.
     Passed to the @ref socket function for the socket creation operation.
*/

#define SOCKET_FLAGS_SSL                                    0x01
/*!<
    This flag may be set in the u8Config parameter of @ref socket, to create a
    TLS socket.\n
    Note that the number of TLS sockets is limited to 4.\n
    This flag is kept for legacy purposes. It is recommended that applications
    use @ref SOCKET_CONFIG_SSL_ON instead.
*/

#define SOCKET_CONFIG_SSL_OFF                               0
/*!<
    This value may be passed in the u8Config parameter of @ref socket, to
    create a socket not capable of TLS.
*/
#define SOCKET_CONFIG_SSL_ON                                1
/*!<
    This value may be passed in the u8Config parameter of @ref socket, to
    create a TLS socket.\n
    Note that the number of TLS sockets is limited to 4.
*/
#define SOCKET_CONFIG_SSL_DELAY                             2
/*!<
    This value may be passed in the u8Config parameter of @ref socket, to
    create a TCP socket which has the potential to upgrade to a TLS socket
    later (by calling @ref secure).\n
    Note that the total number of TLS sockets and potential TLS sockets is
    limited to 4.
*/

#define TCP_SOCK_MAX                                        (7)
/*!<
    Maximum number of simultaneous TCP sockets.
*/

#define UDP_SOCK_MAX                                        4
/*!<
    Maximum number of simultaneous UDP sockets.
*/

#define MAX_SOCKET                                          (TCP_SOCK_MAX + UDP_SOCK_MAX)
/*!<
    Maximum number of simultaneous sockets.
*/

#define SOL_SOCKET                                          1
/*!<
    Socket option.
    Used with the @ref setsockopt function
*/

#define SOL_SSL_SOCKET                                      2
/*!<
    SSL Socket option level.
    Used with the @ref setsockopt function
*/

#define SO_SET_UDP_SEND_CALLBACK                            0x00
/*!<
    Socket option used by the application to enable/disable
    the use of UDP send callbacks.\n
    Used with the @ref setsockopt function.\n
    The option value should be cast to int type.\n
    0: disable UDP send callbacks.\n
    1: enable UDP send callbacks.\n
    Default setting is enable.

    @warning @ref connect and @ref bind cause this setting to
        be lost, so the application should only set this option
        after calling @ref connect or @ref bind.
*/

#define IP_ADD_MEMBERSHIP                                   0x01
/*!<
    Set Socket Option Add Membership command value (to join a multicast group).
    Used with the @ref setsockopt function.
*/

#define IP_DROP_MEMBERSHIP                                  0x02
/*!<
    Set Socket Option Drop Membership command value (to leave a multicast group).
    Used with the @ref setsockopt function.
*/

#define SO_TCP_KEEPALIVE                                    0x04
/*!<
    Socket option to enable or disable TCP keep-alive.\n
    Used with the @ref setsockopt function.\n
    The option value should be cast to int type.\n
    0: disable TCP keep-alive.\n
    1: enable TCP keep-alive.\n
    Default setting is disable.

    @warning @ref connect and @ref bind cause this setting to
    be lost, so the application should only set this option
    after calling @ref connect or @ref bind.
*/

#define SO_TCP_KEEPIDLE                                     0x05
/*!<
    Socket option to set the time period after which the socket will trigger keep-alive transmissions.\n
    Used with the @ref setsockopt function.\n
    The option value should be cast to int type.\n
    Option value is the time period in units of 500ms. Maximum 2^32 - 1.
    Default setting is 120 (60 seconds).

    @warning @ref connect and @ref bind cause this setting to
    be lost, so the application should only set this option
    after calling @ref connect or @ref bind.
*/

#define SO_TCP_KEEPINTVL                                    0x06
/*!<
    Socket option to set the time period between keep-alive retransmissions.\n
    Used with the @ref setsockopt function.\n
    The option value should be cast to int type.\n
    Option value is the time period in units of 500ms. Maximum 255.
    Default setting is 1 (0.5 seconds).

    @warning @ref connect and @ref bind cause this setting to
    be lost, so the application should only set this option
    after calling @ref connect or @ref bind.
*/

#define SO_TCP_KEEPCNT                                      0x07
/*!<
    Socket option to set the number of keep-alive retransmissions to be carried out before declaring that the remote end is not available.\n
    Used with the @ref setsockopt function.\n
    The option value should be cast to int type.\n
    Maximum 255.
    Default setting is 20.

    @warning @ref connect and @ref bind cause this setting to
    be lost, so the application should only set this option
    after calling @ref connect or @ref bind.
*/
/**@}*/     //IPDefines


/**@addtogroup TLSDefines
 * @{
 */
#define ALPN_LIST_MAX_APP_LENGTH                            30
/*!<
    Maximum length of ALPN list that can be specified by the application.
    This length includes separators (spaces) and terminator (NUL).
*/
/**@}*/     // TLSDefines

/**
 * @defgroup TLSDefines TLS Defines
 * @ingroup  SOCKETDEF
 * @ingroup SSLAPI
 */

/**@defgroup  SSLSocketOptions TLS Socket Options
 * @ingroup TLSDefines
 * The following list of macros are used to define SSL Socket options.
 * @{
 * @sa setsockopt
 */

#define SO_SSL_BYPASS_X509_VERIF                            0x01
/*!<
    Allow an opened SSL socket to bypass the X509 certificate verification
    process.
    It is recommended NOT to use this socket option in production software
    applications. It is supported for debugging and testing purposes.\n
    The option value should be casted to int type.\n
    0: do not bypass the X509 certificate verification process (default,
    recommended).\n
    1: bypass the X509 certificate verification process.\n

    This option only takes effect if it is set after calling @ref socket and
    before calling @ref connect or @ref secure.
*/

#define SO_SSL_SNI                                          0x02
/*!<
    Set the Server Name Indicator (SNI) for an SSL socket. The SNI is a NULL-
    terminated string containing the server name associated with the
    connection. Its size must not exceed @ref HOSTNAME_MAX_SIZE. If the SNI is
    not a null string, then TLS Client Hello messages will include the SNI
    extension.\n

    This option only takes effect if it is set after calling @ref socket and
    before calling @ref connect or @ref secure.
*/

#define SO_SSL_ENABLE_SESSION_CACHING                       0x03
/*!<
    This option allow the TLS to cache the session information for fast TLS
    session establishment in future connections using the TLS Protocol session
    resume features.\n
    The option value should be casted to int type.\n
    0: disable TLS session caching (default).\n
    1: enable TLS session caching.\n
    Note that TLS session caching is always enabled in TLS Server Mode and this
    option is ignored.\n

    This option only takes effect if it is set after calling @ref socket and
    before calling @ref connect or @ref secure.
*/

#define SO_SSL_ENABLE_SNI_VALIDATION                        0x04
/*!<
    Enable internal validation of server name against the server's
    certificate subject common name. If there is no server name
    provided (via the @ref SO_SSL_SNI option), setting this option
    does nothing.\n
    The option value should be casted to int type.\n
    0: disable server certificate name validation (default).\n
    1: enable server certificate name validation (recommended).\n

    This option only takes effect if it is set after calling @ref socket and
    before calling @ref connect or @ref secure.
*/

#define SO_SSL_ALPN                                         0x05
/*!<
    Set the list to use for Application-Layer Protocol Negotiation
    for an SSL socket. \n
    This option is intended for internal use and should not be
    used by the application. Applications should use the API @ref
    set_alpn_list.
*/
/**@}*/     //SSLSocketOptions

/**@defgroup  LegacySSLCipherSuite Legacy names for TLS Cipher Suite IDs
 * @ingroup TLSDefines
 * The following list of macros MUST NOT be used. Instead use the new names under SSLCipherSuiteID
 * @sa m2m_ssl_set_active_ciphersuites
 * @{
 */

#define SSL_ENABLE_RSA_SHA_SUITES                           0x01
/*!<
    Enable RSA Hmac_SHA based Cipher suites. For example,
        TLS_RSA_WITH_AES_128_CBC_SHA
*/

#define SSL_ENABLE_RSA_SHA256_SUITES                        0x02
/*!<
    Enable RSA Hmac_SHA256 based Cipher suites. For example,
        TLS_RSA_WITH_AES_128_CBC_SHA256
*/

#define SSL_ENABLE_DHE_SHA_SUITES                           0x04
/*!<
    Enable DHE Hmac_SHA based Cipher suites. For example,
        TLS_DHE_RSA_WITH_AES_128_CBC_SHA
*/

#define SSL_ENABLE_DHE_SHA256_SUITES                        0x08
/*!<
    Enable DHE Hmac_SHA256 based Cipher suites. For example,
        TLS_DHE_RSA_WITH_AES_128_CBC_SHA256
*/

#define SSL_ENABLE_RSA_GCM_SUITES                           0x10
/*!<
    Enable RSA AEAD based Cipher suites. For example,
        TLS_RSA_WITH_AES_128_GCM_SHA256
*/

#define SSL_ENABLE_DHE_GCM_SUITES                           0x20
/*!<
    Enable DHE AEAD based Cipher suites. For example,
        TLS_DHE_RSA_WITH_AES_128_GCM_SHA256
*/

#define SSL_ENABLE_ALL_SUITES                               0x0000003F
/*!<
    Enable all possible supported cipher suites.
*/
/**@}*/     //LegacySSLCipherSuite

/** @defgroup  SSLCipherSuiteID TLS Cipher Suite IDs
 * @ingroup TLSDefines
 * The following list of macros defined the list of supported TLS Cipher suites.
 * Each MACRO defines a single Cipher suite.
 * @sa m2m_ssl_set_active_ciphersuites
 * @{
 */

#define SSL_CIPHER_RSA_WITH_AES_128_CBC_SHA                 NBIT0
#define SSL_CIPHER_RSA_WITH_AES_128_CBC_SHA256              NBIT1
#define SSL_CIPHER_DHE_RSA_WITH_AES_128_CBC_SHA             NBIT2
#define SSL_CIPHER_DHE_RSA_WITH_AES_128_CBC_SHA256          NBIT3
#define SSL_CIPHER_RSA_WITH_AES_128_GCM_SHA256              NBIT4
#define SSL_CIPHER_DHE_RSA_WITH_AES_128_GCM_SHA256          NBIT5
#define SSL_CIPHER_RSA_WITH_AES_256_CBC_SHA                 NBIT6
#define SSL_CIPHER_RSA_WITH_AES_256_CBC_SHA256              NBIT7
#define SSL_CIPHER_DHE_RSA_WITH_AES_256_CBC_SHA             NBIT8
#define SSL_CIPHER_DHE_RSA_WITH_AES_256_CBC_SHA256          NBIT9
#define SSL_CIPHER_ECDHE_RSA_WITH_AES_128_CBC_SHA           NBIT10
#define SSL_CIPHER_ECDHE_RSA_WITH_AES_256_CBC_SHA           NBIT11
#define SSL_CIPHER_ECDHE_RSA_WITH_AES_128_CBC_SHA256        NBIT12
#define SSL_CIPHER_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256      NBIT13
#define SSL_CIPHER_ECDHE_RSA_WITH_AES_128_GCM_SHA256        NBIT14
#define SSL_CIPHER_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256      NBIT15

#define SSL_ECC_ONLY_CIPHERS        \
(\
    SSL_CIPHER_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256  |   \
    SSL_CIPHER_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256      \
)
/*!<
    All ciphers that use ECC crypto only. This excludes ciphers that use RSA. They use ECDSA instead.
    These ciphers are turned off by default at startup.
    The application may enable them if it has an ECC math engine (like ATECC508).
*/
#define SSL_ECC_ALL_CIPHERS     \
(\
    SSL_CIPHER_ECDHE_RSA_WITH_AES_128_CBC_SHA       |   \
    SSL_CIPHER_ECDHE_RSA_WITH_AES_128_CBC_SHA256    |   \
    SSL_CIPHER_ECDHE_RSA_WITH_AES_128_GCM_SHA256    |   \
    SSL_CIPHER_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256  |   \
    SSL_CIPHER_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256      \
)
/*!<
    All supported ECC Ciphers including those ciphers that depend on RSA and ECC.
    These ciphers are turned off by default at startup.
    The application may enable them if it has an ECC math engine (like ATECC508).
*/

#define SSL_NON_ECC_CIPHERS_AES_128 \
(\
    SSL_CIPHER_RSA_WITH_AES_128_CBC_SHA             |   \
    SSL_CIPHER_RSA_WITH_AES_128_CBC_SHA256          |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_128_CBC_SHA         |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_128_CBC_SHA256      |   \
    SSL_CIPHER_RSA_WITH_AES_128_GCM_SHA256          |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_128_GCM_SHA256          \
)
/*!<
    All supported AES-128 Ciphers (ECC ciphers are not counted). This is the default active group after startup.
*/

#define SSL_ECC_CIPHERS_AES_256     \
(\
    SSL_CIPHER_ECDHE_RSA_WITH_AES_256_CBC_SHA   \
)
/*!<
    ECC AES-256 supported ciphers.
*/

#define SSL_NON_ECC_CIPHERS_AES_256 \
(\
    SSL_CIPHER_RSA_WITH_AES_256_CBC_SHA         |   \
    SSL_CIPHER_RSA_WITH_AES_256_CBC_SHA256      |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_256_CBC_SHA     |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_256_CBC_SHA256      \
)
/*!<
    AES-256 Ciphers.
    This group is disabled by default at startup because the WINC HW Accelerator
    supports only AES-128. If the application needs to force AES-256 cipher support,
    it could enable them (or any of them) explicitly by calling m2m_ssl_set_active_ciphersuites.
*/

#define SSL_CIPHER_ALL              \
(\
    SSL_CIPHER_RSA_WITH_AES_128_CBC_SHA             |   \
    SSL_CIPHER_RSA_WITH_AES_128_CBC_SHA256          |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_128_CBC_SHA         |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_128_CBC_SHA256      |   \
    SSL_CIPHER_RSA_WITH_AES_128_GCM_SHA256          |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_128_GCM_SHA256      |   \
    SSL_CIPHER_RSA_WITH_AES_256_CBC_SHA             |   \
    SSL_CIPHER_RSA_WITH_AES_256_CBC_SHA256          |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_256_CBC_SHA         |   \
    SSL_CIPHER_DHE_RSA_WITH_AES_256_CBC_SHA256      |   \
    SSL_CIPHER_ECDHE_RSA_WITH_AES_128_CBC_SHA       |   \
    SSL_CIPHER_ECDHE_RSA_WITH_AES_128_CBC_SHA256    |   \
    SSL_CIPHER_ECDHE_RSA_WITH_AES_128_GCM_SHA256    |   \
    SSL_CIPHER_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256  |   \
    SSL_CIPHER_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256  |   \
    SSL_CIPHER_ECDHE_RSA_WITH_AES_256_CBC_SHA           \
)
/*!<
    Turn On All TLS Ciphers.
*/
/**@}*/     //SSLCipherSuiteID

/**************
Socket Errors
**************/
/**@defgroup  SocketErrorCode Error Codes
 * @ingroup SocketHeader
 * The following list of macros are used to define the possible error codes.
 * Errors are listed in numerical order with the error macro name.
 * @{
 */
#define SOCK_ERR_NO_ERROR                                   0
/*!<
    Successful socket operation. This code is also used with event @ref SOCKET_MSG_RECV if a socket connection is closed.
    In that case, the application should call @ref close().
*/

#define SOCK_ERR_INVALID_ADDRESS                            -1
/*!<
    Socket address is invalid. The socket operation cannot be completed successfully without specifying a specific address
    For example: bind is called without specifying a port number
*/

#define SOCK_ERR_ADDR_ALREADY_IN_USE                        -2
/*!<
    Socket operation cannot bind on the given address. With socket operations, only one IP address per socket is permitted.
    Any attempt for a new socket to bind with an IP address already bound to another open socket,
    will return the following error code. States that bind operation failed.
*/

#define SOCK_ERR_MAX_TCP_SOCK                               -3
/*!<
    Exceeded the maximum number of TCP sockets. A maximum number of TCP sockets opened simultaneously is defined through TCP_SOCK_MAX.
    It is not permitted to exceed that number at socket creation. Identifies that @ref socket operation failed.
*/

#define SOCK_ERR_MAX_UDP_SOCK                               -4
/*!<
    Exceeded the maximum number of UDP sockets. A maximum number of UDP sockets opened simultaneously is defined through UDP_SOCK_MAX.
    It is not permitted to exceed that number at socket creation. Identifies that @ref socket operation failed
*/

#define SOCK_ERR_INVALID_ARG                                -6
/*!<
    An invalid argument is passed to a socket function. Identifies that @ref socket operation failed
*/

#define SOCK_ERR_MAX_LISTEN_SOCK                            -7
/*!<
    Exceeded the maximum number of TCP passive listening sockets.
    Identifies that @ref listen operation failed.
*/

#define SOCK_ERR_INVALID                                    -9
/*!<
    The requested socket operation is not valid in the current socket state.
    For example: @ref accept is called on a TCP socket before @ref bind or @ref listen.
*/

#define SOCK_ERR_ADDR_IS_REQUIRED                           -11
/*!<
    Destination address is required. Failure to provide the socket address required for the socket operation to be completed.
    It is generated as an error to the @ref sendto function when the address required to send the data to is not known.
*/

#define SOCK_ERR_CONN_ABORTED                               -12
/*!<
    The socket is closed (reset) by the peer. If this error is received, the application should call @ref close().
*/

#define SOCK_ERR_TIMEOUT                                    -13
/*!<
    The socket pending operation has timed out. The socket remains open.
*/

#define SOCK_ERR_BUFFER_FULL                                -14
/*!<
    No buffer space available to be used for the requested socket operation.
*/
/**@}*/     //SocketErrorCode

/**@addtogroup  SOCKETBYTEORDER Byte Order
 * @ingroup SocketHeader
 * The following list of macros are used to convert between host representation and network byte order.
 * @{
 */
#ifdef _NM_BSP_BIG_END
#define _htonl(m)               (m)
#define _htons(A)               (A)

#else

#define _htonl(m)       \
    (uint32)(((uint32)(m << 24)) | ((uint32)((m & 0x0000FF00) << 8)) | ((uint32)((m & 0x00FF0000) >> 8)) | ((uint32)(((uint32)m) >> 24)))
/*!<
    Convert a 4-byte integer from the host representation to the Network byte order representation.
*/

#define _htons(A)       (uint16)((((uint16) (A)) << 8) | (((uint16) (A)) >> 8))
/*!<
    Convert a 2-byte integer (short) from the host representation to the Network byte order representation.
*/
#endif

#define _ntohl              _htonl
/*!<
    Convert a 4-byte integer from the Network byte order representation to the host representation .
*/

#define _ntohs              _htons
/*!<
    Convert a 2-byte integer from the Network byte order representation to the host representation .
*/
/**@}*/     //SOCKETBYTEORDER

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**@defgroup  SocketEnums DataTypes
 * @ingroup SocketHeader
 * Specific Enumeration-typedefs used for socket operations
 * @{
 */
/*!
@typedef    \
    SOCKET

@brief
    Definition for socket handler data type.
    Socket ID,used with all socket operations to uniquely identify the socket handler.
    The ID is uniquely assigned at socket creation when calling @ref socket operation.
*/
typedef sint8  SOCKET;

/*!
@enum   \
    tenuSockErrSource

@brief
    Source of socket error (local, remote or unknown).

@see    tstrSockErr
*/

typedef enum {
    SOCKET_ERR_UNKNOWN = 0,
    /*!<
        No detail available (also used when there is no error).
    */
    SOCKET_ERR_TLS_REMOTE,
    /*!<
        TLS Error Alert received from peer.
    */
    SOCKET_ERR_TLS_LOCAL
    /*!<
        TLS Error Alert generated locally.
    */
} tenuSockErrSource;


/*!
@struct \
    in_addr

@brief
    IPv4 address representation.

    This structure is used as a placeholder for IPV4 address in other structures.
@see
    sockaddr_in
*/
typedef struct {
    uint32      s_addr;
    /*!<
        Network Byte Order representation of the IPv4 address. For example,
        the address "192.168.0.10" is represented as 0x0A00A8C0.
    */
} in_addr;

/*!
@struct \
    sockaddr

@brief
    Generic socket address structure.

@see
      sockaddr_in
*/
struct sockaddr {
    uint16      sa_family;
    /*!<
        Socket address family.
    */
    uint8       sa_data[14];
    /*!<
        Maximum size of all the different socket address structures.
    */
};

/*!
@struct \
    sockaddr_in

@brief
    Socket address structure for IPV4 addresses. Used to specify socket address information to connect to.
    Can be cast to @ref sockaddr structure.
*/
struct sockaddr_in {
    uint16          sin_family;
    /*!<
        Specifies the address family(AF).
        Members of AF_INET address family are IPv4 addresses.
        Hence,the only supported value for this is AF_INET.
    */
    uint16          sin_port;
    /*!<
        Port number of the socket.
        Network sockets are identified by a pair of IP addresses and port number.
        It must be set in the Network Byte Order format , @ref _htons (e.g. _htons(80)).
        Can NOT have zero value.
    */
    in_addr         sin_addr;
    /*!<
        IP Address of the socket.
        The IP address is of type @ref in_addr structure.
        Can be set to "0" to accept any IP address for server operation.
    */
    uint8           sin_zero[8];
    /*!<
        Padding to make structure the same size as @ref sockaddr.
    */
};

/*!
@struct \
    tstrSockErr

@brief
    Detail about socket failures. Used with @ref get_error_detail.
*/
typedef struct {
    tenuSockErrSource   enuErrSource;
    /*!<
        Source of socket error (local, remote or unknown).
    */
    uint8               u8ErrCode;
    /*!<
        TLS Alert code as defined in
        https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-6.
    */
} tstrSockErr;
/**@}*/     //SocketEnums

/**@defgroup  AsyncCallback Asynchronous Events
 * @ingroup SocketEnums
 * Specific Enumeration used for asynchronous operations
 * @{ */
/*!
@enum   \
    tenuSocketCallbackMsgType

@brief
    Asynchronous APIs make use of callback functions in-order to return back the results once the corresponding socket operation is completed.
    Hence resuming the normal execution of the application code while the socket operation returns the results.
    Callback functions expect event messages to be passed in, in-order to identify the operation they're returning the results for.
    The following enum identifies the type of events that are received in the callback function.

    Application Use:
    In order for application developers to handle the pending events from the network controller through the callback functions,
    a function call must be made to the function @ref m2m_wifi_handle_events at least once for each socket operation.

@see     bind
@see     listen
@see     accept
@see     connect
@see     send
@see     recv
*/

typedef enum {
    SOCKET_MSG_BIND = 1,
    /*!<
        Bind socket event.
    */
    SOCKET_MSG_LISTEN,
    /*!<
        Listen socket event.
    */
    SOCKET_MSG_DNS_RESOLVE,
    /*!<
        DNS Resolution event.
    */
    SOCKET_MSG_ACCEPT,
    /*!<
        Accept socket event.
    */
    SOCKET_MSG_CONNECT,
    /*!<
        Connect socket event.
    */
    SOCKET_MSG_RECV,
    /*!<
        Receive socket event.
    */
    SOCKET_MSG_SEND,
    /*!<
        Send socket event.
    */
    SOCKET_MSG_SENDTO,
    /*!<
        Sendto socket event.
    */
    SOCKET_MSG_RECVFROM,
    /*!<
        Recvfrom socket event.
    */
    SOCKET_MSG_SECURE
/*!<
        Existing socket made secure event.
*/
} tenuSocketCallbackMsgType;

/*!
@struct \
    tstrSocketBindMsg

@brief  Socket bind status.

    An asynchronous call to the @ref bind socket operation, returns information through this structure in response.
    This structure together with the event @ref SOCKET_MSG_BIND are passed in parameters to the callback function.
@see
     bind
*/
typedef struct {
    sint8       status;
    /*!<
        The result of the bind operation.
        Holding a value of ZERO for a successful bind or otherwise a negative
        error code corresponding to @ref SocketErrorCode.
    */
} tstrSocketBindMsg;

/*!
@struct \
    tstrSocketListenMsg

@brief  Socket listen status.

    Socket listen information is returned through this structure in response to the asynchronous call to the @ref listen function.
    This structure together with the event @ref SOCKET_MSG_LISTEN are passed-in parameters to the callback function.
@see
      listen
*/
typedef struct {
    sint8       status;
    /*!<
        Holding a value of ZERO for a successful listen or otherwise a negative
        error code corresponding to @ref SocketErrorCode.
    */
} tstrSocketListenMsg;

/*!
@struct \
    tstrSocketAcceptMsg

@brief  Socket accept status.

    Socket accept information is returned through this structure in response to the asynchronous call to the @ref accept function.
    This structure together with the event @ref SOCKET_MSG_ACCEPT are passed-in parameters to the callback function.
*/
typedef struct {
    SOCKET      sock;
    /*!<
        On a successful @ref accept operation, the return information is the socket ID for the accepted connection with the remote peer.
        Otherwise a negative error code is returned to indicate failure of the accept operation.
    */
    struct      sockaddr_in strAddr;
    /*!<
        Socket address structure for the remote peer.
    */
} tstrSocketAcceptMsg;

/*!
@struct \
    tstrSocketConnectMsg

@brief  Socket connect status.

    Socket connect information is returned through this structure in response to an asynchronous call to the @ref connect socket function
    or the @ref secure socket function.
    This structure and the event @ref SOCKET_MSG_CONNECT or @ref SOCKET_MSG_SECURE are passed in parameters to the callback function.
    If the application receives this structure with a negative value in s8Error, the application should call @ref close().
*/
typedef struct {
    SOCKET  sock;
    /*!<
        Socket ID referring to the socket passed to the @ref connect or @ref secure function call.
    */
    sint8       s8Error;
    /*!<
        Connect error code:\n
        - ZERO for a successful connect or successful secure. \n
        - Otherwise a negative error code corresponding to the type of error.
    */
} tstrSocketConnectMsg;

/*!
@struct \
    tstrSocketRecvMsg

@brief  Socket recv status.

    Socket receive information is returned through this structure in response to the asynchronous call to the @ref recv or @ref recvfrom socket functions.
    This structure, together with the events @ref SOCKET_MSG_RECV or @ref SOCKET_MSG_RECVFROM, is passed-in parameters to the callback function.
@remark
    After receiving this structure, the application should issue a new call to @ref recv or @ref recvfrom in order to receive subsequent data.\n
    In the case of @ref SOCKET_MSG_RECVFROM (UDP), any further data in the same datagram is dropped, then subsequent datagrams are buffered on the WINC until the application provides a buffer via a new call to @ref recvfrom \n
    In the case of @ref SOCKET_MSG_RECV (TCP), all subsequent data is buffered on the WINC until the application provides a buffer via a new call to @ref recv \n
    A negative or zero buffer size indicates an error with the following code:
    @ref SOCK_ERR_NO_ERROR          : Socket connection closed. The application should now call @ref close().
    @ref SOCK_ERR_CONN_ABORTED      : Socket connection aborted. The application should now call @ref close().
    @ref SOCK_ERR_TIMEOUT           : Socket receive timed out. The socket connection remains open.
*/
typedef struct {
    uint8                   *pu8Buffer;
    /*!<
        Pointer to the USER buffer (passed to @ref recv and @ref recvfrom function) containing the received data chunk.
    */
    sint16                  s16BufferSize;
    /*!<
        The received data chunk size.
        Holds a negative value if there is a receive error or ZERO on success upon reception of close socket message.
    */
    uint16                  u16RemainingSize;
    /*!<
        This field is used internally by the driver. In normal operation, this field will be 0 when the application receives this structure.
    */
    struct sockaddr_in      strRemoteAddr;
    /*!<
        Socket address structure for the remote peer. It is valid for @ref SOCKET_MSG_RECVFROM event.
    */
} tstrSocketRecvMsg;
/**@}*/     //AsyncCallback

/**@defgroup SocketCallbacks Callbacks
 * @ingroup SocketHeader
 * @{
 */
/*!
@typedef \
    tpfAppSocketCb

@brief
                The main socket application callback function. Applications register their main socket application callback through this function by calling  @ref registerSocketCallback.
                In response to events received, the following callback function is called to handle the corresponding asynchronous function called. Example: @ref bind, @ref connect,...etc.

@param [in] sock
                Socket ID for the callback.

                The socket callback function is called whenever a new event is received in response
                to socket operations.

@param [in] u8Msg
                 Socket event type. Possible values are:
                  - @ref SOCKET_MSG_BIND
                  - @ref SOCKET_MSG_LISTEN
                  - @ref SOCKET_MSG_ACCEPT
                  - @ref SOCKET_MSG_CONNECT
                  - @ref SOCKET_MSG_RECV
                  - @ref SOCKET_MSG_SEND
                  - @ref SOCKET_MSG_SENDTO
                  - @ref SOCKET_MSG_RECVFROM
                  - @ref SOCKET_MSG_SECURE

@param [in] pvMsg
                Pointer to message structure. Existing types are:
                  - tstrSocketBindMsg
                  - tstrSocketListenMsg
                  - tstrSocketAcceptMsg
                  - tstrSocketConnectMsg
                  - tstrSocketRecvMsg

@see
    tenuSocketCallbackMsgType
    tstrSocketRecvMsg
    tstrSocketConnectMsg
    tstrSocketAcceptMsg
    tstrSocketListenMsg
    tstrSocketBindMsg
*/
typedef void (*tpfAppSocketCb)(SOCKET sock, uint8 u8Msg, void *pvMsg);

/*!
@typedef    \
    tpfAppResolveCb

@brief
        DNS resolution callback function.
    Applications requiring DNS resolution should register their callback through this function by calling @ref registerSocketCallback.
    The following callback is triggered in response to an asynchronous call to the @ref gethostbyname function (DNS Resolution callback).

@param[in]  pu8DomainName
                Domain name of the host.

@param[in]  u32ServerIP
                Server IPv4 address encoded in Network byte order format. If it is Zero, then the DNS resolution failed.
*/
typedef void (*tpfAppResolveCb)(uint8 *pu8DomainName, uint32 u32ServerIP);

/*!
@typedef \
    tpfPingCb

@brief  PING Callback

    The function delivers the ping statistics for the sent ping triggered by calling
    @ref m2m_ping_req.

@param[in]  u32IPAddr
                Destination IP.

@param[in]  u32RTT
                Round Trip Time.

@param[in]  u8ErrorCode
                Ping error code. It may be one of:
                - PING_ERR_SUCCESS
                - PING_ERR_DEST_UNREACH
                - PING_ERR_TIMEOUT
*/
typedef void (*tpfPingCb)(uint32 u32IPAddr, uint32 u32RTT, uint8 u8ErrorCode);
/**@}*/     //SocketCallbacks

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
FUNCTION PROTOTYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/** @defgroup SocketAPI Functions
 *  @ingroup SocketHeader
 */

/** @defgroup SocketInitializationFn socketInit
 *  @ingroup SocketAPI
 *     The function performs the necessary initializations for the socket library through the following steps:
    - A check made by the global variable gbSocketInit, ensuring that initialization for sockets is performed only once,
     in-order to prevent resetting the socket instances already created in the global socket array (gastrSockets).
    - Zero initializations to the global socket array (gastrSockets), which holds the list of TCP sockets.
    - Registers the socket (Host Interface)hif callback function through the call to the hif_register_cb function.
       This facilitates handling  all of the socket related functions received through interrupts from the firmware.
 */
/**@{*/
/*!
@fn \
    NMI_API void socketInit(void);

@return     void

@remarks
    This initialization function must be invoked before any socket operation is performed.
    No error codes from this initialization function since the socket array is statically allocated based in the maximum number of
    sockets @ref MAX_SOCKET based on the systems capability.
\section socketInit_Ex Example
This example demonstrates the use of the socketinit for socket initialization for an mqtt chat application.
 \code
    tstrWifiInitParam   param;
    tstrNetworkId       strNetworkId;
    tstrAuthPsk         strAuthPsk;
    int8_t ret;
    char topic[strlen(MAIN_CHAT_TOPIC) + MAIN_CHAT_USER_NAME_SIZE + 1];

    //Initialize the board.
    system_init();

    //Initialize the UART console.
    configure_console();

    // Initialize the BSP.
    nm_bsp_init();

    ----------

    // Initialize socket interface.
    socketInit();
    registerSocketCallback(socket_event_handler, socket_resolve_handler);

    // Connect to router.
    strNetworkId.pu8Bssid = NULL;
    strNetworkId.pu8Ssid = MAIN_WLAN_SSID;
    strNetworkId.u8SsidLen = sizeof(MAIN_WLAN_SSID);
    strNetworkId.u8Channel = M2M_WIFI_CH_ALL;

    strAuthPsk.pu8Psk = NULL;
    strAuthPsk.pu8Passphrase = MAIN_WLAN_PSK;
    strAuthPsk.u8PassphraseLen = (uint8)strlen((char*)MAIN_WLAN_PSK);

    m2m_wifi_connect_psk(WIFI_CRED_SAVE_ENCRYPTED, &strNetworkId, &strAuthPsk);
\endcode
*/
NMI_API void socketInit(void);
/** @} */     //SocketInitializationFn

/** @defgroup SocketDeInitializationFn socketDeInit
 *  @ingroup SocketAPI
 */
/**@{*/
/*!
@fn \
    NMI_API void socketDeinit(void);

@brief  Socket Layer De-initialization

    The function performs the necessary cleanup for the socket library static data
    It must be invoked only after all desired socket operations have been performed on any active sockets.
*/
NMI_API void socketDeinit(void);
/** @} */     //SocketDeInitializationFn

/** @defgroup SocketStateFn socketState
 *  @ingroup SocketAPI
 */
/**@{*/
/*!
@fn \
    uint8 IsSocketReady(void);

@see            socketInit
                socketDeinit
@return         If the socket has been initialized and is ready.
                Should return 1 after @ref socketInit and 0 after @ref socketDeinit
*/
NMI_API uint8 IsSocketReady(void);
/** @} */     //SocketStateFn

/** @defgroup SocketCallbackFn registerSocketCallback
 *    @ingroup SocketAPI
 *    Register two callback functions one for asynchronous socket events and the other one for DNS callback registering function.
 *    The registered callback functions are used to retrieve information in response to the asynchronous socket functions called.
 */
/**@{*/
/*!
@fn \
    NMI_API void registerSocketCallback(tpfAppSocketCb socket_cb, tpfAppResolveCb resolve_cb);

@param[in]  socket_cb   tpfAppSocketCb
                Assignment of callback function to the global callback @ref tpfAppSocketCb gpfAppSocketCb. Delivers
                socket messages to the host application. In response to the asynchronous function calls, such as @ref bind
                @ref listen @ref accept @ref connect

@param[in]  resolve_cb  tpfAppResolveCb
                Assignment of callback function to the global callback @ref tpfAppResolveCb gpfAppResolveCb.
                Used for DNS resolving. The DNS resolving technique is determined by the application
                registering the callback.
                NULL is assigned when, DNS resolution is not required.

@return     void
@remarks
        If the socket functionality is not to be used, NULL is passed in as a parameter.
        It must be invoked after socketinit and before other socket layer operations.

\section registerSocketCallback_Ex Example
    This example demonstrates the use of the registerSocketCallback to register a socket callback function with DNS resolution CB set to null
    for a simple UDP server example.
 \code
    tstrWifiInitParam   param;
    tstrNetworkId       strNetworkId;
    tstrAuthPsk         strAuthPsk;
    int8_t ret;
    struct sockaddr_in addr;

    // Initialize the board
    system_init();

    //Initialize the UART console.
    configure_console();

    // Initialize the BSP.
    nm_bsp_init();

    // Initialize socket address structure.
    addr.sin_family = AF_INET;
    addr.sin_port = _htons(MAIN_WIFI_M2M_SERVER_PORT);
    addr.sin_addr.s_addr = _htonl(MAIN_WIFI_M2M_SERVER_IP);

    // Initialize Wi-Fi parameters structure.
    memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

    // Initialize Wi-Fi driver with data and status callbacks.
    param.pfAppWifiCb = wifi_cb;
    ret = m2m_wifi_init(&param);
    if (M2M_SUCCESS != ret) {
        printf("main: m2m_wifi_init call error!(%d)\r\n", ret);
        while (1) {
        }
    }

    // Initialize socket module
    socketInit();
    registerSocketCallback(socket_cb, NULL);

    // Connect to router.
    strNetworkId.pu8Bssid = NULL;
    strNetworkId.pu8Ssid = MAIN_WLAN_SSID;
    strNetworkId.u8SsidLen = sizeof(MAIN_WLAN_SSID);
    strNetworkId.u8Channel = M2M_WIFI_CH_ALL;

    strAuthPsk.pu8Psk = NULL;
    strAuthPsk.pu8Passphrase = MAIN_WLAN_PSK;
    strAuthPsk.u8PassphraseLen = (uint8)strlen((char*)MAIN_WLAN_PSK);

    m2m_wifi_connect_psk(WIFI_CRED_SAVE_ENCRYPTED, &strNetworkId, &strAuthPsk);
\endcode
*/
NMI_API void registerSocketCallback(tpfAppSocketCb socket_cb, tpfAppResolveCb resolve_cb);
/** @} */     //SocketCallbackFn

/** @defgroup SocketFn socket
 *    @ingroup SocketAPI
 *  Synchronous socket allocation function based on the specified socket type. Created sockets are non-blocking and their possible types are either TCP or a UDP sockets.
 *  The maximum allowed number of TCP sockets is @ref TCP_SOCK_MAX sockets while the maximum number of UDP sockets that can be created simultaneously is @ref UDP_SOCK_MAX sockets.
 *
 */
/**@{*/
/*!
@fn \
    NMI_API SOCKET socket(uint16 u16Domain, uint8 u8Type, uint8 u8Config);


@param [in] u16Domain
                Socket family. The only allowed value is AF_INET (IPv4.0) for TCP/UDP sockets.

@param[in]  u8Type
                Socket type. Allowed values are:
                - @ref SOCK_STREAM
                - @ref SOCK_DGRAM

@param[in]  u8Config
                Used to specify the socket configuration. The following
                configuration values are defined:\n
                - @ref SOCKET_CONFIG_SSL_OFF    : The socket is not secured by TLS.\n
                - @ref SOCKET_CONFIG_SSL_ON     : The socket is secured by TLS.
                This value has no effect if u8Type is @ref SOCK_DGRAM
                - @ref SOCKET_CONFIG_SSL_DELAY  : The socket is not secured by
                TLS, but may be secured later, by calling @ref secure.
                This value has no effect if u8Type is @ref SOCK_DGRAM \n
                All other configuration values are reserved and should not be
                used.

@pre
    The @ref socketInit function must be called once at the beginning of the application to initialize the socket handler.
    before any call to the @ref socket function can be made.

@see
    connect
    secure
    bind
    listen
    accept
    recv
    recvfrom
    send
    sendto
    close
    setsockopt
    getsockopt

@return
    On successful socket creation, a non-blocking socket type is created and a socket ID is returned
    In case of failure the function returns a negative value, identifying one of the socket error codes defined.
    For example: @ref SOCK_ERR_INVALID for invalid argument or @ref SOCK_ERR_MAX_TCP_SOCK  if the number of TCP
    allocated sockets exceeds the number of available sockets.

@remarks
        The socket function must be called a priori to any other related socket functions "e.g. send, recv, close ..etc"
\section Socket_Ex Allocation example
    This example demonstrates the use of the socket function to allocate the socket, returning the socket handler to be used for other
socket operations. Socket creation is dependent on the socket type.

UDP example

@code
    SOCKET UdpServerSocket = -1;

    UdpServerSocket = socket(AF_INET, SOCK_DGRAM, 0);

@endcode

TCP example

@code
    static SOCKET tcp_client_socket = -1;

    tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0));
@endcode

SSL example

@code
static SOCKET ssl_socket = -1;

ssl_socket = socket(AF_INET, SOCK_STREAM, SOCK_FLAGS_SSL));
@endcode
*/
NMI_API SOCKET WINC1500_EXPORT(socket)(uint16 u16Domain, uint8 u8Type, uint8 u8Config);
/** @} */     //SocketFn

/** @defgroup BindFn bind
 *  @ingroup SocketAPI
*   Asynchronous bind function associates the provided address and local port to the socket.
*   The function can be used with both TCP and UDP sockets. It is mandatory to call the @ref bind function before starting any UDP or TCP server operation.
*   Upon socket bind completion, the application will receive a @ref SOCKET_MSG_BIND message in the socket callback.
*/
/**@{*/
/*!
\fn \
    NMI_API sint8 bind(SOCKET sock, struct sockaddr *pstrAddr, uint8 u8AddrLen);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.

@param[in]  pstrAddr
                Pointer to socket address structure @ref sockaddr_in.

@param[in]  u8AddrLen
                Size of the given socket address structure in bytes.

@pre
    The socket function must be called to allocate a socket before passing the socket ID to the bind function.

@see    socket
@see    connect
@see    listen
@see    accept
@see    recv
@see    recvfrom
@see    send
@see    sendto

@return
    The function returns ZERO for successful operations and a negative value otherwise.
    The possible error values are:
    - @ref SOCK_ERR_NO_ERROR
        Indicating that the operation was successful.

    - @ref SOCK_ERR_INVALID_ARG
        Indicating passing invalid arguments such as negative socket ID or NULL socket address structure.

    - @ref SOCK_ERR_INVALID
        Indicate socket bind failure.
\section bind_Ex Example
    This example demonstrates the call of the bind socket operation after a successful socket operation.
@code
    struct sockaddr_in  addr;
    SOCKET udpServerSocket =-1;
    int ret = -1;

    if(udpServerSocket == -1)
    {
        udpServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if(udpServerSocket >= 0)
        {
            addr.sin_family         = AF_INET;
            addr.sin_port           = _htons(1234);
            addr.sin_addr.s_addr    = 0;
            ret = bind(udpServerSocket,(struct sockaddr*)&addr,sizeof(addr));

            if(ret != 0)
            {
                printf("Bind Failed. Error code = %d\n",ret);
                close(udpServerSocket);
            }
        }
        else
        {
            printf("UDP Server Socket Creation Failed\n");
            return;
        }
    }
@endcode
*/
NMI_API sint8 WINC1500_EXPORT(bind)(SOCKET sock, struct sockaddr *pstrAddr, uint8 u8AddrLen);
/**@}*/     //BindFn

/** @defgroup ListenFn listen
 *   @ingroup SocketAPI
 *  After successful socket binding to an IP address and port on the system, start listening on a passive socket for incoming connections.
       The socket must be bound on a local port or the listen operation fails.
       Upon the call to the asynchronous listen function, response is received through the event @ref SOCKET_MSG_LISTEN
    in the socket callback.
    A successful listen means the TCP server operation is active. If a connection is accepted, then the application socket callback function is
    notified with the new connected socket through the event @ref SOCKET_MSG_ACCEPT. Hence there is no need to call the @ref accept function
    after calling @ref listen.

    After a connection is accepted, the user is then required to call @ref recv to receive any packets transmitted by the remote host or to receive notification of socket connection
    termination.
 */
/**@{*/
/*!
@fn \
    NMI_API sint8 listen(SOCKET sock, uint8 backlog);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.

@param[in]  backlog
                Not used by the current implementation.

@pre
    The bind function must be called to assign the port number and IP address to the socket before the listen operation.

@see    bind
@see    accept
@see    recv
@see    recvfrom
@see    send
@see    sendto

@return
    The function returns ZERO for successful operations and a negative value otherwise.
    The possible error values are:
    - @ref SOCK_ERR_NO_ERROR
        Indicating that the operation was successful.

    - @ref SOCK_ERR_INVALID_ARG
        Indicating passing invalid arguments such as negative socket ID.

    - @ref SOCK_ERR_INVALID
        Indicate socket listen failure.
\section listen_Ex Example
This example demonstrates the call of the listen socket operation after a successful socket operation.
@code
    static void TCP_Socketcallback(SOCKET sock, uint8 u8Msg, void * pvMsg)
    {
        int ret =-1;

        switch(u8Msg)
        {
        case SOCKET_MSG_BIND:
            {
                tstrSocketBindMsg   *pstrBind = (tstrSocketBindMsg*)pvMsg;
                if(pstrBind != NULL)
                {
                    if(pstrBind->status == 0)
                    {
                        ret = listen(sock, 0);

                        if(ret <0)
                            printf("Listen failure! Error = %d\n",ret);
                    }
                    else
                    {
                        M2M_ERR("bind Failure!\n");
                        close(sock);
                    }
                }
            }
            break;

        case SOCKET_MSG_LISTEN:
            {

                tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg*)pvMsg;
                if(pstrListen != NULL)
                {
                    if(pstrListen->status == 0)
                    {
                        ret = accept(sock,NULL,0);
                    }
                    else
                    {
                        M2M_ERR("listen Failure!\n");
                        close(sock);
                    }
                }
            }
            break;

        case SOCKET_MSG_ACCEPT:
            {
                tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg*)pvMsg;

                if(pstrAccept->sock >= 0)
                {
                    TcpNotificationSocket = pstrAccept->sock;
                    recv(pstrAccept->sock,gau8RxBuffer,sizeof(gau8RxBuffer),TEST_RECV_TIMEOUT);
                }
                else
                {
                    M2M_ERR("accept failure\n");
                }
            }
            break;

        default:
            break;
        }
    }
@endcode
*/
NMI_API sint8 WINC1500_EXPORT(listen)(SOCKET sock, uint8 backlog);
/**@}*/     //ListenFn

/** @defgroup AcceptFn accept
 *    @ingroup SocketAPI
 *  The function has no current implementation. An empty declaration is used to prevent errors when legacy application code is used.
 *     For recent application use, the accept function can be safer as it has no effect and could be safely removed from any application using it.
 */
/**@{*/
/*!
@fn \
    NMI_API sint8 accept(SOCKET sock, struct sockaddr *addr, uint8 *addrlen);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.
@param[in]  addr
                Not used in the current implementation.

@param[in]  addrlen
                Not used in the current implementation.

@return
    The function returns ZERO for successful operations and a negative value otherwise.
    The possible error values are:
    - @ref SOCK_ERR_NO_ERROR
        Indicating that the operation was successful.

    - @ref SOCK_ERR_INVALID_ARG
        Indicating passing invalid arguments such as negative socket ID.
*/
NMI_API sint8 WINC1500_EXPORT(accept)(SOCKET sock, struct sockaddr *addr, uint8 *addrlen);
/** @} */     //AcceptFn

/** @defgroup ConnectFn connect
    @ingroup SocketAPI
        Establishes a TCP connection with a remote server.
    The asynchronous connect function must be called after receiving a valid socket ID from the @ref socket function.
    The application socket callback function is notified of the result of the connection attempt through the event @ref SOCKET_MSG_CONNECT,
    along with a structure @ref tstrSocketConnectMsg.
    If socket connection fails, the application should call @ref close().
    A successful connect means the TCP session is active. The application is then required to make a call to the @ref recv
    to receive any packets transmitted by the remote server, unless the application is interrupted by a notification of socket connection
    termination.
 */
/**@{*/
/*!
@fn \
    NMI_API sint8 connect(SOCKET sock, struct sockaddr *pstrAddr, uint8 u8AddrLen);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.

@param[in]  pstrAddr
                Pointer to socket address structure @ref sockaddr_in.

@param[in]  u8AddrLen
                Size of the given socket address structure in bytes.
                Not currently used, implemented for BSD compatibility only.

@pre
    The socket function must be called to allocate a TCP socket before passing the socket ID to the bind function.
    If the socket is not bound, you do NOT have to call bind before the "connect" function.

@see
    socket
    recv
    send
    close

@return
    The function returns ZERO for successful operations and a negative value otherwise.
    The possible error values are:
    - @ref SOCK_ERR_NO_ERROR
        Indicating that the operation was successful.

    - @ref SOCK_ERR_INVALID_ARG
        Indicating passing invalid arguments such as negative socket ID or NULL socket address structure.

    - @ref SOCK_ERR_INVALID
        Indicate socket connect failure.
\section connect_Ex Example
   The example demonstrates a TCP application, showing how the asynchronous call to the connect function is made through the main function and how the
   callback function handles the @ref SOCKET_MSG_CONNECT event.

   Main Function

@code
    struct sockaddr_in  Serv_Addr;
    SOCKET TcpClientSocket =-1;
    int ret = -1

    TcpClientSocket = socket(AF_INET,SOCK_STREAM,0);
    Serv_Addr.sin_family = AF_INET;
    Serv_Addr.sin_port = _htons(1234);
    Serv_Addr.sin_addr.s_addr = inet_addr(SERVER);
    printf("Connected to server via socket %u\n",TcpClientSocket);

    do
    {
        ret = connect(TcpClientSocket,(sockaddr_in*)&Serv_Addr,sizeof(Serv_Addr));
        if(ret != 0)
        {
            printf("Connection Error\n");
        }
        else
        {
            printf("Connection successful.\n");
            break;
        }
    }while(1)
@endcode

Socket Callback

@code
    if(u8Msg == SOCKET_MSG_CONNECT)
    {
        tstrSocketConnectMsg    *pstrConnect = (tstrSocketConnectMsg*)pvMsg;
        if(pstrConnect->s8Error == 0)
        {
            uint8   acBuffer[GROWL_MSG_SIZE];
            uint16  u16MsgSize;

            printf("Connect success!\n");

            u16MsgSize = FormatMsg(u8ClientID, acBuffer);
            send(sock, acBuffer, u16MsgSize, 0);
            recv(pstrNotification->Socket, (void*)au8Msg,GROWL_DESCRIPTION_MAX_LENGTH, GROWL_RX_TIMEOUT);
            u8Retry = GROWL_CONNECT_RETRY;
        }
        else
        {
            M2M_DBG("Connection Failed, Error: %d\n",pstrConnect->s8Error");
            close(pstrNotification->Socket);
        }
    }
@endcode
*/
NMI_API sint8 WINC1500_EXPORT(connect)(SOCKET sock, struct sockaddr *pstrAddr, uint8 u8AddrLen);

/**@}*/     //ConnectFn

/** @defgroup SecureFn secure
    @ingroup SocketAPI
        Converts an (insecure) TCP connection with a remote server into a secure TLS-over-TCP connection.
    It may be called after both of the following:\n
    - a TCP socket has been created by the @ref socket function, with u8Config parameter set to
    @ref SOCKET_CONFIG_SSL_DELAY \n
    - a successful connection has been made on the socket via the @ref connect function.
    This is an asynchronous API; the application socket callback function is notified of the result
    of the attempt to make the connection secure through the event @ref SOCKET_MSG_SECURE, along
    with a structure @ref tstrSocketConnectMsg.
    If the attempt to make the connection secure fails, the application should call @ref close()
 */
/**@{*/
/*!
@fn \
    sint8 secure(SOCKET sock);

@param[in]  sock
                Socket ID, corresponding to a connected TCP socket.

@pre
    @ref socket and @ref connect must be called to connect a TCP socket before passing the socket ID to this function.
    Value @ref SOCKET_CONFIG_SSL_DELAY must have been set in the u8Config parameter that was passed to @ref socket.

@see
    socket
    connect

@return
    The function returns SOCK_ERR_NO_ERROR for successful operations and a negative error value otherwise.
    The possible error values are:
    - @ref SOCK_ERR_INVALID_ARG
        Indicating passing invalid arguments such as negative socket ID.

    - @ref SOCK_ERR_INVALID
        Indicating failure to process the request.
*/
sint8 WINC1500_EXPORT(secure)(SOCKET sock);

/**@}*/     //SecureFn

/** @defgroup ReceiveFn recv
    @ingroup SocketAPI
        An asynchronous receive function, used to retrieve data from a TCP stream.
    Before calling the recv function, a successful socket connection status must have been received through any of the two socket events
    @ref SOCKET_MSG_CONNECT or @ref SOCKET_MSG_ACCEPT, from  the socket callback. Hence, indicating that the socket is already connected to a remote
    host.
    The application receives the required data in response to this asynchronous call through the reception of the event @ref SOCKET_MSG_RECV in the
    socket callback.

    Receiving the SOCKET_MSG_RECV message in the callback with zero or negative buffer length indicates the following:
    @ref SOCK_ERR_NO_ERROR          : Socket connection closed. The application should now call @ref close().
    @ref SOCK_ERR_CONN_ABORTED      : Socket connection aborted. The application should now call @ref close().
    @ref SOCK_ERR_TIMEOUT           : Socket receive timed out. The socket connection remains open.
    The application code is expected to close the socket through the call to the @ref close function upon the appearance of the above mentioned  errors.
 */
/**@{*/
/*!
@fn \
    NMI_API sint16 recv(SOCKET sock, void *pvRecvBuf, uint16 u16BufLen, uint32 u32Timeoutmsec);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.


@param[in]  pvRecvBuf
                Pointer to a buffer that will hold the received data. The buffer is used
                in the recv callback to deliver the received data to the caller. The buffer must
                be resident in memory (heap or global buffer).

@param[in]  u16BufLen
                The buffer size in bytes.

@param[in]  u32Timeoutmsec
                Timeout for the recv function in milli-seconds. If the value is set to ZERO, the timeout
                will be set to infinite (the recv function waits forever). If the timeout period is
                elapsed with no data received, the socket will get a timeout error.
@pre
    - The socket function must be called to allocate a TCP socket before passing the socket ID to the recv function.
    - The socket in a connected state is expected to receive data through the socket interface.

@see    socket
@see    connect
@see    bind
@see    listen
@see    recvfrom
@see    close

@return
    The function returns ZERO for successful operations and a negative value otherwise.
    The possible error values are:
    - @ref SOCK_ERR_NO_ERROR
        Indicating that the operation was successful.

    - @ref SOCK_ERR_INVALID_ARG
        Indicating passing invalid arguments such as negative socket ID or NULL Receive buffer.

    - @ref SOCK_ERR_BUFFER_FULL
        Indicate socket receive failure.
\section recv_Ex Example
   The example demonstrates a code snippet for the calling of the recv function in the socket callback upon notification of the accept or connect events, and the parsing of the
   received data when the @ref SOCKET_MSG_RECV event is received.
@code
    switch(u8Msg)
    {

    case SOCKET_MSG_ACCEPT:
        {
            tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg*)pvMsg;

            if(pstrAccept->sock >= 0)
            {
                recv(pstrAccept->sock,gau8RxBuffer,sizeof(gau8RxBuffer),TEST_RECV_TIMEOUT);
            }
            else
            {
                M2M_ERR("accept\n");
            }
        }
        break;


    case SOCKET_MSG_RECV:
        {
            tstrSocketRecvMsg   *pstrRx = (tstrSocketRecvMsg*)pvMsg;

            if(pstrRx->s16BufferSize > 0)
            {
                recv(sock,gau8RxBuffer,sizeof(gau8RxBuffer),TEST_RECV_TIMEOUT);
            }
            else
            {
                printf("Socket recv Error: %d\n",pstrRx->s16BufferSize);
                close(sock);
            }
        }
        break;

    default:
        break;
    }
}
@endcode
*/
NMI_API sint16 WINC1500_EXPORT(recv)(SOCKET sock, void *pvRecvBuf, uint16 u16BufLen, uint32 u32Timeoutmsec);
/**@}*/     //ReceiveFn

/** @defgroup ReceiveFromSocketFn recvfrom
 *   @ingroup SocketAPI
 *  Receives data from a UDP Socket.
*
*   The asynchronous recvfrom function is used to retrieve data from a UDP socket. The socket must already be bound to
*   a local port before a call to the recvfrom function is made (i.e message @ref SOCKET_MSG_BIND is received
*   with successful status in the socket callback).
*
*   Upon calling the recvfrom function with a successful return code, the application is expected to receive a notification
*   in the socket callback whenever a message is received through the @ref SOCKET_MSG_RECVFROM event.
*
*   Receiving the SOCKET_MSG_RECVFROM message in the callback with zero, indicates that the socket is closed.
*   Whereby a negative buffer length indicates one of the socket error codes such as socket timeout error @ref SOCK_ERR_TIMEOUT
*
*   The recvfrom callback can also be used to show the IP address of the remote host that sent the frame by
*   using the "strRemoteAddr" element in the @ref tstrSocketRecvMsg structure. (refer to the code example)
 */
/**@{*/
/*!
@fn \
    NMI_API sint16 recvfrom(SOCKET sock, void *pvRecvBuf, uint16 u16BufLen, uint32 u32TimeoutSeconds);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.

@param[in]  pvRecvBuf
                Pointer to a buffer that will hold the received data. The buffer shall be used
                in the recv callback to deliver the received data to the caller. The buffer must
                be resident in memory (heap or global buffer).

@param[in]  u16BufLen
                The buffer size in bytes.

@param[in]  u32TimeoutSeconds
                Timeout for the recv function in milli-seconds. If the value is set to ZERO, the timeout
                will be set to infinite (the recv function waits forever).

@pre
    - The socket function must be called to allocate a UDP socket before passing the socket ID to the recvfrom function.
    - The socket corresponding to the socket ID must be successfully bound to a local port through the call to a @ref bind function.

@see
    socket
    bind
    close

@return
    The function returns ZERO for successful operations and a negative value otherwise.
    The possible error values are:
    - @ref SOCK_ERR_NO_ERROR
        Indicating that the operation was successful.

    - @ref SOCK_ERR_INVALID_ARG
        Indicating passing invalid arguments such as negative socket ID or NULL Receive buffer.

    - @ref SOCK_ERR_BUFFER_FULL
        Indicate socket receive failure.
\section recvfrom_Ex Example
   The example demonstrates a code snippet for the calling of the recvfrom function in the socket callback upon notification of a successful bind event, and the parsing of the
   received data when the @ref SOCKET_MSG_RECVFROM event is received.
@code
    switch(u8Msg)
    {
    case SOCKET_MSG_BIND:
        {
            tstrSocketBindMsg   *pstrBind = (tstrSocketBindMsg*)pvMsg;

            if(pstrBind != NULL)
            {
                if(pstrBind->status == 0)
                {
                    recvfrom(sock, gau8SocketTestBuffer, TEST_BUFFER_SIZE, 0);
                }
                else
                {
                    M2M_ERR("bind\n");
                }
            }
        }
        break;

    case SOCKET_MSG_RECVFROM:
        {
            tstrSocketRecvMsg   *pstrRx = (tstrSocketRecvMsg*)pvMsg;

            if(pstrRx->s16BufferSize > 0)
            {
                //get the remote host address and port number
                uint16 u16port = pstrRx->strRemoteAddr.sin_port;
                uint32 strRemoteHostAddr = pstrRx->strRemoteAddr.sin_addr.s_addr;

                printf("Received frame with size = %d.\tHost address=%x, Port number = %d\n\n",pstrRx->s16BufferSize,strRemoteHostAddr, u16port);

                ret = recvfrom(sock,gau8SocketTestBuffer,sizeof(gau8SocketTestBuffer),TEST_RECV_TIMEOUT);
            }
            else
            {
                printf("Socket recv Error: %d\n",pstrRx->s16BufferSize);
                ret = close(sock);
            }
        }
        break;

    default:
        break;
    }
}
@endcode
*/
NMI_API sint16 WINC1500_EXPORT(recvfrom)(SOCKET sock, void *pvRecvBuf, uint16 u16BufLen, uint32 u32Timeoutmsec);
/**@}*/     //ReceiveFromSocketFn

/** @defgroup SendFn send
 *   @ingroup SocketAPI
*  Asynchronous sending function, used to send data on a TCP/UDP socket.

*  Called by the application code when there is outgoing data available required to be sent on a specific socket handler.
*  The only difference between this function and the similar @ref sendto function, is the type of socket the data is sent on and the parameters passed in.
*  @ref send function is most commonly called for sockets in a connected state.
*  After the data is sent, the socket callback function registered using registerSocketCallback(), is expected to receive an event of type
*  @ref SOCKET_MSG_SEND holding information containing the number of data bytes sent.
 */
/**@{*/
/*!
@fn \
    NMI_API sint16 send(SOCKET sock, void *pvSendBuffer, uint16 u16SendLength, uint16 u16Flags);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.

@param[in]  pvSendBuffer
                Pointer to a buffer  holding data to be transmitted.

@param[in]  u16SendLength
                The buffer size in bytes.

@param[in]  u16Flags
                Not used in the current implementation.

@pre
    Sockets must be initialized using socketInit. \n

    For TCP Socket:\n
        Must use a successfully connected Socket (so that the intended recipient address is known ahead of sending the data).
        Hence this function is expected to be called after a successful socket connect operation(in client case or accept in the
        the server case).\n

    For UDP Socket:\n
        UDP sockets most commonly use @ref sendto function, where the destination address is defined. However, in-order to send outgoing data
        using the @ref send function, at least one successful call must be made to the @ref sendto function a priori the consecutive calls to the @ref send function,
        to ensure that the destination address is saved in the firmware.


@warning
    u16SendLength must not exceed @ref SOCKET_BUFFER_MAX_LENGTH \n
    Use a valid socket identifier through the a prior call to the @ref socket function.
    Must use a valid buffer pointer.
    Successful  completion of a call to send() does not guarantee delivery of the message,
    A negative return value indicates only locally-detected errors.

@see
    socketInit
    recv
    sendto
    socket
    connect
    accept
    sendto

@return
    The function shall return @ref SOCK_ERR_NO_ERROR for successful operation and a negative value (indicating the error) otherwise.
*/
NMI_API sint16 WINC1500_EXPORT(send)(SOCKET sock, void *pvSendBuffer, uint16 u16SendLength, uint16 u16Flags);
/**@}*/     //SendFn

/** @defgroup SendToSocketFn sendto
 *  @ingroup SocketAPI
*    Asynchronous sending function, used to send data on a UDP socket.
*    Called by the application code when there is data required to be sent on a UDP socket handler.
*    The application code is expected to receive data from a successful bounded socket node.
*    The only difference between this function and the similar @ref send function, is the type of socket the data is received on. This function works
*    only with UDP sockets.
*    After the data is sent, the socket callback function registered using registerSocketCallback(), is expected to receive an event of type
*    @ref SOCKET_MSG_SENDTO.
*/
/**@{*/
/*!
@fn \
    NMI_API sint16 sendto(SOCKET sock, void *pvSendBuffer, uint16 u16SendLength, uint16 flags, struct sockaddr *pstrDestAddr, uint8 u8AddrLen);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.

@param[in]  pvSendBuffer
                Pointer to a buffer holding data to be transmitted.
                A NULL value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.

@param[in]  u16SendLength
                The buffer size in bytes. It must not exceed @ref SOCKET_BUFFER_MAX_LENGTH.

@param[in]  flags
                Not used in the current implementation

@param[in]  pstrDestAddr
                The destination address.

@param[in]  u8AddrLen
                Destination address length in bytes.
                Not used in the current implementation, only included for BSD compatibility.
@pre
        Sockets must be initialized using socketInit.

@warning
    u16SendLength must not exceed @ref SOCKET_BUFFER_MAX_LENGTH. \n
    Use a valid socket (returned from socket ).
    A valid buffer pointer must be used (not NULL).\n
    Successful  completion of a call to sendto() does not guarantee delivery of the message,
    A negative return value indicates only locally-detected errors.

@see
    socketInit
    recvfrom
    sendto
    socket
    connect
    accept
    send

@return
    The function  returns @ref SOCK_ERR_NO_ERROR for successful operation and a negative value (indicating the error) otherwise.
*/
NMI_API sint16 WINC1500_EXPORT(sendto)(SOCKET sock, void *pvSendBuffer, uint16 u16SendLength, uint16 flags, struct sockaddr *pstrDestAddr, uint8 u8AddrLen);
/**@}*/     //SendToSocketFn

/** @defgroup CloseSocketFn close
 *  @ingroup SocketAPI
 *  Synchronous close function, releases all the socket assigned resources.
 */
/**@{*/
/*!
@fn \
    NMI_API sint8 close(SOCKET sock);

@param[in]  sock
                Socket ID, must hold a non negative value.
                A negative value will return a socket error @ref SOCK_ERR_INVALID_ARG. Indicating that an invalid argument is passed in.

@pre
        Sockets must be initialized through the call of the socketInit function.
        @ref close is called only for valid socket identifiers created through the @ref socket function.

@warning
    If @ref close is called while there are still pending messages (sent or received ) they will be discarded.

@see
    socketInit
    socket

@return
    The function returned @ref SOCK_ERR_NO_ERROR for successful operation and a negative value (indicating the error) otherwise.
*/
NMI_API sint8 WINC1500_EXPORT(close)(SOCKET sock);
/**@}*/     //CloseSocketFn

/** @defgroup InetAddressFn nmi_inet_addr
*  @ingroup SocketAPI
*  Synchronous  function which returns a BSD socket compliant Internet Protocol (IPv4) socket address.
*  This IPv4 address in the input string parameter could either be specified as a host name, or as a numeric string representation like n.n.n.n known as the IPv4 dotted-decimal format
*   (i.e. "192.168.10.1").
*  This function is used whenever an ip address needs to be set in the proper format
*  (i.e. for the @ref tstrM2MIPConfig structure).
*/
/**@{*/
/*!
@fn \
    NMI_API uint32 nmi_inet_addr(char *pcIpAddr);

@param[in]  pcIpAddr
                A null terminated string containing the IP address in IPv4 dotted-decimal address.

@return
    Unsigned 32-bit integer representing the IP address in Network byte order
    (eg. "192.168.10.1" will be expressed as 0x010AA8C0).
*/
NMI_API uint32 nmi_inet_addr(char *pcIpAddr);
/**@}*/     //InetAddressFn

/** @defgroup gethostbynameFn gethostbyname
 *  @ingroup SocketAPI
*   Asynchronous DNS resolving function. This function use DNS to resolve a domain name into the corresponding IP address.
*   A call to this function will cause a DNS request to be sent and the response will be delivered to the DNS callback function registered using registerSocketCallback()
 */
/**@{*/
/*!
@fn \
    NMI_API sint8 gethostbyname(uint8 * pcHostName);

@param[in]  pcHostName
                NULL terminated string containing the domain name for the remote host.
                Its size must not exceed @ref HOSTNAME_MAX_SIZE.

@warning
    Successful completion of a call to gethostbyname() does not guarantee success of the DNS request,
    a negative return value indicates only locally-detected errors

@see
    registerSocketCallback

@return
    - @ref SOCK_ERR_NO_ERROR
    - @ref SOCK_ERR_INVALID_ARG
*/
NMI_API sint8 WINC1500_EXPORT(gethostbyname)(uint8 *pcHostName);
/**@}*/     //gethostbynameFn

/** @defgroup sslEnableCertExpirationCheckFn sslEnableCertExpirationCheck
 *  @ingroup SocketAPI
 *  Configure the behavior of the SSL Library for Certificate Expiry Validation.
 */
/**@{*/
/*!
@fn \
NMI_API sint8 sslEnableCertExpirationCheck(tenuSslCertExpSettings enuValidationSetting);

@param [in] enuValidationSetting
                See @ref tenuSslCertExpSettings for details.

@sa     tenuSslCertExpSettings

@return
    - @ref SOCK_ERR_NO_ERROR for successful operation and negative error code otherwise.
*/
NMI_API sint8 WINC1500_EXPORT(sslEnableCertExpirationCheck)(tenuSslCertExpSettings enuValidationSetting);
/**@}*/     //sslEnableCertExpirationCheckFn

/** @defgroup SetSocketOptionFn setsockopt
 *  @ingroup SocketAPI
 *  The setsockopt() function shall set the option specified by the option_name
 *  argument, at the protocol level specified by the level argument, to the value
 *  pointed to by the option_value argument for the socket specified by the socket argument.
 */
/**@{*/
/*!
@fn \
    NMI_API sint8 setsockopt(SOCKET socket, uint8 u8Level, uint8 option_name,
       const void *option_value, uint16 u16OptionLen);

@param[in]  socket
                Socket handler.

@param[in]  u8Level
                Protocol level.\n
                Supported protocol levels are @ref SOL_SOCKET and @ref SOL_SSL_SOCKET.

@param[in]  option_name
                Option to be set.\n
                For protocol level @ref SOL_SOCKET, the supported option names are:\n
                    @ref SO_SET_UDP_SEND_CALLBACK \n
                    @ref SO_TCP_KEEPALIVE \n
                    @ref SO_TCP_KEEPIDLE \n
                    @ref SO_TCP_KEEPINTVL \n
                    @ref SO_TCP_KEEPCNT \n
                For protocol level @ref SOL_SSL_SOCKET, the supported option names are:\n
                    @ref SO_SSL_BYPASS_X509_VERIF \n
                    @ref SO_SSL_SNI \n
                    @ref SO_SSL_ENABLE_SESSION_CACHING \n
                    @ref SO_SSL_ENABLE_SNI_VALIDATION \n
                    @ref SO_SSL_ALPN \n

@param[in]  option_value
                Pointer to user provided value.

@param[in]  u16OptionLen
                Length of the option value in bytes. Refer to each option documentation for the required length.

@return
    The function shall return \ref SOCK_ERR_NO_ERROR for successful operation
    and a negative value (indicating the error) otherwise.
*/
NMI_API sint8 WINC1500_EXPORT(setsockopt)(SOCKET socket, uint8 u8Level, uint8 option_name,
                         const void *option_value, uint16 u16OptionLen);
/**@}*/     //SetSocketOptionFn

/** @defgroup GetSocketOptionsFn getsockopt
 *  @ingroup SocketAPI
 *   Get socket options retrieves
*    This Function isn't implemented yet but this is the form that will be released later.
 */
/**@{*/
/*!
@fn \
    sint8 getsockopt(SOCKET sock, uint8 u8Level, uint8 u8OptName, const void *pvOptValue, uint8 * pu8OptLen);

@brief

@param[in]  sock
                Socket Identifier.
@param[in]  u8Level
                The protocol level of the option.
@param[in]  u8OptName
                The u8OptName argument specifies a single option to get.
@param[out] pvOptValue
                The pvOptValue argument contains pointer to a buffer containing the option value.
@param[out] pu8OptLen
                Option value buffer length.
@return
    The function shall return ZERO for successful operation and a negative value otherwise.
*/
NMI_API sint8 WINC1500_EXPORT(getsockopt)(SOCKET sock, uint8 u8Level, uint8 u8OptName, const void *pvOptValue, uint8 *pu8OptLen);
/**@}*/     //GetSocketOptionsFn

/** @defgroup PingFn m2m_ping_req
 *   @ingroup SocketAPI
 *      The function sends ping request to the given IP Address.
 */
/**@{*/
/*!
@fn \
        NMI_API sint8 m2m_ping_req(uint32 u32DstIP, uint8 u8TTL, tpfPingCb fpPingCb);

@param[in]  u32DstIP
                Target Destination IP Address for the ping request. It must be represented in Network byte order.
                The function nmi_inet_addr could be used to translate the dotted decimal notation IP
                to its Network bytes order integer representative.

@param[in]  u8TTL
                IP TTL value for the ping request. If set to ZERO, the default value SHALL be used.

@param[in]  fpPingCb
                Callback will be called to deliver the ping statistics.

@warning    This API should only be used to request one ping at a time; calling this API invalidates callbacks
            for previous ping requests.
@see        nmi_inet_addr
@return     The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
*/
NMI_API sint8 m2m_ping_req(uint32 u32DstIP, uint8 u8TTL, tpfPingCb fpPingCb);

/*!
 * @fn  sint8 set_alpn_list(SOCKET sock, const char *pcProtocolList);
 *
 *  This function sets the protocol list used for application-layer protocol negotiation (ALPN).
 *  If used, it must be called after creating a SSL socket (using @ref socket) and before
 *  connecting/binding (using @ref connect or @ref bind) or securing (using @ref secure).
 *
 * @param[in]   sock
 *                  Socket ID obtained by a call to @ref socket. This is the SSL socket to which
 *                  the ALPN list applies.
 *
 * @param[in]   pcProtocolList
 *                  Pointer to the list of protocols available in the application. \n
 *                  The entries in the list must: \n
 *                  - be separated with ' ' (space). \n
 *                  - not contain ' ' (space) or '\0' (NUL). \n
 *                  - be non-zero length. \n
 *                  .
 *                  The list itself must: \n
 *                  - be terminated with '\0' (NUL). \n
 *                  - be no longer than @ref ALPN_LIST_MAX_APP_LENGTH, including separators (spaces) and terminator (NUL). \n
 *                  - contain at least one entry.
 *
 * @return  The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
 *
 * \section SocketExample9 Example
 *  The example demonstrates an application using @ref set_alpn_list and @ref get_alpn_index to negotiate secure HTTP/2
 *  (with fallback option of HTTP/1.1).

 * \subsection sub5 Main Function
 * @code
 *  SOCKET TcpClientSocket = socket(AF_INET, SOCK_STREAM, SOCKET_CONFIG_SSL_ON);
 *  if (TcpClientSocket >= 0)
 *  {
 *      struct sockaddr_in Serv_Addr = {
 *          .sin_family = AF_INET,
 *          .sin_port = _htons(1234),
 *          .sin_addr.s_addr = inet_addr(SERVER)
 *      };
 *      set_alpn_list(TcpClientSocket, "h2 http/1.1");
 *      connect(TcpClientSocket, &Serv_Addr, sizeof(Serv_Addr));
 *  }
 * @endcode
 * \subsection sub6 Socket Callback
 * @code
 *  if(u8Msg == SOCKET_MSG_CONNECT)
 *  {
 *      tstrSocketConnectMsg    *pstrConnect = (tstrSocketConnectMsg*)pvMsg;
 *      if(pstrConnect->s8Error == 0)
 *      {
 *          uint8   alpn_index = get_alpn_index(pstrConnect->sock);
 *          switch (alpn_index)
 *          {
 *              case 1:
 *                  printf("Negotiated HTTP/2\n");
 *              break;
 *              case 2:
 *                  printf("Negotiated HTTP/1.1\n");
 *              break;
 *              case 0:
 *                  printf("Protocol negotiation did not occur\n");
 *              break;
 *          }
 *      }
 *  }
 * @endcode
*/
sint8 set_alpn_list(SOCKET sock, const char *pcProtocolList);
/*!
 * @fn  sint8 get_alpn_index(SOCKET sock);
 *
 *  This function gets the index of the protocol negotiated via ALPN.
 *  It should be called when a SSL socket connection succeeds, in order to determine which
 *  application-layer protocol must be used.
 *
 * @param[in]   sock
 *                  Socket ID obtained by a call to @ref socket. This is the SSL socket to which
 *                  the ALPN applies.
 *
 * @return  The function returns:\n
 *  - >0: 1-based index of negotiated protocol with respect to the list previously provided to @ref set_alpn_list \n
 *  - 0: No negotiation occurred (eg TLS peer did not support ALPN).\n
 *  - <0: Invalid parameters (socket is not in use, or not an SSL socket).\n
 *
 * @see @ref SocketExample9
*/
sint8 get_alpn_index(SOCKET sock);

/*!
 *@fn   sint8 get_error_detail(SOCKET sock, tstrSockErr *pstrErr);
 *
 *  This function gets detail about a socket failure. The application can call this when notified
 *  of a socket failure via @ref SOCKET_MSG_CONNECT or @ref SOCKET_MSG_RECV.
 *  If used, it must be called before @ref close.

 * @param[in]   sock
 *                  Socket ID obtained by a call to @ref socket.
 *
 * @param[out]  pstrErr
 *                  Pointer to structure to be populated with the details of the socket failure.
 *
 * @return  The function returns @ref SOCK_ERR_NO_ERROR if the request is successful. In this case pstrErr
 *  has been populated.
 *  The function returns a negative value if the request is not successful. In this case pstrErr
 *  has not been populated.
*/
sint8 get_error_detail(SOCKET sock, tstrSockErr *pstrErr);
/**@}*/     //PingFn

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __SOCKET_H__ */
