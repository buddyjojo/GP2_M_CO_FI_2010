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
///
/// @file   mhal_h264.h
/// @brief  H.264 Control Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _HAL_MVD_H_
#define _HAL_MVD_H_
#include "mdrv_h264.h"

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define DEBUG_H264      0
#define HVD_ENABLE_DECODE_TIME_ISR     1
#define HVD_ENABLE_ISR_POLL              1

#define HVD_MIU1_BASE   0x10000000UL

#define HVD_ALIGN_4(_x)                 (((U32)(_x) + 3) & ~3)

typedef enum
{
    USER_CMD_GO             = 0,
    USER_CMD_STOP           = 1,
    USER_CMD_PAUSE          = 2,
    USER_CMD_END            = 3,  //normal end
    USER_CMD_DROP           = 4,  //drop nal
    USER_CMD_COMPRESS       = 6,  //compress mode
    USER_CMD_SKIP8x8        = 7,  //skip8x8
    USER_CMD_OFF_PERF8x8    = 8,  //off perf8x8
    USER_CMD_DUALBUF        = 9,  //dual buffer mode
    USER_CMD_INTERFACE      = 10,  //for to interlace
    USER_CMD_PROGRESSIVE    = 11,  //for to progressive
    USER_CMD_BBU_SIZE       = 12,  //set bbu table threshold
    USER_CMD_STEP           = 13,  // decode one frame/field then pause
    USER_CMD_PARSER_BYPASS  = 14,
    USER_CMD_DEC_I          = 15,  //only decode I frame
    USER_CMD_FLUSH_QUEUE    = 16,  //flush dpb queue
    USER_CMD_DISP_CNT       = 17,  //set display count through mailbox1 //can't set msgbox set
    USER_CMD_FLUSH_DISPLAY  = 18,  //flush dpb queue and remove all display queue
    USER_CMD_FREE_RUN       = 19,  //disable av-sync

    USER_CMD_AV_TS          = (1 << 8),
    USER_CMD_AV_SEI         = (2 << 8),
    USER_CMD_AV_CONST       = (3 << 8),
}USER_CMD;

#define AVCH264_HI_MBOX0        0
#define AVCH264_HI_MBOX1        1
#define AVCH264_RISC_MBOX0      2
#define AVCH264_RISC_MBOX1      3

#define AVCH264_TSP_MODE        0
#define AVCH264_FILE_MODE       1

//reg mask define
#define MASK_REG_ESB_ST_ADR_L    0xFFFF
#define MASK_REG_ESB_ST_ADR_H    0x00FF
#define MASK_REG_ESB_LENGTH_L    0xFFFF
#define MASK_REG_ESB_LENGTH_H    0x0007



#define H264_FBINFO_BASE        0x30001F00
#define H264_PTS_CLK_ADDR       0x30001F08
#define H264_NALU_CNT_BASE      0x30001F58
#define H264_SIZEOF_DIRTYHRD    4   // bytes

#define H264_DISP_DESC_ADDR     (0x30001F20)
#define H264_SIZEOF_DISP_DESC   8   // bytes

#define H264_PIC_INFO_ADDR      (0x30001F38)
#define H264_SIZEOF_PIC_INFO    28  // bytes

#define H264_VUI_DISP_INFO_ADDR 0x30001F60

#define H264_AFD_INFO_ADDR      0x30001F7C
#define H264_AFD_ACTIVE_FORMAT_ADDR 0x30001F94

#define H264_INT_STATUS_ADDR    0x30001FD0

#if 1//def SATURN_IV_ENHANCE
#define H264_BBU_DRAM_ST_ADDR   (0x28000UL)  // bbu table from dram starting address
#define H264_BBU_DRAM_TBL_ENTRY  0x1200UL    // bbu entry. 128bits(16 bytes) every entry.
#define H264_BBU_DRAM_TBL_THR   (H264_BBU_DRAM_TBL_ENTRY - 2)
#endif

#define H264_STATE_TYPE_INIT            ((U8) 0)
#define H264_STATE_TYPE_WAIT_DISPLAY    ((U8) 1)
#define H264_STATE_TYPE_DISPLAYED       ((U8) 2)
#define H264_STATE_TYPE_IDLE            ((U8) 3)
#define H264_STATE_TYPE_SEQ_CHANGE      ((U8) 4)

#define H264_SEQ_INFO_VUI        0x01  // 1<<0
#define H264_SEQ_INFO_CROPPING   0x02  // 1<<1
#define H264_SEQ_INFO_MBS_ONLY   0x04  // 1<<2
#define H264_SEQ_INFO_CHORMA_IDC 0x08  // 1<<3
#define H264_SEQ_INFO_INTERLACE  0x10  // 1<<4

#define PIC_INFO_PANSCAN0        1
#define PIC_INFO_PANSCAN1        2
#define PIC_INFO_CC              4

#define H264_STATUS_USER_DATA        (1 << 0)
#define H264_STATUS_DATA_ERR         (1 << 1)
#define H264_STATUS_PIC_DEC_ERR      (1 << 2)
#define H264_STATUS_DEC_OVER         (1 << 3)
#define H264_STATUS_DEC_UNDER        (1 << 4)
#define H264_STATUS_DEC_I            (1 << 5)
#define H264_STATUS_DIS_READY        (1 << 6)
#define H264_STATUS_SEQ_INFO         (1 << 7)
#define H264_STATUS_VIDEO_SKIP       (1 << 8)
#define H264_STATUS_VIDEO_REPEAT     (1 << 9)
#define H264_STATUS_VIDEO_FREERUN    (1 << 10)
#define H264_STATUS_INVALID_STREAM   (1 << 11)
#define H264_STATUS_VIDEO_AVSYNC_DONE    (1 << 12)
#define H264_STATUS_VIDEO_VSYNC      (1 << 31)












#define MAIN_SD_MVD_MEM_START    0x80100000
#define MAIN_SD_MVD_MEM_SIZE     0x10000






typedef enum {

	H264_STATE_UNINIT=0,
	H264_STATE_INIT,
	H264_STATE_PLAY,
	H264_STATE_STOP,
	H264_STATE_PAUSE

} H264_STATE;


H264STATUS MHal_H264_PowerOn(void);
H264STATUS MHal_H264_PowerOff(void);
H264STATUS MHal_H264_Init(H264_INIT_PARAM* InitParam);
H264STATUS MHal_H264_Play(u8 frc);
H264STATUS MHal_H264_Pause(void);
H264STATUS MHal_H264_Stop(void);
H264STATUS MHal_H264_Decode_IFrame(u32 u32FrameBufAddr, u32 u32StreamBufAddr);
H264STATUS MHal_H264_GetDecdeFrameCount(u32* pu32FrameCount);
H264STATUS MHal_H264_GetPTS(u32* pu32PTSLow, u32* pu32PTSHigh);
H264STATUS MHal_H264_GetPictureData(AVCH264_FRAMEINFO* pPictureData);
H264STATUS MHal_H264_Reset(void);
H264STATUS MHal_H264_Reload(H264_INIT_PARAM* InitParam);
H264STATUS MHal_H264_ToggleAVSync(u8 u8Flag);
U32 MHal_H264_IsSeqChg(void);
void MHal_H264_GetActiveFormat(U8* active_format);

U16 Mhal_H264_SetIntSubscribe(U32 u32Flag);
H264STATUS MHal_H264_GetAvSyncStatus(u32* u32AVSyncStatus);
H264STATUS MHal_H264_SetPictureUserDataBuffer(U8 *pDataBuff, U32 u32SizeOfBuff);

H264STATUS MHal_H264_GetBitStreamBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff);
H264STATUS MHal_H264_GetSVDCodeBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff);
H264STATUS MHal_H264_GetFrameBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff);

H264STATUS MHal_H264_PVRPlayMode(H264_PVR_PlayMode PlayMode, U8  u8FrameRateUnit);
H264STATUS MHal_H264_SetFilePlay(U32 u32StartAddr, U32 u32EndAddr);
H264STATUS MHal_H264_SetFileClose(void);
H264STATUS MHal_H264_SetFilePlay2(U8 u8PlayFrameRate, U8  u8Mode);

U32 MHal_H264_GetPicCount(void);//H.264 update 090812
U8 MHal_H264_GetFirstFrame(void);
U32 MHal_H264_GetDispRdy(void);
U8 MHal_H264_GetSyncStatus(void);
U8 MHal_H264_SetVOPDone(void);
U8 MHal_H264_GetProgInt(void);
U8 MHal_H264_GetVideoSkip(void);
U8 MHal_H264_GetVideoRepeat(void);
U8 MHal_H264_IPicFound(void);
U32 MHal_H264_MEMRead(U32 u32Address);
void MHal_H264_MEMWrite(U32 u32Address, U32 u32Value);
void MHal_H264_MBox0Clear(void);
void MHal_H264_GetNextFrame(void);
U32 MHal_H264_DeRegisterInterrupt(void);
U32 MHal_H264_RegisterInterrupt(void);
H264STATUS MHal_H264_GetVUIInfo(VUI_DISP_INFO *vuiInfo);
H264STATUS MHal_H264_SetDelay(U32 delayTime);
BOOL MHal_H264_FastGetFrameInfo( AVCH264_FRAMEINFO *pinfo);

#if defined(UTOPIA_HVD_DRIVER)
U8 MHal_H264_GetPictType(void);
H264STATUS MHal_H264_SetDecodingSpeed(U32 Speed);
H264STATUS MHal_H264_StepDisplay(void);
U32 MHal_H264_GetDispQueueSize(void);
U32 MHal_H264_GetESDataSize(void);
H264STATUS MHal_H264_SetSyncRepeatTH(U32 Times);
H264STATUS MHal_H264_SetSyncThreshold(U32 threshold);
BOOL MHal_H264_CheckUserDataAvailable(void);
U32 MHal_H264_GetUserData( pH264_Data_Pkt *ppStrLast, U32 *pu32NumOfData );
H264STATUS MHal_H264_GetDecoderStatus(H264_Decoder_Status *status);
H264STATUS MHal_H264_RestartDecoder(void);

#endif //defined(UTOPIA_HVD_DRIVER)
//U8 MHal_MVD_PVRPlayMode(MVD_PVR_PlayMode, u8);
#endif
