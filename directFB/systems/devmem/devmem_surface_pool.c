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

#include <direct/debug.h>
#include <direct/mem.h>

#include <core/surface_pool.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include "devmem.h"
#include "surfacemanager.h"

#if USE_RPC4DTV
#include <pthread.h>
extern pthread_mutex_t g_dlfcn_mutex;
#endif

D_DEBUG_DOMAIN( DevMem_Surfaces, "DevMem/Surfaces", "DevMem Framebuffer Surface Pool" );
D_DEBUG_DOMAIN( DevMem_SurfLock, "DevMem/SurfLock", "DevMem Framebuffer Surface Pool Locks" );

/**********************************************************************************************************************/

typedef struct {
     int             magic;

     SurfaceManager *manager;
} DevMemPoolData;

typedef struct {
     int             magic;

     CoreDFB        *core;
     void           *mem;
} DevMemPoolLocalData;

typedef struct {
     int   magic;

     int   offset;
     int   pitch;
     int   size;

     Chunk *chunk;
} DevMemAllocationData;

/**********************************************************************************************************************/

static int
devmemPoolDataSize( void )
{
     return sizeof(DevMemPoolData);
}

static int
devmemPoolLocalDataSize( void )
{
     return sizeof(DevMemPoolLocalData);
}

static int
devmemAllocationDataSize( void )
{
     return sizeof(DevMemAllocationData);
}

static DFBResult
devmemInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     DFBResult            ret;
     DevMemPoolData      *data   = pool_data;
     DevMemPoolLocalData *local  = pool_local;
     DevMemData          *devmem = system_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( data != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( devmem != NULL );
     D_ASSERT( devmem->shared != NULL );
     D_ASSERT( ret_desc != NULL );

     ret = dfb_surfacemanager_create( core, dfb_config->video_length, &data->manager );
     if (ret)
          return ret;

     ret_desc->caps     = CSPCAPS_NONE;
     ret_desc->access   = CSAF_CPU_READ | CSAF_CPU_WRITE | CSAF_GPU_READ | CSAF_GPU_WRITE | CSAF_SHARED;
     ret_desc->types    = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL;
     ret_desc->priority = CSPP_DEFAULT;
     ret_desc->size     = dfb_config->video_length;

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "/dev/mem" );

     local->core = core;
     local->mem  = devmem->mem;

     D_MAGIC_SET( data, DevMemPoolData );
     D_MAGIC_SET( local, DevMemPoolLocalData );

     devmem->shared->manager = data->manager;

     return DFB_OK;
}

static DFBResult
devmemJoinPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     DevMemPoolData      *data   = pool_data;
     DevMemPoolLocalData *local  = pool_local;
     DevMemData          *devmem = system_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_ASSERT( local != NULL );
     D_ASSERT( devmem != NULL );
     D_ASSERT( devmem->shared != NULL );

     (void) data;

     local->core = core;
     local->mem  = devmem->mem;

     D_MAGIC_SET( local, DevMemPoolLocalData );

     return DFB_OK;
}

static DFBResult
devmemDestroyPool( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );

     dfb_surfacemanager_destroy( data->manager );

     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
devmemLeavePool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );

     (void) data;

     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
devmemTestConfig( CoreSurfacePool         *pool,
                 void                    *pool_data,
                 void                    *pool_local,
                 CoreSurfaceBuffer       *buffer,
                 const CoreSurfaceConfig *config )
{
     DFBResult           ret;
     CoreSurface        *surface;
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     if (surface->type & CSTF_LAYER)
          return DFB_OK;

     ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, NULL, NULL );

     D_DEBUG_AT( DevMem_Surfaces, "  -> %s\n", DirectFBErrorString(ret) );

     return ret;
}

static DFBResult
devmemAllocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
     DFBResult             ret;
     CoreSurface          *surface;
     DevMemPoolData       *data  = pool_data;
     DevMemPoolLocalData  *local = pool_local;
     DevMemAllocationData *alloc = alloc_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     if ((surface->type & CSTF_LAYER)/* && surface->resource_id == DLID_PRIMARY*/)
     {
          int index  = dfb_surface_buffer_index( buffer );

          D_DEBUG_AT( DevMem_Surfaces, "[%s:%d-%s] index = %d (surface->resource_id=%d)\n", __FUNCTION__, __LINE__, __FILE__, index, surface->resource_id);

#if USE_VIRTUALFB
          index = 0;
#endif
          alloc->pitch = MAX( surface->config.size.w, surface->config.min_size.w );
          alloc->pitch = DFB_BYTES_PER_LINE( buffer->format, alloc->pitch );
          alloc->size  = surface->config.size.h * alloc->pitch;
#if !USE_RPC4DTV
          alloc->offset = index * alloc->size;
#else
          int num_of_layers = 0;
          if(!DDI_GPU_GetDeviceCapability || !DDI_GPU_GetLayerRegionInfo)
          {
               assign_symbol_DDI_GPU_FUNCS(&g_dlfcn_mutex);
          }
          
          if(DDI_GPU_GetDeviceCapability)
          {
               GPU_DEVICE_CAPABILITY_INFO_T deviceCapability;
               if( DDI_GPU_GetDeviceCapability(&deviceCapability) != OK)
               {
                    D_ERROR( "[%s:%d-%s]: DDI_GPU_GetDeviceCapability returns failure!\n",__FILE__, __LINE__,  __func__ );
                    return DFB_FAILURE;
               }
               num_of_layers = deviceCapability.maxNumOfLayers;
               D_DEBUG_AT( DevMem_Surfaces, "num_of_layers=%d\n", num_of_layers);
          }
          else
          {
               D_ERROR( "[%s:%d-%s]: Unable to call DDI_GPU_GetDeviceCapability()!\n",__FILE__, __LINE__,  __func__ );
               return DFB_FAILURE;
          }
          D_DEBUG_AT( DevMem_Surfaces, "resource_id = %d\n", surface->resource_id);

          if(DDI_GPU_GetLayerRegionInfo)
          {
               int i;
               int skip_num = surface->resource_id;
               GPU_LAYER_REGION_INFO_T layer_region_info;
               for(i=0; i<num_of_layers; i++)
               {
                    if( DDI_GPU_GetLayerRegionInfo(i, &layer_region_info) != OK)
                    {
                         D_ERROR( "[%s:%d-%s]: DDI_GPU_GetLayerRegionInfo returns failure!\n",__FILE__, __LINE__,  __func__ );
                         return DFB_FAILURE;
                    }

                    if(layer_region_info.property & GPU_PROPERTY_LAYER_FOR_DIRECTFB)
                    {
                         if(skip_num>0)
                         {
                              skip_num--;
                              continue;
                         }
                         D_DEBUG_AT( DevMem_Surfaces, "layer[%d] is selected for resource_id(%d)\n", i, surface->resource_id);
                         if((surface->resource_id == DLID_PRIMARY && index == 0) || (surface->resource_id != DLID_PRIMARY && index == 1))
                         {
                              alloc->offset = layer_region_info.surfaceInfo[0].offset;
                              alloc->pitch = layer_region_info.surfaceInfo[0].pitch;
                              alloc->size = layer_region_info.surfaceInfo[0].height * alloc->pitch;
                         } else {
                              if(layer_region_info.bUseDoubleBuffer)
                              {
                                   alloc->offset = layer_region_info.surfaceInfo[1].offset;
                                   alloc->pitch = layer_region_info.surfaceInfo[1].pitch;
                                   alloc->size = layer_region_info.surfaceInfo[1].height * alloc->pitch;
                              }
                              else
                              {
                                   alloc->offset = layer_region_info.surfaceInfo[0].offset;
                                   alloc->pitch = layer_region_info.surfaceInfo[0].pitch;
                                   alloc->size = layer_region_info.surfaceInfo[0].height * alloc->pitch;
                              }
                         }
                         alloc->chunk = NULL;

                         D_DEBUG_AT( DevMem_Surfaces, "  ->  index=%d, alloc: pitch:%d, size:%d, offset:0x%08x\n", index, alloc->pitch, alloc->size, alloc->offset);

                         break;
                    }
                    else  //Only layer for DirectFB is needed.
                    {
                         D_DEBUG_AT( DevMem_Surfaces, "layer[%d] is discarded\n", i);
                         continue;
                    }
               }
          }
          else
          {
               D_ERROR( "[%s:%d-%s]: Unable to call DDI_GPU_GetLayerRegionInfo()!\n",__FILE__, __LINE__,  __func__ );
               return DFB_FAILURE;
          }
#endif
     }
     else {
          Chunk *chunk;

          ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, allocation, &chunk );
          if (ret)
               return ret;

          D_MAGIC_ASSERT( chunk, Chunk );

          alloc->offset = chunk->offset;
          alloc->pitch  = chunk->pitch;
          alloc->size   = chunk->length;

          alloc->chunk  = chunk;
     }

     D_DEBUG_AT( DevMem_Surfaces, "  -> offset %d, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size );

     allocation->size   = alloc->size;
     allocation->offset = alloc->offset;

     D_MAGIC_SET( alloc, DevMemAllocationData );

     return DFB_OK;
}

static DFBResult
devmemDeallocateBuffer( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     DevMemPoolData       *data  = pool_data;
     DevMemAllocationData *alloc = alloc_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );

     if (alloc->chunk)
          dfb_surfacemanager_deallocate( data->manager, alloc->chunk );

     D_MAGIC_CLEAR( alloc );

     return DFB_OK;
}

static DFBResult
devmemLock( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
     DevMemPoolLocalData  *local = pool_local;
     DevMemAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( DevMem_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

     lock->pitch  = alloc->pitch;
     lock->offset = alloc->offset;
     lock->addr   = local->mem + alloc->offset;
     lock->phys   = dfb_config->video_phys + alloc->offset;

     D_DEBUG_AT( DevMem_SurfLock, "  -> offset %lu, pitch %d, addr %p, phys 0x%08lx\n",
                 lock->offset, lock->pitch, lock->addr, lock->phys );

     return DFB_OK;
}

static DFBResult
devmemUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
     DevMemAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( DevMem_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

     (void) alloc;

     return DFB_OK;
}

const SurfacePoolFuncs devmemSurfacePoolFuncs = {
     .PoolDataSize       = devmemPoolDataSize,
     .PoolLocalDataSize  = devmemPoolLocalDataSize,
     .AllocationDataSize = devmemAllocationDataSize,

     .InitPool           = devmemInitPool,
     .JoinPool           = devmemJoinPool,
     .DestroyPool        = devmemDestroyPool,
     .LeavePool          = devmemLeavePool,

     .TestConfig         = devmemTestConfig,
     .AllocateBuffer     = devmemAllocateBuffer,
     .DeallocateBuffer   = devmemDeallocateBuffer,

     .Lock               = devmemLock,
     .Unlock             = devmemUnlock,
};

