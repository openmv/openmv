/*--------------------------------------------------------------------------
 * Project: D/AVE
 * File:    dave_dlist.c (%version: 32 %)
 *          created Thu Feb 17 21:45:11 2005 by hh04027
 *
 * Description:
 *  %date_modified: Thu Mar 08 15:05:30 2007 %  (%derived_by:  hh74026 %)
 *
 * Changes:
 * 2005-12-22 CSe  - fix nextdlistblock(): jump was inserted after end of block
 *                 - added d2_copydlist_vidmem_intern()
 *                 - renamed block->vram to block->vidmem for consistency
 * 2006-01-05 CSe  - free vidmem when dlist is free'd
 * 2006-02-28 CSe  - adapted to changes in memory allocation of dlist mem
 *                 - added d2_cacheflushdlist_intern()
 * 2006-05-10 CSe  - changes for systems with statically mapped vidmem
 * 2006-10-30 CSe  - removed counting of dlist used slots
 * 2006-11-07 CSe  - changes for new 'd2_df_low_localmem' mode
 * 2006-11-30 CSe  - fixed insertion of dlist special commands
 * 2007-03-08 CSe  - optimized dlist copying and flushing
 * 2007-05-23 MGe  - insert null pointer checks to d2_init_dlist_intern, d2_growdlist_inter,
 *                   d2_nextdlistblock_intern
 * 2007-10-23 CSe  - added dynamic shrinking of display list
 * 2008-01-14 ASc  - changed comments from C++ to C, removed tabs
 * 2008-08-21 MRe  - fixed dynamic shrinking for low_localmem
 * 2010-07-08 MRe  - fixed support for register caching defined by D2_USEREGCACHE 
 *                    new device flag d2_df_no_registercaching
 * 2011-02-07 SSt  - ensure a flush for all dlist ends (needed for multithreading)
 * 2011-02-28 SSt  - fixed while loop
 * 2011-03-10 MRe  - invalidate register chaches with every dlist reset
 * 2011-03-11 MRe  - fixed crash under WIN32 at freemem
 * 2012-07-18 MRe  - added dlist functions d2_executedlist, d2_adddlist
 * 2012-09-25 BSp  - MISRA cleanup
 * 2012-10-16 MRe  - fixed freemem for d2_df_no_dlist
 * 2017-07-27 HFu  - clearly commented and renamed d2_insertwait...dlist_intern functions
 * 2020-02-21 MRe  - removed code for big endian
 *-------------------------------------------------------------------------- */

/*--------------------------------------------------------------------------
 *
 * Title: Dlist Functions
 *
 *
 *-------------------------------------------------------------------------- */


#include "dave_driver.h"
#include "dave_intern.h"
#include "dave_dlist.h"
#include "dave_memory.h"


#define D2_EXECUTE_DLIST_NEEDWAIT defined

/*--------------------------------------------------------------------------*/

static d2_s32 d2_dlist2dlist_intern( d2_device *handle, d2_dlist *dlist, void *address, d2_s32 size );

/*--------------------------------------------------------------------------
 * create a display list block (displaylist memory can be fetched from a
 * different heap, see dave_memory.h for detials)
 * size specifies the number of dlist entries (4 register slots per entry)
 * */
d2_dlist_block * d2_alloc_dlistblock_intern( const d2_device *handle, d2_u32 size )
{
   d2_dlist_block *dlb;
   d1_device      *id = D2_DEV(handle)->hwid;

   /* get controlling block structure */
   dlb = (d2_dlist_block*) d2_getmem_p( sizeof(d2_dlist_block) );

   if(NULL == dlb) 
   {
      return NULL;
   }

   /* fill block */
   dlb->quantity = size;
   dlb->jump   = NULL;

   /* get block data */
   if(0 == (D2_DEV(handle)->flags & d2_df_no_dlist)) 
   {
      dlb->block = (d2_dlist_entry*) d2_getmem_d( handle, sizeof(d2_dlist_entry) * size );
   } 
   else
   {
      dlb->block = (d2_dlist_entry*) d2_getmem_p( sizeof(d2_dlist_entry) * size );
   }

   if(NULL == dlb->block)
   {
      /* failed to get display list memory */
      d2_freemem_p( dlb );
      return NULL;
   }

   dlb->vidmem = NULL;
   if(0 == (D2_DEV(handle)->flags & d2_df_no_dlist) ) 
   {
      if(0 != (D2_DEV(handle)->hwmemarchitecture & d1_ma_mapped))
      {
         dlb->vidmem = d1_maptovidmem(id, dlb->block);
      }
      else if (0 != (D2_DEV(handle)->hwmemarchitecture & d1_ma_separated))
      {
         dlb->vidmem = d1_allocvidmem( id, d1_mem_dlist, sizeof(d2_dlist_entry) * dlb->quantity );
      }
      else
      {
         /* empty else block to satisfy MISRA rule 14.10/2004 */
      }
   }

   dlb->next = NULL;

   return dlb;
}

/*--------------------------------------------------------------------------
 * free display list block.
 * free the specified display list block and all childs in its chain
 * */
void d2_free_dlistblock_intern( const d2_device *handle, d2_dlist_block *data )
{
   d2_dlist_block *n;
   d1_device      *id = D2_DEV(handle)->hwid;

   /* free chain */
   while(NULL != data)
   {
      n = data->next;
      if(0 == (D2_DEV(handle)->flags & d2_df_no_dlist)) 
      {
          d2_freemem_d( handle, data->block );
      }
      else
      {
          d2_freemem_p( data->block );
      }
      if( (NULL != data->vidmem) && (0 != ((D2_DEV(handle)->hwmemarchitecture) & d1_ma_separated)) )
      {
         d1_freevidmem( id, d1_mem_dlist, data->vidmem );
      }
      d2_freemem_p( data );
      data = n;
   }
}

/*--------------------------------------------------------------------------
 * grow display list.
 * allocate a new display block and chain it into the list. new block becomes
 * current block automatically.
 * */
void d2_growdlist_intern( d2_dlist *dlist )
{
   d2_dlist_block *n;

   n = dlist->currentblock->next;
   if(NULL == n)
   {
      /* need a new block */
      n = d2_alloc_dlistblock_intern( dlist->device, dlist->stepsize );
      if(NULL == n)
      {
         D2_DEV(dlist->device)->delayed_errorcode = D2_NOT_ENOUGH_DLISTBLOCKS;
         return;
      }
      /* chain blocks */
      dlist->currentblock->next = n;
   }

   /* reset patch address */
   n->jump = NULL;

   /* switch to next block */
   dlist->position     = n->block;
   dlist->blocksize    = n->quantity;
   dlist->currentblock = n;

   /* tagindex is reset outside */
}

/*--------------------------------------------------------------------------
 * init display list.
 * prepare a display list structure for usage. creates an initial block.
 * returns boolean success.
 * */
d2_s32 d2_initdlist_intern( d2_device *handle, d2_dlist *dlist, d2_u32 initialsize )
{
   void *current_dlist_start;
   dlist->device = handle;
   dlist->vidmem_blocks = NULL;

   if(0 != D2_DEV(handle)->lowlocalmem_mode)
   {
      d2_lowlocalmem_mode* llm_mode = D2_DEV(handle)->lowlocalmem_mode;
      dlist->firstblock = llm_mode->dlist_buffer;

      dlist->vidmem_blocks = d2_getmem_p(sizeof(d2_dlist_vidmem_blocks));

      if(NULL == dlist->vidmem_blocks) 
      {
         return 0;
      }

      {
         d2_dlist_vidmem_blocks *vmem_blocks = dlist->vidmem_blocks;
         d2_u32 i;
         vmem_blocks->blocks = d2_getmem_p(llm_mode->vidmemblocks * sizeof(void*));
         if(NULL == vmem_blocks->blocks)
         {
            return 0;
         }
         vmem_blocks->num_blocks = llm_mode->vidmemblocks;
         vmem_blocks->block_size = llm_mode->vidmemblocksizefactor * D2_DEV(handle)->dlistblocksize;
         vmem_blocks->num_slices = llm_mode->vidmemblocksizefactor;

         for (i=0; i<vmem_blocks->num_blocks; ++i)
         {
            vmem_blocks->blocks[i] = NULL;
         }
         vmem_blocks->currentblock = vmem_blocks->blocks;
         /* allocate first block in vidmem */
         vmem_blocks->blocks[0] = d1_allocvidmem(D2_DEV(handle)->hwid, d1_mem_dlist, sizeof(d2_dlist_entry) * vmem_blocks->block_size);
         if(NULL == vmem_blocks->blocks[0]) 
         {
            return 0;
         }
         vmem_blocks->currentaddress = vmem_blocks->blocks[0];
         vmem_blocks->blocksleft = vmem_blocks->num_blocks;
         vmem_blocks->slicesleft = vmem_blocks->num_slices;
      }
   }
   else
   {
      dlist->firstblock = d2_alloc_dlistblock_intern( handle, initialsize );
   }
   dlist->resumeblock = dlist->firstblock;

   if(NULL == dlist->firstblock) 
   {
      return 0;
   }

   dlist->currentblock = dlist->firstblock;
   dlist->position = dlist->firstblock->block;
   dlist->blocksize = dlist->firstblock->quantity;
   dlist->tagindex = 0;
   dlist->shrinkcount = 0;
   dlist->count = 0;

   dlist->dlist_addresses_max = DLIST_ADRESSES_NUMBER;
   dlist->dlist_addresses     = d2_getmem_p(DLIST_ADRESSES_NUMBER*sizeof(d2_s32*));
   if(NULL == dlist->dlist_addresses) 
   {
      return 0;
   }

   if(NULL != dlist->vidmem_blocks)
   {
      /* low localmem mode: vidmem is already filled */
      current_dlist_start = dlist->vidmem_blocks->blocks[0];
   }
   else if( (0 != ((D2_DEV(handle)->hwmemarchitecture) & (d1_ma_separated | d1_ma_mapped))) && (NULL != dlist->firstblock->vidmem) )
   {
      current_dlist_start = dlist->firstblock->vidmem;
   }
   else
   {
      current_dlist_start = dlist->firstblock->block;
   }
   /* reset list of dlist start addresses */
   d2_clear_dlistlist_intern(handle, dlist);
   (void)d2_add_dlistlist_intern(handle, dlist, current_dlist_start);

   return 1;
}


/*--------------------------------------------------------------------------
 * de-initialize display list.
 * free all allocated memory areas
 * */
void d2_deinitdlist_intern(const d2_dlist *dlist)
{
   if(NULL != dlist->vidmem_blocks)
   {
      d2_u32 i;
      d1_device *id = D2_DEV(dlist->device)->hwid;
      for (i=0; i<dlist->vidmem_blocks->num_blocks; ++i)
      {
         if(NULL != dlist->vidmem_blocks->blocks[i])
         {
            d1_freevidmem(id, d1_mem_dlist, dlist->vidmem_blocks->blocks[i]);
         }
      }
      d2_freemem_p(dlist->vidmem_blocks->blocks);
      d2_freemem_p(dlist->vidmem_blocks);
   }
   else
   {
      d2_free_dlistblock_intern( dlist->device, dlist->firstblock );
   }
   if(NULL != dlist->dlist_addresses)
   {
      d2_freemem_p(dlist->dlist_addresses);
   }
   return;
}


/*--------------------------------------------------------------------------
 * go to next display list block.
 * used when the blocksize of the current dlist block is <= 1
 * the last entry will be spared for a jump which might be necessary
 * */
void d2_nextdlistblock_intern( d2_dlist *dlist )
{
   d2_dlist_vidmem_blocks *vidmemBlocks = dlist->vidmem_blocks;

   if(NULL == vidmemBlocks )
   { /* normal mode -> always insert jump and start next block */
      d2_s32 *patch = d2_insertdlistjump_intern( dlist, NULL );
      d2_insertdlistspecial_intern( dlist, 3 );  /* terminate list correctly */
      /* grow to prepare for next write */
      d2_growdlist_intern( dlist );
      /* check if d2_growdlist succeded */
      if ( D2_DEV(dlist->device)->delayed_errorcode != D2_OK )
      {
         /* no more memory -> terminate current list */
         dlist->position = (d2_dlist_entry *) (((d2_s32*)dlist->position) - 3);
         d2_insertdlistspecial_intern( dlist, 3 );
         return;
      }
      /* insert next block address (fix jump) */
      *patch = (d2_s32) dlist->currentblock->block;
   }
   else if ((dlist->blocksize < 1) || (vidmemBlocks->slicesleft == 1))
   { /* 'low localmem' mode: the spare block for the jump is only needed in last slice */
      d1_device *id = D2_DEV(dlist->device)->hwid;

      if (vidmemBlocks->slicesleft > 1)
      { /* current slice is now completely filled: make sure to pad possible remaining words */
         d2_dlist_block *blk = dlist->currentblock;
         d2_s32* end = (d2_s32*)(blk->block + blk->quantity);
         d2_s32* pos = (d2_s32*)(dlist->position);
         d2_s8 *nextaddress = vidmemBlocks->currentaddress + (sizeof(d2_dlist_entry) * blk->quantity);
         if(NULL != blk->jump)
         {
            *(blk->jump) = (d2_s32)nextaddress;
            blk->jump = 0;
         }
         while (pos < end)
         {
            *pos= (d2_s32)0x80808080u;
            pos++;
         }
         (void)d1_copytovidmem( id, vidmemBlocks->currentaddress, blk->block, sizeof(d2_dlist_entry) * blk->quantity, d1_cf_async );
         vidmemBlocks->currentaddress = nextaddress;
         vidmemBlocks->slicesleft--;
      }
      else if (vidmemBlocks->blocksleft > 1)
      { /* last slice -> allocate new vidmem block (only if blocks left), insert jump */
         d2_dlist_block *blk = dlist->currentblock;
         void **nextblock = vidmemBlocks->currentblock + 1;
         vidmemBlocks->blocksleft--;

         /* allocate next block if necessary */
         if (*nextblock == NULL)
         {
            *nextblock = d1_allocvidmem(id, d1_mem_dlist, sizeof(d2_dlist_entry) * vidmemBlocks->block_size);
            if(NULL == *nextblock)
            {
               D2_DEV(dlist->device)->delayed_errorcode = D2_NOMEMORY ; /* can't get videomemory */
            }
         }

         if(NULL != *nextblock) /* don't insert a jump to null into the list ( happens if allocation of vidmem failed) */
         {
            if(NULL != blk->jump)
            {
               *(blk->jump) = (d2_s32)(*nextblock);
               blk->jump = 0;
            }
            else
            {
               (void)d2_insertdlistjump_intern( dlist, *nextblock );
            }
         }
         d2_insertdlistspecial_intern( dlist, 3 );  /* terminate list correctly */

         /* copy the current dlist block to position inside current vidmem block */
         (void)d1_copytovidmem( id, vidmemBlocks->currentaddress, dlist->currentblock->block, sizeof(d2_dlist_entry) * dlist->currentblock->quantity, d1_cf_async );

         if(NULL != *nextblock)
         {
            vidmemBlocks->currentblock   = nextblock;
            vidmemBlocks->currentaddress = *nextblock;
            vidmemBlocks->slicesleft     = vidmemBlocks->num_slices;
         }
      } 
      else if ( (1 == vidmemBlocks->blocksleft) && (1 == vidmemBlocks->slicesleft) ) 
      {
         D2_DEV(dlist->device)->delayed_errorcode = D2_NOT_ENOUGH_DLISTBLOCKS;      
      }
      else
      {
         /* empty else block to satisfy MISRA rule 14.10/2004 */
      }
    
      d2_growdlist_intern( dlist );  /* go to next dlist buffer in ring */

      /* check if d2_growdlist succeded */
      if ( D2_DEV(dlist->device)->delayed_errorcode != D2_OK )
      {
         return;
      }
   }
   else
   {
      /* empty else block to satisfy MISRA rule 14.10/2004 */
   }
}


/*--------------------------------------------------------------------------
 * pad display list.
 * pad remaining entries of current addressword with 0x80 nop codes and fix
 * address to point onto the next free full entry.
 * */
void d2_paddlist_intern( d2_dlist *dlist )
{
   d2_u32 tagIndex = dlist->tagindex;
   d2_dlist_entry *pos = dlist->position;

   switch (tagIndex)
   {
      case 0:
         /* list already closed */
         return;

      case 1:
         pos->address.mask &= 0x000000ffu;
         pos->address.mask |= 0x80808000u;
         dlist->position = (d2_dlist_entry *) ((d2_s32*)pos + 2);
         break;

      case 2:
         pos->address.mask &= 0x0000ffffu;
         pos->address.mask |= 0x80800000u;
         dlist->position = (d2_dlist_entry *) ((d2_s32*)pos + 3);
         break;

      case 3:
         pos->address.mask &= 0x00ffffffu;
         pos->address.mask |= 0x80000000u;
         dlist->position = (d2_dlist_entry *) ((d2_s32*)pos + 4);
         break;
         
      default:
         break;
   }

   /* fix tag */
   dlist->tagindex = 0;
   dlist->blocksize--;
}

/*-------------------------------------------------------------------------- */
void d2_resetdlist_intern( d2_dlist *dlist )
{
#ifdef D2_USEREGCACHE
   d2_cacheddata *cachedData = & ((d2_devicedata *) dlist->device)->cache;
   d2_s32 regIdx;
   if(0 == (((d2_devicedata *) dlist->device)->flags & d2_df_no_registercaching))
   {
      for (regIdx = 0; regIdx < D2_QUANTITY; regIdx++)
      {
         cachedData->valid[regIdx] = 0;
      }
   }
#endif

   dlist->currentblock = dlist->firstblock;
   dlist->resumeblock  = dlist->firstblock;

   if(NULL != dlist->vidmem_blocks)
   {
      dlist->vidmem_blocks->currentblock   = dlist->vidmem_blocks->blocks;
      dlist->vidmem_blocks->currentaddress = dlist->vidmem_blocks->blocks[0];
      dlist->vidmem_blocks->blocksleft     = dlist->vidmem_blocks->num_blocks;
      dlist->vidmem_blocks->slicesleft     = dlist->vidmem_blocks->num_slices;
   }

   dlist->position  = dlist->firstblock->block;
   dlist->blocksize = dlist->firstblock->quantity;
   dlist->tagindex  = 0;
   dlist->count     = 0;

}

/*--------------------------------------------------------------------------
 * prepare display list for reading.
 * terminates the list with an FF command, applies padding and shrinks dlist
 * if possible
 * */
void* d2_preparedlist_read_intern( const d2_device *handle, d2_dlist *dlist, d2_s32 reset )
{
   void *next_dlist_start = 0;

   /* copy scratch buffer to dlist (dlist cache flush) */
   d2_scratch2dlist_intern( dlist->device );

   /* mark end of scene in dlist */
   d2_insertdlistspecial_intern( dlist, 3 );

   if (D2_DEV(handle)->lowlocalmem_mode == NULL)
   {
      /* update shrinkcount */
      if(NULL != dlist->currentblock->next)
      {
         ++dlist->shrinkcount;  /* list is too long */
      }
      else
      {
         dlist->shrinkcount = 0; /* at end of list */
      }

      if (dlist->shrinkcount == D2_DLISTSHRINKDELAY)
      {
         /* go to penultimate element in list */
         d2_dlist_block *blk = dlist->currentblock;
         while(NULL != blk->next->next)
         {
            blk = blk->next;
         }

         /* free the last element */
         d2_free_dlistblock_intern(dlist->device, blk->next);
         blk->next = NULL;
         dlist->shrinkcount = 0;
      }
   }

   /* copy resp. flush dlist to vidmem */
   if(NULL != dlist->vidmem_blocks)
   {
      /* low_local_memory_mode */
      if(0 != reset)
      {
         (void)d1_copytovidmem( d2_level1interface(dlist->device), 
                                dlist->vidmem_blocks->currentaddress,
                                dlist->currentblock->block, 
                                (d2_u32)(sizeof(d2_dlist_entry) * (dlist->currentblock->quantity - dlist->blocksize)), 
                                0
                                );
      } 
      /* if !reset then d1_copytovidmem is done below by d2_nextdlistblock_intern */
   }
   else if(0 != (D2_DEV(handle)->hwmemarchitecture & d1_ma_separated))
   {
      d2_copydlist_vidmem_intern( dlist );
   }
   else if (0 != (D2_DEV(handle)->hwmemarchitecture & d1_ma_mapped))
   {
      d2_mapdlist_vidmem_intern( dlist );
   }
   else
   {
      d2_cacheflushdlist_intern( dlist );
   }

   if(0 != reset)
   {
      /* reset list */
      d2_resetdlist_intern( dlist );
      dlist->busy = 0;
   }
   else
   { /* goto next dlist block and get the current_dlist_start */

      /* mark dlist block to be full, so d2_nextdlistblock_intern will move to next block in any case */
      dlist->blocksize = 0;

      /* resume adding commands in a new block */
      d2_nextdlistblock_intern( dlist );
      dlist->resumeblock = dlist->currentblock;

      if(NULL != dlist->vidmem_blocks)
      {
         /* low_local_memory_mode */
         next_dlist_start =  dlist->vidmem_blocks->currentaddress;
      }
      else if( (0 != ((D2_DEV(handle)->hwmemarchitecture) & (d1_ma_separated | d1_ma_mapped))) && (NULL != dlist->currentblock->vidmem) )
      {
         next_dlist_start = dlist->currentblock->vidmem;
      }
      else
      {
         next_dlist_start = dlist->position;
      }
   }

   return next_dlist_start;
}

/*--------------------------------------------------------------------------
 * simulated execution of a single display list page. optimized for speed
 * returns end of list argument.
 * note list MUST be terminated correctly
 * */
d2_u32 d2_executedlist_intern( const d2_device *handle, d2_dlist_entry *block )
{
   d2_u32 adrmask;
   d2_u32 argument = 0;
   d1_device *id = D2_DEV(handle)->hwid;
   d2_u32 a1,a2,a3,a4;

   for (;;)
   {
      d2_s32 bEOL = 0;

      adrmask = block->address.mask;
      if(0 != (adrmask & 0x80808080u))
      {
         /* contains special indices */
         if(0x80808080u == adrmask)
         {
            /* completely empty -> skip */
            block = (d2_dlist_entry*) ((d2_s32*)block + 1);
         }
         else if(0 != (adrmask & 0x00008000u))
         {
            /* index1 only */
            if ((adrmask & 0xffu) == D2_DLISTSTART)
            {
               /* catch special case of display list jump */
               return argument;
            } 
            else 
            {
               /* normal rewrite */
               a1 = adrmask & 0xffu;
               d2hw_set(id, a1, block->value[0]);
#ifdef D2_EXECUTE_DLIST_NEEDWAIT
               if(a1 == D2_ORIGIN)
               {
                  d2hw_wait(id);
               }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */
            }
            block = (d2_dlist_entry*) ((d2_s32*)block + 2);
         }
         else if (0 != (adrmask & 0x00800000u))
         {
            /* index1&2 only */
            if( 0xffu == (adrmask & 0xffu) )
            {
               /* end of list word */
               argument = (adrmask >> 8) & 0xffu;
               if(2 == argument)
               {
                  /* just a flush. keep on reading */
                  block = (d2_dlist_entry*) ((d2_s32*)block + 1);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
                  d2hw_wait(id);
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

                  bEOL = 1;
               } 
               else 
               {
                  /* terminate */
                  return argument;
               }
            }

            if(0 == bEOL)
            {
               a1 = adrmask & 0xffu;
               a2 = (adrmask >> 8) & 0xffu;

               d2hw_set(id, a1, block->value[0]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
               if(D2_ORIGIN == a1)
               {
                  d2hw_wait(id);
               }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

               d2hw_set(id, a2, block->value[1]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
               if(D2_ORIGIN == a2)
               {
                  d2hw_wait(id);
               }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

               block = (d2_dlist_entry*) ((d2_s32*)block + 3);
            }
         }
         else if (0 != (adrmask & 0x80000000u))
         {
            /* index1,2,3 only */
            a1 = adrmask & 0xffu;
            a2 = (adrmask >> 8) & 0xffu;
            a3 = (adrmask >> 16) & 0xffu;

            d2hw_set(id, a1, block->value[0]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
            if(D2_ORIGIN == a1)
            {
               d2hw_wait(id);
            }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

            d2hw_set(id, a2, block->value[1]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
            if(D2_ORIGIN == a2)
            {
               d2hw_wait(id);
            }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

            d2hw_set(id, a3, block->value[2]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
            if(D2_ORIGIN == a3)
            {
               d2hw_wait(id);
            }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

            block = (d2_dlist_entry*) ((d2_s32*)block + 4);
         }
         else
         {
            /* end of list word */
            if((adrmask & 0xffu) == 0xffu)
            {
               /* get eol argument */
               argument = (adrmask >> 8) & 0xffu;
               if ((2 == argument) || (4 == argument))
               {
                  /* just a flush. keep on reading */
                  block = (d2_dlist_entry*) ((d2_s32*)block + 1);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
                  d2hw_wait(id);
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */
               } 
               else 
               {
                  /* terminate */
                  return argument;
               }
            } 
            else 
            {
               /* invalid case, no index used */
               block = (d2_dlist_entry*) ((d2_s32*)block + 1);
            }
         }
      } 
      else 
      {
         /* contains simple indices only */
         a1 = adrmask & 0xffu;
         a2 = (adrmask >> 8) & 0xffu;
         a3 = (adrmask >> 16) & 0xffu;
         a4 = adrmask >> 24;

         d2hw_set(id, a1, block->value[0]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
         if(D2_ORIGIN == a1)
         {
            d2hw_wait(id);
         }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

         d2hw_set(id, a2, block->value[1]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
         if(D2_ORIGIN == a2)
         {
            d2hw_wait(id);
         }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

         d2hw_set(id, a3, block->value[2]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
         if(D2_ORIGIN == a3)
         {
            d2hw_wait(id);
         }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

         d2hw_set(id, a4, block->value[3]);

#ifdef D2_EXECUTE_DLIST_NEEDWAIT
         if(D2_ORIGIN == a4)
         {
            d2hw_wait(id);
         }
#endif /* D2_EXECUTE_DLIST_NEEDWAIT */

         /* next entry */
         block++;
      }
   }

   /* never reached */
}

/*-------------------------------------------------------------------------
 * Add a dave flush command into current dlist and wait */
d2_s32 d2_insertwaitpipedlist_intern( d2_device *handle )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* copy scratch buffer to dlist (dlist cache flush) */
   d2_scratch2dlist_intern( handle );
   /* insert 0x04 wait into dlist to wait for pipeline flush */
   d2_insertdlistspecial_intern( &D2_DEV(handle)->selectedbuffer->baselist, 4 );

   D2_RETOK(handle);
}

/*-------------------------------------------------------------------------
 * Add a dave flush command into current dlist wait */
d2_s32 d2_insertwaitfulldlist_intern( d2_device *handle )
{
   D2_VALIDATE( handle, D2_INVALIDDEVICE); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* copy scratch buffer to dlist (dlist cache flush) */
   d2_scratch2dlist_intern( handle );
   /* insert 0x02 wait into dlist to wait for pipeline flush and cache write complete */
   d2_insertdlistspecial_intern( &D2_DEV(handle)->selectedbuffer->baselist, 2 );

   D2_RETOK(handle);
}

/*--------------------------------------------------------------------------
 * Copy scratch buffer into display list (dlist cache flush) */
void d2_scratch2dlist_intern( d2_device *handle )
{
   d2_devicedata *dev = D2_DEV(handle);
   d2_dlist *dlist    = &D2_DEV(handle)->selectedbuffer->baselist;
   d2_dlist_scratch_entry *scratchEntry = dev->dlscratch_base;
   d2_dlist_entry *pos;
   d2_u32 i, cnt = (d2_u32) (dev->dlscratch_pos - dev->dlscratch_base);
   d2_u32 tagIndex;


#ifdef D2_USEREGCACHE
   d2_cacheddata *cachedData = & ((d2_devicedata *) dlist->device)->cache;
   d2_s32  *cache_data  = cachedData->data;
   d2_s8   *cache_valid = cachedData->valid;

   if(0 == (D2_DEV(handle)->flags & d2_df_no_registercaching))
   {
      /* walk scratchbuffer
       */
      tagIndex = dlist->tagindex;
      pos = dlist->position;

      for(i=0; (i < cnt) && (0 ==  D2_DEV(dlist->device)->delayed_errorcode) ; i++)
      {
         d2_s32 bSkip = 0;
         d2_u32 regIdx = scratchEntry->reg;
         d2_s32 val = scratchEntry->value;

         /* next */
         scratchEntry++;

#ifdef _DEBUG
         if (regIdx > D2_QUANTITY)
         {
            bSkip = 1;
         }
#endif

#ifdef _DEBUG
         if(0 == bSkip)
#endif /* _DEBUG */
         {
            if(0 != d2_cacheableregs[regIdx])
            {
               if( (0 != cache_valid[regIdx]) && ( cache_data[regIdx] == val ) ) 
               {
                  bSkip = 1;
               }
               else
               {
                  cache_valid[regIdx] = 1;
                  cache_data[regIdx]  = val;
               }
            }

            if(0 == bSkip)
            {
               /* store in displaylist */
               pos->address.array[tagIndex] = (d2_u8) regIdx;
               pos->value[tagIndex] = val;

               /* next tagindex */
               tagIndex = (tagIndex + 1) & 3;

               dlist->count++;

               /* finish entry */
               if(0 == tagIndex)
               {
                  dlist->position++;
                  dlist->blocksize--;
                  if(dlist->blocksize <= 1)
                  {
                     dlist->tagindex = tagIndex;
                     d2_nextdlistblock_intern( dlist );
                  }
                  pos = dlist->position;
               }

            } /* 0 == bSkip */

         } /* 0 == bSkip */

      } /* for i=0..cnt */

      dlist->tagindex = tagIndex;
      cnt = 0;
   } 
   else 
#endif
   {
      dlist->count += cnt;

      /* copy entries until start of a new dlist entry block */
      tagIndex = dlist->tagindex;
      if(0 == D2_DEV( dlist->device)->delayed_errorcode)
      {
         if(0 != tagIndex)
         {
            pos = dlist->position;

            while( (cnt > 0) && (0 != tagIndex) )
            {
               /* read entry */
               d2_u32 regIdx = scratchEntry->reg;
               d2_s32 scrValue = scratchEntry->value;

               /* next entry */
               scratchEntry++;
               cnt--;

               /* store in displaylist */
               pos->address.array[tagIndex]   = (d2_u8) regIdx;
               pos->value[tagIndex] = scrValue;

               /* next tagindex */
               tagIndex = (tagIndex + 1u) & 3u;
            }

            /* finish entry */
            if(0 == tagIndex)
            {
               dlist->tagindex = tagIndex;
               dlist->position++;
               dlist->blocksize--;
               if(dlist->blocksize <= 1) 
               {
                  d2_nextdlistblock_intern( dlist );
               }
            }
         }

         if(0 == tagIndex)
         {
            /* walk rest of scratchbuffer in multiples of 4
             * tagindex must be 0 at this point (only exception is that cnt is already 0) */
            for(i=0; (i < (cnt / 4)) && (0 == D2_DEV(dlist->device)->delayed_errorcode); i++)
            {
               pos = dlist->position;

               pos->address.array[0] = (d2_u8) scratchEntry[0].reg;
               pos->address.array[1] = (d2_u8) scratchEntry[1].reg;
               pos->address.array[2] = (d2_u8) scratchEntry[2].reg;
               pos->address.array[3] = (d2_u8) scratchEntry[3].reg;

               pos->value[0] = scratchEntry[0].value;
               pos->value[1] = scratchEntry[1].value;
               pos->value[2] = scratchEntry[2].value;
               pos->value[3] = scratchEntry[3].value;

               /* finish entry */
               dlist->position++;
               dlist->blocksize--;
               if ( dlist->blocksize <= 1 ) 
               {
                  d2_nextdlistblock_intern( dlist );
               }
               /* next scratch entry */
               scratchEntry += 4;
            }


            /* copy remaining entries */
            cnt &= 3;
            for(i=0; (i < cnt) && (0 == D2_DEV( dlist->device)->delayed_errorcode); i++)
            {
               d2_u32 regIdx = scratchEntry->reg;
               d2_s32 scrValue = scratchEntry->value;

               /* next */
               scratchEntry++;

               tagIndex = dlist->tagindex;
               pos = dlist->position;

               /* store in displaylist */
               pos->address.array[tagIndex] = (d2_u8) regIdx;
               pos->value[tagIndex] = scrValue;

               /* next tagindex */
               tagIndex = (tagIndex + 1u) & 3u;
               dlist->tagindex = tagIndex;
            }
         }
      }
   }
   /* free scratch memory */
   dev->dlscratch_pos = dev->dlscratch_base;
   dev->dlscratch_cnt = D2_DLISTSCRATCH;
}

/*--------------------------------------------------------------------------
 * Append a prepared display list to the display list. */
static d2_s32 d2_dlist2dlist_intern( d2_device *handle, d2_dlist *dlist, void *address, d2_s32 size )
{
   d2_dlist_entry * pos         = dlist->position;
   d2_u32           tagIndex    = dlist->tagindex;
   d2_s32           cnt         = (size / (d2_s32)sizeof(d2_s32));
   d2_s32           cnt_rem     = (cnt % 5);
   d2_u32 *         dlist_word  = (d2_u32*)address;
   d2_s32           i;

   if(0 != tagIndex)
   {
      /* fill dlist entry with 0x80 indices */
      while(0 != tagIndex)
      {
         /* store in displaylist */
         pos->address.array[tagIndex]   = (d2_u8) 0x80u;
         pos->value[tagIndex] = (d2_s32)0x80808080u; /* pad with NULL command */
         /* next tagindex */
         tagIndex = (tagIndex + 1u) & 3u;
      }
      dlist->tagindex = 0;
      /* finish entry */
      dlist->position++;
      dlist->blocksize--;
      if(dlist->blocksize <= 1) 
      {
         d2_nextdlistblock_intern( dlist );
      }
   }

   /* copy dlist with size of dlist entries */
   while(cnt >= 5)
   {
      d2_u32 *dl = &dlist->position->address.mask;
      dl[0] = dlist_word[0];
      dl[1] = dlist_word[1];
      dl[2] = dlist_word[2];
      dl[3] = dlist_word[3];
      dl[4] = dlist_word[4];
      dl += 5;
      dlist_word += 5;

      /* finish entry */
      dlist->position++;
      dlist->blocksize--;
      if(dlist->blocksize <= 1)
      {
         d2_nextdlistblock_intern( dlist );
      }
      cnt -= 5;
   }

   /* pad dlist entry */
   if(0 != cnt_rem)
   {
      d2_u32 *dl = &dlist->position->address.mask;

      /* copy remaining words */
      cnt--;
      if(0 != cnt)
      {
         do
         {
            *dl = *dlist_word;
            dl++;
            dlist_word++;
            cnt--;
         }
         while(0 != cnt);
      }

      /* pad with 0x80808080 */
      for(cnt_rem = (5 - cnt_rem); cnt_rem > 0; cnt_rem--)
      {
         *dl = 0x80808080u;
         dl++;
      }
   }

   /* clear register cache */
#ifdef D2_USEREGCACHE
   for(i=0; i<D2_QUANTITY; i++)
   {
      D2_DEV(handle)->cache.data[i]  = 0;
      D2_DEV(handle)->cache.valid[i] = 0;
   }
#endif

   return 0;
}

/*--------------------------------------------------------------------------
 * Copy a display list to video memory. */
void d2_copydlist_vidmem_intern( const d2_dlist *dlist )
{
   d1_device *id = D2_DEV(dlist->device)->hwid;
   d2_dlist_block *blk  = dlist->resumeblock;
   d2_dlist_block *last = dlist->currentblock;

   if(0 != (D2_DEV(dlist->device)->flags & d2_df_no_dlist))
   {
      return;
   }

   /* copy all but the last block */
   while(blk != last)
   {
      /* replace jump address with new address inside vidmem */
      if((NULL != blk->next) && (NULL != blk->jump))
      {
         /* replace jump address */
         *(blk->jump) = (d2_s32)(blk->next->vidmem);
      }

      /* write block to video memory */
      (void)d1_copytovidmem( id, blk->vidmem, blk->block, sizeof(d2_dlist_entry) * blk->quantity, 0 );

      /* next block */
      blk = blk->next;
   }

   /* copy last block (not whole block needs to be copied) */
   (void)d1_copytovidmem( id, blk->vidmem, blk->block, sizeof(d2_dlist_entry) * (blk->quantity - dlist->blocksize), 0 );
}


/*--------------------------------------------------------------------------
 * Flush a display list out of data cache. */
void d2_cacheflushdlist_intern( const d2_dlist *dlist )
{
   d1_device *id = D2_DEV(dlist->device)->hwid;
   d2_dlist_block *blk  = dlist->resumeblock;
   d2_dlist_block *last = dlist->currentblock;

   if(0 != (D2_DEV(dlist->device)->flags & d2_df_no_dlist))
   {
      return;
   }

   /* flush all but the first block */
   while(blk != last)
   {
      /* replace jump address with new address inside vidmem */
      (void)d1_cacheblockflush(id, d1_mem_dlist, blk->block, sizeof(d2_dlist_entry) * blk->quantity );
      blk = blk->next;
   }

   /* flush last block (not whole block needs to be flushed) */
   (void)d1_cacheblockflush(id, d1_mem_dlist, blk->block, sizeof(d2_dlist_entry) * (blk->quantity - dlist->blocksize) );
}


/*--------------------------------------------------------------------------
 * Map a displaylist to vidmem. */
void d2_mapdlist_vidmem_intern( const d2_dlist *dlist )
{
   d1_device *id = D2_DEV(dlist->device)->hwid;
   d2_dlist_block *blk  = dlist->resumeblock;
   d2_dlist_block *last = dlist->currentblock;

   if(0 != (D2_DEV(dlist->device)->flags & d2_df_no_dlist))
   {
      return;
   }

   /* flush all but the first block */
   while(blk != last)
   {
      if( (NULL != blk->next) && (NULL != blk->jump) )
      {
         /* replace jump address (vidmem has been assigned by d2_alloc_dlistblock_intern) */
         *(blk->jump) = (d2_s32)(blk->next->vidmem);
      }

      /* replace jump address with new address inside vidmem */
      (void)d1_cacheblockflush(id, d1_mem_dlist, blk->vidmem, sizeof(d2_dlist_entry) * blk->quantity );
      blk = blk->next;
   }

   /* flush last block (not whole block needs to be flushed) */
   (void)d1_cacheblockflush(id, d1_mem_dlist, blk->vidmem, sizeof(d2_dlist_entry) * (blk->quantity - dlist->blocksize) );
}

/*--------------------------------------------------------------------------
 * Clear the list of dlist start addresses. */
void d2_clear_dlistlist_intern( const d2_device *handle, d2_dlist *dlist )
{
   d2_s32 i;
   d2_s32 *dlist_list = (d2_s32 *)dlist->dlist_addresses;
   (void)handle; /* PRQA S 3112 */ /*$Misra: #COMPILER_WARNING $*/

   for (i=0; i<dlist->dlist_addresses_max; i++)
   {
      dlist_list[i] = 0;
   }
   dlist->dlist_addresses_cur = 0;
}

/*--------------------------------------------------------------------------
 * Add an address to the list of dlist start addresses. */
d2_s32* d2_add_dlistlist_intern( const d2_device *handle, d2_dlist *dlist, const void *dlistaddress )
{
   d2_s32 i, pos;
   d2_s32 *dlist_list = (d2_s32 *)dlist->dlist_addresses;
   (void)handle; /* PRQA S 3112 */ /*$Misra: #COMPILER_WARNING $*/

   pos = dlist->dlist_addresses_cur;
   if(pos >= (dlist->dlist_addresses_max - 1)) /* leave last entry 0 */
   {
      /* grow dlist list */
      dlist->dlist_addresses_max = (d2_s16)(dlist->dlist_addresses_max+DLIST_ADRESSES_NUMBER);
      dlist->dlist_addresses      = d2_reallocmem_p( (d2_u32)dlist->dlist_addresses_max * sizeof(d2_s32*), dlist->dlist_addresses, 1 );
      dlist_list = (d2_s32 *)dlist->dlist_addresses;

      /* clear new part */
      for(i=pos; i<dlist->dlist_addresses_max; i++)
      {
         dlist_list[i] = 0;
      }
   }

   dlist_list[pos] = (d2_s32)dlistaddress;
   dlist->dlist_addresses_cur = (d2_s16) (pos + 1);

   return &dlist_list[pos];
}

/*------------------------------------------------------------------------
 * function: d2_executedlist
 *
 * Execute an already prepared display list. 
 * A display list (Dlist) can be created e.g. by <d2_dumprenderbuffer>.  
 *
 * Basically the address will be sent to D/AVE and will be executed immediately.
 * So the dlist address must be accessible by the GPU and 
 * it is in the application's responsibility to prepare the Dlist
 * correctly.
 * 
 * d2_executedlist must not be called while D/AVE is busy rendering
 * (see: <Render Buffers>). 
 *
 * Note: 
 * This has no influence on the render buffer mechanism. The
 * driver will manage cooperation of render buffers and Dlists.
 *
 * Note: 
 * In case d2_opendevice was called with d2_df_no_dlist address must 
 * be accessible by the CPU.
 *
 * parameters:
 *   handle  - device pointer (see: <d2_opendevice>)
 *   address - Address of the Dlist (must be accessible by the GPU)
 *   flags   - reserved (set to 0)
 *
 *
 * returns:
 *   errorcode (D2_OK if successful) see <Errorcodes> for details
 * */
d2_s32 d2_executedlist( d2_device *handle, const void *address, d2_u32 flags )
{
   d2_devicedata *dev = D2_DEV(handle);
   d2_dlist  dlist;
   d2_s32 *dlist_list;
   (void) flags; /* PRQA S 3112 */ /*$Misra: #COMPILER_WARNING $*/

   D2_VALIDATE( handle,  D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( address, D2_INVALIDBUFFER ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   /* put dlist_start to special address list */
   dlist.dlist_addresses     = dev->dlist_list_single;
   dlist.dlist_addresses_max = 2;
   dlist.dlist_addresses_cur = 1;

   dlist_list = (d2_s32 *)dlist.dlist_addresses;
   dlist_list[0] = (d2_s32)address;
   dlist_list[1] = 0;

   /* start dlist execution */
   d2hw_start(handle, &dlist, 1);

   D2_RETOK(handle);
}

/*------------------------------------------------------------------------
 * function: d2_adddlist
 *
 * Add an already prepared display list to the current render buffer.
 * A display list (Dlist) can be created e.g. by <d2_dumprenderbuffer>.  
 *
 * Depending on the flags, the Dlist will be added to the current
 * render buffer by copying its content or by adding 'call' operations
 * to the current render buffer.
 *
 * If a 'postprocess' rendermode is active the layers are merged before
 * the Dlist is added (see: <d2_selectrendermode>).
 *
 * Note: 
 * If d2_al_no_copy is selected, the GPU must be able to select the
 * specified address, in case if d2_al_copy the CPU must be able to 
 * access the Dlist.
 *
 * Note: 
 * For the mode d2_al_no_copy the low level driver must support the handling
 * of lists of Dlist start addresses (see: <d2_level1interface> and 
 * low level device <D1 Display list handling at ../../../driver_l1/files/doc/d1_dlistindirect-txt.html>).
 * 
 * If this handling is not supported then no Dlists can be added in this mode.
 *
 * parameters:
 *   handle  - device pointer (see: <d2_opendevice>)
 *   address - Address of the Dlist 
 *            (in case of d2_al_no_copy must be accessible by the GPU 
 *             in case of d2_al_no_copy must be accessible by CPU)
 *   size    - size of Dlist
 *   flags   - d2_al_copy, d2_al_no_copy
 *
 * returns:
 *   errorcode (D2_OK if successful) see <Errorcodes> for details
 *
 * flags:
 *   d2_al_default - default behavior d2_al_copy.
 *   d2_al_copy    - content will be copied.
 *   d2_al_no_copy - call to the Dlist will be added.
 *
 * see also:
 *   <d2_dumprenderbuffer>, <d2_getrenderbuffersize>, <d2_relocateframe>
 * */
d2_s32 d2_adddlist( d2_device *handle, void *address, d2_s32 size, d2_u32 flags )
{  
   d2_devicedata *dev   = D2_DEV(handle);
   d2_dlist      *dlist;
   void          *current_dlist_start;

   D2_VALIDATE( handle,  D2_INVALIDDEVICE ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( address, D2_INVALIDBUFFER ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/
   D2_VALIDATE( size,    D2_VALUETOOSMALL ); /* PRQA S 4130, 3112 */ /* $Misra: #DEBUG_MACRO $*/

   dlist = &dev->selectedbuffer->baselist;

   /* reset postprocessing mode if still active */
   if(d2_rm_postprocess == dev->rendermode)
   {
      (void)d2_selectrendermode( handle, d2_rm_solid );
   }

   /* merge in layers if they contain data */
   d2_layer2dlist_intern( handle );

   if(d2_al_copy == flags)
   {
      d2_scratch2dlist_intern(handle);

      /* append our dlist to the dlist*/
      (void)d2_dlist2dlist_intern(handle, dlist, address, size);
   }
   else
   {
      if( (0 != dev->dlist_indirect_supported) && (0 == (dev->flags & d2_df_no_dlist)) ) 
      {
         current_dlist_start = d2_preparedlist_read_intern( handle, dlist, 0 );

         /* append dlist_start to address list*/
         (void)d2_add_dlistlist_intern(handle, dlist, address);

         /* append current dlist position to address list */
         (void)d2_add_dlistlist_intern(handle, dlist, current_dlist_start);
      }
   }

   D2_RETOK(handle);
}
