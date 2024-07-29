/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_types.h (%version: 69 %)
 *          created Tue Sep 25 17:31:44 2012 by hh04044
 *
 * Description:
 *  %date_modified: Thu Apr 26 16:21:32 2007 %  (%derived_by:  hh74036 %)
 *
 * Definition of common basic types.
 *
 * Changes:
 *  2012-09-25 BSp  MISRA cleanup
 *
*-------------------------------------------------------------------------- */

#ifndef __DAVE_TYPES_H__
#define __DAVE_TYPES_H__


typedef char           d2_char;   /* character */
typedef signed char    d2_s8;     /* signed byte */
typedef unsigned char  d2_u8;     /* unsigned byte */
typedef unsigned short d2_u16;
typedef signed short   d2_s16;
typedef signed int     d2_s32;
typedef unsigned int   d2_u32;
typedef unsigned long  d2_ulong;  /* platform-dependent: usually 32bit, sometimes 48bit. */
typedef signed long    d2_slong;
typedef float          d2_f32;    /* IEE754 single precision float */
typedef double         d2_f64;    /* IEE754 double precision float */


#endif /* __DAVE_TYPES_H__ */
