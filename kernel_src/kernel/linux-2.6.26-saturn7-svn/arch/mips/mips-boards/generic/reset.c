/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 1999,2000 MIPS Technologies, Inc.  All rights reserved.
 *
 * ########################################################################
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * ########################################################################
 *
 * Reset the MIPS boards.
 *
 */
#include <linux/pm.h>

#include <asm/io.h>
#include <asm/reboot.h>
#include <asm/mips-boards/generic.h>
#if defined(CONFIG_MIPS_ATLAS)
#include <asm/mips-boards/atlas.h>
#endif

//dhjung LGE
#if defined(CONFIG_Titania3)
#define REG_WDT_BASE	0xbf006000
#endif

#if defined(CONFIG_Titania2)
#define REG_WDT_BASE	0xbf8078c0
#endif

static void mips_machine_restart(char *command);
static void mips_machine_halt(void);
#if defined(CONFIG_MIPS_ATLAS)
static void atlas_machine_power_off(void);
#endif

static void mips_machine_restart(char *command)
{
//dhjung LGE
#if defined(CONFIG_Titania2)
#if 0
	unsigned int __iomem *softres_reg =
		ioremap(SOFTRES_REG, sizeof(unsigned int));

	__raw_writel(GORESET, softres_reg);
#else
	*(volatile unsigned long*)(REG_WDT_BASE)        = 0x5678;   //WDT_KEY
	*(volatile unsigned long*)(REG_WDT_BASE + 4)    = 0xFFFE;   //WDT_SEL
	*(volatile unsigned long*)(REG_WDT_BASE + 12)   = 0x6;      //WDT_CTRL
	*(volatile unsigned long*)(REG_WDT_BASE + 8)    = 0x10;     //WDT_INT_SEL
#endif
#endif

#if defined(CONFIG_Titania3)
    unsigned int nSecond = 1;
    unsigned int nCount = nSecond * 12000000;  // 12MHz

    *(volatile unsigned long*)(REG_WDT_BASE + 0x10)   = nCount & 0xFFFF;           // WDT_PERIOD_L
    *(volatile unsigned long*)(REG_WDT_BASE + 0x14)   = (nCount >> 16) & 0xFFFF;   // WDT_PERIOD_H
#endif
}

static void mips_machine_halt(void)
{
//dhjung LGE
#if defined(CONFIG_Titania2)
#if 0
	unsigned int __iomem *softres_reg =
		ioremap(SOFTRES_REG, sizeof(unsigned int));

	__raw_writel(GORESET, softres_reg);
#else
	*(volatile unsigned long*)(REG_WDT_BASE)        = 0x5678;   //WDT_KEY
	*(volatile unsigned long*)(REG_WDT_BASE + 4)    = 0xFFFE;   //WDT_SEL
	*(volatile unsigned long*)(REG_WDT_BASE + 12)   = 0x6;      //WDT_CTRL
	*(volatile unsigned long*)(REG_WDT_BASE + 8)    = 0x10;     //WDT_INT_SEL
#endif
#endif

#if defined(CONFIG_Titania3)
    unsigned int nSecond = 1;
    unsigned int nCount = nSecond * 12000000;  // 12MHz

    *(volatile unsigned long*)(REG_WDT_BASE +  0x10)   = nCount & 0xFFFF;           // WDT_PERIOD_L
    *(volatile unsigned long*)(REG_WDT_BASE +  0x14)   = (nCount >> 16) & 0xFFFF;   // WDT_PERIOD_H
#endif

}

#if defined(CONFIG_MIPS_ATLAS)
static void atlas_machine_power_off(void)
{
	unsigned int __iomem *psustby_reg = ioremap(ATLAS_PSUSTBY_REG, sizeof(unsigned int));

	writew(ATLAS_GOSTBY, psustby_reg);
}
#endif

void mips_reboot_setup(void)
{
	_machine_restart = mips_machine_restart;
	_machine_halt = mips_machine_halt;
#if defined(CONFIG_MIPS_ATLAS)
	pm_power_off = atlas_machine_power_off;
#endif
#if defined(CONFIG_MIPS_MALTA) || defined(CONFIG_MIPS_SEAD)
	pm_power_off = mips_machine_halt;
#endif
}
