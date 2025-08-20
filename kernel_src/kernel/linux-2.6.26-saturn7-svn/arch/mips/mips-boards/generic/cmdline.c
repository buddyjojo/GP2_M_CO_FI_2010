/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 1999,2000 MIPS Technologies, Inc.  All rights reserved.
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Kernel command line creation using the prom monitor (YAMON) argc/argv.
 */
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include <asm/bootinfo.h>

extern int prom_argc;
extern int *_prom_argv;

//dhjung LGE
extern unsigned long simple_strtoul(const char *, char **, unsigned int);
extern int printk(const char *, ...);

//dhjung LGE
char gpargptr[32];

/*
 * YAMON (32-bit PROM) pass arguments and environment as 32-bit pointer.
 * This macro take care of sign extension.
 */
#define prom_argv(index) ((char *)(long)_prom_argv[(index)])

char * __init prom_getcmdline(void)
{
	return &(arcs_cmdline[0]);
}

void  __init prom_init_cmdline(void)
{
	char *cp;
	int actr;

	actr = 1; /* Always ignore argv[0] */

	cp = &(arcs_cmdline[0]);
	while(actr < prom_argc) {
		strcpy(cp, prom_argv(actr));
		cp += strlen(prom_argv(actr));
		*cp++ = ' ';
		actr++;
	}
	if (cp != &(arcs_cmdline[0])) {
		/* get rid of trailing space */
		--cp;
		*cp = '\0';
	}
}

//dhjung LGE
char __init *prom_get_arg(char *cmd)
{
	int i;
	char *argptr, *cmdptr;

	argptr = prom_getcmdline();
	if ((cmdptr = strstr(argptr, cmd)) == NULL) {
		printk(KERN_DEBUG "can't find \"%s\" in cmdline\n", cmd);
		return cmdptr;
	}

	cmdptr += strlen(cmd);
	for(i = 0; *cmdptr; cmdptr++, i++) {
		gpargptr[i] = *cmdptr;
		if (gpargptr[i] == ' ') {
			gpargptr[i] = '\0';
			break;
		}
	}

	return &gpargptr[0];
}

//dhjung LGE
unsigned long __init prom_get_argvalue(char *arg)
{
	char *argv;

	argv = prom_get_arg(arg);
	if (argv == NULL)
		return 0;

	return simple_strtoul(argv, NULL, 10);
}
