/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_triarray.h (%version: 1 %)
 *          created Tue May 23 12:19:25 2006 by hh04027
 *
 * Description:
 *  %date_modified: Tue May 23 12:19:27 2006 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_triarray_h_H
#define __1_dave_triarray_h_H
/*--------------------------------------------------------------------------- */

/* (note) implemented in dave_render.c */
D2_EXTERN void d2_rendertrifan_intern( d2_device *handle, d2_fp_triangle rtri, const d2_point *vert, const d2_u32 *flags, d2_u32 count);

/*--------------------------------------------------------------------------- */
#endif
