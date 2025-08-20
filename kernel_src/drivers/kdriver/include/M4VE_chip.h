#ifndef _M4VE_CHIP_H_
#define _M4VE_CHIP_H_
/*
This file includes all definitions which we use in MHal, MDrv, and MAdp layer.
We also have some definitions only belonged to MHal(in drvM4VE.h), MDrv(in mdrv_m4ve.c), and MAdp(in madp_m4ve.c)
*/
//#define _M4VE_TRITON_
#ifdef _M4VE_TRITON_
#define _DIP_INPUT_     //open in Triton
#define _PARA_FILEOUT_  //open in Triton
#define _ENABLE_AVIMUX_
#define _NO_WAIT_FRAMEDONE_
#define _AEON_PLATFORM_
#endif

//#define _M4VE_BIG2_
#ifdef _M4VE_BIG2_
#define _MIPS_PLATFORM_
#define _NO_WAIT_FRAMEDONE_
#define _FRAMEPTR_IN_
#define _MUL_OUTBITS_BUF_
#endif

//#define _M4VE_OLYMPUS_  //define this when enable on Olympus ARM
#ifdef _M4VE_OLYMPUS_
#define _EN_CRC_
#define _YUVGEN_
#define _MUL_OUTBITS_BUF_
#define _TWO_CORES_
#define _ARM_PLATFORM_
#endif

#define _M4VE_T3_
#ifdef _M4VE_T3_
//#define _AEON_PLATFORM_
#define _MIPS_PLATFORM_
#define _NO_WAIT_FRAMEDONE_
#define _FRAMEPTR_IN_
#define _MUL_OUTBITS_BUF_
#endif


//#define _DIP_INPUT_ //open in Triton
//#define _PARA_FILEOUT_ //open in Triton
#define X_RATE_CONTROL
//#define _ENABLE_AVIMUX_
//#define _NO_WAIT_FRAMEDONE_
#endif

