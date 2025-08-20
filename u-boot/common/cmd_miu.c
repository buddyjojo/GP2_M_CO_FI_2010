////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    miu.c
/// @brief  MIU Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <common.h>
#include <command.h>

unsigned char MIU_Read_DQS_Phase(unsigned int u32MIU_ID)
{
    unsigned int u16PhaseVal = 0;

    if (0 == u32MIU_ID)      // Read DQS phase of MIU0
    {
        u16PhaseVal = *((volatile unsigned int *)0xBF202460) & 0xFF;
    }
    else                     // Read DQS phase of MIU1
    {
        u16PhaseVal = *((volatile unsigned int *)0xBF200C60) & 0xFF;
    }
    return u16PhaseVal;
}

//dhjung LGE
void print_dqsvalue(void)
{
	printf("MIU0 DQS: 0x%x", MIU_Read_DQS_Phase(0));
	printf(", MIU1 DQS: 0x%x\n", MIU_Read_DQS_Phase(1));
}

int do_showdqs ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	printf("\n"); print_dqsvalue(); printf("\n");

	return 0;
}

U_BOOT_CMD(
	showdqs,      2,      0,      do_showdqs,
	"showdqs\t- get dqs value from auto dqs\n",
	" - \n"
);
