#ifndef NRF51822_ERRATAS_H
#define NRF51822_ERRATAS_H

/*

Copyright (c) 2010 - 2018, Nordic Semiconductor ASA All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. Neither the name of Nordic Semiconductor ASA nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdint.h>
#include <stdbool.h>
#include "compiler_abstraction.h"

static bool errata_1(void) __UNUSED;
static bool errata_2(void) __UNUSED;
static bool errata_3(void) __UNUSED;
static bool errata_4(void) __UNUSED;
static bool errata_5(void) __UNUSED;
static bool errata_6(void) __UNUSED;
static bool errata_7(void) __UNUSED;
static bool errata_8(void) __UNUSED;
static bool errata_9(void) __UNUSED;
static bool errata_10(void) __UNUSED;
static bool errata_11(void) __UNUSED;
static bool errata_12(void) __UNUSED;
static bool errata_13(void) __UNUSED;
static bool errata_14(void) __UNUSED;
static bool errata_15(void) __UNUSED;
static bool errata_16(void) __UNUSED;
static bool errata_17(void) __UNUSED;
static bool errata_18(void) __UNUSED;
static bool errata_19(void) __UNUSED;
static bool errata_20(void) __UNUSED;
static bool errata_21(void) __UNUSED;
static bool errata_22(void) __UNUSED;
static bool errata_23(void) __UNUSED;
static bool errata_24(void) __UNUSED;
static bool errata_25(void) __UNUSED;
static bool errata_26(void) __UNUSED;
static bool errata_27(void) __UNUSED;
static bool errata_28(void) __UNUSED;
static bool errata_29(void) __UNUSED;
static bool errata_30(void) __UNUSED;
static bool errata_31(void) __UNUSED;
static bool errata_32(void) __UNUSED;
static bool errata_33(void) __UNUSED;
static bool errata_34(void) __UNUSED;
static bool errata_35(void) __UNUSED;
static bool errata_36(void) __UNUSED;
static bool errata_37(void) __UNUSED;
static bool errata_38(void) __UNUSED;
static bool errata_39(void) __UNUSED;
static bool errata_40(void) __UNUSED;
static bool errata_41(void) __UNUSED;
static bool errata_42(void) __UNUSED;
static bool errata_43(void) __UNUSED;
static bool errata_44(void) __UNUSED;
static bool errata_45(void) __UNUSED;
static bool errata_46(void) __UNUSED;
static bool errata_47(void) __UNUSED;
static bool errata_48(void) __UNUSED;
static bool errata_49(void) __UNUSED;
static bool errata_50(void) __UNUSED;
static bool errata_51(void) __UNUSED;
static bool errata_52(void) __UNUSED;
static bool errata_53(void) __UNUSED;
static bool errata_54(void) __UNUSED;
static bool errata_55(void) __UNUSED;
static bool errata_56(void) __UNUSED;
static bool errata_57(void) __UNUSED;
static bool errata_58(void) __UNUSED;
static bool errata_59(void) __UNUSED;
static bool errata_60(void) __UNUSED;
static bool errata_61(void) __UNUSED;
static bool errata_62(void) __UNUSED;
static bool errata_63(void) __UNUSED;
static bool errata_64(void) __UNUSED;
static bool errata_65(void) __UNUSED;
static bool errata_66(void) __UNUSED;
static bool errata_67(void) __UNUSED;
static bool errata_68(void) __UNUSED;
static bool errata_69(void) __UNUSED;
static bool errata_70(void) __UNUSED;
static bool errata_71(void) __UNUSED;
static bool errata_72(void) __UNUSED;
static bool errata_73(void) __UNUSED;
static bool errata_74(void) __UNUSED;
static bool errata_75(void) __UNUSED;
static bool errata_76(void) __UNUSED;
static bool errata_77(void) __UNUSED;
static bool errata_78(void) __UNUSED;

static bool errata_1(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_2(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_3(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_4(void)
{
    return false;
}

static bool errata_5(void)
{
    return false;
}

static bool errata_6(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_7(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_8(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_9(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_10(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_11(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_12(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_13(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_14(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_15(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_16(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_17(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_18(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_19(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_20(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_21(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_22(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_23(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_24(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_25(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_26(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_27(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_28(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_29(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_30(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_31(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_32(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_33(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_34(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_35(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_36(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_37(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_38(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_39(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_40(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_41(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_42(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_43(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_44(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_45(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_46(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_47(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_48(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_49(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_50(void)
{
    return false;
}

static bool errata_51(void)
{
    return false;
}

static bool errata_52(void)
{
    return false;
}

static bool errata_53(void)
{
    return false;
}

static bool errata_54(void)
{
    return false;
}

static bool errata_55(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_56(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_57(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_58(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_59(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_60(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_61(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_62(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_63(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_64(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_65(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_66(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_67(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_68(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return false;
                case 0x08ul:
                    return false;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return false;
                case 0x0Cul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_69(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_70(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_71(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return false;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_72(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return true;
                case 0x01ul:
                    return true;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_73(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return false;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_74(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_75(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_76(void)
{
    return false;
}

static bool errata_77(void)
{
    return false;
}

static bool errata_78(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x01:
            switch(var2)
            {
                case 0x00ul:
                    return false;
                case 0x01ul:
                    return false;
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x07ul:
                    return true;
                case 0x08ul:
                    return true;
                case 0x09ul:
                    return true;
                case 0x0Bul:
                    return true;
                case 0x0Cul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

#endif /* NRF51822_ERRATAS_H */
