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

#ifndef __MHAL_MSMAILBOX_H__
#define __MHAL_MSMAILBOX_H__

#include <asm-mips/system.h>
#include "mhal_msmailbox_reg.h"
#include "mdrv_msmailbox_st.h"


#define MSMAILBOX_REG_BASE              (0x19C0)
#define MSMAILBOX(x)                    WORD2REAL(MSMAILBOX_REG_BASE+(x))

typedef struct
{
    _ERR_CODE_                          (*Fire)(msmailbox_t*, U16 CPU_HostID);
    _ERR_CODE_                          (*Receive)(msmailbox_t*, U16 CPU_HostID);
    _ERR_CODE_                          (*FireLoopback)(msmailbox_t*, U16 CPU_SlaveID);
    _ERR_CODE_                          (*Init)(void);
    void                                (*Overflow)(U16 CPU_HostID, U8 Flag);
} hal_msmailbox_ops_t;


typedef struct
{
    volatile ms_reg_t                   *REG_MAIL;
    hal_msmailbox_ops_t                 ops;
} hal_msmailbox_t;


/* STATE1 BIT */
#define     _BUSY_S(REG)            {\
                                        U16 val; \
                                        mb();\
                                        val = (REG[0x07].W[0]);\
                                        mb();\
                                        (REG[0x07].W[0]) = val | BIT15;\
                                        mb();\
                                    }

#define     _BUSY_C(REG)            {\
                                        U16 val; \
                                        mb();\
                                        val = (REG[0x07].W[0]);\
                                        mb();\
                                        (REG[0x07].W[0]) = val & ~BIT15;\
                                        mb();\
                                    }

#define     _BUSY(REG)              ((REG[0x07].W[0]) & BIT15);


#define     _ERR_S(REG)             {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x07].W[0]);\
                                        mb();\
                                        (REG[0x07].W[0]) = val | BIT14;\
                                        mb();\
                                    }

#define     _ERR_C(REG)             {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x07].W[0]);\
                                        mb();\
                                        (REG[0x07].W[0]) = val & ~BIT14;\
                                        mb();\
                                    }

#define     _ERR(REG)               ((REG[0x07].W[0]) & BIT14)

#define     _OVERFLOW_S(REG)        {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x07].W[0]);\
                                        mb();\
                                        (REG[0x07].W[0]) = val | BIT13;\
                                        mb();\
                                    }

#define     _OVERFLOW_C(REG)        {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x07].W[0]);\
                                        mb();\
                                        (REG[0x07].W[0]) = val & ~BIT13;\
                                        mb();\
                                    }

#define     _OVERFLOW(REG)          ((REG[0x07].W[0]) & BIT13)


#define     _S1_C(REG)              {\
                                        mb();\
                                        (REG[0x07].B[1])=0x00;\
                                        mb();\
                                    }

/* CONTROL BIT */
#define     _FIRE_S(REG)            {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x00].W[0]);\
                                        mb();\
                                        (REG[0x00].W[0]) = val | BIT0;\
                                        mb();\
                                     }

#define     _FIRE_C(REG)            {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x00].W[0]);\
                                        mb();\
                                        (REG[0x00].W[0]) = val & ~BIT0;\
                                        mb();\
                                     }

#define     _FIRE(REG)              ((REG[0x00].W[0]) & BIT0)


#define     _READBACK_S(REG)        {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x00].W[0]);\
                                        mb();\
                                        (REG[0x00].W[0]) = val | BIT1;\
                                        mb();\
                                    }
#define     _READBACK_C(REG)        {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x00].W[0]);\
                                        mb();\
                                        (REG[0x00].W[0]) = val & ~BIT1;\
                                        mb();\
                                    }

#define     _READBACK(REG)          ((REG[0x00].W[0]) & BIT1)


#define     _INSTANT_S(REG)         {\
                                        U16 val;\
                                        mb();\
                                        val =(REG[0x00].W[0]);\
                                        mb();\
                                        ((REG[0x00].W[0]) = val | BIT2;\
                                        mb();\
                                    }

#define     _INSTANT_C(REG)         {\
                                        U16 val;\
                                        mb();\
                                        val = (REG[0x00].W[0]);\
                                        mb();\
                                        (REG[0x00].W[0]) = val & ~BIT2;\
                                        mb();\
                                    }

#define     _INSTANT(REG)           ((REG[0x00].W[0]) & BIT2)


#define     _MAIL(REG,idx)         ((REG[idx].W[0]))
//#define     _MAIL_ADR(arg,idx)     (&hal_msmailbox.REG_##arg[idx])
#endif

