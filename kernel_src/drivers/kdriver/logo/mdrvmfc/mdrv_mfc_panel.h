#ifndef _MDRV_MFC_PANEL_H_
#define _MDRV_MFC_PANEL_H_


void MDrv_MFC_InitializePanel(void);
void MDrv_MFC_InitializeOD(const U8* pODTbl);
void MDrv_MFC_InitializeScTop2_Bypanel(void);
#if(CODEBASE_SEL == CODEBASE_LINUX)
	void MDrv_MFC_InitializeBypass(void);
#endif
#endif
