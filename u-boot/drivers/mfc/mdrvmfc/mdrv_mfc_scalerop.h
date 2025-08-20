#ifndef _MSSCALEROP_H_
#define _MSSCALEROP_H_

#ifdef _MSSCALEROP_C_
#define _MSSCALEROPDEC_
#else
#define _MSSCALEROPDEC_ extern
#endif

#define Cable_effect 	2
#if(CODEBASE_SEL == CODEBASE_51)
	#if(ENABLE_USER_TOTAL)
	#define USER_HT_50		2200
	#define USER_VT_50		1272
	#define USER_HT_60		2100
	#define USER_VT_60		1150
	#endif
void msSetFPLLOutDClk(U8 ucVfreq, U8 ucVHzFrmT2, BOOL enableFPLL);
	void msSetOutDClk(U8 ucVfreq, U8 ucVHzFrmT2, BOOL enableFPLL);
#else
	void MDrv_MFC_SetOutDClk(U16 u16InputfreqX100, BOOL enableFPLL);
#endif
void MDrv_MFC_SetGainPhase(void);
void MDrv_MFC_InitializeScalerOP(void);
void MDrv_MFC_SetLvdsSSC(U16 u16KHz, U16  u16Percent, BOOL bEnable);
#endif

