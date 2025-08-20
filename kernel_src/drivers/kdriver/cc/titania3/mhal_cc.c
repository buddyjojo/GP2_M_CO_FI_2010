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
#define _MHAL_CC_C
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_cc_.c
/// This file contains the Mstar driver  for Close Caption
/// @author MStar Semiconductor Inc.
/// @brief  Close Caption module
///////////////////////////////////////////////////////////////////////////////


#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/atomic.h>

#include "mhal_cc.h"
#include "mhal_cc_reg.h"
#include "mhal_cc_types.h"

#include "mdrv_types.h"

#ifndef	TTX_DEBUG	// added to remove warning(dreamer@lge.com)
#define	TTX_DEBUG	0
#endif

static atomic_t _aInterruptStop_CC;
void MHal_CC_SetVideoSystem(VIDEOSTANDARD_TYPE etVideoStandard)
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
    	CC_KDBG("PAL\n");
    }

    //msWriteByte(0x00,u8Bank);
}

void MHal_CC_InitVBI(void)
{
    // close caption slicer threshold mode
    //_MHal_W1B(BK_VBI_40, 0x01);
	_MHal_W1B(BK_VBI_40, 0x10);
    // close caption line start 1 (lower 3 bits) = 0
    // close caption lin end 1 = 0
    //_MHal_W1B(BK_VBI_41, 0x00);
    _MHal_W1B(BK_VBI_41, 0xF3);

    // close caption line start 2 = 0
    // close caption CRI zero crossing type :  positive edge
    // close caption clock run-in amplitude upper threshold (upper 2 bits) = 0b01
    //_MHal_W1B(BK_VBI_50, 0x60); //F2
	_MHal_W1B(BK_VBI_50, 0xEF);
    // close caption line end 2 = 0
    // close caption multi-line acquisition mode : 1
    // close caption zero crossing mode : normal.
    // close caption SYNC Found enable mode : 1
    //_MHal_W1B(BK_VBI_51, 0xA0);
    _MHal_W1B(BK_VBI_51, 0xB3);
#if 0
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

    _MHal_W1B(BK_VBI_70,0x80); // enable VPS/WSS //C0
	#endif
}



#if 1
void MHal_CC_OnOffVBISlicer(B16 on)
{

    #if TTX_DEBUG
    CC_KDBG("Calling msAPI_CC_OnOffVBISlicer %bd\r\n", on);
    #endif

    if(on)
    {
        //printk("enable cc vbi slicer\n");
        _MHal_W1BM(BK_VBI_46,0x01, 0x01); // enable CC VBI slicer
    }
    else
    {
        //printk("disable cc vbi slicer\n");
        _MHal_W1BM(BK_VBI_46,0x00, 0x01); // enable CC VBI slicer
    }

}

#else
void MHal_CC_OnOffVBISlicer(B16 on)
{

    #if TTX_DEBUG
    CC_KDBG("Calling msAPI_CC_OnOffVBISlicer %bd\r\n", on);
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
#endif


U16  MHal_CC_GetPacketCount(void)
{
    return _MHal_R1B(BK_VBI_56)<<8 | _MHal_R1B(BK_VBI_5B);
}


U32 MHal_CC_ReadIRQ(void)
{
    return _MHal_R1B(VBI_INTERRUPT_STATUS);
}

void MHal_CC_ClearIRQ(void)
{
    _MHal_W1BM(VBI_INTERRUPT_CLEAR, _BIT2, _BIT2);

    _MHal_W1BM(VBI_INTERRUPT_CLEAR, 0, _BIT2);
}

void MHal_CC_EnableInterrupt(B16 bEnable)
{
	printk("MHal_CC_EnableInterrupt (%x)\n",bEnable);
    if(bEnable)
    {
        atomic_set(&_aInterruptStop_CC, 0x0);
        _MHal_W1BM(IRQ_MASK4, 0x00, _BIT4);

        _MHal_W1BM(BK_VBI_1C, 1, _BIT0);

        _MHal_W1BM(VBI_INTERRUPT_MASK, 0, _BIT2);
    }
    else
    {
        atomic_set(&_aInterruptStop_CC, 0x1);
        _MHal_W1BM(VBI_INTERRUPT_MASK, _BIT2, _BIT2);

        _MHal_W1BM(BK_VBI_1C, 0, _BIT0);

        _MHal_W1BM(IRQ_MASK4, 0x10, _BIT4);
}
}


void MHal_CC_SetSlicingRegion(U16 u16TopLineStart, U16 u16TopLineEnd, U16 u16BtnLineStart, U16 u16BtnLineEnd, BOOL IsNTSC)
{
    if (IsNTSC)//( (_MHal_R2B(BK_AFEC_CC) & 0xE0) == 0x40) // if (BK_AFEC_CC[7:5] == 010) means NTSC, need -3
    {
        u16TopLineStart = ((u16TopLineStart & 0xFF) - 3);
        u16TopLineEnd   = ((u16TopLineEnd   & 0xFF) - 3);
        u16BtnLineStart = ((u16BtnLineStart & 0xFF) - 3);
        u16BtnLineEnd   = ((u16BtnLineEnd   & 0xFF) - 3);
    }
    else //PAL
    {
        u16TopLineStart = (u16TopLineStart & 0xFF);
        u16TopLineEnd   = (u16TopLineEnd   & 0xFF);
        u16BtnLineStart = (u16BtnLineStart & 0xFF);
        u16BtnLineEnd   = (u16BtnLineEnd   & 0xFF);
    }

    _MHal_W1BM(BK_VBI_40, u16TopLineStart << 1, 0x30);
    _MHal_W1BM(BK_VBI_41, u16TopLineStart << 5, 0xE0);
    _MHal_W1BM(BK_VBI_41, u16TopLineEnd, 0x1F);
    _MHal_W1BM(BK_VBI_50, u16BtnLineStart, 0x1F);
    _MHal_W1BM(BK_VBI_51, u16BtnLineEnd, 0x1F);
}


void MHal_CC_SetSlicingSys(VIDEOSTANDARD_TYPE system_type)
{
	switch(system_type)
	{
		case E_VIDEOSTANDARD_PAL_BGHI:	// PAL
		{
			_MHal_W1BM(BK_VBI_42, 0x23, 0x3F);
			_MHal_W1BM(BK_VBI_44, 0x47, 0xFF);
			_MHal_W1BM(BK_VBI_4B, 0x22, 0x3F);
			_MHal_W1BM(BK_VBI_4D, 0xB0, 0xFF);
		}
		break;
		case E_VIDEOSTANDARD_NTSC_M:	//NTSC
		{
			_MHal_W1BM(BK_VBI_42, 0x1C, 0x3F);
			_MHal_W1BM(BK_VBI_44, 0x39, 0xFF);
			_MHal_W1BM(BK_VBI_4B, 0x24, 0x3F);
			_MHal_W1BM(BK_VBI_4D, 0x8E, 0xFF);
		}
		break;
		case E_VIDEOSTANDARD_PAL_M:		//PAL M
		{
			_MHal_W1BM(BK_VBI_42, 0x1C, 0x3F);
			_MHal_W1BM(BK_VBI_44, 0x39, 0xFF);
			_MHal_W1BM(BK_VBI_4B, 0x26, 0x3F);
			_MHal_W1BM(BK_VBI_4D, 0x8E, 0xFF);
		}
		break;
		case E_VIDEOSTANDARD_PAL_N:		//PAL Nc
		{
			_MHal_W1BM(BK_VBI_42, 0x1C, 0x3F);
			_MHal_W1BM(BK_VBI_44, 0x39, 0xFF);
			_MHal_W1BM(BK_VBI_4B, 0x16, 0x3F);
			_MHal_W1BM(BK_VBI_4D, 0x8E, 0xFF);
		}
		break;		
		case E_VIDEOSTANDARD_PAL_60:	//NTSC443
		{
			_MHal_W1BM(BK_VBI_42, 0x23, 0x3F);
			_MHal_W1BM(BK_VBI_44, 0x47, 0xFF);
			_MHal_W1BM(BK_VBI_4B, 0x32, 0x3F);
			_MHal_W1BM(BK_VBI_4D, 0xB0, 0xFF);
		}
		break;
		default:
			CC_KDBG("%s is invoked and invalid system is selected\n", __FUNCTION__);
		break;
	}
}
