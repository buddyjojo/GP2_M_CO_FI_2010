////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ( ß¡±MStar Confidential Information ßÆØ) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#include "datatype.h"
#include "drvGlobal.h"

#define ASSERT(expr)

const unsigned char g_str_assert[] = "ASSERT: %s %d\n";

U8 __MDrv_ReadByte( U16 u16Reg )
{
	return (u16Reg & 0x01) ? (RIU[u16Reg - 1] >> 8) : RIU[u16Reg];
}

U16 __MDrv_Read2Byte( U16 u16Reg )
{
	return __builtin_expect( (u16Reg & 0x01), 0 ) ?
		((RIU[u16Reg - 1] >> 8) + (RIU[u16Reg + 1] << 8)) : RIU[u16Reg];
}

U32 __MDrv_Read3Byte( U16 u16Reg )
{
	return __builtin_expect( (u16Reg & 0x01), 0 ) ?
		((RIU[u16Reg - 1] >> 8) + (RIU[u16Reg + 1] << 8)) :
		((RIU[u16Reg]) + ((RIU[u16Reg + 2] & 0xFF) << 16));
}

U32 __MDrv_Read4Byte( U16 u16Reg )
{
	return __builtin_expect( (u16Reg & 0x01), 0 ) ?
		((RIU[u16Reg - 1] >> 8) + (RIU[u16Reg + 1] << 8) + (RIU[u16Reg + 3] << 24)) :
		((RIU[u16Reg]) + (RIU[u16Reg + 2] << 16));
}

void __MDrv_WriteByte( U16 u16Reg, U8 u8Value )
{
	if ( (u16Reg & 0x01) )
	{
		u16Reg--;
		RIU[u16Reg] = (RIU[u16Reg] & 0x00FF) + (u8Value << 8);
	}
	else
	{
		RIU[u16Reg] = (RIU[u16Reg] & 0xFF00) + (u8Value);
	}
}

void __MDrv_Write2Byte( U16 u16Reg, U16 u16Value )
{
	if ( __builtin_expect( (u16Reg & 0x01), 0 ) )
	{
		u16Reg--;
		RIU[u16Reg] = (RIU[u16Reg] & 0x00FF) + (u16Value << 8);
		u16Reg += 2;
		RIU[u16Reg] = (RIU[u16Reg] & 0xFF00) + (u16Value >> 8);
	}
	else
	{
		RIU[u16Reg] = u16Value;
	}
}

void __MDrv_Write3Byte( U16 u16Reg, U32 u32Value )
{
	if ( __builtin_expect( (u16Reg & 0x01), 0 ) )
	{
		u16Reg--;
		RIU[u16Reg] = (RIU[u16Reg] & 0x00FF) + (U16)(u32Value << 8);
		u16Reg += 2;
		RIU[u16Reg] = (U16)(u32Value >> 8);
	}
	else
	{
		RIU[u16Reg] = u32Value;
		u16Reg += 2;
		RIU[u16Reg] = (RIU[u16Reg] & 0xFF00) + (U8)(u32Value >> 16);
	}
}

void __MDrv_Write4Byte( U16 u16Reg, U32 u32Value )
{
	if ( __builtin_expect( (u16Reg & 0x01), 0 ) )
	{
		u16Reg--;
		RIU[u16Reg] = (RIU[u16Reg] & 0x00FF) + (U16)(u32Value << 8);
		u16Reg += 2;
		u32Value >>= 8;
		RIU[u16Reg] = (U16)(u32Value);
		u16Reg += 2;
		RIU[u16Reg] = (RIU[u16Reg] & 0xFF00) + (U8)(u32Value >> 16);

	}
	else
	{
		RIU[u16Reg] = u32Value;
		RIU[u16Reg + 2] = (u32Value >> 16);
	}
}

void MDrv_WriteRegTbl( const MS_REG_INIT * pRegTable )
{
	U8 u8Length;
	U16 u16Index; // register index
	U16 u16Dummy;
//	U8 c;

	u16Dummy = 2000;

	do
	{
		u16Index = ((pRegTable[0] << 8) + pRegTable[1]);
		if (u16Index == 0xFFFF) // check end of table
			break;

		u8Length = u16Index >> 14;
		u16Index &= 0x3FFF;

		switch ( u8Length )
		{
			case 3:
				MDrv_Write4Byte( u16Index, pRegTable[2] +
						(pRegTable[3] << 8) +
						(pRegTable[4] << 16) +
						(pRegTable[5] << 24) );
				pRegTable += 6;
				break;

			case 2:
				MDrv_Write3Byte( u16Index, pRegTable[2] +
						(pRegTable[3] << 8) +
						(pRegTable[4] << 16) );
				pRegTable += 5;
				break;

			case 1:
				MDrv_Write2Byte( u16Index, pRegTable[2] +
						(pRegTable[3] << 8) );
				pRegTable += 4;
				break;

			case 0:
				MDrv_WriteByte( u16Index, pRegTable[2] );
				pRegTable += 3;
				break;
			default:
				ASSERT(0);
				break;
		}
	} while (--u16Dummy > 0);
}
