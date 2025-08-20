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
#include <linux/module.h>
#include <linux/fs.h>
#include "mhal_semaphore.h"

static void MHal_WriteByte(unsigned int addr, unsigned short value)
{
    if(addr & 1)
        (*(volatile unsigned char *)(0xbf000000+((addr-1)<<1)+1))=(unsigned char)(value & 0xFF);
    else
        (*(volatile unsigned char *)(0xbf000000+(addr<<1)))=(unsigned char)(value & 0xFF);
}

U32 MHal_Semp_ReadReg32(U32 xoffset)
{
    U32 xoffsetValueL; 
    xoffsetValueL = ( *((U32*)((char*)(0xbf000000) + xoffset*2)))&0x0000FFFF;
	return(xoffsetValueL);	
}

U32 MHal_ReadByte(U32 xoffset)
{
    U32 uHighByte;

	uHighByte = xoffset%2;

	if(uHighByte==1)
	{
	    return (MHal_Semp_ReadReg32(xoffset)>>0x08);
	}
	else
	{
        return (MHal_Semp_ReadReg32(xoffset)&0xFF);
	}

}

///-----------------------------------------------------------------------------
/// Grab the Semaphore for Specific HW IP Resource
/// @param u8SemID \b IN: ID for specific HW IP resource
/// @return Success or not
/// - TRUE, HK51 own the HW IP Resource
/// - FALSE, HK51 fail to grab the HW IP Resource
///-----------------------------------------------------------------------------
//===================================
//   mcuID => SEM_HK51ID    = 0x01
//   mcuID => SEM_HKAeon0ID = 0x02
//   mcuID => SEM_MWAeon1ID = 0x03
//===================================
U8 MHal_SEM_Get_Resource(U8 u8SemID)
{
    //printk("\n MDrv_SEM_Get_Resource ,u8SemID %x\n",u8SemID);
    
    return TRUE; // need check //JUST TEMP modify

    if(u8SemID>MAX_SEM_NUM)
        return FALSE;

    u8SemID <<= 1;
    MHal_WriteByte((SEMP_ADDRESS+u8SemID), SEM_MWAeonID);

    //printk("\n MDrv_SEM_Get_Resource : %x -->  %bx",SEMP_ADDRESS+REG_SEM_ID0+u8SemID,MHal_ReadByte(SEMP_ADDRESS+REG_SEM_ID0+u8SemID));

    if(MHal_ReadByte(SEMP_ADDRESS+u8SemID) == SEM_MWAeonID)        
    {
        //printk("MDrv_SEM_Get_Resource : TRUE\n");
        return TRUE;
    }
    else
    {
        //if(ttttt==1)
        //printk("MDrv_SEM_Get_Resource : FALSE \n");
        return FALSE;
    }
}

///-----------------------------------------------------------------------------
/// Free the Semaphore for Specific HW IP Resource
/// @param u8SemID \b IN: ID for specific HW IP resource
///-----------------------------------------------------------------------------
void MHal_SEM_Free_Resource(U8 u8SemID)
{
    SEMDBG(printk("\n MDrv_SEM_Free_Resource ,u8SemID %x\n",u8SemID));
  
    return; // need check //JUST TEMP modify
  
    if(u8SemID>MAX_SEM_NUM)
    {
        SEMDBG(printk("Invalid Sem ID\n"));
    }
    else
    {
        u8SemID <<= 1;
        MHal_WriteByte((SEMP_ADDRESS+u8SemID), SEM_MWAeonID);

        //printk("\n 1 : MDrv_SEM_Free_Resource : %4bx -->  %bx",REG_SEM_ID0+u8SemID,XBYTE[REG_SEM_ID0+u8SemID]);

        if(MHal_ReadByte(SEMP_ADDRESS+u8SemID) == SEM_MWAeonID)             
        {
            // Free SEM resource
            MHal_WriteByte((SEMP_ADDRESS+u8SemID), 0x00);
        }
        else
        {
            SEMDBG(printk("No Sem Resource Free\n"));
        }

       // printk("\n 2 : MDrv_SEM_Free_Resource : %8lx -->  %bx",REG_SEM_ID0+u8SemID,XBYTE[REG_SEM_ID0+u8SemID]);

    }
}

//===================================
//   value = 0x00 => release
//===================================
void MHal_SEM_Reset_Resource(U8 u8SemID)
{
    SEMDBG(printk("\n MDrv_SEM_Reset_Resource ,u8SemID %x\n",u8SemID));
    
    if(u8SemID>MAX_SEM_NUM)
    {
        SEMDBG(printk("Invalid Sem ID\n"));
    }
    else
    {
        u8SemID <<= 1;
        // Free SEM resource
        MHal_WriteByte((SEMP_ADDRESS+u8SemID), 0x00);

        if(MHal_ReadByte(SEMP_ADDRESS+u8SemID) != 0x00)              
        {
            SEMDBG(printk("No Sem Resource Reset\n"));
        }
    }
}

// to check who own the reource
U8  MHal_SEM_Get_ResourceID(U8 u8SemID)
{
    SEMDBG(printk("\n MDrv_SEM_Get_ResourceID ,u8SemID %x\n",u8SemID));
    
    if(u8SemID>MAX_SEM_NUM)
    {
        SEMDBG(printk("Invalid Sem ID\n"));
    }
    else
    {
        u8SemID <<= 1;
        return ((U8)MHal_ReadByte(SEMP_ADDRESS+u8SemID));
    }

    return 0;
}

