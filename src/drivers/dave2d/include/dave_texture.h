/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_texture.h (%version: 2 %)
 *          created Wed Jul 06 14:57:30 2005 by hh04027
 *
 * Description:
 *  %date_modified: Fri Jul 29 13:23:57 2005 %  (%derived_by:  hh04019 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_texture_h_H
#define __1_dave_texture_h_H
/*--------------------------------------------------------------------------- */

#define MAX_CLUT16_ENTRIES 16
#define MAX_CLUT256_ENTRIES 256

D2_EXTERN void d2_setuptextureblend_intern( const d2_devicedata *handle, d2_contextdata *ctx );

D2_EXTERN void d2_calctexturealpha_intern( d2_contextdata *ctx );

D2_EXTERN void d2_setuptexture( d2_devicedata *handle, const d2_contextdata *ctx, const d2_bbox *bbox, d2_s32 flip );

D2_EXTERN void d2_calctexturemask_intern( d2_contextdata *ctx );

/*D2_EXTERN d2_s32 d2_settexclut( d2_device *handle, d2_color* clut );*/ /* defined in dave_driver.h */

D2_EXTERN d2_s32 d2_settexclutentry( d2_device *handle, d2_s32 index, d2_color color ) ;

D2_EXTERN d2_s32 d2_refreshclut( d2_device *handle );
/*--------------------------------------------------------------------------- */
#endif
