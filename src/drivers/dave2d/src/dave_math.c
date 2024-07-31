/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_math.c (%version: 4 %)
 *          created Tue Feb 08 10:17:38 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jul 14 14:56:42 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2007-08-29 ASc  removed tabs, add comments, changed C++ to C comments
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_math.h"

/*-------------------------------------------------------------------------- */

#define D2_INNER_SQRT(s)                        \
   temp = (y << (s)) + (1 << (((s) * 2) - 2));  \
   if(x >= temp)                                \
   {                                            \
      y += 1 << ((s) - 1);                      \
      x -= temp;                                \
   }(void)0                                     /* PRQA S 3412 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/

/*--------------------------------------------------------------------------
 * minimalistic 32->16 bit integer square root (no hw divide/multiply needed)
 * unrolled for speed
 * */
d2_s32 d2_sqrt(d2_u32 x)
{
   d2_u32 temp, y=0;

   if (x >= 0x40000000u)
   {
      y = 0x8000u;
      x -= 0x40000000u;
   }

   D2_INNER_SQRT (15); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT (14); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT (13); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT (12); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT (11); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT (10); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT ( 9); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT ( 8); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT ( 7); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT ( 6); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT ( 5); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT ( 4); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT ( 3); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/
   D2_INNER_SQRT ( 2); /* PRQA S 3112 */ /* $Misra: #MACRO_COMPILER_WARNING_NOP $*/ /* PRQA S 3382 */ /* $Misra: #MISRA_BUG_ZERO_WRAPAROUND $*/

   temp = (y + y) + 1;
   if(x >= temp)
   {
      y++;
   }
   return (d2_s32) y;
}

/*--------------------------------------------------------------------------
 * Find next larger power of two - 1
 * e.g. given number x=73 -> next larger power of two is 128,
 * the result is 128-1 = 127 (number with all bits set)
 * */
d2_u32 d2_pow2mask(d2_u32 x)
{
   x |= (x >> 1);
   x |= (x >> 2);
   x |= (x >> 4);
   x |= (x >> 8);
   x |= (x >> 16);

   return x;
}
