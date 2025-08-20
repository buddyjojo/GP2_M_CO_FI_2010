#include "mdrv_types.h" // LGE drmyung 081014
#include "QualityMode.h"


#define PQMODE_DBG(x)   //x


U16 QM_InputSourceToIndex(PSC_SOURCE_INFO_t pSrcInfo)
{
    U16 u16SrcType;
    U16 u16Input_HSize = QM_GetInputHSize(pSrcInfo);
    U16 u16Input_VSize = QM_GetInputVSize(pSrcInfo);
    U16 u16Input_VFreq = QM_GetInputVFreq(pSrcInfo);
    BOOLEAN bIsInterlaced = QM_IsInterlaced(pSrcInfo);
    VIDEOSTANDARD_TYPE eStandard;

    U16 u16Disp_HSize = QM_GetDispHSize(pSrcInfo);
    U16 u16Disp_VSize = QM_GetDispVSize(pSrcInfo);


    //victor 20080822
    // VGA or Progressive DVI
    if (QM_IsSourceVGA(pSrcInfo) || (QM_IsSourceDVI_HDMIPC(pSrcInfo) && !bIsInterlaced))
    {
        //if (QM_HDMIPC_COLORYUV444(pSrcInfo)) //Add condition to prevent wrong index selection from HDMI to VGA, [100226_Leo]
        if (QM_IsSourceHDMI(pSrcInfo) && QM_HDMIPC_COLORYUV444(pSrcInfo))
        {
            // PC YUV444
            if (u16Input_HSize < u16Disp_HSize)
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_444_PC_Hup_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_444_PC_Hup_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_HDMI_444_PC_Hup_Vno_Main;
                }
            }
            else if (u16Input_HSize > u16Disp_HSize)
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_444_PC_Hdown_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_444_PC_Hdown_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_HDMI_444_PC_Hdown_Vno_Main;
                }
            }
            else
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_444_PC_Hno_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_444_PC_Hno_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_HDMI_444_PC_Hno_Vno_Main;
                }
            }
        }
        //else if (QM_HDMIPC_COLORYUV422(pSrcInfo)) //Add condition to prevent wrong index selection from HDMI to VGA, [100226_Leo]
        else if (QM_IsSourceHDMI(pSrcInfo) && QM_HDMIPC_COLORYUV422(pSrcInfo))
        {
            // PC YUV422
            if (u16Input_HSize < u16Disp_HSize)
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_422_PC_Hup_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_422_PC_Hup_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_HDMI_422_PC_Hup_Vno_Main;
                }
            }
            else if (u16Input_HSize > u16Disp_HSize)
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_422_PC_Hdown_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_422_PC_Hdown_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_HDMI_422_PC_Hdown_Vno_Main;
                }
            }
            else
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_422_PC_Hno_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_HDMI_422_PC_Hno_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_HDMI_422_PC_Hno_Vno_Main;
                }
            }
        }
        else //(QM_HDMIPC_COLORRGB(pSrcInfo)) & default
        {
            // PC RGB
            if (u16Input_HSize < u16Disp_HSize)
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hup_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hup_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hup_Vno_Main;
                }
            }
            else if (u16Input_HSize > u16Disp_HSize)
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hdown_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hdown_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hdown_Vno_Main;
                }
            }
            else
            {
                if (u16Input_VSize < u16Disp_VSize)
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hno_Vup_Main;
                }
                else if (u16Input_VSize > u16Disp_VSize)
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hno_Vdown_Main;
                }
                else
                {
                    u16SrcType = QM_DVI_Dsub_HDMI_RGB_PC_Hno_Vno_Main;
                }
            }
        }
    }
    //victor 20080822
    // HDMI or Interlance DVI
    else if (QM_IsSourceHDMI(pSrcInfo) || (QM_IsSourceDVI_HDMIPC(pSrcInfo) && bIsInterlaced))
    {
        // HDMI
        if ((u16Input_HSize < 800) && (u16Input_VSize < 500))
        {
            if (bIsInterlaced)
                u16SrcType = QM_HDMI_480i_Main;
            else
                u16SrcType = QM_HDMI_480p_Main;
        }
        else if ((u16Input_HSize < 800) && (u16Input_VSize < 600))
        {
            if (bIsInterlaced)
                u16SrcType = QM_HDMI_576i_Main;
            else
                u16SrcType = QM_HDMI_576p_Main;
        }
#if 0   //victor 20081001
        else if ((u16Input_HSize < 1100) && (u16Input_VSize < 800))//victor 20080930
        {
            // Closest setting is HDMI 480i
            if (bIsInterlaced)
                u16SrcType = QM_HDMI_480i_Main;
            else
                u16SrcType = QM_HDMI_480p_Main;
        }
#endif
        else if ((u16Input_HSize < 1300) && (u16Input_VSize < 800) && !bIsInterlaced)//victor 20081001
        {
            if (u16Input_VFreq > 550)
            {
                u16SrcType = QM_HDMI_720p_60hz_Main;
            }
            else if(u16Input_VFreq > 250)
            {
                u16SrcType = QM_HDMI_720p_50hz_Main;
            }
            else
            {
                u16SrcType = QM_HDMI_720p_24hz_Main;
            }
        }
        else
        {
            if (bIsInterlaced)
            {
                if (u16Input_VFreq > 550)
                {
                    u16SrcType = QM_HDMI_1080i_60hz_Main;
                }
                else
                {
                    u16SrcType = QM_HDMI_1080i_50hz_Main;
                }
            }
            else
            {
                if (u16Input_VFreq > 550)
                {
                    u16SrcType = QM_HDMI_1080p_60hz_Main;
                }
                else if(u16Input_VFreq > 250)
                {
                    u16SrcType = QM_HDMI_1080p_50hz_Main;
                }
                else
                {
                    u16SrcType = QM_HDMI_1080p_24hz_Main;
                }
            }
        }
    }
    else if (QM_IsSourceYPbPr(pSrcInfo))
    {
        // YPbPr
        if (u16Input_VSize < 500)
        {
            if (bIsInterlaced)
                u16SrcType = QM_YPbPr_480i_Main;
            else
                u16SrcType = QM_YPbPr_480p_Main;
        }
        else if (u16Input_VSize < 650)
        {
            if (bIsInterlaced)
                u16SrcType = QM_YPbPr_576i_Main;
            else
                u16SrcType = QM_YPbPr_576p_Main;
        }
        else if (u16Input_VSize < 900)
        {
            if (u16Input_VFreq > 550)
            {
                u16SrcType = QM_YPbPr_720p_60hz_Main;
            }
            else if(u16Input_VFreq > 250)
            {
                u16SrcType = QM_YPbPr_720p_50hz_Main;
            }
            else
            {
                u16SrcType = QM_YPbPr_720p_24hz_Main;
            }
        }
        else
        {
            if (bIsInterlaced)
            {
                if (u16Input_VFreq > 550)
                {
                    u16SrcType = QM_YPbPr_1080i_60hz_Main;
                }
                else
                {
                    u16SrcType = QM_YPbPr_1080i_50hz_Main;
                }
            }
            else
            {
                if (u16Input_VFreq > 550)
                {
                    u16SrcType = QM_YPbPr_1080p_60hz_Main;
                }
                else if(u16Input_VFreq > 250)
                {
                    u16SrcType = QM_YPbPr_1080p_50hz_Main;
                }
                else
                {
                    u16SrcType = QM_YPbPr_1080p_24hz_Main;
                }
            }
        }
    }
    else if (QM_IsSourceDTV(pSrcInfo))
    {   // 20091009 daniel.huang: fix 1920x800p video enter to 720p mode
        if (QM_IsDTV_MPEG2(pSrcInfo))
        {
            if (u16Input_HSize <= 720 && u16Input_VSize <= 480)
            {
                if ((u16Input_HSize <= 352) && bIsInterlaced)
                {
                    u16SrcType = QM_DTV_480i_352x480_MPEG2_Main;
                }
                else
                {
                    if (bIsInterlaced)
                        u16SrcType = QM_DTV_480i_MPEG2_Main;
                    else
                        u16SrcType = QM_DTV_480p_MPEG2_Main;
                }
            }
            else if (u16Input_HSize <= 720) // the other case think as 576i/p
            {
                if (bIsInterlaced)
                    u16SrcType = QM_DTV_576i_MPEG2_Main;
                else
                    u16SrcType = QM_DTV_576p_MPEG2_Main;
            }
            else if (u16Input_HSize <= 1280 && u16Input_VSize <= 720 && (bIsInterlaced==0))
            {
                if (u16Input_VFreq > 550)
                {
                    u16SrcType = QM_DTV_720p_60hz_MPEG2_Main;
                }
                else if(u16Input_VFreq > 250)
                {
                    u16SrcType = QM_DTV_720p_50hz_MPEG2_Main;
                }
                else
                {
                    u16SrcType = QM_DTV_720p_24hz_MPEG2_Main;
                }
            }
            else
            {
                if (bIsInterlaced)
                {
                    if (u16Input_VFreq > 550)
                    {
                        u16SrcType = QM_DTV_1080i_60hz_MPEG2_Main;
                    }
                    else
                    {
                        u16SrcType = QM_DTV_1080i_50hz_MPEG2_Main;
                    }
                }
                else
                {
                    if (u16Input_VFreq > 550)
                    {
                        u16SrcType = QM_DTV_1080p_60hz_MPEG2_Main;
                    }
                    else if(u16Input_VFreq > 250)
                    {
                        u16SrcType = QM_DTV_1080p_50hz_MPEG2_Main;
                    }
                    else
                    {
                        u16SrcType = QM_DTV_1080p_24hz_MPEG2_Main;
                    }
                }
            }
        }
        else // QM_IsDTV_H264
        {   // 20091009 daniel.huang: fix 1920x800p video enter to 720p mode
            if (u16Input_HSize <= 720 && u16Input_VSize <= 480)
            {
                if ((u16Input_HSize <= 352) && bIsInterlaced)
                {
                    u16SrcType = QM_DTV_480i_352x480_H264_Main;
                }
                else
                {
                    if (bIsInterlaced)
                        u16SrcType = QM_DTV_480i_H264_Main;
                    else
                        u16SrcType = QM_DTV_480p_H264_Main;
                }
            }
            else if (u16Input_HSize <= 720) // the other case think as 576i/p
            {
                if (bIsInterlaced)
                    u16SrcType = QM_DTV_576i_H264_Main;
                else
                    u16SrcType = QM_DTV_576p_H264_Main;
            }
            else if (u16Input_HSize <= 1280 && u16Input_VSize <= 720 && (bIsInterlaced==0))
            {
                if (u16Input_VFreq > 550)
                {
                    u16SrcType = QM_DTV_720p_60hz_H264_Main;
                }
                else if(u16Input_VFreq > 250)
                {
                    u16SrcType = QM_DTV_720p_50hz_H264_Main;
                }
                else
                {
                    u16SrcType = QM_DTV_720p_24hz_H264_Main;
                }
            }
            else
            {
                if (bIsInterlaced)
                {
                    if (u16Input_VFreq > 550)
                    {
                        u16SrcType = QM_DTV_1080i_60hz_H264_Main;
                    }
                    else
                    {
                        u16SrcType = QM_DTV_1080i_50hz_H264_Main;
                    }
                }
                else
                {
                    if (u16Input_VFreq > 550)
                    {
                        u16SrcType = QM_DTV_1080p_60hz_H264_Main;
                    }
                    else if(u16Input_VFreq > 250)
                    {
                        u16SrcType = QM_DTV_1080p_50hz_H264_Main;
                    }
                    else
                    {
                        u16SrcType = QM_DTV_1080p_24hz_H264_Main;
                    }
                }
            }
        }
    }
    else if (QM_IsSourceMultiMedia(pSrcInfo))
    {
        if (QM_IsMultiMediaMOVIE(pSrcInfo))
        {
            if (u16Input_HSize <= 720 && u16Input_VSize <= 576)
            {
                if (bIsInterlaced)
                {
                    u16SrcType = QM_Multimedia_video_SD_interlace_Main;
                }
                else
                {
                    u16SrcType = QM_Multimedia_video_SD_progressive_Main;
                }
            }
            else
            {
                if (bIsInterlaced)
                {
                    u16SrcType = QM_Multimedia_video_HD_interlace_Main;
                }
                else
                {
                    u16SrcType = QM_Multimedia_video_HD_progressive_Main;
                }
            }
        }
        else //QM_IsMultiMediaPHOTO(pSrcInfo)
        {
            if (u16Input_HSize <= 720 && u16Input_VSize <= 576)
            {
                u16SrcType = QM_Multimedia_photo_SD_progressive_Main;
            }
            else
            {
                u16SrcType = QM_Multimedia_photo_HD_progressive_Main;
            }
        }
    }
    else if (QM_IsSourceScartRGB(pSrcInfo))
    {
        if (u16Input_VFreq > 550)
        {
            u16SrcType = QM_SCART_RGB_NTSC_Main;
        }
        else
        {
            u16SrcType = QM_SCART_RGB_PAL_Main;
        }
    }
    else if (QM_IsSourceScartCVBS(pSrcInfo))
    {
        eStandard = QM_GetATVStandard(pSrcInfo);
        switch (eStandard)
        {
            case E_VIDEOSTANDARD_PAL_M:
                u16SrcType = QM_SCART_AV_PAL_M_Main;
                break;
            case E_VIDEOSTANDARD_PAL_N:
                u16SrcType = QM_SCART_AV_PAL_N_Main;
                break;
            case E_VIDEOSTANDARD_NTSC_44:
                u16SrcType = QM_SCART_AV_NTSC_44_Main;
                break;
            case E_VIDEOSTANDARD_PAL_60:
                u16SrcType = QM_SCART_AV_PAL_60_Main;
                break;
            case E_VIDEOSTANDARD_NTSC_M:
                u16SrcType = QM_SCART_AV_NTSC_M_Main;
                break;
            case E_VIDEOSTANDARD_SECAM:
                u16SrcType = QM_SCART_AV_SECAM_Main;
                break;
            case E_VIDEOSTANDARD_PAL_BGHI:
            default:
                u16SrcType = QM_SCART_AV_PAL_BGHI_Main;
                break;
        }
    }
    else if (QM_IsSourceATV(pSrcInfo))
    {
        eStandard = QM_GetATVStandard(pSrcInfo);
        switch(eStandard)
        {
        case E_VIDEOSTANDARD_PAL_M:
            u16SrcType = QM_RF_PAL_M_Main;
            break;
        case E_VIDEOSTANDARD_PAL_N:
            u16SrcType = QM_RF_PAL_N_Main;
            break;
        case E_VIDEOSTANDARD_NTSC_44:
            u16SrcType = QM_RF_NTSC_44_Main;
            break;
        case E_VIDEOSTANDARD_PAL_60:
            u16SrcType = QM_RF_PAL_60_Main;
            break;
        case E_VIDEOSTANDARD_NTSC_M:
            u16SrcType = QM_RF_NTSC_M_Main;
            break;
        case E_VIDEOSTANDARD_SECAM:
            u16SrcType = QM_RF_SECAM_Main;
            break;
        case E_VIDEOSTANDARD_PAL_BGHI:
        default:
            u16SrcType = QM_RF_PAL_BGHI_Main;
            break;
        }
    }
    else if (QM_IsSourceSV(pSrcInfo))
    {
        eStandard = QM_GetATVStandard(pSrcInfo);
        switch(eStandard)
        {
        case E_VIDEOSTANDARD_PAL_M:
            u16SrcType = QM_SV_PAL_M_Main;
            break;
        case E_VIDEOSTANDARD_PAL_N:
            u16SrcType = QM_SV_PAL_N_Main;
            break;
        case E_VIDEOSTANDARD_NTSC_44:
            u16SrcType = QM_SV_NTSC_44_Main;
            break;
        case E_VIDEOSTANDARD_PAL_60:
            u16SrcType = QM_SV_PAL_60_Main;
            break;
        case E_VIDEOSTANDARD_NTSC_M:
            u16SrcType = QM_SV_NTSC_M_Main;
            break;
        case E_VIDEOSTANDARD_SECAM:
            u16SrcType = QM_SV_SECAM_Main;
            break;
        case E_VIDEOSTANDARD_PAL_BGHI:
        default:
            u16SrcType = QM_SV_PAL_BGHI_Main;
            break;
        }
    }
    else // AV
    {
        eStandard = QM_GetATVStandard(pSrcInfo);
        switch(eStandard)
        {
        case E_VIDEOSTANDARD_PAL_M:
            u16SrcType = QM_AV_PAL_M_Main;
            break;
        case E_VIDEOSTANDARD_PAL_N:
            u16SrcType = QM_AV_PAL_N_Main;
            break;
        case E_VIDEOSTANDARD_NTSC_44:
            u16SrcType = QM_AV_NTSC_44_Main;
            break;
        case E_VIDEOSTANDARD_PAL_60:
            u16SrcType = QM_AV_PAL_60_Main;
            break;
        case E_VIDEOSTANDARD_NTSC_M:
            u16SrcType = QM_AV_NTSC_M_Main;
            break;
        case E_VIDEOSTANDARD_SECAM:
            u16SrcType = QM_AV_SECAM_Main;
            break;
        case E_VIDEOSTANDARD_PAL_BGHI:
        default:
            u16SrcType = QM_AV_PAL_BGHI_Main;
            break;
        }
    }

	//printk("u16SrcType = %d\n",u16SrcType);
    return u16SrcType;
}


