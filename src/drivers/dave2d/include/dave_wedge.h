/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_wedge.h (%version: 2 %)
 *          created Mon Feb 14 14:47:04 2005 by hh04027
 *
 * Description:
 *  %date_modified: Tue Feb 15 17:46:43 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_wedge_h_H
#define __1_dave_wedge_h_H
/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_renderwedge_solid( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderwedge_outline( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderwedge_shadow( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderwedge_solidoutline( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags );
D2_EXTERN d2_s32 d2_renderwedge_solidshadow( d2_device *handle, d2_point x, d2_point y, d2_width r, d2_width w, d2_s32 nx1, d2_s32 ny1, d2_s32 nx2, d2_s32 ny2, d2_u32 flags );

/*--------------------------------------------------------------------------- */
#endif
