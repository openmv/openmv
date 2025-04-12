/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_pattern.c (%version: 9 %)
 *          created Tue Feb 15 12:41:15 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jul 13 18:22:53 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_render.h"
#include "dave_pattern.h"



/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_setupulimiter_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox ); /* MISRA */

static D2_INLINE void d2_setupulimiter_invert_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox ); /* MISRA */



/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_setupulimiter_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox )
{
   d2_s32 x, y, s;

   if(0 != (ctx->patulim[0].mode & d2_grad_aapattern))
   {
      /* special pattern setup for autoaligned mapping using relative point, offset and direction */
      x = ctx->patulim[0].x1;
      y = ctx->patulim[0].y1;
      s = (((x * ctx->patulim[0].xadd) + (y * ctx->patulim[0].yadd)) >> 4) + ctx->patulim[0].xadd2; /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }
   else
   {
      /* normal pattern setup using point and direction */
      x = ctx->patulim[0].x1 - bbox->xmin;
      y = ctx->patulim[0].y1 - bbox->ymin;
      s = ((x * ctx->patulim[0].xadd) + (y * ctx->patulim[0].yadd)) >> 4;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }

   D2_DLISTWRITES( D2_LUSTART, -s );
   D2_DLISTWRITES( D2_LUXADD , ctx->patulim[0].xadd );
   D2_DLISTWRITES( D2_LUYADD , ctx->patulim[0].yadd );

   return;
}

/*--------------------------------------------------------------------------
 *
 * */
static D2_INLINE void d2_setupulimiter_invert_intern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox )
{
   d2_s32 x, y, s;

   if(0 != (ctx->patulim[0].mode & d2_grad_aapattern))
   {
      /* special pattern setup for autoaligned mapping using relative point, offset and direction */
      x = ctx->patulim[0].x1;
      y = (bbox->ymax - bbox->ymin) - ctx->patulim[0].y1;
      s = (((x * ctx->patulim[0].xadd) - (y * ctx->patulim[0].yadd)) >> 4) + ctx->patulim[0].xadd2;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }
   else
   {
      /* normal pattern setup using point and direction */
      x = ctx->patulim[0].x1 - bbox->xmin;
      y = ctx->patulim[0].y1 - bbox->ymax;
      s = ((x * ctx->patulim[0].xadd) + (y * ctx->patulim[0].yadd)) >> 4;  /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }

   D2_DLISTWRITES( D2_LUSTART, -s );
   D2_DLISTWRITES( D2_LUXADD , ctx->patulim[0].xadd );
   D2_DLISTWRITES( D2_LUYADD , -ctx->patulim[0].yadd );

   return;
}

/*--------------------------------------------------------------------------
 *
 * */
void d2_setuppattern( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox, d2_s32 flip )
{
   if(d2_fm_pattern == ctx->fillmode)
   {
      if(0 != flip) 
      {
         d2_setupulimiter_invert_intern( handle, ctx, bbox );
      }
      else
      {
         d2_setupulimiter_intern( handle, ctx, bbox );
      }
   }
}
