#ifndef NRF52832_ERRATAS_H
#define NRF52832_ERRATAS_H

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
static bool errata_7(void) __UNUSED;
static bool errata_8(void) __UNUSED;
static bool errata_9(void) __UNUSED;
static bool errata_10(void) __UNUSED;
static bool errata_11(void) __UNUSED;
static bool errata_12(void) __UNUSED;
static bool errata_15(void) __UNUSED;
static bool errata_16(void) __UNUSED;
static bool errata_17(void) __UNUSED;
static bool errata_20(void) __UNUSED;
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
static bool errata_46(void) __UNUSED;
static bool errata_47(void) __UNUSED;
static bool errata_48(void) __UNUSED;
static bool errata_49(void) __UNUSED;
static bool errata_51(void) __UNUSED;
static bool errata_54(void) __UNUSED;
static bool errata_55(void) __UNUSED;
static bool errata_57(void) __UNUSED;
static bool errata_58(void) __UNUSED;
static bool errata_62(void) __UNUSED;
static bool errata_63(void) __UNUSED;
static bool errata_64(void) __UNUSED;
static bool errata_65(void) __UNUSED;
static bool errata_66(void) __UNUSED;
static bool errata_67(void) __UNUSED;
static bool errata_68(void) __UNUSED;
static bool errata_70(void) __UNUSED;
static bool errata_71(void) __UNUSED;
static bool errata_72(void) __UNUSED;
static bool errata_73(void) __UNUSED;
static bool errata_74(void) __UNUSED;
static bool errata_75(void) __UNUSED;
static bool errata_76(void) __UNUSED;
static bool errata_77(void) __UNUSED;
static bool errata_78(void) __UNUSED;
static bool errata_79(void) __UNUSED;
static bool errata_81(void) __UNUSED;
static bool errata_83(void) __UNUSED;
static bool errata_84(void) __UNUSED;
static bool errata_86(void) __UNUSED;
static bool errata_87(void) __UNUSED;
static bool errata_88(void) __UNUSED;
static bool errata_89(void) __UNUSED;
static bool errata_91(void) __UNUSED;
static bool errata_94(void) __UNUSED;
static bool errata_96(void) __UNUSED;
static bool errata_97(void) __UNUSED;
static bool errata_98(void) __UNUSED;
static bool errata_101(void) __UNUSED;
static bool errata_102(void) __UNUSED;
static bool errata_103(void) __UNUSED;
static bool errata_104(void) __UNUSED;
static bool errata_106(void) __UNUSED;
static bool errata_107(void) __UNUSED;
static bool errata_108(void) __UNUSED;
static bool errata_109(void) __UNUSED;
static bool errata_110(void) __UNUSED;
static bool errata_111(void) __UNUSED;
static bool errata_112(void) __UNUSED;
static bool errata_113(void) __UNUSED;
static bool errata_115(void) __UNUSED;
static bool errata_116(void) __UNUSED;
static bool errata_117(void) __UNUSED;
static bool errata_118(void) __UNUSED;
static bool errata_119(void) __UNUSED;
static bool errata_120(void) __UNUSED;
static bool errata_121(void) __UNUSED;
static bool errata_122(void) __UNUSED;
static bool errata_127(void) __UNUSED;
static bool errata_128(void) __UNUSED;
static bool errata_131(void) __UNUSED;
static bool errata_132(void) __UNUSED;
static bool errata_133(void) __UNUSED;
static bool errata_134(void) __UNUSED;
static bool errata_135(void) __UNUSED;
static bool errata_136(void) __UNUSED;
static bool errata_138(void) __UNUSED;
static bool errata_140(void) __UNUSED;
static bool errata_141(void) __UNUSED;
static bool errata_142(void) __UNUSED;
static bool errata_143(void) __UNUSED;
static bool errata_144(void) __UNUSED;
static bool errata_145(void) __UNUSED;
static bool errata_146(void) __UNUSED;
static bool errata_147(void) __UNUSED;
static bool errata_149(void) __UNUSED;
static bool errata_150(void) __UNUSED;
static bool errata_151(void) __UNUSED;
static bool errata_153(void) __UNUSED;
static bool errata_154(void) __UNUSED;
static bool errata_155(void) __UNUSED;
static bool errata_156(void) __UNUSED;
static bool errata_158(void) __UNUSED;
static bool errata_160(void) __UNUSED;
static bool errata_162(void) __UNUSED;
static bool errata_163(void) __UNUSED;
static bool errata_164(void) __UNUSED;
static bool errata_166(void) __UNUSED;
static bool errata_170(void) __UNUSED;
static bool errata_171(void) __UNUSED;
static bool errata_172(void) __UNUSED;
static bool errata_173(void) __UNUSED;
static bool errata_174(void) __UNUSED;
static bool errata_176(void) __UNUSED;
static bool errata_178(void) __UNUSED;
static bool errata_179(void) __UNUSED;
static bool errata_180(void) __UNUSED;
static bool errata_181(void) __UNUSED;
static bool errata_182(void) __UNUSED;
static bool errata_183(void) __UNUSED;
static bool errata_184(void) __UNUSED;
static bool errata_186(void) __UNUSED;
static bool errata_187(void) __UNUSED;
static bool errata_189(void) __UNUSED;
static bool errata_190(void) __UNUSED;
static bool errata_191(void) __UNUSED;
static bool errata_192(void) __UNUSED;
static bool errata_193(void) __UNUSED;
static bool errata_194(void) __UNUSED;
static bool errata_195(void) __UNUSED;
static bool errata_196(void) __UNUSED;
static bool errata_197(void) __UNUSED;
static bool errata_198(void) __UNUSED;
static bool errata_199(void) __UNUSED;
static bool errata_200(void) __UNUSED;
static bool errata_201(void) __UNUSED;
static bool errata_202(void) __UNUSED;
static bool errata_204(void) __UNUSED;
static bool errata_208(void) __UNUSED;
static bool errata_209(void) __UNUSED;
static bool errata_210(void) __UNUSED;
static bool errata_212(void) __UNUSED;
static bool errata_213(void) __UNUSED;
static bool errata_214(void) __UNUSED;
static bool errata_215(void) __UNUSED;
static bool errata_217(void) __UNUSED;
static bool errata_218(void) __UNUSED;
static bool errata_219(void) __UNUSED;
static bool errata_225(void) __UNUSED;

static bool errata_1(void)
{
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_15(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
                    return false;
                default:
                    return false;
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_51(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_54(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_55(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
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
    uint32_t var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
    uint32_t var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return false;
                case 0x06ul:
                    return false;
                default:
                    return false;
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return false;
                case 0x06ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_72(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
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
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return false;
                case 0x06ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_74(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
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
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_77(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_78(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_79(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_81(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_83(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_84(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_86(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_87(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_88(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_89(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_91(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_94(void)
{
    return false;
}

static bool errata_96(void)
{
    return false;
}

static bool errata_97(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_98(void)
{
    return false;
}

static bool errata_101(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_102(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_103(void)
{
    return false;
}

static bool errata_104(void)
{
    return false;
}

static bool errata_106(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_107(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_108(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_109(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_110(void)
{
    return false;
}

static bool errata_111(void)
{
    return false;
}

static bool errata_112(void)
{
    return false;
}

static bool errata_113(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_115(void)
{
    return false;
}

static bool errata_116(void)
{
    return false;
}

static bool errata_117(void)
{
    return false;
}

static bool errata_118(void)
{
    return false;
}

static bool errata_119(void)
{
    return false;
}

static bool errata_120(void)
{
    return false;
}

static bool errata_121(void)
{
    return false;
}

static bool errata_122(void)
{
    return false;
}

static bool errata_127(void)
{
    return false;
}

static bool errata_128(void)
{
    return false;
}

static bool errata_131(void)
{
    return false;
}

static bool errata_132(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_133(void)
{
    return false;
}

static bool errata_134(void)
{
    return false;
}

static bool errata_135(void)
{
    return false;
}

static bool errata_136(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_138(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_140(void)
{
    return false;
}

static bool errata_141(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_142(void)
{
    return false;
}

static bool errata_143(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_144(void)
{
    return false;
}

static bool errata_145(void)
{
    return false;
}

static bool errata_146(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_147(void)
{
    return false;
}

static bool errata_149(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_150(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_151(void)
{
    return false;
}

static bool errata_153(void)
{
    return false;
}

static bool errata_154(void)
{
    return false;
}

static bool errata_155(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_156(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_158(void)
{
    return false;
}

static bool errata_160(void)
{
    return false;
}

static bool errata_162(void)
{
    return false;
}

static bool errata_163(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_164(void)
{
    return false;
}

static bool errata_166(void)
{
    return false;
}

static bool errata_170(void)
{
    return false;
}

static bool errata_171(void)
{
    return false;
}

static bool errata_172(void)
{
    return false;
}

static bool errata_173(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_174(void)
{
    return false;
}

static bool errata_176(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_178(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_179(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_180(void)
{
    return false;
}

static bool errata_181(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return false;
                default:
                    return false;
                break;
            }
        break;

    }

    return false;
}

static bool errata_182(void)
{
    uint32_t var1 = *(uint32_t *)0x10000130ul;
    uint32_t var2 = *(uint32_t *)0x10000134ul;

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return false;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_183(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_184(void)
{
    return false;
}

static bool errata_186(void)
{
    return false;
}

static bool errata_187(void)
{
    return false;
}

static bool errata_189(void)
{
    return false;
}

static bool errata_190(void)
{
    return false;
}

static bool errata_191(void)
{
    return false;
}

static bool errata_192(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_193(void)
{
    return false;
}

static bool errata_194(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_195(void)
{
    return false;
}

static bool errata_196(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_197(void)
{
    return false;
}

static bool errata_198(void)
{
    return false;
}

static bool errata_199(void)
{
    return false;
}

static bool errata_200(void)
{
    return false;
}

static bool errata_201(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_202(void)
{
    return false;
}

static bool errata_204(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_208(void)
{
    return false;
}

static bool errata_209(void)
{
    return false;
}

static bool errata_210(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_212(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_213(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return true;
                case 0x04ul:
                    return true;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_214(void)
{
    return false;
}

static bool errata_215(void)
{
    return false;
}

static bool errata_217(void)
{
    return false;
}

static bool errata_218(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_219(void)
{
    uint32_t var1;
    uint32_t var2;

    if (*(uint32_t *)0x10000130ul == 0xFFFFFFFF)
    {
        var1 = ((*(uint32_t *)0xF0000FE0ul) & 0x000000FFul);
        var2 = ((*(uint32_t *)0xF0000FE8ul) & 0x000000F0ul) >> 4;
    }
    else
    {
        var1 = *(uint32_t *)0x10000130ul;
        var2 = *(uint32_t *)0x10000134ul;
    }

    switch(var1)
    {
        case 0x06:
            switch(var2)
            {
                case 0x03ul:
                    return false;
                case 0x04ul:
                    return false;
                case 0x05ul:
                    return true;
                case 0x06ul:
                    return true;
                default:
                    return true;
                break;
            }
        break;

    }

    return false;
}

static bool errata_225(void)
{
    return false;
}

#endif /* NRF52832_ERRATAS_H */
