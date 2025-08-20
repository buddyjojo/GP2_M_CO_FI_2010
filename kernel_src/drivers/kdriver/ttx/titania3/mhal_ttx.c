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
#define _MHAL_TTX_C
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ttx_.c
/// This file contains the Mstar driver  for Teletext
/// @author MStar Semiconductor Inc.
/// @brief  Teletext module
///////////////////////////////////////////////////////////////////////////////


#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/atomic.h>
#include "chip_int.h" // LGE drmyung 081013

#include "mhal_ttx.h"
#include "mhal_ttx_reg.h"
#include "mhal_ttx_types.h"

#include "mdrv_types.h"

U32 _VBIBufferBaseAddrTTX = 0;
U32 _VBIBufferUnitTTX = 0;

atomic_t _aInterruptStop = ATOMIC_INIT(1);

U8 _u8Byte2InvByte[256] =
{
0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0, // 00-15
0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8, // 16-31
0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4, // 32-47
0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc, // 48-63
0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2, // 64-
0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff
};

void MHal_TTX_SetVideoSystem(VIDEOSTANDARD_TYPE etVideoStandard)
{
    //U_INT8 u8Bank;

    //u8Bank = msReadByte(0x00);
    //msWriteByte(0x00,0x03);

    if(etVideoStandard == E_VIDEOSTANDARD_SECAM)
    {
        _MHal_W1B(BK_VBI_89, _MHal_R1B(BK_VBI_89)|(_BIT6));
        // for VPS
        _MHal_W1B(BK_VBI_89, _MHal_R1B(BK_VBI_89)|_BIT5);
        _MHal_W1B(BK_VBI_99, 0x6D);
        _MHal_W1B(BK_VBI_9A, 0x9A);
    	//putstr("SECAM\r\n");
    }
    else if(etVideoStandard == E_VIDEOSTANDARD_PAL_N)
    {
        _MHal_W1B(BK_VBI_82, 0x10);
        _MHal_W1B(BK_VBI_83, 0xB9);
        _MHal_W1B(BK_VBI_89, _MHal_R1B(BK_VBI_89)|_BIT6);
        // for VPS
        _MHal_W1B(BK_VBI_89, _MHal_R1B(BK_VBI_89)&(~_BIT5));
        _MHal_W1B(BK_VBI_99, 0x8C);
        _MHal_W1B(BK_VBI_9A, 0x01);
    	//putstr("PAL N\r\n");
    }
    else
    {
        _MHal_W1B(BK_VBI_82, 0x8E);
        _MHal_W1B(BK_VBI_83, 0x6B);
        _MHal_W1B(BK_VBI_89, _MHal_R1B(BK_VBI_89)|_BIT6);
        // for VPS
        _MHal_W1B(BK_VBI_89, _MHal_R1B(BK_VBI_89)&(~_BIT5));
        _MHal_W1B(BK_VBI_99, 0x8C);
        _MHal_W1B(BK_VBI_9A, 0x01);
    	TTX_HAL_DBG("PAL\n");
    }

    //msWriteByte(0x00,u8Bank);
}

void MHal_TTX_InitVBI(void)
{
    // close caption slicer threshold mode
    _MHal_W1B(BK_VBI_40, 0x01);

    // close caption line start 1 (lower 3 bits) = 0
    // close caption lin end 1 = 0
    _MHal_W1B(BK_VBI_41, 0x00);

    // close caption line start 2 = 0
    // close caption CRI zero crossing type :  positive edge
    // close caption clock run-in amplitude upper threshold (upper 2 bits) = 0b01
    _MHal_W1B(BK_VBI_50, 0x60); //F2

    // close caption line end 2 = 0
    // close caption multi-line acquisition mode : 1
    // close caption zero crossing mode : normal.
    // close caption SYNC Found enable mode : 1
    _MHal_W1B(BK_VBI_51, 0xA0);

    // teletext clock run-in amplitude accumulation start point. : 0b00010001
    // For eye-height testing
    _MHal_W1B(BK_VBI_77, 0x11);

    // teletext clock run-in amplitude accumulation start point. : 0b00011010
    // For SuperVHS decode issue
    //_MHal_W1B(BK_VBI_77, 0x1A);

    // teletext VBI line start 1 (odd field) : 0b00100
    // teletext VBI line end (lower 3 bits) 0b000
    _MHal_W1B(BK_VBI_7C, 0x04);

    // teletext data line end 1 (odd field) : 0b10110
    // teletext slicer read mode : 0b1
    // teletext framing code error bond value : 0b0 fully match framing code.
    // teletext framing code windows mode : 0b0
    _MHal_W1B(BK_VBI_7D, 0x36);

    // teletext data line start 2 (even field) : 0b00100
    // teletext slicer threshold fixing mode : 0b0 adjust automatically according to TtSidDetSel
    // teletext slicer level mode : 0b0 original mode.
    // teletext initial packet counter : 0b1 packet counter increases when teletext packet is detected without upper-bound.
    _MHal_W1B(BK_VBI_7E, 0x84);

    // teletext data line end 2 (even field) : 0b10110
    // teletext single line point mode : 0b11 Enable ttslptrmode, start from the line when previous line is no teletext.
    // teletext base address source selecion : 0b1
    _MHal_W1B(BK_VBI_7F, 0xF6);

    _MHal_W1B(BK_VBI_81, 0x52);
    _MHal_W1B(BK_VBI_86, 0xD6); //96
    _MHal_W1B(BK_VBI_89, 0xC2);
    _MHal_W1B(BK_VBI_8A, 0x42);
    _MHal_W1B(BK_VBI_8B, 0x24); // 84
    _MHal_W1B(BK_VBI_8D, 0xA5); // 95
    _MHal_W1B(BK_VBI_C4, 0x32);
    _MHal_W1B(BK_VBI_CB, 0xC4);
    _MHal_W1B(BK_VBI_CC, 0xBD);

    // For VPS detect speed up
    _MHal_W1B(BK_VBI_B4, 0x42);
    _MHal_W1B(BK_VBI_B5, 0x61);
    _MHal_W1B(BK_VBI_BB, 0x06); // 26

    // For VPS data index
    _MHal_W1B(BK_VBI_B6, 0x01 | 0x02 << 4);
    _MHal_W1B(BK_VBI_B7, 0x03 | 0x04 << 4);

    _MHal_W1B(BK_VBI_70,0x80); // enable VPS/WSS //C0
}

void  MHal_TTX_PacketBuffer_Create ( void )
{
    U32 Addr = ((TT_VBI_BUFFER_BASE_ADDR&0x1FFFFFFF)>>3);

    // Initial VBI Buffer Start Address
    _MHal_W2B(VBI_BASEADDR_L, (Addr&0xFFFF));
    _MHal_W1B(VBI_BASEADDR_L+4, (Addr>>16));

    TTX_HAL_DBG("Slicer buf addr:0x%x\n" ,Addr);
    TTX_HAL_DBG("Slicer buf size: 0x%x\n" ,TT_VBI_BUFFER_SIZE);

    // Initial VBI Buffer Field Number
    _MHal_W2B(VBI_BUF_LEN, TT_VBI_BUFFER_UNIT);   // no need to minus 1, follow Venus design
    _MHal_W2B(VBI_BUF_LEN_H, (TT_VBI_BUFFER_UNIT>>8));   // no need to minus 1, follow Venus design.

    TTX_HAL_DBG("Slicer entry: 0x%x\n",TT_VBI_BUFFER_UNIT);

    #if 1
    TTX_HAL_DBG("Slicer buf addr: 0x%x\n", TT_VBI_BUFFER_BASE_ADDR&0xFFFFFFFF);
    TTX_HAL_DBG("Slicer buf size: 0x%x\n",TT_VBI_BUFFER_SIZE);
    TTX_HAL_DBG("Slicer entry: 0x%x\n",TT_VBI_BUFFER_UNIT);
    #endif

}


void MHal_TTX_OnOffVBISlicer(B16 on)
{

    #ifdef TTX_DEBUG
    TTX_HAL_DBG("Calling msAPI_TTX_OnOffVBISlicer %bd\r\n", on);
    #endif

    if(on)
    {
        _MHal_W1B(TT_ENABLE,0x01); // enable TT VBI slicer
    }
    else
    {
        _MHal_W1B(TT_ENABLE,0x00); // disable TT VBI slicer
    }

}

U16 MHal_TTX_ReadWSS(void)
{
    u8 wWssWordL, wWssWordH;

    wWssWordL = _MHal_R1B(BK_VBI_CD);
    wWssWordH = _MHal_R1B(BK_VBI_CE);

    return MAKEWORD(wWssWordH, wWssWordL);

}

B16 MHal_TTX_GatherVpsData(U8* pu8buf)
{   static U8 u8Count = 1;

    switch (u8Count)
    {
    case 1:
        pu8buf[0] = _MHal_R1B(BK_VBI_A6);
        pu8buf[1] = _MHal_R1B(BK_VBI_A7);
        pu8buf[2] = _MHal_R1B(BK_VBI_AD);
        pu8buf[3] = _MHal_R1B(BK_VBI_AE);

        _MHal_W1B(BK_VBI_B6, 0x05 | 0x06 << 4);
        _MHal_W1B(BK_VBI_B7, 0x07 | 0x08 << 4);

        u8Count++;
        break;
    case 2:
        pu8buf[4] = _MHal_R1B(BK_VBI_A6);
        pu8buf[5] = _MHal_R1B(BK_VBI_A7);
        pu8buf[6] = _MHal_R1B(BK_VBI_AD);
        pu8buf[7] = _MHal_R1B(BK_VBI_AE);

        _MHal_W1B(BK_VBI_B6, 0x09 | 0x0A << 4);
        _MHal_W1B(BK_VBI_B7, 0x0B | 0x0C << 4);

        u8Count++;
        break;
    case 3:
        pu8buf[8] = _MHal_R1B(BK_VBI_A6);
        pu8buf[9] = _MHal_R1B(BK_VBI_A7);
        pu8buf[10] = _MHal_R1B(BK_VBI_AD);
        pu8buf[11] = _MHal_R1B(BK_VBI_AE);

        _MHal_W1B(BK_VBI_B6, 0x0D | 0x0E << 4);
        _MHal_W1B(BK_VBI_B7, 0x0F);

        u8Count++;
        break;
    case 4:
        pu8buf[12] = _MHal_R1B(BK_VBI_A6);
        pu8buf[13] = _MHal_R1B(BK_VBI_A7);
        pu8buf[14] = _MHal_R1B(BK_VBI_AD);

        _MHal_W1B(BK_VBI_B6, 0x01 | 0x02 << 4);
        _MHal_W1B(BK_VBI_B7, 0x03 | 0x04 << 4);

        u8Count = 1;
        break;
    default:
        break;
    }

    //if (u8Count == 1)
        return TRUE;
    //else
    //    return FALSE;
}


U16 MHal_TTX_ReadVpsCni(void)
{
    U8 ucVpsDataH, ucVpsDataL;
    U8 ucBK_VBI_A6, ucBK_VBI_A7, ucBK_VBI_AD, ucBK_VBI_AE;

    U8 *pInvBuf;

    ucVpsDataH = 0x00;
    ucVpsDataL = 0x00;
    pInvBuf = &_u8Byte2InvByte[0];

    //MDrv_Timer_Delayms(45);		// 1-frame period pluse 5ms margin. VPS is received once each frame.
    ucBK_VBI_A6 = _MHal_R1B(BK_VBI_A6);  // byte 5          ,selected by BK8_B6[3:0]
    ucBK_VBI_A7 = _MHal_R1B(BK_VBI_A7);  // byte 11 (0xB),selected by BK8_B6[7:4]
    ucBK_VBI_AD = _MHal_R1B(BK_VBI_AD); // byte 13 (0xD),selected by BK8_B7[3:0]
    ucBK_VBI_AE = _MHal_R1B(BK_VBI_AE);  // byte 14 (0xE),selected by BK8_B7[7:4]
    ucVpsDataL = 0x00;
    ucVpsDataH = 0x00;

    /*
    for (ucLoop = _BIT7; ucLoop!=_BIT1; ucLoop>>=1)	// extract byte 14[2:7] as ucVpsDataL[5:0]
    {
        ucVpsDataL >>= 1;
        if (ucBK_VBI_AE&ucLoop)
            ucVpsDataL |= _BIT7;
    } // for
    */
    ucVpsDataL = (pInvBuf[(ucBK_VBI_AE>>2)] >> 2);

    /*
    for (ucLoop = _BIT1; ucLoop!=0; ucLoop>>=1)		// extract byte 11[0:1] as ucVpsDataL[7:6]
    {
        ucVpsDataL >>= 1;
        if (ucBK_VBI_A7&ucLoop)
            ucVpsDataL |= _BIT7;
    } // for
    */
    ucVpsDataL = ucVpsDataL | (pInvBuf[(ucBK_VBI_A7&0x03)] & 0xC0);

    /*
    for (ucLoop = _BIT1; ucLoop!=0; ucLoop>>=1)		// extract byte 14[0:1] as ucVpsDataH[1:0]
    {
        ucVpsDataH >>= 1;
        if (ucBK_VBI_AE&ucLoop)
            ucVpsDataH |= _BIT3;
    } // for
    */
    ucVpsDataH = (pInvBuf[ucBK_VBI_AE&0x03]>>6);


    /*
    for (ucLoop = _BIT7; ucLoop!=_BIT5; ucLoop>>=1)	// extract byte-13[6:7] as ucVpsDataH[3:2]
    {
        ucVpsDataH >>= 1;
        if (ucBK_VBI_AD&ucLoop)
            ucVpsDataH |= _BIT3;
    } // for
    */
    ucVpsDataH = ucVpsDataH | ((pInvBuf[(ucBK_VBI_AD>>6)&0x03]>>6)<<2);
    return MAKEWORD(ucVpsDataH, ucVpsDataL);
}



U32 MHal_TTX_ReadIRQ(void)
{
    return _MHal_R1B(VBI_INTERRUPT_STATUS);
}

void MHal_TTX_ClearIRQ(void)
{
    _MHal_W1BM(VBI_INTERRUPT_CLEAR, _BIT4|_BIT1|_BIT0, _BIT4|_BIT1|_BIT0);
    //mdelay(1);
    _MHal_W1BM(VBI_INTERRUPT_CLEAR, 0, _BIT4|_BIT1|_BIT0);
}

void MHal_TTX_EnableInterrupt(B16 bEnable)
{
    if(bEnable)
    {
        atomic_set(&_aInterruptStop, 0x0);
        _MHal_W1BM(IRQ_MASK4, 0x00, _BIT4);
        //msleep(1);
        //udelay(100);
        _MHal_W1BM(VBI_INTERRUPT_MASK, 0, _BIT4|_BIT1|_BIT0);

    }
    else
    {
        atomic_set(&_aInterruptStop, 0x1);
        _MHal_W1BM(VBI_INTERRUPT_MASK, _BIT4|_BIT1|_BIT0, _BIT4|_BIT1|_BIT0);
        //msleep(1);
        //udelay(100);
        _MHal_W1BM(IRQ_MASK4, 0x10, _BIT4);

        /*
        if(free_irq(VBI_IRQ_NUM, MDrv_TTX_Interrupt, SA_INTERRUPT, "VBI_IRQ", NULL) != 0)
        {
            TTX_HAL_DBG("IRQ request failure\n");
            return -EIO;
        }
        */
    }
}



U32 MHal_TTX_VPS_Count(void)
{
    return (U32)(_MHal_R1B(VBI_VPS_COUNT)>>4);
}

U32 MHal_TTX_WSS_Count(void)
{
    return (U32)(_MHal_R1B(VBI_WSS_COUNT)&0x07);
}

U32 MHal_TTX_TXX_Count(void)
{
    return (_MHal_R1B(VBI_TTX_COUNTL) + (_MHal_R1B(VBI_TTX_COUNTH)<<8));
}

// MStar andy 081110
B16 MHal_TTX_IsValidPalSignal(void)
{
    //TTX_HAL_DBG("0x%0x\n", _MHal_R1B(BK_AFEC_CC));
    if ((_MHal_R1B(BK_AFEC_CC) & 0xE0) != 0x40)
        return TRUE;

    return FALSE;
}

