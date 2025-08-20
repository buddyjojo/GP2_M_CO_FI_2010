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


#include "drvM4VE.h"
#include "udma_share.h"
#include "memmap.h"

#define MAP_FAILED      0
#define OK              0
#define ERROR          -1
#define DEBUG_MSG(x)    printf
#define MEMORY_USAGE_TRACE  printf

#ifndef _AEON_PLATFORM_
U8* global_buf_ptr;
#endif
U32 global_buf_phy;
U32 global_buf_end;
void MMAPInit(U32 buf_start, U32 buf_size) {
#ifdef _WIN32// !defined(_AEON_PLATFORM_) && !defined(_ARM_PLATFORM_) && !defined(_MIPS_PLATFORM_)
    global_buf_ptr = global_buf;
#endif
    global_buf_phy = buf_start;//DRAM_GBUF_BASE; //0x20000
    global_buf_end = global_buf_phy+buf_size;
    printf("MMAPInit: start:0x%x size:0x%x\n", global_buf_phy, buf_size);
}

long MMAPMalloc(U32 size, MEMMAP_t * memmap)
{
    U32 pgsize=16;

    //assert(memmap != NULL);
    printf("in MMAPMalloc 0x%x %d\n", global_buf_phy, size);
    size = (size + (pgsize - 1)) & (~(pgsize - 1));

    memmap->size = size;
    memmap->miuPointer = MAP_FAILED;
    memmap->miuAddress = 0;

    if(global_buf_phy == 0) {
        M4VE_DEBUG("MMAPMalloc: global_buf_phy == 0 \n");
        return ERROR;
    }
#ifdef _WIN32 //!defined(_AEON_PLATFORM_) && !defined(_ARM_PLATFORM_) && !defined(_MIPS_PLATFORM_)
    if(global_buf_ptr == NULL) {
        M4VE_DEBUG("MMAPMalloc: global_buf_ptr == NULL \n");
        return ERROR;
    }
#endif

    memmap->miuAddress = global_buf_phy;
    global_buf_phy += size;

#if defined(_AEON_PLATFORM_) || defined(_ARM_PLATFORM_) || defined(_MIPS_PLATFORM_)
    if (global_buf_phy >= global_buf_end) {
        M4VE_DEBUG("Warning!! M4VE MMAPMalloc over size!!, 0x%x 0x%x 0x%x\n"
            , global_buf_phy, global_buf_end, size);
    }
    memmap->miuPointer = (U8*)addr_phy2log(memmap->miuAddress);//(memmap->miuAddress + MEM_LOGMAPPHY);
#else
    memmap->miuPointer = (U8*)global_buf_ptr;
    global_buf_ptr += memmap->size;
#endif
    MEMORY_USAGE_TRACE("MMAPMalloc:\n");
    MEMORY_USAGE_TRACE("memmap->miuPointer: 0x%08x\n", (U32)memmap->miuPointer);
    MEMORY_USAGE_TRACE("memmap->size      : 0x%08x\n", memmap->size);
    MEMORY_USAGE_TRACE("memmap->miuAddress: 0x%08x\n", memmap->miuAddress);

    return OK;
}

int reg_scan(unsigned short *reg_mask, U32 num_reg, proto_write write_func, proto_read read_func)
{
    U32 i=0, j=0;
    M4VE_DEBUG("get in reg_scan %d\n", num_reg);
    for (i=0; i<num_reg; i++) {
        unsigned short val, temp;
        for (j=1; j<reg_mask[i]; j<<=1) {
            if (!(i==0 && j==1)) {
                val = ((unsigned short)j)&reg_mask[i];
                write_func(i, val);
                read_func(i, &temp);
                if (val != temp) {
                    M4VE_DEBUG("register scan error: reg:%d write:%d read:%d\n", i, val&reg_mask[i], temp);
        }
            } else {
                M4VE_DEBUG("passed reg %d\n", i);
    }
        }
    }
    M4VE_DEBUG("passed reg_scan\n");
    return 0;
}
