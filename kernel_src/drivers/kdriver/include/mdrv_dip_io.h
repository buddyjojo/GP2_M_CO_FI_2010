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


///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_dip_io.h
/// @brief  DIP interface
/// @author MStar Semiconductor Inc.
///
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "mdrv_dip_st.h"

#ifndef __MDRV_DIP_H__
#define __MDRV_DIP_H__

/* Use 's' as magic number */
#define DIP_IOC_MAGIC           'p'


//------------------------------------------------------------------------------
// Signal
//------------------------------------------------------------------------------
#define DIP_IOC_INIT                    _IOW (DIP_IOC_MAGIC, 0x00, U32)
#define DIP_IOC_SET_FRAME_INFO          _IOW (DIP_IOC_MAGIC, 0x01, dip_frameinfo_t)
#define DIP_IOC_INPUT_MODE              _IOW (DIP_IOC_MAGIC, 0x02, U32)
#define DIP_IOC_SET_NRBUF               _IOW (DIP_IOC_MAGIC, 0x03, dip_buf_mgr_t)
#define DIP_IOC_SET_DIBUF               _IOW (DIP_IOC_MAGIC, 0x04, dip_buf_mgr_t)
#define DIP_IOC_GET_HIS                 _IOR (DIP_IOC_MAGIC, 0x05, dip_hist_out_t)
#define DIP_IOC_GET_HIS_DIFF            _IOR (DIP_IOC_MAGIC, 0x06, dip_hist_diff_t)
#define DIP_IOC_GET_DI_OUTPUT_INFO      _IOR (DIP_IOC_MAGIC, 0x07, U32)
#define DIP_IOC_ENABLE_NRDI             _IOW (DIP_IOC_MAGIC, 0x08, U32)
#define DIP_IOC_GET_DI_BUF_COUNT        _IOR (DIP_IOC_MAGIC, 0x09, U32)
#define DIP_IOC_GET_DI_BUF_INFO         _IOR (DIP_IOC_MAGIC, 0x0A, dip_DIbufinfo_t)
#define DIP_IOC_GET_DI_BUF_STATUS       _IOR (DIP_IOC_MAGIC, 0x0B, U32)
#define DIP_IOC_CLEAR_DI_BUF_STATUS     _IOW (DIP_IOC_MAGIC, 0x0C, U32)
#define DIP_IOC_GET_DI_BUF_FRAME_COUNT  _IOR (DIP_IOC_MAGIC, 0x0D, U32)
#define DIP_IOC_SHOW_FIELD_INFO         _IO  (DIP_IOC_MAGIC, 0x0E)

#endif

