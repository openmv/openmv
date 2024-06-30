/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef APP_MAP_H
#define APP_MAP_H

/* Max size of applications for each core;
 * User shall adjust this based on app need.
 */
#define _APP_MAX_SIZE_HE                 0x200000
#define _APP_MAX_SIZE_HP                 0x200000

/* Define the below flag as 1 to support booting from
 * OSPI flash.
 */
#define BOOT_FROM_OSPI_FLASH             0

/*
 * Default XIP addresses for cores:
 * By default M55_HE would boot from 0x8000_0000 (MRAM)
 * or 0xC000_0000 (OSPI FLASH). However, the user can choose
 * any other address.
 */

#if BOOT_FROM_OSPI_FLASH
#define _APP_ADDRESS_HE                  (0xC0000000)
#else
#define _APP_ADDRESS_HE                  (0x80000000)
#endif

#define _APP_ADDRESS_HP                  (_APP_ADDRESS_HE + _APP_MAX_SIZE_HE)

#endif
