#ifndef _RATECTRL_H_
#define _RATECTRL_H_

#ifdef _AEON_PLATFORM_
#ifdef _M4VE_T3_
#else
#include "MsTypes.h"
#endif
#endif
#if defined(_MIPS_PLATFORM_)&&defined(_M4VE_T3_)
#include "mdrv_types.h"
#elif defined(_MIPS_PLATFORM_)
#include <sys/bsdtypes.h>
#endif
#include "Mscommon.h"

//#define _SUPPORT_MBLEVEL_RC_
/*
#ifndef INT64
typedef long long INT64;
#endif
*/
/*
enum RC_METHOD {
    CONST_QUALITY = 1,  //!< Constant qscale: rate control module should not be evolved.
    CONST_BITRATE = 2,  //!< Constant bitrate
    VARIABLE_BITRATE = 3,    //!< Variable bitrate with target average bitrate; 
                                    //!< instant maximal and minimal bitrate is not restricted.
    CONSTRAINED_VARIABLE_BITRATE = 4,    //!< Variable bitrate with target average bitrate; 
                                    //!< instant maximal bitrate is restricted.
};
*/
#define CONST_QUALITY 1
#define CONST_BITRATE 2
#define VARIABLE_BITRATE 3
#define CONSTRAINED_VARIABLE_BITRATE 4

/*
enum RC_GRANULARITY {
    FRAMELEVELRC = 1,   //!< All MB's in one frame uses the same qscale.
    MBLEVELRC = 2       //!< Allow qscale changing within frame, for better target bit-count achievement.
};
*/
#define FRAMELEVELRC 1
#define MBLEVELRC 2

// For CONSTRAINED_VARIABLE_BITRATE
#define MAX_GAUGE_SIZE  60

/* Input for rate control */
typedef struct {
    int nWidth, nHeight;
    float fTargetFrameRate;
    int nBitrate;     // When CONSTRAINED_VARIABLE_BITRATE, 0 means internally-decided.
    int nMaxBitrate;  // Used only when CONSTRAINED_VARIABLE_BITRATE. For others, this should be 0.
    /*bool*/int bFixedFrameRate;
    int nPCount;    // Number of P-frames between I-frames
    /*RC_GRANULARITY*/int rcGranularity;
    /*RC_METHOD*/int rcMethod;
} CVBRRCInfo;

typedef struct {
    /* Input parameters */

    int m_nWidth, m_nHeight;
    float m_fSourceFrameRate, m_fTargetFrameRate;
    int m_nBitrate;
    int m_nMaxBitrate; // Used only when CONSTRAINED_VARIABLE_BITRATE.
    /*bool*/int m_bFixedFrameRate;
    int m_nPCount;
    /*RC_GRANULARITY*/int m_rcGranularity;
    /*RC_METHOD*/int m_rcMethod;
    // Derived variables
    float m_fAvgBitsPerFrame;

    /* rate control variables */

    int m_nFrameCount;  // How many frame coded
    int m_nBufFullness;  // Rate control buffer
    INT64 m_nTotalBits;
    // Last-frame status
    int m_nLastFrameBits, m_nLastFrameAvgQP;
    int m_nLastTargetBits;

    /* Bitrate usage compensation */
    int m_nTargetFullness;
    int m_nDeputyCount, m_nMinDeputyCount;

    /* Variable bitrate */
    float m_fLongTermQP;

    // Model parameters

    int m_nTargetBits;   // target number of bits of current frame
    int m_nN, m_nNFrame;    // number of macroblocks in a frame
    int m_nMBN;

    /* Only for CONSTRAINED_VARIABLE_BITRATE */
    int m_nMaxOffset;
    // Bitrate gauge
    int m_BitrateGauge[MAX_GAUGE_SIZE];
    int m_nGaugeCount, m_nGaugeIndex, m_nGaugeBitrate;
    // Maximal freezed frame
    int m_nMaxFreezedFrame;
} CVBRRateControl;

// public:

// Global
void cvbr_InitRateControl(CVBRRateControl* ct, CVBRRCInfo* pRCInfo);
void cvbr_CloseRateControl(CVBRRateControl* ct);
// Each frame
int cvbr_InitFrame(CVBRRateControl* ct, int nFrameType);    // Return the initial frame qp
int cvbr_UpdateFrame(CVBRRateControl* ct, int totalUsedBits, /*bool*/int bDummyFrame);
// Each macroblock
void cvbr_InitMB(CVBRRateControl* ct, unsigned int MB_Type, int mb_idx);
void cvbr_UpdateMB(CVBRRateControl* ct, int bitsMB, int bitsCoeff, int mb_idx, int QP);

/* Global variables */
extern CVBRRCInfo g_cvbrInfo; // Input parameters, must fill by outsider when calling cvbr_InitRateControl().
extern CVBRRateControl g_cvbrContext; // RC internal variables: Not to be altered by outsider.
extern int g_cvbrFrameSkip; // Filled by cvbr_UpdateFrame()

#endif
