#define _MDRV_MFC_LETTERBOX_H_

#include "mdrv_mfc_platform.h"

#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define diff(a, b)          (((a) > (b)) ? (a-b):(b-a))
#define absGmvX(a)            (diff(a, 0x80))
#define absGmvY(a)            (diff(a, 0x40))
#define Posdiff(a,b)           (((a) > (b)) ? (a-b):0)

#define LB_DEBUG_LINE 0

XDATA WORD g_MfcPanelWidth;
XDATA WORD g_MfcPanelHeight;
XDATA BYTE g_MfcPanelVfreq_lb;

#ifndef _gMfcType_
#define _gMfcType_
typedef struct
{
// REG_29CC
    BYTE GMVX;
    BYTE GMVY        : 7;
    BYTE GMVConfirm    : 2;

    BYTE SecondGMVX;
    BYTE SecondGMVY;

// REG_29CE
    BYTE FrameEnd     : 1;
    BYTE Skip_Thd     : 2;
// REG_29CF
    BYTE Film22Mode : 1;
    BYTE Film32Mode : 1;
    BYTE Film22Idx     : 1;
    BYTE Film32Idx     : 3;
    BYTE FDUP         : 1;

    BYTE MaxSAD_MSB;
    BYTE MinSAD_MSB;
    BYTE maxXNegSpeed; //0x29c2;
    BYTE maxXPosSpeed; //0x29c2;
    BYTE maxYNegSpeed; //0x29c2;
    BYTE maxYPosSpeed; //0x29c2;

// REG_29E0
    WORD ErrorBlk1;
    WORD ErrorBlk2;
    WORD ErrorBlk3;
    WORD ErrorBlk4;
    WORD ErrorBlk5;
    WORD ErrorBlk6;

// ursa3 only
    WORD ErrorBlkA;
    WORD ErrorBlkB;

    WORD ProgrammableStatusCnt1; //--REG_2C60
    WORD ProgrammableStatusCnt2;
    WORD ProgrammableStatusCnt3;

    WORD SecondGmvMinSAD; //--REG_29DC
    WORD SamePixelCnt; //--REG_26D0

    WORD GMV0Err;
    WORD MINonContinuousBoundary;
    WORD gmvBlkCnt;  //--for otherPatchs: dynamic search range
    WORD mv0BlkCnt;  //--for otherPatchs: dynamic search range
    WORD gmvBlkCnt2; //--for getHaloLevel
    WORD mv0BlkCnt2; //--for getHaloLevel
    WORD cplxBlkCnt2;
    WORD MinSAD;
    WORD MaxSAD;
    WORD MovingBlkCnt;
    WORD cplxBlkCnt;
    WORD veryCplxBlkCnt;
    WORD unMatchPointCnt;

    BYTE bMfcInputFreq; // get from scaler chip
}_MfcType;
XDATA _MfcType gMfcType_lb;
#endif

//Letter box veriable (UDLR)
XDATA BYTE g_setLetterBox[4], g_preLetterBox1[4], g_preLetterBox2[4], g_preRawBox1[4], g_preRawBox2[4];
XDATA BYTE g_LetterBoxFrameCnt, g_sameLetterBoxCnt[4],  g_zeroLetterBoxCnt[4];
XDATA BOOL g_preSmallGMVx[4], g_preSmallGMVy[4], Inc[4];
XDATA BOOL g_smallGMVx_valid, g_smallGMVy_valid;
void apiInitializeLetterbox(U16 wPnlWidth, U16 wPnlHeight, U8 ucPnlVfreq)
{
    int i;
    // Bruce add panel global variable, please don't use Macro
    g_MfcPanelWidth = wPnlWidth;
    g_MfcPanelHeight = wPnlHeight;
    g_MfcPanelVfreq_lb = ucPnlVfreq;

    g_smallGMVx_valid = g_smallGMVy_valid = FALSE;

    //initial letter box veriable (UDLR)
    for(i=0; i<4; i++)
    {
        g_setLetterBox[i] = g_preLetterBox1[i] = g_preLetterBox2[i] = g_preRawBox1[i] = g_preRawBox2[i] = 0;
        g_sameLetterBoxCnt[i] = g_zeroLetterBoxCnt[i] = 0;
        g_preSmallGMVx[i] = g_preSmallGMVy[i] = Inc[i] = FALSE;
    }
    g_LetterBoxFrameCnt=0;
    g_smallGMVx_valid = g_smallGMVy_valid = FALSE;

    MDrv_MFC_WriteBit(0x2C44, LB_DEBUG_LINE, _BIT5);

}



BOOL getSamePixelCnt(void)
{
    WORD SamePixelInGreen=0;
    BYTE SamePixelFrameIdx=0;
    WORD SamePixelInGrey=0;
    WORD SamePixelInColor=0;
    BOOL LargeGreyPixel, AlmostGrey, GreyPatternValid;

    if (SamePixelFrameIdx==0)
    {
        //--detect Green next
        MDrv_MFC_WriteByte(0x2632, 0x64); // Y base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2633, 0x40); // Y bias for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2634, 0x4B); // U base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2635, 0x20); // U bias for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2636, 0x7D); // V base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2637, 0x20); // V bias for SamePixelDetection (*4)

        //--get SamePixelInColor result
         SamePixelInColor = gMfcType_lb.SamePixelCnt;

    }
    else if (SamePixelFrameIdx==1)
    {
        //--detect Grey next
        MDrv_MFC_WriteByte(0x2632, 0x80); // Y base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2633, 0x7f); // Y bias for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2634, 0x80); // U base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2635, 0x10); // U bias for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2636, 0x80); // V base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2637, 0x10); // V bias for SamePixelDetection (*4)

        //--get SamePixelInGreen result
        SamePixelInGreen = gMfcType_lb.SamePixelCnt;
    }
    else if (SamePixelFrameIdx==2)
    {
        //--detect Green next
        MDrv_MFC_WriteByte(0x2632, 0x64); // Y base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2633, 0x40); // Y bias for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2634, 0x64); // U base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2635, 0x40); // U bias for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2636, 0x64); // V base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2637, 0x40); // V bias for SamePixelDetection (*4)

        //--get SamePixelInGrey result
        SamePixelInGrey = gMfcType_lb.SamePixelCnt;


    }
    else
    {
        //--detect AllColor next
        MDrv_MFC_WriteByte(0x2632, 0x80); // Y base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2633, 0x7f); // Y bias for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2634, 0x80); // U base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2635, 0x7f); // U bias for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2636, 0x80); // V base for SamePixelDetection (*4)
        MDrv_MFC_WriteByte(0x2637, 0x7f); // V bias for SamePixelDetection (*4)

        //--get SamePixelInGreen result
        SamePixelInGreen = gMfcType_lb.SamePixelCnt;

    }

    if (SamePixelFrameIdx==3)
    {
        SamePixelFrameIdx=0;
    }
    else
    {
        SamePixelFrameIdx++;
    }

    LargeGreyPixel = (SamePixelInColor>24000 && SamePixelInGrey>24000);
    AlmostGrey = (diff(SamePixelInColor, SamePixelInGrey)<2000);
    GreyPatternValid = (AlmostGrey || (LargeGreyPixel && diff(SamePixelInColor, SamePixelInGrey)<(min(SamePixelInColor>>3,6000))));

    MDrv_MFC_Write2Bytes(0x2C4C, SamePixelInGreen);
    MDrv_MFC_WriteBit(0x2C4A, GreyPatternValid, _BIT3);
    return GreyPatternValid;
}

#define BOUNDARY_CHECK_UD 2
#define BOUNDARY_CHECK_LR 1
#define ASPEC_RATIO_CHANGE_UD 128
#define ASPEC_RATIO_CHANGE_LR 96

#define LETTERBOX_Detect_U 0
#define LETTERBOX_Detect_D 1
#define LETTERBOX_Detect_L 0
#define LETTERBOX_Detect_R 1

#define LETTERBOX_STABLE_CNT 16

XDATA BYTE g_UpdateStatusCnt = 0x04; //--film32: 5; film22: 4; video: 2
void DetectLetterBox(void)
{
    BYTE UDLRBox[4];
    BYTE i;
    BOOL smallGMV_y = TRUE;
    BOOL smallGMV_x = TRUE;
    BOOL LG_black_white_bar = FALSE;
    BOOL GreyPatternValid= getSamePixelCnt();

    g_LetterBoxFrameCnt=(g_LetterBoxFrameCnt+1)%g_UpdateStatusCnt;

    //--read LetterBox status
    UDLRBox[0] = MDrv_MFC_ReadByte( 0x29FD );
    UDLRBox[1] = MDrv_MFC_ReadByte( 0x29FC );
    UDLRBox[2] = MDrv_MFC_ReadByte( 0x29FF );
    UDLRBox[3] = MDrv_MFC_ReadByte( 0x29FE );

    //--modify LetterBox status when D/R is zero
    if(UDLRBox[1]>=1) UDLRBox[1] -= LETTERBOX_Detect_D;
    if(UDLRBox[3]>=1) UDLRBox[3] -= LETTERBOX_Detect_R;

    //-- for LG black/white bar (set letterbox status as zero)
     LG_black_white_bar = GreyPatternValid  && gMfcType_lb.cplxBlkCnt<1000 && (absGmvY(gMfcType_lb.GMVY)<=2)
                        && gMfcType_lb.MaxSAD_MSB < 0x8 && gMfcType_lb.ErrorBlk1 <200
                        &&  gMfcType_lb.MINonContinuousBoundary <200;



    #if 0
    if (LG_black_white_bar)
        gDebugLight1 |= _BIT7;
    else
        gDebugLight1 &= ~ _BIT7;
    #endif

    //--set letterbox status as zero when LG_black_white_bar or status overflow
    UDLRBox[0] = (LG_black_white_bar || (UDLRBox[0]>=0xF0)) ? 0 : UDLRBox[0];
    UDLRBox[1] = (LG_black_white_bar || (UDLRBox[1]>=0xF0)) ? 0 : UDLRBox[1];
    UDLRBox[2] = (LG_black_white_bar || (UDLRBox[2]>=0xF0)) ? 0 : UDLRBox[2];
    UDLRBox[3] = (LG_black_white_bar || (UDLRBox[3]>=0xF0)) ? 0 : UDLRBox[3];

    //-- for small GMV (set letterbox value as negtive)
    #if 1
    for(i=0; i<4; i++)
    {
        smallGMV_x &= g_preSmallGMVx[i];
        smallGMV_y &= g_preSmallGMVy[i];
    }
    g_smallGMVx_valid = smallGMV_x & (absGmvX(gMfcType_lb.GMVX)<=4);
    g_smallGMVy_valid = smallGMV_y & (absGmvY(gMfcType_lb.GMVY)<=4);
    MDrv_MFC_WriteBit(0x2C4A, g_smallGMVx_valid,_BIT1);
    MDrv_MFC_WriteBit(0x2C4A, g_smallGMVy_valid,_BIT2);
    #endif

    #if 0
    gDebugLight1 &= ~_BIT4;
    gDebugLight1 &= ~_BIT5;
    if(g_smallGMVy_valid && g_smallGMVx_valid)
    {
        //--do nothing
    }
    else if(g_smallGMVy_valid)
    {
        gDebugLight1 |= _BIT4;
    }
    else if(g_smallGMVx_valid)
    {
        gDebugLight1 |= _BIT5;
    }
    #endif

    //-- for continuously increase (set letterbox as zero)
    #if 1
    for(i=0; i<4; i++)
    {
        if (g_preLetterBox2[i] > (g_preLetterBox1[i]+2) && g_preLetterBox1[i] > (g_setLetterBox[i]+2) && g_preLetterBox2[i] > (g_setLetterBox[i]+4))
            Inc[i] = TRUE;
        else
            Inc[i] = FALSE;
    }
    #endif

    #if 0
    if (Inc[0]||Inc[1]||Inc[2]||Inc[3])
        gDebugLight1 |= _BIT6;
    else
        gDebugLight1 &= ~_BIT6;
    #endif

    //--check stable letterbox status
    {
        for(i=0; i<4; i++)
        {
                //--detect stable letterbox status
                if(i==0 || i==1)
        {
                    if ((UDLRBox[i] <= (g_preRawBox1[i] + BOUNDARY_CHECK_UD) && g_preRawBox1[i] <= (UDLRBox[i] + BOUNDARY_CHECK_UD))
                        && (UDLRBox[i] <= (g_preRawBox2[i] + BOUNDARY_CHECK_UD) && g_preRawBox2[i] <= (UDLRBox[i] + BOUNDARY_CHECK_UD)))
                        g_sameLetterBoxCnt[i]=g_sameLetterBoxCnt[i]<0xff ? g_sameLetterBoxCnt[i]+1 : 0xff;
                            else
                        g_sameLetterBoxCnt[i]=0;
        }
        else
        {
                    if ((UDLRBox[i] <= (g_preRawBox1[i] + BOUNDARY_CHECK_LR) && g_preRawBox1[i] <= (UDLRBox[i] + BOUNDARY_CHECK_LR))
                        && (UDLRBox[i] <= (g_preRawBox2[i] + BOUNDARY_CHECK_LR) && g_preRawBox2[i] <= (UDLRBox[i] + BOUNDARY_CHECK_LR)))
                g_sameLetterBoxCnt[i]=g_sameLetterBoxCnt[i]<0xff ? g_sameLetterBoxCnt[i]+1 : 0xff;
                else
                g_sameLetterBoxCnt[i]=0;
        }

                //--detect stable zero letterbox status
            if (UDLRBox[i] == 0)
                g_zeroLetterBoxCnt[i]=g_zeroLetterBoxCnt[i]<0xff ? g_zeroLetterBoxCnt[i]+1 : 0xff;
        else
                g_zeroLetterBoxCnt[i]=0;
        }
    }

    //--set letterbox status
    {
        //--set when ASPEC_RATIO_CHANGE
            if((((WORD)UDLRBox[0] >= (WORD)g_setLetterBox[0] + ASPEC_RATIO_CHANGE_UD || (WORD)g_setLetterBox[0] >= (WORD)UDLRBox[0] + ASPEC_RATIO_CHANGE_UD) &&
            ((WORD)UDLRBox[1] >= (WORD)g_setLetterBox[1] + ASPEC_RATIO_CHANGE_UD || (WORD)g_setLetterBox[1] >= (WORD)UDLRBox[1] + ASPEC_RATIO_CHANGE_UD)) ||
            (((WORD)UDLRBox[2] >= (WORD)g_setLetterBox[2] + ASPEC_RATIO_CHANGE_LR || (WORD)g_setLetterBox[2] >= (WORD)UDLRBox[2] + ASPEC_RATIO_CHANGE_LR) &&
            ((WORD)UDLRBox[3] >= (WORD)g_setLetterBox[3] + ASPEC_RATIO_CHANGE_LR || (WORD)g_setLetterBox[3] >= (WORD)UDLRBox[3] + ASPEC_RATIO_CHANGE_LR)))
        {
        for(i=0; i<4; i++)
                g_setLetterBox[i] = UDLRBox[i];
        }
        //--set when LETTERBOX_FASTIN/stable_status
        else
        {
        for(i=0; i<4; i++)
        {
                if (i==0 || i==1)
                {
                    if (Inc[i])
                        g_setLetterBox[i] = 0;
                    else if (g_sameLetterBoxCnt[i] > LETTERBOX_STABLE_CNT || UDLRBox[i]+BOUNDARY_CHECK_UD<g_setLetterBox[i])
                        g_setLetterBox[i] = UDLRBox[i];
                }
                else
                 {
                    if (Inc[i])
                        g_setLetterBox[i] = 0;
                    else if (g_sameLetterBoxCnt[i] > LETTERBOX_STABLE_CNT || (((UDLRBox[i]!=0) ? (UDLRBox[i]+BOUNDARY_CHECK_LR<g_setLetterBox[i]): g_zeroLetterBoxCnt[i]>1 )))
                        g_setLetterBox[i] = UDLRBox[i];
                }
            }
        }
}

    //--updated previous status every N frames
    if (g_LetterBoxFrameCnt==(g_UpdateStatusCnt-1))
{
        //--update raw letterbox status
        for(i=0; i<4; i++)
        {
            g_preRawBox2[i]=g_preRawBox1[i];
            g_preRawBox1[i]=UDLRBox[i];
        }

        //--update final letterbox status
        for(i=0; i<4; i++)
        {
            g_preLetterBox2[i]=g_preLetterBox1[i];
            g_preLetterBox1[i]=g_setLetterBox[i];
        }

        //--Update small GMV status
        for(i=3; i>0; i--)
        {
            g_preSmallGMVx[i] = g_preSmallGMVx[i-1];
            g_preSmallGMVy[i] = g_preSmallGMVy[i-1];
        }
        g_preSmallGMVx[0]=(absGmvX(gMfcType_lb.GMVX)<4);
        g_preSmallGMVy[0]=(absGmvY(gMfcType_lb.GMVY)<4);
    }

    //--for small bubble
    if (gMfcType_lb.ErrorBlk1<50 && (absGmvX(gMfcType_lb.GMVX)+absGmvY(gMfcType_lb.GMVY))<=4 && gMfcType_lb.MINonContinuousBoundary <=5)
    {
        g_setLetterBox[0] = 0;
        g_setLetterBox[1] = 0;
        g_setLetterBox[2] = 0;
    }

    #if 0
		g_setLetterBox[0] = UDLRBox[0];
		g_setLetterBox[1] = UDLRBox[1];
		g_setLetterBox[2] = UDLRBox[2];
		g_setLetterBox[3] = UDLRBox[3];

    #endif

    #if LB_DEBUG_LINE
    if(MDrv_MFC_ReadByte(0x2C44)&_BIT5)
    {
        if (g_LetterBoxFrameCnt==0)
                MDrv_MFC_Write2Bytes(0x290C, g_setLetterBox[0] | 0xc000 );
        else if (g_LetterBoxFrameCnt==1)
            MDrv_MFC_Write2Bytes(0x290C, (g_MfcPanelHeight-g_setLetterBox[1]) | 0xc000 );
        else if (g_LetterBoxFrameCnt==2)
            MDrv_MFC_Write2Bytes(0x290C, g_setLetterBox[2] | 0x8000 );
        else
            MDrv_MFC_Write2Bytes(0x290C, ((g_MfcPanelWidth>>1)-g_setLetterBox[3]) | 0x8000 );
    }
    #endif

}

void _SetBoundary_GMV(void)
{
    //--GMV is 2P-base setting (with "1P-base >> 3" operator)
    {
        //--GMV is 2P-base setting  // cityscape star pattern has problem.
        //MDrv_MFC_WriteByte(0x291C, g_setLetterBox[0]>>3);
        //MDrv_MFC_WriteByte(0x291D, (g_MfcPanelHeight-g_setLetterBox[1])>>3);

        //MDrv_MFC_WriteByte(0x291A, g_setLetterBox[2]>>2);
        //MDrv_MFC_WriteByte(0x291B, (g_MfcPanelWidth>>3)-(g_setLetterBox[3]>>2));

        //--Skip is 2P-base setting
        #if 0
        MDrv_MFC_WriteByte(0x29A2, g_setLetterBox[0]>>3);
        MDrv_MFC_WriteByte(0x29A3, (g_MfcPanelHeight-g_setLetterBox[1])>>3);
        MDrv_MFC_WriteByte(0x29A0, g_setLetterBox[2]>>2);
        MDrv_MFC_WriteByte(0x29A1, (g_MfcPanelWidth>>3)-(g_setLetterBox[3]>>2));
        #endif
    }
}

void _SetBoundary_MIMLB(void)
{
    char LETTERBOX_MI_U=2;
    char LETTERBOX_MI_D=2;
    char LETTERBOX_MI_L=2; //--2p-base
    char LETTERBOX_MI_R=2; //--2p-base

    char LETTERBOX_MLB_U=3;
    char LETTERBOX_MLB_D=3;
    char LETTERBOX_MLB_L=3; //--2p-base
    char LETTERBOX_MLB_R=3; //--2p-base

    WORD LETTERBOX_L=0;
    WORD LETTERBOX_R=0;

    //--patch for small GMV (set letterbox patch value as negtive)
    if(0)
    {
        if(g_smallGMVy_valid && g_smallGMVx_valid)
        {
            //--do nothing
        }
        else if(g_smallGMVy_valid)
        {
            LETTERBOX_MI_U=(g_setLetterBox[0]>=8 ? -2 : LETTERBOX_MI_U);
             LETTERBOX_MI_D=(g_setLetterBox[1]>=8 ? -2 : LETTERBOX_MI_D);
        }
        else if(g_smallGMVx_valid)
        {
             LETTERBOX_MI_L=(g_setLetterBox[2]>=8 ? -2 : LETTERBOX_MI_L);
             LETTERBOX_MI_R=(g_setLetterBox[3]>=8 ? -2 : LETTERBOX_MI_R);
        }
    }

    //--Patch for full-screen with zero offset
    if(1)
    {
        if (g_sameLetterBoxCnt[0] > LETTERBOX_STABLE_CNT && g_zeroLetterBoxCnt[0] > LETTERBOX_STABLE_CNT && g_smallGMVy_valid)
        {
            LETTERBOX_MI_U = 0;
            LETTERBOX_MLB_U = 0;
        }
        if (g_sameLetterBoxCnt[1] > LETTERBOX_STABLE_CNT && g_zeroLetterBoxCnt[1] > LETTERBOX_STABLE_CNT && g_smallGMVy_valid)
        {
            LETTERBOX_MI_D = 0;
            LETTERBOX_MLB_D = 0;
        }
        if (g_sameLetterBoxCnt[2] > LETTERBOX_STABLE_CNT && g_zeroLetterBoxCnt[2] > LETTERBOX_STABLE_CNT && g_smallGMVx_valid)
        {
            LETTERBOX_MI_L = 0;
            LETTERBOX_MLB_L = 0;
        }
        if (g_sameLetterBoxCnt[3] > LETTERBOX_STABLE_CNT && g_zeroLetterBoxCnt[3] > LETTERBOX_STABLE_CNT && g_smallGMVx_valid)
        {
            LETTERBOX_MI_R = 0;
            LETTERBOX_MLB_R = 0;
        }
    }

        //--MI is 2P-base setting, MLB is 1P-base setting

        LETTERBOX_L = (g_setLetterBox[2]+LETTERBOX_MLB_L)*2;
        LETTERBOX_R = (g_setLetterBox[3]+LETTERBOX_MLB_R)*2;

    {
        MDrv_MFC_WriteByte(0x2997, g_setLetterBox[0]+LETTERBOX_MI_U);
        MDrv_MFC_WriteByte(0x2996, g_setLetterBox[1]+LETTERBOX_MI_D);
        MDrv_MFC_WriteByte(0x2995, g_setLetterBox[2]+LETTERBOX_MI_L);
        MDrv_MFC_WriteByte(0x2994, g_setLetterBox[3]+LETTERBOX_MI_R);

        MDrv_MFC_WriteByte(0x297C, g_setLetterBox[0]+LETTERBOX_MLB_U);
        MDrv_MFC_WriteByte(0x297D, g_setLetterBox[1]+LETTERBOX_MLB_D);
        MDrv_MFC_Write2Bytes(0x2978, LETTERBOX_L | 0x8000);
        MDrv_MFC_Write2Bytes(0x297A, LETTERBOX_R);

        //--set Halo invalid window(sync to block size 16*8)
        MDrv_MFC_WriteByte(0x2C27, (g_setLetterBox[0]>>3)+4);
        MDrv_MFC_WriteByte(0x2C26, (g_setLetterBox[1]>>3)+4);
        MDrv_MFC_WriteByte(0x2C25, (g_setLetterBox[2]>>3)+4);
        MDrv_MFC_WriteByte(0x2C24, (g_setLetterBox[3]>>3)+4);

    }
}

void _SetBoundary_ME(void)//BOOL notForceWholeScreen)
{
	//char LETTERBOX_ME_U=0;
	//char LETTERBOX_ME_D=0;
	char LETTERBOX_ME_L=0; //--2p-base
	char LETTERBOX_ME_R=0; //--2p-base


	WORD temp;
    BYTE temp2, temp3;
    BYTE ME2L, ME2R;
    //BYTE letterbox_debug = 0;
    if(1)
    {
        temp = (g_MfcPanelWidth - g_setLetterBox[3]-LETTERBOX_ME_R) >> 1 ;
        //letterbox_debug=gReg265C&0x70;
        //if(letterbox_debug==0x30)
            //MDrv_MFC_Write2Bytes(0x290C, temp | 0x8000 ); // debug

        // ME1 block -> not found
        // temp2 = ( g_setLetterBox[2] + 32 ) >> 6;

        // --- ME1 ---
	    temp2 = (( g_setLetterBox[2] + 48 ) >> 5 );
	    temp3 = (( g_setLetterBox[3] + 48 ) >> 5 );

        temp = (0xc000 | (temp2 | ( temp3 << 5 )));
        MDrv_MFC_Write2Bytes(0x2922, temp );

        // --- ME2 ---

        ME2L = ((g_setLetterBox[2]+ 8) >> 3);
        ME2R = ((g_setLetterBox[3]+ 8) >> 3);


        temp = (0x0000 | (( ME2R << 6 ) | ME2L));
        //temp = 0x040F;
        MDrv_MFC_Write2Bytes(0x2944, temp );

        //////////////////////////////
        temp=(g_setLetterBox[2]+ LETTERBOX_ME_L)>>1;
        temp = ((ME2L<<3)-20)<<1;
        //if(letterbox_debug==0x10)
       //   MDrv_MFC_Write2Bytes(0x290C, temp | 0x8000 ); // debug

        #if 0
        temp = (g_MfcPanelHeight- g_prvLetterBoxD-LETTERBOX_ME_D);
        #else
        temp = g_MfcPanelHeight;
        #endif
        //if(letterbox_debug==0x70)
            //MDrv_MFC_Write2Bytes(0x290C, ((temp<0x3ff)?temp:0) | 0xc000 ); // debug

        #if 0
        temp = g_prvLetterBoxU+LETTERBOX_ME_U;
        #else
        temp=0;
        #endif
        //if(letterbox_debug==0x50)
            //MDrv_MFC_Write2Bytes(0x290C, temp | 0xc000 ); // debug
    }
    else
    {
        //MDrv_MFC_Write2Bytes(0x290C, 0 ); // debug

        MDrv_MFC_Write2Bytes(0x2922, 0xc020 ); // ME1 block -> not found
        MDrv_MFC_Write2Bytes(0x2944, 0x0141 );
    }
}


void ProgramLetterBox(void)
{
    _SetBoundary_GMV();
    _SetBoundary_MIMLB();
    _SetBoundary_ME();
 }

#if LETTERBOX_BY_MIPS
void apiGetMfcInfo(void)
{
    BYTE ucTemp;
    WORD u16Temp;

    ucTemp = MDrv_MFC_ReadByte(0x29CF);
    gMfcType_lb.Film22Mode = _bit6_(ucTemp);
    gMfcType_lb.Film32Mode = _bit5_(ucTemp);
    gMfcType_lb.Film22Idx  = _bit4_(ucTemp);
    gMfcType_lb.Film32Idx  = ( ucTemp >> 1 ) & 0x07;
    gMfcType_lb.FDUP       = _bit0_(ucTemp);

    ucTemp = MDrv_MFC_ReadByte(0x29CE);
    gMfcType_lb.FrameEnd =  _bit6_(ucTemp>>1);
    //gMfcType_lb.FrameEnd =  _bit7_(ucTemp);
    gMfcType_lb.Skip_Thd = ( ucTemp >> 4 ) & 0x03;


    u16Temp = MDrv_MFC_Read2Bytes(0x29CC);
    gMfcType_lb.GMVX = u16Temp & 0x00FE;
    gMfcType_lb.GMVY = (u16Temp & 0x7F00) >> 8;
    gMfcType_lb.GMVConfirm = ( ( (u16Temp & 0x0001) << 1) | (u16Temp >> 15));

    u16Temp = MDrv_MFC_Read2Bytes(0x29DA);
    gMfcType_lb.SecondGMVX = u16Temp & 0x00FE;
    gMfcType_lb.SecondGMVY = (u16Temp & 0x7F00) >> 8;

    gMfcType_lb.MinSAD = MDrv_MFC_Read2Bytes(0x29C8);
    gMfcType_lb.MaxSAD = MDrv_MFC_Read2Bytes(0x29CA);
    gMfcType_lb.MaxSAD_MSB = MDrv_MFC_ReadByte(0x29CB);
    gMfcType_lb.MinSAD_MSB = MDrv_MFC_ReadByte(0x29C9);
    gMfcType_lb.maxXNegSpeed = MDrv_MFC_ReadByte(0x29C0);
    gMfcType_lb.maxXPosSpeed = MDrv_MFC_ReadByte(0x29C2);
    gMfcType_lb.maxYNegSpeed = MDrv_MFC_ReadByte(0x29C1);
    gMfcType_lb.maxYPosSpeed = MDrv_MFC_ReadByte(0x29C3);
    gMfcType_lb.GMV0Err = MDrv_MFC_Read2Bytes(0x29DC);
    gMfcType_lb.unMatchPointCnt = MDrv_MFC_Read2Bytes(0x209A);
    gMfcType_lb.ProgrammableStatusCnt1 = MDrv_MFC_Read2Bytes(0x2C60);
    gMfcType_lb.SamePixelCnt = MDrv_MFC_Read2Bytes(0x26D0);
    gMfcType_lb.SecondGmvMinSAD = MDrv_MFC_Read2Bytes(0x29DC);

    gMfcType_lb.bMfcInputFreq = MDrv_MFC_ReadByte(0x2C49);


    gMfcType_lb.MovingBlkCnt = MDrv_MFC_Read2Bytes(0x29EC) & 0x3fff;
    gMfcType_lb.ErrorBlk1 = MDrv_MFC_Read2Bytes(0x29E0) & 0x3fff;
    gMfcType_lb.ErrorBlk2 = MDrv_MFC_Read2Bytes(0x29E2) & 0x3fff;
    gMfcType_lb.ErrorBlk3 = MDrv_MFC_Read2Bytes(0x29E4) & 0x3fff;
    gMfcType_lb.ErrorBlk4 = MDrv_MFC_Read2Bytes(0x29E6) & 0x3fff;
    gMfcType_lb.ErrorBlk5 = MDrv_MFC_Read2Bytes(0x29E8) & 0x3fff;
    gMfcType_lb.ErrorBlk6 = MDrv_MFC_Read2Bytes(0x29EA) & 0x3fff;
    gMfcType_lb.MINonContinuousBoundary = MDrv_MFC_Read2Bytes(0x29DE);
    gMfcType_lb.cplxBlkCnt = MDrv_MFC_Read2Bytes(0x29EE);
    gMfcType_lb.cplxBlkCnt2 = MDrv_MFC_Read2Bytes(0x29D6);
    gMfcType_lb.veryCplxBlkCnt = MDrv_MFC_Read2Bytes(0x29F8);
    gMfcType_lb.ErrorBlkA = MDrv_MFC_Read2Bytes(0x26D8);
    gMfcType_lb.ErrorBlkB = MDrv_MFC_Read2Bytes(0x26DA);
}

void Mfc_Polling(void)
{
    apiGetMfcInfo();
    if (g_MfcPanelVfreq_lb!=60 ) //&& !gMfcType_lb.Film22Mode && !gMfcType_lb.Film32Mode)
        getSamePixelCnt();
    DetectLetterBox();
    ProgramLetterBox();
}
#endif

