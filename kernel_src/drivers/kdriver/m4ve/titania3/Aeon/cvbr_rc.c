//#include <math.h>
//#include <stdio.h>
#include "drvM4VE.h"
#include "cvbr_rc.h"
//#include "../include/vm_enc_defs.h"

extern U8 u8IPlength;
//#define _DEBUG_RC_
#if defined(_DEBUG) && defined(_DEBUG_RC_)
#pragma message("define _DEBUG_RC_ !!!")
#include <windows.h>
void dprintf(char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}
#define DEBUG_RC(x) dprintf##x
#else
#define DEBUG_RC(x)
#endif

#define MAX_Q  15
#define MIN_Q  1

//! How many seconds of frames are responsible for compensation of bitrate usage.
#define CBR_DEPUTY_SECOND     1
#define CVBR_DEPUTY_SECOND    3
#define VBR_DEPUTY_SECOND     10

#define MIN_DEPUTY_FACTOR 10

//! Default I-frame weighting over inter-frame
#define IFRAME_WEIGHT   3
#define PFRAME_WEIGHT   1
#ifdef WIN32
#define FLOAT float
#else
#define FLOAT double
#endif
#ifdef _SUPPORT_MBLEVEL_RC_
FLOAT RC_buf[MAX_BLOCKNUMX*MAX_BLOCKNUMY*3];
#endif

//////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////
CVBRRateControl g_cvbrContext;
CVBRRCInfo g_cvbrInfo;
int g_cvbrFrameSkip;

//private:
int cvbr_ComputeFrameQP(CVBRRateControl* ct, int nFrameType);
int cvbr_ComputeMbQP(CVBRRateControl* ct, int mb_idx);
void cvbr_rc_init(U16 width, U16 height, double frate, U32 bitrate);
U8 cvbr_rc_vop_before(U16 vop_type);
void cvbr_rc_vop_after(U32 num_bytes);
#if 0
void cvbr_InitRateControl(CVBRRateControl* ct, CVBRRCInfo* pRCInfo)
{
    int i;

    ct->m_nFrameCount = 0;

    // Copy parameters, no error checking yet.
    ct->m_nWidth = pRCInfo->nWidth;
    ct->m_nHeight = pRCInfo->nHeight;
    ct->m_fTargetFrameRate = pRCInfo->fTargetFrameRate;
    ct->m_nBitrate = pRCInfo->nBitrate;
    ct->m_nMaxBitrate = pRCInfo->nMaxBitrate;
    ct->m_bFixedFrameRate = pRCInfo->bFixedFrameRate;
    ct->m_nPCount = pRCInfo->nPCount;
    ct->m_rcGranularity = pRCInfo->rcGranularity;
    ct->m_rcMethod = pRCInfo->rcMethod;
    // More bitrate checking
    if (ct->m_rcMethod==CONSTRAINED_VARIABLE_BITRATE) {
        if (ct->m_nBitrate==0)
            ct->m_nBitrate = (int)(ct->m_nMaxBitrate * 0.6f);
        if (ct->m_nMaxBitrate==0)
            ct->m_nMaxBitrate = (int)(ct->m_nBitrate * 1.4f);
        ct->m_nMaxOffset = (int)((ct->m_nMaxBitrate - ct->m_nBitrate) / ct->m_fTargetFrameRate);
        if (ct->m_rcGranularity==FRAMELEVELRC)
            ct->m_nMaxOffset = (ct->m_nMaxOffset) >> 2;
    }
    else {
        ct->m_nMaxBitrate = 0;  // Don't care
    }
    // Derived
    ct->m_fAvgBitsPerFrame = ((FLOAT)ct->m_nBitrate / ct->m_fTargetFrameRate) *
        (ct->m_nPCount+1) / (IFRAME_WEIGHT + PFRAME_WEIGHT*ct->m_nPCount);

    // Bitrate usage monitoring
    ct->m_nMinDeputyCount = (int)(ct->m_fTargetFrameRate*CBR_DEPUTY_SECOND);
    ct->m_nTargetFullness = ct->m_nBitrate >> 1;
    ct->m_nBufFullness = ct->m_nTargetFullness;
    switch (ct->m_rcMethod) {
    case VARIABLE_BITRATE:
        ct->m_nDeputyCount = (int)(ct->m_fTargetFrameRate*VBR_DEPUTY_SECOND);
        break;
    case CONSTRAINED_VARIABLE_BITRATE:
        ct->m_nDeputyCount = (int)(ct->m_fTargetFrameRate*CVBR_DEPUTY_SECOND);
        ct->m_BitrateGauge[0] = (int)ct->m_fAvgBitsPerFrame*IFRAME_WEIGHT;
        for (i=1; i<MAX_GAUGE_SIZE; i++)
            ct->m_BitrateGauge[i] = (int)ct->m_fAvgBitsPerFrame;
        ct->m_nGaugeCount = (int)(ct->m_fTargetFrameRate);
        ct->m_nGaugeIndex = 0;
        ct->m_nGaugeBitrate = (int)(ct->m_nBitrate / ct->m_fTargetFrameRate * ct->m_nGaugeCount);
        break;
    case CONST_BITRATE:
    default:
        ct->m_nDeputyCount = (int)(ct->m_fTargetFrameRate*CBR_DEPUTY_SECOND);
        break;
    }

    ct->m_nFrameCount = 0;
    ct->m_nTotalBits = 0;
    ct->m_nLastFrameAvgQP = ct->m_nLastFrameBits = 0;
    ct->m_nBufFullness = 0;
    ct->m_fLongTermQP = 0;

    ct->m_nNFrame = (ct->m_nWidth>>4)*(ct->m_nHeight>>4);

    g_cvbrFrameSkip = 0;

    DEBUG_RC(("[CVBRRC] InitRateControl: w=%d, h=%d, FPS=%2.3f, FixedFPS=%d, Bitrate=%d\n",
        ct->m_nWidth, ct->m_nHeight, ct->m_fTargetFrameRate, ct->m_bFixedFrameRate, ct->m_nBitrate));
}


void cvbr_CloseRateControl(CVBRRateControl* ct)
{
}

int cvbr_InitFrame(CVBRRateControl* ct, int nFrameType)
{
    int nDeputyCount;
    int delta=0;
    int nInitQP;

    /* Target frame bitcount */

    if (ct->m_nFrameCount>0) {
        // 1. Determine the number of future frame to compensate for current bitrate mismatch.
        if (ct->m_nFrameCount>ct->m_nDeputyCount*MIN_DEPUTY_FACTOR)
            nDeputyCount = ct->m_nDeputyCount;
        else if (ct->m_nFrameCount<ct->m_nMinDeputyCount)
            nDeputyCount = ct->m_nMinDeputyCount;
        else
            nDeputyCount = ct->m_nMinDeputyCount +
            (ct->m_nFrameCount-ct->m_nMinDeputyCount)*(ct->m_nDeputyCount-ct->m_nMinDeputyCount)/(ct->m_nDeputyCount*MIN_DEPUTY_FACTOR-ct->m_nMinDeputyCount);
        // 2. Calculate the bitcount that this frame should be compensate for.
        if (ct->m_rcMethod==CONST_BITRATE) {
            delta = (ct->m_nBufFullness>ct->m_nTargetFullness) ?
                (int)((ct->m_nBufFullness-ct->m_nTargetFullness)/ct->m_fTargetFrameRate) : ct->m_nBufFullness-ct->m_nTargetFullness;
        }
        else if (ct->m_rcMethod==CONSTRAINED_VARIABLE_BITRATE) {
            delta = (ct->m_nBufFullness-ct->m_nTargetFullness) / nDeputyCount;
            if (delta<-ct->m_nMaxOffset)
                delta = -ct->m_nMaxOffset;
        }
        else if (ct->m_rcMethod==VARIABLE_BITRATE) {
            delta = (ct->m_nBufFullness-ct->m_nTargetFullness) / nDeputyCount;
            if (delta>0 && (ct->m_nLastFrameAvgQP>ct->m_fLongTermQP))
                delta = delta>>1;  // Make it more variable bitrate to allow better quality
        }
        // 3. Finally, calculate the target bitcount.
        if (ct->m_nPCount>0 && nFrameType==I_VOP)
            ct->m_nTargetBits = (int)((ct->m_fAvgBitsPerFrame*IFRAME_WEIGHT) - delta);
        else {
            ct->m_nTargetBits = (int)(ct->m_fAvgBitsPerFrame - delta);
        }
        if (ct->m_nTargetBits<=/*1*/((int)ct->m_fAvgBitsPerFrame>>3))
            ct->m_nTargetBits = /*1*/((int)ct->m_fAvgBitsPerFrame>>3);  // Target bitcount must>0 for ComputeFrameQP()
    }
    else {
        ct->m_nTargetBits = (int)(ct->m_fAvgBitsPerFrame*IFRAME_WEIGHT);    // Must be I-frame
    }

    /* Variable initialization */

    ct->m_nMBN = 1;
    ct->m_nN = ct->m_nNFrame;

    /* Return initial frame QP */

    nInitQP = cvbr_ComputeFrameQP(ct, nFrameType);
    if (ct->m_rcGranularity==FRAMELEVELRC)
        ct->m_nLastFrameAvgQP = nInitQP;
    else
        ct->m_nLastFrameAvgQP = 0;  // Will calculate average value later
    DEBUG_RC(("[CVBRRC] %d#%5d InitFrame: TargetBits %7d InitQP %2d Buffer %7d Deputy=%d\n",
        nFrameType, ct->m_nFrameCount, (int)(ct->m_nTargetBits), nInitQP, ct->m_nBufFullness, nDeputyCount));
    return nInitQP;
}

int cvbr_UpdateFrame(CVBRRateControl* ct, int totalUsedBits, /*bool*/int bDummyFrame)
{
    int frameskip;

    // Update counter
    ct->m_nFrameCount++;
    ct->m_nTotalBits += totalUsedBits;
    ct->m_nLastTargetBits = ct->m_nTargetBits;
    ct->m_nLastFrameBits = totalUsedBits;
    if (!bDummyFrame) {
        //     ct->m_nLastTargetBits = ct->m_nTargetBits;
        //     ct->m_nLastFrameBits = totalUsedBits;
        if (ct->m_rcGranularity==MBLEVELRC)
            ct->m_nLastFrameAvgQP = ct->m_nLastFrameAvgQP / ct->m_nNFrame;
        // Variable bitrate
        if (ct->m_rcMethod==VARIABLE_BITRATE/* || ct->m_rcMethod==CONSTRAINED_VARIABLE_BITRATE*/)
            ct->m_fLongTermQP += (ct->m_nLastFrameAvgQP-ct->m_fLongTermQP) / ct->m_nFrameCount;
        else if (ct->m_rcMethod==CONSTRAINED_VARIABLE_BITRATE) {
            if (ct->m_nFrameCount==1)
                ct->m_fLongTermQP = (FLOAT)ct->m_nLastFrameAvgQP;
            else
                ct->m_fLongTermQP = (ct->m_fLongTermQP*(ct->m_nDeputyCount-1) + ct->m_nLastFrameAvgQP) / ct->m_nDeputyCount;
        }
    }
    if (ct->m_rcMethod==CONSTRAINED_VARIABLE_BITRATE) {
        ct->m_nGaugeBitrate -= ct->m_BitrateGauge[ct->m_nGaugeIndex];
        ct->m_nGaugeBitrate += totalUsedBits;
        ct->m_BitrateGauge[ct->m_nGaugeIndex] = totalUsedBits;
        ct->m_nGaugeIndex++;
        if (ct->m_nGaugeIndex==ct->m_nGaugeCount)
            ct->m_nGaugeIndex = 0;
    }

    // Update buffer status
    ct->m_nBufFullness += (int) (totalUsedBits - ct->m_fAvgBitsPerFrame);

    // Check if next skipped frame(s) needed
    frameskip = 0;
    if (!ct->m_bFixedFrameRate)
    {
        if (!bDummyFrame)
        {
            if (ct->m_rcMethod==CONSTRAINED_VARIABLE_BITRATE) {
                if (ct->m_nLastFrameBits >= (ct->m_nLastTargetBits<<1)) {
                    //frameskip = (ct->m_nLastFrameBits / ct->m_nLastTargetBits)-1;
                    frameskip = (int)((ct->m_nLastFrameBits - ct->m_nLastTargetBits) / ct->m_fAvgBitsPerFrame - 1);
                    if (frameskip<0)
                        frameskip = 0;
                    else if (frameskip>ct->m_nMaxFreezedFrame)
                        frameskip = ct->m_nMaxFreezedFrame;
                }
            }
            else {
                // Actual fullness is updated after encoding dummy-P frame
                int nBufFullness = ct->m_nBufFullness;
                while (nBufFullness > ct->m_nTargetFullness)
                {
                    nBufFullness = (int)(nBufFullness - ct->m_fAvgBitsPerFrame);
                    frameskip += 1;
                }
            }
        }
    }

#ifdef DEBUG
#ifndef _AEON_PLATFORM_
    if (frameskip > 255)
    {
        fprintf (stderr, "Warning: frameskip > 255\n");
    }
#endif
#endif

    /*
    DEBUG_RC(("[CVBRRC]       UpdateFrame(%7d bits, %3d%%) AvgQ=%2d LTQ=%2d Bitrate=%8d frameskip=%2d\n",
    totalUsedBits, (totalUsedBits-ct->m_nTargetBits)*100/ct->m_nTargetBits, ct->m_nLastFrameAvgQP,
    (int)ct->m_fLongTermQP, (int)(ct->m_nTotalBits*ct->m_fTargetFrameRate/ct->m_nFrameCount), frameskip));
    */
    DEBUG_RC(("[CVBRRC]       UpdateFrame(%7d bits, %3d%%) AvgQ=%2d LTQ=%2d Bitrate=%8d frameskip=%2d\n",
        totalUsedBits, (totalUsedBits-ct->m_nTargetBits)*100/ct->m_nTargetBits, ct->m_nLastFrameAvgQP,
        (int)ct->m_fLongTermQP, ct->m_nGaugeBitrate, frameskip));

    return frameskip;
}

#define SMOOTH_PERIOD 1.0f
int cvbr_ComputeFrameQP(CVBRRateControl* ct, int nFrameType)
{
    int newQP=0, dQP;
    FLOAT buf_rest;
    int buf_rest_pic;
    int frames_left;
    int nAdjust;

    // For the very first frame, guess one qp!
    if (ct->m_nFrameCount==0) {
        int nbpMb;
        nbpMb = (int)(ct->m_nBitrate/(10*ct->m_nNFrame*ct->m_fTargetFrameRate));
        if (nbpMb>100) {
            ct->m_nMaxFreezedFrame = 1;
            return 1;
        }
        else if (nbpMb>20) {
            ct->m_nMaxFreezedFrame = 2;
            return 4;
        }
        else if (nbpMb>10) {
            ct->m_nMaxFreezedFrame = 3;
            return 8;
        }
        else if (nbpMb>6) {
            ct->m_nMaxFreezedFrame = 5;
            return 10;//12;
        }
        else if (nbpMb>4) {
            ct->m_nMaxFreezedFrame = 8;
            return 12;//16;
        }
        else {
            ct->m_nMaxFreezedFrame = 15;
            return 12;//20;
        }
    }

    if (ct->m_rcMethod==CONST_BITRATE) {
        buf_rest = (((FLOAT)(ct->m_nFrameCount)/ct->m_fTargetFrameRate+SMOOTH_PERIOD) * ct->m_nBitrate) - ct->m_nTotalBits;
        newQP = ct->m_nLastFrameAvgQP;
        frames_left = (int)(SMOOTH_PERIOD * ct->m_fTargetFrameRate);
        //if (frames_left > 0)
        {
            buf_rest_pic = (int)(buf_rest / frames_left);
            dQP = MAX (MIN_Q, ct->m_nLastFrameAvgQP>>3);
            if (ct->m_nLastFrameBits > (buf_rest_pic*9)>>3) {
                newQP = MIN (MAX_Q, ct->m_nLastFrameAvgQP+dQP);
            }
            else if (ct->m_nLastFrameBits < (buf_rest_pic*7)>>3) {
                newQP = MAX (MIN_Q, ct->m_nLastFrameAvgQP-dQP);
            }
        }
    }
    else if (ct->m_rcMethod==CONSTRAINED_VARIABLE_BITRATE) {
        int nLowQ, nHighQ;
        int nLowBound, nHighBound;
        nAdjust = MAX(2, (int)(ct->m_fLongTermQP)>>2);
        nLowBound = (int)(ct->m_fLongTermQP) - nAdjust;
        nHighBound = (int)(ct->m_fLongTermQP) + nAdjust;
        if (ct->m_nPCount>0 && nFrameType==I_VOP) {
            newQP = (int)(ct->m_nLastFrameAvgQP);
            if (ct->m_nGaugeBitrate<ct->m_nBitrate)
                newQP = newQP-1;
            newQP = MAX(1, newQP);
            newQP = MIN(12, newQP);
        }
        else {
            int nAdjLTQ;
            nAdjLTQ = (int)(ct->m_fLongTermQP * (ct->m_nTotalBits/ct->m_nFrameCount) / ct->m_nTargetBits);
            newQP = MIN(MAX_Q, MAX(MIN_Q, nAdjLTQ));
            if (ct->m_nLastFrameBits>ct->m_nLastTargetBits) {
                nAdjust = ((ct->m_nLastFrameBits-ct->m_nLastTargetBits)/ct->m_nMaxOffset) + 1;
                if (nAdjust>3) nAdjust=3;
                if (ct->m_nLastFrameAvgQP>ct->m_fLongTermQP) {  // Danger! Make it more aggressive
                    nHighBound = ct->m_nLastFrameAvgQP+nAdjust;
                    newQP = ct->m_nLastFrameAvgQP+nAdjust;
                }
                else {
                    nHighBound += nAdjust;
                    newQP += nAdjust;
                }
            }
            else if (ct->m_nGaugeBitrate>ct->m_nBitrate) {
                if (newQP<ct->m_nLastFrameAvgQP)
                    newQP = ct->m_nLastFrameAvgQP;
                if (newQP<(int)ct->m_fLongTermQP)
                    newQP = (int)ct->m_fLongTermQP;
            }
            else {
                if (newQP>=(int)ct->m_fLongTermQP)
                    newQP = (int)ct->m_fLongTermQP-1;
                if (ct->m_nTargetFullness>ct->m_nBufFullness) {
                    nAdjust = (ct->m_nTargetFullness-ct->m_nBufFullness) / (int)ct->m_nBitrate;
                    newQP -= nAdjust;
                }
            }
        }
        nHighQ = MIN(MAX_Q, nHighBound);
        newQP = MIN(nHighQ, newQP);
        nLowQ = MAX(MIN_Q, nLowBound);
        newQP = MAX(nLowQ, newQP);
    }
    else if (ct->m_rcMethod==VARIABLE_BITRATE) {
        int nLowQ, nHighQ;
        int nLowBound, nHighBound;
        if (ct->m_nPCount>0 && nFrameType==I_VOP) {
            newQP = (int)(ct->m_nLastFrameAvgQP);
            if (ct->m_nLastFrameAvgQP>ct->m_fLongTermQP)
                newQP = newQP-1;
            newQP = MAX(MIN_Q, newQP);
        }
        else {
            int nAdjLTQ;
            if (ct->m_nFrameCount>=(int)(ct->m_fTargetFrameRate) || ct->m_nPCount==0) {
                nAdjLTQ = (int)(ct->m_fLongTermQP * (ct->m_nTotalBits/ct->m_nFrameCount) / ct->m_fAvgBitsPerFrame);
                nAdjLTQ = MIN(MAX_Q, MAX(MIN_Q, nAdjLTQ));
            }
            else {
                nAdjLTQ = (int)(ct->m_fLongTermQP);   // Wait for stabilization
            }
            ASSERT(ct->m_nTargetBits>0);
            newQP = (int)(nAdjLTQ * ct->m_fAvgBitsPerFrame / ct->m_nTargetBits);

            nAdjust = MAX(2, (int)(nAdjLTQ)>>2);
            nLowBound = (int)(nAdjLTQ) - nAdjust;
            nHighBound = (int)(nAdjLTQ) + nAdjust;
            if (ct->m_nLastFrameBits>ct->m_nLastTargetBits) {
                nAdjust = (int)(ct->m_nLastFrameBits/ct->m_nLastTargetBits);
                if (nAdjust>2) nAdjust=2;
                nHighBound += nAdjust;
            }

            if (ct->m_fAvgBitsPerFrame>ct->m_nTargetBits) {
                nHighQ = MIN(MAX_Q, nHighBound);
                newQP = MIN(nHighQ, newQP);
            }
            else {
                nLowQ = MAX(MIN_Q, nLowBound);
                newQP = MAX(nLowQ, newQP);
            }
        }
    }

    return newQP;
}

void cvbr_InitMB(CVBRRateControl* ct, unsigned int MB_Type, int mb_idx)
{
#ifdef _SUPPORT_MBLEVEL_RC_
    // TODO!
#endif // _SUPPORT_MBLEVEL_RC_
}

void cvbr_UpdateMB(CVBRRateControl* ct, int bitsMB, int bitsCoeff, int mb_idx, int QP)
{
    if (ct->m_rcGranularity==FRAMELEVELRC) {
        ct->m_nN--;
        ct->m_nMBN++;
        return;
    }

#ifdef _SUPPORT_MBLEVEL_RC_
    // TODO!
#endif // _SUPPORT_MBLEVEL_RC_
}

int cvbr_ComputeMbQP(CVBRRateControl* ct, int mb_idx)
{
    int newQP = 0;

#ifdef _SUPPORT_MBLEVEL_RC_
    // TODO!
#endif // _SUPPORT_MBLEVEL_RC_

    return newQP;
}


/////////////////////////////////////////////////////////////////////////////////////////

void cvbr_rc_init(U16 width, U16 height, double frate, U32 bitrate)
{
    g_cvbrInfo.nWidth = width;
    g_cvbrInfo.nHeight = height;
    g_cvbrInfo.fTargetFrameRate = frate;

    g_cvbrInfo.nMaxBitrate=bitrate;
    //g_cvbrInfo.nMaxBitrate = 0;   // Let RC decide
    g_cvbrInfo.nBitrate = 0;   // Let RC decide

    g_cvbrInfo.bFixedFrameRate = 0;
    g_cvbrInfo.nPCount = u8IPlength-1;    // Number of P-frames between I-frames
    g_cvbrInfo.rcMethod = CONSTRAINED_VARIABLE_BITRATE;//VARIABLE_BITRATE;
    g_cvbrInfo.rcGranularity = FRAMELEVELRC;
    cvbr_InitRateControl(&g_cvbrContext, &g_cvbrInfo);
}

U8 cvbr_rc_vop_before(U16 vop_type)
{
    return cvbr_InitFrame(&g_cvbrContext, vop_type);
}

void cvbr_rc_vop_after(U32 num_bytes)
{
    if (g_cvbrFrameSkip>0) {
        cvbr_UpdateFrame(&g_cvbrContext, num_bytes<<3, 1);
        g_cvbrFrameSkip--;
    }
    else {
        g_cvbrFrameSkip =
            cvbr_UpdateFrame(&g_cvbrContext, num_bytes<<3, 0);
    }
}
#endif //if 0
