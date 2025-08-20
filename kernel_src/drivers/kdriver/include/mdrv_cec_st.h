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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_cec_st.h
/// @brief  CEC(Consumer Electronics Control) Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Data structure definition
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_CEC_ST_H_
#define _MDRV_CEC_ST_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mhal_cec_reg.h"

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//for CEC
typedef enum _CEC_DEVICELA
{
    E_LA_TV               =0,
    E_LA_RECORDER1       =1,
    E_LA_RECORDER2       =2,
    E_LA_TUNER1          =3,
    E_LA_PLAYBACK1       =4,
    E_LA_AUDIO_SYS       =5,
    E_LA_TUNER2          =6,
    E_LA_TUNER3          =7,
    E_LA_PLAYBACK2       =8,
    E_LA_RECORER3        =9,
#if(HDMI_CEC_VERSION == CEC_VERSION_13a)
    E_LA_TUNER4          =10,
    E_LA_PLYBACK3        =11,
#endif
    E_LA_FREE_USE        =14,
    E_LA_UNREGISTERED    =15,
    E_LA_BROADCAST       =15,
} CEC_DEVICELA;

typedef enum _CEC_DEVICE_TYPE
{
    E_DEVICE_TYPE_TV                  =0,
    E_DEVICE_TYPE_RECORDING_DEVICE  =1,
    E_DEVICE_TYPE_RESERVED           =2,
    E_DEVICE_TYPE_TUNER              =3,
    E_DEVICE_TYPE_PLAYBACK_DEVICE   =4,
    E_DEVICE_TYPE_AUDIO_SYSTEM      =5,
} CEC_DEVICE_TYPE;

//Power status 1 byte
typedef enum _CEC_MSG_POWER_STATUS_PARM
{
    E_MSG_PWRSTA_ON            = 0x00,
    E_MSG_PWRSTA_STANDBY      = 0x01,
    E_MSG_PWRSTA_STANDBY2ON   = 0x02,
    E_MSG_PWRSTA_ON2STANDBY   = 0x03,
} CEC_MSG_POWER_STATUS_PARM;

typedef enum _CEC_ERROR_CODE
{
    E_CEC_FEATURE_ABORT = 0,
    E_CEC_RX_SUCCESS    = BIT0,
    E_CEC_TX_SUCCESS    = BIT1,
    E_CEC_RF             = BIT2,
    E_CEC_LOST_ABT      = BIT3,
    E_CEC_BIT_SHORT     = BIT4,
    E_CEC_BIT_LONG      = BIT5,
    E_CEC_NACK           = BIT6,
    E_CEC_SYSTEM_BUSY   = BIT7,
} CEC_ERROR_CODE;

//the opcode is referenced from CEC1.3a table 7 ~ 27
typedef enum _MsCEC_MSGLIST
{
//----- One Touch Play ----------------------------
    E_MSG_ACTIVE_SOURCE                 = 0x82,
    E_MSG_OTP_IMAGE_VIEW_ON            = 0x04,
    E_MSG_OTP_TEXT_VIEW_ON             = 0x0D,

//----- Routing Control ---------------------------
    //E_MSG_RC_ACTIVE_SOURCE             = 0x82,
    E_MSG_RC_INACTIVE_SOURCE           =  0x9D,
    E_MSG_RC_REQ_ACTIVE_SOURCE         = 0x85,
    E_MSG_RC_ROUTING_CHANGE            = 0x81,
    E_MSG_RC_ROUTING_INFO              = 0x80,
    E_MSG_RC_SET_STREM_PATH            = 0x86,

//----- Standby Command ---------------------------
    E_MSG_STANDBY                       = 0x36,

//----- One Touch Record---------------------------
    E_MSG_OTR_RECORD_ON                = 0x09,
    E_MSG_OTR_RECORD_OFF               = 0x0B,
    E_MSG_OTR_RECORD_STATUS           = 0x0A,
    E_MSG_OTR_RECORD_TV_SCREEN        = 0x0F,

//----- Timer programmer -------------------------- CEC1.3a
    E_MSG_TP_CLEAR_ANALOG_TIMER       = 0x33,
    E_MSG_TP_CLEAR_DIGITAL_TIMER      =  0x99,
    E_MSG_TP_CLEAR_EXT_TIMER           = 0xA1,
    E_MSG_TP_SET_ANALOG_TIMER          = 0x34,
    E_MSG_TP_SET_DIGITAL_TIMER         = 0x97,
    E_MSG_TP_SET_EXT_TIMER             = 0xA2,
    E_MSG_TP_SET_TIMER_PROGRAM_TITLE  = 0x67,
    E_MSG_TP_TIMER_CLEARD_STATUS      =  0x43,
    E_MSG_TP_TIMER_STATUS              = 0x35,

//----- System Information ------------------------
    E_MSG_SI_CEC_VERSION               = 0x9E,       //1.3a
    E_MSG_SI_GET_CEC_VERSION           = 0x9F,       //1.3a

    E_MSG_SI_REQUEST_PHY_ADDR          = 0x83,
    E_MSG_SI_REPORT_PHY_ADDR           = 0x84,
    E_MSG_SI_GET_MENU_LANGUAGE         = 0x91,
    E_MSG_SI_SET_MENU_LANGUAGE         = 0x32,
    //E_MSG_SI_POLLING_MESSAGE                  = ?,

    //E_MSG_SI_REC_TYPE_PRESET            = 0x00,  //parameter   ?
    //E_MSG_SI_REC_TYPE_OWNSRC            =  0x01,  //parameter   ?

//----- Deck Control Feature-----------------------
    E_MSG_DC_DECK_CTRL                  = 0x42,
    E_MSG_DC_DECK_STATUS                = 0x1B,
    E_MSG_DC_GIVE_DECK_STATUS          = 0x1A,
    E_MSG_DC_PLAY                        = 0x41,

//----- Tuner Control ------------------------------
    E_MSG_TC_GIVE_TUNER_STATUS         = 0x08,
    E_MSG_TC_SEL_ANALOG_SERVICE        = 0x92,
    E_MSG_TC_SEL_DIGITAL_SERVICE       = 0x93,
    E_MSG_TC_TUNER_DEVICE_STATUS       = 0x07,
    E_MSG_TC_TUNER_STEP_DEC            = 0x06,
    E_MSG_TC_TUNER_STEP_INC            = 0x05,

//---------Vendor Specific -------------------------
    //E_MSG_VS_CEC_VERSION                     = 0x9E,       //1.3a
    //E_MSG_VS_GET_CEC_VERSION                 = 0x9F,       //1.3a
    E_MSG_VS_DEVICE_VENDOR_ID           = 0x87,
    E_MSG_VS_GIVE_VENDOR_ID             = 0x8C,
    E_MSG_VS_VENDOR_COMMAND             = 0x89,
    E_MSG_VS_VENDOR_COMMAND_WITH_ID    = 0xA0,      //1.3a
    E_MSG_VS_VENDOR_RC_BUT_DOWN         = 0x8A,
    E_MSG_VS_VENDOR_RC_BUT_UP           = 0x8B,

//----- OSD Display --------------------------------
    E_MSG_SET_OSD_STRING                = 0x64,

//----- Device OSD Name Transfer  -------------------------
    E_MSG_OSDNT_GIVE_OSD_NAME           = 0x46,
    E_MSG_OSDNT_SET_OSD_NAME            = 0x47,

//----- Device Menu Control ------------------------
    E_MSG_DMC_MENU_REQUEST              = 0x8D,
    E_MSG_DMC_MENU_STATUS               = 0x8E,
    //E_MSG_DMC_MENU_ACTIVATED                 = 0x00,   //parameter
    //E_MSG_DMC_MENU_DEACTIVATED             = 0x01,   //parameter
    E_MSG_UI_PRESS                       = 0x44,
    E_MSG_UI_RELEASE                     = 0x45,

//----- Remote Control Passthrough ----------------
//----- UI Message --------------------------------
//#define UI_PRESS             0x44
//#define UI_RELEASE           0x45

//----- Power Status  ------------------------------
    E_MSG_PS_GIVE_POWER_STATUS          = 0x8F,
    E_MSG_PS_REPORT_POWER_STATUS        = 0x90,

//----- General Protocal Message ------------------
//----- Abort Message -----------------------------
    E_MSG_ABORT_MESSAGE                  =  0xFF,
//----- Feature Abort -----------------------------
    E_MSG_FEATURE_ABORT                  = 0x00,

//----- System Audio Control ------------------
    E_MSG_SAC_GIVE_AUDIO_STATUS                = 0x71,
    E_MSG_SAC_GIVE_SYSTEM_AUDIO_MODE_STATUS   = 0x7D,
    E_MSG_SAC_REPORT_AUDIO_STATUS              = 0x7A,
    E_MSG_SAC_SET_SYSTEM_AUDIO_MODE            = 0x72,
    E_MSG_SAC_SYSTEM_AUDIO_MODE_REQUEST       = 0x70,
    E_MSG_SAC_SYSTEM_AUDIO_MODE_STATUS        = 0x7E,

//----- System Audio Control ------------------
    E_MSG_SAC_SET_AUDIO_RATE                   = 0x9A,
} CEC_MSGLIST;

typedef enum _MsCEC_MSG_TRANS_TYPE
{
    E_TRANS_BROADCAST_MSG       = 0,
    E_TRANS_DIRECT_MSG          = 1,
    E_TRANS_BOTHTYPE_MSG        = 2,
} CEC_MSG_TRANS_TYPE;

typedef struct _MDrv_Cec_RxData_Info
{
    U8 ucLength;
    U8 tRxData[16];
} Cec_RxData_Info_t;

typedef struct _MDrv_CEC_INFO_LIST
{
    U8 CecFifoIdxS;
    U8 CecFifoIdxE;
    U8 bCecMsgCnt;
    U16 fCecInitFinish;
    Cec_RxData_Info_t CecRxBuf[CEC_FIFO_CNT];   //TV CEC H/W part
    CEC_DEVICELA MyLogicalAddress;                         //TV related
    U8 MyPhysicalAddress[2];
    CEC_DEVICE_TYPE MyDeviceType;
    CEC_MSG_POWER_STATUS_PARM MyPowerStatus;
    U16 CecDevicesExisted[15];

    U8 ActiveDeviceCECVersion;
    CEC_DEVICE_TYPE ActiveDeviceType;
    CEC_DEVICELA ActiveLogicalAddress;    //the remoter controller's active device
    U8 ActivePhysicalAddress[2];
    CEC_MSG_POWER_STATUS_PARM ActivePowerStatus;
    CEC_ERROR_CODE  RetCode;
    CEC_MSGLIST CecMsg;
    U8  u8MsgID;    //for LGE request
} CEC_INFO_LIST_t;



typedef struct
{
	U8 u8Cmd; //080910_yongs
    CEC_DEVICELA Dst_Addr;
    CEC_MSGLIST Msg;
    U8 Operand[20];
    U8 Len;
    CEC_ERROR_CODE ErrorCode;
    U8  u8MsgID;    //for LGE request
} CEC_TX_INFO_t;

#endif // _MDRV_CEC_ST_H_
