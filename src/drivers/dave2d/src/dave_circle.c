/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_circle.c (%version: 15 %)
 *          created Wed Jan 12 15:53:43 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jan 04 16:46:49 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2008-09-25 MRe  added  ctx->circleextendoffset
 *  2008-10-02 MRe  fix of blurred circles
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_viewport.h"
#include "dave_render.h"
#include "dave_pattern.h"
#include "dave_circle.h"
#include "dave_curve.h"
#include "dave_texture.h"


/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendercircle_intern( d2_devicedata *handle, d2_contextdata *ctx, d2_point x, d2_point y, d2_width r, d2_width w )
{
   d2_bbox bbox;
   d2_u32 delay;
   d2_u32 control;
   d2_u32 tmask;
   d2_s32 bbox_enlarge=0;

   /* check for circle ring */
   if (w > 0)
   {
      /* ring bounding box */
      d2_width wh = w/2;
      d2_width wr = (d2_width)(wh + r);
      bbox.xmin = (d2_point) D2_FLOOR4( x - wr );
      bbox.ymin = (d2_point) D2_FLOOR4( y - wr );
      bbox.xmax = (d2_point) D2_CEIL4( x + wr );
      bbox.ymax = (d2_point) D2_CEIL4( y + wr );

      if( (r - wh) <= (D2_FIX4(1)/2) )
      {
         /* invalid ring, use full circle */
         r = (d2_width)(r + wh);
         w = 0;
      }
   }
   else
   {
      /* solid circle bounding box */
      bbox.xmin = (d2_point) D2_FLOOR4( x - r );
      bbox.ymin = (d2_point) D2_FLOOR4( y - r );
      bbox.xmax = (d2_point) D2_CEIL4( x + r );
      bbox.ymax = (d2_point) D2_CEIL4( y + r );
   }

   if(ctx->circleextendoffset > 0)
   { 
      /* enlarge bbox to avoid cropping due to insufficient accuracy for big circles */
      bbox_enlarge = ctx->circleextendoffset;
      /* bbox_enlarge = (bbox_enlarge > ((256 * 128) - 1)) ? ((256 * 128) - 1) : bbox_enlarge; */ /* (note) dead code since bbox_enlarge is a 16bit signed integer */
                      
      if( ((d2_s32)(bbox.xmin) - bbox_enlarge) < (d2_s32)(handle->clipxmin))
      {
         bbox.xmin = handle->clipxmin;
      }
      else
      {
         bbox.xmin = (d2_point)(bbox.xmin - (d2_point)bbox_enlarge);
      }
      
      if( ((d2_s32)(bbox.ymin) - bbox_enlarge) < (d2_s32)(handle->clipymin) )
      {
         bbox.ymin = handle->clipymin;
      }
      else
      {
         bbox.ymin = (d2_point)(bbox.ymin - (d2_point)bbox_enlarge);
      }
      
      if( ((d2_s32)(bbox.xmax)+bbox_enlarge) > (d2_s32)(handle->clipxmax) )
      {
         bbox.xmax = handle->clipxmax; 
      }
      else
      {
         bbox.xmax = (d2_point)(bbox.xmax + (d2_point)bbox_enlarge);
      }

      if( ((d2_s32)(bbox.ymax) + bbox_enlarge) > (d2_s32)(handle->clipymax) )
      {
         bbox.ymax = handle->clipymax;
      }
      else
      {
         bbox.ymax = (d2_point)(bbox.ymax + (d2_point)bbox_enlarge);
      }
   }

   /* clipping */
   if(0 == d2_clipbbox_intern( handle, &bbox ))
   {
      return 0;
   }

   /* make relative */ 
   x = (d2_point)(x - bbox.xmin);
   y = (d2_point)(y - bbox.ymin);

   /* set register values (material parameters) */
   d2_setupmaterial_intern( handle, ctx );
   tmask = ctx->thresholdmask;

   if(0 == w)
   {
      /* solid circle */

      /* set register values (geometric parameters) */
      d2_circlesetup_intern(handle, ctx, 0, x, y, r, 0, 0, 1);

      control = D2C_LIM1ENABLE  | D2C_QUAD1ENABLE | D2C_SPANABORT | D2C_SPANSTORE | (tmask & D2C_LIM1THRESHOLD);

      if(0 != (D2_DEV(handle)->hwrevision & D2FB_HILIMITERPRECISION))
      {
         control |= (0 != (ctx->features & d2_feat_blur)) ? D2C_LIMITERPRECISION : 0;
      }

      control = d2_initgradients_intern( handle, ctx, &bbox, 2, control );

      D2_DLISTWRITES( D2_CONTROL, (d2_s32)control );

      delay = D2_CEIL4( r+bbox_enlarge ) >> 4;
   }
   else
   {
      /* circle ring */

      /* determine whether we can use bandfilter (2lim) ring without distortion
       * this is the case if r/w is larger than sqrt(r) */
      if( (r > (w * w)) && (0 == (ctx->features & d2_feat_blur)) )
      {
         /* 2lim ring */
         r = (d2_width)(r+D2_EPSILON);
         /* set register values (geometric parameters) */
         d2_circlesetup_intern(handle, ctx, 0, x, y, r, (d2_s32) (((d2_u32)((w + D2_FIX4(1)) / 2)) << (16-4)), 0, 1 );

         control = D2C_LIM1ENABLE | D2C_QUAD1ENABLE | D2C_BAND1ENABLE | (tmask & D2C_LIM1THRESHOLD);
         control = d2_initgradients_intern( handle, ctx, &bbox, 2, control );

         D2_DLISTWRITES( D2_L1BAND, w * 4096 ); /* w << (16-4) */
         D2_DLISTWRITES( D2_CONTROL, (d2_s32)control );

         delay = 0;
      }
      else
      {
         /* 4lim ring (more accurate)
          * set register values (geometric parameters) */
         d2_circlesetup_intern(handle, ctx, 0, x, y, (d2_width)(r + (w / 2)), 0, 0, 1 );

         /* inner circle */
         d2_circlesetup_intern(handle, ctx, 2, x, y, (d2_width)(r - (w / 2)), (-D2_FIX16(1)) / 2, 1, 1 );

         control = D2C_LIM1ENABLE | D2C_QUAD1ENABLE | D2C_LIM3ENABLE | D2C_QUAD2ENABLE | (tmask & (D2C_LIM1THRESHOLD|D2C_LIM3THRESHOLD));

         if(0 != (D2_DEV(handle)->hwrevision & D2FB_HILIMITERPRECISION) )
         {
            control |= (0 != (ctx->features & d2_feat_blur)) ? D2C_LIMITERPRECISION : 0;
         }

         control = d2_initgradients_intern( handle, ctx, &bbox, 4, control );

         D2_DLISTWRITES( D2_CONTROL, (d2_s32)control );

         delay = 0;
      }
   }

   /* start rendering */
   d2_setuppattern( handle, ctx, &bbox, 0 );
   d2_setuptexture( handle, ctx, &bbox, 0 );
   d2_startrender_intern( handle, &bbox, delay );

   return 1;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendercircle_solid( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w )
{
   (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x, y, r, w );
   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendercircle_shadow( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w )
{
   (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline,
                                 (d2_point)(D2_DEV(handle)->soffx + x),
                                 (d2_point)(D2_DEV(handle)->soffy + y),
                                 r, w );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendercircle_outline( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w )
{
   d2_width wo = D2_DEV(handle)->outlinewidth;

   /* skip invisible cases */
   if(wo < D2_EPSILON)
   {
      return D2_OK;
   }

   if(w > 0)
   {
      /* circle ring: we also need an outline at the inside */
      (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline,
                                    x, y, (d2_width)((r - (w / 2)) - (wo / 2)), wo
                                    );
   }

   /* outline on the outside */
   (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline,
                                 x, y, (d2_width)((r + (w / 2)) + (wo / 2)), wo
                                 );

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendercircle_solidoutline( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w )
{
   d2_width wo = D2_DEV(handle)->outlinewidth;

   /* render pre outline */
   if( (w > 0) && (wo > 0) )
   {
      /* circle rings only */
      (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x, y, r, (d2_width)((wo * 2) + w) );
   }
   if( (0 == w) && (wo > 0) )
   {
      /* solid circles only */
      (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline, x, y, (d2_width)(r + ((wo - 16) / 2)), (d2_width)(wo + (16 / 2)) );
   }
   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x, y, r, w );
   d2_rendertobase_intern( handle );

   /* render post outline */

   return D2_OK;
}

/*--------------------------------------------------------------------------
 *
 * */
d2_s32 d2_rendercircle_solidshadow( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w )
{
   /* render shadow */
   (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxoutline,
                                 (d2_point)(D2_DEV(handle)->soffx + x),
                                 (d2_point)(D2_DEV(handle)->soffy + y),
                                 r, w
                                 );

   /* render solid [base part] */
   d2_rendertolayer_intern( handle );
   (void)d2_rendercircle_intern( D2_DEV(handle), D2_DEV(handle)->ctxsolid, x, y, r, w );
   d2_rendertobase_intern( handle );

   return D2_OK;
}
