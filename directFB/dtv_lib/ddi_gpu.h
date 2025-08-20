#ifndef __DDI_GPU_H__
#define __DDI_GPU_H___
#ifdef __cplusplus
extern "C" {
#endif

#include "ddi_gpu_data.h"

#define DDI_GPU_PRINTF(x...) 

#include <dlfcn.h>
#include <pthread.h>

static DTV_STATUS_T (*RPC_OpenDDI_GPU)(SINT32 sock) = NULL;
static void (*RPC_CloseDDI_GPU)() = NULL;
static DTV_STATUS_T (*DDI_GPU_Initialize)(UINT32 width, UINT32 height, GPU_PIXEL_FORMAT_T pixelFormat, GPU_SCREEN_RESOLUTION_T *pScreenResolution) = NULL;
static DTV_STATUS_T (*DDI_GPU_GetDeviceCapability)(GPU_DEVICE_CAPABILITY_INFO_T *pDeviceCapabilityInfo) = NULL;
static DTV_STATUS_T (*DDI_GPU_InitSurfacePool)(UINT32 surfacePoolSize, UINT16 surfacePoolAlignment, GPU_SURFACE_POOL_INFO_T *pSurfacePoolInfo) = NULL;
static DTV_STATUS_T (*DDI_GPU_InitLayer)(UINT8 layerId, GPU_LAYER_CONFIG_T layerConfig) = NULL;
static DTV_STATUS_T (*DDI_GPU_Finalize)(void) = NULL;
static DTV_STATUS_T (*DDI_GPU_GetLayerRegionInfo)(SINT32 layer, GPU_LAYER_REGION_INFO_T *pLayerRegionInfo) = NULL;
static DTV_STATUS_T (*DDI_GPU_SetLayerRegion)(SINT32 layer, GPU_LAYER_CONFIG_FLAGS_T regionFlags, GPU_RECT_T viewRect, GPU_RECT_T dispRect, UINT32 regionAlpha, UINT8 zOrder, UINT32 property, GPU_PIXEL_FORMAT_T pixelFormat) = NULL;
static DTV_STATUS_T (*DDI_GPU_FlipLayerRegion)(SINT32 layer, UINT32 bufferOffset) = NULL;
static DTV_STATUS_T (*DDI_GPU_UpdateLayerRegion)(SINT32 layer, GPU_RECT_T updateRect, UINT32 bufferOffset) = NULL;
static DTV_STATUS_T (*DDI_GPU_IndicateLayerUpdate)(SINT32 layer, GPU_RECT_T updateRect, UINT32 bufferOffset) = NULL;
static DTV_STATUS_T (*DDI_GPU_AllocSurface)(UINT16 width, UINT16 height, GPU_PIXEL_FORMAT_T pixelFormat, GPU_SURFACE_INFO_T *pSurfaceInfo) = NULL;
static DTV_STATUS_T (*DDI_GPU_SetSurfacePalette)(GPU_SURFACE_INFO_T surface, UINT32 palette[256], SINT32 paletteLength) = NULL;
static DTV_STATUS_T (*DDI_GPU_DeallocSurface)(GPU_SURFACE_INFO_T surface) = NULL;
static DTV_STATUS_T (*DDI_GPU_Blit)(GPU_SURFACE_INFO_T srcSurface, GPU_RECT_T srcRect, GPU_SURFACE_INFO_T dstSurface, UINT16 dx, UINT16 dy, GPU_BLIT_FLAGS_T blitFlags, GPU_BLIT_SETTINGS_T blitSettings) = NULL;
static DTV_STATUS_T (*DDI_GPU_StretchBlit)(GPU_SURFACE_INFO_T srcSurface, GPU_RECT_T srcRect, GPU_SURFACE_INFO_T dstSurface, GPU_RECT_T dstRect, GPU_BLIT_FLAGS_T blitFlags, GPU_BLIT_SETTINGS_T blitSettings) = NULL;
static DTV_STATUS_T (*DDI_GPU_FillRectangle)(GPU_SURFACE_INFO_T dstSurface, GPU_RECT_T dstRect, UINT32 color, GPU_DRAW_FLAGS_T drawFlags, GPU_DRAW_SETTINGS_T drawSettings) = NULL;
static DTV_STATUS_T (*DDI_GPU_DrawRectangle)(GPU_SURFACE_INFO_T dstSurface, GPU_RECT_T dstRect, UINT32 color, GPU_DRAW_FLAGS_T drawFlags, GPU_DRAW_SETTINGS_T drawSettings) = NULL;
static DTV_STATUS_T (*DDI_GPU_DrawLine)(GPU_SURFACE_INFO_T dstSurface, GPU_LINE_T dstLine, UINT32 color, GPU_DRAW_FLAGS_T drawFlags, GPU_DRAW_SETTINGS_T drawSettings) = NULL;
static DTV_STATUS_T (*DDI_GPU_SyncGraphic)(void) = NULL;
static DTV_STATUS_T (*DDI_GPU_TrapezoidBlit)(GPU_SURFACE_INFO_T srcSurface, GPU_SURFACE_INFO_T dstSurface, GPU_TRAPEZOID_T trapezoidSet, GPU_BLIT_FLAGS_T blitFlags, GPU_BLIT_SETTINGS_T blitSettings) = NULL;
static DTV_STATUS_T (*DDI_GPU_DecodeImage)(GPU_IMAGE_INFO_T imageInfo, GPU_SURFACE_INFO_T dstSurface, GPU_DECODEIMAGE_FLAGS_T decodeFlags) = NULL;

static void* gRPCHandle;

static inline void assign_symbol_DDI_GPU_FUNCS(pthread_mutex_t *pMutex)
{
	if(DDI_GPU_Initialize == NULL)
	{
		/* dlclose(gRPCHandle); */
		if(pMutex){
			pthread_mutex_lock(pMutex);
		}
		gRPCHandle = dlopen("librpc4dtv.so.1", RTLD_NOW);
		if(pMutex){
			pthread_mutex_unlock(pMutex);
		}
		if(!gRPCHandle)
		{
			DDI_GPU_PRINTF("Error: dlopen... Missing library-librpc4dtv.so..\n");
		} else {

			if(pMutex){
				pthread_mutex_lock(pMutex);
			}
			*(void **) (&RPC_OpenDDI_GPU) = dlsym(gRPCHandle, "RPC_OpenDDI_GPU");
			*(void **) (&RPC_CloseDDI_GPU) = dlsym(gRPCHandle, "PRC_CloseDDI_GPU");
			*(void **) (&DDI_GPU_Initialize) = dlsym(gRPCHandle, "DDI_GPU_Initialize");
			*(void **) (&DDI_GPU_GetDeviceCapability) = dlsym(gRPCHandle, "DDI_GPU_GetDeviceCapability");
			*(void **) (&DDI_GPU_InitSurfacePool) = dlsym(gRPCHandle, "DDI_GPU_InitSurfacePool");
			*(void **) (&DDI_GPU_InitLayer) = dlsym(gRPCHandle, "DDI_GPU_InitLayer");
			*(void **) (&DDI_GPU_Finalize) = dlsym(gRPCHandle, "DDI_GPU_Finalize");
			*(void **) (&DDI_GPU_GetLayerRegionInfo) = dlsym(gRPCHandle, "DDI_GPU_GetLayerRegionInfo");
			*(void **) (&DDI_GPU_SetLayerRegion) = dlsym(gRPCHandle, "DDI_GPU_SetLayerRegion");
			*(void **) (&DDI_GPU_FlipLayerRegion) = dlsym(gRPCHandle, "DDI_GPU_FlipLayerRegion");
			*(void **) (&DDI_GPU_UpdateLayerRegion) = dlsym(gRPCHandle, "DDI_GPU_UpdateLayerRegion"); 
			*(void **) (&DDI_GPU_IndicateLayerUpdate) = dlsym(gRPCHandle, "DDI_GPU_IndicateLayerUpdate"); 
			*(void **) (&DDI_GPU_AllocSurface) = dlsym(gRPCHandle, "DDI_GPU_AllocSurface");
			*(void **) (&DDI_GPU_SetSurfacePalette) = dlsym(gRPCHandle, "DDI_GPU_SetSurfacePalette");
			*(void **) (&DDI_GPU_DeallocSurface) = dlsym(gRPCHandle, "DDI_GPU_DeallocSurface");
			*(void **) (&DDI_GPU_Blit) = dlsym(gRPCHandle, "DDI_GPU_Blit");
			*(void **) (&DDI_GPU_StretchBlit) = dlsym(gRPCHandle, "DDI_GPU_StretchBlit");
			*(void **) (&DDI_GPU_FillRectangle) = dlsym(gRPCHandle, "DDI_GPU_FillRectangle");
			*(void **) (&DDI_GPU_DrawRectangle) = dlsym(gRPCHandle, "DDI_GPU_DrawRectangle");
			*(void **) (&DDI_GPU_DrawLine) = dlsym(gRPCHandle, "DDI_GPU_DrawLine");
			*(void **) (&DDI_GPU_SyncGraphic) = dlsym(gRPCHandle, "DDI_GPU_SyncGraphic");
			*(void **) (&DDI_GPU_TrapezoidBlit) = dlsym(gRPCHandle, "DDI_GPU_TrapezoidBlit");
			*(void **) (&DDI_GPU_DecodeImage) = dlsym(gRPCHandle, "DDI_GPU_DecodeImage");
			
			if(pMutex){
				pthread_mutex_unlock(pMutex);
			}
		}
	}
}

#endif

#ifdef __cplusplus
}
#endif
#endif /* __DDI_GPU_H__ */
