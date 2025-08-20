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

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/kernel.h>
#include "mhal_mad_reg.h"
#include "mhal_semaphore.h"

#include <asm/atomic.h>
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
atomic_t    semCounter = ATOMIC_INIT(0);

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

#define MAD_DEBUG_P1(msg) //msg
#define MAD_DEBUG_P2(msg) //msg
#define MAD_DEBUG_P3(msg) //msg

#define semID 4
#define countExpire 1000000 //wait 1 sec then expire

void MHal_MAD_WriteRegTbl(MST_REG_TYPE *pTable)
{
    U32 count=0;	


    atomic_inc(&semCounter);
    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }

    while(pTable->addr != 0xFFFF)
    {
        if((pTable->addr &0xF0000)== 0xA0000)
	 {
            MHal_MAD_AbsWriteMaskByte((pTable->addr & 0xFFFF), pTable->mask, pTable->value);
	 }	 	
        else if((pTable->addr &0xF0000)== 0xB0000)
        {
            MHal_MAD_AbsWriteMaskByte(((pTable->addr & 0xFFFF)+0x100000), pTable->mask, pTable->value);
        }
        else
        {
            MHal_MAD_WriteMaskByte(pTable->addr, pTable->mask, pTable->value);
        }

        pTable++;
    }

    
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

U16 MHal_MAD_ReadReg(U32 u32Reg)
{
    U32 count=0;	
    U16 u16data;
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    u16data = READ_WORD (MAD_REG_BASE + ((u32Reg) << 1));//CHECK

    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);

    return (u16data);
}

U8 MHal_MAD_ReadByte(U32 u32Reg)
{
    U32 count=0;	
    U8 Data8;	
     
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    Data8 = READ_BYTE (MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1));

    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);

   return Data8;
}


U16 MHal_MAD_DecReadReg(U32 u32Reg)
{
    U32 count=0;	
    U16 u16data;
	
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    u16data = READ_WORD (MAD_REG_BASE + ((u32Reg) << 1));   //CHECK
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
    
    return (u16data);
}

U16 MHal_MAD_SeReadReg(U32 u32Reg)
{
    U32 count=0;	
    U16 u16data;
	
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    u16data =READ_WORD (MAD_REG_BASE + ((u32Reg) << 1));   
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
    
    return (u16data);
}

U8 MHal_MAD_SeReadRegByte(U32 u32Reg)
{
    U8  Data8;
    U32 count=0;	
	
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    Data8 = READ_BYTE (MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1));
    	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
	
    return Data8;
}

void MHal_MAD_WriteReg(U32 u32Reg, U16 u16Val)
{
#if 0 //JUST TEMP modify
    U32 count=0;	

    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
#endif    
    MAD_DEBUG_P3(printk("MHal_MAD_WriteReg(0x%x, 0x%x)\r\n", u32Reg, u16Val));
    WRITE_WORD(MAD_REG_BASE + ((u32Reg) << 1), u16Val); 

#if 0 //JUST TEMP modify
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
#endif    
}

void MHal_MAD_WriteByte(U32 u32Reg, U8 u8Val)
{
#if 0	//JUST TEMP modify 
    U32 count=0;	

    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
#endif    
    WRITE_BYTE(MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), u8Val); 
#if 0    
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)	
    MHal_SEM_Free_Resource(semID);
#endif
}


void MHal_MAD_DecWriteReg(U32 u32Reg, U16 u16Val)
{
    U32 count=0;	
	
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
   }
   WRITE_WORD(MAD_REG_BASE + ((u32Reg) << 1), u16Val);  
    
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_DecWriteRegByte(U32 u32Reg, U8 u8Val)
{
    U32 count=0;	
	
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    WRITE_BYTE(MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), u8Val);  
    
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)	
    MHal_SEM_Free_Resource(semID);
}


void MHal_MAD_SeWriteReg(U16 u32Reg, U16 u16Val)
{
    U32 count=0;	
	
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    WRITE_WORD(MAD_REG_BASE + ((u32Reg) << 1), u16Val);  
    
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)	
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_SeWriteRegByte(U32 u32Reg, U8 u8Val)
{
    U32 count=0;	
	
    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    WRITE_BYTE(MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), u8Val);
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_WriteMaskReg(U16 u16Addr, U16 u16Mask, U16 u16Value)
{
    U16 u16Data;
    U32 count=0;	

    atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    MAD_DEBUG_P3(printk("MHal_MAD_WriteMaskReg(0x%x, 0x%x, 0x%x)\r\n", u16Addr, u16Mask, u16Value));
    u16Data = MAD_REG(u16Addr) & (~u16Mask);
    MAD_REG(u16Addr) = u16Data | (u16Value & u16Mask);

    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_DecWriteRegMask(U32 u32RegAddr, U16 u16Mask, U16 u16Val)
{
    U16 u16RegVal;
    U32 count=0;	
	    
    atomic_inc(&semCounter);
    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    u16RegVal = MHal_MAD_ReadReg(u32RegAddr);
    u16RegVal = ((u16RegVal & (~(u16Mask))) | (u16Val & u16Mask));
    MHal_MAD_WriteReg(u32RegAddr, u16RegVal);	   
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_SeWriteRegMask(U32 u32RegAddr, U16 u16Mask, U16 u16Val)
{
    U16 u16RegVal;
    U32 count=0;	

	atomic_inc(&semCounter);

    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }  
    u16RegVal = MHal_MAD_ReadReg(u32RegAddr);
    u16RegVal = ((u16RegVal & (~(u16Mask))) | (u16Val & u16Mask));
    MHal_MAD_WriteReg(u32RegAddr, u16RegVal);
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_DecWriteRegMaskByte(U32 u32Reg, U8 u8Mask, U8 u8Val)
{
    U32 count=0;	
	    
    atomic_inc(&semCounter);
    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    WRITE_BYTE(MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), (R1BYTE((u32Reg), 0xFF) & ~(u8Mask)) | ((u8Val) & (u8Mask)));
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_DecWriteIntMaskByte(U32 u32Reg, U8 u8Mask)
{
    U32 count=0;	
	    
    atomic_inc(&semCounter);
    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    } 
    WRITE_BYTE(MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), (R1BYTE((u32Reg), 0xFF) & ~(u8Mask)) | 0x20);	
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}


void MHal_MAD_SeWriteRegMaskByte(U32 u32Reg, U8 u8Mask, U8 u8Val)
{
    U32 count=0;	
	    
    atomic_inc(&semCounter);
    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    WRITE_BYTE(MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), (R1BYTE((u32Reg), 0xFF) & ~(u8Mask)) | ((u8Val) & (u8Mask)));
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_WriteMaskByte(U32 u32Reg, U8 u8Mask, U8 u8Val)
{
    U32 count=0;	
	    
    atomic_inc(&semCounter);
    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    WRITE_BYTE(MAD_REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), (R1BYTE((u32Reg), 0xFF) & ~(u8Mask)) | ((u8Val) & (u8Mask)));
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}

void MHal_MAD_AbsWriteMaskReg(U32 u32Reg, U16 u16Mask, U16 u16Val)//JUST TEMP modify U8->U16
{
#if 0 //JUST TEMP modify	 
    U32 count=0;	
   
    atomic_inc(&semCounter);
    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
#endif    
    WRITE_BYTE(REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), (R1BYTE((u32Reg), 0xFF) & ~(u16Mask)) | ((u16Val) & (u16Mask)));
#if 0	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
#endif
}

void MHal_MAD_AbsWriteMaskByte(MS_U32 u32Reg, MS_U8 u8Mask, MS_U8 u8Val)
{
    U32 count=0;	
	    
    atomic_inc(&semCounter);
    while( !MHal_SEM_Get_Resource(semID) )
    {
    	udelay(1);
	if(++count>=countExpire)
	{
		MHal_SEM_Reset_Resource(semID);
		count=0;
	}
    }
    WRITE_BYTE(REG_BASE + ((u32Reg) << 1) - ((u32Reg) & 1), (AbsR1BYTE((u32Reg), 0xFF) & ~(u8Mask)) | ((u8Val) & (u8Mask)));
	
    atomic_dec(&semCounter);
    
    if(atomic_read(&semCounter)==0)
    MHal_SEM_Free_Resource(semID);
}


