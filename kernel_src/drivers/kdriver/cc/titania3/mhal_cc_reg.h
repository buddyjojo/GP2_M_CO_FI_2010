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
#ifndef _MHAL_CC_REG_H
#define _MHAL_CC_REG_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_cc_reg.h
/// This file contains the Mstar driver register for Close Caption
/// @author MStar Semiconductor Inc.
/// @brief  Close Caption module
///////////////////////////////////////////////////////////////////////////////

#define VBI_REG_BASE (0x3700)
#define AFEC_REG_BASE	(0x3500)
#define BK_AFEC_CC  BYTE2REAL(AFEC_REG_BASE+0xCC)


/* VBI area */

#define BK_VBI_1F  BYTE2REAL(VBI_REG_BASE+0x1F)
#define BK_VBI_2A  BYTE2REAL(VBI_REG_BASE+0x2A)
#define BK_VBI_40  BYTE2REAL(VBI_REG_BASE+0x40)
#define BK_VBI_41  BYTE2REAL(VBI_REG_BASE+0x41)
#define BK_VBI_42  BYTE2REAL(VBI_REG_BASE+0x42)
#define BK_VBI_44  BYTE2REAL(VBI_REG_BASE+0x44)
#define BK_VBI_45  BYTE2REAL(VBI_REG_BASE+0x45)
#define BK_VBI_46  BYTE2REAL(VBI_REG_BASE+0x46)
#define BK_VBI_4A  BYTE2REAL(VBI_REG_BASE+0x4A)
#define BK_VBI_4B  BYTE2REAL(VBI_REG_BASE+0x4B)
#define BK_VBI_4D  BYTE2REAL(VBI_REG_BASE+0x4D)
#define BK_VBI_4F  BYTE2REAL(VBI_REG_BASE+0x4F)
#define BK_VBI_50  BYTE2REAL(VBI_REG_BASE+0x50)
#define BK_VBI_51  BYTE2REAL(VBI_REG_BASE+0x51)
#define BK_VBI_55  BYTE2REAL(VBI_REG_BASE+0x55)
#define BK_VBI_56  BYTE2REAL(VBI_REG_BASE+0x56)
#define BK_VBI_57  BYTE2REAL(VBI_REG_BASE+0x57)
#define BK_VBI_58  BYTE2REAL(VBI_REG_BASE+0x58)
#define BK_VBI_59  BYTE2REAL(VBI_REG_BASE+0x59)
#define BK_VBI_5A  BYTE2REAL(VBI_REG_BASE+0x5A)
#define BK_VBI_5B  BYTE2REAL(VBI_REG_BASE+0x5B)
#define BK_VBI_5C  BYTE2REAL(VBI_REG_BASE+0x5C)
#define BK_VBI_5D  BYTE2REAL(VBI_REG_BASE+0x5D)
#define BK_VBI_5E  BYTE2REAL(VBI_REG_BASE+0x5E)
#define BK_VBI_5F  BYTE2REAL(VBI_REG_BASE+0x5F)
#define BK_VBI_70  BYTE2REAL(VBI_REG_BASE+0x70)
#define BK_VBI_71  BYTE2REAL(VBI_REG_BASE+0x71)
#define BK_VBI_72  BYTE2REAL(VBI_REG_BASE+0x72)
#define BK_VBI_77  BYTE2REAL(VBI_REG_BASE+0x77)
#define BK_VBI_7C  BYTE2REAL(VBI_REG_BASE+0x7C)
#define BK_VBI_7D  BYTE2REAL(VBI_REG_BASE+0x7D)
#define BK_VBI_7E  BYTE2REAL(VBI_REG_BASE+0x7E)
#define BK_VBI_7F  BYTE2REAL(VBI_REG_BASE+0x7F)
#define BK_VBI_81  BYTE2REAL(VBI_REG_BASE+0x81)
#define BK_VBI_82  BYTE2REAL(VBI_REG_BASE+0x82)
#define BK_VBI_83  BYTE2REAL(VBI_REG_BASE+0x83)
#define BK_VBI_86  BYTE2REAL(VBI_REG_BASE+0x86)
#define BK_VBI_89  BYTE2REAL(VBI_REG_BASE+0x89)
#define BK_VBI_8A  BYTE2REAL(VBI_REG_BASE+0x8A)
#define BK_VBI_8B  BYTE2REAL(VBI_REG_BASE+0x8B)
#define BK_VBI_8D  BYTE2REAL(VBI_REG_BASE+0x8D)
#define BK_VBI_91  BYTE2REAL(VBI_REG_BASE+0x91)
#define BK_VBI_92  BYTE2REAL(VBI_REG_BASE+0x92)
#define BK_VBI_99  BYTE2REAL(VBI_REG_BASE+0x99)
#define BK_VBI_9A  BYTE2REAL(VBI_REG_BASE+0x9A)
#define BK_VBI_A6  BYTE2REAL(VBI_REG_BASE+0xA6)
#define BK_VBI_A7  BYTE2REAL(VBI_REG_BASE+0xA7)
#define BK_VBI_AD  BYTE2REAL(VBI_REG_BASE+0xAD)
#define BK_VBI_AE  BYTE2REAL(VBI_REG_BASE+0xAE)
#define BK_VBI_AF  BYTE2REAL(VBI_REG_BASE+0xAF)
#define BK_VBI_B7  BYTE2REAL(VBI_REG_BASE+0xB7)
#define BK_VBI_B4  BYTE2REAL(VBI_REG_BASE+0xB4)
#define BK_VBI_B5  BYTE2REAL(VBI_REG_BASE+0xB5)
#define BK_VBI_B8  BYTE2REAL(VBI_REG_BASE+0xB8)
#define BK_VBI_BB  BYTE2REAL(VBI_REG_BASE+0xBB)
#define BK_VBI_C4  BYTE2REAL(VBI_REG_BASE+0xC4)
#define BK_VBI_CA  BYTE2REAL(VBI_REG_BASE+0xCA)
#define BK_VBI_CB  BYTE2REAL(VBI_REG_BASE+0xCB)
#define BK_VBI_CC  BYTE2REAL(VBI_REG_BASE+0xCC)
#define BK_VBI_CD  BYTE2REAL(VBI_REG_BASE+0xCD)
#define BK_VBI_CE  BYTE2REAL(VBI_REG_BASE+0xCE)

//  CC registers
#define BK_VBI_1C  BYTE2REAL(VBI_REG_BASE+0x1C)

#define VBI_BASEADDR_L          BYTE2REAL(0x38 + (VBI_REG_BASE))
#define VBI_BASEADDR_M          BYTE2REAL(0x39 + (VBI_REG_BASE))
#define VBI_BASEADDR_H          BYTE2REAL(0x3A + (VBI_REG_BASE))
#define VBI_BUF_LEN             BYTE2REAL(0x3B + (VBI_REG_BASE))
#define VBI_BUF_LEN_H           BYTE2REAL(0x3C + (VBI_REG_BASE))

#define VBI_W_COUNT             BYTE2REAL(0x3D + (VBI_REG_BASE))
#define VBI_PKTCNT_L            BYTE2REAL(0x3D + (VBI_REG_BASE))

#define VBI_INTERRUPT_MASK      BYTE2REAL(0x6C + (VBI_REG_BASE))
#define VBI_INTERRUPT_CLEAR     BYTE2REAL(0x6D + (VBI_REG_BASE))
#define VBI_INTERRUPT_STATUS    BYTE2REAL(0x6E + (VBI_REG_BASE))
#define VBI_INTERRUPT_RAW       BYTE2REAL(0x6F + (VBI_REG_BASE))

#define VBI_VPS_COUNT           BYTE2REAL(0xA5 + (VBI_REG_BASE))
#define VBI_WSS_COUNT           BYTE2REAL(0xA5 + (VBI_REG_BASE))

#define VBI_TTX_COUNTL          BYTE2REAL(0x3D + (VBI_REG_BASE))
#define VBI_TTX_COUNTH          BYTE2REAL(0x3E + (VBI_REG_BASE))

#define TT_ENABLE               BYTE2REAL(0x10 + (VBI_REG_BASE))


#define INTERRUPT_BASE          (0x2B00)

#define IRQ_MASK1               BYTE2REAL((0x18) + (INTERRUPT_BASE))
#define IRQ_MASK2               BYTE2REAL((0x19) + (INTERRUPT_BASE))
#define IRQ_MASK3               BYTE2REAL((0x1A) + (INTERRUPT_BASE))
#define IRQ_MASK4               BYTE2REAL((0x1B) + (INTERRUPT_BASE))

#define IRQ_FINAL_STATUS1       BYTE2REAL((0x28) + (INTERRUPT_BASE))
#define IRQ_FINAL_STATUS2       BYTE2REAL((0x29) + (INTERRUPT_BASE))
#define IRQ_FINAL_STATUS3       BYTE2REAL((0x2A) + (INTERRUPT_BASE))
#define IRQ_FINAL_STATUS4       BYTE2REAL((0x2B) + (INTERRUPT_BASE))

// AFEC registers
#define BK_AFEC_6B		BYTE2REAL(AFEC_REG_BASE+0x6B)
#endif

