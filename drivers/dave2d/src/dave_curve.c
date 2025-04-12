/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_curve.c (%version: 12 %)
 *          created Wed Oct 10 12:30:33 2012 by hh04044
 *
 * Description:
 *  %date_modified: Wed Oct 10 12:30:33 2012 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2008-10-02 MRe  fix of blurred circles
 *  2008-11-24 AJ   modify code to support IAR compiler. (no 64bit support)
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_curve.h"


#define TWEAK 3


void d2_circlesetup_intern(d2_devicedata *handle, const d2_contextdata *ctx, d2_u32 index, d2_point x, d2_point y, d2_width r, d2_s32 band, d2_s32 invert, d2_s32 hiprec )
{
   d2_s32 a, b, f, ir;
   d2_u32 ib;
   d2_point xr, yr;
   d2_width blur;

   /* reduce bitsize (x&y are approx r if unclipped) */
   xr = (d2_point)(x - r);
   yr = (d2_point)(y - r);
   
   if(0 != (ctx->features & d2_feat_blur))
   {
      if( (0 == hiprec) || (0 == (D2_DEV(handle)->hwrevision & D2FB_HILIMITERPRECISION)) )
      {
         d2_int64 f64, ir64;
#ifdef _NO_LL_
         d2_int64 tmp, tmp2;
#endif
        
         /* get parameters for blurred circle rendering */
         blur = ctx->blurring; 
         ib = ctx->invblur;    

         /* calc circle parameters (16:16 from 10:4) */
         ir =
            (
               (
                  (-1 * D2_FIX16(16))
                  * D2_FIX4(1)
                ) 
               *
               (D2_FIX4(1) / 2)
             ) 
            / (r * blur);               /* 1 / 2rb */     
#ifndef _NO_LL_
         ir64 = 
            (
               (
                  (
                     (
                        (-1 * (d2_int64)D2_FIX16(16<<0))
                        * D2_FIX4(1)
                      )
                     *
                     (D2_FIX4(1) / 2)
                   )
                  / (d2_int64)r
                )
               * (1 << 8)
             )
            / (d2_int64)blur;               /* 1 / 2rb */  
#else
         d2_cast32to64(
            (
               (
                  (-1 * D2_FIX16(16<<0))
                  * D2_FIX4(1)
                ) 
               * 
               (D2_FIX4(1) / 2)
             ) 
            / r, &ir64
                       );
         d2_shiftleft64(&ir64, 8, &tmp);
         d2_div6432(&tmp, blur, &ir64);               /* 1 / 2rb */  
#endif
       
         f = 
            (
               (
                  (
                     (
                        (xr * xr) + (yr * yr)
                      )
                     / (1 << 8)
                   ) 
                  * ir
                )
               / (1 << 4)
             ) 
            -
            (
               (
                  (
                     (r / 2)
                     +
                     (xr + yr)
                   ) 
                  * (d2_s32)ib
                ) 
               / (1 << 4)
             );      /* (x²+y²-r²) * ir */ 
        
#ifndef _NO_LL_
         f64 = 
            (
               (
                  (
                     ((xr * xr) + (yr * yr))
                     / (1 << 8)
                   )
                  * (d2_int64)ir64
                )
               / (1 << 4)
             )
            - 
            (
               (
                  ((r / 2) + (xr + yr))
                  * (d2_int64)ib
                )
               * (1 << 4)
             );      /* (x²+y²-r²) * ir */ 
         f = (d2_s32)(f64 / (1 << 8));
#else
         d2_mul3264(
            (
               (
                  (xr * xr)
                  +
                  (yr * yr)
                )
               / (1 << 8)
             ),
            &ir64,
            &tmp
                    );
         d2_shiftright64(&tmp, 4, &f64);
         d2_mul3232to64(((r / 2) + (xr + yr)), (d2_s32)ib, &tmp);
         d2_shiftleft64(&tmp, 4, &tmp2);
         d2_sub64(&f64, &tmp2, &tmp);      /* (x²+y²-r²) * ir */ 
         d2_shiftright64(&tmp, 8, &f64);
         f = d2_cast64to32(&f64);
#endif
         ir = (ir / (1 << 4));
        
#ifndef _NO_LL_
         ir = (d2_s32)(ir64 / (1 << (4 + 8)));
#else
         d2_shiftright64(&ir64, (4 + 8), &tmp);
         ir = d2_cast64to32(&tmp);
#endif
         a = ((16 - (x * 2)) * ir) / (1 << 4);           /* (1 - 2x) * ir */  
         b = ((16 - (y * 2)) * ir) / (1 << 4);           /* (1 - 2y) * ir */ 
        
      }
      else
      {
         d2_int64 f64;
#ifdef _NO_LL_
         d2_int64 tmp, tmp2;
#endif
         /* get parameters for blurred circle rendering */
         blur = ctx->blurring; 
         ib = ctx->invblur;  

         /* calc circle parameters (16:16 from 10:4) */
         ir = 
            (
               (
                  (
                     (
                        (-1 * D2_FIX16(16<<0))
                        * D2_FIX4(1)
                      )
                     *
                     (D2_FIX4(1) / 2)
                   )
                  / r
                )
               * (1 << LIMITER_HIPRECISION)
             )
            / blur;               /* 1 / 2rb */   
#ifndef _NO_LL_
         f64 =
            (
               (
                  (
                     (
                        (xr*xr) + (yr*yr)
                      )
                     / (1 << 8)
                   )
                  * (d2_int64)ir
                )
               / (1 << 4)
             ) 
            - 
            (
               (
                  (
                     (r / 2) + (xr + yr)
                   )
                  * (d2_int64)ib
                )
               * (1 << (LIMITER_HIPRECISION - 4))
             );      /* (x²+y²-r²) * ir */
#else
         d2_mul3232to64(((xr * xr) + (yr * yr)) / (1 << 8), ir, &tmp);
         d2_shiftright64(&tmp, 4, &f64);
         d2_mul3232to64(((r / 2) + (xr + yr)), (d2_s32)ib, &tmp);
         d2_shiftleft64(&tmp, (LIMITER_HIPRECISION - 4), &tmp2); 
         d2_sub64(&f64, &tmp2, &tmp);      /* (x²+y²-r²) * ir */
#endif 
         /* ir = ir / (1 << 4); */    /* testcase failure */
         ir >>= 4;                    /* works */       /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         a = ((16 - (x * 2)) * (d2_s32)ir) / (1 << (4 + 0));          /* (1 - 2x) * ir */  
         b = ((16 - (y * 2)) * (d2_s32)ir) / (1 << (4 + 0));          /* (1 - 2y) * ir */ 
#ifndef _NO_LL_
         f = (d2_s32)f64;   
#else
         f = d2_cast64to32(&tmp);
#endif
      }

   }
   else
   {
      /* not blurred case (b == 1 && ib == 1) */
      if(0 == (xr | yr)) /* PRQA S 4130 */ /* $Misra: #PERF_BITWISE $*/
      {
         /* integer aligned, nonclipped case (xr == 0 && yr == 0) */
         ir =
            (
               (
                  (-1 * D2_FIX16(1))
                  *
                  (D2_FIX4(1) / 2)
                )
             ) 
            / r;
         f = (-r) * (1 << (16 - 4 - 1));
         a = b = ((16 - (r * 2)) * ir) / 16;
      }
      else
      {
         d2_int64 ft;
         /*d2_int64 ftXXX;*/
         d2_s32 xq,yq;
#ifdef _NO_LL_
         d2_int64 tmp;
#endif
         /* subpixel positioned or clipped */
         ir =
            (
               (-1 * D2_FIX16(1 << TWEAK)) 
               *
               (D2_FIX4(1) / 2)
             )
            / r;
         xq = xr * xr;
         yq = yr * yr;

#ifndef _NO_LL_
         /* (note) using the '/' variant of the following line will actually call a 64bit division subroutine
          *         the result of it sometimes differs by one bit compared to the '>>' variant, making
          *         "test_renderpolyline2" fail.
          *         however, this codepath is only taking in the windows softdave emulation environment
          */
         ft = ((xq + yq) * (d2_int64)ir) >> (8+TWEAK);              /* works */
         /*ft = ((xq + yq) * (d2_int64)ir) / (1 << (8 + TWEAK));*/  /* test_renderpolyline2 fails */
         /* ftXXX = ((xq + yq) * (d2_int64)ir) / (1 << (8 + TWEAK));*/   /* test_renderpolyline2 fails */

         /*if(ft != ftXXX)
         {
            int i = 42;
         }*/

         f = (d2_s32)ft - (((r / 2) + (xr + yr)) * (1 << (16 - 4)));
#else
         d2_mul3232to64((xq + yq), ir, &tmp);
         d2_shiftright64(&tmp, (8 + TWEAK), &ft);
         f  = d2_cast64to32(&ft) - (((r / 2) + (xr + yr)) * (1 << (16 - 4)));
#endif
         /* ir /= (1 << TWEAK); */   /* testcase failure */
         ir >>= TWEAK;                           /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
         a = ((16 - (x * 2)) * ir) / (1 << 4);
         b = ((16 - (y * 2)) * ir) / (1 << 4);
      }
   }


   /* set register values (geometric parameters) */
   if(0 == invert)
   {
      D2_DLISTWRITES( D2_L1START + index, f + band );
      D2_DLISTWRITES( D2_L2START + index, a ); /* equal l1_xadd */
      D2_DLISTWRITES( D2_L1YADD  + index, b );
      D2_DLISTWRITES( D2_L2XADD  + index, ir * 2 );
      D2_DLISTWRITES( D2_L2YADD  + index, ir * 2 );
   }
   else
   {
      D2_DLISTWRITES( D2_L1START + index, -f - band );
      D2_DLISTWRITES( D2_L2START + index, -a ); /* equal l1_xadd */
      D2_DLISTWRITES( D2_L1YADD  + index, -b );
      D2_DLISTWRITES( D2_L2XADD  + index, -ir * 2 );
      D2_DLISTWRITES( D2_L2YADD  + index, -ir * 2 );
   }
}
