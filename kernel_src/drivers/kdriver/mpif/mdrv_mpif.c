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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_mpif.c
/// @brief  MPIF Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include "mdrv_types.h"
#include "mdrv_mpif.h"
#include "mhal_mpif.h"

static U8 checksum = MPIF_CHECKSUM_ENABLE;
static U8 rtx = MPIF_MAX_RTX_0;
static spinlock_t _spinlock_mpif;

#define WAIT_TIMEOUT		  (MASTER_CLOCK_HZ>>12)//0xFFFFFFFF	


//Not that first setp, the commend width and data width of MPIF and SPIF must be the same at init state,
//then MPIF can change SPIF commend width and data width
BOOL MDrv_MPIF_SetCmdDataWidth(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth)
{
    U16 u16data;
	U8 u8olddata, u8data;
	BOOL bRes = FALSE;
	
    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

    //---- Configure SPIF first ---------
    if(!MHal_MPIF_LC1A_RW(0, u8slaveid, MPIF_LC1A_INDEX_1, &u8olddata))
		goto SET_CMD_DATAWIDTH_END;
    u8data = MPIF_2X_FROMSPIF;
    if(!MHal_MPIF_LC1A_RW(1, u8slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto SET_CMD_DATAWIDTH_END;
	
    if(!MHal_MPIF_LC2A_RW(0, u8slaveid, checksum, rtx, REG_SPIF_MISC1, &u16data))
        goto SET_CMD_DATAWIDTH_END;
    u16data &= ~0xF0;
    u16data |= (((U16)(u8cmdwidth & 0x03) << 4) | ((U16)(u8datawidth & 0x03) << 6));
    if(!MHal_MPIF_LC2A_RW(1, u8slaveid, checksum, rtx, REG_SPIF_MISC1, &u16data))
        goto SET_CMD_DATAWIDTH_END;

	u8data = u8olddata;
    if(!MHal_MPIF_LC1A_RW(1, u8slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto SET_CMD_DATAWIDTH_END;

    //---- Then change MPIF setting -----------
    bRes = MHal_MPIF_SetCmdDataMode(u8slaveid, u8cmdwidth, u8datawidth);

SET_CMD_DATAWIDTH_END:
	
	spin_unlock_irqrestore(&_spinlock_mpif,spflag);

    return bRes;
}

BOOL MDrv_MPIF_SafeSetCmdDataWidth(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth)
{
    U16 u16data;
    U8 _u8cmdwidth, _u8datawidth;
    BOOL bFoundCorrectWidthSettings = FALSE;

    for (_u8cmdwidth = 0; _u8cmdwidth < 4; _u8cmdwidth++)
    {
        for (_u8datawidth = 0; _u8datawidth < 4; _u8datawidth++)
        {
            printk("test cmd: %u data: %u\n", _u8cmdwidth, _u8datawidth);
            //---- Change MPIF setting -----------
            MHal_MPIF_SetCmdDataMode(u8slaveid, _u8cmdwidth, _u8datawidth);


            //---- Configure SPIF ---------
            if(!MHal_MPIF_LC2A_RW(0, u8slaveid, checksum, rtx, REG_SPIF_MISC1, &u16data))
                continue;   // cmdwidth/datawidth incorrect
            u16data &= ~0xF0;
            u16data |= (((U16)(_u8cmdwidth & 0x03) << 4) | ((U16)(_u8datawidth & 0x03) << 6));
            if(!MHal_MPIF_LC2A_RW(1, u8slaveid, checksum, rtx, REG_SPIF_MISC1, &u16data))
                continue;   // cmdwidth/datawidth incorrect
            //---------------------------------

            bFoundCorrectWidthSettings = TRUE;
            goto CHANGE_CMD_DATA_WIDTH;
        }
    }

    if (!bFoundCorrectWidthSettings)
        return FALSE;

CHANGE_CMD_DATA_WIDTH:
    printk("Current cmd width=%u, data width=%u\n", _u8cmdwidth, _u8datawidth);

    MDrv_MPIF_InitSPIF(u8slaveid);

    if (!MDrv_MPIF_SetCmdDataWidth(u8slaveid, u8cmdwidth, u8datawidth)) {
        return FALSE;
    }

    printk("New cmd width=%u, data width=%u\n", u8cmdwidth, u8datawidth);
    return TRUE;
}

static BOOL MDrv_MPIF_Set2Xmode_FromXIU(U8 slaveid, BOOL bFromXIU)
{
    U8 u8data;
	BOOL bRes = FALSE;
	
    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	 

    // set SPIF from XIU
    if (bFromXIU)
        u8data = MPIF_2X_FROMXIU;
    else
        u8data = MPIF_2X_FROMSPIF;

    bRes = MDrv_MPIF_LC1A_RW(1, slaveid, MPIF_LC1A_INDEX_1, &u8data);

#if 0   // check 2X mode is set
    printk("check MPIF_2X:");
    if(!MDrv_MPIF_LC1A_RW(0, slaveid, MPIF_LC1A_INDEX_1, &u8data))
    {
        printk("fail1\n");
        return FALSE;
    }

    if ((bFromXIU && u8data == MPIF_2X_FROMXIU) || (!bFromXIU && u8data == MPIF_2X_FROMSPIF))
        printk("ok\n");
    else
        printk("fail2\n");
#endif

	spin_unlock_irqrestore(&_spinlock_mpif, spflag); 

    return bRes;
}

BOOL MDrv_MPIF_InitSPIF(U8 u8slaveid)
{
    U8 u8data, i;
	BOOL bRes = FALSE;

    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

    u8data = 0x80;
    // MPIF reset SPIF with 1A-write transaction twice consecutively
    // according to PIF rules
    for (i=0; i<2; i++)
    {
        if(!MHal_MPIF_LC1A_RW(1, u8slaveid, MPIF_LC1A_INDEX_0, &u8data))
            goto INIFSPIF_END;
    }
    // force 2X mode always from XIU
    bRes = MDrv_MPIF_Set2Xmode_FromXIU(u8slaveid, TRUE);

INIFSPIF_END:

	spin_unlock_irqrestore(&_spinlock_mpif, spflag); 

    return bRes;
}

BOOL MDrv_MPIF_SetSlave_ClkInv_Delay(U8 u8slaveid, U8 u8DlyBufNum)
    {
	BOOL bRes;

    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

	bRes = MHal_MPIF_SetSlave_ClkInv_Delay(u8slaveid, u8DlyBufNum);

	spin_unlock_irqrestore(&_spinlock_mpif, spflag ); 

	return bRes;
}

BOOL MDrv_MPIF_Init(U8 u8clk, U8 u8trc, U8 u8wc)
{
	BOOL bRes;
    U32 spflag ;
	spin_lock_init(&_spinlock_mpif);
	
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

	bRes = MHal_MPIF_Init(u8clk, u8trc, u8wc);

	spin_unlock_irqrestore(&_spinlock_mpif, spflag ); 
	
    return bRes;
}

BOOL MDrv_MPIF_LC1A_RW(U8 u8bWite, U8 slaveid, U8 index, U8* pu8data)
{
	BOOL bRes;

    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

	bRes = MHal_MPIF_LC1A_RW(u8bWite, slaveid, index, pu8data);

	spin_unlock_irqrestore(&_spinlock_mpif, spflag); 
	
    return bRes;
}

BOOL MDrv_MPIF_LC2A_RW(U8 u8bWite, U8 slaveid, U16 addr, U16 *pu16data)
{
	BOOL bRes;

    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 
	
	bRes = MHal_MPIF_LC2A_RW(u8bWite, slaveid, checksum, rtx, addr, pu16data);

	spin_unlock_irqrestore(&_spinlock_mpif, spflag);
	
    return bRes;
}

BOOL MDrv_MPIF_LC2B_RW(U8 u8bWite, U8 slaveid, U16 addr, U16 *pu16data)
{
	BOOL bRes;

	spin_lock(&_spinlock_mpif);

	bRes = MHal_MPIF_LC2B_RW(u8bWite, slaveid, checksum, rtx, addr, pu16data);

	spin_unlock(&_spinlock_mpif);
	
    return bRes;
}

BOOL MDrv_MPIF_LC3A_RIU_RW(MPIF_LC3XRIU_PARAM* p3xriu)
{		
	U16 u16data;
	U8 u8olddata, u8data;
	BOOL bRes = FALSE;
	
    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

	if(!MHal_MPIF_LC1A_RW(0, p3xriu->slaveid, MPIF_LC1A_INDEX_1, &u8olddata))
		goto LC3A_RIU_END;
    u8data = MPIF_2X_FROMSPIF;
    if(!MHal_MPIF_LC1A_RW(1, p3xriu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC3A_RIU_END;	

	if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC3ATX, WAIT_TIMEOUT))
    	goto LC3A_RIU_END;   	

	//----------------------- Configure SPIF------------------------------		
	if(!MHal_MPIF_GetSPIF_ChBusySts(p3xriu->slaveid, SPIF_LC3A_BUSY, WAIT_TIMEOUT))
		goto LC3A_RIU_END;  	

	u16data = 1;
	if(!MHal_MPIF_LC2A_RW(1, p3xriu->slaveid, checksum, rtx, REG_SPIF_LC3A_PACKET_LEN, &u16data))  
		goto LC3A_RIU_END;	

	u16data = (MS_U16)(p3xriu->u8bWite & 0x01) | (((MS_U16)p3xriu->u8chksun & 0x01) << 1); //from RIU
	if(!MHal_MPIF_LC2A_RW(1, p3xriu->slaveid, checksum, rtx, REG_SPIF_LC3A_PACKET_CONTROL, &u16data))  
		goto LC3A_RIU_END;		
	//-------------------------------------------------------------------	
	
	bRes = MHal_MPIF_LC3A_RIURW(p3xriu->u8bWite, p3xriu->slaveid, p3xriu->u8chksun, p3xriu->u8fastmode,
		p3xriu->u8retrx, p3xriu->u8retrx_limit, p3xriu->u8wcnt, p3xriu->pu8data, p3xriu->u8datalen);

LC3A_RIU_END:

	u8data = u8olddata;
	if(!MHal_MPIF_LC1A_RW(1, p3xriu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC3A_RIU_END;	

	spin_unlock_irqrestore(&_spinlock_mpif, spflag );

	return bRes;
}

BOOL MDrv_MPIF_LC3B_RIU_RW(MPIF_LC3XRIU_PARAM* p3xriu)
{		
	U16 u16data;
	U8 u8olddata, u8data;
	BOOL bRes = FALSE;
	
    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	
	
	if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC3BTX, WAIT_TIMEOUT))
    	goto LC3B_RIU_END;    

	//----------------------- Configure SPIF------------------------------		
    if(!MHal_MPIF_LC1A_RW(0, p3xriu->slaveid, MPIF_LC1A_INDEX_1, &u8olddata))
		goto LC3B_RIU_END;
    u8data = MPIF_2X_FROMSPIF;
    if(!MHal_MPIF_LC1A_RW(1, p3xriu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC3B_RIU_END;
	
	if(!MHal_MPIF_GetSPIF_ChBusySts(p3xriu->slaveid, SPIF_LC3B_BUSY, WAIT_TIMEOUT))
		goto LC3B_RIU_END;  

	u16data = 1;
	if(!MHal_MPIF_LC2B_RW(1, p3xriu->slaveid, checksum, rtx, REG_SPIF_LC3B_PACKET_LEN, &u16data))  
		goto LC3B_RIU_END;	

	u16data = (MS_U16)(p3xriu->u8bWite & 0x01) | (((MS_U16)p3xriu->u8chksun & 0x01) << 1); //from RIU
	if(!MHal_MPIF_LC2B_RW(1, p3xriu->slaveid, checksum, rtx, REG_SPIF_LC3B_PACKET_CONTROL, &u16data))  
		goto LC3B_RIU_END;		
	//-------------------------------------------------------------------	
	
	bRes = MHal_MPIF_LC3B_RIURW(p3xriu->u8bWite, p3xriu->slaveid, p3xriu->u8chksun, p3xriu->u8fastmode,
		p3xriu->u8retrx, p3xriu->u8retrx_limit, p3xriu->u8wcnt, p3xriu->pu8data, p3xriu->u8datalen);

LC3B_RIU_END:

	u8data = u8olddata;
    if(!MHal_MPIF_LC1A_RW(1, p3xriu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC3B_RIU_END;

	spin_unlock_irqrestore(&_spinlock_mpif, spflag);

	return bRes;
}

BOOL MDrv_MPIF_LC3A_MIU_RW(MPIF_LC3XMIU_PARAM* p3xmiu)
{
	U16 u16data;
	U8 u8olddata, u8data;
	BOOL bRes = FALSE;
	
    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

	if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC3ATX, WAIT_TIMEOUT))
    	goto LC3A_MIU_END;

	//----------------------- Configure SPIF------------------------------	
	if(!MHal_MPIF_LC1A_RW(0, p3xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8olddata))
		goto LC3A_MIU_END;
    u8data = MPIF_2X_FROMSPIF;
    if(!MHal_MPIF_LC1A_RW(1, p3xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC3A_MIU_END;
	
    if(!MHal_MPIF_GetSPIF_ChBusySts(p3xmiu->slaveid, SPIF_LC3A_BUSY, WAIT_TIMEOUT))
        goto LC3A_MIU_END;
	
    u16data = (((MS_U16)p3xmiu->u8bWite) & 0x01) | ((((MS_U16)p3xmiu->u8chksun) & 0x01) << 1) | 0x04; //MIU 0				
    if(!MHal_MPIF_LC2A_RW(1, p3xmiu->slaveid, checksum, rtx, REG_SPIF_LC3A_PACKET_CONTROL, &u16data))
        goto LC3A_MIU_END;
	
    u16data = (MS_U16)(p3xmiu->u32datalen >> 4); //unit is 16 bytes
    if(!MHal_MPIF_LC2A_RW(1, p3xmiu->slaveid, checksum, rtx, REG_SPIF_LC3A_PACKET_LEN, &u16data))
        goto LC3A_MIU_END;
	
    u16data = (MS_U16)((p3xmiu->u32spif_mdr >> 3) & 0xFFFF);
    if(!MHal_MPIF_LC2A_RW(1, p3xmiu->slaveid, checksum, rtx, REG_SPIF_LC3A_MADR_L, &u16data))
        goto LC3A_MIU_END;
	
    u16data = (MS_U16)((p3xmiu->u32miuaddr >> 19) & 0x1FF);
    if(!MHal_MPIF_LC2A_RW(1, p3xmiu->slaveid, checksum, rtx, REG_SPIF_LC3A_MADR_H, &u16data))
        goto LC3A_MIU_END;
	
    //--------------------------------------------------------------------	
	
	bRes =  MHal_MPIF_LC3A_MIURW(p3xmiu->u8bWite, p3xmiu->slaveid, p3xmiu->u8chksun, p3xmiu->u8fastmode,
		p3xmiu->u8retrx, p3xmiu->u8retrx_limit, p3xmiu->u8wcnt, p3xmiu->u32miuaddr, p3xmiu->u32spif_mdr,
		p3xmiu->u32datalen);

LC3A_MIU_END:

	u8data = u8olddata;
	if(!MHal_MPIF_LC1A_RW(1, p3xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC3A_MIU_END;

	spin_unlock_irqrestore(&_spinlock_mpif, spflag);

	return bRes;
}

BOOL MDrv_MPIF_LC3B_MIU_RW(MPIF_LC3XMIU_PARAM* p3xmiu)
{
	U16 u16data;
	U8 u8olddata, u8data;
	BOOL bRes = FALSE;
	
    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

	if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC3BTX, WAIT_TIMEOUT))
    	goto LC3B_MIU_END;

	//----------------------- Configure SPIF------------------------------	
	if(!MHal_MPIF_LC1A_RW(0, p3xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8olddata))
		goto LC3B_MIU_END;
    u8data = MPIF_2X_FROMSPIF;
    if(!MHal_MPIF_LC1A_RW(1, p3xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC3B_MIU_END;
	
    if(!MHal_MPIF_GetSPIF_ChBusySts(p3xmiu->slaveid, SPIF_LC3B_BUSY, WAIT_TIMEOUT))
        goto LC3B_MIU_END;
	
    u16data = (((MS_U16)p3xmiu->u8bWite) & 0x01) | ((((MS_U16)p3xmiu->u8chksun) & 0x01) << 1) | 0x04; //MIU 0				
    if(!MHal_MPIF_LC2B_RW(1, p3xmiu->slaveid, checksum, rtx, REG_SPIF_LC3B_PACKET_CONTROL, &u16data))
        goto LC3B_MIU_END;
	
    u16data = (MS_U16)(p3xmiu->u32datalen >> 4); //unit is 16 bytes
    if(!MHal_MPIF_LC2B_RW(1, p3xmiu->slaveid, checksum, rtx, REG_SPIF_LC3B_PACKET_LEN, &u16data))
        goto LC3B_MIU_END;
	
    u16data = (MS_U16)((p3xmiu->u32spif_mdr >> 3) & 0xFFFF);
    if(!MHal_MPIF_LC2B_RW(1, p3xmiu->slaveid, checksum, rtx, REG_SPIF_LC3B_MADR_L, &u16data))
        goto LC3B_MIU_END;
	
    u16data = (MS_U16)((p3xmiu->u32miuaddr >> 19) & 0x1FF);
    if(!MHal_MPIF_LC2B_RW(1, p3xmiu->slaveid, checksum, rtx, REG_SPIF_LC3B_MADR_H, &u16data))
        goto LC3B_MIU_END;	
    //--------------------------------------------------------------------	
	
	bRes =  MHal_MPIF_LC3B_MIURW(p3xmiu->u8bWite, p3xmiu->slaveid, p3xmiu->u8chksun, p3xmiu->u8fastmode,
		p3xmiu->u8retrx, p3xmiu->u8retrx_limit, p3xmiu->u8wcnt, p3xmiu->u32miuaddr, p3xmiu->u32spif_mdr,
		p3xmiu->u32datalen);

LC3B_MIU_END:

	u8data = u8olddata;
	if(!MHal_MPIF_LC1A_RW(1, p3xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC3B_MIU_END;

	spin_unlock_irqrestore(&_spinlock_mpif, spflag);

	return bRes;
}

BOOL MDrv_MPIF_LC4A_MIU_RW(MPIF_LC4A_PARAM* p4xmiu)
{
	U16 u16data;
	U8 u8olddata, u8data;	
	
    U32 spflag ;
	spin_lock_irqsave(&_spinlock_mpif, spflag); 	

	//----------------------- Configure SPIF------------------------------	
	if(!MHal_MPIF_LC1A_RW(0, p4xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8olddata))
		goto LC4A_END;
    u8data = MPIF_2X_FROMSPIF;
    if(!MHal_MPIF_LC1A_RW(1, p4xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC4A_END;
	 
    if(!MHal_MPIF_GetSPIF_ChBusySts(p4xmiu->slaveid, SPIF_LC4A_BUSY, WAIT_TIMEOUT))
        goto LC4A_END;
	
    u16data = (((MS_U16)p4xmiu->u8bWite) & 0x01) | ((MS_U16)(p4xmiu->u8granular & 0x03) << 1) | ((MS_U16)(p4xmiu->u8miusel &0x01) << 4); 
    if(!MHal_MPIF_LC2A_RW(1, p4xmiu->slaveid, checksum, rtx, REG_SPIF_LC4A_CONTROL, &u16data))
        goto LC4A_END;

	u16data = p4xmiu->u32datalen;
    if(!MHal_MPIF_LC2A_RW(1, p4xmiu->slaveid, checksum, rtx, REG_SPIF_LC4A_STREAM_LEN, &u16data))
        goto LC4A_END;
	
    u16data = (MS_U16)((p4xmiu->u32spif_mdr >> 3) & 0xFFFF);
    if(!MHal_MPIF_LC2A_RW(1, p4xmiu->slaveid, checksum, rtx, REG_SPIF_LC4A_MADR_L, &u16data))
        goto LC4A_END;
	
    u16data = (MS_U16)((p4xmiu->u32miuaddr >> 19) & 0x1FF);
    if(!MHal_MPIF_LC2A_RW(1, p4xmiu->slaveid, checksum, rtx, REG_SPIF_LC4A_MADR_H, &u16data))
        goto LC4A_END;	
    //--------------------------------------------------------------------	   
	
	p4xmiu->bRet = MHal_MPIF_LC4A_MIURW(p4xmiu->u8bWite, p4xmiu->slaveid, p4xmiu->u8retrx_limit, p4xmiu->u8miusel,
		p4xmiu->u8granular, p4xmiu->u8wcnt, p4xmiu->u32miuaddr, p4xmiu->u32spif_mdr, p4xmiu->u32datalen);

LC4A_END:

	u8data = u8olddata;
	if(!MHal_MPIF_LC1A_RW(1, p4xmiu->slaveid, MPIF_LC1A_INDEX_1, &u8data))
		goto LC4A_END;
	
	spin_unlock_irqrestore(&_spinlock_mpif, spflag);

	return p4xmiu->bRet;
}

EXPORT_SYMBOL(MDrv_MPIF_InitSPIF);
EXPORT_SYMBOL(MDrv_MPIF_Init);
EXPORT_SYMBOL(MDrv_MPIF_LC1A_RW);
EXPORT_SYMBOL(MDrv_MPIF_LC2A_RW);
EXPORT_SYMBOL(MDrv_MPIF_SafeSetCmdDataWidth);
EXPORT_SYMBOL(MDrv_MPIF_SetCmdDataWidth);
EXPORT_SYMBOL(MDrv_MPIF_LC2B_RW);
EXPORT_SYMBOL(MDrv_MPIF_LC3A_RIU_RW);
EXPORT_SYMBOL(MDrv_MPIF_LC3B_RIU_RW);
EXPORT_SYMBOL(MDrv_MPIF_LC3A_MIU_RW);
EXPORT_SYMBOL(MDrv_MPIF_LC3B_MIU_RW);
EXPORT_SYMBOL(MDrv_MPIF_LC4A_MIU_RW);

