/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_viewport.c (%version: 21 %)
 *          created Wed Jan 12 16:11:17 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Jan 12 14:15:14 2007 %  (%derived_by:  hh04046 %)
 *
 * Changes:
 *  2007-08-31 ASc  removed tabs, changed C++ to C comments, added
 *                   descriptions for d2_getcliprect and d2_getframebuffer
 *  2008-07-17 MRe  added RGBA formats
 *  2009-10-29 CSe  Allow cliprect of width or height 1.
 *  2011-09-05 MRe  removed check of fb format change; check now done at render start
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */


/*--------------------------------------------------------------------------
 *
 * Title: Viewport Functions
 * Framebuffer and view specific functions.
 *
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_viewport.h"
#include "dave_dlist.h"

/*--------------------------------------------------------------------------
 * Group: Clipping
 * */

/*--------------------------------------------------------------------------
 * function: d2_cliprect
 * Specify the clipping rectangle.
 *
 * No pixels outside the clipping rectangle are touched (rendered or read).
 * The clipping borders are specified as integer positions (in contrast to
 * rendering where subpixel positions are supported).
 *
 * The driver checks whether the specified rectangle fits within the framebuffer
 * memory (by checking the dimensions given with <d2_framebuffer>).
 * Cliprects will be automatically adjusted to fit inside the framebuffer region.
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   xmin - integer pixel position of left border (inclusive)
 *   ymin - integer pixel position of top border (inclusive)
 *   xmax - integer pixel position of right border (inclusive)
 *   ymax - integer pixel position of bottom border (inclusive)
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_cliprect( d2_device *handle, d2_border xmin, d2_border ymin, d2_border xmax, d2_border ymax )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );        /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( xmax >= xmin, D2_INVALIDWIDTH );   /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ymax >= ymin, D2_INVALIDHEIGHT );  /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/

   /* restrict cliprect to framebuffer */
   if(xmin < 0)
   {
      xmin = 0;
   }

   if(ymin < 0)
   {
      ymin = 0;
   }

   if(xmax >= (d2_border)D2_DEV(handle)->fbwidth)
   {
      xmax = (d2_border) ( D2_DEV(handle)->fbwidth - 1 );
   }

   if(ymax >= (d2_border)D2_DEV(handle)->fbheight)
   {
      ymax = (d2_border) ( D2_DEV(handle)->fbheight - 1 );
   }

   if(xmax < 0)
   {
      xmax = 0;
   }

   if(ymax < 0)
   {
      ymax = 0;
   }

   if(xmin >= (d2_border)D2_DEV(handle)->fbwidth)
   {
      xmin = (d2_border) ( D2_DEV(handle)->fbwidth - 1 );
   }

   if(ymin >= (d2_border)D2_DEV(handle)->fbheight)
   {
      ymin = (d2_border) ( D2_DEV(handle)->fbheight - 1 );
   }

   /* store boundary */
   D2_DEV(handle)->clipxmin = (d2_point) D2_FIX4( xmin );    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   D2_DEV(handle)->clipymin = (d2_point) D2_FIX4( ymin );    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   D2_DEV(handle)->clipxmax = (d2_point) D2_FIX4( xmax );    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/
   D2_DEV(handle)->clipymax = (d2_point) D2_FIX4( ymax );    /* PRQA S 4131 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_LEFT $*/

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * function: d2_getcliprect
 * Get the clipping rectangle.
 *
 * No pixels outside the clipping rectangle are touched (rendered or read).
 * The clipping borders are specified as integer positions (in contrast to
 * rendering where subpixel positions are supported).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   xmin - pointer to an integer that receives pixel position of left border
 *   ymin - pointer to an integer that receives pixel position of top border
 *   xmax - pointer to an integer that receives pixel position of right border
 *   ymax - pointer to an integer that receives pixel position of bottom border
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_getcliprect( d2_device *handle, d2_border *xmin, d2_border *ymin, d2_border *xmax, d2_border *ymax )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/

   if(0 != xmin)
   {
      *xmin = D2_INT4( D2_DEV(handle)->clipxmin );    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }

   if(0 != ymin)
   {
      *ymin = D2_INT4( D2_DEV(handle)->clipymin );    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }

   if(0 != xmax)
   {
      *xmax = D2_INT4( D2_DEV(handle)->clipxmax );    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }

   if(0 != ymax)
   {
      *ymax = D2_INT4( D2_DEV(handle)->clipymax );    /* PRQA S 0502 */ /* $Misra: #PERF_ARITHMETIC_SHIFT_RIGHT $*/
   }

   D2_RETOK( handle );
}


/*--------------------------------------------------------------------------
 * Group: Framebuffer management
 * */

/*--------------------------------------------------------------------------
 * function: d2_framebuffer
 * Specify the rendering target.
 *
 * Use this function to define the framebuffer's memory area and layout.
 * All subsequent <Rendering Functions> will work on the given buffer.
 *
 * Setting a new framebuffer will automatically adjust the cliprect to
 * the maximum range [0 .. width-1] and [0 .. height-1]
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ptr    - address of the top left pixel (coordinate 0,0)
 *   pitch  - number of pixels (*not bytes*) per scanline
 *   width  - width of framebuffer in pixels (equal or less than pitch)
 *   height - height of framebuffer in pixels
 *   format - pixel encoding type (framebuffer format)
 *
 * framebuffer formats:
 *
 *  d2_mode_alpha8   - monochrome 8bit per pixel
 *  d2_mode_rgb565   - colored 16bit per pixel
 *  d2_mode_argb8888 - colored 32bit per pixel
 *  d2_mode_argb4444 - colored 16bit per pixel
 *  d2_mode_rgba8888 - colored 32bit per pixel
 *  d2_mode_rgba4444 - colored 16bit per pixel
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 *
 * note:
 *   this function has no effect on what memory area is currently visible
 * */
d2_s32 d2_framebuffer( d2_device *handle, void *ptr, d2_s32 pitch, d2_u32 width, d2_u32 height, d2_s32 format )
{
   d2_contextdata *ctx;

   D2_VALIDATE( handle, D2_INVALIDDEVICE );       /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( ptr, D2_NOVIDEOMEM );             /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( width  > 1, D2_VALUETOOSMALL );   /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/
   D2_CHECKERR( height > 1, D2_VALUETOOSMALL );   /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/

   /* use new parameters */
   D2_DEV(handle)->framebuffer = ptr;
   D2_DEV(handle)->fbformat    = format;
   D2_DEV(handle)->pitch       = pitch;
   D2_DEV(handle)->fbwidth     = width;
   D2_DEV(handle)->fbheight    = height;

   /* calc bpp and mask */
   switch (format)
   {
      case d2_mode_alpha8:
         D2_DEV(handle)->fbstylemask = 0;
         D2_DEV(handle)->bpp         = 1;
         break;

      case d2_mode_rgb565:
         D2_DEV(handle)->fbstylemask = D2C_WRITEFORMAT1;
         D2_DEV(handle)->bpp         = 2;
         break;

      case d2_mode_argb8888:
      case d2_mode_rgb888:
         D2_DEV(handle)->fbstylemask = D2C_WRITEFORMAT2;
         D2_DEV(handle)->bpp         = 4;
         break;

      case d2_mode_rgba8888:
         D2_DEV(handle)->fbstylemask = D2C_WRITEFORMAT3 | D2C_WRITEFORMAT2;
         D2_DEV(handle)->bpp         = 4;
         break;

      case d2_mode_argb4444:
      case d2_mode_rgb444:
         D2_DEV(handle)->fbstylemask = D2C_WRITEFORMAT2 | D2C_WRITEFORMAT1;
         D2_DEV(handle)->bpp         = 2;
         break;

      case d2_mode_rgba4444:
         D2_DEV(handle)->fbstylemask = D2C_WRITEFORMAT3 | D2C_WRITEFORMAT2 | D2C_WRITEFORMAT1;
         D2_DEV(handle)->bpp         = 2;
         break;

      default:
         break;
   }

   /* update context modeflags */
   ctx = D2_DEV(handle)->ctxchain;

   while(NULL != ctx)
   {
      ctx->cr2mask = 
         D2_DEV(handle)->fbstylemask | 
         ctx->blendmask              | 
         ctx->tbstylemask            | 
         ctx->alphablendmask         | 
         ctx->rlemask                | 
         ctx->clutmask               | 
         ctx->colkeymask             ;

      ctx = ctx->next;
   }

   /* reset cliprect */
   (void)d2_cliprect( handle, 0, 0, (d2_border) (width-1), (d2_border) (height-1) );

   D2_RETOK(handle);
}


/*--------------------------------------------------------------------------
 * function: d2_getframebuffer
 * Get information about the rendering target
 *
 * Use this function to get information about the framebuffer's
 * memory area and layout (framebuffer pointer, pitch, size and format).
 *
 * parameters:
 *   handle - device pointer (see: <d2_opendevice>)
 *   ptr    - pointer to receive address of the top left pixel (coordinate 0,0)
 *   pitch  - pointer to receive number of pixels (*not bytes*) per scanline
 *   width  - pointer to receive width of framebuffer in pixels (equal or less than pitch)
 *   height - pointer to receive height of framebuffer in pixels
 *   format - pointer to receive pixel encoding type (framebuffer format)
 *
 * framebuffer formats:
 *
 *  d2_mode_alpha8   - monochrome 8bit per pixel
 *  d2_mode_rgb565   - colored 16bit per pixel
 *  d2_mode_argb8888 - colored 32bit per pixel
 *  d2_mode_argb4444 - colored 16bit per pixel
 *
 * returns:
 *   errorcode (D2_OK if successfull) see list of <Errorcodes> for details
 * */
d2_s32 d2_getframebuffer( d2_device *handle, void** ptr, d2_s32 * pitch, d2_u32 * width, d2_u32 * height, d2_s32 * format )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE );   /* PRQA S 4130, 3112, 3453 */ /* $Misra: #DEBUG_MACRO $*/

   if(NULL != ptr)
   {
      *ptr    = D2_DEV(handle)->framebuffer;
   }

   if(0 != pitch)
   {
      *pitch  = D2_DEV(handle)->pitch;
   }

   if(0 != width)
   {
      *width  = D2_DEV(handle)->fbwidth;
   }

   if(0 != height)
   {
      *height = D2_DEV(handle)->fbheight;
   }

   if(0 != format)
   {
      *format = D2_DEV(handle)->fbformat;
   }

   D2_RETOK( handle );
}


/*--------------------------------------------------------------------------
 * */
d2_s32 d2_clipbbox_intern( const d2_devicedata *handle, d2_bbox *box )
{
   /* trivial reject */
   if( (box->xmin > handle->clipxmax) ||
       (box->ymin > handle->clipymax) ||
       (box->xmax < handle->clipxmin) ||
       (box->ymax < handle->clipymin)
       )
   {
      return 0;
   } 
   else
   {
      /* box clipping */
      if(box->xmin < handle->clipxmin)
      {
         box->xmin = handle->clipxmin;
      }

      if(box->ymin < handle->clipymin)
      {
         box->ymin = handle->clipymin;
      }

      if(box->xmax > handle->clipxmax)
      {
         box->xmax = handle->clipxmax;
      }

      if(box->ymax > handle->clipymax)
      {
         box->ymax = handle->clipymax;
      }

      return 1;
   }
}

