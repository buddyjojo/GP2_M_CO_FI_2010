
#define _MDRV_BDMA_C_

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/stat.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include <linux/delay.h>

#include "chip_setup.h"
#include "mst_devid.h"
#include "mdrv_types.h"
#include "mdrv_system.h"

#include "mdrv_bdma.h"
#include "mdrv_bdma_io.h"
#include "mdrv_bdma_st.h"
#include "mhal_bdma.h"

#include "mdrv_probe.h"

//----------------------------------------------------------------------
#define OPT_BDMA_DRV_DEBUG
#undef BDMA_DRV_DBG
#ifdef OPT_BDMA_DRV_DEBUG
    #define BDMA_DRV_DBG(fmt, args...)      printk(KERN_WARNING "[BDMA_DRV][%05d]" fmt, __LINE__, ## args)
#else
    #define BDMA_DRV_DBG(fmt, args...)
#endif

#undef BDMA_DRV_DBGX
#define BDMA_DRV_DBGX(fmt, args...)
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//#define MDRV_NAME_BDMA                  "bdma"
//#define MDRV_MAJOR_BDMA                 0xff
//#define MDRV_MINOR_BDMA                 0x00
#define BDMA_DEVICE_COUNT    1
//----------------------------------------------------------------------

static U32 _u32BdmaMajor = MDRV_MAJOR_BDMA;
static U32 _u32BdmaMinor = MDRV_MINOR_BDMA;

static struct cdev _stBdmaDev;


////////////////////////////////////////////////////////////////////////////////
// Local defines & local structures
////////////////////////////////////////////////////////////////////////////////
#define BDMA_WAIT_TIME              (10000) //millisecs
#define BDMA_IS_TIMEOUT(x)          ((x) ? FALSE : TRUE)

#define BDMA_DFLUSH(base, size)     //MsOS_Dcache_Flush(MS_PA2KSEG0(base), size)


typedef void (*BDMA_ISR_CBF)(BDMA_RESULT eRet);

typedef struct _BDMA_OP_CB  //control block
{
    U8   u8OpCfg;
    U8   u8SrcDevCfg;
    U8   u8DstDevCfg;
    U8   u8DmyWrCnt;
    U32  u32SrcAddr;
    U32  u32DstAddr;
    U32  u32DataSize;
    U32  u32Cmd0;
    U32  u32Cmd1;
    BDMA_ACT_t        eAct;
    BDMA_ISR_CBF    pCbf;
}BDMA_OP_CB;

////////////////////////////////////////////////////////////////////////////////
// Local & Global Variables
////////////////////////////////////////////////////////////////////////////////
static U32 _gu32Miu1Base = 0;
static B16  _gbInitBDMA = FALSE;
static BDMA_HWINFO_t _gsHwInfo;

//ISR callback function for channel 0 & channel 1
static BDMA_ISR_CBF pIsrCbf[E_BDMA_CH_END];

////////////////////////////////////////////////////////////////////////////////
// Local Function
////////////////////////////////////////////////////////////////////////////////
static B16 _MDrv_BDMA_Is_Init(void);
//static B16 _MDrv_BDMA_Is_Done_Int(U8 u8Ch);
static B16 _MDrv_BDMA_Get_FreeCh(U8 *pu8Ch);
static BDMA_RESULT _MDrv_BDMA_Start(U8 u8Ch);
static void _MDrv_BDMA_CopyFlush(BDMA_DEVICE_t eDev, U32 u32Base, U32 u32Size);
static BDMA_RESULT _MDrv_BDMA_WaitDone(U8 u8Ch, BDMA_ACT_t eAct);
static BDMA_RESULT _MDrv_BDMA_CmnHnd(U8 *pu8Ch, BDMA_OP_CB sOpCB);
static BDMA_RESULT _MDrv_BDMA_Check_Device(BDMA_SRCDEV_t eSrc, BDMA_DSTDEV_t eDst);
static U8 _MDrv_BDMA_GetDevCfg(BDMA_DEVICE_t eDev);
static B16 _MDrv_BDMA_GetHwInfo(BDMA_HWINFO_t *pstHwInfo);
static BDMA_DEVICE_t _MDrv_BDMA_Check_MIUDev(BDMA_DEVICE_t eDev, U32 u32Addr);
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_Is_Init
/// @brief \b Function  \b Description: check if BDMA is initial
/// @param <IN>         \b None:
/// @param <OUT>        \b None:
/// @param <RET>        \b TRUE: Initial FALSE: Not initial
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static B16 _MDrv_BDMA_Is_Init(void)
{
    return _gbInitBDMA;
}
#if 0
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_Is_Int
/// @brief \b Function  \b Description: Check if interrupted for BDMA done
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Interrupted FALSE: No interrupt
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static B16 _MDrv_BDMA_Is_Done_Int(U8 u8Ch)
{
    return MHal_BDMA_Is_Int(u8Ch);
}
#endif
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_Get_FreeCh
/// @brief \b Function  \b Description: get free channel for BDMA action
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static B16 _MDrv_BDMA_Get_FreeCh(U8 *pu8Ch)
{
    U32 u32Delay = BDMA_WAIT_TIME;

    if (!pu8Ch)
        return FALSE;

    *pu8Ch = E_BDMA_CH_START;

    do
    {
        if (!MHal_BDMA_Is_Queued(*pu8Ch))
            break;

        if(BDMA_IS_TIMEOUT(u32Delay))
        {
            *pu8Ch = 0xFF;
            return FALSE;
        }

        if (++*pu8Ch == E_BDMA_CH_END)
            *pu8Ch = E_BDMA_CH_START;
        msleep(1);
        u32Delay--;
    }while(1);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_Start
/// @brief \b Function  \b Description: Start BDMA action for channel 0/1
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static BDMA_RESULT _MDrv_BDMA_Start(U8 u8Ch)
{
    if (MHal_BDMA_Is_Queued(u8Ch) || MHal_BDMA_Is_Busy(u8Ch))
    {
        return E_BDMA_FAIL;
    }

    MHal_BDMA_TrigOn(u8Ch);

    return E_BDMA_OK;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_CopyFlush
/// @brief \b Function  \b Description: flush dcache
/// @param <IN>         \b eDev: selected device
/// @param <IN>         \b u32Base: address
/// @param <IN>         \b u32Size: len
/// @param <OUT>        \b None :
/// @param <RET>        \b None :
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static void _MDrv_BDMA_CopyFlush(BDMA_DEVICE_t eDev, U32 u32Base, U32 u32Size)
{
    if (E_BDMA_DEV_MIU1 == eDev)
        BDMA_DFLUSH((u32Base|_gu32Miu1Base), u32Size);
    else if (E_BDMA_DEV_MIU0 == eDev)
        BDMA_DFLUSH(u32Base, u32Size);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_WaitDone
/// @brief \b Function  \b Description: wait done for bdma action
/// @param <IN>         \b eAct: action
/// @param <IN>         \b u8Ch: channel
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static BDMA_RESULT _MDrv_BDMA_WaitDone(U8 u8Ch, BDMA_ACT_t eAct)
{
    U32 u32Delay = BDMA_WAIT_TIME;

    if (E_BDMA_ACT_MAX <= eAct)
    {
        return E_BDMA_FAIL;
    }

    do
    {
        if (MHal_BDMA_Is_Done(u8Ch, eAct))
        {
            return E_BDMA_OK;
        }
        msleep(1);
        u32Delay--;
    }while(!BDMA_IS_TIMEOUT(u32Delay));

    return E_BDMA_TIMEOUT;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_CmnHnd
/// @brief \b Function  \b Description: Handle for common actions
/// @param <IN>         \b sOpCB : control block
/// @param <OUT>        \b u8Ch : action channel
/// @param <RET>        \b TRUE : Success FALSE: Fail
/// @param <GLOBAL>     \b sOpCB : global control block
////////////////////////////////////////////////////////////////////////////////
static BDMA_RESULT _MDrv_BDMA_CmnHnd(U8 *pu8Ch, BDMA_OP_CB sOpCB)
{
    if (!pu8Ch || !_MDrv_BDMA_Get_FreeCh(pu8Ch))
        return E_BDMA_FAIL;

    //clear status first
    MDrv_BDMA_Stop(*pu8Ch);
    MHal_BDMA_Clear_Status(*pu8Ch);

    //Set start address
    MHal_BDMA_SetSrcAddr(*pu8Ch, sOpCB.u32SrcAddr);
    MHal_BDMA_SetDstAddr(*pu8Ch, sOpCB.u32DstAddr);
    //Set size
    MHal_BDMA_SetLen(*pu8Ch, sOpCB.u32DataSize);

    MHal_BDMA_SetSrcDev(*pu8Ch, sOpCB.u8SrcDevCfg);
    MHal_BDMA_SetDstDev(*pu8Ch, sOpCB.u8DstDevCfg);
    MHal_BDMA_SetCmd0(*pu8Ch, sOpCB.u32Cmd0);
    MHal_BDMA_SetCmd1(*pu8Ch, sOpCB.u32Cmd1);
    //Set INT
    MHal_BDMA_Enable_INT(*pu8Ch, (sOpCB.pCbf ? TRUE:FALSE));
    // Set address direction
    MHal_BDMA_Set_Addr_Dec(*pu8Ch, sOpCB.u8OpCfg & BDMA_OPCFG_INV_COPY);
    // Set crc reflection
    MHal_BDMA_Set_CRC_Reflect(*pu8Ch, sOpCB.u8OpCfg & BDMA_OPCFG_CRC_REFLECT);
    MHal_BDMA_SetDmyWrCnt(*pu8Ch, sOpCB.u8DmyWrCnt);

    if (_MDrv_BDMA_Start(*pu8Ch))
    {
        if (sOpCB.pCbf)
        {
            pIsrCbf[*pu8Ch] = sOpCB.pCbf;
            return E_BDMA_OK;
        }
        return _MDrv_BDMA_WaitDone(*pu8Ch, sOpCB.eAct);
    }
    else
    {
        return E_BDMA_FAIL;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_Check_Device
/// @brief \b Function  \b Description: Check if support selected device
/// @param <IN>         \b eSrc: source device
/// @param <IN>         \b eDst: destination device
/// @param <OUT>        \b None :
/// @param <RET>        \b BDMA_RESULT
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT _MDrv_BDMA_Check_Device(BDMA_SRCDEV_t eSrc, BDMA_DSTDEV_t eDst)
{
    switch(eSrc)
    {
        case E_BDMA_SRCDEV_MIU0:
        case E_BDMA_SRCDEV_MIU1:
        case E_BDMA_SRCDEV_FLASH:
        case E_BDMA_SRCDEV_MEM_FILL:
            break;

        default:
            return E_BDMA_FAIL;
    }
    if (E_BDMA_DSTDEV_NOT_SUPPORT <= eDst)
    {
        return E_BDMA_FAIL;
    }

    if ((E_BDMA_DEV_MIU1 == eSrc || E_BDMA_DEV_MIU1 == eDst) && !_gsHwInfo.bEnMIU1)
    {
        return E_BDMA_FAIL;
    }

    if ((!_gsHwInfo.bEnDSP && E_BDMA_DSTDEV_DSP == eDst)
        || (!_gsHwInfo.bEnTSP && E_BDMA_DSTDEV_TSP == eDst)
        || (!_gsHwInfo.bEnHK51_1KSRAM && E_BDMA_DSTDEV_HK51_1KSRAM == eDst))
    {
        return E_BDMA_NOT_SUPPORT;
    }

    return E_BDMA_OK;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_GetDevCfg
/// @brief \b Function  \b Description: Get device cfg for BDMA copy
/// @param <IN>         \b eDev: device
/// @param <OUT>        \b None :
/// @param <RET>        \b src & dst device configuration for bdma copy
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static U8 _MDrv_BDMA_GetDevCfg(BDMA_DEVICE_t eDev)
{
    if (!_gsHwInfo.bEnDevDw)
        return eDev;

    return (U8)(eDev | MHal_BDMA_GetDevDw(eDev));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_GetHwInfo
/// @brief \b Function  \b Description: Get hardware support facilities
/// @param <IN>         \b None :
/// @param <OUT>        \b *psHwInfo: Hardware information
/// @param <RET>        \b TRUE : Success FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static B16 _MDrv_BDMA_GetHwInfo(BDMA_HWINFO_t *pstHwInfo)
{
    if (!pstHwInfo)
        return FALSE;

    pstHwInfo->bEnMemFill = BDMA_MEM_FILL;
    pstHwInfo->bEnFlsCpy = BDMA_FLASH_COPY;
    pstHwInfo->bEnDevDw = BDMA_DEV_DATA_WIDTH;
    pstHwInfo->bEnDmyWrCnt = BDMA_DUMMY_WRCNT;
    pstHwInfo->bEnTSP = BDMA_TSP;
    pstHwInfo->bEnDSP = BDMA_DSP;
    pstHwInfo->bEnHK51_1KSRAM = BDMA_HK51_1KSRAM;

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _BDMA_Check_MIUDev
/// @brief \b Function  \b Description: check if address within miu range
/// @param <IN>         \b eDev : device
/// @param <IN>         \b u32Addr : start address
/// @param <OUT>        \b None:
/// @param <RET>        \b BDMA_Dev: device
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static BDMA_DEVICE_t _MDrv_BDMA_Check_MIUDev(BDMA_DEVICE_t eDev, U32 u32Addr)
{
    if (eDev != E_BDMA_DEV_MIU1 && eDev != E_BDMA_DEV_MIU0)
        return eDev;

    if (u32Addr & _gu32Miu1Base)
    {
        return E_BDMA_DEV_MIU1;
    }
    else
    {
        return E_BDMA_DEV_MIU0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Global variable
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_WaitFlashDone
/// @brief \b Function  \b Description: Wait for flash copy done
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_WaitFlashDone(void)
{
    return _MDrv_BDMA_WaitDone(E_BDMA_CH0, E_BDMA_ACT_COPY_FLASH);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_Init
/// @brief \b Function  \b Description: Init BDMA driver
/// @param u32Miu1Base  \b if 0, not support miu1
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_Init(void)
{
    U32 u32Miu1Base, u32Miu1Size;
    MDrv_SYS_GetMMAP( E_SYS_MMAP_MIU1_BASE, &u32Miu1Base, &u32Miu1Size);

    if (!_gbInitBDMA)
    {
        //BDMA_DRV_DBG("Init BDMA\n");
        _MDrv_BDMA_GetHwInfo(&_gsHwInfo);
        _gu32Miu1Base = u32Miu1Base;
        _gsHwInfo.bEnMIU1 = (u32Miu1Base) ? TRUE : FALSE;

        _gbInitBDMA = TRUE;
        return E_BDMA_OK;
    }
    else
    {
        //BDMA_DRV_DBG("Warning, BDMA has already inited\n");
        return E_BDMA_OK;
    }

    return E_BDMA_FAIL;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_Exit
/// @brief \b Function  \b Description: Exit BDMA driver
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_Exit(void)
{
    if (!_gbInitBDMA)
    {
        //BDMA_DBG_INFO("BDMA not init!\n");
        return E_BDMA_FAIL;
    }

    _gbInitBDMA = FALSE;

    return E_BDMA_OK;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_Stop
/// @brief \b Function  \b Description: Stop BDMA actions in specific channel
/// @param u8Ch         \b IN: Channel 0/1
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_Stop(U8 u8Ch)
{
    MHal_BDMA_Stop(u8Ch);

    return E_BDMA_OK;

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_Stop_All
/// @brief \b Function  \b Description: Stop all BDMA actions
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_Stop_All(void)
{
    MDrv_BDMA_Stop(E_BDMA_CH0);
    MDrv_BDMA_Stop(E_BDMA_CH1);

    return E_BDMA_OK;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_PatternFill
/// @brief \b Function  \b Description: Fill pattern to filled dst device
/// @param u32Addr      \b IN: Physical Start address
/// @param u32Len       \b IN: Fill Pattern length
/// @param u32Pattern   \b IN: Fill Pattern data
/// @param eDev         \b IN: filled dst device
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_PatternFill(U32 u32Addr, U32 u32Len, U32 u32Pattern, BDMA_DSTDEV_t eDev)
{
    U8 u8Ch;
    BDMA_OP_CB sOpCB;

    if (!_MDrv_BDMA_Is_Init())
        return E_BDMA_FAIL;

    if (DISABLE == _gsHwInfo.bEnMemFill)
    {
        //BDMA_DBG_ERR("Mem fill is not supported!\n");
        return E_BDMA_NOT_SUPPORT;
    }

    //BDMA_DBG_INFO("BDMA Fill pattern start....\n");
    if (E_BDMA_OK != _MDrv_BDMA_Check_Device(E_BDMA_SRCDEV_MEM_FILL, eDev))
        return E_BDMA_FAIL;

    sOpCB.eAct = E_BDMA_ACT_MEM_FILL;
    sOpCB.u32SrcAddr = 0;
    sOpCB.u32DstAddr = u32Addr;
    sOpCB.u32DataSize = u32Len;

    sOpCB.u8SrcDevCfg = _MDrv_BDMA_GetDevCfg(E_BDMA_DEV_MEM_FILL);
    sOpCB.u8DstDevCfg = _MDrv_BDMA_GetDevCfg(eDev);
    sOpCB.u32Cmd0 = u32Pattern;
    sOpCB.u32Cmd1 = 0;
    sOpCB.u8OpCfg = BDMA_OPCFG_DEF;
    sOpCB.u8DmyWrCnt = 0;
    sOpCB.pCbf = (void *)0;

    return _MDrv_BDMA_CmnHnd(&u8Ch, sOpCB);
}

#if defined(T3_REMOVE_LATER)
static U32 _MDrv_BDMA_FIX_ALIGN(BDMA_DEVICE_t eSrcDev, BDMA_DEVICE_t eDstDev, U32 u32SrcAddr, U32 u32Len)
{
    if ((E_BDMA_DEV_MIU0 == eSrcDev && E_BDMA_DEV_MIU1 == eDstDev)
        || (E_BDMA_DEV_MIU1 == eSrcDev && E_BDMA_DEV_MIU0 == eDstDev))
    {
        U8 remind = (u32Len + (u32SrcAddr & 0x0000000F))%128;
        //printf("remind:%u\n", remind);
        if (remind <= 16)
            return (u32Len + (17 - remind));
    }
    return u32Len;
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_CopyHnd
/// @brief \b Function  \b Description: Handle for BDMA copy data from src to dst
/// @param u32SrcAddr   \b IN: Physical source address
/// @param u32DstAddr   \b IN: Physical dst address
/// @param u32Len       \b IN: data length
/// @param eCpyType     \b IN: BDMA copy type
/// @param u8OpCfg      \b IN: u8OpCfg: default is BDMA_OPCFG_DEF
///                         - Bit0: inverse mode --> BDMA_OPCFG_INV_COPY
///                         - Bit2: Copy & CRC check in wait mode --> BDMA_OPCFG_CRC_COPY
///                         - Bit3: Copy without waiting --> BDMA_OPCFG_NOWAIT_COPY
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_CopyHnd
(
    U32 u32SrcAddr, U32 u32DstAddr, U32 u32Len, BDMA_CPY_TYPE_t eCpyType, U8 u8OpCfg
)
{
    U8 u8Ch;
    U32 u32CRCVal = 0xFFFFFFFF;
    BDMA_OP_CB sOpCB;
    BDMA_DEVICE_t eSrcDev, eDstDev;
    BDMA_RESULT eRet = E_BDMA_FAIL;

    if (!_MDrv_BDMA_Is_Init())
        return eRet;

    eSrcDev = (BDMA_DEVICE_t)(eCpyType & 0x0F);
    eDstDev = (BDMA_DEVICE_t)(eCpyType >> 8);
    if (eSrcDev == eDstDev && u32SrcAddr == u32DstAddr)
    {
        BDMA_DRV_DBG("No meaning copy: Src is same as Dst!\n");
        return E_BDMA_OK;
    }

    if (!u32Len || BDMA_CPYTYPE_MAX <= eCpyType)
    {
        BDMA_DRV_DBG("BDMA copy critical error len:%u type:%u!!\n", u32Len, eCpyType);
        return eRet;
    }

    sOpCB.u32SrcAddr = u32SrcAddr;
    sOpCB.u32DstAddr = u32DstAddr;
#if defined(T3_REMOVE_LATER)
    u32Len = _MDrv_BDMA_FIX_ALIGN(eSrcDev, eDstDev, u32SrcAddr, u32Len);
#endif
    sOpCB.u32DataSize = u32Len;

    //avoid address overlapping
    if (eSrcDev == eDstDev)
    {
        if (u32SrcAddr < u32DstAddr && u32SrcAddr+u32Len >= u32DstAddr)
            u8OpCfg |= BDMA_OPCFG_INV_COPY;
    }
    if (u8OpCfg & BDMA_OPCFG_INV_COPY)
    {
        sOpCB.u32SrcAddr += (sOpCB.u32DataSize -1);
        sOpCB.u32DstAddr += (sOpCB.u32DataSize -1);
        BDMA_DRV_DBG("a4 copy inv src:0x%x dst:0x%x\n", sOpCB.u32SrcAddr, sOpCB.u32DstAddr);
    }

    if (E_BDMA_OK != _MDrv_BDMA_Check_Device((BDMA_SRCDEV_t)eSrcDev, (BDMA_DSTDEV_t)eDstDev))
        return eRet;
    sOpCB.u8SrcDevCfg = _MDrv_BDMA_GetDevCfg(eSrcDev);
    sOpCB.u8DstDevCfg = _MDrv_BDMA_GetDevCfg(eDstDev);

    if (E_BDMA_DEV_MIU0 == eDstDev || E_BDMA_DEV_MIU1 == eDstDev)
        sOpCB.u8DmyWrCnt = HAL_BDMA_DMY_WRCNT;
    else
        sOpCB.u8DmyWrCnt = 0;
    sOpCB.u32Cmd0 = sOpCB.u32DstAddr;
    sOpCB.u32Cmd1 = 0;

    sOpCB.u8OpCfg = u8OpCfg;
    sOpCB.eAct = (E_BDMA_DEV_FLASH == eSrcDev) ? E_BDMA_ACT_COPY_FLASH : E_BDMA_ACT_COPY_MEM;
    //TBD
    sOpCB.pCbf = (void *)0;

    _MDrv_BDMA_CopyFlush(eSrcDev, u32SrcAddr, sOpCB.u32DataSize);
    _MDrv_BDMA_CopyFlush(eDstDev, u32DstAddr, sOpCB.u32DataSize);

    if (u8OpCfg & BDMA_OPCFG_NOWAIT_COPY)
    {
        BDMA_DRV_DBG("BDMA no wait copy\n");
        return _MDrv_BDMA_CmnHnd(&u8Ch, sOpCB);
    }

    //Copy and CRC32 check
    if (u8OpCfg & BDMA_OPCFG_CRC_COPY)
        u32CRCVal = MDrv_BDMA_CRC32(u32SrcAddr, u32Len, BDMA_CRC32_POLY, BDMA_CRC_SEED_0, (BDMA_SRCDEV_t)eSrcDev, FALSE);
    if(E_BDMA_OK == (eRet = _MDrv_BDMA_CmnHnd(&u8Ch, sOpCB)))
    {
        //flush miu FIFO data to MIU
        if (!_gsHwInfo.bEnDmyWrCnt && (E_BDMA_DEV_MIU0 == eDstDev || E_BDMA_DEV_MIU1 == eDstDev))
            MDrv_BDMA_CRC32(sOpCB.u32DstAddr, HAL_BDMA_DMY_WRCNT, BDMA_CRC32_POLY, BDMA_CRC_SEED_0, (BDMA_SRCDEV_t)eDstDev, FALSE);

        if (u8OpCfg & BDMA_OPCFG_CRC_COPY)
        {
            if (u32CRCVal != (u32DstAddr = MDrv_BDMA_CRC32(u32DstAddr, u32Len, BDMA_CRC32_POLY, BDMA_CRC_SEED_0, (BDMA_SRCDEV_t)eDstDev, FALSE)))
            {
                BDMA_DRV_DBG("Copy check CRC error: (0x%x != 0x%x)\n", u32DstAddr, u32CRCVal);
                return E_BDMA_FAIL;
            }
        }
        return E_BDMA_OK;
    }

    BDMA_DRV_DBG("Copy error:%u\n", eRet);
    return eRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_MemCopy
/// @brief \b Function  \b Description: Copy dram to dram no matter dram address within miu0 or miu1
/// @param u32SrcAddr   \b IN: Physical Source address
/// @param u32DstAddr   \b IN: Physical Dst address
/// @param u32Len       \b IN: data length
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_MemCopy(U32 u32SrcAddr, U32 u32DstAddr, U32 u32Len)
{
    U8 eSrc, eDst;
    BDMA_CPY_TYPE_t eCpyType;

    eSrc = (U8)_MDrv_BDMA_Check_MIUDev(E_BDMA_DEV_MIU0, u32SrcAddr);
    eDst = (U8)_MDrv_BDMA_Check_MIUDev(E_BDMA_DEV_MIU0, u32DstAddr);
    eCpyType = (BDMA_CPY_TYPE_t)(eSrc | (eDst << 8));
    return MDrv_BDMA_CopyHnd(u32SrcAddr, u32DstAddr, u32Len, eCpyType, BDMA_OPCFG_DEF);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_FlashCopy2Dram
/// @brief \b Function  \b Description: Copy data from flash to dram no matter dram address within miu0 or miu1
/// @param u32FlashAddr \b IN: Physical Source address in flash
/// @param u32DramAddr  \b IN: Physical Dst address in dram
/// @param u32Len       \b IN: data length
/// @return             \b BDMA_RESULT
////////////////////////////////////////////////////////////////////////////////
BDMA_RESULT MDrv_BDMA_FlashCopy2Dram(U32 u32FlashAddr, U32 u32DramAddr, U32 u32Len)
{
    U8 eDst = (U8)_MDrv_BDMA_Check_MIUDev(E_BDMA_DEV_MIU0, u32DramAddr);
    BDMA_CPY_TYPE_t eCpyType;

    eCpyType = (BDMA_CPY_TYPE_t)(E_BDMA_DEV_FLASH | (eDst << 8));
    return MDrv_BDMA_CopyHnd(u32FlashAddr, u32DramAddr, u32Len, eCpyType, BDMA_OPCFG_DEF);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_Search
/// @brief \b Function  \b Description: Search pattern from MIU
/// @param u32Addr      \b IN: Physical Start address
/// @param u32Len       \b IN: Search Pattern length
/// @param u32Pattern   \b IN: Search Pattern data
/// @param u32ExcluBit  \b IN: Don't care bit filter
/// @param eDev         \b IN: source device for searching
/// @return             \b U32 : matched address
////////////////////////////////////////////////////////////////////////////////
U32 MDrv_BDMA_Search(U32 u32Addr, U32 u32Len, U32 u32Pattern, U32 u32ExcluBit, BDMA_SRCDEV_t eDev)
{
    U8 u8Ch;
    BDMA_OP_CB sOpCB;

    if (!_MDrv_BDMA_Is_Init() || !u32Len || (E_BDMA_OK != _MDrv_BDMA_Check_Device(eDev, E_BDMA_DSTDEV_SEARCH)))
    {
        //BDMA_DBG_ERR("Critial error! Len:%lx!\n", u32Len);
        return 0xFFFFFFFF;
    }

    sOpCB.eAct = E_BDMA_ACT_SEARCH;
    sOpCB.u32SrcAddr = u32Addr;
    sOpCB.u32DstAddr = 0;
    sOpCB.u32DataSize = u32Len;

    sOpCB.u8SrcDevCfg = _MDrv_BDMA_GetDevCfg((BDMA_DEVICE_t)eDev);
    sOpCB.u8DstDevCfg = _MDrv_BDMA_GetDevCfg(E_BDMA_DEV_SEARCH);
    sOpCB.u32Cmd0 = u32Pattern;
    sOpCB.u32Cmd1 = u32ExcluBit;

    sOpCB.u8OpCfg = BDMA_OPCFG_DEF;
    sOpCB.u8DmyWrCnt = 0;
    sOpCB.pCbf = (void *)0;

    if (E_BDMA_OK == _MDrv_BDMA_CmnHnd(&u8Ch, sOpCB))
    {
        if (MHal_BDMA_Is_Found(u8Ch))
            return MHal_BDMA_GetMatched(u8Ch, u32Addr);
        else
            BDMA_DRV_DBG("Search not found!\n");
    }

    return 0xFFFFFFFF;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_DramSearch
/// @brief \b Function  \b Description: Search pattern from MIU
/// @param u32Addr      \b IN: Physical Start address
/// @param u32Len       \b IN: Search Pattern length
/// @param u32Pattern   \b IN: Search Pattern data
/// @param u32ExcluBit  \b IN: Don't care bit filter
/// @return             \b U32 : matched address
////////////////////////////////////////////////////////////////////////////////
U32 MDrv_BDMA_DramSearch(U32 u32SrcAddr, U32 u32Len, U32 u32Pattern, U32 u32ExcluBit)
{
    BDMA_SRCDEV_t eSrc;

    eSrc = (BDMA_SRCDEV_t)_MDrv_BDMA_Check_MIUDev(E_BDMA_DEV_MIU0, u32SrcAddr);
    return MDrv_BDMA_Search(u32SrcAddr, u32Len, u32Pattern, u32ExcluBit, eSrc);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_CRC
/// @brief \b Function  \b Description: Crc32 for BDMA copy action
/// @param u32Addr      \b IN: Physical Start address
/// @param u32Len       \b IN: data length
/// @param u32Poly      \b IN: CRC Poly Ex: 0xEDB88320 or invert 0x04C11DB7
/// @param u32Seed      \b IN: Start value Ex: 0x00000000 or 0xFFFFFFFF
/// @param eDev         \b IN: Source device
/// @param bReflect     \b IN: TRUE: CRC reflection FALSE: No reflection
/// @return             \b U32: result of CRC
////////////////////////////////////////////////////////////////////////////////
U32 MDrv_BDMA_CRC32(U32 u32Addr, U32 u32Len, U32 u32Poly, U32 u32Seed, BDMA_SRCDEV_t eDev, B16 bReflect)
{
    U8 u8Ch;
    BDMA_OP_CB sOpCB;

    if (!_MDrv_BDMA_Is_Init() || (E_BDMA_OK != _MDrv_BDMA_Check_Device(eDev, E_BDMA_DSTDEV_CRC32)))
        return 0xFFFFFFFF;
    //BDMA_DBG_INFO("BDMA CRC32 start....\n");
    sOpCB.eAct = E_BDMA_ACT_CRC32;
    sOpCB.u32SrcAddr = u32Addr;
    sOpCB.u32DstAddr = 0;
    sOpCB.u32DataSize = u32Len;

    sOpCB.u8SrcDevCfg = _MDrv_BDMA_GetDevCfg((BDMA_DEVICE_t)eDev);
    sOpCB.u8DstDevCfg = _MDrv_BDMA_GetDevCfg(E_BDMA_DEV_CRC32);
    sOpCB.u32Cmd0 = u32Poly;
    sOpCB.u32Cmd1 = u32Seed;

    //only care reflection bit
    sOpCB.u8OpCfg = (bReflect) ? BDMA_OPCFG_CRC_REFLECT : BDMA_OPCFG_DEF;
    sOpCB.u8DmyWrCnt = 0;
    sOpCB.pCbf = (void *)0;

    if (E_BDMA_OK == _MDrv_BDMA_CmnHnd(&u8Ch, sOpCB))
        return MHal_BDMA_GetCRC32(u8Ch);
    return 0xFFFFFFFF;
}


static int _MDrv_BDMA_Open(struct inode *inode, struct file *filp)
{

    return 0;
}

static int _MDrv_BDMA_Release(struct inode *inode, struct file *filp)
{

    return 0;
}

static int _MDrv_BDMA_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{

	int err = 0;
    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (_IOC_TYPE(cmd) != BDMA_IOC_MAGIC)
        return -ENOTTY;
    if (_IOC_NR(cmd) > BDMA_IOC_MAXNR)
        return -ENOTTY;
    /*
         * the type is a bitmask, and VERIFY_WRITE catches R/W
         * transfers. Note that the type is user-oriented, while
         * verify_area is kernel-oriented, so the concept of "read" and
         * "write" is reversed
         */
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err)
        return -EFAULT;


    PROBE_IO_ENTRY(MDRV_MAJOR_BDMA, _IOC_NR(cmd));

    switch(cmd)
    {
        case BDMA_IOC_INIT:
        {
            MDrv_BDMA_Init();
        }
            break;

        case BDMA_IOC_COPY:
        {
            BDMA_PARA_t stBdmaPara;
            if (copy_from_user(&stBdmaPara, (BDMA_PARA_t __user *) arg, sizeof(BDMA_PARA_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EFAULT;
            }
            if(MDrv_BDMA_CopyHnd(stBdmaPara.u32SrcAddr, stBdmaPara.u32DstAddr, stBdmaPara.u32DataSize,
                                    stBdmaPara.eCpyType, stBdmaPara.u8OpCfg) != E_BDMA_OK)
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EIO;
            }
        }
            break;

        case BDMA_IOC_COPY_MEM2MEM:
        {
            BDMA_PARA_t stBdmaPara;
            if (copy_from_user(&stBdmaPara, (BDMA_PARA_t __user *) arg, sizeof(BDMA_PARA_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EFAULT;
            }
            if(MDrv_BDMA_MemCopy(stBdmaPara.u32SrcAddr, stBdmaPara.u32DstAddr, stBdmaPara.u32DataSize) != E_BDMA_OK)
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EIO;
            }
        }
            break;

        case BDMA_IOC_COPY_FLASH2DRAM:
        {
            BDMA_PARA_t stBdmaPara;
            if (copy_from_user(&stBdmaPara, (BDMA_PARA_t __user *) arg, sizeof(BDMA_PARA_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EFAULT;
            }
            if(MDrv_BDMA_FlashCopy2Dram(stBdmaPara.u32SrcAddr, stBdmaPara.u32DstAddr, stBdmaPara.u32DataSize) != E_BDMA_OK)
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EIO;
            }
        }
            break;

        case BDMA_IOC_SEARCH_DRAM:
        {
            BDMA_PARA_t stBdmaPara;
            if (copy_from_user(&stBdmaPara, (BDMA_PARA_t __user *) arg, sizeof(BDMA_PARA_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EFAULT;
            }
            if(MDrv_BDMA_DramSearch(stBdmaPara.u32SrcAddr, stBdmaPara.u32DataSize, stBdmaPara.u32Pattern, stBdmaPara.u32ExcluBit) != E_BDMA_OK)
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EIO;
            }
            if (copy_to_user((BDMA_PARA_t __user *) arg, &stBdmaPara, sizeof(BDMA_PARA_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
                return -EFAULT;
            }
        }
            break;

        default:
            PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
            return -ENOTTY;
            break;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_BDMA, _IOC_NR(cmd));
    return err;
}


static struct file_operations bdma_fops = {
    .owner =    THIS_MODULE,
    .open =     _MDrv_BDMA_Open,
    .release =  _MDrv_BDMA_Release,
    .ioctl =    _MDrv_BDMA_Ioctl,
};

static int __init BDMA_Init_Module(void)
{
    int result;
    dev_t dev = 0;

    if (_u32BdmaMajor)
    {
        dev = MKDEV(_u32BdmaMajor, _u32BdmaMinor);
        result = register_chrdev_region(dev, BDMA_DEVICE_COUNT, MDRV_NAME_BDMA);
    }
    else
    {
        result = alloc_chrdev_region(&dev, _u32BdmaMinor, BDMA_DEVICE_COUNT, MDRV_NAME_BDMA);
        _u32BdmaMajor = MAJOR(dev);
    }
    if (result < 0)
    {
        BDMA_DRV_DBG("can't get major %d\n", _u32BdmaMajor);
        return result;
    }

    cdev_init(&_stBdmaDev, &bdma_fops);
    _stBdmaDev.owner = THIS_MODULE;
    _stBdmaDev.ops = &bdma_fops;
    result = cdev_add (&_stBdmaDev, dev, BDMA_DEVICE_COUNT);

    if (result)
    {
        BDMA_DRV_DBG("Error %d adding BDMA", result);
        unregister_chrdev_region(dev, BDMA_DEVICE_COUNT);
        return result;
    }

    BDMA_DRV_DBG("init %d\n", _u32BdmaMajor);

    MDrv_BDMA_Init();

	return 0;

}

static void __exit BDMA_Cleanup_Module(void)
{
	cdev_del(&_stBdmaDev);
	unregister_chrdev_region(MKDEV(_u32BdmaMajor, _u32BdmaMinor), BDMA_DEVICE_COUNT);

    BDMA_DRV_DBG("exit %d\n", _u32BdmaMajor);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////

module_init(BDMA_Init_Module);
module_exit(BDMA_Cleanup_Module);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("Byte DMA driver");
MODULE_LICENSE("MSTAR");

