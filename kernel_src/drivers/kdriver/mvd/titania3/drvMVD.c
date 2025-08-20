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
/// @brief MPEG-2/4 Video Decoder Driver
/// @author MStar Semiconductor Inc.
///
////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#ifdef REDLION_LINUX_KERNEL_ENVI
#include "drvMVD_Common.h"
#include "drvMVD.h"
#include "regMVD.h"
#include "halMVD.h"
#include "halVPU.h"
#else

// Common Definition
#include "MsCommon.h"
#include "MsVersion.h"
#include "halCHIP.h"
#include "drvMMIO.h"
#include "drvBDMA.h"
#include "drvMVD.h"

// Internal Definition
#include "regMVD.h"
#include "halMVD.h"
#include "asmCPU.h"
#include "osalMVD.h"
#include "drvmvd_cc.h"      //ATSC Closed Caption
#endif


////////////////////////////////////////////////////////////////////////////////
// Local defines & local structures
////////////////////////////////////////////////////////////////////////////////
//Different level debug message
#define MVD_DEBUGVERBAL(x)    if (_u8DbgLevel>4)  { (x); }
#define MVD_DEBUGINFO(x)      if (_u8DbgLevel>1)  { (x); }
#define MVD_DEBUGERROR(x)     if (_u8DbgLevel>0)  { (x); }
#define MVD_FUNC_ENTRY()      {}//printf(" $$$ drvMVD::%s\n", __FUNCTION__)
#define _SLQTBL_DUMP_PTS      FALSE//TRUE
#define _SLQTBL_DUMP_PUSHQ    FALSE//TRUE
#define _SLQTBL_DUMP_PKT      FALSE//TRUE
#define _STEPDISP_DISABLE_AVSYNC FALSE//TRUE
#define _DUMP_FRMINFO         FALSE
#define MVD_TURBO_INIT        FALSE//TRUE
#define MVD_PROFILING         FALSE//TRUE

#if (defined(CHIP_T3) || defined(CHIP_U3) || defined(CHIP_JANUS) || defined(CHIP_T4))
#define _MVD4 //mvd4 or mvd4p
#endif

#define _MS_TO_90K(x)   (x*90)  //ms ==> 90k counter
#define _90K_TO_MS(x)   (x/90)  //90k counter ==> ms


#define _MVD_Memset(pDstAddr, u32value, u32Size)                        \
            do {                                                        \
                MS_U32 i = 0;                                           \
                for (i = 0; i < (u32Size/4); i=i+4)                     \
                {                                                       \
                    HAL_MVD_MemWrite4Byte(pDstAddr+i,u32value);         \
                }                                                       \
            } while (0)

#define _MVD_Memcpy(pDstAddr, pSrcAddr, u32Size)                        \
            do {                                                        \
                MS_U32 i = 0;                                           \
                volatile MS_U8 *dst = (volatile MS_U8 *)(pDstAddr);     \
                volatile MS_U8 *src = (volatile MS_U8 *)(pSrcAddr);     \
                for (i = 0; i < (u32Size); i++)                         \
                {                                                       \
                    dst[i] = src[i];                                    \
                }                                                       \
            } while (0)


#define MVD_U32_MAX                 0xffffffffUL
#define MVD_NULLPKT_PTS             MVD_U32_MAX

#define SLQ_ENTRY_MAX               1024
#define SLQ_ENTRY_LEN               8   //8-byte per entry
#define SLQ_TBL_SIZE                (SLQ_ENTRY_MAX * SLQ_ENTRY_LEN)
#define ES_TBL_SIZE                 (SLQ_ENTRY_MAX * 8) //8-byte per entry

#define MVD_FW_SLQTBL_PTS_LEN       32

#define SLQ_TBL_SAFERANGE              (20*SLQ_ENTRY_LEN)
//this should be smaller than FW's lookup range (current it's 16 entries)

#define SLQTBL_CHECKVACANCY_WATERLEVEL (24*SLQ_ENTRY_LEN)

#define DIVX_PATTERN    0x63643030
#define FLV_PATTERN     0xffff0000
#define MPEG_PATTERN_0  0xC6010000 //this SC just for mpeg2/4

#define VC1_PATTERN_0   0xff010000
#define VC1_PATTERN_1   0x0000ffff
#define VC1_PATTERN_2   0xffffff01

#define RCV_PATTERN_0   0x00000000
#define RCV_PATTERN_1   0xffffffff
#define RCV_PATTERN_2   0x00000000

#define DUMMY_PATTERN   0xBE010000
#define DUMMY_SIZE      0x2000     //8K

#define END_PATTERN_0   0xFF010000
#define END_PATTERN_1   0xDDCCBBAA
#define END_PATTERN_2   0xBBAAFFEE
#define END_PATTERN_3   0xFFEEDDCC
#define END_PATTERN_SIZE 256

#define SKIP_PATTERN_0  0xc5010000
#define SKIP_PATTERN_1  0x270608ab
#define SKIP_PATTERN_SIZE 8
#define HW_FIFO_DEPTH   256
#define CMD_TIMEOUT_MS  500

//Length of internal buffers
#define MVD3_FW_IAP_BUF_LEN                 (0x4000UL)   // 16k
#define MVD3_FW_DP_BUF_LEN                  (0x40000UL)  //256k (1920/16) * (1088/16)* 4 * 8 = 255KB
#define MVD3_FW_MV_BUF_LEN                  (0x40000UL)  //256K ((1920/16)*4*1.5+((1920/16)*(1088/16)*4*1.5)*4
#define MVD3_FW_VOL_INFO_BUF_LEN            (0x1000UL)   //  4K
#define MVD3_FW_FRAME_INFO_BUF_LEN          (0x1000UL)   //  4K
#define MVD3_FW_DIVX_INFO_BUF_LEN           (0x1000UL)   //  4K
#define MVD3_FW_USER_DATA_BUF_LEN           (0x1000UL)   //  4K
#define MVD3_FW_USER_DATA_BUF_BACKUP_LEN    MVD3_FW_USER_DATA_BUF_LEN // 4K
#define MVD3_FW_SLQ_TAB_BUF_LEN             (0x2000UL)   //  8K
#define MVD3_FW_SLQ_TAB_TMPBUF_LEN          (0x200UL)

#define MVD_FW_SLQTBL_PTS_BUF_LEN           (SLQ_ENTRY_MAX*MVD_FW_SLQTBL_PTS_LEN)

#define MVD_FW_DYN_SCALE_BUF_LEN            (0x1000UL)   //  4K
#define MVD_FW_SCALER_INFO_BUF_LEN          (0x100UL)    // 256bytes reserved

//Table of frame rate code for MPEG-2
static const MS_U16 stFrameRateCode[9]=
{
    NULL,23976,24000,25000,29976,30000,50000,59947,60000
};

typedef struct _MVD_SLQ_TBL_ST
{
    MS_U32 u32StAdd;
    MS_U32 u32EndAdd;
    MS_U32 u32EntryCntMax;

    MS_U32 u32RdPtr;
    MS_U32 u32WrPtr;
    MS_U32 u32Empty;
} MVD_SLQ_TBL_ST;

typedef struct _MVD_SLQ_ES_ST
{
    MS_U32 u32StAdd;
    MS_U32 u32EndAdd;

    MS_U32 u32RdPtr;
    MS_U32 u32WrPtr;
} MVD_SLQ_ES_ST;

typedef struct _MVD_CMD_QUEUE
{
    MS_U32 u32PtsBase;
} MVD_CMD_QUEUE;

////////////////////////////////////////////////////////////////////////////////
// Local Global Variables
////////////////////////////////////////////////////////////////////////////////
/// Version string
static MSIF_Version _drv_mvd_version = {
    .DDI = { MVD_DRV_VERSION, },
};

static MS_U32 pu8MVDSetHeaderBufStart=NULL;
static MS_U32 pu8MVDGetVolBufStart=NULL;
static MS_U32 pu8MVDGetFrameInfoBufStart=NULL;
static MS_U32 u32MVDSLQTBLPostion = NULL;
static MS_U32 u32MVDFWSLQTABTmpbufAdr = NULL;
static MS_U32 u32MVDFWPtsTblAddr = NULL;
static MS_U32 u32DecFrmInfoAdd = NULL;
static MS_U32 u32DynScalingAdd = NULL;
static MS_U32 u32ScalerInfoAdd = NULL;
static MS_U8  u8DynScalingDepth = 0;
static MS_U32 u32MVDFirstVOLUpdate=FALSE;
static MS_U32 u32VolAdd;

static FW_DIVX_INFO gdivxInfo;
#if (!defined(CHIP_T7))
static FW_VOL_INFO gvolInfo;
#endif
static MS_BOOL _bDecodeIFrame;
static MS_BOOL _bDrvInit;

static MS_U8 u8FBMode;
static MVD_TrickDec eTrickMode = E_MVD_TRICK_DEC_UNKNOWN;
static MS_BOOL bAVSyncOn = FALSE;

static MS_U8 _u8DbgLevel = 0;
static MVD_DrvInfo _stDrvInfo = {
                                    1,      //1 MVD HW
                                    0,      //Device#   fixme
                                    MVD_FW_VERSION,        //firmware version
                                    { FALSE, FALSE, FALSE } //capability
                                };

static MVD_FrameInfo stPreFrmInfo;
static MVD_CodecType curCodecType;
static MVD_SrcMode curSrcMode;
MVD_MEMCfg stMemCfg;
static MS_BOOL bStepDecode = FALSE;
static MS_U32 u32StepDecodeCnt = 0;
static MS_BOOL bStepDisp = FALSE;
static MS_BOOL bStep2Pts = FALSE;
static MS_BOOL bSkip2Pts = FALSE;
static MS_BOOL bEnableLastFrmShow = FALSE;
static MS_U32 u32LastPts = 0;
static MS_U32 u32DummyPktCnt = 0;
static MS_U32 u32SlqByteCnt = 0;
static MS_BOOL bSlqTblSync = FALSE; //need to BDMA SLQ table from DrvProcBuff to BitstreamBuff
static MS_U32 u32FileEndPtr;

#ifndef REDLION_LINUX_KERNEL_ENVI
static BDMA_CpyType bdmaCpyType = E_BDMA_CPYTYPE_MAX;
#endif
//static MVD_PlayMode ePlayMode = E_MVD_UNKNOWMODE;

static MVD_SLQ_TBL_ST _drvSlqTbl;
static MS_U32 u32PreEsRd = 0;
static MS_U32 u32PreEsWr = 0;

static MVD_SLQ_ES_ST _drvEsTbl;
static MVD_SLQ_ES_ST _drvDivxTbl;

static MVD_CMD_QUEUE _mvdCmdQ;

static MVD_TIMESTAMP_TYPE _eFileSyncMode;
static MVD_AVSyncCfg stSyncCfg;
static MVD_AVSyncCfg stSyncCfgBfStepDisp;
static MVD_SpeedType ePreSpeedType = E_MVD_SPEED_DEFAULT;
#if defined(CHIP_T7)
static MS_U8 u8MvdPlayMode = 0;
#endif

#ifdef MVD_ENABLE_ISR
MVD_InterruptCb pfnCallback;
static MVD_Event eEventFlag;
static MVD_Event eCurEvent;
#endif

static MVD_InternalMemCfg _stInternalMemCfg;

////////////////////////////////////////////////////////////////////////////////
// Local functions
////////////////////////////////////////////////////////////////////////////////
static MS_BOOL MVD_MVDSetInternalBuffAddr(MS_U32 u32start, MS_U32 u32len);
static void MVD_InitVar(void);
static MS_BOOL MVD_Init(void);
static MS_U8 MVD_MapCodecType(MVD_CodecType type);
static MS_U8 MVD_MapSrcMode(MVD_SrcMode mode);
static void MVD_WriteDivx311Data(FW_DIVX_INFO *divxInfo);
//static void MVD_ReadVolInfo(FW_VOL_INFO *volInfo);
static MVD_ErrStatus MVD_GetErrShapeStat(MS_U32 u32errStat);
static MVD_ErrStatus MVD_GetErrSpriteStat(MS_U32 u32errStat);
static MS_U8 MVD_MapFrcMode(MVD_FrcMode eFrcMode);

#ifndef REDLION_LINUX_KERNEL_ENVI
static MS_BOOL MVD_CCGetIsActiveFormat(MVD_CCFormat eCCFormat);
#endif

static MS_BOOL MVD_SLQTblSendPacket(MVD_PacketInfo *pstVideoPKT);
static void MVD_SLQTblGetDivxHdrPkt(MVD_PacketInfo* pDivxHdr, MVD_PacketInfo* pDivxData);
static MS_BOOL MDrv_MVD_Stop(void);
static void MVD_SLQTblInit(void);
//static void _SLQTbl_DumpInfo(MVD_SLQ_TBL_ST* pInfo);
static MS_U32 MVD_GetMemOffset(MS_PHYADDR u32PhyAdd);
static MS_U32 MVD_Map2DrvSlqTbl(MS_U32 u32HWPtr);
static void MVD_SLQTblGetDummyPkt(MVD_CodecType eType, MVD_PacketInfo* pDummy);
static void MVD_SLQTblGetFileEndPkt(MVD_CodecType eType, MVD_PacketInfo* pFileEnd);
static MS_BOOL MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_CMD eCmd);
#if (defined(CHIP_T3)||defined(CHIP_U3))
static MS_BOOL MDrv_MVD_ClearCmdFinished(MVD_HANDSHAKE_CMD eCmd);
#endif
static MS_BOOL MVD_SLQTblInsertPattern(MVD_PatternType ePattern);
#if _SLQTBL_DUMP_PTS
static void _SLQTbl_DumpPtsTbl(MS_U32 u32EntryStart, MS_U32 u32EntryEnd);
#endif

#if _DUMP_FRMINFO
static void MVD_DumpFrmInfo(MVD_FrmInfo* pInfo)
{
    if (NULL == pInfo)
    {
        printf("%s: pInfo invalid!\n", __FUNCTION__);
        return;
    }

    printf("u32LumaAddr  =0x%lx\n", pInfo->u32LumaAddr  );
    printf("u32ChromaAddr=0x%lx\n", pInfo->u32ChromaAddr);
    printf("u32TimeStamp =0x%lx\n", pInfo->u32TimeStamp );
    printf("u32ID_L      =0x%lx\n", pInfo->u32ID_L      );
    printf("u32ID_H      =0x%lx\n", pInfo->u32ID_H      );
    printf("u16Pitch     =0x%x\n", pInfo->u16Pitch      );
    printf("u16Width     =0x%x\n", pInfo->u16Width      );
    printf("u16Height    =0x%x\n", pInfo->u16Height     );
    printf("eFrmType     =0x%x\n", pInfo->eFrmType      );
    return;
}
#endif


//------------------------------------------------------------------------------
/// Get MVD driver information
/// @return -the pointer to the driver information
//------------------------------------------------------------------------------
const MVD_DrvInfo* MDrv_MVD_GetInfo(void)
{
    MDrv_MVD_GetCaps(&_stDrvInfo.stCaps);
    return (&_stDrvInfo);
}


//------------------------------------------------------------------------------
/// Get MVD driver version
/// @return -the pointer to the driver version
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_GetLibVer(const MSIF_Version **ppVersion)
{
    if (!ppVersion)
        return E_MVD_RET_FAIL;

    *ppVersion = &_drv_mvd_version;
    return E_MVD_RET_OK;
}

//------------------------------------------------------------------------------
/// Get MVD driver status
/// @return -the pointer to the driver status
//------------------------------------------------------------------------------
void MDrv_MVD_GetStatus(MVD_DrvStatus* pDrvStatus)
{
    if (!pDrvStatus)
       return;

    if ((pDrvStatus->u32FWVer = MDrv_MVD_GetFWVer()) == 0) //timeout to get FW version
    {
        pDrvStatus->bIsBusy = TRUE;
    }
    else
    {
        pDrvStatus->bIsBusy = FALSE;
    }
    pDrvStatus->eDecStat = MDrv_MVD_GetDecodeStatus();
    pDrvStatus->u8LastFWCmd = MDrv_MVD_GetLastCmd();
    return;
}


//------------------------------------------------------------------------------
/// Set detailed level of MVD driver debug message
/// 0: None, 1: MVD_DEBUGERROR, 2: MVD_DEBUGINFO
/// @param level  \b IN  level from 0 to 2
//------------------------------------------------------------------------------
void MDrv_MVD_SetDbgLevel(MS_U8 level)
{
    _u8DbgLevel = level;
    if (level >= 2)
    {
        HAL_MVD_SetDbgLevel(level-1);
    }
    return;
}


//------------------------------------------------------------------------------
/// Get MVD firmware version
/// @return -firmware version
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetFWVer(void)
{
    return HAL_MVD_GetFWVer();
}

//------------------------------------------------------------------------------
/// Set SLQ table buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void MDrv_MVD_SetSLQTblBufStartEnd(MS_U32 u32start, MS_U32 u32end)
{
    MVD_FUNC_ENTRY();
    MVD_DEBUGINFO(printf("%s st=0x%lx end=0x%lx\n", __FUNCTION__, u32start, u32end));
    HAL_MVD_SetSLQTblBufStartEnd(u32start, u32end);
    return;
}


//------------------------------------------------------------------------------
/// Set internal buffer address to MVD
/// @param -u32addr \b IN : start address
/// @param -u32len  \b IN : length
//------------------------------------------------------------------------------
static MS_BOOL MVD_MVDSetInternalBuffAddr(MS_U32 u32start, MS_U32 u32len)
{
#if(defined(CHIP_T7))//fix me later
      return __MVDSetInternalBuffAddr(u32start,u32len);
#else
    MS_U32 tmpAdr, tmpLen;
    MS_U32 i;

    tmpAdr = u32start;

    tmpAdr += MVD_FW_CODE_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x02000));//8k align

#if(defined(CHIP_T2))
    MVD_DEBUGINFO(printf("set MVD3_FW_IAP_BUF_ADR =%lx\n",tmpAdr));
    HAL_MVD_SetIAPBufferAddr(tmpAdr);
    tmpAdr += MVD3_FW_IAP_BUF_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x08));//8-byte align

    MVD_DEBUGINFO(printf("set MVD3_FW_DP_BUF_ADR=%lx\n",tmpAdr));
    HAL_MVD_SetDPBufferAddr(tmpAdr);
    tmpAdr += MVD3_FW_DP_BUF_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x0800));//2k align

    MVD_DEBUGINFO(printf("set MVD3_FW_MV_BUF_ADR=%lx\n",tmpAdr));
    HAL_MVD_SetMVBufferAddr(tmpAdr);
    tmpAdr += MVD3_FW_MV_BUF_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x1000));//4k align
#endif

    MVD_DEBUGINFO(printf("set MVD3_FW_VOL_INFO_BUF_ADR 2=%lx\n",tmpAdr));
    HAL_MVD_SetVolInfoBufferAddr(tmpAdr);

    pu8MVDGetVolBufStart = tmpAdr;
    tmpAdr += MVD3_FW_VOL_INFO_BUF_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x1000));//4k align

    for (i=0; i<MVD3_FW_VOL_INFO_BUF_LEN; i+=4)
    {
        HAL_MVD_MemWrite4Byte(pu8MVDGetVolBufStart+i, 0x00UL);
    }

#if 1
    u32VolAdd = pu8MVDGetVolBufStart;

    if (stMemCfg.bFWMiuSel == MIU_SEL_1)
    {
        u32VolAdd += stMemCfg.u32Miu1BaseAddr;
    }

    u32VolAdd = HAL_MVD_PA2NonCacheSeg(u32VolAdd);

    MVD_DEBUGVERBAL(printf("gvolInfo = 0x%lx, volBuf=0x%lx\n", u32VolAdd, pu8MVDGetVolBufStart));
#endif

    MVD_DEBUGINFO(printf("set MVD3_FW_FRAME_INFO_BUF_ADR 3=%lx\n",tmpAdr));
    HAL_MVD_SetFrameInfoBufferAddr(tmpAdr);

    pu8MVDGetFrameInfoBufStart = tmpAdr;
    tmpAdr += MVD3_FW_FRAME_INFO_BUF_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x1000));//4k align

    for (i=0; i<MVD3_FW_FRAME_INFO_BUF_LEN; i+=4)
    {
        HAL_MVD_MemWrite4Byte(pu8MVDGetFrameInfoBufStart+i, 0x00UL);
    }

    MVD_DEBUGINFO(printf("set MVD3_FW_DIVX_INFO_BUF_ADR=%lx\n",tmpAdr));
    HAL_MVD_SetHeaderBufferAddr(tmpAdr);

    pu8MVDSetHeaderBufStart = tmpAdr;
    tmpAdr += MVD3_FW_DIVX_INFO_BUF_LEN;
    //set user data
    tmpAdr = (MemAlign(tmpAdr, 0x1000));//4k align

    MVD_DEBUGINFO(printf("set MVD3_FW_USER_DATA_BUF_ADR 4 =%lx\n",tmpAdr));
    HAL_MVD_SetUserDataBuf(tmpAdr, 0x1000UL);
    _stInternalMemCfg.u32UserDataBuf = tmpAdr;

    tmpAdr += MVD3_FW_USER_DATA_BUF_LEN;

    tmpAdr = (MemAlign(tmpAdr, 0x1000));//4k align
    tmpAdr += MVD3_FW_USER_DATA_BUF_BACKUP_LEN;

    tmpAdr = (MemAlign(tmpAdr, 0x1000));//4k align

    tmpAdr += 0x10000;  //sync to MM

    u32MVDSLQTBLPostion = tmpAdr;
    //(printf("set u32SLQTBLPostion=%lx\n",tmpAdr));

    tmpAdr += MVD3_FW_SLQ_TAB_BUF_LEN;

    tmpAdr = (MemAlign(tmpAdr, 0x1000));//4k align
    //MVD_DEBUGINFO(printf("set MVD3_FW_SLQ_TAB_TMPBUF_ADR=%x\n",MVD3_FW_SLQ_TAB_TMPBUF_ADR));
    MVD_DEBUGINFO(printf("set MVD3_FW_SLQ_TAB_TMPBUF_ADR=%lx\n",tmpAdr));
    tmpAdr += MVD3_FW_SLQ_TAB_BUF_LEN;  //sync to MM

    u32MVDFWSLQTABTmpbufAdr = tmpAdr;

    MVD_DEBUGINFO(printf("set u32MVDFWSLQTABTmpbufAdr=%lx\n",tmpAdr));

    if ((u32start + u32len) < tmpAdr)
    {
        MS_ASSERT(0);
        //return FALSE;
    }

    if (curSrcMode != E_MVD_TS_FILE_MODE)
    {
        //tmpAdr = tmpAdr - u32start;  //sync to MM
        HAL_MVD_MemWrite4Byte(tmpAdr, 0xBE010000UL);
        tmpAdr+=4;
        HAL_MVD_MemWrite4Byte(tmpAdr, 0x000000FAUL);
        tmpAdr+=4;
        for (i=8; i<MVD3_FW_SLQ_TAB_TMPBUF_LEN; i+=4)
        {
            HAL_MVD_MemWrite4Byte(tmpAdr, 0x00UL);
            tmpAdr+=4;
        }

        u32MVDFWPtsTblAddr = tmpAdr;
        HAL_MVD_SetPtsTblAddr(u32MVDFWPtsTblAddr);
        for (i=0; i<MVD_FW_SLQTBL_PTS_BUF_LEN; i+=MVD_FW_SLQTBL_PTS_LEN)
        {
            HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+i, 0);     //byteCnt
            HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+i+4, 0);   //dummyPktCnt
            HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+i+8, 0);   //idLow
            HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+i+12, 0);  //idHigh

            HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+i+16, MVD_NULLPKT_PTS);  //PTS
            HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+i+20, 0);  //reserved0
            HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+i+24, 0);  //reserved1
            HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+i+28, 0);  //reserved2
        }

        MS_ASSERT((u32MVDFWPtsTblAddr+MVD_FW_SLQTBL_PTS_BUF_LEN)<=(u32start+u32len));
        MVD_DEBUGINFO(printf("PTS tbl start=%lx end=%lx\n",
               u32MVDFWPtsTblAddr, (u32MVDFWPtsTblAddr+MVD_FW_SLQTBL_PTS_BUF_LEN)));
        tmpAdr += MVD_FW_SLQTBL_PTS_BUF_LEN;
    }
    else
    {
        tmpAdr += (MVD3_FW_SLQ_TAB_TMPBUF_LEN + MVD_FW_SLQTBL_PTS_BUF_LEN);
    }

    u32DynScalingAdd = tmpAdr;
    if (stMemCfg.bEnableDynScale)
    {
        HAL_MVD_SetDynamicScaleAddr(tmpAdr);
        u8DynScalingDepth = 12; //HAL_MVD_GetDynamicScaleDepth
        MVD_DEBUGINFO(printf("bEnableMIUSel    = 0x%x\n", stMemCfg.bFWMiuSel));
        MVD_DEBUGINFO(printf("u32DynScalingAddr= 0x%lx\n", u32DynScalingAdd));
        MVD_DEBUGINFO(printf("u8DynScalingDepth= 0x%x\n", u8DynScalingDepth));
    }
    MVD_DEBUGINFO(printf("DynScaling start=%lx end=%lx\n",
           u32DynScalingAdd, (u32DynScalingAdd+MVD_FW_DYN_SCALE_BUF_LEN)));
    tmpAdr += MVD_FW_DYN_SCALE_BUF_LEN;

    u32ScalerInfoAdd = tmpAdr;
    MVD_DEBUGINFO(printf("ScalerInfo start=%lx end=%lx\n",
           u32ScalerInfoAdd, (u32ScalerInfoAdd+MVD_FW_SCALER_INFO_BUF_LEN)));
    tmpAdr += MVD_FW_SCALER_INFO_BUF_LEN;

    u32DecFrmInfoAdd = tmpAdr;
    HAL_MVD_SetDecFrmInfoAddr(tmpAdr);
    MVD_DEBUGINFO(printf("DecFrmInfo start=%lx\n", u32DecFrmInfoAdd));

    HAL_MVD_MemGetMap(E_MVD_MMAP_FB, &tmpAdr, &tmpLen);
    MVD_DEBUGINFO(printf("set MVD_FRAMEBUFFER_ADR=%lx\n",tmpAdr));
    MDrv_MVD_SetFrameBuffAddr(tmpAdr);

#if defined(_MVD4)
//  In order to let VD_MHEG5(CPU) and MVD HW engine run on different MIU,
// we allocate IAP, DP, and MV buffers after FB.
// The reason is that these 3 buffers are used by MVD HW engine.
#define MVD4_FBSIZE_MIN    0xFD2000 //5 * 2048 * 1080 * 1.5
    tmpAdr += MVD4_FBSIZE_MIN;
    tmpAdr = (MemAlign(tmpAdr, 0x100000));
    //it's not constraint, just to let IAP start from (FB_Start+16MB)

    MVD_DEBUGINFO(printf("set MVD3_FW_IAP_BUF_ADR =%lx\n",tmpAdr));
    HAL_MVD_SetIAPBufferAddr(tmpAdr);
    tmpAdr += MVD3_FW_IAP_BUF_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x08));//8-byte align

    MVD_DEBUGINFO(printf("set MVD3_FW_DP_BUF_ADR=%lx\n",tmpAdr));
    HAL_MVD_SetDPBufferAddr(tmpAdr);
    tmpAdr += MVD3_FW_DP_BUF_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x0800));//2k align

    MVD_DEBUGINFO(printf("set MVD3_FW_MV_BUF_ADR=%lx\n",tmpAdr));
    HAL_MVD_SetMVBufferAddr(tmpAdr);
    tmpAdr += MVD3_FW_MV_BUF_LEN;
    tmpAdr = (MemAlign(tmpAdr, 0x1000));//4k align
    //printf("===> MVbuff end = 0x%lx\n", tmpAdr);
#endif

    HAL_MVD_MemGetMap(E_MVD_MMAP_BS, &tmpAdr, &tmpLen);
    MVD_DEBUGINFO(printf("set MVD_BITSTREAM_ADR=%lx\n",tmpAdr));
    MDrv_MVD_SetBitStreamAddr(tmpAdr,tmpAdr+tmpLen);

    MVD_DEBUGINFO(printf("set pu8MVDGetVolBufStart=%lx\n",pu8MVDGetVolBufStart));
    MVD_DEBUGINFO(printf("set pu8MVDGetFrameInfoBufStart=%lx\n",pu8MVDGetFrameInfoBufStart));
    MVD_DEBUGINFO(printf("set pu8MVDSetHeaderBufStart=%lx\n",pu8MVDSetHeaderBufStart));

    return TRUE;
#endif
}


//------------------------------------------------------------------------------
/// Initialize variables for MVD driver
//------------------------------------------------------------------------------
static void MVD_InitVar(void)
{
    pu8MVDSetHeaderBufStart=0;
    pu8MVDGetVolBufStart=0;
    pu8MVDGetFrameInfoBufStart=0;
    u32MVDFirstVOLUpdate=FALSE;
    bAVSyncOn = FALSE;
    bStepDecode = FALSE;
    u32StepDecodeCnt = 0;
    bStepDisp = FALSE;
    bStep2Pts = FALSE;
    bEnableLastFrmShow = FALSE;
    u32LastPts = 0;
    eTrickMode = E_MVD_TRICK_DEC_UNKNOWN;
    u32PreEsRd = 0;
    u32PreEsWr = 0;

    stPreFrmInfo.u16HorSize   = 0;
    stPreFrmInfo.u16VerSize   = 0;
    stPreFrmInfo.u8AspectRate = 0;
    stPreFrmInfo.u32FrameRate = 0;
    stPreFrmInfo.u8Interlace  = 0;

    curCodecType = E_MVD_CODEC_UNKNOWN;
    curSrcMode = E_MVD_SRC_UNKNOWN;
    _eFileSyncMode = E_MVD_TIMESTAMP_FREERUN;
    ePreSpeedType = E_MVD_SPEED_DEFAULT;

    stSyncCfg.bEnable = FALSE;
    stSyncCfg.u32Delay = 0;
    stSyncCfg.u16Tolerance = 0;
    stSyncCfgBfStepDisp.bEnable = FALSE;
    stSyncCfgBfStepDisp.u32Delay = 0;
    stSyncCfgBfStepDisp.u16Tolerance = 0;
#if(defined(CHIP_T7))
    u8MvdPlayMode = 0;
#endif

    return;
}

#if 0
static void MVD_DumpMemCfg(MVD_MEMCfg* pCfg)
{
    if (pCfg)
    {
        printf("u32FWBinAddr          = 0x%lx\n", pCfg->u32FWBinAddr);
        printf("u32FWBinSize          = 0x%lx\n", pCfg->u32FWBinSize);
        printf("u32FWCodeAddr         = 0x%lx\n", pCfg->u32FWCodeAddr);
        printf("u32FWCodeSize         = 0x%lx\n", pCfg->u32FWCodeSize);
        printf("u32FBAddr             = 0x%lx\n", pCfg->u32FBAddr);
        printf("u32FBSize             = 0x%lx\n", pCfg->u32FBSize);
        printf("u32BSAddr             = 0x%lx\n", pCfg->u32BSAddr);
        printf("u32BSSize             = 0x%lx\n", pCfg->u32BSSize);
        printf("u32DrvBufAddr         = 0x%lx\n", pCfg->u32DrvBufAddr);
        printf("u32DrvBufSize         = 0x%lx\n", pCfg->u32DrvBufSize);
        printf("u32DynSacalingBufAddr = 0x%lx\n", pCfg->u32DynSacalingBufAddr);
        printf("u32DynSacalingBufSize = 0x%lx\n", pCfg->u32DynSacalingBufSize);
        printf("u32Miu1BaseAddr       = 0x%lx\n", pCfg->u32Miu1BaseAddr);
        printf("bFWMiuSel             = 0x%x\n", pCfg->bFWMiuSel);
        printf("bHWMiuSel             = 0x%x\n", pCfg->bHWMiuSel);
    }
}
#endif

#define MVD_HD_FBSIZE 0xbf4000  //Framebuffer size minimum for High Definition
//------------------------------------------------------------------------------
/// Configure MVD for memory and firmware.
/// Notice:
///     (1) u32FWAddr & u32DrvBufAddr should be on the same MIU
///     (2) u32FBAddr & u32BSAddr should be on the same MIU
/// @param -fwCfg  \b IN : pointer to firmware configuration
/// @param -memCfg \b IN : pointer to memory configuration
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_SetCfg(MVD_FWCfg* fwCfg, MVD_MEMCfg* memCfg)
{
    MS_BOOL bDrvBuffMiuSel=FALSE;
    MVD_FUNC_ENTRY();
    if (!fwCfg || !memCfg)
        return E_MVD_RET_INVALID_PARAM;

    //config firmware: framebuffer mode
    if (memCfg->u32FBSize < MVD_HD_FBSIZE)
    {   //Set Framebuffer mode as SD mode if FB < (1920*1088*1.5*4)
        u8FBMode = MVD3_SD_MODE;    //720*576
    }
    else
    {
        u8FBMode = MVD3_HD_MODE;    //1920*1088
    }

    //HAL_MVD_SetFWBinID(fwCfg->u32FWSrc);

    if (memCfg->u32Miu1BaseAddr == 0)
    {
#if MVD_ENABLE_MSOS_MIU1_BASE
        memCfg->u32Miu1BaseAddr = HAL_MIU1_BASE;
#else
        memCfg->u32Miu1BaseAddr = MVD_MIU1_BASE_ADDRESS;
#endif
    }

    memCfg->bFWMiuSel = (memCfg->u32FWCodeAddr < memCfg->u32Miu1BaseAddr)?MIU_SEL_0:MIU_SEL_1;
    memCfg->bHWMiuSel = (memCfg->u32FBAddr < memCfg->u32Miu1BaseAddr)?MIU_SEL_0:MIU_SEL_1;
    MVD_DEBUGINFO(printf("MIUSEL:: FW=%x HW=%x, miu1base=0x%lx\n", 
                  memCfg->bFWMiuSel, memCfg->bHWMiuSel, memCfg->u32Miu1BaseAddr));

    //memCfg->u32DrvBufAddr = 0xBBE0000;
    bDrvBuffMiuSel = (memCfg->u32DrvBufAddr < memCfg->u32Miu1BaseAddr)?MIU_SEL_0:MIU_SEL_1;
    MVD_DEBUGINFO(printf("bDrvBuffMiuSel = %x\n", bDrvBuffMiuSel));
    MS_ASSERT(bDrvBuffMiuSel == memCfg->bFWMiuSel);

    //set these attributes before HAL_MVD_MemSetMap() and HAL_MVD_LoadCode()
    stMemCfg.u32Miu1BaseAddr = memCfg->u32Miu1BaseAddr;
    stMemCfg.bFWMiuSel = memCfg->bFWMiuSel;
    stMemCfg.bHWMiuSel = memCfg->bHWMiuSel;

    //config memory
    HAL_MVD_MemSetMap(E_MVD_MMAP_FW, memCfg->u32FWCodeAddr, memCfg->u32FWCodeSize);
    HAL_MVD_MemSetMap(E_MVD_MMAP_FB, memCfg->u32FBAddr, memCfg->u32FBSize);
    HAL_MVD_MemSetMap(E_MVD_MMAP_BS, memCfg->u32BSAddr, memCfg->u32BSSize);
    HAL_MVD_MemSetMap(E_MVD_MMAP_DRV, memCfg->u32DrvBufAddr, memCfg->u32DrvBufSize);

    //both stMemCfg & memCfg are physical addr, except field u32FWSrcVAddr
    stMemCfg.eFWSrcType = memCfg->eFWSrcType;

    if (stMemCfg.eFWSrcType == E_MVD_FW_SOURCE_DRAM)
    {
        stMemCfg.u32FWSrcVAddr = HAL_MVD_PA2NonCacheSeg((MS_U32)memCfg->u32FWBinAddr);
    }
    else
    {
        stMemCfg.u32FWSrcVAddr = NULL;
    }

    stMemCfg.u32FWBinAddr = memCfg->u32FWBinAddr;
    stMemCfg.u32FWBinSize = memCfg->u32FWBinSize;
    stMemCfg.u32FWCodeAddr = memCfg->u32FWCodeAddr;
    stMemCfg.u32FWCodeSize = memCfg->u32FWCodeSize;
    stMemCfg.u32FBAddr = memCfg->u32FBAddr;
    stMemCfg.u32FBSize = memCfg->u32FBSize;
    stMemCfg.u32BSAddr = memCfg->u32BSAddr;
    stMemCfg.u32BSSize = memCfg->u32BSSize;
    stMemCfg.u32DrvBufAddr = memCfg->u32DrvBufAddr;
    stMemCfg.u32DrvBufSize = memCfg->u32DrvBufSize;
    stMemCfg.u32DynSacalingBufAddr = memCfg->u32DynSacalingBufAddr;
    stMemCfg.u32DynSacalingBufSize = memCfg->u32DynSacalingBufSize;
    stMemCfg.bEnableDynScale = memCfg->bEnableDynScale;

    bSlqTblSync = ((stMemCfg.u32DrvBufAddr < stMemCfg.u32BSAddr) ||
                   ((stMemCfg.u32DrvBufAddr+stMemCfg.u32DrvBufSize) > (stMemCfg.u32BSAddr+stMemCfg.u32BSSize)));
    MVD_DEBUGINFO(printf("bSlqTblSync = %x\n", bSlqTblSync));
    //MVD_DumpMemCfg(&stMemCfg);

    return E_MVD_RET_OK;
}


static MS_BOOL MVD_Init(void)
{
#ifndef REDLION_LINUX_KERNEL_ENVI
    OSAL_MVD_MutexInit();
#endif
#if MVD_PROFILING
    MS_U32 t1=0, t2=0;
    t1 = MsOS_GetSystemTime();
#endif
    //load code
    if (!HAL_MVD_LoadCode())
    {
        MVD_DEBUGERROR(printf("_MVD_Init:MDrv_MVD_LoadCode failed\n"));
        return FALSE;
    }
    else
    {
        MVD_DEBUGINFO(printf("_MVD_Init:MDrv_MVD_LoadCode success\n"));
    }
#if MVD_PROFILING
    t2 = MsOS_GetSystemTime();
    printf("i000: t2=%ld, diff=%ld\n", t2, t2-t1);
    t1 = t2;
#endif
    if (!HAL_MVD_InitHW())
    {
        MVD_DEBUGERROR(printf("_MVD_Init:HAL_MVD_InitHW failed\n"));
        return FALSE;
    }
    else
    {
        MVD_DEBUGINFO(printf("_MVD_Init:HAL_MVD_InitHW success\n"));
    }
#if MVD_PROFILING
    t2 = MsOS_GetSystemTime();
    printf("i001: t2=%ld, diff=%ld\n", t2, t2-t1);
    t1 = t2;
#endif
    if (!HAL_MVD_InitFW())
    {
        MVD_DEBUGERROR(printf("_MVD_Init:HAL_MVD_InitFW failed\n"));
        return FALSE;
    }
    else
    {
        MVD_DEBUGINFO(printf("_MVD_Init:HAL_MVD_InitFW success\n"));
    }
#if MVD_PROFILING
    t2 = MsOS_GetSystemTime();
    printf("i002: t2=%ld, diff=%ld\n", t2, t2-t1);
    t1 = t2;
#endif
    return TRUE;
}

void MDrv_MVD_RegSetBase(MS_U32 u32RegBaseAddr)
{
    HAL_MVD_RegSetBase(u32RegBaseAddr);
}

//------------------------------------------------------------------------------
/// MVD driver initialization
/// @return TRUE or FALSE
///     - TRUE, Success
///     - FALSE, Failed
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_Init(void)
{
    MS_U32 u32Addr = 0;
    MS_U32 u32Len = 0;

    MVD_DEBUGINFO(printf("MDrv_MVD_Init:start\n"));

    MVD_InitVar();
    _bDecodeIFrame = FALSE;
    _bDrvInit = FALSE;

    if (!MVD_Init())
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_Init:_MVD_Init failed\n"));
        return FALSE;
    }
    else
    {
        MVD_DEBUGINFO(printf("MDrv_MVD_Init:_MVD_Init success\n"));
    }

    HAL_MVD_MemGetMap(E_MVD_MMAP_FW, &u32Addr, &u32Len);
    if(!MVD_MVDSetInternalBuffAddr(u32Addr, u32Len))
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_Init:_MVD_MVDSetInternalBuffAddr failed\n"));
        return FALSE;
    }
    else
    {
        MVD_DEBUGINFO(printf("MDrv_MVD_Init:_MVD_MVDSetInternalBuffAddr success\n"));
    }

    _bDrvInit = TRUE;

    return TRUE;
}


//------------------------------------------------------------------------------
/// MVD driver exit
/// @return TRUE or FALSE
///     - TRUE, Success
///     - FALSE, Failed
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_Exit(void)
{
    MVD_DEBUGINFO(printf("MDrv_MVD_Exit:start\n"));

    //do nothing if driver is not initialized
    if (_bDrvInit != TRUE)
    {
        MVD_DEBUGINFO(printf("MDrv_MVD_Exit: not-init yet\n"));
#if (!defined(CHIP_T2))&&(!defined(CHIP_T7))
        HAL_MVD_PowerCtrl(DISABLE);
        HAL_VPU_PowerCtrl(DISABLE);
#endif
        return FALSE;
    }

    //stop and reset FW/HW
    if (MDrv_MVD_Stop() != TRUE)
    {
        MVD_DEBUGINFO(printf("MDrv_MVD_Exit: stop fail\n"));
        //return FALSE;
    }

#ifdef MVD_ENABLE_ISR
    //dettach isr
    if (eEventFlag)
    {
        MDrv_MVD_SetIsrEvent(E_MVD_EVENT_DISABLE_ALL, (MVD_InterruptCb)NULL);
    }
#endif

    //reset internal variables.
    MVD_InitVar();
    _bDecodeIFrame = FALSE;
    _bDrvInit = FALSE;

#if (!defined(CHIP_T2))&&(!defined(CHIP_T7))
    HAL_MVD_PowerCtrl(DISABLE);
    HAL_VPU_PowerCtrl(DISABLE);
#endif

    return TRUE;
}

//------------------------------------------------------------------------------
/// Reset MVD
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_Rst( void )
{
    MVD_FUNC_ENTRY();
    if (TRUE == HAL_MVD_SoftRstHW())
    {
        //notice: T2 MVD3 does not support soft-reset HW.
        //Thus, MDrv_MVD_Rst()==MDrv_MVD_Init() for T2.
        return E_MVD_RET_OK;
    }

    //For T3&Euclid MVD4, do re-init only when SoftRst does not work.
    if (FALSE == MDrv_MVD_Init())
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_Rst:_MVD_Init failed\n"));
        return E_MVD_RET_FAIL;
    }
    else
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_Rst:_MVD_Init success\n"));
    }

    return E_MVD_RET_OK;
}

//------------------------------------------------------------------------------
/// Allow MVD to start decoding even without sequence header.
//------------------------------------------------------------------------------
void MDrv_MVD_EnableForcePlay(void)
{
    MVD_FUNC_ENTRY();
    if (HAL_MVD_EnableForcePlay() == FALSE)
    {
        MVD_DEBUGERROR( printf("HAL_MVD_EnableForcePlay fail!!\r\n") );
        return;
    }
    return;
}

//------------------------------------------------------------------------------
/// Issue Play command to MVD
//------------------------------------------------------------------------------
void MDrv_MVD_Play(void)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
#if defined(CHIP_T7)
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0= u8MvdPlayMode;
    if (HAL_MVD_MVDCommand( CMD_PLAY, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_PLAY ) );
        return;
    }
#else
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = 1;
    if (HAL_MVD_MVDCommand( CMD_DIU_WIDTH_ALIGN, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DIU_WIDTH_ALIGN ) );
        return;
    }

#if _STEPDISP_DISABLE_AVSYNC
    ePlayMode = E_MVD_PLAY;
    if (bStepDisp == TRUE)
    {
        //if driver has been set to StepDisp, restore avsyn cfg when back to play
        MVD_DEBUGINFO(printf("MDrv_MVD_Play reset avsync: bEnable=0x%x u32Delay=0x%lx u16Tolerance=0x%x\n",
                      stSyncCfgBfStepDisp.bEnable, stSyncCfgBfStepDisp.u32Delay, stSyncCfgBfStepDisp.u16Tolerance));
        (printf("MDrv_MVD_Play reset avsync: bEnable=0x%x u32Delay=0x%lx u16Tolerance=0x%x\n",
                      stSyncCfgBfStepDisp.bEnable, stSyncCfgBfStepDisp.u32Delay, stSyncCfgBfStepDisp.u16Tolerance));

        MDrv_MVD_SetAVSync(stSyncCfgBfStepDisp.bEnable, stSyncCfgBfStepDisp.u32Delay);
        if (stSyncCfgBfStepDisp.u16Tolerance!=0)
        {
            MDrv_MVD_ChangeAVsync(stSyncCfgBfStepDisp.bEnable, stSyncCfgBfStepDisp.u16Tolerance);
        }
        bStepDisp = FALSE;
    }
#endif

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_PLAY, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_PLAY ) );
        return;
    }

#if (!defined(CHIP_T2))
    if (HAL_MVD_Resume() == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: HAL_MVD_Resume fail!!\r\n" ) );
        return;
    }
#endif
#endif
    _bDecodeIFrame = FALSE;
    return;
}

//------------------------------------------------------------------------------
/// Set bit stream buffer address to MVD
/// @param -u32start \b IN : start address
/// @param -u32end \b IN : end address
//------------------------------------------------------------------------------
void MDrv_MVD_SetBitStreamAddr(MS_U32 u32start, MS_U32 u32end)
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32start%8)==0);
    u32start >>= 3;
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32start);
    mvdcmd.Arg1 = H_WORD(u32start);
    mvdcmd.Arg2 = L_DWORD(u32start);
    mvdcmd.Arg3 = 0;

    if (HAL_MVD_MVDCommand( CMD_STREAM_BUF_START, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_STREAM_BUF_START ) );
        return;
    }

    MS_ASSERT((u32end%8)==0);
    u32end >>= 3;
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32end);
    mvdcmd.Arg1 = H_WORD(u32end);
    mvdcmd.Arg2 = L_DWORD(u32end);
    mvdcmd.Arg3 = 0;

    if (HAL_MVD_MVDCommand( CMD_STREAM_BUF_END, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_STREAM_BUF_END ) );
        return;
    }
    return;
}

//------------------------------------------------------------------------------
/// Set frame buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void MDrv_MVD_SetFrameBuffAddr(MS_U32 u32addr)
{
    HAL_MVD_SetFrameBuffAddr(u32addr, u8FBMode);
}


//------------------------------------------------------------------------------
/// Set MVD SLQ start & end address
/// @param -u32start \b IN : start address
/// @param -u32end \b IN : end address
//------------------------------------------------------------------------------
void MDrv_MVD_SetSLQStartEnd(MS_U32 u32start, MS_U32 u32end)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SET_CMDARG(mvdcmd, (u32end+1));
    if (HAL_MVD_MVDCommand( CMD_SLQ_END, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SLQ_END ) );
        return;
    }

    SET_CMDARG(mvdcmd, u32start);
    if (HAL_MVD_MVDCommand( CMD_SLQ_START, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SLQ_START ) );
        return;
    }

    return;
}

//------------------------------------------------------------------------------
/// Get SLQ available level
/// @return SLQ available level
//------------------------------------------------------------------------------
MS_U8 MDrv_MVD_GetSLQAvailableLevel(void)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_SLQ_AVAIL_LEVEL, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SLQ_AVAIL_LEVEL ) );
        return 0;
    }
    MVD_DEBUGINFO( printf( "MDrv_MVD_GetSLQAvailableLevel=%x\n", mvdcmd.Arg2 ) );

    return mvdcmd.Arg2;
}


//Map driver CodecType to firmware CodecType
static MS_U8 MVD_MapCodecType(MVD_CodecType type)
{
    MS_U8 u8type = 0xff;
    switch (type)
    {
        case E_MVD_CODEC_MPEG2:
            u8type = CODEC_MPEG2;
            break;
        case E_MVD_CODEC_MPEG4:
            u8type = CODEC_MPEG4;
            break;
        case E_MVD_CODEC_MPEG4_SHORT_VIDEO_HEADER:
            u8type = CODEC_MPEG4_SHORT_VIDEO_HEADER;
            break;
        case E_MVD_CODEC_DIVX311:
            u8type = CODEC_DIVX311;
            break;

        case E_MVD_CODEC_FLV:
            u8type = 0x03;
            break;

        case E_MVD_CODEC_VC1_MAIN: //RCV
            u8type = 0x05;
            break;

        case E_MVD_CODEC_VC1_ADV:  //VC1
            u8type = 0x04;
            break;

        default:
            break;
    }

    return u8type;
}


//Map driver SrcType to firmware SrcType
static MS_U8 MVD_MapSrcMode(MVD_SrcMode mode)
{
    MS_U8 u8mode = 0xff;
    switch (mode)
    {
        case E_MVD_TS_MODE:
            u8mode = STREAM_MODE;
            break;
        case E_MVD_FILE_MODE:
            u8mode = FILE_MODE;
            break;
        case E_MVD_SLQ_MODE:
            u8mode = SLQ_MODE;
            break;
        case E_MVD_SLQ_TBL_MODE:
            u8mode = SLQ_TBL_MODE;
            break;
        case E_MVD_TS_FILE_MODE:
            u8mode = TS_FILE_MODE;
            break;

        default:
            break;
    }

    return u8mode;
}


//------------------------------------------------------------------------------
/// Set codec type.
/// @param -u8CodecType \b IN : 0: mpeg4, 1: mpeg4 with short_video_header, 2: DivX311
/// @param -u8BSProviderMode \b IN : TS live stream, file, SLQ, SLQ table link and TS file mode.
//------------------------------------------------------------------------------
void MDrv_MVD_SetCodecInfo(MVD_CodecType u8CodecType, MVD_SrcMode u8BSProviderMode, MS_U8 bDisablePESParsing)
{
    MVD_CmdArg stCmdArg;

    MVD_FUNC_ENTRY();
    //printf("u8CodecType=0x%x\n", u8CodecType);
    //printf("u8BSProviderMode=0x%x\n", u8BSProviderMode);
    SETUP_CMDARG(stCmdArg);
    stCmdArg.Arg0 = MVD_MapCodecType(u8CodecType);
    stCmdArg.Arg1 = MVD_MapSrcMode(u8BSProviderMode);
    stCmdArg.Arg2 = bDisablePESParsing;
    //arg2 is only valid for STREAM_MODE and TS_FILE_MODE
    //set as 0 to enable MVD parser and parser interrupt
    stCmdArg.Arg3 = 0;
    MVD_DEBUGINFO(printf("MDrv_MVD_SetCodecInfo: Cmd: %x, Arg0: %x, Arg1: %x. Arg2: %x\n",
                  CMD_CODEC_INFO, stCmdArg.Arg0, stCmdArg.Arg1, stCmdArg.Arg2));
    if (HAL_MVD_MVDCommand(CMD_CODEC_INFO, &stCmdArg) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_CODEC_INFO ) );
        return;
    }

    //Refer to msAPI_VDPlayer_DecodeMPEG4.c (core\kernel\api\videoplayer)
    if (u8BSProviderMode == E_MVD_SLQ_TBL_MODE)
    {
        if ((u8CodecType == E_MVD_CODEC_MPEG4) ||
            (u8CodecType == E_MVD_CODEC_DIVX311) ||
            (u8CodecType == E_MVD_CODEC_MPEG4_SHORT_VIDEO_HEADER))
        {
#if (defined(CHIP_T2))
            //This patch is only for T2
            // Enable MC delay mode for penguiy.avi
            SETUP_CMDARG(stCmdArg);
            stCmdArg.Arg0 = 0x5f;
            stCmdArg.Arg1 = 0;
            stCmdArg.Arg2 = 0x42;//0x55: delay 700 cycles SD; 0x42: delay 500 cycles HD
            stCmdArg.Arg3 = 0x01;
            HAL_MVD_MVDCommand(CMD_WR_IO, &stCmdArg);
#endif
            // Enable PackMode
            SETUP_CMDARG(stCmdArg);
            stCmdArg.Arg0 = 3;
            HAL_MVD_MVDCommand(CMD_PARSE_M4V_PACKMD, &stCmdArg);

            // Set DIU width of rounding mode (align to 8byte)
            SETUP_CMDARG(stCmdArg);
            stCmdArg.Arg0 = 1;
            HAL_MVD_MVDCommand(CMD_DIU_WIDTH_ALIGN, &stCmdArg);
        }
        MVD_SLQTblInit();
    }

    curCodecType = u8CodecType;
    curSrcMode = u8BSProviderMode;
    return;
}

//------------------------------------------------------------------------------
/// Set DivX patch.
/// @param -u8MvAdjust \b IN : chroma adjustment
/// @param -u8IdctSel \b IN : idct algorithm selection
//------------------------------------------------------------------------------
void MDrv_MVD_SetDivXCfg(MS_U8 u8MvAdjust, MS_U8 u8IdctSel)
{
    MVD_CmdArg stCmdArg;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(stCmdArg);
    stCmdArg.Arg0 = u8MvAdjust;
    if (HAL_MVD_MVDCommand(CMD_DIVX_PATCH, &stCmdArg) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DIVX_PATCH ) );
        return;
    }

    SETUP_CMDARG(stCmdArg);
    stCmdArg.Arg0 = u8IdctSel;
    if (HAL_MVD_MVDCommand(CMD_IDCT_SEL, &stCmdArg) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_IDCT_SEL ) );
        return;
    }

    return;
}


//------------------------------------------------------------------------------
/// Set DivX311 stream info.
/// @param divxInfo \b IN : DivX311 stream info.
//------------------------------------------------------------------------------
static void MVD_WriteDivx311Data(FW_DIVX_INFO *divxInfo)
{
    HAL_MVD_MemWrite4Byte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_VOL_HANDLE_DONE,divxInfo->vol_handle_done);
    HAL_MVD_MemWrite4Byte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_WIDTH,divxInfo->width);
    HAL_MVD_MemWrite4Byte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_HEIGHT,divxInfo->height);
    HAL_MVD_MemWrite4Byte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_FRAME_COUNT,divxInfo->frame_count);
    HAL_MVD_MemWrite4Byte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_FRAME_TIME,divxInfo->frame_time);
    HAL_MVD_MemWrite2Byte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_PTS_INCR,divxInfo->pts_incr);
#if (defined(CHIP_T2))
    HAL_MVD_MemWrite2Byte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_FRAME_RATE,divxInfo->frame_rate);
#else
    HAL_MVD_MemWrite4Byte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_FRAME_RATE,divxInfo->frame_rate);
#endif
    HAL_MVD_MemWriteByte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_ASPECT_RATIO,divxInfo->aspect_ratio);
    HAL_MVD_MemWriteByte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_ASPECT_RATIO,divxInfo->progressive_sequence);
    HAL_MVD_MemWriteByte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_MPEG1,divxInfo->mpeg1);
    HAL_MVD_MemWriteByte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_PLAY_MODE,divxInfo->play_mode);
    HAL_MVD_MemWriteByte(pu8MVDSetHeaderBufStart+OFFSET_DIVX_MPEG_FRC_MODE,divxInfo->mpeg_frc_mode);
    return;
}

#if 0
//------------------------------------------------------------------------------
/// Get Vol Info.
/// @param volInfo \b OUT : Vol Info.
//------------------------------------------------------------------------------
static void MVD_ReadVolInfo(FW_VOL_INFO *volInfo)
{
    volInfo->vol_info               = HAL_MVD_MemRead2Byte(pu8MVDGetVolBufStart);
    volInfo->sprite_usage           = HAL_MVD_MemRead2Byte(pu8MVDGetVolBufStart + OFFSET_SPRITE_USAGE);
    volInfo->width                  = HAL_MVD_MemRead4Byte(pu8MVDGetVolBufStart + OFFSET_WIDTH);
    volInfo->height                 = HAL_MVD_MemRead4Byte(pu8MVDGetVolBufStart + OFFSET_HEIGHT);
    volInfo->pts_incr               = HAL_MVD_MemRead2Byte(pu8MVDGetVolBufStart + OFFSET_PTS_INCR);
    volInfo->frame_rate             = HAL_MVD_MemRead2Byte(pu8MVDGetVolBufStart + OFFSET_FRAME_RATE);
    volInfo->aspect_ratio           = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_ASPECT_RATIO);
    volInfo->progressive_sequence   = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_PROGRESSIVE_SEQUENCE);
    volInfo->mpeg1                  = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_MPEG1);
    volInfo->play_mode              = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_PLAY_MODE);
    volInfo->mpeg_frc_mode          = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_MPEG_FRC_MODE);
    volInfo->low_delay              = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_LOW_DELAY);
    volInfo->video_range            = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_VIDEO_RANGE);
    volInfo->bit_rate               = HAL_MVD_MemRead4Byte(pu8MVDGetVolBufStart + OFFSET_BIT_RATE);
    volInfo->par_width              = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_PAR_WIDTH);
    volInfo->par_height             = HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_PAR_HEIGHT);
    volInfo->vol_time_incr_res      = HAL_MVD_MemRead2Byte(pu8MVDGetVolBufStart + OFFSET_VOL_TIME_INCR_RES);
    volInfo->fixed_vop_time_incr    = HAL_MVD_MemRead2Byte(pu8MVDGetVolBufStart + OFFSET_FIXED_VOP_TIME_INCR);

#if defined(_MVD4)
    volInfo->vc1_frame_rate=HAL_MVD_MemRead4Byte(pu8MVDGetVolBufStart+OFFSET_VC1_FRAME_RATE);
    //printf("vc1_frameRate=%d\n", volInfo->vc1_frame_rate);
#endif

#if 0
    printf("MVD_ReadVolInfo \n");
    printf("pu8MVDGetVolBufStart = 0x%lx\n", pu8MVDGetVolBufStart);
    printf("volInfo->vol_info = 0x%x \n", volInfo->vol_info);
    printf("volInfo->sprite_usage = 0x%x \n", volInfo->sprite_usage);
    printf("volInfo->width = %d \n", volInfo->width);
    printf("volInfo->height = %d \n", volInfo->height);
    printf("volInfo->pts_incr = 0x%x \n", volInfo->pts_incr);
    printf("volInfo->frame_rate = %u \n", volInfo->frame_rate);
    printf("volInfo->aspect_ratio = 0x%x \n", volInfo->aspect_ratio);
    printf("volInfo->progressive_sequence = 0x%x \n", volInfo->progressive_sequence);
    printf("volInfo->mpeg1 = 0x%x \n", volInfo->mpeg1);
    printf("volInfo->play_mode = 0x%x \n", volInfo->play_mode);
    printf("volInfo->mpeg_frc_mode = 0x%x \n", volInfo->mpeg_frc_mode);
    //printf("volInfo->invalidstream = 0x%x \n", volInfo->invalidstream);
    printf("volInfo->video_range = 0x%x \n", volInfo->video_range);
    printf("volInfo->bit_rate = 0x%x \n", volInfo->bit_rate);
#endif
    return;
}
#endif

static MS_U32 writePtrLast;
//------------------------------------------------------------------------------
/// Get MVD decoded picture counter
/// @return -decoded picture counter
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetPicCounter( void )
{
    //printf("==>> wr=%lx rpt=%lx\n", _drvSlqTbl.u32WrPtr, MDrv_MVD_GetSLQReadPtr());
#if 0
    if (MDrv_MVD_GetVldErrCount()!=0)
    {
        printf("$$ wPtr= 0x%lx(0x%lx) rPtr=0x%lx(0x%lx) vldErr=0x%lx\n", writePtrLast, _drvSlqTbl.u32WrPtr,
        MDrv_MVD_GetSLQReadPtr(), _drvSlqTbl.u32RdPtr, MDrv_MVD_GetVldErrCount());
        _SLQTbl_DumpPtsTbl(0x530, 0x620);
        while(1);
    }
#endif
    MVD_FUNC_ENTRY();
#if (defined(CHIP_T7))
    return __MVD_GetPicCounter();
#else
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetPicCounter error: pu8MVDGetFrameInfoBufStart=NULL\n"));
            return 0;
    }

    return HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_FRAME_COUNT);
#endif
}


//------------------------------------------------------------------------------
/// Get MVD skipped picture counter
/// @return -skipped picture counter
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetSkipPicCounter( void )
{
    MVD_FUNC_ENTRY();
    #if (defined(CHIP_T7))
    return 0;
    #else
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetSkipPicCounter error: pu8MVDGetFrameInfoBufStart=NULL\n"));
        return 0;
    }

    return HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart + OFFSET_SKIP_FRAME_COUNT);
    #endif
}

//------------------------------------------------------------------------------
/// Get MVD picture type
/// @return - picture type
//------------------------------------------------------------------------------
MVD_PicType MDrv_MVD_GetPicType( void )
{
    MS_U32 picType = 0xff;
    MVD_PicType ret = E_MVD_PIC_UNKNOWN;

    MVD_FUNC_ENTRY();
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetSkipPicCounter error: pu8MVDGetFrameInfoBufStart=NULL\n"));
        return E_MVD_PIC_UNKNOWN;
    }

    picType = HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_PICTURE_TYPE);
    switch (picType)
    {
        case 0:
            ret = E_MVD_PIC_I;
            break;
        case 1:
            ret = E_MVD_PIC_P;
            break;
        case 2:
            ret = E_MVD_PIC_B;
            break;
        default:
            break;
    }
    return ret;
}

//------------------------------------------------------------------------------
/// Get software index
/// @return -software index
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetSWIdx( void )
{
    MVD_FUNC_ENTRY();

    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetSkipPicCounter error: pu8MVDGetFrameInfoBufStart=NULL\n"));
        return 0;
    }
    return HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_SLQ_SW_INDEX);
}

//------------------------------------------------------------------------------
/// Get MVD bit rate (bits/sec)
/// @return -bit rate
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetBitsRate( void )
{
    MVD_FUNC_ENTRY();

    if(pu8MVDGetVolBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetValidStreamFlag error: pu8MVDGetVolBufStart=NULL\n"));
        return FALSE;
    }

    //Ref 13818-2, this flag is unit of 400 bits/sec
    return HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_BIT_RATE)*400;
}


//------------------------------------------------------------------------------
/// Get video_range flag.
/// Supported after MVD FW release v00720390
/// Color team need this information for better color quality.
/// When video_range = 0  gives a range of Y from 16 to 235 and Cb , Cr from 16 to 240
/// When video_range = 1  gives a range of Y from  0 to 255 and Cb , Cr from  0 to 255
/// @return -video_range
//------------------------------------------------------------------------------
MS_U8 MDrv_MVD_GetVideoRange( void )
{
    MVD_FUNC_ENTRY();
    if(pu8MVDGetVolBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetVideoRange error: pu8MVDGetVolBufStart=NULL\n"));
        return FALSE;
    }

    return HAL_MVD_MemReadByte(pu8MVDGetVolBufStart+OFFSET_VIDEO_RANGE);
}

//------------------------------------------------------------------------------
/// Get LowDelay flag
/// @return -True/False
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetLowDelayFlag( void )
{
    MVD_FUNC_ENTRY();

    if(pu8MVDGetVolBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetLowDelayFlag error: pu8MVDGetVolBufStart=NULL\n"));
        return FALSE;
    }

    return HAL_MVD_MemReadByte(pu8MVDGetVolBufStart+OFFSET_LOW_DELAY);
}

//------------------------------------------------------------------------------
/// Get MVD SLQ read pointer
/// @return -SLQ read pointer
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetSLQReadPtr( void )
{
    MS_U32 u32RdPtr = 0;
#if 0
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetSkipPicCounter error: pu8MVDGetFrameInfoBufStart=NULL\n"));
        return 0;
    }

    readPtr=HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_SLQ_TBL_RPTR);
#else
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_SLQ_GET_TBL_RPTR, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SLQ_GET_TBL_RPTR ) );
        return 0;
    }
    u32RdPtr = mvdcmd.Arg0 | (mvdcmd.Arg1<<8) | (mvdcmd.Arg2<<16) | (mvdcmd.Arg3<<24);
        //printf("##### MDrv_MVD_GetSLQReadPtr 0x%lx\n", readPtr);
#endif

    if(u32RdPtr == 0)//not start decode yet,MVD return 0
    {
        u32RdPtr = 0;//u32MVDSLQTBLPostion;// SLQ_TBL_POSITION;
        MVD_DEBUGINFO(printf("SLQ_TBL_POSITION = %lx\n", u32MVDSLQTBLPostion));
    }
    else
    {
        u32RdPtr = u32RdPtr << 3;
    }

    return u32RdPtr;
}

//------------------------------------------------------------------------------
/// Get MVD SLQ write pointer
/// @return -SLQ write pointer
//------------------------------------------------------------------------------
void MDrv_MVD_SetSLQWritePtr ( MS_U32 writePtr )
{
    MVD_CmdArg mvdcmd;
    MS_U32 u32WrPtr;

    MS_ASSERT(_drvSlqTbl.u32WrPtr < _drvSlqTbl.u32EndAdd);
    MS_ASSERT(_drvSlqTbl.u32WrPtr >= _drvSlqTbl.u32StAdd);

    //should report (WrPtr-8)
    if (_drvSlqTbl.u32WrPtr != _drvSlqTbl.u32StAdd)
    {
        u32WrPtr = _drvSlqTbl.u32WrPtr - 8;
    }
    else
    {
        u32WrPtr = _drvSlqTbl.u32EndAdd - 8;
    }
    //printf("%s wPtr = 0x%lx rPtr=0x%lx\n", __FUNCTION__, *pu32WrPtr, MDrv_MVD_GetSLQReadPtr());

#ifndef REDLION_LINUX_KERNEL_ENVI
    //if (stMemCfg.bFWMiuSel != stMemCfg.bHWMiuSel)
    if (bSlqTblSync)
    {
        //Update SLQ table, DivX311 patterns, and dummy patterns to bitstream buffer
        MS_U32 u32SrcOffset = _drvSlqTbl.u32StAdd;
        MS_U32 u32SrcAdd = u32SrcOffset;
        MS_U32 u32DstAdd = stMemCfg.u32BSAddr;
        MS_U32 u32DstOffset = MVD_GetMemOffset(stMemCfg.u32BSAddr);
        MS_U32 u32TblWr;
        //printf("===>offset(Src=0x%lx, Dst=0x%lx)", u32SrcAdd, u32DstAdd);
        if (stMemCfg.bFWMiuSel == MIU_SEL_1)
        {
            u32SrcAdd = u32SrcOffset + stMemCfg.u32Miu1BaseAddr;
        }
        //printf(" PA(Src=0x%lx, Dst=0x%lx)", u32SrcAdd, u32DstAdd);

        HAL_MVD_CPU_Sync();

        BDMA_Result bdmaRlt;

        bdmaRlt = MDrv_BDMA_CopyHnd(u32SrcAdd, u32DstAdd, (SLQ_TBL_SIZE*2+DUMMY_SIZE), bdmaCpyType, BDMA_OPCFG_DEF);

        if (E_BDMA_OK != bdmaRlt)
        {
            printf("%s MDrv_BDMA_MemCopy fail ret=%x!\n", __FUNCTION__, bdmaRlt);
        }

        u32TblWr = u32DstOffset + (u32WrPtr - _drvSlqTbl.u32StAdd);
        //printf(" wr=0x%lx, tblWr=0x%lx\n", writePtr, u32TblWr);
        u32WrPtr = u32TblWr;
    }
    //printf("wPtr= 0x%lx(0x%lx) rPtr=0x%lx(0x%lx)\n", writePtr, _drvSlqTbl.u32WrPtr,
    //    MDrv_MVD_GetSLQReadPtr(), _drvSlqTbl.u32RdPtr);
#endif

    writePtrLast = u32WrPtr;

    MS_ASSERT((u32WrPtr%8)==0);
    u32WrPtr >>= 3;

    SET_CMDARG(mvdcmd, u32WrPtr);

    HAL_MVD_CPU_Sync();

    if (HAL_MVD_MVDCommand( CMD_SLQ_UPDATE_TBL_WPTR, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SLQ_UPDATE_TBL_WPTR ) );
        return;
    }

    return;
}

//------------------------------------------------------------------------------
/// Get MVD decoded frame index.  For debug.
/// @return -frame index of the decoded frame
//------------------------------------------------------------------------------
MS_U8 MDrv_MVD_GetDecodedFrameIdx ( void )
{
    MVD_FUNC_ENTRY();
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetDecodedFrameIdx error: pu8MVDGetFrameInfoBufStart=NULL\n"));
        return 0;
    }

    return HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_FB_INDEX);
}

//------------------------------------------------------------------------------
/// Get MVD VLD error count
/// @return -VLD error count
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetVldErrCount( void )
{
    MVD_FUNC_ENTRY();
#if defined(CHIP_T7)
    return __MVD_GetVldErrCount();
#else
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetVldErrCount error!\n"));
        return 0;
    }

    return HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart + OFFSET_VLD_ERR_COUNT);
#endif
}


//Get ErrStatus when ErrCode==ErrShape
static MVD_ErrStatus MVD_GetErrShapeStat(MS_U32 u32errStat)
{
    MVD_ErrStatus st = E_MVD_ERR_STATUS_UNKOWN;
    switch (u32errStat)
    {
        case 0:
            st = E_MVD_ERR_SHAPE_RECTANGULAR;
            break;
        case 1:
            st = E_MVD_ERR_SHAPE_BINARY;
            break;
        case 2:
            st = E_MVD_ERR_SHAPE_BINARY_ONLY;
            break;
        case 3:
            st = E_MVD_ERR_SHAPE_GRAYSCALE;
            break;
        default:
            break;
    }
    return st;
}

//Get ErrStatus when ErrCode==ErrSprite
static MVD_ErrStatus MVD_GetErrSpriteStat(MS_U32 u32errStat)
{
    MVD_ErrStatus st = E_MVD_ERR_STATUS_UNKOWN;
    switch (u32errStat)
    {
        case 0:
            st = E_MVD_ERR_USED_SPRITE_UNUSED;
            break;
        case 1:
            st = E_MVD_ERR_USED_SPRITE_STATIC;
            break;
        case 2:
            st = E_MVD_ERR_USED_SPRITE_GMC;
            break;
        case 3:
            st = E_MVD_ERR_USED_SPRITE_RESERVED;
            break;
        default:
            break;
    }
    return st;
}


//------------------------------------------------------------------------------
/// Get MVD error info.
/// This function can be used to diagnosis when the 1st DispReady doesn't occur.
///
/// @param - errorCode \b OUT : error code
/// @param - errorStatus \b OUT : error status
//------------------------------------------------------------------------------
void MDrv_MVD_GetErrInfo(MVD_ErrCode *errCode, MVD_ErrStatus *errStatus)
{
    MS_U32 errorCode = E_MVD_ERR_UNKNOWN;
    MS_U32 errorStatus = E_MVD_ERR_STATUS_UNKOWN;

    MVD_FUNC_ENTRY();
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetErrInfo error!\n"));
        return;
    }

    errorCode = HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart + OFFSET_ERROR_CODE);
    errorStatus = HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart + OFFSET_ERROR_STATUS);

    switch (errorCode)
    {
        case VOL_SHAPE:
            *errCode = E_MVD_ERR_SHAPE;
            *errStatus = MVD_GetErrShapeStat(errorStatus);
            break;
        case VOL_USED_SPRITE:
            *errCode = E_MVD_ERR_USED_SPRITE;
            *errStatus = MVD_GetErrSpriteStat(errorStatus);
            break;
        case VOL_NOT_8_BIT:
            *errCode = E_MVD_ERR_NOT_8_BIT;
            *errStatus = E_MVD_ERR_STATUS_NONE;
            break;
        case VOL_NERPRED_ENABLE:
            *errCode = E_MVD_ERR_NERPRED_ENABLE;
            *errStatus = E_MVD_ERR_STATUS_NONE;
            break;
        case VOL_REDUCED_RES_ENABLE:
            *errCode = E_MVD_ERR_REDUCED_RES_ENABLE;
            *errStatus = E_MVD_ERR_STATUS_NONE;
            break;
        case VOL_SCALABILITY:
            *errCode = E_MVD_ERR_SCALABILITY;
            *errStatus = E_MVD_ERR_STATUS_NONE;
            break;
        case VOL_H263_ERROR:
            *errCode = E_MVD_ERR_H263_ERROR;
            *errStatus = E_MVD_ERR_STATUS_NONE;
            break;
        case VOL_RES_NOT_SUPPORT:
            *errCode = E_MVD_ERR_RES_NOT_SUPPORT;
            *errStatus = E_MVD_ERR_STATUS_NONE;
            break;
        case VOL_MPEG4_NOT_SUPPORT:
            *errCode = E_MVD_ERR_MPEG4_NOT_SUPPORT;
            *errStatus = E_MVD_ERR_STATUS_NONE;
            break;
        case VOL_OTHER:
            *errCode = E_MVD_ERR_OTHER;
            *errStatus = E_MVD_ERR_STATUS_NONE;
            break;
        default:
            *errCode = E_MVD_ERR_UNKNOWN;
            *errStatus = E_MVD_ERR_STATUS_UNKOWN;
            break;
    }

    return;
}

//------------------------------------------------------------------------------
/// Get valid MVD stream detected flag
/// For MPEG-2, width<16(1 macroblock), height<16, width>1920, or height>1080,
/// the stream would be considered as invalid.
/// For VC-1, width or height <32 or width>2048 or height>1080, the stream is
/// invalid.
/// @return -decoded flag
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetValidStreamFlag( void )
{
    MVD_FUNC_ENTRY();
#if defined(CHIP_T7)
    return __MVD_GetValidStreamFlag();
#else
    //printf("  validStream=%d\n\n",
    //    !HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart + OFFSET_INVALIDSTREAM));
    return !HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart + OFFSET_INVALIDSTREAM);
#endif
}


//------------------------------------------------------------------------------
/// Get video frame information from MVD
/// @param -pinfo \b IN : pointer to video frame information
//------------------------------------------------------------------------------
void MDrv_MVD_GetFrameInfo(MVD_FrameInfo *pinfo)
{
    MVD_FUNC_ENTRY();

#if defined (CHIP_T7)
    __MVD_GetFrameInfo(pinfo);
   
#else
    MS_U32 u32DAR_Width=0,u32DAR_Height=0,u32PAR_Width=0,u32PAR_Height=0;
    MS_U32 u32Vertical_Size=0, u32Horizontal_Size=0;
    //13818-2 page 38 Table 6-3
    MS_U8 u8DARTable[5][2] = { {1,1}, {1,1}, {4,3}, {16,9}, {221,100} };

    if (pu8MVDGetVolBufStart == NULL)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetFrameInfo error: pu8MVDGetVolBufStart=NULL\n"));
        return;
    }

//    MVD_ReadVolInfo(&gvolInfo);
    HAL_MVD_CPU_Sync();
    HAL_MVD_ReadMemory();
    gvolInfo = *(volatile FW_VOL_INFO*)u32VolAdd;

#if 0
    MVD_DEBUGINFO(printf("vol info,vol_info=%d,sprite_usage=%d,pts_incr=%d,\n",
                  gvolInfo.vol_info,gvolInfo.sprite_usage,gvolInfo.pts_incr));
    MVD_DEBUGINFO(printf("vol info,width=%x,height=%x,frame_rate=%d,aspect_ratio=%x,\n",
                  gvolInfo.width,gvolInfo.height,gvolInfo.frame_rate,gvolInfo.aspect_ratio));
    MVD_DEBUGINFO(printf("vol info,progressive_sequence=%x,mpeg1=%x,play_mode=%x,bit_rate=%x,\n",
                  gvolInfo.progressive_sequence,gvolInfo.mpeg1,gvolInfo.play_mode,gvolInfo.bit_rate));
#endif

    pinfo->u16HorSize = (MS_U16) gvolInfo.width;
    pinfo->u16VerSize = (MS_U16) gvolInfo.height;
    pinfo->u8par_width  = (MS_U8) gvolInfo.par_width;
    pinfo->u8par_height = (MS_U8) gvolInfo.par_height;
    pinfo->u8AspectRate = gvolInfo.aspect_ratio;

    pinfo->u16CropBottom = 0;
    pinfo->u16CropTop = 0;
    pinfo->u16CropLeft = 0;

    if (pinfo->u16HorSize & 0x07)
    {
        //Notice: Firmware and driver have to be consistent for this part,
        // otherwise the pitch will not match and video is noisy.
        pinfo->u16CropRight = 8 - (pinfo->u16HorSize & 0x0007);
        pinfo->u16HorSize = ((pinfo->u16HorSize >> 3) + 1) << 3;
    }
    else
    {
        pinfo->u16CropRight = 0;
    }

    if (gvolInfo.progressive_sequence == 0)
    {
        pinfo->u8Interlace=1;
    }
    else
    {
        pinfo->u8Interlace=0;
    }

    ///Calculate PAR info
    if(pinfo->u8par_width == 0 || pinfo->u8par_height == 0)
    {
        if ((pinfo->u8AspectRate > 0) && (pinfo->u8AspectRate < 5 ))
        {
            u32DAR_Width = (MS_U32)u8DARTable[pinfo->u8AspectRate][0];
            u32DAR_Height = (MS_U32)u8DARTable[pinfo->u8AspectRate][1];
        }
        else
        {
            //set to 0 to indicate it's abnormal
            u32DAR_Width = 0;
            u32DAR_Height = 0;
        }

        u32Vertical_Size = (MS_U32)pinfo->u16VerSize;
        u32Horizontal_Size = (MS_U32)pinfo->u16HorSize;

        MVD_DEBUGVERBAL(printf("u32DAR_Width:%ld,u32DAR_Height%ld\n",u32DAR_Width,u32DAR_Height));

        u32PAR_Width = u32DAR_Width * u32Vertical_Size;
        u32PAR_Height = u32DAR_Height * u32Horizontal_Size;
        MVD_DEBUGVERBAL(printf("u32PAR_Width:%ld,u32PAR_Height%ld\n",u32PAR_Width,u32PAR_Height));

        pinfo->u8par_width = (MS_U16)u32PAR_Width;
        pinfo->u8par_height = (MS_U16)u32PAR_Height;
        MVD_DEBUGVERBAL(printf("pinfo->u8par_width:%d, pinfo->u8par_height:%d\n",pinfo->u8par_width, pinfo->u8par_height));
        pinfo->u16SarWidth  = (MS_U16)u32DAR_Width;
        pinfo->u16SarHeight = (MS_U16)u32DAR_Width;
    }
    else
    {
        //mpeg4 usually has non-zero u8par_width and u8par_height
        pinfo->u16SarWidth  = (MS_U16)((MS_U32)pinfo->u8par_width * u32Horizontal_Size);
        pinfo->u16SarHeight = (MS_U16)((MS_U32)pinfo->u8par_height* u32Vertical_Size);
    }
    MVD_DEBUGVERBAL(printf("pinfo->u16SarWidth:%d, pinfo->u16SarHeight:%d\n",pinfo->u16SarWidth, pinfo->u16SarHeight));

#if defined(_MVD4)
    if (curCodecType == E_MVD_CODEC_MPEG2)
    {
        if ((gvolInfo.frame_rate > 8 && pinfo->u8Interlace == 0)
           ||(gvolInfo.frame_rate > 5 && pinfo->u8Interlace == 1))
        {
            pinfo->u32FrameRate = 0;
        }
        else
        {
            pinfo->u32FrameRate = stFrameRateCode[gvolInfo.frame_rate];
        }
    }
    else if ((curCodecType == E_MVD_CODEC_VC1_ADV) || (curCodecType == E_MVD_CODEC_VC1_MAIN))
    {
        if (gvolInfo.vc1_frame_rate != 0)
        {
            pinfo->u32FrameRate = gvolInfo.vc1_frame_rate;
        }
        else
        {
            pinfo->u32FrameRate = gdivxInfo.frame_rate; //report framerate specified in MDrv_MVD_SetFrameInfo()
        }

        MVD_DEBUGVERBAL(printf("MVD: vc1_frameRate=%ld\n", pinfo->u32FrameRate));
    }
    else if (curCodecType == E_MVD_CODEC_MPEG4)
    {
        if (gdivxInfo.frame_rate != 0)
        {
            pinfo->u32FrameRate = gdivxInfo.frame_rate;
            //report framerate specified in MDrv_MVD_SetFrameInfo(), which is usually obtained from container
        }
        else if (gvolInfo.frame_rate != 0)
        {
            pinfo->u32FrameRate = gvolInfo.frame_rate;  //report framerate from f/w
        }
        else if (gvolInfo.fixed_vop_time_incr != 0)
        {
            pinfo->u32FrameRate = (gvolInfo.vol_time_incr_res * 1000) / gvolInfo.fixed_vop_time_incr;
            //calculate framerate according to vol header
        }
        else
        {
            pinfo->u32FrameRate = 25000; //set default frame rate according to country
        }
        MVD_DEBUGVERBAL(printf("MVD: vol_time_incr_res=%d, fixed_vop_time_incr=%d\n", gvolInfo.vol_time_incr_res, gvolInfo.fixed_vop_time_incr));
    }
    else
    {
        if (gvolInfo.frame_rate != 0)
        {
            pinfo->u32FrameRate = gvolInfo.frame_rate;
        }
        else
        {
            pinfo->u32FrameRate = gdivxInfo.frame_rate; //report framerate specified in MDrv_MVD_SetFrameInfo()
        }
    }
#else
    if((gvolInfo.frame_rate > 8 && pinfo->u8Interlace == 0)
       ||(gvolInfo.frame_rate > 5 && pinfo->u8Interlace == 1))
    {
        pinfo->u32FrameRate = 0;
    }
    else
    {
        pinfo->u32FrameRate = stFrameRateCode[gvolInfo.frame_rate];
    }
#endif

    //printf("===> MVD: frameRate=%d  curCodecType=0x%x\n", pinfo->u32FrameRate, curCodecType);

    //for MM
    pinfo->u8MPEG1=gvolInfo.mpeg1;
    pinfo->u16PTSInterval=gvolInfo.pts_incr;
    pinfo->u8PlayMode=gvolInfo.play_mode;
    pinfo->u8FrcMode=gvolInfo.mpeg_frc_mode;

    //for dynamic scaling
    pinfo->bEnableMIUSel = stMemCfg.bFWMiuSel;
    if (stMemCfg.bEnableDynScale)
    {
        if (stMemCfg.bFWMiuSel == MIU_SEL_1)
        {
            pinfo->u32DynScalingAddr= u32DynScalingAdd + stMemCfg.u32Miu1BaseAddr;
        }
        else
        {
            pinfo->u32DynScalingAddr= u32DynScalingAdd;
        }
        pinfo->u8DynScalingDepth= u8DynScalingDepth;
    }
    else
    {
        pinfo->u32DynScalingAddr= NULL;
        pinfo->u8DynScalingDepth= 0;
    }

    return;
#endif
}

//------------------------------------------------------------------------------
/// Set video frame information from MVD
/// @param -pinfo \b IN : pointer to video frame information
//------------------------------------------------------------------------------
void MDrv_MVD_SetFrameInfo(MVD_FrameInfo *pinfo )
{
    MVD_FUNC_ENTRY();
    if(pu8MVDSetHeaderBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_SetFrameInfo error: pu8MVDSetHeaderBufStart=NULL\n"));
        return;
    }

    u32MVDFirstVOLUpdate=TRUE;

    gdivxInfo.width=pinfo->u16HorSize;
    gdivxInfo.height=pinfo->u16VerSize;
    gdivxInfo.aspect_ratio=pinfo->u8AspectRate;
    gdivxInfo.frame_rate = pinfo->u32FrameRate;

    //for MM
    gdivxInfo.mpeg1=pinfo->u8MPEG1;
    gdivxInfo.pts_incr=pinfo->u16PTSInterval;
    gdivxInfo.play_mode=pinfo->u8PlayMode;
    gdivxInfo.mpeg_frc_mode=pinfo->u8FrcMode;

    if(pinfo->u8Interlace==0)
        gdivxInfo.progressive_sequence=1;
    else
        gdivxInfo.progressive_sequence=0;

    gdivxInfo.frame_count=0;
    gdivxInfo.frame_time=0;
    gdivxInfo.vol_handle_done=0;
//    gdivxInfo.invalidstream=0;
    MVD_DEBUGINFO(printf("set vol info,pts_incr=%d,\n",gdivxInfo.pts_incr));
    MVD_DEBUGINFO(printf("set vol info,width=%x,height=%x,frame_rate=%d,aspect_ratio=%x,\n",
                  (unsigned int)gdivxInfo.width,(unsigned int)gdivxInfo.height,gdivxInfo.frame_rate,gdivxInfo.aspect_ratio));
    MVD_DEBUGINFO(printf("set vol info,progressive_sequence=%x,mpeg1=%x,play_mode=%x,\n",
                  gdivxInfo.progressive_sequence,gdivxInfo.mpeg1,gdivxInfo.play_mode));

    MVD_WriteDivx311Data(&gdivxInfo);
    return;
}


//------------------------------------------------------------------------------
/// Reset for I-frame decoding
/// @return -none
//------------------------------------------------------------------------------
void MDrv_MVD_RstIFrameDec( void )
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_STOP, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_STOP ) );
    }

    MDrv_MVD_Rst();
    return;
}

//take care MIU1 address
static MS_U32 MVD_GetMemOffset(MS_PHYADDR u32PhyAdd)
{
    if (u32PhyAdd >= stMemCfg.u32Miu1BaseAddr)
    {
        return (u32PhyAdd - stMemCfg.u32Miu1BaseAddr);
    }
    else
    {
        return u32PhyAdd;
    }
}

//------------------------------------------------------------------------------
/// Decode I-frame
/// @param -u32FrameBufAddr \b IN : start address of frame buffer
/// @param -u32StreamBufAddr \b IN : start address of stream buffer
/// @return -return decode I-frame success or not
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_DecodeIFrame(MS_PHYADDR u32FrameBufAddr, MS_PHYADDR u32StreamBufAddr, MS_PHYADDR u32StreamBufEndAddr )
{
#if defined(CHIP_T7)
    u32FrameBufAddr     = MVD_GetMemOffset(u32FrameBufAddr);
    u32StreamBufAddr    = MVD_GetMemOffset(u32StreamBufAddr);
    u32StreamBufEndAddr = MVD_GetMemOffset(u32StreamBufEndAddr);
    _bDecodeIFrame = TRUE;
    return __MVD_DecodeIFrame( u32FrameBufAddr, u32StreamBufAddr, u32StreamBufEndAddr);
#else
    MS_U32 u32deley = 0;
    MS_U32 u32time = 0;
    MVD_CmdArg mvdcmd;

    MVD_DEBUGINFO(printf("%s input  FBAdd=0x%lx streamStart=0x%lx streamEnd=0x%lx\n",
            __FUNCTION__, u32FrameBufAddr, u32StreamBufAddr, u32StreamBufEndAddr));
    u32FrameBufAddr     = MVD_GetMemOffset(u32FrameBufAddr);
    u32StreamBufAddr    = MVD_GetMemOffset(u32StreamBufAddr);
    u32StreamBufEndAddr = MVD_GetMemOffset(u32StreamBufEndAddr);
    MVD_DEBUGINFO(printf("%s offset FBAdd=0x%lx streamStart=0x%lx streamEnd=0x%lx\n",
            __FUNCTION__, u32FrameBufAddr, u32StreamBufAddr, u32StreamBufEndAddr));

    MDrv_MVD_SetFrameBuffAddr( u32FrameBufAddr);
    MDrv_MVD_SetCodecInfo(E_MVD_CODEC_MPEG2, E_MVD_SLQ_MODE, DISABLE_PARSER);

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = 1;
    if (HAL_MVD_MVDCommand( CMD_DISPLAY_CTL, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DISPLAY_CTL ) );
        return FALSE;
    }

    if (MDrv_MVD_StepDecode() == FALSE)
    {
        MVD_DEBUGERROR( printf( "MDrv_MVD_StepDecode fail!!\r\n") );
        return FALSE;
    }

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = 1;
    mvdcmd.Arg1 = 1;
    if (HAL_MVD_MVDCommand(CMD_FAST_SLOW, &mvdcmd) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_FAST_SLOW ) );
        return FALSE;
    }

    //set data
    HAL_MVD_Delayms(2);

    HAL_MVD_CPU_Sync();
    HAL_MVD_FlushMemory();

    MDrv_MVD_SetSLQStartEnd(u32StreamBufAddr, u32StreamBufEndAddr);
    MVD_DEBUGINFO(printf("set MVD3_FW_SLQ_TAB_TMPBUF_ADR=%lx\n",u32MVDFWSLQTABTmpbufAdr));

    HAL_MVD_CPU_Sync();
    HAL_MVD_FlushMemory();
    MDrv_MVD_SetSLQStartEnd(u32MVDFWSLQTABTmpbufAdr, u32MVDFWSLQTABTmpbufAdr+MVD3_FW_SLQ_TAB_TMPBUF_LEN);

    HAL_MVD_CPU_Sync();
    HAL_MVD_ReadMemory();

    // wait decode complete
#define WAIT_DECODE_DONE_TIME 0x1000
    while (MDrv_MVD_GetPicCounter()<1 && u32deley++ < WAIT_DECODE_DONE_TIME);
    if(u32deley >= WAIT_DECODE_DONE_TIME)
    {
        MVD_DEBUGERROR(printf ("MDrv_MVD_DecodeIFrame time out(%ld, %ld)\n", u32time, u32deley));
        //printf("frmCnt=%ld state=0x%x lastCmd=0x%x\n", MDrv_MVD_GetPicCounter(), MDrv_MVD_GetDecodeStatus(), MDrv_MVD_GetLastCmd());
        return FALSE;
    }
    MVD_DEBUGINFO(printf ("MDrv_MVD_DecodeIFrame time (%ld, %ld)\n", u32time, u32deley));
    //printf("frmCnt=%ld state=0x%x lastCmd=0x%x\n", MDrv_MVD_GetPicCounter(), MDrv_MVD_GetDecodeStatus(), MDrv_MVD_GetLastCmd());

    _bDecodeIFrame = TRUE;

    return TRUE;
#endif
}

//------------------------------------------------------------------------------
/// Query if MVD is decoding frame
/// @return -MVD is decoding frame or not
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetIsIFrameDecoding(void)
{
    return _bDecodeIFrame;
}

//------------------------------------------------------------------------------
/// Set bitstream buffer overflow threshold
/// @return -none
//------------------------------------------------------------------------------
void MDrv_MVD_SetOverflowTH(MS_U32 u32Threshold)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    MS_ASSERT((u32Threshold%8)==0);
    u32Threshold >>= 3;
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32Threshold);
    mvdcmd.Arg1 = H_WORD(u32Threshold);
    mvdcmd.Arg2 = L_DWORD(u32Threshold);
    mvdcmd.Arg3 = 0;

    if (HAL_MVD_MVDCommand( CMD_DMA_OVFTH, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DMA_OVFTH ) );
        return;
    }
    return;
}

//------------------------------------------------------------------------------
/// Set bitstream buffer underflow threshold
/// @return -none
//------------------------------------------------------------------------------
void MDrv_MVD_SetUnderflowTH(MS_U32 u32Threshold)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    MS_ASSERT((u32Threshold%8)==0);
    u32Threshold >>= 3;
    SETUP_CMDARG(mvdcmd);    
    mvdcmd.Arg0 = L_WORD(u32Threshold);
    mvdcmd.Arg1 = H_WORD(u32Threshold);
    mvdcmd.Arg2 = L_DWORD(u32Threshold);
    mvdcmd.Arg3 = 0;

    if (HAL_MVD_MVDCommand( CMD_DMA_UNFTH, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DMA_UNFTH ) );
        return;
    }
    return;
}

//------------------------------------------------------------------------------
/// Get MVD active format
/// @return -active format
//------------------------------------------------------------------------------
MS_U8 MDrv_MVD_GetActiveFormat(void)
{
#if defined (CHIP_T7)
    return __MVD_GetActiveFormat();
#else
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_GET_AFD, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_GET_AFD  ) );
        return 0xff;
    }
    return mvdcmd.Arg0;
#endif
}

//------------------------------------------------------------------------------
/// Enable or disable AV synchronization.
/// Delay u32Delay ms if AVSync is enabled.
/// @return -none
//------------------------------------------------------------------------------
void MDrv_MVD_SetAVSync(MS_BOOL bEnable, MS_U32 u32Delay)
{
#if defined(CHIP_T7)
    __MVD_SetAVSync(bEnable, u32Delay);
    bAVSyncOn = bEnable;    //for IsAVSyncOn
    stSyncCfg.bEnable = bEnable;
    stSyncCfg.u32Delay = u32Delay;
    return;
#else
    MVD_CmdArg mvdcmd;

    //printf("%s bEnable=%d u32Delay=%ld\n",__FUNCTION__,bEnable,u32Delay);
    MVD_FUNC_ENTRY();

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = bEnable;
    bAVSyncOn = bEnable;    //for IsAVSyncOn
    if (HAL_MVD_MVDCommand( CMD_SYNC_ON, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_SYNC_ON  ) );
        return;
    }

    if (bEnable && (u32Delay != 0))
    {
        SET_CMDARG(mvdcmd, _MS_TO_90K(u32Delay)); //u32Delay ms ==> 90k counter
        if (HAL_MVD_MVDCommand( CMD_SYNC_OFFSET, &mvdcmd ) == FALSE)
        {
            MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_SYNC_ON  ) );
            return;
        }
    }

    stSyncCfg.bEnable = bEnable;
    stSyncCfg.u32Delay = u32Delay;
    return;
#endif
}

//------------------------------------------------------------------------------
/// Set skip & repeat frame mode.
/// mode : 1 skip B frame 0 : skip display frame
/// @return -none
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SetSkipRepeatMode(MS_U8 u8Mode)
{
    MVD_CmdArg mvdcmd;
	int i = 0;
    MVD_FUNC_ENTRY();
	
    if (u8Mode != 1 && u8Mode != 0)
    {
        return FALSE; //invalid parameter, do nothing
    }
	
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg4 = u8Mode;
    mvdcmd.Arg5 = u8Mode;
    if (HAL_MVD_MVDCommand( CMD_SYN_THRESHOLD , &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SYN_THRESHOLD ) );
        return FALSE;
    }
	return TRUE;
}

//------------------------------------------------------------------------------
/// Set the maximum repeat times for one frame when av is not sync.
/// E.g. Arg5=0x01 mean that frame will be repeated once if av is not sync.
///      Arg5=0xff mean that frame will be always repeated when av is not sync.
/// @return -none
//------------------------------------------------------------------------------
void MDrv_MVD_SetAVSyncThreshold(MS_U32 u32Th)
{
#if defined(_MVD4)
    MVD_CmdArg mvdcmd;
    MVD_FUNC_ENTRY();

    if (u32Th == 0x00)
    {
        return; //invalid parameter, do nothing
    }
    if (u32Th > 0xff)
    {
        u32Th = 0xff; //set to maximum
    }

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg5 = u32Th;
    if (HAL_MVD_MVDCommand(CMD_SYN_THRESHOLD, &mvdcmd) == FALSE)
    {
        MVD_DEBUGERROR(printf("Ctrl: 0x%x fail!!\n", CMD_SYN_THRESHOLD));
        return;
    }

    return;
#endif
}


//------------------------------------------------------------------------------
/// Get if AVSync is turned on
/// @return -TRUE for yes or FALSE for no
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetIsAVSyncOn(void)
{
    return bAVSyncOn;
}


//------------------------------------------------------------------------------
/// Get if firmware is repeating frame for AVSync.
/// @return -TRUE for yes or FALSE for no
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetIsSyncRep(void)
{
    MS_U32 u32IntStat = 0;
    MVD_CmdArg mvdcmd;
    MS_BOOL bRet = FALSE;

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_GET_INT_STAT, &mvdcmd ) == TRUE)
    {
        u32IntStat = (((MS_U32)mvdcmd.Arg2) << 8) |
                     (((MS_U32)mvdcmd.Arg1) << 4) |
                     (((MS_U32)mvdcmd.Arg0));
        bRet = ((u32IntStat&INT_SYN_REP)==INT_SYN_REP) ? TRUE : FALSE;
    }
    else
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\n", CMD_GET_INT_STAT) );
    }

    return bRet;
}


//------------------------------------------------------------------------------
/// Get if firmware is skipping frame for AVSync.
/// @return -TRUE for yes or FALSE for no
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetIsSyncSkip(void)
{
    MS_U32 u32IntStat = 0;
    MVD_CmdArg mvdcmd;
    MS_BOOL bRet = FALSE;

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_GET_INT_STAT, &mvdcmd ) == TRUE)
    {
        u32IntStat = (((MS_U32)mvdcmd.Arg2) << 8) |
                     (((MS_U32)mvdcmd.Arg1) << 4) |
                     (((MS_U32)mvdcmd.Arg0));
        bRet = ((u32IntStat&INT_SYN_SKIP)==INT_SYN_SKIP) ? TRUE : FALSE;
    }
    else
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\n", CMD_GET_INT_STAT) );
    }

    return bRet;
}

//------------------------------------------------------------------------------
/// Change PTS threshold for doing AVSync.
/// Scenario: When disable black screen earlier than normal case, use this function
/// to avoid frame skip too fast or video lag.
/// @param -bEnable \b IN : enable this configuration
/// @param -u16PTS \b IN : PTS threshold (unit: 90k counter, i.e. x/90 ms, max 728ms)
/// @return -TRUE for success or FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_ChangeAVsync(MS_BOOL bEnable, MS_U16 u16PTS)
{
    MVD_CmdArg mvdcmd;

    stSyncCfg.u16Tolerance = u16PTS;

    u16PTS = _MS_TO_90K(u16PTS); //u16PTS ms ==> 90k counter
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = (MS_U8)bEnable;
    mvdcmd.Arg1 = 0;
    mvdcmd.Arg2 = (MS_U8)(u16PTS&0xff);
    mvdcmd.Arg3 = (MS_U8)((u16PTS&0xff00)>>8);

    if (HAL_MVD_MVDCommand( CMD_MVD_FAST_INT, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_MVD_FAST_INT  ) );
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
/// Get size of residual bitstream.
/// @return -size of residual bitstream
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetResidualStreamSize( void )
{
#if defined(CHIP_T7)
    return __MVD_GetResidualStreamSize();
#else
    MS_U32 u32Ret = 0;
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_VBUFFER_COUNT, &mvdcmd ) == TRUE)
    {
        u32Ret = (((MS_U32)mvdcmd.Arg2) << 19) |
                 (((MS_U32)mvdcmd.Arg1) << 11) |
                 (((MS_U32)mvdcmd.Arg0) << 3);
    }
    else
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\n", CMD_VBUFFER_COUNT  ) );
    }

    return u32Ret;
#endif
}

//------------------------------------------------------------------------------
/// Get address of the first I-frame.
/// @return -address of the first I-frame
//------------------------------------------------------------------------------
MS_U8 MDrv_MVD_GetIsIPicFound()
{
    MVD_FUNC_ENTRY();
#if defined(CHIP_T7)
    return __MVD_GetIsIPicFound();
#else
    return HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart + OFFSET_FIRST_I_FOUND);
#endif
}

//------------------------------------------------------------------------------
/// Get sync status to know whether sync is complete or not
/// @return - 1        : sync complete
////        - otherwise: sync not complete
//------------------------------------------------------------------------------
MS_U8 MDrv_MVD_GetSyncStatus(void)
{
#if defined(CHIP_T7)
    return __MVD_GetSyncStatus();
#else
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_GET_SYNC_STAT, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_GET_SYNC_STAT  ) );
        return 0xFF;
    }
    MVD_DEBUGINFO(printf("Sync On/Off %x, Sync Done %x\n", mvdcmd.Arg0, mvdcmd.Arg1));
    if(mvdcmd.Arg1 == MVD_SYNC_DONE)
        return 1;
    else
        return 0xFF;
#endif
}

//------------------------------------------------------------------------------
/// Get PTS (Presentation Time Stamp)
/// @return -PTS (unit: ms)
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetPTS(void)
{
#if defined(CHIP_T7)
    return __MVD_GetPTS();
#else
    MVD_CmdArg mvdcmd;
    MS_U32 u32PTS = 0;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_RD_PTS, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR(printf( "Ctrl: CMD_RD_PTS fail!!\n"  ) );
        u32PTS = MVD_U32_MAX;
    }
    else
    {
        u32PTS = ((MS_U32)mvdcmd.Arg3 << 24 | (MS_U32)mvdcmd.Arg2 << 16 |
                  (MS_U32)mvdcmd.Arg1 << 8  | (MS_U32)mvdcmd.Arg0);
        MVD_DEBUGVERBAL(printf("MDrv_MVD_GetPTS:0x%lx\n", u32PTS));
    }

    if (MVD_U32_MAX != u32PTS)
    {
        u32PTS = _90K_TO_MS(u32PTS);
    }

    return u32PTS;
#endif
}


//------------------------------------------------------------------------------
/// Query if MVD is ready to display
/// @return -MS_U8  0: not ready, !0: ready.
//------------------------------------------------------------------------------
MS_U8 MDrv_MVD_GetDispRdy(void)
{
    MVD_FUNC_ENTRY();
#if (defined(CHIP_T7))
    return __MVD_GetDispRdy();
#else
#if (defined(CHIP_T2))
    return HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart + OFFSET_FIRST_FRAME);
#else //T3 & U3
    if (MDrv_MVD_GetPicCounter() > 0)
        return 1;
    else
        return 0;
#endif
#endif
}

//------------------------------------------------------------------------------
/// Query if the first frame is showed after play function is called.
/// Whenever the first display frame is displayed, f/w will set this flag to 1.
/// @return VDEC_Result
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_Is1stFrmRdy(void)
{
    if (pu8MVDGetVolBufStart==0)
    {
        MVD_DEBUGERROR(printf("%s err: pu8MVDGetVolBufStart=NULL\n", __FUNCTION__));
        return FALSE;
    }

    return HAL_MVD_MemReadByte(pu8MVDGetVolBufStart + OFFSET_FIRST_DISPLAY);
}

//------------------------------------------------------------------------------
/// Get picture count of current GOP (Group of Picture)
/// @return -picture count of GOP
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetGOPCount(void)
{
    MVD_FUNC_ENTRY();
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetPicCounter error: pu8MVDGetFrameInfoBufStart=NULL\n"));
            return 0;
    }

    return HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_GOP_I_FCNT)
         + HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_GOP_P_FCNT)
         + HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_GOP_B_FCNT);
}

//------------------------------------------------------------------------------
/// Enable or disable dropping error frames
/// @return -command is set successfully or not
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_DropErrorFrame(MS_BOOL bDrop)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg1 = (MS_U8)bDrop;
    if (HAL_MVD_MVDCommand( CMD_DISPLAY_CTL, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_DISPLAY_CTL  ) );
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
/// Get byte count of parser.
/// @return -byte count of parser
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetParserByteCnt(void)
{
    MVD_CmdArg mvdcmd;
    MS_U32 u32Cnt = 0;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
#if (defined(CHIP_T2))
    mvdcmd.Arg0 = 0x28;
    if (HAL_MVD_MVDCommand( CMD_RD_IO, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_RD_IO  ) );
        return 0;
    }
    u32Cnt = (((MS_U32)mvdcmd.Arg2) << 16);

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = 0x27;
    if (HAL_MVD_MVDCommand( CMD_RD_IO, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_RD_IO  ) );
        return 0;
    }
    u32Cnt = u32Cnt + (((MS_U32)mvdcmd.Arg2)) + ((MS_U32)mvdcmd.Arg3 << 8);
#elif defined(_MVD4)
    mvdcmd.Arg0 = 0x34;
    mvdcmd.Arg1 = 0x2;
    if (HAL_MVD_MVDCommand( CMD_RD_IO, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_RD_IO  ) );
        return 0;
    }
    u32Cnt = (((MS_U32)mvdcmd.Arg2)) | ((MS_U32)mvdcmd.Arg3 << 8) |
             (((MS_U32)mvdcmd.Arg4) << 16) | (((MS_U32)mvdcmd.Arg5) << 24);
#endif

    //printf("  parser byte count = %lu byte \n", u32Cnt);
    return u32Cnt;
}


//------------------------------------------------------------------------------
/// Get the decode status.
/// @return -the decode status
//------------------------------------------------------------------------------
MVD_DecStat MDrv_MVD_GetDecodeStatus(void)
{
#if defined(CHIP_T7)
    return __MVD_GetDecodeStatus();
#else
    MVD_CmdArg mvdcmd;
    MVD_DecStat stat = E_MVD_STAT_UNKNOWN;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand(CMD_DECODE_STATUS, &mvdcmd) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DECODE_STATUS ) );
        return E_MVD_STAT_UNKNOWN;
    }

    switch(mvdcmd.Arg1)
    {
        case DEC_STAT_IDLE:
            stat = E_MVD_STAT_IDLE;
            break;
        case DEC_STAT_FIND_SC:
            stat = E_MVD_STAT_FIND_STARTCODE;
            break;
        case DEC_STAT_FIND_SPE_SC:
            stat = E_MVD_STAT_FIND_SPECIALCODE;
            break;
        case DEC_STAT_FIND_FRAMEBUFFER:
            stat = E_MVD_STAT_FIND_FRAMEBUFFER;
            break;
        case DEC_STAT_WAIT_DECODE_DONE:
            stat = E_MVD_STAT_WAIT_DECODEDONE;
            break;
        case DEC_STAT_DECODE_DONE:
            stat = E_MVD_STAT_DECODE_DONE;
            break;
        case DEC_STAT_WAIT_VDFIFO:
            stat = E_MVD_STAT_WAIT_VDFIFO;
            break;
        case DEC_STAT_INIT_SUCCESS:
            stat = E_MVD_STAT_INIT_SUCCESS;
            break;
        default:
            break;
    }

    return stat;
#endif
}


//------------------------------------------------------------------------------
/// Get the last command to firmware.
/// @return -the last command
//------------------------------------------------------------------------------
MS_U8 MDrv_MVD_GetLastCmd(void)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_DECODE_STATUS, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DECODE_STATUS ) );
        return 0xff;
    }

    return (mvdcmd.Arg0);
}


//------------------------------------------------------------------------------
/// Skip data in ES buffer.  It is used for flushing ES buffer.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SkipData(void)
{
#if defined(_MVD4)
    #define SKIP_DATA_TIMEOUT 0x20
    MS_U32 u32TimeOut = 0;
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);

    while(MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_SKIP_DATA) && (u32TimeOut < SKIP_DATA_TIMEOUT))
    {
        if (HAL_MVD_MVDCommand(CMD_SKIP_DATA, &mvdcmd) == FALSE)
        {
            MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_SKIP_DATA ) );
            return FALSE;
        }
        u32TimeOut++;
    }

    if(u32TimeOut >= SKIP_DATA_TIMEOUT)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_SKIP_DATA ) );
        return FALSE;
    }
    return TRUE;
#else
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand(CMD_SKIP_DATA, &mvdcmd) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_SKIP_DATA ) );
        return FALSE;
    }
    return TRUE;
#endif
}


//------------------------------------------------------------------------------
/// Skip to I frame.
/// Used for MediaCodec when input data is not continuous or in the beginning of
/// streams.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SkipToIFrame(void)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand(CMD_START_DEC_STRICT, &mvdcmd) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_START_DEC_STRICT  ) );
        return FALSE;
    }
    return TRUE;
}


//Map driver FRC (Frame rate conversion) mode to firmware's.
static MS_U8 MVD_MapFrcMode(MVD_FrcMode eFrcMode)
{
    MS_U8 frcMode = 0xf;
    switch (eFrcMode)
    {
        case E_MVD_FRC_NORMAL:
            frcMode = FrcNormal;
            break;
        case E_MVD_FRC_DISP_TWICE:
            frcMode = FrcDisplayTwice;
            break;
        case E_MVD_FRC_3_2_PULLDOWN:    //film 24 -> 50i
            frcMode = Frc32Pulldown;
            break;
        case E_MVD_FRC_PAL_TO_NTSC:
            frcMode = FrcPALtoNTSC;
            break;
        case E_MVD_FRC_NTSC_TO_PAL:
            frcMode = FrcNTSCtoPAL;
            break;
        case E_MVD_FRC_DISP_ONEFIELD:
            frcMode = FrcShowOneFiled;
            break;
        default:
            break;
    }
    return frcMode;
}


//------------------------------------------------------------------------------
/// Display control for decoding order, enabling drop error frame, dropping
/// display and setting FRC (frame rate conversion) mode.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_DispCtrl(MS_BOOL bDecOrder, MS_BOOL bDropErr, MS_BOOL bDropDisp, MVD_FrcMode eFrcMode)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = (MS_U8)bDecOrder;
    mvdcmd.Arg1 = (MS_U8)bDropErr;
    mvdcmd.Arg2 = (MS_U8)bDropDisp;
    mvdcmd.Arg3 = MVD_MapFrcMode(eFrcMode);

    if (HAL_MVD_MVDCommand( CMD_DISPLAY_CTL, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_DISPLAY_CTL  ) );
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
/// Enable/Disable to repeat the last field when repeat for avsync or display pause.
/// E.g. Top field first: T-B-B-B-...; Bottom field first: B-T-T-T-...
/// Usage scenario: scaler framebufferless mode to avoid frame flicker
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_DispRepeatField(MS_BOOL bEnable)
{
    #if (defined(CHIP_T7))
    MVD_DEBUGINFO( printf( "This Cmd is bypass in MVD1 <%x>!!\r\n", bEnable  ) );
    #else
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = bEnable;
    if (HAL_MVD_MVDCommand( CMD_REPEAT_MODE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\r\n", CMD_REPEAT_MODE  ) );
        return FALSE;
    }
    #endif
    return TRUE;
}


//------------------------------------------------------------------------------
/// Get RFF (Repeat First Field) flag.
/// @return -TRUE or FALSE
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetTop1stField(void)
{
    MVD_FUNC_ENTRY();

    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetTop1stField error: pu8MVDGetFrameInfoBufStart=NULL\n"));
        return FALSE;
    }

    if (HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart + OFFSET_TOP_FF))
        return TRUE;
    else
        return FALSE;
}


//------------------------------------------------------------------------------
/// Get RFF (Repeat First Field) flag.
/// @return -TRUE or FALSE
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetRepeat1stField(void)
{
    MVD_FUNC_ENTRY();

    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetRepeat1stField error: pu8MVDGetFrameInfoBufStart=NULL\n"));
        return FALSE;
    }

    if (HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart + OFFSET_REPEAT_FF))
        return TRUE;
    else
        return FALSE;
}

//------------------------------------------------------------------------------
/// Get tmp ref flag.
/// @return -TRUE or FALSE
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetTmpRefField(void)
{
    MVD_FUNC_ENTRY();

    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_GetTmpRefField error: pu8MVDGetFrameInfoBufStart=NULL\n"));
        return FALSE;
    }

    if (HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart + OFFSET_TMP_REF))
        return TRUE;
    else
        return FALSE;
}


//------------------------------------------------------------------------------
/// Issue Pause command.
//------------------------------------------------------------------------------
void MDrv_MVD_Pause(void)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
#if defined(_MVD4)
    mvdcmd.Arg0 = DISPLAY_PAUSE_ON;
    if (HAL_MVD_MVDCommand(CMD_DISPLAY_PAUSE, &mvdcmd)== FALSE)
#else
    if (HAL_MVD_MVDCommand(CMD_PAUSE, &mvdcmd)== FALSE)
#endif
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_PAUSE) );
        return;
    }

    HAL_MVD_Delayms(4);

    return;
}


//------------------------------------------------------------------------------
/// Issue "Decode Pause" command.
//------------------------------------------------------------------------------
void MDrv_MVD_DecodePause(void)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand(CMD_PAUSE, &mvdcmd)== FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_PAUSE) );
        return;
    }

    HAL_MVD_Delayms(4);

    return;
}


//------------------------------------------------------------------------------
/// Issue Resume command.
//------------------------------------------------------------------------------
void MDrv_MVD_Resume(void)
{
    MVD_FUNC_ENTRY();
#if defined(_MVD4)
    HAL_MVD_Resume();
    return;
#else
    MDrv_MVD_SetAVSync(TRUE, 0);
    MDrv_MVD_Play();
#endif

    return;
}


//------------------------------------------------------------------------------
/// Issue Stop command.
//------------------------------------------------------------------------------
static MS_BOOL MDrv_MVD_Stop(void)
{
    MVD_CmdArg mvdcmd;
    //MS_U32 u32TimeCnt;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand(CMD_STOP, &mvdcmd)== FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_STOP) );
        return FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
/// Is MVD step decode done after step decode command.
/// @return - TRUE/FALSE
/// @retval     -FALSE(0): decoding, or user did not send step decode command.
/// @retval     -TRUE(1): decode done
//-----------------------------------------------------------------------------
MS_BOOL MDrv_MVD_IsStepDecodeDone(void)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32DecodedFrameCnt = 0;

    MVD_FUNC_ENTRY();

    if (bStepDecode)
    {
        u32DecodedFrameCnt = MDrv_MVD_GetPicCounter();
        #if (defined(CHIP_T2))||(defined(CHIP_T7))
        if (u32StepDecodeCnt != u32DecodedFrameCnt)
        {
            u32StepDecodeCnt = u32DecodedFrameCnt;
            bStepDecode = FALSE;
            bRet = TRUE;
        }
        #else
        if (MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_SINGLE_STEP))
        {
            printf( "MVD_HANDSHAKE_SINGLE_STEP -------: 0x%x \n", MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_SINGLE_STEP));
            bStepDecode = FALSE;
            bRet = TRUE;
        }
        #endif
    }
    return bRet;
}

//------------------------------------------------------------------------------
/// Issue StepPlay command.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_StepDecode(void)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_SINGLE_STEP, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SINGLE_STEP ) );
        return FALSE;
    }

    if (HAL_MVD_Resume() == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: HAL_MVD_Resume fail!!\r\n" ) );
        return FALSE;
    }
    bStepDecode = TRUE;
    return TRUE;
}

//-----------------------------------------------------------------------------
/// Is MVD step display done after step display command.
/// @return - TRUE/FALSE
/// @retval     -FALSE(0): decoding, or user did not send step decode command.
/// @retval     -TRUE(1): decode done
//-----------------------------------------------------------------------------
MS_BOOL MDrv_MVD_IsStepDispDone(void)
{
    MVD_FUNC_ENTRY();

#if defined(_MVD4)
    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("%s err: pu8MVDGetFrameInfoBufStart=NULL\n", __FUNCTION__));
        return FALSE;
    }

    return HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart+OFFSET_STEP_DISP_DONE);
#else
    return FALSE;
#endif
}

//------------------------------------------------------------------------------
/// Issue StepDisplay command.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_StepDisp(void)
{
    MVD_FUNC_ENTRY();

#if _STEPDISP_DISABLE_AVSYNC
    if (ePlayMode == E_MVD_PLAY)
    {
    printf("SP: en=%x delay=0x%lx tolerance=0x%x\n", stSyncCfg.bEnable,
        stSyncCfg.u32Delay, stSyncCfg.u16Tolerance);
        //store avsync parameters before StepDisp
        stSyncCfgBfStepDisp.bEnable = stSyncCfg.bEnable;
        stSyncCfgBfStepDisp.u32Delay = stSyncCfg.u32Delay;
        stSyncCfgBfStepDisp.u16Tolerance = stSyncCfg.u16Tolerance;

        //disable avsyn to notify f/w to ignore stc
        MDrv_MVD_SetAVSync(FALSE, 0);
    }
    ePlayMode = E_MVD_STEPDISP;
#endif

    MDrv_MVD_SetSpeed(E_MVD_SPEED_DEFAULT, 1);
    bStepDisp = TRUE;
    if (HAL_MVD_StepDisp() == FALSE)
    {
        MVD_DEBUGERROR( printf( "%s fail!!\n", __FUNCTION__ ) );
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
/// Is MVD step display done after step display command.
/// @return - TRUE/FALSE
/// @retval     -FALSE(0): decoding, or user did not send step decode command.
/// @retval     -TRUE(1): decode done
//-----------------------------------------------------------------------------
MS_BOOL MDrv_MVD_IsStep2PtsDone(void)
{
    MVD_FUNC_ENTRY();

#if defined(_MVD4)
    if (!bStep2Pts)
    {
        return FALSE;
    }

    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("%s err: pu8MVDGetFrameInfoBufStart=NULL\n", __FUNCTION__));
        return FALSE;
    }

    return HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart+OFFSET_STEP_TO_PTS_DONE);
#else
    return FALSE;
#endif
}

//------------------------------------------------------------------------------
/// Step to the specific PTS (u32Pts ms)
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SeekToPTS(MS_U32 u32Pts)
{
    MVD_FUNC_ENTRY();

    MDrv_MVD_SetSpeed(E_MVD_SPEED_DEFAULT, 1);
    bStep2Pts = HAL_MVD_SeekToPTS(_MS_TO_90K(u32Pts));
    return bStep2Pts;
}

//------------------------------------------------------------------------------
/// Skip to the specific PTS (u32Pts ms)
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SkipToPTS(MS_U32 u32Pts)
{
    MVD_FUNC_ENTRY();

    bSkip2Pts = HAL_MVD_SkipToPTS(_MS_TO_90K(u32Pts));
    return bSkip2Pts;
}

//------------------------------------------------------------------------------
/// Issue TrickPlay command.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_TrickPlay(MVD_TrickDec trickDec, MS_U8 u8DispDuration)
{
    MVD_CmdArg mvdcmd;
    MS_U8 u8DecType;
    MVD_FUNC_ENTRY();

    switch (trickDec)
    {
        case E_MVD_TRICK_DEC_ALL:
            u8DecType = 0;
            break;
        case E_MVD_TRICK_DEC_I:
            u8DecType = 1;
            break;
        case E_MVD_TRICK_DEC_IP:
            u8DecType = 2;
            break;
        default:
            return FALSE;
            break;
    }

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = u8DecType;
    mvdcmd.Arg1 = u8DispDuration;
    eTrickMode = trickDec;    //for MDrv_MVD_GetTrickMode

    if (HAL_MVD_MVDCommand(CMD_FAST_SLOW, &mvdcmd) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_FAST_SLOW) );
        return FALSE;
    }
    MDrv_MVD_SetAVSync(FALSE, 0);
    return TRUE;
}


//------------------------------------------------------------------------------
/// Get the trick mode which has been set.
/// @return MVD_TrickDec
//------------------------------------------------------------------------------
MVD_TrickDec MDrv_MVD_GetTrickMode(void)
{
    return eTrickMode;
}


//------------------------------------------------------------------------------
/// Flush display buffer.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_FlushDisplayBuf(void)
{
    MVD_CmdArg mvdcmd;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(mvdcmd);
#if (defined(CHIP_T2) || defined(CHIP_JANUS) || defined(CHIP_T4))
    if (HAL_MVD_MVDCommand(CMD_FLUSH_LAST_IPFRAME, &mvdcmd) == FALSE)
#else
    if (HAL_MVD_MVDCommand(CMD_FLUSH_DISP_QUEUE, &mvdcmd) == FALSE)
#endif
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_FLUSH_LAST_IPFRAME) );
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
/// Get capabilities of MPEG Video Decoder.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GetCaps(MVD_Caps* pCaps)
{
    MS_U8 caps = HAL_MVD_GetCaps();
    if (!pCaps)
        return FALSE;

    pCaps->bMPEG2 = ((caps & MVD_SUPPORT_MPEG2) == MVD_SUPPORT_MPEG2);
    pCaps->bMPEG4 = ((caps & MVD_SUPPORT_MPEG4) == MVD_SUPPORT_MPEG4);
    pCaps->bVC1   = ((caps & MVD_SUPPORT_VC1) == MVD_SUPPORT_VC1);

    //printf("MP2=%d, MP4=%d, VC1=%d\n", pCaps->bMPEG2, pCaps->bMPEG4, pCaps->bVC1);
    return TRUE;
}


//------------------------------------------------------------------------------
/// Get if MVD decoder is playing.
/// @return TRUE or FALSE
///     - TRUE, Yes
///     - FALSE, No
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_IsPlaying(void)
{
    MS_BOOL bIsPlaying = FALSE;
    bIsPlaying = ( MDrv_MVD_GetDecodeStatus() == E_MVD_STAT_WAIT_DECODEDONE );

    return bIsPlaying;
}

//------------------------------------------------------------------------------
/// Get if MVD decoder is in idle state.
/// @return TRUE or FALSE
///     - TRUE, Yes
///     - FALSE, No
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_IsIdle(void)
{
    return (MDrv_MVD_GetDecodeStatus() == E_MVD_STAT_IDLE);
}


////////////////////////////////////////////////////////////////////////////////
//  FIXME::  Below functions are under construction.
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
/// Enable/Disable error concealment function
/// @param -bDisable \b IN : enable/disable this function
/// @return -return E_MVD_Result success/fail to enable/disable
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_DisableErrConceal(MS_BOOL bDisable)
{
    return E_MVD_RET_FAIL;
}

//------------------------------------------------------------------------------
/// Push queue.
/// @param -u32StAddr \b IN : the start address of the queue
/// @param -u32Size \b IN : the data size to be pushed
/// @param -u32TimeStamp \b IN : the corresponding PTS
/// @return -return E_MVD_Result success/fail to push queue
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_PushQueue(MVD_PacketInfo* pInfo)
{
    E_MVD_Result eRet=E_MVD_RET_INVALID_PARAM;
#if _SLQTBL_DUMP_PUSHQ
    static MS_U32 u32pqTimes = 0;

    printf("Push[%lx] st=0x%lx len=0x%lx pts=0x%lx\n", u32pqTimes++,
            pInfo->u32StAddr, pInfo->u32Length, pInfo->u32TimeStamp);
#endif
    MVD_FUNC_ENTRY();
    //check input parameters
    if (pInfo == NULL)
    {
        printf("PushQueue NULL pInfo\n");
        eRet=E_MVD_RET_INVALID_PARAM;
    }
#if 0
    else if ((pInfo->u32TimeStamp!= 0) && (pInfo->u32Length==0))
    {
        printf("PushQueue invalid pInfo pts=0x%lx\n", pInfo->u32TimeStamp);
        eRet=E_MVD_RET_INVALID_PARAM;
    }
#endif
    else if (pInfo->u32StAddr >= stMemCfg.u32BSSize)
    {
        //since u32StAddr is offset, it shouldn't be larger than size.
        printf("PushQueue invalid u32StAddr=0x%lx, bsSize=0x%lx\n", pInfo->u32StAddr, stMemCfg.u32BSSize);
        eRet=E_MVD_RET_INVALID_PARAM;
    }
    else if ((pInfo->u32TimeStamp == MVD_NULLPKT_PTS) && (pInfo->u32Length==0))
    {
        // AVI NULL packet.
        u32DummyPktCnt++;
        //printf("Pos:0x%x%08x; PTS:%08d; NullPKT:%d\n", pInfo->u32ID_H, pInfo->u32ID_L, pInfo->u32TimeStamp, u32DummyPktCnt);
        return E_MVD_RET_OK;
    }

    //printf(".Pos:0x%x%08x; PTS:%08d; NullPKT:%d\n", pInfo->u32ID_H, pInfo->u32ID_L, pInfo->u32TimeStamp, u32DummyPktCnt);
    if (pInfo->u32TimeStamp == u32LastPts)
    {
        //printf("Repeat PTS!\n");
        if (pInfo->u32TimeStamp != MVD_NULLPKT_PTS)
            pInfo->u32TimeStamp = MVD_NULLPKT_PTS; //repeat PTS
    }
    else
    {
        u32LastPts = pInfo->u32TimeStamp;
    }

    //check queue vacancy
    if (_drvSlqTbl.u32Empty >= SLQ_TBL_SAFERANGE)
    {   //put packets
#if 1
        if (E_MVD_CODEC_DIVX311 == curCodecType)
        {
            MVD_PacketInfo stDivxHdr;
            MVD_SLQTblGetDivxHdrPkt(&stDivxHdr, pInfo);
            MVD_SLQTblSendPacket(&stDivxHdr);
        }
#endif
        MVD_SLQTblSendPacket(pInfo);
        eRet=E_MVD_RET_OK;
    }
    else
    {
        MS_ASSERT(0); //shouldn't be here!
        printf("PushQueue FULL!!! empty=0x%lx\n", _drvSlqTbl.u32Empty);
        //Player will only push queue when queue vacancy != 0
        eRet=E_MVD_RET_QUEUE_FULL;
    }

    if (E_MVD_RET_OK != eRet)
        printf("%s ret = %d\n", __FUNCTION__, eRet);
    return eRet;
}


static E_MVD_Result MVD_FlushTSQueue(void)
{
    MS_U32 u32TimeOut = 10;
    static MS_BOOL bSetSkip = FALSE;

    if (MDrv_MVD_GetLastCmd()!=CMD_PAUSE)
    {
        MDrv_MVD_Pause();
        MDrv_MVD_DecodePause();
        MDrv_MVD_FlushDisplayBuf();
    }
    if (!bSetSkip)
    {
        MDrv_MVD_SkipData();
        bSetSkip = TRUE;
    }

    while (u32TimeOut>0)
    {
        HAL_MVD_Delayms(5);

        if ( (MDrv_MVD_GetDecodeStatus()==E_MVD_STAT_FIND_SPECIALCODE)
              && (MDrv_MVD_GetLastCmd()==CMD_PAUSE) )
        {
            break;
        }
#if defined(_MVD4)
        else if (MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_SKIP_DATA))
        {
            break;
        }
#endif
        u32TimeOut--;
    }

    if (u32TimeOut==0)
    {
        MVD_DEBUGERROR(printf("\n***** MVD_FlushTSQueue TS flush TIMEOUT!!! *****\n\n"));
        return E_MVD_RET_FAIL;
    }

    bSetSkip = FALSE;
    return E_MVD_RET_OK;
}

static MS_BOOL MVD_PatternLenIsValid(MS_U32 u32Len)
{
#if defined(_MVD4)
    MS_U32 u32ValidLen = 0;
#define MAX_VALIDLEN    0x200000    //2M

    if (E_MVD_CODEC_VC1_MAIN != curCodecType)
    {
        return TRUE;
    }
    else
    {
        //only RCV has to check this
        u32ValidLen = HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_RCV_PAYLOAD_LENGTH);
        if (u32ValidLen > MAX_VALIDLEN)
        {   //avoid the extreme large value due to error bitstream
            u32ValidLen = MAX_VALIDLEN; 
        }
        MVD_DEBUGVERBAL(printf("(%x) ValidLen=0x%lx CurLen=0x%lx\n",
                       (u32Len > u32ValidLen), u32ValidLen, u32Len));
        return (u32Len > u32ValidLen);
    }
#else
    return TRUE;
#endif
}


static E_MVD_Result MVD_FlushSlqTblQueue(void)
{
    MS_U32 u32TimeCnt, u32TimeOut;
    MS_U32 u32PatternByteCnt = 0;

    MDrv_MVD_Pause();
    MDrv_MVD_DecodePause();
    MDrv_MVD_FlushDisplayBuf();

    u32TimeCnt = HAL_MVD_GetTime();
    while (((HAL_MVD_GetTime() - u32TimeCnt) < CMD_TIMEOUT_MS) ||
           (TRUE != MVD_PatternLenIsValid(u32PatternByteCnt)))
    {
        if (MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_PAUSE))
        {
            MVD_DEBUGVERBAL(printf("\nPause finished!\n"));
            break;
        }

        if (_drvSlqTbl.u32Empty < (SLQ_TBL_SAFERANGE/2))
        {
            MDrv_MVD_GetQueueVacancy(FALSE); //update _drvSlqTbl.u32Empty
        }
        //insert dummy pattern
        if (TRUE == MVD_SLQTblInsertPattern(E_MVD_PATTERN_FLUSH))
        {
            u32PatternByteCnt += DUMMY_SIZE*2; //2 dummy were inserted
        }
    }
    MVD_DEBUGVERBAL(printf("====> %s (t1=%lu t2=%lu diff=%lu)\n", __FUNCTION__,
            u32TimeCnt, HAL_MVD_GetTime(), (HAL_MVD_GetTime() - u32TimeCnt)));
    //if ((HAL_MVD_GetTime() - u32TimeCnt) >= CMD_TIMEOUT_MS)
    if (TRUE != MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_PAUSE))
    {
        MVD_DEBUGERROR(printf("\n***** MDrv_MVD_FlushQueue PAUSE TIMEOUT!!! *****\n\n"));
        MVD_DEBUGERROR(printf("ValidLen=0x%lx CurPatternLen=0x%lx\n",
                      HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+92), u32PatternByteCnt));
        
        return E_MVD_RET_FAIL;
    }

    //flush ES buffer & reset SLQ tbl
    if (HAL_MVD_SlqTblRst() == TRUE)
    {
        //return E_MVD_RET_OK;
    }

    u32TimeOut = 10;
    while (u32TimeOut>0)
    {
        if (MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_SLQ_RST))
        {
            //printf("\nSlqRst finished! x=0x%lx\n", x);
            break;
        }
        u32TimeOut--;
        HAL_MVD_Delayms(10);
    }
    //printf("\n\n=====> SlqRst u32TimeOut =%lx\n", u32TimeOut);
    if (u32TimeOut==0)
    {
        MVD_DEBUGERROR(printf("\n***** MDrv_MVD_FlushQueue SlqRst TIMEOUT!!! *****\n\n"));
        return E_MVD_RET_FAIL;
    }

    MVD_SLQTblInit(); //reset related buffers

    return E_MVD_RET_OK;
}

//------------------------------------------------------------------------------
/// Flush queue: flush ES buffer, reset SLQ TABLE, flush decoded frame, and
///              keep the display frame.
/// @return -return E_MVD_Result success/fail to flush queue
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_FlushQueue(void)
{
    E_MVD_Result eRet = E_MVD_RET_OK;

    MVD_FUNC_ENTRY();

    if (MDrv_MVD_GetDecodeStatus() == E_MVD_STAT_IDLE)
    {
        return eRet;
    }

    //flush ES buffer (and cmd queue if used)
    if (E_MVD_SLQ_TBL_MODE == curSrcMode)
    {
        eRet = MVD_FlushSlqTblQueue();
    }
    else if (E_MVD_TS_FILE_MODE == curSrcMode)
    {
        eRet = MVD_FlushTSQueue();
    }
    if (E_MVD_RET_OK != eRet)
    {
        return eRet;
    }

    //flush display buffer
    if (MDrv_MVD_FlushDisplayBuf() != TRUE)
    {
        return E_MVD_RET_FAIL;
    }

    if (TRUE == MDrv_MVD_SkipToIFrame())
    {
        return E_MVD_RET_OK;
    }
    else
    {
        return E_MVD_RET_FAIL;
    }
}

static MS_U32 MVD_Map2DrvSlqTbl(MS_U32 u32HWPtr)
{
    MS_U32 u32HWSt = MVD_GetMemOffset(stMemCfg.u32BSAddr);
    MS_U32 u32DrvPtr;

    //if (stMemCfg.bFWMiuSel != stMemCfg.bHWMiuSel)
    if (bSlqTblSync)
    {
        u32DrvPtr = _drvSlqTbl.u32StAdd + (u32HWPtr - u32HWSt);
        return u32DrvPtr;
    }
    return u32HWPtr;
}


//------------------------------------------------------------------------------
/// Get the queue vacancy.
/// @param -pQueueVacancy \b IN : pointer to the queue vacancy
/// @return -return E_MVD_Result success/fail to get the queue vacancy
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetQueueVacancy(MS_BOOL bCached)
{
    MS_U32 u32Empty;

#if 0
    if (MDrv_MVD_GetVldErrCount()!=0)
    {
        printf("QQQ wPtr= 0x%lx(0x%lx) rPtr=0x%lx(0x%lx) vldErr=0x%lx\n", writePtrLast, _drvSlqTbl.u32WrPtr,
        MDrv_MVD_GetSLQReadPtr(), _drvSlqTbl.u32RdPtr, MDrv_MVD_GetVldErrCount());
        printf("Previous EsRead=0x%lx EsWrite=0x%lx\n", u32PreEsRd, u32PreEsWr);
        _SLQTbl_DumpPtsTbl(0x0, 0x620);
        while(1);
    }
#endif

    u32Empty = _drvSlqTbl.u32Empty;
    if ((TRUE == bCached) && (u32Empty > SLQTBL_CHECKVACANCY_WATERLEVEL))
    {
        //To have better performance, we only query F/W read pointer
        //when queue_vacancy is lower than water_level
        return u32Empty;
    }

    _drvSlqTbl.u32RdPtr = MVD_Map2DrvSlqTbl(MDrv_MVD_GetSLQReadPtr());
    //printf("QV=0x%lx rd=0x%lx==>", u32Empty, _drvSlqTbl.u32RdPtr);
    if (_drvSlqTbl.u32RdPtr > _drvSlqTbl.u32StAdd)
    {
        if (_drvSlqTbl.u32RdPtr >= _drvSlqTbl.u32EndAdd)
        {
            printf("%s: readPtr out of range: too large!\n", __FUNCTION__);
        }
        _drvSlqTbl.u32RdPtr -= SLQ_ENTRY_LEN;
    }
    else if (_drvSlqTbl.u32RdPtr == _drvSlqTbl.u32StAdd)
    {
        _drvSlqTbl.u32RdPtr = _drvSlqTbl.u32EndAdd - SLQ_ENTRY_LEN;
    }
    else
    {
        printf("%s: readPtr out of range: too small!\n", __FUNCTION__);
        _drvSlqTbl.u32RdPtr = _drvSlqTbl.u32StAdd;
    }
    //printf("0x%lx\n", _drvSlqTbl.u32RdPtr);

    if (_drvSlqTbl.u32WrPtr > _drvSlqTbl.u32RdPtr)
    {
        u32Empty = SLQ_TBL_SIZE - (_drvSlqTbl.u32WrPtr - _drvSlqTbl.u32RdPtr);
    }
    else
    {
        u32Empty = _drvSlqTbl.u32RdPtr - _drvSlqTbl.u32WrPtr;
    }

    if (u32Empty == 0)// && (_drvSlqTbl.u32WrPtr == _drvSlqTbl.u32StAdd))
    {
        u32Empty = SLQ_TBL_SIZE;
    }

    _drvSlqTbl.u32Empty = u32Empty;

    if (u32Empty < SLQ_TBL_SAFERANGE)
    {//to avoid write_pointer catch up to read_pointer
        u32Empty= 0;
    }

    //printf("%s r=0x%lx w=0x%lx u32Empty=0x%lx idx=0x%lx\n", __FUNCTION__,
    //        _drvSlqTbl.u32RdPtr, _drvSlqTbl.u32WrPtr, u32Empty, MDrv_MVD_GetSWIdx());
    return u32Empty;
}


#define _IsNotInStreamBuff(x)                                       \
        (((x) < stMemCfg.u32DrvBufSize) ||   \
         ((x) > stMemCfg.u32BSSize) )

//------------------------------------------------------------------------------
/// Get read pointer in ElementaryStream buffer for SLQ table mode
/// @return -the read pointer
//------------------------------------------------------------------------------
static MS_U32 MVD_GetSlqTblESReadPtr(void)
{
    MS_U32 u32Idx;
    MS_U32 u32SlqRp = MVD_Map2DrvSlqTbl(MDrv_MVD_GetSLQReadPtr());//_drvSlqTbl.u32RdPtr;
    MS_U32 u32EsRp;
    MS_U32 u32EsEnd;
    MS_U32 u32EsStart;

    //report (readPtr-1) for HW may still use (readPtr)
    if (u32SlqRp > (_drvSlqTbl.u32StAdd))
    {
        u32Idx = ((u32SlqRp - _drvSlqTbl.u32StAdd)/SLQ_ENTRY_LEN) - 1;
    }
    else
    {
        u32Idx = SLQ_ENTRY_MAX - 1;
    }
    u32EsRp = _drvEsTbl.u32StAdd + u32Idx*8;

    u32EsEnd = HAL_MVD_MemRead4Byte(u32EsRp+4);
    u32EsStart = HAL_MVD_MemRead4Byte(u32EsRp); //report StartAdd as read_pointer
    //printf("GetESReadPtr ES[%lx] Start=0x%lx End=0x%lx u32EsRp=%lx u32SlqRp=%lx\n",
    //    u32Idx, HAL_MVD_MemRead4Byte(u32EsRp), u32EsEnd, u32EsRp, u32SlqRp);
#if 0
    printf("GetESReadPtr[%lx] SlqRp=%lx EsStart=0x%lx EsEnd=0x%lx u32EsRp=%lx\n",
        u32Idx, u32SlqRp, u32EsStart, u32EsEnd, u32EsRp);
#endif

    if (u32EsStart == stMemCfg.u32DrvBufSize)
    {
        //Got init value by MVD_SLQTblInit(), i.e. decoder hasn't started decoding.
        //Therefore, report the 1st entry's ES_START.
        u32EsStart = HAL_MVD_MemRead4Byte(_drvEsTbl.u32StAdd);
        MVD_DEBUGVERBAL(printf("Not start decoding: EsRd=0x%lx\n", u32EsStart));
    }

    if ((_IsNotInStreamBuff(u32EsStart)) && (u32EsStart != 0))
    {   //ESRead is not in BS buffer, so this entry is a divx pattern.
        //Report the last ESRead, instead of this one.
        MVD_DEBUGINFO(printf("0x%lx Not in BS, report u32PreEsRd=0x%lx\n", u32EsStart, u32PreEsRd));
        return u32PreEsRd;
    }
    u32PreEsRd = u32EsStart;

    return u32EsStart;
}

//------------------------------------------------------------------------------
/// Get write pointer in ElementaryStream buffer for SLQ table mode
/// @return -the read pointer
//------------------------------------------------------------------------------
static MS_U32 MVD_GetSlqTblESWritePtr(void)
{
    MS_U32 u32EsWp;
    MS_U32 u32EsEnd;
    MS_U32 u32Idx;

    if (_drvSlqTbl.u32WrPtr > (_drvSlqTbl.u32StAdd))
    {
        u32Idx = ((_drvSlqTbl.u32WrPtr - _drvSlqTbl.u32StAdd)/SLQ_ENTRY_LEN) - 1;
    }
    else
    {
        u32Idx = SLQ_ENTRY_MAX - 1;
    }
    u32EsWp = _drvEsTbl.u32StAdd + u32Idx*8;

    u32EsEnd = HAL_MVD_MemRead4Byte(u32EsWp+4);
#if 0
    printf("GetESWritePtr[%lx] ES Start=0x%lx End=0x%lx u32EsWp=%lx\n",
        (u32EsWp - _drvEsTbl.u32StAdd)/8,
        HAL_MVD_MemRead4Byte(u32EsWp), u32EsEnd, u32EsWp);
#endif

    if ((_IsNotInStreamBuff(u32EsEnd)) && (u32EsEnd != 0))
    {   //ESRead is not in BS buffer, so this entry is a divx pattern.
        //Report the last ESRead, instead of this one.
        MVD_DEBUGINFO(printf("0x%lx Not in BS, report u32PreEsWr=0x%lx\n", u32EsEnd, u32PreEsWr));
        return u32PreEsWr;
    }
    u32PreEsWr = u32EsEnd;

    return u32EsEnd;
}

//------------------------------------------------------------------------------
/// Get ES read address for TS file mode.
/// @return ES read address
//------------------------------------------------------------------------------
static MS_U32 MVD_GetTsFileESReadPtr(void)
{
#if defined(_MVD4)
    MS_U32 u32Add = 0;
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = 1;
    if (HAL_MVD_MVDCommand( CMD_PARSER_READ_POSITION, &mvdcmd ) == TRUE)
    {
        u32Add = (((MS_U32)mvdcmd.Arg3) <<24) |
                 (((MS_U32)mvdcmd.Arg2) <<16) |
                 (((MS_U32)mvdcmd.Arg1) << 8) |
                 (((MS_U32)mvdcmd.Arg0));
    }
    else
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\n", CMD_PARSER_READ_POSITION) );
    }

    return (u32Add*8);
#else
    return 0;
#endif
}

//------------------------------------------------------------------------------
/// Get ES write address for TS file mode.
/// @return ES write address
//------------------------------------------------------------------------------
static MS_U32 MVD_GetTsFileESWritePtr(void)
{
#if defined(_MVD4)
    MS_U32 u32Diff = 0;
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = 2;    //ES diff
    if (HAL_MVD_MVDCommand( CMD_PARSER_READ_POSITION, &mvdcmd ) == TRUE)
    {
        u32Diff = (((MS_U32)mvdcmd.Arg3) <<24) | (((MS_U32)mvdcmd.Arg2) <<16) |
                  (((MS_U32)mvdcmd.Arg1) << 8) | (((MS_U32)mvdcmd.Arg0));
    }
    else
    {
        MVD_DEBUGERROR( printf( "Ctrl: 0x%x fail!!\n", CMD_PARSER_READ_POSITION) );
    }

    return (u32Diff*8 + MVD_GetTsFileESReadPtr());
#else
    return 0;
#endif
}

//------------------------------------------------------------------------------
/// Get read pointer in ElementaryStream buffer
/// @return -the read pointer
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetESReadPtr(void)
{
    MS_U32 u32ESR = 0;
    if (E_MVD_SLQ_TBL_MODE == curSrcMode)
    {
        u32ESR = MVD_GetSlqTblESReadPtr();
    }
    else if (E_MVD_TS_FILE_MODE == curSrcMode || E_MVD_TS_MODE == curSrcMode)
    {
        u32ESR = MVD_GetTsFileESReadPtr();
    }
    return u32ESR;
}


//------------------------------------------------------------------------------
/// Get write pointer in ElementaryStream buffer
/// @return -the read pointer
//------------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetESWritePtr(void)
{
    MS_U32 u32ESW = 0;
    if (E_MVD_SLQ_TBL_MODE == curSrcMode)
    {
        u32ESW = MVD_GetSlqTblESWritePtr();
    }
    else if (E_MVD_TS_FILE_MODE == curSrcMode || E_MVD_TS_MODE == curSrcMode)
    {
        u32ESW = MVD_GetTsFileESWritePtr();
    }
    return u32ESW;
}


//------------------------------------------------------------------------------
/// Enable/Disable driver to show the last frame
/// @param -bEnable \b IN : enable/disable this function
/// @return -return E_MVD_Result success/fail to enable/disable
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_EnableLastFrameShow(MS_BOOL bEnable)
{
    MVD_FUNC_ENTRY();

    if (E_MVD_SLQ_TBL_MODE == curSrcMode)
    {
        //save current writePtr
        if (_drvSlqTbl.u32WrPtr != _drvSlqTbl.u32StAdd)
        {
            u32FileEndPtr = _drvSlqTbl.u32WrPtr - SLQ_ENTRY_LEN;
        }
        else
        {
            if (_drvSlqTbl.u32WrPtr != _drvSlqTbl.u32RdPtr)
            {
                u32FileEndPtr = _drvSlqTbl.u32EndAdd - SLQ_ENTRY_LEN;
            }
            else
            {
                //R==W==ST indicates SlqTbl may just be reset
                u32FileEndPtr = _drvSlqTbl.u32StAdd;
            }
        }
        MVD_DEBUGVERBAL(printf("fe=%lx, rd=%lx, wr=%lx\n", u32FileEndPtr,
                        _drvSlqTbl.u32RdPtr, _drvSlqTbl.u32WrPtr));
    }

    if (HAL_MVD_EnableLastFrameShow(bEnable) == FALSE)
    {
        MVD_DEBUGERROR(printf("%s fail!!\n", __FUNCTION__));
        return E_MVD_RET_FAIL;
    }
    bEnableLastFrmShow = bEnable;
    return E_MVD_RET_OK;
}

#define FLAG_LAST_FRM_SHOW (HAL_MVD_MemReadByte(pu8MVDGetFrameInfoBufStart+OFFSET_CMD_LAST_FRAME_SHOW))
//------------------------------------------------------------------------------
/// Get if MVD decoder finish display.
/// @return TRUE or FALSE
///     - TRUE, Yes
///     - FALSE, No
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_IsDispFinish(void)
{
#if defined(_MVD4)
    MS_U32 u32TimeCnt;

    MVD_FUNC_ENTRY();

    //printf("MDrv_MVD_IsDispFinish::");
    if (bEnableLastFrmShow != TRUE)
    {
        MVD_DEBUGINFO(printf("%s: bEnableLastFrmShow!=TRUE\n", __FUNCTION__));
        return E_MVD_RET_FAIL;
    }

    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("%s err: pu8MVDGetFrameInfoBufStart=NULL\n", __FUNCTION__));
        return E_MVD_RET_FAIL;
    }

    //printf("0x%x\n", FLAG_LAST_FRM_SHOW);
    if ((E_MVD_SLQ_TBL_MODE == curSrcMode) && (TRUE != FLAG_LAST_FRM_SHOW))
    {
        if (u32FileEndPtr == MVD_Map2DrvSlqTbl(MDrv_MVD_GetSLQReadPtr()))
        {
            //insert padding pattern until timeout
            u32TimeCnt= HAL_MVD_GetTime();
            while ((HAL_MVD_GetTime() - u32TimeCnt) < CMD_TIMEOUT_MS)
            {
                if (TRUE == FLAG_LAST_FRM_SHOW)
                {
                    //printf("\nDisp finished!\n");
                    break;
                }
                //insert file-end pattern again
                MVD_SLQTblInsertPattern(E_MVD_PATTERN_FILEEND);
            }
            if ((HAL_MVD_GetTime() - u32TimeCnt) >= CMD_TIMEOUT_MS)
            {
                MVD_DEBUGERROR(printf("\n***** MDrv_MVD_IsDispFinish TIMEOUT!!! *****\n\n"));
                return E_MVD_RET_TIME_OUT;
            }
            else
            {
                return E_MVD_RET_OK;
            }
        }
        else
        {
            //just return fail if readPtr is not closed to file-end ptr
            MVD_DEBUGVERBAL(printf("fe=%lx, rd=%lx(%lx), wr=%lx, empty=%lx\n", u32FileEndPtr, _drvSlqTbl.u32RdPtr,
                            MVD_Map2DrvSlqTbl(MDrv_MVD_GetSLQReadPtr()), _drvSlqTbl.u32WrPtr, _drvSlqTbl.u32Empty));
            return E_MVD_RET_FAIL;
        }
    }

    if (FLAG_LAST_FRM_SHOW)
    {
        return E_MVD_RET_OK;
    }
    else
    {
        return E_MVD_RET_FAIL;
    }
#else
    MVD_FUNC_ENTRY();
    return E_MVD_RET_FAIL;
#endif
}

//------------------------------------------------------------------------------
/// Set speed.
/// @param -eSpeedType \b IN : specify the speed
/// @param -u8Multiple \b IN :
/// @return -return E_MVD_Result success/fail to set the speed
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_SetSpeed(MVD_SpeedType eSpeedType, MS_U8 u8Multiple)
{
    static MVD_AVSyncCfg stNFPSyncCfg;

    if ((u8Multiple > 32) || (eSpeedType==E_MVD_SPEED_TYPE_UNKNOWN))
    {
        MVD_DEBUGINFO(printf("%s: invalid para!\n", __FUNCTION__));
        return E_MVD_RET_INVALID_PARAM;
    }

    if (ePreSpeedType == E_MVD_SPEED_DEFAULT)
    {   //save avsync config when normal play for restoring it later
        stNFPSyncCfg.bEnable = stSyncCfg.bEnable;
        stNFPSyncCfg.u32Delay = stSyncCfg.u32Delay;
        stNFPSyncCfg.u16Tolerance = stSyncCfg.u16Tolerance;
        MVD_DEBUGINFO(printf("MDrv_MVD_SetSpeed save avsync: bEnable=0x%x u32Delay=0x%lx u16Tolerance=0x%x\n",
                  stNFPSyncCfg.bEnable, stNFPSyncCfg.u32Delay, stNFPSyncCfg.u16Tolerance));
    }

    if (E_MVD_SPEED_DEFAULT != eSpeedType) //fast or slow forward
    {
        //disable avsyn when fast/slow forward
        MDrv_MVD_SetAVSync(FALSE, 0);
    }
    #if 0
    else
    {
        if (ePreSpeedType != E_MVD_SPEED_DEFAULT)
        {
            //set AVSync again for firmware doesn't remember AVSync settings after FF
            MVD_DEBUGINFO(printf("MDrv_MVD_SetSpeed reset avsync: bEnable=0x%x u32Delay=0x%lx u16Tolerance=0x%x\n",
                          stNFPSyncCfg.bEnable, stNFPSyncCfg.u32Delay, stNFPSyncCfg.u16Tolerance));
            (printf("MDrv_MVD_SetSpeed reset avsync: bEnable=0x%x u32Delay=0x%lx u16Tolerance=0x%x\n",
                          stNFPSyncCfg.bEnable, stNFPSyncCfg.u32Delay, stNFPSyncCfg.u16Tolerance));

            MDrv_MVD_SetAVSync(stNFPSyncCfg.bEnable, stNFPSyncCfg.u32Delay);
            if ((stNFPSyncCfg.u16Tolerance!=0) && (FALSE == MDrv_MVD_ChangeAVsync(stNFPSyncCfg.bEnable, stNFPSyncCfg.u16Tolerance)))
            {
                return E_MVD_RET_FAIL;
            }
        }
    }
    #endif

    if (HAL_MVD_SetSpeed(eSpeedType, u8Multiple) == TRUE)
    {
        if (E_MVD_SPEED_DEFAULT == eSpeedType) //Normal Play
        {
            if (ePreSpeedType != E_MVD_SPEED_DEFAULT)
            {
                //set AVSync again for firmware doesn't remember AVSync settings after FF
                MVD_DEBUGINFO(printf("MDrv_MVD_SetSpeed reset avsync: bEnable=0x%x u32Delay=0x%lx u16Tolerance=0x%x\n",
                              stNFPSyncCfg.bEnable, stNFPSyncCfg.u32Delay, stNFPSyncCfg.u16Tolerance));
                (printf("MDrv_MVD_SetSpeed reset avsync: bEnable=0x%x u32Delay=0x%lx u16Tolerance=0x%x\n",
                              stNFPSyncCfg.bEnable, stNFPSyncCfg.u32Delay, stNFPSyncCfg.u16Tolerance));

                MDrv_MVD_SetAVSync(stNFPSyncCfg.bEnable, stNFPSyncCfg.u32Delay);
                if ((stNFPSyncCfg.u16Tolerance!=0) && (FALSE == MDrv_MVD_ChangeAVsync(stNFPSyncCfg.bEnable, stNFPSyncCfg.u16Tolerance)))
                {
                    return E_MVD_RET_FAIL;
                }
            }
        }

        ePreSpeedType = eSpeedType;
        return E_MVD_RET_OK;
    }
    else
    {
        return E_MVD_RET_FAIL;
   }
}

//------------------------------------------------------------------------------
/// Set PTS base to MVD F/W
/// @param -u32pts \b IN : pts unit in 90k counter
//------------------------------------------------------------------------------
static void MVD_SetPTSBase(MS_U32 u32pts)
{
#if defined(_MVD4)
    MVD_CmdArg mvdcmd;

    SET_CMDARG(mvdcmd, u32pts);
    if (HAL_MVD_MVDCommand( CMD_PTS_BASE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_PTS_BASE ) );
    }
#endif
    return;
}

//------------------------------------------------------------------------------
/// Reset Presentation Time Stamp according to u32PtsBase
/// @param -u32PtsBase \b IN : the PTS base specified by user
/// @return -return E_MVD_Result success/fail to reset PTS base
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_ResetPTS(MS_U32 u32PtsBase)
{
    _mvdCmdQ.u32PtsBase = _MS_TO_90K(u32PtsBase);
    MVD_SetPTSBase(_MS_TO_90K(u32PtsBase));
    return E_MVD_RET_OK;
}


//------------------------------------------------------------------------------
/// Get if sequence change (width/height/framerate/interlace) occurs.
/// @return TRUE or FALSE
///     - TRUE, Yes
///     - FALSE, No
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_IsSeqChg(void)
{
    static MVD_FrameInfo stFrmInfo;

    if (MDrv_MVD_GetDispRdy() && TRUE == MDrv_MVD_GetValidStreamFlag())
    {
        MDrv_MVD_GetFrameInfo(&stFrmInfo);
        stFrmInfo.u8AFD = MDrv_MVD_GetActiveFormat();

        if (FALSE == stMemCfg.bEnableDynScale)
        {
            if((stPreFrmInfo.u16HorSize   != stFrmInfo.u16HorSize) ||
               (stPreFrmInfo.u16VerSize   != stFrmInfo.u16VerSize) ||
               (stPreFrmInfo.u32FrameRate != stFrmInfo.u32FrameRate) ||
               (stPreFrmInfo.u8Interlace  != stFrmInfo.u8Interlace))
            {
#if 0
                (printf( "MDrv_MVD_IsSeqChg::Previous\n" ));
                (printf( "H=%u\n", stPreFrmInfo.u16HorSize ));
                (printf( "V=%u\n", stPreFrmInfo.u16VerSize ));
                (printf( "F=%lu\n", stPreFrmInfo.u32FrameRate ));
                (printf( "A=%u\n", stPreFrmInfo.u8AspectRate ));
                (printf( "I=%u\n", stPreFrmInfo.u8Interlace ));
#endif
#if 0
                (printf( "MDrv_MVD_IsSeqChg\n" ));
                (printf( "H=%u\n", stFrmInfo.u16HorSize ));
                (printf( "V=%u\n", stFrmInfo.u16VerSize ));
                (printf( "F=%lu\n", stFrmInfo.u32FrameRate ));
                (printf( "A=%u\n", stFrmInfo.u8AspectRate ));
                (printf( "I=%u\n", stFrmInfo.u8Interlace ));
#endif

                //memcpy(&stPreFrmInfo, &stFrmInfo, sizeof(MVD_FrameInfo));
                stPreFrmInfo.u16HorSize   = stFrmInfo.u16HorSize;
                stPreFrmInfo.u16VerSize   = stFrmInfo.u16VerSize;
                stPreFrmInfo.u8AspectRate = stFrmInfo.u8AspectRate;
                stPreFrmInfo.u32FrameRate = stFrmInfo.u32FrameRate;
                stPreFrmInfo.u8Interlace  = stFrmInfo.u8Interlace;
#if 0
                (printf( "MDrv_MVD_IsSeqChg===>\n" ));
                (printf( "H=%u\n", stFrmInfo.u16HorSize ));
                (printf( "V=%u\n", stFrmInfo.u16VerSize ));
                (printf( "F=%u\n", stFrmInfo.u32FrameRate ));
                (printf( "A=%u\n", stFrmInfo.u8AspectRate ));
                (printf( "I=%u\n", stFrmInfo.u8Interlace ));
#endif
                return TRUE;
            }
        }
        else
        {
            //When dynamic scaling is enabled, f/w will handle the width/height change.
            //Therefore, only report SeqChg if framerate or interlace flag changes.
            if((stPreFrmInfo.u32FrameRate != stFrmInfo.u32FrameRate) ||
               (stPreFrmInfo.u8Interlace  != stFrmInfo.u8Interlace))
            {
                stPreFrmInfo.u8AspectRate = stFrmInfo.u8AspectRate;
                stPreFrmInfo.u32FrameRate = stFrmInfo.u32FrameRate;
                stPreFrmInfo.u8Interlace  = stFrmInfo.u8Interlace;
                return TRUE;
            }
        }
    }

    return FALSE;
}

//------------------------------------------------------------------------------
/// Set Debug Data which will be queried by MDrv_MVD_DbgGetData()
/// @param -u32Addr \b IN : address of debug data
/// @param -u32Data \b IN : the debug data
/// @return -return E_MVD_Result success/fail to set debug data
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_DbgSetData(MS_U32 u32Addr, MS_U32 u32Data)
{
    return E_MVD_RET_FAIL;
}

//------------------------------------------------------------------------------
/// Get Debug Data
/// @param -u32Addr \b IN : address of debug data
/// @param -u32Data \b IN : pointer to the debug data
/// @return -return E_MVD_Result success/fail to get debug data
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_DbgGetData(MS_U32 u32Addr, MS_U32* u32Data)
{
    MVD_CmdArg mvdcmd;
    MS_U32 u32Val;

    if (!u32Data)
    {
        return E_MVD_RET_INVALID_PARAM;
    }

    SET_CMDARG(mvdcmd, u32Addr);
    if (HAL_MVD_MVDCommand( CMD_RD_IO, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_RD_IO ) );
        return E_MVD_RET_FAIL;
    }

#if (defined(CHIP_T2))
    u32Val = (((MS_U32)mvdcmd.Arg0)) | ((MS_U32)mvdcmd.Arg1 << 8) |
             (((MS_U32)mvdcmd.Arg2) << 16) | (((MS_U32)mvdcmd.Arg3) << 24);
#else
    u32Val = (((MS_U32)mvdcmd.Arg2)) | ((MS_U32)mvdcmd.Arg3 << 8) |
             (((MS_U32)mvdcmd.Arg4) << 16) | (((MS_U32)mvdcmd.Arg5) << 24);
#endif
    *u32Data = u32Val;

    return E_MVD_RET_OK;
}


//------------------------------------------------------------------------------
/// Set MVD firmware command
/// @param -u8cmd \b IN : MVD command
/// @param -pstCmdArg \b IN : pointer to command argument
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_MVDCommand ( MS_U8 u8cmd, MVD_CmdArg *pstCmdArg )
{
    return HAL_MVD_MVDCommand(u8cmd, pstCmdArg);
}



#ifndef REDLION_LINUX_KERNEL_ENVI
//------------- Below functions are for ATSC Closed Caption --------------------
//static MVD_CCCfg stCCParam = {E_MVD_CC_NONE, E_MVD_CC_TYPE_NONE, 0, 0};

///Only support parsing one format CC at a time
static MS_BOOL MVD_CCGetIsActiveFormat(MVD_CCFormat eCCFormat)
{
#if 1
    return TRUE;    //fixme
#else
    if (stCCParam.eFormat == eCCFormat)
    {
        return TRUE;
    }
    else
    {
        printf("CC format %d is not activated!\n", eCCFormat);
        return FALSE;
    }
#endif
}

//------------------------------------------------------------------------------
/// Reset MVD CC.  It can be called when overflow occurrs.
/// @return -Result of Reset CC.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_CCRst(MVD_CCCfg* pCCParam)
{
    return MDrv_MVD_CCStartParsing(pCCParam);
}


//------------------------------------------------------------------------------
/// Start CC data parsing.
/// @return -Result of issuing command to firmware.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_CCStartParsing(MVD_CCCfg* pCCParam)
{
    MS_U8 u8CC608 = (MS_U8)TRUE;
    MS_U8 u8Operation = 0;

    if (!pCCParam || (pCCParam->eFormat==E_MVD_CC_NONE) || (pCCParam->eType==E_MVD_CC_TYPE_NONE))
    {
        return E_MVD_RET_INVALID_PARAM;
    }

    switch (pCCParam->eType)
    {
        case E_MVD_CC_TYPE_NTSC_FIELD1:
            u8Operation = 0x01;
            break;
        case E_MVD_CC_TYPE_NTSC_FIELD2:
            u8Operation = 0x02;
            break;
        case E_MVD_CC_TYPE_NTSC_TWOFIELD:
            u8Operation = 0x03;
            break;
        case E_MVD_CC_TYPE_DTVCC:
            u8Operation = 0x04;
            u8CC608 = FALSE;
            break;
        default:
            break;
    }

    MDrv_CC_Init();
    MDrv_CC_CM_SetMVDRB_HWAddr(pCCParam->u32BufStAdd, u8CC608);//fixme: VA2PA?
    MDrv_CC_CM_SetParsingType(u8Operation, (MS_U16)pCCParam->u32BufSize, u8CC608);

    return E_MVD_RET_OK;
}


//------------------------------------------------------------------------------
/// Stop CC data parsing.
/// @return -Result of issuing command to firmware.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_CCStopParsing(MVD_CCFormat eCCFormat)
{
    MS_U8 u8CC608 = 0xff;

    if (!MVD_CCGetIsActiveFormat(eCCFormat))
        return E_MVD_RET_INVALID_PARAM;

    switch (eCCFormat)
    {
        case E_MVD_CC_608:
            u8CC608 = 1;
            break;
        case E_MVD_CC_708:
            u8CC608 = 0;
            break;
        default:
            return E_MVD_RET_INVALID_PARAM;
    }
    MDrv_CC_CM_DisableParsing(u8CC608);
    return E_MVD_RET_OK;
}


//------------------------------------------------------------------------------
/// Get write pointer of CC data buffer.
/// @return -Result of the query.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_CCGetWritePtr(MVD_CCFormat eCCFormat, MS_U32* pWrite)
{
    MS_U8 u8CC608;
    if (!MVD_CCGetIsActiveFormat(eCCFormat) || !pWrite)
        return E_MVD_RET_INVALID_PARAM;

    u8CC608 = ((eCCFormat==E_MVD_CC_608)?1:0);
    *pWrite = MDrv_CC_PM_GetMVDRB_WriteAddr(u8CC608);
    return E_MVD_RET_OK;
}


//------------------------------------------------------------------------------
/// Get the read pointer of CC data buffer.
/// @return -Result of the query.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_CCGetReadPtr(MVD_CCFormat eCCFormat, MS_U32* pRead)
{
    MS_U8 u8CC608;
    if (!MVD_CCGetIsActiveFormat(eCCFormat) || !pRead)
        return E_MVD_RET_INVALID_PARAM;

    u8CC608 = ((eCCFormat==E_MVD_CC_608)?1:0);
    *pRead = MDrv_CC_PM_GetMVDRB_ReadAddr(u8CC608);
    return E_MVD_RET_OK;
}


//------------------------------------------------------------------------------
/// Update the read pointer of CC data buffer.
/// @return -Result of the update.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_CCUpdateReadPtr(MVD_CCFormat eCCFormat, MS_U32 u32EachPacketSize)
{
    MS_U8 u8CC608;

    if (!MVD_CCGetIsActiveFormat(eCCFormat))
        return E_MVD_RET_INVALID_PARAM;

    u8CC608 = ((eCCFormat==E_MVD_CC_608)?1:0);
    MDrv_CC_PM_SetMVDRB_ReadAddr(u32EachPacketSize, u8CC608);

    return E_MVD_RET_OK;
}


//------------------------------------------------------------------------------
/// Get if CC data buffer is overflow.
/// @return -Result of the query.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_CCGetIsOverflow(MVD_CCFormat eCCFormat, MS_BOOL* pbOverflow)
{
    MS_U8 u8CC608;
    if (!pbOverflow || !MVD_CCGetIsActiveFormat(eCCFormat))
        return E_MVD_RET_INVALID_PARAM;

    u8CC608 = ((eCCFormat==E_MVD_CC_608)?1:0);
    *pbOverflow = MDrv_CC_CM_GetOverflowStatus(u8CC608);

    return E_MVD_RET_OK;
}
#endif



//------------- Below functions are for MediaCodec SLQ Table --------------------
#define SLQTABLE_V_QUEUE_COUNT      (2048UL)                            // may support up to 4M buffer
#define SLQTABLE_V_QUEUE_SIZE       (SLQTABLE_V_QUEUE_COUNT * 8)        // (0x4000) ,16k
#define SLQTABLE_V_PTS_QUEUE_COUNT  (1024UL)                            // don't change this value, we only have 10 bits for stuffing SLQ idx. 2^10=1024
#define SLQTABLE_V_PTS_QUEUE_SIZE   (SLQTABLE_V_PTS_QUEUE_COUNT * 8)    // (0x2000) ,8k
#define SLQTABLE_FIRE_THD           (0x100)     // 32queue*8bytes
#define SLQTABLE_AVAILABLELEVEL     (3UL * 8)
#define SLQTABLE_DIVX_PATTERN_ADDR  (SLQTABLE_V_QUEUE_SIZE + SLQTABLE_V_PTS_QUEUE_SIZE)
#define SLQTABLE_DIVX_PATTERN_SIZE  (SLQTABLE_V_QUEUE_COUNT * 8)
#define SLQTABLE_SKIPPATTERN_ADDR   (SLQTABLE_DIVX_PATTERN_ADDR + SLQTABLE_DIVX_PATTERN_SIZE)
#define SLQTABLE_SKIPPATTERN_SIZE   (8 * 16)
#define SLQTABLE_TOTAL_SIZE         (SLQTABLE_SKIPPATTERN_ADDR + SLQTABLE_SKIPPATTERN_SIZE)

#define HAL_MEMORY_BARRIER()
#define _MapVA2PA(x)    x
#define _MapPA2VA(x)    x
#define MVD_SLQTABLE_INFO(x)

//ST_SLQTABLE_INFO    _stSLQTable;

#if _SLQTBL_DUMP_PTS
static void _SLQTbl_DumpPtsTbl(MS_U32 u32EntryStart, MS_U32 u32EntryEnd)
{
    MS_U32 i;
    MS_U32 u32EsRp, u32EsStart, u32EsEnd;

    for (i=u32EntryStart; i<u32EntryEnd; i++)
    {
        u32EsRp = _drvEsTbl.u32StAdd + i*8;

        u32EsEnd = HAL_MVD_MemRead4Byte(u32EsRp+4);
        u32EsStart = HAL_MVD_MemRead4Byte(u32EsRp); //report StartAdd as read_pointer
        printf("ES[%lx] Start=0x%lx End=0x%lx u32EsRp=%lx\n",
            i, u32EsStart, u32EsEnd, u32EsRp);
    }

    printf("\n=======Dump PTS table========\n");
    printf("addr\t byte_cnt\t dummy_cnt\t id_low\t id_high\t time_stamp\n");
    for (i=u32EntryStart; i<u32EntryEnd; i++)
    {
        printf("0x%lx\t 0x%08lx\t 0x%08lx\t 0x%08lx\t 0x%08lx\t 0x%08lx\n", u32MVDFWPtsTblAddr+i*MVD_FW_SLQTBL_PTS_LEN,
               HAL_MVD_MemRead4Byte(u32MVDFWPtsTblAddr+i*MVD_FW_SLQTBL_PTS_LEN),   //byteCnt
               HAL_MVD_MemRead4Byte(u32MVDFWPtsTblAddr+i*MVD_FW_SLQTBL_PTS_LEN+4), //dummyPktCnt
               HAL_MVD_MemRead4Byte(u32MVDFWPtsTblAddr+i*MVD_FW_SLQTBL_PTS_LEN+8), //idLow
               HAL_MVD_MemRead4Byte(u32MVDFWPtsTblAddr+i*MVD_FW_SLQTBL_PTS_LEN+12),//idHigh
               HAL_MVD_MemRead4Byte(u32MVDFWPtsTblAddr+i*MVD_FW_SLQTBL_PTS_LEN+16) //pts
               );
    }
    printf("=====================================\n");
}
#endif


static MS_BOOL MVD_SLQTblInsertPattern(MVD_PatternType ePattern)
{
    MS_U32 i;

    if (_drvSlqTbl.u32Empty < (SLQ_TBL_SAFERANGE/2))
    {
        MVD_DEBUGINFO(printf("SLQTbl full!(0x%lx) Cannot insert pattern any more!\n", _drvSlqTbl.u32Empty));
        return FALSE;
    }

    for (i =0; i<2; i++)
    {   //insert dummy pattern
        MVD_PacketInfo stDummyPkt;

        if (E_MVD_PATTERN_FLUSH == ePattern)
        {
            MVD_SLQTblGetDummyPkt(curCodecType, &stDummyPkt);
        }
        else if (E_MVD_PATTERN_FILEEND == ePattern)
        {
            MVD_SLQTblGetFileEndPkt(curCodecType, &stDummyPkt);
        }
        else
        {
            MVD_DEBUGERROR(printf("Invalid MVD_PatternType! Won't insert pattern!\n"));
            return FALSE;
        }

        //printf("WrPtr 0x%lx ", _drvSlqTbl.u32WrPtr);
        MVD_SLQTblSendPacket(&stDummyPkt);
        //printf("==> 0x%lx\n", _drvSlqTbl.u32WrPtr);
        MDrv_MVD_SetSLQWritePtr(0);
    }
    return TRUE;
}


static MS_BOOL MVD_SLQTblSendPacket(MVD_PacketInfo *pstVideoPKT)
{
    static MS_U32 u32EsLast;
    static MS_U32* u32LastEntry = &_drvSlqTbl.u32WrPtr;
    MS_U32* pu32EsNew = &_drvEsTbl.u32WrPtr;
    MS_U32 u32EntryWord = 0;
    MS_U32 u32Index;
    MS_U32 u32Pts;
    MS_U32 u32ESStart=0;
    MS_U32 u32ESEnd=0;
#if _SLQTBL_DUMP_PKT
    static MS_U32 u32SendTimes = 0;

    printf("Pkt[%lx] st=0x%lx len=0x%lx pts=0x%lx id_l=0x%lx id_h=0x%lx\n", u32SendTimes++,
        pstVideoPKT->u32StAddr, pstVideoPKT->u32Length, pstVideoPKT->u32TimeStamp,
        pstVideoPKT->u32ID_L, pstVideoPKT->u32ID_H);
#endif

    MS_ASSERT(u32MVDFWPtsTblAddr != NULL);
    if (u32MVDFWPtsTblAddr)
    {
        u32Index = (_drvSlqTbl.u32WrPtr - _drvSlqTbl.u32StAdd)/8;
        if (pstVideoPKT->u32TimeStamp != MVD_NULLPKT_PTS)
        {
            u32Pts = _MS_TO_90K(pstVideoPKT->u32TimeStamp);
        }
        else
        {
            u32Pts = MVD_NULLPKT_PTS;
        }
        HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+u32Index*MVD_FW_SLQTBL_PTS_LEN, u32SlqByteCnt&0xffffff); //byteCnt
        HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+u32Index*MVD_FW_SLQTBL_PTS_LEN+4, u32DummyPktCnt);   //dummyPktCnt
        HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+u32Index*MVD_FW_SLQTBL_PTS_LEN+8, pstVideoPKT->u32ID_L);  //idLow
        HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+u32Index*MVD_FW_SLQTBL_PTS_LEN+12, pstVideoPKT->u32ID_H); //idHigh
        HAL_MVD_MemWrite4Byte(u32MVDFWPtsTblAddr+u32Index*MVD_FW_SLQTBL_PTS_LEN+16, u32Pts);  //PTS
        //printf("PTS=0x%lx(%lx), idx=0x%lx add=0x%lx\n", pstVideoPKT->u32TimeStamp,
        //        HAL_MVD_MemRead4Byte(u32MVDFWPtsTblAddr+u32Index*MVD_FW_SLQTBL_PTS_LEN+4),
        //        u32Index, u32MVDFWPtsTblAddr+u32Index*MVD_FW_SLQTBL_PTS_LEN+4);
    }
#if _SLQTBL_DUMP_PTS
    if (u32Index == 0x177)
    {
        _SLQTbl_DumpPtsTbl(0, 0x179);
    }
#endif

    u32SlqByteCnt += pstVideoPKT->u32Length;

    //update SLQ tbl entry
    //u32EsLast = (pstVideoPKT->u32StAddr)+_drvSlqTbl.u32StAdd;
    u32EsLast = (pstVideoPKT->u32StAddr)+MVD_GetMemOffset(stMemCfg.u32BSAddr);
    HAL_MVD_MemWrite4Byte(*pu32EsNew, u32EsLast-MVD_GetMemOffset(stMemCfg.u32BSAddr));
    u32ESStart = (u32EsLast) & 0x0FFFFFFF; //28-bit

    u32EsLast += pstVideoPKT->u32Length; //update ES write pointer
    //Notice: This is for MVD HW, so no need to minus one.
    HAL_MVD_MemWrite4Byte((*pu32EsNew)+4, u32EsLast-MVD_GetMemOffset(stMemCfg.u32BSAddr));
    u32ESEnd    = (u32EsLast) & 0x0FFFFFFF; //28-bit

    *pu32EsNew += 8;
    if (*pu32EsNew >= _drvEsTbl.u32EndAdd)
    {   //wrap to the beginning of the table
        MVD_DEBUGINFO(printf("...ES wrapping to the beginning!\n"));
        *pu32EsNew = _drvEsTbl.u32StAdd;
    }

    //printf("===>[%lx] u32ESStart=0x%lx u32ESEnd=0x%lx u32EsLast=0x%lx\n",
    //    u32SendTimes++, u32ESStart, u32ESEnd, u32EsLast);

    u32EntryWord = u32ESEnd | (u32ESStart << 28);
    HAL_MVD_MemWrite4Byte(*u32LastEntry, u32EntryWord);
    //printf("===> u32EntryWord1 addr=0x%lx\n", (*u32LastEntry));
    //printf("===> u32EntryWord0=0x%lx\n", u32EntryWord);

    u32EntryWord = ((pstVideoPKT->u32ID_L & 0xff)<<24) | (u32ESStart >> 4);

    HAL_MVD_MemWrite4Byte((*u32LastEntry)+4, u32EntryWord);
    //printf("===> u32EntryWord1 addr=0x%lx\n", (*u32LastEntry)+4);
    //printf("===> u32EntryWord1=0x%lx\n", u32EntryWord);

    *u32LastEntry += 8;
    if (*u32LastEntry >= _drvSlqTbl.u32EndAdd)
    {   //wrap to the beginning of the table
        MVD_DEBUGINFO(printf("...wrapping to the beginning!\n"));
        *u32LastEntry = _drvSlqTbl.u32StAdd;
        //also wrap DivX311 pattern table
        _drvDivxTbl.u32WrPtr = _drvDivxTbl.u32StAdd;
    }

    if (_drvSlqTbl.u32Empty)
    {
        _drvSlqTbl.u32Empty -= SLQ_ENTRY_LEN;
    }

    return TRUE;
}

static void MVD_SLQTblGetDivxHdrPkt(MVD_PacketInfo* pDivxHdr, MVD_PacketInfo* pDivxData)
{
    MS_U32 u32DivXPattern = _drvDivxTbl.u32WrPtr;
    MS_U32 u32FrmSize = pDivxData->u32Length;

    HAL_MVD_MemWrite4Byte(u32DivXPattern, DIVX_PATTERN);
    HAL_MVD_MemWrite4Byte(u32DivXPattern+4, u32FrmSize);

    pDivxHdr->u32StAddr = u32DivXPattern - MVD_GetMemOffset(stMemCfg.u32DrvBufAddr);
    pDivxHdr->u32Length = 8;
    pDivxHdr->u32TimeStamp = pDivxData->u32TimeStamp; //unit: ms
    pDivxHdr->u32ID_L = MVD_U32_MAX;
    pDivxHdr->u32ID_H = MVD_U32_MAX;
    //printf("u32DivXPattern(0x%lx==>0x%lx)=0x%lx 0x%lx\n", u32DivXPattern, pDivxHdr->u32StAddr,
    //        HAL_MVD_MemRead4Byte(u32DivXPattern), HAL_MVD_MemRead4Byte(u32DivXPattern+4));

    _drvDivxTbl.u32WrPtr += 8;
#if 0   //move to SendPacket
    if (_drvDivxTbl.u32WrPtr >= _drvDivxTbl.u32EndAdd)
    {   //wrap to the beginning of the table
        printf("...wrapping to the DivXTbl beginning!\n");
        _drvDivxTbl.u32WrPtr = _drvDivxTbl.u32StAdd;
    }
#endif
}


static void MVD_SLQTblGetFileEndPkt(MVD_CodecType eType, MVD_PacketInfo* pFileEnd)
{
    MS_U32 u32EndPattern = MVD_GetMemOffset(stMemCfg.u32DrvBufAddr+SLQ_TBL_SIZE*2+END_PATTERN_SIZE);
    MS_U32 i;

    for (i=0; i<END_PATTERN_SIZE; i+=4)
    {
        HAL_MVD_MemWrite4Byte(u32EndPattern+i, 0xffffffff);
    }
    if ((E_MVD_CODEC_FLV == eType)||(E_MVD_CODEC_MPEG4_SHORT_VIDEO_HEADER == eType))
    {
        HAL_MVD_MemWrite4Byte(u32EndPattern, FLV_PATTERN);
        HAL_MVD_MemWrite4Byte(u32EndPattern+4, 0xffffffff);
        HAL_MVD_MemWrite4Byte(u32EndPattern+8, END_PATTERN_1); //scw
        HAL_MVD_MemWrite4Byte(u32EndPattern+12,END_PATTERN_2); //scw
        HAL_MVD_MemWrite4Byte(u32EndPattern+16,END_PATTERN_3); //scw
        //printf("##########FileEnd for FLV/SVH!, u32EndPattern=%lx\n",u32EndPattern);
    }
    else if (E_MVD_CODEC_DIVX311 == eType)
    {
        HAL_MVD_MemWrite4Byte(u32EndPattern, DIVX_PATTERN);
        HAL_MVD_MemWrite4Byte(u32EndPattern+4, 0xffffffff);
        HAL_MVD_MemWrite4Byte(u32EndPattern+8, END_PATTERN_1); //scw
        HAL_MVD_MemWrite4Byte(u32EndPattern+12,END_PATTERN_2); //scw
        HAL_MVD_MemWrite4Byte(u32EndPattern+16,END_PATTERN_3); //scw
        //printf("##########FileEnd for DIVX311!, u32EndPattern=%lx\n",u32EndPattern);
    }
    else if ((E_MVD_CODEC_MPEG2 == eType)||(E_MVD_CODEC_MPEG4 == eType))
    {
        HAL_MVD_MemWrite4Byte(u32EndPattern, MPEG_PATTERN_0);
        HAL_MVD_MemWrite4Byte(u32EndPattern+4, END_PATTERN_1);
        HAL_MVD_MemWrite4Byte(u32EndPattern+8, END_PATTERN_2);
        HAL_MVD_MemWrite4Byte(u32EndPattern+12,END_PATTERN_3);
        //printf("##########FileEnd for MPEG2/4!, u32EndPattern=%lx\n",u32EndPattern);
    }
    else
    {
        HAL_MVD_MemWrite4Byte(u32EndPattern, END_PATTERN_0);
        HAL_MVD_MemWrite4Byte(u32EndPattern+4, END_PATTERN_1);
        HAL_MVD_MemWrite4Byte(u32EndPattern+8, END_PATTERN_2); //scw
        HAL_MVD_MemWrite4Byte(u32EndPattern+12,END_PATTERN_3); //scw
        //printf("##########FileEnd for VC1!, u32EndPattern=%lx\n",u32EndPattern);
    }

    pFileEnd->u32StAddr = u32EndPattern - MVD_GetMemOffset(stMemCfg.u32DrvBufAddr);
    pFileEnd->u32Length = END_PATTERN_SIZE;
    pFileEnd->u32TimeStamp = MVD_NULLPKT_PTS;
    pFileEnd->u32ID_L = MVD_U32_MAX;
    pFileEnd->u32ID_H = MVD_U32_MAX;
    MVD_DEBUGINFO(printf("u32EndPattern(0x%lx==>0x%lx)=0x%lx 0x%lx\n", u32EndPattern, pFileEnd->u32StAddr,
            HAL_MVD_MemRead4Byte(u32EndPattern), HAL_MVD_MemRead4Byte(u32EndPattern+4)));
}


static void MVD_SLQTblGetDummyPkt(MVD_CodecType eType, MVD_PacketInfo* pDummy)
{
    MS_U32 u32DummyES = MVD_GetMemOffset(stMemCfg.u32DrvBufAddr+SLQ_TBL_SIZE*2);
    MS_U32 u32DummyPattern[3];
    MS_U32 u32PatternSize;
    MS_U32 i;

    //printf("eType = 0x%x\n", eType);
    //initial content for dummy packet
    for (i=0; i<DUMMY_SIZE; i+=4)
    {
        HAL_MVD_MemWrite4Byte(u32DummyES+i, 0xffffffff);
    }

    switch (eType)
    {
        case E_MVD_CODEC_FLV:
        case E_MVD_CODEC_MPEG4_SHORT_VIDEO_HEADER:
            u32DummyPattern[0] = FLV_PATTERN;
            u32PatternSize = 1;
            break;

        case E_MVD_CODEC_DIVX311:
            u32DummyPattern[0] = DIVX_PATTERN;
            u32PatternSize = 1;
            break;

        case E_MVD_CODEC_VC1_ADV: //vc1
            u32DummyPattern[0] = VC1_PATTERN_0;
            u32DummyPattern[1] = VC1_PATTERN_1;
            u32DummyPattern[2] = VC1_PATTERN_2;
            u32PatternSize = 3;
            break;

        case E_MVD_CODEC_VC1_MAIN: //rcv
            u32DummyPattern[0] = RCV_PATTERN_0;
            u32DummyPattern[1] = RCV_PATTERN_1;
            u32DummyPattern[2] = RCV_PATTERN_2;
            u32PatternSize = 3;
            break;

        default:
            u32DummyPattern[0] = DUMMY_PATTERN;
            u32PatternSize = 1;
            break;
    }
    for (i=0; i<u32PatternSize; i++)
    {
        HAL_MVD_MemWrite4Byte(u32DummyES+i*4, u32DummyPattern[i]);
    }
#if 0
    printf("u32DummyES(0x%lx)=0x%08lx 0x%08lx 0x%08lx 0x%08lx\n", u32DummyES, HAL_MVD_MemRead4Byte(u32DummyES),
            HAL_MVD_MemRead4Byte(u32DummyES+4),HAL_MVD_MemRead4Byte(u32DummyES+8),HAL_MVD_MemRead4Byte(u32DummyES+12));
#endif
    pDummy->u32StAddr = u32DummyES - MVD_GetMemOffset(stMemCfg.u32DrvBufAddr);
    pDummy->u32Length = DUMMY_SIZE;
    pDummy->u32TimeStamp = MVD_NULLPKT_PTS;
    pDummy->u32ID_L = MVD_U32_MAX;
    pDummy->u32ID_H = MVD_U32_MAX;
#if 0
    printf("u32DummyES(0x%lx-->0x%lx, size=0x%lx)=0x%08lx 0x%08lx 0x%08lx 0x%08lx\n", u32DummyES,
            pDummy->u32StAddr, pDummy->u32Length, HAL_MVD_MemRead4Byte(u32DummyES),
            HAL_MVD_MemRead4Byte(u32DummyES+4),HAL_MVD_MemRead4Byte(u32DummyES+8),HAL_MVD_MemRead4Byte(u32DummyES+12));
#endif

}


//------------------------------------------------------------------------------------------------------------
// Layout of drvProcBuffer
//              -----------------
// drvProcBuff |                 | <- _drvSlqTbl.u32StAdd
//             |     8K bytes    |
//             | (1024 entries)  |
//             |                 |
//             |-----------------|
//             |                 | <- _drvSlqTbl.u32EndAdd   <- _drvDivxTbl.u32StAdd
//             |     8K bytes    |
//             | (1024 entries)  |
//             |                 |
//             |-----------------|
//             |                 |.......................... <- _drvDivxTbl.u32EndAdd   <- _drvEsTbl.u32StAdd
//             |     8K bytes    |
//             | (1024 entries)  |
//             |                 |
//             |-----------------|
//             |                 |..................................................... <- _drvEsTbl.u32EndAdd
//             |                 |
//
//------------------------------------------------------------------------------------------------------------
static void MVD_SLQTblInit(void)
{
    //printf("%s\n", __FUNCTION__);
    MS_U32 u32Addr, u32Len, i;//, u32EsStart;

    u32DummyPktCnt = 0;//reset dummy packet counter
    u32SlqByteCnt = 0; //reset SLQ table byte counter

    HAL_MVD_MemGetMap(E_MVD_MMAP_DRV, &u32Addr, &u32Len);
    //u32EsStart = u32Addr + u32Len;
    //printf("DRV_PROC@0x%lx 0x%lx es@0x%lx\n", u32Addr, u32Len, u32EsStart);

    //init SLQ table attributes
    _drvSlqTbl.u32StAdd  = u32Addr;
    _drvSlqTbl.u32EndAdd = u32Addr + SLQ_TBL_SIZE;
    _drvSlqTbl.u32EntryCntMax = SLQ_ENTRY_MAX;
    u32FileEndPtr = _drvSlqTbl.u32StAdd;

    //reset SLQ table read/write pointers
    _drvSlqTbl.u32RdPtr = _drvSlqTbl.u32StAdd;
    _drvSlqTbl.u32WrPtr = _drvSlqTbl.u32StAdd;
    _drvSlqTbl.u32Empty = SLQ_TBL_SIZE;

    //_SLQTbl_DumpInfo(&_drvSlqTbl);

    {
        _mvdCmdQ.u32PtsBase = MVD_NULLPKT_PTS;
    }

#if (!MVD_TURBO_INIT)
    //reset SLQ table
    _MVD_Memset(_drvSlqTbl.u32StAdd, 0, SLQ_TBL_SIZE);
#endif

    //set SLQ table start/end to F/W
    MDrv_MVD_SetSLQTblBufStartEnd(MVD_GetMemOffset(stMemCfg.u32BSAddr), MVD_GetMemOffset(stMemCfg.u32BSAddr+SLQ_TBL_SIZE));
    //if (stMemCfg.bFWMiuSel != stMemCfg.bHWMiuSel)
    if (bSlqTblSync)
    {
        //init BDMA for SLQ table update when MDrv_MVD_SetSLQWritePtr

#ifndef REDLION_LINUX_KERNEL_ENVI
        const BDMA_Info* pBDMA;
        pBDMA = MDrv_BDMA_GetInfo();
        if ((pBDMA == NULL) || (pBDMA->bInit != TRUE))
        {
            if (E_BDMA_OK != MDrv_BDMA_Init(stMemCfg.u32Miu1BaseAddr))
            {
                printf("%s fail at MDrv_BDMA_Init!!!\n", __FUNCTION__);
            }
        }

        if (stMemCfg.bHWMiuSel == MIU_SEL_1)
        {
            if (stMemCfg.bFWMiuSel == MIU_SEL_0)
            {
                bdmaCpyType = E_BDMA_SDRAM2SDRAM1;
            }
            else if (stMemCfg.bFWMiuSel == MIU_SEL_1)
            {
                bdmaCpyType = E_BDMA_SDRAM12SDRAM1;
            }
            else
            {
                MS_ASSERT(0);
            }
        }
        else if (stMemCfg.bHWMiuSel == MIU_SEL_0)
        {
            if (stMemCfg.bFWMiuSel == MIU_SEL_0)
            {
                bdmaCpyType = E_BDMA_SDRAM2SDRAM;
            }
            else if (stMemCfg.bFWMiuSel == MIU_SEL_1)
            {
                bdmaCpyType = E_BDMA_SDRAM12SDRAM;
            }
            else
            {
                MS_ASSERT(0);
            }
        }
        else
        {
            MS_ASSERT(0);
        }
#endif
    }

    ///// init SLQ entries for DivX311
    _drvDivxTbl.u32StAdd = _drvSlqTbl.u32EndAdd;
    _drvDivxTbl.u32EndAdd= _drvDivxTbl.u32StAdd + SLQ_TBL_SIZE;
    _drvDivxTbl.u32WrPtr = _drvDivxTbl.u32StAdd;
    //_drvDivxTbl.u32RdPtr = _drvDivxTbl.u32StAdd;
#if (!MVD_TURBO_INIT)
    //reset DivX311 pattern table
    _MVD_Memset(_drvDivxTbl.u32StAdd, 0, SLQ_TBL_SIZE);
#endif

#if (!MVD_TURBO_INIT)
    //dummy pattern entries
    _MVD_Memset(_drvDivxTbl.u32EndAdd, 0, DUMMY_SIZE);
#endif

    ///// init ES table
    _drvEsTbl.u32StAdd = _drvDivxTbl.u32EndAdd + DUMMY_SIZE;
    _drvEsTbl.u32EndAdd= _drvEsTbl.u32StAdd + ES_TBL_SIZE;
    _drvEsTbl.u32WrPtr = _drvEsTbl.u32StAdd;
    //reset ES table
    for (i = 0; i < ES_TBL_SIZE; i+=4)
    {
        HAL_MVD_MemWrite4Byte(_drvEsTbl.u32StAdd+i, stMemCfg.u32DrvBufSize);
    }

    return;
}


#if 0
static void _SLQTbl_DumpInfo(MVD_SLQ_TBL_ST* pInfo)
{
    printf("str=0x%lx\n", pInfo->u32StAdd);
    printf("end=0x%lx\n", pInfo->u32EndAdd);
    printf("cnt=0x%lx\n", pInfo->u32EntryCntMax);
    printf("rd =0x%lx\n", pInfo->u32RdPtr);
    printf("wr =0x%lx\n", pInfo->u32WrPtr);
    return;
}
#endif

#ifdef MVD_ENABLE_ISR
static MS_BOOL MVD_IntHasUsrData(MS_U32 u32IntStat)
{
    return (((u32IntStat&INT_USER_DATA)==INT_USER_DATA) ? TRUE : FALSE);
}

static MS_BOOL MVD_IntIsDispRdy(MS_U32 u32IntStat)
{
    return (((u32IntStat&INT_DISP_RDY)==INT_DISP_RDY) ? TRUE : FALSE);
}

static MS_BOOL MVD_IntHas1stFrame(MS_U32 u32IntStat)
{
    return (((u32IntStat&INT_FIRST_FRAME)==INT_FIRST_FRAME) ? TRUE : FALSE);
}

///Notice: This function only works when being called by fnHandler, which was
///registered by AP using MDrv_MVD_SetIsrEvent()
MVD_Event MDrv_MVD_GetIsrEvent(void)
{
    return eCurEvent;
}

void MVD_IsrProc(void)
{
    MS_U32 u32IntStat = 0;
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_GET_INT_STAT, &mvdcmd ) == TRUE)
    {
        u32IntStat = (((MS_U32)mvdcmd.Arg3) << 8) |
                     (((MS_U32)mvdcmd.Arg2));
    }

    if (u32IntStat != 0)
    {
        //MVD_DEBUGINFO(printf("MVD_IsrProc u32IntStat=%lx\n", u32IntStat));
        eCurEvent = E_MVD_EVENT_DISABLE_ALL;

        if ((eEventFlag & E_MVD_EVENT_USER_DATA) == E_MVD_EVENT_USER_DATA)
        {
            if (MVD_IntHasUsrData(u32IntStat))
            {
                eCurEvent |= E_MVD_EVENT_USER_DATA;
                MVD_DEBUGINFO(printf("===> UsrData!!!\n"));
            }
        }
        if ((eEventFlag & E_MVD_EVENT_DISP_RDY) == E_MVD_EVENT_DISP_RDY)
        {
            if (MVD_IntIsDispRdy(u32IntStat))
            {
                eCurEvent |= E_MVD_EVENT_DISP_RDY;
                MVD_DEBUGINFO(printf("===> DispRdy!!!\n"));
            }
        }
        if ((eEventFlag & E_MVD_EVENT_FIRST_FRAME) == E_MVD_EVENT_FIRST_FRAME)
        {
            if (MVD_IntHas1stFrame(u32IntStat))
            {
                eCurEvent |= E_MVD_EVENT_FIRST_FRAME;
                MVD_DEBUGINFO(printf("===> 1stFrame!!!\n"));
            }
        }

        //Events that user registered occurred, call user's callback function
        if ((eCurEvent!=E_MVD_EVENT_DISABLE_ALL) && (pfnCallback!=NULL))
        {
            pfnCallback();
        }
    }

    //clear interrupt & events
    eCurEvent = E_MVD_EVENT_DISABLE_ALL;
    HAL_MVD_ClearIRQ();
    OSAL_MVD_IntEnable(); //enable cpu interrupt mask
}

//-----------------------------------------------------------------------------
/// @brief \b Function \b Name: MDrv_HVD_SetISREvent()
/// @brief \b Function \b Description: Set the ISR event type sended by HVD fw.
/// @param -eEvent \b IN : event types
/// @param -fnISRHandler \b IN : function pointer to a interrupt handler.
/// @return -The result of command set ISR event.
//-----------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_SetIsrEvent(MVD_Event eEvent, MVD_InterruptCb fnHandler)
{
    static MS_BOOL bIsrAttached = FALSE;

    if (_bDrvInit != TRUE)
    {
        MVD_DEBUGERROR(printf("Call %s before Init\n", __FUNCTION__));
        return E_MVD_RET_FAIL;
    }

    if (eEvent == E_MVD_EVENT_DISABLE_ALL)
    {
        //HAL_MVD_Enable_ISR(FALSE);
        OSAL_MVD_IntDisable();
        OSAL_MVD_IsrDetach();
        bIsrAttached = FALSE;
        pfnCallback = NULL;
        eEventFlag = E_MVD_EVENT_DISABLE_ALL;
    }
    else
    {
        if(fnHandler != NULL)
        {
            //disable int & dettach isr?

            pfnCallback = (MVD_InterruptCb)fnHandler;
            eEventFlag |= eEvent;
            if (bIsrAttached != TRUE)
            {
                if (OSAL_MVD_IsrAttach((void*)MVD_IsrProc) != TRUE)
                {
                    MVD_DEBUGERROR(printf("fail to attach MVD_IsrProc!\n"));
                    return E_MVD_RET_FAIL;
                }
                if (OSAL_MVD_IntEnable() != TRUE)
                {
                    MVD_DEBUGERROR(printf("fail to OSAL_MVD_IntEnable!\n"));
                    return E_MVD_RET_FAIL;
                }
                bIsrAttached = TRUE;
            }
            MVD_DEBUGINFO(printf("attach ISR number:%d\n" , E_INT_IRQ_MVD));
            return E_MVD_RET_OK;
        }
        else
        {
            MVD_DEBUGERROR(printf( "SetISREvent with NULL pointer. ISR type:%lu\n", (MS_U32)eEvent));
            return E_MVD_RET_INVALID_PARAM;
        }
    }
    return E_MVD_RET_OK;
}

#endif //MVD_ENABLE_ISR


//------------------------------------------------------------------------------
/// Set AVSync mode for file mode.
/// @param -eSyncMode: avsync mode for file mode
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SetFileModeAVSync(MVD_TIMESTAMP_TYPE eSyncMode)
{
    _eFileSyncMode = eSyncMode;
    MVD_DEBUGINFO(printf("%s eSyncMode=%d\n", __FUNCTION__, eSyncMode));
    return HAL_MVD_SetFileModeAVSync(eSyncMode);
}


//-----------------------------------------------------------------------------
/// Is MVD firmware command finished.
/// @return - TRUE/FALSE
/// @retval     -FALSE(0): not finished.
/// @retval     -TRUE(1): well done!
//-----------------------------------------------------------------------------
static MS_BOOL MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_CMD eCmd)
{

#if defined(_MVD4)
    MVD_CMD_HANDSHADE_INDEX u32CmdState;
    MS_BOOL bCmdRdy = FALSE;
    MVD_FUNC_ENTRY();

    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("%s err: pu8MVDGetFrameInfoBufStart=NULL\n", __FUNCTION__));
        return FALSE;
    }

    u32CmdState.value = HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_CMD_HANDSHAKE_INDEX);
    MVD_DEBUGINFO(printf("u32CmdState.value = 0x%x\n", u32CmdState.value));
    switch (eCmd)
    {
        case MVD_HANDSHAKE_PAUSE:
            bCmdRdy = (MS_BOOL)(u32CmdState.mvdcmd_handshake_pause);
            break;
        case MVD_HANDSHAKE_SLQ_RST:
            bCmdRdy = (MS_BOOL)(u32CmdState.mvdcmd_handshake_slq_reset);
            break;
        case MVD_HANDSHAKE_STOP:
            bCmdRdy = (MS_BOOL)(u32CmdState.mvdcmd_handshake_stop);
            break;
        case MVD_HANDSHAKE_SKIP_DATA:
            bCmdRdy = (MS_BOOL)(u32CmdState.mvdcmd_handshake_skip_data);
            break;
        case MVD_HANDSHAKE_SINGLE_STEP:
            bCmdRdy = (MS_BOOL)(u32CmdState.mvdcmd_handshake_single_step);
            break;
        case MVD_HANDSHAKE_SCALER_INFO:
            bCmdRdy = (MS_BOOL)(u32CmdState.mvdcmd_handshake_scaler_data_ready);
            break;
        default:
            bCmdRdy = FALSE;
            break;
    }
    return bCmdRdy;
#else
    MVD_FUNC_ENTRY();
    return FALSE;
#endif
}

#if (defined(CHIP_T3)||defined(CHIP_U3))
//-----------------------------------------------------------------------------
/// Clear MVD firmware command finished bit.
/// @return - TRUE/FALSE
/// @retval     -FALSE(0): not finished.
/// @retval     -TRUE(1): well done!
//-----------------------------------------------------------------------------
static MS_BOOL MDrv_MVD_ClearCmdFinished(MVD_HANDSHAKE_CMD eCmd)
{
#if defined(_MVD4)
    MVD_CMD_HANDSHADE_INDEX u32CmdState;
    MVD_FUNC_ENTRY();

    if(pu8MVDGetFrameInfoBufStart==0)
    {
        MVD_DEBUGERROR(printf("%s err: pu8MVDGetFrameInfoBufStart=NULL\n", __FUNCTION__));
        return FALSE;
    }

    u32CmdState.value = HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_CMD_HANDSHAKE_INDEX);
    MVD_DEBUGINFO(printf("before CLR u32CmdState.value = 0x%x\n", u32CmdState.value));
    switch (eCmd)
    {
        case MVD_HANDSHAKE_SCALER_INFO:
            u32CmdState.mvdcmd_handshake_scaler_data_ready = 0;
            break;
        default:
            break;
    }

    //write the value back: may it overwrite the value that f/w is supposed to write?
    HAL_MVD_MemWrite4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_CMD_HANDSHAKE_INDEX, u32CmdState.value);
    MVD_DEBUGINFO(printf("after CLR u32CmdState.value = 0x%lx\n",
                  HAL_MVD_MemRead4Byte(pu8MVDGetFrameInfoBufStart+OFFSET_CMD_HANDSHAKE_INDEX)));
    return TRUE;
#else
    MVD_FUNC_ENTRY();
    return FALSE;
#endif
}
#endif

//-----------------------------------------------------------------------------
/// @brief Check if all of the buffers(display, decoded, bitstream) are empty.
/// @return - TRUE / FALSE
/// @retval     -FALSE(0): Not Empty.
/// @retval     -TRUE(1): Empty.
//-----------------------------------------------------------------------------
MS_BOOL MDrv_MVD_IsAllBufferEmpty(void)
{
    MVD_DEBUGINFO(printf("%s stat=0x%x, cmd=0x%x\n", __FUNCTION__,
                  MDrv_MVD_GetDecodeStatus(), MDrv_MVD_GetLastCmd()));
#if defined(CHIP_T2)
    return ((MDrv_MVD_GetDecodeStatus()!=E_MVD_STAT_FIND_SPECIALCODE)
         && (MDrv_MVD_GetDecodeStatus()!=E_MVD_STAT_WAIT_VDFIFO)
         && (MDrv_MVD_GetLastCmd()==CMD_PAUSE));
#else
    return MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_SKIP_DATA);
#endif
}


//-----------------------------------------------------------------------------
/// @brief Generate specific pattern to support some special function.
/// @param -u32VAddr \b IN : the virtual address of specific pattern
/// @param -u32Size \b IN, OUT :
///                             IN: the input array size.
///                             OUT: the used array size.
/// @return -The result of command generate specific pattern
//-----------------------------------------------------------------------------
MS_BOOL MDrv_MVD_GenPattern(MVD_PatternType ePattern, MS_PHYADDR u32PAddr, MS_U32* pu32Size)
{
    MS_U32* pu32DummyData = NULL;

    if ((!pu32Size) || (*pu32Size < SKIP_PATTERN_SIZE))
    {
        return FALSE;
    }

    pu32DummyData = (MS_U32 *) HAL_MVD_PA2NonCacheSeg(u32PAddr);

    switch (ePattern)
    {
        case E_MVD_PATTERN_FLUSH:
            *pu32DummyData = SKIP_PATTERN_0;
            pu32DummyData++;
            *pu32DummyData = SKIP_PATTERN_1;
            *pu32Size = SKIP_PATTERN_SIZE;
            //printf("##########FLUSH!, 0x%lx 0x%lx\n",*pu32DummyData, *(pu32DummyData-1));
            break;
        case E_MVD_PATTERN_FILEEND:
            if ((E_MVD_CODEC_FLV == curCodecType)||(E_MVD_CODEC_MPEG4_SHORT_VIDEO_HEADER == curCodecType))
            {
                *pu32DummyData = FLV_PATTERN;
                pu32DummyData++;
                *pu32DummyData = 0xffffffff;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_1;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_2;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_3;
                //printf("##########FileEnd for FLV/SVH!, *pu32DummyData=%lx\n",*pu32DummyData);
            }
            else if (E_MVD_CODEC_DIVX311 == curCodecType)
            {
                *pu32DummyData = DIVX_PATTERN;
                pu32DummyData++;
                *pu32DummyData = 0xffffffff;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_1;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_2;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_3;
                //printf("##########FileEnd for DIVX311!, *pu32DummyData=%lx\n",*pu32DummyData);
            }
            else if ((E_MVD_CODEC_MPEG2 == curCodecType)||(E_MVD_CODEC_MPEG4 == curCodecType))
            {
                *pu32DummyData = MPEG_PATTERN_0;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_1;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_2;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_3;
                //printf("##########FileEnd for MPEG2/4!, *pu32DummyData=%lx\n",*pu32DummyData);
            }
            else
            {
                *pu32DummyData = END_PATTERN_0;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_1;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_2;
                pu32DummyData++;
                *pu32DummyData = END_PATTERN_3;
                //printf("##########FileEnd for VC1!, *pu32DummyData=%lx\n",*pu32DummyData);
            }
            *pu32Size = 16;
            break;
        default:
            break;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
/// @brief Get driver specific data information
/// @return -the information of choosed type
//-----------------------------------------------------------------------------
MS_U32 MDrv_MVD_GetPatternInfo(void)
{
    return HW_FIFO_DEPTH;
}

//-----------------------------------------------------------------------------
/// @brief Pass scalar parameters to f/w
/// @return -The result of command.
//-----------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_SetDynScalingParam(MS_PHYADDR u32StAddr, MS_U32 u32Size)
{
#if (defined(CHIP_T3)||defined(CHIP_U3))
    #define SCALER_INFO_TIMEOUT 0x1000
    MS_U32 u32TimeOut = 0;
    MS_U32 u32SrcAdd = NULL;
    MS_U32 i;

    if ((u32StAddr==NULL) || (u32Size==0) || (u32Size>MVD_FW_SCALER_INFO_BUF_LEN))
    {
        MVD_DEBUGERROR(printf("%s invalid para u32StAddr=0x%lx, u32Size=0x%lx\n",
                       __FUNCTION__, u32StAddr, u32Size));
        return E_MVD_RET_INVALID_PARAM;
    }
    if (TRUE != stMemCfg.bEnableDynScale)
    {
        MVD_DEBUGERROR(printf("%s !bEnableDynScale\n", __FUNCTION__));
        return E_MVD_RET_FAIL;
    }

    //copy data
    u32SrcAdd = HAL_MVD_PA2NonCacheSeg(u32StAddr);
    u32Size = ((u32Size+3)>>2)<<2;
    MVD_DEBUGINFO(printf("u32Size= 0x%lx, u32SrcAdd= 0x%lx\n", u32Size, u32SrcAdd));
    for (i=0; i<u32Size; i=i+4)
    {
        HAL_MVD_MemWrite4Byte(u32ScalerInfoAdd+i, *(volatile MS_U32*)(u32SrcAdd+i));
        MVD_DEBUGINFO(printf("0x%lx = 0x%lx\n", u32ScalerInfoAdd+i, HAL_MVD_MemRead4Byte(u32ScalerInfoAdd+i)));
    }

    //notify f/w
    if (TRUE!=HAL_MVD_SetScalerInfoAddr(u32ScalerInfoAdd)) //Set the buffer address (MIU offset) to f/w
    {
        MVD_DEBUGERROR(printf("%s fail to set ScalerInfoAdd\n", __FUNCTION__));
        return E_MVD_RET_FAIL;
    }

    //check f/w already handle the data
    while(MDrv_MVD_IsCmdFinished(MVD_HANDSHAKE_SCALER_INFO) && (u32TimeOut < SCALER_INFO_TIMEOUT))
    {
        u32TimeOut++;
    }
    if(u32TimeOut >= SCALER_INFO_TIMEOUT)
    {
        MVD_DEBUGERROR(printf("%s timeout!!!\n", __FUNCTION__));
        return E_MVD_RET_FAIL;
    }

    //clear ack bit
    MDrv_MVD_ClearCmdFinished(MVD_HANDSHAKE_SCALER_INFO);

    MVD_DEBUGINFO(printf("=====> %s u32TimeOut = 0x%lx\n", __FUNCTION__, u32TimeOut));
    return E_MVD_RET_OK;
#else
    return E_MVD_RET_FAIL;
#endif
}

//------------------------------------------------------------------------------
/// Set the dynamic scale base address
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SetDynamicScaleAddr(MS_U32 u32addr)
{
    MS_U32 u32offset;
    u32offset = MVD_GetMemOffset(u32addr);
    return HAL_MVD_SetDynamicScaleAddr(u32offset);
}

//------------------------------------------------------------------------------
/// Set virtual box width/height to F/W.
/// F/W will use the same w/h as scaler to calculate scaling factor.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SetVirtualBox(MS_U16 u16Width, MS_U16 u16Height)
{
    MVD_DEBUGINFO(printf("%s: w=0x%x h=0x%x\n", __FUNCTION__, u16Width, u16Height));
    return HAL_MVD_SetVirtualBox(u16Width, u16Height);
}

//------------------------------------------------------------------------------
/// Enable VD_MHEG5 QDMA
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_EnableQDMA(void)
{
    return HAL_MVD_EnableQDMA();
}


//------------------------------------------------------------------------------
/// Set blue screen
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL MDrv_MVD_SetBlueScreen(MS_BOOL bEn)
{
    return HAL_MVD_SetBlueScreen(bEn);
}


//------------------------------------------------------------------------------
/// Enable FW only show one field.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_EnableDispOneField(MS_BOOL bEn)
{
    MVD_FrcMode eFrcMode = E_MVD_FRC_NORMAL;
    if (bEn)
    {
        eFrcMode = E_MVD_FRC_DISP_ONEFIELD;
    }
    if (TRUE == MDrv_MVD_DispCtrl(FALSE, FALSE, FALSE, eFrcMode))
        return E_MVD_RET_OK;
    else
        return E_MVD_RET_FAIL;
}


//------------------------------------------------------------------------------
/// Get info of the decoded/displaying frame.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_GetFrmInfo(MVD_FrmInfoType eType, MVD_FrmInfo* pInfo)
{
    E_MVD_Result eRet = E_MVD_RET_OK;
    if (NULL == pInfo)
    {
        return E_MVD_RET_INVALID_PARAM;
    }
    if (NULL == u32DecFrmInfoAdd)
    {
        return E_MVD_RET_FAIL;
    }

    //printf("%s u32DecFrmInfoAdd = 0x%lx\n", __FUNCTION__, u32DecFrmInfoAdd);

#if defined(_MVD4)
    if (E_MVD_FRMINFO_DECODE == eType)
    {
        pInfo->u32LumaAddr  = HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_LUMAADDR);
        pInfo->u32ChromaAddr= HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_CHROMAADDR);
        pInfo->u32TimeStamp = _90K_TO_MS(HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_TIMESTAMP));
        pInfo->u32ID_L  = HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_ID_L);
        pInfo->u32ID_H  = HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_ID_H);
        pInfo->u16Pitch = HAL_MVD_MemRead2Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_PITCH);
        pInfo->u16Width = HAL_MVD_MemRead2Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_WIDTH);
        pInfo->u16Height= HAL_MVD_MemRead2Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_HEIGHT);
        pInfo->eFrmType = (MVD_PicType)HAL_MVD_MemRead2Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DEC_FRAMETYPE);
    }
    else if (E_MVD_FRMINFO_DISPLAY == eType)
    {
        pInfo->u32LumaAddr  = HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_LUMAADDR);
        pInfo->u32ChromaAddr= HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_CHROMAADDR);
        pInfo->u32TimeStamp = _90K_TO_MS(HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_TIMESTAMP));
        pInfo->u32ID_L  = HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_ID_L);
        pInfo->u32ID_H  = HAL_MVD_MemRead4Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_ID_H);
        pInfo->u16Pitch = HAL_MVD_MemRead2Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_PITCH);
        pInfo->u16Width = HAL_MVD_MemRead2Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_WIDTH);
        pInfo->u16Height= HAL_MVD_MemRead2Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_HEIGHT);
        pInfo->eFrmType = (MVD_PicType)HAL_MVD_MemRead2Byte(u32DecFrmInfoAdd + OFFSET_DECFRAMEINFO_DISP_FRAMETYPE);
    }
    else
#endif
    {
        eRet = E_MVD_RET_INVALID_PARAM;
    }

    if (stMemCfg.bHWMiuSel == MIU_SEL_0)
    {
        pInfo->u32LumaAddr = pInfo->u32LumaAddr * 8;
        pInfo->u32ChromaAddr = pInfo->u32ChromaAddr * 8;
    }
    else
    {
        pInfo->u32LumaAddr = pInfo->u32LumaAddr * 8 + stMemCfg.u32Miu1BaseAddr;
        pInfo->u32ChromaAddr = pInfo->u32ChromaAddr * 8 + stMemCfg.u32Miu1BaseAddr;
    }

    return eRet;
}


//------------------------------------------------------------------------------
/// Enable/Disable freezing display
/// Once this flag is set, firmware continue decode new frame
/// but not to push this frame into display queue.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
E_MVD_Result MDrv_MVD_SetFreezeDisp(MS_BOOL bEn)
{
    MDrv_MVD_SetSpeed(E_MVD_SPEED_DEFAULT, 1);
    if (TRUE == HAL_MVD_SetFreezeDisp(bEn))
        return E_MVD_RET_OK;
    else
        return E_MVD_RET_FAIL;
}

MVD_InternalMemCfg *MDrv_MVD_GetInternalMemCfg(void)
{
    return &_stInternalMemCfg;
}

//------------------------------------------------------------------------------
/// Debug function to dump useful info.
//------------------------------------------------------------------------------
void MDrv_MVD_DbgDump(void)
{
    MVD_FrmInfo stFrm;
    MS_U32 u32VdCnt=0;
    static MS_U32 u32PreVdCnt=0;
    MS_U32 u32ErrCnt=0;
    static MS_U32 u32PreErrCnt=0;

    if (!_bDrvInit)
    {
        printf("%s: _bDrvInit false!\n", __FUNCTION__);
        return;
    }
    u32VdCnt=MDrv_MVD_GetParserByteCnt();
    u32ErrCnt=MDrv_MVD_GetVldErrCount();
    printf("input: vfifo=%d(full=%d,empty=%d), vdCnt=%ld(%ld), vldErr=%ld(%ld); ",
            HAL_MVD_RegReadByte(0x1564)>>6, HAL_MVD_RegReadByte(0x1564)&0x20,
            HAL_MVD_RegReadByte(0x1564)&0x10, u32VdCnt, (u32VdCnt-u32PreVdCnt),
            u32ErrCnt, (u32ErrCnt-u32PreErrCnt));
    u32PreVdCnt = u32VdCnt;
    u32PreErrCnt = u32ErrCnt;
#if defined(_MVD4)
    printf("state: fw=0x%x, lastCmd=0x%x, pc=0x%lx\n",
            MDrv_MVD_GetDecodeStatus(), MDrv_MVD_GetLastCmd(), HAL_VPU_GetProgCnt());
#else
    printf("state: fw=0x%x, lastCmd=0x%x\n",
            MDrv_MVD_GetDecodeStatus(), MDrv_MVD_GetLastCmd());
#endif
    printf("seq(%d): w=%d, h=%d, i/p=%x, fps=%ld; ", MDrv_MVD_GetDispRdy(), stPreFrmInfo.u16HorSize,
            stPreFrmInfo.u16VerSize, stPreFrmInfo.u8Interlace, stPreFrmInfo.u32FrameRate);
    printf("cnt: dec=%ld, skip=%ld, drop=%ld\n", MDrv_MVD_GetPicCounter(), MDrv_MVD_GetSkipPicCounter(), 0x0UL);
    printf("avsync on=%x, delay=%ld, tolerance=%d, done=%x, skip=%x, repeat=%x, pts=%ldms\n",
            stSyncCfg.bEnable, stSyncCfg.u32Delay, stSyncCfg.u16Tolerance, (MDrv_MVD_GetSyncStatus()==1),
            MDrv_MVD_GetIsSyncSkip(), MDrv_MVD_GetIsSyncRep(), MDrv_MVD_GetPTS());
    MDrv_MVD_GetFrmInfo(E_MVD_FRMINFO_DECODE, &stFrm);
    printf("frm Dec Y=%lx UV=%lx Pitch=%x; ", stFrm.u32LumaAddr, stFrm.u32ChromaAddr, stFrm.u16Pitch);
    MDrv_MVD_GetFrmInfo(E_MVD_FRMINFO_DISPLAY, &stFrm);
    printf("Disp Y=%lx UV=%lx Pitch=%x\n", stFrm.u32LumaAddr, stFrm.u32ChromaAddr, stFrm.u16Pitch);
    if (curSrcMode == E_MVD_SLQ_TBL_MODE)
    {
        printf("fe=%lx, rd=%lx(%lx), wr=%lx, empty=%lx; ", u32FileEndPtr, _drvSlqTbl.u32RdPtr,
                MVD_Map2DrvSlqTbl(MDrv_MVD_GetSLQReadPtr()), _drvSlqTbl.u32WrPtr, _drvSlqTbl.u32Empty);
        printf("es rd=0x%lx, wr=0x%lx\n", MDrv_MVD_GetESReadPtr(), MDrv_MVD_GetESWritePtr());
    }
    else if (curSrcMode == E_MVD_TS_FILE_MODE)
    {
        printf("es rd=0x%lx, wr=0x%lx\n", MDrv_MVD_GetESReadPtr(), MDrv_MVD_GetESWritePtr());
    }
}

//------------------------------------------------------------------------------
/// Dump the bitstream to predefined buffer address.
/// Before PLAY command, set the bitstream base & bitstream length, and then
/// CPU would continue to dump bitstream at the base address.
/// @param -u32base \b IN : start address (MIU offset, e.g. 128M==>0MB)
/// @param -u32size \b IN : size (bytes)
//------------------------------------------------------------------------------
void MDrv_MVD_DbgDumpBitstream(MS_U32 u32base, MS_U32 u32size)
{
#if defined(_MVD4)
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32base%8)==0);
    u32base >>= 3;
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32base);
    mvdcmd.Arg1 = H_WORD(u32base);
    mvdcmd.Arg2 = L_DWORD(u32base);
    mvdcmd.Arg3 = H_DWORD(u32base);
    if (HAL_MVD_MVDCommand( CMD_DUMP_BITSTREAM_BASE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\n", CMD_DUMP_BITSTREAM_BASE ) );
        return;
    }

    MS_ASSERT((u32size%8)==0);
    u32size >>= 3;
    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32size);
    mvdcmd.Arg1 = H_WORD(u32size);
    mvdcmd.Arg2 = L_DWORD(u32size);
    mvdcmd.Arg3 = H_DWORD(u32size);
    if (HAL_MVD_MVDCommand( CMD_DUMP_BITSTREAM_LENGTH, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\n", CMD_DUMP_BITSTREAM_LENGTH ) );
        return;
    }
    return;
#endif
}


#if (defined(CHIP_T7))
//-----------------------------------------------------------------------------
/// MVD set VOP Done
/// @return - TRUE/FALSE
/// @retval     -FALSE(0): not finished.
/// @retval     -TRUE(1): well done!
//-----------------------------------------------------------------------------
void MDrv_Mvd_SetVOPDone(void)
{
    MVD_CmdArg stCmdArg;

    MVD_FUNC_ENTRY();
    SETUP_CMDARG(stCmdArg);
    stCmdArg.Arg0 = VOP_INI_DONE;
    stCmdArg.Arg1 = 0;
    stCmdArg.Arg2 = 1;
    stCmdArg.Arg3 = 0;

    if (HAL_MVD_MVDCommand(CMD_SET_PARAMETER, &stCmdArg) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", VOP_INI_DONE ) );
        return;
    }

}

/********************************************************************************/
/// Set mvd play mode
/// @return -none
/********************************************************************************/
//need check

void MDrv_Mvd_SetPlayMode(MS_U8 bFileMode, MS_U8 bDisablePESParsing)
{
    u8MvdPlayMode = (MS_U8)((bFileMode?1:0)<<2) | (MS_U8)((bDisablePESParsing?1:0)<<3);
}

void MDrv_Mvd_CallDummy(void)
{
    u32MVDFWSLQTABTmpbufAdr = NULL;
    u32MVDFWPtsTblAddr = NULL;
    u32DecFrmInfoAdd = NULL;
    u32DynScalingAdd = NULL;
    u8DynScalingDepth = 0;
    u32ScalerInfoAdd = NULL;
    u32VolAdd = 0;
}
#endif
// only for VDEC internal link patch
MS_BOOL MDrv_MVD_LinkWeakSymbolPatch(void)
{
    return TRUE;
}


