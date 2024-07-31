/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_utility.c (%version: 8 %)
 *          created Tue Jul 19 12:53:28 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Nov 16 10:56:45 2006 %  (%derived_by:  hh74040 %)
 *
 * Changes:
 *  2007-12-07 ASc  add error checking to d2_utility_perspectivewarp() 
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs 
 *  2012-09-25 BSp  MISRA cleanup
 *  2016-06-29 MHo  bugfix in fbblitcopy: handle SRC below DST in case of overlapping rects correctly in presence of an asymmetric clip rect
 *  2017-07-27 HFu  bugfix in fbblitcopy: corrected wait-insert to full wait instead of pipeline only
 */

/*--------------------------------------------------------------------------
 *
 * Title: Utility Functions
 * Triangle mapping and perspective warp operations
 *
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_texture.h"
#include "dave_utility.h"


/*--------------------------------------------------------------------------
 * function: d2_utility_maptriangle
 * This function uses d2_settexturemapping to map a texture on a triangle.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   points - ptr to array of six floats containing the screen positions (x,y pairs)
 *   uvs - ptr to array of six floats containing the texture coordinates (u,v pairs)
 *
 * Please remember that this function has to access the current texture details
 * (width, height and pitch) in order to calculate a correct mapping. So the
 * final texture must be active in the selected context at the time
 * d2_utility_maptriangle is called.
 * d2_utility_maptriangle is internally using d2_settexturemapping
 * and is overwriting the current values.
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * see also:
 *  <d2_settexturemapping> */
d2_s32 d2_utility_maptriangle( d2_device *handle, const d2_f32 *points, const d2_f32 *uvs )
{
   d2_contextdata *ctx;
   d2_s32 dxu, dxv, dyu, dyv;
   d2_f32 d, dx, dy;
   d2_f32 tw, th;

   ctx = D2_DEV(handle)->ctxselected;
   tw = ((d2_f32)ctx->texwidth) * 65536.0f;
   th = ((d2_f32)ctx->texheight) * 65536.0f;

   d = 
      (
         (points[0 * 2] - points[2 * 2]) 
         * 
         (points[(1 * 2) + 1] - points[(2 * 2) + 1])
       )
      -
      (
         (points[1 * 2] - points[2*2])
         *
         (points[(0 * 2) + 1] - points[(2 * 2) + 1])
       )
      ;
   /* validate. if d==0 use dxyuv=0 [no area triangle] */
   dx = tw / d;
   dy = th / d;

   dxu = (d2_s32)
      (
         (
            (
               (
                  uvs[0 * 2]
                  -
                  uvs[2 * 2]
                )
               *
               (
                  points[(1 * 2) + 1]
                  -
                  points[(2 * 2) + 1]
                )
             )
            -
            (
               (
                  uvs[1 * 2]
                  -
                  uvs[2 * 2]
                )
               *
               (
                  points[(0 * 2) + 1]
                  -
                  points[(2 * 2) + 1]
                )
             )
          )
         * dx
       );

   dxv = (d2_s32)
      (
         (
            (
               (
                  uvs[(0 * 2) + 1]
                  -
                  uvs[(2 * 2) + 1]
                )
               *
               (
                  points[(1 * 2) + 1]
                  -
                  points[(2 * 2) + 1]
                )
             )
            - 
            (
               (
                  uvs[(1 * 2) + 1]
                  -
                  uvs[(2 * 2) + 1]
                )
               *
               (
                  points[(0 * 2) + 1]
                  - 
                  points[(2 * 2) + 1]
                )
             )
          )
         * dy
       );
   
   dyu = (d2_s32)
      (
         (
            (
               (
                  uvs[1 * 2]
                  -
                  uvs[2 * 2]
                )
               *
               (
                  points[0 * 2]
                  -
                  points[2 * 2]
                )
             )
            -
            (
               (
                  uvs[0 * 2]
                  -
                  uvs[2 * 2]
                )
               *
               (
                  points[1 * 2]
                  -
                  points[2 * 2]
                )
             )
          )
         * dx
       );

   dyv = (d2_s32)
      (
         (
            (
               (
                  uvs[(1 * 2) + 1]
                  -
                  uvs[(2 * 2) + 1]
                )
               *
               (
                  points[0 * 2]
                  -
                  points[2 * 2]
                )
             )
            -
            (
               (
                  uvs[(0 * 2) + 1]
                  -
                  uvs[(2 * 2) + 1]
                )
               *
               (
                  points[1 * 2]
                  -
                  points[2 * 2]
                )
             )
          )
         * dy
       );
       
   
   return d2_settexturemapping( handle, (d2_point) (points[0] * 16), (d2_point) (points[1] * 16), (d2_s32)(uvs[0] * tw), (d2_s32)(uvs[1] * th), dxu, dyu, dxv, dyv );
}


/*--------------------------------------------------------------------------
 * function: d2_utility_perspectivewarp
 * Copies an image in perspective manner to a destination rectangle.
 *
 * d2_utility_perspectivewarp is internally using d2_settexturemapping
 * and is overwriting the current values.
 *
 * (see pwe.gif)
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   srcwidth - width of source rectangle in pixels
 *   srcheight - heigth of source rectangle in pixels
 *   srcx - x position in source bitmap
 *   srcy - y position in source bitmap
 *   dstwidth - width of destination rectangle in pixels
 *   dstheight - height of destination rectangle in pixels
 *   dstx - x position in destination bitmap
 *   dsty - y position in destination bitmap
 *   wt - 1/z (z>1) value in 16 Bit fraction (z=2 -> wt=65536/2)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * example:
 * Copy a 256x256 source texture into a 640x480 destination rectangle:
 *
 * (start code)
 * ...
 * d2_settexture( handle, texture, 256, 256, 256, d2_mode_rgb565 );
 * d2_settexturemode( handle, 0 );
 * d2_setfillmode( handle, d2_fm_texture );
 * d2_utility_perspectivewarp(  handle, 256, 256, 0, 0, 640, 480, 0, 0, 65536/4 );
 * ...
 * (end code)
 *
 * limitations:
 * This function is not meant for dynamic perspective animations.
 *
 * see also:
 *  <d2_settexturemapping>
 * */
d2_s32 d2_utility_perspectivewarp( d2_device *handle, d2_u16 srcwidth, d2_u16 srcheight, d2_s16 srcx, d2_s16 srcy, d2_s16 dstwidth, d2_s16 dstheight, d2_s16 dstx, d2_s16 dsty, d2_u16 wt )
{
   d2_u32 y;
   d2_u32 pu  = 0;
   d2_u32 pv  = 0;
   d2_u32 aw  = (d2_u32) ( (65536 - wt) / dstheight );
   d2_u32 ayv = (d2_u32) ( (1 << 16) / dstheight );
   d2_u32 axu = wt;
   d2_u32 ayu = (d2_u32) ( (32767 - (wt >> 1)) / dstheight );

   for(y=0; y<(d2_u32)dstheight; y++ )
   {
      d2_s32 result;
      d2_u32 tty = ((pv << 16) / wt) * srcheight;
      d2_u32 tlx = ((pu << 16) / wt) * srcwidth;
      d2_u32 dxu = (d2_u32) ( (((axu << 16) / wt) * srcwidth) / (d2_u16)dstwidth );

      result = d2_settexturemapping(
         handle,
         (d2_point)(dstx << 4),                 /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         (d2_point)((dsty + (d2_s16)y) << 4),   /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         (d2_s32)tlx + ((d2_s32)srcx << 16),    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         (d2_s32)tty + ((d2_s32)srcy << 16),    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
         (d2_s32)dxu,
         0, 0, 0
                                    );
      if(D2_OK != result)
      {
         return result;
      }

      result = d2_renderbox( handle, (d2_point)(dstx << 4), (d2_point)((dsty + (d2_s16)y) << 4), (d2_width)(dstwidth << 4), 1 << 4 );  /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
      if(D2_OK != result)
      {
         return result;
      }

      pu += ayu;
      pv += ayv;
      wt = (d2_u16)(wt + (d2_u16)aw);
   }

   return D2_OK;
}


/*--------------------------------------------------------------------------
 * function: d2_utility_fbblitcopy
 * Copy a rectangular part inside the current framebuffer.
 *
 * This is a wrapper function around <d2_blitcopy>, which copies a rectangular region inside a single buffer.
 * Source and destination buffer are the current framebuffer, set via <d2_framebuffer>.
 * This function handles the different cases of potentially overlapping source and destination areas and makes sure
 * no read-after-write problems arise.
 * Note that both source and destination rectangles need to lie fully inside the framebuffer for this function to work
 * correctly.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   width, height - size of source/destination rectangle in pixels (integer)
 *   srcx, srcy  - top/left coordinate of source rectangle (integer)
 *   dstx, dsty - top/left coordinate of destination rectangle (integer)
 *   flags - any combination of blit flag bits (see below), passed on to <d2_blitcopy>
 *
 * flags parameter bits (only one bit allowed):
 *   d2_bf_no_blitctxbackup - for this blit don't backup context data for better performance; previous texture modes get lost and must be set again
 *
 * returns:
 *   errorcode (D2_OK if successful) see list of <Errorcodes> for details
 * */
d2_s32 d2_utility_fbblitcopy( d2_device *handle, d2_u16 width, d2_u16 height, d2_blitpos srcx, d2_blitpos srcy, d2_blitpos dstx, d2_blitpos dsty, d2_u32 flags)
{
   d2_s32 result = D2_OK;
   void *fb_ptr, *fb_ptr_blit;
   d2_s32 fb_pitch, fb_pitch_blit;
   d2_u16 fb_width, fb_height;
   d2_u8 fb_format, fb_bpp;
   d2_blitpos srcy_blit;
   d2_u8 flipped = 0;
   d2_s16 clip_ymin = 0, clip_ymax = 0;

   D2_VALIDATEP( handle, D2_INVALIDDEVICE );                                                     /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( width>0, D2_INVALIDWIDTH );                                                      /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( height>0, D2_INVALIDHEIGHT );                                                    /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( width<=1024, D2_INVALIDWIDTH );                                                  /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( height<=1024, D2_INVALIDHEIGHT );                                                /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* get current framebuffer parameters and set as blit source */
   fb_ptr    = D2_DEV(handle)->framebuffer;
   fb_format = (d2_u8)(D2_DEV(handle)->fbformat);
   fb_bpp    = (d2_u8)(D2_DEV(handle)->bpp);
   fb_pitch  = D2_DEV(handle)->pitch;
   fb_width  = (d2_u16)(D2_DEV(handle)->fbwidth);
   fb_height = (d2_u16)(D2_DEV(handle)->fbheight);

   /* blit pitch has to be positive. flip vertical axis for blit source/texture if required */
   if (fb_pitch < 0)
   {
      d2_s32 fb_ptr_offset = (d2_s32)(fb_height-1)*fb_pitch*(d2_s32)fb_bpp;
      fb_ptr_blit   = (void*)( ((d2_char*)fb_ptr) + fb_ptr_offset );
      fb_pitch_blit = -fb_pitch;
      srcy_blit     = (d2_blitpos)(d2_s32)(fb_height-height-srcy);
      flags        ^= d2_bf_mirrorv;
   }
   else
   {
      fb_ptr_blit   = fb_ptr;
      fb_pitch_blit = fb_pitch;
      srcy_blit     = srcy;
   }

   result = d2_setblitsrc(handle, fb_ptr_blit, fb_pitch_blit, (d2_s32)fb_width, (d2_s32)fb_height, (d2_u32)fb_format);
   if (D2_OK != result)
   {
      return result;
   }

   /* since we now read the framebuffer as texture, we need to make sure everything has been flushed first */
   (void)d2_insertwaitfulldlist_intern(handle);

   /* worst case: horizontal offset, destination region right of source region, overlap */
   if ((dsty == srcy) && (dstx > srcx) && (dstx < (srcx+width)))
   {
      /* calculate stripe size */
      d2_blitpos offset = (d2_blitpos)(dstx-srcx);
      d2_u16 strwidth   = offset;
      d2_blitpos strx   = (d2_blitpos)(srcx+width);  /* start on the right side */
      
      do
      {
         /* advance to next stripe: last columns need a slimmer stripe */
         if (((d2_s32)strx-(d2_s32)strwidth) < (d2_s32)srcx)
         {
            strwidth = (d2_u16)(strx-srcx);
         }
         strx = (d2_blitpos)(strx-strwidth);
         result = d2_blitcopy(handle, (d2_s32)strwidth,            (d2_s32)height,            strx,                           srcy_blit,
                                      (d2_width)D2_FIX4(strwidth), (d2_width)D2_FIX4(height), (d2_point)D2_FIX4(strx+offset), (d2_point)D2_FIX4(srcy), flags);
         if (D2_OK != result)
         {
            return result;
         }
      } while (strx > srcx);
      return result;
   }

   /* vertical offset with destination region AFTER source region and overlap: vertical flip in source and destination */
   if ( (dsty > srcy)
        && (dsty < (srcy+height))
        && ((dstx+width) > srcx) && (dstx < (srcx+width)) )
   {
      /* go to start of last line */
      d2_s32 fb_ptr_offset = (d2_s32)(fb_height-1)*fb_pitch*(d2_s32)fb_bpp;
      D2_DEV(handle)->framebuffer = (void*)( ((d2_u8*)fb_ptr) + fb_ptr_offset );
      D2_DEV(handle)->pitch       = -fb_pitch;
      dsty = (d2_blitpos)(fb_height-height-dsty);
      clip_ymax = D2_DEV(handle)->clipymax; 
      clip_ymin = D2_DEV(handle)->clipymin;
      D2_DEV(handle)->clipymax = (d2_point) ((d2_s16)(D2_FIX4(fb_height-1)) - clip_ymin);  // MHO: once the framebuffer is mirrored temporarily, any coordinates 
      D2_DEV(handle)->clipymin = (d2_point) ((d2_s16)(D2_FIX4(fb_height-1)) - clip_ymax);  // MHO: ..of significance in that time, need to be mirrored internally, too
      flags ^= d2_bf_mirrorv;
      flipped = 1;
   }

   result = d2_blitcopy(handle, (d2_s32)width,            (d2_s32)height,            srcx,                    srcy_blit,
                                (d2_width)D2_FIX4(width), (d2_width)D2_FIX4(height), (d2_point)D2_FIX4(dstx), (d2_point)D2_FIX4(dsty), flags);

   if (1 == flipped)
   {
      /* restore framebuffer settings */
      D2_DEV(handle)->framebuffer = fb_ptr;
      D2_DEV(handle)->pitch       = fb_pitch;
      D2_DEV(handle)->clipymax = clip_ymax;   // MHO: undo the mirroring of the clip rect
      D2_DEV(handle)->clipymin = clip_ymin;  
   }

   return result;
}


/*--------------------------------------------------------------------------
 * function: d2_rendercircle_no_hilimiterprecision
 * Enable or disable the high limiter precision feature of the latest 
 * D/AVE 2D renderer for drawing blurred circles with higher image quality. 
 * Note: This function can be used to de-activate the feature for 
 * compatibility reasons (clear D2FB_HILIMITERPRECISION bit in HW revision)!
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   flag - 0 will activate the high limiter precision if possible,
 *          1 will deactivate the feature even on newer version of D/AVE 2D.
 *
 * */
void d2_rendercircle_no_hilimiterprecision( d2_device *handle, d2_u32 flag )
{
   if(0 == flag)
   {
      /* set the hilimiterprecision bit if possible to support this feature from now on */
      if(0 != D2_DEV(handle)->hilimiterprecision_supported)
      {
         D2_DEV(handle)->hwrevision |= D2FB_HILIMITERPRECISION;
      }
   }
   else
   {
      /* clear the hilimiterprecision bit to not support this feature from now on */
      D2_DEV(handle)->hwrevision &= ~D2FB_HILIMITERPRECISION;
   }
}
