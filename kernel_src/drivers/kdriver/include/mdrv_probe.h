////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MDRV_PROBE_H_
#define __MDRV_PROBE_H__
#define ENABLE_PROBE    1
//#define ENABLE_PROBE    0

#if ENABLE_PROBE


extern void PROBE_IO_ENTRY(int DrvID, int IONum);
extern void PROBE_IO_EXIT(int DrvID, int IONum);
extern void PROBE_JIFFIES(void);
extern void PROBE_INT_ENTRY(int INTNum);
extern void PROBE_INT_EXIT(int INTNum);
#else

#define PROBE_IO_ENTRY(DrvID, IONum)
#define PROBE_IO_EXIT(DrvID, IONum)
#define PROBE_JIFFIES()
#define PROBE_INT_ENTRY(INTNum)
#define PROBE_INT_EXIT(INTNum)

#endif

#endif // __MDRV_PROBE_H__

