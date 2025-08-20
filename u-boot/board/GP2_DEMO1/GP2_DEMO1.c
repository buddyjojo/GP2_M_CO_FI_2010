/*
 * Board initialize code for TANBAC Evaluation board TB0229.
 *
 * (C) Masami Komiya <mkomiya@sonare.it> 2004
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#include <common.h>
#include <command.h>
#include <asm/addrspace.h>
#include <pci.h>

#ifdef CONFIG_GPIO_FOR_DDRSIZE
#define BIT0	                    0x00000001
#define BIT1	                    0x00000002
#define BIT2	                    0x00000004
#define BIT3	                    0x00000008
#define BIT4	                    0x00000010
#define BIT5	                    0x00000020
#define BIT6	                    0x00000040
#define BIT7	                    0x00000080
#define BIT8	                    0x00000100
#define BIT9	                    0x00000200
#define BIT10	                    0x00000400
#define BIT11	                    0x00000800
#define BIT12	                    0x00001000
#define BIT13	                    0x00002000
#define BIT14	                    0x00004000
#define BIT15  	                    0x00008000

//GPIO reset
#define REG_CHIP_BASE              	0xBF203C00
#define REG_MIPS_BASE              	0xBF000000//Use 8 bit addressing
#define MHal_GPIO_REG(addr)    		(*(volatile U8*)(REG_MIPS_BASE + (((addr) & ~1)<<1) + (addr & 1)))

static void  gpio42_set_in(void)
{
    MHal_GPIO_REG(0x101ea1) &=~ BIT7;
    MHal_GPIO_REG(0x101e5c) |= BIT6;
}

U8 gpio42_Read(void)
{
    unsigned char u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(0x101e50) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}
#endif

extern void Chip_Flush_Memory(void);
long int initdram(int board_type)
{
#ifdef CONFIG_GPIO_FOR_DDRSIZE
    unsigned char value;

    gpio42_set_in();
    value = gpio42_Read();

	if (value == 0)
		return 128;
	else
		return 256;
#else
	#if 0
	char *s;

	s = getenv("memsize");

	if (s == NULL)
		return 128;
	else
		return simple_strtoul (s, NULL, 10);
	#else
	volatile unsigned int *uncached_dram_start = (unsigned int *)0xA0000000;
	volatile unsigned int *uncached_128M_start = (unsigned int *)0xA8000000;
	unsigned int start_mark_1 = 0xA5A5A5A5;
	unsigned int start_mark_2 = 0xB9B9B9B9;
	unsigned int start_very_1;
	unsigned int start_very_2;

	*uncached_dram_start = start_mark_1;
	*uncached_128M_start = start_mark_2;

	Chip_Flush_Memory();

	start_very_1 = *uncached_dram_start;
	start_very_2 = *uncached_128M_start;

	if (start_very_1 == start_very_2)
		return 128;
	else
		return 256;
	#endif
#endif
}

int checkboard (void)
{
	int uSpeed_L = 0, uSpeed_H = 0;
	int uMemClk_L = 0, uMemClk_H = 0;

	printf("Board\t: MSTAR TITANIA ");

	uSpeed_L  = (*(volatile u32*)(0xBF221864))&0xFF;
	uSpeed_H  = (*(volatile u32*)(0xBF221868))&0xFF;
	uMemClk_L = *(volatile u32*)(0xBF202440);
	uMemClk_H = *(volatile u32*)(0xBF202444);

	if(uSpeed_H==0x1c)
	{
		printf("(CPU speed is 12MHz, ");
	}
	else if(uSpeed_L==0x14)
	{
		printf("(CPU speed is %dMHz, ", (int)(uSpeed_H*24));
	}
	else if(uSpeed_L==0x18)
	{
		printf("(CPU speed is %dMHz setting, ", (int)(uSpeed_H*48));
	}

	if((uMemClk_H==0x0500)&&(uMemClk_L==0x02B4)) //800M
	{
		printf("Memory clock is 800MHz) \n");
	}
	else if((uMemClk_H==0x0300)&&(uMemClk_L==0x0250))  //1.1G
	{
		printf("Memory clock is 1100MHz)\n");
	}
	else if((uMemClk_H==0x0300)&&(uMemClk_L==0x0200))  //1.3G
	{
		printf("Memory clock is 1300MHz)\n");
	}
	else
	{
		printf("Unknow Memory clock)\n");
	}
	//printf("(CPU Speed %d MHz)\n", (int)CPU_CLOCK_RATE/1000000);

	return 0;
}
