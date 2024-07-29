/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_gradient.c (%version: 8 %)
 *          created Fri Jan 21 16:45:30 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Jun 13 11:46:15 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_render.h"


/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE d2_u32 d2_initgradient_lin( d2_devicedata *handle, const d2_gradientdata *grad, const d2_bbox *bbox, d2_u32 index, d2_u32 usehiprecision ); /* to satisfy MISRA rule 3450 */

static D2_INLINE d2_u32 d2_initgradient_quad( const d2_devicedata *handle, const d2_gradientdata *grad, const d2_bbox *bbox, d2_u32 index );


/*--------------------------------------------------------------------------
 *
 * */
void d2_initgradient_intern( d2_gradientdata *grad )
{
   grad->mode  = d2_grad_none;
   grad->x1    = 0;
   grad->y1    = 0;
   grad->xadd  = 0;
   grad->yadd  = 0;
   grad->xadd2 = 0;
   grad->yadd2 = 0;
}

/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE d2_u32 d2_initgradient_lin( d2_devicedata *handle, const d2_gradientdata *grad, const d2_bbox *bbox, d2_u32 index, d2_u32 usehiprecision )
{
   d2_s32 x, y, s;
   d2_u32 ctrl;

   x = grad->x1 - bbox->xmin;
   y = grad->y1 - bbox->ymin;
   s = ((x * grad->xadd) + (y * grad->yadd)) >> 4;        /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/

   if(0 != (grad->mode & d2_grad_rightedge))
   {
      s += D2_EPSILON;
   }

   if(0 != (grad->mode & d2_grad_offset))
   {
      s -= D2_FIX16(1);
   }

   ctrl = D2C_LIM1ENABLE << index;

   if(0 != (grad->mode & d2_grad_threshold))
   {
      ctrl |= D2C_LIM1THRESHOLD << index;

      // compensate that threshold moves actual edge to pixel center
      s -= (D2_FIX16(1)>>1) + D2_EPSILON;
   }

   if(0 != (grad->mode & d2_grad_concave))
   {
      ctrl |= D2C_UNION12 << (index >> 1);
   }

   if(0 == usehiprecision)
   {
      D2_DLISTWRITES( D2_L1START + index, -s );
      D2_DLISTWRITES( D2_L1XADD  + index, grad->xadd );
      D2_DLISTWRITES( D2_L1YADD  + index, grad->yadd );
   }
   else
   {
      D2_DLISTWRITES( D2_L1START + index, -s << LIMITER_HIPRECISION );         /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      D2_DLISTWRITES( D2_L1XADD  + index, grad->xadd << LIMITER_HIPRECISION ); /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      D2_DLISTWRITES( D2_L1YADD  + index, grad->yadd << LIMITER_HIPRECISION ); /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   }

   return ctrl;
}

/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE d2_u32 d2_initgradient_quad( const d2_devicedata *handle, const d2_gradientdata *grad, const d2_bbox *bbox, d2_u32 index )
{
   /* dummy */
   (void)handle;  /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/
   (void)grad;    /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/
   (void)bbox;    /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/
   (void)index;   /* PRQA S 3112 */ /* $Misra: #COMPILER_WARNING $*/

   return 0;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_u32 d2_initgradients_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox, d2_u32 limindex, d2_u32 control )
{
   if(0 != (ctx->gradients & 1))
   {
      if(0 != (ctx->gradient[0].mode & d2_grad_linear))
      {
         if(limindex > 5)
         {
            return control; /* assume 6limiter architecture */
         }

         control |= d2_initgradient_lin( handle, &ctx->gradient[0], bbox, limindex, control & D2C_LIMITERPRECISION );
         control &= ~D2C_SPANSTORE;
         if(0 != (ctx->gradient[0].mode & d2_grad_concave))
         {
            control &= ~D2C_SPANABORT;
         }

         limindex += 1;
      }
      else
      {
         if(limindex > 4)
         {
            return control; /* assume 6limiter architecture */
         }

         control |= d2_initgradient_quad( handle, &ctx->gradient[0], bbox, limindex );
         control &= ~D2C_SPANSTORE;

         limindex += 2;
      }
   }

   if(0 != (ctx->gradients & 2))
   {
      if(0 != (ctx->gradient[1].mode & d2_grad_linear))
      {
         if(limindex > 5)
         {
            return control; /* assume 6limiter architecture */
         }

         control |= d2_initgradient_lin( handle, &ctx->gradient[1], bbox, limindex, control & D2C_LIMITERPRECISION );
         control &= ~D2C_SPANSTORE;
         if(0 != (ctx->gradient[1].mode & d2_grad_concave))
         {
            control &= ~D2C_SPANABORT;
         }

         limindex += 1;
      }
      else
      {
         if(limindex > 4)
         {
            return control; /* assume 6limiter architecture */
         }

         control |= d2_initgradient_quad( handle, &ctx->gradient[1], bbox, limindex );
         control &= ~D2C_SPANSTORE;

         limindex += 2;
      }
   }

   if( (0 != (ctx->alphamode & d2_am_gradient1)) && (0 != (ctx->gradients & 4)) )
   {
      if(0 != (ctx->gradient[2].mode & d2_grad_linear))
      {
         if(limindex > 5)
         {
            return control; /* assume 6limiter architecture */
         }

         control |= d2_initgradient_lin( handle, &ctx->gradient[2], bbox, limindex, control & D2C_LIMITERPRECISION );
         control &= ~D2C_SPANSTORE;

         limindex += 1;
      }
      else
      {
         if(limindex > 4)
         {
            return control; /* assume 6limiter architecture */
         }

         control |= d2_initgradient_quad( handle, &ctx->gradient[2], bbox, limindex );
         control &= ~D2C_SPANSTORE;

         limindex += 2;
      }
   }

   if( (0 != (ctx->alphamode & d2_am_gradient2)) && (0 != (ctx->gradients & 8)) )
   {
      if(0 != (ctx->gradient[3].mode & d2_grad_linear))
      {
         if(limindex > 5)
         {
            return control; /* assume 6limiter architecture */
         }

         control |= d2_initgradient_lin( handle, &ctx->gradient[3], bbox, limindex, control & D2C_LIMITERPRECISION );
         control &= ~D2C_SPANSTORE;

         limindex += 1;
      }
      else
      {
         if(limindex > 4)
         {
            return control; /* assume 6limiter architecture */
         }

         control |= d2_initgradient_quad( handle, &ctx->gradient[3], bbox, limindex );
         control &= ~D2C_SPANSTORE;

         limindex += 2;
      }
   }

   return control;
}
