#ifndef _MSFB_H_
#define _MSFB_H_

#ifdef _MSFB_C_
#define _MSFBDEC_
#else
#define _MSFBDEC_ extern
#endif

#include "mdrv_mfc_platform.h"

#define HINIBBLE(value)           ((value) / 0x10)
#define LONIBBLE(value)           ((value) & 0x0f)
#define msCalculateDecimal(dwX,dwY)         ((((U32)dwX) + (dwY / 2)) / dwY)

typedef struct MST_MFC_MiuBaseAddr_s
{
	U8  u8MfcMode:2;
	U32 u32MfcBase;
	U32 u32MfcSize;

    U8  u8IpMode;///:3;
	U32 u32IpYcoutBase;
	U32 u32IpYcoutSize;

	U8  u8GammaMode:2;
	U32 u32GammaBase0;
	U32 u32GammaBase1;
	U32 u32GammaMLSize;

	U8  u8OdMode;
	U32 u32OdBaseEven;
	U32 u32OdLimitEven;
	U32 u32OdBaseOdd;
	U32 u32OdLimitOdd;
	U32 u32OdSize;
	U32 u32OdSizehalf;

	U32 u32OdLsbBase;
	U32 u32OdLsbLimit;
	U32 u32OdLsbSize;

	U32 u32AutoBist;

	U8  u8XDataOnDramMode:1;
	U32 u8XDataOnDramBase;
	U32 u8XDataOnDramSize;

	U8	u8MPifMode:1;
	U32	u32PifBase;
	U32	u32PifBase2;
	U32 u32PifSize;
}MST_MFC_MiuBaseAddr_t, *PMST_MFC_MiuBaseAddr_t;
		
	_MSFBDEC_ XDATA MST_MFC_MiuBaseAddr_t gmfcMiuBaseAddr;

#define ENABLE_MEM_DQS_SELF_TEST	0

//====Block Size======================================================
#if 0
#if (MEMORY_TYPE_SEL == DDR1_08M_16BITS_1)
  #define MEMORY_SIZE  			0x1000000
#elif (MEMORY_TYPE_SEL == DDR1_08M_16BITS_2 || MEMORY_TYPE_SEL == DDR2_16M_16BITS_1)
  #define MEMORY_SIZE  			0x2000000
#elif (MEMORY_TYPE_SEL == DDR2_16M_16BITS_2 || MEMORY_TYPE_SEL == DDR2_32M_16BITS_1)
  #define MEMORY_SIZE  			0x4000000
#elif (MEMORY_TYPE_SEL == DDR2_32M_16BITS_2)
  #define MEMORY_SIZE  			0x8000000
#endif
#endif

void MDrv_MFC_AutoMemPhase(void);
void MDrv_MFC_InitializeMiu(void);
void MDrv_MFC_SetMiuSSC(U16 u16KHz, U16  u16Percent, BOOL bEnable);
#endif

