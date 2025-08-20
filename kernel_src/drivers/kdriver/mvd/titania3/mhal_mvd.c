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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    Mhal_mvd.c
/// @brief  MVD HAL layer Driver
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include "Board.h"
#include "chip_int.h"
#include "mhal_mvd.h"
#include "mhal_chiptop_reg.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "mdrv_mvd_st.h"
#include "mvd4_interface.h"
#include "mhal_mvd_reg.h"

#include "drvMVD_Common.h"
#include "drvMVD.h"

#define COMBU16(hi,lo)          ((((U16)hi)<<8) | ((U16)lo))
#define COMBU32(hh,hl,lh,ll)    ((((U32)hh)<<24) | (((U32)hl)<<16) | (((U32)lh)<<8) | ((U32)ll))

#define MVD_DBG_LEVEL               0
#define MVD_SYNC_DELAY              0     // unit: ms
#define MVD_IFRAME_TIMEOUT          1000

//memory map
#define MVD3_FW_USER_DATA_BUF_LEN   0x1000  // 4k
#define MVD3_FW_SLQ_TAB_TMPBUF_LEN  0x200

#if defined(CONFIG_MSTAR_TITANIA_MMAP_32MB_32MB) || \
    defined(CONFIG_MSTAR_TITANIA_MMAP_64MB_32MB) || \
    defined(CONFIG_MSTAR_TITANIA_MMAP_64MB_64MB) || \
    defined(CONFIG_MSTAR_TITANIA_MMAP_128MB_128MB) || \
    defined(CONFIG_MSTAR_TITANIA_MMAP_256MB_256MB)
#define MVD_ADDR_OFFSET              0x10000000UL // MIU1 256MB
#else
#define MVD_ADDR_OFFSET              0
#endif

#define MVD_TRACE_MSG   0
#if (MVD_TRACE_MSG == 1)
#define MHAL_MVD_TRACE(...) printk(__VA_ARGS__)
#else
#define MHAL_MVD_TRACE(...)
#endif

#define _MVD_RET_HANDLE(x)                                      \
    do                                                          \
    {                                                           \
        if (E_MVD_RET_OK != (x))                                \
        {                                                       \
            printk("[%s:%d fail\n]", __FUNCTION__, __LINE__);   \
            return MVDSTATUS_FAIL;                              \
        }                                                       \
    } while (0)

#define _BOOL_TO_MHAL_RESULT(b)                                 \
    do                                                          \
    {                                                           \
        if (TRUE != (b))                                        \
        {                                                       \
            printk("[%s:%d fail\n]", __FUNCTION__, __LINE__);   \
            return MVDSTATUS_FAIL;                              \
        }                                                       \
    } while (0)


//by swen for T3
#define FPGA_VERIFY 1

extern U8 *pDataMem;
extern U32 u32NumOfData;
extern U32 *pMVDDataMemSerNumb;

extern MS_BOOL      HAL_MVD_MVDCommand(MS_U8 u8cmd, MVD_CmdArg *pstCmdArg);
//------------------------------------------------------------------------------
// Local Functions Prototype
//------------------------------------------------------------------------------
static void         _MHal_MVD_InitVar(void);
static void         _MHal_MVD_UsrDataInit(void);

//------------------------------------------------------------------------------
//  Global Variables
//------------------------------------------------------------------------------
U32 gInterruptFlag                      = 0;
//------------------------------------------------------------------------------
// Local Veriables
//------------------------------------------------------------------------------
static U32 gu32PictureUserDataStart     = 0;
static U32 gu32PictureUserDataEnd       = 0;
static U32 gu32PictureUserDataPrev      = 0;

#ifdef __aeon__
    static U32 u32MVDRegBase = 0xA0200000;
#else
    #ifdef CHIP_T2
    static U32 u32MVDRegBase = 0xBF800000;
    #else
    static U32 u32MVDRegBase = 0xBF200000;
    #endif
#endif

static MVD_CodecType _gMVD_Codec        = E_MVD_CODEC_UNKNOWN;

BOOL _bMVD_State_Init  = FALSE;

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------
static void _MHal_MVD_InitVar(void)
{
    gu32PictureUserDataPrev = 0;
}

static void _MHal_MVD_UsrDataInit(void)
{
    MVD_CmdArg cmdArg;

    memset(&cmdArg, 0, sizeof(MVD_CmdArg));

    cmdArg.Arg0 = 1;

    if (FALSE == HAL_MVD_MVDCommand(CMD_CLOSE_CC, &cmdArg))
    {
        printk("Command CC fail!!\r\n");
    }
}

//------------------------------------------------------------------------------
/// TEST MVD FW location
/// @return TRUE if on miu-1
//------------------------------------------------------------------------------
BOOL MHal_MVD_IsOnMIU1(void)
{
    U32 u32CodeAddr, u32CodeSize;
    
    MDrv_SYS_GetMMAP(E_SYS_MMAP_SVD, &u32CodeAddr, &u32CodeSize);

    if (u32CodeAddr < MIU1_MEM_BASE_ADR)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

//------------------------------------------------------------------------------
/// MPEG Video Decoder(MVD) power on.
/// @return MVDSTATUS_SUCCESSS: Process success.
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_PowerOn(void)
{
    // by swen for T3
#if FPGA_VERIFY
    CKLGEN0_REG(REG_CLKGEN0_MVD) &= ~(CLKGEN0_CKG_MVD_DIS);
    CKLGEN0_REG(0x0038) &= ~(_BIT(8));
#else
    //enable MVD clock/MVD boot clock
    TOP_REG(REG_TOP_STC0_MAD_MVD) &= ~(TOP_CKG_MVD_DIS);
    TOP_REG(REG_TOP_MVD_DC0_RVD_GE) &= ~(TOP_CKG_MVD_IAP_RMEM_DIS);
#endif

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// MPEG Video Decoder(MVD) power off.
/// @return MVDSTATUS_SUCCESSS: Process success.
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_PowerOff(void)
{
    // by swen for T3
#if FPGA_VERIFY
    CKLGEN0_REG(REG_CLKGEN0_MVD) |= (CLKGEN0_CKG_MVD_DIS);
    CKLGEN0_REG(0x0038) |= (_BIT(8));
#else
    //disable MVD clock/MVD boot clock
    TOP_REG(REG_TOP_STC0_MAD_MVD) |= (TOP_CKG_MVD_DIS);
    TOP_REG(REG_TOP_MVD_DC0_RVD_GE) |= (TOP_CKG_MVD_IAP_RMEM_DIS);
#endif

    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_Init(MVD_Codec_Type codec)
{
    MVD_FWCfg fwCfg;
    MVD_MEMCfg memCfg;
    U32 u32CodeAddr;
    U32 u32CodeSize;
    U32 u32FBAddr;
    U32 u32FBSize;
    U32 u32BSAddr;
    U32 u32BSSize;

    memset(&fwCfg, 0, sizeof(MVD_FWCfg));
    memset(&memCfg, 0, sizeof(MVD_MEMCfg));

    MDrv_MVD_SetDbgLevel(MVD_DBG_LEVEL);

    MDrv_SYS_GetMMAP(E_SYS_MMAP_SVD, &u32CodeAddr, &u32CodeSize);
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_FB, &u32FBAddr, &u32FBSize);
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_BS, &u32BSAddr, &u32BSSize);

    memCfg.u32Miu1BaseAddr = MIU1_MEM_BASE_ADR;
    memCfg.u32FWCodeAddr = u32CodeAddr;
    memCfg.u32FWCodeSize = u32CodeSize;
    memCfg.u32FBAddr = u32FBAddr;
    memCfg.u32FBSize = u32FBSize;
    memCfg.u32BSAddr = u32BSAddr;
    memCfg.u32BSSize = u32BSSize;

    MDrv_MVD_SetCfg(&fwCfg, &memCfg);

    // set register base
    MDrv_MVD_RegSetBase(u32MVDRegBase);

    //(*(volatile unsigned int*)(0xBF203D4C)) &= ~(0xF);
    //(*(volatile unsigned int*)(0xBF203D4C)) |= 2;

    if (!MDrv_MVD_Init())
    {
        MVD_Debug("%d: MVD init fail\n", __LINE__);
        return MVDSTATUS_FAIL;
    }

    _MHal_MVD_InitVar();

    _MHal_MVD_UsrDataInit();

    _gMVD_Codec = E_MVD_CODEC_UNKNOWN;
    
    if (E_MVDCODEC_MPEG2 == codec)
    {
        _gMVD_Codec = E_MVD_CODEC_MPEG2;
    }
    else if (E_MVDCODEC_MPEG4_PART2 == codec)
    {
        _gMVD_Codec = E_MVD_CODEC_MPEG4;
    }
    
    if (E_MVD_CODEC_UNKNOWN == _gMVD_Codec)
    {
        MVD_Debug("%s: unsupported codec\n", __FUNCTION__);
        return MVDSTATUS_INVALID_PARAMETERS;
    }

    MDrv_MVD_SetCodecInfo(_gMVD_Codec, E_MVD_TS_MODE, FALSE);

    MHal_MVD_SetSyncThreshold(0xff);

	MHal_MVD_ClearIRQ();
    _bMVD_State_Init=TRUE;
	enable_irq(E_IRQ_MVD);

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Play input video stream
/// @param frc : frame rate conversion mode.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_STREAM_ID: Invalid input stream index.
/// @return MVDSTATUS_INVALID_COMMAND: Invalid operation command.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_Play(U8 frc)
{
    U32 u32BSBufAddr, u32BSBufSize;
	
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_BS, &u32BSBufAddr, &u32BSBufSize);
    MHal_MVD_SetOverflowTH((((u32BSBufSize * 15) / 16) >> 3) << 3);
    MHal_MVD_SetUnderflowTH(((u32BSBufSize) / 16 >> 3) << 3);
    MDrv_MVD_Play();

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Pause input video stream
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_COMMAND: Invalid operation command.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_Pause(void)
{
    MDrv_MVD_Pause();

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Stop input video stream
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_Stop(void)
{
	if (_bMVD_State_Init==TRUE)
	{
        _bMVD_State_Init=FALSE;
        disable_irq(E_IRQ_MVD);
	    MHal_MVD_ClearIRQ();
        MDrv_MVD_Exit();
	}

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Decodes one I frame. The I frame is not immediately displayed.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_Decode_IFrame(U32 u32StreamBufStart, U32 u32StreamBufEnd)
{
    U32 u32FBAddr;
    U32 u32FBSize;
    
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_FB, &u32FBAddr, &u32FBSize);
    
    _BOOL_TO_MHAL_RESULT(MDrv_MVD_DecodeIFrame(u32FBAddr, u32StreamBufStart, 
        u32StreamBufEnd));

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get the decoding frame count.
/// @param pu16FrameCount \b OUT: Decding frame count
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_STREAM_ID: Invalid input stream index.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetDecdeFrameCount(U16 *pu16FrameCount)
{  
    if (NULL == pu16FrameCount)
    {
        return MVDSTATUS_INVALID_PARAMETERS;
    }
    
    *pu16FrameCount = MDrv_MVD_GetPicCounter();
    
    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get the Presentation time stamp (PTS) of input stream
/// @param pu32PTSLow \b OUT: PTS (bit0 ~ bit31)
/// @param pu32PTSHigh \b OUT: PTS (bit32)
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_STREAM_ID: Invalid input stream index.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetPTS(U32 *pu32PTSLow, U32 *pu32PTSHigh)
{
    if (NULL == pu32PTSLow || NULL == pu32PTSHigh)
    {
        return MVDSTATUS_INVALID_PARAMETERS;
    }
    
    *pu32PTSLow = MDrv_MVD_GetPTS() * 90; // 90 kHz
    *pu32PTSHigh = 0;

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get video resolution, fram rate, and aspect ratio
/// @param pPictureData \b OUT: Picture data information.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_STREAM_ID: Invalid input stream index.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetPictureData(MVD_FRAMEINFO *pPictureData)
{
    MVD_FrameInfo info;

    memset(&info, 0, sizeof(info));

    MDrv_MVD_GetFrameInfo(&info);

    pPictureData->u16HorSize = info.u16HorSize;
    pPictureData->u16VerSize = info.u16VerSize;
    pPictureData->u16FrameRate = (U16) info.u32FrameRate;
    pPictureData->u8AspectRatio = info.u8AspectRate;
    pPictureData->u8Interlace = info.u8Interlace;
    pPictureData->u8AFD = info.u8AFD;
    pPictureData->u16CropRight = info.u16CropRight;
    pPictureData->u16CropLeft = info.u16CropLeft;
    pPictureData->u16CropTop = info.u16CropTop;
    pPictureData->u16CropBottom = info.u16CropBottom;
    pPictureData->u32BitRate = MDrv_MVD_GetBitsRate();
    pPictureData->u8LowDelay = MDrv_MVD_GetLowDelayFlag();

    MHAL_MVD_TRACE("[%s]: w=%d, h=%d, framerate=%d\n",
        __FUNCTION__,
        pPictureData->u16HorSize,
        pPictureData->u16VerSize,
        pPictureData->u16FrameRate);

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Reset MVD HW blocks.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_Reset(MVD_Codec_Type codec)
{
    U32 u32Ret;
    wait_queue_head_t wq;
    init_waitqueue_head(&wq);

    u32Ret = MHal_MVD_Init(codec);
    wait_event_interruptible_timeout(wq, 0, HZ * 50 / 1000);

    return u32Ret;
}

//------------------------------------------------------------------------------
/// change the AV sync state
/// @param u8Flag \b IN: 1 for enabling AV sync, 0 for disabling AV sync.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_ToggleAVSync(u8 u8Flag)
{
    MDrv_MVD_SetAVSync(u8Flag, MVD_SYNC_DELAY);/* Original : 100ms ->  New : 0ms */

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// set the AV sync threshold
/// @param u16Threshold \b IN: Value of threshold.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_SetAVSyncThreshold(U16 u16Threshold)
{
    _BOOL_TO_MHAL_RESULT(MDrv_MVD_ChangeAVsync(TRUE, u16Threshold / 90));

    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_GetActiveFormat(U32 *active_format)
{
    if (NULL == active_format)
    {
        return MVDSTATUS_INVALID_PARAMETERS;
    }
    
    *active_format = MDrv_MVD_GetActiveFormat();

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// reading MVD interrupt status
/// @param none
/// @return Interrupt status
/// @internal
//------------------------------------------------------------------------------
U16 MHal_MVD_ReadIRQ(void)
{
    MVD_CmdArg cmdArg;

    memset(&cmdArg, 0, sizeof(MVD_CmdArg));
    
    if (FALSE == HAL_MVD_MVDCommand(CMD_GET_INT_STAT, &cmdArg))
    {
        printk("[%s] failed\n", __FUNCTION__);
        return MVDSTATUS_FAIL;
    }

    return COMBU16(cmdArg.Arg3, cmdArg.Arg2);
}

//------------------------------------------------------------------------------
/// Clean the IRQ bit (in interrupt handler should call this function while the
/// interrupt has been triggered.
/// @param none
/// @return none
/// @internal
//------------------------------------------------------------------------------
void MHal_MVD_ClearIRQ(void)
{
    MVD_REG(MVD_STAT_CTRL) |= MVD_INT_CLR;
    return;
}

//------------------------------------------------------------------------------
/// Set watching interrupt
/// @param  u32Flag    \b IN: Flag for subscribing interrupt event
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
U16 Mhal_MVD_SetIntSubscribe(U32 u32Flag)
{
    if (u32Flag)
    {
        u32NumOfData = 0;
        
        if (pDataMem != NULL)
        {
            memset(pDataMem, 0, 100 * 256);
            *pMVDDataMemSerNumb = 0;
        }
		//if (_bMVD_State_Init == TRUE)
	    //    enable_irq(E_IRQ_MVD);
    }

    gInterruptFlag = u32Flag;

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get the picture header information
/// @param  pPIC_Hdr_Data    \b OUT: output the picture header information
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetPictureHeader(pMVD_PIC_HEADER_T pPIC_Hdr_Data)
{
    MVD_PicType picType = MDrv_MVD_GetPicType();

    switch (picType)
    {
        case E_MVD_PIC_I:
            pPIC_Hdr_Data->u8PicType = 1;
            break;
        case E_MVD_PIC_P:
            pPIC_Hdr_Data->u8PicType = 2;
            break;
        case E_MVD_PIC_B:
            pPIC_Hdr_Data->u8PicType = 3;
            break;
        default:
            break;
    }

    pPIC_Hdr_Data->u8Top_ff = MDrv_MVD_GetTop1stField();
    pPIC_Hdr_Data->u8Rpt_ff = MDrv_MVD_GetRepeat1stField();
    pPIC_Hdr_Data->u16TmpRef = MDrv_MVD_GetTmpRefField();

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get the sequence header information
/// @param  pMVD_SEQUENCE_HEADER_Tret    \b OUT: output the sequence header information
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetSequenceHeader(MVD_SEQUENCE_HEADER_T *pSeqHdrT)
{
    MVD_FrameInfo info;

    memset(&info, 0, sizeof(info));

    MDrv_MVD_GetFrameInfo(&info);

    pSeqHdrT->u16HorSize    = info.u16HorSize;
    pSeqHdrT->u16VerSize    = info.u16VerSize;
    pSeqHdrT->u16FrameRate  = (U16) info.u32FrameRate;
    pSeqHdrT->u8AspectRatio = info.u8AspectRate;
    pSeqHdrT->u8Progressive = !info.u8Interlace;
    pSeqHdrT->u32BitRate    = MDrv_MVD_GetBitsRate();
    
    MHAL_MVD_TRACE("[%s] w=%d, h=%d, framerate=%d, system time =%lu\n",
            __FUNCTION__,
            pSeqHdrT->u16HorSize,
            pSeqHdrT->u16VerSize,
            pSeqHdrT->u16FrameRate,
            MDrv_SYS_GetSyetemTime());

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get the picture header & user data
/// @param  ppStrLast     \b IN/OUT: pointer of pMVD_Data_Pkt
/// @param  pu32NumOfData \b IN/OUT: pointer of Pkt number
/// @param  u32Skip       \b IN: Skip update PKt
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetPictureHeaderAndUserData(pMVD_Data_Pkt *ppStrLast,
                                               U32 *pu32NumOfData,
                                               U32 u32Skip)
{
    U32 u32SizeOfHeaderAndUserData  = 0;
    U32 u32tmpSizeOfUserData        = 0;
    U32 u32AddressWritePt           = 0;
    U32 u32AddressReadPt            = 0;
    U32 u32tmpAddressReadPt         = 0;
    pMVD_Data_Pkt pMVD_Data_PktIs   = NULL;
    U32 i                           = 0;
    MVD_CmdArg cmdArg;

    //get User Data
    pMVD_Data_PktIs = (pMVD_Data_Pkt) *ppStrLast;

    cmdArg.Arg0 = 0;
    cmdArg.Arg1 = 0;
    cmdArg.Arg2 = 0;
    cmdArg.Arg3 = 2;
    cmdArg.Arg4 = 0;
    cmdArg.Arg5 = 0;

    if (FALSE == HAL_MVD_MVDCommand(CMD_RD_USER_WP, &cmdArg))
    {
        printk("MVD: Get picture user address failed!!\n");
        return MVDSTATUS_FAIL;
    }

    u32AddressWritePt = COMBU32(cmdArg.Arg3, cmdArg.Arg2, cmdArg.Arg1, cmdArg.Arg0);

    MHAL_MVD_TRACE("Write Ptr 0x%x\n", u32AddressWritePt);

    // The MVD design is aligned 8-bits boundary and the address is in byte (8-bit)
    //
    //gu32PictureUserDataStart              gu32PictureUserDataEnd
    // |                                    |
    // v                                    v
    // +------------------------------------+
    // |  |XXXXX|                           |
    // +------------------------------------+
    //    ^     ^
    //    |     |
    //    |     u32AddressWritePt
    //    |
    //    gu32PictureUserDataPrev

    pMVD_Data_PktIs->u32SizeOfDataPkt = 0;//just init return value.

    if (0 == gu32PictureUserDataPrev)
    {
        //this behavior will drop first picture user data
        gu32PictureUserDataPrev = u32AddressWritePt;
        pMVD_Data_PktIs->u32SizeOfDataPkt = 0;
        return MVDSTATUS_SUCCESS;
    }
    else
    {
        u32AddressReadPt = gu32PictureUserDataPrev;

        if (u32AddressReadPt == u32AddressWritePt)
        {
            //no new data
            pMVD_Data_PktIs->u32SizeOfDataPkt = 0;
            return MVDSTATUS_SUCCESS;
        }
        if ((u32AddressWritePt < gu32PictureUserDataStart) ||
            (u32AddressWritePt >= gu32PictureUserDataEnd))
        {
            //incorrect memory range
            MVD_Debug("MVD:PIC USR DATA Write pt in Wrong Range 0x%x - 0x%x, 0x%x\n",
                      gu32PictureUserDataStart,
                      gu32PictureUserDataEnd,
                      u32AddressWritePt);
            pMVD_Data_PktIs->u32SizeOfDataPkt = 0;
            return MVDSTATUS_FAIL;
        }

        if (u32AddressReadPt > u32AddressWritePt) // round back to beginning
        {
            gu32PictureUserDataPrev = u32AddressWritePt;

            if (u32Skip != 0)
            {
                u32SizeOfHeaderAndUserData = (gu32PictureUserDataEnd -
                                              u32AddressReadPt) +
                                             (u32AddressWritePt -
                                              gu32PictureUserDataStart);
                //MVD_Debug( "Size of Header & User Data = %d \n", u32SizeOfHeaderAndUserData);
                if (u32SizeOfHeaderAndUserData <= DATA_SIZE)
                {
                    //MVD_Debug("=========================> Roll Back 1: ReadPt 0x%x, WritePt 0x%x, Size 0x%x\n",
                    //    u32AddressReadPt, u32AddressWritePt, u32SizeOfHeaderAndUserData);

                    if (MHal_MVD_IsOnMIU1())
                    {
                        u32tmpSizeOfUserData = (U32)
                                               (*((U8 *)
                                               ((u32AddressReadPt + 3) |
                                               0xD0000000)));
                        u32tmpAddressReadPt = u32AddressReadPt | 0xD0000000;
                    }
                    else
                    {
                        u32tmpSizeOfUserData = (U32)
                                              (*((U8 *)
                                              ((u32AddressReadPt + 3) |
                                               0xA0000000)));
                        u32tmpAddressReadPt = u32AddressReadPt | 0xA0000000;
                    }

                    memcpy(&pMVD_Data_PktIs->pPktData,
                           (U8 *) u32tmpAddressReadPt,
                           (u32tmpSizeOfUserData + (sizeof(U8) * 4) + sizeof(U16)));

                    /*
                    MVD_Debug("PicType 0x%x, top_ff 0x%x, rpt_ff 0x%x, User Data Size %d, tmpRef 0x%x\n",
                        *(U8 *)(u32tmpAddressReadPt),
                        *(U8 *)(u32tmpAddressReadPt+1),
                        *(U8 *)(u32tmpAddressReadPt+2),
                        *(U8 *)(u32tmpAddressReadPt+3),
                        *(U16 *)(u32tmpAddressReadPt+4));
                        */


                    pMVD_Data_PktIs->u32SizeOfDataPkt = u32tmpSizeOfUserData +
                                                        (sizeof(U8) * 4) +
                                                        sizeof(U16) +
                                                        (sizeof(U32) * 3);
                    //MVD_Debug("u32SizeOfDataPkt %d\n", pMVD_Data_PktIs->u32SizeOfDataPkt);
                    pMVD_Data_PktIs->u32TypeOfStruct = 6;

                    //MVD_Debug("1 u32NumOfData %d\n", *pu32NumOfData);
                    pMVD_Data_PktIs->u32NumOfDataPkt = (*pu32NumOfData)++;
                	*pMVDDataMemSerNumb = (*pu32NumOfData);
                    //MVD_Debug("2 u32NumOfData %d\n", *pu32NumOfData);

                    //MVD_Debug("1 pStrLast 0x%x\n", (U32)*ppStrLast);

                    if ((*pu32NumOfData) % QUEUE_SIZE == 0)
                    {
                        *((U8 * *) ppStrLast) = pDataMem;
                        //printk("Type %d PKT Queue Roll Back!! Num 0x%x\n", pMVD_Data_PktIs->u32TypeOfStruct, *pu32NumOfData);
                    }
                    else
                    {
                        *((U8 * *) ppStrLast) += PKT_SIZE;
                    }

                    //MVD_Debug("2 pStrLast 0x%x\n", (U32)*ppStrLast);
                    pMVD_Data_PktIs = (pMVD_Data_Pkt) * ppStrLast;//for missing userdata 20090106
                }
                else
                {
                    //MVD_Debug("=========================> Roll Back 2: ReadPt 0x%x, WritePt 0x%x, Size 0x%x\n",
                    //    u32AddressReadPt, u32AddressWritePt, u32SizeOfHeaderAndUserData);

                    u32SizeOfHeaderAndUserData = gu32PictureUserDataEnd -
                                                 u32AddressReadPt;
                    //MVD_Debug( "Size of Header & User Data = %d \n", u32SizeOfHeaderAndUserData);
                    for (i = 0;
                         i < (u32SizeOfHeaderAndUserData / DATA_SIZE);
                         i++)
                    {

                        if (MHal_MVD_IsOnMIU1())
                        {
                            u32tmpSizeOfUserData = (U32)
                                                   (*((U8 *)
                                                   ((u32AddressReadPt + 3) |
                                                   0xD0000000)));
                            u32tmpAddressReadPt = u32AddressReadPt | 0xD0000000;
                        }
                        else
                        {
                            u32tmpSizeOfUserData = (U32)
                                                  (*((U8 *)
                                                  ((u32AddressReadPt + 3) |
                                                   0xA0000000)));
                            u32tmpAddressReadPt = u32AddressReadPt | 0xA0000000;
                        }

                        memcpy(&pMVD_Data_PktIs->pPktData,
                               (U8 *)
                               u32tmpAddressReadPt,
                               (u32tmpSizeOfUserData +
                                (sizeof(U8) * 4) +
                                sizeof(U16)));

                        /*
                        MVD_Debug("PicType 0x%x, top_ff 0x%x, rpt_ff 0x%x, User Data Size %d, tmpRef 0x%x\n",
                            *(U8 *)(u32tmpAddressReadPt),
                            *(U8 *)(u32tmpAddressReadPt+1),
                            *(U8 *)(u32tmpAddressReadPt+2),
                            *(U8 *)(u32tmpAddressReadPt+3),
                            *(U16 *)(u32tmpAddressReadPt+4));
                            */


                        pMVD_Data_PktIs->u32SizeOfDataPkt = u32tmpSizeOfUserData +
                                                            (sizeof(U8) * 4) +
                                                            sizeof(U16) +
                                                            (sizeof(U32) * 3);
                        //MVD_Debug("u32SizeOfDataPkt %d\n", pMVD_Data_PktIs->u32SizeOfDataPkt);
                        pMVD_Data_PktIs->u32TypeOfStruct = 6;

                        //MVD_Debug("1 u32NumOfData %d\n", *pu32NumOfData);
                        pMVD_Data_PktIs->u32NumOfDataPkt = (*pu32NumOfData)++;
                        //MVD_Debug("2 u32NumOfData %d\n", *pu32NumOfData);
                		*pMVDDataMemSerNumb = (*pu32NumOfData);
                        //MVD_Debug("1 pStrLast 0x%x\n", (U32)*ppStrLast);

                        if ((*pu32NumOfData) % QUEUE_SIZE == 0)
                        {
                            *((U8 * *) ppStrLast) = pDataMem;
                            //printk("Type %d PKT Queue Roll Back!! Num 0x%x\n", pMVD_Data_PktIs->u32TypeOfStruct, *pu32NumOfData);
                        }
                        else
                        {
                            *((U8 * *) ppStrLast) += PKT_SIZE;
                        }

                        //MVD_Debug("2 pStrLast 0x%x\n", (U32)*ppStrLast);

                        u32AddressReadPt += DATA_SIZE;
                        pMVD_Data_PktIs = (pMVD_Data_Pkt) * ppStrLast;
                    }

                    u32SizeOfHeaderAndUserData = u32AddressWritePt -
                                                 gu32PictureUserDataStart;
                    //MVD_Debug( "Size of Header & User Data = %d \n", u32SizeOfHeaderAndUserData);

                    u32AddressReadPt = gu32PictureUserDataStart;
                    for (i = 0;
                         i < (u32SizeOfHeaderAndUserData / DATA_SIZE);
                         i++)
                    {
                         if (MHal_MVD_IsOnMIU1())
                         {
                             u32tmpSizeOfUserData = (U32)
                                                    (*((U8 *)
                                                    ((u32AddressReadPt + 3) |
                                                    0xD0000000)));
                             u32tmpAddressReadPt = u32AddressReadPt | 0xD0000000;
                         }
                         else
                         {
                             u32tmpSizeOfUserData = (U32)
                                                   (*((U8 *)
                                                   ((u32AddressReadPt + 3) |
                                                    0xA0000000)));
                             u32tmpAddressReadPt = u32AddressReadPt | 0xA0000000;
                        }

                        memcpy(&pMVD_Data_PktIs->pPktData,
                               (U8 *)
                               u32tmpAddressReadPt,
                               (u32tmpSizeOfUserData +
                                (sizeof(U8) * 4) +
                                sizeof(U16)));

                        /*
                        MVD_Debug("PicType 0x%x, top_ff 0x%x, rpt_ff 0x%x, User Data Size %d, tmpRef 0x%x\n",
                            *(U8 *)(u32tmpAddressReadPt),
                            *(U8 *)(u32tmpAddressReadPt+1),
                            *(U8 *)(u32tmpAddressReadPt+2),
                            *(U8 *)(u32tmpAddressReadPt+3),
                            *(U16 *)(u32tmpAddressReadPt+4));
                            */


                        pMVD_Data_PktIs->u32SizeOfDataPkt = u32tmpSizeOfUserData +
                                                            (sizeof(U8) * 4) +
                                                            sizeof(U16) +
                                                            (sizeof(U32) * 3);
                        //MVD_Debug("u32SizeOfDataPkt %d\n", pMVD_Data_PktIs->u32SizeOfDataPkt);
                        pMVD_Data_PktIs->u32TypeOfStruct = 6;

                        //MVD_Debug("1 u32NumOfData %d\n", *pu32NumOfData);
                        pMVD_Data_PktIs->u32NumOfDataPkt = (*pu32NumOfData)++;
                        //MVD_Debug("2 u32NumOfData %d\n", *pu32NumOfData);
                		*pMVDDataMemSerNumb = (*pu32NumOfData);
                        //MVD_Debug("1 pStrLast 0x%x\n", (U32)*ppStrLast);

                        if ((*pu32NumOfData) % QUEUE_SIZE == 0)
                        {
                            *((U8 * *) ppStrLast) = pDataMem;
                            //printk("Type %d PKT Queue Roll Back!! Num 0x%x\n", pMVD_Data_PktIs->u32TypeOfStruct, *pu32NumOfData);
                        }
                        else
                        {
                            *((U8 * *) ppStrLast) += PKT_SIZE;
                        }

                        //MVD_Debug("2 pStrLast 0x%x\n", (U32)*ppStrLast);

                        u32AddressReadPt += DATA_SIZE;
                        pMVD_Data_PktIs = (pMVD_Data_Pkt) * ppStrLast;//for missing userdata 20090106
                    }
                }
            }
        }
        else //(u32AddressReadPt < u32AddressWritePt) Normal case
        {
            gu32PictureUserDataPrev = u32AddressWritePt;

            if (u32Skip != 0)
            {
                u32SizeOfHeaderAndUserData = u32AddressWritePt -
                                             u32AddressReadPt;
                //MVD_Debug( "Size of Header & User Data = %d \n", u32SizeOfHeaderAndUserData);

                for (i = 0; i < (u32SizeOfHeaderAndUserData / DATA_SIZE); i++)
                {
                    //map to physicall address is
                    if (MHal_MVD_IsOnMIU1())
                    {
                        u32tmpSizeOfUserData = (U32)
                                               (*((U8 *)
                                               ((u32AddressReadPt + 3) |
                                               0xD0000000)));
                        u32tmpAddressReadPt = u32AddressReadPt | 0xD0000000;
                    }
                    else
                    {
                        u32tmpSizeOfUserData = (U32)
                                              (*((U8 *)
                                              ((u32AddressReadPt + 3) |
                                               0xA0000000)));
                        u32tmpAddressReadPt = u32AddressReadPt | 0xA0000000;
                    }

                    memcpy(&pMVD_Data_PktIs->pPktData,
                           (U8 *)
                           u32tmpAddressReadPt,
                           (u32tmpSizeOfUserData +
                            (sizeof(U8) * 4) +
                            sizeof(U16)));

                    /*
                    MVD_Debug("PicType 0x%x, top_ff 0x%x, rpt_ff 0x%x, User Data Size %d, tmpRef 0x%x\n",
                        *(U8 *)(u32tmpAddressReadPt),
                        *(U8 *)(u32tmpAddressReadPt+1),
                        *(U8 *)(u32tmpAddressReadPt+2),
                        *(U8 *)(u32tmpAddressReadPt+3),
                        *(U16 *)(u32tmpAddressReadPt+4));
                        */


                    pMVD_Data_PktIs->u32SizeOfDataPkt = u32tmpSizeOfUserData +
                                                        (sizeof(U8) * 4) +
                                                        sizeof(U16) +
                                                        (sizeof(U32) * 3);
                    //MVD_Debug("u32SizeOfDataPkt %d\n", pMVD_Data_PktIs->u32SizeOfDataPkt);
                    pMVD_Data_PktIs->u32TypeOfStruct = 6;

                    //MVD_Debug("1 u32NumOfData %d\n", *pu32NumOfData);
                    pMVD_Data_PktIs->u32NumOfDataPkt = (*pu32NumOfData)++;
                    //MVD_Debug("2 u32NumOfData %d\n", *pu32NumOfData);
                	*pMVDDataMemSerNumb = (*pu32NumOfData);
                    //MVD_Debug("1 pStrLast 0x%x\n", (U32)*ppStrLast);

                    if ((*pu32NumOfData) % QUEUE_SIZE == 0)
                    {
                        *((U8 * *) ppStrLast) = pDataMem;
                        //printk("Type %d PKT Queue Roll Back!! Num 0x%x\n", pMVD_Data_PktIs->u32TypeOfStruct, *pu32NumOfData);
                    }
                    else
                    {
                        *((U8 * *) ppStrLast) += PKT_SIZE;
                    }

                    //MVD_Debug("2 pStrLast 0x%x\n", (U32)*ppStrLast);

                    u32AddressReadPt += DATA_SIZE;
                    pMVD_Data_PktIs = (pMVD_Data_Pkt) * ppStrLast;
                }
            }
        }
    }

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get get the status of the PTS-STC sync for video and audio
/// @param  u32AVSyncStatus    \b OUT: the av sync status
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetAvSyncStatus(u32 *u32AVSyncStatus)
{
    if (NULL == u32AVSyncStatus)
    {
        return MVDSTATUS_INVALID_PARAMETERS;
    }
    
    *u32AVSyncStatus = MDrv_MVD_GetSyncStatus();
    
    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Set the picture user data buffer for MVD
/// @param  pDataBuff    \b IN/OUT: Buffer start position
/// @param  u32SizeOfBuff \b IN: size of pDataBuff
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_SetPictureUserDataBuffer(U8 *pDataBuff, U32 u32SizeOfBuff)
{
    U32 PicUserDataBufferStart  = 0;
    U32 PicUserDataBufferSize   = 0;

    PicUserDataBufferStart = (U32) pDataBuff;
    PicUserDataBufferSize = u32SizeOfBuff;

    // set the buffer start and end (remove highest byte! please refer MIPS memory architecture)
    gu32PictureUserDataStart = (PicUserDataBufferStart - MVD_ADDR_OFFSET) & 0x0fffffff;
    gu32PictureUserDataEnd = (PicUserDataBufferStart -
                              MVD_ADDR_OFFSET +
                              PicUserDataBufferSize) & 0x0fffffff;

    MHAL_MVD_TRACE("Mhal: Picture User Data addr 0x%x, end 0x%x\n",
              gu32PictureUserDataStart,
              gu32PictureUserDataEnd);

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// get the MVD bit-stream buffer memory location
/// @param  pu32BufferStart     \b OUT: Buffer start position
/// @param  pu32SizeOfBuff      \b OUT: size of bit-stream buffer
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetBitStreamBuffer(U32 *pu32BufferStart,
                                      U32 *pu32SizeOfBuff)
{
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_BS, pu32BufferStart, pu32SizeOfBuff);
    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// get the MVD frame buffer memory location
/// @param  pu32BufferStart     \b OUT: Buffer start position
/// @param  pu32SizeOfBuff      \b OUT: size of frame buffer
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetFrameBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff)
{
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_FB, pu32BufferStart, pu32SizeOfBuff);
    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// get the MVD user data buffer memory location
/// @param  pu32BufferStart     \b OUT: Buffer start position
/// @param  pu32SizeOfBuff      \b OUT: size of frame buffer
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetUserDataBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff)
{
    MVD_InternalMemCfg *cfg = MDrv_MVD_GetInternalMemCfg();

    *pu32BufferStart = cfg->u32UserDataBuf;
    *pu32SizeOfBuff = MVD3_FW_USER_DATA_BUF_LEN;

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// fast forward/backward/slow motion play
/// @param PlayMode \b IN: fast forward/backward/slow motion play mode
/// @param u8FrameRateUnit \b IN: frame display duration (frame rate unit)
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_PVRPlayMode(MVD_PVR_PlayMode PlayMode, U8  u8FrameRateUnit)
{
    //MVD_ARG MvdArg;
    MVD_CmdArg cmdArg;
    U32 u32BSBufAddr, u32BSBufSize;

    MHal_MVD_GetBitStreamBuffer(&u32BSBufAddr, &u32BSBufSize);

    if ((u32BSBufSize / 16) < 0x400)
    {
        printk("Set PVR Threshold error\n");
        return MVDSTATUS_FAIL;
    }

    MHal_MVD_SetUnderflowTH((((u32BSBufSize * 15) / 16) >> 3) << 3);
    MHal_MVD_SetOverflowTH(((u32BSBufSize - 0x400) >> 3) << 3);

    MHal_MVD_SetCodecInfo(_gMVD_Codec, TS_FILE_MODE, ENABLE_PKT_LEN);

    // turn off av sync if not normal mode
    MHal_MVD_ToggleAVSync(PlayMode ? FALSE : TRUE);

    //set play command
    /*
    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));   
    if (_MHal_MVD_SetCmd(CMD_PLAY, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_PLAY);
        return MVDSTATUS_FAIL;
    }
    */
    
    memset(&cmdArg, 0, sizeof(MVD_CmdArg));
    
    if (FALSE == HAL_MVD_MVDCommand(CMD_PLAY, &cmdArg))
    {
        printk("%s failed\n", __FUNCTION__);
        return MVDSTATUS_FAIL;
    }
    
    // set display speed control command
    /*
    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    if ((PlayMode == E_MVD_PVR_DECODE_IP) || (PlayMode == E_MVD_PVR_DECODE_I))
    {
        MvdArg.u8Arg0 = 1;
    }
    else if (PlayMode == E_MVD_PVR_SLOW_MOTION)
    {
        MvdArg.u8Arg0 = 2;
    }
    else
    {
        MvdArg.u8Arg0 = 0;
    }

    MvdArg.u8Arg1 = u8FrameRateUnit;

    if (_MHal_MVD_SetCmd(CMD_DISP_SPEED_CTRL, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_DISP_SPEED_CTRL);
        return MVDSTATUS_FAIL;
    }    
    */

    memset(&cmdArg, 0, sizeof(MVD_CmdArg));

    if ((PlayMode == E_MVD_PVR_DECODE_IP) || (PlayMode == E_MVD_PVR_DECODE_I))
    {
        cmdArg.Arg0 = 1;
    }
    else if (PlayMode == E_MVD_PVR_SLOW_MOTION)
    {
        cmdArg.Arg0 = 2;
    }
    else
    {
        cmdArg.Arg0 = 0;
    }    

    cmdArg.Arg1 = u8FrameRateUnit;
    
    if (FALSE == HAL_MVD_MVDCommand(CMD_DISP_SPEED_CTRL, &cmdArg))
    {
        printk("%s failed\n", __FUNCTION__);
        return MVDSTATUS_FAIL;
    }

#if 0
    // set 1x or 2x or 4x
    memset((void*)&MvdArg, 0, sizeof(MVD_ARG));
    if ((PlayMode == E_MVD_PVR_DECODE_IP) || (PlayMode == E_MVD_PVR_DECODE_I))
    {
        if (u8FrameRateUnit == 2)
            MvdArg.u8Arg0 = 1;
        else if(u8FrameRateUnit == 4)
            MvdArg.u8Arg0 = 2;
        else
            MvdArg.u8Arg0 = 0;

        MvdArg.u8Arg1 = 0;
    }
#if 0 //TO DO : remove from latest F/W?
    if (_MHal_MVD_SetCmd(CMD_FW_ADJ_FF, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_FW_ADJ_FF);
    return MVDSTATUS_FAIL;
    }
#endif

    if ((PlayMode == E_MVD_PVR_DECODE_IP) || (PlayMode == E_MVD_PVR_DECODE_I))
        u8FrameRateUnit = 1;

    // set fast_slow command
    memset((void*)&MvdArg, 0, sizeof(MVD_ARG));
    MvdArg.u8Arg0 = PlayMode;
    MvdArg.u8Arg1 = u8FrameRateUnit;
    if (_MHal_MVD_SetCmd(CMD_FAST_SLOW, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_FAST_SLOW);
        return MVDSTATUS_FAIL;
    }
#endif

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Play input video stream
/// @param u32StartAddr : input address
/// @param u32EndAddr : input length
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_SetFilePlay(U32 u32StartAddr, U32 u32EndAddr)
{
    //MVD_ARG MvdArg;
    MVD_CmdArg cmdArg;
    U32 u32Time;
    U32 u32Addr         = 0;
    U32 u32Size         = 0;
    U16 u16DecodeCount  = 0;


    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_FB, &u32Addr, &u32Size);
    MHal_MVD_SetFrameBufferAddr(u32Addr);

    //Test File Play
    /*
        {
            U8 *pTemp = NULL;
            MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_BS,&u32Addr,&u32Size);
            u32StartAddr = u32Addr;
            u32EndAddr = u32Addr+(((414632+0x7)>>3)<<3);
            pTemp = ioremap(u32StartAddr, 414632);
            memcpy( pTemp, DripData, 414632);
        }
    */

    MHal_MVD_SetCodecInfo(_gMVD_Codec, SLQ_MODE, DISABLE_PARSER);//mpeg2,SLQ mode


    //Set Play command
    /*
    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    if (_MHal_MVD_SetCmd(CMD_PLAY, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_PLAY);
        return MVDSTATUS_FAIL;
    }
    */

    memset(&cmdArg, 0, sizeof(MVD_CmdArg));

    if (FALSE == HAL_MVD_MVDCommand(CMD_PLAY, &cmdArg))
    {
        printk("%s failed\n", __FUNCTION__);
        return MVDSTATUS_FAIL;
    }

    /*
        memset((void*)&MvdArg, 0, sizeof(MVD_ARG));
        if (_MHal_MVD_SetCmd(CMD_SINGLE_STEP, &MvdArg) != MVDSTATUS_SUCCESS)
        {
            MVD_Debug("Command: 0x%x fail!!\r\n", CMD_SINGLE_STEP);
            return MVDSTATUS_FAIL;
        }
    */
    /*
    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    MvdArg.u8Arg0 = 0; //normal
    MvdArg.u8Arg1 = 1;
    if (_MHal_MVD_SetCmd(CMD_FAST_SLOW, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_FAST_SLOW);
        return MVDSTATUS_FAIL;
    }
    */

    memset(&cmdArg, 0, sizeof(MVD_CmdArg));

    cmdArg.Arg0 = 0;
    cmdArg.Arg1 = 1;

    if (FALSE == HAL_MVD_MVDCommand(CMD_FAST_SLOW, &cmdArg))
    {
        printk("%s failed\n", __FUNCTION__);
        return MVDSTATUS_FAIL;
    }

    MVD_Debug("Stream Start 0x%x, end 0x%x", u32StartAddr, u32EndAddr);
    /*
    MVD_Debug("SLQTMP Addr 0x%x, end 0x%x",
              u32SLQTabTMPBufAddr,
              u32SLQTabTMPBufAddr + MVD3_FW_SLQ_TAB_TMPBUF_LEN);
    */
    
    //set data
    MHal_MVD_SetSLQStartEnd((u32StartAddr - MVD_ADDR_OFFSET),
                            (u32EndAddr - MVD_ADDR_OFFSET));
    //MHal_MVD_SetSLQStartEnd((u32SLQTabTMPBufAddr - MVD_ADDR_OFFSET), (u32SLQTabTMPBufAddr+MVD3_FW_SLQ_TAB_TMPBUF_LEN - MVD_ADDR_OFFSET));

    if (MHal_MVD_GetDecdeFrameCount(&u16DecodeCount) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("MHal_MVD_Decode_IFrame get decode frame count fail\n");
        return MVDSTATUS_FAIL;
    }

    //Wait frame decode Done
    u32Time = ((unsigned long) ((jiffies) * (1000 / HZ))); //MsOS_GetSystemTime();
    //while((MsOS_GetSystemTime() - u32Time) < MVD_IFRAME_TIMEOUT)
    while ((u16DecodeCount < 1) &&
           ((((unsigned long) ((jiffies) * (1000 / HZ))) - u32Time) <
            MVD_IFRAME_TIMEOUT))
    {
        mdelay(100);

        if (MHal_MVD_GetDecdeFrameCount(&u16DecodeCount) != MVDSTATUS_SUCCESS)
        {
            MVD_Debug("MHal_MVD_Decode_IFrame get decode frame count fail\n");
            return MVDSTATUS_FAIL;
        }
    }

    if ((((unsigned long) ((jiffies) * (1000 / HZ))) - u32Time) >
        MVD_IFRAME_TIMEOUT)
    {
        MVD_Debug("MHal_MVD_Decode_IFrame time out\n");
        return MVDSTATUS_FAIL;
    }

    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Close input video stream
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_SetFileClose(void)
{
    return MVDSTATUS_SUCCESS;
}


//------------------------------------------------------------------------------
/// Play input video stream
/// @param u8PlayFrameRate : frame rate conversion mode.
/// @param u8Mode : for file mode
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return MVDSTATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_SetFilePlay2(U8 u8PlayFrameRate, U8  u8Mode)
{
    return MVDSTATUS_FAIL;
}

//------------------------------------------------------------------------------
/// Get MVD first frame decode status
/// @return -first frame decoded ready or not
//------------------------------------------------------------------------------
U8 MHal_MVD_GetFirstFrame(void)
{
    return MHal_MVD_GetDispRdy();
}

//------------------------------------------------------------------------------
/// Get MVD first frame header decode status
///   - 0, sequence ready
///   - 1, not ready
/// @return -first frame decoded ready or not
//------------------------------------------------------------------------------
U8 MHal_MVD_GetDispRdy(void)
{
    if (MDrv_MVD_GetDispRdy())
    {
        MHAL_MVD_TRACE("First Frame Found!!\n");
        return 0;
    }
    else
    {
        return 1;
    }
}

//------------------------------------------------------------------------------
/// Get sync status to know whether sync is complete or not
//------------------------------------------------------------------------------
U8 MHal_MVD_GetSyncStatus(void)
{
    MHAL_MVD_TRACE("[%s] sync status = %d\n", __FUNCTION__, MDrv_MVD_GetSyncStatus());
    return MDrv_MVD_GetSyncStatus();
}

U8 MHal_MVD_SetVOPDone(void)
{
    return TRUE;
}

//------------------------------------------------------------------------------
/// Get video progressive or interlace
/// @return -video progressive or interlace
//------------------------------------------------------------------------------
U8 MHal_MVD_GetProgInt(void)
{
    MVD_FrameInfo info;

    memset(&info, 0, sizeof(info));

    MDrv_MVD_GetFrameInfo(&info);

    return !info.u8Interlace;
}

//------------------------------------------------------------------------------
/// Get number of video SKIP
/// @return - number of video SKIP ready or not
//------------------------------------------------------------------------------
U8 MHal_MVD_GetVideoSkip(void)
{
    return MDrv_MVD_GetSkipPicCounter();
}

//------------------------------------------------------------------------------
/// Get number of video REPEAT
/// @return - number of video REPEAT ready or not
//------------------------------------------------------------------------------
U8 MHal_MVD_GetVideoRepeat(void)
{
    return 0;
}

U8 MHal_MVD_IPicFound(void)
{
    return MDrv_MVD_GetIsIPicFound();
}

//------------------------------------------------------------------------------
/// Set frame buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void MHal_MVD_SetFrameBufferAddr(U32 u32addr)
{
    if (MHal_MVD_IsOnMIU1())
    {
        u32addr -= MVD_MIU1_BASE_ADDR;
    }

    MDrv_MVD_SetFrameBuffAddr(u32addr);
}

void MHal_MVD_SetSLQStartEnd(U32 u32start, U32 u32end)
{
    MDrv_MVD_SetSLQStartEnd(u32start, u32end);
}

//------------------------------------------------------------------------------
//@param -u8CodecType \b IN : 0: mpeg4, 1: mpeg4 with short_video_header, 2: DivX311
//       -u8BSProviderMode \b IN : ??
//------------------------------------------------------------------------------
void MHal_MVD_SetCodecInfo(U8 u8CodecType,
                           U8 u8BSProviderMode,
                           U8 bDisablePESParsing)
{
    MDrv_MVD_SetCodecInfo(u8CodecType,
        u8BSProviderMode, bDisablePESParsing);
}

//------------------------------------------------------------------------------
/// Set bitstream buffer overflow threshold
/// @return -none
//------------------------------------------------------------------------------
void MHal_MVD_SetOverflowTH(U32 u32Threshold)
{
    MDrv_MVD_SetOverflowTH(u32Threshold);
}

//------------------------------------------------------------------------------
/// Set bitstream buffer underflow threshold
/// @return -none
//------------------------------------------------------------------------------
void MHal_MVD_SetUnderflowTH(U32 u32Threshold)
{
    MDrv_MVD_SetUnderflowTH(u32Threshold);
}

U32 MHal_MVD_SetDelay(U32 delayTime)
{
    MDrv_MVD_SetAVSync(TRUE, delayTime / 90);

    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_SetSkipRepeatMode(U8 u8Mode)
{
    MDrv_MVD_SetSkipRepeatMode(u8Mode);

    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_SetSyncThreshold(U32 u32Threshold)
{
    MDrv_MVD_SetAVSyncThreshold(u32Threshold);

    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_SetPlayMode(U32 u32Mode)
{
    MHal_MVD_SetCodecInfo(_gMVD_Codec, u32Mode, TRUE);
    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_StepPlay(void)
{
    MVD_CmdArg cmdArg;

    memset(&cmdArg, 0, sizeof(MVD_CmdArg));

    if (FALSE == HAL_MVD_MVDCommand(CMD_SINGLE_STEP, &cmdArg))
    {
        printk("%s failed\n", __FUNCTION__);
        return MVDSTATUS_FAIL;        
    }

    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_GetESDataSize(U32 *pu32DataSize)
{
    if (NULL == pu32DataSize)
    {
        return MVDSTATUS_INVALID_PARAMETERS;
    }

    *pu32DataSize = MDrv_MVD_GetResidualStreamSize();

    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_GetESRdPtr(U32 *u32Ptr)
{
    if (NULL == u32Ptr)
    {
        return MVDSTATUS_INVALID_PARAMETERS;
    }

    *u32Ptr = MDrv_MVD_GetESReadPtr();

    return MVDSTATUS_SUCCESS;
}

MVDSTATUS MHal_MVD_GetESWrPtr(U32 *u32Ptr)
{
    if (NULL == u32Ptr)
    {
        return MVDSTATUS_INVALID_PARAMETERS;
    }

    *u32Ptr = MDrv_MVD_GetESWritePtr();

    return MVDSTATUS_SUCCESS;
}

#if 0
static MVDSTATUS _MHal_MVD_WaitCommandDone(void)
{
    U32 uCommand= MVD_REG(MVD_STAT_CTRL);
    U32 u32Count= MVD_PollingCount;

    while ((!(uCommand & (MVD_CMD_RDY << 8))) && (u32Count > 0))
    {
        //Command not ready
        uCommand = MVD_REG(MVD_STAT_CTRL);
        u32Count--;
    }

    if (u32Count == 0)
    {
        MVD_Debug("[MVD] _MHal_MVD_WaitCommandDone: wait cmd rdy timeout!\n");
        return MVDSTATUS_FAIL;
    }

    return MVDSTATUS_SUCCESS;
}


static MVDSTATUS _MHal_MVD_SetCmd(u32 u32Cmd, MVD_ARG *pArg)
{
    spin_lock(&mvd_lock);
    
    if (_MHal_MVD_WaitCommandDone() != MVDSTATUS_SUCCESS) //wait cmd rdy
    {
        MVD_Debug("[MVD] _MHal_MVD_SetCmd: wait cmd rdy timeout!\n");
        goto FAILJUMPOUT;
    }

    MVD_REG(MVD_ARG1_ARG0) = ((U16) (pArg->u8Arg0)) |
                             ((U16) (pArg->u8Arg1)) <<
                             8;
    MVD_REG(MVD_ARG3_ARG2) = ((U16) (pArg->u8Arg2)) |
                             ((U16) (pArg->u8Arg3)) <<
                             8;
    MVD_REG(MVD_COMMAND) = u32Cmd;

    if (_MHal_MVD_WaitCommandDone() != MVDSTATUS_SUCCESS) //wait cmd done
    {
        MVD_Debug("[MVD] _MHal_MVD_SetCmd: wait cmd rdy timeout!\n");
        goto FAILJUMPOUT;
    }

    pArg->u8Arg0 = (U8) (MVD_REG(MVD_ARG1_ARG0));
    pArg->u8Arg1 = (U8) (MVD_REG(MVD_ARG1_ARG0) >> 8);
    pArg->u8Arg2 = (U8) (MVD_REG(MVD_ARG3_ARG2));
    pArg->u8Arg3 = (U8) (MVD_REG(MVD_ARG3_ARG2) >> 8);


    spin_unlock(&mvd_lock);

    return MVDSTATUS_SUCCESS;
    FAILJUMPOUT : spin_unlock(&mvd_lock);
    return MVDSTATUS_FAIL;
}

#ifndef _HI_HBIU_
static U32 _MHal_MVD_MemRead4Bye(U32 u32Address)
{
    U32 u32Addr;

    u32Addr = (U32) MDrv_SYS_PA2NonCacheSeg((void *) u32Address);

    return *(U32 *) (u32Addr);
}

static U16 _MHal_MVD_MemRead2Bye(U32 u32Address)
{
    U32 u32Addr;

    u32Addr = (U32) MDrv_SYS_PA2NonCacheSeg((void *) u32Address);

    return *(U16 *) (u32Addr);
}

static MVDSTATUS _MHal_MVD_MemWrite4Bye(U32 u32Address, U32 u32Value)
{
    U32 u32Addr;

    u32Addr = *((U32 *) MDrv_SYS_PA2NonCacheSeg((void *) u32Address));
    
    *(U32 *) (u32Addr) = u32Value;

    return MVDSTATUS_SUCCESS;
}

static MVDSTATUS _MHal_MVD_MemWrite2Bye(U32 u32Address, U16 u16Value)
{
    U32 u32Addr;

    u32Addr = *((U32 *) MDrv_SYS_PA2NonCacheSeg((void *) u32Address));

    *(U16 *) (u32Addr) = u16Value;

    return MVDSTATUS_SUCCESS;
}

static MVDSTATUS _MHal_MVD_MemWriteBye(U32 u32Address, U8 u8Value)
{
    U32 u32Addr;

    u32Addr = *((U32 *) MDrv_SYS_PA2NonCacheSeg((void *) u32Address));

    *(U8 *) (u32Addr) = u8Value;

    return MVDSTATUS_SUCCESS;
}

static U8 _MHal_MVD_MemReadBye(U32 u32Address)
{
    U32 u32Addr;

    u32Addr = (U32) MDrv_SYS_PA2NonCacheSeg((void *) u32Address);

    return *(U8 *) (u32Addr);
}
#else
static U32 _MHal_MVD_MemRead4Bye(U32 u32Address)
{
    U32 u32WaitCnt  = 0;
    U32 u32Value    = 0xFFFFFFFF;

    if (u32Address & 0x03)
    {
        MVD_Debug("_MHal_MVD_MemRead4Bye Addr is not 4 byte alignment!!\n");
        return 0;
    }

    SVD_REG(REG_H264_ADDR_L) = (u32Address & 0x000000FF) |
                               (u32Address & 0x0000FF00);
    SVD_REG(REG_H264_ADDR_H) = ((u32Address & 0x00FF0000) >> 16) |
                               ((u32Address & 0xFF000000) >> 16);

    SVD_REG(REG_H264_HIRW_FIRE) = H264_HIRW_READY + H264_HIRW_READ;

    // Check Command Ready
    while ((SVD_REG(REG_H264_HIRW_FIRE) & H264_HIRW_READY))
    {
        if (u32WaitCnt++ >= 10000)
        {
            MVD_Debug("_MHal_MVD_MemRead4Bye timeout \n");
            return 0;
        }
    }
    u32Value = SVD_REG(REG_H264_DATAR_H);
    u32Value <<= 16;
    u32Value += SVD_REG(REG_H264_DATAR_L);

    return u32Value;
}

static U16 _MHal_MVD_MemRead2Bye(U32 u32Address)
{
    U32 u32ReadAddr;
    U32 u32ReadValue;
    U16 u16Value;
    U8 u8Shift;

    u32ReadAddr = (u32Address >> 2) << 2;
    u8Shift = (U8) ((u32Address & 0x03) * 8);
    u32ReadValue = _MHal_MVD_MemRead4Bye(u32ReadAddr);
    /*
        MVD_Debug("===>_MHal_MVD_MemRead2Bye \n");
        MVD_Debug("===>addr = 0x%x \n", u32Address);
        MVD_Debug("===>read addr = 0x%x\n", u32ReadAddr);
        MVD_Debug("===>u32ReadValue = 0x%x\n", u32ReadValue);
    */
    /* FIXME */
    u16Value = (U16) (u32ReadValue >> u8Shift);
    if (u8Shift == 24)
    {
        u32ReadValue = _MHal_MVD_MemRead4Bye(u32ReadAddr + 4);
        u16Value = u16Value << 8 || (U16) (u32ReadValue & 0xFF);
    }

    return u16Value;
}

static U8 _MHal_MVD_MemReadBye(U32 u32Address)
{
    U32 u32ReadAddr;
    U32 u32ReadValue;
    U8 u8Value;
    U8 u8Shift;

    u32ReadAddr = (u32Address >> 2) << 2;
    u8Shift = (U8) ((u32Address & 0x03) * 8);
    u32ReadValue = _MHal_MVD_MemRead4Bye(u32ReadAddr);
    u8Value = (U8) (u32ReadValue >> u8Shift);

    /*
       MVD_Debug("->_MHal_MVD_MemReadBye \n");
       MVD_Debug("->addr = 0x%x \n", u32Address);
       MVD_Debug("->read addr = 0x%x\n", u32ReadAddr);
       MVD_Debug("->u8Shift = 0x%x\n", u8Shift);
       MVD_Debug("->u32ReadValue = 0x%x\n", u32ReadValue);
       MVD_Debug("->u8Value = 0x%x\n", u8Value);
    */

    return u8Value;
}

static MVDSTATUS _MHal_MVD_MemWrite4Bye(U32 u32Address, U32 u32Value)
{
    U32 u32WaitCnt  = 0;

    //MVD_Debug("MDrv_AVCH264_MEMWrite, address = 0x%08x, value = 0x%08x \n", u32Address, u32Value);
    SVD_REG(REG_H264_ADDR_L) = (u32Address & 0x000000FF) |
                               (u32Address & 0x0000FF00);
    SVD_REG(REG_H264_ADDR_H) = ((u32Address & 0x00FF0000) >> 16) |
                               ((u32Address & 0xFF000000) >> 16);

    SVD_REG(REG_H264_DATAW_L) = (u32Value & 0x000000FF) |
                                (u32Value & 0x0000FF00);
    SVD_REG(REG_H264_DATAW_H) = ((u32Value & 0x00FF0000) >> 16) |
                                ((u32Value & 0xFF000000) >> 16);

    // Fire
    SVD_REG(REG_H264_HIRW_FIRE) = H264_HIRW_WRITE;
    SVD_REG(REG_H264_HIRW_FIRE) |= H264_HIRW_READY;

    // Check Command Ready
    while ((SVD_REG(REG_H264_HIRW_FIRE) & H264_HIRW_READY))
    {
        if (u32WaitCnt++ >= 10000)
        {
            MVD_Debug("_MHal_MVD_MemWrite4Bye timeout \n");
            return MVDSTATUS_FAIL;
        }
    }

    return MVDSTATUS_SUCCESS;
}

static MVDSTATUS _MHal_MVD_MemWriteBye(U32 u32Address, U8 u8Value)
{
    U32 u32ReadAddr;
    U32 u32ReadValue;
    U8 u8Shift;

    u32ReadAddr = (u32Address >> 2) << 2;
    u8Shift = (U8) ((u32Address & 0x03) * 8);
    u32ReadValue = _MHal_MVD_MemRead4Bye(u32ReadAddr);
    u32ReadValue &= ~(0xFF << u8Shift);
    u32ReadValue |= u8Value << u8Shift;

    return _MHal_MVD_MemWrite4Bye(u32ReadAddr, u32ReadValue);
}

static MVDSTATUS _MHal_MVD_MemWrite2Bye(U32 u32Address, U16 u16Value)
{
    U32 u32ReadAddr = 0;
    U32 u32ReadValue= 0;
    U8 u8Shift;

    u8Shift = (U8) ((u32Address & 0x03) * 8) ;
    if (u8Shift < 24)
    {
        u32ReadAddr = (u32Address >> 2) << 2;
        u32ReadValue = _MHal_MVD_MemRead4Bye(u32ReadAddr);
        u32ReadValue &= ~(0xFFFF << u8Shift);
        u32ReadValue |= u16Value << u8Shift;
        if (_MHal_MVD_MemWrite4Bye(u32ReadAddr, u32ReadValue) !=
            MVDSTATUS_SUCCESS)
        {
            MVD_Debug("_MHal_MVD_MemWrite2Bye Fail \n");
            return MVDSTATUS_FAIL;
        }
    }
    else
    {
        if (_MHal_MVD_MemWriteBye(u32ReadAddr, (U8) (u16Value)) !=
            MVDSTATUS_SUCCESS)
        {
            MVD_Debug("_MHal_MVD_MemWrite2Bye Fail \n");
            return MVDSTATUS_FAIL;
        }
        if (_MHal_MVD_MemWriteBye(u32ReadAddr + 1, (U8) (u16Value >> 8)) !=
            MVDSTATUS_SUCCESS)
        {
            MVD_Debug("_MHal_MVD_MemWrite2Bye Fail \n");
            return MVDSTATUS_FAIL;
        }
    }
    return MVDSTATUS_SUCCESS;
}

static void _MHal_MVD_Clean_Mem(U32 u32start, U32 size)
{
    // FIXME : use bytedma or ge to clean dram.

    U32 i;
    for (i = 0; i < size / 4; i++)
    {
        _MHal_MVD_MemWrite4Bye(u32start + i, 0x0);
    }
}
#endif


void MHal_MVD_SetUserDataBuf(U32 u32addr, U32 u32size)
{
    MVD_ARG MvdArg;

    if (MHal_MVD_IsOnMIU1())
    {
        u32addr -= MVD_ADDR_OFFSET;
    }
    
    MVD_Debug("[MVD]SetUserDataBuf MIU1 Address 0x%x\n", u32addr);

    if ((u32size % 8) != 0)
    {
        MVD_Debug("MHal_MVD_SetUserDataBuf: addr is not alignment!\n");
    }
    u32addr >>= 3;

    MvdArg.u8Arg0 = (U8) (u32addr & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32addr & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32addr & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = 2; //user data buffer

    if (_MHal_MVD_SetCmd(CMD_USER_BUF_START, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_USER_BUF_START);
        return;
    }

    u32size >>= 3;

    MvdArg.u8Arg0 = (U8) (u32size & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32size & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32size & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = 2; //user data buffer

    if (_MHal_MVD_SetCmd(CMD_USER_BUF_SIZE, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_USER_BUF_SIZE);
        return;
    }
}

//------------------------------------------------------------------------------
/// Get the picture user data
/// @param  pDataBuff    \b IN/OUT: Buffer for filling the picture user data
/// @param  u32SizeOfBuff \b IN: size of pDataBuff
/// @param  pRetDataSize  \b OUT: the byte count of picture user data
/// @return MVDSTATUS_SUCCESS: success; MVDSTATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
MVDSTATUS MHal_MVD_GetPictureUserData(U8 *pDataBuff,
                                      U32 u32SizeOfBuff,
                                      U32 *pRetDataSize)
{
    U32 u32SizeOfUserData       = 0;
    U32 u32tmpSizeOfUserData    = 0;
    MVD_ARG mvdcmd;
    U32 u32AddressWritePtWord   = 0;
    U32 u32AddressWritePt       = 0;
    U32 u32AddressReadPt        = 0;
    U8 *pTmpPicUserDataPt;

    //sprintf(pDataBuff, "NOKIA-N95 >>> MOVING FORWARD");//for testing
    //u32SizeOfUserData = strlen(pDataBuff);
    //(*pRetDataSize) = u32SizeOfUserData;

    // get Picture User Data Buffer write pointer address
    mvdcmd.u8Arg0 = 0x00;//
    mvdcmd.u8Arg1 = 0;
    mvdcmd.u8Arg2 = 0;
    mvdcmd.u8Arg3 = 0;
    
    if (_MHal_MVD_SetCmd(CMD_RD_USER_WP, &mvdcmd) == FALSE)
    {
        MVD_Debug("MVD: Get picture user address failed!!\n");
        return MVDSTATUS_FAIL;
    }
    u32AddressWritePtWord = COMBU32(mvdcmd.u8Arg3,
                                    mvdcmd.u8Arg2,
                                    mvdcmd.u8Arg1,
                                    mvdcmd.u8Arg0);
    //((mvdcmd.u8Arg0&0x000000ff)|
    // ((mvdcmd.u8Arg1&0x000000ff)<<8)|
    // ((mvdcmd.u8Arg2&0x000000ff)<<16));
    // The MVD design is aligned 64-bits boundary and the address is in word (64-bit)
    //
    //gu32PictureUserDataStart              gu32PictureUserDataEnd
    // |                                    |
    // v                                    v
    // +------------------------------------+
    // |  |XXXXX|                           |
    // +------------------------------------+
    //    ^     ^
    //    |     |
    //    |     u32AddressWritePtWord
    //    |
    //    gu32PictureUserDataPrev

    u32AddressWritePt = u32AddressWritePtWord << 3;//next write pt (int 8-bit)
    (*pRetDataSize) = 0;//just init return value.

    if (0 == gu32PictureUserDataPrev)
    {
        //this behavior will drop first picture user data
        gu32PictureUserDataPrev = u32AddressWritePt;
        (*pRetDataSize) = 0;
        return MVDSTATUS_SUCCESS;
    }
    else
    {
        u32AddressReadPt = gu32PictureUserDataPrev;

        if (u32AddressReadPt == u32AddressWritePt)
        {
            //no new data
            (*pRetDataSize) = 0;
            return MVDSTATUS_SUCCESS;
        }
        if ((u32AddressWritePt < gu32PictureUserDataStart) ||
            (u32AddressWritePt >= gu32PictureUserDataEnd))
        {
            //incorrect memory range
            MVD_Debug("MVD:PIC USR DATA Write pt in Wrong Range 0x%x\n",
                      u32AddressWritePt);
            (*pRetDataSize) = 0;
            return MVDSTATUS_SUCCESS;
        }
        if (u32AddressReadPt > u32AddressWritePt) // round back to beginning
        {
            gu32PictureUserDataPrev = u32AddressWritePt;

            //need two steps copy
            u32SizeOfUserData = (gu32PictureUserDataEnd - u32AddressReadPt) +
                                (u32AddressWritePt - gu32PictureUserDataStart);
            //MVD_Debug( "MVD:PIC USR DATA from 0x%x to 0x%x Size = %d \n", u32AddressReadPt, u32AddressWritePt, u32SizeOfUserData);
            if (u32SizeOfUserData <= u32SizeOfBuff)
            {
                //map to physicall address is
                //u32AddressWritePt = u32AddressWritePt | 0xA0000000;
                //u32AddressReadPt = u32AddressReadPt | 0xA0000000;
                //copy data to return buffer
                u32tmpSizeOfUserData = (gu32PictureUserDataEnd -
                                        u32AddressReadPt);
#if 0
                if(u32tmpSizeOfUserData <= 0)
                {   //no copy
                    pTmpPicUserDataPt = pDataBuff;
                    (*pRetDataSize) = 0;
                    return MVDSTATUS_SUCCESS;
                }
#endif
                //                else{
                u32AddressReadPt = u32AddressReadPt | 0xA0000000;
                memcpy(pDataBuff,
                       (U8 *) u32AddressReadPt,
                       u32tmpSizeOfUserData);
                pTmpPicUserDataPt = pDataBuff + u32tmpSizeOfUserData;
                //                }
                u32tmpSizeOfUserData = (u32AddressWritePt -
                                        gu32PictureUserDataStart);
#if 0
                if( 0 <= u32tmpSizeOfUserData)
                {
                    //no copy
                    (*pRetDataSize) = 0;
                    return MVDSTATUS_SUCCESS;
                }
                else
                {
#endif
                u32AddressWritePt = gu32PictureUserDataStart | 0xA0000000;
                memcpy(pTmpPicUserDataPt,
                       (U8 *) u32AddressWritePt,
                       u32tmpSizeOfUserData);
                //                }
                (*pRetDataSize) = u32SizeOfUserData;
                return MVDSTATUS_SUCCESS;
            }
            else
            {
                //exceed the buffer size
                MVD_Debug("MVD:PIC USR DATA Exceed Copy buffer! [1] %d\n",
                          u32SizeOfUserData);
                (*pRetDataSize) = 0;
                return MVDSTATUS_SUCCESS;
            }
        }
        else //(u32AddressReadPt < u32AddressWritePt) Normal case
        {
            gu32PictureUserDataPrev = u32AddressWritePt;
            u32SizeOfUserData = u32AddressWritePt - u32AddressReadPt;
            //MVD_Debug( "MVD:PIC USR DATA from 0x%x to 0x%x Size = %d \n", u32AddressReadPt, u32AddressWritePt, u32SizeOfUserData);
            //map to physicall address is
            u32AddressReadPt = u32AddressReadPt | 0xA0000000;
            if (u32SizeOfUserData <= u32SizeOfBuff)
            {
                memcpy(pDataBuff, (U8 *) u32AddressReadPt, u32SizeOfUserData);
                (*pRetDataSize) = u32SizeOfUserData;
                return MVDSTATUS_SUCCESS;
            }
            else
            {
                //exceed the buffer size
                MVD_Debug("MVD:PIC USR DATA Exceed Copy buffer! [2] %d\n",
                          u32SizeOfUserData);
                (*pRetDataSize) = 0;
                return MVDSTATUS_SUCCESS;
            }
        }
    }

    return MVDSTATUS_SUCCESS;
}

U32 MHal_MVD_GetVldErrorCount(void)
{
    if (pu8MVDGetFrameInfoBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetVldErrorCount error!\n");
        return 0;
    }

    return _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                 OFFSET_VLD_ERR_COUNT);
}

void MHal_MVD_GetErrorInfo(U32 *errorCode, U32 *errorStatus)
{
    if (pu8MVDGetFrameInfoBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetErrorInfo error!\n");
        return;
    }

    *errorCode = _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                       OFFSET_ERROR_CODE);
    *errorStatus = _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                         OFFSET_ERROR_STATUS);
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
B16 MHal_MVD_GetValidStreamFlag(void)
{
    return !_MHal_MVD_MemReadBye(pu8MVDGetFrameInfoBufStart +
                                 OFFSET_INVALIDSTREAM);
}

U8 MHal_MVD_GetDecodedFrameIdx(void)
{
    if (pu8MVDGetFrameInfoBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetDecodedFrameIdx error: pu8MVDGetFrameInfoBufStart=NULL\n");
        return 0;
    }

    return _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart + OFFSET_FB_INDEX);
}

U32 MHal_MVD_GetSLQReadPtr(void)
{
    U32 readPtr = 0;

    if (pu8MVDGetFrameInfoBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetSLQReadPtr error: pu8MVDGetFrameInfoBufStart=NULL\n");
        return 0;
    }

    readPtr = _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                    OFFSET_SLQ_TBL_RPTR);

    if (readPtr == 0)//not start decode yet,MVD return 0
    {
        if (u32SLQTBLPosition == 0)
        {
            MVD_Debug("MHal_MVD_GetSLQReadPtr error: u32SLQTBLPosition=NULL\n");
            return 0;
        }
        readPtr = u32SLQTBLPosition;
    }
    else
    {
        readPtr = readPtr << 3;
    }

    return readPtr;
}

void MHal_MVD_SetSLQWritePtr(U32 writePtr)
{
    MVD_ARG MvdArg;

    if ((writePtr % 8) != 0)
    {
        MVD_Debug("MHal_MVD_SetSLQWritePtr: writePtr is not alignment!\n");
    }
    writePtr >>= 3;

    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    MvdArg.u8Arg0 = L_WORD(writePtr);
    MvdArg.u8Arg1 = H_WORD(writePtr);
    MvdArg.u8Arg2 = L_DWORD(writePtr);
    MvdArg.u8Arg3 = H_DWORD(writePtr);

    if (_MHal_MVD_SetCmd(CMD_SLQ_UPDATE_TBL_WPTR, &MvdArg) !=
        MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_SLQ_UPDATE_TBL_WPTR);
        return;
    }
}

U32 MHal_MVD_GetBitsRate(void)
{
    U32 bitsRate;

    if (pu8MVDGetVolBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetBitsRate error: pu8MVDGetVolBufStart=NULL\n");
        return FALSE;
    }

    bitsRate = _MHal_MVD_MemRead4Bye(pu8MVDGetVolBufStart + 24) * 400;

    return bitsRate;
}

//------------------------------------------------------------------------------
/// Get MVD decoded picture counter
/// @return -decoded picture counter
//------------------------------------------------------------------------------
U32 MHal_MVD_GetSkipPicCounter(void)
{
    if (pu8MVDGetFrameInfoBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetSkipPicCounter error: pu8MVDGetFrameInfoBufStart=NULL\n");
        return 0;
    }

    return _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                 OFFSET_SKIP_FRAME_COUNT);
}

void MHal_MVD_SetDIVXPatch(U8 u8MvAdjust, U8 u8IdctSel)
{
    MVD_ARG MvdArg;

    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    MvdArg.u8Arg0 = u8MvAdjust;

    if (_MHal_MVD_SetCmd(CMD_DIVX_PATCH, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_DIVX_PATCH);
        return;
    }

    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    MvdArg.u8Arg0 = u8IdctSel;

    if (_MHal_MVD_SetCmd(CMD_IDCT_SEL, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_IDCT_SEL);
        return;
    }
}

U8 MHal_MVD_GetSLQAvailableLevel(void)
{
    MVD_ARG MvdArg;

    MvdArg.u8Arg0 = 0;
    MvdArg.u8Arg1 = 0;
    MvdArg.u8Arg2 = 0;
    MvdArg.u8Arg3 = 0;

    if (_MHal_MVD_SetCmd(CMD_SLQ_AVAIL_LEVEL, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_SLQ_AVAIL_LEVEL);
        return 0xFF;
    }

    MVD_Debug("MHal_MVD_GetSLQAvailableLevel=%x\n", MvdArg.u8Arg2);

    return MvdArg.u8Arg2;
}

void MHal_MVD_SetSLQTabBufStartEnd(U32 u32start, U32 u32end)
{
    MVD_ARG MvdArg;
    U32 u32val  = u32end >> 3;
    MvdArg.u8Arg0 = (U8) (u32val & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32val & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32val & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((u32val & 0xff000000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_SLQ_TBL_BUF_END, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_SLQ_TBL_BUF_END);
        return;
    }

    u32val = (u32start) >> 3;
    MvdArg.u8Arg0 = (U8) (u32val & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32val & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32val & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((u32val & 0xff000000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_SLQ_TBL_BUF_START, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_SLQ_TBL_BUF_START);
        return;
    }
}

//------------------------------------------------------------------------------
/// Set bit stream buffer address to MVD
/// @param -u32start \b IN : start address
/// @param -u32end \b IN : end address
//------------------------------------------------------------------------------
void MHal_MVD_SetBitStreamAddr(U32 u32start, U32 u32end)
{
    MVD_ARG MvdArg;

    //u32start -= MVD_ADDR_OFFSET; // MIU1 128MB
    //u32end -= MVD_ADDR_OFFSET; // MIU1 128MB
    if (MHal_MVD_IsOnMIU1())
    {
        u32start -= MVD_MIU1_BASE_ADDR ;
        u32end -= MVD_MIU1_BASE_ADDR ;
    }

    MVD_Debug("[MVD]SetBitStreamAddr MIU1 Address Start 0x%x, End 0x%x\n",
              u32start,
              u32end);

    if ((u32start % 8) != 0)
    {
        MVD_Debug("MHal_MVD_SetBitStreamAddr: start is not alignment!\n");
    }
    u32start >>= 3;

    //Set Stream buffer start.
    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    MvdArg.u8Arg0 = (U8) (u32start & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32start & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32start & 0x00ff0000) >> 16);

    if (_MHal_MVD_SetCmd(CMD_STREAM_BUF_START, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_STREAM_BUF_START);
        return;
    }



    if ((u32end % 8) != 0)
    {
        MVD_Debug("MHal_MVD_SetBitStreamAddr: end is not alignment\n");
    }
    u32end >>= 3;

    //Set Stream buffer end.
    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    MvdArg.u8Arg0 = (U8) (u32end & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32end & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32end & 0x00ff0000) >> 16);
    if (_MHal_MVD_SetCmd(CMD_STREAM_BUF_END, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_STREAM_BUF_END);
        return;
    }
}

void MHal_MVD_SetMVBufferAddr(U32 u32addr)
{
    MVD_ARG MvdArg;

    u32addr -= MVD_ADDR_OFFSET; // MIU1 128MB

    MVD_Debug("[MVD]SetMVBufferAddr MIU1 Address 0x%x\n", u32addr);

    if ((u32addr % 2048) != 0)
    {
        MVD_Debug("MHal_MVD_SetMVBufferAddr: addr is not alignment!\n");
    }
    u32addr >>= 3;

    MvdArg.u8Arg0 = (U8) (u32addr & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32addr & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32addr & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((u32addr & 0xff000000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_MV_BUF_START, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_MV_BUF_START);
        return;
    }
}

//------------------------------------------------------------------------------
/// Set DP buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void MHal_MVD_SetDPBufferAddr(U32 u32addr)
{
    MVD_ARG MvdArg;

    u32addr -= MVD_ADDR_OFFSET; // MIU1 128MB

    MVD_Debug("[MVD]SetDPBufferAddr MIU1 Address 0x%x\n", u32addr);

    if ((u32addr % 8) != 0)
    {
        MVD_Debug("MHal_MVD_SetDPBufferAddr: addr is not alignment!\n");
    }
    u32addr >>= 3;

    MvdArg.u8Arg0 = (U8) (u32addr & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32addr & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32addr & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((u32addr & 0xff000000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_DP_BUF_START, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_DP_BUF_START);
        return;
    }
}

//------------------------------------------------------------------------------
/// Set IAP buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void MHal_MVD_SetIAPBufferAddr(U32 u32addr)
{
    MVD_ARG MvdArg;

    u32addr -= MVD_ADDR_OFFSET; // MIU1 128MB

    MVD_Debug("[MVD]SetIAPBufferAddr MIU1 Address 0x%x\n", u32addr);

    if ((u32addr % 8192) != 0)
    {
        MVD_Debug("MHal_MVD_SetIAPBufferAddr: addr is not alignment!\n");
    }
    u32addr >>= 13;

    MvdArg.u8Arg0 = (U8) (u32addr & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32addr & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32addr & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((u32addr & 0xff000000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_IAP_BUF_START, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_IAP_BUF_START);
        return;
    }
}

//------------------------------------------------------------------------------
/// Set IAP,DP,MV buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
B16 MHal_MVD_SetMPEG4ExtraBufferAddr(U32 u32addr)
{
#define MAX_MVD_MPEG4_EXTRA_BUFFER_SIZE 0x64000//400k

    U32 u32InfoBufferStart  = MemAlign(u32addr, 8192);

    MHal_MVD_SetIAPBufferAddr(u32InfoBufferStart);
    u32InfoBufferStart += 0x2800;//10k
    u32InfoBufferStart = MemAlign(u32addr, 8);

    MHal_MVD_SetDPBufferAddr(u32InfoBufferStart);
    u32InfoBufferStart += 0x40000;//256k DP buffer  : (1920/16) * (1088/16)* 4 * 8 = 255KB
    u32InfoBufferStart = MemAlign(u32addr, 2048);

    MHal_MVD_SetMVBufferAddr(u32InfoBufferStart);
    u32InfoBufferStart += 0x10000;//64k MV buffer :  ( (1920/16) * 2 +  (1920/16) * (1088/16)* 2 )*4  =~ 64kB
    u32InfoBufferStart = MemAlign(u32addr, 2048);

    if ((u32InfoBufferStart - u32addr) > MAX_MVD_MPEG4_EXTRA_BUFFER_SIZE)
    {
        MVD_Debug("Error! MAX_MVD_MPEG4_EXTRA_BUFFER_SIZE > 0x64000\n");
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
/// Set frame info buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void MHal_MVD_SetFrameInfoBufferAddr(U32 u32addr)
{
    MVD_ARG MvdArg;

    u32addr -= MVD_ADDR_OFFSET; // MIU1 128MB

    MVD_Debug("[MVD]SetFrameInfoBufferAddr MIU1 Address 0x%x\n", u32addr);

    if ((u32addr % 8) != 0)
    {
        MVD_Debug("MHal_MVD_SetFrameInfoBufferAddr: addr is not alignment!\n");
    }
    u32addr >>= 3;

    MvdArg.u8Arg0 = (U8) (u32addr & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32addr & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32addr & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((u32addr & 0xff000000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_FRAME_INFO_BUF, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_FRAME_INFO_BUF);
        return;
    }
}

//------------------------------------------------------------------------------
/// Set vol info buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void MHal_MVD_SetVolInfoBufferAddr(U32 u32addr)
{
    MVD_ARG MvdArg;

    u32addr -= MVD_ADDR_OFFSET; // MIU1 128MB

    MVD_Debug("[MVD]SetVolInfoBufferAddr MIU1 Address 0x%x\n", u32addr);

    if ((u32addr % 8) != 0)
    {
        MVD_Debug("MHal_MVD_SetVolInfoBufferAddr: addr is not alignment!\n");
    }
    u32addr >>= 3;

    MvdArg.u8Arg0 = (U8) (u32addr & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32addr & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32addr & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((u32addr & 0xff000000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_VOL_INFO_BUF, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_VOL_INFO_BUF);
        return;
    }
}

//------------------------------------------------------------------------------
/// Set header buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void MHal_MVD_SetHeaderBufferAddr(U32 u32addr)
{
    MVD_ARG MvdArg;

    u32addr -= MVD_ADDR_OFFSET; // MIU1 128MB

    MVD_Debug("[MVD]SetHeaderBufferAddr MIU1 Address 0x%x\n", u32addr);

    if ((u32addr % 8) != 0)
    {
        MVD_Debug("MHal_MVD_SetHeaderBufferAddr: addr is not alignment!\n");
    }
    u32addr >>= 3;

    MvdArg.u8Arg0 = (U8) (u32addr & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((u32addr & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((u32addr & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((u32addr & 0xff000000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_HEADER_INFO_BUF, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_HEADER_INFO_BUF);
        return;
    }
}

U32 MHal_MVD_GetSWIdx(void)
{
    //  FW_FRAME_INFO* ptempFrame;
    //  U32 xAddr;
    //  U16 SdramAddr;

    if (pu8MVDGetFrameInfoBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetSWIdx error: pu8MVDGetFrameInfoBufStart=NULL\n");
        return 0;
    }
    return _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                 OFFSET_SLQ_SW_INDEX);
}

static void _MHal_MVD_WriteDivx311Data(FW_DIVX_INFO *divxInfo)
{
    // FIXME: use hi-interfae to write data

    _MHal_MVD_MemWrite4Bye(pu8MVDSetHeaderBufStart +
                           OFFSET_DIVX_VOL_HANDLE_DONE,
                           divxInfo->vol_handle_done);
    _MHal_MVD_MemWrite4Bye(pu8MVDSetHeaderBufStart +
                           OFFSET_DIVX_VOL_HANDLE_DONE,
                           divxInfo->width);
    _MHal_MVD_MemWrite4Bye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_HEIGHT,
                           divxInfo->height);
    _MHal_MVD_MemWrite4Bye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_FRAME_COUNT,
                           divxInfo->frame_count);
    _MHal_MVD_MemWrite4Bye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_FRAME_TIME,
                           divxInfo->frame_time);
    _MHal_MVD_MemWrite2Bye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_PTS_INCR,
                           divxInfo->pts_incr);
    _MHal_MVD_MemWrite2Bye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_FRAME_RATE,
                           divxInfo->frame_rate);
    _MHal_MVD_MemWriteBye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_ASPECT_RATIO,
                          divxInfo->aspect_ratio);
    _MHal_MVD_MemWriteBye(pu8MVDSetHeaderBufStart +
                          OFFSET_DIVX_PROGRESSIVE_SEQUENCE,
                          divxInfo->progressive_sequence);
    _MHal_MVD_MemWriteBye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_MPEG1,
                          divxInfo->mpeg1);
    _MHal_MVD_MemWriteBye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_PLAY_MODE,
                          divxInfo->play_mode);
    _MHal_MVD_MemWriteBye(pu8MVDSetHeaderBufStart + OFFSET_DIVX_MPEG_FRC_MODE,
                          divxInfo->mpeg_frc_mode);
    MHal_MVD_SetHeaderBufferAddr(pu8MVDSetHeaderBufStart);
}

B16 MHal_MVD_GetVOLUpdate(void)
{
    U32 u32VolUpdate;
    if (pu8MVDGetFrameInfoBufStart == NULL)
    {
        MVD_Debug("MHal_MVD_GetVOLUpdate error: pu8MVDGetFrameInfoBufStart=NULL\n");
        return FALSE;
    }

    u32VolUpdate = _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                         OFFSET_VOL_UPDATE);
    if (u32VolUpdate != u32MVDFirstVOLUpdate)
    {
        u32MVDFirstVOLUpdate = u32VolUpdate;
    }
    
    if (u32VolUpdate)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void VPU_REG_WriteMask(U16 u16Addr, U16 u16Data, U16 u16Mask)
{
    U16 u16Tmp;
    u16Tmp = VPU_REG(u16Addr) & (~u16Mask);
    VPU_REG(u16Addr) = u16Tmp | (u16Data & u16Mask);
}

static U32 _MHal_MVD_Get_FW_Version(void)
{
    MVD_ARG MvdArg;

    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));
    if (_MHal_MVD_SetCmd(CMD_GET_FW_VERSION, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_GET_FW_VERSION);
        return 0;
    }

    return COMBU32(MvdArg.u8Arg3, MvdArg.u8Arg2, MvdArg.u8Arg1, MvdArg.u8Arg0);
}

///-----------------------------------------------------------------------------
/// config AVCH264 CPU
/// @param u32StAddr \b IN: CPU binary code base address in DRAM.
/// @param u8dlend_en \b IN: endian
///     - 1, little endian
///     - 0, big endian
///-----------------------------------------------------------------------------
static void _MHal_MVD_CPUSetting(U32 u32StAddr, U8 u8dlend_en)
{
    U32 u32SdrBase;

    //    u32StAddr -= MVD_ADDR_OFFSET; // MIU1 128MB
    if (MHal_MVD_IsOnMIU1())
    {
        u32StAddr -= MVD_MIU1_BASE_ADDR;
    }

    MVD_Debug("[MVD]CPUSetting MIU1 Address 0x%x\n", u32StAddr);

    //VD_MHEG5 has a Q Memory as a cache for firmware.
    //QMEM_BASE and QMEM_MASK are using to set the start address and size of QMEM.
    VPU_REG(VPU_REG_QMEM_MASK_L) = 0xe000;  //QMEM MASK
    VPU_REG(VPU_REG_QMEM_MASK_H) = 0xffff;  //QMEM MASK
    VPU_REG(VPU_REG_QMEM_BASE_L) = 0x0000;  //QMEM_BASE
    VPU_REG(VPU_REG_QMEM_BASE_H) = 0x2000;  //QMEM_BASE

    //SPI_BASE
    VPU_REG(VPU_REG_SPI_BASE) = 0xc000;

    //MVD_BASE
    // CPU RIU base address: 0x8008 (MVD), 0x8009 (HVD), 0x800A (RVD)
    VPU_REG(VPU_REG_REG_BASE) = VPU_REG_REG_BASE_MVD;

    //VD_MHEG5 has diff path to access instruction & data,
    //so we have to configure base addr for both.
    //Data sdr_base address, unit:8byte
    u32SdrBase = u32StAddr / 8;
    VPU_REG(VPU_REG_SDR_BASE_L) = (u32SdrBase & 0x0000FFFF);
    VPU_REG_WriteMask(VPU_REG_SDR_BASE_H,
                      (u32SdrBase >> 16) & 0x000000FF,
                      0x00FF);

    //Instruction base address, i.e. firmware address
    //Program counter will run from (insn_base + 100).
    // 26-bit h'73[9:8],h'71[15:8],h'72[15:0]
    //          e7[1:0],  e3[7:0],   e5[7:0],  e4[7:0]
    // insn_base, unit:16byte
    u32SdrBase = u32StAddr / 16;
    VPU_REG(VPU_REG_INSN_BASE_L) = (u32SdrBase & 0x0000FFFF);
    VPU_REG_WriteMask(VPU_REG_INSN_BASE_H,
                      ((u32SdrBase >> 16) & 0x000000FF) << 8,
                      0xFF00);
    VPU_REG_WriteMask(VPU_REG_INSN_ECO,
                      ((u32SdrBase >> 24) & 0x00000003) << 8,
                      0xFF00);

    VPU_REG_WriteMask(VPU_REG_INSN_ECO, 0x02, 0x00FF);
}

static MVDSTATUS _MHal_MVD_SwCPURst(void)
{
    VPU_REG_WriteMask(VPU_REG_CPU_RST_FINISH, 0x01, BIT1);
    mdelay(100);
    return MVDSTATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Wait MVD command ready or timeout
/// @return -MVD command ready or timeout
//------------------------------------------------------------------------------
static MVDSTATUS _MHal_MVD_TimeOut(void)
{
    U32 i;

    for (i = 0; i < MVD_PollingTimes; i++)
    {
        ///- wait until MVD command ready or timeout
        if ((MVD_REG(MVD_STAT_CTRL) & 0x200) == 0x200)
        {
            return MVDSTATUS_SUCCESS;
        }
    }
    MVD_Debug("MVD_TimeOut=%x\n", i);

    return MVDSTATUS_FAIL;
}

static MVDSTATUS _MHal_MVD_Check_FW_Rdy(void)
{
    U32 u32TimeOut  = 2000;

    while (FW_VERSION != _MHal_MVD_Get_FW_Version())
    {
        u32TimeOut--;

        if (u32TimeOut == 0)
        {
            //MVD_Debug("_MHal_MVD_Check_FW_Rdy time out,FW_VERSION=%x,_MDrv_Mvd_Get_FW_Version()=%x\n",MVD3_FW_VERSION,_MHal_MVD_Get_FW_Version());
            return MVDSTATUS_FAIL;
        }
    }
    //MVD_Debug("MVD FW version = %x\n", MVD3_FW_VERSION);

    return MVDSTATUS_SUCCESS;
}

// needn't call this function in T3
static MVDSTATUS _MHal_MVD_SelectChip( U8 u8ChipID )
{
    MVD_ARG MvdArg;

    printk("Cmd: 0x%x, Arg0: 0x%x\n", CMD_CHIPID, u8ChipID);

    memset((void*)&MvdArg, 0, sizeof(MVD_ARG));
    MvdArg.u8Arg0 = u8ChipID;
    if (_MHal_MVD_SetCmd(CMD_CHIPID , &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_CHIPID);
        return MVDSTATUS_FAIL;
    }

    return MVDSTATUS_SUCCESS;

}

MVDSTATUS MHal_MVD_MVDSetInternalBuffAddr(void)
{
    U32 u32Addr = 0;
    U32 u32Size = 0;
    U8 *pSLQTemp= NULL;
    MVD_ARG MvdArg;
    U32 baseAddr;

    MDrv_SYS_GetMMAP(E_SYS_MMAP_SVD, &u32Addr, &u32Size);

    u32MVD3FWCode = (MemAlign(u32Addr, 0x2000));//8k align
    MVD_Debug("set MVD3_FW_CODE_ADR=%x, Size=%x\n",
              u32MVD3FWCode,
              MVD3_FW_CODE_LEN);

    baseAddr = u32Addr;

    MvdArg.u8Arg0 = (U8) (baseAddr & 0x000000ff);
    MvdArg.u8Arg1 = (U8) ((baseAddr & 0x0000ff00) >> 8);
    MvdArg.u8Arg2 = (U8) ((baseAddr & 0x00ff0000) >> 16);
    MvdArg.u8Arg3 = (U8) ((baseAddr & 0x00ff0000) >> 24);

    if (_MHal_MVD_SetCmd(CMD_HEADER_INFO_BUF, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_HEADER_INFO_BUF);
        return MVDSTATUS_FAIL;
    }

    u32IAPBufAddr = (MemAlign(u32MVD3FWCode + MVD3_FW_CODE_LEN, 0x2000));//8k align
    MVD_Debug("set MVD3_FW_IAP_BUF_ADR=%x, Size=%x\n",
              u32IAPBufAddr,
              MVD3_FW_IAP_BUF_LEN);
    MHal_MVD_SetIAPBufferAddr(u32IAPBufAddr);

    u32DPBufAddr = (MemAlign(u32IAPBufAddr + MVD3_FW_IAP_BUF_LEN, 0x08));//8 align
    MVD_Debug("set MVD3_FW_DP_BUF_ADR=%x, Size=%x\n",
              u32DPBufAddr,
              MVD3_FW_DP_BUF_LEN);
    MHal_MVD_SetDPBufferAddr(u32DPBufAddr);

    u32MVBufAddr = (MemAlign(u32DPBufAddr + MVD3_FW_DP_BUF_LEN, 0x800));//2k align
    MVD_Debug("set u32MVBufAddr=%x, Size=%x\n",
              u32MVBufAddr,
              MVD3_FW_MV_BUF_LEN);
    MHal_MVD_SetMVBufferAddr(u32MVBufAddr);

    pu8MVDGetVolBufStart = (MemAlign(u32MVBufAddr + MVD3_FW_MV_BUF_LEN, 0x1000));//4K align for read write at same xdata window
    MVD_Debug("set MVD3_FW_VOL_INFO_BUF_ADR=%x, Size=%x\n",
              pu8MVDGetVolBufStart,
              MVD3_FW_VOL_INFO_BUF_LEN);
    _MHal_MVD_Clean_Mem(pu8MVDGetVolBufStart, MVD3_FW_VOL_INFO_BUF_LEN);
    MHal_MVD_SetVolInfoBufferAddr(pu8MVDGetVolBufStart);

    pu8MVDGetFrameInfoBufStart = (MemAlign(pu8MVDGetVolBufStart +
                                           MVD3_FW_VOL_INFO_BUF_LEN,
                                           0x1000));//4K align for read write at same xdata window
    MVD_Debug("set MVD3_FW_FRAME_INFO_BUF_ADR=%x, Size%x\n",
              pu8MVDGetFrameInfoBufStart,
              MVD3_FW_FRAME_INFO_BUF_LEN);
    _MHal_MVD_Clean_Mem(pu8MVDGetFrameInfoBufStart, MVD3_FW_FRAME_INFO_BUF_LEN);
    MHal_MVD_SetFrameInfoBufferAddr(pu8MVDGetFrameInfoBufStart);

    pu8MVDSetHeaderBufStart = (MemAlign(pu8MVDGetFrameInfoBufStart +
                                        MVD3_FW_FRAME_INFO_BUF_LEN,
                                        0x1000));//4K align for read write at same xdata window
    MVD_Debug("set MVD3_FW_DIVX_INFO_BUF_ADR=%x, Size=%x\n",
              pu8MVDSetHeaderBufStart,
              MVD3_FW_DIVX_INFO_BUF_LEN);
    _MHal_MVD_Clean_Mem(pu8MVDSetHeaderBufStart, MVD3_FW_DIVX_INFO_BUF_LEN);
    MHal_MVD_SetHeaderBufferAddr(pu8MVDSetHeaderBufStart);

    //set user data
    u32UserDataAddr = (MemAlign(pu8MVDSetHeaderBufStart +
                                MVD3_FW_DIVX_INFO_BUF_LEN,
                                0x1000));//4K align for read write at same xdata window
    MVD_Debug("set MVD3_FW_USER_DATA_BUF_ADR=%x, Size=%x\n",
              u32UserDataAddr,
              MVD3_FW_USER_DATA_BUF_LEN);
    MHal_MVD_SetUserDataBuf(u32UserDataAddr,MVD3_FW_USER_DATA_BUF_LEN);

    u32SLQTabBufAddr = (MemAlign(u32UserDataAddr + MVD3_FW_USER_DATA_BUF_LEN,
                                 0x1000));//4K align for read write at same xdata window
    MVD_Debug("set MVD3_FW_SLQ_TAB_BUF_ADR=%x, Size=%x\n",
              u32SLQTabBufAddr,
              MVD3_FW_SLQ_TAB_BUF_LEN);

    u32SLQTabTMPBufAddr = (MemAlign(u32SLQTabBufAddr +
                                    MVD3_FW_SLQ_TAB_BUF_LEN,
                                    0x1000));//4K align for read write at same xdata window
    MVD_Debug("set MVD3_FW_SLQ_TAB_TMPBUF_ADR=%x, Size=%x\n",
              u32SLQTabTMPBufAddr,
              MVD3_FW_SLQ_TAB_TMPBUF_LEN);

    //pSLQTemp = ioremap(u32SLQTabTMPBufAddr, MVD3_FW_SLQ_TAB_TMPBUF_LEN);
    pSLQTemp = (U8 *) MDrv_SYS_PA2NonCacheSeg((void *) u32SLQTabTMPBufAddr);

    memset(pSLQTemp, 0xFF, MVD3_FW_SLQ_TAB_TMPBUF_LEN);

    pSLQTemp[0] = 0x00;
    pSLQTemp[1] = 0x00;
    pSLQTemp[2] = 0x01;
    pSLQTemp[3] = 0xBE;//padding
    pSLQTemp[4] = 0xFA;
    pSLQTemp[5] = 0x00;

    u32SLQTBLPosition = u32SLQTabTMPBufAddr;

    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_FB, &u32Addr, &u32Size);
    MVD_Debug("set MVD_FRAMEBUFFER_ADR=%x\n", u32Addr);
    MHal_MVD_SetFrameBufferAddr(u32Addr);

    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_BS, &u32Addr, &u32Size);
    MVD_Debug("set MVD_BITSTREAM_ADR=%x\n", u32Addr);
    MHal_MVD_SetBitStreamAddr(u32Addr, u32Addr + u32Size);

    return MVDSTATUS_SUCCESS;
}

U32 MHal_MVD_GetPicCounter(void)
{
    if (pu8MVDGetFrameInfoBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetPicCounter error: pu8MVDGetFrameInfoBufStart=NULL\n");
        return 0;
    }
    return _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                 OFFSET_FRAME_COUNT);
}

void MHal_MVD_ReadVolInfo(FW_VOL_INFO *volInfo)
{
    volInfo->vol_info = _MHal_MVD_MemRead2Bye(pu8MVDGetVolBufStart);
    volInfo->sprite_usage = _MHal_MVD_MemRead2Bye(pu8MVDGetVolBufStart +
                                                  OFFSET_SPRITE_USAGE);
    volInfo->width = _MHal_MVD_MemRead4Bye(pu8MVDGetVolBufStart + OFFSET_WIDTH);
    volInfo->height = _MHal_MVD_MemRead4Bye(pu8MVDGetVolBufStart +
                                            OFFSET_HEIGHT);
    volInfo->pts_incr = _MHal_MVD_MemRead2Bye(pu8MVDGetVolBufStart +
                                              OFFSET_PTS_INCR);
    volInfo->frame_rate = _MHal_MVD_MemRead2Bye(pu8MVDGetVolBufStart +
                                                OFFSET_FRAME_RATE);
    volInfo->aspect_ratio = _MHal_MVD_MemReadBye(pu8MVDGetVolBufStart +
                                                 OFFSET_ASPECT_RATIO);
    volInfo->progressive_sequence = _MHal_MVD_MemReadBye(pu8MVDGetVolBufStart +
                                                         OFFSET_PROGRESSIVE_SEQUENCE);
    volInfo->mpeg1 = _MHal_MVD_MemReadBye(pu8MVDGetVolBufStart + OFFSET_MPEG1);
    volInfo->play_mode = _MHal_MVD_MemReadBye(pu8MVDGetVolBufStart +
                                              OFFSET_PLAY_MODE);
    volInfo->mpeg_frc_mode = _MHal_MVD_MemReadBye(pu8MVDGetVolBufStart +
                                                  OFFSET_MPEG_FRC_MODE);
    volInfo->low_delay = _MHal_MVD_MemReadBye(pu8MVDGetVolBufStart +
                                              OFFSET_LOW_DELAY);
    volInfo->bit_rate = _MHal_MVD_MemRead4Bye(pu8MVDGetVolBufStart +
                                              OFFSET_BIT_RATE);

#if 0
    printk("MHal_MVD_ReadVolInfo \n");
    printk("volInfo->vol_info = 0x%x \n", volInfo->vol_info);
    printk("volInfo->sprite_usage = 0x%x \n", volInfo->sprite_usage);
    printk("volInfo->width = %u \n", volInfo->width);
    printk("volInfo->height = %u \n", volInfo->height);
    printk("volInfo->pts_incr = 0x%x \n", volInfo->pts_incr);
    printk("volInfo->frame_rate = %u \n", volInfo->frame_rate);
    printk("volInfo->aspect_ratio = 0x%x \n", volInfo->aspect_ratio);
    printk("volInfo->progressive_sequence = 0x%x \n", volInfo->progressive_sequence);
    printk("volInfo->mpeg1 = 0x%x \n", volInfo->mpeg1);
    printk("volInfo->play_mode = 0x%x \n", volInfo->play_mode);
    printk("volInfo->mpeg_frc_mode = 0x%x \n", volInfo->mpeg_frc_mode);
    printk("volInfo->bit_rate = 0x%x \n", volInfo->bit_rate);
#endif
}

U8 MHal_MVD_GetPicType(void)
{
    if (pu8MVDGetFrameInfoBufStart == 0)
    {
        MVD_Debug("MHal_MVD_GetPicType error: pu8MVDGetFrameInfoBufStart=NULL\n");
        return 0;
    }

    return _MHal_MVD_MemRead4Bye(pu8MVDGetFrameInfoBufStart +
                                 OFFSET_PICTURE_TYPE);
}

//------------------------------------------------------------------------------
/// Get video frame information from MVD
/// @param -pinfo \b IN : pointer to video frame information
//------------------------------------------------------------------------------
void MHal_MVD_GetFrameInfo(MVD_FRAMEINFO *pinfo)
{
    if (pu8MVDGetVolBufStart == NULL)
    {
        MVD_Debug("MHal_MVD_GetFrameInfo error: pu8MVDGetVolBufStart=NULL\n");
        return;
    }
    MHal_MVD_ReadVolInfo(&gvolInfo);
    //MVD_Debug("vol info,vol_info=%dx,sprite_usage=%dx,pts_incr=%dx,invalidstream=%bx,\n",gvolInfo.vol_info,gvolInfo.sprite_usage,gvolInfo.pts_incr,gvolInfo.invalidstream);
    //MVD_Debug("vol info,width=%lx,height=%lx,frame_rate=%d,aspect_ratio=%bx,\n",gvolInfo.width,gvolInfo.height,gvolInfo.frame_rate,gvolInfo.aspect_ratio);
    //MVD_Debug("vol info,progressive_sequence=%bx,mpeg1=%bx,play_mode=%bx,bit_rate=%lx,\n",gvolInfo.progressive_sequence,gvolInfo.mpeg1,gvolInfo.play_mode,gvolInfo.bit_rate);

    pinfo->u16HorSize = (U16) gvolInfo.width;
    pinfo->u16VerSize = (U16) gvolInfo.height;
    pinfo->u8AspectRatio = gvolInfo.aspect_ratio;
    if (gvolInfo.mpeg1 == 1)
    {
        pinfo->u8Interlace = 0;
    }
    else
    {
        if (gvolInfo.progressive_sequence == 0)
        {
            pinfo->u8Interlace = 1;
        }
        else
        {
            pinfo->u8Interlace = 0;
        }
    }

    if (((gvolInfo.frame_rate > 8) && (pinfo->u8Interlace == 0)) ||
        ((gvolInfo.frame_rate > 5) && (pinfo->u8Interlace == 1)))
    {
        MVD_Debug("MHal_MVD_GetFrameInfo error: frame rate error!!\n");
        pinfo->u16FrameRate = 0;
    }
    else
    {
        pinfo->u16FrameRate = gvolInfo.frame_rate;
    }
#if 0
    pinfo->u8MPEG1=gvolInfo.mpeg1;
    pinfo->u16PTSInterval=gvolInfo.pts_incr;
    pinfo->u8PlayMode=gvolInfo.play_mode;
    pinfo->u8FrcMode=gvolInfo.mpeg_frc_mode;
#endif
}

#define MIU_PROTECT_EN                          (0x101200+0xC0)
#define MIU_PROTECT0_ID0                        (0x101200+0xC2)

#define REG_WI(_reg_, _bit_, _pos_)    \
        do{                          \
        REG_ADDR(_reg_) = (_bit_) ? (REG_ADDR(_reg_) | _pos_) : (REG_ADDR(_reg_) & ~(_pos_));  \
        }while(0)

#define REG_WR(_reg_, _val_)        do{ REG_ADDR(_reg_) = (_val_); }while(0)
#define REG_ADDR(addr)              (*((volatile U16*)(0xBF000000 + ((addr) << 1))))

static void _MHal_MIU_Protect(U8 u8Blockx,
                      U8   *pu8ProtectId,
                      U32  u32Start,
                      U32  u32End,
                      BOOL bSetFlag)
{
    U32 u32RegAddr;
    U16 u16Data;
    U8  u8Data;

    printk("u32Start=%08lx, u32End=%08lx \n",u32Start,u32End);

    u8Data = 1 << u8Blockx;

    u32RegAddr = MIU_PROTECT0_ID0 + (6 * u8Blockx);
    if (u8Blockx!=0)
    {
        u32RegAddr += 2;
    }

    // Disable MIU protect
    REG_WI(MIU_PROTECT_EN, 0, u8Data);

    if ( bSetFlag )
    {
        // Protect IDs
        REG_WR(u32RegAddr, ((pu8ProtectId[1]<<8) | pu8ProtectId[0]));
        u32RegAddr += 2;
        if (u8Blockx==0)
        {
            REG_WR(u32RegAddr, ((pu8ProtectId[3]<<8) | pu8ProtectId[2]));
            u32RegAddr += 2;
        }

        // Start Address
        u16Data = (U16)(u32Start >> 12);   //4k/unit; s7 initial here
        REG_WR(u32RegAddr, u16Data);

        // End Address
        u16Data = (U16)( u32End >> 12);   //4k/unit; s7 init herre
        REG_WR(u32RegAddr + 2, u16Data);

        // Enable MIU protect
        REG_WI(MIU_PROTECT_EN, 1, u8Data);
    }
}

U32 MHal_MVD_GetResidualStreamSize(void)
{
    MVD_Debug("didn't handle MHal_MVD_GetResidualStreamSize\n");
    return 0;
}

//------------------------------------------------------------------------------
/// Reset for I-frame decoding
/// @return -none
//------------------------------------------------------------------------------
void MHal_MVD_ResetIFrameDecode(void)
{
    MVD_ARG MvdArg;

    memset((void *) &MvdArg, 0, sizeof(MVD_ARG));

    if (_MHal_MVD_SetCmd(CMD_STOP, &MvdArg) != MVDSTATUS_SUCCESS)
    {
        MVD_Debug("Command: 0x%x fail!!\r\n", CMD_STOP);
        return;
    }

    MHal_MVD_Reset(_gMVD_Codec);
}

void MHal_MVD_SetVOLHandleDone(void)
{
    if (pu8MVDSetHeaderBufStart == 0)
    {
        MVD_Debug("MHal_MVD_SetVOLHandleDone error: pu8MVDSetHeaderBufStart=NULL\n");
        return;
    }

    //gdivxInfo.width=0;
    //gdivxInfo.height=0;
    //gdivxInfo.aspect_ratio=0;
    //gdivxInfo.frame_rate=0;
    //gdivxInfo.mpeg1=0;
    //gdivxInfo.pts_incr=0;
    //gdivxInfo.play_mode=0;
    //gdivxInfo.mpeg_frc_mode=0;
    //gdivxInfo.progressive_sequence=0;
    //gdivxInfo.frame_count=0;
    //gdivxInfo.frame_time=0;
    gdivxInfo.vol_handle_done = 1;
    _MHal_MVD_WriteDivx311Data(&gdivxInfo);
}

#endif

