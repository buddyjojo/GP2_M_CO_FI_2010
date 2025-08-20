////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////
//
// File Name: BD_MST052D-D01A_S.H
// Description: Customization and Specialization for this board!
// Revision History:
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _MSBOARD_H_
#define _MSBOARD_H_

#include "board.h"
#include "hwreg.h"

//------------------------------MS_BOARD_TYPE_SEL-------------------------------
#define BOARD_NAME              "BD_S6_VENUS"
#define BOARD_SUB_TYPE          BD_S6_T2U3//BD_S6_T2U1

#define BD_S6_T2U1              0x0001
#define BD_S6_T2U2              0x0002
#define BD_S6_T2U3              0x0003


//#define LGE_GP_COMMERCIAL_MODEL

//------Peripheral Device Setting-----------------------------------------------



//------GPIO setting------------------------------------------------------------
//------PM0---------------------------------------------------------------------
#define GPIO_PM0_INPUT()    GPIO_PM0_DIR(1)
#define GPIO_PM0_OUTPUT()   GPIO_PM0_DIR(0)
#define GPIO_PM0_OUTPUT_HIGH()    do{\
                            GPIO_PM0_DIR(0);\
                            GPIO_PM0_SET(1);\
                            }while(0)
#define GPIO_PM0_OUTPUT_LOW()    do{\
                            GPIO_PM0_DIR(0);\
                            GPIO_PM0_SET(0);\
                            }while(0)
//WARM_LED
//for LED & PDP
#define WARM_LED_DISABLE()    _FUNC_NOT_USED()
#define WARM_LED_ENABLE()     _FUNC_NOT_USED()
#define WARM_LED_ON()   _FUNC_NOT_USED()
#define WARM_LED_OFF()    _FUNC_NOT_USED()

//------PM1---------------------------------------------------------------------
#define GPIO_PM1_INPUT()     GPIO_PM1_DIR(1)
#define GPIO_PM1_OUTPUT()    GPIO_PM1_DIR(0)
#define GPIO_PM1_OUTPUT_HIGH()    do{\
                            GPIO_PM1_DIR(0);\
                            GPIO_PM1_SET(1);\
                            }while(0)
#define GPIO_PM1_OUTPUT_LOW()    do{\
                            GPIO_PM1_DIR(0);\
                            GPIO_PM1_SET(0);\
                            }while(0)


#if defined(LGE_GP_COMMERCIAL_MODEL)
#define UART_TX_ENABLE()    GPIO_PM1_OUTPUT() //Let GPIO_PM1 to be output mode for UART TX
#define UART_TX_DISABLE()   GPIO_PM1_INPUT() //Let GPIO_PM1 to be input mode for disable UART TX
#else
//AC_DET for LCD
#define AC_DET_DISABLE()    GPIO_PM1_OUTPUT()
#define AC_DET_ENABLE()     GPIO_PM1_INPUT()
#define AC_IS_DET()         (GPIO_PM1_BIT() != GPIO_PM1_GET())
#define UART_TX_ENABLE()    GPIO_PM1_OUTPUT() //Let GPIO_PM1 to be output mode for UART TX
#endif


//------PM2---------------------------------------------------------------------
#define GPIO_PM2_INPUT()     GPIO_PM2_DIR(1)
#define GPIO_PM2_OUTPUT()    GPIO_PM2_DIR(0)
#define GPIO_PM2_OUTPUT_HIGH()    do{\
                            GPIO_PM2_DIR(0);\
                            GPIO_PM2_SET(1);\
                            }while(0)
#define GPIO_PM2_OUTPUT_LOW()    do{\
                            GPIO_PM2_DIR(0);\
                            GPIO_PM2_SET(0);\
                            }while(0)
//INV_CTRL for LCD
#define INV_CTL_DISABLE()    GPIO_PM2_INPUT()
#define INV_CTL_ENABLE()     GPIO_PM2_OUTPUT()
#define INV_CTL_ON()    GPIO_PM2_OUTPUT_LOW()
#define INV_CTL_OFF()    GPIO_PM2_OUTPUT_HIGH()

//VAVS_ON for PDP
#define VAVS_ON_DISABLE()    GPIO_PM2_INPUT()
#define VAVS_ON_ENABLE()     GPIO_PM2_OUTPUT()
#define VAVS_ON_ON()    GPIO_PM2_OUTPUT_HIGH()
#define VAVS_ON_OFF()    GPIO_PM2_OUTPUT_LOW()

#define VAVS_ON_HIGH()	    GPIO_PM2_SET(1);
#define VAVS_ON_LOW()    	GPIO_PM2_SET(0);

//------PM3---------------------------------------------------------------------
#define GPIO_PM3_INPUT()     GPIO_PM3_DIR(1)
#define GPIO_PM3_OUTPUT()    GPIO_PM3_DIR(0)
#define GPIO_PM3_OUTPUT_HIGH()    do{\
                            GPIO_PM3_DIR(0);\
                            GPIO_PM3_SET(1);\
                            }while(0)
#define GPIO_PM3_OUTPUT_LOW()    do{\
                            GPIO_PM3_DIR(0);\
                            GPIO_PM3_SET(0);\
                            }while(0)
//PANEL_ON for LCD
#define PANEL_CTL_DISABLE()    GPIO_PM3_INPUT()
#define PANEL_CTL_ENABLE()     GPIO_PM3_OUTPUT()
#define PANEL_CTL_ON()    GPIO_PM3_OUTPUT_LOW()
#define PANEL_CTL_OFF()    GPIO_PM3_OUTPUT_HIGH()
//DISP_EN for PDP
#define DISP_EN_DISABLE()    GPIO_PM3_INPUT()
#define DISP_EN_ENABLE()     GPIO_PM3_OUTPUT()
#define DISP_EN_ON()    GPIO_PM3_OUTPUT_HIGH()
#define DISP_EN_OFF()    GPIO_PM3_OUTPUT_LOW()



//------PM4---------------------------------------------------------------------
#define GPIO_PM4_INPUT()     GPIO_PM4_DIR(1)
#define GPIO_PM4_OUTPUT()    GPIO_PM4_DIR(0)
#define GPIO_PM4_OUTPUT_HIGH()    do{\
                            GPIO_PM4_DIR(0);\
                            GPIO_PM4_SET(1);\
                            }while(0)
#define GPIO_PM4_OUTPUT_LOW()    do{\
                            GPIO_PM4_DIR(0);\
                            GPIO_PM4_SET(0);\
                            }while(0)
//POWER_ON_OFF for LCD & PDP
#define POWER_ON()    GPIO_PM4_INPUT()
#define POWER_OFF()    GPIO_PM4_OUTPUT_LOW()



//------PM5---------------------------------------------------------------------
#define GPIO_PM5_INPUT()     GPIO_PM5_DIR(1)
#define GPIO_PM5_OUTPUT()    GPIO_PM5_DIR(0)
#define GPIO_PM5_OUTPUT_HIGH()    do{\
                            GPIO_PM5_DIR(0);\
                            GPIO_PM5_SET(1);\
                            }while(0)
#define GPIO_PM5_OUTPUT_LOW()    do{\
                            GPIO_PM5_DIR(0);\
                            GPIO_PM5_SET(0);\
                            }while(0)
//DBG_RXD for LCD & PDP
#define DBG_RXD_DISABLE()    GPIO_PM5_OUTPUT()
#define DBG_RXD_ENABLE()     GPIO_PM5_INPUT()




//------PM6---------------------------------------------------------------------
#define GPIO_PM6_INPUT()     GPIO_PM6_DIR(1)
#define GPIO_PM6_OUTPUT()    GPIO_PM6_DIR(0)
#define GPIO_PM6_OUTPUT_HIGH()    do{\
                            GPIO_PM6_DIR(0);\
                            GPIO_PM6_SET(1);\
                            }while(0)
#define GPIO_PM6_OUTPUT_LOW()    do{\
                            GPIO_PM6_DIR(0);\
                            GPIO_PM6_SET(0);\
                            }while(0)

#if defined(LGE_GP_COMMERCIAL_MODEL)
//AC_DET for LCD
#define AC_DET_DISABLE()    GPIO_PM6_OUTPUT()
#define AC_DET_ENABLE()     GPIO_PM6_INPUT()
#define AC_IS_DET()         (GPIO_PM6_BIT() != GPIO_PM6_GET())
#else
//ISP_TXD for LCD & PDP
#define ISP_TXD_DISABLE()    GPIO_PM6_OUTPUT()
#define ISP_TXD_ENABLE()     GPIO_PM6_INPUT()
#endif


//------SA0--------------------------------------------------------------------
#define GPIO_SAR0_INPUT()    GPIO_SAR0_DIR(1)
#define GPIO_SAR0_OUTPUT()   GPIO_SAR0_DIR(0)
#define GPIO_SAR0_OUTPUT_HIGH()    do{\
                            GPIO_SAR0_DIR(0);\
                            GPIO_SAR0_SET(1);\
                            }while(0)
#define GPIO_SAR0_OUTPUT_LOW()    do{\
                            GPIO_SAR0_DIR(0);\
                            GPIO_SAR0_SET(0);\
                            }while(0)
//KEY1 for LCD & PDP
#define KEY1_DISABLE()    GPIO_SAR0_OUTPUT()
#define KEY1_ENABLE()     GPIO_SAR0_INPUT()




//------SA1--------------------------------------------------------------------
#define GPIO_SAR1_INPUT()    GPIO_SAR1_DIR(1)
#define GPIO_SAR1_OUTPUT()   GPIO_SAR1_DIR(0)
#define GPIO_SAR1_OUTPUT_HIGH()    do{\
                            GPIO_SAR1_DIR(0);\
                            GPIO_SAR1_SET(1);\
                            }while(0)
#define GPIO_SAR1_OUTPUT_LOW()    do{\
                            GPIO_SAR1_DIR(0);\
                            GPIO_SAR1_SET(0);\
                            }while(0)
//KEY2 for LCD & PDP
#define KEY2_DISABLE()    GPIO_SAR1_OUTPUT()
#define KEY2_ENABLE()     GPIO_SAR1_INPUT()




//------SA2--------------------------------------------------------------------
#define GPIO_SAR2_INPUT()    GPIO_SAR2_DIR(1)
#define GPIO_SAR2_OUTPUT()   GPIO_SAR2_DIR(0)
#define GPIO_SAR2_OUTPUT_HIGH()    do{\
                            GPIO_SAR2_DIR(0);\
                            GPIO_SAR2_SET(1);\
                            }while(0)
#define GPIO_SAR2_OUTPUT_LOW()    do{\
                            GPIO_SAR2_DIR(0);\
                            GPIO_SAR2_SET(0);\
                            }while(0)
//SCART2_ID for LCD & PDP
#define SCART2_ID_DISABLE()    GPIO_SAR2_OUTPUT()
#define SCART2_ID_ENABLE()     GPIO_SAR2_INPUT()



//------SA3--------------------------------------------------------------------
#define GPIO_SAR3_INPUT()    GPIO_SAR3_DIR(1)
#define GPIO_SAR3_OUTPUT()   GPIO_SAR3_DIR(0)
#define GPIO_SAR3_OUTPUT_HIGH()    do{\
                            GPIO_SAR3_DIR(0);\
                            GPIO_SAR3_SET(1);\
                            }while(0)
#define GPIO_SAR3_OUTPUT_LOW()    do{\
                            GPIO_SAR3_DIR(0);\
                            GPIO_SAR3_SET(0);\
                            }while(0)
//SB_MUTE for LCD
#define SB_MUTE_DISABLE()    GPIO_SAR3_INPUT()
#define SB_MUTE_ENABLE()     GPIO_SAR3_OUTPUT()
#define SB_MUTE_OFF()    GPIO_SAR3_OUTPUT_LOW()
#define SB_MUTE_ON()    GPIO_SAR3_OUTPUT_HIGH()


//------MST Keypad definition---------------------------------------------------



#endif // _MSBOARD_H_
