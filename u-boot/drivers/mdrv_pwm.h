////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_PWM_H_
#define _MDRV_PWM_H_

#include <asm/system.h>

/* CMNIO PWM */
/**
 * type definition about PWM device's number
 *
*/
typedef enum {
	PWM_DEV_PIN0	= 0,	/**< PWM# 0 */
	PWM_DEV_PIN1,			/**< PWM# 1 */
	PWM_DEV_PIN2,			/**< PWM# 2 */
	PWM_DEV_PIN3,			/**< PWM# 3 */
	PWM_DEV_MAX,			/**< PWM# MAX */
} PWM_PIN_SEL_T;

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//balup_pwm
#define PWM_VBR_A_INIT_FREQUENCY_VALUE	0x1ff// 1	//KWON_0818 TEMP	100		//150Hz ->100hz
#define PWM_VBR_B_INIT_FREQUENCY_VALUE	0x07//0x1ff// 1		//by hwangbos 100 -> 1		//150Hz
#define PWM_VBR_B_INIT_FREQ_VALUE_200HZ	0	/* requested by yoonmy, 27-FAB-2009 */

#define PWM_VBR_A_INIT_VALUE			0x0f	
#define PWM_VBR_B_INIT_VALUE			 (12000000 / (PWM_VBR_B_INIT_FREQUENCY_VALUE+1)) 

//balup_pwm
#define PWM_VBR_A_START_VALUE		0xff	//KWON_0819 TEMP	0xff
#define PWM_VBR_B_START_VALUE		0xff	//by hwangbos 0x7b -> 0xe9 for temperal
#define PWM_VBR_B_START_VALUE_EDGE	0xfc


#define PWM_VBR_A_INIT_END_VALUE		0x7b//0x08	//KWON_0819 TEMP	0xff
#define PWM_VBR_B_INIT_END_VALUE		0xfc//0xe9	//by hwangbos 0x7b -> 0xe9 for temperal
#define PWM_PORT_VBR_A					PWM_DEV_PIN0	//KWON_0819	PWM_DEV_PIN3
#define PWM_PORT_VBR_B					PWM_DEV_PIN1	//KWON_0819	PWM_DEV_PIN2
//#ifdef PWM2_OUT
#define PWM_PORT_VBR_B2					PWM_DEV_PIN2	//KWON_0819	PWM_DEV_PIN3
//#endif

void DDI_PWM_Init( UINT32 WidthPortB);
void DDI_PQ_PWMInitContol(UINT32 WidthPortB);
#endif // MDRV_PWM_H

