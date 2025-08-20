/******************************************************************************
 Copyright (c) 2004 MStar Semiconductor, Inc.
 All rights reserved.

 [Module Name]: Pnl_Sh20.h
 [Date]:        11-Feb-2004
 [Comment]:
   Panel parameters.[B7B-PH-SM]
 [Reversion History]:
*******************************************************************************/

#ifndef _PNL_LCEAll_H
#define _PNL_LCEAll_H

///////////////////////////////////////////////
// Common setting
///////////////////////////////////////////////
#define PANEL_NAME      			"PNL_LCEAll"
#define PANEL_INC_VTOTAL_FOR_50HZ				1

    extern code U8 tOverDrive[];
    extern code U8 tODGIP[];
    extern code U8 tOD55[];
    extern code U8 tOD_Array[][1056];
	extern code U8 tInitializeDAC[];
	extern code U8 tInitializeVCOM[];
	extern U32 gDACLen;
	extern U32 gVCOMLen;

#endif
