
/*--------------------------------------------------------------------*/
/*--- Cache simulation                                    cg_sim.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Cachegrind, a Valgrind tool for cache
   profiling programs.

   Copyright (C) 2002-2013 Nicholas Nethercote
      njn@valgrind.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

/* Notes:
  - simulates a write-allocate cache
  - (block --> set) hash function uses simple bit selection
  - handling of references straddling two cache blocks:
      - counts as only one cache access (not two)
      - both blocks hit                  --> one hit
      - one block hits, the other misses --> one miss
      - both blocks miss                 --> one miss (not two)
*/

/* adding compression */ //gpekhime
static void print_value(UChar * addr, UChar size)
{
    Int i;
    for(i = size - 1; i >= 0; i--){
       VG_(printf)("%02x", addr[i]);
    }
       VG_(printf)("\n");

}

#include "compression.c"
static long long int gCount = 0;

typedef struct {
   Int          size;                   /* bytes */
   Int          assoc;
   Int          line_size;              /* bytes */
   Int          sets;
   Int          sets_min_1;
   Int          line_size_bits;
   Int          tag_shift;
   HChar        desc_line[128];
   UWord*       tags;
   /*gpekhime*/
   UWord*       compSizes;
} cache_t2;

/* By this point, the size/assoc/line_size has been checked. */
static void cachesim_initcache(cache_t config, cache_t2* c)
{
   Int i;

   c->size      = config.size;
   c->assoc     = config.assoc;
   c->line_size = config.line_size;

   c->sets           = (c->size / c->line_size) / c->assoc;
   c->sets_min_1     = c->sets - 1;
   c->line_size_bits = VG_(log2)(c->line_size);
   c->tag_shift      = c->line_size_bits + VG_(log2)(c->sets);

   if (c->assoc == 1) {
      VG_(sprintf)(c->desc_line, "%d B, %d B, direct-mapped", 
                                 c->size, c->line_size);
   } else {
      VG_(sprintf)(c->desc_line, "%d B, %d B, %d-way associative\n",
                                 c->size, c->line_size, c->assoc);
      VG_(printf)("%d B, %d B, %d-way associative\n",
                                 c->size, c->line_size, c->assoc);
   }

   c->tags = VG_(malloc)("cg.sim.ci.1",
                         sizeof(UWord) * c->sets * c->assoc * 2); //gpekhime

   for (i = 0; i < c->sets * c->assoc * 2; i++)
      c->tags[i] = 0;

   c->compSizes = VG_(malloc)("cg.sim.ci.2",
                         sizeof(UWord) * c->sets * c->assoc * 2); //gpekhime

   for (i = 0; i < c->sets * c->assoc * 2; i++)
      c->compSizes[i] = c->line_size;
}

/* This attribute forces GCC to inline the function, getting rid of a
 * lot of indirection around the cache_t2 pointer, if it is known to be
 * constant in the caller (the caller is inlined itself).
 * Without inlining of simulator functions, cachegrind can get 40% slower.
 */
__attribute__((always_inline))
static __inline__
Bool cachesim_setref_is_miss(cache_t2* c, UInt set_no, UWord tag, UChar compSize)
{
   int i, j;
   UWord *set;
   UWord *set4Sizes;

   set = &(c->tags[set_no * c->assoc * 2]);
   set4Sizes = &(c->compSizes[set_no * c->assoc * 2]);

   /* This loop is unrolled for just the first case, which is the most */
   /* common.  We can't unroll any further because it would screw up   */
   /* if we have a direct-mapped (1-way) cache.                        */
   if (tag == set[0]){
      // gpekhime
      // Updating size
      set4Sizes[0] = compSize;
      return False;
   }

   /* If the tag is one other than the MRU, move it into the MRU spot  */
   /* and shuffle the rest down.                                       */
   for (i = 1; i < c->assoc * 2; i++) {
      if (tag == set[i]) {
         for (j = i; j > 0; j--) {
            set[j] = set[j - 1];
            set4Sizes[j] = set4Sizes[j - 1];    
         }
         set[0] = tag;
         // gpekhime
         // Updating size
         set4Sizes[0] = compSize;
         return False;
      }
   }

   /* A miss;  install this tag as MRU, shuffle rest down. */
   for (j = 2 * c->assoc - 1; j > 0; j--) {
      set[j] = set[j - 1];
      // gpekhime
      // Updating size
      set4Sizes[j] = set4Sizes[j - 1];
   }
   set[0] = tag;
   // Updating size
   set4Sizes[0] = compSize;

   //resize after insert
   int valid = 0;
   int spaceUsed = 0;
   for (i = 0; i < c->assoc * 2; i++){
        if(set[i] > 0){
           valid++;
           spaceUsed += set4Sizes[i];
        }
      }
   //VG_(printf)(" Used = %d Total = %d \n", spaceUsed, c->line_size * c->assoc);
   if (spaceUsed <= c->line_size * c->assoc)
      return True;
   int compensate = spaceUsed - c->line_size * c->assoc;
   //VG_(printf)(" Init Compensate = %d \n", compensate);
   for (j = 2 * c->assoc - 1; j > 0; j--) {
      if (!set[j])
         continue;
      if (compensate <= 0)
        break;
      //invalidate blocks until we have enough size
      set[j] = 0;
      compensate -= set4Sizes[j];
      //VG_(printf)("Evicted entry in set %d way %d CompSize == %d \n",
      //          set_no, j, set4Sizes[j]);
      //VG_(printf)(" Dec Compensate = %d \n", compensate);        
   }


   return True;
}

__attribute__((always_inline))
static __inline__
Bool cachesim_ref_is_miss(cache_t2* c, Addr a, UChar size, UChar compSize)
{
   /* A memory block has the size of a cache line */
   UWord block1 =  a         >> c->line_size_bits;
   UWord block2 = (a+size-1) >> c->line_size_bits;
   UInt  set1   = block1 & c->sets_min_1;

   /* Tags used in real caches are minimal to save space.
    * As the last bits of the block number of addresses mapping
    * into one cache set are the same, real caches use as tag
    *   tag = block >> log2(#sets)
    * But using the memory block as more specific tag is fine,
    * and saves instructions.
    */
   UWord tag1   = block1;

   /* Access entirely within line. */
   if (block1 == block2)
      return cachesim_setref_is_miss(c, set1, tag1, compSize);

   /* Access straddles two lines. */
   else if (block1 + 1 == block2) {
      UInt  set2 = block2 & c->sets_min_1;
      UWord tag2 = block2;

      /* always do both, as state is updated as side effect */
      if (cachesim_setref_is_miss(c, set1, tag1, compSize)) {
         cachesim_setref_is_miss(c, set2, tag2, compSize);
         return True;
      }
      return cachesim_setref_is_miss(c, set2, tag2, compSize);
   }
   VG_(printf)("addr: %lx  size: %u  blocks: %ld %ld",
               a, size, block1, block2);
   VG_(tool_panic)("item straddles more than two cache sets");
   /* not reached */
   return True;
}


static cache_t2 LL;
static cache_t2 I1;
static cache_t2 D1;

static void cachesim_initcaches(cache_t I1c, cache_t D1c, cache_t LLc)
{
   cachesim_initcache(I1c, &I1);
   cachesim_initcache(D1c, &D1);
   cachesim_initcache(LLc, &LL);
}

__attribute__((always_inline))
static __inline__
void cachesim_I1_doref_Gen(Addr a, UChar size, ULong* m1, ULong *mL)
{
   if (cachesim_ref_is_miss(&I1, a, size, LL.line_size)) {
      (*m1)++;
      if (cachesim_ref_is_miss(&LL, a, size, LL.line_size))
         (*mL)++;
   }
}

// common special case IrNoX
__attribute__((always_inline))
static __inline__
void cachesim_I1_doref_NoX(Addr a, UChar size, ULong* m1, ULong *mL)
{
   UWord block  = a >> I1.line_size_bits;
   UInt  I1_set = block & I1.sets_min_1;

   // use block as tag
   if (cachesim_setref_is_miss(&I1, I1_set, block,  LL.line_size)) {
      UInt  LL_set = block & LL.sets_min_1;
      (*m1)++;
      // can use block as tag as L1I and LL cache line sizes are equal
      if (cachesim_setref_is_miss(&LL, LL_set, block,  LL.line_size))
         (*mL)++;
   }
}

__attribute__((always_inline))
static __inline__
void cachesim_D1_doref(Addr a, UChar size, ULong* m1, ULong *mL)
{
  //call compression
  unsigned compSize;
  // = GeneralCompress((char *)((a >> LL.line_size_bits)<<LL.line_size_bits), LL.line_size, 1);
  //if(a < 0x41001dc && a > 0x4000000){
     compSize =  GeneralCompress((char *)((a >> LL.line_size_bits)<<LL.line_size_bits), LL.line_size, 1);
    // VG_(printf)("addr: %lx  size: %u  compSize: %u \n",
    //           a, size, compSize);
    // if(compSize == 1) 
    // 	gCount++;
 // }
  if (cachesim_ref_is_miss(&D1, a, size, LL.line_size)) {
      (*m1)++;
        /* gpekhime */ 
       //call compression
      // compSize = GeneralCompress((char *)((a >> LL.line_size_bits)<<LL.line_size_bits), LL.line_size, 1);
      // if(a > 0x41001dc || a < 0x4000000)
      //   compSize = LL.line_size;
      // VG_(printf)("addr: %lx  size: %u  compSize: %u \n",
      //             a, size, compSize);   
 
      if (cachesim_ref_is_miss(&LL, a, size, compSize))
         (*mL)++;
   }
}

/* Check for special case IrNoX. Called at instrumentation time.
 *
 * Does this Ir only touch one cache line, and are L1I/LL cache
 * line sizes the same? This allows to get rid of a runtime check.
 *
 * Returning false is always fine, as this calls the generic case
 */
static Bool cachesim_is_IrNoX(Addr a, UChar size)
{
   UWord block1, block2;

   if (I1.line_size_bits != LL.line_size_bits) return False;
   block1 =  a         >> I1.line_size_bits;
   block2 = (a+size-1) >> I1.line_size_bits;
   if (block1 != block2) return False;

   return True;
}

/*--------------------------------------------------------------------*/
/*--- end                                                 cg_sim.c ---*/
/*--------------------------------------------------------------------*/

