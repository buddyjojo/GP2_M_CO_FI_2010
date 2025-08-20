#define _AUSIF_C_

#include <linux/kernel.h>
#include <linux/delay.h>
#include "mst_utility.h"
// Internal
#include "mhal_mad_reg.h"
#include "mdrv_mad_common.h"
#include "mdrv_mad_dvb.h"
#include "mdrv_mad_dvb2.h"
#include "mdrv_mad_sif.h"
#include "SifIntialTable.h"
#include "mdrv_system.h"

#define MAD_DEBUG_P1(msg) //msg
#define MAD_DEBUG_P2(msg) //msg
#define MAD_DEBUG_P3(msg) //msg
#define DBG_AUDIO_ERROR(msg) msg

#define VIF_Mode 0
#define PCM_energy_addr 0x3E50

ADEC_SIF_AVAILE_STANDARD_T u8SifStandardType; // A2, Nimca, FM..
ADEC_SIF_SOUNDSYSTEM_T u8SifSoundSystemType;  // BG, DK, I...
BOOL hidevEnabled = 0;


MS_U16 sif_gain_0[6], sif_shift_0[6];
MS_BOOL GetGain0_Done = 0;
MS_U8 g_u8SifDspType = DSP_SE; // for T3, sif is at SE DSP.
MS_U8 g_u8DspCodeType, g_u8Dsp2CodeType;
static MS_BOOL bADCFromVifPathSupported = FALSE, bADCFromVifPathEnabled = FALSE;   // for T3, default enable vif path.
extern spinlock_t                      _mad_spinlock;
//#define AU_CMD_STANDARD                 0x2D20, define in mhal_mad_reg.h
//#define AU_CMD_HIDEVBW                  0x2D22
//#define AU_CMD_MODE1                    0x2D24        //BTSC/A2/EIAJ
//#define AU_CMD_MODE2                    0x2D26        //NICAM
//#define AU_CMD_DEBUG                    0x2D2A
//#define AU_CMD_DBG_CMD                  0x2D38
//#define AU_CMD_DBG_DATA_H               0x2D3A
//#define AU_CMD_DBG_DATA_M               0x2D3C
//#define AU_CMD_DBG_DATA_L               0x2D3E

//#define AU_STATUS_STANDARD              0x2D40
//#define AU_STATUS_MODE1                 0x2D44        //BTSC/A2/EIAJ
//#define AU_STATUS_MODE2                 0x2D46        //NICAM
//#define AU_STATUS_NICAM                 0x2D46
//#define AU_STATUS_DEBUG1                0x2D4D
//#define AU_STATUS_DEBUG2                0x2D4E
//#define AU_STATUS_DBG_H                 0x2D5A
//#define AU_STATUS_DBG_M                 0x2D5C
//#define AU_STATUS_DBG_L                 0x2D5E



// modify threshold for for ADC new setting 090825
THR_TBL_TYPE code AuSifInitThreshold_A2_M[]=
{
#if 1 // change threshold 091208 by LGE Jonghyuk, Lee
	 {CARRIER1_ON_AMP  ,0x06 ,0x80 ,} ,
	 {CARRIER1_OFF_AMP ,0x05 ,0x80 ,} ,
#endif
	 {CARRIER1_ON_NSR  ,0x10 ,0x00 ,} ,
	 {CARRIER1_OFF_NSR ,0x15 ,0x00 ,} ,
#if 0
     {CARRIER2_ON_AMP  ,0x01 ,0x00,} ,
     {CARRIER2_OFF_AMP ,0x00 ,0x80 ,} ,
#else	 // change threshold 100201 by LGE Jonghyuk, Lee
	{CARRIER2_ON_AMP  ,0x03 ,0x00 ,} ,
     {CARRIER2_OFF_AMP ,0x02 ,0x00 ,} ,
#endif
#if 1 // change threshold for Korean fieldtest by LGE Jonghyuk, Lee 100112
     {CARRIER2_ON_NSR  ,0x09/*10*/ ,0x00 ,} ,
     {CARRIER2_OFF_NSR ,0x0b/*50*/ ,0x00 ,} ,
#endif
	 {A2_PILOT_ON_AMP  ,0x02 ,0x00 ,} ,
     {A2_PILOT_OFF_AMP ,0x01 ,0x00 ,} ,
     {A2_PILOT_ON_PHASE ,0x35 ,0x00 ,} ,   // change for the Korea A2 threhsold function.
     {A2_PILOT_OFF_PHASE,0x40 ,0x00 ,} ,   // change for the Korea A2 threhsold function.
};

THR_TBL_TYPE code AuSifInitThreshold_A2_BG[]=
{
#if 1 // change threshold 091208 by LGE Jonghyuk, Lee
  	 {CARRIER1_ON_AMP  ,0x06 ,0x80 ,} ,
     {CARRIER1_OFF_AMP ,0x05 ,0x80 ,} ,
#endif
	 {CARRIER1_ON_NSR  ,0x10 ,0x00 ,} ,
     {CARRIER1_OFF_NSR ,0x7F ,0xFF ,} ,
#if 1 // change threshold 091112 by LGE Jonghyuk, Lee
     {CARRIER2_ON_AMP  ,0x03/*0x04*/ ,0x00/*0x80*/ ,} ,
     {CARRIER2_OFF_AMP ,0x02/*0x03*/ ,0x00 ,} ,
#endif
#if 0
     {CARRIER2_ON_NSR  ,0x25 ,0x00 ,} ,
     {CARRIER2_OFF_NSR ,0x30 ,0x00 ,} ,
#else
	{CARRIER2_ON_NSR  ,0x10 ,0x00 ,} ,
     {CARRIER2_OFF_NSR ,0x15 ,0x00 ,} ,
#endif
     {A2_PILOT_ON_AMP  ,0x02 ,0x00 ,} ,
     {A2_PILOT_OFF_AMP ,0x01 ,0x00 ,} ,
     {A2_PILOT_ON_PHASE ,0x14 ,0x00 ,} ,
     {A2_PILOT_OFF_PHASE,0x1E ,0x00 ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_A2_DK[]=
{
#if 1 // change threshold 091208 by LGE Jonghyuk, Lee
     {CARRIER1_ON_AMP  ,0x06 ,0x80 ,} ,
     {CARRIER1_OFF_AMP ,0x05 ,0x80 ,} ,
#endif
     {CARRIER1_ON_NSR  ,0x10 ,0x00 ,} ,
     {CARRIER1_OFF_NSR ,0x7F ,0xFF ,} ,
#if 1 // change threshold 091112 by LGE Jonghyuk, Lee
     {CARRIER2_ON_AMP  ,0x03/*0x04*/ ,0x00/*0x80*/ ,} ,
     {CARRIER2_OFF_AMP ,0x02/*0x03*/ ,0x00 ,} ,
#endif
     {CARRIER2_ON_NSR  ,0x10 ,0x00 ,} ,
     {CARRIER2_OFF_NSR ,0x15 ,0x00 ,} ,
     {A2_PILOT_ON_AMP  ,0x02 ,0x00 ,} ,
     {A2_PILOT_OFF_AMP ,0x01 ,0x00 ,} ,
     {A2_PILOT_ON_PHASE ,0x14 ,0x00 ,} ,
     {A2_PILOT_OFF_PHASE,0x1E ,0x00 ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_FM_I[]=
{
#if 1 // change threshold 091208 by LGE Jonghyuk, Lee
     {CARRIER1_ON_AMP  ,0x06 ,0x80 ,} ,
	 {CARRIER1_OFF_AMP ,0x05 ,0x80 ,} ,
#endif
	 {CARRIER1_ON_NSR  ,0x10 ,0x00 ,} ,
	 {CARRIER1_OFF_NSR ,0x7F ,0xFF ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_AM[]=
{
#if 1 // change threshold 091208 by LGE Jonghyuk, Lee
         {CARRIER1_ON_AMP  ,0x06 ,0x00 ,} ,
         {CARRIER1_OFF_AMP ,0x05 ,0x00 ,} ,
#endif
};

THR_TBL_TYPE code AuSifInitThreshold_Nicam_BG[]=
{
         {NICAM_ON_SIGERR  ,0x23 ,0x00 ,} ,
         {NICAM_OFF_SIGERR ,0x3f ,0x00 ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_Nicam_I[]=
{
        {NICAM_ON_SIGERR  ,0x23 ,0x00 ,} ,
        {NICAM_OFF_SIGERR ,0x3f ,0x00 ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_HIDEV_M[]=
{
        {CARRIER1_ON_AMP  ,0x00 ,0x80 ,} ,
        {CARRIER1_OFF_AMP ,0x00 ,0x40 ,} ,
        {CARRIER1_ON_NSR  ,0x1A ,0x50 ,} ,
        {CARRIER1_OFF_NSR ,0x3A ,0xF0 ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_HIDEV_BG[]=
{
        {CARRIER1_ON_AMP  ,0x00 ,0xB0 ,} ,
        {CARRIER1_OFF_AMP ,0x00 ,0x60 ,} ,
        {CARRIER1_ON_NSR  ,0x16 ,0x50 ,} ,
        {CARRIER1_OFF_NSR ,0x2A ,0xF0 ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_HIDEV_DK[]=
{
	{CARRIER1_ON_AMP  ,0x00 ,0xB0 ,} ,
	{CARRIER1_OFF_AMP ,0x00 ,0x60 ,} ,
	{CARRIER1_ON_NSR  ,0x1E ,0xA0 ,} ,
	{CARRIER1_OFF_NSR ,0x28 ,0x00 ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_HIDEV_I[]=
{
	{CARRIER1_ON_AMP  ,0x00 ,0xB0 ,} ,
	{CARRIER1_OFF_AMP ,0x00 ,0x60 ,} ,
	{CARRIER1_ON_NSR  ,0x16 ,0x50 ,} ,
	{CARRIER1_OFF_NSR ,0x2A ,0xF0 ,} ,
};

THR_TBL_TYPE code AuSifInitThreshold_BTSC[]=
{
    { BTSC_MONO_ON_NSR_RATIO  ,	0x20,	0x00,} ,
    {BTSC_MONO_OFF_NSR_RATIO,	0x33,	0x33,} ,
// Modified by LGE
    {BTSC_PILOT_AMPLITUDE_ON,	0x20,	0x00,} ,
    {BTSC_PILOT_AMPLITUDE_OFF,	0x15,	0x00,} ,
//    {BTSC_PILOT_AMPLITUDE_ON,	0x25,	0x00,} ,
//    {BTSC_PILOT_AMPLITUDE_OFF,	0x20,	0x00,} ,
    { BTSC_SAP_ON_NSR_RATIO,	0x35,	0x00,} ,
    {BTSC_SAP_OFF_NSR_RATIO,	0x50,	0x50,} ,
// Modified by LGE
//    { BTSC_STEREO_ON_RATIO,		0x0B,	0x00,} ,
//    {BTSC_STEREO_OFF_NSR_RATIO,	0x12,	0x00,} ,
    { BTSC_STEREO_ON_RATIO,		0x20,	0x00,} ,
    {BTSC_STEREO_OFF_NSR_RATIO,	0x26,	0x00,} ,
    { BTSC_SAP_AMPLITUDE_ON,	0x02,	0x00,} ,
    { BTSC_SAP_AMPLITUDE_OFF,	0x01,	0x80,} ,
    {BTSC_HIDEV_ON_NSR_RATIO ,  0x42,	0x00,} ,
    {BTSC_HIDEV_OFF_NSR_RATIO,  0x50,	0x00,} ,
};


#if 1
#define LGBOARD	// by bionhu(Jonghyuk LEE) 080904
#endif

extern U8 dtv_mode_en;

BYTE code au_pal_sys_info[5][5]=
{
    //num,  default,        2nd,                3nd             4nd
    {2, AU_SYS_BG_A2,       AU_SYS_BG_NICAM,    0,              0,              },
    {4, AU_SYS_DK1_A2,      AU_SYS_DK2_A2,      AU_SYS_DK3_A2,  AU_SYS_DK_NICAM,},
    {1, AU_SYS_I_NICAM,     0,                  0,              0,              },
    {1, AU_SYS_L_NICAM,     0,                  0,              0,              },
    {1, AU_SYS_NOT_READY,   0,                  0,              0,              },
};

// drivers for SIF output gain control //
U32  PRESCALE_STEP_TBL[]=
{
    0x7E2B,       // -0.125dB
    0x7C5E,       // -0.25dB
    0x78D6,       // -0.5dB
    0x7214,       // -1dB
    0x65AC,       // -2dB
    0x50C3,       // -4dB
};

U32 PAL_SIF_PM_GAIN_TBL[6][2]=
{
   { A2_OUTPUT_GAIN_X_ADDR,  A2_OUTPUT_GAIN_X_ADDR+1},        //A2 / FM-MONO
   { NICAM_OUTPUT_GAIN_ADDR,  NICAM_OUTPUT_GAIN_ADDR+1},    //NICAM
   { AM_OUTPUT_GAIN_ADDR,  AM_OUTPUT_GAIN_ADDR+1},              //AM
   { A2_OUTPUT_GAIN_X_ADDR+2,  A2_OUTPUT_GAIN_X_ADDR+3},  //HIDEV
   { A2_OUTPUT_GAIN_M_ADDR,  A2_OUTPUT_GAIN_M_ADDR+1},  //FM-MONO M
   { A2_OUTPUT_GAIN_M_ADDR+2,  A2_OUTPUT_GAIN_M_ADDR+3},  //FM-MONO M HIDEV
};

U32 BTSC_SIF_PM_GAIN_TBL[1][2]=
{
   { BTSC_OUTPUT_GAIN_ADDR,  BTSC_OUTPUT_GAIN_ADDR+1},        // BTSC
};
//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_AuSif_setMemInfo
//  [Description]:
//      Set memory information
//  [Arguments]:
//      None
//  [Return]:
//      None
//  [for Doxygen]
//*******************************************************************************
void MDrv_MAD_AuSif_setMemInfo(void)
{
    //printk("MDrv_AuSif_setMemInfo MDrv_MAD_AuSif_setMemInfo = 0x%08LX\n", MadBaseBufferAdr);

    // ICACHE BASE
    MHal_MAD_DecWriteReg(REG_DEC_DSP_ICACHE_BASE_L, MAD_MEM_START>>11);     // MAD_MEM_START: 0x1F00000, 0x001F);

    // MAD BASE
    MHal_MAD_DecWriteReg(REG_DEC_MAD_OFFSET_BASE_H, MAD_MEM_START>>11);      // MAD_MEM_START: 0x1F00000, 0x001F);
/*****
    // Config as SIF ch1
    MDrv_AuWriteByte(REG_DEC_MCFG, 0x0000);
    MDrv_AuWriteByte(REG_DEC_MBASE_H, 0x0000);       //
    MDrv_AuWriteByte(REG_DEC_MSIZE_H, 0x000F);       //

    // Config as SIF ch2
    MDrv_AuWriteByte(REG_DEC_MCFG, 0x0001);
    MDrv_AuWriteByte(REG_DEC_MBASE_H, 0x0030);       //
    MDrv_AuWriteByte(REG_DEC_MSIZE_H, 0x000F);       //
*****/

    // Sound Effect DMA
    //MHal_MAD_DecWriteReg(AUD_MEM_CFG, 0x0006);
    //MHal_MAD_DecWriteReg(AUD_MBASE_H, 0x00B0);     // SE_DMA base : 0x9000, for non-AAC case
    //MHal_MAD_DecWriteReg(AUD_MSIZE_L, 0x001B);     // SE_DMA size : 56KB

    // Reset MAD module
    MDrv_MAD_ResetMAD();

    //printk("ICACHE addr = %2bx\r\r", MDrv_AuReadByte(REG_DEC_DSP_ICACHE_BASE_L));
    //printk("MAD Base addr = %2bx\r\r", MDrv_AuReadByte(REG_DEC_MAD_OFFSET_BASE_H));
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_SIF_LoadCode
//  [Description]:
//      This routine load the SIF algorithm code.
//  [Arguments]:
//      None
//  [Return]:
//      None
//
//*******************************************************************************

BOOL MDrv_MAD_SIF_LoadCode(U8 u8Type)
{
    MS_BOOL    ret_status;
    MS_U8 u8Type1;

    if (MDrv_MAD_SIF_GetADCFromVifPathFlag() == TRUE)
        u8Type1 = u8Type | SIF_ADC_FROM_VIF_PATH;
    else
        u8Type1 = u8Type & (~SIF_ADC_FROM_VIF_PATH);
    if ((u8Type & 0xf0) == AU_STANDARD_SIF_TYPE) {//sif load code
//        if ((MDrv_MAD_SIF_GetDspCodeType())== u8Type)                           //if u8Type is the same, don't reload again
        if (((MDrv_MAD_SIF_GetDspCodeType())== u8Type) &&                           //if u8Type is the same, don't reload again
            (MDrv_MAD_SIF_GetDspCodeVifFlag() == MDrv_MAD_SIF_GetADCFromVifPathFlag()))
            return TRUE;

        MAD_DEBUG_P1(printk("MDrv_MAD_SIF_LoadCode(u8Type= SIF(%x))\r\n",u8Type));
        MDrv_MAD_SIF_ENABLE_CHANNEL(false);        //Reset SIF
        MDrv_MAD_SIF_WriteCommand(AU_SIF_WRITE_CARRIER_DEBOUNCE, 0x80);
        MDrv_MAD_SIF_WriteCommand(AU_SIF_WRITE_DISABLE_SIF_SYNTHESIZER,0x00);             // disable SIF Audio sythesizer & disable DVB fix-sync mode

        if (MDrv_MAD_SIF_GetDspType() == DSP_DEC) {

            MDrv_MAD_DspLoadCode(u8Type1);
            MDrv_MAD_SetIsDtvFlag(FALSE);
        }
        else if (MDrv_MAD_SIF_GetDspType() == DSP_SE) {
            //MsOS_DisableInterrupt(E_FIQ_DEC_DSP2UP);//CHECK
            MAD_LOCK();
            // Reset MAD module
            MDrv_MAD2_DisEn_MIUREQ();
            ret_status = MDrv_MAD_Alg2ReloadCode(u8Type1);
            //MsOS_EnableInterrupt(E_FIQ_DEC_DSP2UP);//CHECK
            MAD_UNLOCK();
        }
        MDrv_MAD_SIF_SetDspCodeType(u8Type1);
        MDrv_MAD_SIF_ResetGetGain0();
        MDrv_MAD_SIF_GetOrginalGain();

        if (u8Type1==AU_SIF_BTSC)
        MDrv_MAD_WriteSIFThresholdTbl_BTSC();
        else
        MDrv_MAD_WriteSIFThresholdTbl_PAL();

        MDrv_MAD_SIF_ENABLE_CHANNEL(true);
        // Enable SIF Audio synthesizer here to prevent reload unstable noise.
        MDrv_MAD_SIF_WriteCommand(AU_SIF_WRITE_ENABLE_SIF_SYNTHESIZER,0x04);            // enable SIF Audio sythesizer
        MDrv_MAD_SIF_TriggerSifPLL(); // add for SIF ADC
        MHal_MAD_SeWriteRegMaskByte(AU_CMD_FC_TRACK, 0x80, 0x80); // enable the DK real-time monitor //CHECK
        return TRUE;
    }
    else {
        DBG_AUDIO_ERROR(printk("MDrv_MAD_SIF_LoadCode: type(0x%x) is invalid\r\n",u8Type));
        return FALSE;
    }
}


//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_SIF_ReLoadCode
//  [Description]:
//      This routine reload the SIF algorithm code.
//  [Arguments]:
//      None
//  [Return]:
//      None
//
//*******************************************************************************

BOOL MDrv_MAD_SIF_ReLoadCode(U8 u8Type)
{
    MS_U8 u8Type1;
    dtv_mode_en = 0;
    MAD_DEBUG_P1(printk("=====MDrv_MAD_SIF_ReLoadCode (U8: type= 0x(%x)== \r\n",u8Type));
    MDrv_SIF_ADC_Reset();
    if (MDrv_MAD_SIF_GetADCFromVifPathFlag() == TRUE)
        u8Type1 = u8Type | SIF_ADC_FROM_VIF_PATH;
    else
        u8Type1 = u8Type & (~SIF_ADC_FROM_VIF_PATH);
    if ((u8Type & 0xf0) == AU_STANDARD_SIF_TYPE) {//sif load code
//        if ((HAL_SIF_GetDspCodeType())== u8Type)                           //if u8Type is the same, don't reload again
        if (((MDrv_MAD_SIF_GetDspCodeType())== u8Type) &&                           //if u8Type is the same, don't reload again
            (MDrv_MAD_SIF_GetDspCodeVifFlag() == MDrv_MAD_SIF_GetADCFromVifPathFlag()))
            return TRUE;

        MAD_DEBUG_P2(printk("MDrv_SIF_ReLoadCode(u8Type= SIF(%x))\r\n",u8Type));

        MDrv_MAD_SIF_ENABLE_CHANNEL(false);        //Reset SIF
        MDrv_MAD_SIF_WriteCommand(AU_SIF_WRITE_CARRIER_DEBOUNCE, 0x80);
        MDrv_MAD_SIF_WriteCommand(AU_SIF_WRITE_DISABLE_SIF_SYNTHESIZER,0x00);             // disable SIF Audio sythesizer & disable DVB fix-sync mode

        if (MDrv_MAD_SIF_GetDspType() == DSP_DEC) {
            MAD_DEBUG_P3(printk("MDrv_SIF_ReLoadCode(u8Type= SIF(%x))\r\n",u8Type));

            ////MsOS_DisableInterrupt(E_FIQ_SE_DSP2UP);//CHECK//CHECK
            // Reset MAD module
            MDrv_MAD_DisEn_MIUREQ();
            MDrv_MAD_AlgReloadCode(u8Type1);
            ////MsOS_EnableInterrupt(E_FIQ_SE_DSP2UP);//CHECK//CHECK
            MDrv_MAD_SetIsDtvFlag(FALSE);
        }
        else if (MDrv_MAD_SIF_GetDspType() == DSP_SE) {
            //MsOS_DisableInterrupt(E_FIQ_DEC_DSP2UP);//CHECK
            MAD_LOCK();
            // Reset MAD module
            MDrv_MAD2_DisEn_MIUREQ();
            MDrv_MAD_Alg2ReloadCode(u8Type1);
            //MsOS_EnableInterrupt(E_FIQ_DEC_DSP2UP);//CHECK
            MAD_UNLOCK();
        }
        MDrv_MAD_SIF_SetDspCodeType(u8Type1);
        MDrv_MAD_SIF_ResetGetGain0();
        MDrv_MAD_SIF_GetOrginalGain();

        if (u8Type1==AU_SIF_BTSC)
        MDrv_MAD_WriteSIFThresholdTbl_BTSC();
        else
        MDrv_MAD_WriteSIFThresholdTbl_PAL();

        MDrv_MAD_SIF_ENABLE_CHANNEL(true);
        // Enable SIF Audio synthesizer here to prevent reload unstable noise.
        MDrv_MAD_SIF_TriggerSifPLL(); // add for SIF ADC
        MDrv_MAD_SIF_WriteCommand(AU_SIF_WRITE_ENABLE_SIF_SYNTHESIZER,0x04);            // enable SIF Audio sythesizer
        MHal_MAD_SeWriteRegMaskByte(AU_CMD_FC_TRACK, 0x80, 0x80); // enable the DK real-time monitor //CHECK
        return TRUE;
    }
    else {
        DBG_AUDIO_ERROR(printk("MDrv_MAD_SIF_ReLoadCode: type(0x%x) is invalid\r\n",u8Type));
        return FALSE;
    }
}

BOOL MDrv_MAD_GetBtscA2StereoLevel(U16 *pLevel)
{
    U8  data[2];
       U8 u8codeTypeDSP = MDrv_MAD_SIF_GetDspCodeType();

    if (u8codeTypeDSP == AU_SIF_PALSUM) {
    // use debug command 0x32 to get strength of 2nd fm amplitude
    // use debug command 0x3D to get strength of 2nd fm NSR by Allan.Liang
        MHal_MAD_DecWriteRegByte(ADEC_SIF_DEBUG_CMD, 0x3D);
        MAD_Delayms(5);
        data[1] = (U8)MHal_MAD_ReadByte(REG_MB_DEC_RESULT_D1);
        data[0] = (U8)MHal_MAD_ReadByte(REG_MB_DEC_RESULT_D2);
        *pLevel =   (((U32)data[1])<<8)  | data[0];
        return (TRUE);
    }
    else if (u8codeTypeDSP == AU_SIF_BTSC) {
        MHal_MAD_DecWriteRegByte(ADEC_SIF_DEBUG_CMD, 0x87);
        MAD_Delayms(5);
        data[1] = (U8)MHal_MAD_ReadByte(ADEC_SIF_DEBUG_DATA1);
        data[0] = (U8)MHal_MAD_ReadByte(ADEC_SIF_DEBUG_DATA2);
        *pLevel =   (((U32)data[1])<<8)  | data[0];
        return (TRUE);
    }
    return (FALSE);
}

void MDrv_MAD_GetCurAnalogMode(U16* AlgMode)
{
#if 1 //CHECK
    U8 mode=0,uc2D44, uc2D46;
    u8  uc2D20=MHal_MAD_SeReadRegByte(AU_CMD_STANDARD)&0xFF;//CHECK
    U8 u8codeTypeDSP = MDrv_MAD_SIF_GetDspCodeType();


    if (u8codeTypeDSP == AU_SIF_BTSC) {
        uc2D44 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE1);
        if((uc2D44&0x07)==0x07)
            mode = ATV_GET_NTSC_BTSC_SAP_STEREO;
        else if((uc2D44&0x07)==0x03)
            mode = ATV_GET_NTSC_BTSC_STEREO;
        else if ((uc2D44&0x07)==0x05)
            mode = ATV_GET_NTSC_BTSC_SAP_MONO;
        else
            mode = ATV_GET_NTSC_BTSC_MONO; // for LGE spec, no singal it need to white noise
    }

    else if (u8SifStandardType == ADEC_SIF_NICAM) {
        uc2D46 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE2);
        if((uc2D46&0x70)==0x10)
            mode = ATV_GET_PAL_NICAM_MONO;
        else if((uc2D46&0x70)==0x20)
            mode = ATV_GET_PAL_NICAM_STEREO;
        else if ((uc2D46&0x70)==0x30)
            mode = ATV_GET_PAL_NICAM_DUAL;
        else
            mode = ATV_GET_PAL_MONO;
    }
    else if ((u8SifStandardType == ADEC_SIF_A2)&&(uc2D20!=0x23)) {/*한국향 stereo,dual 안 됨 수정  2008.10.29 change from (uc2D20!=0x03)to (uc2D20!=0x23) */
        uc2D44 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE1);
        if((uc2D44&0x3B)==0x3B)
            mode = ATV_GET_PAL_STEREO;
        else if((uc2D44&0x3D)==0x3D)
            mode = ATV_GET_PAL_DUAL;
        else
            mode = ATV_GET_PAL_MONO;
    }
    else if ((u8SifStandardType == ADEC_SIF_A2)&&(uc2D20==0x23)) {/*한국향 stereo,dual 안 됨 수정  2008.10.29 change from (uc2D20!=0x03)to (uc2D20!=0x23) */
        uc2D44 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE1);
        if((uc2D44&0x3B)==0x3B)
            mode = ATV_GET_NTSC_A2_STEREO;
        else if((uc2D44&0x3D)==0x3D)
            mode = ATV_GET_NTSC_A2_SAP;
        else
            mode = ATV_GET_NTSC_A2_MONO;
    }
    else // for ADEC_SIF_FM
            mode = ATV_GET_PAL_MONO;

    *AlgMode = mode;
#endif
}

U16 MDrv_MAD_SetUserAnalogMode(U16 AlgMode)
{
MAD_DEBUG_P1(printk("%s is called.Set Analog Mode = 0x%x\r\n", __FUNCTION__, AlgMode));
#if 1 //CHECK
    switch(AlgMode){
        case ATV_SET_PAL_MONO:
            if (u8SifStandardType == ADEC_SIF_NICAM) {
                MHal_MAD_SeWriteRegByte(AU_CMD_MODE2,0x01);
                //MHal_MAD_DecWriteReg(AU_CMD_MODE1,0x80);
                MHal_MAD_SeWriteRegByte(AU_CMD_MODE1,0x00);// uese the force mode by Allan.Liang
            }
            else // (u8SifStandardType == ADEC_SIF_A2) or (u8SifStandardType == ADEC_SIF_FM)
           	{
#if 1 // Set NICAM register for MONO only by Jonghyuk, Lee 100205
				MHal_MAD_SeWriteRegByte(AU_CMD_MODE2,0x01);
#endif
				//MHal_MAD_DecWriteReg(AU_CMD_MODE1,0x80);
                MHal_MAD_SeWriteRegByte(AU_CMD_MODE1,0x00);// uese the force mode by Allan.Liang
           	}
            MAD_DEBUG_P2(printk("[SetUserAnalogMode:SET_MONO]\n"));
            break;
        case ATV_SET_PAL_MONO_FORCED:
            if (u8SifStandardType == ADEC_SIF_NICAM) {
                MHal_MAD_SeWriteRegByte(AU_CMD_MODE2,0x01);
                MHal_MAD_SeWriteRegByte(AU_CMD_MODE1,0x00);
            }
            else // (u8SifStandardType == ADEC_SIF_A2) or (u8SifStandardType == ADEC_SIF_FM)
           	{
#if 1 // Set NICAM register for MONO only by Jonghyuk, Lee 100205
            	MHal_MAD_SeWriteRegByte(AU_CMD_MODE2,0x01);
#endif
                MHal_MAD_SeWriteRegByte(AU_CMD_MODE1,0x00);
           	}
            MAD_DEBUG_P2(printk("[SetUserAnalogMode:SET_MONO]\n"));
            break;
        case ATV_SET_PAL_STEREO:
            //MHal_MAD_DecWriteReg(AU_CMD_MODE1, 0x81);
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x01);// uese the force mode by Allan.Liang
            MAD_DEBUG_P2(printk("[SetUserAnalogMode:SET_STEREO]\n"));
            break;
        case ATV_SET_PAL_STEREO_FORCED:
#if 1	//By Jonghyuk, LEE 080910:: To avoid FM mode setting even if the system is not FM.
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x00);
#else
            MHal_MAD_DecWriteReg(AU_CMD_MODE1, 0x01);
#endif
            break;
        case ATV_SET_PAL_DUALI:
            //MHal_MAD_DecWriteReg(AU_CMD_MODE1, 0x80);// uese the force mode by Allan.Liang
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x00);
            break;
        case ATV_SET_PAL_DUALII:
            //MHal_MAD_DecWriteReg(AU_CMD_MODE1, 0x82);// uese the force mode by Allan.Liang
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x02);
            break;
        case ATV_SET_PAL_DUALI_II:
            //MHal_MAD_DecWriteReg(AU_CMD_MODE1, 0x83);// uese the force mode by Allan.Liang
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x03); //uese the force mode by Allan.Liang
            break;
        case ATV_SET_PAL_NICAM_MONO:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE2, 0x08);// uese the force mode by Allan.Liang
            break;
        case ATV_SET_PAL_NICAM_MONO_FORCED:
#if 1	 // LGE By JongHyuk Lee 2008.08.20
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE2, 0x01);
#else
            MHal_MAD_DecWriteReg(AU_CMD_MODE2, 0x88);
#endif
            break;
        case ATV_SET_PAL_NICAM_STEREO:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE2, 0x02);
            break;
        case ATV_SET_PAL_NICAM_STEREO_FORCED:
#if 1 	// LGE By JongHyuk Lee 2008.08.20
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE2, 0x01);
#else
            MHal_MAD_DecWriteReg(AU_CMD_MODE2, 0x82);
#endif
            break;
        case ATV_SET_PAL_NICAM_DUALI:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE2, 0x06);
            break;
        case ATV_SET_PAL_NICAM_DUALII:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE2, 0x07);
            break;
        case ATV_SET_PAL_NICAM_DUALI_II:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE2, 0x05);
            break;
        case ATV_SET_PAL_NICAM_DUAL_FORCED:
#if 1	 // LGE By JongHyuk Lee 2008.08.20
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE2, 0x01);
#else
            MHal_MAD_DecWriteReg(AU_CMD_MODE2, 0x87);
#endif
            break;
         case ATV_SET_NTSC_A2_MONO:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x00);
            break;
         case ATV_SET_NTSC_A2_STEREO:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x01);
            break;
         case ATV_SET_NTSC_A2_SAP:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x02);
            break;
         case ATV_SET_NTSC_BTSC_MONO:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x00);
            break;
         case ATV_SET_NTSC_BTSC_STEREO:
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x01);
            break;
         case ATV_SET_NTSC_BTSC_SAP_MONO:   //need to check what is SAP Mono
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x02);
            break;
         case ATV_SET_NTSC_BTSC_SAP_STEREO: // need to check what is SAP Stereo
            MHal_MAD_SeWriteRegByte(AU_CMD_MODE1, 0x02);
            break;

          default:
              DBG_AUDIO_ERROR(printk("[SetUserAnalogMode]:ERROR!!\n"));
              break;
        }

    return 0;
	#endif
	return 0;
}

BOOL MDrv_MAD_SIF_SetModeSetup(U8 SystemType)
{
    U8 sifStandardCmd;

    // fix me: set to driver
    if (SystemType == ADEC_SIF_BG_NICAM) {
        sifStandardCmd = 0x48;
        u8SifStandardType = ADEC_SIF_NICAM;
    }
    else if (SystemType == ADEC_SIF_BG_FM) {
        if (hidevEnabled)
            sifStandardCmd = 0x14;
        else
        sifStandardCmd = 0x04;
        u8SifStandardType = ADEC_SIF_FM;
    }
    else if (SystemType == ADEC_SIF_BG_A2) {
        sifStandardCmd = 0x24;
        u8SifStandardType = ADEC_SIF_A2;
    }
#if 0 /* 9.25 released version */
    else if (SystemType == ADEC_SIF_I_NICAM) {
    	sifStandardCmd = 0x4A;
        u8SifStandardType = ADEC_SIF_NICAM;
    }
    else if (SystemType == ADEC_SIF_I_FM) {
        if (hidevEnabled)
            sifStandardCmd = 0x1A;
        else
        sifStandardCmd = 0x0A;
        u8SifStandardType = ADEC_SIF_FM;
    }
#endif
    else if ((SystemType == ADEC_SIF_I_NICAM) || (SystemType == ADEC_SIF_I_FM)) {
        if (hidevEnabled)
            sifStandardCmd = 0x5A;
        else
        sifStandardCmd = 0x4A;
        u8SifStandardType = ADEC_SIF_NICAM;
    }
    else if (SystemType == ADEC_SIF_DK_NICAM)   {
        sifStandardCmd = 0x49;
        u8SifStandardType = ADEC_SIF_NICAM;
    }
    else if (SystemType == ADEC_SIF_DK_FM)  {
        if (hidevEnabled)
            sifStandardCmd = 0x15;
        else
        sifStandardCmd = 0x05;
        u8SifStandardType = ADEC_SIF_FM;
    }
    else if (SystemType == ADEC_SIF_DK1_A2) {
        sifStandardCmd = 0x25;
        u8SifStandardType = ADEC_SIF_A2;
    }
    else if (SystemType == ADEC_SIF_DK2_A2) {
        sifStandardCmd = 0x26;
        u8SifStandardType = ADEC_SIF_A2;
    }
    else if (SystemType == ADEC_SIF_DK3_A2) {
        sifStandardCmd = 0x27;
        u8SifStandardType = ADEC_SIF_A2;
    }
    else if ((SystemType == ADEC_SIF_L_NICAM) || (SystemType == ADEC_SIF_L_AM)) {
        sifStandardCmd = 0x4B;
        u8SifStandardType = ADEC_SIF_NICAM;
    }
    else if (SystemType == ADEC_SIF_MN_A2)  {
        sifStandardCmd = 0x23;
        u8SifStandardType = ADEC_SIF_A2;
    }
        else if (SystemType == ADEC_SIF_MN_BTSC)    {
        sifStandardCmd = 0x01;
        u8SifStandardType = ADEC_SIF_MN_BTSC;
    }
    else {
        return FALSE;
    }
	MHal_MAD_SeWriteRegMaskByte(ADEC_SIF_CMD_STANDARD, 0xFF, (U16)sifStandardCmd);

    if ((SystemType == ADEC_SIF_L_NICAM) || (SystemType == ADEC_SIF_L_AM)){
        MHal_MAD_SeWriteRegMask(AU_CMD_PFIRBANDWIDTH, 0x0100, 0x0000 );//disable the SIF AGC by Allan Liang
        msleep(10);
        MHal_MAD_SeWriteRegMask(AU_CMD_PFIRBANDWIDTH, 0x0200, 0x0200 );//reset the SIF AGC by Allanliang
        msleep(10);
        MHal_MAD_SeWriteRegMask(AU_CMD_PFIRBANDWIDTH, 0x0200, 0x0000 );//reset the SIF AGC by Allanliang
        //MAD_DEBUG_P2(printk("######MDrv_MAD_SIF_SetModeSetup AGL L: type(%x) \r\n",SystemType));

    }
    else{
    	MHal_MAD_SeWriteRegMask(AU_CMD_PFIRBANDWIDTH, 0x0100, 0x0100 );//enable the SIF AGC by Allan Liang
        //MAD_DEBUG_P2(printk("######MDrv_MAD_SIF_SetModeSetup AGC: type(%x)  \r\n",SystemType));
    }



    return TRUE;
}

BOOL MDrv_MAD_SIF_SetHDEVMode(U8 bOnOff)
{
    hidevEnabled = bOnOff;
    return TRUE;
}

BOOL   MDrv_MAD_SIF_GetBandDetect(U8 soundSystem, U32 *bandStrength)
{
    U8  sifStandardCmd;
    U8  data[3];

    // fix me: get from driver
    *bandStrength = 0; // initial strength to 0.

    if (soundSystem == ADEC_SIF_SYSTEM_BG) sifStandardCmd = 0x04;
    else if (soundSystem == ADEC_SIF_SYSTEM_DK) sifStandardCmd = 0x05;
    else if (soundSystem == ADEC_SIF_SYSTEM_I) sifStandardCmd = 0x0A;
    else if (soundSystem == ADEC_SIF_SYSTEM_MN) sifStandardCmd = 0x03;
    else return (FALSE);

    MHal_MAD_SeWriteRegMaskByte(ADEC_SIF_CMD_STANDARD, 0xEF, (U16)sifStandardCmd);
#if 1 // LGE ::  Sound System detection is sometimes wrong for AUS. Jonghyuk, Lee 090216
	MAD_Delayms(100);
#endif

    // use debug command 0x30 to get strength of the standard.

//    MHal_MAD_DecWriteRegByte(ADEC_SIF_DEBUG_CMD, 0x30);
    MHal_MAD_SeWriteRegByte(ADEC_SIF_DEBUG_CMD, 0x3C); //to used the 1/NSR value replace the amplitude
#if 1 // LGE ::  Sound System detection is sometimes wrong for AUS. Jonghyuk, Lee 090216
    MAD_Delayms(200/*100*/);        // 200ms
#endif
    data[2] = (U8)MHal_MAD_SeReadRegByte(REG_MB_SE_ACK1);
    data[1] = (U8)MHal_MAD_SeReadRegByte(REG_MB_SE_ACK2+1);//morris:check
    data[0] = (U8)MHal_MAD_SeReadRegByte(REG_MB_SE_ACK2);

    *bandStrength = ((((U32)data[2])<<16)  |  (((U32)data[1])<<8)  | data[0]);
    return (TRUE);
}

BOOL MDrv_MAD_SIF_SetBandSetup(U8 SifBand)
{
    U8  sifStandardCmd;

    // fix me: set to driver
    if (SifBand == ADEC_SIF_SYSTEM_BG) {
        sifStandardCmd = 0x04;
        u8SifSoundSystemType = ADEC_SIF_SYSTEM_BG;
    }
    else if (SifBand == ADEC_SIF_SYSTEM_DK) {
        sifStandardCmd = 0x05;
        u8SifSoundSystemType = ADEC_SIF_SYSTEM_DK;
    }
    else if (SifBand == ADEC_SIF_SYSTEM_I) {
        sifStandardCmd = 0x0A;
        u8SifSoundSystemType = ADEC_SIF_SYSTEM_I;
    }
    else if (SifBand == ADEC_SIF_SYSTEM_L) {
        sifStandardCmd = 0x04B;
        u8SifSoundSystemType = ADEC_SIF_SYSTEM_L;
    }
    else if (SifBand == ADEC_SIF_SYSTEM_MN) {
        sifStandardCmd = 0x03;
        u8SifSoundSystemType = ADEC_SIF_SYSTEM_MN;
    }
    else return (FALSE);

    MHal_MAD_SeWriteRegMaskByte(ADEC_SIF_CMD_STANDARD, 0xFF, sifStandardCmd);


    if (SifBand == ADEC_SIF_SYSTEM_L){
        MHal_MAD_SeWriteRegMask(AU_CMD_PFIRBANDWIDTH, 0x0100, 0x0000 );//disable the SIF AGC by Allan Liang
        msleep(10);
        MHal_MAD_SeWriteRegMask(AU_CMD_PFIRBANDWIDTH, 0x0200, 0x0200 );//reset the SIF AGC by Allanliang
        msleep(10);
        MHal_MAD_SeWriteRegMask(AU_CMD_PFIRBANDWIDTH, 0x0200, 0x0000 );//reset the SIF AGC by Allanliang
        //MAD_DEBUG_P1(printk("########MDrv_MAD_AuSif_AGC: type(%x) ############\r\n",SifBand));
    }
    else{
    	MHal_MAD_DecWriteRegMask(0x2D22, 0x0100, 0x0100 );//enable the SIF AGC by Allan Liang
    }

    return (TRUE) ;
}

BOOL MDrv_MAD_SIF_CheckAvailableSystem(U8 standard, U8 *exist)
{
    U8 uc2D20=0, uc2D44=0, uc2D46=0; // register buffer
    U8 sifStandardCmd;
    U8 u8codeTypeDSP = MDrv_MAD_SIF_GetDspCodeType();
    MHal_MAD_SeWriteRegMaskByte(AU_CMD_FC_TRACK, 0x80, 0x80); // enable the DK real-time monitor //CHECK
    *exist  = ADEC_SIF_ABSENT;

    uc2D20  = MHal_MAD_SeReadRegByte(ADEC_SIF_CMD_STANDARD);
    sifStandardCmd = uc2D20 & 0x0F;
    if (u8codeTypeDSP == AU_SIF_PALSUM) {
        if ((sifStandardCmd < 0x03) || (sifStandardCmd > 0xB))
            return FALSE;
    }
    else return FALSE;

    uc2D44 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE1);//CHECK
    uc2D46 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE2);//CHECK

    if (standard == ADEC_SIF_NICAM) {
        if ((uc2D46&0x0f) == 0x05)
            *exist  = ADEC_SIF_PRESENT;
    }
    else if (standard == ADEC_SIF_A2) {
        if (((uc2D44&0x3B) == 0x3B) || ((uc2D44&0x3D) == 0x3D))
            *exist  = ADEC_SIF_PRESENT;
    }
    else if (standard == ADEC_SIF_FM) {
#if 1	//By Jonghyuk, LEE 080910:: To avoid FM mode setting even if the system is not FM.
   		if((((uc2D44&0x10) == 0x10) && (((uc2D44&0x3B) != 0x3B) && ((uc2D44&0x3D) != 0x3D) && ((uc2D46&0x0f) != 0x05)))
			||(((uc2D44&0xFF) == 0x01) && ((uc2D46&0x0f) != 0x05)))
#else
		if(uc2D44&0x10)
#endif
            *exist  = ADEC_SIF_PRESENT;
    }

    // fix me: check if standard is avaliable and return to *availability
    return(TRUE);
}


BOOL MDrv_MAD_SIF_CheckA2DK(U8 SystemType, U8 *exist)
{
    U8 uc2D20=0, uc2D44=0, uc2D46=0; // register buffer
    U8 sifStandardCmd;
    U8 u8codeTypeDSP = MDrv_MAD_SIF_GetDspCodeType();

    *exist  = ADEC_SIF_ABSENT;
    MHal_MAD_SeWriteRegMaskByte(AU_CMD_FC_TRACK, 0x80, 0x80); // enable the DK real-time monitor //CHECK

    uc2D20  = MHal_MAD_SeReadReg(ADEC_SIF_CMD_STANDARD);
    sifStandardCmd = uc2D20 & 0x0F;
    if (u8codeTypeDSP == AU_SIF_PALSUM) {
        if ((sifStandardCmd < 0x05) || (sifStandardCmd > 9))
            return FALSE;
    }
    else return FALSE;

    uc2D44 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE1);//CHECK
    uc2D46 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE2);//CHECK
#if 1	// 080925 Need to check DK3 A2. So, disable check for other mode. By LGE Jonghyuk, Lee
    if (SystemType == ADEC_SIF_DK_NICAM) {
        if ((uc2D46&0x0f) == 0x05)
            *exist  = ADEC_SIF_PRESENT;
    }
    else if (SystemType == ADEC_SIF_DK1_A2) {
        if (((uc2D44&0xFB) == 0x3B) || ((uc2D44&0xFD) == 0x3D)) /* 09.23 chnageed by mstar*/
            *exist  = ADEC_SIF_PRESENT;
    }
    else if (SystemType == ADEC_SIF_DK2_A2) {
        if (((uc2D44&0xFB) == 0x7B) || ((uc2D44&0xFD) == 0x7D))
            *exist  = ADEC_SIF_PRESENT;
    }
    else if (SystemType == ADEC_SIF_DK3_A2) {
        if (((uc2D44&0xFB) == 0xBB) || ((uc2D44&0xFD) == 0xBD))// update the value for DK3 carrier report 090316
            *exist  = ADEC_SIF_PRESENT;
    }
    else if (SystemType == ADEC_SIF_DK_FM) {
        if(uc2D44&0x10)
            *exist  = ADEC_SIF_PRESENT;
    }
#else
	if (SystemType == ADEC_SIF_DK3_A2) {
        if (((uc2D44&0xFB) == 0xBB) || ((uc2D44&0xFD) == 0xBD))// update the value for DK3 carrier report 090316
            *exist  = ADEC_SIF_PRESENT;
	}
#endif

    // fix me: check if standard is avaliable and return to *availability
    return(TRUE);
}



void MDrv_MAD_CheckNicamDigital(U8 *isNicamDetect)
{
    U8 uc2D46=0; // register buffer

    *isNicamDetect = ADEC_SIF_ABSENT;

    uc2D46 = MHal_MAD_SeReadRegByte(AU_STATUS_MODE2);//CHECK
    if ((uc2D46&0x0f) == 0x05)
        *isNicamDetect  = ADEC_SIF_PRESENT;
}


BOOL MDrv_MAD_SIF_GetSoundStandard(U8 *B_SifStandard)
{
    U8 sifStandard;
    U8 uc2D44=0, uc2D46=0; // register buffer
    U8 curr_sound_sys, ucCurStatus = 0x00; // current status buffer

    MHal_MAD_SeWriteReg(ADEC_SIF_CMD_STANDARD, 0x00);
    MAD_Delayms(1);
    MHal_MAD_SeWriteRegMask(ADEC_SIF_CMD_STANDARD, 0xE0, 0xE0);
    MAD_Delayms(800);
    if( 0x80 == ( sifStandard = MHal_MAD_SeReadReg(ADEC_SIF_STATUS_STANDARD)) )
    {
        //MAD_DEBUG_P2(printk("Audio DSP Busy, %x\n", sifStandard));
        return FALSE;
    }
    //MAD_DEBUG_P2(printk("mode standard: %x\n", sifStandard));

    uc2D44 = MHal_MAD_SeReadReg(AU_STATUS_MODE1);//CHECK
    uc2D46 = MHal_MAD_SeReadReg(AU_STATUS_MODE2);//CHECK
    curr_sound_sys = sifStandard & 0xFF; //MAdp_MAD_ReadReg(0x2D20)&0x0F;
    if (_bit4_(uc2D44))
        ucCurStatus |= IS_MAIN_CARRIER;

    if (curr_sound_sys != AU_SYS_I_NICAM){
        if(_bit5_(uc2D44) )
        ucCurStatus |= IS_A2;
    }

    if ((uc2D46&0x0f) == 0x05)
        ucCurStatus |= IS_NICAM;

    else if (curr_sound_sys == 0x03){
        sifStandard = ADEC_SIF_MN_A2;   // palsum only support M_A2, no BTSC.
    }

    if (curr_sound_sys == 0x04) {
        if (ucCurStatus & IS_A2)
            sifStandard = ADEC_SIF_BG_A2;
        else sifStandard = ADEC_SIF_BG_FM;
    }
    else if (curr_sound_sys == 0x08)
        sifStandard = ADEC_SIF_BG_NICAM;
    else if (curr_sound_sys == 0x09)
        sifStandard = ADEC_SIF_DK_NICAM;
    else if (curr_sound_sys == 0x05) {
        if (ucCurStatus & IS_A2)
            sifStandard = ADEC_SIF_DK1_A2;
        else sifStandard = ADEC_SIF_DK_FM;
    }
    else if (curr_sound_sys == 0x06){
        sifStandard = ADEC_SIF_DK2_A2;
    }
    else if (curr_sound_sys == 0x07){
        sifStandard = ADEC_SIF_DK3_A2;
    }
    else if (curr_sound_sys == 0x0A){
        if (ucCurStatus & IS_NICAM)
            sifStandard = ADEC_SIF_I_NICAM;
        else sifStandard = ADEC_SIF_I_FM;
    }

    *B_SifStandard = sifStandard;
    return( TRUE ) ;

}

BOOL MDrv_MAD_SIF_RedoSetStandard()
{
    U8   ori_val;

    ori_val = MHal_MAD_SeReadRegByte(ADEC_SIF_CMD_STANDARD);
    MHal_MAD_SeWriteRegByte(ADEC_SIF_CMD_STANDARD, 0);                             // clear cmd
    MAD_Delayms(1);
    MHal_MAD_SeWriteRegByte(ADEC_SIF_CMD_STANDARD, ori_val);
    MAD_Delayms(1);
    return TRUE;
}


//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_AuSifInit
//  [Description]:
//      This routine is the initialization for SIF module.
//  [Arguments]:
//      None
//  [Return]:
//      None
//
//*******************************************************************************
void MDrv_MAD_AuSifInit(void)
{
    U8 u8codeTypeDSP = MDrv_MAD_SIF_GetDspCodeType();

    MDrv_MAD_AuSif_setMemInfo();
    MDrv_MAD_SIF_LoadCode(u8codeTypeDSP);

    //fr_threshold_on = NICAM_ENHANCE_ON_THRESHOLD;
    //fr_threshold_off = NICAM_ENHANCE_OFF_THRESHOLD;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SIF_Init()
/// @brief \b Function \b Description: This routine is the initialization for SIF
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SIF_Init(void)
{//CHECK
    //disable SIF Channel
    //HAL_AUDIO_WriteMaskByte(0x2CA4, 0x08, 0x00);
    //HAL_AUDIO_WriteByte(0x2D20, 0x04);
}

void MDrv_MAD_SIF_TriggerSifPLL(void)
{
  // Recommand by audio designer
     // set the initial sequence provided by billy, to be verified.
#if VIF_Mode
    if(Chip_Query_Rev() == 0)//U01
    MHal_MAD_AbsWriteMaskByte(0x100B42, 0xFF, 0x00);
  //else//non U01
	//MHal_MAD_AbsWriteMaskByte(0x100B42, 0xFF, 0x0D);
    MHal_MAD_AbsWriteMaskByte(0x100B43, 0xFF, 0x00);

    MHal_MAD_AbsWriteMaskByte(0x103314, 0xFF, 0x00);
    MHal_MAD_AbsWriteMaskByte(0x103315, 0xFF, 0x00);

    MHal_MAD_AbsWriteMaskByte(0x00346A, 0xFF, 0x04);
    MHal_MAD_AbsWriteMaskByte(0x00346B, 0xFF, 0x16);
    msleep(1);

    MHal_MAD_AbsWriteMaskByte(0x00346A, 0xFF, 0x04);
    MHal_MAD_AbsWriteMaskByte(0x00346B, 0xFF, 0x06);

    MHal_MAD_AbsWriteMaskByte(0x003466, 0xFF, 0x02);
    MHal_MAD_AbsWriteMaskByte(0x003467, 0xFF, 0x09);

    MHal_MAD_AbsWriteMaskByte(0x003460, 0xFF, 0x00);
    MHal_MAD_AbsWriteMaskByte(0x003461, 0xFF, 0x10);

    MHal_MAD_AbsWriteMaskByte(0x003402, 0xFF, 0x20);
    MHal_MAD_AbsWriteMaskByte(0x003403, 0xFF, 0x02);

    MHal_MAD_AbsWriteMaskByte(0x00341E, 0xFF, 0x80);
    MHal_MAD_AbsWriteMaskByte(0x00341F, 0xFF, 0x00);

    MHal_MAD_AbsWriteMaskByte(0x003418, 0xFF, 0x02);
    MHal_MAD_AbsWriteMaskByte(0x003419, 0xFF, 0x00);

    MHal_MAD_AbsWriteMaskByte(0x112C02, 0x04, 0x04);

    MHal_MAD_AbsWriteMaskByte(0x112CC8, 0xFF, 0xE0);

    MHal_MAD_AbsWriteMaskByte(0x112CC9, 0xFF, 0x30); // Defaule enable VIF
  #else
    if(Chip_Query_Rev() == 0)//U01
    MHal_MAD_AbsWriteMaskByte(0x100B42, 0xFF, 0x00);
  //else//non U01
	//MHal_MAD_AbsWriteMaskByte(0x100B42, 0xFF, 0x0D);
    MHal_MAD_AbsWriteMaskByte(0x100B43, 0xFF, 0x00);

    MHal_MAD_AbsWriteMaskByte(0x103314, 0xFF, 0x00);
    MHal_MAD_AbsWriteMaskByte(0x103315, 0xFF, 0x00);
/*
    MHal_MAD_AbsWriteMaskByte(0x00346A, 0xFF, 0x04);
    MHal_MAD_AbsWriteMaskByte(0x00346B, 0xFF, 0x16);
    msleep(1);

    MHal_MAD_AbsWriteMaskByte(0x00346A, 0xFF, 0x04);
    MHal_MAD_AbsWriteMaskByte(0x00346B, 0xFF, 0x06);

    MHal_MAD_AbsWriteMaskByte(0x003466, 0xFF, 0x02);
    MHal_MAD_AbsWriteMaskByte(0x003467, 0xFF, 0x09);

    MHal_MAD_AbsWriteMaskByte(0x003460, 0xFF, 0x00);
    MHal_MAD_AbsWriteMaskByte(0x003461, 0xFF, 0x10);

    MHal_MAD_AbsWriteMaskByte(0x003402, 0xFF, 0x40);  //
    MHal_MAD_AbsWriteMaskByte(0x003403, 0xFF, 0x04);  // not the same with RD side

    MHal_MAD_AbsWriteMaskByte(0x00341E, 0xFF, 0x80);
    MHal_MAD_AbsWriteMaskByte(0x00341F, 0xFF, 0x00);

    MHal_MAD_AbsWriteMaskByte(0x003418, 0xFF, 0x00); // not the same with RD side
    MHal_MAD_AbsWriteMaskByte(0x003419, 0xFF, 0x00);
*/
    MHal_MAD_AbsWriteMaskByte(0x112C02, 0x04, 0x04);

    MHal_MAD_AbsWriteMaskByte(0x112CC8, 0xFF, 0xE0);

    MHal_MAD_AbsWriteMaskByte(0x112CC9, 0xFF, 0x30); // Defaule enable VIF
    MHal_MAD_AbsWriteMaskByte(0x112CC9, 0xFF, 0x20); // LGE used the Non-VIF

    MHal_MAD_AbsWriteMaskByte(0x003460, 0xFF, 0x00); // it need to check that other IP setted this register.
    MHal_MAD_AbsWriteMaskByte(0x003461, 0xFF, 0x10);

  #endif
}
/* change for the Korea A2 threhsold function.
U16 code RF2NSR_map[32] =  defult 11

{
    0x1db8,0x1cbb,0x1baa,0x19db,0x18d8,0x17ea,0x16e4,0x1491,0x1300,0x111a,      //
    0x0fb9,0x0d00,0x0cd6,0x0ba0,0x0a77,0x0964,0x0887,0x07a0,0x06d8,0x0635,      //
    0x058b,0x050c,0x0492,0x0420,0x03c0,0x037e,0x0320,0x02e6,0x02bd,0x028d,      //
    0x0278,0x0252,                                                              //
};
*/

U16 code RF2NSR_map[40] = /* defult 11  modified for 40 steps*/
{
    0x1db8,0x1cbb,0x1baa,0x19db,0x18d8,0x17ea,0x16e4,0x1491,0x1300,0x111a,      //
    0x0fb9,0x0d00,0x0cd6,0x0ba0,0x0a77,0x0964,0x0887,0x07a0,0x06d8,0x0635,      //
    0x058b,0x050c,0x0492,0x0420,0x03c0,0x037e,0x0320,0x02e6,0x02bd,0x028d,      //
    0x0278,0x0252,0x0200,0x01b0,0x0180,0x0140,0x0100,0x00b0,0x0080,0x0040,                                                              //
};


U16 code RFPilotAmp_Map[]=
{
    0x0010, 0x0020, 0x0038, 0x0071, 0x0097, 0x00a5, 0x00b5, 0x00d5, 0x00e5, 0x0110,
    0x0400, 0x0800, 0x0a00, 0x0d00, 0x0f00, 0x1000, 0x1100, 0x12E0, 0x1770, 0x1A30,
    0x1B00, 0x1b19, 0x1beb, 0x1bf4, 0x1c0e, 0x1c5a, 0x1c7c, 0x1cb3, 0x1d09, 0x1d32,
    0x1d44, 0x1d52, 0x1d79, 0x1d9f, 0x1da6, 0x1dae, 0x1db5, 0x1dc5, 0x1da7, 0x1dd2,
};
U16 code RF2PilotPhase_map[40] =
{
    0x1db8,0x1cbb,0x1baa,0x19db,0x18d8,0x17ea,0x16e4,0x1491,0x12bf,0x111a,      // 0~9      20 ~ 29dB
    0x0fb9,0x0e45,0x0cd6,0x0ba0,0x0a77,0x0964,0x0887,0x07a0,0x06d8,0x0635,      // 10~ 19   30 ~ 39dB
    0x058b,0x050c,0x0492,0x0420,0x03c0,0x037e,0x0320,0x02e6,0x02bd,0x028d,      // 20~ 29   40 ~ 49dB
    0x0278,0x0252,                                                              // 30~ 31   50 ~ 51dB
};

U16 code BTSC_Pilot_map[32] =
{

    0x3e00,0x3c00,0x3a00,0x3800,0x3600,0x3400,0x3200,0x3000,0x2E00,0x2C00,
    0x2A00,0x2800,0x2600,0x2400,0x2200,0x2000,0x1e00,0x1d00,0x1b00,0x1900,// defaul 15
    0x1700,0x1500,0x1300,0x1100,0x0f00,0x0d00,0x0b00,0x0900,0x0700,0x0500,
    0x0300,0x0100,
};


#define Thr_C2ONOFF_Gap             3
#define Thr_PilotONOFF_Gap          7
#define Thr_BTSC_PilotONOFF_Gap     6 // add by allanliang
BOOL MDrv_MAD_SetBtscA2ThresholdLevel(U16 thresholdLevel)
{
    U8 u8codeTypeDSP = MDrv_MAD_SIF_GetDspCodeType();

    MAD_DEBUG_P1(printk("\r\n------ MDrv_MAD_SetBtscA2ThresholdLevel is------%x",thresholdLevel));
 // change for the Korea A2 threhsold function.
    if (u8codeTypeDSP == AU_SIF_PALSUM) {
        MDrv_MAD_SIF_AccessThreshold(WRITE_THRESHOLD|A2_BG_STANDARD, A2_PILOT_ON_AMP, (U16*)(&RF2NSR_map[thresholdLevel]));
        MDrv_MAD_SIF_AccessThreshold(WRITE_THRESHOLD|A2_DK_STANDARD, A2_PILOT_ON_AMP, (U16*)(&thresholdLevel));
        MDrv_MAD_SIF_AccessThreshold(WRITE_THRESHOLD|A2_M_STANDARD,   CARRIER2_ON_NSR, (U16*)(&RF2NSR_map[(thresholdLevel+Thr_C2ONOFF_Gap)>39?39:(thresholdLevel+Thr_C2ONOFF_Gap)]));// modified for 40 step //CHECK
        MDrv_MAD_SIF_AccessThreshold(WRITE_THRESHOLD|A2_M_STANDARD,   CARRIER2_OFF_NSR, (U16*)(&RF2NSR_map[thresholdLevel]));//CHECK
        MDrv_MAD_SIF_RedoSetStandard();
        return (TRUE);
    }
    else if (u8codeTypeDSP == AU_SIF_BTSC) {
        MDrv_MAD_SIF_AccessThreshold(WRITE_THRESHOLD,   BTSC_PILOT_AMPLITUDE_ON, (U16*)(&BTSC_Pilot_map[thresholdLevel]));//CHECK
    	MDrv_MAD_SIF_AccessThreshold(WRITE_THRESHOLD,   BTSC_PILOT_AMPLITUDE_OFF, (U16*)(&BTSC_Pilot_map[(thresholdLevel+Thr_BTSC_PilotONOFF_Gap)>31?31:(thresholdLevel+Thr_BTSC_PilotONOFF_Gap)]));//CHECK
    	return (TRUE);
    }
    return (FALSE);
}


BOOL MDrv_MAD_SIF_AccessThreshold(AUDIO_SIF_SYSTEM_TYPE rw_standard_type, AUDIO_SIF_THRESHOLD_TYPE  u8Threshold_type, MS_U16 *u16Value)
{
    MS_BOOL  writethd;
    MS_U16 standard_type;
    MS_U8   SIF_DSP_type;
    writethd = HINIBBLE(rw_standard_type) ;
    standard_type = LONIBBLE(rw_standard_type) ;
    SIF_DSP_type=MDrv_MAD_SIF_GetDspCodeType(); // 20090828 To get the correct SIF DSP type
#if 0//CHECK
    if (((MDrv_MAD_SIF_GetDspCodeType()) == AU_SIF_PALSUM) &&( standard_type >= BTSC_STANDARD))
        return FALSE;
    else if(((MDrv_MAD_SIF_GetDspCodeType()) == AU_SIF_BTSC) &&( standard_type != BTSC_STANDARD))
        return FALSE;
#endif

if (SIF_DSP_type==AU_SIF_BTSC)
{
    if (writethd)
    {
          MDrv_MAD2_Write_DSP_sram((WORD)au_ntsc_sys_threshold[standard_type][u8Threshold_type],((MS_U32) *u16Value)<<8, DSP_MEM_TYPE_PM);
        return TRUE;
    }
    else
    {
        return (MDrv_MAD2_Read_DSP_sram((WORD)au_ntsc_sys_threshold[standard_type][u8Threshold_type],DSP_MEM_TYPE_PM));
    }

}
else
{
    if (writethd)
    {
          MDrv_MAD2_Write_DSP_sram((WORD)au_pal_sys_threshold[standard_type][u8Threshold_type],((MS_U32) *u16Value)<<8, DSP_MEM_TYPE_PM);
        return TRUE;
    }
    else
    {
        return (MDrv_MAD2_Read_DSP_sram((WORD)au_pal_sys_threshold[standard_type][u8Threshold_type],DSP_MEM_TYPE_PM));
    }

}


}

//TBD
BOOL MDrv_MAD_SIF_WritePARAMETER(WORD dsp_addr, DWORD value)
{
/*
    U8  dat[3];
    //MHal_MAD_WriteMaskReg(0x3CEC, 0x40, 0x00);          //DMA sel to RIU
    MHal_MAD_AbsWriteMaskByte(0x103C14, 0x01, 0x00);

    dat[2] = H2BYTE(value);
    dat[1] = HIBYTE(value);
    dat[0] = LOBYTE(value);
    MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x00);//need to choose DSP for DEC/SE once use IDMA/BDMA
    MHal_MAD_DecWriteReg( REG_DEC_IDMA_WRBASE_ADDR_L, dsp_addr);
    //MAD_Delayms(2);//20090205 mstar remove
    MHal_MAD_DecWriteRegMask(REG_DEC_IDMA_CTRL0, 0x0004,0x0000);

    MHal_MAD_DecWriteReg(REG_DEC_DSP_BRG_DATA_L, dat[1] | (dat[2] <<8));
    if (MDrv_MAD_DecDSP_chkIdmaReady(AUD_CHK_DSP_WRITE_RDY)==FALSE) {
        printk("\r\n chkIdamReady error 1\r\n");
        return FALSE;
    }

    MHal_MAD_DecWriteReg(REG_DEC_DSP_BRG_DATA_L, dat[0]);
    if (MDrv_MAD_DecDSP_chkIdmaReady(AUD_CHK_DSP_WRITE_RDY)==FALSE) {
        printk("\r\n chkIdamReady error 2\r\n");
        return FALSE;
    }
*/
    return TRUE;
}

//TBD
BOOL MDrv_MAD_SIF_ReadPARAMETER(WORD dsp_addr, DWORD *value)
{
/*
    BYTE  dat[3];
    *value = 0x0;

    //printk("\r\n MDrv_AuSifReadPARAMETER is add=%x",dsp_addr);

    MHal_MAD_DecWriteReg( REG_DEC_IDMA_RDBASE_ADDR_L, dsp_addr);
    MHal_MAD_DecWriteRegMask(REG_DEC_IDMA_CTRL0, 0x08, 0x08);

    dat[2] = MHal_MAD_ReadByte(0x2D0D);
    dat[1] = MHal_MAD_ReadByte(REG_DEC_IDMA_RDDATA_H_0);
    dat[0] = MHal_MAD_ReadByte(REG_DEC_IDMA_RDDATA_L);
    //printk("\r\n dat0 is%x",dat[0]);
    //printk("\r\n dat1 is%x",dat[1]);
    //printk("\r\n dat2 is%x",dat[2]);

    *value = (((DWORD)dat[2])<<16)  |  (((DWORD)dat[1])<<8)  | dat[0];
//    printk("\r\n value is %x",(DWORD)value);
*/
    return TRUE;
}

//TBD
//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_AuSifSetPrescale
//  [Description]:
//      This routine is to set the SIF Prescale.
//  [Arguments]:
//      type    : Type of SIF (A2/NICAM/AM)
//      value   : The prescale value, unit is 0.25dB.
//                   0: 0db, 1:  0.25dB,  2:     0.5dB, ...,   4: 1.0dB, ...,   8:  2dB
//                     -1:-0.25dB, -2:-0.5dB, ..., -4:-1.0dB, ..., -8:-2dB
//  [Return]:
//      None
//
//*******************************************************************************
BOOL MDrv_MAD_SIF_SetPrescale(U8 type, S32 db_value)
{
    BYTE i, i_min, i_max;
    int  normalized_db;
    WORD sif_shift, sif_gain, shift_value;
    DWORD gain;
    U8 u8codeTypeDSP = MDrv_MAD_SIF_GetDspCodeType();

    if (db_value == 99)
    {
        if (u8codeTypeDSP == AU_SIF_PALSUM) {
            if (MDrv_MAD_SIF_WritePARAMETER(PAL_SIF_PM_GAIN_TBL[type][0],0) == FALSE)
                return FALSE;
            if (MDrv_MAD_SIF_WritePARAMETER(PAL_SIF_PM_GAIN_TBL[type][1],0) == FALSE)
                return FALSE;
        }
        else if (u8codeTypeDSP == AU_SIF_BTSC) {
            if (MDrv_MAD_SIF_WritePARAMETER(BTSC_SIF_PM_GAIN_TBL[type][0],0) == FALSE)
                return FALSE;
            if (MDrv_MAD_SIF_WritePARAMETER(BTSC_SIF_PM_GAIN_TBL[type][1],0) == FALSE)
                return FALSE;
        }
    }
    sif_gain = sif_gain_0[type];
    sif_shift = sif_shift_0[type];
    MAD_DEBUG_P2(printk("\r\n dB_value *4 is %d  \r\n ",(WORD)db_value));
    MAD_DEBUG_P2(printk("\r\n sif_gain is %x",(WORD)sif_gain));
    MAD_DEBUG_P2(printk("\r\n sif_shift is %x",(WORD)sif_shift));

    //read user DB setting
    i_min = 8/PRESCALE_STEP_ONE_DB-1;
    i_max = sizeof(PRESCALE_STEP_TBL)/sizeof(WORD);
    //caculate new gain & shift
    if(db_value>(18*PRESCALE_STEP_ONE_DB))
    {
        shift_value = 5;// 3;
        db_value = 0;
    }
    else if (db_value > (12*PRESCALE_STEP_ONE_DB))
    {
        shift_value = 5;// 3;
        db_value = db_value - (18*PRESCALE_STEP_ONE_DB);
    }
    else if (db_value > (6*PRESCALE_STEP_ONE_DB))
    {
        shift_value = 4;// 2;
        db_value = db_value - (12*PRESCALE_STEP_ONE_DB);
    }
    else if (db_value > 0 )
    {
        shift_value = 3;// 1;
        db_value = db_value - (6*PRESCALE_STEP_ONE_DB);
    }
    else if (db_value > (-6*PRESCALE_STEP_ONE_DB) )
    {
        shift_value = 2;//0;
    }
    else if (db_value > (-12*PRESCALE_STEP_ONE_DB) )
    {
        shift_value = 1;//-1;
        db_value = db_value + (6*PRESCALE_STEP_ONE_DB);
    }
    else
    {
        shift_value = 0;//-2;
        db_value = 0;
    }
    gain = 0x7FFF;
    normalized_db = -db_value;

    for(i=i_min;i<i_max;i++)
    {
        if(normalized_db & (1<<(i-i_min)))
        {
            gain = ((DWORD)PRESCALE_STEP_TBL[i])*((DWORD)gain);
            gain = gain>>15;
        }
    }
    // check for
    while ((sif_shift+shift_value) <2)
    {
        gain = (0x4000*gain);
        gain = gain>>15;
        shift_value++;
    }

    gain = (gain*((DWORD)sif_gain))>>15;
    sif_gain = (WORD) gain;
    sif_shift = sif_shift+shift_value-2;
    //set new gain & shift to PM
    if (u8codeTypeDSP == AU_SIF_PALSUM) {
        if (MDrv_MAD_SIF_WritePARAMETER(PAL_SIF_PM_GAIN_TBL[type][0],(((DWORD)sif_gain)<<8)) == FALSE)
            return FALSE;
        if (MDrv_MAD_SIF_WritePARAMETER(PAL_SIF_PM_GAIN_TBL[type][1],(DWORD)sif_shift) == FALSE)
            return FALSE;
    }
    else if (u8codeTypeDSP == AU_SIF_BTSC) {
        if (MDrv_MAD_SIF_WritePARAMETER(BTSC_SIF_PM_GAIN_TBL[type][0],(((DWORD)sif_gain)<<8)) == FALSE)
            return FALSE;
        if (MDrv_MAD_SIF_WritePARAMETER(BTSC_SIF_PM_GAIN_TBL[type][1],(DWORD)sif_shift) == FALSE)
            return FALSE;
    }
    return TRUE;
    MAD_DEBUG_P2(printk("\r\n --sif_gain is%x \r\n",(WORD) sif_gain));
    MAD_DEBUG_P2(printk("\r\n --sif_shift is%x \r\n",(WORD) sif_shift));
}
void MDrv_MAD_SetThresholdType(THR_TBL_TYPE code *ThrTbl, BYTE num,  BYTE standardtype)
{
	BYTE i;
    U16 value;
	for(i=0;i< num;i++)
	{
	value =( (0x00FF & ThrTbl->HiByteValue) <<8)|(0x00FF & ThrTbl->LowByteValue);
	MDrv_MAD_SIF_AccessThreshold(WRITE_THRESHOLD|standardtype,ThrTbl->Thrtype,&value);//CHECK
	ThrTbl++;
	}
}
void MDrv_MAD_WriteSIFThresholdTbl_PAL(void)
{

	MDrv_MAD_SetThresholdType(AuSifInitThreshold_A2_M,  sizeof(AuSifInitThreshold_A2_M)/sizeof(THR_TBL_TYPE), A2_M_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_A2_BG, sizeof(AuSifInitThreshold_A2_BG)/sizeof(THR_TBL_TYPE),A2_BG_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_A2_DK, sizeof(AuSifInitThreshold_A2_DK)/sizeof(THR_TBL_TYPE),A2_DK_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_FM_I, sizeof(AuSifInitThreshold_FM_I)/sizeof(THR_TBL_TYPE),A2_I_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_AM, sizeof(AuSifInitThreshold_AM)/sizeof(THR_TBL_TYPE),AM_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_Nicam_BG, sizeof(AuSifInitThreshold_Nicam_BG)/sizeof(THR_TBL_TYPE),NICAM_BG_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_Nicam_I,sizeof(AuSifInitThreshold_Nicam_I)/sizeof(THR_TBL_TYPE), NICAM_I_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_HIDEV_M, sizeof(AuSifInitThreshold_HIDEV_M)/sizeof(THR_TBL_TYPE),HIDEV_M_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_HIDEV_BG,sizeof(AuSifInitThreshold_HIDEV_BG)/sizeof(THR_TBL_TYPE), HIDEV_BG_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_HIDEV_DK, sizeof(AuSifInitThreshold_HIDEV_DK)/sizeof(THR_TBL_TYPE),HIDEV_DK_STANDARD);
    MDrv_MAD_SetThresholdType(AuSifInitThreshold_HIDEV_I, sizeof(AuSifInitThreshold_HIDEV_I)/sizeof(THR_TBL_TYPE),HIDEV_I_STANDARD);
	MDrv_MAD_SIF_RedoSetStandard();
}
#if 1 //08.10.25 jaehoon BTSC no sound 임시 대책 // sebeom change 10/27
void MDrv_MAD_WriteSIFThresholdTbl_BTSC(void)
{
 	MDrv_MAD_SetThresholdType(AuSifInitThreshold_BTSC,  sizeof(AuSifInitThreshold_BTSC)/sizeof(THR_TBL_TYPE), BTSC_STANDARD);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SIF_GetDspType()
/// @brief \b Function \b Description:  This function is used to get the DSP(DSP_DEC or DSP_SE) which SIF module used.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b MS_U8   : DSP which SIF modules (DSP_DEC or DSP_SE)
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD_SIF_GetDspType(void)
{
    return g_u8SifDspType;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SIF_SetDspCodeType()
/// @brief \b Function \b Description:  This function is used to set the DSP code type SIF module used.
/// @param <IN>        \b MS_U8: SIF DSP code type.
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SIF_SetDspCodeType(MS_U8 u8Type)
{
    g_u8Dsp2CodeType=u8Type;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SIF_GetDspCodeType()
/// @brief \b Function \b Description:  This function is used to set the DSP code type SIF module used.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b MS_U8: SIF DSP code type.
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD_SIF_GetDspCodeType(void)
{
    return (g_u8Dsp2CodeType & (~SIF_ADC_FROM_VIF_PATH));
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SIF_GetDspCodeVifFlag()
/// @brief \b Function \b Description:  This function is used to get the vif flag of the DSP code SIF module used.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b MS_BOOL: TRUE or FALSE.
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_SIF_GetDspCodeVifFlag(void)
{

    if (g_u8Dsp2CodeType & SIF_ADC_FROM_VIF_PATH) return TRUE;
    else return FALSE;
}


void MDrv_MAD_SIF_ENABLE_CHANNEL(MS_BOOL bEnable)
{
    bEnable=bEnable;
    /* T2 setting !!
   if (bEnable == 0)
        MHal_MAD_WriteMaskByte(0x2CA4, 0x08, 0x00);  // disable
    else
        MHal_MAD_WriteMaskByte(0x2CA4, 0x08, 0x08);  // enable
   */
}

///////////////////////////////////////////////////////////////////////////////////
//Volume control.
//Gain setting = 0db _ VOL*L0 dB (0db ~ -114db)
//VOL = 0 ~ 11 (+12~+1db)
//VOL = 12 (0db)
//VOL = 13 ~ 126 (-1 ~ -114db)
//VOL = 127, mute
///////////////////////////////////////////////////////////////////////////////////
//REG_2D2E[6:0]: beeper tone frequency (maximum 32 step(4KHz))
//0x01: 125Hz
//0x02: 250Hz
//...
//0x08: 1kHz
//...
//0x11: 2kHz
//...
//0x19: 3kHz
//...
//0x21: 4kHz
///////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_SIF_BeeperToneSetting(MS_U8 tone,MS_U8 beeper_volume)
{
    tone=tone;
    beeper_volume=beeper_volume;

/*
    if (tone > 0x21)
    {
        DBG_AUDIO_ERROR(printk("\r\n Tone frequency out of range [%x]>0x21!!\r\n",tone));
        return FALSE;
    }
    MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT0_VOL_FRAC, 0xFF, beeper_volume);
    MHal_MAD_WriteMaskByte(0x2DCE, 0x7F, tone);
    MAD_DEBUG_P1(printk("\r\n Tone [%x]!!\r\n",tone));
*/
    return TRUE;
}

//@@Need_Modify
MS_BOOL MDrv_MAD_SIF_BeeperEnable(MS_U8 beeper_enable)
{
    beeper_enable=beeper_enable;

/*
    if (beeper_enable)
        MHal_MAD_WriteMaskByte(0x2DCE, 0x80, 0x80);
    else
        MHal_MAD_WriteMaskByte(0x2DCE, 0x80, 0x00);

    MAD_DEBUG_P1(printk("\r\n beeper [%x]!!\r\n",beeper_enable));
*/
    return TRUE;
}

MS_BOOL MDrv_MAD_SIF_AskADCFromVifPathSupported(void)
{
    return bADCFromVifPathSupported;
}

MS_BOOL MDrv_MAD_SIF_SetADCFromVifPath(MS_BOOL bEnableVifPath)
{
    if (bADCFromVifPathSupported == TRUE) {
        if (bEnableVifPath == TRUE) {
            if (bADCFromVifPathEnabled == FALSE) {
                MHal_MAD_AbsWriteMaskByte(0x112CC9, 0x10, 0x10); //  enable VIF
                bADCFromVifPathEnabled = TRUE;
            }
        }
        else {
            if (bADCFromVifPathEnabled == TRUE) {
                MHal_MAD_AbsWriteMaskByte(0x112CC9, 0x10, 0x00); //  disalbe VIF
                bADCFromVifPathEnabled = FALSE;
            }
        }
        return TRUE;
    }
    else return FALSE;
}

MS_BOOL MDrv_MAD_SIF_GetADCFromVifPathFlag(void)
{
    return bADCFromVifPathEnabled;
}

MS_BOOL MDrv_MAD_SIF_WriteCommand(AUDIO_SIF_CMD_STATUS CmdType, MS_U8 value)
{
    switch (CmdType){
        case AU_SIF_WRITE_SUB_CARRIER_STD_SET:
            MHal_MAD_SeWriteRegByte(AU_CMD_STANDARD, value);
            break;
        case AU_SIF_WRITE_CARRIER_DEBOUNCE:
            MHal_MAD_SeWriteRegMaskByte(AU_CMD_PFIRBANDWIDTH, 0x80, value);
            break;
        case AU_SIF_WRITE_STD_SET:
            MHal_MAD_SeWriteRegMaskByte(AU_CMD_STANDARD,0x0f,value);
            break;
        case AU_SIF_WRITE_SUB_CARRIER_SET:
            MHal_MAD_SeWriteRegMaskByte(AU_CMD_STANDARD, 0xf0,value);
            break;
        case AU_SIF_WRITE_STD_HIDEV_SET:
            MHal_MAD_SeWriteRegMaskByte(AU_CMD_STANDARD, 0x10,value);
            break;
        case AU_SIF_WRITE_PFIRBANDWIDTH:
            MHal_MAD_SeWriteRegMaskByte(AU_CMD_PFIRBANDWIDTH, 0x30, value);
            break;
        case AU_SIF_WRITE_ENABLE_SIF_SYNTHESIZER:
            MHal_MAD_SeWriteRegMaskByte(REG_AUDIO_INPUT_CFG , 0x04, value);
            break;
        case AU_SIF_WRITE_DISABLE_SIF_SYNTHESIZER:
            MHal_MAD_SeWriteRegMaskByte(REG_AUDIO_INPUT_CFG , 0x44, value);
            break;
        default:
            return (FALSE);
            break;
    }
    return (TRUE);
}

void MDrv_MAD_SIF_ResetGetGain0(void)
{
      GetGain0_Done = 0;
}

MS_BOOL MDrv_MAD_SIF_GetOrginalGain(void)
{
    MS_U8  i;
    if (GetGain0_Done == 0)
    {
        if ((MDrv_MAD_SIF_GetDspCodeType())== AU_SIF_BTSC)
       {
            for(i = 0; i< sizeof(SIF_PM_GAIN_TBL_BTSC)/(sizeof(MS_U16)*2); i++)
            {
 //              sif_gain_0[i] = (MDrv_MAD2_Read_DSP_sram (SIF_PM_GAIN_TBL_BTSC[i][0],DSP_MEM_TYPE_PM) >> 8);
 //               sif_shift_0[i] = MDrv_MAD2_Read_DSP_sram (SIF_PM_GAIN_TBL_BTSC[i][1],DSP_MEM_TYPE_PM);
            }
            GetGain0_Done = 1;
        }
        else if ((MDrv_MAD_SIF_GetDspCodeType())== AU_SIF_PALSUM)
        {
            for(i = 0; i< sizeof(SIF_PM_GAIN_TBL_PAL)/(sizeof(MS_U16)*2); i++)
            {
//                sif_gain_0[i] = (MDrv_MAD2_Read_DSP_sram (SIF_PM_GAIN_TBL_PAL[i][0],DSP_MEM_TYPE_PM) >> 8);
//                sif_shift_0[i] = MDrv_MAD2_Read_DSP_sram (SIF_PM_GAIN_TBL_PAL[i][1],DSP_MEM_TYPE_PM);
            }
            GetGain0_Done = 1;
        }
        else if ((MDrv_MAD_SIF_GetDspCodeType())== AU_SIF_EIAJ)
        {
        }
        else
        {
            DBG_AUDIO_ERROR(printk("\r\n get original gain DSP type error!! \r\n"));
            return FALSE;
        }
    }

    return TRUE;

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name  : HAL_SELECT_SIF_FIFOMODE
/// @brief \b Function \b Description : This routine to select SIF FIFO Mode
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SELECT_SIF_FIFOMODE(MS_BOOL bSelect)
{
    bSelect=bSelect;
    /* T2 setting !!
    if (bSelect == 0)
        HAL_AUDIO_WriteMaskByte(0x2CCC, 0x40, 0x00);  // 2-stage FIFO
    else
        HAL_AUDIO_WriteMaskByte(0x2CCC, 0x40, 0x40);  // 4-stage FIFO
     */
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_DisEn_MIUREQ()
/// @brief \b Function \b Description: This routine is to reset DVB1 MIU request command.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_DisEn_MIUREQ(void)
{
    // Disable MIU Request
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0040 );
    //msleep(1);
    udelay(1);//loadcheck
    // Enable MIU Request
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0000 );
}

#define DemodCmdWrReg    0x01    // write register.
#define DemodCmdRdReg    0x02    // read register.
#define DemodCmd         0x110500L
#define DemodAdrL        0x110501L
#define DemodAdrH        0x110502L
#define DemodData        0x110503L

void MDrv_T3VIF_WriteByte( U32 u32Reg, U8 u8Val)

{
    if (VDMCU_DSP_VIF != MDrv_SYS_VDmcuGetType())
    {
        MAD_DEBUG_P2(printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType()));
        return;
    }

   //  U32 u32Timeout=5000;
   // while (MHal_MAD_ReadByte(DemodCmd) && u32Timeout ) u32Timeout--; // wait VDMCU ready
    MHal_MAD_AbsWriteMaskByte(DemodAdrL, 0xFF,u32Reg&0xFF);
    MHal_MAD_AbsWriteMaskByte(DemodAdrH, 0xFF,(u32Reg>>8)&0xFF);
    MHal_MAD_AbsWriteMaskByte(DemodData, 0xFF,u8Val);
    MHal_MAD_AbsWriteMaskByte(DemodCmd, 0xFF,DemodCmdWrReg);
    msleep(2);
}


void MDrv_SIF_ADC_Reset(void) // modify the value for ADC setting 090825
{
    MAD_DEBUG_P1(printk("====== MDrv_SIF_ADC_Reset ====== \r\n"));
    MHal_MAD_AbsWriteMaskByte(0x103314, 0xFF, 0x00); //2009 0917 ADC power on
    MDrv_T3VIF_WriteByte(0x003460, 0x00);
    MDrv_T3VIF_WriteByte(0x003461, 0x10);
    MDrv_T3VIF_WriteByte(0x003466, 0x02);
    MDrv_T3VIF_WriteByte(0x003467, 0x09);
    MDrv_T3VIF_WriteByte(0x003468, 0x00);
    MDrv_T3VIF_WriteByte(0x003469, 0x00);
    MDrv_T3VIF_WriteByte(0x00346A, 0x04);
    MDrv_T3VIF_WriteByte(0x00346B, 0x16);
    MDrv_T3VIF_WriteByte(0x00346A, 0x04);
    MDrv_T3VIF_WriteByte(0x00346B, 0x06);
    MDrv_T3VIF_WriteByte(0x003440, 0x00);
    MDrv_T3VIF_WriteByte(0x003441, 0x00);
    MDrv_T3VIF_WriteByte(0x003416, 0x05);
    MDrv_T3VIF_WriteByte(0x003417, 0x05);
    MDrv_T3VIF_WriteByte(0x003418, 0x00);
    MDrv_T3VIF_WriteByte(0x003419, 0x00);
    MDrv_T3VIF_WriteByte(0x003402, 0x20);
    MDrv_T3VIF_WriteByte(0x003403, 0x10);
}

void MDrv_MAD_Set_ADC_Threshold(MS_U8 u8Threshold)
{
    MHal_MAD_WriteByte(REG_SOUND_NR_THRESHOLD, u8Threshold);
}

MS_U32 MDrv_MAD_Get_PCM_Energy(void)
{
    return  MDrv_MAD2_Read_DSP_sram(PCM_energy_addr, DSP_MEM_TYPE_DM);
}

