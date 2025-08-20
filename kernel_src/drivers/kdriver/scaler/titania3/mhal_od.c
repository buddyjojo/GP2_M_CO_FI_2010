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


//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mdrv_types.h"
#include "mhal_scaler_reg.h"
#include "mhal_utility.h"
#include "mhal_od.h"

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures Start
//-------------------------------------------------------------------------------------------------
extern unsigned long irq_flags_swbk;
extern spinlock_t switch_bk_lock;
extern U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Macro definition
//--------------------------------------------------------------------------------------------------
#define OPT_OD_MHAL_DBG    0

#if (OPT_OD_MHAL_DBG)
#define OD_MHAL_PRINT(fmt, args...)      printk("[Mhal_SC][%05d] " fmt, __LINE__, ## args)
#else
#define OD_MHAL_PRINT(fmt, args...)
#endif

#define assert(p)   do {\
	if (!(p)) {\
		printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
				__FILE__, __LINE__, #p);\
	}\
} while (0)

//-------------------------------------------------------------------------------------------------
//  Definition
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////
// Michu OD
// Turn OD function on/OFF
////////////////////////////////////////////////////////////////////////
void MHal_SC_OverDriverSwitch( BOOL SwitchOnOff )
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_OD);
    //REG_WI(REG_SC_OD(0x10), SwitchOnOff, BIT0);
    if ( SwitchOnOff )
        REG_WL(REG_SC_OD(0x10), 0x2D);
    else
        REG_WL(REG_SC_OD(0x10), 0x2C);
    SC_BK_RESTORE;
}
void debugodtable(void )
{
    U8 y = 0;
    U16 wCount;


    printk("\n");
    OS_Delayms(20);    
    printk("OD = 0x%x\n", REG_RL(REG_SC_OD(0x10)));
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_OD);
    REG_WL(REG_SC_OD(0x01),0x0E);
    SC_BK_RESTORE;
    for (wCount=0; wCount<272; wCount++)
    {
        SC_BK_STORE;
        SC_BK_SWICH(REG_SC_BK_OD);
        REG_WR(REG_SC_OD(0x02), wCount|0x4000);
        while(REG_RH(REG_SC_OD(0x02)) & BIT6);
        SC_BK_RESTORE;
        OS_Delayms(20);
        printk("OD read back: REG_SC_OD(0x04) = 0x%x\n", REG_RL(REG_SC_OD(0x04)));

        y++;
        if ( ( y % 8 ) == 0x00 )
           printk("\n");
    }
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_OD);
    REG_WL(REG_SC_OD(0x01),0x00);
    SC_BK_RESTORE;
    printk("\n");
    OS_Delayms(20);
}

void MHal_SC_OD_Init(U32 OdBufferAddr, U32 OdMsdLimitAddr, U32 OdLsbBufferAddr, U32 OdLsdLimitAddr, const U8* pODTbl)
{
    U16 wCount;
    U8 ucTARGET;
    //U8* pODTbl = tOverDrive;
    //U8 i = 0;

    REG_WI(REG_CHIPTOP(0x1A), ENABLE, BIT2);
    MHal_SC_OverDriverSwitch( DISABLE );

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_OD);
    REG_WL(REG_SC_OD(0x01), 0x0E);

    ucTARGET=*(pODTbl+9);
    for (wCount=0; wCount<272; wCount++)
    {
        REG_WL(REG_SC_OD(0x03), (wCount == 9)?ucTARGET:(ucTARGET ^ *(pODTbl+wCount)));
        //i=(wCount == 9)?ucTARGET:(ucTARGET ^ *(pODTbl+wCount));
        REG_WR(REG_SC_OD(0x02), wCount|0x8000);
        //printk("OD Init = 0x%0x \n", i);
        //printk("OD Init = 0x%0x \n", *(pODTbl+wCount));
        //i++;
        //if ( (i % 8)  == 0x00 ) printk("\n");
        while(REG_RH(REG_SC_OD(0x02)) & BIT7);
    }
    ucTARGET=*(pODTbl+272+19);
    for (wCount=0; wCount<272; wCount++)
    {
        REG_WL(REG_SC_OD(0x06), (wCount == 19)?ucTARGET:(ucTARGET ^ *(pODTbl+272+wCount)));
        REG_WR(REG_SC_OD(0x05), wCount|0x8000);
        while(REG_RH(REG_SC_OD(0x05)) & BIT7);
    }
    ucTARGET=*(pODTbl+272*2+29);
    for (wCount=0; wCount<256; wCount++)
    {
        REG_WL(REG_SC_OD(0x09), (wCount == 29)?ucTARGET:(ucTARGET ^ *(pODTbl+272*2+wCount)));
        REG_WR(REG_SC_OD(0x08), wCount|0x8000);
        while(REG_RH(REG_SC_OD(0x08)) & BIT7);
    }
    ucTARGET=*(pODTbl+272*2+256+39);
    for (wCount=0; wCount<256; wCount++)
    {
        REG_WL(REG_SC_OD(0x0c), (wCount == 39)?ucTARGET:(ucTARGET ^ *(pODTbl+272*2+256+wCount)));
        REG_WR(REG_SC_OD(0x0b), wCount|0x8000);
        while(REG_RH(REG_SC_OD(0x0d)) & BIT7);
    }

    REG_WL(REG_SC_OD(0x01),0x00);

    REG_W3(REG_SC_OD(0x15), (OdBufferAddr) );
    REG_W3(REG_SC_OD(0x17), (OdMsdLimitAddr) );

    REG_W3(REG_SC_OD(0x4f), (OdLsbBufferAddr) );
    REG_WH(REG_SC_OD(0x50), (OdLsdLimitAddr) );

    REG_W3(REG_SC_OD(0x39), (OdLsdLimitAddr) );
    REG_W3(REG_SC_OD(0x3b), (OdLsdLimitAddr) );

    REG_WR(REG_SC_OD( 0x1C ), 0x4020 );//request read FIFO limit threshold
    REG_WR(REG_SC_OD( 0x1A ), 0x4020 );


    REG_WI(REG_SC_OD(0x10), ENABLE, BIT5);
    REG_L_WM(REG_SC_OD(0x10), 0x0C, 0x0E);
    //REG_L_WM(REG_SC_OD(0x10), 0x0A, 0x0E );

    REG_WI(REG_SC_OD(0x10), ENABLE, BIT0);
    //REG_WL(REG_SC_OD(0x10), 0x2D);

    //REG_L_WM(REG_SC_OD(0x12), 0x0b, 0x3f);
    //REG_H_WM(REG_SC_OD(0x12), 0x80, 0x80);

    SC_BK_RESTORE;
}

