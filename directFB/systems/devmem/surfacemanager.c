/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <fusion/shmalloc.h>

#include <directfb.h>
#include <directfb_util.h>

#include <core/core.h>

#include <core/gfxcard.h>
#include <core/surface.h>
#include <core/surface_buffer.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <gfx/convert.h>

#include "surfacemanager.h"

#if USE_RPC4DTV
#include <core/palette.h>
#include <pthread.h>
extern pthread_mutex_t g_dlfcn_mutex;
static Chunk* alloc_chunk( SurfaceManager *manager, Chunk *c, int offset, int length );

void dealloc_chunk(SurfaceManager *manager, Chunk* chunk);

D_DEBUG_DOMAIN( SurfRPC, "RPC SurfaceManager", "DirectFB RPC Surface M" );
#define D_PRINTF(x...) D_DEBUG_AT( SurfRPC, x )
#endif


D_DEBUG_DOMAIN( SurfMan, "SurfaceManager", "DirectFB Surface Manager" );


static Chunk *split_chunk ( SurfaceManager *manager,
                            Chunk          *chunk,
                            int             length );

static Chunk *free_chunk  ( SurfaceManager *manager,
                            Chunk          *chunk );

static Chunk *occupy_chunk( SurfaceManager        *manager,
                            Chunk                 *chunk,
                            CoreSurfaceAllocation *allocation,
                            int                    length,
                            int                    pitch );


DFBResult
dfb_surfacemanager_create( CoreDFB         *core,
                           unsigned int     length,
                           SurfaceManager **ret_manager )
{
     FusionSHMPoolShared *pool;
     SurfaceManager      *manager;
     Chunk               *chunk;

     D_DEBUG_AT( SurfMan, "%s( %p, %d )\n", __FUNCTION__, core, length );

     D_ASSERT( core != NULL );
     D_ASSERT( ret_manager != NULL );

#if USE_RPC4DTV
     if(DDI_GPU_AllocSurface == NULL)
     {
          assign_symbol_DDI_GPU_FUNCS(&g_dlfcn_mutex);
     }
#endif

     pool = dfb_core_shmpool( core );

     manager = SHCALLOC( pool, 1, sizeof(SurfaceManager) );
     if (!manager)
          return D_OOSHM();

     chunk = SHCALLOC( pool, 1, sizeof(Chunk) );
     if (!chunk) {
          D_OOSHM();
          SHFREE( pool, manager );
          return DFB_NOSHAREDMEMORY;
     }

     manager->shmpool = pool;
     manager->chunks  = chunk;
     manager->offset  = 0;
     manager->length  = length;
     manager->avail   = manager->length - manager->offset;

     D_MAGIC_SET( manager, SurfaceManager );

     chunk->offset    = manager->offset;
     chunk->length    = manager->avail;

     D_MAGIC_SET( chunk, Chunk );

     D_DEBUG_AT( SurfMan, "  -> %p\n", manager );

     *ret_manager = manager;

     return DFB_OK;
}

void
dfb_surfacemanager_destroy( SurfaceManager *manager )
{
     Chunk *chunk;
     void  *next;

     D_DEBUG_AT( SurfMan, "%s( %p )\n", __FUNCTION__, manager );

     D_MAGIC_ASSERT( manager, SurfaceManager );

     /* Deallocate all video chunks. */
     chunk = manager->chunks;
     while (chunk) {
          next = chunk->next;

          D_MAGIC_CLEAR( chunk );

          SHFREE( manager->shmpool, chunk );

          chunk = next;
     }

     D_MAGIC_CLEAR( manager );

     /* Deallocate manager struct. */
     SHFREE( manager->shmpool, manager );
}

/** public functions NOT locking the surfacemanger theirself,
    to be called between lock/unlock of surfacemanager **/

DFBResult dfb_surfacemanager_allocate( CoreDFB                *core,
                                       SurfaceManager         *manager,
                                       CoreSurfaceBuffer      *buffer,
                                       CoreSurfaceAllocation  *allocation,
                                       Chunk                 **ret_chunk )
#if USE_RPC4DTV
{
     Chunk *c;

     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( buffer->surface, CoreSurface );

     if (ret_chunk)
          D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     else
          D_ASSUME( allocation == NULL );

     D_DEBUG_AT( SurfMan, "%s( %p ) <- %dx%d %s\n", __FUNCTION__, buffer,
               buffer->surface->config.size.w, buffer->surface->config.size.h,
               dfb_pixelformat_name( buffer->surface->config.format ) );

     // check minimum size of surface
	 if(!(buffer->surface->config.caps & DSCAPS_VIDEOONLY))
	 {
		 //if((buffer->surface->config.size.w * buffer->surface->config.size.h) < 256)  //18*18=324
		 if((buffer->surface->config.size.w * buffer->surface->config.size.h) < 900)  //30*30=900 rule of thumb for bcm3549 platform. S/W blit is faster than H/W below.
		 {
			 D_PRINTF("The surface size is too small to be allocated in video memory.\n");
			 return DFB_NOVIDEOMEMORY;
		 }

		 if(((buffer->surface->config.size.w * buffer->surface->config.size.h) < 2500) && (buffer->surface->config.size.w < 7))  // rule of thumb for bcm3549 platform. S/W blit is faster than H/W below.
		 {
			 D_PRINTF("The surface size and width is to small to be allocated in video memory.\n");
			 return DFB_NOVIDEOMEMORY;
		 }
	 }

#if 0 // 20081016_yducky -->
     // can be skipped?
     if (!ret_chunk)
          return DFB_NOVIDEOMEMORY;
#endif // 20081016_yducky <--

     if (manager->suspended)
          return DFB_SUSPENDED;

     /* examine chunks */
     c = manager->chunks;
     D_MAGIC_ASSERT( c, Chunk );


     if(DDI_GPU_AllocSurface == NULL)
     {
          assign_symbol_DDI_GPU_FUNCS(&g_dlfcn_mutex);
     } 

     if(DDI_GPU_AllocSurface != NULL)
     {
          CoreSurface *surface = buffer->surface;
          GPU_SURFACE_INFO_T surfaceInfo;
          unsigned short width = 0;
          unsigned short height = 0;
          GPU_PIXEL_FORMAT_T pixel_format = -1;

          D_PRINTF("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

          D_PRINTF("width:%d, height:%d, format:%d \n", surface->config.size.w, surface->config.size.h, buffer->format);

          width = surface->config.size.w;
          height = surface->config.size.h;

          switch(buffer->format)
          {
               case DSPF_UNKNOWN:
                    D_PRINTF("format: DSPF_UNKNOWN\n");
                    break;

                    /* 16 bit  ARGB (2 byte, alpha 1@15, red 5@10, green 5@5, blue 5@0) */
               case DSPF_ARGB1555:
                    // br support!
                    D_PRINTF("format: DSPF_ARGB1555\n");
                    pixel_format = GPU_PIXEL_FORMAT_ARGB1555;
                    break;

                    /* 16 bit   RGB (2 byte, red 5@11, green 6@5, blue 5@0) */
               case DSPF_RGB16:
                    // br support!
                    D_PRINTF("format: DSPF_RGB16\n"); 
                    pixel_format = GPU_PIXEL_FORMAT_RGB16; 
                    break;

                    /* 24 bit   RGB (3 byte, red 8@16, green 8@8, blue 8@0) */
               case DSPF_RGB24:
                    D_PRINTF("format: DSPF_RGB24\n");
                    break;

                    /* 24 bit   RGB (4 byte, nothing@24, red 8@16, green 8@8, blue 8@0) */
               case DSPF_RGB32:
                    D_PRINTF("format: DSPF_RGB32\n");
                    break;

                    /* 32 bit  ARGB (4 byte, alpha 8@24, red 8@16, green 8@8, blue 8@0) */
               case DSPF_ARGB:
                    // br support!
                    D_PRINTF("format: DSPF_ARGB\n");
                    pixel_format = GPU_PIXEL_FORMAT_ARGB; 
                    break;

                    /*  8 bit alpha (1 byte, alpha 8@0), e.g. anti-aliased glyphs */
               case DSPF_A8:
                    D_PRINTF("format: DSPF_A8\n");
                    pixel_format = GPU_PIXEL_FORMAT_A8; 
                    break;

                    /* 16 bit   YUV (4 byte/ 2 pixel, macropixel contains CbYCrY [31:0]) */
               case DSPF_YUY2:
                    D_PRINTF("format: DSPF_YUY2\n");
                    break;

                    /*  8 bit   RGB (1 byte, red 3@5, green 3@2, blue 2@0) */
               case DSPF_RGB332:
                    D_PRINTF("format: DSPF_RGB332\n");
                    break;

                    /* 16 bit   YUV (4 byte/ 2 pixel, macropixel contains YCbYCr [31:0]) */
               case DSPF_UYVY:
                    D_PRINTF("format: DSPF_UYVY\n");
                    break;

                    /* 12 bit   YUV (8 bit Y plane followed by 8 bit quarter size U/V planes) */
               case DSPF_I420:
                    D_PRINTF("format: DSPF_I420\n");
                    break;

                    /* 12 bit   YUV (8 bit Y plane followed by 8 bit quarter size V/U planes) */
               case DSPF_YV12:
                    D_PRINTF("format: DSPF_YV12\n");
                    break;

                    /*  8 bit   LUT (8 bit color and alpha lookup from palette) */
               case DSPF_LUT8:
                    D_PRINTF("format: DSPF_LUT8\n");
                    pixel_format = GPU_PIXEL_FORMAT_LUT8;
                    break;

                    /*  8 bit  ALUT (1 byte, alpha 4@4, color lookup 4@0) */
               case DSPF_ALUT44:
                    D_PRINTF("format: DSPF_ALUT44\n");
                    break;

                    /* 32 bit  ARGB (4 byte, inv. alpha 8@24, red 8@16, green 8@8, blue 8@0) */
               case DSPF_AiRGB:
                    D_PRINTF("format: DSPF_AiRGB\n");
                    break;

                    /*  1 bit alpha (1 byte/ 8 pixel, most significant bit used first) */
               case DSPF_A1:
                    D_PRINTF("format: DSPF_A1\n");
                    break;

                    /* 12 bit   YUV (8 bit Y plane followed by one 16 bit quarter size CbCr [15:0] plane) */
               case DSPF_NV12:
                    D_PRINTF("format: DSPF_NV12\n");
                    break;

                    /* 16 bit   YUV (8 bit Y plane followed by one 16 bit half width CbCr [15:0] plane) */
               case DSPF_NV16:
                    D_PRINTF("format: DSPF_NV16\n");
                    break;

                    /* 16 bit  ARGB (2 byte, alpha 2@14, red 5@9, green 5@4, blue 4@0) */
               case DSPF_ARGB2554:
                    D_PRINTF("format: DSPF_ARGB2554\n");
                    break;

                    /* 16 bit  ARGB (2 byte, alpha 4@12, red 4@8, green 4@4, blue 4@0) */
               case DSPF_ARGB4444:
                    // br support!
                    D_PRINTF("format: DSPF_ARGB4444\n");
                    pixel_format = GPU_PIXEL_FORMAT_ARGB4444;   
                    break;

                    /* 12 bit   YUV (8 bit Y plane followed by one 16 bit quarter size CrCb [15:0] plane) */
               case DSPF_NV21:
                    D_PRINTF("format: DSPF_NV21\n");
                    break;

                    /* 32 bit  AYUV (4 byte, alpha 8@24, Y 8@16, Cb 8@8, Cr 8@0) */
               case DSPF_AYUV:
                    D_PRINTF("format: DSPF_AYUV\n");
                    break;

                    /*  4 bit alpha (1 byte/ 2 pixel, more significant nibble used first) */
               case DSPF_A4:
                    D_PRINTF("format: DSPF_A4\n");
                    break;

                    /*  1 bit alpha (3 byte/  alpha 1@18, red 6@16, green 6@6, blue 6@0) */
               case DSPF_ARGB1666:
                    D_PRINTF("format: DSPF_ARGB1666\n");
                    break;

                    /*  6 bit alpha (3 byte/  alpha 6@18, red 6@16, green 6@6, blue 6@0) */
               case DSPF_ARGB6666:
                    D_PRINTF("format: DSPF_ARGB6666\n");
                    break;

                    /*  6 bit   RGB (3 byte/   red 6@16, green 6@6, blue 6@0) */
               case DSPF_RGB18:
                    D_PRINTF("format: DSPF_RGB18\n");
                    break;

                    /*  2 bit   LUT (1 byte/ 4 pixel, 2 bit color and alpha lookup from palette) */
               case DSPF_LUT2:
                    D_PRINTF("format: DSPF_LUT2\n");
                    break;

                    /* 16 bit   RGB (2 byte, nothing @12, red 4@8, green 4@4, blue 4@0) */
               case DSPF_RGB444:
                    D_PRINTF("format: DSPF_RGB444\n");
                    break;

                    /* 16 bit   RGB (2 byte, nothing @15, red 5@10, green 5@5, blue 5@0) */
               case DSPF_RGB555:
                    D_PRINTF("format: DSPF_RGB555\n");
                    break;

                    /* 16 bit   BGR (2 byte, nothing @15, blue 5@10, green 5@5, red 5@0) */
               case DSPF_BGR555:
                    D_PRINTF("format: DSPF_BGR555\n");
                    break;

               default:
                    D_PRINTF("format: Unknown!\n");
                    break;
          }

          // Call DDI_GPU_AllocSurface! 
          if(pixel_format == -1)
          {
               return DFB_NOVIDEOMEMORY;
          }

          if(DDI_GPU_AllocSurface(width, height, pixel_format, &surfaceInfo) != OK)
          {
               return DFB_NOVIDEOMEMORY;
          }

          D_PRINTF("pitch = %d, bpp = %d, offset = 0x%x\n", surfaceInfo.pitch, surfaceInfo.bpp, surfaceInfo.offset);

          /* NULL means check only. */
          if (ret_chunk)
          {
               *ret_chunk = alloc_chunk( manager, c, surfaceInfo.offset, surfaceInfo.pitch*height );
               (*ret_chunk)->allocation = allocation;
               (*ret_chunk)->buffer     = allocation->buffer;
               (*ret_chunk)->pitch      = surfaceInfo.pitch;
               (*ret_chunk)->surface_info = surfaceInfo;
          }
          else
          {
               //D_WARN("You can NOT be here!!!\n");
               DDI_GPU_DeallocSurface(surfaceInfo);
               return DFB_OK;
          }

          manager->min_toleration++;

          //D_PRINTF("chunk.pitch = %d, chunk.offset = 0x%x, chunk.id = %d\n", surface.pitch, surface.offset, surface.id);
          //    buffer->video.chunk->buffer = buffer;

          return DFB_OK;
     }
     else    
     {
          return DFB_NOVIDEOMEMORY;
     }
}
#else
{
     int pitch;
     int length;
     Chunk *c;
     CoreGraphicsDevice *device;

     Chunk *best_free = NULL;

     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( buffer->surface, CoreSurface );

     if (ret_chunk)
          D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     else
          D_ASSUME( allocation == NULL );

     D_DEBUG_AT( SurfMan, "%s( %p ) <- %dx%d %s\n", __FUNCTION__, buffer,
               buffer->surface->config.size.w, buffer->surface->config.size.h,
               dfb_pixelformat_name( buffer->surface->config.format ) );

     if (manager->suspended)
          return DFB_SUSPENDED;

     /* FIXME: Only one global device at the moment. */
     device = dfb_core_get_part( core, DFCP_GRAPHICS );
     D_ASSERT( device != NULL );

     dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &length );

     D_DEBUG_AT( SurfMan, "  -> avail %d, pitch %d, length %d\n", manager->avail, pitch, length );

     if (manager->avail < length)
          return DFB_TEMPUNAVAIL;

     /* examine chunks */
     c = manager->chunks;
     D_MAGIC_ASSERT( c, Chunk );

     /* FIXME_SC_2  Workaround creation happening before graphics driver initialization. */
     if (!c->next) {
          int length = dfb_gfxcard_memory_length();

          if (c->length != length - manager->offset) {
               D_WARN( "workaround" );

               manager->length = length;
               manager->avail  = length - manager->offset;

               c->length = length - manager->offset;
          }
     }

     while (c) {
          D_MAGIC_ASSERT( c, Chunk );

          if (!c->buffer && c->length >= length) {
               /* NULL means check only. */
               if (!ret_chunk)
                    return DFB_OK;

               /* found a nice place to chill */
               if (!best_free  ||  best_free->length > c->length)
                    /* first found or better one? */
                    best_free = c;

               if (c->length == length)
                    break;
          }

          c = c->next;
     }

     /* if we found a place */
     if (best_free) {
          D_DEBUG_AT( SurfMan, "  -> found free (%d)\n", best_free->length );

          /* NULL means check only. */
          if (ret_chunk)
               *ret_chunk = occupy_chunk( manager, best_free, allocation, length, pitch );

          return DFB_OK;
     }

     D_DEBUG_AT( SurfMan, "  -> failed (%d/%d avail)\n", manager->avail, manager->length );

     /* no luck */
     return DFB_NOVIDEOMEMORY;
}
#endif

DFBResult dfb_surfacemanager_displace( CoreDFB           *core,
                                       SurfaceManager    *manager,
                                       CoreSurfaceBuffer *buffer )
{
     int                    length;
     Chunk                 *multi_start = NULL;
     int                    multi_size  = 0;
     int                    multi_tsize = 0;
     int                    multi_count = 0;
     Chunk                 *bestm_start = NULL;
     int                    bestm_count = 0;
     int                    bestm_size  = 0;
     int                    min_toleration;
     Chunk                 *chunk;
     CoreGraphicsDevice    *device;
     CoreSurfaceAllocation *smallest = NULL;

     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( buffer->surface, CoreSurface );

     D_DEBUG_AT( SurfMan, "%s( %p ) <- %dx%d %s\n", __FUNCTION__, buffer,
                 buffer->surface->config.size.w, buffer->surface->config.size.h,
                 dfb_pixelformat_name( buffer->surface->config.format ) );

#if USE_RPC4DTV
	return DFB_NOVIDEOMEMORY;
#endif
     /* FIXME: Only one global device at the moment. */
     device = dfb_core_get_part( core, DFCP_GRAPHICS );
     D_ASSERT( device != NULL );

     dfb_gfxcard_calc_buffer_size( dfb_core_get_part( core, DFCP_GRAPHICS ), buffer, NULL, &length );

     min_toleration = manager->min_toleration/8 + 2;

     D_DEBUG_AT( SurfMan, "  -> %7d required, min toleration %d\n", length, min_toleration );

     chunk = manager->chunks;
     while (chunk) {
          CoreSurfaceAllocation *allocation;

          D_MAGIC_ASSERT( chunk, Chunk );

          allocation = chunk->allocation;
          if (allocation) {
               CoreSurfaceBuffer *other;
               int                size;

               D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
               D_ASSERT( chunk->buffer == allocation->buffer );
               D_ASSERT( chunk->length >= allocation->size );

               other = allocation->buffer;
               D_MAGIC_ASSERT( other, CoreSurfaceBuffer );

               if (other->locked) {
                    D_DEBUG_AT( SurfMan, "  ++ %7d locked %dx\n", allocation->size, other->locked );
                    goto next_reset;
               }

               if (other->policy > buffer->policy) {
                    D_DEBUG_AT( SurfMan, "  ++ %7d policy %d > %d\n", allocation->size, other->policy, buffer->policy );
                    goto next_reset;
               }

               if (other->policy == CSP_VIDEOONLY) {
                    D_DEBUG_AT( SurfMan, "  ++ %7d policy videoonly\n", allocation->size );
                    goto next_reset;
               }

               chunk->tolerations++;
               if (chunk->tolerations > 0xff)
                    chunk->tolerations = 0xff;

               if (other->policy == buffer->policy && chunk->tolerations < min_toleration) {
                    D_DEBUG_AT( SurfMan, "  ++ %7d tolerations %d/%d\n",
                                allocation->size, chunk->tolerations, min_toleration );
                    goto next_reset;
               }

               size = allocation->size;

               if (chunk->prev && !chunk->prev->allocation)
                    size += chunk->prev->length;

               if (chunk->next && !chunk->next->allocation)
                    size += chunk->next->length;

               if (size >= length) {
                    if (!smallest || smallest->size > allocation->size) {
                         D_DEBUG_AT( SurfMan, "  => %7d [%d] < %d, tolerations %d\n",
                                     allocation->size, size, smallest ? smallest->size : 0, chunk->tolerations );

                         smallest = allocation;
                    }
                    else
                         D_DEBUG_AT( SurfMan, "  -> %7d [%d] > %d\n", allocation->size, size, smallest->size );
               }
               else
                    D_DEBUG_AT( SurfMan, "  -> %7d [%d]\n", allocation->size, size );
          }
          else
               D_DEBUG_AT( SurfMan, "  -  %7d free\n", chunk->length );


          if (!smallest) {
               if (!multi_start) {
                    multi_start = chunk;
                    multi_tsize = chunk->length;
                    multi_size  = chunk->allocation ? chunk->length : 0;
                    multi_count = chunk->allocation ? 1 : 0;
               }
               else {
                    multi_tsize += chunk->length;
                    multi_size  += chunk->allocation ? chunk->length : 0;
                    multi_count += chunk->allocation ? 1 : 0;

                    while (multi_tsize >= length && multi_count > 1) {
                         if (!bestm_start || bestm_size > multi_size * multi_count / bestm_count) {
                              D_DEBUG_AT( SurfMan, "                =====> %7d, %7d %2d used [%7d %2d]\n",
                                          multi_tsize, multi_size, multi_count, bestm_size, bestm_count );

                              bestm_size  = multi_size;
                              bestm_start = multi_start;
                              bestm_count = multi_count;
                         }
                         else
                              D_DEBUG_AT( SurfMan, "                -----> %7d, %7d %2d used\n",
                                          multi_tsize, multi_size, multi_count );

                         if (multi_count <= 2)
                              break;

                         if (!multi_start->allocation) {
                              multi_tsize -= multi_start->length;
                              multi_start  = multi_start->next;
                         }

                         D_ASSUME( multi_start->allocation != NULL );

                         multi_tsize -= multi_start->length;
                         multi_size  -= multi_start->allocation ? multi_start->length : 0;
                         multi_count -= multi_start->allocation ? 1 : 0;
                         multi_start  = multi_start->next;
                    }
               }
          }

          chunk = chunk->next;

          continue;


next_reset:
          multi_start = NULL;

          chunk = chunk->next;
     }

     if (smallest) {
          D_MAGIC_ASSERT( smallest, CoreSurfaceAllocation );
          D_MAGIC_ASSERT( smallest->buffer, CoreSurfaceBuffer );

          smallest->flags |= CSALF_MUCKOUT;

          D_DEBUG_AT( SurfMan, "  -> offset %lu, size %d\n", smallest->offset, smallest->size );

          return DFB_OK;
     }

     if (bestm_start) {
          chunk = bestm_start;

          while (bestm_count) {
               CoreSurfaceAllocation *allocation = chunk->allocation;

               if (allocation) {
                    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
                    D_MAGIC_ASSERT( allocation->buffer, CoreSurfaceBuffer );
                    
                    allocation->flags |= CSALF_MUCKOUT;

                    bestm_count--;
               }

               D_DEBUG_AT( SurfMan, "  ---> offset %d, length %d\n", chunk->offset, chunk->length );

               chunk = chunk->next;
          }

          return DFB_OK;
     }

     return DFB_NOVIDEOMEMORY;
}

DFBResult dfb_surfacemanager_deallocate( SurfaceManager *manager,
                                         Chunk          *chunk )
{
     CoreSurfaceBuffer *buffer;

     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( chunk, Chunk );

     buffer = chunk->buffer;
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( buffer->surface, CoreSurface );

     D_DEBUG_AT( SurfMan, "%s( %p ) <- %dx%d %s\n", __FUNCTION__, buffer,
                 buffer->surface->config.size.w, buffer->surface->config.size.h,
                 dfb_pixelformat_name( buffer->surface->config.format ) );

#if USE_RPC4DTV
     if(DDI_GPU_DeallocSurface)
     {
          GPU_SURFACE_INFO_T surfaceInfo;

          DDI_GPU_DeallocSurface(chunk->surface_info);
          dealloc_chunk(manager, chunk);
     }
#else
     free_chunk( manager, chunk );
#endif

     return DFB_OK;
}

/** internal functions NOT locking the surfacemanager **/

static Chunk *
split_chunk( SurfaceManager *manager, Chunk *c, int length )
{
     Chunk *newchunk;

     D_MAGIC_ASSERT( c, Chunk );

     if (c->length == length)          /* does not need be splitted */
          return c;

     newchunk = (Chunk*) SHCALLOC( manager->shmpool, 1, sizeof(Chunk) );
     if (!newchunk) {
          D_OOSHM();
          return NULL;
     }

     /* calculate offsets and lengths of resulting chunks */
     newchunk->offset = c->offset + c->length - length;
     newchunk->length = length;
     c->length -= newchunk->length;

     /* insert newchunk after chunk c */
     newchunk->prev = c;
     newchunk->next = c->next;
     if (c->next)
          c->next->prev = newchunk;
     c->next = newchunk;

     D_MAGIC_SET( newchunk, Chunk );

     return newchunk;
}

static Chunk *
free_chunk( SurfaceManager *manager, Chunk *chunk )
{
     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( chunk, Chunk );

     if (!chunk->buffer) {
          D_BUG( "freeing free chunk" );
          return chunk;
     }
     else {
          D_DEBUG_AT( SurfMan, "Deallocating %d bytes at offset %d.\n", chunk->length, chunk->offset );
     }

     if (chunk->buffer->policy == CSP_VIDEOONLY)
          manager->avail += chunk->length;

     chunk->allocation = NULL;
     chunk->buffer     = NULL;

     manager->min_toleration--;

     if (chunk->prev  &&  !chunk->prev->buffer) {
          Chunk *prev = chunk->prev;

          //D_DEBUG_AT( SurfMan, "  -> merging with previous chunk at %d\n", prev->offset );

          prev->length += chunk->length;

          prev->next = chunk->next;
          if (prev->next)
               prev->next->prev = prev;

          //D_DEBUG_AT( SurfMan, "  -> freeing %p (prev %p, next %p)\n", chunk, chunk->prev, chunk->next);

          D_MAGIC_CLEAR( chunk );

          SHFREE( manager->shmpool, chunk );
          chunk = prev;
     }

     if (chunk->next  &&  !chunk->next->buffer) {
          Chunk *next = chunk->next;

          //D_DEBUG_AT( SurfMan, "  -> merging with next chunk at %d\n", next->offset );

          chunk->length += next->length;

          chunk->next = next->next;
          if (chunk->next)
               chunk->next->prev = chunk;

          D_MAGIC_CLEAR( next );

          SHFREE( manager->shmpool, next );
     }

     return chunk;
}

static Chunk *
occupy_chunk( SurfaceManager *manager, Chunk *chunk, CoreSurfaceAllocation *allocation, int length, int pitch )
{
     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( chunk, Chunk );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( allocation->buffer, CoreSurfaceBuffer );
     
     if (allocation->buffer->policy == CSP_VIDEOONLY)
          manager->avail -= length;

     chunk = split_chunk( manager, chunk, length );
     if (!chunk)
          return NULL;

     D_DEBUG_AT( SurfMan, "Allocating %d bytes at offset %d.\n", chunk->length, chunk->offset );

     chunk->allocation = allocation;
     chunk->buffer     = allocation->buffer;
     chunk->pitch      = pitch;

     manager->min_toleration++;

     return chunk;
}

#if USE_RPC4DTV
static Chunk* alloc_chunk( SurfaceManager *manager, Chunk *c, int offset, int length )
{
     Chunk *newchunk;

     newchunk = (Chunk*) SHCALLOC( manager->shmpool, 1, sizeof(Chunk) );

     /* calculate offsets and lengths of resulting chunks */
     newchunk->offset = offset;
     newchunk->length = length;

     /* insert newchunk after chunk c */
     newchunk->prev = c;
     newchunk->next = c->next;
     if (c->next)
          c->next->prev = newchunk;
     c->next = newchunk;

     D_MAGIC_SET( newchunk, Chunk );

     return newchunk;
}

void dealloc_chunk(SurfaceManager *manager, Chunk* chunk)
{

     if (chunk->buffer->policy == CSP_VIDEOONLY)
          manager->avail += chunk->length;

     chunk->allocation = NULL;
     chunk->buffer     = NULL;

     manager->min_toleration--;

     /* delete chunk from list */
     if(chunk->prev)
     {
          Chunk *prev = chunk->prev;
          prev->next = chunk->next;

          if(chunk->next)
               chunk->next->prev = prev;

          D_MAGIC_CLEAR( chunk );
          SHFREE(manager->shmpool, chunk);
          chunk = prev;
     }

}
#endif
