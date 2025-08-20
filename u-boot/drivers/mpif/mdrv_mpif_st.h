////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
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

#ifndef __DRV_MPIF_ST_H__
#define __DRV_MPIF_ST_H__

#define ENABLE_MPIF_IOCTL_INTERFACE     1
//------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------

#define MPIF_CLOCK_74MHZ                                0x00
#define MPIF_CLOCK_123MHZ                               0x04
#define MPIF_CLOCK_108MHZ                               0x08
#define MPIF_CLOCK_80MHZ                                0x0C
#define MPIF_CLOCK_160MHZ                               0x10
#define MPIF_CLOCK_62MHZ                                0x14
#define MPIF_CLOCK_54MHZ                                0x18
#define MPIF_CLOCK_40MHZ                                0x1C
#define MPIF_CLOCK_20MHZ                                0x20
#define MPIF_CLOCK_13_5MHZ                              0x24
#define MPIF_CLOCK_XTAL                                 0x3C
#define MPIF_CLOCK_SEL                                  MPIF_CLOCK_XTAL

#define MPIF_TR_CYCLE_SET                               1 //1T, 0T ~ 7T
#define MPIF_WAIT_CYCLE_SET                             1 //1T, 0T ~ 3T

#define MPIF_SLAVE_ID_0                                 0x00
#define MPIF_SLAVE_ID_1                                 0x01
#define MPIF_SLAVE_ID_2                                 0x02
#define MPIF_SLAVE_ID_3                                 0x03
#define MPIF_SLAVE_ID_SEL                               MPIF_SLAVE_ID_0

//--------------- Commend & Data Width --------------------------------
#define MPIF_DATAWIDTH_1                                0
#define MPIF_DATAWIDTH_2                                1
#define MPIF_DATAWIDTH_4                                2
#define MPIF_DATAWIDTH_8                                3

#define MPIF_CMD_DATAWIDTH                              MPIF_DATAWIDTH_1

#define MPIF_SLAVE0_DATAWIDTH                           MPIF_DATAWIDTH_1
#define MPIF_SLAVE1_DATAWIDTH                           MPIF_DATAWIDTH_1
#define MPIF_SLAVE2_DATAWIDTH                           MPIF_DATAWIDTH_1
#define MPIF_SLAVE3_DATAWIDTH                           MPIF_DATAWIDTH_1
//----------------------------------------------------------------------

#define MPIF_LC1A_INDEX_0                               (0x00 << 1)
#define MPIF_LC1A_INDEX_1                               (0x01 << 1)
#define MPIF_LC1A_INDEX_2                               (0x02 << 1)
#define MPIF_LC1A_INDEX_3                               (0x03 << 1)
#define MPIF_LC1A_INDEX_4                               (0x04 << 1)
#define MPIF_LC1A_INDEX_5                               (0x05 << 1)
#define MPIF_LC1A_INDEX_6                               (0x06 << 1)
#define MPIF_LC1A_INDEX_7                               (0x07 << 1)

#define MPIF_CHECKSUM_DISABLE                           0x00
#define MPIF_CHECKSUM_ENABLE                            0x01

#define MPIF_RTX_INDICATOR_DISABLE                      0x00
#define MPIF_RTX_INDICATOR_ENABLE                       0x01

#define MPIF_MAX_RTX_0                                  0x00    // 0 time
#define MPIF_MAX_RTX_1                                  0x01    // 1 times
#define MPIF_MAX_RTX_2                                  0x02    // 2 times
#define MPIF_MAX_RTX_3                                  0x03    // 3 times

#define MPIF_LC4A_CHECKPOINT_0                          0x00    // 1*256 bytes
#define MPIF_LC4A_CHECKPOINT_1                          0x01    // 2*256 bytes
#define MPIF_LC4A_CHECKPOINT_2                          0x02    // 3*256 bytes
#define MPIF_LC4A_CHECKPOINT_3                          0x03    // 4*256 bytes

#define MPIF_MIUID_0                                    0x00
#define MPIF_MIUID_1                                    0x01

#define MPIF_FASTMODE_DISABLE                           0x00
#define MPIF_FASTMODE_ENABLE                            0x01

#define MPIF_2X_FROMSPIF                                0
#define MPIF_2X_FROMXIU                                 1


//------------------------------------------------------------------------------
// Structure
//------------------------------------------------------------------------------

typedef struct
{
    U8 u8clk;
    U8 u8trc;
    U8 u8wc;
} MPIF_INIT_PARAM;

typedef struct
{
    U8 u8slaveid;
    U8 u8cmdwidth;
    U8 u8datawidth;
} MPIF_BUS_PARAM;


typedef struct
{
    U8 u8bWite;
    U8 slaveid;
    U16 addr;
    U32 data;
    BOOL bRet;
} MPIF_PARAM;


#endif//__DRV_MPIF_ST_H__
