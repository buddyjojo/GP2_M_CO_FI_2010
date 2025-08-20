

#ifndef _HAL_MVOP_H_
#define _HAL_MVOP_H_

#include "mhal_mvop_reg.h"
#include "mdrv_mvop.h"
#include "mdrv_mvd.h"




void MHal_MVOP_Enable ( B16 bEnable );
void MHal_MVOP_AutoGenMPEGTiming(MS_MVOP_TIMING *pMVOPTiming,
                                MVD_FRAMEINFO *pstVideoStatus,
                                B16 bEnHDup);
void MHal_MVOP_Output_EnableInterlace ( B16 bEnable );
void MHal_MVOP_Input_Mode ( MVOPINPUTMODE mode, MVOPINPUTPARAM *pparam );
void MHal_MVOP_Set_YUV_Address ( MVOPINPUTMODE mode, MVOPINPUTPARAM *pparam );
void MHal_MVOP_SetMVOPSynClk ( MS_MVOP_TIMING *ptiming );
void MHal_MVOP_Init(void);
void MHal_MVOP_EnableBlackBg (void);
void MHal_MVOP_SetMVOPH264 (U16 u16HorSize, U16 u16VerSize, U16 u16Pitch, B16 bInterlace);
void MHal_MVOP_SetOutputTiming ( MS_MVOP_TIMING *ptiming );
void MHal_MVOP_SetMlinkMode ( B16 bMode );
void MHal_MVOP_SetMlinkByPassMode ( B16 bMode );

void MHal_MVOP_SetH264HardwireMode(void);
void MHal_MVOP_SetRMHardwireMode(void);
void MHAL_MVOP_SetJpegHardwireMode(void);

void MHal_MVOP_SetTestPattern(U8 patternIdx, U8 pattern);	// lemonic LGE 080908

void MHal_MVOP_SetUVShift(B16 bEnable);//LGE gbtogether(081217) ->to fix ChromaArtifact when 420to422 by Junyou.Lin
void MHal_MVOP_SetH264_Order(B16 bEnable);//junyou add for H264 thumbnail<20091001>
#endif
