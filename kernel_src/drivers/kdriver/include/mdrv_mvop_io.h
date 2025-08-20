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

#ifndef _MDRV_MVOP_IO_H_
#define _MDRV_MVOP_IO_H_


/* Use 'v' as magic number */
#define MVOP_IOC_MAGIC           'w'

#define MVOP_IOC_INIT            	_IO(MVOP_IOC_MAGIC, 0)
#define MVOP_IOC_HARDWIREMODE       _IOW(MVOP_IOC_MAGIC, 1,int)
#define MVOP_IOC_HARDWIRECLIPMODE   _IOW(MVOP_IOC_MAGIC, 2,int)
#define MVOP_IOC_MCUMODE         	_IOW(MVOP_IOC_MAGIC, 3,int)
#define MVOP_IOC_SETTIMING          _IOW(MVOP_IOC_MAGIC, 4,int)
#define MVOP_IOC_ENABLEBLACKBG      _IO(MVOP_IOC_MAGIC, 5)
#define MVOP_IOC_ENABLEINTERLACE	_IOW(MVOP_IOC_MAGIC, 6,int)
#define MVOP_IOC_SETMVOPSYNCCLK     _IOW(MVOP_IOC_MAGIC, 7,int)
#define MVOP_IOC_SETMLINKMODE       _IOW(MVOP_IOC_MAGIC, 8,int)
#define MVOP_IOC_SETMLINKBYPASS     _IOW(MVOP_IOC_MAGIC, 9,int)
#define MVOP_IOC_SETMVOPOPERATION   _IOW(MVOP_IOC_MAGIC, 10,int)
#define MVOP_IOC_HARDWAREMODE       _IO(MVOP_IOC_MAGIC,  11)
#define MVOP_IOC_HARDWARECLIPMODE   _IOW(MVOP_IOC_MAGIC, 12,int)
#define MVOP_IOC_SET_TEST_PATTERN  	_IOW(MVOP_IOC_MAGIC, 13,int)	// lemonic LGE 080908
#define MVOP_IOC_SETYUVADDR         _IOW(MVOP_IOC_MAGIC, 14,int)
#define MVOP_IOC_SETUVSHIT          _IOW(MVOP_IOC_MAGIC, 15,int)//LGE gbtogether(081217) ->to fix ChromaArtifact when 420to422 by Junyou.Lin
#define MVOP_IOC_SETH264_ORDER      _IOW(MVOP_IOC_MAGIC, 16,int)//junyou add for H264 thumbnail<20091001>
#define MVOP_IOC_MAXNR           17


#endif
