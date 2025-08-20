//sif threshold address
#define BTSC_THRESHOLD_ADDR         0x1923
#define M_A2_THRESHOLD_ADDR             0x1920
#define BG_A2_THRESHOLD_ADDR        0x192F
#define DK_A2_THRESHOLD_ADDR        0x193E
#define I_FM_THRESHOLD_ADDR         0x194D
#define L_AM_THRESHOLD_ADDR         0x1951
#define NICAM_BGDKL_THRESHOLD_ADDR  0x1964
#define NICAM_I_THRESHOLD_ADDR      0x1967

//sif gain address
#define A2_OUTPUT_GAIN_X_ADDR   0x1825
#define NICAM_OUTPUT_GAIN_ADDR  0x1829
#define AM_OUTPUT_GAIN_ADDR     0x182B
#define A2_OUTPUT_GAIN_M_ADDR   0x1821
#define BTSC_OUTPUT_GAIN_ADDR   0x1921
#define BTSC_MTS_OUTPUT_GAIN    0x1934

// Palsum
#define A2_M_THRESHOLD_ADDR 0x1920
#define A2_BG_THRESHOLD_ADDR 0x192F
#define A2_DK_THRESHOLD_ADDR 0x193E
#define A2_I_THRESHOLD_ADDR   0x194D
#define AM_THRESHOLD_ADDR 0x1951
#define HIDEV_M_THRESHOLD_ADDR      0x1954
#define HIDEV_BG_THRESHOLD_ADDR     0x1958
#define HIDEV_DK_THRESHOLD_ADDR     0x195C
#define HIDEV_I_THRESHOLD_ADDR      0x1960
#define NICAM_BG_PHERR_THR 0x1964
#define NICAM_I_PHERR_THR 0x1967

#define A2_M_STANDARD     0x0
#define A2_BG_STANDARD    0x1
#define A2_DK_STANDARD    0x2
#define A2_I_STANDARD     0x3
#define AM_STANDARD       0x4
#define NICAM_BG_STANDARD 0x5
#define NICAM_I_STANDARD  0x6
#define HIDEV_M_STANDARD  0x7
#define HIDEV_BG_STANDARD 0x8
#define HIDEV_DK_STANDARD 0x9
#define HIDEV_I_STANDARD  0xA
#define BTSC_STANDARD    0x00
#define WRITE_THRESHOLD 0x10

#define CARRIER1_ON_AMP 0x0
#define CARRIER1_OFF_AMP 0x1
#define CARRIER1_ON_NSR 0x2
#define CARRIER1_OFF_NSR 0x3
#define CARRIER2_ON_AMP 0x4
#define CARRIER2_OFF_AMP 0x5
#define CARRIER2_ON_NSR 0x6
#define CARRIER2_OFF_NSR 0x7
#define A2_PILOT_ON_AMP 0x8
#define A2_PILOT_OFF_AMP 0x9
#define A2_PILOT_ON_PHASE  0xA   // change for the Korea A2 threhsold function.
#define A2_PILOT_OFF_PHASE 0xB    // change for the Korea A2 threhsold function.


#define NICAM_ON_SIGERR 0x0
#define NICAM_OFF_SIGERR 0x1

 // change for the Korea A2 threhsold function.
/*
const MS_U16 au_pal_sys_threshold[12]=
{
    M_A2_THRESHOLD_ADDR,
    BG_A2_THRESHOLD_ADDR,
    DK_A2_THRESHOLD_ADDR,
    I_FM_THRESHOLD_ADDR,
    L_AM_THRESHOLD_ADDR,
    NICAM_BGDKL_THRESHOLD_ADDR,
    NICAM_I_THRESHOLD_ADDR,
    HIDEV_M_THRESHOLD_ADDR,
    HIDEV_BG_THRESHOLD_ADDR,
    HIDEV_DK_THRESHOLD_ADDR,
    HIDEV_I_THRESHOLD_ADDR,
    BTSC_THRESHOLD_ADDR,
};
*/
WORD au_pal_sys_threshold[11][12]=
{
    {A2_M_THRESHOLD_ADDR    , A2_M_THRESHOLD_ADDR+1     , A2_M_THRESHOLD_ADDR+2     , A2_M_THRESHOLD_ADDR+3     , A2_M_THRESHOLD_ADDR+4   , A2_M_THRESHOLD_ADDR+5 , A2_M_THRESHOLD_ADDR+6 , A2_M_THRESHOLD_ADDR+7 ,A2_M_THRESHOLD_ADDR+8 ,A2_M_THRESHOLD_ADDR+9 ,A2_M_THRESHOLD_ADDR+10 ,A2_M_THRESHOLD_ADDR+11, },
    {A2_BG_THRESHOLD_ADDR   , A2_BG_THRESHOLD_ADDR+1    , A2_BG_THRESHOLD_ADDR+2    , A2_BG_THRESHOLD_ADDR+3    , A2_BG_THRESHOLD_ADDR+4  , A2_BG_THRESHOLD_ADDR+5, A2_BG_THRESHOLD_ADDR+6, A2_BG_THRESHOLD_ADDR+7,A2_BG_THRESHOLD_ADDR+8 ,A2_BG_THRESHOLD_ADDR+9 , A2_BG_THRESHOLD_ADDR+10 ,A2_BG_THRESHOLD_ADDR+11 ,},
    {A2_DK_THRESHOLD_ADDR   , A2_DK_THRESHOLD_ADDR+1    , A2_DK_THRESHOLD_ADDR+2    , A2_DK_THRESHOLD_ADDR+3    , A2_DK_THRESHOLD_ADDR+4  , A2_DK_THRESHOLD_ADDR+5, A2_DK_THRESHOLD_ADDR+6, A2_DK_THRESHOLD_ADDR+7,A2_DK_THRESHOLD_ADDR+8 ,A2_DK_THRESHOLD_ADDR+9 , A2_DK_THRESHOLD_ADDR+10 ,A2_DK_THRESHOLD_ADDR+11 ,},
    {A2_I_THRESHOLD_ADDR    , A2_I_THRESHOLD_ADDR+1     , A2_I_THRESHOLD_ADDR+2     , A2_I_THRESHOLD_ADDR+3     , NULL                    , NULL                  , NULL                  , NULL                  , NULL                  ,NULL                  ,NULL                  ,NULL                  ,},
    {AM_THRESHOLD_ADDR      , AM_THRESHOLD_ADDR+1       , NULL                      , NULL                      , NULL                    , NULL                  , NULL                  , NULL                  , NULL                  ,NULL                  ,NULL                  ,NULL                  ,},
    {NICAM_BG_PHERR_THR     , NICAM_BG_PHERR_THR+1      , NULL                      , NULL                      , NULL                    , NULL                  , NULL                  , NULL                  , NULL                  ,NULL                  ,NULL                  ,NULL                  ,},
    {NICAM_I_PHERR_THR      , NICAM_I_PHERR_THR+1       , NULL                      , NULL                      , NULL                    , NULL                  , NULL                  , NULL                  , NULL                  ,NULL                  ,NULL                  ,NULL                  ,},
    {HIDEV_M_THRESHOLD_ADDR , HIDEV_M_THRESHOLD_ADDR+1  , HIDEV_M_THRESHOLD_ADDR+2  , HIDEV_M_THRESHOLD_ADDR+3  , NULL                    , NULL                  , NULL                  , NULL                  , NULL                  ,NULL                  ,NULL                  ,NULL                  ,},
    {HIDEV_BG_THRESHOLD_ADDR, HIDEV_BG_THRESHOLD_ADDR+1 , HIDEV_BG_THRESHOLD_ADDR+2 , HIDEV_BG_THRESHOLD_ADDR+3 , NULL                    , NULL                  , NULL                  , NULL                  , NULL                  ,NULL                  ,NULL                  ,NULL                  ,},
    {HIDEV_DK_THRESHOLD_ADDR, HIDEV_DK_THRESHOLD_ADDR+1 , HIDEV_DK_THRESHOLD_ADDR+2 , HIDEV_DK_THRESHOLD_ADDR+3 , NULL                    , NULL                  , NULL                  , NULL                  , NULL                  ,NULL                  ,NULL                  ,NULL                  ,},
    {HIDEV_I_THRESHOLD_ADDR , HIDEV_I_THRESHOLD_ADDR+1  , HIDEV_I_THRESHOLD_ADDR+2  , HIDEV_I_THRESHOLD_ADDR+3  , NULL                    , NULL                  , NULL                  , NULL                  , NULL                  ,NULL                  ,NULL                  ,NULL                  ,},
};






const MS_U16  SIF_PM_GAIN_TBL_PAL[6][2]=
{
   { A2_OUTPUT_GAIN_X_ADDR,  A2_OUTPUT_GAIN_X_ADDR+1},        //A2 / FM-MONO
   { NICAM_OUTPUT_GAIN_ADDR,  NICAM_OUTPUT_GAIN_ADDR+1},    //NICAM
   { AM_OUTPUT_GAIN_ADDR,  AM_OUTPUT_GAIN_ADDR+1},              //AM
   { A2_OUTPUT_GAIN_X_ADDR+2,  A2_OUTPUT_GAIN_X_ADDR+3},  //HIDEV
   { A2_OUTPUT_GAIN_M_ADDR,  A2_OUTPUT_GAIN_M_ADDR+1},  //FM-MONO M
   { A2_OUTPUT_GAIN_M_ADDR+2,  A2_OUTPUT_GAIN_M_ADDR+3},  //FM-MONO M HIDEV
};

const MS_U16  SIF_PM_GAIN_TBL_BTSC[4][2]=
{
   { BTSC_OUTPUT_GAIN_ADDR,  BTSC_OUTPUT_GAIN_ADDR+1},        // BTSC
   { BTSC_MTS_OUTPUT_GAIN,  BTSC_MTS_OUTPUT_GAIN+1},                    //BTSC Mono
   { BTSC_MTS_OUTPUT_GAIN+2,  BTSC_MTS_OUTPUT_GAIN+3},                    //BTSC Stereo
   { BTSC_MTS_OUTPUT_GAIN+4,  BTSC_MTS_OUTPUT_GAIN+5},                //BTSC SAP
};

WORD code au_ntsc_sys_threshold[1][12]=
{
    {
    BTSC_THRESHOLD_ADDR  + BTSC_MONO_ON_NSR_RATIO,
    BTSC_THRESHOLD_ADDR  + BTSC_MONO_OFF_NSR_RATIO,
    BTSC_THRESHOLD_ADDR  + BTSC_PILOT_AMPLITUDE_ON,
    BTSC_THRESHOLD_ADDR  + BTSC_PILOT_AMPLITUDE_OFF,
    BTSC_THRESHOLD_ADDR  + BTSC_SAP_ON_NSR_RATIO,
    BTSC_THRESHOLD_ADDR  + BTSC_SAP_OFF_NSR_RATIO,
    BTSC_THRESHOLD_ADDR  + BTSC_STEREO_ON_RATIO,
    BTSC_THRESHOLD_ADDR  + BTSC_STEREO_OFF_NSR_RATIO,
    BTSC_THRESHOLD_ADDR  + BTSC_SAP_AMPLITUDE_ON,
    BTSC_THRESHOLD_ADDR  + BTSC_SAP_AMPLITUDE_OFF,
    BTSC_THRESHOLD_ADDR  + BTSC_HIDEV_ON_NSR_RATIO,
    BTSC_THRESHOLD_ADDR  + BTSC_HIDEV_OFF_NSR_RATIO,
    },
};

