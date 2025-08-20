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


#if 0
#include <stdlib.h>
#include <assert.h>
#endif
//#include "common.h"
#include "CheckSum.h"

//#define _COUNT4BYTES_
#ifdef _COUNT4BYTES_
#define COUNT_BYTE 4
#else
#define COUNT_BYTE 8
#endif
// Adler-32 has a weakness for short messages with few hundred bytes, because the checksums for these
// messages have a poor coverage of the 32 available bits.
// Problem: The problem is that sum A does not wrap for short messages. The maximum value of A for a 128-byte
// message is 32640, which is below the value 65521 used by the modulo operation.
//unsigned int adler;

void InitCheckSum(unsigned int *p_adler)
{
    *p_adler = 1;
}
//Dn : n-th byte
//s1: 1 + D1 + D2 + D3 + ... + Dn
//s2: (1+D1) + (1+D1+D2) + ... + (1+D1+D2+..+Dn) = n*D1 + (n-1)*D2 + (n-2)*D3 + ... + Dn + n
#if 1
void UpdateCheckSum(unsigned char* ptr, int len, unsigned int *p_adler)
{
    /* RFC 1950 -- Adler-32 checksum */
    typedef unsigned int u32;
    unsigned char * buf = (unsigned char *) ptr;
    const u32 BASE = 65521; //the largest prime number smaller than 2^16
    unsigned int u32adler = *p_adler;
    u32 s1 = u32adler & 0xffff;
    u32 s2 = (u32adler >> 16) & 0xffff;
    /*
    while (len % COUNT_BYTE != 0) {
      s1 += *buf++;
      s2 += s1;
      --len;
    }*/
    do {
        u32 count = (len > 5552 - COUNT_BYTE) ? (5552 - COUNT_BYTE) : len; //5552-8:  the largest number of sums that can be performed without overflowing s2
        len -= count;
        while (count) {
            int i;
            if(len==0 && count<COUNT_BYTE) {
                for (i=0; i<count; i++) {
                    s1 += buf[i];
                    s2 += s1;
                }
                for (; i<8; i++) {
                    s1+=/*buf[i]*/0;
                    s2 += s1;
                }
                count = 0;
                break;
            }
          s1 += buf[0]; s2 += s1;
          s1 += buf[1]; s2 += s1;
          s1 += buf[2]; s2 += s1;
          s1 += buf[3]; s2 += s1;
#ifdef _COUNT4BYTES_
          s1 += 0; s2 += s1;
          s1 += 0; s2 += s1;
          s1 += 0; s2 += s1;
          s1 += 0; s2 += s1;
#else
          s1 += buf[4]; s2 += s1;
          s1 += buf[5]; s2 += s1;
          s1 += buf[6]; s2 += s1;
          s1 += buf[7]; s2 += s1;
#endif
          count -= COUNT_BYTE;
          buf += COUNT_BYTE;
        }
        s1 %= BASE;
        s2 %= BASE;
    } while (len);
    u32adler = (s2 << 16) + s1;
    *p_adler = u32adler;
    // Copy from decoder END
}
#else
void UpdateCheckSum(unsigned char* ptr, int len)
{
    /* RFC 1950 -- Adler-32 checksum */
    typedef unsigned int u32;
    unsigned char * buf = (unsigned char *) ptr;
    const u32 BASE = 65521; //the largest prime number smaller than 2^16
    u32 s1 = adler & 0xffff;
    u32 s2 = (adler >> 16) & 0xffff;
    while (len % COUNT_BYTE != 0) {
      s1 += *buf++;
      s2 += s1;
      --len;
    }
    do {
        u32 count = (len > 5552 - COUNT_BYTE) ? (5552 - COUNT_BYTE) : len; //5552-8:  the largest number of sums that can be performed without overflowing s2
        len -= count;
        while (count) {
          s1 += buf[0]; s2 += s1;
          s1 += buf[1]; s2 += s1;
          s1 += buf[2]; s2 += s1;
          s1 += buf[3]; s2 += s1;
#ifdef _COUNT4BYTES_
          s1 += 0; s2 += s1;
          s1 += 0; s2 += s1;
          s1 += 0; s2 += s1;
          s1 += 0; s2 += s1;
#else
          s1 += buf[4]; s2 += s1;
          s1 += buf[5]; s2 += s1;
          s1 += buf[6]; s2 += s1;
          s1 += buf[7]; s2 += s1;
#endif
          count -= COUNT_BYTE;
          buf += COUNT_BYTE;
        }
        s1 %= BASE;
        s2 %= BASE;
    } while (len);
    adler = (s2 << 16) + s1;
    // Copy from decoder END
}
#endif
#if 0
void crco_add_frame(uchar* y, uchar* u, uchar* v,
                    int width, int height,
                    int stride_y, int stride_uv,
                    int active_x, int active_y
                    )
{
    int i;
    uchar *src_y;
    uchar *src_u;
    uchar *src_v;

    src_y = (uchar*)y + active_y*stride_y+active_x;
    src_u = (uchar*)u + (active_y>>1)*stride_uv+(active_x>>1);
    src_v = (uchar*)v + (active_y>>1)*stride_uv+(active_x>>1);
    for (i=0; i<height; i++) {
        //fwrite(src_y, 1, width, ofp);
        UpdateCheckSum(src_y, width);
        src_y += stride_y;
    }
    for (i=0; i<height/2; i++) {
        UpdateCheckSum(src_u, width/2);
        src_u += stride_uv;
    }
    for (i=0; i<height/2; i++) {
        UpdateCheckSum(src_v, width/2);
        src_v += stride_uv;
    }
}
#endif
