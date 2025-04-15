//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_base.h (%version: 33 %)
//          created Fri Aug 19 11:00:59 2005 by hh04027
//
// Description:
//  %date_modified: Thu Oct 18 12:36:18 2007 %  (%derived_by:  hh74040 %)
//
// Changes:
//  2005-12-22 CSe  - added enums for timeouts and copyflags
//                  - changed declaration of d1_waitforirq to d1_queryirq
//  2006-01-04 CSe  - added new memarchitecture flags
//                  - added 2 memory management functions
//                  - changed display controller interface
//                  - added timer interface
//  2006-01-13 NSc  - moved videomode descriptor to videomodes.h
//  2006-02-16 CSe  - added d1_deviceclkfreq function
//  2006-02-28 CSe  - added d1_cacheblockflush function
//  2006-03-02 CSe  - extensions for video-input (vip)
//  2006-03-08 CSe  - added d1_displaytriggerupdate function
//  2006-03-15 CSe  - added d1_callirqhandler function
//  2006-03-22 CSe  - added d1_displayswitchlayersource function
//  2006-04-12 CSe  - changed d1_irqtypes to single bit values
//  2006-06-01 CSe  - added d1_getthreadid function
//  2006-06-15 CSe  - extension for system ID
//  2006-07-06 MGe  - added DAVE_TRACE deviceid ( dave_tace level1 driver )
//  2007-02-21 MGe  - added d1_irq_enum to d1_irqtypes
//  2007-10-05 CSe  - added function d1_displaygetactivelayersource
//  2007-10-18 ASc  - added display related structs to allow compilation of 
//                    dave driver without the display driver
//  2009-03-17 MMa  - support for TES DISPLAY CONTROLLER:
//                  - added device ID for TES Display Controller
//                  - added enums d1_alphamodes and d1_blendmodes
//                  - added new layermodes to d1_layermodes
//                  - added fields to d1_displaycaps
//                  - added fields to d1_layercaps
//                  - added display controller functions
//  2009-10-30 ASt  - extended enumeration type d1_layermodes (contains now the 6 supported formats)
//  2012-08-27 MRe  - added d1 devices D1_USERDATA and D1_DLISTINDIRECT (new version 1.3)

/*--------------------------------------------------------------------------
*  Title: D1 API
*
* Hardware dependent  D/AVE 2D low level driver
*
*---------------------------------------------------------------------
*/


#ifndef __1_dave_base_h_H
#define __1_dave_base_h_H
#ifdef __cplusplus
extern "C" {
#endif

#define D1_VERSION_MAJOR    1
#define D1_VERSION_MINOR    4

//---------------------------------------------------------------------------
// types

#ifndef D1_STDCALL
#ifdef WIN32
#define D1_STDCALL __stdcall
#else
#define D1_STDCALL
#endif
#endif



//---------------------------------------------------------------------------
// device id's

#define D1_DAVE2D       1
#define D1_BUTTONS      2
#define D1_LCDALTERA    3
#define D1_DAVEDBG      4
#define D1_VIP          5
#define D1_SYSID        6
#define D1_DAVE_TRACE   7
#define D1_LCDTES       8
#define D1_USERDATA     9
#define D1_DLISTINDIRECT   10
#define D1_TOUCHSCREEN  11


/*---------------------------------------------------------------------------
*  Enum: d1_buttons
*
*  Buttons
*
*  Values:
*    d1_btn_1       -  Button 1
*    d1_btn_2       -  Button 2
*    d1_btn_3       -  Button 3
*    d1_btn_4       -  Button 4
*    d1_btn_5       -  Button 5
*    d1_btn_6       -  Button 6
*
*/
enum d1_buttons
{
    d1_btn_1 = 1,
    d1_btn_2 = 2,
    d1_btn_3 = 4,
    d1_btn_4 = 8,
    d1_btn_5 = 16,
    d1_btn_6 = 32
};


/*---------------------------------------------------------------------------
*  Section: Types and enumerations
*/

#ifndef __1_dave_driver_h_H
/*---------------------------------------------------------------------------
*  Type: d1_device
*
*    Abstract type void
*
*    The application uses pointers of this type to hold address of 
*    low level device structure without knowing the internal layout. The low level 
*    device structure layout being devie dependent.
*    See <d1_opendevice>
*/
typedef void d1_device;
#endif

typedef void  ( D1_STDCALL *d1_interrupt)( int irqtype, void *irqdata, void *usrdata );

/*---------------------------------------------------------------------------*/

enum d1_timeouts
{
  d1_to_wait_forever = -1,
  d1_to_no_wait      =  0
};

/*---------------------------------------------------------------------------*/

enum d1_memtypes
{
  d1_mem_display = 1,
  d1_mem_texture = 2,
  d1_mem_dlist   = 4,

  d1_mem_any     = 7
};

/*---------------------------------------------------------------------------*/

enum d1_memqueries
{
  d1_mq_installed = 0,
  d1_mq_available,
  d1_mq_free,
  d1_mq_allocated
};

/*---------------------------------------------------------------------------*/

enum d1_mapflags
{
  d1_mf_read = 1,
  d1_mf_write = 2,
  d1_mf_readwrite = 3,
  d1_mf_direct = 4
};

/*---------------------------------------------------------------------------*/

enum d1_copyflags
{
  d1_cf_shorts = 1,
  d1_cf_ints   = 2,
  d1_cf_async  = 16
};

/*---------------------------------------------------------------------------*/

enum d1_irqtypes
{
  d1_irq_break = 1,
  d1_irq_vbi   = 2,
  d1_irq_dlist = 4,
  d1_irq_vip   = 8,
  d1_irq_enum  = 16
};

/*---------------------------------------------------------------------------
*  Enum: d1_memarchitectures
*  
*  Memory architecture types
*  
*  Values:
*    d1_ma_unified     -  CPU can directly access pointers returned by allocvidmem                                                 
*    d1_ma_separated   -  Video memory is separated from system memory and can be accessed by transfer or mapping functions only.  
*    d1_ma_uncached    -  CPU access to mapped video memory is not cached                                                          
*    d1_ma_fulluma     -  Static arrays and malloced memory can be accessed by the hardware                                        
*    d1_ma_mapped      -  alloced video mem is static mapped into system mem -> use <d1_mapfromvidmem>                                
*    d1_ma_fullmapped  -  hardware can access malloced memory and static arrays  -> use <d1_mapfromvidmem> and <d1_maptovidmem>
*
*/

enum d1_memarchitectures
{
  d1_ma_unified    = 0x1,   
  d1_ma_separated  = 0x2,   
  d1_ma_uncached   = 0x4,   
  d1_ma_fulluma    = 0x8,   
  d1_ma_mapped     = 0x10,  
  d1_ma_fullmapped = 0x20
};



/*--------------------------------------------------------------------------
*  Enum: enum d1_layermodes
*  
*  Video input formats for layers.
* 
*  Values:
*    d1_mode_alpha8   - 8 bit per pixel, alpha only
*    d1_mode_rgb565   - 16 bit per pixel: 5 red, 6 green, 5 blue
*    d1_mode_rgb888   - 32 bit per pixel: 8 red, 8 green, 8 blue, 8 unused
*    d1_mode_rgb444   - 16 bit per pixel: 4 red, 4 green, 4 blue, 4 unused
*    d1_mode_argb8888 - 32 bit per pixel: 8 alpha, 8 red, 8 green, 8 blue
*    d1_mode_argb4444 - 16 bit per pixel: 4 alpha, 4 red, 4 green, 4 blue
*    d1_mode_argb1555 - 16 bit per pixel: 1 alpha, 5 red, 5 green, 5 blue
*    d1_mode_rgba8888 - 32 bit per pixel: 8 red, 8 green, 8 blue, 8 alpha
*    d1_mode_rgba4444 - 16 bit per pixel: 4 red, 4 green, 4 blue, 4 alpha
*    d1_mode_grey8    -  8 bit per pixel: greyscale
*/
enum d1_layermodes
{
   d1_mode_alpha8       = 0,
   d1_mode_rgb565       = 1,
   d1_mode_rgb888       = 2,
   d1_mode_rgb444       = 3,
   d1_mode_argb8888     = 4,
   d1_mode_argb4444     = 5,
   d1_mode_argb1555     = 6,
   d1_mode_rgba8888     = 7,
   d1_mode_rgba4444     = 8,
   d1_mode_rgba5551     = 9,
   d1_mode_grey8        = 10,
   d1_mode_count
};


/*--------------------------------------------------------------------------
* type: enum d1_alphamodes
* Constant alpha modes
*
* values:
*  d1_alpha_none    - constant alpha is ignored
*  d1_alpha_replace - constant alpha is used for layer
*  d1_alpha_mul     - original alpha is multiplied by constant alpha
*/
enum d1_alphamodes {
    d1_alpha_none    = 0,
    d1_alpha_replace = 1,
    d1_alpha_mul     = 2        
};

/*--------------------------------------------------------------------------
* type: enum d1_blendmodes
* Alpha Blending Modes  
*
* values:
*  d1_blend_zero        - set alpha to 0.0
*  d1_blend_one         - set alpha to 1.0
*  d1_blend_app         - leave alpha unchanged
*  d1_blend_inv_app     - use inverted alpha
*  d1_blend_sub_app     - use alpha of subjacent layer
*  d1_blend_inv_sub_app - use inverted alpha of subjacent layer
*/
enum d1_blendmodes {
    d1_blend_zero        = 0,
    d1_blend_one         = 1,
    d1_blend_app         = 2,
    d1_blend_inv_app     = 3,
    d1_blend_sub_app     = 4,
    d1_blend_inv_sub_app = 5
};


/*--------------------------------------------------------------------------
*  Type: struct d1_videomode
*
*  Display controller video output parameters.
* 
*  Members:
*    modeid - global ID for mode
*    width  - width of output picture in pixels
*    height - height of output picture in pixels
*    vfreq  - refresh rate in Hz
*
*/
typedef struct _d1_videomode
{
   unsigned int modeid;
   unsigned int width;
   unsigned int height;
   unsigned int vfreq;
   
} d1_videomode;


/*--------------------------------------------------------------------------
*  Type: struct d1_layercaps
*
*  Display controller layer capabilities.
* 
*  Members:
*    modecount  - number of modes supported by layer
*    layermodes - array of modes (pixel formats) supported by layer of type (<enum d1_layermodes>)
*    constant   - true when layer allows constant color only (e.g. background) (use: <d1_displaysetlayercolor>)
*    alpha      - true when layer supports alpha (use: <d1_displaysetlayeralpha>)
*    window     - true when layer supports windowing (use: <d1_displaysetlayerwindow>)
*    swappable  - true when top-down position of layer can be changed (use: <d1_displaysetlayerorder>)
*   moveable     - true when the position of layer can be changed (use <d1_displaysetlayerwindow>)
*  resizeable   - true when layer can be resized if windowing is enabled (use <d1_displaysetlayerwindow>)
*   framecolour  - true when a framecolour can be set for layer (use <d1_displaysetlayerframecolour>)
*   alphalayer   - true when a alphalayer is available for layer (use <d1_displaysetlayerenable>)
*   pitch        - true when the width can be smaller than the pitch (use <d1_displaysetlayerformat> and <d1_displaysetlayerwindow>)
*   duplicate    - true when the layer can be duplicated in horizontal and/or vertical direction (use <d1_displaysetlayerduplication>)
*   colourkey    - true when a colourkey can be set for the layer (use <d1_displaysetlayercolourkey>)
*   a1_modecount - number blendmodes for upper alpha supported by layer
*   a1_modes     - array of blendmodes for upper alpha supported by layer of type (<enum d1_blendmodes>) (use <d1_displaysetlayerblendmode>)
*   a0_modecount - number blendmodes for lower alpha supported by layer
*   a0_modes     - array of blendmodes for lower alpha supported by layer of type (<enum d1_blendmodes>) (use <d1_displaysetlayerblendmode>)
*/
typedef struct _d1_layercaps
{
  unsigned int modecount;
  const int *layermodes;
  int constant;  
  int alpha;     
  int window;    
  int swappable; 
  int moveable;
  int resizeable;
  int framecolour;
  int alphalayer;
  int pitch;
  int duplicate;
  int colourkey;
  int a1_modecount;
  int *a1_modes;
  int a0_modecount;
  int *a0_modes;
} d1_layercaps;


/*--------------------------------------------------------------------------
*  Type: struct d1_displaycaps
*  
*  Display controller capabilities.
* 
*  Members:
*   modecount     - number of output modes supported by display controller
*   videomodes    - array of output modes supported by display controller of type <struct d1_videomode>
*   layercount    - number of layers supported by display controller
*   layercaps     - array of layers capabilities of type <struct d1_layercaps>
*   major_rev     - major hardware revision of display controller
*   minor_rev     - minor hardware revision of display controller
*   blind_mode    - true when controller runs in blind mode
*   shadowing     - true when controller has shadow registers
*   gammaramp     - true when controller has a gammaramp
*   windowing     - true when controller supports windowing for layers
*   precise_blend - true when a precise alphablending method is used
*   red_bits      - number of red output bits
*   green_bits    - number of green output bits
*   blue_bits     - number of blue output bits
* 
*/ 
typedef struct _d1_displaycaps
{
  unsigned int modecount;
  const d1_videomode *videomodes;
  unsigned int       layercount;
  const d1_layercaps *layercaps;
  int major_rev;
  int minor_rev;
  int blind_mode;
  int shadowing;
  int gammaramp;
  int windowing;
  int precise_blend;
  int red_bits;
  int green_bits;
  int blue_bits;
} d1_displaycaps;



/*---------------------------------------------------------------------------
*  Section: Basic Functions
*/

/*--------------------------------------------------------------------------- 
*  Function: d1_getversion
*   
*  Query version ID (device dependent).
*  
*  Parameters:
*    None
*  
*  Returns:
*    The driver revision as a single 32-bit integer (major version in the upper 16 bits)
*
*  See also:
*    <d1_getversionstring>
*/
extern int d1_getversion();

/*--------------------------------------------------------------------------- 
*  Function: d1_getversionstring
*   
*  Query version ID string (device dependent).
*  
*  Parameters:
*    None
*  
*  Returns:
*    the driver version as a atring
*
*  See also:
*    <d1_getversion>
*
*/
extern const char * d1_getversionstring();

/*--------------------------------------------------------------------------- 
*  Function: d1_opendevice
*   
*  Creates a device handle to access hardware
*
*  This function sets the D/AVE ISR, enables the interrupt handling and allocates
*  memory for a d1 handle.
*  It is called by the D/AVE 2D driver when initialising the hardware <d2_inithw>.
*  
*  Parameters:
*    flags - reserved. pass 0
*  
*  Returns:
*    pointer to a <d1_device> object or 0 if hardware access failed
*
*/
extern d1_device * d1_opendevice( long flags );

/*--------------------------------------------------------------------------- 
*  Function: d1_closedevice
*   
*  Close a device handle
*
*  It is called by the D/AVE 2D driver when de-initialising the hardware <d2_deinithw>.
*  Disables all interrupts.
*  
*  Parameters:
*    handle - pointer to the <d1_device> object
*  
*  Returns:
*    int - 0 if error else 1.
*
*/
extern int d1_closedevice( d1_device *handle );

/*--------------------------------------------------------------------------- 
*  Function: d1_getthreadid 
*    
*  Return current thread ID
*    
*  Parameters:
*    handle - device handle <d1_device>, see <d1_opendevice>
*  
*  Returns:
*    Id of the current thread based on query to OS.
*/
extern int d1_getthreadid( d1_device *handle );

/*---------------------------------------------------------------------------
*  Section: Memory management  
*/

/*--------------------------------------------------------------------------- 
*  Function: d1_memsize
*   
*  Returns the size of the given memory block
*  
*  Parameters:
*    ptr - pointer to a memory block in Heap
*  
*  Returns:
*    int - size of the memory block.
*
*/
extern unsigned int d1_memsize(void * ptr);

/*--------------------------------------------------------------------------- 
*  Function: d1_allocmem
*   
*  Allocates memory on the CPU heap
*  
*  Parameters:
*    size - Size of the memory to be allocated
*  
*  Returns:
*    void * - Pointer to the start address of the allocated memory block
*               0 if not successful
*
*/
extern void * d1_allocmem(unsigned int size );

/*--------------------------------------------------------------------------- 
*  Function: d1_freemem
*   
*  Free the specified memory area on the CPU heap
*  
*  Parameters:
*    ptr - Pointer to the memory area to be freed
*  
*  Returns:
*    None
*
*/
extern void d1_freemem(void *ptr );

/*--------------------------------------------------------------------------
*  Function: d1_allocvidmem
*
*  Allocate video memory
* 
*  For systems without a unified memory architecture all memory that will
*  be accessed directly by hardware has to be video memory.
*  On a UMA platform d1_allocvidmem will most likely behave just like a simple malloc.
* 
*  Note that not every platform will make use of the 'memtype' it is
*  used as a hint in order to get the memory best suited for the specified
*  purpose.
* 
*  All memory allocated using this function must be released by <d1_freevidmem>
* 
*  Parameters:
*    handle  - device handle (see: <d1_opendevice>)
*    memtype - type of memory (see list below)
*    size    - number of bytes
* 
*  Memtypes:
*    d1_mem_display - used if the memory is going to be displayed (framebuffer)
*    d1_mem_texture - used for textures (accessed by dave but not by the displaycontroller)
*    d1_mem_dlist   - used for display lists (read only for hardware)
* 
*  Returns:
*    pointer to memorychunk or NULL.
*    
*    the pointer returned by allocvidmem points directly to video memory and can
*    not be accessed by the CPU (unless the memory architecture is unified - see
*    <d1_queryarchitecture>). Transfer functions like <d1_copytovidmem>
*    and <d1_copyfromvidmem> can access video memory directly.
*/
extern void * d1_allocvidmem( d1_device *handle, int memtype, unsigned int size );

/*--------------------------------------------------------------------------
*  Function: d1_freevidmem
*
*  Release video memory
* 
*  The memtype should be identical to what has been specified when allocating
*  the block.
* 
*  Parameters:
*    handle  - device handle (see: <d1_opendevice>)
*    memtype - memory pool id (see: <d1_allocvidmem>)
*    ptr     - address returned by <d1_allocvidmem>
*
*  Returns:
*    None
*/
extern void d1_freevidmem( d1_device *handle, int memtype, void *ptr );

/*--------------------------------------------------------------------------
*  Function: d1_queryvidmem
* 
*  Get current memory status
* 
*  Not every platform will treat all memory pools as distinct heaps, so
*  the return values might be the same for different memtype's.
* 
*  Parameters:
*    handle  - device handle (see: <d1_opendevice>)
*    memtype - memory pool id (see: <d1_allocvidmem>)
*    query   - type of requested information (see below)
* 
*  Query types:
*    d1_mq_installed - size of installed memory
*    d1_mq_available - largest available free chunk
*    d1_mq_free      - total amount of free memory
*    d1_mq_allocated - total amount of allocated memory
* 
*  returns:
*    number of bytes
*/
extern int d1_queryvidmem( d1_device *handle, int memtype, int query );

/*--------------------------------------------------------------------------
*  Function: d1_mapvidmem
* 
*  Map video memory for direct CPU access
* 
*  In order to access video memory directly with the CPU it has to be
*  mapped into system memory (unless the host has a unified memory architecture).
*  This function always maps an entire block (as allocated by <d1_allocvidmem>)
*  and therfore does not need a 'size' argument.
* 
*  Mappings must be released before the memory is freed by <d1_freevidmem>.
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
*    ptr    - video memory address returned by <d1_allocvidmem>
*    flags  - memory mapping flags (see below)
* 
*  Memory mapping flags:
*    d1_mf_read      - mapped memory will be read only
*    d1_mf_write     - mapped memory will be written only
*    d1_mf_readwrite - full access to mapped memory
*    d1_mf_direct    - try mapping to the same address (make videomemory visible to
*                      current process using its original address)
* 
*  Returns:
*    CPU accessible pointer to the same memory region
* 
*  See also:
*    <d1_unmapvidmem>
*/  

extern void * d1_mapvidmem( d1_device *handle, void *ptr, int flags );

/*--------------------------------------------------------------------------
*  Function: d1_unmapvidmem
* 
*  Release memory mapping
*  
*  Memory mapped using <d1_mapvidmem> should be unmapped before it is released.
*  
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
*    ptr    - mapped video memory address returned by <d1_mapvidmem>
*  
*  Returns:
*    boolean success.
*/
extern int d1_unmapvidmem( d1_device *handle, void *ptr );

/*--------------------------------------------------------------------------
*  Function: d1_maptovidmem
* 
*  Map CPU accessible address of a video memory block back to video memory
*  address
* 
*  In systems, which have CPU and hardware address spaces statically mapped to each other
*  (see: <d1_queryarchitecture>), this function can be used to convert an address inside
*  the CPU address space and pointing to a block of video memory back to a video memory address.
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
*    ptr    - CPU accessible address pointing to a video memory block originally allocated using <d1_allocvidmem>
* 
*  Returns:
*    hardware accessible pointer to the same memory region or NULL if mapping not possible
* 
*  See also:
*    <d1_mapfromvidmem>
*/
extern void * d1_maptovidmem( d1_device *handle, void *ptr );

/*--------------------------------------------------------------------------
*   Function: d1_mapfromvidmem
* 
*   Map already allocated video memory address to an address for direct CPU access
*  
*   In systems, which have CPU and hardware address spaces statically mapped to each other
*   (see: <d1_queryarchitecture>), this function can be used to convert a video memory
*   address to an address in the CPU address space.
*  
*   Parameters:
*     handle - device handle (see: <d1_opendevice>)
*     ptr    - video memory address returned by <d1_allocvidmem>
*  
*   Returns:
*     CPU accessible pointer to the same memory region or NULL if mapping not possible
*  
*   See also:
*     <d1_maptovidmem>
*/
extern void * d1_mapfromvidmem( d1_device *handle, void *ptr );

/*--------------------------------------------------------------------------
*  Function: d1_copytovidmem
* 
*  Copy data to video memory
* 
*  Destination (video) memory area has to be allocated by <d1_allocvidmem>.
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
*    dst    - pointer into video memory (destination)
*    src    - pointer into system memory (source)
*    size   - number of bytes to copy
*    flags  - bitfield containing additional information on data to be copied
*    
*  Flags:
*    d1_cf_shorts - data consists of 16 bit values
*    d1_cf_ints   - data consists of 32 bit values
*    d1_cf_async  - allow asynchronous copy and return before the transfer is finished
* 
*  Returns:
*    boolean success.
* 
*  Note:
*    A cache flush is implicitly done if the flag 'd1_cf_async' is not passed.
*    To be sure, an asynchronous copy was finished, either start another copy or use see <d1_cacheflush>.
*/
extern int d1_copytovidmem( d1_device *handle, void *dst, const void *src, unsigned int size, int flags );

/*--------------------------------------------------------------------------
*  Function: d1_copyfromvidmem
* 
*  Copy data from video memory
*  
*  Source (video) memory area has to be allocated by <d1_allocvidmem>.
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
*    dst    - pointer into system memory (destination)
*    src    - pointer into video memory (source)
*    size   - number of bytes to copy
*    flags  - reserved for future use. pass 0 for now.
* 
*  Returns:
*    boolean success.
* 
*  Note:
*    a cache flush is implicitly done to be sure to use correct data.
*/
extern int d1_copyfromvidmem( d1_device *handle, void *dst, const void *src, unsigned int size, int flags );

/*--------------------------------------------------------------------------
*  Function: d1_cacheflush
* 
*  Flush CPU data caches
*  
*  When accessing video memory using the CPU it might be necessary to flush
*  CPU data caches in order for the DAVE hardware to see the changes.
* 
*  Parameters:
*    handle  - device handle (see: <d1_opendevice>)
*    memtype - memory pools to flush (can be or'ed together)
* 
*  Memtypes:
*    d1_mem_any     - flush datacache entirely
*    d1_mem_display - flush addresses from d1_mem_display only
*    d1_mem_texture - flush addresses from d1_mem_texture only
*    d1_mem_dlist   - flush addresses from d1_mem_dlist only
*    
*  Returns:
*    boolean success.
* 
*  See also:
*    <d1_queryarchitecture>
*/
extern int d1_cacheflush( d1_device *handle, int memtype );

/*--------------------------------------------------------------------------
*  Function: d1_cacheblockflush
* 
*  Flush part of CPU data caches
*  
*  When accessing video memory using the CPU it might be necessary to flush
*  CPU data caches in order for the DAVE hardware to see the changes.
* 
*  Parameters:
*    handle  - device handle (see: <d1_opendevice>)
*    memtype - memory type to be flushed
*    ptr     - start address of memory to be flushed
*    size    - size of memory to be flushed
* 
*  Memtypes:
*    d1_mem_display - flush addresses from d1_mem_display only
*    d1_mem_texture - flush addresses from d1_mem_texture only
*    d1_mem_dlist   - flush addresses from d1_mem_dlist only
* 
*  Returns:
*    boolean success.
* 
*  See also:
*    <d1_queryarchitecture>
*/
extern int d1_cacheblockflush( d1_device *handle, int memtype, const void *ptr, unsigned int size );

/*--------------------------------------------------------------------------
*  Function: d1_queryarchitecture
* 
*  Return hints about systems memory architecture
*  
*  The application can often avoid memory transfers if a unified memory
*  architecture is available on the host system. This function can be used
*  to query the host memory architecture.
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
* 
*  Returns:
* 
*    a bitfield with the following bits set if applicable :
* 
*    d1_ma_unified    - CPU can directly access pointers returned by <d1_allocvidmem>.
*                       Transfer and mapping functions will work but are unnecessary.
*    d1_ma_separated  - Video memory is separated from CPU memory and can be
*                       accessed by copy or mapping functions only.
*    d1_ma_uncached   - CPU access to mapped video memory is not cached.
*    d1_ma_fulluma    - Static arrays and malloced memory can be accessed by the hardware.
*    d1_ma_mapped     - Allocated video memory is statically mapped into CPU memory.
*                       Use <d1_mapfromvidmem> and <d1_maptovidmem> to convert addresses.
*    d1_ma_fullmapped - The whole CPU memory (malloced memory as well as static arrays)
*                       is statically mapped to the address space of the hardware.
*                       Use <d1_mapfromvidmem> and <d1_maptovidmem> to convert addresses
*                       between CPU address space and hardware address space.
*/
extern int d1_queryarchitecture( d1_device *handle );

/*---------------------------------------------------------------------------
*    Section: Hardware port interface
*/

/*--------------------------------------------------------------------------- 
*   Function: d1_devicesupported
*    
*   Check if the specified device is supported
*
*   Different hardware components and revisions offer different hardware registers (see: <Device IDs>). 
*   Use this function to verify that a specific hardware interface is available on the current host system.
*
*   Parameters:
*     handle   - pointer to a low level device <d1_device>, see <d1_opendevice>
*     deviceId - used to specify target hardware component, see <Device IDs> 
*
*   Returns:
*     int - 0 if not supported else 1
*
*/
extern int d1_devicesupported( d1_device *handle, int deviceid );

/*--------------------------------------------------------------------------- 
*   Function: d1_deviceclkfreq
*    
*   Returns the clock frequency of the D/AVE 2D hardware
*
*   Parameters:
*     handle   - pointer to a low level device <d1_device>, see <d1_opendevice>
*     deviceid - device type
*   
*   Returns:
*     unsigned long - clock frequency of the drawing engine
*
*/
extern unsigned long d1_deviceclkfreq( d1_device *handle, int deviceid );

/*--------------------------------------------------------------------------- 
*   Function: d1_setregister
*    
*   Writes to hardware register
*
*   Access to an invalid or unsupported deviceid will be ignored
*
*   Parameters:
*     handle   - pointer to a device <d1_device>, see <d1_opendevice>
*     deviceid - used to specify target hardware component, see <Device IDs>
*     index    - register index (starts with 0)
*     value    - 32-bit value to write
*       
*   Returns:
*     None
*
*/
extern void d1_setregister( d1_device *handle, int deviceid, int index, long value );

/*--------------------------------------------------------------------------- 
*   Function: d1_getregister
*    
*   Read from hardware register
*
*   Reading a register from an invalid or unsupported deviceId will always return 0
*
*   Parameters:
*     handle   - pointer to a low level device <d1_device>, see <d1_opendevice>
*     deviceid - used to specify target hardware component, see <Device IDs>
*     index    - register index (starts with 0)
*   
*   Returns:
*     long     - 32-bit value of the register
*
*/
extern long d1_getregister( d1_device *handle, int deviceid, int index );


/*---------------------------------------------------------------------------
*    Section: Interrupt control
*/

/*--------------------------------------------------------------------------- 
*   Function: d1_setirqhandler
*   
*   Register an IRQ handler
*
*   Registered callback will be called when the specified IRQ triggers. 
*   The IRQ will be reset automatically after the callback has been executed.  
*   Note that the callback code might run at a different privilege level than the main application.
*
*   You can pass NULL instead of a function pointer in order to remove a registered callback.
*   
*   Not every interrupt is available on every platform.  Unsupported IRQs will never trigger but 
*   setting or removing a callback for them does no harm.
*    
*   Parameters:
*     handle  - device handle (see: <d1_opendevice>)
*     irqtype - interrupt ID (see below)
*     code    - callback address (NULL to remove handler)
*     data    - userdefined data (passed on to callback directly)
*
*   Interrupt IDs:
*     d1_irq_break - userbreak
*     d1_irq_vbi   - vertical blank
*     d1_irq_dlist - display list finished
*   
*   Callback signature:
*   
*     | void D1_STDCALL d1_interrupt( int irqtype, void *irqdata, void *usrdata );
*   
*   Note:
*   
*     Consider that existing ISR registrations might be disabled when setting new irq handlers. 
*     An ISR should always call the original ISR that was registered before d1_setirqhandler was 
*     called (use <d1_getirqhandler>). Otherwise, e.g. in case of the vertical blank irq (d1_irq_vbi),
*     this will disable the display controller driver.
*
*   See also:
*     <d1_getirqhandler>, <d1_getirqdata>
*/
extern void d1_setirqhandler( d1_device *handle, int irqtype, d1_interrupt code, void *data );

/*--------------------------------------------------------------------------- 
*  Function: d1_getirqhandler
*   
*  Retrieve an IRD handler
*
*  IRQ handler callbacks installed by <d1_setirqhandler> can be retrieved using this function
*
*  Parameters:
*    handle  - device handle (see: <d1_opendevice>)
*    irqtype - interrupt ID (see: <d1_setirqhandler>)
*
*  Returns:
*    callback address or 0 if no callback is registered for given IRQ       
*
*  See also:
*    <d1_getirqdata>, <d1_setirqhandler>
*/
extern d1_interrupt d1_getirqhandler( d1_device *handle, int irqtype );

/*--------------------------------------------------------------------------- 
*   d1_callirqhandler
*
*/
extern int d1_callirqhandler( d1_device *handle, int irqtype, void *irqdata );

/*--------------------------------------------------------------------------
*  Function: d1_getirqdata
*
*  Retrieve user defined data of specified IRQ
*  
*  User defined data assigned by <d1_setirqhandler> can be retrieved
*  using this function.
*  
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
*    irqtype - interrupt ID (see: <d1_setirqhandler>)
*  
*  Returns:
*    userdefined data or NULL if no callback is registered for given IRQ
*  
*  See also:
*    <d1_getirqhandler>, <d1_setirqhandler>
*/
extern void * d1_getirqdata( d1_device *handle, int irqtype );

/*--------------------------------------------------------------------------
*  Function: d1_queryirq
*
*  Wait for next execution of specified IRQ
*  
*  Instead of using IRQ callbacks you can manually block execution until a
*  certain IRQ triggers. This is equivalent to setting up a callback that
*  signals an Event and a 'WaitForEvent' inside the main thread.
*  
*  This function is often used to synchronize rendering with the vertical
*  blank for flicker free animation.
*   
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
*    irqmask - interrupt ID (see: <d1_setirqhandler>)
*    timeout - flag to specify how long to wait for the interrupt (see below)
*   
*  Returns:
*     irqID that triggered
*   
*  Timeout flags:
*     d1_to_wait_forever - wait forever
*     d1_to_no_wait - do not wait at all
*/
extern int d1_queryirq( d1_device *handle, int irqmask, int timeout );

/*---------------------------------------------------------------------------
*   Section: Timer interface
*   Not required by the d2 driver at all: Only for better platform portability of applications.
*/

/*--------------------------------------------------------------------------
*  Function: d1_timerres
*
*  Get the resolution of the timer
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
* 
*  Returns:
*    resolution of the timer (in microseconds)
* 
*  See also:
*    <d1_timerlimit>, <d1_timerreset>, <d1_timervalue>
*/
extern unsigned long d1_timerres  ( d1_device *handle );

/*--------------------------------------------------------------------------
*  Function: d1_timerlimit
*
*  Get the maximum value of the timer
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
* 
*  Returns:
*    the maximum timer value
* 
*  See also:
*    <d1_timerlimit>, <d1_timerreset>, <d1_timervalue>
*/
extern unsigned long d1_timerlimit( d1_device *handle );

/*--------------------------------------------------------------------------
*  Function: d1_timerreset
*
*  Reset the timer to zero
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
* 
*  See also:
*    <d1_timerlimit>, <d1_timerreset>, <d1_timervalue>
*/
extern void          d1_timerreset( d1_device *handle );

/*--------------------------------------------------------------------------
*  Function: d1_timervalue
*
*  Retrieves the number of microseconds that have elapsed since the
*  last reset
*
*  It is limited to the resolution of the used timer (see <d1_timerres>).
* 
*  Parameters:
*    handle - device handle (see: <d1_opendevice>)
* 
*  Returns:
*    the return value is the number of microseconds that have elapsed since
*    the last reset
* 
*  See also:
*    <d1_timerlimit>, <d1_timerreset>, <d1_timervalue>
*/
extern unsigned long d1_timervalue( d1_device *handle );

#ifndef Dx4
/*---------------------------------------------------------------------------
*   Section: Display controller interface
*   Not required by the d2 driver at all: Only for better platform portability of applications.
*/

//--------------------------------------------------------------------------
// function: d1_displaysetmode
//
// Set display controller output mode.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//  mode   - ID of output mode (see <struct d1_videomode>)
//
// Pre-defined mode IDs:
//  D1_VIDEO_OFF        - turn off display controller (if possible) 
//  D1_VIDEO_DEFAULT    - use default mode (first mode in list of
//                        possible modes from <d1_displaygetcaps>)
//  D1_VIDEO_640_400_60 - 640x400 pixels at 60 Hz
//  D1_VIDEO_640_480_60 - 640x480 pixels at 60 Hz
//  D1_VIDEO_800_600_60 - 800x600 pixels at 60 Hz
//
// Mode IDs of modes that are supported can be found out using
// <d1_displaygetcaps>.
//
// returns:
//  boolean success (false if display is not capable)
//
extern int d1_displaysetmode( d1_device *handle, unsigned int mode );

extern int d1_displaysetlayerblendmode( d1_device *handle, int layer, int a1_mode, int a0_mode );

extern int d1_displaysetlayeralphamode( d1_device *handle, int layer, int mode );

extern int d1_displaysetlayercolourkey( d1_device *handle, int layer, unsigned int colourkey, int enable );

extern int d1_displaysetlayerduplication( d1_device *handle, int layer, int hor_duplication, int ver_duplication );

extern int d1_displaysetlayerframecolour( d1_device *handle, int layer, unsigned int framecolour );

//--------------------------------------------------------------------------
// function: d1_displaysetlayersource
//
// Set pointer to frame buffer for non-constant layer.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//  layer  - layer number starting at 0 (see <d1_displaygetcaps> for number of layers/capabilities) 
//  data   - start address of frame buffer
//
// returns:
//  boolean success (false if layer is not capable)
//
extern int d1_displaysetlayersource( d1_device *handle, int layer, void *data );

//--------------------------------------------------------------------------
// function: d1_displaysetlayerformat
//
// Set input pixel format for layer.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//  layer  - layer number starting at 0 (see <d1_displaygetcaps> for number of layers/capabilities) 
//  mode   - pixel format (see <enum d1_layermodes> for possible values)
//  pitch  - length of lines in pixels (normally width of layer)
//
// returns:
//  boolean success (false if layer is not capable)
//
extern int d1_displaysetlayerformat( d1_device *handle, int layer, int mode, int pitch );

//--------------------------------------------------------------------------
// function: d1_displaysetlayerwindow
//
// Set window for layer if layer is capable.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//  layer  - layer number starting at 0 (see <d1_displaygetcaps> for number of layers/capabilities) 
//  x      - x position of top left corner in display pixel coordinates
//  y      - y position of top left corner in display pixel coordinates
//  width  - width of window in pixels
//  height - height of window in pixels
//
// returns:
//  boolean success (false if layer is not capable)
//
extern int d1_displaysetlayerwindow( d1_device *handle, int layer, int x, int y, int width, int height );

//--------------------------------------------------------------------------
// function: d1_displaysetlayeralpha
//
// Set constant alpha value for layer if layer is capable.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//  layer  - layer number starting at 0 (see <d1_displaygetcaps> for number of layers/capabilities) 
//  alpha  - alpha value in layer alpha format (see <d1_displaysetlayerformat>)
//
// returns:
//  boolean success (false if layer is not capable)
//
extern int d1_displaysetlayeralpha( d1_device *handle, int layer, int alpha );

//--------------------------------------------------------------------------
// function: d1_displaysetlayercolor
//
// Set constant color value for layer if layer is capable.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//  layer  - layer number starting at 0 (see <d1_displaygetcaps> for number of layers/capabilities) 
//  color  - color value in layer pixel format (see <d1_displaysetlayerformat>)
//
// returns:
//  boolean success (false if layer is not capable)
//
extern int d1_displaysetlayercolor( d1_device *handle, int layer, int color );

//--------------------------------------------------------------------------
// function: d1_displaysetlayerorder
//
// Change top-down order of layers if possible.
//
// parameters:
//  handle   - device handle (see <d1_opendevice>)
//  layer    - layer number starting at 0 (see <d1_displaygetcaps> for number of layers/capabilities) 
//  position - new position of layer (0 is bottom)
//
// If a layer is moved to a new position the positions of the other layers are affected
// in the following way:
// Default layer position is the same as layer number (layer 0 is at bottom).
// When a new position is assigned the layer is removed where it was before and inserted at the
// new position thus shifting all other layers in between by 1 towards the position where the layer
// was located before.
//
// returns:
//  boolean success (false if display/controller is not capable)
//
extern int d1_displaysetlayerorder( d1_device *handle, int layer, int position );

//--------------------------------------------------------------------------
// function: d1_displaysetlayerenable
//
// Switch separate layers on/off.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//  layer  - layer number starting at 0
//  enable - boolean enable
//
// returns:
//  boolean success (false if display/controller is not capable)
//
extern int d1_displaysetlayerenable( d1_device *handle, int layer, int enable );

//--------------------------------------------------------------------------
// function: d1_displayswitchlayersource
//
// Change pointer to frame buffer for non-constant layer immediately or synchronized to vertical blanking.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//  layer  - layer number starting at 0 (see <d1_displaygetcaps> for number of layers/capabilities) 
//  data   - start address of frame buffer
//  sync   - boolean: synchronize change to vertical blanking or not
//
// Not every display controller might be capable of changing the frame buffer source out of sync.
// In this case, the 'sync' parameter is not considered.
//
// returns:
//  boolean success (false if layer is not capable)
//
extern int d1_displayswitchlayersource( d1_device *handle, int layer, void *data, int sync );

//--------------------------------------------------------------------------
// function: d1_displaytriggerupdate
//
// Trigger an update of new display controller settings during next vertical
// blanking.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//
// Activates all new settings made by d1_displayset... functions since
// last update during next vertical blanking.
//
// Returns immediately: Settings made after this call before next vertical
// blanking are also activated!!
//
// returns:
//  boolean success
//
extern int d1_displaytriggerupdate( d1_device *handle );

//--------------------------------------------------------------------------
// function: d1_displayupdate
//
// Apply new display controller settings and update screen.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//
// Activates all new settings made by d1_displayset... functions since
// last update during next vertical blanking.
// Returns after next vertical blanking. 
//
// returns:
//  boolean success
//
extern int d1_displayupdate( d1_device *handle );

extern void * d1_displaygetactivelayersource( d1_device *handle, int layer ); 

//--------------------------------------------------------------------------
// function: d1_displaygetcaps
//
// Query display controller/layer capabilities.
//
// parameters:
//  handle - device handle (see <d1_opendevice>)
//
// returns:
//  pointer to (<struct d1_displaycaps>)
//
extern const d1_displaycaps * d1_displaygetcaps( d1_device *handle );
#endif /* Dx4 */

/*--------------------------------------------------------------------------
*   Function: d1_createdifbo
* 
*   Create a DI FBO for a given frame buffer.
*
*   Parameters:
*     handle - device handle (see: <d1_opendevice>)
*     ptr    - gpu video memory address returned by <see: d1_allocvidmem>
*     width  - frame buffer width
*     height - frame buffer height in pixels
*     pitch  - frame buffer pitch in bytes
*     bpp    - frame buffer bytes per pixel
*  
*   Returns:
*     DI FBO handle for the given frame buffer.
*
*/
#ifndef di_fbo_t
typedef struct _di_fbo di_fbo_t;
#endif
extern di_fbo_t * d1_createdifbo(d1_device *handle, void *ptr, unsigned int width, unsigned int height, unsigned int pitch, int bpp);

/*--------------------------------------------------------------------------
*   Function: d1_deletedifbo
* 
*   Delete a DI FBO.
*
*   Parameters:
*     handle - device handle (see: <d1_opendevice>)
*     difbo  - DI FBO handle to delete <see: d1_createdifbo>
*
*/
extern void d1_deletedifbo(d1_device *handle, di_fbo_t *fbo);

#ifdef __cplusplus
}
#endif
#endif
