/*
 *  Cache-handling routined for MIPS 4K CPUs
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <config.h>
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>

	.set	reorder
/*******************************************************************************
*
* cp0_bev_control
*
* RETURNS : N/A
*/
	.globl	cp0_bev_ctrl
	.ent	cp0_bev_ctrl
cp0_bev_ctrl:
	mfc0	k0,		CP0_STATUS
	li		t0, 	0x00000000
	beq		t0, 	a0, 	bev_to_0	/* argument is 0 */

bev_to_1:
	li		k1, 	ST0_BEV
	or		k0, 	k1
	b		bev_set

bev_to_0:
	li		k1, 	~ST0_BEV
	and		k0, 	k1

bev_set:
	mtc0	k0, 	CP0_STATUS
	j		ra

	.end	cp0_bev_ctrl

/*******************************************************************************
*
* read_cp0conf
*
* RETURNS : CP0 Config Register value
*/

	.globl	read_cp0conf
	.ent	read_cp0conf
read_cp0conf:
	mfc0	v0, 	CP0_CONFIG
	j		ra

	.end	read_cp0conf
