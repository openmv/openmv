/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_hardware.h (%version: 6 %)
 *          created Tue Jan 11 14:52:38 2005 by hh04027
 *
 * Description:
 *  %date_modified: Tue Aug 30 16:02:19 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_hardware_h_H
#define __1_dave_hardware_h_H
/*--------------------------------------------------------------------------- */

D2_EXTERN d1_device * d2hw_acquire( d2_device *handle, d2_u32 flags );
D2_EXTERN d2_s32 d2hw_release( d1_device * hwid );

D2_EXTERN void d2hw_set(d1_device * hwid, d2_u32 index, d2_s32 value);
D2_EXTERN d2_s32 d2hw_get(d1_device * hwid, d2_u32 index);

D2_EXTERN void d2hw_wait(d1_device * hwid);

/*--------------------------------------------------------------------------- */

/* return after rendering has finished */
D2_EXTERN void d2hw_finish(const d2_device *handle);

/* start rendering primitives */
D2_EXTERN void d2hw_start(d2_device *handle, const d2_dlist *dlist, d2_s32 startnoblk);

/*--------------------------------------------------------------------------- */
#endif
