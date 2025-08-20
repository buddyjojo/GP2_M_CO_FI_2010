
#ifndef _MDRV_BDMA_H_
#define _MDRV_BDMA_H_

#include "mdrv_bdma_st.h"


#define BDMA_SEARCH_ALL_MATCHED (0)
#define BDMA_CRC32_POLY         (0x04C11DB7)
#define BDMA_CRC16_POLY         (0x8005)
#define BDMA_CRC_SEED_0         (0)
#define BDMA_CRC_SEED_F         (0xFFFFFFFF)

/// Operation cfg
#define BDMA_OPCFG_DEF          0x0
#define BDMA_OPCFG_INV_COPY     0x0
#define BDMA_OPCFG_CRC_REFLECT  0x1     //bit reflection of each input byte
#define BDMA_OPCFG_CRC_COPY     0x2     //copy then crc check
#define BDMA_OPCFG_NOWAIT_COPY  0x4     //copy then quit


#define BDMA_SET_DEV(drvDev, val) E_BDMA_##drvDev = val
#define BDMA_SRCDEV_COL \
            BDMA_SET_DEV(SRCDEV_MIU0, E_BDMA_DEV_MIU0) \
            ,BDMA_SET_DEV(SRCDEV_MIU1, E_BDMA_DEV_MIU1) \
            ,BDMA_SET_DEV(SRCDEV_MEM_FILL, E_BDMA_DEV_MEM_FILL) \
            ,BDMA_SET_DEV(SRCDEV_FLASH, E_BDMA_DEV_FLASH) \
            ,BDMA_SET_DEV(SRCDEV_NOT_SUPPORT, E_BDMA_DEV_NOT_SUPPORT)

#define BDMA_DSTDEV_COL \
            BDMA_SET_DEV(DSTDEV_MIU0, E_BDMA_DEV_MIU0) \
            ,BDMA_SET_DEV(DSTDEV_MIU1, E_BDMA_DEV_MIU1) \
            ,BDMA_SET_DEV(DSTDEV_SEARCH, E_BDMA_DEV_SEARCH) \
            ,BDMA_SET_DEV(DSTDEV_CRC32, E_BDMA_DEV_CRC32) \
            ,BDMA_SET_DEV(DSTDEV_VDMCU, E_BDMA_DEV_VDMCU) \
            ,BDMA_SET_DEV(DSTDEV_DSP, E_BDMA_DEV_DSP) \
            ,BDMA_SET_DEV(DSTDEV_TSP, E_BDMA_DEV_TSP) \
            ,BDMA_SET_DEV(DSTDEV_HK51_1KSRAM, E_BDMA_DEV_1KSRAM_HK51) \
            ,BDMA_SET_DEV(DSTDEV_DRAM_AEON, E_BDMA_DEV_RESERVED) \
            ,BDMA_SET_DEV(DSTDEV_NOT_SUPPORT, E_BDMA_DEV_NOT_SUPPORT)

typedef enum BDMA_SRCDEV_e
{
    BDMA_SRCDEV_COL
}BDMA_SRCDEV_t;

typedef enum BDMA_DSTDEV_e
{
    BDMA_DSTDEV_COL
}BDMA_DSTDEV_t;

typedef struct BDMA_HWINFO_st
{
    B16 bEnMIU1;        //MIU1
    B16 bEnHost;        //bdma host (T2)
    B16 bEnMemFill;     //memory fill (T3)
    B16 bEnFlsCpy;      //flash copy (T3)
    B16 bEnDevDw;       //bdma device data width (T3)
    B16 bEnDmyWrCnt;    //bdma dummy wr count (T3)
    B16 bEnTSP;         //bdma to TSP (T3)
    B16 bEnDSP;         //bdma to DSP (T3)
    B16 bEnHK51_1KSRAM; //bdma to HK51_1KSRAM (T3)
}BDMA_HWINFO_t;

typedef enum BDMA_RESULT_e
{
    E_BDMA_NOT_SUPPORT = -1,
    E_BDMA_FAIL = 0,
    E_BDMA_OK = 1,
    E_BDMA_TIMEOUT,
    E_BDMA_QUEUE_FULL,
    E_BDMA_BUSY,
}BDMA_RESULT_t, BDMA_RESULT;


#ifdef _MDRV_BDMA_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

//----------------------------------------------------------------------
INTERFACE BDMA_RESULT MDrv_BDMA_WaitFlashDone(void);
INTERFACE BDMA_RESULT MDrv_BDMA_Init(void);
INTERFACE BDMA_RESULT MDrv_BDMA_Exit(void);
INTERFACE BDMA_RESULT MDrv_BDMA_Stop(U8 u8Ch);
INTERFACE BDMA_RESULT MDrv_BDMA_Stop_All(void);
INTERFACE BDMA_RESULT MDrv_BDMA_PatternFill(U32 u32Addr, U32 u32Len, U32 u32Pattern, BDMA_DSTDEV_t eDev);
INTERFACE BDMA_RESULT MDrv_BDMA_CopyHnd(U32 u32SrcAddr, U32 u32DstAddr, U32 u32Len, BDMA_CPY_TYPE_t eCpyType, U8 u8OpCfg);
INTERFACE BDMA_RESULT MDrv_BDMA_MemCopy(U32 u32SrcAddr, U32 u32DstAddr, U32 u32Len);
INTERFACE BDMA_RESULT MDrv_BDMA_FlashCopy2Dram(U32 u32FlashAddr, U32 u32DramAddr, U32 u32Len);
INTERFACE U32 MDrv_BDMA_Search(U32 u32Addr, U32 u32Len, U32 u32Pattern, U32 u32ExcluBit, BDMA_SRCDEV_t eDev);
INTERFACE U32 MDrv_BDMA_DramSearch(U32 u32SrcAddr, U32 u32Len, U32 u32Pattern, U32 u32ExcluBit);
INTERFACE U32 MDrv_BDMA_CRC32(U32 u32Addr, U32 u32Len, U32 u32Poly, U32 u32Seed, BDMA_SRCDEV_t eDev, B16 bReflect);
//----------------------------------------------------------------------

#undef INTERFACE

#endif

