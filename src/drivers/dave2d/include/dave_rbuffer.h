/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_rbuffer.h (%version: 4 %)
 *          created Tue May 10 09:42:33 2005 by hh04027
 *
 * Description:
 *  %date_modified: Tue Jan 02 14:33:37 2007 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2008-01-14 ASc  changed comments from C++ to C, removed tabs
 *  2009-03-06 LBe  added closed flag for renderbuffer struct d2_rbuffer
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_rbuffer_h_H
#define __1_dave_rbuffer_h_H
/*--------------------------------------------------------------------------- */

#include "dave_dlist.h"

/*--------------------------------------------------------------------------- */

typedef struct _d2_rb_layer
{
   /* layer data */
   d2_dlist_scratch_entry *scratch;
   d2_u32                  fullsize;
   d2_u32                  freesize;
   /* backup data used during target buffer switch */
   d2_dlist_scratch_entry *backup_base;
   d2_dlist_scratch_entry *backup_pos;
   d2_s32                  backup_cnt;
   d2_fp_scratchfull       backup_hook;
} d2_rb_layer;

typedef struct _d2_rbuffer
{
   d2_dlist    baselist;
   d2_rb_layer layer[1];
   d2_s32      closed; 
} d2_rbuffer;

/*--------------------------------------------------------------------------- */

D2_EXTERN d2_s32 d2_initrblayer_intern( const d2_device *handle, d2_rb_layer *layer, d2_u32 size );

D2_EXTERN void d2_rendertolayer_intern( d2_device *handle );
D2_EXTERN void d2_rendertobase_intern( d2_device *handle );
D2_EXTERN void d2_layer2dlist_intern( d2_device *handle );

/*--------------------------------------------------------------------------- */
#endif
