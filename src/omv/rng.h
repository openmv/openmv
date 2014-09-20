/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * RNG (Random Number Generator).
 *
 */
#ifndef __RNG_H__
#define __RNG_H__
void rng_init();
uint32_t rng_randint(uint32_t min, uint32_t max);
#endif // __RNG_H__
