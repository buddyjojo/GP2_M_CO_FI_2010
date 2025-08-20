////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_mpif.c
/// @brief  MPIF Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//#include <linux/kernel.h>

#include "mdrv_types.h"
#include "mdrv_mpif.h"
#include "mhal_mpif.h"
#include "mdrv_mpif_io_uboot.h"

extern void	printf(const char *fmt, ...);
#define printk printf

static U8 checksum = MPIF_CHECKSUM_DISABLE;
static U8 rtx = MPIF_MAX_RTX_0;
BOOL MDrv_MPIF_SetCmdDataWidth(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth)
{
    U16 u16data;

    //---- Configure SPIF first ---------
    if(!MDrv_MPIF_LC2A_RW(0, u8slaveid, REG_SPIF_MISC1, &u16data))
        return FALSE;
    u16data &= ~0xF0;
    u16data |= (((U16)(u8cmdwidth & 0x03) << 4) | ((U16)(u8datawidth & 0x03) << 6));
    if(!MDrv_MPIF_LC2A_RW(1, u8slaveid, REG_SPIF_MISC1, &u16data))
        return FALSE;

    //---- Then change MPIF setting -----------
    MHal_MPIF_SetCmdDataMode(u8slaveid, u8cmdwidth, u8datawidth);

    //---- Check SPIF is config correctly ---------
    if(!MDrv_MPIF_LC2A_RW(0, u8slaveid, REG_SPIF_MISC1, &u16data))
        return FALSE;
    if ((((u16data >> 4) & 0x3) != u8cmdwidth) || (((u16data >> 6) & 0x3) != u8datawidth)) {
        printk("[SetCmdDataWidth fail]MPIF cmd: %u, data:%u, SPIF cmd: %u, data:%u\n",
            u8cmdwidth, u8datawidth, (u16data >> 4) & 0x3, (u16data >> 6) & 0x3);
        return FALSE;
    }
    return TRUE;
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
            if(!MDrv_MPIF_LC2A_RW(0, u8slaveid, REG_SPIF_MISC1, &u16data))
                continue;   // cmdwidth/datawidth incorrect
            u16data &= ~0xF0;
            u16data |= (((U16)(_u8cmdwidth & 0x03) << 4) | ((U16)(_u8datawidth & 0x03) << 6));
            if(!MDrv_MPIF_LC2A_RW(1, u8slaveid, REG_SPIF_MISC1, &u16data))
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
    // set SPIF from XIU
    if (bFromXIU)
        u8data = MPIF_2X_FROMXIU;
    else
        u8data = MPIF_2X_FROMSPIF;

    if(!MDrv_MPIF_LC1A_RW(1, slaveid, MPIF_LC1A_INDEX_1, &u8data))
    {
        printk("Set MPIF_2X_FROM%s fail\n", (bFromXIU? "XIU":"SPIF"));
        return FALSE;
    }

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

    return TRUE;
}

BOOL MDrv_MPIF_InitSPIF(U8 u8slaveid)
{
    U8 u8data, i;

    u8data = 0x80;
    // MPIF reset SPIF with 1A-write transaction twice consecutively
    // according to PIF rules
    for (i=0; i<2; i++)
    {
        if(!MHal_MPIF_LC1A_RW(1, u8slaveid, MPIF_LC1A_INDEX_0, &u8data))
            return FALSE;
    }
    // force 2X mode always from XIU
    if (!MDrv_MPIF_Set2Xmode_FromXIU(u8slaveid, TRUE))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL MDrv_MPIF_Init(U8 u8clk, U8 u8trc, U8 u8wc)
{
    return MHal_MPIF_Init(u8clk, u8trc, u8wc);
}

BOOL MDrv_MPIF_LC1A_RW(U8 u8bWite, U8 slaveid, U8 index, U8* pu8data)
{
    return MHal_MPIF_LC1A_RW(u8bWite, slaveid, index, pu8data);
}

BOOL MDrv_MPIF_LC2A_RW(U8 u8bWite, U8 slaveid, U16 addr, U16 *pu16data)
{
    return MHal_MPIF_LC2A_RW(u8bWite, slaveid, checksum, rtx, addr, pu16data);
}

BOOL MDrv_MPIF_LC2B_RW(U8 u8bWite, U8 slaveid, U16 addr, U16 *pu16data)
{
    return MHal_MPIF_LC2B_RW(u8bWite, slaveid, checksum, rtx, addr, pu16data);
}

