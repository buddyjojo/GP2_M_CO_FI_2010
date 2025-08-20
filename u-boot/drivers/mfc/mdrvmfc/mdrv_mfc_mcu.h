/******************************************************************************
 Copyright (c) 2005 MStar Semiconductor, Inc.
 All rights reserved.

 [Module Name]: MsMcu.h
 [Date]:        17-Aug-2005
 [Comment]:
   Mcu control definition.
 [Reversion History]:
*******************************************************************************/
#ifndef _MDRV_MFC_MCU_H_
#define _MDRV_MFC_MCU_H_

#define ISR_EXT_TIMER0_INT		1
#define ISR_EXT_TIMER1_INT		1

void MDrv_MFC_McuWatchDogClear(void);
void MDrv_MFC_SetInterrupt(BOOL bCtrl);
void MDrv_MFC_McuInitialize(void);

#endif
