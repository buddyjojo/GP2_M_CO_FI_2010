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

#ifndef _MHAL_MIU_H_
#define _MHAL_MIU_H_


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define _FUNC_NOT_USED()        do {} while ( 0 )

#define REG_MIU_MUX    0x1E5A

#define MIU_OPM_R_MASK 0x0667
#define MIU_OPM_W_MASK 0x0666
#define MIU_MVD_R_MASK 0x06F6
#define MIU_MVD_W_MASK 0x06F7

//$ MIU0 Request Mask functions
#define _MaskMiuReq_OPM_R( m )     HAL_MIU_WriteRegBit(MIU_RQ1L_MASK, m, BIT0)

#define _MaskMiuReq_DNRB_W( m )    HAL_MIU_WriteRegBit(MIU_RQ1L_MASK, m, BIT6)
#define _MaskMiuReq_DNRB_R( m )    HAL_MIU_WriteRegBit(MIU_RQ1L_MASK, m, BIT7)
#define _MaskMiuReq_DNRB_RW( m )   HAL_MIU_WriteRegBit(MIU_RQ1L_MASK, m, BIT7|BIT6)

#define _MaskMiuReq_SC_RW( m )     HAL_MIU_WriteRegBit(MIU_RQ1L_MASK, m, BIT7|BIT6|BIT0)

#define _MaskMiuReq_MVOP_R( m )    HAL_MIU_WriteRegBit(MIU_RQ1L_MASK, m, BIT1)

#define _MaskMiuReq_MVD_R( m )     HAL_MIU_WriteRegBit(MIU_MVD_R_MASK, m, BIT0)
#define _MaskMiuReq_MVD_W( m )     HAL_MIU_WriteRegBit(MIU_MVD_W_MASK, m, BIT3)
#define _MaskMiuReq_MVD_RW( m )    do { _MaskMiuReq_MVD_R( m ); _MaskMiuReq_MVD_W( m ); } while (0)

#define _MaskMiuReq_AUDIO_RW( m )  _FUNC_NOT_USED()


//$ MIU1 Request Mask functions (only some engine can be in MIU1)
#define _MaskMiu1Req_OPM_R( m )    HAL_MIU_WriteRegBit(MIU_OPM_R_MASK, m, BIT3)
#define _MaskMiu1Req_DNRB_W( m )   HAL_MIU_WriteRegBit(MIU_OPM_W_MASK, m, BIT7)
#define _MaskMiu1Req_DNRB_R( m )   HAL_MIU_WriteRegBit(MIU_OPM_R_MASK, m, BIT0)

#define _MaskMiu1Req_DNRB_RW( m )  do { _MaskMiu1Req_DNRB_W( m );             \
                                       _MaskMiu1Req_DNRB_R( m );              \
                                   } while (0)
#define _MaskMiu1Req_SC_RW( m )    do { _MaskMiu1Req_DNRB_W( m );             \
                                       HAL_MIU_WriteRegBit(MIU_OPM_R_MASK, m, BIT3|BIT0);\
                                   } while (0)

#define _MaskMiu1Req_MVOP_R( m )   _FUNC_NOT_USED()

#define _MaskMiu1Req_MVD_R( m )    _FUNC_NOT_USED()
#define _MaskMiu1Req_MVD_W( m )    _FUNC_NOT_USED()
#define _MaskMiu1Req_MVD_RW( m )   _FUNC_NOT_USED()

#define _MaskMiu1Req_AUDIO_RW( m ) _FUNC_NOT_USED()


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
MS_BOOL HAL_MIU_WriteRegBit(MS_U32 u32RegAddr, MS_U8 u8Mask, MS_BOOL bEnable);
MS_BOOL HAL_MIU_Protect(MS_U8 u8Blockx, MS_U8 *pu8ProtectId, MS_U32 u32Start,
                        MS_U32 u32End, MS_BOOL  bSetFlag);
#endif // _MHAL_MIU_H_
