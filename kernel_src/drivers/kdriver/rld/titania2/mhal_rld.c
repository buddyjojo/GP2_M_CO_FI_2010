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

////////////////////////////////////////////////////////////////////////////////
//
/// @file mhal_rld.c
/// @brief RLD driver.
/// @author MStar Semiconductor, Inc.
//
////////////////////////////////////////////////////////////////////////////////

#define DRVRLD_C

//-------------------------------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <asm/io.h>

#include "mst_devid.h"
#include "mdrv_types.h"
#include "mhal_rld_reg.h"
#include "mst_platform.h"

//-------------------------------------------------------------------------------------------------
// Local Defines
//-------------------------------------------------------------------------------------------------
#define RLD_WARNING(fmt, args...)       printk(KERN_WARNING "[RLDMOD][%06d] " fmt, __LINE__, ## args)
#define RLD_PRINT(fmt, args...)         printk("[RLDMOD][%06d]     " fmt, __LINE__, ## args)
//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define RLD_POLL_INTERVAL   10
#define RLD_RUN_TIME        500
//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local Function Prototypes
//-------------------------------------------------------------------------------------------------

static void MHal_RLD_WriteReg(U32 u32addr, U32 u32val)
{
    RLD_Reg(u32addr) = u32val;
    //u32RegAddr = REG_RLD_BASE + (u32addr *4);
    //*(U32 *)(u32RegAddr) = u32val;
    //u16RegValue1 = *(U32 *)(u32RegAddr);
    //u16RegValue2 = *(U32 *)(u32RegAddr);
    //RLD_PRINT("Write Reg addr %lx %lx value %lx ", u32addr, u32RegAddr, u32val);
    //RLD_PRINT("get %x get again %x\n", u16RegValue1, u16RegValue2);
    //RLD_Reg(u32addr) = u32val;
}

static U32 MHal_RLD_ReadReg(U32 u32addr)
{
    //U32 u32RegAddr;
    //u32RegAddr = REG_RLD_BASE + (u32addr *4);
    //RLD_PRINT("Read Reg at %lx (%lx) get %lx\n", u32RegAddr, u32addr, *(U32 *)(u32RegAddr));

    //return *(U32 *)(u32RegAddr);
    return RLD_Reg(u32addr);
}

/******************************************************************************/
/// Start and run the RLD.
/******************************************************************************/
void MHal_RLD_Init(void)
{
    U32 u32RegValue;
    u32RegValue = *(U32 *)RLD_CHIPTOP;
    u32RegValue &= 0xfffe;
    *(U32 *)RLD_CHIPTOP = u32RegValue;
}


/******************************************************************************/
/// Start and run the RLD.
/******************************************************************************/
void MHal_RLD_Start(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_CTRL);

    u32RegValue &= 0xff00;  // clear lower 8 bit
    u32RegValue |= 0x0001;
    MHal_RLD_WriteReg(STRLD_CTRL, u32RegValue);     // send 0x01
    u32RegValue &= 0xff00;  // clear lower 8 bit
    MHal_RLD_WriteReg(STRLD_CTRL, u32RegValue);     // send 0x00
}

/******************************************************************************/
/// Reset the RLD.
/******************************************************************************/
void MHal_RLD_Reset(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_CTRL);

    u32RegValue &= 0xff00;  // clear lower 8 bit
    u32RegValue |= 0x0080;
    MHal_RLD_WriteReg(STRLD_CTRL, u32RegValue);     // send 0x80
    u32RegValue &= 0xff00;  // clear lower 8 bit
    MHal_RLD_WriteReg(STRLD_CTRL, u32RegValue);     // send 0x00
}

/******************************************************************************/
/// Set the output format.
/// @param u8Value \b IN The output format for I8, I4 and I2.
/// @return FALSE if \e u8Value is not valid.
/******************************************************************************/
BOOL MHal_RLD_SetOutputFormat(U8 u8Value)
{
    U32 u32RegValue;
    if( (u8Value!= 0x02) && (u8Value!= 0x04) && (u8Value!= 0x08))
        return FALSE;
    u32RegValue = MHal_RLD_ReadReg(STRLD_OUTPUT_FORMAT);
    u32RegValue &= 0xff00;  // clear lower 8 bit
    u32RegValue |= u8Value ;

    MHal_RLD_WriteReg(STRLD_OUTPUT_FORMAT, u32RegValue);
    return TRUE;
}

/******************************************************************************/
/// Set the enlarge rate.
/// @param u8Value \b IN The enlarge rate. Only 1 ~ 7 is valid.
/// @return FALSE if \e u8Value is not valid.
/******************************************************************************/
BOOL MHal_RLD_SetEnlargeRate(U8 u8Value)
{
    U32 u32RegValue;
    // 1 ~ 7
    if(( u8Value < 1) || ( u8Value> 7))
        return FALSE;

    u32RegValue = MHal_RLD_ReadReg(STRLD_ENLARGE_RATE);
    u32RegValue &= 0x00ff;  // clear higher 8 bit
    u32RegValue |= u8Value << 8;

    MHal_RLD_WriteReg(STRLD_ENLARGE_RATE, u32RegValue);

    return TRUE;
}

/******************************************************************************/
/// Set transparent key.
/// @param u8Value \b IN The transparent key value.
/******************************************************************************/
void MHal_RLD_SetTransparentKey(U8 u8Value)
{
    U32 u32RegValue;

    u32RegValue = MHal_RLD_ReadReg(STRLD_TRANSPARENT_KEY);
    u32RegValue &= 0xff00;  // clear lower 8 byte
    u32RegValue |= u8Value;

    MHal_RLD_WriteReg(STRLD_TRANSPARENT_KEY, u32RegValue);
}

/******************************************************************************/
/// Set top field length
/// @param u16Value \b IN The length of the top field.
/******************************************************************************/
void MHal_RLD_SetTopFieldLength(U16 u16Value)
{
    //U16 u16RegValue;
    MHal_RLD_WriteReg(STRLD_TOP_LEN,u16Value);
    //u16RegValue = MHal_RLD_ReadReg(STRLD_TOP_LEN);
    //RLD_PRINT("set %lx get %lx\n", u16Value, u16RegValue);
    //RLD_PRINT("set top %x get top %x\n", u16Value, u16RegValue);
    //u32RegAddr = 0xbf805248;
    //RLD_PRINT("direct get %lx\n", *(U32 *)(u32RegAddr));

    //u32RegAddr = REG_RLD_BASE + (STRLD_TOP_LEN *4);
    //RLD_PRINT("Setting address %lx ", u32RegAddr);
    //*(U32 *)(u32RegAddr) = 0x000006d6;
    //RLD_PRINT("get its value %lx\n", *(U32 *)(u32RegAddr));
}

/******************************************************************************/
/// Set bottom field length
/// @param u16Value \b IN The length of the bottom field.
/******************************************************************************/
void MHal_RLD_SetBottomFieldLength(U16 u16Value)
{
    MHal_RLD_WriteReg(STRLD_BOT_LEN,u16Value);
}

/******************************************************************************/
/// Set region width.
/// @param u16Value \b IN Region width
/******************************************************************************/
void MHal_RLD_SetRegionWidth(U16 u16Value)
{
    MHal_RLD_WriteReg(STRLD_REGION_WIDTH,u16Value);
}

/******************************************************************************/
/// Set region height.
/// @param u16Value \b IN Region height
/******************************************************************************/
void MHal_RLD_SetRegionHeight(U16 u16Value)
{
    MHal_RLD_WriteReg(STRLD_REGION_HEIGHT,u16Value);
}

/******************************************************************************/
/// Set region pitch. 8 byte alignment is required.
/// @param u16Value \b IN Region pitch.
/******************************************************************************/
void MHal_RLD_SetRegionPitch(U16 u16Value)
{
    MHal_RLD_WriteReg(STRLD_REGION_PITCH,u16Value);
}

/******************************************************************************/
/// Set object X offset relative to the region X position.
/// @param u16Value \b IN The X position.
/******************************************************************************/
void MHal_RLD_SetObjXOffset(U16 u16Value)
{
    MHal_RLD_WriteReg(STRLD_OBJ_X_OFFSET,u16Value);
}

/******************************************************************************/
/// Set object Y offset relative to the region Y position.
/// @param u16Value \b IN The Y position.
/******************************************************************************/
void MHal_RLD_SetObjYOffset(U16 u16Value)
{
    MHal_RLD_WriteReg(STRLD_OBJ_Y_OFFSET,u16Value);
}

/******************************************************************************/
/// Set region pixel offset. Trim if overflow.
/// @param u8Value \b IN offset value.
/******************************************************************************/
void MHal_RLD_SetRegionOffset(U8 u8Value)
{
    U32 u32RegValue;

    u32RegValue = MHal_RLD_ReadReg(STRLD_REGION_OFFSET);
    u32RegValue &= 0xff00;  // clear lower 8 byte
    u32RegValue |= u8Value;
    MHal_RLD_WriteReg(STRLD_REGION_OFFSET, u32RegValue);

}

/******************************************************************************/
/// Set region color depth.
/// @param u8Value \b IN Depth.
/******************************************************************************/
void MHal_RLD_SetRegionDepth(U8 u8Value)
{
    U32 u32RegValue;

    u32RegValue = MHal_RLD_ReadReg(STRLD_BITSTREAM_DEPTH);
    u32RegValue &= 0x00ff;  // clear higher 8 byte
    u32RegValue |= u8Value << 8;
    MHal_RLD_WriteReg(STRLD_BITSTREAM_DEPTH, u32RegValue);

}


/******************************************************************************/
/// Set region foreground color key.
/// @param u8Value \b IN Color key value.
/******************************************************************************/
void MHal_RLD_SetRegionColorKeyFG(U8 u8Value)
{
    U32 u32RegValue;

    u32RegValue = MHal_RLD_ReadReg(STRLD_REGION_FG_INDEX);
    u32RegValue &= 0x00ff;  // clear higher 8 byte
    u32RegValue |= u8Value << 8;
    MHal_RLD_WriteReg(STRLD_REGION_FG_INDEX, u32RegValue);
}

/******************************************************************************/
/// Set region background color key.
/// @param u8Value \b IN Color key value.
/******************************************************************************/
void MHal_RLD_SetRegionColorKeyBG(U8 u8Value)
{
    U32 u32RegValue;

    u32RegValue = MHal_RLD_ReadReg(STRLD_REGION_BG_INDEX);
    u32RegValue &= 0xff00;  // clear lower 8 byte
    u32RegValue |= u8Value;
    MHal_RLD_WriteReg(STRLD_REGION_BG_INDEX, u32RegValue);
}

/******************************************************************************/
/// Set the address of the top field.
/// @param u8Value \b IN The SDRAM address of top field.
/******************************************************************************/
void MHal_RLD_SetTopFieldAddress(U32 u32Value)
{
   // u32Value = u32Value / 8; //move to madp level

    MHal_RLD_WriteReg(STRLD_TOP_ADDR0, u32Value & 0xffff);
    MHal_RLD_WriteReg(STRLD_TOP_ADDR1, u32Value >> 16 );
}

/******************************************************************************/
/// Set the address of the bottom field.
/// @param u8Value \b IN The SDRAM address of bottom field.
/******************************************************************************/
void MHal_RLD_SetBottomFieldAddress(U32 u32Value)
{
   // u32Value = u32Value / 8; //move to madp level

    MHal_RLD_WriteReg(STRLD_BOT_ADDR0, u32Value & 0xffff);
    MHal_RLD_WriteReg(STRLD_BOT_ADDR1, u32Value >> 16);
}

/******************************************************************************/
/// Set the output address.
/// @param u32Value \b IN The output SDRAM address.
/******************************************************************************/
void MHal_RLD_SetOutputAddress(U32 u32Value)
{
    U32 u32Address;
    //u32VirtualAddress = (U32)ioremap(u32Value, 65536);
    //u32KernelAddress = (U32)kmalloc(1024, 0 );
    //RLD_PRINT("Phy addr %lx Kernel addr: %lx Kmalloc address %lx\n", u32Value, u32VirtualAddress, u32KernelAddress);

    u32Address = u32Value;
    u32Value = u32Value / 8;
    //u32Value = 0x0a00;

    //memset((void *)u32Address, 0xff, 65536);
    MHal_RLD_WriteReg(STRLD_OUTPUT_ADDR0, u32Value & 0xffff);
    MHal_RLD_WriteReg(STRLD_OUTPUT_ADDR1, u32Value >> 16);
}

/******************************************************************************/
/// Enable the out-of-width ECO.
/******************************************************************************/
void MHal_RLD_SetOutOfWidthPatch(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_PATCH); // higher 8 bits
    u32RegValue |= MASK_OUT_OF_WIDTH_PATCH;      // out of width eco
    MHal_RLD_WriteReg( STRLD_PATCH, u32RegValue );

}

#if 0
/******************************************************************************/
/// Enable the 8-byte alignment ECO.
/******************************************************************************/
void MHal_RLD_Set8ByteAlignmentPatch()
{
    U8 u8Value = MDrv_ReadByte(STRLD_PATCH);

    u8Value |= MASK_8BYTE_ALIGHMENT_PATCH;
    MDrv_WriteByte(STRLD_PATCH, u8Value);
}
#endif

/******************************************************************************/
/// Enable the NonModify Color ECO.
/******************************************************************************/
void MHal_RLD_SetNonModifyingPatch(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_PATCH); // higher 8 bits
    u32RegValue |= MASK_NON_MODIFYING_PATCH;   // color reduction eco bit 6
    MHal_RLD_WriteReg( STRLD_PATCH, u32RegValue );
}


/******************************************************************************/
/// Enable color reduction ECO.
/******************************************************************************/
void MHal_RLD_SetColorReductionPatch(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_PATCH); // higher 8 bits
    u32RegValue |= MASK_COLOR_REDUCTION_PATCH;   // color reduction eco bit 6
    MHal_RLD_WriteReg( STRLD_PATCH, u32RegValue );

}

/******************************************************************************/
/// Enable color mapping ECO.
/******************************************************************************/
void MHal_RLD_SetColorMappingPatch(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_PATCH); // higher 8 bits
    u32RegValue |= MASK_COLOR_MAPPING_PATCH;    // keep maptable eco bit 7
    MHal_RLD_WriteReg( STRLD_PATCH, u32RegValue );
}

/******************************************************************************/
/// Disable color mapping ECO.
/******************************************************************************/
void MHal_RLD_ClearColorMappingPatch(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_PATCH); // higher 8 bits
    u32RegValue &= ~(MASK_COLOR_MAPPING_PATCH);
    MHal_RLD_WriteReg( STRLD_PATCH, u32RegValue );
}

/******************************************************************************/
/// Enable EndLastData Patch.
/******************************************************************************/
void MHal_RLD_SetEndLastDataPatch(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_PATCH); // higher 8 bits
    u32RegValue |= MASK_END_LASTDATA_PATCH;
    MHal_RLD_WriteReg( STRLD_PATCH, u32RegValue );
}

/******************************************************************************/
/// Get return value.
/// @return The 0x29e0 register value
/******************************************************************************/
U8 MHal_RLD_GetReturn(void)
{
    U32 u32RegValue;
    u32RegValue = MHal_RLD_ReadReg(STRLD_RETURN); // higher 8 bits
    return u32RegValue & 0x00ff;
}


/******************************************************************************/
/// Wait for return value.
/// @return The 0x29e0 register value
/******************************************************************************/
U8 MHal_RLD_WaitReturn(void)
{
    U32 u32RegValue;
    U16 u16ElpseTime;
    u32RegValue = MHal_RLD_ReadReg(STRLD_RETURN); // higher 8 bits
    u16ElpseTime = 0;
    while((u32RegValue& 0x0001) == 0x0)
    {
        msleep_interruptible(RLD_POLL_INTERVAL);
        u16ElpseTime+=RLD_POLL_INTERVAL;
        if (u16ElpseTime > RLD_RUN_TIME)
            break;
        u32RegValue = MHal_RLD_ReadReg(STRLD_RETURN);
    }
    //RLD_PRINT("Wait Time:%x\n",u16ElpseTime);
    return u32RegValue & 0x00ff;
}


/******************************************************************************/
/// Set 2-to-4 color mapping table
/// @param u16Value \b IN The value of the mapping value.
/******************************************************************************/
void MHal_RLD_SetMappingTable2to4(U16 u16Value)
{
    U16 u16MapValue;

    u16MapValue = (u16Value & 0x000F) | ((u16Value & 0x00F0) << 4);
    MHal_RLD_WriteReg(STRLD_CLUT_2TO4_0, u16MapValue);

    u16MapValue = ((u16Value & 0x0F00) >> 8)|((u16Value & 0xF000)>> 4);

    MHal_RLD_WriteReg(STRLD_CLUT_2TO4_1, u16MapValue);
}

/******************************************************************************/
/// Set 2-to-8 color mapping table
/// @param u32Value \b IN The value of the mapping value.
/******************************************************************************/
void MHal_RLD_SetMappingTable2to8(U32 u32Value)
{
    MHal_RLD_WriteReg(STRLD_CLUT_2TO8_0, u32Value & 0xffff);
    MHal_RLD_WriteReg(STRLD_CLUT_2TO8_1, u32Value>>16);

}

/******************************************************************************/
/// Set 4-to-8 color mapping table
/// @param u32Value0 \b IN The value of the mapping value, entries  0 ~  3
/// @param u32Value1 \b IN The value of the mapping value, entries  4 ~  7
/// @param u32Value2 \b IN The value of the mapping value, entries  8 ~ 11
/// @param u32Value3 \b IN The value of the mapping value, entries 12 ~ 16
/******************************************************************************/
void MHal_RLD_SetMappingTable4to8(U32 u32Value0, U32 u32Value1, U32 u32Value2, U32 u32Value3)
{
    MHal_RLD_WriteReg(STRLD_CLUT_4TO8_0, u32Value0 & 0xffff);
    MHal_RLD_WriteReg(STRLD_CLUT_4TO8_1, u32Value0 >> 16);
    MHal_RLD_WriteReg(STRLD_CLUT_4TO8_2, u32Value1 & 0xffff);
    MHal_RLD_WriteReg(STRLD_CLUT_4TO8_3, u32Value1 >> 16);
    MHal_RLD_WriteReg(STRLD_CLUT_4TO8_4, u32Value2 & 0xffff);
    MHal_RLD_WriteReg(STRLD_CLUT_4TO8_5, u32Value2 >> 16);
    MHal_RLD_WriteReg(STRLD_CLUT_4TO8_6, u32Value3 & 0xffff);
    MHal_RLD_WriteReg(STRLD_CLUT_4TO8_7, u32Value3 >> 16);
}


void MHal_RLD_2BS_Reset(void)
{
    MHal_RLD_WriteReg(STRLD_PATCH_06 , 0x00);
    MHal_RLD_WriteReg(STRLD_PATCH , 0x00);

    MHal_RLD_WriteReg(RLD_MPEG_CTRL , 0x00);
    MHal_RLD_WriteReg(RLD_MPEG_CTRL , 0x82);
}

void MHal_RLD_2BS_Start(void)
{
    MHal_RLD_WriteReg(RLD_MPEG_CTRL , 0x83);
    MHal_RLD_WriteReg(RLD_MPEG_CTRL , 0x82);    
}

U8   MHal_RLD_2BS_GetReturn(void)
{
    return MHal_RLD_ReadReg(RLD_MPEG_RETURN);
}

void MHal_RLD_2BS_SetCtrl(U8 u8BotField)
{
    U8 u8Value = 0x05; // default [0]two lines [2]kill out of height
    if(u8BotField == 1)
    {
        u8Value |= MASK_BOTTOM_FIELD;
    }
    else
    {
        u8Value &= ~MASK_BOTTOM_FIELD;
    }

    /*if(u8Mpeg4Mode == 1)
        u8Value |= MASK_MPEG4_MODE;
    else
    u8Value &= ~MASK_MPEG4_MODE;*/

    MHal_RLD_WriteReg(RLD_MPEG_CTRL1 , u8Value);

}

void MHal_RLD_SetNonModifying_Color(U8 u8Value)
{
    if(u8Value == 1)
    {
        U8 u8Value = MHal_RLD_ReadReg(STRLD_PATCH);
        u8Value |= MASK_NON_MODIFYING_FLAG;
        MHal_RLD_WriteReg(STRLD_PATCH, u8Value);

        u8Value = MHal_RLD_ReadReg(STRLD_TRANSPARENT_KEY);
        u8Value = 0x01;    // only for color "0x01"
        MHal_RLD_WriteReg(STRLD_TRANSPARENT_KEY, u8Value);
    }
    else
    {
        U8 u8Value = MHal_RLD_ReadReg(STRLD_PATCH);
        u8Value &= ~MASK_NON_MODIFYING_FLAG;
        MHal_RLD_WriteReg(STRLD_PATCH, u8Value);
    }
}

#undef DRVRLD_C
