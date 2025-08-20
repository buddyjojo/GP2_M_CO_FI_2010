#ifndef _MDRV_MFC_LETTERBOX_H_
#define _MDRV_MFC_LETTERBOX_H_

void apiInitializeLetterbox(U16 wPnlWidth, U16 wPnlHeight, U8 ucPnlVfreq);
BOOL getSamePixelCnt(void);
void Mfc_Polling(void);
void DetectLetterBox(void);
void ProgramLetterBox(void);
void apiGetMfcInfo(void);


#endif
