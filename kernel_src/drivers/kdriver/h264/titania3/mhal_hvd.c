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
/// @file   mhal_hvd.c
/// @brief  HVD Driver Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/interrupt.h>
//#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/poll.h>
//#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
//#include <asm/io.h>
#include "Board.h"
#include "chip_setup.h"
#include "mhal_h264.h"
//#include "mdrv_h264.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "chip_int.h"
//#include "mhal_h264_interrupt.h"

#include "drvHVD_Common.h"
#include "drvHVD.h"
//------------------------------------------------------------------------------
// Local Veriables
//------------------------------------------------------------------------------

#define ENABLE_HVD_FILEMODE_PLAYBACK    0
#define ENABLE_HVD_STATUS_MONITOR    0

#define MVD_IFRAME_TIMEOUT      1000
#define  HVD_DRV_PROC_BUFFER    32
#define HVD_CPU_MIU_1_BASE  0x08000000UL
#define _HVD_LIB_PRINT(x)   x
#define _HVD_PRINT_ERR(format, args...)  printk( format, ##args)
#define _HVD_PRINT_INFO(format, args...)  //printk( format, ##args)
#define _HVD_PRINT_DBG(format, args...)  //printk( format, ##args)

HVD_Disp_Info LastDispInfo;//H.264 update 090812
U32 LastDecCnt=0;
BOOL gbRegHVDIRQ=FALSE;
U8 gu8VideoSkip=0;
U8 gu8VideoRepeat=0;
BOOL bgDisplayReady=FALSE;
U32 gH264InterruptFlag = 0;
U32 gu32H264NumOfData = 0;

#if HVD_ENABLE_ISR_POLL
U32 gu32H264NumOfDataLast = 0;
#endif
U32 gu32H264FrameCnt=0;
U32 gu32HVDDecErrCnt=0;
U32 gu32HVDDataErrCnt=0;
#if ENABLE_HVD_FILEMODE_PLAYBACK
U8  pDrvProcBuf[HVD_DRV_PROC_BUFFER];
#endif
BOOL _bH264_State_Init  = FALSE;
U32 u32ISR_decFramecnt=0;
U32 u32ISR_SPSReportCnt=0;
U32 u32Restart_WaitSyncReach=FALSE;

static U8 u8ISR_UserDataReadIdx = 0xFF;
static U32 u32FWCmdArg=0;
static U8 _u8DisplayReady=FALSE;
static U32 u32PreDispFrmAddr=0;
static HVD_Disp_Info DispInfo;
#if defined (__aeon__)
static U32 u32HVDRegOSBase=0xA0200000;
#else
#if defined( CHIP_T2 )
static U32 u32HVDRegOSBase=0xBF800000;
#else
static U32 u32HVDRegOSBase=0xBF200000;
#endif
#endif

#if ENABLE_HVD_STATUS_MONITOR
static U32 u32StatusMonitorTimer=0;
#endif

#if HVD_ENABLE_ISR_POLL
extern wait_queue_head_t        gHVD_wait_queue;
#endif
extern U32 u32MIU1BaseAddr;
extern U32 u32CodeVAddr;
extern U32 u32CodeAddr;
extern U32 u32CodeSize;
extern U32 u32FrmVAddr;
extern U32 u32FrmAddr;
extern U32 u32FrmSize;
extern U32 u32ESVAddr;
extern U32 u32ESAddr;
extern U32 u32ESSize;
extern U32 u32DrvProcVAddr;
extern U32 u32DrvProcAddr;
extern U32 u32DrvProcSize;
extern U8* pH264DataMem;
extern U32 *pH264DataMemSerNumb;

#if S7_TEMP_PATCH_KER
#define HVD_MACRO_STARTs    do {
#define HVD_MACRO_ENDs       } while (0)
#define HVD_RIU_BASEs        0xBF200000UL//u32HVDRegOSBase
#define READ_WORDs(_reg)             (*(volatile MS_U16*)(_reg))
#define WRITE_BYTEs(_reg, _val)      { (*((volatile MS_U8*)(_reg))) =(MS_U8)(_val); }
#define WRITE_WORDs(_reg, _val)      { (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }
#define HVD_RIU_WRITE_BYTEs(addr, val)      { WRITE_BYTEs( HVD_RIU_BASEs+(addr),val); }
#define HVD_RIU_READ_WORDs(addr)   ( READ_WORDs( HVD_RIU_BASEs + (addr) ) )
#define HVD_RIU_WRITE_WORDs(addr, val)      { WRITE_WORDs( HVD_RIU_BASEs+(addr), val); }
#define HVD_RIU_READ_BYTEs(addr)   ( READ_BYTEs( HVD_RIU_BASEs + (addr) ) )


#define _HVD_WriteByte( u32Reg, u8Val ) \
    HVD_MACRO_STARTs \
    HVD_RIU_WRITE_BYTEs(((u32Reg) << 1) - ((u32Reg) & 1), u8Val);   \
    HVD_MACRO_ENDs
#define _HVD_Read2Bytes( u32Reg )    (HVD_RIU_READ_WORDs((u32Reg)<<1))
#define _HVD_WriteByteMasks( u32Reg, u8Val, u8Msk )                                      \
    HVD_MACRO_STARTs                                                                     \
    HVD_RIU_WRITE_BYTEs( (((u32Reg) <<1) - ((u32Reg) & 1)), (HVD_RIU_READ_BYTEs((((u32Reg) <<1) - ((u32Reg) & 1))) & ~(u8Msk)) | ((u8Val) & (u8Msk)));                   \
    HVD_MACRO_ENDs

#define _HVD_WriteWordMasks( u32Reg, u16Val , u16Msk)                                               \
    HVD_MACRO_STARTs                                                                     \
    if ( ((u32Reg) & 0x01) )                                                        \
    {                                                                                           \
        _HVD_WriteByteMasks( ((u32Reg)+1) , (((u16Val) & 0xff00)>>8) , (((u16Msk)&0xff00)>>8) );                                                                          \
        _HVD_WriteByteMasks( (u32Reg) , ((u16Val) & 0x00ff) , ((u16Msk)&0x00ff) );                                                                          \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        HVD_RIU_WRITE_WORDs( ((u32Reg)<<1) ,  (((u16Val) & (u16Msk))  | (_HVD_Read2Byte( u32Reg  ) & (~( u16Msk ))))  );                                                       \
    }                                                                               \
    HVD_MACRO_ENDs

#endif


#if 0
//static wait_queue_head_t  mvd_wait;
//static struct timer_list mvd_timer;
//static U32 mvd_stop_wait =0;    // continue to wait or not
//static U32 mvd_state_change =0; // mvd_frameinfo state change or not

//static PIC_INFO _PicInfo;
//static VUI_DISP_INFO _VUIInfo;
//static AFD_Info _AFDInfo;
//static U32 u32InterruptEnable;
//static volatile U8 _u8CurrAFD;
//BOOL gbGetGirstFrame=FALSE;
//BOOL gbIsAVSync=FALSE;
//BOOL gbIsAVSyncDone=FALSE;
//static U8 _u8PrevFBIndex=0x00;
//U32 gu32Vop2H264INTCnt;
//U32 gu32H2642HK51INTCnt;
//BOOL bFBIndexErr;
//U32 _u32H264StartTime = 0;
//U32 u320x30001FC0, u320x30001FC4, u320x30001FCC, u320x30001FD0, u320x30001FD4;

//static U8 _u8CurrAFD;
//static U32 _u32CurrNalCnt;
//static U16 _u16ErrorCode;
void MHal_H264_Debug(void);

///-----------------------------------------------------------------------------
/// AVCH264 Reset
/// @return TRUE or FALSE
///     - TRUE, Success
///     - FALSE, Failed
///-----------------------------------------------------------------------------
BOOL MHal_H264_Reset(void)
{
    U32 u32Timeout = 5000;
	U32 u32FirmVer;
    U32 u32Retry=0;
    _MHal_H264_EnableInt(FALSE);
    //AVCH264DBG(if(XBYTE[VOP_CTRL0] & 0x01) printk("Error: Plz disable VOP before reset H.264! \n"););
    if(!_MHal_H264_SwCPURst())
    {
        printk("AVCH264 reset failed...\n");
        return H264STATUS_FAIL;
    }

    // Samue Huang, 20081006
    *((volatile unsigned int*)(0xBF800000 + (0x0C8C))) |= (1<<9) ;  // MVD_R miu mask
    *((volatile unsigned int*)(0xBF800000 + (0x0C50))) |= (1<<11) ;  // MVD_W miu mask
    msleep(1);

    MVD_REG(MVD_STAT_CTRL) |= BIT0; // fixme later
    msleep(2);
    while((MVD_REG(MVD_STAT_CTRL)&BIT9)==0 && u32Retry++ <1000)msleep(1);
    msleep(1);
    MVD_REG(MVD_STAT_CTRL) |= BIT4;
    u32Retry=0;
    while((MVD_REG(MVD_STAT_CTRL)&BIT4)==0 && u32Retry++ <200)
    {
        msleep(1);
        MVD_REG(MVD_STAT_CTRL) |= BIT4;
    }

    // Samue Huang, 20081006
    *((volatile unsigned int*)(0xBF800000 + (0x0C8C))) &= ~(1<<9) ;  // MVD_R miu mask
    *((volatile unsigned int*)(0xBF800000 + (0x0C50))) &= ~(1<<11) ;  // MVD_W miu mask
    msleep(1);

    printk("MVD_REG(MVD_STAT_CTRL) %x %d\n",MVD_REG(MVD_STAT_CTRL),u32Retry);
    _MHal_H264_SwRstRelse();
    _MHal_H264_CPURstRelse();

    while(u32Timeout)
    {
        if(_MHal_H264_MBoxReady(AVCH264_RISC_MBOX0))
        {
            _MHal_H264_MBoxRead(AVCH264_RISC_MBOX0, &u32FirmVer);
            //MBox0Clear()
            H264_REG(REG_H264_RISC_MBOX_CLR) |= REG_H264_RISC_MBOX0_CLR;
            break;
        }
        u32Timeout--;
    }
    if(u32Timeout <= 0)
    {
        printk("Cannot get H.264 firmware version !! \n");
        return H264STATUS_FAIL;
    }
    printk("H.264 firmware version 0x%x\n", u32FirmVer);
    //_bIsFrameRdy = FALSE;

    //DISABLE ES buffer overflow blocking
    H264_REG(REG_H264_BYPASS_STOP_MVD) |= REG_H264_BYPASS_STOP_BIT;
    //printk("addr: 0xA56 = 0x%bx\n", XBYTE[0xA56]);
    //FIXME
    //MDrv_VOP_SetMonoMode(FALSE);
    gbIsAVSyncDone=bgDisplayReady=gbIsAVSync=gbGetGirstFrame=FALSE;
    gu32H264FrameCnt=0;
    gu32H264FrameCnt=0;
    _u8PrevFBIndex=0x00;
    gu8VideoSkip=0;
    gu8VideoRepeat=0;
    _u8DisplayReady=FALSE;
    //_MHal_H264_EnableInt(FALSE);
    return H264STATUS_SUCCESS;

}

///-----------------------------------------------------------------------------
/// Mailbox from AVCH264 clear bit resest
///-----------------------------------------------------------------------------
void MHal_H264_MBox0Clear(void)
{
    H264_REG(REG_H264_RISC_MBOX_CLR) |= REG_H264_RISC_MBOX0_CLR;
    _u8DisplayReady=TRUE;
    //_MHal_H264_EnableInt(TRUE);
}
#endif

void MHal_H264_Delay_ms( unsigned long ms )
{
    mdelay(ms);
}

unsigned long MHal_H264_GetSyetemTime( void )
{
    struct timespec  ts;
    ts=current_kernel_time();
    return ts.tv_sec* 1000+ ts.tv_nsec/1000000;
}

extern void MHal_H264_Isr(void);
static void _MHal_H264_RstIsrVari(void)
{
    u32Restart_WaitSyncReach=0;
    LastDecCnt=0;
    gu32H264NumOfData = 0;
#if HVD_ENABLE_ISR_POLL
    gu32H264NumOfDataLast = 0;
#endif
    if( pH264DataMem != NULL)
    {
        memset( pH264DataMem  ,  0  ,  HVD_ISR_SHAREDATA_NUMB * HVD_ISR_SHAREDATA_ENTRY_SIZE  );
        *(pH264DataMemSerNumb)=0xffffffff;
    #if HVD_ENABLE_ISR_POLL
        *(pH264DataMemSerNumb+1)=0xffffffff;
    #endif
    }
    memset( &LastDispInfo  ,  0  ,   sizeof(HVD_Disp_Info));
}

//move out "MHal_H264_PA2VA" function  //H.264 update 090812
void _MHal_H264_RstVariables(void)
{
    u32FWCmdArg=0;
    _u8DisplayReady=FALSE;
    u32PreDispFrmAddr=0;
    gu32HVDDecErrCnt=0;
    gu32HVDDataErrCnt=0;
    bgDisplayReady=FALSE;
    gu8VideoSkip=0;
    gu8VideoRepeat=0;
    gu32H264FrameCnt=0;

    u32ISR_decFramecnt=0;
    u32ISR_SPSReportCnt=0;
    u8ISR_UserDataReadIdx=0xFF;

    memset( (void*)(&DispInfo)  , 0   , sizeof(HVD_Disp_Info) );
    memset( (void*)(&LastDispInfo)  , 0   , sizeof(HVD_Disp_Info) );//H.264 update 090812
}

///-----------------------------------------------------------------------------
/// get frame count between 2 I frames
/// @return frame count between 2 I frames
///-----------------------------------------------------------------------------
U32 MHal_H264_GetGOPCount(void)
{
    return 0;
}

///-----------------------------------------------------------------------------
/// Enable/Disable ES buffer overflow parser stop.
/// @param bEnable \b IN: Enable or Disable
///-----------------------------------------------------------------------------
U32 MHal_H264_EnableParserStop(BOOL bEnable)
{
    return 0;
}

///-----------------------------------------------------------------------------
/// get level idc,
/// @return level idc
///-----------------------------------------------------------------------------
U8 MHal_H264_GetLevelIDC(void)
{
    U8 u8LevelIDC = 0;
    u8LevelIDC = MDrv_HVD_GetData(E_HVD_GDATA_TYPE_AVC_LEVEL_IDC);
    return u8LevelIDC;
}


//------------------------------------------------------------------------------
/// get the MVD bit-stream buffer memory location
/// @param  pu32BufferStart     \b OUT: Buffer start position
/// @param  pu32SizeOfBuff      \b OUT: size of bit-stream buffer
/// @return H264STATUS_SUCCESS: success; H264STATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_GetBitStreamBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff)
{
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_BS,pu32BufferStart,pu32SizeOfBuff);

    #if 0 // HVD need not this
    // this is a temp solution, H264 FW need to fix this. Samuel 20081028
    if( *pu32SizeOfBuff>=0x200000 && *pu32SizeOfBuff<0x400000 )
        *pu32SizeOfBuff = 0x200000 ;
    else if( *pu32SizeOfBuff>=0x400000 && *pu32SizeOfBuff<0x800000 )
        *pu32SizeOfBuff = 0x400000 ;
    else if( *pu32SizeOfBuff>=0x800000 && *pu32SizeOfBuff<0x1000000 )
        *pu32SizeOfBuff = 0x800000 ;
    else
        *pu32SizeOfBuff = 0x400000 ;
    #endif
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// get the SVD memory location
/// @param  pu32BufferStart     \b OUT: SVD start position
/// @param  pu32SizeOfBuff      \b OUT: size of SVD
/// @return H264STATUS_SUCCESS: success; H264STATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_GetSVDCodeBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff)
{
    MDrv_SYS_GetMMAP(E_SYS_MMAP_SVD,pu32BufferStart,pu32SizeOfBuff);
    return H264STATUS_SUCCESS;
}
//------------------------------------------------------------------------------
/// get the MVD frame buffer memory location
/// @param  pu32BufferStart     \b OUT: Buffer start position
/// @param  pu32SizeOfBuff      \b OUT: size of frame buffer
/// @return H264STATUS_SUCCESS: success; H264STATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_GetFrameBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff)
{
    #ifdef _FIRMWARE_MVD_SD_
        MDrv_SYS_GetMMAP(E_SYS_MMAP_VD_3DCOMB,pu32BufferStart,pu32SizeOfBuff);
    #else
        MDrv_SYS_GetMMAP(E_SYS_MMAP_MVD_FB,pu32BufferStart,pu32SizeOfBuff);
    #endif
    return H264STATUS_SUCCESS;
}

void MHal_H264_AllocBuf(void)
{
    U32 tmp=0;
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MIU1_BASE, &u32MIU1BaseAddr, &tmp);
    MHal_H264_GetSVDCodeBuffer( (U32*)&u32CodeAddr , (U32*)&u32CodeSize);
    if( u32CodeVAddr ==0 )
    {
        u32CodeVAddr=(U32)MDrv_SYS_PA2NonCacheSeg((void*)u32CodeAddr);//H.264 update 090812
    }
    MHal_H264_GetFrameBuffer( (U32*)&u32FrmAddr , (U32*)&u32FrmSize);
    if( u32FrmVAddr ==0)
    {
        u32FrmVAddr=(U32)MDrv_SYS_PA2NonCacheSeg((void*)u32FrmAddr);//H.264 update 090812
    }
    MHal_H264_GetBitStreamBuffer(  (U32*)&u32ESAddr , (U32*)&u32ESSize  );
    if( u32ESVAddr ==0 )
    {
        u32ESVAddr=(U32)MDrv_SYS_PA2NonCacheSeg((void*)u32ESAddr);//H.264 update 090812
    }
    u32DrvProcVAddr=0;
    u32DrvProcAddr=0;
    u32DrvProcSize =0 ;
}

void MHal_H264_DeAllocBuf(void)
{
    if( u32CodeVAddr !=0 )
    {
        //iounmap((void*)u32CodeVAddr);
        //u32CodeVAddr=0;
    }
    if( u32FrmVAddr ==0)
    {
        //iounmap((void*)u32FrmVAddr);
        //u32FrmVAddr=0;
    }
    if( u32ESVAddr ==0 )
    {
        //iounmap((void*)u32ESVAddr);
        //u32ESVAddr=0;
    }
    u32DrvProcVAddr=0;
    u32DrvProcAddr=0;
    u32DrvProcSize =0 ;
}


H264STATUS MHal_H264_Reload(H264_INIT_PARAM* InitParam)
{
    return MHal_H264_Init(InitParam);
}

#define FRAME_RATE_COUNT 9
static U16 stFrameRateCode[FRAME_RATE_COUNT] = { 0, 23976, 24000, 25000, 29976, 30000, 50000, 59947, 60000 };

HVD_Result MHal_H264_GetDispInfo( HVD_Disp_Info *pinfo )
{
	HVD_Result ret;
	U16 i=0;
	U32 framerate=0;
	ret = MDrv_HVD_GetDispInfo(pinfo);
	framerate = pinfo->u32FrameRate;
	pinfo->u32FrameRate = stFrameRateCode[3];
	for(i = 0; i < FRAME_RATE_COUNT; i++)
	{
		if((stFrameRateCode[i]/10) == (framerate/10))
		{
			pinfo->u32FrameRate = framerate;
			break;
		}
	}
	return ret;
}

///-----------------------------------------------------------------------------
/// Fast get frame information from H264
/// @param -pinfo \b IN : pointer to video sequence information
///-----------------------------------------------------------------------------
BOOL MHal_H264_FastGetFrameInfo( AVCH264_FRAMEINFO *pinfo)
{
    MHal_H264_GetDispInfo( &DispInfo );
    pinfo->u16FrameRate = DispInfo.u32FrameRate;
    pinfo->u8AspectRatio = DispInfo.u8AspectRate;
    pinfo->u8Interlace = DispInfo.u8Interlace;
    pinfo->u16HorSize = DispInfo.u16HorSize;
    pinfo->u16VerSize = DispInfo.u16VerSize;
    pinfo->u16Pitch = DispInfo.u16Pitch;
    return TRUE;
}



///-----------------------------------------------------------------------------
/// Is AVCH264 sequence change
///-----------------------------------------------------------------------------
U32 MHal_H264_IsSeqChg(void)
{
    return MDrv_HVD_IsDispInfoChg();
}


///-----------------------------------------------------------------------------
/// Get current AFD
/// @return current AFD
///-----------------------------------------------------------------------------
void MHal_H264_GetActiveFormat(U8* active_format)
{
    *active_format = MDrv_HVD_GetActiveFormat();
}

///-----------------------------------------------------------------------------
/// Get idle counter
/// @return idle counter
///-----------------------------------------------------------------------------
U32 MHal_H264_GetIdleCnt(void)
{
    U32 u32Val;
    if( MDrv_HVD_IsIdle() )
    {
        u32Val=1000;
    }
    else
    {
        u32Val=0;
    }
    return u32Val;
}

///-----------------------------------------------------------------------------
/// Get picture counter
/// @return picture counter
///-----------------------------------------------------------------------------
U32 MHal_H264_GetPicCount(void)
{
    return MDrv_HVD_GetDecodeCnt();
}

///-----------------------------------------------------------------------------
/// Is 1st frame ready
/// @return ready or not
///-----------------------------------------------------------------------------
BOOL MHal_H264_IsFrameRdy(void)
{
    if(MDrv_HVD_Is1stFrmRdy() &&  MDrv_HVD_IsSyncReach() )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//------------------------------------------------------------------------------
/// Set watching interrupt
/// @param  u32Flag    \b IN: Flag for subscribing interrupt event
/// @return H264STATUS_SUCCESS: success; H264STATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
U16 Mhal_H264_SetIntSubscribe(U32 u32Flag)
{
    gH264InterruptFlag = u32Flag;
    if( u32Flag  )
    {
        HVD_Result eRet=E_HVD_OK;
    #if HVD_ENABLE_DECODE_TIME_ISR
        HVD_ISR_Event eISREventType = E_HVD_ISR_DEC_ONE| E_HVD_ISR_DEC_DISP_INFO_CHANGE ;
    #else
        HVD_ISR_Event eISREventType = E_HVD_ISR_DISP_ONE| E_HVD_ISR_DISP_REPEAT ;
    #endif
        _HVD_PRINT_DBG("HVD: reset ISR event:\n");
        _MHal_H264_RstIsrVari();
        if( _bH264_State_Init )
        {
            eRet=MDrv_HVD_SetISREvent( eISREventType  , (HVD_InterruptCb)MHal_H264_Isr );
        }
        if( eRet == E_HVD_OK)
        {
            if( _bH264_State_Init )
            {
                MDrv_HVD_SetEnableISR(TRUE);
            }
            return H264STATUS_SUCCESS;
        }
        else
        {
            return H264STATUS_FAIL;
        }
    }
    else
    {
        if( _bH264_State_Init )
        {
            MDrv_HVD_SetEnableISR(FALSE);
        }
        return H264STATUS_SUCCESS;
    }
}


//------------------------------------------------------------------------------
/// Set the picture user data buffer for MVD
/// @param  pDataBuff    \b IN/OUT: Buffer start position
/// @param  u32SizeOfBuff \b IN: size of pDataBuff
/// @return H264STATUS_SUCCESS: success; H264STATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_SetPictureUserDataBuffer(U8 *pDataBuff, U32 u32SizeOfBuff)
{
   //FIXME
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get get the status of the PTS-STC sync for video and audio
/// @param  u32AVSyncStatus    \b OUT: the av sync status
/// @return H264STATUS_SUCCESS: success; H264STATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_GetAvSyncStatus(u32* u32AVSyncStatus)
{
    //*u32AVSyncStatus = gbIsAVSync;
    *u32AVSyncStatus = MDrv_HVD_GetPlayMode(E_HVD_GMODE_IS_SYNC_ON);
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// MPEG Video Decoder(MVD) power on.
/// @return MVDSTATUS_SUCCESSS: Process success.
//------------------------------------------------------------------------------
H264STATUS MHal_H264_PowerOn(void)
{
    MDrv_HVD_PowerCtrl(TRUE);
    return H264STATUS_SUCCESS;
}


//------------------------------------------------------------------------------
/// MPEG Video Decoder(MVD) power off.
/// @return MVDSTATUS_SUCCESSS: Process success.
//------------------------------------------------------------------------------
H264STATUS MHal_H264_PowerOff(void)
{
    MDrv_HVD_PowerCtrl(FALSE);
    return H264STATUS_SUCCESS;
}

H264STATUS MHal_H264_Init(H264_INIT_PARAM* InitParam)
{
    U32 InitTime=0;
    HVD_Init_Settings initsetting =  {
        0,     ///< init mode flag, use HVD_INIT_* to setup HVD.
        0,                           ///< default frame rate.
        0,     ///< default frame rate base. The value of u32FrameRate /u32FrameRateBase must be frames per sec.
        0,    ///< set the min frame gap.
        E_HVD_SYNC_ATS,         ///< HVD_Sync_Type. sync type of current playback.
        0,   ///< not zero: specify the pitch. 0: use default value.
        0,   ///< not zero: specify the max decode tick. 0: use default value.
        0,   ///< TRUE: sync STC at each frame. FALSE: not sync each frame.
        TRUE,   ///< TRUE: auto free ES buffer when ES buffer is full. FALSE: not do the auto free.
        TRUE,   ///< TRUE: auto power saving. FALSE: not do the auto power saving.
        FALSE,   ///< TRUE: enable Dynamic Scaling. FALSE: disable Dynamic Scaling.
        TRUE,    ///< TRUE: enable Fast Display. FALSE: disable Fast Display.
        FALSE,   ///< TRUE: enable processing User data. FALSE: disable processing User data.
        0,  ///< HVD_TurboInitLevel. set the turbo init mode.
        0,   ///< HVD_Time_Unit_Type.set the type of input/output time unit.
        0,      ///< HVD decoder clock speed. 0: default value. non-zero: any nearist clock.
        0};


    HVD_MemCfg deHVDMemcfg = {
        E_HVD_FW_SOURCE_NONE,                  //!< the input FW source type.
        0,                  //!<  virtual address of input FW binary in DRAM
        0,                //!< the physical memory start address in Flash/DRAM memory of FW code
        0,               //!< the FW code size
        0,
        0,
        0,
        u32MIU1BaseAddr,       //!< the physical memory start address of MIU 1 base address. 0: default value.
        u32CodeVAddr,        //!< the virtual memory start address of code buffer
        u32CodeAddr,         //!< the physical memory start address of code buffer
        u32CodeSize,             //!< the code buffer size
        u32FrmVAddr,           //!< the virtual memory start address of frame buffer
        u32FrmAddr,            //!< the physical memory start address of frame buffer
        u32FrmSize,                //!< the frame buffer size
        u32ESVAddr,           //!< the virtual memory start address of bit stream buffer
        u32ESAddr,                //!< the physical memory start address of bit stream buffer
        u32ESSize,            //!< the bit stream buffer size
        u32DrvProcVAddr,       //!< the virtual memory start address of driver process buffer
        u32DrvProcAddr,       //!< the physical memory start address of driver process buffer
        u32DrvProcSize};        //!< the driver process buffer size

    if( _bH264_State_Init  == TRUE )
    {
        return H264STATUS_FAIL;
    }
    if( InitParam == NULL )
    {
        return H264STATUS_FAIL;
    }

    // AVS stream
    if( InitParam->eCodecType==E_H264_CODEC_TYPE_AVS )
    {
        initsetting.u32ModeFlag  =HVD_INIT_HW_AVS;
    }
    else
    {
        initsetting.u32ModeFlag  =HVD_INIT_HW_AVC;
    }

    if( InitParam->eInputPath == E_H264_INPUT_FILE )
    {
        initsetting.u32ModeFlag |=HVD_INIT_INPUT_DRV;
        initsetting.u32ModeFlag |=HVD_INIT_MAIN_FILE_RAW;
        deHVDMemcfg.u32DrvProcessBufAddr = u32ESAddr;
        deHVDMemcfg.u32DrvProcessBufVAddr = u32ESVAddr;
        deHVDMemcfg.u32DrvProcessBufSize = 0x8000;
    }
    else
    {
        initsetting.u32ModeFlag |=HVD_INIT_MAIN_LIVE_STREAM;
        initsetting.u32ModeFlag |=HVD_INIT_INPUT_TSP;
    }

    MDrv_HVD_SetOSRegBase((MS_U32)u32HVDRegOSBase);

    _HVD_LIB_PRINT(MDrv_HVD_SetDbgLevel(E_HVD_UART_LEVEL_DBG));

    _MHal_H264_RstVariables();
    InitTime=MHal_H264_GetSyetemTime();

    if (MDrv_HVD_Init(  &deHVDMemcfg , &initsetting )== E_HVD_OK)
    {
        HVD_Disp_Info_Threshold  DispInfoTH;
        DispInfoTH.u32FrmrateLowBound=10000;
        DispInfoTH.u32FrmrateUpBound=170000;
        DispInfoTH.u32MvopLowBound=16000000;
        DispInfoTH.u32MvopUpBound=0;
        MDrv_HVD_SetDispInfoTH(  &DispInfoTH );

        MDrv_HVD_SetErrConceal(TRUE);
        MDrv_HVD_SetSettings_Pro( E_HVD_SSET_MIU_BURST_CNT_LEVEL , 0 );

        InitTime=MHal_H264_GetSyetemTime()-InitTime;
        _bH264_State_Init  = TRUE;
//        printk("MHal_H264_Init Done: OK :%d\n",InitTime);
        _HVD_PRINT_INFO("MHal_H264_Init Done: OK :%d\n",InitTime);
        return H264STATUS_SUCCESS;
    }
    else
    {
        _HVD_PRINT_ERR("MHal_H264_Init Done: Failed\n");
        return H264STATUS_FAIL;
    }
}

//------------------------------------------------------------------------------
/// Play input video stream
/// @param frc : frame rate conversion mode.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_STREAM_ID: Invalid input stream index.
/// @return MVDSTATUS_INVALID_COMMAND: Invalid operation command.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_Play(U8 frc)
{
#if (!ENABLE_HVD_FILEMODE_PLAYBACK)
    MDrv_HVD_SetSyncActive(TRUE);
#endif
    MDrv_HVD_Play();
    MDrv_HVD_SetEnableISR(TRUE);
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Pause input video stream
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_COMMAND: Invalid operation command.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_Pause(void)
{
    MDrv_HVD_Pause();
    MDrv_HVD_SetEnableISR(FALSE);
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Stop input video stream
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_Stop(void)
{
    _MHal_H264_RstVariables();
    _bH264_State_Init  = FALSE;
    MDrv_HVD_Exit();
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Decodes one I frame. The I frame is not immediately displayed.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_Decode_IFrame(U32 u32StreamBufStart, U32 u32StreamBufEnd)
{
    U32 timer = 300;
    HVD_Packet_Info packet;
    packet.u32Staddr = u32StreamBufStart - u32ESAddr;
    packet.u32Length = u32StreamBufEnd - u32StreamBufStart;
    packet.u32TimeStamp = 0xFFFFFFFF;
    packet.u32ID_L=0;
    packet.u32ID_H=0;
    if( MDrv_HVD_StepDecode() != E_HVD_OK)
    {
        printk( "MHal_H264_Decode_IFrame: step decode failed\n" );
        return H264STATUS_FAIL;
    }
    MDrv_HVD_SetSkipDecMode(E_HVD_SKIP_DECODE_I);
    MDrv_HVD_PushQueue( &packet  );
    MDrv_HVD_PushQueue_Fire();
    while( timer )
    {
        msleep(1);
        if( MDrv_HVD_IsStepDecodeDone() )
        {
            break;
        }
        timer--;
    }
    if( timer ==0)
    {
        printk( "MHal_H264_Decode_IFrame: decode time out, not enough data\n" );
        return H264STATUS_NO_AVAIL_STREAM;
    }
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get the decoding frame count.
/// @param pu16FrameCount \b OUT: Decding frame count
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_STREAM_ID: Invalid input stream index.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_GetDecdeFrameCount(U32* pu32FrameCount)
{
#if HVD_ENABLE_DECODE_TIME_ISR
    *pu32FrameCount = MDrv_HVD_GetDecodeCnt();
#else
    *pu32FrameCount=gu32H264FrameCnt;
#endif
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Get MVD first frame decode status
/// @return -first frame decoded ready or not
//------------------------------------------------------------------------------
U8 MHal_H264_GetFirstFrame(void)
{
    return MDrv_HVD_IsFrameShowed();
}

//------------------------------------------------------------------------------
/// Get MVD first frame header decode status
///   - 0, sequence ready
///   - 1, not ready
/// @return -first frame decoded ready or not
//------------------------------------------------------------------------------
U32 MHal_H264_GetDispRdy(void)//H.264 update 090812
{
    HVD_Result eRet;
    if ( MDrv_HVD_IsDispInfoChg() || _u8DisplayReady)
    {
        //printk( "GetDispRdy() and remove blue screen:%d\n" , (U16)MDrv_HVD_CheckDispInfoRdy());
        //MDrv_HVD_SetBlueScreen(FALSE);
        return 0;
    }
    eRet =MDrv_HVD_CheckDispInfoRdy();
    if( eRet  == E_HVD_OK  )
    {
        return 0;
    }
    return 1;
}

//------------------------------------------------------------------------------
/// Get sync status to know whether sync is complete or not
//------------------------------------------------------------------------------
U8 MHal_H264_GetSyncStatus(void)
{
    return MDrv_HVD_IsSyncReach();
}


U8 MHal_H264_IPicFound(void)
{
    return MDrv_HVD_IsIFrmFound();
}

//------------------------------------------------------------------------------
/// Get the Presentation time stamp (PTS) of input stream
/// @param pu32PTSLow \b OUT: PTS (bit0 ~ bit31)
/// @param pu32PTSHigh \b OUT: PTS (bit32)
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_STREAM_ID: Invalid input stream index.
/// @internal
//------------------------------------------------------------------------------

H264STATUS MHal_H264_GetPTS(U32* pu32PTSLow, U32* pu32PTSHigh)
{
    *pu32PTSLow=MDrv_HVD_GetPTS();
    *pu32PTSHigh=0;
    return H264STATUS_SUCCESS;
}


//------------------------------------------------------------------------------
/// Get video resolution, fram rate, and aspect ratio
/// @param pPictureData \b OUT: Picture data information.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @return MVDSTATUS_INVALID_STREAM_ID: Invalid input stream index.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_GetPictureData(AVCH264_FRAMEINFO *pPictureData)
{
    MHal_H264_GetDispInfo( &DispInfo );
    pPictureData->u16HorSize = DispInfo.u16HorSize;
    pPictureData->u16VerSize = DispInfo.u16VerSize;
    pPictureData->u16FrameRate = DispInfo.u32FrameRate;
    pPictureData->u8AspectRatio = DispInfo.u8AspectRate;
    pPictureData->u8Interlace = DispInfo.u8Interlace;
    pPictureData->u16SarWidth = DispInfo.u16SarWidth;
    pPictureData->u16SarHeight = DispInfo.u16SarHeight;
    pPictureData->u16CropRight = DispInfo.u16CropRight;
    pPictureData->u16CropLeft= DispInfo.u16CropLeft;
    pPictureData->u16CropBottom = DispInfo.u16CropBottom;
    pPictureData->u16CropTop = DispInfo.u16CropTop;
    pPictureData->u16Pitch = DispInfo.u16Pitch;

    if(DispInfo.bChroma_idc_Mono)
    {
        //FIXME
        //MONO
        //MDrv_VOP_SetMonoMode(TRUE);
    }
    //FIXME: temporary solution
    if(pPictureData->u16FrameRate < 20000 || pPictureData->u16FrameRate > 60000)
    {
        ;
        //printk("Assign dummy frame rate 25000: %x\n" , pPictureData->u16FrameRate);
        //pPictureData->u16FrameRate = 25000;
    }
    return H264STATUS_SUCCESS;
}


//------------------------------------------------------------------------------
/// Get video progressive or interlace
/// @return -video progressive or interlace
//------------------------------------------------------------------------------
U8 MHal_H264_GetProgInt (void)
{
    return (DispInfo.u8Interlace)?1:0;//H264STATUS_SUCCESS;//MvdArg.u8Arg2; //H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Play input video stream
/// @param u32StartAddr : input address
/// @param u32EndAddr : input length
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_SetFilePlay(U32 u32StartAddr, U32 u32EndAddr)
{
    HVD_Packet_Info packet;
    packet.u32Staddr = u32StartAddr - u32ESAddr;
    packet.u32Length = u32EndAddr - u32StartAddr;
    packet.u32TimeStamp = 0xFFFFFFFF;
    packet.u32ID_L=0;
    packet.u32ID_H=0;
    if( MDrv_HVD_Play() != E_HVD_OK)
    {
        printk( "MHal_H264_Decode_IFrame: step decode failed\n" );
        return H264STATUS_FAIL;
    }
    MDrv_HVD_PushQueue( &packet  );
    MDrv_HVD_PushQueue_Fire();
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// Close input video stream
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_SetFileClose(void)
{
    #if 1
    _MHal_H264_RstVariables();
    MDrv_HVD_Exit();
    _bH264_State_Init  = FALSE;
    #endif
    return H264STATUS_SUCCESS;
}


//------------------------------------------------------------------------------
/// Play input video stream
/// @param u8PlayFrameRate : frame rate conversion mode.
/// @param u8Mode : for file mode
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_SetFilePlay2(U8 u8PlayFrameRate, U8  u8Mode)
{
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// fast forward/backward/slow motion play
/// @param PlayMode \b IN: fast forward/backward/slow motion play mode
/// @param u8FrameRateUnit \b IN: frame display duration (frame rate unit)
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_PVRPlayMode(H264_PVR_PlayMode PlayMode, U8  u8FrameRateUnit)
{
    switch(  PlayMode )//H.264 update 090812
    {
    case E_H264_PVR_NORMAL:
        MDrv_HVD_SetSkipDecMode( E_HVD_SKIP_DECODE_ALL );
        MDrv_HVD_SetDispSpeed(  E_HVD_DISP_SPEED_NORMAL_1X );
        MHal_H264_ToggleAVSync(  TRUE );
        break;

    case E_H264_PVR_DECODE_I:
        MHal_H264_ToggleAVSync(  FALSE );
        MDrv_HVD_SetSkipDecMode( E_HVD_SKIP_DECODE_I );
        break;
    case E_H264_PVR_DECODE_IP:
        MDrv_HVD_SetSkipDecMode( E_HVD_SKIP_DECODE_ALL );
        MDrv_HVD_SetDispSpeed(  E_HVD_DISP_SPEED_FF_2X );
        MHal_H264_ToggleAVSync(  TRUE );
        break;
    case E_H264_PVR_SLOW_MOTION:
        MDrv_HVD_SetSkipDecMode( E_HVD_SKIP_DECODE_ALL );
        {
            S32 s32SlowMotionSpeed = 0;
            s32SlowMotionSpeed = (-1)* (S32)u8FrameRateUnit;
            MDrv_HVD_SetDispSpeed(  (HVD_Drv_Disp_Speed)s32SlowMotionSpeed);
        }
        MHal_H264_ToggleAVSync(  TRUE );
        break;
    default:
        break;
    }

    //set play command
    MDrv_HVD_Play();

    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// It is necessary to invoke MDrv_Mvd_SetVOPDone before MDrv_Mvd_GetSyncStatus().
//------------------------------------------------------------------------------



U8 MHal_H264_SetVOPDone(void)
{
    MDrv_HVD_SetBlueScreen(FALSE);
    _u8DisplayReady=TRUE;
#if 0//S7_TEMP_PATCH_KER
        _HVD_WriteWordMasks((0x2f00|(( 0x0000)<<1)),  0x12 , 0xFFFF   );
        _HVD_WriteWordMasks((0x2f00|(( 0x0001)<<1)),  BIT(7) , BIT(7)   );
#endif
    return TRUE;
}


//------------------------------------------------------------------------------
/// change the AV sync state
/// @param u8Flag \b IN: 1 for enabling AV sync, 0 for disabling AV sync.
/// @return MVDSTATUS_SUCCESSS: Process success.
/// @return H264STATUS_FAIL: Process fail.
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_ToggleAVSync(u8 u8Flag)
{
    if( u8Flag )
    {
        MDrv_HVD_SetSyncActive(TRUE);
    }
    else
    {
        MDrv_HVD_SetSyncActive(FALSE);
    }
    return H264STATUS_SUCCESS;
}


//------------------------------------------------------------------------------
/// Get number of video SKIP
/// @return - number of video SKIP ready or not
//------------------------------------------------------------------------------
U8 MHal_H264_GetVideoSkip(void)
{
    return MDrv_HVD_GetData( E_HVD_GDATA_TYPE_SKIP_CNT );
}

//------------------------------------------------------------------------------
/// Get number of video REPEAT
/// @return - number of video REPEAT ready or not
//------------------------------------------------------------------------------
U8 MHal_H264_GetVideoRepeat(void)
{
    return gu8VideoRepeat;
}

void MHal_H264_GetNextFrame(void)
{
    U32 u32DispFrmLumaAddr=0;
    HVD_Frame_Info FrmInfo;
    MDrv_HVD_GetFrmInfo( E_HVD_GFRMINFO_DISPLAY , &FrmInfo);
    u32DispFrmLumaAddr = FrmInfo.u32LumaAddr;

    if(u32PreDispFrmAddr != u32DispFrmLumaAddr)
    {
        gu32H264FrameCnt++;
        u32PreDispFrmAddr = u32DispFrmLumaAddr;
    }
}

U32 MHal_H264_RegisterInterrupt(void)
{
    HVD_Result eRet=E_HVD_RET_ILLEGAL_ACCESS;
#if HVD_ENABLE_DECODE_TIME_ISR
    HVD_ISR_Event eISREventType = E_HVD_ISR_DEC_ONE| E_HVD_ISR_DEC_DISP_INFO_CHANGE ;
#else
    HVD_ISR_Event eISREventType = E_HVD_ISR_DISP_ONE| E_HVD_ISR_DISP_REPEAT ;
#endif
    gbRegHVDIRQ = TRUE;
    gu32H264NumOfData = 0;
    _HVD_PRINT_DBG("HVD: reg ISR event:\n");
    eRet=MDrv_HVD_SetISREvent( eISREventType  , (HVD_InterruptCb)MHal_H264_Isr );
    if( eRet == E_HVD_OK)
    {
        MDrv_HVD_SetEnableISR(TRUE);
        return 0;
    }
    else
    {
        return 1;
    }
}

U32 MHal_H264_DeRegisterInterrupt(void)
{
    _HVD_PRINT_DBG("HVD: de-reg ISR event:\n");
    MDrv_HVD_SetISREvent( E_HVD_ISR_NONE  , (HVD_InterruptCb)MHal_H264_Isr );
    gbRegHVDIRQ=FALSE;
    _MHal_H264_RstIsrVari();
#if HVD_ENABLE_ISR_POLL
    wake_up(&gHVD_wait_queue);
#endif
    return 0;
}

BOOL MHal_H264_SetCMD(U32 u32Cmd)
{
    if( MDrv_HVD_SetCmd_Dbg(u32Cmd , u32FWCmdArg ) == E_HVD_OK)
    {
        u32FWCmdArg = 0;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL  MHal_H264_SetCMDArg(U32 u32Arg)
{
    u32FWCmdArg = u32Arg;
    return TRUE;
}


H264STATUS MHal_H264_GetVUIInfo(VUI_DISP_INFO *vuiInfo)
{
    if(  vuiInfo != NULL)
    {
        U32 source= MDrv_HVD_GetData( E_HVD_GDATA_TYPE_AVC_VUI_DISP_INFO );
        memcpy( (void*)vuiInfo  ,  (void*)source   , sizeof(VUI_DISP_INFO) );
        return H264STATUS_SUCCESS;
    }
    else
    {
        return H264STATUS_FAIL;
    }
}

H264STATUS MHal_H264_SetDelay(U32 delayTime)
{
    if( MDrv_HVD_SetSyncVideoDelay(delayTime) == E_HVD_OK)
    {
        return H264STATUS_SUCCESS;
    }
    else
    {
        return H264STATUS_FAIL;
    }
}

H264STATUS MHal_H264_SetDecodingSpeed(U32 Speed)
{
    if( MDrv_HVD_SetDispSpeed( Speed ) == E_HVD_OK )
    {
        return H264STATUS_SUCCESS;
    }
    else
    {
        return H264STATUS_FAIL;
    }
}

U8 MHal_H264_GetPictType(void)
{
    HVD_Frame_Info FrmInfo;
    //if( MDrv_HVD_GetFrmInfo( E_HVD_GFRMINFO_DISPLAY  , &FrmInfo ) ==  E_HVD_OK)
    if( MDrv_HVD_GetFrmInfo( E_HVD_GFRMINFO_DECODE  , &FrmInfo ) ==  E_HVD_OK)
    {  //picture type: I==0, P==1, B==2 frame
        return (U8)((FrmInfo.eFrmType)+1);//H.264 update 090812
    }
    else
    {
        printk( "HVD MHal_H264_GetPictType: err:%d\n" , FrmInfo.eFrmType  );
        return (U8)(-1);
    }
}

H264STATUS MHal_H264_StepDisplay(void)
{
    if( MDrv_HVD_StepDisp() == E_HVD_OK )
    {
        return H264STATUS_SUCCESS;
    }
    else
    {
        return H264STATUS_FAIL;
    }
}

U32 MHal_H264_GetDispQueueSize(void)
{
    U32 u32Ret = (U32)MDrv_HVD_GetData(E_HVD_GDATA_TYPE_DISP_Q_SIZE);
    return u32Ret;
}

U32 MHal_H264_GetESDataSize(void)
{
    U32 u32ReadPtr=0, u32WritePtr=0;
    U32 u32DataSize=0;
    u32ReadPtr = MDrv_HVD_GetESReadPtr();
    u32WritePtr = MDrv_HVD_GetESWritePtr();
    if(u32WritePtr >= u32ReadPtr)
    {
        u32DataSize = u32WritePtr - u32ReadPtr;
    }
    else
    {
        u32DataSize = u32ESSize - u32ReadPtr - u32WritePtr;
    }
    return u32DataSize;
}

H264STATUS MHal_H264_SetSyncRepeatTH(U32 Times)
{
    if( MDrv_HVD_SetSyncRepeatTH(Times) == E_HVD_OK)
    {
        return H264STATUS_SUCCESS;
    }
    else
    {
        return H264STATUS_FAIL;
    }
}

H264STATUS MHal_H264_SetSyncThreshold(U32 threshold)
{
    if( MDrv_HVD_SetSyncTolerance(threshold) == E_HVD_OK)
    {
        return H264STATUS_SUCCESS;
    }
    else
    {
        return H264STATUS_FAIL;
    }
}


void HVD_Dump_DispFrmInfo(void)
{
    HVD_Frame_Info FrmInfo;
    MDrv_HVD_GetFrmInfo( E_HVD_GFRMINFO_DISPLAY , &FrmInfo);
    printk( "HVD DispFraInfo: Luma:%x Cham:%x %d %d (%d %d %d)\n" ,
        (U32)FrmInfo.u32LumaAddr , (U32)FrmInfo.u32ChromaAddr ,
         (U32)FrmInfo.u32TimeStamp , (U32)FrmInfo.eFrmType ,
         (U32)FrmInfo.u16Width , (U32)FrmInfo.u16Height ,   (U32)FrmInfo.u16Pitch);
}

BOOL MHal_H264_CheckUserDataAvailable(void)
{
    if( u8ISR_UserDataReadIdx != (U8)MDrv_HVD_GetUserData_Wptr() )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

U32 MHal_H264_GetUserData( pH264_Data_Pkt *ppStrLast, U32 *pu32NumOfData )
{
    U32 u32userDataPacketAddr = 0;
    U32 u32userDataPacketSize = 0;
    pH264_Data_Pkt pH264_Data_PktIs = NULL;
    //U8 u8UserdataOffset = 0;
    U8 u8UserDataBufSize=0;

    // default: 12 is the range of index of userdata ring buffer index
    u8UserDataBufSize=MDrv_HVD_GetData(E_HVD_GDATA_TYPE_USERDATA_IDX_TBL_SIZE);
    // default: 256
    u32userDataPacketSize=MDrv_HVD_GetData(E_HVD_GDATA_TYPE_USERDATA_PACKET_SIZE);

    for ( ; MHal_H264_CheckUserDataAvailable()  ; u8ISR_UserDataReadIdx = (u8ISR_UserDataReadIdx + 1) % u8UserDataBufSize)
    {
        u32userDataPacketAddr= (U32)MDrv_HVD_GetUserData_Packet(  (MS_U32)u8ISR_UserDataReadIdx  , (MS_U32*)(&u32userDataPacketSize)   );
        if(  (u32userDataPacketSize != 0) &&  ( u32userDataPacketAddr != 0) )
        {
            u32userDataPacketAddr+=u32CodeVAddr;
            pH264_Data_PktIs = (pH264_Data_Pkt)*ppStrLast;
            pH264_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pH264_Data_PktIs->u32TypeOfStruct  = 2;
            pH264_Data_PktIs->u32NumOfDataPkt  = (*pu32NumOfData)++;
            memcpy(&pH264_Data_PktIs->pPktData, (U8*)u32userDataPacketAddr, u32userDataPacketSize);
#if 0
            U8 *i = (U8 *)(&pH264_Data_PktIs->pPktData);
            printk("%02d_%02d_%x__", u8UserDataWriteIdx, u8UserdataOffset, i[0]);
            printk("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
                    i[16], i[17], i[18], i[19],
                    i[20], i[21], i[22], i[23], i[24], i[25], i[26], i[27], i[28], i[29],
                    i[30], i[31], i[32], i[33], i[34], i[35], i[36], i[37], i[38], i[39]);
#endif
            *pH264DataMemSerNumb = (*pu32NumOfData);
            if((*pu32NumOfData) % HVD_ISR_SHAREDATA_NUMB == 0)
            {
                *((U8 **)ppStrLast) = pH264DataMem;
            }
            else
            {
                *((U8 **)ppStrLast) += PKT_SIZE;
            }

        }
    }

    return H264STATUS_SUCCESS;

}

//------------------------------------------------------------------------------
/// get the decoder status
/// @param  status     \b OUT: status
/// @return H264STATUS_SUCCESS: success; H264STATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_GetDecoderStatus(H264_Decoder_Status *status)
{
    HVD_DrvStatus  statusin;
    MDrv_HVD_GetStatus(&statusin);
    status->bInit =statusin.bInit;
    status->bIdle=MDrv_HVD_IsIdle();

    if( MDrv_HVD_IsAlive() == E_HVD_OK )
    {
        status->bAlive=TRUE;
    }
    else
    {
        status->bAlive=FALSE;
    }
#if ENABLE_HVD_STATUS_MONITOR
    if(  (MHal_H264_GetSyetemTime()-u32StatusMonitorTimer) > 2000   )
    {
        printk( "DecoderStatus: alive:%d ESR:%x PC:%x\n"  , (U32)status->bAlive ,  (U32)MDrv_HVD_GetESReadPtr() ,(U32)MDrv_HVD_GetMem_Dbg(2) );
        u32StatusMonitorTimer= MHal_H264_GetSyetemTime();
    }
#endif
    return H264STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
/// restart decoder
/// @return H264STATUS_SUCCESS: success; H264STATUS_FAIL: fail
/// @internal
//------------------------------------------------------------------------------
H264STATUS MHal_H264_RestartDecoder(void)
{
    //printk( " MHal_H264_RestartDecoder start\n");
    if( MDrv_HVD_Rst(FALSE) == E_HVD_OK )
    {
        u32Restart_WaitSyncReach=TRUE;
        return H264STATUS_SUCCESS;
    }
    else
    {
        return H264STATUS_FAIL;
    }
}



