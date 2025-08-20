#define _MSSCALERIP_C_
#include "mdrv_mfc_platform.h"
#include "mdrv_mfc_fb.h"
#include "mdrv_mfc_scalerip.h"

extern void	udelay	      (unsigned long);
#define FORCE_S7M_TTL 0 // for debug ttl in U01/U02

#if(CODESIZE_SEL == CODESIZE_ALL)

//set in mirror mode
U16 Set_PacketCount(U8 u8IpMode, U16 u16PnlWidth)
{
    if (u8IpMode==IP_YC_10BIT)
        return PacketCountCheck(u16PnlWidth, 64);
    else  if((u8IpMode==IP_YC_8BIT) || (u8IpMode==IP_YC_10BIT_SPECIAL))
        return PacketCountCheck(u16PnlWidth, 16);
    else
	 return PacketCountCheck(u16PnlWidth, 64);
}

U16 IPM_LineLimit(U8 u8IpMode, U16 u16PnlWidth)
{
	if (u8IpMode==IP_YC_10BIT)
		return LimitCheck(u16PnlWidth, 64);
    else  ///IP_YC_8BIT or IP_YC_10BIT_SPECIAL
		return LimitCheck(u16PnlWidth, 16);

}


U16 IPM_LinePitch(U8 u8IpMode, U16 u16PnlWidth)
{
    if(gmfcSysInfo.u8MirrorMode)
    {
            return RemainCheck_Add1(u16PnlWidth,16);
    }
    else
    {
        if (u8IpMode==IP_YC_10BIT)
            return LimitCheck(u16PnlWidth, 64)/16;
        else  ///IP_YC_8BIT or IP_YC_10BIT_SPECIAL
            return LimitCheck(u16PnlWidth, 16)/16;
    }
}

U16 Ycout_LinePitch(U8 u8IpMode, U16 u16PnlWidth)
{
    if(gmfcSysInfo.u8MirrorMode)
    {
	    if (u8IpMode==IP_YC_10BIT)
        {
            if(IsMultipCheck(u16PnlWidth,64))
		        return (U16)LimitCheck(u16PnlWidth, 64)/64*10;
    	    else
                return (U16)(u16PnlWidth/64*10 + ((RemainCheck_Add1(u16PnlWidth%64,16)*2+2)));
        }
	    else  if((u8IpMode==IP_YC_8BIT) || (u8IpMode==IP_YC_10BIT_SPECIAL))
            return (U16)RemainCheck_Add1(u16PnlWidth,16)*2;
    }
    else
    {
	if (u8IpMode==IP_YC_10BIT)
            return (U16)LimitCheck(u16PnlWidth, 64)/64*10;
        else  if((u8IpMode==IP_YC_8BIT) || (u8IpMode==IP_YC_10BIT_SPECIAL))
    		return (U16)LimitCheck(u16PnlWidth, 16)/16*2;
        else  if(u8IpMode==IP_RGB_10BIT)
    	    return (U16)LimitCheck(u16PnlWidth, 16)/4;
        else
            return (U16)LimitCheck(u16PnlWidth, 16)/16*3;
    }
    return 0;
}

#if ( TwoChip_Func == TwoChip_Slave )
U32 Ycout_LinePitchDW(U8 u8IpMode, U32 u32PnlWidth)
{
	if (u8IpMode==IP_YC_10BIT)
		return LimitCheck(u32PnlWidth, 64)/64*10;
    else   ///IP_YC_8BIT or IP_YC_10BIT_SPECIAL
		return LimitCheck(u32PnlWidth, 16)/16*2;
}
#endif

U8 FBNumber(void)
{
	return 4;///7;  //frame buffer number=8(0~7)
}


BOOL IS_IP_RGB(U8 u8IpType)
{
    if((u8IpType&0xF0) == 0x10)
        return 1;
    else
        return 0;
}

BOOL IS_IP_YUV(U8 u8IpType)
{
    if((u8IpType&0xF0) == 0x10)
        return 0;
    else
        return 1;
}
void MDrv_MFC_IP_SetMemMode(U8 u8IpMode)
{
    U8 u8Value=0 , u8Value2=0 , u8Value3=0 , u8Value4=0,  u8Value5=0;

    if(IS_IP_RGB(u8IpMode))
    {
	    u8Value = 0x05 ;
	    u8Value2 = 0x00 | (u8IpMode&0x0f) ;  //YC filter=0 for RGB
	    u8Value3 = 0X62 ;
	    u8Value4 = 0X00 ;
	    u8Value5 = 0X30 ;  //Darren Memc pip
    }
    else
    {
    	u8Value2 = 0x90 | (u8IpMode&0x0f) ;
	    u8Value3 = 0X63 ;
	    u8Value4 = 0X01 ;
	    #if (TwoChip_Func)
	      u8Value5 = 0X40 ;
	    #else
	    u8Value5 = 0X70 ;
	    #endif	


        if (u8IpMode==IP_YC_10BIT)
        {
           u8Value = 0x77 ;
        }
        else if (u8IpMode==IP_YC_10BIT_SPECIAL)
        {
            u8Value = 0x05 ;
            if(gmfcSysInfo.u8MirrorMode>1) //V && HV
            {
                MDrv_MFC_WriteBit(0x2005, 1, _BIT5);
            }
            else
            {
                MDrv_MFC_WriteBit(0x2005, 0, _BIT5);
            }

            if(gmfcSysInfo.u8MirrorMode%2)
                u8Value2 |= _BIT6;
            else
                u8Value2 &= ~_BIT6;
        }
        else
        {
            u8Value = 0x05 ;
        }
     }

    MDrv_MFC_WriteByte(0x2002, u8Value);
    MDrv_MFC_WriteByte(0x2003, u8Value2); //[7:5]:C Filter, =1 090820 suchiun for OSD color issue
									      //[4]: RGB/YUV

    MDrv_MFC_WriteByte(0x2661, u8Value3); //[0]: RGB/YUV
    MDrv_MFC_WriteByte(0x2667, u8Value4); //[0]: RGB/YUV
    MDrv_MFC_WriteByte(0x2666, u8Value5); //[6]: RGB/YUV

    if( IS_CHIP_3xxx_U03_AND_AFTER(gmfcSysInfo.u8ChipRevision) )
    {
        if(  (u8IpMode==IP_YC_10BIT_SPECIAL) || (u8IpMode==IP_RGB_10BIT_SPECIAL) )
        MDrv_MFC_WriteBit(0x2160, 1, _BIT3); //for Lsb 0,1
    }

}

/*
LVDS input selection:
0x2202[0]=1:Back-side pin sequence swap
0x2250[6]=1:LVDS clock input select from even
0x2300[7]=1:TI mode, [2]=1:10bits, [1]=1:Single
0x2328[7]=1:MSB/LSB swap, [6]=1:P/N swap, [4]=1:Odd/Even swap
0x233A[3]=1:6bits
*/
void MDrv_MFC_InitializeRx(void)
{
    if (S7M)
    {
        if (IS_CHIP_3xxx_U03_AND_AFTER(gmfcSysInfo.u8ChipRevision))// TTL input
	{
	MDrv_MFC_WriteByteMask(0x2214, (_BIT3|_BIT5), (_BIT2|_BIT3|_BIT5));
	MDrv_MFC_WriteByteMask(0x2216, (_BIT3|_BIT4), (_BIT3|_BIT4));
	MDrv_MFC_WriteByteMask(0x221E, (_BIT3|_BIT4), (_BIT3|_BIT4));
	MDrv_MFC_WriteByteMask(0x228E, 0xF8, 0xF8);
	}
    else//LVDS input
	{
        MDrv_MFC_WriteBit(0x2214, 1, _BIT2);//lvds internal 100 ohm
        //For power down the consumption 20100115 I-Chang
        MDrv_MFC_WriteBit(0x2228, 1, _BIT4);//enable current adjust
        MDrv_MFC_WriteByte(0x2224, 0xCC);
	}
    }

	#if(CODEBASE_SEL == CODEBASE_UTOPIA)
	if (gmfcSysInfo.u8PanelVfreq==60)
	{
	    MDrv_MFC_Write2Bytes(0x320A, 0x6008);
	    MDrv_MFC_Write2Bytes(0x3200, 0x0001);
	    MDrv_MFC_Write2Bytes(0x2A06, 0x0006);
	    MDrv_MFC_Write2Bytes(0x321E, 0x2200);
		MDrv_MFC_WriteBit(0x2A83, 0, _BIT0);
		MDrv_MFC_WriteBit(0x2A83, 1, _BIT1);
	}
#endif
	MDrv_MFC_WriteByte(0x22AC, 0x11);

#if(CODEBASE_SEL == CODEBASE_51)
	#if ((BOARD_TYPE_SEL == BD_MST054C_C01A_S) )
		MDrv_MFC_WriteByte(0x2202, 0x00);
	#else
		MDrv_MFC_WriteByte(0x2202, 0x01); // [0]=1: back-side pin sequence swap
	#endif
#else
		MDrv_MFC_WriteByte(0x2202, 0x01); // [0]=1: back-side pin sequence swap
#endif

	MDrv_MFC_WriteByte(0x2250, 0x40); // [6]=1: LVDS clock input select from even

#if (CODEBASE_SEL == CODEBASE_51)
    #if ((BOARD_TYPE_SEL == BD_MST054C_C01A_S))
	MDrv_MFC_WriteByte(0x2300, 0x70|(gmfcSysInfo.u8LVDSTiMode?_BIT7:0)|(gmfcSysInfo.u8LVDSBitNum==_10BITS?_BIT2:0)|(gmfcSysInfo.u8LVDSChannel==_SINGLE?_BIT1:0));
	MDrv_MFC_WriteByte(0x233A, 0xB0|(gmfcSysInfo.u8LVDSBitNum==_6BITS?_BIT3:0));
	MDrv_MFC_WriteByte(0x2328, (gmfcSysInfo.u8LVDSSwapMsbLsb?_BIT7:0)|(gmfcSysInfo.u8LVDSSwap_P_N?_BIT6:0)|(gmfcSysInfo.u8LVDSSwapOddEven?_BIT4:0));
    #endif
	#endif
	#if(CODEBASE_SEL == CODEBASE_51)
  		#if (REG_DIRECT_ACCESS_BY_I2C)
			if (gmfcSysInfo.u8Preset == 0x01)
  		#endif
	#endif
	{
		// 0x2300, [7]=1:TI mode, [2]=1:10bits, [1]=1:Single
		MDrv_MFC_WriteByte(0x2300, (gmfcSysInfo.u8LVDSTiMode?_BIT7:0)
							| (gmfcSysInfo.u8LVDSBitNum==_10BITS?_BIT2:0)
							| (gmfcSysInfo.u8LVDSChannel==_SINGLE?_BIT1:0)
							| (gmfcSysInfo.u8PanelBlankCPVC?_BIT6:0)
							| (gmfcSysInfo.u8PanelBlankOEC?_BIT5:0)
							| (gmfcSysInfo.u8PanelBlankTPC?_BIT4:0)
							| (gmfcSysInfo.u8PanelBlankSTHC?_BIT3:0));
        //0x2328, [7]=1:MSB/LSB swap, [6]=1:P/N swap, [4]=1:Odd/Even swap
		MDrv_MFC_WriteByte(0x2328, (gmfcSysInfo.u8LVDSSwapMsbLsb?_BIT7:0)|
		                           (gmfcSysInfo.u8LVDSSwap_P_N?_BIT6:0)|
		                           (gmfcSysInfo.u8LVDSSwapOddEven?_BIT4:0));
        if(S7M)
			{
            #if FORCE_S7M_TTL
            if(1)
            #else
            if( IS_CHIP_3xxx_U03_AND_AFTER(gmfcSysInfo.u8ChipRevision) )
            #endif
            {
                //S7M path selection; TTL(1)/LVDS(0)
                MDrv_MFC_WriteBit(0x22E1, 1, _BIT3);
                MDrv_MFC_WriteBit(0x22E1, 1, _BIT4);
                //Panel bit number selection
                if(gmfcSysInfo.u8PanelType == _MINI_LVDS
		            || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP
	  	            || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)

                {
                    if(gmfcSysInfo.u8PanelBitNum == _10BITS)
                    {
	  	                MDrv_MFC_WriteBit(0x2301, 1, _BIT7);
                    }
                    else//_8BITS
                    {
                        MDrv_MFC_WriteBit(0x2301, 0, _BIT7);
                    }
                }
                MDrv_MFC_WriteBit(0x2301, 1, _BIT5);//receive TTL rx data from LVDS pin
                MDrv_MFC_WriteBit(0x2328, (gmfcSysInfo.u8LVDSSwapOddEven?_BIT4:0), _BIT4);
            }
            else//U02 chip will not use TTL path
            {
                MDrv_MFC_WriteBit(0x2301, 0, _BIT7);
                MDrv_MFC_WriteBit(0x2301, 0, _BIT5);
                MDrv_MFC_WriteBit(0x2328, 0, _BIT4);//LVDS case u8LVDSSwapOddEven = 0
            }
            }
            else
        {//not ttl
            if( IS_CHIP_3xxx_U03_AND_AFTER(gmfcSysInfo.u8ChipRevision) )
            {
                MDrv_MFC_WriteBit(0x22E1, 0, _BIT3);
                MDrv_MFC_WriteBit(0x22E1, 0, _BIT4);
            }
            else
            {
                MDrv_MFC_WriteByte(0x2301, 0x00);
            }
			}
		// 0x233A, [3]=1:6bits
		MDrv_MFC_WriteByte(0x233A, 0xB0|(gmfcSysInfo.u8LVDSBitNum==_6BITS?_BIT3:0));
	}

    MDrv_MFC_WriteByte(0x2283, ((TwoChip_Func)?0xc4:0xf1)); //0xc4 //Ip current &phase for flash line issue from cloud 091030
    MDrv_MFC_WriteByte(0x2285, ((TwoChip_Func)?0xc4:0xf1)); //0xc4
    MDrv_MFC_WriteByte(0x228A, ((TwoChip_Func)?0x00:0x10)); //skew jitter adjust for flash line issue from alex_cw 091030
    MDrv_MFC_WriteByte(0x228C, ((TwoChip_Func)?0x00:0x10));
	
    MDrv_MFC_WriteByteMask(0x228F, 0, 0x0F);

	//printk("MDrv_MFC_InitializeRx()\n");
}

void MDrv_MFC_InitializeIP_PtnGen(void)
{
    MDrv_MFC_WriteByte(0x20E0, 0x00);
	MDrv_MFC_Write2Bytes(0x20E2, gmfcSysInfo.u16HTotal);
	MDrv_MFC_Write2Bytes(0x20E4, gmfcSysInfo.u16VTotal);
	MDrv_MFC_Write2Bytes(0x20E6, 0);
	MDrv_MFC_Write2Bytes(0x20E8, gmfcSysInfo.u16Width);
	MDrv_MFC_Write2Bytes(0x20EA, 0);
	MDrv_MFC_Write2Bytes(0x20EC, gmfcSysInfo.u16Height);
	//printk("MDrv_MFC_InitializeIP_PtnGen()\n");
}

code MST_MFC_RegUnitType_t tInitializeOPMAddr[]=
{
  {0x2138, 0x4c},
  {0x2139, 0x0d},
  {0x213A, 0x00},
  {0x213B, 0x00},

  {0x213C, 0xEC},
  {0x213D, 0xfe},
  {0x213E, 0x04},
  {0x213F, 0x00},

  {0x2140, 0x8C},
  {0x2141, 0xf0},
  {0x2142, 0x09},
  {0x2143, 0x00},

  {0x2144, 0x2C},
  {0x2145, 0xe2},
  {0x2146, 0x0e},
  {0x2147, 0x00},

  {0x2148, 0xCC},
  {0x2149, 0xd3},
  {0x214A, 0x13},
  {0x214B, 0x00},

  {0x214C, 0xA9},
  {0x214D, 0x00},
  {0x214E, 0x20},
  {0x214F, 0x00},

  {0x2150, 0xAE},
  {0x2151, 0x00},
  {0x2152, 0x28},
  {0x2153, 0x00},
{_END_OF_TBL_, _END_OF_TBL_},
};

code MST_MFC_RegUnitType_t tInitializeIP[]=
{
#if 1 // Input 60Hz
	//======================
	// 60Hz
	//======================
	//[IP]
	{0x203E, 0x2f}, // rfifo rfifo thr
	{0x203F, 0x1f}, // rfifo high pri thr
	{0x2040, 0x20}, // wfifo wfifo thr
	{0x2041, 0x40}, // wfifo high pri thr
	{0x2042, 0x20}, // rreq_len_y
	{0x2043, 0x08}, // rreq_len_mr
	{0x2044, 0x40},///58 // wreq_len_ycout //Titan.sun suggestion 081027
	{0x2045, 0x00}, // wreq_len_mr
#else
	//======================
	// 120Hz
	//======================
	//[IP]
	{0x203E, 0x20}, // rfifo rfifo thr
	{0x203F, 0x18}, // rfifo high pri thr
	{0x2040, 0x20}, // wfifo wfifo thr
	{0x2041, 0x28}, // wfifo high pri thr
	{0x2042, 0x20}, // rreq_len_y
	{0x2043, 0x08}, // rreq_len_mr
	{0x2044, 0x50}, // wreq_len_ycout
	{0x2045, 0x08}, // wreq_len_mr
#endif
	{0x204A, 0x00}, // r_mask_num
	{0x204B, 0x00}, // w_mask_num
	{0x2060, 0x03}, // [1]:de_only, [0]:mode_det_en
	{0x2061, 0x00}, //
	{0x2062, 0x03}, // vpulse line for IP
	{0x2063, 0x00}, // vpulse line for IP
	{0x2064, 0x01}, // vpulse line for Frame Lock
	{0x2065, 0x00}, // vpulse line for Frame Lock
	{0x2066, 0x10}, // lock interrupt control, [4]:ref_h
	{0x2068, 0xb4}, // blank_boundary (720)
	{0x2069, 0x00}, // blank_boundary (720)
	//{0x20D0, 0xF1}, // [7:6]:ip unstable control,[5:4]:ip no signal control,[0]:reference clock mode detection
	{0x20D0, 0x31}, // For Demo Board Use, !!???
	{0x20D4, 0x05},

	{0x2001, 0x03}, // [1]Write request enable, [0]Read request enable

{_END_OF_TBL_, _END_OF_TBL_},
};

void MDrv_MFC_SetMirrorMode(MirrorModeType ucMirrorMode)
{
	U8 i;
    gmfcSysInfo.u8MirrorMode=ucMirrorMode;
    MDrv_MFC_IP_SetMemMode(IP_MODE);


    //I-Chang; for Line Pitch update
    MDrv_MFC_Write2Bytes(0x2114, Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width)*2);
    MDrv_MFC_Write2Bytes(0x2034, Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width));
    MDrv_MFC_Write2Bytes(0x203A, Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width));
    MDrv_MFC_Write2Bytes(0x2038, IPM_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width)); //IP Linefetch
    if(gmfcSysInfo.u8MirrorMode)
        MDrv_MFC_Write2Bytes(0x2046, gmfcSysInfo.u16Width);
    else
        MDrv_MFC_Write2Bytes(0x2046, IPM_LineLimit(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width));




	MDrv_MFC_WriteByte(0x205D, Set_PacketCount(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width));

    //IP Init
	for (i=0; i<5; i++)
	{
		MDrv_MFC_Write3Bytes(0x2006+4*i, (gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*i)>>4);
	}
	for (i=0; i<3; i++)
	{
		MDrv_MFC_Write3Bytes(0x2020+4*i, (gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+5))>>4);
	}

    //OPM Setting
	for (i=0; i<5; i++)
	{
		MDrv_MFC_Write3Bytes(0x2138+4*i, MDrv_MFC_Read3Bytes(0x2006+4*i));
	}
	for (i=0; i<3; i++)
	{
		MDrv_MFC_Write3Bytes(0x214C+4*i, MDrv_MFC_Read3Bytes(0x2020+4*i));
	}

    switch(ucMirrorMode)
    {
        case MIRROR_H_MODE:
            //Set H mode IP address
        	for (i=0; i<5; i++)
        	{
        		MDrv_MFC_Write3Bytes(0x2006+4*i,
        			((gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*i)>>4)
        			+Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width)-1);
        	}
        	for (i=0; i<3; i++)
        	{
        		MDrv_MFC_Write3Bytes(0x2020+4*i,
        			((gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+5))>>4)
        			+Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width)-1);
        	}
            //Enable OPM setting
            MDrv_MFC_WriteByteMask(0x2136, _BIT0, _BIT0);
            //Enable H mode
            MDrv_MFC_WriteByteMask(0x205C, _BIT0, _BIT1|_BIT0);
            #if (BUF_NUM==5)
            MDrv_MFC_WriteByte(0x2004, BUF_NUM);
            MDrv_MFC_WriteByte(0x2910, 0xEE);
            #endif

            break;

        case MIRROR_V_MODE:
            //Set V mode IP address
        	for (i=0; i<5; i++)
        	{
        		MDrv_MFC_Write3Bytes(0x2006+4*i,
        			(((gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+1))>>4)
        			-Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width)));
        	}
		for (i=0; i<3; i++)
        	{
        		MDrv_MFC_Write3Bytes(0x2020+4*i,
        			(((gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+6))>>4)
        			-Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width)));
        	}
            //Enable OPM setting
            MDrv_MFC_WriteByteMask(0x2136, _BIT0, _BIT0);
            //Enable V mode
            MDrv_MFC_WriteByteMask(0x205C, _BIT1, _BIT1|_BIT0);
            #if (BUF_NUM==5)
            MDrv_MFC_WriteByte(0x2004, BUF_NUM);
            MDrv_MFC_WriteByte(0x2910, 0xEE);
            #endif
            break;

        case MIRROR_HV_MODE:
            //Set HV mode IP address
        	for (i=0; i<5; i++)
        	{
        		MDrv_MFC_Write3Bytes(0x2006+4*i, (((gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+1))>>4)-1));
        	}
	       for (i=0; i<3; i++)
        	{
        		//MDrv_MFC_Write3Bytes(0x2006+4*i, (((gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+1))>>4)-1));
        		MDrv_MFC_Write3Bytes(0x2020+4*i, (((gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+6))>>4)-1));
        	}
            //Enable OPM setting
            MDrv_MFC_WriteByteMask(0x2136, _BIT0, _BIT0);
            //Enable H mode
            MDrv_MFC_WriteByteMask(0x205C, _BIT1|_BIT0, _BIT1|_BIT0);
            #if (BUF_NUM==5)
            MDrv_MFC_WriteByte(0x2004, BUF_NUM);
            MDrv_MFC_WriteByte(0x2910, 0xEE);
            #endif
            break;

        case MIRROR_OFF:
        default:
            //Set Original IP base Asddress
        	for (i=0; i<5; i++)
        	{
        		MDrv_MFC_Write3Bytes(0x2006+4*i, (gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*i)>>4);
        	}
        	for (i=0; i<3; i++)
        	{
        		MDrv_MFC_Write3Bytes(0x2020+4*i, (gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+5))>>4);
        	}
            //Disable OPM setting
            MDrv_MFC_WriteByteMask(0x2136, 0, _BIT0);
            //Disable HV mode
            MDrv_MFC_WriteByteMask(0x205C, 0, _BIT1|_BIT0);
            #if (BUF_NUM==5)
            MDrv_MFC_WriteByte(0x2004, BUF_NUM-1);
            MDrv_MFC_WriteByte(0x2910, REG_2910_DEFAULT);
            #endif
            break;
    }
}


#if ( TwoChip_Func == TwoChip_Slave )
void MDrv_MFC_SetOPMBaseAddr(void)
{
	U32 dwOPMBase;
	U8 i ;

    //OPM Setting
        dwOPMBase = Ycout_LinePitchDW(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width/2 -GarBand );  //-64
	for (i=0; i<5; i++)
	{
		MDrv_MFC_Write3Bytes(0x2138+4*i, MDrv_MFC_Read3Bytes(0x2006+4*i) + dwOPMBase);
	}

	for (i=0; i<3; i++)
	{
		MDrv_MFC_Write3Bytes(0x214C+4*i, MDrv_MFC_Read3Bytes(0x2020+4*i) + dwOPMBase);
	}
	//  MDrv_MFC_WriteRegsTbl(0x2100, tInitializeOPMAddr); // initialize all of bank
}
#endif

void MDrv_MFC_InitializeIP(void)
{
	U8 i;

	MDrv_MFC_IP_SetMemMode(gmfcMiuBaseAddr.u8IpMode);
	//[Frame Buffer]
	MDrv_MFC_WriteByte(0x2004, FBNumber());

	for (i=0; i<5; i++)  //j090423
	{
		MDrv_MFC_Write3Bytes(0x2006+4*i, (gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*i)>>4);
	}

	MDrv_MFC_WriteByte(0x201B, 0x04); //Ip to mc sel

	for (i=0; i<3; i++)
	{
		MDrv_MFC_Write3Bytes(0x2020+4*i, (gmfcMiuBaseAddr.u32IpYcoutBase+gmfcMiuBaseAddr.u32IpYcoutSize*(i+5))>>4);
	}

	MDrv_MFC_Write2Bytes(0x2034, Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width));
	MDrv_MFC_Write2Bytes(0x203A, Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width));
    MDrv_MFC_Write2Bytes(0x2038, IPM_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width)); //IP Linefetch

    if(gmfcSysInfo.u8MirrorMode)
        MDrv_MFC_Write2Bytes(0x2046, gmfcSysInfo.u16Width);
    else
        MDrv_MFC_Write2Bytes(0x2046, IPM_LineLimit(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width));

	MDrv_MFC_Write2Bytes(0x2048, gmfcSysInfo.u16Height);
	if(gmfcSysInfo.u8MirrorMode)
    	MDrv_MFC_SetMirrorMode(MIRROR_HV_MODE);

    MDrv_MFC_WriteRegsTbl(0x2000, tInitializeIP); // initialize all of bank
    //printk("MDrv_MFC_InitializeIP()\n");
}

void MDrv_MFC_SoftwareResetIP(void)
{
	MDrv_MFC_WriteByte(0x2000, 0x01); // [0]IP software reset
	MDrv_MFC_WriteByte(0x2000, 0x00);
}

code MST_MFC_RegUnitType_t tInitializeOPM[]=
{
	{0x210E, 0x08}, // MLB output data rest cycle number between two line //30->40 pip deep ,suchiun suggestion 080911
	{0x2110, 0x70}, ///90 // OPM fifo normal threshold to trigger read request
	{0x2111, 0x60}, // OPM fifo high threshold to trigger read request
	{0x2112, 0x7f}, //0x50, ///48// Maxinum length of OPM read request
	{0x2102, 0x01}, // Enable OPM/MLB disp
{_END_OF_TBL_, _END_OF_TBL_},
};

void MDrv_MFC_InitializeOPM(void)
{
//#if ( TwoChip_Func != TwoChip_OFF )
 //        MDrv_MFC_Write2Bytes(0x2106, gmfcSysInfo.u16Width/2 +64); //j091021
//#else
         MDrv_MFC_Write2Bytes(0x2106, WidthCheck(gmfcSysInfo.u16Width));
//#endif
	MDrv_MFC_Write2Bytes(0x2108, gmfcSysInfo.u16Height);
	MDrv_MFC_WriteByte(0x2C47, gmfcSysInfo.u8PanelVfreq);
	MDrv_MFC_Write2Bytes(0x2114, Ycout_LinePitch(gmfcMiuBaseAddr.u8IpMode, gmfcSysInfo.u16Width)*2);
	//MDrv_MFC_Write2Bytes(0x2116, MrLinePitch(gmfcSysInfo.u16Width));
    MDrv_MFC_WriteRegsTbl(0x2100, tInitializeOPM); // initialize all of bank
	//printk("MDrv_MFC_InitializeOPM()\n");
}
void MDrv_MFC_SoftwareResetOPM(void)
{
	MDrv_MFC_WriteByte(0x2103, 0x03); // [1]MLB's software reset, [0]OPM's software reset, 1:reset
    mfcSleepMs(100);
	MDrv_MFC_WriteByte(0x2103, 0x00); // OPM/MLB enable
}

#if 0
void MDrv_MFC_SoftwareResetScaler(void)
{
	MDrv_MFC_WriteBit(0x1E03, 1, _BIT0);
	MDrv_MFC_WriteBit(0x1E03, 0, _BIT0);
}
#endif

#if ( TwoChip_Func == TwoChip_Master )
#include "mpif.h"
void msReset2Chip(void)
{
    U16 wStatus;
    U8 ucCnt;
          ucCnt=200;
          while(ucCnt--)
	  {
   		 wStatus= pifRead2Byte2A(0x2A08);
		  if(wStatus&0x0001)
	        {
			MDrv_MFC_SoftwareResetScaler();
		         break;
	         }
	  }//while
}
#endif

code MST_MFC_RegUnitType_t tInitializeSnr[]=
{
	{0x2E60, 0X03},
	{0x2E61, 0X00},
	{0x2E62, 0X0F},
	{0x2E63, 0X00},
	{0x2EA0, 0X03},
	{0x2EA1, 0X00},
	{0x2EA8, 0X30},
	{0x2EA9, 0X00},
	{0x2EAA, 0X11},
	{0x2EAB, 0X00},
{_END_OF_TBL_, _END_OF_TBL_},
};

void MDrv_MFC_InitializeScaler(void)
{
    MDrv_MFC_WriteRegsTbl(0x2E00, tInitializeSnr); // initialize all of bank
    //printk("MDrv_MFC_InitializeScaler()\n");
}

void MDrv_MFC_InitializeScalerIP(void)
{
	MDrv_MFC_InitializeRx();
	MDrv_MFC_InitializeIP_PtnGen();
	MDrv_MFC_InitializeIP();
	MDrv_MFC_InitializeOPM();
	MDrv_MFC_InitializeScaler();
	//printk("MDrv_MFC_InitializeScalerIP()\n");
}

#if 0
U16 msIPGetHdeActive(void)
{
	return MDrv_MFC_Read2Bytes(0x207C);
}

U16 msIPGetHtotal(void)
{
	return MDrv_MFC_Read2Bytes(0x2078);
}
#endif

/*
void MDrv_MFC_SoftwareResetScalerInt(void)
{
   		MDrv_MFC_WriteBit(0x1E03, 1, _BIT0);
	  	MDrv_MFC_WriteBit(0x1E03, 0, _BIT0);
}
*/

U16 msIPGetVtotal(void)
{
	return MDrv_MFC_Read2Bytes(0x207A);
}

U16 msIPGetHdeCount(void)
{
	return MDrv_MFC_Read2Bytes(0x20D8);
}

U8 apiIPGetVfreq(void)
{
	U16 wTemp;
	U32 dwFreq;

    // XTAL / Count = Vtotal * Vfreq

	wTemp = msIPGetHdeCount();
	//printf("\r\nHcnt[%x]", wTemp>>15);
	if (!(wTemp&_BIT15))
		return 0xFF;
	wTemp &= ~_BIT15;
	//printf(" [%d]", wTemp);
	dwFreq = MST_CLOCK_HZ / wTemp;

	wTemp = msIPGetVtotal();
	//printf("_____Vtot[%x]", wTemp>>15);
	#ifndef FrameVt_Change
	if (!(wTemp&_BIT15))   //j090302 for ausu
		return 0xFF;
	#endif

	wTemp &= ~_BIT15;
	//printf(" [%d]", wTemp);
	dwFreq /= wTemp;

	//printf("_____ndwFreq[%d]", dwFreq);
	return dwFreq;
}
#endif

