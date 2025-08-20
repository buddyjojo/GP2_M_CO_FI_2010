////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("; MStar; Confidential; Information;") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_tcon.h
/// @brief  MStar Scaler Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DRV_TCON_H__
#define __DRV_TCON_H__


//#include "mdrv_tcon_tbl.h"
//#include "mdrv_scaler.h"
#define TCON_TAB_MAX_SIZE           2048
#define BOOL U8

#define REG_TCON_BASE              0x103000
#define TCON_REG(_x_)               (REG_TCON_BASE | (_x_ << 1))


typedef enum
{
    SC_SIGNAL_POL,
    SC_SIGNAL_VGH,
    SC_SIGNAL_SOE,
    SC_SIGNAL_VST,
    SC_SIGNAL_GLK,
    SC_SIGNAL_NUMS,
} SC_POWER_SEQUENCE_SIGNAL_TYPE;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16tconpanelIdx;
} SC_TCON_MAP_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16tconpanelIdx;
    U16 u16tabtype;
    U32 u32Tabsize;
    U8  u8TconTab[TCON_TAB_MAX_SIZE];
} SC_TCON_TAB_INFO_t;


typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16tconpanelIdx;
    BOOL benable;
    SC_POWER_SEQUENCE_SIGNAL_TYPE u8Tcontype;
} SC_TCON_POW_SEQ_INFO_t;

void MDrv_SC_Set_TCONMap(SC_TCON_MAP_t *pTconMap);
//void MDrv_SC_Get_TCONTab_Info(SC_TCON_TAB_INFO_t* pTconTabInfo);
void MDrv_SC_Set_TCONPower_Sequence(SC_TCON_POW_SEQ_INFO_t tconpower_sequence, U8 u8Tcontype);
void MDrv_TCONMAP_DumpPowerOnSequenceReg(void);

#endif//__DRV_ADC_H__
