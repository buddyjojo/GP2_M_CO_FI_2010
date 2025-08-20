
#ifndef _MDRV_DEMOD_ST_H_
#define _MDRV_DEMOD_ST_H_

#include "mdrv_types.h"

typedef struct DEMOD_REG_s
{
    U32 u32Address;
    U8  u8Value;
} DEMOD_REG_t, *pDEMOD_REG_t;
//mstar 0901 update
typedef enum DEMOD_STANDARD_TYPE_e
{
    DEMOD_STANDARD_UNKNOWN = 0,
    DEMOD_STANDARD_DVBT,
    DEMOD_STANDARD_DVBC,
    DEMOD_STANDARD_ATSC,
} DEMOD_STANDARD_TYPE_t;
// 0402 change for tuner option
typedef enum TUNER_TYPE_e
{
    TUNER_TYPE_UNKNOWN = 0,
    TUNER_TYPE_INNOTEK,
    TUNER_TYPE_SANYO,
} TUNER_TYPE_t;
// 0402 change for tuner option
typedef struct DEMOD_DSP_PARAM_s
{
    DEMOD_STANDARD_TYPE_t eDSP_Type;
    TUNER_TYPE_t  eTuner_Type;
} DEMOD_DSP_PARAM_t, *pDEMOD_DSP_PARAM_t;

#endif

