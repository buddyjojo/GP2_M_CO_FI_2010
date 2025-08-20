#define _MSSCALEROP_C_
#include "mdrv_mfc_platform.h"

#include "mdrv_mfc_scalerop.h"
#include "mdrv_mfc_scalerip.h"
#include "mdrv_mfc_fb.h"
#include "mdrv_mfc_panel.h"

#if(CODESIZE_SEL == CODESIZE_ALL)


#ifndef MFC_ENABLE_LVDS_SSC
#define MFC_ENABLE_LVDS_SSC 0 // don't change //default please set to 0
#define MFC_LVDS_SSC_SPAN_DEFAULT		200
#define MFC_LVDS_SSC_STEP_DEFAULT		100
#endif

/*
#if(CODEBASE_SEL == CODEBASE_LINUX)
DECLARE_MUTEX(MFC_MUTEX);
#define MUTEX_LOCK()  down(&MFC_MUTEX)
#define MUTEX_UNLOCK()   up(&MFC_MUTEX)
#endif
*/
extern U8 gLgModelType;

#define _RATIO			1 // 905,969,664
#define _STEP			4 // 4,294,967,295
#define _GAIN_P(f)		(f/(_STEP))
#define _GAIN_I(f)		(f/(_STEP*_STEP/2))

U32 Panel_Dclk_Hz(U16 u16Htotal, U16 u16Vtotal, U8 u8Vfreq)
{
	return (U32)u16Htotal*u16Vtotal*u8Vfreq;
}

U32 FPLL_Panel_Dclk_Hz(U16 u16Htotal, U16 u16Vtotal, U8 u8Vfreq)
{
	return (U32)u16Htotal*u16Vtotal*u8Vfreq;
}

U32 Panel_Dclk_Hz2(U16 u16Htotal, U16 u16Vtotal, U8 u8Vfreq)
{
	U32 u32Dclk=0;
	u32Dclk = msCalculateDecimal(u16Htotal*u16Vtotal*u8Vfreq,1000000);
	return u32Dclk;
}

U32 FPLL_Panel_Dclk_Hz2(U16 u16Htotal, U16 u16Vtotal, U8 u8Vfreq)
{
	U32 u32Dclk=0;
	u32Dclk = msCalculateDecimal(u16Htotal*u16Vtotal*u8Vfreq,1000000);
	return u32Dclk  ;
}

U16 MDrv_MFC_DE_XEnd(U16 u16PnlWidth)
{
	if (IsMultipCheck(u16PnlWidth,4))
		return (gmfcSysInfo.u16HStart/((TwoChip_Func)?2:1) +
				gmfcSysInfo.u16Width/((TwoChip_Func)?2:1)-1);
	else
		return (gmfcSysInfo.u16HStart/((TwoChip_Func)?2:1)+
				gmfcSysInfo.u16Width/((TwoChip_Func)?2:1)+1);
}

void MDrv_MFC_SetGainPhase(void)
{
	U8 i, u8Val;
	U32 u32ClockDivider, u32Factor;

	u32ClockDivider = (Panel_Dclk_Hz(gmfcSysInfo.u16HTotal, gmfcSysInfo.u16VTotal,gmfcSysInfo.u8PanelVfreq)/60*_RATIO)>>5;
	u32Factor = (216ul*524288*8+(7ul*MST_CLOCK_MHZ*u32ClockDivider)/2)/(7ul*MST_CLOCK_MHZ*u32ClockDivider);

	for (i=0; i<0x0F; i++)
	{
		if ((1ul<<i) >= _GAIN_P(u32Factor))
			break;
	}
	u8Val = (i+1)<<4;
	//printf("\r\n_GAIN_P[%d]", _GAIN_P);
	//printf(" i[%d]", i);

	for (i=0; i<0x0F; i++)
	{
		if ((1ul<<i) >= _GAIN_I(u32Factor))
			break;
	}
	u8Val |= i+1;
	//printf("\r\n_GAIN_I[%d]", _GAIN_I);
	//printf(" i[%d]", i);

	//if ((gmfcSysInfo.u8PanelVfreq/60)==2)
	if (gmfcSysInfo.u8PanelVfreq>90)
		i = 0x10; // {ovs_frame_div[3:0],ivs_frame_div[3:0]}
	else
		i = 0x00; // {ovs_frame_div[3:0],ivs_frame_div[3:0]}

	MDrv_MFC_WriteByte(0x2A17, u8Val);
	MDrv_MFC_WriteByte(0x2A19, i);
	//printk("MDrv_MFC_SetGainPhase()\n");
}

U32 MDrv_MFC_SetVCO(U32 u32Dclk, U8 u8Vfreq)
{
	U32 u32VCO;

	if (gmfcSysInfo.u8PanelType == _MINI_LVDS || gmfcSysInfo.u8PanelType ==_MINI_LVDS_GIP || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
	{
    	u32VCO = u32Dclk*4;
		if(u8Vfreq==60)
		{
			if(u32VCO>313)
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT1, _BIT2|_BIT1|_BIT0); ///2
                MDrv_MFC_WriteByte(0x2A2C,_BIT3|_BIT2);
			}
			else if(u32VCO>156)
			{
			  	MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C,_BIT4|_BIT2);
			}
			else
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C, _BIT4 | _BIT3|_BIT2);
			}
		}
		else //120
		{
			if(u32VCO>625)
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT1, _BIT2|_BIT1|_BIT0); ///2
				MDrv_MFC_WriteByte(0x2A2C, 0);
			}
			else if(u32VCO>313)
			{
	            MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C, _BIT3|_BIT2);
			}
			else
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0); ///4
                MDrv_MFC_WriteByte(0x2A2C, _BIT4|_BIT2);
			}
		}
    }
    else //LVDS
	{
    	if (gmfcSysInfo.u8PanelChannel == _DUAL)
    	{
			u32Dclk = u32Dclk/2;
			u32VCO = u32Dclk*7;
			if(u32VCO>625)
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C, 0);
			}
			else
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C, _BIT3|_BIT2);
			}
		}
		else if (gmfcSysInfo.u8PanelChannel == _QUAD || gmfcSysInfo.u8PanelChannel == _QUAD_LR)
		{
			u32VCO = u32Dclk/2*7;
			if(u32VCO>625)
			{
				MDrv_MFC_WriteByteMask(0x2A06,_BIT1, _BIT2|_BIT1|_BIT0); ///2
                MDrv_MFC_WriteByte(0x2A2C, 0);
			}
			else  if(u32VCO>313)
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0);  ///6
                MDrv_MFC_WriteByte(0x2A2C, _BIT3|_BIT2);
			}
			else
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C, _BIT4|_BIT2);
			}
		}
		else if (gmfcSysInfo.u8PanelChannel == _V_BY1)
		{
			u32VCO = u32Dclk*10;
			if(u32VCO>625)
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C, 0);
			}
			else
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C, _BIT3|_BIT2);
			}
		}
		else //single
		{
			u32Dclk = u32Dclk/2;
			u32VCO = u32Dclk*7;
			if(u32VCO>625)
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C, 0);
			}
			else
			{
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C, _BIT3|_BIT2);
			}
		}
    }
    #if ( TwoChip_Func != TwoChip_OFF )
      MDrv_MFC_WriteBit(0x2A2C, 1, _BIT0);
    #endif
	return u32Dclk;
}

void MDrv_MFC_SetLvdsSSC(U16 u16KHz, U16  u16Percent, BOOL bEnable)
{
	// SPAN value, recommend value is 30KHz ~ 40KHz
    // STEP percent value, recommend is under 3%
    U8  u8temp;
    U16 u16Span, u16Step;
    U32 u32Set;

    if(u16KHz&&bEnable)
    {
    	MDrv_MFC_WriteBit(0x2A1B, _ENABLE, _BIT3);
        //Get SET
        u8temp  = MDrv_MFC_ReadByte (0x2A20);
        u32Set = u8temp;
        u8temp  = MDrv_MFC_ReadByte (0x2A1F);
        u32Set = (u32Set << 8) | u8temp;
        u8temp = MDrv_MFC_ReadByte (0x2A1E);
        u32Set = (u32Set << 8) | u8temp;

        if(u32Set*u16KHz>999)
        {
            u16Span = ((131072*10*216)/(u32Set*u16KHz/1000));
            if(u16Span)
            {
                u16Step = (u32Set*u16Percent)/((U32)u16Span*10000);

                if(u16Step>0x3ff) u16Step=0x3ff;
                if(u16Span>0x3fff) u16Span=0x3fff;

                MDrv_MFC_WriteByte (0x2A2E, 	(U8)u16Step);  //write step 10 bits
                MDrv_MFC_WriteByteMask (0x2A2F, (U8)((u16Step&0x0300)>>8), 0x03);
                MDrv_MFC_WriteByte (0x2A30, 	(U8)u16Span);  //write SPAN 14 bits
                MDrv_MFC_WriteByteMask (0x2A31, (U8)((u16Span&0x3F00)>>8), 0x3F);
                return; // set SSC
            }
        }
    }
    else
    {
    // disable
    MDrv_MFC_WriteByte(0x2A2E, 0x00);
    MDrv_MFC_WriteByte(0x2A2F, 0x00);
    MDrv_MFC_WriteByte(0x2A30, 0x00);
    MDrv_MFC_WriteByte(0x2A31, 0x00);
	MDrv_MFC_WriteBit(0x2A1B, _DISABLE, _BIT3);
}
}


#if(CODEBASE_SEL == CODEBASE_51)

void msSetFPLLGainPhase(void)
	{
	U8 i, u8Val;
	U32 u32ClockDivider, u32Factor;

	u32ClockDivider = (FPLL_Panel_Dclk_Hz(gmfcSysInfo.u16HTotal, gmfcSysInfo.u16VTotal,gmfcSysInfo.u8PanelVfreq)/60*_RATIO)>>5;
	u32Factor = (216ul*524288*8+(7ul*MST_CLOCK_MHZ*u32ClockDivider)/2)/(7ul*MST_CLOCK_MHZ*u32ClockDivider);

	for (i=0; i<0x0F; i++)
	{
		if ((1ul<<i) >= _GAIN_P(u32Factor))
			break;
	}
	u8Val = (i+1)<<4;
	//printf("\r\n_GAIN_P[%d]", _GAIN_P);
	//printf(" i[%d]", i);

	for (i=0; i<0x0F; i++)
	{
		if ((1ul<<i) >= _GAIN_I(u32Factor))
			break;
    }
	u8Val |= i+1;
	//printf("\r\n_GAIN_I[%d]", _GAIN_I);
	//printf(" i[%d]", i);


	if ((gmfcSysInfo.u8PanelVfreq/60)==2)
		i = 0x10; // {ovs_frame_div[3:0],ivs_frame_div[3:0]}
	else
		i = 0x00; // {ovs_frame_div[3:0],ivs_frame_div[3:0]}

	MDrv_MFC_WriteByte(0x2A17, u8Val);
	MDrv_MFC_WriteByte(0x2A19, i);
}

U32 msLPLL_SetVCO(U32 dwDclk, U8 ucPanelVfreq)
{
	U32 dwVCO;
	//printf("\r\n dwDclk = %x", dwDclk);
	//printf("\r\n ucVfreq = %x", ucVfreq);

    if (gmfcSysInfo.u8PanelType == _MINI_LVDS || gmfcSysInfo.u8PanelType ==_MINI_LVDS_GIP || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
    {
    	dwVCO = dwDclk*4;

	if(ucPanelVfreq==60)
	{
		if(dwVCO>313)
		{
			MDrv_MFC_WriteByteMask(0x2A06, _BIT1, _BIT2|_BIT1|_BIT0); ///2
                MDrv_MFC_WriteByte(0x2A2C, _BIT3|_BIT2);
		}
		else if(dwVCO>156)
		{
			MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C, _BIT4|_BIT2);
		}
		else
		{
			MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C, _BIT4 | _BIT3|_BIT2);
		}
	}
	else //120
	{
              if(dwVCO>625)
              {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT1, _BIT2|_BIT1|_BIT0); ///2
				MDrv_MFC_WriteByte(0x2A2C, 0);
              }
              else if(dwVCO>313)
              {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C, _BIT3|_BIT2);
	}
	       else
              {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0); ///4
                MDrv_MFC_WriteByte(0x2A2C, _BIT4|_BIT2);
              }
	}
    }
    else //LVDS
    {
    	if (gmfcSysInfo.u8PanelChannel == _DUAL)
	{
			//printf("\r\nDual = %x", 0);

	   dwDclk = dwDclk/2;
	   dwVCO = dwDclk*7;
          if(dwVCO>625)
          {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C,0); //0
          }
          else
          {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C,_BIT3|_BIT2); //c
          }
	}
		else if (gmfcSysInfo.u8PanelChannel == _QUAD || gmfcSysInfo.u8PanelChannel == _QUAD_LR)
	{
	   dwVCO = dwDclk*3.5;
          if(dwVCO>625)
          {
				MDrv_MFC_WriteByteMask(0x2A06,_BIT1, _BIT2|_BIT1|_BIT0); ///2
                MDrv_MFC_WriteByte(0x2A2C,0); //0
          }
          else  if(dwVCO>313)
          {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0);  ///6
                MDrv_MFC_WriteByte(0x2A2C,_BIT3|_BIT2); //c
          }
	   else
          {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C,_BIT4|_BIT2); //c
          }
	}
		else if (gmfcSysInfo.u8PanelChannel == _V_BY1)
	{
          dwVCO = dwDclk*10;
          if(dwVCO>625)
          {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C,0); //0
          }
          else
          {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C,_BIT3|_BIT2); //c
          }
	}
	else //single
	{
	   dwDclk = dwDclk/2;
	   dwVCO = dwDclk*7;
          if(dwVCO>625)
          {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2|_BIT1, _BIT2|_BIT1|_BIT0); ///6
                MDrv_MFC_WriteByte(0x2A2C,0); //0
          }
          else
          {
				MDrv_MFC_WriteByteMask(0x2A06, _BIT2, _BIT2|_BIT1|_BIT0);  ///4
                MDrv_MFC_WriteByte(0x2A2C,_BIT3|_BIT2); //c
          }
	}
    }
	#if ( TwoChip_Func != TwoChip_OFF )
	  MDrv_MFC_WriteBit(0x2A2C, 1, _BIT0);
	#endif
	return dwDclk;
}

void msSetOutDClk(U8 u8Inputfreq, U8 ucVHzFrmT2/*Set panel frequence from T2*/, BOOL enableFPLL)
{
	U16 wVTotal = gmfcSysInfo.u16VTotal;
       U32 dwDClk ;
	if(ucVHzFrmT2)
	{
		gmfcSysInfo.u8PanelVfreq = ucVHzFrmT2;
		MDrv_MFC_SetGainPhase();
	}

    #if !SW_FILM_STATE_MACHINE
    if(u8Inputfreq>45&&u8Inputfreq<55)
    {
        MDrv_MFC_WriteBit(0x2080, _DISABLE, _BIT0); // disable film32
    }
    else
    {
        MDrv_MFC_WriteBit(0x2080, _ENABLE, _BIT0); // enable film32
    }
    #endif

    MDrv_MFC_WriteBit(0x2A18, 0, _BIT3); // PLL set chnage Toggle
    #if(ENABLE_USER_TOTAL)
        if (u8Inputfreq==50)
        {
            gmfcSysInfo.u16HTotal = USER_HT_50;
            gmfcSysInfo.u16VTotal = USER_VT_50;
        }
        else //60
        {
            gmfcSysInfo.u16HTotal = USER_HT_60;
            gmfcSysInfo.u16VTotal = USER_VT_60;
        }
        dwDClk = MDrv_MFC_SetVCO(Panel_Dclk_Hz2(gmfcSysInfo.u16HTotal, gmfcSysInfo.u16VTotal, u8Inputfreq/*u8Inputfreq*/), gmfcSysInfo.u8PanelVfreq);
            MDrv_MFC_Write2BytesINT(REG_2F02, gmfcSysInfo.u16VTotal-1);
                  MDrv_MFC_Write2BytesINT(REG_2F04, gmfcSysInfo.u16HTotal-1);
    #else
	dwDClk = MDrv_MFC_SetVCO(Panel_Dclk_Hz2(gmfcSysInfo.u16HTotal, gmfcSysInfo.u16VTotal, 60/*u8Inputfreq*/), gmfcSysInfo.u8PanelVfreq);

	if (u8Inputfreq==50)
{
		if (gmfcSysInfo.u8PanelIncVtotalFor50Hz)
		{
			wVTotal = wVTotal*6/5;
		}
		else
		{
			dwDClk = dwDClk*5/6;
		}
	}
       MDrv_MFC_Write2BytesINT(REG_2F02, wVTotal-1);
    #endif

    //printf("\r\nVTotal[%d]", wVTotal);
    //printf("\r\nDClk[%x]", dwDClk>>16);
    //printf("\r\n DClk1[%x]", dwDClk);
    //printf("u8Inputfreq=[%x]", u8Inputfreq);

	if (gmfcSysInfo.u8PanelType == _MINI_LVDS
	 || gmfcSysInfo.u8PanelType == _RSDS
	 || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP
	 || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
    {
	    if(gmfcSysInfo.u8PanelVfreq == 60)
	    dwDClk = (U32)PANEL_DCLK_FACTOR_TCON_60 / dwDClk;
	    else
	    dwDClk = (U32)PANEL_DCLK_FACTOR_TCON / dwDClk;
	}
	else
	    dwDClk = (U32)PANEL_DCLK_FACTOR / dwDClk;

	if (gmfcSysInfo.u8PanelChannel == _QUAD || gmfcSysInfo.u8PanelChannel == _QUAD_LR)
	{
	    //putstr("\r\nmsSetOutDClk(Quad)");
	}
	else if(gmfcSysInfo.u8PanelChannel == _DUAL)
              {
	    //putstr("\r\nmsSetOutDClk(Dual)");
	    dwDClk /= 2;
              }

    //printf("\r\nLPLL[%x]", dwDClk>>16);
    //printf("[%x]", dwDClk);
	MDrv_MFC_Write3Bytes(0x2A1E, dwDClk);

    if(enableFPLL) MDrv_MFC_WriteBit(0x2A18, 1, _BIT3); // PLL set chnage Toggle
    MDrv_MFC_WriteByte(0x2A0A, 0x24);// make frame lock faster

    MDrv_MFC_SetLvdsSSC(MFC_LVDS_SSC_SPAN_DEFAULT, MFC_LVDS_SSC_STEP_DEFAULT, MDrv_MFC_ReadByte(0x2C42)&0x20);
}
/*
void MDrv_MFC_SetLvdsSSC_INT(U16 u16KHz, U16  u16Percent, BOOL bEnable)
{
	// SPAN value, recommend value is 30KHz ~ 40KHz
    // STEP percent value, recommend is under 3%
    U8  u8temp;
    U16 u16Span, u16Step;
    U32 u32Set;

    if(u16KHz&&bEnable)
    {
    	MDrv_MFC_WriteBitINT(0x2A1B, _ENABLE, _BIT3);
        //Get SET
        u8temp  = MDrv_MFC_ReadByte (0x2A20);
        u32Set = u8temp;
        u8temp  = MDrv_MFC_ReadByte (0x2A1F);
        u32Set = (u32Set << 8) | u8temp;
        u8temp = MDrv_MFC_ReadByte (0x2A1E);
        u32Set = (u32Set << 8) | u8temp;

        if(u32Set*u16KHz>999)
        {
            u16Span = ((131072*10*216)/(u32Set*u16KHz/1000));
            if(u16Span)
            {
                u16Step = (u32Set*u16Percent)/((U32)u16Span*10000);

                if(u16Step>0x3ff) u16Step=0x3ff;
                if(u16Span>0x3fff) u16Span=0x3fff;

                MDrv_MFC_WriteByte (0x2A2E, (U8  )u16Step);  //write step 10 bits
                MDrv_MFC_WriteByteMaskINT (0x2A2F, (U8  )((u16Step&0x0300)>>8), 0x03);
                MDrv_MFC_WriteByte (0x2A30, (U8  )u16Span);  //write SPAN 14 bits
                MDrv_MFC_WriteByteMaskINT (0x2A31, (U8  )((u16Span&0x3F00)>>8), 0x3F);
                return; // enable SSC
            }
        }
	}

    // disable
    MDrv_MFC_WriteByte(0x2A2E, 0x00);
    MDrv_MFC_WriteByte(0x2A2F, 0x00);
    MDrv_MFC_WriteByte(0x2A30, 0x00);
    MDrv_MFC_WriteByte(0x2A31, 0x00);
	MDrv_MFC_WriteBitINT(0x2A1B, _DISABLE, _BIT3);
}
*/
#if 0
void msReportCurser(void)
{
WORD wTemp1;
BYTE i;
	MDrv_MFC_WriteByte(REG_2F40, 0x4B); //10
	MDrv_MFC_WriteByte(REG_2F41, 0x84); //81
	MDrv_MFC_WriteByte(REG_2F42, 0x00);
	MDrv_MFC_WriteByte(REG_2F43, 0x01);
	MDrv_MFC_WriteBit(0x30A0, 1, _BIT1);

	for(i=0;i<=0x64;i++)
	{
       	wTemp1 = MDrv_MFC_Read2Bytes(0x30A4);
		//printf(" \r\n data= %x", wTemp1);
    		 Delay1ms_Nop(0x10) ;

		if(wTemp1 != 0x00)
		       printf(" \r\n different --------= %x", wTemp1);

	}

	    printf(" \r\n end----- %x", wTemp1);
}
#endif

#if (ENABLE_Mst_func3_PWM_Freq)
extern XDATA BYTE updatePwmFreq;
#endif
//XDATA BYTE OPMReset=1;
void msSetFPLLOutDClk(U8 ucVfreq, U8 ucVHzFrmT2/*Set panel frequence from T2*/, BOOL enableFPLL)
{
    #if(PANEL_TYPE_SEL ==PNL_TVBOX_1080P_60HZ)
    	U16 wHTotal = 2200;
    #else
	U16 wVTotal = gmfcSysInfo.u16VTotal;
    #endif
       U32 dwDClk ;

    MDrv_MFC_WriteByte(0x2C49, ucVfreq);
#if 0
    if(/*enableFPLL&&*/OPMReset)
    {
        MDrv_MFC_SoftwareResetScaler();
        OPMReset=0;
    }
#endif
    if(ucVfreq>45&&ucVfreq<55)
    {
#if (ENABLE_Mst_func3_PWM_Freq)
        updatePwmFreq=100;
#endif
        #if !SW_FILM_STATE_MACHINE
        MDrv_MFC_WriteBit(0x2080, _DISABLE, _BIT0); // disable film32
        #endif
    }
    else
    {
#if (ENABLE_Mst_func3_PWM_Freq)
        updatePwmFreq=120;
#endif
        #if !SW_FILM_STATE_MACHINE
        MDrv_MFC_WriteBit(0x2080, _ENABLE, _BIT0); // enable film32
        #endif
    }


	if(ucVHzFrmT2)
              {
		gmfcSysInfo.u8PanelVfreq = ucVHzFrmT2;
		msSetFPLLGainPhase();
              }

    MDrv_MFC_WriteBit(0x2A18, 0, _BIT3); // PLL set chnage Toggle

    #if(ENABLE_USER_TOTAL)
        if (ucVfreq==50)
        {
            gmfcSysInfo.u16HTotal = USER_HT_50;
            gmfcSysInfo.u16VTotal = USER_VT_50;
    }
        else //60
              {
            gmfcSysInfo.u16HTotal = USER_HT_60;
            gmfcSysInfo.u16VTotal = USER_VT_60;
              }
        dwDClk = msLPLL_SetVCO(FPLL_Panel_Dclk_Hz2(gmfcSysInfo.u16HTotal, gmfcSysInfo.u16VTotal, ucVfreq/*u8Inputfreq*/), gmfcSysInfo.u8PanelVfreq);
            MDrv_MFC_Write2Bytes(REG_2F02, gmfcSysInfo.u16VTotal-1);
                  MDrv_MFC_Write2Bytes(REG_2F04, gmfcSysInfo.u16HTotal-1);
    #else
	dwDClk = msLPLL_SetVCO(FPLL_Panel_Dclk_Hz2(gmfcSysInfo.u16HTotal, gmfcSysInfo.u16VTotal, 60/*u8Inputfreq*/), gmfcSysInfo.u8PanelVfreq);

	if (ucVfreq==50)
	{
		if (gmfcSysInfo.u8PanelIncVtotalFor50Hz)
		{
			#if(PANEL_TYPE_SEL ==PNL_TVBOX_1080P_60HZ)
				wHTotal = 2640;
			#else
				wVTotal = wVTotal*6/5;
			#endif

		}
		else
		{
			dwDClk = dwDClk*5/6;
		}
	}
	 #if(PANEL_TYPE_SEL ==PNL_TVBOX_1080P_60HZ)
          MDrv_MFC_Write2Bytes(REG_2F04, wHTotal-1);
	 #else
          MDrv_MFC_Write2Bytes(REG_2F02, wVTotal-1);
    #endif
    #endif

    //printf("\r\nVTotal[%d]", wVTotal);
    //printf("\r\nDClk[%x]", dwDClk>>16);
    //printf("\r\n DClk2[%x]", dwDClk);
    //printf("ucVfreq=[%x]", ucVfreq);


	if (gmfcSysInfo.u8PanelType == _MINI_LVDS
	 || gmfcSysInfo.u8PanelType == _RSDS
	 || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP
	 || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
	{
	    if(gmfcSysInfo.u8PanelVfreq == 60)
	    dwDClk = (U32)PANEL_DCLK_FACTOR_TCON_60 / dwDClk;
	    else
	    dwDClk = (U32)PANEL_DCLK_FACTOR_TCON / dwDClk;
	}
	else
	    dwDClk = (U32)PANEL_DCLK_FACTOR / dwDClk;

	if (gmfcSysInfo.u8PanelChannel == _QUAD || gmfcSysInfo.u8PanelChannel == _QUAD_LR)
          {
	    //putstr("\r\nmsSetOutDClk(Quad)");
          }
	else if(gmfcSysInfo.u8PanelChannel == _DUAL)
          {
	    //putstr("\r\nmsSetOutDClk(Dual)");
	    dwDClk /= 2;
          }

    //printf("\r\nLPLL[%x]", dwDClk>>16);
    //printf("[%x]", dwDClk);
    MDrv_MFC_Write3Bytes(0x2A1E, dwDClk);
    if(enableFPLL) MDrv_MFC_WriteBit(0x2A18, 1, _BIT3); // PLL set chnage Toggle
    MDrv_MFC_WriteByte(0x2A0A, 0x24);// make frame lock faster

    MDrv_MFC_SetLvdsSSC(MFC_LVDS_SSC_SPAN_DEFAULT, MFC_LVDS_SSC_STEP_DEFAULT, MDrv_MFC_ReadByte(0x2C42)&0x20);
}

#else
// [u8Vfreq] for detected input frequence
// [enableFPLL] FPLL enable/disable
#if (ENABLE_Mst_func3_PWM_Freq)
extern XDATA U8 updatePwmFreq;
#endif
void MDrv_MFC_SetOutDClk(U16 u16InputfreqX100, BOOL enableFPLL)
{
    U8  ucDCLKVFreq;
	U16 u16VTotal = gmfcSysInfo.u16VTotal;
    U32 u32DClk ;

    MDrv_MFC_WriteByte(0x2C49, (U8)(u16InputfreqX100/100));
#if 0
    if(/*enableFPLL && */(MDrv_MFC_ReadByte(0x2c45)&0x20) == 0 )
    {
   		MDrv_MFC_WriteBit(0x1E03, 1, _BIT0);
	  	MDrv_MFC_WriteBit(0x1E03, 0, _BIT0);

        MDrv_MFC_WriteByteMask(0x2c45, 0x20, 0x20);
    }
#endif
    if(u16InputfreqX100>4500&&u16InputfreqX100<5500)
    {
        #if !SW_FILM_STATE_MACHINE
        MDrv_MFC_WriteBit(0x2080, _DISABLE, _BIT0); // disable film32
        #endif
        #if (ENABLE_Mst_func3_PWM_Freq)
        updatePwmFreq=100;
        #endif
    }
    else //if(u16InputfreqX100>=55&&u16InputfreqX100<65)
    {
        #if !SW_FILM_STATE_MACHINE
        MDrv_MFC_WriteBit(0x2080, _ENABLE, _BIT0);  // enable film32
        #endif
        #if (ENABLE_Mst_func3_PWM_Freq)
        updatePwmFreq=120;
        #endif
    }

//	MUTEX_LOCK();
	//printf("\n\n\n\n\nGet input freq from T2=[%d]\n\n\n\n\n", u16InputfreqX100);
	if(!MDrv_MFC_GetInitStatus())
	{
		//MUTEX_UNLOCK();
		return;
	}
     MDrv_MFC_WriteBit(0x2A18, 0, _BIT3); // PLL set chnage Toggle
    ucDCLKVFreq = gmfcSysInfo.u8PanelVfreq/2;
	u32DClk = MDrv_MFC_SetVCO(
                  msCalculateDecimal(
			  					Panel_Dclk_Hz(gmfcSysInfo.u16HTotal,
			  					  			  gmfcSysInfo.u16VTotal,
			  					  			  ucDCLKVFreq),
			  					1000000
			  					), gmfcSysInfo.u8PanelVfreq);
    #if 0
	if (u8Inputfreq==50)
	{
		if (gmfcSysInfo.u8PanelIncVtotalFor50Hz)
          {
			u16VTotal = u16VTotal*6/5;
          }
	   else
          {
			u32DClk = u32DClk*5/6;
          }
	}
	#else
    if( (MDrv_MFC_ReadByte(0x1E4C)&_BIT0)==0 )
    {
        if (gmfcSysInfo.u8PanelIncVtotalFor50Hz
            && ( gmfcSysInfo.u8PanelVfreq >90 || gmfcSysInfo.u8PanelVfreq <65))
    {
        u16VTotal = (U16)((U32)u16VTotal*6000/u16InputfreqX100);
    }
    else
    {
        u32DClk = u32DClk*u16InputfreqX100/6000;
    }
    }
	#endif
	//MDrv_MFC_WriteByte(0x2A1A,MDrv_MFC_ReadByte(0x2A1A)|0x03);
	MDrv_MFC_WriteBit(0x2A18, 0, _BIT3); // PLL set chnage Toggle
	MDrv_MFC_Write2Bytes(0x2F02, u16VTotal-1);

	if (gmfcSysInfo.u8PanelType == _MINI_LVDS
	 || gmfcSysInfo.u8PanelType == _RSDS
	 || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP
	 || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
          {
	    if(gmfcSysInfo.u8PanelVfreq == 60 && gmfcSysInfo.u8PanelChannel == _DUAL)
	    	u32DClk = (U32)PANEL_DCLK_FACTOR_TCON_60 / u32DClk;
	    else
	    	u32DClk = (U32)PANEL_DCLK_FACTOR_TCON / u32DClk;
          }
          else
	    u32DClk = (U32)PANEL_DCLK_FACTOR / u32DClk;

	if (gmfcSysInfo.u8PanelChannel == _QUAD || gmfcSysInfo.u8PanelChannel == _QUAD_LR)
          {
        if( MDrv_MFC_ReadByte(0x2C47)== 60)
            u32DClk /= 2;
	    //printk("\nmsSetOutDClk(Quad)");
          }
	else if(gmfcSysInfo.u8PanelChannel == _DUAL)
	{
	    //printk("\nmsSetOutDClk(Dual)");
	    u32DClk /= 2;
	}
	MDrv_MFC_Write3Bytes(0x2A1E, u32DClk);
	if(enableFPLL
        &&(gmfcSysInfo.u8PanelVfreq>90||gmfcSysInfo.u8PanelVfreq<65)
        &&((MDrv_MFC_ReadByte(0x1E4C)&_BIT0)==0)
    )
        MDrv_MFC_WriteBit(0x2A18, 1, _BIT3); // PLL set chnage Toggle

    MDrv_MFC_WriteByte(0x2A0A, 0x24);// make frame lock faster

    #if (ENABLE_Mst_func3_PWM_Freq)
    //appPWMHandler();
    #endif

    MDrv_MFC_SetLvdsSSC(MFC_LVDS_SSC_SPAN_DEFAULT, MFC_LVDS_SSC_STEP_DEFAULT, MDrv_MFC_ReadByte(0x2C42)&0x20);
	//MUTEX_UNLOCK();
}
#endif


void MDrv_MFC_LPLL_Initialize(void)
{
	MDrv_MFC_Write2Bytes(0x2A5C,  0xffec);
	MDrv_MFC_WriteBit(0x2A06, 0, _BIT5); // power gating

    if (gmfcSysInfo.u8PanelType == _MINI_LVDS || gmfcSysInfo.u8PanelType ==_MINI_LVDS_GIP || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
    {
		MDrv_MFC_WriteByte(0x2A03, 0x01); //div_2nd
		MDrv_MFC_WriteBit(0x2A06, 0, _BIT7); //dual to quad mode
		MDrv_MFC_WriteByte(0x2A02, 0x03); //div_1st
		MDrv_MFC_WriteBit(0x2A04, 1, _BIT0); // mini_en
		MDrv_MFC_WriteByteMask(0x2A56, 0, _BIT5 | _BIT4);  //sel_432m // v_by1_en
		MDrv_MFC_WriteBit(0x2A5C, 1, _BIT6); //sel_216m
	    MDrv_MFC_WriteByteMask(0x2A5A, _BIT3, _BIT3|_BIT2);  //test bus = 8
    }
#if (!TwoChip_Func)
    else if(gmfcSysInfo.u8PanelType == _TTL) //j091118
    {
            MDrv_MFC_WriteByte(0x2A03, 0x04);   //div_1st
            MDrv_MFC_WriteBit(0x2A06, 1, _BIT7); //dual to quad mode

            MDrv_MFC_WriteByte(0x2A02, 0x03);
            MDrv_MFC_WriteBit(0x2A04, 0, _BIT0); // mini_en
            MDrv_MFC_WriteByteMask(0x2A56, 0, _BIT5 | _BIT4);  //sel_432m // v_by1_en
            MDrv_MFC_WriteBit(0x2A5C, 1, _BIT6); //sel_216m
            MDrv_MFC_WriteByteMask(0x2A5A, 0,  _BIT3|_BIT2);  //test bus = 0
    }
 #endif

	else
	{
		if (gmfcSysInfo.u8PanelChannel == _DUAL)
		{
			MDrv_MFC_WriteByte(0x2A03, 0x04);   //div_1st
			MDrv_MFC_WriteBit(0x2A06, 0, _BIT7); //dual to quad mode
		}
		else if (gmfcSysInfo.u8PanelChannel == _QUAD || gmfcSysInfo.u8PanelChannel == _QUAD_LR)
		{
			MDrv_MFC_WriteByte(0x2A03, 0x04);
            //MDrv_MFC_WriteBit(0x2A06, (TwoChip_Func)?0:1, _BIT7); //dual to quad mode  //j090923
            MDrv_MFC_WriteBit(0x2A06, 1, _BIT7); //dual to quad mode

        }
		else if (gmfcSysInfo.u8PanelChannel == _V_BY1)
		{
			MDrv_MFC_WriteByte(0x2A03, 0x01);
			MDrv_MFC_WriteBit(0x2A06, 0, _BIT7); //dual to quad mode
		}
		else //single
		{
			MDrv_MFC_WriteByte(0x2A03, 0x04);   //div_1st
			MDrv_MFC_WriteBit(0x2A06, 0, _BIT7); //dual to quad mode
		}
	    MDrv_MFC_WriteByte(0x2A02, 0x03);
		MDrv_MFC_WriteBit(0x2A04, 0, _BIT0); // mini_en
		MDrv_MFC_WriteByteMask(0x2A56, 0, _BIT5 | _BIT4);  //sel_432m // v_by1_en
		MDrv_MFC_WriteBit(0x2A5C, 1, _BIT6); //sel_216m
	    MDrv_MFC_WriteByteMask(0x2A5A, 0,  _BIT3|_BIT2);  //test bus = 0
	    //printk("MDrv_MFC_InitializeLPLL()\n");
    }
}

code MST_MFC_RegUnitType_t tInitializeDispLpll[]=
{
    {0x2A00, LPLL_INPUT_DIVIDER_1ST},
    {0x2A01, LPLL_INPUT_DIVIDER_2ND},
   	//{0x2A03, 0x04},
   	//{0x2A06, 0x02},
    {0x2A82,0x45}, //AuPll
    {0x2A83,0x01}, //AuPll
    {0x2AA8,0x10},//Bit4, AuPll Lock En
	{0x2A0B, 0xC0}, // lock thr,
	{0x2A0C, 0x00}, //0x01, // limit_d5d6d7
	{0x2A0D, 0x00}, //0x80, // limit_d5d6d7
	{0x2A0E, 0x01}, //0x06, // limit_d5d6d7
	{0x2A0F, 0x00},
	{0x2A12, 0x03}, // limit_d5d6d7
	{0x2A14, 0x00},
    {0x2A15, 0xd0},//0x80, //j090105 for Tvbox fpll lock
	{0x2A16, 0x00}, // {p_gain_prd[3:0]  ,i_gain_prd[3:0]}
	#if(CODEBASE_SEL == CODEBASE_51)
	{0x2A1B, 0x07}, // sel ovs as clock div
	#else
	{0x2A1B, 0x06}, // sel ovs as clock div (FPLL do not check no_signal flag when TTL input)
	#endif
	{_END_OF_TBL_, _END_OF_TBL_},
};

code MST_MFC_RegUnitType_t tInitializeDispTgen[]=
{
  #if(PANEL_TYPE_SEL ==PNL_TVBOX_1080P_60HZ)
		{0x2F06, 0x10},
  #endif
	{0x2F2E, 0x01}, // 0x07 -> test pattern
	{_END_OF_TBL_, _END_OF_TBL_},
};

void MDrv_MFC_InitializeDispTgen(void)
{
	// DISP_TGEN
#if (CODEBASE_SEL == CODEBASE_51)
#if (REG_DIRECT_ACCESS_BY_I2C)
		if (gmfcSysInfo.u8Preset == 0x01)
#endif
		#endif
	{
		MDrv_MFC_Write2Bytes(0x2F02, gmfcSysInfo.u16VTotal-1);
		MDrv_MFC_Write2Bytes(0x2F04, gmfcSysInfo.u16HTotal/((TwoChip_Func)?2:1)-1);
        if(gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP|| gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
		{
    		MDrv_MFC_Write2Bytes(0x2F14, gmfcSysInfo.u16VStart-6);
    		MDrv_MFC_Write2Bytes(0x2F16, gmfcSysInfo.u16VStart+gmfcSysInfo.u16Height);
		}
		else
		{
		    MDrv_MFC_Write2Bytes(0x2F14, gmfcSysInfo.u16VStart);
		    MDrv_MFC_Write2Bytes(0x2F16, gmfcSysInfo.u16VStart+gmfcSysInfo.u16Height-1);
		}

		#if ( TwoChip_Func == TwoChip_Master )
			MDrv_MFC_Write2Bytes(0x2F18, gmfcSysInfo.u16HStart/2);
			MDrv_MFC_Write2Bytes(0x2F1A, MDrv_MFC_DE_XEnd(gmfcSysInfo.u16Width));
		#elif ( TwoChip_Func == TwoChip_Slave )
			MDrv_MFC_Write2Bytes(0x2F18, gmfcSysInfo.u16HStart/2 -Cable_effect);
			MDrv_MFC_Write2Bytes(0x2F1A, MDrv_MFC_DE_XEnd(gmfcSysInfo.u16Width) - Cable_effect );
		#else
			MDrv_MFC_Write2Bytes(0x2F18, gmfcSysInfo.u16HStart);
			MDrv_MFC_Write2Bytes(0x2F1A, MDrv_MFC_DE_XEnd(gmfcSysInfo.u16Width));
		#endif
	}

	MDrv_MFC_Write2Bytes(0x2F1C, gmfcSysInfo.u16VStart);
	MDrv_MFC_Write2Bytes(0x2F1E, gmfcSysInfo.u16VStart+gmfcSysInfo.u16Height-1);

	#if ( TwoChip_Func == TwoChip_Master )
		MDrv_MFC_Write2Bytes(0x2F20, gmfcSysInfo.u16HStart/2);
		MDrv_MFC_Write2Bytes(0x2F22, MDrv_MFC_DE_XEnd(gmfcSysInfo.u16Width));
	#elif ( TwoChip_Func == TwoChip_Slave )
		MDrv_MFC_Write2Bytes(0x2F20, gmfcSysInfo.u16HStart/2 -Cable_effect);
		MDrv_MFC_Write2Bytes(0x2F22, 0x040d);//DE_XEnd2(gmfcSysInfo.u16Width -Cable_effect)); //j090508
	#else
		MDrv_MFC_Write2Bytes(0x2F20, gmfcSysInfo.u16HStart);
		MDrv_MFC_Write2Bytes(0x2F22, MDrv_MFC_DE_XEnd(gmfcSysInfo.u16Width));
	#endif

    MDrv_MFC_WriteRegsTbl(0x2F00, tInitializeDispTgen); // initialize all of bank
	MDrv_MFC_WriteRegsTbl(0x2A00, tInitializeDispLpll);
 	if((gmfcSysInfo.u16VStart+gmfcSysInfo.u16Height)> gmfcSysInfo.u16VTotal)
  		MDrv_MFC_Write2Bytes(0x2F06,  gmfcSysInfo.u16VStart+gmfcSysInfo.u16Height -gmfcSysInfo.u16VTotal);//Vtrig_Y
  	else
        MDrv_MFC_Write2Bytes(0x2F06,  gmfcSysInfo.u16VStart+gmfcSysInfo.u16Height+14);//Vtrig_Y
	MDrv_MFC_Write2Bytes(0x2F08, (gmfcSysInfo.u16HStart+gmfcSysInfo.u16Width+15)/((TwoChip_Func)?2:1)); //Vtrig_X
	MDrv_MFC_LPLL_Initialize();
	MDrv_MFC_SetGainPhase();

	#if(CODEBASE_SEL == CODEBASE_51)
        msSetOutDClk(60, 0, FALSE);
 	#else
 		MDrv_MFC_SetInitStatus(1);
		MDrv_MFC_SetOutDClk(6000, FALSE); // when init, the output should be free run.
		MDrv_MFC_SetInitStatus(0);
 	#endif

    #ifdef FrameVt_Change
    	MDrv_MFC_WriteBit(0x2A1A, 1, _BIT0); //
    #endif

	if (gmfcSysInfo.u8PanelType == _RSDS)
	{
		MDrv_MFC_WriteByte(0x1E0E, 0x04); 	// [3:0]od clk:[2]=1:1/2,[1]=1:inverse,[0]=1:gating; [7:4]op2 sram;
		MDrv_MFC_WriteByteMask(0x3200, _BIT1|_BIT0, _BIT1|_BIT0); // for od path, [1]=1:od, =0:mft
		MDrv_MFC_WriteBit(0x3201, 1, _BIT1); // [1]=1, dual/quad mode
		//MDrv_MFC_WriteByteMask(0x2A06, _BIT7|_BIT6, _BIT7|_BIT6); // [7]=1:LPLL double; [6]=1:LPLL type for Rsds
	}
	else if (gmfcSysInfo.u8PanelType == _TTL)
    {
		MDrv_MFC_WriteByte(0x1E0E, 0x04); // [3:0]od clk:[2]=1:1/2,[1]=1:inverse,[0]=1:gating; [7:4]op2 sram;
		MDrv_MFC_WriteByteMask(0x3200, _BIT1|_BIT0, _BIT1|_BIT0); // for od path, [1]=1:od, =0:mft
		MDrv_MFC_WriteBit(0x3201, 0, _BIT1); // [1]=1, dual/quad mode
		//MDrv_MFC_WriteByteMask(0x2A06, _BIT7|_BIT1|_BIT0, _BIT7|_BIT1|_BIT0); // [7]=1:LPLL double;
	}
	else
	{
	  	if ( (gmfcSysInfo.u8LVDSChannel==_SINGLE && gmfcSysInfo.u8PanelChannel==_SINGLE))
		{
		  	MDrv_MFC_WriteByte(0x1E0E, 0x04); // [3:0]od clk:[2]=1:1/2,[1]=1:inverse,[0]=1:gating; [7:4]op2 sram;
			MDrv_MFC_WriteByteMask(0x3200, _BIT0, _BIT1|_BIT0); // for od path, [1]=1:od, =0:mft
		}
		else
		{
		    if (gmfcSysInfo.u8PanelType == _MINI_LVDS || gmfcSysInfo.u8PanelType ==_MINI_LVDS_GIP || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
		    {
    			MDrv_MFC_WriteByte(0x1E0E, 0x02); // [3:0]od clk:[2]=1:1/2,[1]=1:inverse,[0]=1:gating; [7:4]op2 sram;

    			MDrv_MFC_WriteBit(0x1E22, 0, _BIT1);  // DPM gpio1 enable.
    			MDrv_MFC_WriteBit(0x1E1A, 0, _BIT1);  // DPM enable.
    			//MDrv_MFC_WriteBit(0x1E22, 0, _BIT3);  // gpio3 enable.
    			//MDrv_MFC_WriteBit(0x1E1A, 0, _BIT3);  // gpio3 set to Low.
            }
			else
			{
				MDrv_MFC_WriteByte(0x1E0E, 0x00); // [3:0]od clk:[2]=1:1/2,[1]=1:inverse,[0]=1:gating; [7:4]op2 sram;
			}
			MDrv_MFC_WriteByteMask(0x3200, _BIT0, _BIT1|_BIT0); // for od path, [1]=1:od, =0:mft

            #if(PANEL_TYPE_SEL == PNL_INN26_WXGA_120HZ)
				MDrv_MFC_WriteByte(0x1E0E, 0x00);
            #endif
		}

        if (//(gmfcSysInfo.u8LVDSChannel==_SINGLE && gmfcSysInfo.u8PanelChannel==_DUAL) ||
          (gmfcSysInfo.u8LVDSChannel==_DUAL && (gmfcSysInfo.u8PanelChannel==_QUAD || gmfcSysInfo.u8PanelChannel==_QUAD_LR)))
	  	{
			MDrv_MFC_WriteBit(0x3201, 1, _BIT1); // [1]=1, dual/quad mode
			//	MDrv_MFC_WriteByteMask(0x2A06, _BIT7, _BIT7|_BIT6); // [7]=1:LPLL double; [6]=0:LPLL type for Lvds
            #if ( (PANEL_TYPE_SEL == PNL_LC320WXD_WXGA_120HZ) || (PANEL_TYPE_SEL == PNL_LC150OLED_WXGA_120HZ) || (PANEL_TYPE_SEL == PNL_LC150OLED_WXGA_60HZ)  )
				MDrv_MFC_WriteBit(0x3201, 0, _BIT1);
				//   MDrv_MFC_WriteByteMask(0x2A06, 0, _BIT7|_BIT6);
            #endif
            #ifdef Mst_func_3DDemo
				//  msWriteBit(0x2A06, 0, _BIT7);
            #endif
        }
	 	else
	  	{
			MDrv_MFC_WriteBit(0x3201, 0, _BIT1);
			//MDrv_MFC_WriteByteMask(0x2A06, 0, _BIT7|_BIT6); // [7]=0:LPLL normal; [6]=0:LPLL type for Lvds
	  	}

		if ((gmfcSysInfo.u8PanelType == _MINI_LVDS) || (gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP) ||(gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5))
		{
			MDrv_MFC_WriteByte(0x2A05, 0x28);
			//MDrv_MFC_WriteByte(0x2A04, 0x01);
            MDrv_MFC_WriteByteMask(0x3275, _BIT7 , _BIT7);  //j080912 for power consumption
		}
        //else if ( (gmfcSysInfo.u8PanelType == _LVDS) || (gmfcSysInfo.u8PanelType == _TTL) )
        	//MDrv_MFC_WriteByteMask(0x2A04, 0 , _BIT0);  //j080912 for power consumption
    }

	if(gmfcMiuBaseAddr.u8GammaMode == GAMMA_OFF)
		MDrv_MFC_WriteByteMask(0x1E0E, _BIT6, _BIT6); //disable op2_sramclk

    //printk("MDrv_MFC_InitializeDispTgen()\n");
}


/*
LVDS output selection:
0x320B[6]TI mode, [5]=0:pair shift, [3]Polarity swap, [2]pair swap
0x320A=0x08:8bits TI mode, 0x0C:6bits TI mode, 0x00:others
0x321F:LVDS channel swap,
[1:0]Aout, [3:2]Bout, [5:4]Cout, [7:6]Dout,
'00b'Ain,  '01b'Bin,  '10b'Cin,  '11b'Din
*/
//_RSDS
code MST_MFC_RegUnitType_t tInitializeScTop1[]=
{
	{0x3205, 0x08}, // Rsds mode
	{0x320B, 0x20}, // lvds THine mode(h5[14]=0)

	{0x3220, 0x55}, // set output configure to rsds data output = "01" for 26 channel
	{0x3221, 0x55}, // rsds clk output = "11"
	{0x3222, 0x57}, //
	{0x3223, 0x55}, //
	{0x3224, 0x55}, //
	{0x3225, 0x5d}, //
	{0x3226, 0x05}, //
	{0x3228, 0xff}, // set ttl_oe = "11" when rsds output for 26 channel
	{0x3229, 0xff}, //
	{0x322A, 0xff}, //
	{0x322B, 0xff}, //
	{0x322C, 0xff}, //
	{0x322D, 0xff}, //
	{0x322E, 0x0f}, //
	{0x3230, 0x00}, // reg_ext_en = 26'h0
	{0x3231, 0x00}, //
	{0x3232, 0x00}, //
	{0x3233, 0x00}, //
	{0x324E, 0x0f}, // enable mod_atop IB,CLK
	{0x324F, 0x10},
	{0x3250, 0x00}, // 0x4c,
	{0x3253, 0x80}, // enable da bias
{_END_OF_TBL_, _END_OF_TBL_},
};

//Pre-Emphasis setting (for Gip)
code MST_MFC_RegUnitType_t tIniTconCommPreEmphasis[]=
{
	{0x3242, 0xff},
	{0x3243, 0xff},
	{0x3244, 0xff},
	{0x3245, 0x03},

        {0x324E, 0x0f}, //enable IB, CLK
        {0x324F, 0x75}, //enable IB, CLK //George recommand 090722
	{0x3268, 0xff},
	{0x3269, 0x03},
    {_END_OF_TBL_, _END_OF_TBL_},
};

// _MINI_LVDS
code MST_MFC_RegUnitType_t tInitializeScTop2_Comm[]=
{
	{0x3200, 0x01}, //[9]quad mode
	{0x3201, 0x02},
	{0x3204, 0x00}, //en CRC
	{0x3205, 0x80},
	{0x320A, 0x00}, //[14]LVDS TI mode, [13]pdp10 bit
	{0x320B, 0x20},
	{0x3220, 0x55},
	{0x3221, 0x57},
	//{0x3222, 0x5D},//0x59,//[3:2]=10, use mini-lvds clock configuration for right---I-Chang 09092008 //j090210
	{0x3223, 0x54},
	{0x3224, 0x5D},
	//{0x3225, 0x75},//0x65,//[13:12]=10, use mini-lvds clock configuration for left---I-Chang 09092008//j090210
	{0x3226, 0x00},
	{0x3227, 0x00},
	{0x3228, 0xff},
	{0x3229, 0xff},
	{0x322A, 0xff},
	{0x322B, 0xff},
	{0x322C, 0xff},
	{0x322D, 0xff},
	{0x322E, 0x0f},
	{0x322F, 0x00},
	{0x3230, 0xff},
	{0x3231, 0xff},
	{0x3232, 0xff},
	{0x3233, 0x03},

    {0x3242, 0xff},
    {0x3243, 0xff},
    {0x3244, 0xff},
    {0x3245, 0x03},
	{0x3246, 0x00},
	{0x3247, 0x00},
	{0x3248, 0x00},
	{0x3249, 0x00},
	{0x324E, 0x0f}, //enable IB, CLK
	{0x324F, 0x7D}, //enable IB, CLK
	{0x3250, (0x5C|MOD_POWER_ON_AFTER_INIT)}, //swing
	{0x3251, 0x00},
	{0x3252, 0x40}, //enable da bias
	{0x3253, 0xC1},
	{0x3254, 0x04}, //ch26~35 type
	{0x3255, 0x00},
	{0x3256, 0x00}, //ch26~35 type
	{0x3257, 0x00},
	{0x3260, 0xff}, //reg_gpo_oez_ch26_35---------------------------0826_I-Chang
	{0x3261, 0xff},
	{0x3262, 0x0f}, //reg_gpo_oez_ch26_35
	{0x3263, 0x00},
	{0x3264, 0x00}, //ch26~35 extern enable
	{0x3265, 0x00},
	{0x3270, 0x80}, //[7]reg_tcon_en [4]reg_tcon_swap---------------------------0826_I-Chang
	{0x3271, 0x00},
	{0x3272, 0x00},
	{0x3273, 0xc6}, //[15:8]reg_mini_ch_swap
	{0x3276, 0xFF},
	{_END_OF_TBL_, _END_OF_TBL_},
};

//  _MINI_LVDS_GIP
code MST_MFC_RegUnitType_t tInitializeScTop2_GIP[]=
{
	{0x3200, 0x01}, //[9]quad mode
	{0x3201, 0x02},
	{0x3204, 0x00}, //en CRC
	{0x3205, 0x80},
	{0x320A, 0x00}, //[14]LVDS TI mode, [13]pdp10 bit
	{0x320B, 0x20},

	{0x3220, 0x55},
	{0x3221, 0x57},
	{0x3222, 0x04},
	{0x3223, 0x54},
	{0x3224, 0x5d},
	{0x3225, 0x11},
	{0x3226, 0x00},
	{0x3227, 0x00},
	{0x3228, 0xff},
	{0x3229, 0xff},
	{0x322A, 0xff},
	{0x322B, 0xff},
	{0x322C, 0xff},
	{0x322D, 0xff},
	{0x322E, 0x0f},
	{0x322F, 0x00},

    {0x3240, 0x90}, //[15]reg_mini [7]msb_p [6]msb_s [4]data_swap
	{0x3241, 0x80},
	{0x3246, 0x00},
	{0x3247, 0x00},
	{0x3248, 0x00},
	{0x3249, 0x00},
    {0x324F, 0x75}, //enable IB, CLK //George recommand 090722
	{0x3250, (0x5C|MOD_POWER_ON_AFTER_INIT)},//0x52, //swing
	{0x3251, 0x00},
    {0x3252, 0x40}, //enable da bias //George recommand 090722
    {0x3253, 0xC1},//0x81,
	{0x3254, 0x04}, //ch26~35 type
	{0x3255, 0x00},
	{0x3256, 0x00}, //ch26~35 type
	{0x3257, 0x00},
	{0x3260, 0xff}, //reg_gpo_oez_ch26_35---------------------------0826_I-Chang
	{0x3261, 0xff},
	{0x3262, 0x0f}, //reg_gpo_oez_ch26_35
	{0x3263, 0x00},
	{0x3264, 0x00}, //ch26~35 extern enable
	{0x3265, 0x00},
	{0x3270, 0x90}, //[7]reg_tcon_en [4]reg_tcon_swap-----0826_I-Chang //j081031
	{0x3271, 0x00},
	{0x3272, 0x00},
	{0x3273, 0xc6}, //[15:8]reg_mini_ch_swap
	{0x3276, 0xFF},

{_END_OF_TBL_, _END_OF_TBL_},
};

//TTL
code MST_MFC_RegUnitType_t tInitializeScTop3[]=
{
	{0x3220, 0x00}, // set output configure to TTL output
	{0x3221, 0x00}, //
	{0x3222, 0x00}, //
	{0x3223, 0x00}, //
	{0x3224, 0x00}, //
	{0x3225, 0x00}, //
	{0x3226, 0x00}, //

	{0x3228, 0xff}, // set ttl_oe = "11" when lvds output for 26 channel
	{0x3229, 0xff}, //
	{0x322A, 0xff}, //
	{0x322B, 0xff}, //
	{0x322C, 0xff}, //
	{0x322D, 0xff}, //
	{0x322E, 0x0f}, //

	{0x3230, 0x00}, // reg_ext_en = 26'h0
	{0x3231, 0x00}, //
	{0x3232, 0x00}, //
	{0x3233, 0x00}, //
	{0x3234, 0xff}, // reg_ext_en = 26'h0
	{0x3235, 0xff}, //
	{0x3236, 0xff}, //
	{0x3237, 0x03}, //

	{0x324E, 0x0f}, // enable mod_atop IB,CLK
	{0x3250, 0x4c}, // Bruce for Ursa 2 new setting; Ursa 1 is null register.
	{0x3253, 0x80}, // enable da bias

	{0x3254, 0x00},
	{0x3255, 0x00},
	{0x3256, 0x00},
    {0x3257, 0x00},
	{0x3264, 0x00},
	{0x3265, 0x00},
	{0x3266, 0xFF},
    {0x3267, 0x03},
    {_END_OF_TBL_, _END_OF_TBL_},
};

//LVDS
code MST_MFC_RegUnitType_t tInitializeScTop4[]=
{
#if (CODEBASE_SEL == CODEBASE_51 || CODEBASE_SEL==CODEBASE_UTOPIA)
    {0x3220, 0x55}, // set output configure to lvds output = "01" for 26 channel
	{0x3221, 0x55}, //
	{0x3222, 0x55}, //
	{0x3223, 0x55}, //
	{0x3224, 0x55}, //
	{0x3225, 0x55}, //
	{0x3226, 0x05}, //
#endif
	{0x3228, 0xff}, // set ttl_oe = "11" when lvds output for 26 channel
	{0x3229, 0xff}, //
	{0x322A, 0xff}, //
	{0x322B, 0xff}, //
	{0x322C, 0xff}, //
	{0x322D, 0xff}, //
	{0x322E, 0x0f}, //

	{0x3230, 0x00}, // reg_ext_en = 26'h0
	{0x3231, 0x00}, //
	{0x3232, 0x00}, //
	{0x3233, 0x00}, //
	{0x324E, 0x0f}, // enable mod_atop IB,CLK
    {0x3250, (0x50|MOD_POWER_ON_AFTER_INIT)}, //swing // 46 -> 78 -> 50 andy.jeong for temporary(AUO dot noise) 20100202)
	{0x3253, 0x80}, // enable da bias

	{0x3254, 0x55}, // Bruce for Ursa 2
	{0x3255, 0x55}, // Bruce for Ursa 2
	{0x3256, 0x05}, // Bruce for Ursa 2
    {_END_OF_TBL_, _END_OF_TBL_},
};

void MDrv_MFC_InitializeScTop(void)
{
	MDrv_MFC_WriteBit(0x30BF, 1, _BIT6);
        	MDrv_MFC_Write2Bytes(0x30C4, 0x0400);
        	MDrv_MFC_Write2Bytes(0x30C6, 0x0400);
        	MDrv_MFC_Write2Bytes(0x30C8, 0x0400);

	if (gmfcSysInfo.u8PanelType == _MINI_LVDS
  	 || gmfcSysInfo.u8PanelType == _LVDS
  	 || gmfcSysInfo.u8PanelType == _TTL
  	 || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP
  	 || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
	{
		//Auto no signal blue screen
		MDrv_MFC_WriteBit(0x3017, 1, _BIT3);
		MDrv_MFC_WriteBit(0x3017, 1, _BIT5);
		MDrv_MFC_WriteBit(0x3017, 1, _BIT6);
		if (gmfcSysInfo.u8PanelChannel == _QUAD_LR)
		{
			MDrv_MFC_Write2Bytes(0x3100, (gmfcSysInfo.u16Width+2)/4-1);
		    MDrv_MFC_WriteBit(0x3101, 1, _BIT7);
			MDrv_MFC_WriteByte(0x3103, 0xa0);
		}

        if( ( gmfcSysInfo.u8PanelChannel==_SINGLE ) || (gmfcSysInfo.u8PanelType==_TTL) )
		{
              MDrv_MFC_WriteBit(0x3102, 1, _BIT4);  //Tx single channel mux
	  		MDrv_MFC_WriteByte(0x3105, 0x00); // [7]OD odd/even swap
		}
	  	else  if(gmfcSysInfo.u8LVDSChannel!=gmfcSysInfo.u8PanelChannel)
	  		MDrv_MFC_WriteByte(0x3105, 0x00); // [7]OD odd/even swap
		else
			MDrv_MFC_WriteByte(0x3105, 0x80); // [7]OD odd/even swap

	  	if ((gmfcSysInfo.u8PanelType == _MINI_LVDS) || (gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP) ||(gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5))
	  	{
            if(gmfcSysInfo.u8PanelType == _MINI_LVDS)
            {
                MDrv_MFC_Write2Bytes(0x3120, 0x8080); //[15]oeswap [14]odd mode
            }
            else
            {
                // set by gip timing table.
            }
            #if (PANEL_TYPE_SEL == PNL_INN26_WXGA_120HZ)
			     MDrv_MFC_WriteBit(0x3102, 0, _BIT5);//j090210
			#else
			     MDrv_MFC_WriteBit(0x3102, 1, _BIT5); //LTD on
            #endif
	  	}

		MDrv_MFC_WriteByte(0x320A, 0x00);
		if (gmfcSysInfo.u8PanelLVDSTiMode)
		{
			if (gmfcSysInfo.u8PanelBitNum==_8BITS)
				MDrv_MFC_WriteByte(0x320A, 0x08); // lvds TI 8bit(h5[3:2]=10)
			else if (gmfcSysInfo.u8PanelBitNum==_6BITS)
				MDrv_MFC_WriteByte(0x320A, 0x0C); // lvds TI 6bit(h5[3:2]=11)
		}
		if (gmfcSysInfo.u8PanelType == _TTL)
		MDrv_MFC_WriteByteMask(0x320A, _BIT1| _BIT0, _BIT1| _BIT0); //[0]:TTL 10 bit
              //MDrv_MFC_WriteBit(0x320A, 1, _BIT1);
              

	#if (CODEBASE_SEL == CODEBASE_51)
		#if (REG_DIRECT_ACCESS_BY_I2C)
			if (gmfcSysInfo.u8Preset == 0x01)
		#endif
	#endif
		{
			// [6]TI mode, [5]=0:pair shift, [3]Polarity swap, [2]pair swap
			MDrv_MFC_WriteByte(0x320B, (gmfcSysInfo.u8PanelLVDSTiMode?_BIT6:0)
								| (gmfcSysInfo.u8PanelLVDSShiftPair?0:_BIT5)
								| (gmfcSysInfo.u8PanelLVDSSwapPol?_BIT3:0)
								| (gmfcSysInfo.u8PanelLVDSSwapPair?_BIT2:0));
                #ifdef PNL_LCEAll_En //for local dimming auto detection
                    MDrv_MFC_WriteBit(0x320B, ((MDrv_MFC_ReadByte(0x1E18)&0x08)?0:1), _BIT3);
                #endif
			MDrv_MFC_WriteByte(0x321F, gmfcSysInfo.u8PanelLVDSSwapCH); // channel swap
		}
	}

    if(gmfcSysInfo.u8PanelType == _RSDS )
    {
		MDrv_MFC_WriteRegsTbl(0x3200, tInitializeScTop1); //j081014
    }
    else if(gmfcSysInfo.u8PanelType == _MINI_LVDS )
    {
		MDrv_MFC_WriteRegsTbl(0x3200, tInitializeScTop2_Comm);
          MDrv_MFC_InitializeScTop2_Bypanel();
    }
    else if(gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5 )
    {
		MDrv_MFC_WriteRegsTbl(0x3200, tInitializeScTop2_GIP);
    }
    else if(gmfcSysInfo.u8PanelType == _TTL )
    {
		MDrv_MFC_WriteRegsTbl(0x3200, tInitializeScTop3);
    }
    else  //LVDS
    {
		MDrv_MFC_WriteRegsTbl(0x3200, tInitializeScTop4);
    }

    //Pre-Emphasis setting
	if(gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5 )
    {
       MDrv_MFC_WriteRegsTbl(0x3200, tIniTconCommPreEmphasis);

	   if( IS_CHIP_3xxx_U01(gmfcSysInfo.u8ChipRevision) )
	   {
	       MDrv_MFC_Write2Bytes(0x324E, 0x4c0f);
	       MDrv_MFC_Write2Bytes(0x3252, 0xc1c0);
	   }
    }
    //printk("MDrv_MFC_InitializeScTop()\n");
}

code MST_MFC_RegUnitType_t tInitializeFRC[]=
{
	{0x307E, 0x16},//0x17
	{0x307F, 0x00},
	{0x3080, 0x23},
	{0x3081, 0x20},
	{0x3082, 0xf3},
	{0x3083, 0x3c},
	{0x3084, 0xc9},
	{0x3085, 0x9c},
	{0x3086, 0xc9},
	{0x3087, 0x9c},
	{0x3088, 0xaa},
	{0x3089, 0xaa},
	{0x308A, 0x50},
	{0x308B, 0x22},
	{0x308C, 0xd8},
	{0x308D, 0xd8},
	{0x308E, 0x72},
	{0x308F, 0x72},
	{0x3090, 0x8d},
	{0x3091, 0x8d},
	{0x3092, 0x27},
	{0x3093, 0xd8},
	{0x3094, 0x72},
	{0x3095, 0x8d},
{_END_OF_TBL_, _END_OF_TBL_},
};

void MDrv_MFC_InitializeOPMFC(void)
{
    	MDrv_MFC_WriteRegsTbl(0x3000, tInitializeFRC); // initialize all of bank
     #if 1
        if(( gmfcSysInfo.u8PanelType == _MINI_LVDS
            ||gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP
            || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5 )
            && gmfcSysInfo.u8PanelBitNum == _8BITS
            && IS_CHIP_3xxx_U03_AND_AFTER(gmfcSysInfo.u8ChipRevision)
            )
            MDrv_MFC_WriteBit(0x307E, _ENABLE, _BIT0);
        else
            MDrv_MFC_WriteBit(0x307E, _DISABLE, _BIT0);
    #else
    MDrv_MFC_WriteBit(0x307E, gmfcSysInfo.u8PanelDither, _BIT0);
    #endif
}

void MDrv_MFC_InitializeScalerOP(void)
{
	MDrv_MFC_InitializeDispTgen();
	MDrv_MFC_InitializeScTop();
    MDrv_MFC_WriteBit(0x2C42, MFC_ENABLE_LVDS_SSC, _BIT5); // enable ssc
    MDrv_MFC_SetLvdsSSC(MFC_LVDS_SSC_SPAN_DEFAULT, MFC_LVDS_SSC_STEP_DEFAULT, MDrv_MFC_ReadByte(0x2C42)&0x20);
	MDrv_MFC_InitializeOPMFC();
}

#endif
