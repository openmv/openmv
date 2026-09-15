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

/*!
@file
    ecc_types.h

@brief  Elliptic Curve Cryptography Module Interface

    The file defines the APIs and data types that abstract the ECC operations
    required for TLS library.
*/

#ifndef __ECC_TYPES_H__
#define __ECC_TYPES_H__

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "m2m_types.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/



#define ECC_LARGEST_CURVE_SIZE                      (32)
/*!<
    The size of the the largest supported EC. For now, assuming
    the 256-bit EC is the largest supported curve type.
*/


#define ECC_POINT_MAX_SIZE                          ECC_LARGEST_CURVE_SIZE
/*!<
    Maximum size of one coordinate of an EC point.
*/


#define ECC_POINT_MAX_SIZE_WORDS                    (ECC_POINT_MAX_SIZE / 4)
/*!<
    SIZE in 32-bit words.
*/


#define ECC_NUM_SUPP_CURVES                         ((sizeof(gastrECCSuppList)) / (sizeof(tstrEllipticCurve)))
/*!<
*/


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/**@addtogroup  SSLEnums
 * @{
 */
/*!
@enum \
    tenuEcNamedCurve

@brief  EC Named Curves

    Defines a list of supported ECC named curves.
*/
typedef enum{
    EC_SECP192R1        = 19,
    /*!<
        It is defined by NIST as P192 and by the SEC Group as secp192r1.
    */
    EC_SECP256R1        = 23,
    /*!<
        It is defined by NIST as P256 and by the SEC Group as secp256r1.
    */
    EC_SECP384R1        = 24,
    /*!<
        It is defined by NIST as P384 and by the SEC Group as secp384r1.
    */
    EC_SECP521R1        = 25,
    /*!<
        It is defined by NIST as P521 and by the SEC Group as secp521r1.
    */
    EC_UNKNOWN          = 255
}tenuEcNamedCurve;


/*!
@struct \
    tstrECPoint

@brief  Elliptic Curve point representation
*/
typedef struct EcPoint{
    union {
        uint8_t XY[2*ECC_POINT_MAX_SIZE];
        /*!<
            Concatenation of X and Y coordinates of the ec point.
        */
        struct {
            uint8_t X[ECC_POINT_MAX_SIZE];
            /*!<
                The X-coordinate of the ec point.
            */
            uint8_t Y[ECC_POINT_MAX_SIZE];
            /*!<
                The Y-coordinate of the ec point.
            */
        };
    };
    uint16_t    u16Size;
    /*!<
        Point size in bytes (for each of the coordinates).
    */
    uint16_t    u16PrivKeyID;
    /*!<
        ID for the corresponding private key.
    */
}tstrECPoint;


/*!
@struct \
    tstrECDomainParam

@brief  ECC Curve Domain Parameters

    The structure defines the ECC domain parameters for curves defined over prime finite fields.
*/
typedef struct EcDomainParam{
    uint32_t    p[ECC_POINT_MAX_SIZE_WORDS];
    uint32_t    a[ECC_POINT_MAX_SIZE_WORDS];
    uint32_t    b[ECC_POINT_MAX_SIZE_WORDS];
    tstrECPoint G;
}tstrECDomainParam;


/*!
@struct \
    tstrEllipticCurve

@brief
    Definition of an elliptic curve
*/
typedef struct{
    tenuEcNamedCurve    enuType;
    tstrECDomainParam   strParam;
}tstrEllipticCurve;

/*!
@enum \
    tenuEccREQ

@brief  ECC operations that may be requested by WINC.

    These are passed in the u16REQ field of @ref tstrEccReqInfo, which is used in both the request
    from the WINC and the response to the WINC.
*/
typedef enum{
    ECC_REQ_NONE,
    /*!< */
    ECC_REQ_CLIENT_ECDH,
    /*!<
        Derive the shared secret from ECDHE key exchange as client.
    */
    ECC_REQ_SERVER_ECDH,
    /*!<
        Derive the shared secret from ECDHE key exchange as server.
    */
    ECC_REQ_GEN_KEY,
    /*!<
        Generate a key pair to be used in ECDHE key exchange as server.
    */
    ECC_REQ_SIGN_GEN,
    /*!<
        Generate the signature for a given curve and value. The value needs to be retrieved via
        @ref m2m_ssl_retrieve_hash.
    */
    ECC_REQ_SIGN_VERIFY
    /*!<
        Verify a list of signatures. Each set of value/signature/key/curve information needs to be
        retrieved via @ref m2m_ssl_retrieve_next_for_verifying.
    */
}tenuEccREQ;

/*!
@struct \
    tstrEcdhReqInfo

@brief  Information relating to operations of type @ref ECC_REQ_CLIENT_ECDH, @ref ECC_REQ_GEN_KEY
        and @ref ECC_REQ_SERVER_ECDH.
*/
typedef struct{
    tstrECPoint strPubKey;
    /*!<
        Public key. Usage depends on type of operation.
    */
    uint8_t     au8Key[ECC_POINT_MAX_SIZE];
    /*!<
        Pre master secret generated during operations of type @ref ECC_REQ_CLIENT_ECDH and
        @ref ECC_REQ_SERVER_ECDH.
    */
}tstrEcdhReqInfo;

/*!
@struct \
    tstrEcdsaVerifyReqInfo

@brief  Information relating to requests of type @ref ECC_REQ_SIGN_VERIFY.
*/
typedef struct{
    uint32_t    u32nSig;
    /*!<
        Number of sets of value/signature/key/curve information for verifying. Each set needs to be
        retrieved via @ref m2m_ssl_retrieve_next_for_verifying.
    */
}tstrEcdsaVerifyReqInfo;

/*!
@struct \
    tstrEcdsaSignReqInfo

@brief  Information relating to requests of type @ref ECC_REQ_SIGN_GEN.
*/
typedef struct{
    uint16_t    u16CurveType;
    /*!<
        The named curve to be used for signing, to be cast to type @ref tenuEcNamedCurve.
    */
    uint16_t    u16HashSz;
    /*!<
        The size of the value to be signed. The value needs to be retrieved via
        @ref m2m_ssl_retrieve_hash.
    */
}tstrEcdsaSignReqInfo;

/*!
@struct \
    tstrEccReqInfo

@brief  Information relating to ECC operations.

    This is used in both the request from the WINC and the response to the WINC.
*/
typedef struct{
    uint16_t    u16REQ;
    /*!<
        The requested ECC operation, to be cast to type @ref tenuEccREQ.
    */
    uint16_t    u16Status;
    /*!<
        The status of the operation: zero for success; non-zero for failure.
        This field should be used in responses and ignored in requests.
    */
    uint32_t    u32UserData;
    /*!<
        This value is used internally. The value set in the response must match the value received
        in the request.
    */
    uint32_t    u32SeqNo;
    /*!<
        This value is used internally. The value set in the response must match the value received
        in the request.
    */
    union{
        tstrEcdhReqInfo         strEcdhREQ;
        /*!<
            Information relating to an operation of type @ref ECC_REQ_CLIENT_ECDH,
            @ref ECC_REQ_GEN_KEY or @ref ECC_REQ_SERVER_ECDH.
        */
        tstrEcdsaSignReqInfo    strEcdsaSignREQ;
        /*!<
            Information relating to an operation of type @ref ECC_REQ_SIGN_GEN.
            This field is used in requests and ignored in responses.
        */
        tstrEcdsaVerifyReqInfo  strEcdsaVerifyREQ;
        /*!<
            Information relating to an operation of type @ref ECC_REQ_SIGN_VERIFY.
            This field is used in requests and ignored in responses.
        */
    };
}tstrEccReqInfo;
/**@}*/     //SSLEnums

#endif /* __ECC_TYPES_H__ */
