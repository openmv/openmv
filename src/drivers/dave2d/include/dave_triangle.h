/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_triangle.h (%version: 4 %)
 *          created Mon Jan 24 16:23:01 2005 by hh04027
 *
 * Description:
 *  %date_modified: Wed Feb 09 13:41:12 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_triangle_h_H
#define __1_dave_triangle_h_H
/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_rendertri_solid( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags );

D2_EXTERN d2_s32 d2_rendertri_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags );

D2_EXTERN d2_s32 d2_rendertri_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags );

D2_EXTERN d2_s32 d2_rendertri_outline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags );

D2_EXTERN d2_s32 d2_rendertri_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_u32 flags );

/*--------------------------------------------------------------------------- */
#endif
