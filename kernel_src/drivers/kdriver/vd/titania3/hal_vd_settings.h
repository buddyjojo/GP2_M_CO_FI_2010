#ifndef _HAL_VD_SETTINGS_H_
#define _HAL_VD_SETTINGS_H_


//------------------------------------------------------------------------------
//
//  Patch Option
//
//------------------------------------------------------------------------------

#define MST_VD_PATCH_07112001 0//Already fixed in Eris


#define FINE_TUNE_COMB_F2   1   //fine-tune for bug's life, for Eris-ATT, James.Lu, 20080327
#define FINE_TUNE_3D_COMB   1   //fine-tune 3D Comb, for Eris-ATT, James.Lu, 20080328
#define FINE_TUNE_FH_DOT    1   //fine-tune for Fh image quality, Eris-ATT, James.Lu, 20080328

#define FINE_TUNE_FSC_SHIFT 1   //fine-tune for Fsc shift cause color bar edge blur, James.Lu, 20080919

#define FINE_TUNE_AFEC_72   1   //fine-tune for adaptive Hsync tracking, 20081107

// shjang_091020
//#define AVD_COMB_3D_MID            1  //Merge from utopia CL:135135 20090721 Ethan

#define NEW_VD_MCU                      1  //20090826EL

//------------------------------------------------------------------------------
//
//  Local Defines
//
//------------------------------------------------------------------------------
#define VD_DSP_VER  362             // VD DSP version (abandoned, just info, and update when VD DSP changed)
#define TEST_VD_DSP 0               // 1: for TEST VD DSP, 0: for MSTAR VD DSP

#define TV_NTSC                 0	// added to remove warning(dreamer@lge.com)
#define TV_PAL                  1	// added to remove warning(dreamer@lge.com)
#define TV_CHINA                2	// added to remove warning(dreamer@lge.com)
#define TV_SYSTEM TV_PAL


//------------------------------------
//
//  AGC Configuration
//
//------------------------------------

// !!! These defines should be specified in the board head files.       !!!
// !!! Here are just for the default setting if they are not specified. !!!

// Select Gain Control Type (default value and should be defined in the board head file)
#if !defined(VD_GAIN_OF_RF_SEL)
    #define VD_GAIN_OF_RF_SEL           VD_USE_AUTO_GAIN
#endif
#if !defined(VD_GAIN_OF_AV_SEL)
    #define VD_GAIN_OF_AV_SEL           VD_USE_AUTO_GAIN//VD_USE_AUTO_GAIN  //20090630 Ethan
#endif

// Fix Gain value for RF (i.e. ATV) (default value and should be defined in the board head file)
#if !defined(VD_RF_COARSE_GAIN_USED)
/*
    #if(FRONTEND_IF_DEMODE_TYPE == MSTAR_VIF)
        #define VD_RF_COARSE_GAIN_USED     VD_AGC_COARSE_GAIN_X_2
    #else
        #define VD_RF_COARSE_GAIN_USED     VD_AGC_COARSE_GAIN_X_1
    #endif
*/
        #define VD_RF_COARSE_GAIN_USED     VD_AGC_COARSE_GAIN_X_1
#endif
#if !defined(VD_RF_FINE_GAIN_H_USED)
/*
    #if(FRONTEND_IF_DEMODE_TYPE == MSTAR_VIF)
        #define VD_RF_FINE_GAIN_H_USED     0xC0
    #else
        #define VD_RF_FINE_GAIN_H_USED     0xF0//0xF0, for Daughter Board, 0xD0
    #endif
*/
#define VD_RF_FINE_GAIN_H_USED     0xF0//0xF0, for Daughter Board, 0xD0
#endif
#if !defined(VD_RF_FINE_GAIN_L_USED)
     #define VD_RF_FINE_GAIN_L_USED     0x00
#endif

// Fix Gain value for AV (except ATV) (default value and should be defined in the board head file)
#if !defined(VD_AV_COARSE_GAIN_USED)
     #define VD_AV_COARSE_GAIN_USED     VD_AGC_COARSE_GAIN_X_0_5
#endif
#if !defined(VD_AV_FINE_GAIN_H_USED)
     #define VD_AV_FINE_GAIN_H_USED     0x72
#endif
#if !defined(VD_AV_FINE_GAIN_L_USED)
     #define VD_AV_FINE_GAIN_L_USED     0x00
#endif


// for auto-tuning
#if !defined(HSEN_CHAN_SCAN_DETECT_WIN_BEFORE_LOCK)
#define HSEN_CHAN_SCAN_DETECT_WIN_BEFORE_LOCK   0x03 // 0x03
#endif
#if !defined(HSEN_CHAN_SCAN_DETECT_WIN_AFTER_LOCK)
#define HSEN_CHAN_SCAN_DETECT_WIN_AFTER_LOCK    0x03 // 0x03
#endif
#if !defined(HSEN_CHAN_SCAN_CNTR_FAIL_BEFORE_LOCK)
#define HSEN_CHAN_SCAN_CNTR_FAIL_BEFORE_LOCK    0x08 // 0x08
#endif
#if !defined(HSEN_CHAN_SCAN_CNTR_SYNC_BEFORE_LOCK)
#define HSEN_CHAN_SCAN_CNTR_SYNC_BEFORE_LOCK    0x20 // 0x30
#endif
#if !defined(HSEN_CHAN_SCAN_CNTR_SYNC_AFTER_LOCK)
#define HSEN_CHAN_SCAN_CNTR_SYNC_AFTER_LOCK     0x0A // 0x05
#endif


//------------------------------------
//
//  Color Kill Configuration (PAL)
//
//------------------------------------

#define COLOR_KILL_HIGH_BOUND       0x0E // 0x0E //0x89 // adjust this value for color on when RF signal is weak
#define COLOR_KILL_LOW_BOUND        0x0E // 0x0E //0x15 // adjust this value for color kill level
#define COLOR_KILL_HIGH_BOUND_1     0x0E // Minerva(CAN TUNER): adjust this value for color on when RF signal is weak
#define COLOR_KILL_LOW_BOUND_1      0x0E // Minerva(CAN TUNER): adjust this value for color kill level


//------------------------------------
//
//  H Sensitivity Configuration
//
//------------------------------------

// for normal mode
#define HSEN_NORMAL_DETECT_WIN_BEFORE_LOCK      0x08
#define HSEN_NOAMRL_DETECT_WIN_AFTER_LOCK       0x08
#define HSEN_NORMAL_CNTR_FAIL_BEFORE_LOCK       0x0F
#define HSEN_NORMAL_CNTR_SYNC_BEFORE_LOCK       0x10
#define HSEN_NORMAL_CNTR_SYNC_AFTER_LOCK        0x1C

//------------------------------------
//
//  Misc.
//
//------------------------------------

#define PATCH_COMB_ZONEPLATE            0
#define PATCH_COMB_STILLIMAGE           1
#define ENABLE_DYNAMIC_RAMFILTER        0
#define ENABLE_DLC_YMAX_YMIN_FROM_ISR   0
#define PATCH_VD_VIF_BRI            	0



// Signal Swing (just used in normal mode)
#define SIG_SWING_THRESH            2



//   BK_AFEC_DC
#define MSK_UD7_BANK                0x03
#define VAL_UD7_BANK6               0x00
#define VAL_UD7_BANK7               0x01
#define MSK_UD7_STATE               0x0C
#define VAL_UD7_FREE                0x00
#define VAL_UD7_READ                0x04
#define VAL_UD7_WRITE               0x08
#define VAL_UD7_READ_END            0x0C

#define FSC_MODE_PAL                0x00
#define FSC_MODE_SECAM              0x01
#define FSC_MODE_NTSC               0x02
#define FSC_MODE_NTSC_443           0x03
#define FSC_MODE_PAL_M              0x04
#define FSC_MODE_PAL_60             0x05
#define FSC_MODE_PAL_N              0x06

#define FSC_AUTO_DET_ENABLE         0x01
#define FSC_AUTO_DET_DISABLE        0x00

#define AFE_AGC_ENABLE              0x00
#define AFE_AGC_DISABLE             0x03

#define AGC_DISABLE                 0x03

/* VD register table */
typedef struct
{
    U32 u32RegIdx;   ///< register index
    U8  u08RegVal;     ///< register value
} VD_REG_TABLE_t;


// input source selection
// VD main channel

// Project doesn't use these define
#if 0
#define MSVD_MAIN_CVBS0             IP_CH0_Y
#define MSVD_MAIN_CVBS1             IP_CH1_Y
#define MSVD_MAIN_CVBS2             IP_CH2_Y
#define MSVD_MAIN_CVBS3             IP_CH3_Y

#define MSVD_MAIN_SY0               IP_CH4_Y
#define MSVD_MAIN_SY1               IP_CH5_Y
#define MSVD_MAIN_SC0               IP_CH6_Y
#define MSVD_MAIN_SC1               IP_CH7_Y

#define MSVB_SUB_NONE               IP_NONE_C
#define MSVB_SUB_FB0                IP_FB_RGB0 // scart fast blanking switch(CVBS<->RGB)
#define MSVB_SUB_FB1                IP_FB_RGB1
#define MSVB_SUB_FB2                IP_FB_RGB2

#define MSVD_SUB_SY0                IP_CH4_Y0
#define MSVD_SUB_SY1                IP_CH5_Y1
#define MSVD_SUB_SC0                IP_CH6_C0
#define MSVD_SUB_SC1                IP_CH7_C1
#endif

#ifdef VD_USE_FB
#undef VD_USE_FB
#endif
#define VD_USE_FB                   0


#define ENABLE_DYNAMIC_VD_HTOTAL    1
#if 0
#define VD_HT_NTSC                  (1135L*3/2)
#define VD_HT_PAL                   (1135L*3/2)
#define VD_HT_SECAM                 (1135L*3/2)
#define VD_HT_NTSC_443              (1135L*3/2)
#define VD_HT_PAL_M                 (1135L*3/2)
#define VD_HT_PAL_NC                (1135L*3/2)
#define VD_HT_PAL_60                VD_HT_NTSC_443

#define MSVD_HSTART_NTSC            0xF8
//#define MSVD_HSTART_PAL             0xCF
#define MSVD_HSTART_PAL             0xF8
#define MSVD_HSTART_SECAM           0xD5
//#define MSVD_HSTART_NTSC_433        0xC0
#define MSVD_HSTART_NTSC_433        0xF8
#define MSVD_HSTART_PAL_M           0xCC
#define MSVD_HSTART_PAL_NC          0xD7
#define MSVD_HSTART_PAL_60          MSVD_HSTART_NTSC_433

#define MSVD_HACTIVE_NTSC           (((U32)720*VD_HT_NTSC+429)/858)
#define MSVD_HACTIVE_PAL            (((U32)720*VD_HT_PAL+432)/864)
#define MSVD_HACTIVE_SECAM          (((U32)720*VD_HT_SECAM+432)/864)
#define MSVD_HACTIVE_NTSC_443       (((U32)720*VD_HT_NTSC_443+432)/864)
#define MSVD_HACTIVE_PAL_M          (((U32)720*VD_HT_PAL_M+429)/858)
#define MSVD_HACTIVE_PAL_NC         (((U32)720*VD_HT_PAL_NC+429)/858)
#define MSVD_HACTIVE_PAL_60         MSVD_HACTIVE_NTSC_443
#endif

#define VD_DOUBLE_SAMPLING_ENABLE	1

#if VD_DOUBLE_SAMPLING_ENABLE
#define VD_HT_NTSC                  (1135L*3/2)
#define VD_HT_PAL                   (1135L*3/2)
#define VD_HT_SECAM                 (1135L*3/2)
#define VD_HT_NTSC_443              (1135L*3/2)
#define VD_HT_PAL_M                 (1135L*3/2)
#define VD_HT_PAL_NC                (1135L*3/2)
#define VD_HT_PAL_60                VD_HT_NTSC_443

//TODO
#define MSVD_HSTART_NTSC            261//261//262//263//0x107//0xF8
#define MSVD_HSTART_PAL_M           265//261//262//264//263//262//204

#define MSVD_HSTART_PAL             259//257//258//259//260//261//262//264//0x108
#define MSVD_HSTART_SECAM           248//249//250//213
#define MSVD_HSTART_PAL_NC          271//263//264//263//262//260//258//256

#define MSVD_HSTART_NTSC_433        243//244//245//246//247//248
#define MSVD_HSTART_PAL_60          MSVD_HSTART_NTSC_433

#define MSVD_HSTART_NTSC_SVIDEO            275//276//277//278//276//274//272//266//262
#define MSVD_HSTART_PAL_SVIDEO             270//271//272//270//268//262//259
#define MSVD_HSTART_SECAM_SVIDEO           249//250
#define MSVD_HSTART_NTSC_433_SVIDEO        256//257//258//257//256//254//252//248//244
#define MSVD_HSTART_PAL_M_SVIDEO           276//277//278//276//274//272//268//264
#define MSVD_HSTART_PAL_NC_SVIDEO          278//279//280//278//274//268//264
#define MSVD_HSTART_PAL_60_SVIDEO          256//257//258//256//255//254//250//245

#define MSVD_HACTIVE_NTSC           (((U32)704*VD_HT_NTSC+429)/858)
#define MSVD_HACTIVE_PAL_M          (((U32)704*VD_HT_PAL_M+429)/858)

#define MSVD_HACTIVE_PAL            (((U32)704*VD_HT_PAL+432)/864)
#define MSVD_HACTIVE_SECAM          (((U32)704*VD_HT_SECAM+432)/864)
#define MSVD_HACTIVE_PAL_NC         (((U32)704*VD_HT_PAL_NC+432)/864)

#define MSVD_HACTIVE_NTSC_443       (((U32)704*VD_HT_NTSC_443+429)/858)
#define MSVD_HACTIVE_PAL_60         MSVD_HACTIVE_NTSC_443
#else
#define VD_HT_NTSC                  (858*1L)
#define VD_HT_PAL                   (864*1L)
#define VD_HT_SECAM                 (864*1L)
#define VD_HT_NTSC_443              (864*1L)
#define VD_HT_PAL_M                 (858*1L)
#define VD_HT_PAL_NC                (858*1L)
#define VD_HT_PAL_60                VD_HT_NTSC_443

#define MSVD_HACTIVE_NTSC           (704*1)//(((U32)720*VD_HT_NTSC+429)/858)
#define MSVD_HACTIVE_PAL            (704*1)//(((U32)720*VD_HT_PAL+432)/864)
#define MSVD_HACTIVE_SECAM          (704*1)//(((U32)720*VD_HT_SECAM+432)/864)
#define MSVD_HACTIVE_NTSC_443       (704*1)//(((U32)720*VD_HT_NTSC_443+432)/864)
#define MSVD_HACTIVE_PAL_M          (704*1)//(((U32)720*VD_HT_PAL_M+429)/858)
#define MSVD_HACTIVE_PAL_NC         (704*1)//(((U32)720*VD_HT_PAL_NC+429)/858)
#define MSVD_HACTIVE_PAL_60         MSVD_HACTIVE_NTSC_443

#define MSVD_HSTART_NTSC            83//85//86//((VD_HT_NTSC-MSVD_HACTIVE_NTSC)/2)
#define MSVD_HSTART_PAL             82//83//((VD_HT_PAL-MSVD_HACTIVE_PAL)/2)
#define MSVD_HSTART_SECAM           77//78//79//((VD_HT_SECAM-MSVD_HACTIVE_SECAM)/2)
#define MSVD_HSTART_NTSC_433        75//76//77//((VD_HT_NTSC_443-MSVD_HACTIVE_NTSC_443)/2)	// 360
#define MSVD_HSTART_PAL_M           83//82//83//84//85//((VD_HT_PAL_M-MSVD_HACTIVE_PAL_M)/2)
#define MSVD_HSTART_PAL_NC          80//81//82//((VD_HT_PAL_NC-MSVD_HACTIVE_PAL_NC)/2)
#define MSVD_HSTART_PAL_60          75//76//77//MSVD_HSTART_NTSC_433

// 20080805 swwo LGE for SVIDEO position
#define MSVD_HSTART_NTSC_SVIDEO            92//91//90//88//86//((VD_HT_NTSC-MSVD_HACTIVE_NTSC)/2)
#define MSVD_HSTART_PAL_SVIDEO             89//88//87//86//84//((VD_HT_PAL-MSVD_HACTIVE_PAL)/2)
#define MSVD_HSTART_SECAM_SVIDEO           78//79//((VD_HT_SECAM-MSVD_HACTIVE_SECAM)/2)
#define MSVD_HSTART_NTSC_433_SVIDEO        82//81//80//79//77//((VD_HT_NTSC_443-MSVD_HACTIVE_NTSC_443)/2)	// 360
#define MSVD_HSTART_PAL_M_SVIDEO           92//91//90//88//85//((VD_HT_PAL_M-MSVD_HACTIVE_PAL_M)/2)
#define MSVD_HSTART_PAL_NC_SVIDEO          88//87//86//85//82//((VD_HT_PAL_NC-MSVD_HACTIVE_PAL_NC)/2)
#define MSVD_HSTART_PAL_60_SVIDEO          82//81//80//79//77//MSVD_HSTART_NTSC_433

#endif

/* Remove picture quility settings from VD if PQ_IN_VD_ENABLE is 0 */
#define PQ_IN_VD_ENABLE             0

/* 3D Comb speed up improves 3D comb performance at the channel is chaged. */
////#define LOCK3DSPEEDUP

/* PLL Tracking Speed up */
#define PLL_TRACKING_SPEEDUP

/* Gain control */
#define VD_USE_FIX_GAIN             0
#define VD_USE_AUTO_GAIN            1

/* Coarse gain */
/* x0.5 */
#define VD_AGC_COARSE_GAIN_X_0_5    0
/* x1 */
#define VD_AGC_COARSE_GAIN_X_1      1
/* x2 */
#define VD_AGC_COARSE_GAIN_X_2      2
/* x4 */
#define VD_AGC_COARSE_GAIN_X_4      3



#define COMB_3D
//#define DSPBIN_LOAD_FROM_SERIAL_FLASH
#define DSPBIN_LOAD_FROM_MEM

#ifdef COMB_3D
// 3D_COMB buffer start address

/* 3D Comb speed up improves 3D comb performance at the channel is chaged. */
#define LOCK3DSPEEDUP

#define COMB_3D_BUF_START_ADR   _u32Comb3DBufStartAdrVD
#define COMB_3D_BUF_LEN         _u32Comb3DBufLenVD            // 3.9MB

#else
#endif

//------------------------------------------------------------------------------
//
//  Macros
//
//------------------------------------------------------------------------------



#endif
