#ifndef _MDRV_MFC_PANEL_H_
#define _MDRV_MFC_PANEL_H_


void MDrv_MFC_InitializePanel(void);
void MDrv_MFC_InitializeOD(const U8* pODTbl);
void Read_OD(void);
void MDrv_MFC_InitializeScTop2_Bypanel(void);
void MDrv_MFC_SetPWMFreq(U8 u8GroupIndex ,U8 u8Duty, U16 u16Shift, BOOL bEnable);
	void MDrv_MFC_InitializeBypass(void);
void appPWMHandler(void);
#endif
