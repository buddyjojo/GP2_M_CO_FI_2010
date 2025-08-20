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
#ifndef __DRV_SCLAER_IO_H__
#define __DRV_SCLAER_IO_H__

//------------------------------------------------------------------------------
// Driver Name
//------------------------------------------------------------------------------
#define SC_MODULE_KERNAL_NAME       "/dev/scaler"

//------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------

// use 's' as magic number
#define SC_IOCTL_MAGIC      's'

// Scaler IO number 1 ~ 80
#define IOCTL_SC_INIT                             _IOW (SC_IOCTL_MAGIC,   0, int)
#define IOCTL_SC_SET_PANELOUTPUT                  _IOW (SC_IOCTL_MAGIC,   1, int)
#define IOCTL_SC_SET_NO_SIGNAL_COLOR              _IOW (SC_IOCTL_MAGIC,   2, int)
#define IOCTL_SC_SET_INPUTSOURCE                  _IOW (SC_IOCTL_MAGIC,   3, int)
#define IOCTL_SC_SET_MVD_SIGNAL_INFO              _IOW (SC_IOCTL_MAGIC,   4, int)
#define IOCTL_SC_SET_VD_SIGNAL_INFO               _IOW (SC_IOCTL_MAGIC,   5, int)
#define IOCTL_SC_SET_CAPTURE_WIN                  _IOW (SC_IOCTL_MAGIC,   6, int)
#define IOCTL_SC_GET_CAPTURE_WIN                  _IOWR(SC_IOCTL_MAGIC,   7, int)
#define IOCTL_SC_GET_REAL_CAPTURE_WIN             _IOWR(SC_IOCTL_MAGIC,   8, int) //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
#define IOCTL_SC_SET_CROPWIN                      _IOW (SC_IOCTL_MAGIC,   9, int)
#define IOCTL_SC_GET_CROP_WIN                     _IOWR(SC_IOCTL_MAGIC,  10, int)
#define IOCTL_SC_SET_DISPLAY_WIN                  _IOW (SC_IOCTL_MAGIC,  11, int)
#define IOCTL_SC_GET_DISPLAY_WIN                  _IOWR(SC_IOCTL_MAGIC,  12, int)
#define IOCTL_SC_ACTIVE                           _IOW (SC_IOCTL_MAGIC,  13, int)
#define IOCTL_SC_SET_BLACK_SCREEN                 _IOW(SC_IOCTL_MAGIC,   14, int)//lachesis_090831
#define IOCTL_SC_SET_FREEZEIMG                    _IOW (SC_IOCTL_MAGIC,  15, int)
#define IOCTL_SC_SET_FREERUNCOLORENABLE           _IOW (SC_IOCTL_MAGIC,  16, int)
#define IOCTL_SC_SET_FREERUNCOLOR                 _IOW (SC_IOCTL_MAGIC,  17, int)
#define IOCTL_SC_SET_FRAMERATE                    _IOW (SC_IOCTL_MAGIC,  18, int)
#define IOCTL_SC_GET_FRAMERATE                    _IOWR(SC_IOCTL_MAGIC,  19, int)
#define IOCTL_SC_GET_INPUTTIMINGINFO              _IOWR(SC_IOCTL_MAGIC,  20, int)
#define IOCTL_SC_SET_FRAMECOLOR                   _IOW (SC_IOCTL_MAGIC,  21, int)
#define IOCTL_SC_NOTIFY_CHANGED_FMT               _IOW (SC_IOCTL_MAGIC,  22, int)
#define IOCTL_SC_SET_MVDTYPE                      _IOW (SC_IOCTL_MAGIC,  23, int)
#define IOCTL_SC_SELECT_CSC                       _IOW (SC_IOCTL_MAGIC,  24, int) //victor 20080821
#define IOCTL_SC_SET_SC_SIGNAL_INFO               _IOW (SC_IOCTL_MAGIC,  25, int) // LGE drmyung 081022   // check
#define IOCTL_SC_SET_DUPLICATE          	      _IOW (SC_IOCTL_MAGIC,  26, int) //LGE gbtogether 081021
#define IOCTL_SC_GET_FRAMEDATA                    _IOWR(SC_IOCTL_MAGIC,  27, int)
#define IOCTL_SC_SET_FRAMEDATA                    _IOWR(SC_IOCTL_MAGIC,  28, int)
#define IOCTL_SC_SETTIMINGCHGSTATUS               _IOW (SC_IOCTL_MAGIC,  29, int) //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
#define IOCTL_SC_SET_LPLL                         _IOW (SC_IOCTL_MAGIC,  30, int) // FitchHsu 20080811 implement LPLL type
#define IOCTL_SET_PANELDATA				       	  _IOW (SC_IOCTL_MAGIC,  31, int)//balup_081231
#define IOCTL_SC_MWE_Enable                       _IOW (SC_IOCTL_MAGIC,  32, int) // CC Chen 20081124 MWE implement
#define IOCTL_SC_MWE_SetWinType                   _IOW (SC_IOCTL_MAGIC,  33, int) // CC Chen 20081124 MWE implement
#define IOCTL_SC_GET_FRAMELOCK_STATUS             _IOW (SC_IOCTL_MAGIC,  34, int) // LGE [vivakjh]  2008/12/11 Merge!! FitchHsu 20081209 implement frame lock status report
#define IOCTL_SC_SET_FRAME_TO_48HZ 			      _IOW (SC_IOCTL_MAGIC,  35, int)	// LGE[totaesun] 2008.12.27 24P 입력일때 48Hz 출력으로 바꾸기 위함.
#define IOCTL_SC_SET_LVDS_SSC                     _IOW (SC_IOCTL_MAGIC,  36, int)
#define IOCTL_SC_SET_3DCOMB                       _IOR (SC_IOCTL_MAGIC,  37, int)
#define IOCTL_SC_EMP_PREVIEW                      _IOW (SC_IOCTL_MAGIC,  38, int) //FitchHsu 20081119 EMP preview setting for 24P and 30P
#define IOCTL_SC_EMP_PLAYING_VIDEO                _IOW (SC_IOCTL_MAGIC,  39, int)//victor 20090108, add emp video input source
#define IOCTL_SC_GOPSEL                           _IOW (SC_IOCTL_MAGIC,  41, int)
#define IOCTL_SC_SET_GOP_TO_IP                    _IOW (SC_IOCTL_MAGIC,  42, int)
#define IOCTL_SC_SET_GOP_TO_VOP                   _IOW (SC_IOCTL_MAGIC,  43, int)
#define IOCTL_SC_GOP_SAVE_SETTING                 _IOW (SC_IOCTL_MAGIC,  44, int) //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
#define IOCTL_SC_GOP_RESTORE_SETTING              _IOW (SC_IOCTL_MAGIC,  45, int) //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
#define IOCTL_SC_SET_FBL                          _IOW (SC_IOCTL_MAGIC,  46, int)
#define IOCTL_SC_SET_FREERUN                      _IOWR(SC_IOCTL_MAGIC,  47, int)
#define IOCTL_SC_EMP_JPEG                         _IOW (SC_IOCTL_MAGIC,  48, int) //[090910_Leo]
#define IOCTL_SC_IPMUX_PATTERN                    _IOW (SC_IOCTL_MAGIC,  49, int) // 20091021 daniel.huang: add ipmux test pattern for inner test pattern

//------------------------------------------------------------------------------
// VIDEO MIRROR
// Michu 20090903
//------------------------------------------------------------------------------
#define IOCTL_SC_SET_VIDEOMIRROR                  _IOW (SC_IOCTL_MAGIC,  50, int)
//------------------------------------------------------------------------------
// End of VIDEO MIRROR
//------------------------------------------------------------------------------

// Michu 20091026, OD
#define IOCTL_SC_SET_ODTABLE                      _IOW (SC_IOCTL_MAGIC,  51, int)
#define IOCTL_SC_SET_ODENABLE                     _IOW (SC_IOCTL_MAGIC,  52, int)
#define IOCTL_SC_SETFD_MASK                       _IOW (SC_IOCTL_MAGIC,  53, int)
#define IOCTL_SC_SETDITHERING                     _IOW (SC_IOCTL_MAGIC,  54, int)

// PC mode (VGA & YPbPr) IO number 81 ~ 95
#define IOCTL_SC_SET_MODETABLE                    _IOW (SC_IOCTL_MAGIC,  81, int)
#define IOCTL_SC_GET_PCMODEINFO                   _IOWR(SC_IOCTL_MAGIC,  82, int)
#define IOCTL_SC_SET_PCMODEINFO                   _IOW (SC_IOCTL_MAGIC,  83, int)
#define IOCTL_SC_SET_PCMODE_RESOLUTION_INDEX      _IOW (SC_IOCTL_MAGIC,  84, int) // swwoo LGE 080626
#define IOCTL_SC_GET_PCMODEINFO_AUTOTUNE	      _IOWR(SC_IOCTL_MAGIC,  85, int)

// shjang_091006 20091006 ykkim5
#define	IOCTL_SC_SET_COMP_SYNCLEVEL				_IOW (SC_IOCTL_MAGIC,  86, int)

#define	IOCTL_SC_GET_ALL_DATA					_IOW (SC_IOCTL_MAGIC,  87, int)  //ykkim5 091122

// HDMI mode (HDMI & DVI) IO number 96 ~ 130
#define IOCTL_SC_GET_HDMI_INFO                    _IOWR(SC_IOCTL_MAGIC,  96, int)
#define IOCTL_SC_GET_HDMI_ASPECTRATIO             _IOWR(SC_IOCTL_MAGIC,  97, int)
#define IOCTL_SC_GET_HDMI_XVYCC                   _IOWR(SC_IOCTL_MAGIC,  98, int)
#define IOCTL_SC_SET_HDMI_HDCP                    _IOW (SC_IOCTL_MAGIC,  99, int) // LGE wlgnsl99 080902
#define IOCTL_SC_GET_HDMI_COLOR_DOMAIN            _IOWR(SC_IOCTL_MAGIC, 100, int)  //victor 20080910
#define IOCTL_SC_IS_HDMI                          _IOWR(SC_IOCTL_MAGIC, 101, int)  //victor 20080923
#define IOCTL_SC_SET_HDMI_EQ                      _IOWR(SC_IOCTL_MAGIC, 102, int)  // 081027 wlgnsl99 LGE : set HDMI EQ
#define IOCTL_SC_ENABLE_DDCBUS                    _IOW (SC_IOCTL_MAGIC, 103, int)//victor 20081215, DDC
#define IOCTL_SC_ENABLE_DVI_CLOCK                 _IOW (SC_IOCTL_MAGIC, 104, int)//lachesis_081229 DVI clock control
#define IOCTL_SC_HDMI_RESET_PACKET                _IOW (SC_IOCTL_MAGIC, 105, int)//victor 20081230, HDMI Reset Packet
#define IOCTL_SC_SET_HDMI_HPD				      _IOW (SC_IOCTL_MAGIC, 106, int)//lachesis_090723 HPD control
#define IOCTL_SC_GET_HDMI_VSI                     _IOWR(SC_IOCTL_MAGIC, 107, int)
#define IOCTL_SC_GET_HDMI_AVI                     _IOWR(SC_IOCTL_MAGIC, 108, int)
#define IOCTL_SC_HDMI_INIT                        _IOW (SC_IOCTL_MAGIC, 109, int) // LGE wlgnsl99 080902
#define IOCTL_SC_HDMI_SETMUX                      _IOW (SC_IOCTL_MAGIC, 110, int) // daniel.huang 20090625
#define IOCTL_SC_GET_HDMIINPUTTIMINGINFO          _IOWR(SC_IOCTL_MAGIC, 111, int)

// Scart mode IO number 131 ~ 140
#define IOCTL_SC_GET_SCART_FB_MODE                _IOWR(SC_IOCTL_MAGIC, 131, int)
#define IOCTL_SC_GET_SCART_INPUT_AR               _IOWR(SC_IOCTL_MAGIC, 132, int)
#define IOCTL_SC_SET_SCART_OVERLAY                _IOW (SC_IOCTL_MAGIC, 133, int)

//ADC Calibration IO number 141 ~ 175
#define IOCTL_SC_SET_GAIN                         _IOW (SC_IOCTL_MAGIC, 141, int)//thchen 20080729
#define IOCTL_SC_GET_GAIN                         _IOW (SC_IOCTL_MAGIC, 142, int)//thchen 20080729
#define IOCTL_SC_SET_OFFSET                       _IOWR(SC_IOCTL_MAGIC, 143, int)//thchen 20080729
#define IOCTL_SC_GET_OFFSET				          _IOWR(SC_IOCTL_MAGIC, 144, int)//thchen 20080729
#define IOCTL_SC_SET_ADC_SOURCE                   _IOW (SC_IOCTL_MAGIC, 145, int) // daniel.huang 20090615
#define IOCTL_SC_SET_ADC_MUX                      _IOW (SC_IOCTL_MAGIC, 146, int) // daniel.huang 20090615
#define IOCTL_SC_SET_ADC_CVBSO                    _IOW (SC_IOCTL_MAGIC, 147, int) // daniel.huang 20090615
#define IOCTL_SC_SET_ADC_CVBSO_MUX                _IOW (SC_IOCTL_MAGIC, 148, int) // daniel.huang 20090615
#define IOCTL_SC_AUTOADJUST                       _IOW (SC_IOCTL_MAGIC, 149, int)
#define IOCTL_SC_PHASEADJUST				      _IOW (SC_IOCTL_MAGIC, 150, int) // swwoo LGE 080626

// MACE and PQL IO number 176 ~ 240
#define IOCTL_SC_SET_YUV2RGB_MTX                  _IOW (SC_IOCTL_MAGIC, 176, int)
#define IOCTL_SC_SET_CONTRAST                     _IOW (SC_IOCTL_MAGIC, 177, int)
#define IOCTL_SC_SET_SATURATION                   _IOW (SC_IOCTL_MAGIC, 178, int)
#define IOCTL_SC_SET_HUE                          _IOW (SC_IOCTL_MAGIC, 179, int)
#define IOCTL_SC_SET_RGB_EX                       _IOW (SC_IOCTL_MAGIC, 180, int)
#define IOCTL_SC_SET_BRIGHTNESS                   _IOW (SC_IOCTL_MAGIC, 181, int)
#define IOCTL_SC_SET_PRE_CONTRAST_BRIGHTNESS      _IOW (SC_IOCTL_MAGIC, 182, int)//victor 20081016, ContrastBrightness
#define IOCTL_SC_SET_POST_CONTRAST_BRIGHTNESS     _IOW (SC_IOCTL_MAGIC, 183, int)//victor 20081016, ContrastBrightness
#define IOCTL_SC_SET_BLACKLEVEL                   _IOW (SC_IOCTL_MAGIC, 184, int)//victor 20081106
#define IOCTL_SC_SET_CDNRENABLE                   _IOW (SC_IOCTL_MAGIC, 185, int)//[090615_Leo]
#define IOCTL_SC_SET_CDNR_INDEX                   _IOW (SC_IOCTL_MAGIC, 186, int)//[090616_Leo]
#define IOCTL_SC_SET_CDNR_GAIN                    _IOW (SC_IOCTL_MAGIC, 187, int)//[090617_Leo]
#define IOCTL_SC_SET_AUTO_NR_ENABLE               _IOW (SC_IOCTL_MAGIC, 188, int)//[090617_Leo]
#define IOCTL_SC_GET_HISTOGRAM_INFO               _IOWR(SC_IOCTL_MAGIC, 189, int)
#define IOCTL_SC_SET_LUMA_CURVE                   _IOW (SC_IOCTL_MAGIC, 190, int)
#define IOCTL_SC_SET_LUMA_CURVE_ENABLE            _IOW (SC_IOCTL_MAGIC, 191, int)
#define IOCTL_SC_SET_HISTOGRAM_REQ_ENABLE         _IOW (SC_IOCTL_MAGIC, 192, int)
#define IOCTL_SC_DLCINIT                          _IOW (SC_IOCTL_MAGIC, 193, int)
#define IOCTL_SC_SET_COLOR_RANGE                  _IOW (SC_IOCTL_MAGIC, 194, int)//[090601_Leo]
#define IOCTL_SC_SET_ICC_SATURATION_ADJ           _IOW (SC_IOCTL_MAGIC, 195, int)
#define IOCTL_SC_SET_IBC_Y_ADJ                    _IOW (SC_IOCTL_MAGIC, 196, int)
#define IOCTL_SC_SET_IHC_HUE_ADJ                  _IOW (SC_IOCTL_MAGIC, 197, int)
#define IOCTL_SC_SET_ICC_SATURATION_ENABLE        _IOW (SC_IOCTL_MAGIC, 198, int)
#define IOCTL_SC_SET_IBC_Y_ENABLE                 _IOW (SC_IOCTL_MAGIC, 199, int)
#define IOCTL_SC_SET_IHC_HUE_ENABLE               _IOW (SC_IOCTL_MAGIC, 200, int)
#define IOCTL_SC_SET_ICC_REGION                   _IOW (SC_IOCTL_MAGIC, 201, int)
#define IOCTL_SC_SET_IHC_REGION                   _IOW (SC_IOCTL_MAGIC, 202, int)
#define IOCTL_SC_SET_ICC_YMODEL_ENABLE            _IOW (SC_IOCTL_MAGIC, 203, int)  //victor 20080818
#define IOCTL_SC_SET_IHC_YMODEL_ENABLE            _IOW (SC_IOCTL_MAGIC, 204, int)  //victor 20080818
#define IOCTL_SC_SET_BLUE_STRETCH_ENABLE          _IOW (SC_IOCTL_MAGIC, 205, int)  //victor 20080830
#define IOCTL_SC_SET_CSC_OFFSET_ENABLE            _IOW (SC_IOCTL_MAGIC, 206, int)  //victor 20080830
#define IOCTL_SC_SET_CSC_ENABLE                   _IOW (SC_IOCTL_MAGIC, 207, int)  //victor 20080909
#define IOCTL_SC_GET_VIP_CSC_ENABLE               _IOR (SC_IOCTL_MAGIC, 208, int) //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
#define IOCTL_SC_SET_DEFEATHER_TH                 _IOW (SC_IOCTL_MAGIC, 209, int)  //victor 20080923
#define IOCTL_SC_SET_IHC_HUE_COLOR_DIFF_ADJ       _IOW (SC_IOCTL_MAGIC, 210, int) //[090623_Leo] // check
#define IOCTL_SC_SET_IHC_YMODE_DIFF_COLOR_ENABLE  _IOW (SC_IOCTL_MAGIC, 211, int) //[090623_Leo] // check
#define IOCTL_SC_SET_GAMMA_TABLE                  _IOW (SC_IOCTL_MAGIC, 212, int)
#define IOCTL_SC_SET_FILM_MODE                    _IOW (SC_IOCTL_MAGIC, 213, int)
#define IOCTL_SC_SET_QMAP_TYPE                    _IOW (SC_IOCTL_MAGIC, 214, int) //victor 20081210, QMap
#define IOCTL_SC_PQDUMP_REGTABLE                  _IOW (SC_IOCTL_MAGIC, 215, int)
#define IOCTL_SC_PQ_FASTPLAYBACK                  _IOW (SC_IOCTL_MAGIC, 216, int) //FitchHsu 20081113 EMP when PAUSE, little shaking
#define IOCTL_SC_SET_THX                          _IOW (SC_IOCTL_MAGIC, 217, int) // FitchHsu 20081209 implement THX mode
#define IOCTL_SC_SET_COLOR_WASH_ENABLE 	          _IOW (SC_IOCTL_MAGIC, 218, int) // LGE [vivakjh] 2008/12/09 	For setting the PDP's Color Wash
#define IOCTL_SC_SET_ADAPTIVE_CGAIN               _IOW (SC_IOCTL_MAGIC, 219, int) //[090814_Leo]
#define IOCTL_SC_SET_PIECEWISE_ENABLE             _IOW (SC_IOCTL_MAGIC, 220, int) //[090825_Leo]
#define IOCTL_SC_SET_ADAPTIVE_CGAIN_EN            _IOW (SC_IOCTL_MAGIC, 221, int) //[090921_Leo]

// TCON IO number 241 ~ 250
#define IOCTL_SC_GET_TCONTAB_INFO      	          _IOWR(SC_IOCTL_MAGIC, 241, int)
#define IOCTL_SC_SET_TCONMAP         	          _IOWR(SC_IOCTL_MAGIC, 242, int)
#define IOCTL_SC_SET_TCONPOWERSEQUENCE            _IOWR(SC_IOCTL_MAGIC, 243, int)
#define IOCTL_SC_SET_TCONCOUNT_RESET              _IOWR(SC_IOCTL_MAGIC, 244, int)

#define IOCTL_SC_MAXNR    256

//------MCU use Scaler internal MPLL clock-------------------
#define MCU_CLOCK_SEL                   MCUCLK_144MHZ

#define MST_XTAL_CLOCK_HZ               FREQ_12MHZ
#define MST_XTAL_CLOCK_KHZ              (MST_XTAL_CLOCK_HZ/1000UL)
#define MST_XTAL_CLOCK_MHZ              (MST_XTAL_CLOCK_KHZ/1000UL)

// MCU clock
#define MCUCLK_XTAL     0x00
#define MCUCLK_172P8MHZ 0x01
#define MCUCLK_160MHZ   0x02
#define MCUCLK_144MHZ   0x03
#define MCUCLK_123MHZ   0x04
#define MCUCLK_108MHZ   0x05
#define MCUCLK_MEM      0x06
#define MCUCLK_MEMD2    0x07
#define MCUCLK_0        0x08
//Freq
#define FREQ_12MHZ              (12000000UL)
#define FREQ_14P318MHZ          (14318180UL)
#define FREQ_24MHZ              (24000000UL)

#endif // __DRV_SCLAER_IO_H__
