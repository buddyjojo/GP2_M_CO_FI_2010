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
#include <linux/kernel.h>   /* printk() */
#include <linux/delay.h>

#include "mdrv_types.h"
#include "mdrv_scaler_st.h"
#include "mhal_hdmi.h"
#include "mhal_hdmi_reg.h"
#include "mhal_ddc2bi_reg.h"
#include "mhal_adc_reg.h"
#include "mhal_scaler_reg.h"
#include "mhal_utility.h"



//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#define HDMI_MHAL_DGB(fmt, args...)  printk(" " fmt, ##args)
//-------------------------------------------------------------------------------------------------
//  Constant
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void MHal_HDMI_Init( void )
{
    // Power up all clocks for DVI/HDMI/HDCP
    REG_WR(REG_DVI_ATOP(0x06), 0x00); // clock power down control
    //REG_WM(REG_PM_SLEEP(0x27), 0x00, (BIT0|BIT1|BIT2|BIT3)); //hot plug init
    REG_WM(REG_PM_SLEEP(0x27), 0xF0, 0xFF); //hot plug init

    // DVI setting
    REG_WM(REG_DVI_DTOP(0x29), 0xE300, 0xFF00); // [15]:update Bch slowly; [14:8]: 0x63(100 lines)
    REG_WR(REG_DVI_DTOP(0x2A), 0xE3E3); // [15:8]: update Rch slowly, [7:0]:update Gch slowly
    REG_WI(REG_DVI_DTOP(0x1E), ENABLE, BIT4); // enable Vsync glitch filter
    REG_WI( REG_DDC2BI(0x3A), TRUE, BIT5 ); // HDCP enable for ddc

    // [5:4] = 2'b 01, Audio FIFO 1/4 full
    REG_WM(REG_HDMI_CONFIG, BIT4, (BIT5|BIT4));
    // [15]: black level when YCbCr, [14]: DC info, [13]: Audio flat bit check, [11:10]: Blanking when AVMUTE, [9]: HDMI/DVI mode HW detect
    REG_WH(REG_HDMI_CONFIG1, 0xEE);
    // [1:0]: enable HDMI deep color mode
    REG_WL(REG_HDMI_CONFIG2, 0x03);
    // AUPLL2X enable for HDMI deep color mode
    REG_WM(REG_AUDIO(0x12), BIT5, BIT5);

    // [7]: Frame repetition manual mode, [3]: auto DSD detection, [1]: auto non-PCM detection
    REG_WL(REG_HDMI_RESET_PACKET, (BIT7|BIT3|BIT1));

    MHal_HDMI_AudioOutput( ENABLE );

    // turn off all the interrupt
    //REG_WR(REG_HDMI_INT_MASK, 0xFFFF);

    // DDC must to be enabled if HDMI is enabled
    REG_WM(REG_DDC2BI(0x4A), 0, 0x80);
    REG_WM(REG_DDC2BI(0x4B), 0, 0x8080);
    REG_WM(REG_DDC2BI(0x4C), 0, 0x80);
#if 0 // remove this patch because no need in T3
    //victor 20081229, DVI+HDCP snow noise patch
    //FitchHsu 20081113 fix DVI clock unlcok issue
    REG_WL(REG_DVI_DTOP(0x13), 0x52);
#endif
}

void MHal_HDMI_DVIClock_Enable(BOOL bEnable, SC_INPUT_SOURCE_e enInput)
{
    switch(enInput)
    {
        case INPUT_SOURCE_HDMI_A:
            REG_WM(REG_PM_SLEEP(0x4B), (bEnable ? 0: BIT11), BIT11);
            break;
        case INPUT_SOURCE_HDMI_B:
            REG_WM(REG_PM_SLEEP(0x4B), (bEnable ? 0: BIT12), BIT12);
            break;
        case INPUT_SOURCE_HDMI_C:
            REG_WM(REG_PM_SLEEP(0x4B), (bEnable ? 0: BIT14), BIT14);
            break;
        case INPUT_SOURCE_HDMI_D:
            REG_WM(REG_PM_SLEEP(0x4B), (bEnable ? 0: BIT13), BIT13);
            break;
        default:
            break;
    }
}

void MHal_HDMI_Reset(void)
{
    //reset HDMI
    REG_WM(REG_DVI_ATOP(0x07), BIT4|BIT5|BIT6, 0xFF);
    REG_WM(REG_DVI_ATOP(0x07), 0x00, 0xFF);
}

#if 0
void MHal_HDMI_SetBlackLevel(U32 ace_level_gain, U32 ace_level_offset)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_DLC);
    REG_WR(REG_SC_DLC(0x0F), ace_level_gain);
    REG_WR(REG_SC_DLC(0x14), ace_level_offset);
    SC_BK_RESTORE;
}

//=================================================
//  HDCP
//=================================================
void MHal_HDCP_Enable(BOOL bEnable)
{
    REG_WI( REG_DVI_ATOP( 0x60 ), !bEnable, BIT13 );
    REG_WI( REG_DVI_ATOP( 0x60 ), !bEnable, BIT14 );
    REG_WI( REG_DVI_ATOP( 0x69 ), !bEnable, BIT13 );

    if(bEnable)
    {
        //Enable HPD
    }
    else
    {
        REG_WL( REG_HDCP(0x01), 0x00);
        //Disable HPD
    }
}
#endif

void MHal_HDCP_InitProductionKey(U8* pu8HdcpKeyData)		// 081112 wlgnsl99 change hpcp key write by victor
{
    U16 i, reg_val;
	//printk("MHal_HDCP_InitProductionKey %x %x %x %x %x \n",pu8HdcpKeyData[0],pu8HdcpKeyData[1],pu8HdcpKeyData[2],pu8HdcpKeyData[3],pu8HdcpKeyData[4]);
    REG_WM(REG_PM_SLP(0x21), 0x02, 0x1F);
    reg_val = REG_RR(REG_DDC2BI(0x3A)) & ~0x23;
    REG_WR(REG_DDC2BI(0x3A), reg_val | BIT1 | BIT3 | BIT5);//victor 20081113, HDCP ,wlgnsl99 sky life DVI 입력시 HDMI 나오지 않는 문제 수정.

    for (i = 0; i < 5; i++)
    {
        REG_WR(REG_DDC2BI(0x3B) , i);
        REG_WR(REG_DDC2BI(0x3D), BIT2);

        REG_WR(REG_DDC2BI(0x3C), *pu8HdcpKeyData);
        pu8HdcpKeyData += 1;
        REG_WR(REG_DDC2BI(0x3D), BIT1);
		msleep(1);// wait for sram writing down
    }

    // Bcaps
    REG_WR(REG_DDC2BI(0x3B), 0x0040);               // Write Address
    REG_WR(REG_DDC2BI(0x3D), 0x0004);
    REG_WR(REG_DDC2BI(0x3C), 0x0080);
    REG_WR(REG_DDC2BI(0x3D), 0x0002);

    reg_val = REG_RR(REG_DDC2BI(0x3A)) & ~0x23;
    REG_WR(REG_DDC2BI(0x3A), reg_val | BIT0 | BIT1 | BIT5 );

    // write HDCP key
    for (i = 0; i < 284; i++)
    {
        REG_WR(REG_DDC2BI(0x3B), i);
        REG_WR(REG_DDC2BI(0x3D), BIT2);
        REG_WR(REG_DDC2BI(0x3C), *(pu8HdcpKeyData));
        pu8HdcpKeyData += 1;
        REG_WR(REG_DDC2BI(0x3D), BIT1);
        //msleep(1);// wait for sram writing down  // delay 추가요인.
    }

    reg_val = REG_RR(REG_HDCP_00) | BIT0 | BIT5 | BIT8 | BIT9 | BIT10;
    REG_WR(REG_HDCP_00, reg_val);
}

#if 0
BOOL MHal_HDCP_Exist( void )
{
    U8 bIsExist = 0;

    bIsExist = (REG_RL(REG_HDCP_STATUS) & ( BIT0|BIT1|BIT2|BIT4|BIT5));


    if(bIsExist)
        return TRUE;
    else
        return FALSE;
}
#endif

void MHal_HDCP_ClearStatus(void)
{
    REG_WL(REG_HDCP_STATUS, 0x00);
}

void MHal_HDCP_SetEQ(U8 u8EqLevel)		// 081027 wlgnsi99 LGE : set HDMI EQ
{

	//printk("HOONI_EQEQEQ==[%d]",u8EqLevel);

	switch(u8EqLevel)
	{
		case EQ_NORMAL:
			REG_WM(REG_DVI_DTOP(0x20), 0x15, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0));

			break;

		case EQ_MAXIMUM:
			REG_WM(REG_DVI_DTOP(0x20), 0x3F, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0));

			break;

		case EQ_MINIMUM:
			REG_WM(REG_DVI_DTOP(0x20), 0x00, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0));

			break;

		case EQ_NORMAL_HIGH:
			REG_WM(REG_DVI_DTOP(0x20), 0x3F, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0));

			break;

		default:
			REG_WM(REG_DVI_DTOP(0x20), 0x15, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0));

			break;

	}

}

//=================================================
// HDMI HPD
//
// ****HPD Spec****
// HIGH Level 2.4 ~ 5.3V
// LOW Level 0 ~ 0.4V
// Enable hotplug GPIO[0] output and set output value
//
//=================================================
void MHal_HDMI_HPD_High(BOOL bHigh, SC_INPUT_SOURCE_e enInput)
{
    switch(enInput)
    {
        case INPUT_SOURCE_HDMI_A:
            REG_WM(REG_PM_SLEEP(0x27), (!bHigh <<4), (BIT4));   // 20090720 daniel.huang: change for LG board
            break;

        case INPUT_SOURCE_HDMI_B:
            REG_WM(REG_PM_SLEEP(0x27), (!bHigh <<5), (BIT5));   // 20090720 daniel.huang: change for LG board
            break;

        case INPUT_SOURCE_HDMI_C:
            REG_WM(REG_PM_SLEEP(0x27), (!bHigh <<6), (BIT6));   // 20090720 daniel.huang: change for LG board
            break;

        case INPUT_SOURCE_HDMI_D:
            REG_WM(REG_PM_SLEEP(0x27), (!bHigh <<7), (BIT7));   // 20090720 daniel.huang: change for LG board
            break;

        default: // For system first power on with the unknow HDMI port.
            break;
    }
}

#if 0
void MHal_HPD_PullHPD_Src1(BOOL HPD_INVERSE, U8 u8HighLow)
{
    if (HPD_INVERSE )
        REG_L_WM( REG_DVI_ATOP(0x68), u8HighLow ? BIT5 : 0, (BIT7 | BIT5) );
    else
        REG_L_WM( REG_DVI_ATOP( 0x68 ), u8HighLow ? 0 : BIT5, (BIT7 | BIT5) );
}

void MHal_HPD_PullHPD_Src2(BOOL HPD_INVERSE, U8 u8HighLow)
{
    if (HPD_INVERSE )
        REG_L_WM( REG_DVI_ATOP(0x68), u8HighLow ? BIT4 : 0, (BIT6 | BIT4) );
    else
        REG_L_WM( REG_DVI_ATOP( 0x68 ), u8HighLow ? 0 : BIT4, (BIT6 | BIT4) );
}

void MHal_HPD_PullHPD_Src3(BOOL HPD_INVERSE, U8 u8HighLow)
{
    if (HPD_INVERSE )
        REG_L_WM( REG_DVI_ATOP(0x6B), u8HighLow ? BIT4 : 0, (BIT5 | BIT4) );
    else
        REG_L_WM( REG_DVI_ATOP(0x6B), u8HighLow ? BIT4 : 0, (BIT5 | BIT4) );
}
#endif
//=================================================
//HDMI Audio Function
//=================================================
void MHal_HDMI_AudioOutput(BOOL bEnable)
{
    // Control HDMI audio PLL DCLK/FBCLK clock
    //thchen 20080815 for T2 settings
	if ( bEnable )
		{
			REG_WH(REG_HDMI_CONFIG3, (BIT2 | BIT0));
		}
		else
		{
			REG_WH(REG_HDMI_CONFIG3, 0x00);
		}
}

HAL_HDMI_MODE_RPT_e MHal_HDMI_GetModeRPT(void)
{
    return REG_RI(REG_HDCP_STATUS, BIT0);
}

void MHal_HDMI_ResetPacket(HDMI_REST_t breset)
{
    //FitchHsu 20090526 modify HDMI packet reset
    REG_WR(REG_HDMI_PLL_CTRL3, breset);
    REG_WR(REG_HDMI_PLL_CTRL3, 0x00);
}

U16 MHal_HDMI_GetHDMIST1(void)
{
    U16 u16ST1;
    u16ST1 = REG_RR(REG_HDMI_ST1);
    // Must clear the status bit, the bits will be 1 again after received next packet info.
    REG_WR(REG_HDMI_ST1, u16ST1);
    return u16ST1;
}

//victor 20081117, ACP
U8 MHal_HDMI_GetACPHB1(void)
{
    return REG_RL(REG_HDMI_ACP_HB1);
}

BOOL MHal_HDMI_IsAVMUTE(void)
{
    return (BOOL)(REG_RI(REG_HDMI_GCONTROL, BIT0));
}

HAL_HDMI_COLOR_FORMAT_e MHal_HDMI_GetPacketColorFormat(void)
{
    HAL_HDMI_COLOR_FORMAT_e format;
    switch (REG_RR(REG_HDMI_AVI_IF0) & 0x0060)
    {
        case 0x00:
            format = HDMI_COLOR_FORMAT_RGB;
            break;
        case 0x20:
            format = HDMI_COLOR_FORMAT_YUV_422;
            break;
        case 0x40:
            format = HDMI_COLOR_FORMAT_YUV_444;
            break;
        default:
            format = HDMI_COLOR_FORMAT_RGB;
            break;
    }
    return format;
}

SC_HDMI_COLORIMETRY_FORMAT_e MHal_HDMI_GetExtendColorimetryInfo(void)
{
    return ((REG_RR(REG_HDMI_AVI_IF0) & 0xC000) >> 14);
}

SC_HDMI_EXTENDEDCOLORIMETRY_FORMAT_e MHal_HDMI_GetPacketExtendedColorimetry(void)
{
    return ((REG_RR(REG_HDMI_AVI_IF1) & 0x30) >> 4);
}

void MHal_HDMI_GetGMPktGBDInfo(U8* pu8GBDInfo)
{
    U8 i;

    for(i=0; i<((GM_GBD_MAX_SIZE+0x1) & ~0x1); i+=2)
    {
        *pu8GBDInfo++ = REG_RL((REG_HDMI_GM_DATA0+(i/2)));
        *pu8GBDInfo++ = REG_RH((REG_HDMI_GM_DATA0+(i/2)));
    }
}

U8 MHal_HDMI_GetGMPktCurrentGamutSeq(void)
{
    return (REG_RH(REG_HDMI_GM_HB) & 0x0F);
}

U8 MHal_HDMI_GetAspectRatio(void)
{
    U16 u16Val;
    U8  u8AR = 0;
    u16Val = REG_RR(REG_HDMI_AVI_IF0);
	//printk("MHal_HDMI_GetAspectRatio====[0x%x]\n",u16Val);
	u8AR = (U8) ((u16Val & 0x3F00) >> 8);
	//printk("u8AR[2]====[0x%x]\n",u8AR);
    return u8AR;
}

BOOL MHal_HDMI_IsAudioNotPCM(void)
{
    return (BOOL)(REG_RR(REG_HDMI_CS0) & BIT6);
}

BOOL MHal_HDMI_IsChecksumOrBCHParityErr(void)
{
    return (BOOL)(REG_RR(REG_HDMI_ERR1) & (BIT2 | BIT1));
}

BOOL MHal_HDMI_IsAudioPacketErr(void)
{
    return (BOOL)(REG_RR(REG_HDMI_ERR1) & (BIT4 | BIT5));
}

void MHal_HDMI_ClearErr(void)
{
    U16 u16Err;
    u16Err = REG_RR(REG_HDMI_ERR1);
    REG_WR(REG_HDMI_ERR1, u16Err);
}
#if 0
void MHal_HDMI_SetPostSettingForHDMI(void)
{
    REG_WL(REG_HDMI_PLL_CTRL1, 0x00);
}

void MHal_HDMI_EnableDeep(void)
{
    //thchen 20080815 for T2 settings
        REG_WH(REG_HDMI_CONFIG1, 0x2E);
        REG_WL(REG_HDMI_CONFIG2, 0x03);
        REG_WM(REG_HDMI_CONFIG1, (BIT15|BIT14), (BIT15|BIT14));//thchen 20081209 for High Byte for deep color blanking area

}
#endif

//victor 20081215, DDC
void MHal_HDMI_EnableDDCBus(BOOL bEnable)
{
	//printk("MHal_HDMI_EnableDDCBus bEnable=%d !!!\n",bEnable);
	REG_WI( REG_DDC2BI(0x3A), bEnable, BIT5 ); // HDCP enable for ddc
}

#if 0 // remove this patch because no need in T3
//victor 20081229, DVI+HDCP snow noise patch
void MHal_DVI_AdjustADC(BOOL bClockLessThan70MHz)
{
    if (bClockLessThan70MHz)
    {   // DVI CLK is smaller than 70MHz
        //printk("Smaller than 70 MHz\n"); //thchen 20090117
        REG_L_WM(REG_DVI_ATOP(0x64), BIT7, BIT7);
        REG_H_WM(REG_DVI_ATOP(0x64), BIT0, BIT0);
    }
    else
    {
        //printk("Larger than 70 MHz\n"); //thchen 20090117
        REG_L_WM(REG_DVI_ATOP(0x64), 0, BIT7);
        REG_H_WM(REG_DVI_ATOP(0x64), 0, BIT0);
    }
}

//victor 20081229, DVI+HDCP snow noise patch
BOOL MHal_DVI_IsVideoStable(void)
{
    return REG_RH(REG_VIVALDI_BANK(0x19)) & BIT5;
}

//victor 20081229, DVI+HDCP snow noise patch
U16 MHal_DVI_TMDSClock(void)
{
    return REG_RR(REG_HDMI(0x11));
}

//victor 20081229, DVI+HDCP snow noise patch
U16 MHal_HDMI_InterruptStatus(void)
{
    return REG_RR(REG_HDMI_INT_STATUS);
}

//victor 20081229, DVI+HDCP snow noise patch
void MHal_HDMI_ResetPacketStatus(void)
{
	//printk("MHal_HDMI_ResetPacketStatus!!!\n");
    //thchen 20080815 for T2 settings
    REG_WH(REG_HDMI_PLL_CTRL3, ~BIT5); //thchen 20090115
    REG_WH(REG_HDMI_PLL_CTRL3, 0x00); //thchen 20090115
}
#endif

void MHal_HDMI_GetVSI(SC_GET_HDMI_VSI_PACKET_t *pvsipacket)
{
    U8 i;

    //low byte of VS_IF0 is checksum, [091130_Leo]
    pvsipacket->vsi_packet.IEERegId[0] = REG_RH(REG_HDMI_VS_IF0);
    pvsipacket->vsi_packet.IEERegId[1] = REG_RL(REG_HDMI_VS_IF1);
    pvsipacket->vsi_packet.IEERegId[2] = REG_RH(REG_HDMI_VS_IF1);
    //pvsipacket->vsi_packet.PayLoad[0] = REG_RL(REG_HDMI_VS_IF2);
    for (i = 0; i < 3; i++)
    {
        HDMI_MHAL_DGB("pvsipacket->vsi_packet.IEERegId[%d] = %x\n",i ,pvsipacket->vsi_packet.IEERegId[i]);
    }

    for (i = 0; i < 12; i++)
    {
        pvsipacket->vsi_packet.PayLoad[(2*i)]= REG_RL(( REG_HDMI_VS_IF2 +(2*i)));
        pvsipacket->vsi_packet.PayLoad[((2*i)+1) ]= REG_RH((REG_HDMI_VS_IF2 + (2*i)));
        HDMI_MHAL_DGB("pvsipacket->vsi_packet.PayLoad[%d] = %x\n",(2*i) ,pvsipacket->vsi_packet.PayLoad[(2*i)]);
        HDMI_MHAL_DGB("pvsipacket->vsi_packet.PayLoad[%d] = %x\n",(2*i)+1 ,pvsipacket->vsi_packet.PayLoad[(2*i)+1]);
    }

    for (i = 0; i < 14; i++)
    {
        pvsipacket->vsi_packet.packet.dataBytes[(2*i)]= REG_RL(( REG_HDMI_VS_IF0 +(2*i)));
        pvsipacket->vsi_packet.packet.dataBytes[((2*i)+1) ]= REG_RH((REG_HDMI_VS_IF0 +(2*i)));
        HDMI_MHAL_DGB("pvsipacket->vsi_packet.packet.dataBytes[%d] = %x\n",(2*i) ,pvsipacket->vsi_packet.packet.dataBytes[(2*i)]);
        HDMI_MHAL_DGB("pvsipacket->vsi_packet.packet.dataBytes[%d] = %x\n",((2*i)+1) ,pvsipacket->vsi_packet.packet.dataBytes[(2*i)+1]);
    }
    pvsipacket->vsi_packet.packet.type = (REG_RL(REG_HDMI_VS_HDR0)) & 0x7F;
    pvsipacket->vsi_packet.packet.version = REG_RH(REG_HDMI_VS_HDR0);
    pvsipacket->vsi_packet.packet.length = (REG_RL(REG_HDMI_VS_HDR1)) & 0x1F;
    HDMI_MHAL_DGB("pvsipacket->vsi_packet.packet.type = %x\n",pvsipacket->vsi_packet.packet.type);
    HDMI_MHAL_DGB("pvsipacket->vsi_packet.packet.version = %x\n",pvsipacket->vsi_packet.packet.version);
    HDMI_MHAL_DGB("pvsipacket->vsi_packet.packet.length = %x\n",pvsipacket->vsi_packet.packet.length);
}

void MHal_HDMI_GetAVI(SC_GET_HDMI_AVI_PACKET_t *pavipacket)
{
    U8 i;
    for (i = 0; i < 6; i++)
    {
        pavipacket->avi_packet.packet.dataBytes [(2*i)] = REG_RL(( REG_HDMI_AVI_IF0 +(2*i)));
        pavipacket->avi_packet.packet.dataBytes [((2*i)+1) ] = REG_RH((REG_HDMI_AVI_IF0 +(2*i)));
        HDMI_MHAL_DGB("pavipacket->avi_packet.packet.dataBytes [%d] = %x\n",(2*i),pavipacket->avi_packet.packet.dataBytes [(2*i)]);
        HDMI_MHAL_DGB("pavipacket->avi_packet.packet.dataBytes [%d] = %x\n",(2*i+1),pavipacket->avi_packet.packet.dataBytes [(2*i)+1]);
    }
    pavipacket->avi_packet.packet.dataBytes [12] = REG_RL(REG_HDMI_AVI_IF6);
    pavipacket->avi_packet.ePixelEncoding =MHal_HDMI_GetPacketColorFormat();
    pavipacket->avi_packet.eActiveInfo = ((REG_RL(REG_HDMI_AVI_IF0) & 0x10) >> 4);
    pavipacket->avi_packet.eBarInfo = ((REG_RL(REG_HDMI_AVI_IF0) & 0x0C) >> 2);
    pavipacket->avi_packet.eScanInfo = (REG_RL(REG_HDMI_AVI_IF0) & 0x03);
    pavipacket->avi_packet.eColorimetry = MHal_HDMI_GetExtendColorimetryInfo();
    pavipacket->avi_packet.ePictureAspectRatio = ((REG_RH(REG_HDMI_AVI_IF0) & 0x30) >> 4);
    pavipacket->avi_packet.eActiveFormatAspectRatio = (REG_RH(REG_HDMI_AVI_IF0) & 0x0F);
    pavipacket->avi_packet.eScaling = (REG_RL(REG_HDMI_AVI_IF1) & 0x03);
    pavipacket->avi_packet.VideoIdCode = REG_RH(REG_HDMI_AVI_IF1);
    pavipacket->avi_packet.PixelRepeat = REG_RL(REG_HDMI_AVI_IF2);
    pavipacket->avi_packet.eITContent = ((REG_RL(REG_HDMI_AVI_IF1) & 0x80) >> 7);
    pavipacket->avi_packet.eExtendedColorimetry = ((REG_RL(REG_HDMI_AVI_IF1) & 0x30) >> 4);
    pavipacket->avi_packet.eRGBQuantizationRange = ((REG_RL(REG_HDMI_AVI_IF1) & 0x0C) >> 2);
    pavipacket->avi_packet.TopBarEndLineNumber = ((REG_RL(REG_HDMI_AVI_IF3) << 8) | (REG_RH(REG_HDMI_AVI_IF2)));
    pavipacket->avi_packet.BottomBarStartLineNumber = ((REG_RL(REG_HDMI_AVI_IF4) << 8) | (REG_RH(REG_HDMI_AVI_IF3)));
    pavipacket->avi_packet.LeftBarEndPixelNumber = ((REG_RL(REG_HDMI_AVI_IF5) << 8) | (REG_RH(REG_HDMI_AVI_IF4)));
    pavipacket->avi_packet.RightBarEndPixelNumber = ((REG_RL(REG_HDMI_AVI_IF6) << 8) | (REG_RH(REG_HDMI_AVI_IF5)));
    HDMI_MHAL_DGB("pavipacket->avi_packet.packet.dataBytes [12] = %x\n",pavipacket->avi_packet.packet.dataBytes [12]);
    HDMI_MHAL_DGB("pavipacket->avi_packet.ePixelEncoding = %x\n",pavipacket->avi_packet.ePixelEncoding);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eActiveInfo = %x\n",pavipacket->avi_packet.eActiveInfo);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eBarInfo = %x\n",pavipacket->avi_packet.eBarInfo);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eScanInfo = %x\n",pavipacket->avi_packet.eScanInfo);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eColorimetry = %x\n",pavipacket->avi_packet.eColorimetry);
    HDMI_MHAL_DGB("pavipacket->avi_packet.ePictureAspectRatio = %x\n",pavipacket->avi_packet.ePictureAspectRatio);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eActiveFormatAspectRatio = %x\n",pavipacket->avi_packet.eActiveFormatAspectRatio);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eScaling = %x\n",pavipacket->avi_packet.eScaling);
    HDMI_MHAL_DGB("pavipacket->avi_packet.VideoIdCode = %x\n",pavipacket->avi_packet.VideoIdCode);
    HDMI_MHAL_DGB("pavipacket->avi_packet.PixelRepeat = %x\n",pavipacket->avi_packet.PixelRepeat);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eITContent = %x\n",pavipacket->avi_packet.eITContent);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eExtendedColorimetry = %x\n",pavipacket->avi_packet.eExtendedColorimetry);
    HDMI_MHAL_DGB("pavipacket->avi_packet.eRGBQuantizationRange = %x\n",pavipacket->avi_packet.eRGBQuantizationRange);
    HDMI_MHAL_DGB("pavipacket->avi_packet.TopBarEndLineNumber = %x\n",pavipacket->avi_packet.TopBarEndLineNumber);
    HDMI_MHAL_DGB("pavipacket->avi_packet.BottomBarStartLineNumber = %x\n",pavipacket->avi_packet.BottomBarStartLineNumber);
    HDMI_MHAL_DGB("pavipacket->avi_packet.LeftBarEndPixelNumber = %x\n",pavipacket->avi_packet.LeftBarEndPixelNumber);
    HDMI_MHAL_DGB("pavipacket->avi_packet.RightBarEndPixelNumber = %x\n",pavipacket->avi_packet.RightBarEndPixelNumber);

}


#if 0 // remove this patch because no need in T3
// LGE earnest 2009/01/24 		Tuner init time is slower because of this interrupt in PDP. We must move init routine after starting HDMI.
void MHal_HDMI_EnableISR4VideoClkFreqBigChange(void)
{
	REG_WR(REG_HDMI_INT_MASK, 0xDEFF);
}

void MHal_HDMI_DisableISR4VideoClkFreqBigChange(void)
{
	REG_WR(REG_HDMI_INT_MASK, 0xFEFF);
}
#endif

void MHal_HDMI_SetMux(SC_INPUT_SOURCE_e enInput)
{

    if(enInput == INPUT_SOURCE_HDMI_C)
    {
        // HDMI input port mux
        // enable DVI function
        REG_WI(REG_DVI_ATOP(0x00), 0x00, BIT2);
        // enable dvi port2 input
        REG_WI(REG_DVI_ATOP(0x6A), BIT0, BIT0);
        // power down DVI port 1
        REG_WR(REG_DVI_ATOP(0x60), 0xFFFF);
        // power down DVI port 2
        REG_WR(REG_DVI_ATOP(0x69), 0x0000);
        //  power down dvi clock
        REG_WM(REG_PM_SLEEP(0x4B), BIT8, (BIT8 | BIT10));
    }
    else if((enInput == INPUT_SOURCE_HDMI_A) || (enInput == INPUT_SOURCE_HDMI_B) || (enInput == INPUT_SOURCE_HDMI_D))
    {
        // HDMI input port mux
        // enable DVI function
        REG_WI(REG_DVI_ATOP(0x00), BIT2, BIT2);
        // enable dvi port2 input
        REG_WI(REG_DVI_ATOP(0x6A), 0x00, BIT0);
        // power down DVI port 1
        REG_WR(REG_DVI_ATOP(0x60), 0x0000);
        // power down DVI port 2
        REG_WR(REG_DVI_ATOP(0x69), 0xFFFF);
        //  power down dvi clock
        REG_WM(REG_PM_SLEEP(0x4B), BIT10, (BIT8 | BIT10));
    }
    else
    {
        // HDMI input port mux power down
        REG_WI(REG_DVI_ATOP(0x00), 0x00, BIT2);
        REG_WI(REG_DVI_ATOP(0x6A), 0x00, BIT0);
        REG_WR(REG_DVI_ATOP(0x60), 0xFFFF);
        REG_WR(REG_DVI_ATOP(0x69), 0xFFFF);
        REG_WM(REG_PM_SLEEP(0x4B), (BIT8 | BIT10), (BIT8 | BIT10));
    }
    switch(enInput)
    {
        case INPUT_SOURCE_HDMI_A:
            // enable dvi port a data input switch
            REG_WM(REG_DVI_ATOP(0x61), BIT0, (BIT0 | BIT1 | BIT2));
            // GCR_SWCK_ONL
            REG_WM(REG_PM_SLEEP(0x4B), BIT3, (BIT3 | BIT4 | BIT5));
            break;

        case INPUT_SOURCE_HDMI_B:
            // enable dvi port b data input switch
            REG_WM(REG_DVI_ATOP(0x61), BIT1, (BIT0 | BIT1 | BIT2));
            // GCR_SWCK_ONL
            REG_WM(REG_PM_SLEEP(0x4B), BIT4, (BIT3 | BIT4 | BIT5));
            break;

        case INPUT_SOURCE_HDMI_C:
            // disable dvi port a,b,d data input switch
            REG_WM(REG_DVI_ATOP(0x61), 0x00, (BIT0 | BIT1 | BIT2));
            // GCR_SWCK_ONL
            REG_WM(REG_PM_SLEEP(0x4B), 0x00, (BIT3 | BIT4 | BIT5));
            break;

        case INPUT_SOURCE_HDMI_D:
            // enable dvi port d data input switch
            REG_WM(REG_DVI_ATOP(0x61), BIT2, (BIT0 | BIT1 | BIT2));
            // GCR_SWCK_ONL
            REG_WM(REG_PM_SLEEP(0x4B), BIT5, (BIT3 | BIT4 | BIT5));
            break;

        default: // For system first power on with the unknow HDMI port.
            // disable dvi port a,b,d data input switch
            REG_WM(REG_DVI_ATOP(0x61), 0x00, (BIT0 | BIT1 | BIT2));
            // GCR_SWCK_ONL
            REG_WM(REG_PM_SLEEP(0x4B), 0x00, (BIT3 | BIT4 | BIT5));
            break;
    }
}

//------------------------------------------------------------------------------
//  T3 DVI ATOP/DTOP
//------------------------------------------------------------------------------

BOOL MHal_HDMI_DVISyncLoss(SC_INPUT_SOURCE_e enInput)
{
    BOOL bDetected;
    // INPUT_SOURCE_HDMI_A, INPUT_SOURCE_HDMI_B, INPUT_SOURCE_HDMI_D
    if ((enInput == INPUT_SOURCE_HDMI_A) || (enInput == INPUT_SOURCE_HDMI_B) || (enInput == INPUT_SOURCE_HDMI_D))
    {
        bDetected = (REG_RR(REG_DVI_DTOP(0x17)) & BIT15) ? TRUE : FALSE;
    } // INPUT_SOURCE_HDMI_C
    else if (enInput == INPUT_SOURCE_HDMI_C)
    {
        bDetected = (REG_RR(REG_DVI_DTOP(0x18)) & BIT15) ? TRUE : FALSE;
    }
    else
    {
        bDetected = TRUE;
    }
    return bDetected;
}

//------------------------------------------------------------------------------
//  T3 AUDIO function
//------------------------------------------------------------------------------
void MHal_AuSPDIF_HWEN(BOOL bEnable)
{
    // T3 TODO
#if 0
    REG_WI(REG_VIVALDI_BANK(0x45), bEnable, BIT7);
#endif
}

//victor 20081215, HDMI audio
//******************************************************************************
//  [Function Name]:
//      void AuSet_HDMI_Down_Sample(void)
//  [Description]:
//      This routine is used to set HDMI audio sample rate below 48K.
//      crystal: 12MHz(216MHz)
//  [Arguments]:
//      None:
//  [Return]:
//      None,
//*******************************************************************************
void MHal_HDMI_AuDownSample(void)
{
    // T3 TODO
#if 0
    U8 temp;
    U16 freq;
	static U8 _temp = 0xff;
    freq = REG_RR(REG_VIVALDI_BANK(0x16));

    if(freq & 0x8000)
        return;   //no signal

    if(freq < (0x08CA-20))      //> 96K
    {
        temp = 2;
    }
    else if(freq < (0x1194-20)) //> 48K
    {
        temp = 1;
    }
    else
    {
        temp = 0;
    }

	if (_temp != temp)
	{
		REG_WM(REG_VIVALDI_BANK(0x24), temp, 0x03);
	}
	_temp = temp;
#endif
}



