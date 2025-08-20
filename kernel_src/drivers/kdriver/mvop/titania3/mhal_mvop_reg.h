
#include "mdrv_types.h"

#ifndef _REG_MVOP_H_
#define _REG_MVOP_H_

#define REG_MVOP_BASE               0xBF202800
#define MVOP_REG(addr)              (*((volatile U32*)(REG_MVOP_BASE + ((addr)<<2))))

#define VOP_ON_MIU1                 (((*((volatile U32*)(0xBF2025E4))) & BIT3) == BIT3)

#define MIU_MASK_MVOP_0  \
do { \
(*((volatile U32*)(0xBF2024CC))) |= (BIT3); \
}while(0)

#define MIU_UNMASK_MVOP_0  \
do { \
(*((volatile U32*)(0xBF2024CC))) &= ~(BIT3); \
}while(0)

#define MIU_MASK_MVOP_1  \
do { \
(*((volatile U32*)(0xBF200CCC))) |= (BIT3); \
}while(0)

#define MIU_UNMASK_MVOP_1  \
do { \
(*((volatile U32*)(0xBF200CCC))) &= ~(BIT3); \
}while(0)

#define MVOP_VBlank  45
#define MVOP_HBlank_SD  150
#define MVOP_HBlank_HD  300
#define DC_BaseClock  86400 //86.4*1000

//------------------------------------------------------------------------------
// MMVOP Reg
//------------------------------------------------------------------------------
#define MVOP_FRAME_VCOUNT                        0x00
#define MVOP_FRAME_HCOUNT                        0x01
#define MVOP_VB0_STR                             0x02
#define MVOP_VB0_END                             0x03
#define MVOP_VB1_STR                             0x04
#define MVOP_VB1_END                             0x05
#define MVOP_TF_STR                              0x06
#define MVOP_BF_STR                              0x07
#define MVOP_HACT_STR                            0x08
#define MVOP_IMG_HSTR                            0x09
#define MVOP_IMG_VSTR0                           0x0A
#define MVOP_IMG_VSTR1                           0x0B
#define MVOP_TF_VS                               0x0C
#define MVOP_BF_VS                               0x0D
#define MVOP_HI_TSH                              0x10
#define MVOP_CTRL0                               0x11
#define MVOP_TST_IMG                             0x12
#define MVOP_U_PAT                               0x13

#define MVOP_INT_MASK                            0x1F
#define MVOP_MPG_JPG_SWITCH                      0x20
#define MVOP_JPG_YSTR0_L                         0x21
#define MVOP_JPG_YSTR0_H                         0x22
#define MVOP_JPG_UVSTR0_L                        0x23
#define MVOP_JPG_UVSTR0_H                        0x24
#define MVOP_JPG_HSIZE                           0x25
#define MVOP_JPG_VSIZE                           0x26
#define MVOP_REG_WR                              0x27

#define MVOP_AUDIO_INFO_IN                       0x28
#define MVOP_INPUT_SWITCH                        0x28
#define MVOP_DEBUG_R                             0x2F

#define MVOP_UV_SHT                              0x30 //LGE gbtogether(081217) ->to fix ChromaArtifact when 420to422 by Junyou.Lin
#define MVOP_JPG_YSTR1_L                         0x31
#define MVOP_JPG_YSTR1_H                         0x32
#define MVOP_JPG_UVSTR1_L                        0x33
#define MVOP_JPG_UVSTR1_H                        0x34


#define MVOP_DC1_FRAME_VCOUNT                    0x35
#define MVOP_DC1_FRAME_HCOUNT                    0x36

#define MVOP_STRIP_ALIGN                         0x3f

#endif

