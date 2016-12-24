/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#define OTHER_LOG2_2(x)   (((x) &                0x2ULL) ? ( 2                           ) :                1) // NO ({ ... }) !
#define OTHER_LOG2_4(x)   (((x) &                0xCULL) ? ( 2 +  OTHER_LOG2_2((x) >>  2)) :  OTHER_LOG2_2(x)) // NO ({ ... }) !
#define OTHER_LOG2_8(x)   (((x) &               0xF0ULL) ? ( 4 +  OTHER_LOG2_4((x) >>  4)) :  OTHER_LOG2_4(x)) // NO ({ ... }) !
#define OTHER_LOG2_16(x)  (((x) &             0xFF00ULL) ? ( 8 +  OTHER_LOG2_8((x) >>  8)) :  OTHER_LOG2_8(x)) // NO ({ ... }) !
#define OTHER_LOG2_32(x)  (((x) &         0xFFFF0000ULL) ? (16 + OTHER_LOG2_16((x) >> 16)) : OTHER_LOG2_16(x)) // NO ({ ... }) !
#define OTHER_LOG2(x)     (((x) & 0xFFFFFFFF00000000ULL) ? (32 + OTHER_LOG2_32((x) >> 32)) : OTHER_LOG2_32(x)) // NO ({ ... }) !
