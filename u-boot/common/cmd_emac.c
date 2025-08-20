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
/// @file   cmd_emac.c
/// @brief  u-boot EMAC Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------#include "e1000.h"

/*
 * EMAC Read/Write Utilities
 */
#include <common.h>
#include <command.h>

/*-----------------------------------------------------------------------
 * Definitions
 */
#define EMAC_INPUT_LEN					2
#define EMAC_ADDR_INPUT_LEN				7

#define MAX_EMAC_BYTES					0x1000	/* Maximum number of bytes we can handle */

extern int MDrv_EMAC_initialize(bd_t * bis);
extern u8 MY_MAC[6];
/*
 * emac init
 *
 * Syntax:
 *   estart
 */
#define MACADDR_ENV    "macaddr"
#define MACADDR_FORMAT "XX:XX:XX:XX:XX:XX"

int do_emac (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    bd_t * bis=NULL;
    char * macaddr;

    if (   NULL != (macaddr = getenv(MACADDR_ENV))
        && strlen(macaddr) == strlen(MACADDR_FORMAT)
        && ':' == macaddr[2]
        && ':' == macaddr[5]
        && ':' == macaddr[8]
        && ':' == macaddr[11]
        && ':' == macaddr[14]
        )
    {
        macaddr[2]  = '\0';
        macaddr[5]  = '\0';
        macaddr[8]  = '\0';
        macaddr[11] = '\0';
        macaddr[14] = '\0';

        MY_MAC[0]   = (u8)simple_strtoul(&(macaddr[0]),  NULL, 16);
        MY_MAC[1]   = (u8)simple_strtoul(&(macaddr[3]),  NULL, 16);
        MY_MAC[2]   = (u8)simple_strtoul(&(macaddr[6]),  NULL, 16);
        MY_MAC[3]   = (u8)simple_strtoul(&(macaddr[9]),  NULL, 16);
        MY_MAC[4]   = (u8)simple_strtoul(&(macaddr[12]), NULL, 16);
        MY_MAC[5]   = (u8)simple_strtoul(&(macaddr[15]), NULL, 16);

        /* set back to ':' or the environment variable would be destoried */ // REVIEW: this coding style is dangerous
        macaddr[2]  = ':';
        macaddr[5]  = ':';
        macaddr[8]  = ':';
        macaddr[11] = ':';
        macaddr[14] = ':';
    }

    MDrv_EMAC_initialize(bis);
//dhjung LGE
//	printf("(Re)start EMAC...\n");

	return 0;
}

int do_setMac(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    char cmdline_buf[]          = "setenv "MACADDR_ENV" "MACADDR_FORMAT;
    const char cmdline_format[] = "setenv "MACADDR_ENV" %02X:%02X:%02X:%02X:%02X:%02X";

    if ((argc < EMAC_ADDR_INPUT_LEN) || (argc > EMAC_ADDR_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    MY_MAC[0]    = simple_strtoul(argv[1], NULL, 16);
    MY_MAC[1]    = simple_strtoul(argv[2], NULL, 16);
    MY_MAC[2]    = simple_strtoul(argv[3], NULL, 16);
    MY_MAC[3]    = simple_strtoul(argv[4], NULL, 16);
    MY_MAC[4]    = simple_strtoul(argv[5], NULL, 16);
    MY_MAC[5]    = simple_strtoul(argv[6], NULL, 16);

    sprintf(cmdline_buf, cmdline_format, MY_MAC[0], MY_MAC[1], MY_MAC[2], MY_MAC[3], MY_MAC[4], MY_MAC[5]);

    run_command(cmdline_buf, 0);
    run_command("saveenv", 0);

   	printf("New MAC Address is %02X:%02X:%02X:%02X:%02X:%02X\n", MY_MAC[0], MY_MAC[1], MY_MAC[2], MY_MAC[3], MY_MAC[4], MY_MAC[5]);

    //Set MAC address ------------------------------------------------------
    //MHal_EMAC_Write_SA1_MAC_Address(sa1[0],sa1[1],sa1[2],sa1[3],sa1[4],sa1[5]);

    return 0;
}


/***************************************************/

U_BOOT_CMD(
	estart, EMAC_INPUT_LEN,	1,	do_emac,
	"estart\t- EMAC start\n",
	"reset\t- reset EMAC controller\n"
	"start\t- start EMAC controller\n"
);

U_BOOT_CMD(
	macaddr, EMAC_ADDR_INPUT_LEN,	1,	do_setMac,
	"macaddr\t- setup EMAC MAC addr\n",
    "XX XX XX XX XX XX"
);
