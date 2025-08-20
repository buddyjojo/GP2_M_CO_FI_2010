#ifndef _MSSCALERIP_H_
#define _MSSCALERIP_H_

#ifdef _MSSCALERIP_C_
#define _MSSCALERIPDEC_
#else
#define _MSSCALERIPDEC_ extern
#endif

// type definition
typedef enum _MirrorModeType
{
    MIRROR_OFF,
    MIRROR_H_MODE,
    MIRROR_V_MODE,
    MIRROR_HV_MODE
}MirrorModeType;


U16 Ycout_LinePitch(U8 u8IpMode, U16 wPnlWidth);

void MDrv_MFC_SetMirrorMode(MirrorModeType ucMirrorMode);
void MDrv_MFC_InitializeScalerIP(void);
void MDrv_MFC_InitializeOPM(void);
void MDrv_MFC_SetOPMBaseAddr(void);
void MDrv_MFC_SoftwareResetIP(void);
void MDrv_MFC_SoftwareResetOPM(void);
void msReset2Chip(void);
//void MDrv_MFC_SoftwareResetScaler(void);
//_MSSCALERIPDEC_ void MDrv_MFC_SoftwareResetScalerInt(void);
_MSSCALERIPDEC_ BOOL IS_IP_YUV(U8 u8IpType);
_MSSCALERIPDEC_ BOOL IS_IP_RGB(U8 u8IpType);


#endif

