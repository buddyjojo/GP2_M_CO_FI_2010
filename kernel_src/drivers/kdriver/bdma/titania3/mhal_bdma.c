
#define _MHAL_BDMA_C_

#include <linux/kernel.h>

#include "mdrv_types.h"

#include "mdrv_bdma_st.h"
#include "mhal_bdma.h"
#include "mhal_bdma_reg.h"


//----------------------------------------------------------------------
#define OPT_BDMA_HAL_DEBUG
#undef BDMA_HAL_DBG
#ifdef OPT_BDMA_HAL_DEBUG
    #define BDMA_HAL_DBG(fmt, args...)      printk(KERN_WARNING "[BDMA_HAL][%05d]" fmt, __LINE__, ## args)
#else
    #define BDMA_HAL_DBG(fmt, args...)
#endif

#undef BDMA_HAL_DBGX
#define BDMA_HAL_DBGX(fmt, args...)
//----------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
// Local Function
////////////////////////////////////////////////////////////////////////////////
static B16 _MHal_BDMA_Is_Status_On(U8 u8Ch, U8 u8Status);
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: _HAL_BDMA_Is_Status_On
/// @brief \b Function  \b Description: Get byte DMA status for channel 0/1
/// @param <IN>         \b u8Status : queried status
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
static B16 _MHal_BDMA_Is_Status_On(U8 u8Ch, U8 u8Status)
{
    U8 tmp = _MHal_R1B(BDMA_REG_STATUS + BDMA_REG_CH_OFFSET * u8Ch);
    return (tmp & u8Status) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Global Function
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_SetSrcAddr
/// @brief \b Function  \b Description: Set source address
/// @param <IN>         \b u32RegAddr: Source address
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_SetSrcAddr(U8 u8Ch, U32 u32Addr)
{
    _MHal_W2B(BDMA_REG_SRC_ADDR_L + BDMA_REG_CH_OFFSET * u8Ch, u32Addr & 0xFFFF);
    _MHal_W2B(BDMA_REG_SRC_ADDR_H + BDMA_REG_CH_OFFSET * u8Ch, u32Addr >> 16);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_SetDstAddr
/// @brief \b Function  \b Description: Set destination address
/// @param <IN>         \b u32RegAddr: destination address
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_SetDstAddr(U8 u8Ch, U32 u32Addr)
{
    _MHal_W2B(BDMA_REG_DST_ADDR_L + BDMA_REG_CH_OFFSET * u8Ch, u32Addr & 0xFFFF);
    _MHal_W2B(BDMA_REG_DST_ADDR_H + BDMA_REG_CH_OFFSET * u8Ch, u32Addr >> 16);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_GetSrcAddr
/// @brief \b Function  \b Description: Get destination address
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <RET>        \b destinaiton address
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
U32 MHal_BDMA_GetSrcAddr(U8 u8Ch)
{
    return _MHal_R2B(BDMA_REG_SRC_ADDR_L + BDMA_REG_CH_OFFSET * u8Ch) |
           (_MHal_R2B(BDMA_REG_SRC_ADDR_H + BDMA_REG_CH_OFFSET * u8Ch < 16));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_GetDstAddr
/// @brief \b Function  \b Description: Get destination address
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <RET>        \b destinaiton address
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
U32 MHal_BDMA_GetDstAddr(U8 u8Ch)
{
    return _MHal_R2B(BDMA_REG_DST_ADDR_L + BDMA_REG_CH_OFFSET * u8Ch) |
           (_MHal_R2B(BDMA_REG_DST_ADDR_H + BDMA_REG_CH_OFFSET * u8Ch < 16));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_GetDevDw
/// @brief \b Function  \b Description: Get device data width
/// @param <IN>         \b eDev: Device
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
U8 MHal_BDMA_GetDevDw(BDMA_DEVICE_t eDev)
{
    switch (eDev)
    {
        case E_BDMA_DEV_MIU0:
        case E_BDMA_DEV_MIU1:
        case E_BDMA_DEV_FLASH:
            return E_BDMA_DW_16BYTE;

        case E_BDMA_DEV_MEM_FILL:
            return E_BDMA_DW_4BYTE;

        default:
            return E_BDMA_DW_1BYTE;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_SetSrcDev
/// @brief \b Function  \b Description: Set source device
/// @param <IN>         \b u8DevCfg: Device config
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_SetSrcDev(U8 u8Ch, U8 u8DevCfg)
{
    _MHal_W1B(BDMA_REG_SRC_SEL + BDMA_REG_CH_OFFSET * u8Ch, u8DevCfg);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_SetSrcDev
/// @brief \b Function  \b Description: Set source device
/// @param <IN>         \b u8DevCfg: Device config
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_SetDstDev(U8 u8Ch, U8 u8DevCfg)
{
    _MHal_W1B(BDMA_REG_DST_SEL + BDMA_REG_CH_OFFSET * u8Ch, u8DevCfg);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_SetCmd0
/// @brief \b Function  \b Description: Set command 0
/// @param <IN>         \b u32Cmd: Command 0 value
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_SetCmd0(U8 u8Ch, U32 u32Cmd)
{
    _MHal_W2B(BDMA_REG_CMD0_L + BDMA_REG_CH_OFFSET * u8Ch, u32Cmd & 0xFFFF);
    _MHal_W2B(BDMA_REG_CMD0_H + BDMA_REG_CH_OFFSET * u8Ch, u32Cmd >> 16);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_SetCmd1
/// @brief \b Function  \b Description: Set command 1
/// @param <IN>         \b u32Cmd: Command 1 value
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_SetCmd1(U8 u8Ch, U32 u32Cmd)
{
    _MHal_W2B(BDMA_REG_CMD1_L + BDMA_REG_CH_OFFSET * u8Ch, u32Cmd & 0xFFFF);
    _MHal_W2B(BDMA_REG_CMD1_H + BDMA_REG_CH_OFFSET * u8Ch, u32Cmd >> 16);

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_GetCmd0
/// @brief \b Function  \b Description: Get command 0
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b *pu32Cmd : Command 0 value
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
U32 MHal_BDMA_GetCmd0(U8 u8Ch)
{
    return _MHal_R2B(BDMA_REG_CMD0_L + BDMA_REG_CH_OFFSET * u8Ch) |
           (_MHal_R2B(BDMA_REG_CMD0_H + BDMA_REG_CH_OFFSET * u8Ch < 16));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_GetCmd1
/// @brief \b Function  \b Description: Get command 1
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b *pu32Cmd : Command 1 value
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
U32 MHal_BDMA_GetCmd1(U8 u8Ch)
{
    return _MHal_R2B(BDMA_REG_CMD1_L + BDMA_REG_CH_OFFSET * u8Ch) |
           (_MHal_R2B(BDMA_REG_CMD1_H + BDMA_REG_CH_OFFSET * u8Ch < 16));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_GetCRC32
/// @brief \b Function  \b Description: Get crc32 result
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None:
/// @param <RET>        \b Matched address
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
U32 MHal_BDMA_GetCRC32(U8 u8Ch)
{
    return MHal_BDMA_GetCmd1(u8Ch);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_GetMatched
/// @brief \b Function  \b Description: Get matched address
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None:
/// @param <RET>        \b Matched address
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
U32 MHal_BDMA_GetMatched(U8 u8Ch, U32 u32SrcAddr)
{
    return (u32SrcAddr + MHal_BDMA_GetSrcAddr(u8Ch) - 1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_SetLen
/// @brief \b Function  \b Description: Set data size
/// @param <IN>         \b u32Len: Data size
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_SetLen(U8 u8Ch, U32 u32Len)
{
    _MHal_W2B(BDMA_REG_SIZE_L + BDMA_REG_CH_OFFSET * u8Ch, u32Len & 0xFFFF);
    _MHal_W2B(BDMA_REG_SIZE_H + BDMA_REG_CH_OFFSET * u8Ch, u32Len >> 16);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Set_Addr_Dec
/// @brief \b Function  \b Description: Set address increasing/decreasing direction
/// @param <IN>         \b bDec: TRUE: Decreasing FALSE: Increasing
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_Set_Addr_Dec(U8 u8Ch, B16 bDec)
{
    _MHal_W1Bb(BDMA_REG_MISC + BDMA_REG_CH_OFFSET * u8Ch, bDec, BDMA_CH_ADDR_DECDIR);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Set_CRC_Reflect
/// @brief \b Function  \b Description: Set CRC value reflection
/// @param <IN>         \b bReflect: TRUE: Reflection FALSE: Not Reflection
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_Set_CRC_Reflect(U8 u8Ch, B16 bReflect)
{
    _MHal_W1Bb(BDMA_REG_MISC + BDMA_REG_CH_OFFSET * u8Ch, bReflect, BDMA_CH_CRC_REFLECTION);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_SetDmyWrCnt
/// @brief \b Function  \b Description: Set Dummy write count
/// @param <IN>         \b u8Cnt: Dummy count
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_SetDmyWrCnt(U8 u8Ch, U8 u8Cnt)
{
    _MHal_W1B(BDMA_REG_DWUM_CNT + BDMA_REG_CH_OFFSET * u8Ch, u8Cnt);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_TrigOn
/// @brief \b Function  \b Description: Trigger on BDMA action for channel 0/1
/// @param <IN>         \b eAct: BDMA action
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <IN>         \b u8Para: trigger parameter
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_TrigOn(U8 u8Ch)
{
    _MHal_W1Bb(BDMA_REG_CTRL + BDMA_REG_CH_OFFSET * u8Ch, ENABLE, BDMA_CH_TRIGGER);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Stop
/// @brief \b Function  \b Description: Stop BDMA operations
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_Stop(U8 u8Ch)
{
    _MHal_W1Bb(BDMA_REG_CTRL + BDMA_REG_CH_OFFSET * u8Ch, ENABLE, BDMA_CH_STOP);
    _MHal_W1Bb(BDMA_REG_CTRL + BDMA_REG_CH_OFFSET * u8Ch, DISABLE, BDMA_CH_STOP);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Enable_Done_INT
/// @brief \b Function  \b Description: Enable interrupt when action done
/// @param <IN>         \b bEnable: TRUE: Enable FALSE: Disable
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_Enable_INT(U8 u8Ch, B16 bEnable)
{
    _MHal_W1Bb(BDMA_REG_MISC + BDMA_REG_CH_OFFSET * u8Ch, bEnable, BDMA_CH_DONE_INT_EN);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Is_Queued
/// @brief \b Function  \b Description: Check if any action is queued
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: queued FALSE: empty
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
B16 MHal_BDMA_Is_Queued(U8 u8Ch)
{
    return _MHal_BDMA_Is_Status_On(u8Ch, BDMA_CH_QUEUED);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_BDMA_Is_Busy
/// @brief \b Function  \b Description: Check if Byte DMA is busy
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Busy FALSE: Not busy
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
B16 MHal_BDMA_Is_Busy(U8 u8Ch)
{
    return _MHal_BDMA_Is_Status_On(u8Ch, BDMA_CH_BUSY);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Is_Int
/// @brief \b Function  \b Description: Check if interrupted when Byte DMA is done
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Interrupted FALSE: No interrupt
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
B16 MHal_BDMA_Is_Int(U8 u8Ch)
{
    return _MHal_BDMA_Is_Status_On(u8Ch, BDMA_CH_INT);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Is_Done
/// @brief \b Function  \b Description: Check if Byte DMA action is done
/// @param <IN>         \b eAct: BDMA action
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Done FALSE: Not Done
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
B16 MHal_BDMA_Is_Done(U8 u8Ch, BDMA_ACT_t eAct)
{
    if (E_BDMA_ACT_SEARCH == eAct && MHal_BDMA_Is_Found(u8Ch))
        return TRUE;
    return _MHal_BDMA_Is_Status_On(u8Ch, BDMA_CH_DONE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Is_Found
/// @brief \b Function  \b Description: Check if Byte DMA find matched pattern
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: found FALSE: Not found
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
B16 MHal_BDMA_Is_Found(U8 u8Ch)
{
    return _MHal_BDMA_Is_Status_On(u8Ch, BDMA_CH_RESULT);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Clear_Status
/// @brief \b Function  \b Description: Clear BDMA action status
/// @param <IN>         \b eAct: BDMA action
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
void MHal_BDMA_Clear_Status(U8 u8Ch)
{
    _MHal_W1B(BDMA_REG_STATUS + BDMA_REG_CH_OFFSET * u8Ch, BDMA_CH_CLEAR_STATUS);
}

