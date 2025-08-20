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

#define MS_IR_C

/******************************************************************************/
/*                    Header Files                                            */
/* ****************************************************************************/
#include <intrins.h>
#include "DataType.h"
#include "hwreg.h"
//#include "drvSys.h"

#include "msIR.h"


U8 data g_bIrStartReceived = FALSE;
U16 data g_wIrPulseCounter = 0;
U8 data g_ucIrPulseCntIndex = 0;
U8 data g_ucIrByteIndex = 0;
U8 data g_ucIrRepeatTimer = 0;
U8 data g_bIrDetect = FALSE;
U8 data g_bIrRepeat = FALSE;

U32 data gIRKey;
U8 data gIRCount;

U8 msIR_GetIrKeyData( void )
{
    U8 IrKeyData;

    IrKeyData = XBYTE[IR_KEY];
    XBYTE[IR_FIFO_READ_PULSE] = 0x01;

    return(IrKeyData);
}


//*************************************************************************
//Function name:    msIR_WriteByte
//Passing parameter: U16 u16RegIndex:   register address
//                   U8  u8Value:       value
//Return parameter:     none
//Description: write 2 register values
//*************************************************************************
void msIR_WriteByte(U16 u16RegIndex, U8 u8Value)
{
    XBYTE[u16RegIndex] = u8Value;
}

//*************************************************************************
//Function name:    msIR_Write2Byte
//Passing parameter: U16 u16RegIndex:   register address
//                   U16 u16Value:      value
//Return parameter:     none
//Description: write 2 register values
//*************************************************************************
void msIR_Write2Byte( U16 u16RegIndex, U16 u16Value )
{
    XBYTE[u16RegIndex]     = LOWBYTE(u16Value);
    XBYTE[u16RegIndex + 1] = HIGHBYTE(u16Value);
}

//*************************************************************************
//Function name:    msIR_Write3Byte
//Passing parameter: U16 u16Regndex:   register address
//                   U32 u32Value:   value
//Return parameter:     none
//Description: write 3 register values
//*************************************************************************
void msIR_Write3Byte( U16 u16Regndex, U32 u32Value )
{
    XBYTE[u16Regndex]     = VARBYTE(u32Value, 3);
    XBYTE[u16Regndex + 1] = VARBYTE(u32Value, 2);
    XBYTE[u16Regndex + 2] = VARBYTE(u32Value, 1);
}

U8 msIR_ReadByte( U16 u16RegIndex )
{
    return XBYTE[u16RegIndex];
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void msIR_Initialize(U8 irclk_mhz)
{

    msIR_WriteByte(IR_CTRL, IR_LEADER_CODE_CHECKING_OPTION);     // enable control
    msIR_WriteByte(IR_CHECK, 0X01);    // enable timerout

    #define msIR_GetCnt(time, tolerance) \
    ((time) * (100UL + (tolerance)) * (irclk_mhz) / ((irclk_mhz) + 1) / 100)

    // header code upper bound
    msIR_Write2Byte(IR_HDC_UPB_L, msIR_GetCnt(IR_HEADER_CODE_TIME, IR_HEADER_CODE_TIME_UB));
    // header code lower bound
    msIR_Write2Byte(IR_HDC_LOB_L, msIR_GetCnt(IR_HEADER_CODE_TIME, IR_HEADER_CODE_TIME_LB));
    // off code upper bound
    msIR_Write2Byte(IR_OFC_UPB_L, msIR_GetCnt(IR_OFF_CODE_TIME, IR_OFF_CODE_TIME_UB));
    // off code lower bound
    msIR_Write2Byte(IR_OFC_LOB_L, msIR_GetCnt(IR_OFF_CODE_TIME, IR_OFF_CODE_TIME_LB));
    // off code repeat upper bound
    msIR_Write2Byte(IR_OFC_RP_UPB_L, msIR_GetCnt(IR_OFF_CODE_RP_TIME, IR_OFF_CODE_RP_TIME_UB));
    // off code repeat lower bound
    msIR_Write2Byte(IR_OFC_RP_LOB_L, msIR_GetCnt(IR_OFF_CODE_RP_TIME, IR_OFF_CODE_RP_TIME_LB));
    // logical 0/1 high upper bound
    msIR_Write2Byte(IR_LG01H_UPB_L, msIR_GetCnt(IR_LOGI_01H_TIME, IR_LOGI_01H_TIME_UB));
    // logical 0/1 high lower bound
    msIR_Write2Byte(IR_LG01H_LOB_L, msIR_GetCnt(IR_LOGI_01H_TIME, IR_LOGI_01H_TIME_LB));
    // logical 0 upper bound
    msIR_Write2Byte(IR_LG0_UPB_L, msIR_GetCnt(IR_LOGI_0_TIME, IR_LOGI_0_TIME_UB));
    // logical 0 lower bound
    msIR_Write2Byte(IR_LG0_LOB_L, msIR_GetCnt(IR_LOGI_0_TIME, IR_LOGI_0_TIME_LB));
    // logical 1 upper bound
    msIR_Write2Byte(IR_LG1_UPB_L, msIR_GetCnt(IR_LOGI_1_TIME, IR_LOGI_1_TIME_UB));
    // logical 1 lower bound
    msIR_Write2Byte(IR_LG1_LOB_L, msIR_GetCnt(IR_LOGI_1_TIME, IR_LOGI_1_TIME_LB));
    // Ir timerout counter
    msIR_Write3Byte(IR_TIMEOUT_CYC_0, msIR_GetCnt(IR_TIMEOUT_CYC, 0) + 0x300000UL);

    msIR_WriteByte(IR_CKDIV_NUM_REG, irclk_mhz);

    msIR_WriteByte(IR_CCODE_L, IR_HEADER_CODE0);
    msIR_WriteByte(IR_CCODE_H, IR_HEADER_CODE1);

    msIR_WriteByte(IR_CODEBYTE, 0x14);      //ir cc code byte enable,ir_code byte = 4
    msIR_WriteByte(IR_GLHRM_NUM_L, 0x04);   // GLHRM number[7:0]

#if(IR_MODE_SEL == IR_TYPE_FULLDECODE_MODE)
    msIR_WriteByte(IR_FIFO_CTRL, 0x0F);     // {2'b0,IR_SHOT_SEL[1:0],IR_FIFO_FULL_EN,FIFO_DEPTH[2:0]}
    msIR_WriteByte(IR_GLHRM_NUM_H, 0x38);   // {IR_DECOMODE[1:0], GLHRM_EN,GLHRM_NUM[10:8]}
#elif(IR_MODE_SEL == IR_TYPE_RAWDATA_MODE)
    msIR_WriteByte(IR_FIFO_CTRL, 0x0F);     // {2'b0,IR_SHOT_SEL[1:0],IR_FIFO_FULL_EN,FIFO_DEPTH[2:0]}
    msIR_WriteByte(IR_GLHRM_NUM_H, 0x28);   // {IR_DECOMODE[1:0], GLHRM_EN,GLHRM_NUM[10:8]}
#elif(IR_MODE_SEL == IR_TYPE_SWDECODE_MODE)
    msIR_WriteByte(IR_FIFO_CTRL, 0x2F);     // Only NSHOT edge detect for counter
    msIR_WriteByte(IR_GLHRM_NUM_H, 0x18);
#endif
}

#if 0
/******************************************************************************/
/// IR get key value and repeat flag
/// @param pkey \b IN return the key value
/// @param pu8flag \b IN return the repeat flag(1:Repeat)
/******************************************************************************/
BOOLEAN msIR_GetIRKeyCode(U8 *pkey, U8 *pu8flag)
{
    if (g_bIrDetect)
    {
        if(LOWBYTE(gIRKey >> 16) == ~(HIGHBYTE(gIRKey >> 16)))
        {
            *pkey = LOWBYTE(gIRKey >> 16);
            *pu8flag = 0;
            g_bIrDetect = 0;
            gIRKey = 0;
            return MSRET_OK;
        }
    }
    return MSRET_ERROR;
}


void MDrv_IR_SW_Isr(U8 *pkey)
{

    if(0 == g_bIrDetect)
    {
        if(XBYTE[IR_SHOT_CNT_1] & 0x08)
        {
            gIRKey = gIRKey >> 1 | 0x80000000;
        }
        else
        {
            gIRKey = gIRKey >> 1;
        }

        if(LOWBYTE(gIRKey) == IR_HEADER_CODE0)
        {
            if(HIGHBYTE(gIRKey) == IR_HEADER_CODE1)
            {
                g_bIrDetect = 1;
            }

        }
    }
    else if (1 == g_bIrDetect)
    {
        if(LOWBYTE(gIRKey >> 16) == ~(HIGHBYTE(gIRKey >> 16)))
        {
            *pkey = LOWBYTE(gIRKey >> 16);
        }
        gIRKey = 0;
        g_bIrDetect = 0;
    }

}
#endif
void msIR_Clear_FIFO(void)
{
    #if((IR_MODE_SEL==IR_TYPE_RAWDATA_MODE) || (IR_MODE_SEL==IR_TYPE_FULLDECODE_MODE))
    U8 i;

    for ( i = 0; i < 8; i++ )
    {
        U8 garbage;

        if ( XBYTE[IR_RPT_FIFOEMPTY] & 0x2 )
        {
            break;
        }

        garbage = msIR_GetIrKeyData();

        garbage = XBYTE[IR_RPT_FIFOEMPTY];
    }
    #endif
}

