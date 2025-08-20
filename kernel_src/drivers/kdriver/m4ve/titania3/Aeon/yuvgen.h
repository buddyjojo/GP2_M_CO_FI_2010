#ifndef _YUVGEN_H_
#define _YUVGEN_H_

// #define _MAIN_
#ifndef _MAIN_
#include "drvM4VE.h"
#endif
void yuv2tile(/*FILE *fp*/unsigned char *output, int width, int height, U8 *buff , int Ypart);
void YuvGen(unsigned char seed, int width, int height, volatile unsigned char *buff);

#endif

