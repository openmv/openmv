/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/** @file drivers_common.h
*
* @brief Header file for common macros used by shared drivers
*
* @par
*/

#ifndef DRIVERS_COMMON_H
#define DRIVERS_COMMON_H

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

// Read from registers
#define READ_REGISTER_U8(Address)         (*((uint8_t volatile *)(Address)))
#define READ_REGISTER_U16(Address)        (*((uint16_t volatile *)(Address)))
#define READ_REGISTER_U32(Address)        (*((uint32_t volatile *)(Address)))

// Write to registers
#define WRITE_REGISTER_U8(Address, Value)  \
                        (*((uint8_t volatile*)(Address)) = (uint8_t)(Value))
#define WRITE_REGISTER_U16(Address, Value)  \
                        (*((uint16_t volatile*)(Address)) = (uint16_t)(Value))
#define WRITE_REGISTER_U32(Address, Value)  \
                        (*((uint32_t volatile*)(Address)) = (uint32_t)(Value))

// Bit operations
#define SET_REGISTER_BITS_U8(Address, Value)  \
                  (*(volatile uint8_t *)(Address) |= (uint8_t)(Value))
#define CLEAR_REGISTER_BITS_U8(Address, Value)  \
                  (*(volatile uint8_t *)(Address) &= ~((uint8_t)(Value)))
#define CLEAR_AND_SET_BITS_U8(Address, AndData, OrData)  \
            WRITE_REGISTER_U8(Address, (READ_REGISTER_8(Address) & ~(uint8_t)(AndData)) | (uint8_t)(OrData))

#define SET_REGISTER_BITS_U16(Address, Value)  \
                  (*(volatile uint16_t *)(Address) |= (uint16_t)(Value))
#define CLEAR_REGISTER_BITS_U16(Address, Value)  \
                  (*(volatile uint16_t *)(Address) &= ~((uint16_t)(Value)))
#define CLEAR_AND_SET_BITS_U16(Address, AndData, OrData)  \
            WRITE_REGISTER_U16(Address, (READ_REGISTER_U16(Address) & ~(uint16_t)(AndData)) | (uint16_t)(OrData))

#define SET_REGISTER_BITS_U32(Address, Value)  \
                  (*(volatile uint32_t *)(Address) |= (uint32_t)(Value))
#define CLEAR_REGISTER_BITS_U32(Address, Value)  \
                  (*(volatile uint32_t *)(Address) &= ~((uint32_t)(Value)))
#define CLEAR_AND_SET_BITS_U32(Address, AndData, OrData)  \
            WRITE_REGISTER_U32(Address, (READ_REGISTER_U32(Address) & ~(uint32_t)(AndData)) | (uint32_t)(OrData))


#endif /* DRIVERS_COMMON_H */
