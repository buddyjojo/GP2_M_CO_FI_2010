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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_pws_st.h
/// @brief  PWS Driver Interface
/// @author MStar Semiconductor,Inc.
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _DRVPWS_ST_H_
#define _DRVPWS_ST_H_


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
/*
#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
*/
#define MSIF_PWS_LIB_CODE                     {'P','W','S'}        //Lib code
#define MSIF_PWS_LIBVER                       {'0','0'}            //LIB version
#define MSIF_PWS_BUILDNUM                     {'0','0'}            //Build Number
#define MSIF_PWS_CHANGELIST                   {'0','0','1','3','5','1','0','2'} //P4 ChangeList Number

#define PWS_API_VERSION                /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    MSIF_PWS_LIB_CODE,                        /* IP__                                             */  \
    MSIF_PWS_LIBVER,                          /* 0.0 ~ Z.Z                                        */  \
    MSIF_PWS_BUILDNUM,                        /* 00 ~ 99                                          */  \
    MSIF_PWS_CHANGELIST,                      /* CL#                                              */  \
    MSIF_OS

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef enum
{
    PWS_API_FAIL    = 0,
    PWS_API_SUCCESS = 1
} E_PWS_API_Result;

typedef enum
{
    PWS_ADD_SOURCE  = 0,
    PWS_DEL_SOURCE  = 1,
    PWS_INVAILD_OP  = 2
} E_PWS_API_OP_Info;

typedef enum
{
    PWS_FULL    = 0,
    PWS_OFF_LINE_DETECT = 1
} E_PWS_API_SYNC_Info;

typedef enum
{
    _NO_SOURCE_   = 0,
    _USB_         = BIT0,
    _SV_          = BIT1,
    _HDMI4_       = BIT2,
    _HDMI3_       = BIT3,
    _HDMI2_       = BIT4,
    _HDMI1_       = BIT5,
    _YPbPr_       = BIT6,
    _SCART_       = BIT7,
    _RGB_         = BIT8,
    _CVBS_        = BIT9,
    _ATV_SSIF_    = BIT10,
    _ATV_VIF_     = BIT11,
    _DTV_ATSC_    = BIT12,
    _DTV_DVB_     = BIT13,
    _UNKNOWN_     = BIT14
} E_PWS_API_SouceInfo;

typedef enum
{
    E_PWS_DBGLV_NONE,          //no debug message
    E_PWS_DBGLV_ERR_ONLY,      //show error only
    E_PWS_DBGLV_REG_DUMP,      //show error & reg dump
    E_PWS_DBGLV_INFO,          //show error & informaiton
    E_PWS_DBGLV_ALL            //show error, information & funciton name
} E_PWS_API_DBG_LEVEL;

typedef struct
{
    E_PWS_API_DBG_LEVEL u8DbgLevel;
    U8 bInit;
} PWS_Status;

typedef struct
{
    E_PWS_API_SouceInfo SourceList;
    U32 u32IOMap;
} PWS_Info;

#endif //_DRVPWS_ST_H_

