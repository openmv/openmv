/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_math.h (%version: 4 %)
 *          created Thu Jan 13 09:41:58 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jul 14 14:56:48 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2007-08-29 ASc  - remove tabs, add ndoc comments, change C++ to C comments
 *  2007-12-06 ASc  - fixed macro name in description of D2_FIX4
 *-------------------------------------------------------------------------- */

/*--------------------------------------------------------------------------
 * Title: Math Functions
 * Package of useful macros and functions e.g. for fixedpoint operations.
 *
 *-------------------------------------------------------------------------- */


#ifndef __1_dave_math_h_H
#define __1_dave_math_h_H
/*--------------------------------------------------------------------------- */



/* Group: Fixedpoint Macros */


/* Function: D2_FIX4(x) 
 * Integer to n:4 fixedpoint conversion */
#define D2_FIX4(x)      ((x) << 4)                          /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
                 
/* Macro: D2_INT4(x) 
 * n:4 to integer conversion  */
#define D2_INT4(x)      ((x) >> 4)                          /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

/* Macro: D2_FLOOR4(x)
 * n:4 fixedpoint unsigned floor (round towards -inf) */
#define D2_FLOOR4(x)    (((d2_u32)(x)) & ~15u)                

/* Macro: D2_CEIL4(x)
 * n:4 fixedpoint unsigned ceil (round towards +inf) */
#define D2_CEIL4(x)     ((((d2_u32)(x)) + 15u) & ~15u)

/* Macro: D2_FRAC4(x)
 * n:4 fixedpoint fractional part only */
#define D2_FRAC4(x)     (((d2_u32)(x)) & 15u)

/* Macro: D2_FIX16(x)
 * integer to n:16 fixedpoint conversion */
#define D2_FIX16(x)     ((x) << 16)                         /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

/* Macro: D2_INT16(x)
n:16 to integer conversion */
#define D2_INT16(x)     ((x) >> 16)                         /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

/* Macro: D2_FLOOR16(x)
 * n:16 fixedpoint unsigned floor (round towards -inf) */
#define D2_FLOOR16(x)   (((d2_u32)(x)) & ~65535u)

/* Macro: D2_CEIL16(x)
 * n:16 fixedpoint unsigned ceil (round towards +inf) */
#define D2_CEIL16(x)    ((((d2_u32)(x)) + 65535u) & ~65535u)

/* Macro: D2_FRAC16(x)
 * n:16 fixedpoint fractional part only */
#define D2_FRAC16(x)    (((d2_u32)(x)) & 65535u)

/* Macro: D2_EPSILON
 * Smallest representable positive number (all fixedpoint formats) */
#define D2_EPSILON      1                          

/*--------------------------------------------------------------------------- */


/* Group: Math Functions */

/*--------------------------------------------------------------------------
 * Function: d2_sqrt
 * Minimalistic 32->16 bit integer square root (no hw divide/multiply needed)
 * unrolled for speed.
 *
 * parameters:
 *   x - fixedpoint number
 *
 * returns:
 *   the square root of x
 *
 * */
D2_EXTERN d2_s32 d2_sqrt(d2_u32 x);


/*--------------------------------------------------------------------------
 * Function: d2_pow2mask
 * Find next larger power of two minus 1
 *
 * E.g. given number x=73 -> next larger power of two is 128,
 * the result is 128 minus 1 = 127 (number with all bits set)
 *
 * parameters:
 *   x - positive number
 *
 * returns:
 *   the next larger number of two minus 1 of x
 *
 * */
D2_EXTERN d2_u32 d2_pow2mask(d2_u32 x);

/*--------------------------------------------------------------------------- */
#endif
