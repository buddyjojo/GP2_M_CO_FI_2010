

#ifndef __QUALITY_MODE_H__
#define __QUALITY_MODE_H__

#include "mdrv_types.h"
#include "mst_platform.h"

#include "mdrv_scaler_io.h"
#include "mdrv_scaler_st.h"
#include "mdrv_scaler.h"
#include "mdrv_hdmi.h"
#include "mdrv_scaler_pcmode.h"

//-------------------------------------------------------------------------------------------------
//  PQL Define
//-------------------------------------------------------------------------------------------------
//#define QMAP_DEBUG_PRINT	// LGE drmyung 081014
#ifdef QMAP_DEBUG_PRINT
#define QMAP_DBG(fmt, args...)    printk("\033[47;31m" fmt "\033[0m", ## args)
#else
#define QMAP_DBG(fmt, args...)
#endif

/* Video Standard type */
typedef enum
{
    E_VIDEOSTANDARD_PAL_BGHI        = 0x00,        ///< Video standard PAL BGHI
    E_VIDEOSTANDARD_NTSC_M          = 0x01,        ///< Video standard NTSC M
    E_VIDEOSTANDARD_SECAM           = 0x02,        ///< Video standard SECAM
    E_VIDEOSTANDARD_NTSC_44         = 0x03,        ///< Video standard  NTSC 44
    E_VIDEOSTANDARD_PAL_M           = 0x04,        ///< Video standard  PAL M
    E_VIDEOSTANDARD_PAL_N           = 0x05,        ///< Video standard  PAL N
    E_VIDEOSTANDARD_PAL_60          = 0x06,        ///< Video standard PAL 60
    E_VIDEOSTANDARD_NOTSTANDARD     = 0x07        ///< NOT Video standard
} VIDEOSTANDARD_TYPE;


#ifndef BOOLEAN
#define BOOLEAN    U8
#endif

#define QM_IsSourceVGA(x)            (Use_VGA_Source(x->SrcType))
#define QM_IsSourceHDMI(x)           ((Use_HDMI_Source(x->SrcType)) && (x->bHDMIMode == TRUE))
#define QM_IsSourceDVI_HDMIPC(x)     ((Use_HDMI_Source(x->SrcType)) && (x->bHDMIMode == FALSE))
#define QM_HDMIPC_COLORRGB(x)        (x->u8HDMIColorFormat == HDMI_COLOR_RGB)
#define QM_HDMIPC_COLORYUV422(x)     (x->u8HDMIColorFormat == HDMI_COLOR_YUV_422)
#define QM_HDMIPC_COLORYUV444(x)     (x->u8HDMIColorFormat == HDMI_COLOR_YUV_444)
#define QM_IsSourceYPbPr(x)          (Use_YPbPr_Source(x->SrcType))
#define QM_IsSourceDTV(x)            ((Use_DTV_Source(x->SrcType)) && (!(x->bMediaPhoto || x->bEmpPlayingVideo)))

#define QM_IsSourceMultiMedia(x)     ((x->bEmpPlayingVideo)  || (x->bMediaPhoto))
#define QM_IsMultiMediaMOVIE(x)      (x->bEmpPlayingVideo)
#define QM_IsMultiMediaPHOTO(x)      (x->bMediaPhoto) //[090910_Leo]
#define QM_IsSourceMultiMediaPreview(x) (x->bEmppreview)
#define QM_IsDTV_MPEG2(x)            (x->u16mvdtype == 0)
#define QM_IsDTV_H264(x)             (x->u16mvdtype == 1)
#define QM_IsSourceScartRGB(x)       (Use_SCART_Source(x->SrcType) && MHal_SC_GetScartMode() == SCART_MODE_RGB)
#define QM_IsSourceScartCVBS(x)      (Use_SCART_Source(x->SrcType) && MHal_SC_GetScartMode() == SCART_MODE_CVBS)
#define QM_IsSourceATV(x)            (Use_ATV_Source(x->SrcType))
#define QM_IsSourceSV(x)             (Use_SV_Source(x->SrcType))
#define QM_IsSourceAV(x)             (Use_AV_Source(x->SrcType))
#define QM_GetInputHSize(x)          (Use_YPbPr_Source(x->SrcType) ? MDrv_SC_PCMode_GetStdModeResH(x->u8ModeIdx): x->u16H_CapSize)
#define QM_GetInputVSize(x)          (x->u16V_CapSize)
#define QM_GetInputVFreq(x)          (x->u16Input_VFreq)
#define QM_IsInterlaced(x)           (x->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
#define QM_GetDispHSize(x)           (x->u16H_DispSize)   //victor 20080929
#define QM_GetDispVSize(x)           (x->u16V_DispSize)   //victor 20080929
#define QM_GetATVStandard(x)         (x->u8VideoSystem)

#endif
