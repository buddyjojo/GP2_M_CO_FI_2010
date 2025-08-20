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

#ifndef _GPIO_MACRO_H_
#define _GPIO_MACRO_H_


//----------------------------------------------------------------------------
// Private Macros
//----------------------------------------------------------------------------
#define MDrv_ReadByte( u16Reg )  XBYTE[u16Reg]
#define MDrv_WriteByte( u16Reg, u8Value )   (XBYTE[u16Reg] = u8Value)

#define MDrv_ReadRegBit( u16Reg, u8BitPos ) \
    (MDrv_ReadByte( u16Reg ) & (u8BitPos))

#define MDrv_WriteRegBit( u16Reg, bBit, u8BitPos ) \
    (XBYTE[u16Reg] = (bBit) ? (XBYTE[u16Reg] | (u8BitPos)) : (XBYTE[u16Reg] & ~(u8BitPos)))

#define __GPIO_REG(g)       ___GPIO_REG(g)
#define __GPIO_BIT(g)       ___GPIO_BIT(g)

#define __GPIO_DIR(g, d)    MDrv_WriteRegBit(___GPIO_REG(g), d, ___GPIO_BIT(g))
#define __GPIO_SET(g, v)    MDrv_WriteRegBit(___GPIO_REG(g), v, ___GPIO_BIT(g))
#define __GPIO_GET(g)       MDrv_ReadRegBit(___GPIO_REG(g), ___GPIO_BIT(g))

// don't use these outside this header
#define ___GPIO_REG(r, b)   r
#define ___GPIO_BIT(r, b)   b

//----------------------------------------------------------------------------
// Internal GPIO
//----------------------------------------------------------------------------
//#define GPIO134_PAD     PAD_GPIO_PM0
#define GPIO_PM0_OEN     0x1002, BIT0
#define GPIO_PM0_OUT     0x1004, BIT0
#define GPIO_PM0_IN      0x1006, BIT0
#define GPIO_PM0_DIR(d)  __GPIO_DIR(GPIO_PM0_OEN, d)
#define GPIO_PM0_SET(v)  __GPIO_SET(GPIO_PM0_OUT, v)
#define GPIO_PM0_GET()   __GPIO_GET(GPIO_PM0_IN)

//#define GPIO_PM1_PAD     PAD_GPIO_PM1
#define GPIO_PM1_OEN     0x1002, BIT1
#define GPIO_PM1_OUT     0x1004, BIT1
#define GPIO_PM1_IN      0x1006, BIT1
#define GPIO_PM1_DIR(d)  __GPIO_DIR(GPIO_PM1_OEN, d)
#define GPIO_PM1_SET(v)  __GPIO_SET(GPIO_PM1_OUT, v)
#define GPIO_PM1_GET()   __GPIO_GET(GPIO_PM1_IN)
#define GPIO_PM1_BIT()   __GPIO_BIT(GPIO_PM1_IN)

//#define GPIO_PM2_PAD     PAD_GPIO_PM2
#define GPIO_PM2_OEN     0x1002, BIT2
#define GPIO_PM2_OUT     0x1004, BIT2
#define GPIO_PM2_IN      0x1006, BIT2
#define GPIO_PM2_DIR(d)  __GPIO_DIR(GPIO_PM2_OEN, d)
#define GPIO_PM2_SET(v)  __GPIO_SET(GPIO_PM2_OUT, v)
#define GPIO_PM2_GET()   __GPIO_GET(GPIO_PM2_IN)
#define GPIO_PM2_BIT()   __GPIO_BIT(GPIO_PM2_IN)

//#define GPIO_PM3_PAD     PAD_GPIO_PM3
#define GPIO_PM3_OEN     0x1002, BIT3
#define GPIO_PM3_OUT     0x1004, BIT3
#define GPIO_PM3_IN      0x1006, BIT3
#define GPIO_PM3_DIR(d)  __GPIO_DIR(GPIO_PM3_OEN, d)
#define GPIO_PM3_SET(v)  __GPIO_SET(GPIO_PM3_OUT, v)
#define GPIO_PM3_GET()   __GPIO_GET(GPIO_PM3_IN)
#define GPIO_PM3_BIT()   __GPIO_BIT(GPIO_PM3_IN)

//#define GPIO_PM4_PAD     PAD_GPIO_PM4
#define GPIO_PM4_OEN     0x1002, BIT4
#define GPIO_PM4_OUT     0x1004, BIT4
#define GPIO_PM4_IN      0x1006, BIT4
#define GPIO_PM4_DIR(d)  __GPIO_DIR(GPIO_PM4_OEN, d)
#define GPIO_PM4_SET(v)  __GPIO_SET(GPIO_PM4_OUT, v)
#define GPIO_PM4_GET()   __GPIO_GET(GPIO_PM4_IN)
#define GPIO_PM4_BIT()   __GPIO_BIT(GPIO_PM4_IN)

//#define GPIO_PM5_PAD     PAD_GPIO_PM5
#define GPIO_PM5_OEN     0x1002, BIT5
#define GPIO_PM5_OUT     0x1004, BIT5
#define GPIO_PM5_IN      0x1006, BIT5
#define GPIO_PM5_DIR(d)  __GPIO_DIR(GPIO_PM5_OEN, d)
#define GPIO_PM5_SET(v)  __GPIO_SET(GPIO_PM5_OUT, v)
#define GPIO_PM5_GET()   __GPIO_GET(GPIO_PM5_IN)
#define GPIO_PM5_BIT()   __GPIO_BIT(GPIO_PM5_IN)

//#define GPIO_PM6_PAD     PAD_GPIO_PM6
#define GPIO_PM6_OEN     0x1002, BIT6
#define GPIO_PM6_OUT     0x1004, BIT6
#define GPIO_PM6_IN      0x1006, BIT6
#define GPIO_PM6_DIR(d)  __GPIO_DIR(GPIO_PM6_OEN, d)
#define GPIO_PM6_SET(v)  __GPIO_SET(GPIO_PM6_OUT, v)
#define GPIO_PM6_GET()   __GPIO_GET(GPIO_PM6_IN)
#define GPIO_PM6_BIT()   __GPIO_BIT(GPIO_PM6_IN)

//#define GPIO_PM7_PAD     PAD_GPIO_PM7
#define GPIO_PM7_OEN     0x1002, BIT7
#define GPIO_PM7_OUT     0x1004, BIT7
#define GPIO_PM7_IN      0x1006, BIT7
#define GPIO_PM7_DIR(d)  __GPIO_DIR(GPIO_PM7_OEN, d)
#define GPIO_PM7_SET(v)  __GPIO_SET(GPIO_PM7_OUT, v)
#define GPIO_PM7_GET()   __GPIO_GET(GPIO_PM7_IN)
#define GPIO_PM7_BIT()   __GPIO_BIT(GPIO_PM7_IN)

//#define GPIO_PM8_PAD     PAD_GPIO_PM8
#define GPIO_PM8_OEN     0x1003, BIT0
#define GPIO_PM8_OUT     0x1005, BIT0
#define GPIO_PM8_IN      0x1007, BIT0
#define GPIO_PM8_DIR(d)  __GPIO_DIR(GPIO_PM8_OEN, d)
#define GPIO_PM8_SET(v)  __GPIO_SET(GPIO_PM8_OUT, v)
#define GPIO_PM8_GET()   __GPIO_GET(GPIO_PM8_IN)
#define GPIO_PM8_BIT()   __GPIO_BIT(GPIO_PM8_IN)

//#define GPIO_PM9_PAD     PAD_GPIO_PM9
#define GPIO_PM9_OEN     0x1003, BIT1
#define GPIO_PM9_OUT     0x1005, BIT1
#define GPIO_PM9_IN      0x1007, BIT1
#define GPIO_PM9_DIR(d)  __GPIO_DIR(GPIO_PM9_OEN, d)
#define GPIO_PM9_SET(v)  __GPIO_SET(GPIO_PM9_OUT, v)
#define GPIO_PM9_GET()   __GPIO_GET(GPIO_PM9_IN)
#define GPIO_PM9_BIT()   __GPIO_BIT(GPIO_PM9_IN)

//#define GPIO_PM10_PAD     PAD_GPIO_PM10
#define GPIO_PM10_OEN     0x1003, BIT2
#define GPIO_PM10_OUT     0x1005, BIT2
#define GPIO_PM10_IN      0x1007, BIT2
#define GPIO_PM10_DIR(d)  __GPIO_DIR(GPIO_PM10_OEN, d)
#define GPIO_PM10_SET(v)  __GPIO_SET(GPIO_PM10_OUT, v)
#define GPIO_PM10_GET()   __GPIO_GET(GPIO_PM10_IN)
#define GPIO_PM10_BIT()   __GPIO_BIT(GPIO_PM10_IN)

//#define GPIO_SAR0_PAD     PAD_SAR0
#define GPIO_SAR0_SW      0x1420, BIT0
#define GPIO_SAR0_OEN     0x1420, BIT4
#define GPIO_SAR0_OUT     0x1421, BIT0
#define GPIO_SAR0_IN      0x1424, BIT0
#define GPIO_SAR0_DIR(d)  do{\
                            __GPIO_SET(GPIO_SAR0_SW, d);\
                            __GPIO_DIR(GPIO_SAR0_OEN, d);\
                            }while(0)
#define GPIO_SAR0_SET(v)  __GPIO_SET(GPIO_SAR0_OUT, v)
#define GPIO_SAR0_GET()   __GPIO_GET(GPIO_SAR0_IN)
#define GPIO_SAR0_BIT()   __GPIO_BIT(GPIO_SAR0_IN)

//#define GPIO_SAR1_PAD     PAD_SAR1
#define GPIO_SAR1_SW      0x1420, BIT1
#define GPIO_SAR1_OEN     0x1420, BIT5
#define GPIO_SAR1_OUT     0x1421, BIT1
#define GPIO_SAR1_IN      0x1424, BIT1
#define GPIO_SAR1_DIR(d)  do{\
                            __GPIO_SET(GPIO_SAR1_SW, d);\
                            __GPIO_DIR(GPIO_SAR1_OEN, d);\
                            }while(0)
#define GPIO_SAR1_SET(v)  __GPIO_SET(GPIO_SAR1_OUT, v)
#define GPIO_SAR1_GET()   __GPIO_GET(GPIO_SAR1_IN)
#define GPIO_SAR1_BIT()   __GPIO_BIT(GPIO_SAR1_IN)

//#define GPIO_SAR2_PAD     PAD_SAR2
#define GPIO_SAR2_SW      0x1420, BIT2
#define GPIO_SAR2_OEN     0x1420, BIT6
#define GPIO_SAR2_OUT     0x1421, BIT2
#define GPIO_SAR2_IN      0x1424, BIT2
#define GPIO_SAR2_DIR(d)  do{\
                            __GPIO_SET(GPIO_SAR2_SW, d);\
                            __GPIO_DIR(GPIO_SAR2_OEN, d);\
                            }while(0)
#define GPIO_SAR2_SET(v)  __GPIO_SET(GPIO_SAR2_OUT, v)
#define GPIO_SAR2_GET()   __GPIO_GET(GPIO_SAR2_IN)
#define GPIO_SAR2_BIT()   __GPIO_BIT(GPIO_SAR2_IN)

//#define GPIO_SAR3_PAD     PAD_SAR3
#define GPIO_SAR3_SW      0x1420, BIT3
#define GPIO_SAR3_OEN     0x1420, BIT7
#define GPIO_SAR3_OUT     0x1421, BIT3
#define GPIO_SAR3_IN      0x1424, BIT3
#define GPIO_SAR3_DIR(d)  do{\
                            __GPIO_SET(GPIO_SAR3_SW, d);\
                            __GPIO_DIR(GPIO_SAR3_OEN, d);\
                            }while(0)
#define GPIO_SAR3_SET(v)  __GPIO_SET(GPIO_SAR3_OUT, v)
#define GPIO_SAR3_GET()   __GPIO_GET(GPIO_SAR3_IN)
#define GPIO_SAR3_BIT()   __GPIO_BIT(GPIO_SAR3_IN)


#endif /* _GPIO_MACRO_H_ */
