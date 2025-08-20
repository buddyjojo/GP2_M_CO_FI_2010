
#define _MHAL_DEMOD_C_

#include <linux/kernel.h>
#include <linux/delay.h>

#include "mdrv_types.h"

#include "mhal_demod.h"
#include "mhal_demod_reg.h"

#include "mdrv_system.h"//mail box crash protection 2009-11-06


//----------------------------------------------------------------------
#define OPT_DEMOD_HAL_DEBUG
#undef DEMOD_HAL_DBG
#ifdef OPT_DEMOD_HAL_DEBUG
    #define DEMOD_HAL_DBG(fmt, args...)      printk(KERN_WARNING "[DEMOD_HAL][%05d]" fmt, __LINE__, ## args)
#else
    #define DEMOD_HAL_DBG(fmt, args...)
#endif

#undef DEMOD_HAL_DBGX
#define DEMOD_HAL_DBGX(fmt, args...)
//----------------------------------------------------------------------

#define DEMOD_VERIFY_DSP 1

static U8 _dsp_dvbt[] = {
    #include "msb1228_demod_dvbt.dat"
};

static U8 _dsp_dvbc[] = {
    #include "msb1228_demod_dvbc.dat"
};

static U8 _dsp_atsc[] = {
    #include "msb1501_demod_atsc.dat"
};
static U8 _dsp_atsc_sanyo[] = {
    #include "msb1501_demod_atsc_sanyo.dat"
};


void MHal_Demod_WriteReg(U32 u32Addr, U8 u8Value)
{
    _MHal_W1B(BYTE2REAL(u32Addr & 0x000FFFFF), u8Value);
    //DEMOD_HAL_DBG("set reg(0x%06x) to 0x%02x\n", u32Addr, u8Value);
}

void MHal_Demod_ReadReg(U32 u32Addr, U8 *pu8Value)
{
    *pu8Value = _MHal_R1B(BYTE2REAL(u32Addr & 0x000FFFFF));
    //DEMOD_HAL_DBG("reg(0x%06x) = 0x%02x\n", u32Addr, *pu8Value);
}

B16 MHal_Demod_MB_WriteReg(U32 u32Addr, U8 u8Value)
{
	U8	    RegVal = 0;
	U16	    u8WaitCnt = 0;

	//mail box crash protection 2009-11-06
    if ((VDMCU_DSP_DVBT != MDrv_SYS_VDmcuGetType()) && (VDMCU_DSP_DVBC != MDrv_SYS_VDmcuGetType()))
    {
        printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType());
        return FALSE;
    }

	MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);

	if (RegVal)
    {
        //Driver update 2009/11/20
        U8  tmp_h, tmp_l;
        U16 tmp_addr;
        MHal_Demod_ReadReg(MBRegBase + 0x02, &tmp_h);
        MHal_Demod_ReadReg(MBRegBase + 0x01, &tmp_l);
        tmp_addr = ( (tmp_h << 8) | tmp_l );

        DEMOD_HAL_DBG("Mailbox crash (write reg 0x%04x) (last reg 0x%04x)\n", u32Addr, tmp_addr);
        return FALSE;
    }

	MHal_Demod_WriteReg(MBRegBase + 0x02, (U8)(u32Addr >> 8));		// ADDR_H
	MHal_Demod_WriteReg(MBRegBase + 0x01, (U8)u32Addr);				// ADDR_L
	MHal_Demod_WriteReg(MBRegBase + 0x03, u8Value);					// REG_DATA
	MHal_Demod_WriteReg(MBRegBase + 0x00, 0x02);					// MB_CNTL set write mode
//andy 20090720 update start
	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x02);		// assert interrupt to VD MCU51
	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x00);		// de-assert interrupt to VD MCU51
//andy 20090720 update end
	MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
	while(RegVal != 0xFF)			// wait done flag
	{
		MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);

		if (u8WaitCnt++ > 0x7FFF)
		{
			DEMOD_HAL_DBG("Demod WriteReg Fail!\n");
			return FALSE;
		}
	}

	MHal_Demod_WriteReg(MBRegBase + 0x00, 0x00);					// MB_CNTL clear

    return TRUE;
}

B16 MHal_Demod_MB_ReadReg(U32 u32Addr, U8 *pu8Value)
{
	U16		u8WaitCnt = 0;
	U8	    RegVal;

	//mail box crash protection 2009-11-06
    if ((VDMCU_DSP_DVBT != MDrv_SYS_VDmcuGetType()) && (VDMCU_DSP_DVBC != MDrv_SYS_VDmcuGetType()))
    {
        printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType());
        return FALSE;
    }

	MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);

	if (RegVal)
    {
        //Driver update 2009/11/20
        U8  tmp_h, tmp_l;
        U16 tmp_addr;
        MHal_Demod_ReadReg(MBRegBase + 0x02, &tmp_h);
        MHal_Demod_ReadReg(MBRegBase + 0x01, &tmp_l);
        tmp_addr = ( (tmp_h << 8) | tmp_l );

        DEMOD_HAL_DBG("Mailbox crash (read reg 0x%04x) (last reg 0x%04x)\n", u32Addr, tmp_addr);
        return FALSE;
    }

	MHal_Demod_WriteReg(MBRegBase + 0x02, (U8)(u32Addr >> 8));		// ADDR_H
	MHal_Demod_WriteReg(MBRegBase + 0x01, (U8)u32Addr);				// ADDR_L
	MHal_Demod_WriteReg(MBRegBase + 0x00, 0x01);					// MB_CNTL set read mode
//andy new driver update 090720 start
	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x02);		// assert interrupt to VD MCU51
	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x00);		// de-assert interrupt to VD MCU51
//andy new driver update 090720 end
	MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
	while(RegVal != 0xFF)			// wait MB_CNTL set done
	{
		MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);

		if (u8WaitCnt++ > 0x7FFF)
		{
			DEMOD_HAL_DBG("Demod ReadReg Fail!\n");
			return FALSE;
		}
	}

	MHal_Demod_ReadReg(MBRegBase + 0x03, pu8Value);				    // REG_DATA get
	MHal_Demod_WriteReg(MBRegBase + 0x00, 0x00);				    // MB_CNTL clear

    return TRUE;
}

B16 MHal_Demod_MB_WriteDspReg(U32 u32Addr, U8 u8Value)
{
	U8	    RegVal = 0;
	U16	    u8WaitCnt = 0;

    if ((VDMCU_DSP_DVBT != MDrv_SYS_VDmcuGetType()) && (VDMCU_DSP_DVBC != MDrv_SYS_VDmcuGetType()))
    {
        printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType());
        return FALSE;
    }

	MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);

	if (RegVal)
    {
        U8  tmp_h, tmp_l;
        U16 tmp_addr;
        MHal_Demod_ReadReg(MBRegBase + 0x02, &tmp_h);
        MHal_Demod_ReadReg(MBRegBase + 0x01, &tmp_l);
        tmp_addr = ( (tmp_h << 8) | tmp_l );

        DEMOD_HAL_DBG("Mailbox crash (write reg 0x%04x) (last reg 0x%04x)\n", u32Addr, tmp_addr);
        return FALSE;
    }

	MHal_Demod_WriteReg(MBRegBase + 0x02, (U8)(u32Addr >> 8));		// ADDR_H
	MHal_Demod_WriteReg(MBRegBase + 0x01, (U8)u32Addr);				// ADDR_L
	MHal_Demod_WriteReg(MBRegBase + 0x03, u8Value);					// REG_DATA
	MHal_Demod_WriteReg(MBRegBase + 0x00, 0x04);					// MB_CNTL set write dsp mode

	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x02);					// assert interrupt to VD MCU51
	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x00);					// de-assert interrupt to VD MCU51

	MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
	while(RegVal != 0xFF)											// wait done flag
	{
		MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);

		if (u8WaitCnt++ > 0x7FFF)
		{
			DEMOD_HAL_DBG(">> DVBT WriteDspReg Fail!");
			return FALSE;
		}
	}

	MHal_Demod_WriteReg(MBRegBase + 0x00, 0x00);					// MB_CNTL clear

	return TRUE;

}

B16 MHal_Demod_MB_ReadDspReg(U32 u32Addr, U8 *pu8Value)
{
	U16		u8WaitCnt = 0;
	U8	    RegVal;

    if ((VDMCU_DSP_DVBT != MDrv_SYS_VDmcuGetType()) && (VDMCU_DSP_DVBC != MDrv_SYS_VDmcuGetType()))
    {
        printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType());
        return FALSE;
    }

	MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);

	if (RegVal)
    {
        U8  tmp_h, tmp_l;
        U16 tmp_addr;
        MHal_Demod_ReadReg(MBRegBase + 0x02, &tmp_h);
        MHal_Demod_ReadReg(MBRegBase + 0x01, &tmp_l);
        tmp_addr = ( (tmp_h << 8) | tmp_l );

        DEMOD_HAL_DBG("Mailbox crash (read reg 0x%04x) (last reg 0x%04x)\n", u32Addr, tmp_addr);
        return FALSE;
    }

	MHal_Demod_WriteReg(MBRegBase + 0x02, (U8)(u32Addr >> 8));		// ADDR_H
	MHal_Demod_WriteReg(MBRegBase + 0x01, (U8)u32Addr);				// ADDR_L
	MHal_Demod_WriteReg(MBRegBase + 0x00, 0x03);					// MB_CNTL set read dsp mode

	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x02);					// assert interrupt to VD MCU51
	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x00);					// de-assert interrupt to VD MCU51

	MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
	while(RegVal != 0xFF)											// wait MB_CNTL set done
	{
		MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);

		if (u8WaitCnt++ > 0x7FFF)
		{
			DEMOD_HAL_DBG(">> DVBT ReadReg Fail!");
			return FALSE;
		}
	}

	MHal_Demod_ReadReg(MBRegBase + 0x03, pu8Value);					// REG_DATA get
	MHal_Demod_WriteReg(MBRegBase + 0x00, 0x00);					// MB_CNTL clear

    return TRUE;

}

B16 MHal_Demod_MB_WriteReg_ATSC(U32 u32Addr, U8 u8Value)
{
    U16 u16CheckCount;
    U8 u8CheckFlag;

	//mail box crash protection 2009-11-06
    if (VDMCU_DSP_ATSC != MDrv_SYS_VDmcuGetType())
    {
        printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType());
        return FALSE;
    }


    MHal_Demod_WriteReg(0x110500, (u32Addr&0xff));
    MHal_Demod_WriteReg(0x110501, (u32Addr>>8));
    MHal_Demod_WriteReg(0x110510, u8Value);
    MHal_Demod_WriteReg(0x11051E, 0x01);

	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x02);		// assert interrupt to VD MCU51
	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x00);		// de-assert interrupt to VD MCU51

    // andy 2009-7-27 ¢¯AAu 11:35:48 for ( u16CheckCount=0; u16CheckCount < 1000 ; u16CheckCount++ )//
    for ( u16CheckCount=0; u16CheckCount < 200 ; u16CheckCount++ )//total 200ms
    {
      MHal_Demod_ReadReg(0x11051E, &u8CheckFlag);
      if ((u8CheckFlag&0x01)==0)
        return TRUE;
		udelay(10);
    }
    return FALSE;
}

B16 MHal_Demod_MB_ReadReg_ATSC(U32 u32Addr, U8 *pu8Value)
{
    U16 u16CheckCount;
    U8 u8CheckFlag;

	//mail box crash protection 2009-11-06
    if (VDMCU_DSP_ATSC != MDrv_SYS_VDmcuGetType())
    {
        printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType());
        return FALSE;
    }

    MHal_Demod_WriteReg(0x110500, (u32Addr&0xff));
    MHal_Demod_WriteReg(0x110501, (u32Addr>>8));
    MHal_Demod_WriteReg(0x11051E, 0x02);

	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x02);		// assert interrupt to VD MCU51
	MHal_Demod_WriteReg(VDMcuBase + 0x63, 0x00);		// de-assert interrupt to VD MCU51

	for ( u16CheckCount=0; u16CheckCount < 200 ; u16CheckCount++ )//andy new driver update 090720
    {
      MHal_Demod_ReadReg(0x11051E, &u8CheckFlag);
      if ((u8CheckFlag&0x02)==0)
      {
        MHal_Demod_ReadReg(0x110510, pu8Value);
        return TRUE;
      }
		udelay(10);
    }
    return FALSE;
}

// 0402 change for tuner option
// update 0402 for ATSC
U8 Demod_Flow_register_Tuner_Innotek[17] =   {0x52, 0x72, 0x52, 0x72, 0x4C, 0x52, 0xA3, 0xEC, 0xEA, 0x05,
    0x74, 0x1E, 0x42, 0x42, 0x1C, 0x62, 0x00};//#03.0E

U8 Demod_Flow_register_Tuner_Sanyo[17] =   {0x52, 0x72, 0x52, 0x72, 0x60, 0x60, 0xA4, 0xEC, 0xEA, 0x05,
		0x74, 0x1E, 0x5C, 0x5C, 0x08, 0x64, 0x0C};//#03.10

// 0402 change for tuner option
B16 MHal_Demod_LoadDSPCode(DEMOD_DSP_PARAM_t stParam)
{
    DEMOD_STANDARD_TYPE_t eStandard;
    TUNER_TYPE_t eTunerType;

    U8* pu8Demod_DSP;
    U32 u32Demod_DSP_len;

    U32 i;
    B16 bRet = TRUE;
// 0402 change for tuner option
    eStandard = stParam.eDSP_Type;
    eTunerType = stParam.eTuner_Type;

    //DSP branch
    if(eStandard == DEMOD_STANDARD_DVBT)
    {
        pu8Demod_DSP = (U8*)_dsp_dvbt;
        u32Demod_DSP_len = sizeof(_dsp_dvbt);
    }
    else if (eStandard == DEMOD_STANDARD_DVBC)
    {
        pu8Demod_DSP = (U8*)_dsp_dvbc;
        u32Demod_DSP_len = sizeof(_dsp_dvbc);
    }
    else if (eStandard == DEMOD_STANDARD_ATSC)
    {
    	if(eTunerType == TUNER_TYPE_INNOTEK)
    	{
        	pu8Demod_DSP = (U8*)_dsp_atsc;
        	u32Demod_DSP_len = sizeof(_dsp_atsc);
    	}	
		else
		{
		    pu8Demod_DSP = (U8*)_dsp_atsc_sanyo;
        	u32Demod_DSP_len = sizeof(_dsp_atsc_sanyo);
		}
    }
    else
    {
        DEMOD_HAL_DBG("no such standard %d\n", eStandard);
		//mail box crash protection 2009-11-06
		MDrv_SYS_VDmcuSetType(VDMCU_DSP_UNKNOWN);
        return FALSE;
    }

    DEMOD_HAL_DBG("DSP type = %d, code size = %u\n", eStandard, u32Demod_DSP_len);

    // Prevent over the max size of VD DSP code.
    if(u32Demod_DSP_len > 0x5c00)
    {
        DEMOD_HAL_DBG("DSP code size (%u) is larger than 23k bytes\n", u32Demod_DSP_len);
		//mail box crash protection 2009-11-06
		MDrv_SYS_VDmcuSetType(VDMCU_DSP_UNKNOWN);
        return FALSE;
    }

#if 0
    if (1 <= Chip_Query_Rev())
    {
        *((volatile unsigned int*)0xBF201684) &= ~0x1F ;
        *((volatile unsigned int*)0xBF201684) |= 0x0D ;
    }
#endif

    /* reset VD MCU */
    _MHal_W1Bb(VD_MCU_RST, 0x01, _BIT0);

    /* disable sram */
    _MHal_W1Bb(VD_MCU_SRAM_EN, 0x00, _BIT0);

    /* enable down load code */
    _MHal_W1BM(VD_MCU_KEY, 0x50, 0xF0);

    /* enable address auto increment */
    _MHal_W1Bb(VD_MCU_ADDR_AUTO_INC, 0x01, _BIT0);

    /* reset sram address to 0 */
    _MHal_W1B(VD_MCU_ADDR_L, 0x00);
    _MHal_W1B(VD_MCU_ADDR_H, 0x00);

    /* download code */
    for (i=0; i<u32Demod_DSP_len; i++)
    {
        //access VD_MCU_WDATA and VD_MCU_WDATA_CTRL must use BYTE access
        _MHal_W1B(VD_MCU_SRAM_WD, pu8Demod_DSP[i]);
    }

	// update 0929 for ATSC
	if (eStandard == DEMOD_STANDARD_ATSC)
	{
		/* reset sram address to 0 */
		_MHal_W1B(VD_MCU_ADDR_L, 0x80);
		_MHal_W1B(VD_MCU_ADDR_H, 0x5B);

		// 0402 change for tuner option
		/* download code */
        switch (eTunerType)  //PLEASE FIX IT
        {
            case TUNER_TYPE_INNOTEK:
				for (i=0; i<sizeof(Demod_Flow_register_Tuner_Innotek); i++)
				{
					//access VD_MCU_WDATA and VD_MCU_WDATA_CTRL must use BYTE access
					_MHal_W1B(VD_MCU_SRAM_WD, Demod_Flow_register_Tuner_Innotek[i]);
				}
                break;

            case TUNER_TYPE_SANYO:
				for (i=0; i<sizeof(Demod_Flow_register_Tuner_Sanyo); i++)
				{
					//access VD_MCU_WDATA and VD_MCU_WDATA_CTRL must use BYTE access
					_MHal_W1B(VD_MCU_SRAM_WD, Demod_Flow_register_Tuner_Sanyo[i]);
				}

                break;

            default:
                MDrv_SYS_VDmcuSetType(VDMCU_DSP_UNKNOWN);
				printk("ATSC DSP code loaded fail !!! not defined tuner type \n");// update 0402 for ATSC
                return FALSE;
                break;
        }
	}

#if DEMOD_VERIFY_DSP
    /* reset sram address to 0 */
    _MHal_W1B(VD_MCU_ADDR_L, 0x00);
    _MHal_W1B(VD_MCU_ADDR_H, 0x00);

    for (i=0; i<u32Demod_DSP_len; i++)
    {
        if (pu8Demod_DSP[i] != _MHal_R1B(VD_MCU_SRAM_RD))
        {
            DEMOD_HAL_DBG("DSP code loaded fail (error at byte %u)\n", i);
            bRet = FALSE;
            break;
        }
    }

    if (i == u32Demod_DSP_len)
        DEMOD_HAL_DBG("DSP code loaded successfully\n");
#endif

    /* disable address auto increment */
    _MHal_W1Bb(VD_MCU_ADDR_AUTO_INC, 0x00, _BIT0);

    /* disable down load code */
    _MHal_W1BM(VD_MCU_KEY, 0x00, 0xF0);

    /* enable sram */
    _MHal_W1Bb(VD_MCU_SRAM_EN, 0x01, _BIT0);

    /* release VD mcu */
    _MHal_W1Bb(VD_MCU_RST, 0x00, _BIT0);

	//mail box crash protection 2009-11-06
	if (bRet)
	{
        MDrv_SYS_VDmcuSetType((VDMCU_DSP_TYPE_t)eStandard);
	}
    else
    {
		MDrv_SYS_VDmcuSetType(VDMCU_DSP_UNKNOWN);
    }

    return bRet;
}
//mstar 0901 update
B16 _MHal_Demod_DVBT_Stop (void)
{
    U16     u8WaitCnt=0;
    U8	    RegVal = 0;

    MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
    if (RegVal)
    {
        DEMOD_HAL_DBG(">> MB Busy!\n");
        return FALSE;
    }

    MHal_Demod_WriteReg(MBRegBase + 0x00, 0xA5);                 // MB_CNTL set pause mcu

    MHal_Demod_WriteReg(0x103463, 0x02);                         // assert interrupt to VD MCU51
    MHal_Demod_WriteReg(0x103463, 0x00);                         // de-assert interrupt to VD MCU51

    MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
    while(RegVal != 0x5A)                                        // wait MB_CNTL set done
    {
        MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
        if (u8WaitCnt++ >= 0xFF)
        {
            DEMOD_HAL_DBG(">> DVBT Exit Fail!\n");
            return FALSE;
        }
    }

    MHal_Demod_WriteReg(0x103460, 0x01);                         // reset VD_MCU
    MHal_Demod_WriteReg(MBRegBase + 0x00, 0x00);                 // MB_CNTL clear

    //diable clk gen
    //MDrv_WriteByte(0x103314, 0x01);   // reg_ckg_dvbtc_adc@0x0a[3:0] : ADC_CLK
    //MDrv_WriteByte(0x103315, 0x01);   // reg_ckg_dvbtc_innc@0x0a[11:8]

    MHal_Demod_WriteReg(0x10330a, 0x01);   // reg_ckg_atsc_adcd_sync@0x05[3:0] : ADCCLK
    MHal_Demod_WriteReg(0x10330b, 0x00);

    MHal_Demod_WriteReg(0x10330c, 0x01);   // reg_ckg_dvbtc_inner1x@0x06[3:0] : MPLLDIV10/4=21.5MHz
    MHal_Demod_WriteReg(0x10330d, 0x01);   // reg_ckg_dvbtc_inner2x@0x06[11:8]: MPLLDIV10/2=43.2MHz

    MHal_Demod_WriteReg(0x10330e, 0x01);   // reg_ckg_dvbtc_inner4x@0x07[3:0] : MPLLDIV10=86.4MHz
    MHal_Demod_WriteReg(0x10330f, 0x00);

    MHal_Demod_WriteReg(0x103310, 0x01);   // reg_ckg_dvbtc_outer1x@0x08[3:0] : MPLLDIV10/2=43.2MHz
    MHal_Demod_WriteReg(0x103311, 0x01);   // reg_ckg_dvbtc_outer2x@0x08[11:8]: MPLLDIV10=86.4MHz

    MHal_Demod_WriteReg(0x103312, 0x01);   // dvbt_t:0x0000, dvb_c: 0x0004
    MHal_Demod_WriteReg(0x103313, 0x00);

    MHal_Demod_WriteReg(0x103314, 0x01);   // reg_ckg_dvbtc_adc@0x0a[3:0] : ADC_CLK
    MHal_Demod_WriteReg(0x103315, 0x01);   // reg_ckg_dvbtc_innc@0x0a[11:8]

    MHal_Demod_WriteReg(0x103316, 0x01);   // reg_ckg_dvbtc_eq8x@0x0b[3:0] : MPLLDIV3/2=144MHz
    MHal_Demod_WriteReg(0x103317, 0x01);   // reg_ckg_dvbtc_eq@0x0b[11:8] : MPLLDIV3/16=18MHz

    MHal_Demod_WriteReg(0x103318, 0x11);   // reg_ckg_dvbtc_sram0~3@0x0c[13:0]
    MHal_Demod_WriteReg(0x103319, 0x11);

    MHal_Demod_WriteReg(0x103308, 0x01);   // parallel mode:0x0001 / serial mode: 0x0401
    MHal_Demod_WriteReg(0x103309, 0x05);   // reg_ckg_dvbtc_ts@0x04

    return TRUE;
}

B16 _MHal_Demod_DVBC_Stop (void)
{
    U16     u8WaitCnt=0;
    U8      RegVal=0;

    MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
    if (RegVal)
    {
        DEMOD_HAL_DBG(">> MB Busy!\n");
        return FALSE;
    }

    MHal_Demod_WriteReg(MBRegBase + 0x00, 0xA5);                 // MB_CNTL set read mode

    MHal_Demod_WriteReg(0x103463, 0x02);                         // assert interrupt to VD MCU51
    MHal_Demod_WriteReg(0x103463, 0x00);                         // de-assert interrupt to VD MCU51

    MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
    while(RegVal != 0x5A)                                        // wait MB_CNTL set done
    {
        MHal_Demod_ReadReg(MBRegBase + 0x00, &RegVal);
        if (u8WaitCnt++ >= 0xFF)
        {
            DEMOD_HAL_DBG(">> DVBC Exit Fail!\n");
            return FALSE;
        }
    }

    MHal_Demod_WriteReg(0x103460, 0x01);                         // reset VD_MCU
    MHal_Demod_WriteReg(MBRegBase + 0x00, 0x00);                 // MB_CNTL clear

    //diable clk gen
    MHal_Demod_WriteReg(0x10330a, 0x01);   // reg_ckg_atsc_adcd_sync@0x05[3:0] : ADCCLK
    MHal_Demod_WriteReg(0x10330b, 0x00);

    MHal_Demod_WriteReg(0x10330c, 0x01);   // reg_ckg_dvbtc_inner1x@0x06[3:0] : MPLLDIV10/4=21.5MHz
    MHal_Demod_WriteReg(0x10330d, 0x01);   // reg_ckg_dvbtc_inner2x@0x06[11:8]: MPLLDIV10/2=43.2MHz

    MHal_Demod_WriteReg(0x10330e, 0x01);   // reg_ckg_dvbtc_inner4x@0x07[3:0] : MPLLDIV10=86.4MHz
    MHal_Demod_WriteReg(0x10330f, 0x00);

    MHal_Demod_WriteReg(0x103310, 0x01);   // reg_ckg_dvbtc_outer1x@0x08[3:0] : MPLLDIV10/2=43.2MHz
    MHal_Demod_WriteReg(0x103311, 0x01);   // reg_ckg_dvbtc_outer2x@0x08[11:8]: MPLLDIV10=86.4MHz

    MHal_Demod_WriteReg(0x103312, 0x05);   // dvbt_t:0x0000, dvb_c: 0x0004
    MHal_Demod_WriteReg(0x103313, 0x00);

    MHal_Demod_WriteReg(0x103314, 0x01);   // reg_ckg_dvbtc_adc@0x0a[3:0] : ADC_CLK
    MHal_Demod_WriteReg(0x103315, 0x01);   // reg_ckg_dvbtc_innc@0x0a[11:8]

    MHal_Demod_WriteReg(0x103316, 0x01);   // reg_ckg_dvbtc_eq8x@0x0b[3:0] : MPLLDIV3/2=144MHz
    MHal_Demod_WriteReg(0x103317, 0x01);   // reg_ckg_dvbtc_eq@0x0b[11:8] : MPLLDIV3/16=18MHz

    MHal_Demod_WriteReg(0x103318, 0x11);   // reg_ckg_dvbtc_sram0~3@0x0c[13:0]
    MHal_Demod_WriteReg(0x103319, 0x11);

    MHal_Demod_WriteReg(0x103308, 0x01);   // parallel mode:0x0001 / serial mode: 0x0401
    MHal_Demod_WriteReg(0x103309, 0x05);   // reg_ckg_dvbtc_ts@0x04

    return TRUE;
}

B16 _MHal_Demod_ATSC_Stop (void)
{
    U16      u8CheckCount = 0;
    U8      RegVal = 0;

    //Notify firmware before reset by mailbox
    MHal_Demod_WriteReg(0x11051C, 0x01);
    MHal_Demod_WriteReg(0x103463, 0x02);                         // assert interrupt to VD MCU51
    MHal_Demod_WriteReg(0x103463, 0x00);                         // de-assert interrupt to VD MCU51

    MHal_Demod_ReadReg(0x11051C, &RegVal);
    while ((RegVal&0x02) != 0x02)
    {
        MHal_Demod_ReadReg(0x11051C, &RegVal);
        if (u8CheckCount++ >= 0xFF)
        {
            DEMOD_HAL_DBG(">> ATSC Exit Fail!\n");
            return FALSE;
        }
    }

    return TRUE;
}

B16 MHal_Demod_Stop(DEMOD_STANDARD_TYPE_t eStandard)
{
    B16 bRet = TRUE;

	//mail box crash protection 2009-11-06
	if ((VDMCU_DSP_TYPE_t)eStandard != MDrv_SYS_VDmcuGetType())
    {
        printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType());
        return FALSE;
    }
		
    DEMOD_HAL_DBG("MHal_Demod_Stop !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    switch (eStandard)
    {
        case DEMOD_STANDARD_DVBT:
            bRet = _MHal_Demod_DVBT_Stop();
            DEMOD_HAL_DBG("mode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DEMOD_STANDARD_DVBT\n");
            break;

        case DEMOD_STANDARD_DVBC:
            bRet = _MHal_Demod_DVBC_Stop();
            DEMOD_HAL_DBG("mode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DEMOD_STANDARD_DVBC\n");
            break;

        case DEMOD_STANDARD_ATSC:
            bRet = _MHal_Demod_ATSC_Stop();
            DEMOD_HAL_DBG("mode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DEMOD_STANDARD_ATSC\n");
            break;

        default:
            DEMOD_HAL_DBG("no such standard %d\n", eStandard);
            bRet = FALSE;
            break;
    }


    return bRet;
}

