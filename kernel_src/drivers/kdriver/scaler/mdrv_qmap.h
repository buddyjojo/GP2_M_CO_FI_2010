
#ifndef __MDRV_QMAP_H
#define __MDRV_QMAP_H

#define PQ_ENABLE_DEBUG     1
#define PQ_ENABLE_CHECK     0
#define PQ_IP_COMM          0xfe
#define PQ_IP_ALL           0xff
#define MAX_WINDOW          2

#define E_XRULE_HSD         0
#define E_XRULE_VSD         1
#define E_XRULE_HSP         2
#define E_XRULE_VSP         3
#define E_XRULE_CSC         4
#define E_XRULE_NUM         5

#if PQ_ENABLE_DEBUG
extern BOOL bEnPQDbg;
#define PQ_DBG(x)  \
do{                \
    if (bEnPQDbg){ \
        x;         \
    }              \
}while(0)

#else
#define PQ_DBG(x) //x
#endif

typedef enum {
    PQ_TABTYPE_GENERAL,
    PQ_TABTYPE_COMB,
    PQ_TABTYPE_SCALER,
    PQ_TABTYPE_SRAM1,
    PQ_TABTYPE_SRAM2,
    PQ_TABTYPE_SRAM3,
    PQ_TABTYPE_SRAM4,
    PQ_TABTYPE_C_SRAM1,
    PQ_TABTYPE_C_SRAM2,
    PQ_TABTYPE_C_SRAM3,
    PQ_TABTYPE_C_SRAM4,
    PQ_TABTYPE_SRAM_COLOR_INDEX,
    PQ_TABTYPE_SRAM_COLOR_GAIN_SNR,
    PQ_TABTYPE_SRAM_COLOR_GAIN_DNR,
} EN_PQ_TABTYPE;

typedef struct
{
    U8 *pIPCommTable;
    U8 *pIPTable;
    U8 u8TabNums;
    U8 u8TabType;
} EN_IPTAB_INFO;

#if 0
typedef struct
{
    U8 *pIPTable;
    U8 u8TabNums;
    U8 u8TabType;
    U8 u8TabIdx;
} EN_IP_Info;
#endif

typedef struct {
  U8 u8PQ_InputType_Num;
  U8 u8PQ_IP_Num;
  U8 *pQuality_Map_Aray;
  EN_IPTAB_INFO *pIPTAB_Info;
  U8 *pSkipRuleIP;

  U8 u8PQ_XRule_IP_Num[E_XRULE_NUM];
  U8 *pXRule_IP_Index[E_XRULE_NUM];
  U8 *pXRule_Array[E_XRULE_NUM];

  U8 u8PQ_GRule_Num;
  U8 u8PQ_GRule_IPNum;
  U8 *pGRule_IP_Index;
  U8 *pGRule_Array;
} PQTABLE_INFO;

void MDrv_SCMAP_DumpTable(EN_IP_Info* pIP_Info);
/*
void MDrv_SCMAP_DumpGeneralRegTable(EN_IP_Info* pIP_Info);
void MDrv_SCMAP_DumpCombRegTable(EN_IP_Info* pIP_Info);
void MDrv_SCMAP_DumpScalerRegTable(EN_IP_Info* pIP_Info);
void MDrv_SCMAP_DumpFilterTable(EN_IP_Info* pIP_Info);
*/

void MDrv_SCMAP_Init(U8 u8QMapType);
void MDrv_SCMAP_DesideSrcType(U8 u8WinIdx, PSC_SOURCE_INFO_t pSrcInfo);
U16 MDrv_SCMAP_GetSrcTypeIndex(U8 u8WinIdx);

BOOL MDrv_SCMAP_LoadScalingTable(U8 u8WinIdx,
                           U8 eScalingType,
                           BOOL bPreV_ScalingDown,
                           BOOL bInterlace,
                           BOOL bColorSpaceYUV,
                           U16 u16InputSize,
                           U16 u16SizeAfterScaling);

void MDrv_SCMAP_SetFilmMode(U8 u8WinIdx);//for reseting film setting, [091012_Leo]

void MDrv_SCMAP_SetMemFormat(U8 u8WinIdx, U8 *u8BitsPerPixel, BOOL *bMemFormat422);
void MDrv_Set_QMap(U8 u8WinIdx);

void MDrv_SCMAP_SetColorRange(U8 u8WinIdx, BOOL bColorRange0_255);
BOOL MDrv_SCMAP_SetCSC(U8 u8WinIdx);
//void MDrv_SCMAP_SetFilmMode(U8 u8WinIdx, BOOL bEnable); //marked unused function, [091012_Leo]
void MDrv_SCMAP_SetNonLinearScaling(U8 u8WinIdx, U8 u8Level, BOOL bEnable);
void MDrv_SCMAP_DumpScalerRegTable_AP(EN_IP_Info_AP* pIP_Info);
void MDrv_SCMAP_DumpCombRegTable_AP(EN_IP_Info_AP* pIP_Info);

#endif

