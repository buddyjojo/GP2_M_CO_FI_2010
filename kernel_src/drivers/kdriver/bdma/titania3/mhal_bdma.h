
#ifndef _MHAL_BDMA_H_
#define _MHAL_BDMA_H_

#include "mdrv_types.h"

#include "mdrv_bdma_st.h"


#define HAL_BDMA_DMY_WRCNT      (4)
#define T3_REMOVE_LATER

#define BDMA_MEM_FILL           (ENABLE)
#define BDMA_FLASH_COPY         (ENABLE)
#define BDMA_DUMMY_WRCNT        (ENABLE)
#define BDMA_DEV_DATA_WIDTH     (ENABLE)
#define BDMA_TSP                (ENABLE)
#define BDMA_DSP                (ENABLE)
#define BDMA_HK51_1KSRAM        (ENABLE)


typedef enum BDMA_CHANNEL_e
{
    E_BDMA_CH_START,
    E_BDMA_CH0 = E_BDMA_CH_START,
    E_BDMA_CH1,
    E_BDMA_CH_END,
}BDMA_CHANNEL_t;

typedef enum BDMA_ACT_e
{
    E_BDMA_ACT_COPY_MEM,
    E_BDMA_ACT_COPY_FLASH,
    E_BDMA_ACT_MEM_FILL,
    E_BDMA_ACT_SEARCH,
    E_BDMA_ACT_CRC32,
    E_BDMA_ACT_MAX,
}BDMA_ACT_t;

typedef enum BDMA_DATAWIDTH_e
{
    E_BDMA_DW_1BYTE     = 0x00,
    E_BDMA_DW_2BYTE     = 0x10,
    E_BDMA_DW_4BYTE     = 0x20,
    E_BDMA_DW_8BYTE     = 0x30,
    E_BDMA_DW_16BYTE    = 0x40,
    E_BDMA_DW_MAX       = E_BDMA_DW_16BYTE,
}BDMA_DATAWIDTH_t;


#ifdef _MHAL_BDMA_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

//----------------------------------------------------------------------
INTERFACE void MHal_BDMA_SetSrcAddr(U8 u8Ch, U32 u32Addr);
INTERFACE void MHal_BDMA_SetDstAddr(U8 u8Ch, U32 u32Addr);
INTERFACE U32 MHal_BDMA_GetDstAddr(U8 u8Ch);
INTERFACE U32 MHal_BDMA_GetSrcAddr(U8 u8Ch);
INTERFACE U8 MHal_BDMA_GetDevDw(BDMA_DEVICE_t eDev);
INTERFACE void MHal_BDMA_SetSrcDev(U8 u8Ch, U8 u8DevCfg);
INTERFACE void MHal_BDMA_SetDstDev(U8 u8Ch, U8 u8DevCfg);
INTERFACE void MHal_BDMA_SetCmd0(U8 u8Ch, U32 u32Cmd);
INTERFACE void MHal_BDMA_SetCmd1(U8 u8Ch, U32 u32Cmd);
INTERFACE U32 MHal_BDMA_GetCmd0(U8 u8Ch);
INTERFACE U32 MHal_BDMA_GetCmd1(U8 u8Ch);
INTERFACE U32 MHal_BDMA_GetCRC32(U8 u8Ch);
INTERFACE U32 MHal_BDMA_GetMatched(U8 u8Ch, U32 u32SrcAddr);
INTERFACE void MHal_BDMA_SetLen(U8 u8Ch, U32 u32Len);
INTERFACE void MHal_BDMA_Set_Addr_Dec(U8 u8Ch, B16 bDec);
INTERFACE void MHal_BDMA_Set_CRC_Reflect(U8 u8Ch, B16 bReflect);
INTERFACE void MHal_BDMA_SetDmyWrCnt(U8 u8Ch, U8 u8Cnt);
INTERFACE void MHal_BDMA_TrigOn(U8 u8Ch);
INTERFACE void MHal_BDMA_Stop(U8 u8Ch);
INTERFACE void MHal_BDMA_Enable_INT(U8 u8Ch, B16 bEnable);
INTERFACE B16 MHal_BDMA_Is_Queued(U8 u8Ch);
INTERFACE B16 MHal_BDMA_Is_Busy(U8 u8Ch);
INTERFACE B16 MHal_BDMA_Is_Int(U8 u8Ch);
INTERFACE B16 MHal_BDMA_Is_Done(U8 u8Ch, BDMA_ACT_t eAct);
INTERFACE B16 MHal_BDMA_Is_Found(U8 u8Ch);
INTERFACE void MHal_BDMA_Clear_Status(U8 u8Ch);
//----------------------------------------------------------------------

#undef INTERFACE

#endif

