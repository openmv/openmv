#ifndef NRF5340_APPLICATION_ERRATAS_H
#define NRF5340_APPLICATION_ERRATAS_H

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

static bool errata_3(void) __UNUSED;
static bool errata_5(void) __UNUSED;
static bool errata_7(void) __UNUSED;
static bool errata_8(void) __UNUSED;
static bool errata_13(void) __UNUSED;
static bool errata_15(void) __UNUSED;
static bool errata_16(void) __UNUSED;
static bool errata_18(void) __UNUSED;
static bool errata_19(void) __UNUSED;
static bool errata_20(void) __UNUSED;
static bool errata_21(void) __UNUSED;
static bool errata_22(void) __UNUSED;
static bool errata_23(void) __UNUSED;
static bool errata_26(void) __UNUSED;
static bool errata_27(void) __UNUSED;
static bool errata_28(void) __UNUSED;
static bool errata_37(void) __UNUSED;
static bool errata_42(void) __UNUSED;
static bool errata_44(void) __UNUSED;
static bool errata_45(void) __UNUSED;
static bool errata_46(void) __UNUSED;
static bool errata_47(void) __UNUSED;
static bool errata_49(void) __UNUSED;
static bool errata_55(void) __UNUSED;
static bool errata_57(void) __UNUSED;
static bool errata_58(void) __UNUSED;
static bool errata_59(void) __UNUSED;
static bool errata_62(void) __UNUSED;
static bool errata_64(void) __UNUSED;
static bool errata_65(void) __UNUSED;

static bool errata_3(void)
{
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_5(void)
{
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_47(void)
{
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_49(void)
{
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_55(void)
{
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_57(void)
{
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_62(void)
{
    return false;
}

static bool errata_64(void)
{
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
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
    uint32_t var1 = *(uint32_t *)0x00FF0130ul;
    uint32_t var2 = *(uint32_t *)0x00FF0134ul;

    switch(var1)
    {
        case 0x07:
            switch(var2)
            {
                case 0x02ul:
                    return true;
                case 0x03ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

#endif /* NRF5340_APPLICATION_ERRATAS_H */
