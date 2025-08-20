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


#ifndef _DRV_M4VE_ST_H_
#define _DRV_M4VE_ST_H_


#define MAX_BITS_FRAME 3

typedef struct{
    int width;
    int height;
    int BitRate;
    unsigned long BitsBuffStart;
    unsigned long BitsBuffSize;
    double video_framerate;
}PVR_Info;


typedef struct {
    unsigned long miuAddress;    //hardware physical
    unsigned long miuPointer;    //need to OR 0xA0000000
    unsigned long virtual_addr;
    long size;
    int is_frame_done;
//    int is_buffull; //1: M4VE bitsbuf all used, 0: M4VE has available bitsbuf
    int is_more_bits; //1: there are more bitstream packet;  0: this is the last bitstream packet
    unsigned char  voptype;
    unsigned long IVOP_address;
} BITSFRAME_INFO;


#endif
