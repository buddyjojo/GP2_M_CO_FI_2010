////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
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


/******************************************************************************/
/*                    Header Files                        */
/* ****************************************************************************/
#include "datatype.h"
#include "hwreg.h"
#include "drvGlobal.h"
#include "mdrv_pm.h"

PM_CMD gPmCMD;

/********************************************************************************/
/*                   Local Function Prototypes                  */
/********************************************************************************/

/******************************************************************************/
///Write 1 byte data to M4 Lite register
///@param u16Addr \b IN  register address
///@param u8Data \b IN  8 bits (1 byte) value to write
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
/******************************************************************************/
U8 MDrv_PM_RegWrite( U16 u16Addr, U8 u8Data )
{
    PM_CMD   send_write_cmd, recv_write_cmd;

    gPmCMD.Preamble++;

    send_write_cmd.Opcode = WRITE_CMD;

    MDrv_WriteByte(REG_HK2PM_MBOX1 + 1, (U8)(u16Addr >> 8));
    MDrv_WriteByte(REG_HK2PM_MBOX1 + 0, (U8)(u16Addr));

    // Clear PM2HK_MBOX0 interrupt
    MDrv_WriteByte(REG_PM_INT_STATUS + 1, BIT1);
#if __C51__
    MDrv_WriteByte(REG_HK2PM_MBOX0 + 1, u8Data);
    MDrv_WriteByte(REG_HK2PM_MBOX0 + 0, (gPmCMD.Preamble<<6) | (send_write_cmd.Opcode));
#else
    MDrv_Write2Byte(REG_HK2PM_MBOX0, (u8Data << 8) | (gPmCMD.Preamble << 6) | (send_write_cmd.Opcode));
#endif

    //polling interrput status
    while( !MDrv_ReadRegBit(REG_PM_INT_STATUS + 1, BIT1) )
        /* forever */;

    recv_write_cmd.Preamble = (MDrv_ReadByte(REG_PM2HK_MBOX0 + 0) & 0xC0) >> 6;
    recv_write_cmd.Opcode = MDrv_ReadByte(REG_PM2HK_MBOX0 + 0);

    // Clear PM2HK_MBOX0 interrupt
    MDrv_WriteByte(REG_PM_INT_STATUS + 1, BIT1);

    if( gPmCMD.Preamble == recv_write_cmd.Preamble \
        && 0x3F == recv_write_cmd.Opcode )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/******************************************************************************/
///Read 1 byte data from M4 Lite register
///@param u16Addr \b IN  register address
///@return
///- 8 bits (1 byte) value for read
///- FALSE: Fail
/******************************************************************************/
U8 MDrv_PM_RegRead( U16 u16Addr )
{
    PM_CMD   send_read_cmd, recv_read_cmd;
    U8 u8Data;

    gPmCMD.Preamble++;

    send_read_cmd.Opcode = READ_CMD;

    MDrv_WriteByte(REG_HK2PM_MBOX1 + 1, (U8)(u16Addr >> 8));
    MDrv_WriteByte(REG_HK2PM_MBOX1 + 0, (U8)(u16Addr));

    // Clear PM2HK_MBOX0 interrupt
    MDrv_WriteByte(REG_PM_INT_STATUS + 1, BIT1);

    MDrv_WriteByte(REG_HK2PM_MBOX0 + 1, (gPmCMD.Preamble<<6) | (send_read_cmd.Opcode));

    //polling interrput status
    while( !MDrv_ReadRegBit(REG_PM_INT_STATUS + 1, BIT1) )
        /* forever */;

    recv_read_cmd.Preamble = (MDrv_ReadByte(REG_PM2HK_MBOX0 + 0) & 0xC0) >> 6;
    recv_read_cmd.Opcode = MDrv_ReadByte(REG_PM2HK_MBOX0 + 0);
    u8Data = MDrv_ReadByte(REG_PM2HK_MBOX0 + 1);

    // Clear PM2HK_MBOX0 interrupt
    MDrv_WriteByte(REG_PM_INT_STATUS + 1, BIT1);

    if( gPmCMD.Preamble == recv_read_cmd.Preamble \
        && 0x3F == recv_read_cmd.Opcode )
    {
        return u8Data;
    }
    else
    {
        return FALSE;
    }
}

void MAdp_WriteByteMask( U16 u16Reg, U8 u8Val, U32 u8Msk )
{
	MDrv_WriteByteMask(u16Reg ,u8Val ,u8Msk );
}

U32 MAdp_Read3Byte( U16 u16Reg )
{
	return MDrv_Read3Byte( u16Reg );
}

