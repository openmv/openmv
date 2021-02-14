/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Hash functions.
 */
#ifndef __HASH_H__
#define __HASH_H__
int hash_start();
int hash_update(uint8_t *buffer, uint32_t size);
int hash_digest(uint8_t *buffer, uint32_t size, uint8_t *digest);
int hash_from_file(const char *path, uint8_t *digest);
#endif /* __HASH_H__ */
