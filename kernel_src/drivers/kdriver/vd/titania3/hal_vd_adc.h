#ifndef __HAL_VD_ADC_H__
#define __HAL_VD_ADC_H__

#include "hal_vd_types.h"

/******************************************************************************/
/*                           Constant                                         */
/******************************************************************************/
#define  ADC_POWERINIT_MSK_04_L      (_BIT7 | _BIT6 | _BIT0)
#define  ADC_POWERINIT_MSK_04_H      (_BIT0)
#define  ADC_POWERINIT_MSK_05_L      (_BIT6 | _BIT5 | _BIT2)
#define  ADC_POWERINIT_MSK_06_L      (_BIT0)
#define  ADC_POWERINIT_MSK_06_H      (_BIT4 | _BIT2)
#define  ADC_POWERINIT_MSK_08_L      (_BIT4)
#define  ADC_POWERINIT_MSK_60_L      (_BIT6 | _BIT4)
#define  ADC_POWERINIT_MSK_60_H      (0x00)
#define  ADC_POWERINIT_MSK_69_L      (_BIT6 | _BIT4)
#define  ADC_POWERINIT_MSK_69_H      (0x00)

//------------------------------------------------------------------------------
// ADC
//
typedef enum
{
    MS_ADC_A_POWER_ON,
    MS_ADC_B_POWER_ON,
    MS_VDB_CVBS_POWER_ON,
    MS_VDB_SV_POWER_ON,
    MS_VDB_FBLANK_POWER_ON,
    MS_VDA_CVBS_POWER_ON,
    MS_VDA_SV_POWER_ON,
    MS_VDA_FBLANK_POWER_ON,
    MS_DVI_POWER_ON,
    MS_ADC_VD_BLEND_POWER_ON,
    MS_ADC_POWER_ALL_OFF,
} MS_ADC_POWER_ON_TYPE;


//------------------------------------------------------------------------------
// ADC

#define ADC_AMUXA_MASK      BITMASK(1:0)
#define ADC_YMUX_MASK       BITMASK(3:0)
#define ADC_CMUX_MASK       BITMASK(7:4)

typedef enum
{
    En_ADC_A    =  0x01,
    En_ADC_B    =  0x02,
    En_DVI      =  0x04,
    En_VD       =  0x08,
    En_VD_YC    =  0x10,
    En_FB_RGB   =  0x20,
    En_ADC_AMUX =  0x40,
    EN_ADC_FB   =  0x80,
    Mask_VD_En  =  En_VD|En_VD_YC|En_FB_RGB,
} ADC_EN_TYPE;


/* SAR */

#define MAX_BACKLIGHT                   100
#define SAR_ADC_CHANNEL_DATA_MASK       0x3F

extern void
MHal_VD_ADC_PowerOn(MS_ADC_POWER_ON_TYPE enADCPowerType);

extern MS_ADC_POWER_ON_TYPE
MHal_VD_ADC_SetMUX(INPUT_PORT_TYPE enInputPortType, U8 u8VDYMux, U8 u8VDCMux);

extern B16 MHal_VD_ADC_GetScart1IDLevel(U32 *pu32Scart1IDLevel);
extern U32 MHal_VD_ADC_GetScart2IDLevel(void);


#endif

