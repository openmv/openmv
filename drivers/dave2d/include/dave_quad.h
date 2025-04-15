/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_quad.h (%version: 2 %)
 *          created Fri Feb 11 14:14:25 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Feb 11 15:35:29 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2010-09-09 MRe: added renderquad functions for solidoutlined etc. 
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_quad_h_H
#define __1_dave_quad_h_H
/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_renderquad_solid( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderquad_solidoutline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderquad_shadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderquad_solidshadow( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderquad_outline( d2_device *handle, d2_point x1, d2_point y1, d2_point x2, d2_point y2, d2_point x3, d2_point y3, d2_point x4, d2_point y4, d2_u32 flags );

/*--------------------------------------------------------------------------- */
#endif
