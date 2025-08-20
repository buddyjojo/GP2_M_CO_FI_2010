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

#ifndef __MDRV_RLD_IO_H__
#define __MDRV_RLD_IO_H__

//-------------------------------------------------------------------------------------------------
//  ioctl method
//-------------------------------------------------------------------------------------------------
/* Use 'M' as magic number */
#define RLD_IOC_MAGIC                'r'

#define MDRV_RLD_IOC_RLD_START               _IO  (RLD_IOC_MAGIC, 0x01)
#define MDRV_RLD_IOC_RLD_RESET               _IO  (RLD_IOC_MAGIC, 0x02)
#define MDRV_RLD_IOC_SET_OUTPUTFORMAT        _IOW (RLD_IOC_MAGIC, 0x03,U8)
#define MDRV_RLD_IOC_SET_ENLARGE_RATE        _IOW (RLD_IOC_MAGIC, 0x04,U8)
#define MDRV_RLD_IOC_SET_TRANSPARENT_KEY     _IOW (RLD_IOC_MAGIC, 0x05,U8)
#define MDRV_RLD_IOC_SET_TOPFIELD_LENGTH     _IOW (RLD_IOC_MAGIC, 0x06,U16)
#define MDRV_RLD_IOC_SET_BOTTOMFIELD_LENGTH  _IOW (RLD_IOC_MAGIC, 0x07,U16)
#define MDRV_RLD_IOC_SET_REGION_WIDTH        _IOW (RLD_IOC_MAGIC, 0x08,U16)
#define MDRV_RLD_IOC_SET_REGION_HEIGHT       _IOW (RLD_IOC_MAGIC, 0x09,U16)
#define MDRV_RLD_IOC_SET_REGION_PITCH        _IOW (RLD_IOC_MAGIC, 0x0A,U16)
#define MDRV_RLD_IOC_SET_OBJ_XOFFSET         _IOW (RLD_IOC_MAGIC, 0x0B,U16)

#define MDRV_RLD_IOC_SET_OBJ_YOFFSET         _IOW (RLD_IOC_MAGIC, 0x0C,U16)
#define MDRV_RLD_IOC_SET_REGION_OFFSET       _IOW (RLD_IOC_MAGIC, 0x0D,U8)
#define MDRV_RLD_IOC_SET_REGION_DEPTH        _IOW (RLD_IOC_MAGIC, 0x0E,U8)
#define MDRV_RLD_IOC_SET_REGION_COLORKEYFG   _IOW (RLD_IOC_MAGIC, 0x0F,U8)
#define MDRV_RLD_IOC_SET_REGION_COLORKEYBG   _IOW (RLD_IOC_MAGIC, 0x10,U8)
#define MDRV_RLD_IOC_SET_TOPFIELD_ADDRESS    _IOW (RLD_IOC_MAGIC, 0x11,U32)
#define MDRV_RLD_IOC_SET_BOTTOMFIELD_ADDRESS _IOW (RLD_IOC_MAGIC, 0x12,U32)
#define MDRV_RLD_IOC_SET_OUTPUT_ADDRESS      _IOW (RLD_IOC_MAGIC, 0x13,U32)
#define MDRV_RLD_IOC_SET_MAPPINGTABLE_2TO4   _IOW (RLD_IOC_MAGIC, 0x14,U16)
#define MDRV_RLD_IOC_SET_MAPPINGTABLE_2TO8   _IOW (RLD_IOC_MAGIC, 0x15,U32)
#define MDRV_RLD_IOC_SET_MAPPINGTABLE_4TO8   _IOW (RLD_IOC_MAGIC, 0x16,RLD_4to8Mapping_t)

#define MDRV_RLD_IOC_RLD_GETRETURN           _IOR (RLD_IOC_MAGIC, 0x17,U8)

#define MDRV_RLD_IOC_PATCH_OUTOFWIDTH        _IO  (RLD_IOC_MAGIC, 0x18)
#define MDRV_RLD_IOC_PATCH_ENDLASTDATA       _IO  (RLD_IOC_MAGIC, 0x19)
#define MDRV_RLD_IOC_PATCH_COLORREDUCTION    _IO  (RLD_IOC_MAGIC, 0x1a)
#define MDRV_RLD_IOC_PATCH_SETCOLORMAPPING   _IO  (RLD_IOC_MAGIC, 0x1b)
#define MDRV_RLD_IOC_PATCH_CLRCOLORMAPPING   _IO  (RLD_IOC_MAGIC, 0x1c)

#define MDRV_RLD_IOC_RLD_INIT                _IO  (RLD_IOC_MAGIC, 0x1d)

#define MDRV_RLD_IOC_RLD_SETCOLORKEYFG       _IO  (RLD_IOC_MAGIC, 0x1e)
#define MDRV_RLD_IOC_RLD_SETCOLORKEYBG       _IO  (RLD_IOC_MAGIC, 0x1f)
#define MDRV_RLD_IOC_PATCH_NONMODIFY         _IO  (RLD_IOC_MAGIC, 0x20)

#define MDRV_RLD_IOC_RLD_WAITRETURN          _IOR (RLD_IOC_MAGIC, 0x21,U8)
#define MDRV_RLD_IOC_GET_MMAPINFO            _IOR (RLD_IOC_MAGIC, 0x22,RLD_MMAPInfo_t)
#define MDRV_RLD_IOC_FLUSHDCACHE             _IOW (RLD_IOC_MAGIC, 0x23,RLD_MMAPInfo_t)

#define MDRV_RLD_IOC_2BS_RESET               _IO  (RLD_IOC_MAGIC, 0x30)
#define MDRV_RLD_IOC_2BS_START               _IO  (RLD_IOC_MAGIC, 0x31)
#define MDRV_RLD_IOC_2BS_GETRETURN           _IOR (RLD_IOC_MAGIC, 0x32, U8)
#define MDRV_RLD_IOC_2BS_SETCTRL             _IOW (RLD_IOC_MAGIC, 0x33, U8)

#define MDRV_RLD_IOC_SETNONMODIFYCOLOR			 _IOW (RLD_IOC_MAGIC, 0x40, U8)

#endif // #ifndef __MDRV_RLD_IO_H__
