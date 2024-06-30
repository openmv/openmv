/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef OSPI_XIP_AES_KEY_H_
#define OSPI_XIP_AES_KEY_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "ospi_xip_user.h"

#if OSPI_XIP_ENABLE_AES_DECRYPTION
#define AES_128_KEY_SIZE            16
/* The 128 bit AES key to be used for decrypting the image in the flash */
static const uint8_t ospi_aes_user_key[AES_128_KEY_SIZE] = {0x00, 0x01, 0x02, 0x03,
                                                            0x04, 0x05, 0x06, 0x07,
                                                            0x08, 0x09, 0x0a, 0x0b,
                                                            0x0c, 0x0d, 0x0e, 0x0f};
#endif /* OSPI_XIP_ENABLE_AES_DECRYPTION */

#ifdef  __cplusplus
}
#endif

#endif /* OSPI_XIP_AES_KEY_H_ */

