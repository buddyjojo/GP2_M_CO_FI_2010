#define _MSINIT_C_

#include "mdrv_mfc_platform.h"
#include "mdrv_mfc_fb.h"

#if(CODESIZE_SEL == CODESIZE_ALL)

//chip top
code MST_MFC_RegUnitType_t tInitializeChip[]=
{
// chip top
	{0x1E03, 0x07},
		{0x1E04, 0x0C},
	//{0x1E05, 0x00}, //_BIT4 | _BIT0
	//{0x1E0B, 0x10},  //ckg_fclk
    //{0x1E0C, 0x00}, //_BIT4 | _BIT0
    //{0x1E0D, 0x00}, // _BIT0
	{0x1E0E, 0x00}, // _BIT0
    //{0x1E0F, 0x00}, // _BIT5
    //{0x1E11, 0x46},//0x00, // _BIT0  //_BIT5 |_BIT4

	{0x1E0A, 0x00},
	{0x1E1E, 0x00},
	{0x1E36, 0x40}, // set reg_enable_pad 1
	{0x1E37, 0x00}, // set reg_enable_pad 1
	{0x1E3E, 0x00},
	{0x1E3F, 0x00},
	{0x1E42, 0x95},
	{0x1E43, 0xf0},
	{0x1E46, 0x70}, // [3:2] reg_gpio_i2cm_out
					// [5;4] reg_gpio_i2cm_oen
					// [6] reg_mod_gate, od clock gating
	{0x1E47, 0x00},
	{_END_OF_TBL_, _END_OF_TBL_},
};

code U8 tClockSel[][5] =
{
    //1E0F  1E10  1E11 1E05 1E0B
   {0x40, 0x00, 0x04, 0x00, 0x18}, //_RSDS          //0x10 to 18
   {0x32, 0x00, 0x46, 0x60, 0x18}, //_MIN_LVDS      //0x12 to 0x18
   {0x10, 0x00, 0x00, 0x00, 0x18}, //_LVDS or _TTL  //0x10 to 0x18 //F_Clk to AuPLL_Clk //Cloud090616
};

void MDrv_MFC_InitializeChip(void)
{
	U8 ucIdx;

	if (gmfcSysInfo.u8PanelType == _RSDS)
		ucIdx = 0;
	else if ((gmfcSysInfo.u8PanelType == _MINI_LVDS)||(gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP)||( gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5))
		ucIdx = 1;
	else
		ucIdx = 2;	// _LVDS or  _TTL

	MDrv_MFC_WriteByte(0x1E0F, tClockSel[ucIdx][0]); // [7:4]rsds clk:[6]=1:1/2,[5]=1:inverse,[4]=1:gating; [3:0]misc clk
	MDrv_MFC_WriteByte(0x1E10, tClockSel[ucIdx][1]);
	MDrv_MFC_WriteByte(0x1E11, tClockSel[ucIdx][2]);
	MDrv_MFC_WriteByte(0x1E05, tClockSel[ucIdx][3]);
	MDrv_MFC_WriteByte(0x1E0B, tClockSel[ucIdx][4]);
    MDrv_MFC_WriteRegsTbl(0x1E00, tInitializeChip); // initialize all of bank
    MDrv_MFC_WriteByte(0x1E23, 0x5F); //Control GPIO-12,GPIO-14; 0:out , 1:in

    //putstr("\r\n MDrv_MFC_InitializeChip()");
}


code MST_MFC_RegUnitType_t tPowerDownChip[]=
{
	{0x3220, 0x00},
	{0x3221, 0x00},
	{0x3222, 0x00},
	{0x3223, 0x00},
	{0x3224, 0x00},
	{0x3225, 0x00},
	{0x3226, 0x00},
	{0x3227, 0x00},
	{0x3228, 0xFF},
	{0x3229, 0xFF},
	{0x322A, 0xFF},
	{0x322B, 0xFF},
	{0x322C, 0xFF},
	{0x322D, 0xFF},
	{0x322E, 0xFF},
	{0x322F, 0xFF},
{_END_OF_TBL_, _END_OF_TBL_},
};

#if 0
void MDrv_MFC_PowerDownChip(void)
{
	MDrv_MFC_WriteByte(0x2a06, 0x20); //lpll_pd
	MDrv_MFC_WriteByte(0x324e, 0x00); //diable ck ib
	MDrv_MFC_WriteByte(0x3250, 0x80); //mod regu pd
	MDrv_MFC_WriteByte(0x1e80, 0x80); //mpll pd
	MDrv_MFC_WriteByte(0x1e02, 0x02); //[1]pd_all,[0]reg_ckf_all_dft=1
	MDrv_MFC_WriteByte(0x1225, 0x02); //ddr_pd
	MDrv_MFC_WriteByte(0x2216, 0x18); //[4]bandgap_pd,[3]lvds regu pd
	MDrv_MFC_WriteByte(0x221e, 0x18); //[4]bandgap_pd,[3]lvds regu pd
	MDrv_MFC_WriteByte(0x2210, 0x28); //[5]lvds limit amp pd,[3]PHDAC pd
	MDrv_MFC_WriteByte(0x2214, 0x28); //[5]lvds limit amp pd,[3]PHDAC pd
	MDrv_MFC_WriteByte(0x224e, 0x07); //[2]bandgap pd,[1]first ch regu pd,[0]second ch regu pd
	MDrv_MFC_WriteByte(0x2228, 0x08); //[3] osc 400 pd
	MDrv_MFC_WriteByte(0x2278, 0x00); //disable all clock
	MDrv_MFC_WriteByte(0x220c, 0x40); //disable lvds clock gen
	MDrv_MFC_WriteByte(0x1e06, 0x00); //mcu clock = dft live
	MDrv_MFC_WriteByte(0x1e36, 0x00); //all pad output disable
	MDrv_MFC_WriteByte(0x1203, 0xf0); //ddr output disable
	MDrv_MFC_WriteByte(0x1e0e, 0x41); //[6]disable op2_sramclk,[0]disable odclk
	MDrv_MFC_WriteByte(0x1e0f, 0x11); //[4]disable rsds_clk,[0]disable misc_clk
	MDrv_MFC_WriteByte(0x1e11, 0x01); //[0]disable mod_clk
	MDrv_MFC_WriteByte(0x1e04, 0x01); //[0]disable miu clk
	MDrv_MFC_WriteByte(0x1e3e, 0x01); //[0]disable pafrc clk
    MDrv_MFC_WriteRegsTbl(0x3200, tPowerDownChip); // initialize all of bank
}
#endif

void MDrv_MFC_PowerDownChipU3(void)
{
    if( IS_CHIP_3xxx_U02_AND_AFTER(gmfcSysInfo.u8ChipRevision) )
    {
    	MDrv_MFC_WriteBit(0x3000, 1, _BIT1); //op2 power gatting

       //MDrv_MFC_WriteByteMask(0x2987, 0  , _BIT0); //disable mfc power gatting
       //MDrv_MFC_WriteByteMask(0x266F, 0  , _BIT1); //disable mfc power gatting
    }
	MDrv_MFC_Write2Bytes(0x28B6,  0x0000);   //OD
    MDrv_MFC_WriteBit(0x3104, 1, _BIT0); //MFT
    MDrv_MFC_WriteByteMask(0x2161, _BIT5 , _BIT5|_BIT4); //opm
    MDrv_MFC_WriteByteMask(0x266E, _BIT7|_BIT6 |_BIT3|_BIT2|_BIT1|_BIT0,  _BIT7|_BIT6|  _BIT3|_BIT2|_BIT1|_BIT0);
}

#endif
