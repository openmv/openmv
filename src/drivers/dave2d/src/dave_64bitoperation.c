/*
****************************************************************************
PROJECT : D/AVE
FILE    : $Id: dave_64bitoperation.c 23623 2011-08-19 12:01:17Z nan.wang $
============================================================================ 
DESCRIPTION
D/AVE driver
============================================================================
C O P Y R I G H T                                    
============================================================================
Copyright (c) 2011
by 
Renesas Electronics (Europe) GmbH. 
Arcadiastrasse 10
D-40472 Duesseldorf
Germany
All rights reserved.
============================================================================
Purpose: only for testing

Warranty Disclaimer

Because the Product(s) is licensed free of charge, there is no warranty 
of any kind whatsoever and expressly disclaimed and excluded by Renesas, 
either expressed or implied, including but not limited to those for 
non-infringement of intellectual property, merchantability and/or 
fitness for the particular purpose. 
Renesas shall not have any obligation to maintain, service or provide bug 
fixes for the supplied Product(s) and/or the Application.

Each User is solely responsible for determining the appropriateness of 
using the Product(s) and assumes all risks associated with its exercise 
of rights under this Agreement, including, but not limited to the risks 
and costs of program errors, compliance with applicable laws, damage to 
or loss of data, programs or equipment, and unavailability or 
interruption of operations.

Limitation of Liability

In no event shall Renesas be liable to the User for any incidental, 
consequential, indirect, or punitive damage (including but not limited 
to lost profits) regardless of whether such liability is based on breach 
of contract, tort, strict liability, breach of warranties, failure of 
essential purpose or otherwise and even if advised of the possibility of 
such damages. Renesas shall not be liable for any services or products 
provided by third party vendors, developers or consultants identified or
referred to the User by Renesas in connection with the Product(s) and/or the 
Application.

****************************************************************************
*/
/*--------------------------------------------------------------------------
 *
 * Title: functions for 64bit operations
 * 
 * This module contains all the necessary function for 64bit operation in case
 * the chosen compiler does not support long long. Per default this module is
 * linked. 
 * This module is excluded only when the compiler supports long long
 * 
 *
 *-------------------------------------------------------------------------- */


/***********************************************************
 *
 *                       Includes
 *
 ************************************************************
 */
#include "dave_driver.h"
#include "dave_intern.h"


#ifdef _NO_LL_

#ifdef __CA850__
#elif defined(__ICCV850__)
#include "intrinsics.h"
#else
#include "v800_ghs.h"
D2_EXTERN d2_s32 __MULSH(d2_s32 a, d2_s32 b);
D2_EXTERN d2_s32 __MULUH(d2_u32 a, d2_u32 b);
#endif


/***********************************************************
 *
 *    Function: d2_cast32to64
 *    
 *    Cast an 32bit integer to 64bit variable
 *
 *    Parameters:
 *
 *        par   - 32bit integer to be casted
 *
 *    Return:
 *
 *        d2_int64 - a 64bit value
 */
void d2_cast32to64(d2_s32 par, d2_int64 *res)
{
   res->low32  = (d2_u32)par;
   
   /* Check sign*/
   if(par < 0)
   {
      res->high32 = 0xffffffffu;
   }
   else
   {
      res->high32 = 0;
   }   
}

/***********************************************************
 *
 *    Function: d2_cast64to32
 *    
 *    Cast an 64bit variable to a32bit variable
 *
 *    Parameters:
 *
 *        par   - 64bit variable of type d2_int64
 *
 *    Return:
 *
 *        int - a 32bit value
 */
d2_s32 d2_cast64to32(const d2_int64 *par)
{
   return ((d2_s32)par->low32);
}

/***********************************************************
 *
 *    Function: d2_add64
 *    
 *    Addition of 2 64bit value
 *
 *    Parameters:
 *
 *        a   - 64bit variable of type d2_int64
 *        b   - pointer to a 64bit variable of type d2_int64
 *        res - pointer to a 64bit variable of type d2_int64.
 *              The result will be stored in that variable.
 *
 *    Return:
 *
 *        none
 */
void d2_add64(const d2_int64 *a, const d2_int64 *b, d2_int64 *res)
{
   d2_u32 tmp;
   
   res->low32   = a->low32  + b->low32;
   res->high32  = a->high32 + b->high32;

#ifdef __CA850__
   tmp = (d2_u32) __satadd(a->low32, b->low32);        /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#elif defined(__ICCV850__)
   tmp = __upper_mul64(a->low32, b->low32);   /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#else
   tmp = ADD_SAT(a->low32, b->low32);         /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#endif

   if( (0x7FFFFFFFu == tmp) || (0x80000000u == tmp) )
   {
      res->high32++;
   }
   
}

/***********************************************************
 *
 *    Function: d2_sub64
 *    
 *    Substraction of 2 64bit value
 *
 *    Parameters:
 *
 *        a   - 64bit variable of type d2_int64
 *        b   - pointer to a 64bit variable of type d2_int64
 *        res - pointer to a 64bit variable of type d2_int64.
 *              The result will be stored in that variable.
 *
 *    Return:
 *
 *        none
 */
void d2_sub64(const d2_int64 *a, const d2_int64 *b, d2_int64 *res)
{
   res->low32   = a->low32  - b->low32;
   res->high32  = a->high32 - b->high32;

   if(a->low32 < b->low32)
   {
      res->high32  -= 1;
   }
}

/***********************************************************
 *
 *    Function: d2_mul3232to64
 *    
 *    multiply two 32-bit variables and return a 64bit value
 *
 *    Parameters:
 *
 *        a   - int
 *        b   - int
 *        res - pointer to a variable of type d2_int64
 *              The result will be stored in that variable.
 *
 *    Return:
 *
 *        none
 */

#ifdef __CA850__
#elif defined(__ICCV850__)
#include "intrinsics.h"
#else
D2_EXTERN d2_s32 __MULSH(d2_s32 a, d2_s32 b); 
#endif

void d2_mul3232to64(d2_s32 a, d2_s32 b, d2_int64 *res)
{
   res->low32  = (d2_u32)a * (d2_u32)b;
#ifdef __CA850__
   res->high32 = __mul32(a, b);         /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#elif defined(__ICCV850__)
   res->high32 = __upper_mul64(a, b);   /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#else
   res->high32 = __MULSH(a, b);         /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#endif
}

/***********************************************************
 *
 *    Function: d2_mul3264
 *    
 *    multiply two a 32bit varibales with a 64bit one and 
 *    return a 64bit result
 *
 *    Parameters:
 *
 *        a   - int
 *        b   - pointer to a 64bit variable of type d2_int64
 *        res - pointer to a 64bit variable of type d2_int64.
 *              The result will be stored in that variable.
 *
 *    Return:
 *
 *        None
 */
void d2_mul3264(d2_s32 a, const d2_int64 *b, d2_int64 *res)
{
   d2_u32 negative = 0;
   d2_u32 ui_b_high32;
   d2_u32 ui_b_low32;
   d2_u32 ui_a;
   
   /* get sign */
   if(b ->high32 < 0)
   {
      negative = 1u;
      ui_b_high32 = ((d2_u32)b->high32 ^ 0xFFFFFFFFu);
      ui_b_low32  = (d2_u32)((b->low32 ^ 0xFFFFFFFFu) + 1u); 
   }
   else
   {
      ui_b_high32 = (d2_u32) b->high32;
      ui_b_low32  = (d2_u32) b->low32; 
   }

   if(a < 0)
   {
      negative ^= 1u;
      ui_a = (d2_u32) (-a);
   }
   else
   {
      ui_a = (d2_u32) a;
   }
      
   res->low32  = (ui_a) * (ui_b_low32);
#ifdef __CA850__
   res->high32 = __mul32u(ui_a, ui_b_low32);      /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#elif defined(__ICCV850__)
   res->high32 = __upper_mul64(ui_a, ui_b_low32); /* !!!!! Wrong should be unsigned mul!!!*/  /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#else
   res->high32 = __MULUH(ui_a, ui_b_low32);      /* PRQA S 3335 */ /* $Misra: #COMPILER_INTRINSIC $*/
#endif
   res->high32 += (d2_s32) (ui_a * ui_b_high32);
   
   /* restore sign*/
   if(0 != negative)
   {
      if(0 != res->low32)
      {
         res->high32 = (d2_s32)((d2_u32)res->high32 ^ 0xFFFFFFFFu);
         res->low32  = (res->low32 ^ 0xFFFFFFFFu) + 1u;
      }
      else
      {
         if(0 != res->high32)
         {
            res->high32 = -res->high32;
         }
      }
   }
}

/***********************************************************
 *
 *    Function: d2_div6432
 *    
 *    division of 64bit value by a 32bit. The return result 
 *    is a 64bit value.
 *
 *    Parameters:
 *
 *        dividend   - pointer to a 64bit variable of type d2_int64
 *        divisor    - 32bit variable of type int
 *        res        - pointer to a 64bit variable of type d2_int64
 *                     The result will be stored in this variable
 *  
 *    Return:
 *
 *        None
 */
void d2_div6432(const d2_int64 *dividend, d2_s32 divisor, d2_int64 *res)
{
   d2_u32 remainder;
   d2_u32 shift           = 0u;
   d2_u32 negative        = 0u;
   d2_s32 dividend_high32 = dividend->high32; 
   d2_u32 dividend_low32  = dividend->low32; 
   
   
   res->high32 = 0;
   res->low32  = 0;
   
   /* Test for division by 0*/
   if(0 == divisor)
   {
      return; /* return 0*/
   }
   
   /* Test for sign*/
   if(dividend_high32 < 0)
   {
      dividend_high32 = -dividend_high32 - 1; /*  invert sign */
      dividend_low32  = (dividend_low32 ^ 0xFFFFFFFFu) + 1u; /*  invert sign */
      negative = 1;
   }

   if(divisor < 0)
   {
      divisor   = -divisor; /*  invert sign */
      negative ^= 1u;
   }
      
   /*if divisor > dividend  return 0*/
   if( (0 == dividend_high32) && ((d2_u32)divisor > dividend_low32))
   {
      return; /* return 0*/
   }
   
   res->high32 = dividend_high32 / divisor;
   res->low32  = dividend_low32  / (d2_u32)divisor;
   
   remainder = (d2_u32) (dividend_high32 - (res->high32 * divisor));

   if(0 != remainder)
   {
      d2_int64 tmp64;
      d2_u32 tmp32;
      d2_u32 remMask  = 0x80000000u;
      shift = 0;
      for(;;)
      {
         if(0 != (remainder & remMask))
         {
            tmp32 = remainder << shift;
            tmp32 = tmp32 / (d2_u32)divisor;
            res->low32 += tmp32 << (32u - shift);
            d2_mul3232to64((d2_s32)res->low32, divisor, &tmp64);

            if(0 == tmp64.high32)
            {
               /* result is 32-bit */
               remainder = dividend_low32 - (res->low32 * (d2_u32)divisor);
               res->low32 += remainder / (d2_u32)divisor;
            }
            else
            {
               /* result is 64-bit */
               d2_int64 tmp64_2;
               d2_int64 tmp64_3;
               tmp64_3.low32  = dividend_low32;
               tmp64_3.high32 = dividend_high32;
               d2_sub64(&tmp64_3, &tmp64, &tmp64_2);
               d2_div6432(&tmp64_2, divisor, &tmp64);    /* PRQA S 3670 */ /* $Misra: #DIV64_RECURSION $*/
               d2_add64(&tmp64, res, &tmp64_2);
               res->low32 = tmp64_2.low32;
               res->high32 = tmp64_2.high32;
            }
            break;
         }

         shift++;   
         remMask >>= 1u;
      }
   }
         
   /* restore sign*/
   if(0 != negative)
   {
      if(0 != res->low32)
      {
         res->high32 = -res->high32 - 1;
         res->low32  = (res->low32 ^ 0xFFFFFFFFu) + 1u;
      }
      else
      {
         if(0 != res->high32)
         {
            res->high32 = -res->high32;
         }
      }
   }
}

/***********************************************************
 *
 *    Function: d2_shiftleft64
 *    
 *    Shift left a variable of type d2_int64
 *
 *    Parameters:
 *
 *        var   - pointer to a 64bit variable of type d2_int64
 *        index - integer, number of bit to shift
 *        res   - pointer to a 64bit variable of type d2_int64
 *                The result will be stored in this variable
 *  
 *    Return:
 *
 *        None
 */
void d2_shiftleft64(const d2_int64 *var, d2_s32 index, d2_int64 *res)
{
   res->low32  = var->low32 << index;
   res->high32 = (var->high32 << index) + (d2_s32)(var->low32 >> (32 - index));  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
}

/***********************************************************
 *
 *    Function: d2_shiftright64
 *    
 *    Shift right a variable of type d2_int64
 *
 *    Parameters:
 *
 *        var   - pointer to a 64bit variable of type d2_int64
 *        index - integer, number of bit to shift
 *        res   - pointer to a 64bit variable of type d2_int64
 *                The result will be stored in this variable
 *  
 *    Return:
 *
 *        None
 */
void d2_shiftright64(const d2_int64 *var, d2_s32 index, d2_int64 *res)
{
   res->low32  = (d2_u32)(var->high32 << (32 - index)) + (var->low32 >> index);   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   res->high32 = var->high32 >> index;                                            /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
}

#endif /* end _NO_LL_ */
