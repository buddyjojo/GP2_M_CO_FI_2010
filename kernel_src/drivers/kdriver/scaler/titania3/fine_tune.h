////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __COLOR_FINE_TUNE_H
#define __COLOR_FINE_TUNE_H


//////////////////////////////////////////////////////////////////////
// ARC
///////////////////////////////////////////////////////////////////////
#define ARC_ZOOM_OVS_UP                 112
#define ARC_ZOOM_OVS_DOWN               112
#define ARC_ZOOM_OVS_LEFT               0
#define ARC_ZOOM_OVS_RIGHT              0

#define ARC_ZOOM1_OVS_UP                60
#define ARC_ZOOM1_OVS_DOWN              60
#define ARC_ZOOM1_OVS_LEFT              60
#define ARC_ZOOM1_OVS_RIGHT             60

#define ARC_ZOOM1_OVS_MAX_UP            240
#define ARC_ZOOM1_OVS_MIN_UP            10
#define ARC_ZOOM1_OVS_MAX_DOWN          240
#define ARC_ZOOM1_OVS_MIN_DOWN          10
#define ARC_ZOOM1_OVS_MAX_LEFT          240
#define ARC_ZOOM1_OVS_MIN_LEFT          10
#define ARC_ZOOM1_OVS_MAX_RIGHT         240
#define ARC_ZOOM1_OVS_MIN_RIGHT         10

#define ARC_ZOOM2_OVS_UP                60
#define ARC_ZOOM2_OVS_DOWN              60
#define ARC_ZOOM2_OVS_LEFT              0
#define ARC_ZOOM2_OVS_RIGHT             0

#define ARC_ZOOM2_OVS_MAX_UP            240
#define ARC_ZOOM2_OVS_MIN_UP            10
#define ARC_ZOOM2_OVS_MAX_DOWN          240
#define ARC_ZOOM2_OVS_MIN_DOWN          10
#define ARC_ZOOM2_OVS_MAX_LEFT          0
#define ARC_ZOOM2_OVS_MIN_LEFT          0
#define ARC_ZOOM2_OVS_MAX_RIGHT         0
#define ARC_ZOOM2_OVS_MIN_RIGHT         0

#define ARC_14X19_OVS_UP                62
#define ARC_14X19_OVS_DOWN              62

#define ARC_CINEMA_OVS_H                100
#define ARC_CINEMA_OVS_V                100
//////////////////////////////////////////////////////////////////////
// Over Scan
///////////////////////////////////////////////////////////////////////
#define OVERSCAN_DEFAULT_H              30 // 1.0%
#define OVERSCAN_DEFAULT_V              35 // 1.0%

// VD
#define MSVD_OVERSCAN_H_NTSC            38
#define MSVD_OVERSCAN_V_NTSC            24
#define MSVD_OVERSCAN_H_PAL             40
#define MSVD_OVERSCAN_V_PAL             28
#define MSVD_OVERSCAN_H_SECAM           50
#define MSVD_OVERSCAN_V_SECAM           26
#define MSVD_OVERSCAN_H_NTSC_443        50
#define MSVD_OVERSCAN_V_NTSC_443        22
#define MSVD_OVERSCAN_H_PAL_M           44
#define MSVD_OVERSCAN_V_PAL_M           22
#define MSVD_OVERSCAN_H_PAL_N           50
#define MSVD_OVERSCAN_V_PAL_N           28

// DTV
#define OVERSCAN_DTV_DEFAULT_H          10
#define OVERSCAN_DTV_DEFAULT_V          10
#define OVERSCAN_DTV_480I_H             35
#define OVERSCAN_DTV_480I_V             28
#define OVERSCAN_DTV_480P_H             34
#define OVERSCAN_DTV_480P_V             34
#define OVERSCAN_DTV_576I_H             32
#define OVERSCAN_DTV_576I_V             23
#define OVERSCAN_DTV_576P_H             20
#define OVERSCAN_DTV_576P_V             20
#define OVERSCAN_DTV_720P_H             34
#define OVERSCAN_DTV_720P_V             34
#define OVERSCAN_DTV_1080I_H            31
#define OVERSCAN_DTV_1080I_V            30
#define OVERSCAN_DTV_1080P_H            33
#define OVERSCAN_DTV_1080P_V            33

//YPBPR
#define OVERSCAN_YPBPR_480I_H          30
#define OVERSCAN_YPBPR_480I_V          22
#define OVERSCAN_YPBPR_480P_H           28//30
#define OVERSCAN_YPBPR_480P_V           22
#define OVERSCAN_YPBPR_576I_H           20//30
#define OVERSCAN_YPBPR_576I_V           22
#define OVERSCAN_YPBPR_576P_H        20//30
#define OVERSCAN_YPBPR_576P_V           22
#define OVERSCAN_YPBPR_720P_50_H        24//30
#define OVERSCAN_YPBPR_720P_50_V        22
#define OVERSCAN_YPBPR_720P_60_H       24// 30
#define OVERSCAN_YPBPR_720P_60_V        22
#define OVERSCAN_YPBPR_1080I_H          24//30
#define OVERSCAN_YPBPR_1080I_V          18//22
#define OVERSCAN_YPBPR_1080P_50_H      24// 33
#define OVERSCAN_YPBPR_1080P_50_V       22
#define OVERSCAN_YPBPR_1080P_60_H       24//32
#define OVERSCAN_YPBPR_1080P_60_V       22
#define OVERSCAN_YPBPR_1080P_30_H       33
#define OVERSCAN_YPBPR_1080P_30_V       21
#define OVERSCAN_YPBPR_1080P_25_H       33
#define OVERSCAN_YPBPR_1080P_25_V       21

// HDMI
#define OVERSCAN_HDMI_480I_H            32
#define OVERSCAN_HDMI_480I_V            22
#define OVERSCAN_HDMI_480P_H           26// 32
#define OVERSCAN_HDMI_480P_V            17//22
#define OVERSCAN_HDMI_1440_480I_H       30
#define OVERSCAN_HDMI_1440_480I_V       26
#define OVERSCAN_HDMI_1440_480P_H       32
#define OVERSCAN_HDMI_1440_480P_V       22
#define OVERSCAN_HDMI_576I_H            24//32
#define OVERSCAN_HDMI_576I_V            24
#define OVERSCAN_HDMI_576P_H            24//32
#define OVERSCAN_HDMI_576P_V            22
#define OVERSCAN_HDMI_720P_H           24// 32
#define OVERSCAN_HDMI_720P_V            22
#define OVERSCAN_HDMI_1080I_H         24//  32
#define OVERSCAN_HDMI_1080I_V           22
#define OVERSCAN_HDMI_1080P_H           24//32
#define OVERSCAN_HDMI_1080P_V           22

//-------- Scaler -------------------------
#define V_DE_SHIFT_FOR_GARBAGE          (3)//(0)
#define V_CAP_ADD_FOR_GARBAGE           (2)
#define H_CAP_ADD_FOR_GARBAGE           (2)

#define V_VOP_START_SHIFT               (5)

#define FREE_RUN_DEFAUL_CLK             500

#define VIDEO_DARK_LEVEL                300 // 300/100 = 0.3%

#endif // __COLOR_FINE_TUNE_H

