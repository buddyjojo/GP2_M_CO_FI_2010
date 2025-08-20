
#ifndef _MSINIT_H_
#define _MSINIT_H_

#ifdef _MSINIT_C_
#define _MSINITDEC_
#else
#define _MSINITDEC_ extern
#endif

void MDrv_MFC_InitializeChip(void);
//void MDrv_MFC_PowerDownChip(void);
void MDrv_MFC_PowerDownChipU3(void);
#endif

