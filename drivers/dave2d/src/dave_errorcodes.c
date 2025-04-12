/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_errorcodes.c (%version: 2 %)
 *          created Tue Jan 11 13:45:53 2005 by hh04027
 *
 * Description:
 *  %date_modified: Wed Feb 09 13:40:19 2005 %  (%derived_by:  hh04027 %)
 *
 * Changes:
 *  2007-08-31 ASc  removed tabs, changed C++ to C comments,
 *                   changed g_errorcodes to const pointer to const data
 *  2012-09-25 BSp  MISRA cleanup
 *-------------------------------------------------------------------------- */

#include "dave_driver.h"
#include "dave_intern.h"


/*-------------------------------------------------------------------------- */
#define ERR(x,y) y " (" #x ")" ,

/*-------------------------------------------------------------------------- */
const d2_char *d2_translateerror( d2_s32 errorcode )
{

static const d2_char* const g_errorcodes[ D2_ERROR_QUANTITY+1 ] = {
#include "dave_errorcodes.h"                                         /* PRQA S 5087 */ /* $Misra: #INCLUDE_ERRCODES $*/
   "undefined error" };

   /* map undefined indices to last index (undef error) */
   if((d2_u32)errorcode > (d2_u32)D2_ERROR_QUANTITY)
   {
      errorcode = D2_ERROR_QUANTITY;
   }

   /* return errordescription */
   return g_errorcodes[ errorcode ];
}
