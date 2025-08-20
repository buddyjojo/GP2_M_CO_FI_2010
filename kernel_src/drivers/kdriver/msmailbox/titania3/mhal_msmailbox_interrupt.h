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

#ifndef _MHAL_MSMAILBOX_INTERRUPT_H__
#define _MHAL_MSMAILBOX_INTERRUPT_H__

#include "mdrv_types.h"
#include "mhal_msmailbox_reg.h"

typedef struct
{
    void                        (*IEnable)  (U32);
    void                        (*IDisable) (U32);
    U32                         (*IStatus)  (U32);
    void                        (*FEnable)  (U32);
    void                        (*FDisable) (U32);
    U32                         (*FStatus)  (U32);
    void                        (*FClear)   (U32);

    void                        (*IAEON)    (void);
    void                        (*IMIPS)    (void);
    void                        (*ICO_PROC)     (U32);
    void                        (*IMIPS_VPE0)   (U32);
    void                        (*IPROC)    (U32, U32);
} hal_int_ops_t;


typedef struct
{
    volatile ms_reg_t           *REG;
    volatile ms_reg_t           *REG_CPU_INT;
    hal_int_ops_t               ops;
} hal_int_t;

#endif

