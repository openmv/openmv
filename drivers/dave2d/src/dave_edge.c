/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_edge.c (%version: 14 %)
 *          created Mon Jan 31 18:40:01 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jan 04 13:51:46 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2010-09-28 MRe  use 64bit arithmetic for blurring calculation
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_edge.h"


/*--------------------------------------------------------------------------
 * line length may not be zero.
 * */
void d2_lineedge_setup3blur_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_u32 flags, const d2_contextdata *ctx, d2_s32 *length )
{
   d2_s32 xa, ya, l;
   d2_s32 s1, s2, s3;
   d2_s32 oa, ob;

   /* get extension offsets */
   if(0 != (flags & d2_le_exclude_start))
   {
      oa = 0;
   }
   else
   {
      oa = (d2_s32) (ctx->invblur << 0);
   }

   if(0 != (flags & d2_le_exclude_end))
   {
      ob = 0;
   }
   else
   {
      ob = (d2_s32) (ctx->invblur << 0);
   }

   l = ((d2_s32)(ctx->invblur * 16u)) / d2_sqrt( (d2_u32) ((dx * dx) + (dy * dy)) );

   if(NULL != length)
   {
      *length = l;
   }

   xa = (-dy * l) >> 4;                              /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   ya = ( dx * l) >> 4;                              /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   s1 = (((-px * xa) - (py * ya) ) >> 4);            /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   s2 = (((-px * ya) + (py * xa) ) >> 4);            /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   s3 = (((px + dx) * ya) - ((py + dy) * xa)) >> 4;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   /* store limiter parameters */
   lim[0].start = s1 + (d2_s32)(ctx->invblur / 2u);
   lim[0].xadd  = xa;
   lim[0].yadd  = ya;
   lim[1].start = s2 + oa;
   lim[1].xadd  = ya;
   lim[1].yadd  = -xa;
   lim[2].start = s3 + ob;
   lim[2].xadd  = -ya;
   lim[2].yadd  = xa;
}


/*--------------------------------------------------------------------------
 * line length may not be zero.
 * */
void d2_lineedge_setup3sqrt_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_u32 flags, d2_s32 *length )
{
   d2_s32 xa, ya, l;
   d2_s32 s1, s2, s3;
   d2_s32 oa, ob;

   /* get extension offsets */
   if(0 != (flags & d2_le_exclude_start))
   {
      oa = 0;
   }
   else
   {
      oa = D2_FIX16(1);
   }

   if(0 != (flags & d2_le_exclude_end))
   {
      ob = 0;
   }
   else
   {
      ob = D2_FIX16(1);
   }

   l = (65536 * 16) / d2_sqrt( (d2_u32)( (dx * dx) + (dy * dy) ) );

   if(NULL != length)
   {
      *length = l;
   }

   xa = (-dy * l) >> 4;                              /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   ya = ( dx * l) >> 4;                              /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   s1 = ((-px * xa) - (py * ya) ) >> 4;              /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   s2 = ((-px * ya) + (py * xa) ) >> 4;              /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   s3 = (((px + dx) * ya) - ((py + dy) * xa)) >> 4;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   /* store limiter parameters */
   lim[0].start = s1 + (D2_FIX16(1) / 2);
   lim[0].xadd  = xa;
   lim[0].yadd  = ya;
   lim[1].start = s2 + oa;
   lim[1].xadd  = ya;
   lim[1].yadd  = -xa;
   lim[2].start = s3 + ob;
   lim[2].xadd  = -ya;
   lim[2].yadd  = xa;
}

/*--------------------------------------------------------------------------
 * note use for thin lines (w<2) only (due to l1 norm approximation)
 * line length may not be zero.
 * */
void d2_lineedge_setup3_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_u32 flags, d2_s32 *length )
{
   d2_s32 n, xa, ya;
   d2_s32 s1, s2, s3;
   d2_s32 oa, ob;

   /* get extension offsets */
   if(0 != (flags & d2_le_exclude_start))
   {
      oa = 0;
   }
   else
   {
      oa = D2_FIX16(1);
   }

   if(0 != (flags & d2_le_exclude_end))
   {
      ob = 0;
   }
   else
   {
      ob = D2_FIX16(1);
   }

   /* find line octant */
   if(dx < 0)
   {
      if(dy < 0)
      {
         /* dx -
          * dy - */
         if(dx < dy)
         {
            /* 3. octant
             * l = -dx, nx = -dy / -dx, ny = -1 */

            n = (dy << 16) / dx;                                       /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s1 = (py << (16 - 4)) - ((px * n) >> 4);                   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s2 = (px << (16 - 4)) + ((py * n) >> 4);                   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/  
            s3 = -((px + dx) << (16 - 4)) - (((py + dy) * n) >> 4);    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            xa = n;
            ya = D2_FIX16( -1 );                                       /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

            if(NULL != length)
            {
               *length = -dx;
            }

         }
         else
         {
            /* 4. octant
             * l = -dy, nx = 1, ny = dx / -dy */

            n = (dx << 16) / -dy;                                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s1 = (-px << (16 - 4)) - ((py * n) >> 4);                  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s2 = ( py << (16 - 4)) - ((px * n) >> 4);                  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s3 = (((px + dx) * n) >> 4) - ((py + dy) << (16 - 4));     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            xa = D2_FIX16( 1 );                                        /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            ya = n;

            if(NULL != length)
            {
               *length = -dy;
            }
         }
      }
      else
      {
         /* dx -
          * dy + */
         if(-dx > dy)
         {
            /* 5. octant
             * l = dx, nx = -dy / -dx, ny = -1 */

            n = (dy << 16) / dx;                                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s1 = (py << (16 - 4)) - ((px * n) >> 4);                 /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s2 = (px << (16 - 4)) + ((py * n) >> 4);                 /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s3 = -((px + dx) << (16 - 4)) - (((py + dy) * n) >> 4);  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            xa = n;
            ya = D2_FIX16( -1 );                                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

            if(NULL != length)
            {
               *length = dx;
            }

         }
         else
         {
            /* 6. octant
             * l = dy, nx = -1, ny = dx / dy */

            n = (dx << 16) / dy;                                    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s1 = ( px << (16 - 4)) - ((py * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s2 = (-py << (16 - 4)) - ((px * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s3 = (((px + dx) * n) >> 4) + ((py + dy) << (16 - 4));  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            xa = D2_FIX16( -1 );                                    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            ya = n;

            if(NULL != length)
            {
               *length = dy;
            }
         }
      }
   }
   else
   {
      if(dy < 0)
      {
         /* dx +
          * dy - */
         if(dx > -dy)
         {
            /* 1. octant
             * l = dx, nx = -dy / dx, ny = 1 */

            n = (dy << 16) / -dx;                                   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s1 = (-py << (16 - 4)) - ((px * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s2 = (-px << (16 - 4)) + ((py * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s3 = ((px + dx) << (16 - 4)) - (((py + dy) * n) >> 4);  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            xa = n;
            ya = D2_FIX16( 1 );                                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

            if(NULL != length)
            {
               *length = dx;
            }

         }
         else
         {
            /* 2. octant
             * l = -dy, nx = 1, ny =  dx / -dy */

            n = (dx << 16) / -dy;                                   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s1 = (-px << (16 - 4)) - ((py * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            s2 = ( py << (16 - 4)) - ((px * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            s3 = (((px + dx) * n) >> 4) - ((py + dy) << (16 - 4));  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
            xa = D2_FIX16( 1 );                                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            ya = n;
            
            if(NULL != length)
            {
               *length = -dy;
            }
         }
      }
      else
      {
         /* dx +
          * dy + */
         if(dx > dy)
         {
            /* 8. octant
             * l = dx, nx = -dy / dx, ny = 1 */

            n = (dy << 16) / -dx;                                   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s1 = (-py << (16 - 4)) - ((px * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            s2 = (-px << (16 - 4)) + ((py * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            s3 = ((px + dx) << (16 - 4)) - (((py + dy) * n) >> 4);  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = n;
            ya = D2_FIX16( 1 );                                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

            if(NULL != length)
            {
               *length = dx;
            }

         }
         else
         {
            /* 7. octant
             * l = dy, nx = -1, ny =  dx / dy */

            n = (dx << 16) / dy;                                    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s1 = ( px << (16 - 4)) - ((py * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            s2 = (-py << (16 - 4)) - ((px * n) >> 4);               /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            s3 = (((px + dx) * n) >> 4) + ((py + dy) << (16 - 4));  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = D2_FIX16( -1 );                                    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            ya = n;

            if(NULL != length)
            {
               *length = dy;
            }
         }
      }
   }

   /* store limiter parameters */
   lim[0].start = s1 + (D2_FIX16(1) / 2);                           /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   lim[0].xadd  = xa;
   lim[0].yadd  = ya;
   lim[1].start = s2 + oa;
   lim[1].xadd  = ya;
   lim[1].yadd  = -xa;
   lim[2].start = s3 + ob;
   lim[2].xadd  = -ya;
   lim[2].yadd  = xa;
}


/*--------------------------------------------------------------------------
 * edge deltas may not both be zero. centering of start value (+0.5) is
 * omitted here. this was necessary to allow highler level to change center
 * for non shared edges.
 * */
void d2_triedge_setup_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_u32 rightedge )
{
   d2_s32 n;
   d2_s32 s,xa,ya;

   /* find edge octant */
   if(dx < 0)
   {
      if(dy < 0)
      {
         /* dx -
          * dy - */
         px = (d2_point)(px + (d2_point) dx);
         py = (d2_point)(py + (d2_point) dy);

         if(dx > dy)
         {
            /* 3. octant */
            n = (dx << 16) / dy;                       /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = ((py * n) >> 4) - (px << (16 - 4));    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = D2_FIX16( 1 );                        /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            ya = -n;
         }
         else
         {
            /* 4. octant */
            n = (dy << 16) / dx;                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = (py << (16 - 4)) - ((px * n) >> 4);   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = n;
            ya = D2_FIX16( -1 );                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         }
      }
      else
      {
         /* dx -
          * dy + */
         if(-dx > dy)
         {
            /* 5. octant */
            n = (dy << 16) / dx;                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = (py << (16 - 4)) - ((px * n) >> 4);   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = n;
            ya = D2_FIX16( -1 );                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         }
         else
         {
            /* 6. octant */
            n = (dx << 16) / dy;                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = (px << (16 - 4)) - ((py * n) >> 4);   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = D2_FIX16( -1 );                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            ya = n;
         }
      }
   }
   else
   {
      if(dy < 0)
      {
         /* dx +
          * dy - */
         px = (d2_point)(px + (d2_point) dx);
         py = (d2_point)(py + (d2_point) dy);

         if(dx > -dy)
         {
            /* 1. octant */
            n = (dy << 16) / dx;                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = ((px * n) >> 4) - (py << (16 - 4));  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = -n;
            ya = D2_FIX16( 1 );                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         }
         else
         {
            /* 2. octant */
            n = (dx << 16) / dy;                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = ((py * n) >> 4) - (px << (16 - 4));  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = D2_FIX16( 1 );                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            ya = -n;
         }
      }
      else
      {
         /* dx +
          * dy + */
         if(dx > dy)
         {
            /* 8. octant */
            n = (dy << 16) / dx;                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = ((px * n) >> 4) - (py << (16 - 4));  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = -n;
            ya = D2_FIX16( 1 );                      /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         }
         else
         {
            /* 7. octant */ 
            n = (dx << 16) / dy;                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = (px << (16 - 4)) - ((py * n) >> 4);  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = D2_FIX16( -1 );                     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            ya = n;
         }
      }
   }

   if(0 != rightedge)
   {
      s += D2_EPSILON;
   }

   lim->start = s;
   lim->xadd  = xa;
   lim->yadd  = ya;
}

/*--------------------------------------------------------------------------
 * note should use sqrtsetup when bluring long distance
 * edge deltas may not both be zero.
 * */
void d2_triedge_setupblur_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, const d2_contextdata *ctx )
{
   d2_int64 n;
   d2_int64 tmp;
   d2_s32 s,xa,ya;
   d2_s32 invBlur;

   /* reduce bitsize */
   invBlur = (d2_s32)ctx->invblur;

   /* find edge octant */
   if ( dx < 0 )
   {
      if ( dy < 0 )
      {
         /* dx -
          * dy - */
         px = (d2_point)(px + (d2_point) dx);
         py = (d2_point)(py + (d2_point) dy);

         if(dx > dy)
         {
            /* 3. octant */
            D2_CAST32TO64((dx << 16) / dy, &n); /* n = (dx << 16) / dy; */     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = ((py * D2_CAST64TO32(&n)) >> 4) - (px << (16 - 4));            /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = invBlur;
            D2_MUL3264(invBlur, &n, &tmp);
            D2_SHIFTRIGHT64(&tmp, 16, &n);
            ya = -D2_CAST64TO32(&n);
         }
         else
         {
            /* 4. octant */
            D2_CAST32TO64((dy << 16) / dx, &n); /* n = (dy << 16) / dx; */     /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = (py << (16 - 4)) - ((px * D2_CAST64TO32(&n)) >> 4);            /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            D2_MUL3264(invBlur, &n, &tmp);
            D2_SHIFTRIGHT64(&tmp, 16, &n);
            xa = D2_CAST64TO32(&n);
            ya = -invBlur;
         }
      }
      else
      {
         /* dx -
          * dy + */
         if(-dx > dy)
         {
            /* 5. octant */
            D2_CAST32TO64((dy << 16) / dx, &n); /* n = (dy << 16) / dx; */    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = (py << (16 - 4)) - ((px * D2_CAST64TO32(&n)) >> 4);           /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            D2_MUL3264(invBlur, &n, &tmp);
            D2_SHIFTRIGHT64(&tmp, 16, &n);
            xa = D2_CAST64TO32(&n); /* (n * redinv) >> 8; */
            ya = -invBlur;
         }
         else
         {
            /* 6. octant */
            D2_CAST32TO64((dx << 16) / dy, &n); /* n = (dx << 16) / dy; */   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = (px << (16 - 4)) - ((py * D2_CAST64TO32(&n)) >> 4);          /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = -invBlur;
            D2_MUL3264(invBlur, &n, &tmp);
            D2_SHIFTRIGHT64(&tmp, 16, &n);
            ya = D2_CAST64TO32(&n); /* (n * redinv) >> 8; */
         }
      }
   }
   else
   {
      if(dy < 0)
      {
         /* dx +
          * dy - */
         px = (d2_point)(px + (d2_point) dx);
         py = (d2_point)(py + (d2_point) dy);

         if(dx > -dy)
         {
            /* 1. octant */
            D2_CAST32TO64((dy << 16) / dx, &n); /* n = (dy << 16) / dx; */  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = ((px * D2_CAST64TO32(&n)) >> 4) - (py << (16 - 4));         /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            D2_MUL3264(invBlur, &n, &tmp);
            D2_SHIFTRIGHT64(&tmp, 16, &n);
            xa = -D2_CAST64TO32(&n); /* (-n * redinv) >> 8; */
            ya = invBlur;
         }
         else
         {
            /* 2. octant */
            D2_CAST32TO64((dx << 16) / dy, &n); /* n = (dx << 16) / dy; */  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = ((py * D2_CAST64TO32(&n)) >> 4) - (px << (16 - 4));         /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = invBlur;
            D2_MUL3264(invBlur, &n, &tmp);
            D2_SHIFTRIGHT64(&tmp, 16, &n);
            ya = -D2_CAST64TO32(&n);
         }
      }
      else
      {
         /* dx +
          * dy + */
         if(dx > dy)
         {
            /* 8. octant */
            D2_CAST32TO64((dy << 16) / dx, &n); /* n = (dy << 16) / dx; */  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = ((px * D2_CAST64TO32(&n)) >> 4) - (py << (16 - 4));         /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            D2_MUL3264(invBlur, &n, &tmp);
            D2_SHIFTRIGHT64(&tmp, 16, &n);
            xa = -D2_CAST64TO32(&n);
            ya = invBlur;
         }
         else
         {
            /* 7. octant */
            D2_CAST32TO64((dx << 16) / dy, &n); /* n = (dx << 16) / dy; */  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
            s = (px << (16 - 4)) - ((py * D2_CAST64TO32(&n)) >> 4);         /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/ /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
            xa = -invBlur;
            D2_MUL3264(invBlur, &n, &tmp);
            D2_SHIFTRIGHT64(&tmp, 16, &n);
            ya = D2_CAST64TO32(&n); /* (n * redinv) >> 8; */
         }
      }
   }

   s = (s << 4) / ctx->blurring;                                            /* PRQA S 4131       */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

   lim->start = s + (invBlur / 2);
   lim->xadd  = xa;
   lim->yadd  = ya;
}

/*--------------------------------------------------------------------------
 * warning: will overflow earlier than aa version. might blow 11bit border
 * edge deltas may not both be zero.
 * */
void d2_triedge_setupnoaa_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_s32 rightedge )
{
   d2_s32 s,xa,ya;

   /* flip if negative (necessary for gapless rendering of adjacent edges) */
   if ( dy < 0 )
   {
      px = (d2_point)(px + (d2_point) dx);
      py = (d2_point)(py + (d2_point) dy);
   }

   /* non normalized limiter base: not suited for aa rendering but fast */
   s = ((px * dy) - (py * dx)) << (16 - 8);    /* PRQA S 4131       */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   xa = (-dy) << (16 - 4);                     /* PRQA S 4131       */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   ya =   dx  << (16 - 4);                     /* PRQA S 4131       */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

   /* use > instead of >= threshold on right/top edges */
   if(0 != rightedge)
   {
      s += D2_EPSILON;
   }

   lim->start = s + (D2_FIX16(1)/2);
   lim->xadd  = xa;
   lim->yadd  = ya;
}

/*--------------------------------------------------------------------------
 *
 * */
void d2_triedge_setupsqrt_intern( d2_limdata *lim, d2_point px, d2_point py, d2_s32 dx, d2_s32 dy, d2_s32 rightedge )
{
   d2_s32 s, xa, ya, l;

   /* flip if negative (necessary for gapless rendering of adjacent edges) */
   if(dy < 0)
   {
      px = (d2_point)(px + (d2_point) dx);
      py = (d2_point)(py + (d2_point) dy);
   }

   l = (d2_s32)( (65536u * 16u) / (d2_u32)d2_sqrt( (d2_u32)( (dx * dx) + (dy * dy) ) ) );

   xa = (-dy * l) >> 4;                 /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
   ya = ( dx * l) >> 4;                 /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 
   s = ((-px * xa) - (py * ya) ) >> 4;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/ 

   /* use > instead of >= threshold on right/top edges */
   if(0 != rightedge)
   {
      s += D2_EPSILON;
   }

   lim->start = s + (D2_FIX16(1)/2);
   lim->xadd  = xa;
   lim->yadd  = ya;
}

/*--------------------------------------------------------------------------
 * used to setup bottom-up rendering
 * */
void d2_invertlimiter_intern(d2_limdata *lim, d2_s32 ystep)
{
   /* prestep and flip */
   lim->start =  lim->start + (lim->yadd * ystep);
   lim->yadd  = -lim->yadd;
}
