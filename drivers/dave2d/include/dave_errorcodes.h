/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_errorcodes.h (%version: 9 %)
 *          created Tue Jan 11 13:41:35 2005 by hh04027
 *
 * Description:
 *  %date_modified: Mon Feb 19 15:59:15 2007 %  (%derived_by:  hh74036 %)
 *
 * Changes:
 * 2007-08-31 ASc  - removed tabs, changed C++ to C comments,
 *-------------------------------------------------------------------------- */

/* Title: Errorcodes
 * List of all dave driver errorcodes.
 *
 * Every device stores the errorcode returned by the last function executed
 * for this device. Successfull operations reset this code to D2_OK.
 *
 * Latest errorcode can be queried by <d2_geterror> / <d2_geterrorstring> functions
 *
 * Errorcodes:
 *
 *   D2_OK - success
 *   D2_NOMEMORY - memory allocation failed
 *   D2_INVALIDDEVICE - invalid device
 *   D2_INVALIDCONTEXT - invalid rendering context
 *   D2_INVALIDBUFFER - invalid renderbuffer context
 *   D2_HWINUSE   - hardware device already in use
 *   D2_DEVASSIGNED - device already assigned
 *   D2_DEFCONTEXT - cannot operate on default context
 *   D2_INVALIDINDEX - index is out of bounds
 *   D2_ILLEGALMODE  - rendermode not supported
 *   D2_INVALIDWIDTH - width out of legal range
 *   D2_INVALIDHEIGHT - height out of legal range
 *   D2_NOVIDEOMEM - illegal framebuffer address
 *   D2_VALUETOOSMALL - parameter too close to zero
 *   D2_VALUENEGATIVE - parameter is negative
 *   D2_VALUETOOBIG - parameter value is too large
 *   D2_INVALIDENUM  - unsupported mode
 *   D2_NULLPOINTER  - source pointer may not be null
 *   D2_DEVICEBUSY - operation cannot execute while hardware is busy
 *   D2_DEFBUFFER -  cannot operate on default buffer
 *   D2_NO_DISPLAYLIST  - usage of displaylists a mandatory in d2_low_localmemmode
 *   D2_NOT_ENOUGH_DLISTBLOCKS - amount of displaylist blocks as specified in <d2_lowlocalmemmode> is not sufficient
 * */

/*--------------------------------------------------------------------------- */

   ERR(  D2_OK                      , "success" )
   ERR(  D2_NOMEMORY                , "memory allocation failed" )
   ERR(  D2_INVALIDDEVICE           , "invalid device" )
   ERR(  D2_INVALIDCONTEXT          , "invalid rendering context" )
   ERR(  D2_INVALIDBUFFER           , "invalid renderbuffer context" )
   ERR(  D2_HWINUSE                 , "hardware device already in use" )
   ERR(  D2_DEVASSIGNED             , "device already assigned" )
   ERR(  D2_DEFCONTEXT              , "cannot operate on default context" )
   ERR(  D2_INVALIDINDEX            , "index is out of bounds" )
   ERR(  D2_ILLEGALMODE             , "rendermode not supported" )
   ERR(  D2_INVALIDWIDTH            , "width out of legal range" )
   ERR(  D2_INVALIDHEIGHT           , "height out of legal range" )
   ERR(  D2_NOVIDEOMEM              , "illegal framebuffer address" )
   ERR(  D2_VALUETOOSMALL           , "parameter too close to zero" )
   ERR(  D2_VALUENEGATIVE           , "parameter is negative" )
   ERR(  D2_VALUETOOBIG             , "parameter value is too large" )
   ERR(  D2_INVALIDENUM             , "unsupported mode" )
   ERR(  D2_NULLPOINTER             , "source pointer may not be null" )
   ERR(  D2_DEVICEBUSY              , "operation cannot execute while hardware is busy" )
   ERR(  D2_DEFBUFFER               , "cannot operate on default buffer" )
   ERR(  D2_NO_DISPLAYLIST          , "d2_df_no_dlist is not supported in low_localmemmode")
   ERR(  D2_NOT_ENOUGH_DLISTBLOCKS  , "not enough dlistblocks. please adjust in d2_lowlocalmemmode(...)")

/*--------------------------------------------------------------------------- */
