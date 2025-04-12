/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_dlist.h (%version: 20 %)
 *          created Thu Feb 17 21:44:45 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Jan 25 17:39:49 2007 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 * 2005-12-22 CSe  - added d2_copydlist_vidmem_intern()
 *                 - renamed block->vram to block->vidmem for consistency
 *                 - big endian change + define for BIG_ENDIAN
 * 2005-02-28 CSe  - added d2_cacheflushdlist_intern()
 * 2006-05-10 CSe  - added d2_mapdlist_vidmem_intern()
 * 2006-10-30 CSe  - removed counting of dlist used slots
 * 2006-11-07 CSe  - changes for new 'd2_df_low_localmem' mode
 * 2006-11-30 CSe  - fixed insertion of dlist special commands
 * 2007-01-26 CSe  - made 'low localmem' mode configurable
 * 2007-03-08 CSe  - optimized dlist copying and flushing
 * 2008-01-14 ASc  - changed comments from C++ to C, removed tabs
 * 2012-08-23 MRe  - extended dlist structure for dlist lists
 * 2012-09-25 BSp  - MISRA cleanup
 * 2017-07-27 HFu  - clearly commented and renamed d2_insertwait...dlist_intern functions
 *-------------------------------------------------------------------------- */

#ifndef __1_dave_dlist_h_H
#define __1_dave_dlist_h_H
/*--------------------------------------------------------------------------- */

#define D2_DLISTWRITES(x, y)  d2_add2dlist_intern( handle, x, y )             /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/
#define D2_DLISTWRITEU(x, y)  d2_add2dlist_intern( handle, x, (d2_s32) (y) )  /* PRQA S 3453 */ /* $Misra: #MACRO_FXN_FORWARD $*/

/*-------------------------------------------------------------------------- */
#define D2_DLISTBLOCKSIZE 204    /* blocksize: initial number and number of entrys to grow when dlist is full (5*4*204 = approx. 4kBytes) */
#define D2_DLISTSCRATCH   64     /* number of scratch buffer entries */
#define D2_DLISTSHRINKDELAY 60   /* number of frames, the amount of display list blocks needs to be too large, before it is reduced */


/*--------------------------------------------------------------------------- */

typedef struct _d2_dlist_vidmem_blocks
{
   void **   blocks;          /* array of dlist blocks in vidmem */
   d2_u32    num_blocks;      /* number of entries in above array */
   d2_u32    block_size;      /* size of blocks in vidmem */
   void **   currentblock;    /* pointer into above array of vidmem blocks */
   d2_s8 *   currentaddress;  /* pointer into vidmem inside current block (multiple local blocks are copied into one vidmem block) */
   d2_u32    num_slices;      /* one vidmem blocks consists of multiple slices (see above) */
   d2_u32    slicesleft;      /* number of slices left */
   d2_u32    blocksleft;      /* number of vidmem blocks left */
} d2_dlist_vidmem_blocks;


typedef struct _d2_dlist_entry
{
   union d2_addresstag                 /* register tag field (4 reg indices) */
   {
      d2_u32 mask;
      d2_u8  array[4];
   } address;

   d2_s32 value[4];                       /* register value field */
} d2_dlist_entry;


typedef struct _d2_dlist_block
{
   struct _d2_dlist_block *next;

   d2_dlist_entry * block;              /* display list memory */
   d2_u32           quantity;           /* number of dlist_entries */
   d2_s32 *         jump;               /* pointer to addressentry of next display list or null */
   void *           vidmem;             /* address of memoryblock in videoram or null */
} d2_dlist_block ;


typedef struct _d2_dlist
{
   d2_device              *device;
   d2_dlist_block         *firstblock;
   d2_dlist_block         *currentblock;
   d2_dlist_block         *resumeblock;
   d2_dlist_entry         *position;
   d2_dlist_vidmem_blocks *vidmem_blocks;   /* used in low local memory mode only */

   d2_u32                  tagindex;
   d2_u32                  blocksize;
   d2_u32                  stepsize;
   d2_s32                  busy;
   d2_u32                  shrinkcount;
   d2_u32                  count;

   void *                  dlist_addresses;        /* list of dlist start addresses accessible by Dave */
   d2_s16                  dlist_addresses_max;    /* max number of dlist start addresses in dlist_addresses */
   d2_s16                  dlist_addresses_cur;    /* current number of dlist start addresses in dlist_addresses */
} d2_dlist;


typedef struct _d2_dlist_scratch_entry
{
   d2_u32             reg;
   d2_s32             value;
} d2_dlist_scratch_entry;


typedef struct _d2_lowlocalmem_mode
{
   d2_dlist_block *dlist_buffer; /* two dlist blocks linked as ring */
   d2_u32 vidmemblocksizefactor; /* factor for size of blocks in vidmem relative to local size */
   d2_u32 vidmemblocks;          /* maximum number of dlist blocks in vidmem */
} d2_lowlocalmem_mode;

/*--------------------------------------------------------------------------- */

D2_EXTERN void d2_paddlist_intern( d2_dlist *dlist );

D2_EXTERN void d2_growdlist_intern( d2_dlist *dlist );

D2_EXTERN d2_dlist_block * d2_alloc_dlistblock_intern( const d2_device *handle, d2_u32 size );

D2_EXTERN void d2_free_dlistblock_intern( const d2_device *handle, d2_dlist_block *data );

D2_EXTERN d2_s32 d2_initdlist_intern( d2_device *handle, d2_dlist *dlist, d2_u32 initialsize );

D2_EXTERN void d2_deinitdlist_intern(const d2_dlist *dlist);

D2_EXTERN void *d2_preparedlist_read_intern( const d2_device *handle, d2_dlist *dlist, d2_s32 reset );

D2_EXTERN void d2_nextdlistblock_intern( d2_dlist *dlist );

D2_EXTERN d2_u32 d2_executedlist_intern( const d2_device *handle, d2_dlist_entry *block );

D2_EXTERN void d2_copydlist_vidmem_intern( const d2_dlist *dlist );

D2_EXTERN void d2_mapdlist_vidmem_intern( const d2_dlist *dlist );

D2_EXTERN void d2_cacheflushdlist_intern( const d2_dlist *dlist );

D2_EXTERN d2_s32 d2_insertwaitpipedlist_intern( d2_device *handle );

D2_EXTERN d2_s32 d2_insertwaitfulldlist_intern( d2_device *handle );

D2_EXTERN void d2_resetdlist_intern( d2_dlist *dlist );

D2_EXTERN void d2_scratch2dlist_intern( d2_device *handle );

D2_EXTERN void d2_clear_dlistlist_intern( const d2_device *handle, d2_dlist *dlist );

D2_EXTERN d2_s32* d2_add_dlistlist_intern( const d2_device *handle, d2_dlist *dlist, const void *dlistaddress );


/*--------------------------------------------------------------------------- */

/* call only when usedslots>0
 * assumes full dlist_entries. padded entries supported at end of list only */
static D2_INLINE d2_dlist_entry* d2_getdlist_intern( d2_dlist *dlist ) /* PRQA S 1527 */ /* $Misra: #PERF_INLINE_FUNC $*/ /* PRQA S 3219, 3450, 3480 */ /* $Misra: #UTIL_INLINE_FUNC $*/
{
   d2_dlist_entry *current;

   current = dlist->position;
   dlist->position++;
   dlist->blocksize--;

   if (dlist->blocksize < 1)
   {
      /* this block ran empty, setup for next read */
      d2_dlist_block *n = dlist->currentblock->next;
      if(NULL != n)
      {
         /* switch to next block */
         dlist->position     = n->block;
         dlist->blocksize    = n->quantity;
         dlist->currentblock = n;
      }
   }

   return current;
}

/* insert a jump into the dlist to target, return patch address */
static D2_INLINE d2_s32* d2_insertdlistjump_intern( d2_dlist *dlist, const void *target ) /* PRQA S 1527 */ /* $Misra: #PERF_INLINE_FUNC $*/ /* PRQA S 3219, 3450, 3480 */ /* $Misra: #UTIL_INLINE_FUNC $*/
{
   d2_s32 *patch;

   /* this block ran empty. insert dlist-jump and mark end */
   dlist->position->address.mask = D2_DLISTSTART | 0x80808000u;
   dlist->position->value[0] = (d2_s32)target;

   /* remember value offset for later patching */
   patch = & dlist->position->value[0];
   dlist->currentblock->jump = patch;

   /* skip jump */
   dlist->position = (d2_dlist_entry*) (((d2_s32 *)dlist->position) + 2);
   dlist->tagindex = 0;

   return patch;
}


/* store a end of dlist special word at current position */
static D2_INLINE void d2_insertdlistspecial_intern( d2_dlist *dlist, d2_u32 argument ) /* PRQA S 1527 */ /* $Misra: #PERF_INLINE_FUNC $*/ /* PRQA S 3219, 3450, 3480 */ /* $Misra: #UTIL_INLINE_FUNC $*/
{
   d2_u32 tagIndex = dlist->tagindex;
   d2_dlist_entry *pos = dlist->position;

   switch(tagIndex)
   {
      case 0:
         /* list already closed -> the address word is not needed */
         pos->address.mask = 0xffu | (argument << 8);
         dlist->position = (d2_dlist_entry *) ((d2_s32*)pos + 1);
         break;

      case 1:
         /* already one entry in list */
         pos->address.mask &= 0x000000ffu;
         pos->address.mask |= 0x80808000u;
         pos->value[tagIndex] = (d2_s32) (0xffu | (argument << 8));
         dlist->position = (d2_dlist_entry *) ((d2_s32*)pos + 3);
         break;

      case 2:
         /* already two entries in list */
         pos->address.mask &= 0x0000ffffu;
         pos->address.mask |= 0x80800000u;
         pos->value[tagIndex] = (d2_s32) (0xffu | (argument << 8));
         dlist->position = (d2_dlist_entry *) ((d2_s32*)pos + 4);
         break;

      case 3:
         /* already three entries in list */
         pos->address.mask &= 0x00ffffffu;
         pos->address.mask |= 0x80000000u;
         pos->value[tagIndex] = (d2_s32) (0xffu | (argument << 8));
         dlist->position++;
         break;

      default:
         break;
   }

   /* fix tag */
   dlist->tagindex = 0;
   dlist->blocksize--;

   /* make sure not to go beyond end of block in case the list is not finished!! */
   if(0 == (argument & 1))
   {
      if(dlist->blocksize <= 1)
      {
         d2_nextdlistblock_intern(dlist);
      }
   }
}
/*--------------------------------------------------------------------------- */
#endif

