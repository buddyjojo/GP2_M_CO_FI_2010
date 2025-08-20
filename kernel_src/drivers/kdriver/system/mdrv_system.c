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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    drv_system_io.c
/// @brief  System Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <asm/io.h>     //robert.yang
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/fadvise.h>
#include <linux/poll.h>
#include <linux/wait.h>

#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "mdrv_system_io.h"
#include "mdrv_system_st.h"
#include "Board.h"
#include "chip_setup.h"
#include "mhal_chiptop_reg.h"
#include "mhal_MIU.h"
//#include "AeonBin.h"



//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
/*
#define DEF_STC_SYNTH               0x14000000                          // 216/2.5  = 86.4, 2.5(dec) = 00010.1(bin) = 0x14000000(u5.27)
#define MAX_STC_SYNTH               0x15000000                          // 216/2.75 = 78.55
#define MIN_STC_SYNTH               0x13000000                          // 216/2.25 = 96
*/

#define SYS_DEBUG                   1
#define REG_ADDR(addr)              (*((volatile U16*)(0xBF000000 + (addr << 1))))

#define PMMCU_REG(addr)             (*((volatile u32*)(REG_PMMCU_BASE + ((addr)<<2))))



#if SYS_DEBUG
#define SYS_PRINT(fmt, args...)     printk("[SYS][%06d]     " fmt, __LINE__, ## args)
#else
#define SYS_PRINT(fmt, args...)
#endif

#define ALL_INT_DISABLE()       local_irq_disable()
#define ALL_INT_ENABLE()        local_irq_enable()
#define printf                  printk



//-------------------------------------------------------------------------------------------------
//  Export
//-------------------------------------------------------------------------------------------------
EXPORT_SYMBOL(MDrv_SYS_GetPanelInfo);
//EXPORT_SYMBOL(MDrv_SYS_GetMMAP);
EXPORT_SYMBOL(MDrv_SYS_EnableSVDCPU); // samuel, 20081107
EXPORT_SYMBOL(MDrv_SYS_Enable3DCOM); // samuel, 20081107
EXPORT_SYMBOL(MDrv_SYS_Flush_Memory); // dhjung LGE
EXPORT_SYMBOL(MDrv_SYS_CPU_Sync);
EXPORT_SYMBOL(MDrv_SYS_GetSyetemTime);
EXPORT_SYMBOL(MDrv_SYS_Read_Memory);
EXPORT_SYMBOL(MDrv_SYS_PA2NonCacheSeg);

//mail box crash protection 2009-11-06
EXPORT_SYMBOL(MDrv_SYS_VDmcuSetType);
EXPORT_SYMBOL(MDrv_SYS_VDmcuGetType);

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
MST_PLATFORM_INFO_t gPlatformInfo;
extern int g_in_hotel_mode ;
extern int g_in_SetUart_mode;
extern unsigned char  uart_base[];
extern unsigned int   uart_len ;
extern unsigned int   uart_wptr;
extern unsigned int   uart_rptr ;
extern unsigned int   uart_c;
extern wait_queue_head_t        uart_wait_queue;
extern struct semaphore UartSem;
extern int g_emp_movie_in_play ; // Samuel, 090109

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#if 0
static SYS_Info                  sysInfo =
{
	.SWLib.Board = BOARD_NAME,
	.SWLib.Platform = CHIP_NAME,
};

#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

/*
   inline void _System_SetStcSynth(MS_U8 u8StcId, MS_U32 u32StcCw)
   {
   REG(REG_STC0_SYNC_CW_LO + (8*u8StcId)) = (MS_U16)(u32StcCw & 0xFFFF);
   REG(REG_STC0_SYNC_CW_HI + (8*u8StcId)) = (MS_U16)(u32StcCw >> 16);
   REG(REG_DCSYNC_MISC) = (REG(REG_DCSYNC_MISC) & ~(STC0_CW_SEL_TSP<<(1*u8StcId))) | (UPDATE_STC0_CW << (1*u8StcId));
   }
   */

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
U32 MDrv_SYS_SetPanelInfo(U32 arg)
{
	IO_SYS_PANEL_INFO_t info;

	SYS_PRINT("%s\n", __FUNCTION__);

	if (copy_from_user(&info, (IO_SYS_PANEL_INFO_t __user*)arg, sizeof(IO_SYS_PANEL_INFO_t)))
	{
		return -EFAULT;
	}

	if (info.u16Len != sizeof(MST_PANEL_INFO_t))
	{
		return  -EFAULT;
	}

	if (copy_from_user((void*)&gPlatformInfo.panel, (void __user*)info.pPanelInfo, sizeof(MST_PANEL_INFO_t)))
	{
		return -EFAULT;
	}

	return 0;
}

void MDrv_SYS_GetPanelRes(U32 arg)
{
	IO_SYS_PANEL_GET_RES_t info;

	info.u16Width  = gPlatformInfo.panel.u16Width;
	info.u16Height  = gPlatformInfo.panel.u16Height;


	if (copy_to_user((void*)arg, (void*)&info, sizeof(IO_SYS_PANEL_GET_RES_t)))
	{
		return;
	}


}


void MDrv_SYS_ReadGeneralRegister(U32 arg)
{
	U16 u16ReadReg;
	BOOL bHiByte;

	if (copy_from_user(&u16ReadReg, (void __user *)arg, sizeof(U16)))
	{
		return;
	}
	bHiByte = ((u16ReadReg & 0x00FF)/2) ? TRUE : FALSE;
	if (bHiByte)
	{
		u16ReadReg= u16ReadReg - 1;
		u16ReadReg = ((REG_ADDR(u16ReadReg) & 0xFF00) >> 8);
	}
	else
	{
		u16ReadReg = (REG_ADDR(u16ReadReg) & 0x00FF);
	}
	SYS_PRINT ("u16ReadReg= %x ", u16ReadReg);

	if (copy_to_user((void *)arg, &u16ReadReg, sizeof(U16)))
	{
		return;
	}

}

void MDrv_SYS_WriteGeneralRegister(U32 arg)
{
	IO_SYS_GENERAL_REG_t param;
	U16 u16RegValue;
	BOOL bHiByte;

	if (copy_from_user(&param, (void __user *)arg, sizeof(IO_SYS_GENERAL_REG_t)))
	{
		return;
	}
	bHiByte = ((param.u16Reg & 0x00FF)%2) ? TRUE : FALSE;
	if (bHiByte)
	{
		param.u16Reg = param.u16Reg - 1;
		u16RegValue = (REG_ADDR(param.u16Reg));
		u16RegValue = ((u16RegValue & 0x00FF) | ((param.u8Value) << 8));
		REG_ADDR(param.u16Reg) = u16RegValue;
	}
	else
	{
		u16RegValue = (REG_ADDR(param.u16Reg));
		u16RegValue = ((u16RegValue & 0xFF00) | (param.u8Value));
		REG_ADDR(param.u16Reg) = u16RegValue;
	}
}


// samuel, 20081107
void MDrv_SYS_EnableSVDCPU( int enable ){
	unsigned int tmp ;

	if( enable ){
		// enable cpu
		tmp = *(volatile unsigned int*)(0xBF200600+(0x78<<2)) ;
		tmp &= ~(0x3) ;
		*(volatile unsigned int*)(0xBF200600+(0x78<<2)) = tmp ;
	}else{
		// disable cpu
		tmp = *(volatile unsigned int*)(0xBF200600+(0x78<<2)) ;
		tmp |= (0x3) ;
		*(volatile unsigned int*)(0xBF200600+(0x78<<2)) = tmp ;
	}
}

// Samuel, 20081110
int g_system_3dcom_state = -1 ; // -1: undefined, 1: enabled, 0: disabled
EXPORT_SYMBOL(g_system_3dcom_state); // Samuel, 20081110

// samuel, 20081107
void MDrv_SYS_Enable3DCOM( int enable ){
	unsigned int tmp ;

	if( enable ){
		// enable com
		tmp = *(volatile unsigned int*)(0xBF000000+(0x103610<<1)) ;
		tmp &= 0xFF00 ;
		tmp |= 0x0017 ;
		g_system_3dcom_state = 1 ; // Samuel, 20081110
		*(volatile unsigned int*)(0xBF000000+(0x103610<<1)) = tmp ;
	}else{
		// disable com
		tmp = *(volatile unsigned int*)(0xBF000000+(0x103610<<1)) ;
		tmp &= 0xFF00 ;
		tmp |= 0x0012 ;
		g_system_3dcom_state = 0 ; // Samuel, 20081110
		*(volatile unsigned int*)(0xBF000000+(0x103610<<1)) = tmp ;
	}
}


void MDrv_SYS_LoadAeon(U32 arg)
{
	//    U32 u32AeonMemAddr = 0;
	//    U32 u32AeonMemLen = 0x300000 - 0x1000;
#if 1
	IO_SYS_AEONBIN_INFO_t AeonInfo;

	U8* pu8Aeon = NULL;
	BOOL bRet = TRUE;
	SYS_PRINT("Load Aeon Code.\n");

	if (copy_from_user(&AeonInfo, (void __user *)arg, sizeof(IO_SYS_AEONBIN_INFO_t)))
	{
		return;
	}

	MDrv_SYS_DisableAeon();

	pu8Aeon = (U8*)0xA0000000 ; // ioremap(u32AeonMemAddr, u32AeonMemLen);
	SYS_PRINT("Aeon star addr: 0x%x.\n", (U32)pu8Aeon);

	//Un-protect 0x00000000 ~ 0x00100000
	REG_ADDR(0x1012C0) &= (~0x08);

	REG_ADDR(0x1033DE) = 0x01;

	//printk( "sizeof AeonBin=%d\n", sizeof(AeonBin) ) ;
	//memcpy((void*)pu8Aeon, (void*)AeonBin, sizeof(AeonBin) ) ;
	if (copy_from_user(pu8Aeon, (void __user *)AeonInfo.pu8AeonStart, AeonInfo.u32Len))
	{
		return;
	}
    Chip_Flush_Memory();
	mb(); // samuel patch 20081203

#if 1
	//Protect 0x00000000 ~ 0x0010000
	REG_ADDR(0x1012D6) = 0x0005;//protect ID , Only AEON can Read/Write
	REG_ADDR(0x1012D8) = 0x0;//Protect start address
	REG_ADDR(0x1012DA) = 0x200;//Protect End address, 4k byte unit
	REG_ADDR(0x1012C0) |= 0x08;
#endif
#if defined(CONFIG_Triton)
	REG_ADDR(0x101E00) |= 0x02;
#endif

	MDrv_SYS_EnableAeon();
	msleep(200);
	memcpy((void*)&AeonInfo.bRet, &bRet, sizeof(BOOL));
#else
	U8* pu8Aeon = NULL;
	BOOL bRet = TRUE;
	SYS_PRINT("Load Aeon Code.\n");
	MDrv_SYS_DisableAeon();

	pu8Aeon = (U8*)0xA0000000 ; // ioremap(u32AeonMemAddr, u32AeonMemLen);
	SYS_PRINT("Aeon star addr: 0x%x.\n", (U32)pu8Aeon);

	//XBYTE[0x1033DE]=0x01; // for aeon judgement
	REG_ADDR(0x1033DE) = 0x01;
	//memcpy((void*)pu8Aeon, (void*)AeonBin, sizeof(AeonBin[]));
	//printk( "sizeof AeonBin=%d\n", sizeof(AeonBin) ) ;
	memcpy((void*)pu8Aeon, (void*)AeonBin, sizeof(AeonBin) ) ;
#if defined(CONFIG_Triton)
	REG_ADDR(0x101E00) |= 0x02;
#endif

	MDrv_SYS_EnableAeon();

	msleep(200);

	memcpy((void*)arg, &bRet, sizeof(BOOL));
#endif

}

void MDrv_SYS_ResetAeon(U32 arg)
{
	SYS_PRINT("Reset Aeon. \n");

#ifdef CONFIG_Titania2
	REG_ADDR(0x0FE6) = 0x1;
	//    msleep(1);
	msleep(50); // samuel patch 20081203
	REG_ADDR(REG_SW_RESET_CPU_AEON) &= ~AEON_SW_RESET;
	REG_ADDR(REG_SW_RESET_CPU_AEON) = AEON_SW_RESET;
#endif

#ifdef CONFIG_Titania3
	REG_ADDR(0x100FE6) = 0x1;
	//    msleep(1);
	msleep(50); // samuel patch 20081203
	PMMCU_REG(REG_SW_RESET_CPU_AEON) &= ~AEON_SW_RESET;
	PMMCU_REG(REG_SW_RESET_CPU_AEON) = AEON_SW_RESET;
#endif
}

//static unsigned int vd_clock ;

void MDrv_SYS_EnableAeon(void)
{
	//    unsigned int tmp ;
	int i;
	MDrv_SYS_DisableAeon();
	SYS_PRINT("Enable Aeon. \n");

	REG_ADDR(0x103384) = 0x0000; //clear 0x103384 ,after enable Aeon, AEON will write 0x1818
	// disable VD
	//vd_clock = *(volatile unsigned int*)(0xBF000000+(0x101E2C<<1)) ;
	//*(volatile unsigned int*)(0xBF000000+(0x101E2C<<1)) |= (1<<12) ;
	//    vd_clock = tmp = *(volatile unsigned int*)(0xBF000000+(0x103610<<1)) ;
	//    tmp &= 0xFF00 ;
	//    tmp |= 0x0012 ;
	//    *(volatile unsigned int*)(0xBF000000+(0x103610<<1)) = tmp ;
	MDrv_SYS_Enable3DCOM(0) ;  // samuel, 20081107

	REG_ADDR(0x100F80) = 0x2; // Let aeon command ¨in order〃

#ifdef CONFIG_Titania2
	REG_ADDR(REG_CKG_AEONTS0) &= ~AEON_CLK_DISABLE;
#endif
#ifdef CONFIG_Titania3
	CKLGEN0_REG(REG_CLKGEN0_AEON) &= ~CLKGEN0_CKG_AEON_DIS;
#endif

	//REG_ADDR(AEON_SPI_ADDR0) = 0xA100;

	//Mask Aeon FIQ/IRQ
	REG_ADDR(0x101948) = 0xFFFF;
	REG_ADDR(0x10194A) = 0xFFFF;
	REG_ADDR(0x101968) = 0xFFFF;
	REG_ADDR(0x10196A) = 0xFFFF;

	/*
	   REG_ADDR(0x103380) = 0;
	   REG_ADDR(0x103382) = 0;
	   REG_ADDR(0x103384) = 0;
	   REG_ADDR(0x103386) = 0;
	   REG_ADDR(0x103388) = 0;
	   REG_ADDR(0x10338A) = 0;
	   REG_ADDR(0x10338C) = 0;
	   REG_ADDR(0x10338E) = 0;
	   REG_ADDR(0x103390) = 0;
	   REG_ADDR(0x103392) = 0;
	   REG_ADDR(0x103394) = 0;
	   REG_ADDR(0x103396) = 0;
	   REG_ADDR(0x103398) = 0;
	   REG_ADDR(0x10339A) = 0;
	   REG_ADDR(0x10339C) = 0;
	   REG_ADDR(0x10339E) = 0;
	   */

	REG_ADDR(AEON_REG_CTRL) |= (AEON_CTRL_EN + AEON_CTRL_RST);
#ifdef CONFIG_Titania2
	REG_ADDR(REG_SW_RESET_CPU_AEON) |= AEON_SW_RESET;
#endif
#ifdef CONFIG_Titania3
	PMMCU_REG(REG_SW_RESET_CPU_AEON) = AEON_SW_RESET;
#endif


	//==============================================
	// bit0 = 1/0 : Aeon stop/start
	// bit1 = 1 for patch1
	// bit2 = 1 for patch2
	REG_ADDR(0x100FE6) = 0x02;

	for(i=0;i<1000;i++)
	{
		if(REG_ADDR(0x103384)==0x1818)//After Aeon enable , AEON will write 0x1818 to 0x103384
		{
			SYS_PRINT("Enable Aeon Done.... \n");

			//XBYTE[0x1246] &= (~0x10);
			//REG_ADDR(0x1246) &= (~0x10);
			g_emp_movie_in_play = 1 ; // Samuel, 090109
			return;
		}
		udelay(1000);//delay 1ms
	}

	SYS_PRINT("\n\n!!!!!!!!!!!! Enable Aeon ERROR.!!!!!!!!!!!!\n\n");
}


static unsigned char* aeon_msg = (unsigned char*)(0xA017E000) ;
static unsigned int aeon_idx = 0 ;
#define AEON_MSG_LEN_MASK   0x7FFFF

void MDrv_SYS_DumpAeonMessage( void ){
	aeon_msg[AEON_MSG_LEN_MASK+1] = 0 ;
	rmb() ;
	while( 0!=aeon_msg[aeon_idx] ){
		printk( aeon_msg+aeon_idx ) ;
		aeon_idx += strlen(aeon_msg+aeon_idx)+1 ;
		aeon_idx &= AEON_MSG_LEN_MASK ;
	}
}

void MDrv_SYS_DisableAeon(void)
{
	U16 u16Reg = 0;
	SYS_PRINT("Disable Aeon. \n");

#ifdef CONFIG_Titania2
	u16Reg = REG_ADDR(REG_SW_RESET_CPU_AEON);
#endif
#ifdef CONFIG_Titania3
	u16Reg = PMMCU_REG(REG_SW_RESET_CPU_AEON);
#endif

	if (u16Reg & AEON_SW_RESET)
	{
		REG_ADDR(0x100FE6) = 0x01; //Stop Aeon
		msleep(50); // samuel patch 20081203
#ifdef CONFIG_Titania2
		REG_ADDR(REG_SW_RESET_CPU_AEON) = u16Reg & (~AEON_SW_RESET);
#endif
#ifdef CONFIG_Titania3
		PMMCU_REG(REG_SW_RESET_CPU_AEON) = u16Reg & (~AEON_SW_RESET);
#endif
		REG_ADDR(AEON_REG_CTRL) &= ~(AEON_CTRL_EN + AEON_CTRL_RST);
	}

	SYS_PRINT("Disable Aeon Done. \n");

	// restore VD
	//if( 0==(vd_clock&(1<<12)) )
	//    *(volatile unsigned int*)(0xBF000000+(0x101E2C<<1)) &= ~(1<<12) ;

	//    vd_clock &= 0xFF00 ;
	//    vd_clock |= 0x0017 ;
	//    *(volatile unsigned int*)(0xBF000000+(0x103610<<1)) = vd_clock ;
	MDrv_SYS_Enable3DCOM(1);	//Ethan 091009
	aeon_idx=0; //reset aeon_idx for debug message
	g_emp_movie_in_play = 0 ; // Samuel, 090109
}

void MDrv_SYS_SwitchUart(U32 arg)
{
	if (*((U8*)arg) == TRUE)
	{
		//SYS_PRINT("UART Switch to Aeon\n");
		TOP_REG(REG_TOP_UART) &= ~0x0F;
		TOP_REG(REG_TOP_UART) |= 0x02;
	}
	else
	{
		TOP_REG(REG_TOP_UART) &= ~0x0F;
	}
}

U32 MDrv_SYS_IsAeonEnable(U32 arg)
{
	U32 u32Temp = 0;
	if (REG_ADDR(0x103384) == 0x1818)
	{
		u32Temp = 1;
	}

	if (copy_to_user((void*)arg, &u32Temp, sizeof(U32)))
	{
		return -EFAULT;
	}
	return 0;
}

PMST_PANEL_INFO_t MDrv_SYS_GetPanelInfo(void)
{
	return &gPlatformInfo.panel;
}

//U32 MDrv_SYS_SetBoardInfo(U32 arg)
//{
//
//
//}

#define SYS_REGOP(addr) *((volatile unsigned int*)(0xBF000000 + (addr)))

#ifdef IO_SYS_REG_OP

typedef struct REG_OPERATION{
	// 0=hw address mode(refer to excel files)
	// 1=8051 address mode(using 8051 address space)
	// 2=MIPS address mode(using MIPS address space)
	unsigned int mode ;
	unsigned int op ; // 0=read, 1=write
	unsigned int addr ; // address
	unsigned int value ; // set or get value
} REG_OPERATION_T ;

U32 MDrv_SYS_RegOP(U32 arg){
	REG_OPERATION_T op ;

	if( copy_from_user( &op, (REG_OPERATION_T*)arg, sizeof(op) ) )
		return -EFAULT;    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)

	if( 0==op.op ){ // read
		op.value = SYS_REGOP(op.addr) ;
		if( copy_to_user( (void *)arg, &op, sizeof(op) ) )
			return -EFAULT;    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
	}else{          // write
		SYS_REGOP(op.addr) = op.value ;
	}

	return 0 ;
}
#endif

#ifdef IO_SYS_GET_RAW_UART


#ifdef CONFIG_Titania2
#define uartreg(addr) *((volatile unsigned int*)(0xBF801300 + ((addr))))
#endif
#ifdef CONFIG_Titania3
#define uartreg(addr) *((volatile unsigned int*)(0xBF201300 + ((addr))))
#endif

//#define RAWUART_MAX_SIZE 204800
unsigned char g_get_raw_uart[RAWUART_MAX_SIZE] ;
#ifdef CONFIG_Titania2
#define UART_REG_ONE_BYTE(addr) *((volatile unsigned char*)(0xBF801300 + ((addr))))
#endif
#ifdef CONFIG_Titania3
#define UART_REG_ONE_BYTE(addr) *((volatile unsigned char*)(0xBF201300 + ((addr))))
#endif

//dhjung LGE
extern int g_in_raw_get_mode;

#ifdef CONFIG_Titania3

U32 MDrv_SYS_SendRawUART(U32 arg)
{
    int c, i, t ;
    unsigned char tmp[RAWUART_MAX_SIZE] ;
    unsigned int uarg ;
    unsigned int len ;
    unsigned int uart_num ;

    if( copy_from_user( &uarg, (unsigned int*)(arg), sizeof(uarg)) ){
        return -EFAULT;
    }
    if( copy_from_user( &len, (unsigned int*)(arg+sizeof(uarg)), sizeof(len)) ){
        return -EFAULT;
    }

    if( copy_from_user( &uart_num, (unsigned int*)(arg+sizeof(uarg)+sizeof(len)), sizeof(uart_num)) ){
        return -EFAULT;
    }


	t = c = 0 ;
	while( 1 ){

		if (copy_from_user( tmp, (unsigned char*)(uarg+(t*RAWUART_MAX_SIZE)), RAWUART_MAX_SIZE))
		{
			return -EFAULT;
		}

		for( i=0; i<RAWUART_MAX_SIZE; i++ ){
			// char output
			if( (i&0xF) == 0 ){
				while( 0==(*((volatile unsigned int*)(0xBF201300+(7<<3))) & (1<<2)) ) udelay(100);
			}
			*((volatile unsigned int*)(0xBF201300)) = tmp[i] ;
			c++ ;
			if( c>=len )
				return(0) ;
		}
		t++ ;
		//msleep(1) ; /* 연속으로 Data 전송 안되는 문제로 인해 msleep(1) 대신 위의 udelay(100) 추가 by 박경원C */
	}
    return( 0 ) ;
}

U32 MDrv_SYS_GetRawUART(U32 arg)
{
    unsigned int uarg ;
    unsigned int len ;
    unsigned int uart_num ;
    unsigned int c, i ;

    if( copy_from_user( &uarg, (unsigned int*)(arg), sizeof(uarg)) ){
        return -EFAULT;
    }
    if( copy_from_user( &len, (unsigned int*)(arg+sizeof(uarg)), sizeof(len)) ){
        return -EFAULT;
    }

    if( copy_from_user( &uart_num, (unsigned int*)(arg+sizeof(uarg)+sizeof(len)), sizeof(uart_num)) ){
        return -EFAULT;
    }

    c = uart_c ;
    if( c>len ) // limit length
        c = len ;
    for( i=0; i<c; i++ ){
        copy_to_user( (void*)uarg, ((unsigned char*)((((unsigned int)uart_base)+(uart_rptr&(uart_len-1))))), 1 ) ;
    }
    down(&UartSem) ;
    uart_rptr += c ;
    uart_c    -= c ;
    up(&UartSem) ;
    copy_to_user( (void*)(arg+sizeof(uarg)+sizeof(len)), &c, sizeof(c) ) ; // return count
    return( 0 ) ;
}

#if 0
U32 MDrv_SYS_GetRawUART(U32 arg)
{
	unsigned int flags ;

	unsigned int i, c, ch, offset, size, force_e, uart_lsr, uart_error, uart_error_idx ;
	unsigned char uart_int, uart_int_old ;

	local_irq_save(flags);
	local_irq_disable() ;

	uart_error = 0;
	uart_error_idx = 0;

	//unsigned int dummy ;

	CKLGEN0_REG(0x8) |= 0x0010;    //Disable MCU
	CKLGEN0_REG(REG_CLKGEN0_AEON) &= 0x0001;   //Disable AEON

	// disable Rx/Tx interrupts.
	uart_int_old = uart_int = uartreg(2<<2) ;
	uart_int &= (~0x03) ;
	uartreg(2<<2) = uart_int ;

	g_in_raw_get_mode = 1 ;

	// to make sure Rx FIFO is empty
	while( uartreg(0xa<<2)&1 ){
		uart_int = uartreg(0<<2) ;
	}

	printk( "start get\n" ) ;

	offset = 0 ;
	force_e = 0 ;

start_raw_get:
	i = 0 ;
	while( i<RAWUART_MAX_SIZE ){
		c = 0 ;
		uart_lsr = uartreg(0xa<<2);  // Check error
		/*
		   uart_lsr = uartreg(0xa<<2);  // Check error
		   if (uart_lsr & 0xE)
		   {
		   uart_error = uart_lsr & 0xE;
		   uart_error_idx = i;
		   }
		   */
		while( 0==(uart_lsr&1) ){
			c++ ;
			if( i>0 && c>=0x3FFFF ){
				printk( "no more data!\n" ) ;
				force_e = 1 ;
				goto force_end ;
			}
			uart_lsr = uartreg(0xa<<2);
		}
		//ndelay(500) ;
		ch = uartreg(0<<2) ;
		//dummy = uartreg(3<<2) ;
		g_get_raw_uart[i++] = (ch&0xFF) ;
	}


force_end:
	size = i+offset ;
	copy_to_user( (void *)arg, &size, sizeof(size) ) ;
	copy_to_user( (void *)arg+sizeof(size)+offset, g_get_raw_uart, i ) ;
	offset += i ;
	if( !force_e && i>=RAWUART_MAX_SIZE )
		goto start_raw_get ;

	uartreg(2<<2) = uart_int_old ;

	g_in_raw_get_mode = 0 ;

	printk( "done!\n" ) ;

	local_irq_restore(flags);
	return 0 ;
}
#endif

#elif CONFIG_Titania2

U32 MDrv_SYS_GetRawUART(U32 arg)
{
	//unsigned int flags ;

	unsigned int i, c, ch, offset, size, force_e ;
	unsigned char uart_int, uart_int_old ;
	//unsigned int dummy ;

	// disable Rx/Tx interrupts.
	uart_int_old = uart_int = uartreg(1<<2) ;
	uart_int &= (~0x03) ;
	uartreg(1<<2) = uart_int ;

	g_in_raw_get_mode = 1 ;

	// to make sure Rx FIFO is empty
	while( uartreg(5<<2)&1 ){
		uart_int = uartreg(0<<2) ;
	}

	printk( "start get\n" ) ;

	offset = 0 ;
	force_e = 0 ;

start_raw_get:
	i = 0 ;
	while( i<RAWUART_MAX_SIZE ){
		c = 0 ;
		while( 0==(uartreg(5<<2)&1) ){
			c++ ;
			if( i>0 && c>=0x3FFFF ){
				printk( "no more data!\n" ) ;
				force_e = 1 ;
				goto force_end ;
			}
		}
		ch = uartreg(0<<2) ;
		//dummy = uartreg(3<<2) ;
		g_get_raw_uart[i++] = (ch&0xFF) ;
	}


force_end:
	size = i+offset ;
	copy_to_user( (void *)arg, &size, sizeof(size) ) ;
	copy_to_user( (void *)arg+sizeof(size)+offset, g_get_raw_uart, i ) ;
	offset += i ;
	if( !force_e && i>=RAWUART_MAX_SIZE )
		goto start_raw_get ;

	uartreg(1<<2) = uart_int_old ;

	g_in_raw_get_mode = 0 ;

	printk( "done!\n" ) ;

	return 0 ;
}

#else

U32 MDrv_SYS_GetRawUART(U32 arg)
{
	//unsigned int flags ;

	unsigned int i, c, ch, offset, size, force_e ;
	unsigned char uart_int ;
	unsigned int s ;
	unsigned int aeon_image ;
	unsigned char* aeon_ptr = (unsigned char*)0xA0000000 ;

	aeon_image = 0 ;

	//    local_irq_save( flags ) ;
	//    local_irq_disable() ;
	uart_int = UART_REG_ONE_BYTE(5) ;
	UART_REG_ONE_BYTE(5) = 0 ;

	for( i=0; i<8; i++ )
		uartreg(0x0c) |= 0x8000 ;  // pop Rx FIFO

	while( 1==(uartreg(0x0c)&0x00000001) ) ;

	printk( "start get\n" ) ;

	offset = 0 ;
	force_e = 0 ;

start_raw_get:
	i = 0 ;
	while( i<RAWUART_MAX_SIZE ){
		c = 0 ;
		while( 0==(uartreg(0x0c)&0x00000001) ){
			c++ ;
			if( i>0 && c>=0x3FFFF ){
				printk( "no more data!\n" ) ;
				force_e = 1 ;
				goto force_end ;
			}
		}
		ch = uartreg(0x04) ;
		g_get_raw_uart[i++] = (ch&0xFF) ;
		uartreg(0x0c) |= 0x8000 ;  // pop Rx FIFO
	}


force_end:
	size = i+offset ;
	copy_to_user( (void *)arg, &size, sizeof(size) ) ;
	copy_to_user( (void *)arg+sizeof(size)+offset, g_get_raw_uart, i ) ;


	// detect AEON image
	if( 0==offset ){
		for( s=0; s<32; s++ ){
			if( g_get_raw_uart[0]!=0 )
				break ;
		}
		if( s>=32 ){
			MDrv_SYS_DisableAeon() ;
			REG_ADDR(0x1033DE) = 0x01;
			aeon_image = 1 ;
			aeon_msg[0] = 0 ;
			aeon_idx = 0 ;
		}
	}

	if( aeon_image )
		memcpy( aeon_ptr+offset, g_get_raw_uart, i ) ;


	offset += i ;
	if( !force_e && i>=RAWUART_MAX_SIZE )
		goto start_raw_get ;

	UART_REG_ONE_BYTE(5) = uart_int ;
	//    local_irq_restore( flags ) ;
	//    local_irq_enable() ;

	if( aeon_image ){
		REG_ADDR(0x101E00) |= 0x02;
		MDrv_SYS_EnableAeon();
		msleep(200);
		printk( "AEON reloaded\n" ) ;
	}

	printk( "done!\n" ) ;
	return 0 ;
}


#endif


#endif

void MDrv_SYS_ChangeUart( U32 arg )
{
	int uart_idx ;
	if( copy_from_user( &uart_idx, (int*)arg, sizeof(uart_idx) ) )
		return;    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)

#if CONFIG_Titania3
	switch(uart_idx)
	{
		case UART1_ENABLE:
		//			printf("UART0 Disable \n");
		*((volatile unsigned int*)(0xbf001c24)) &= 0xF7FF;
		*((volatile unsigned int*)(0xbf203d4c)) &= 0xFFFB;
		*((volatile unsigned int*)(0xbf203c04)) &= 0xFBFF;

		//			printf("UART2 Disable \n");			 
		*((volatile unsigned int*)(0xbf203d4c)) &= 0xFBFF;
		*((volatile unsigned int*)(0xbf203c08)) &= 0xF7FF;
		*((volatile unsigned int*)(0xbf203c08)) |= 0x0400;

		//			printf("UART1 Enable \n");			
		*((volatile unsigned int*)(0xbf203d4c)) |= 0x0040;
		*((volatile unsigned int*)(0xbf203c08)) |= 0x0300;
		break;

		case UART2_ENABLE:
		//			printf("UART0 Disable \n");
		*((volatile unsigned int*)(0xbf001c24)) &= 0xF7FF;
		*((volatile unsigned int*)(0xbf203d4c)) &= 0xFFFB;
		*((volatile unsigned int*)(0xbf203c04)) &= 0xFBFF;

		//			printf("UART1 Disable \n");
		*((volatile unsigned int*)(0xbf203d4c)) &= 0xFFBF;
		*((volatile unsigned int*)(0xbf203c08)) &= 0xFCFF;

		//			printf("UART2 Enable \n");
		*((volatile unsigned int*)(0xbf203d4c)) |= 0x0400;
		*((volatile unsigned int*)(0xbf203c08)) |= 0x0800;
		*((volatile unsigned int*)(0xbf203c08)) &= 0xFBFF;		
		break;

		default:
		break;
	}

#endif

	
#ifdef CONFIG_Titania2
	// TODO: Need check
	if( 0==uart_idx ){ // 0==isp_port
		SYS_REGOP(0x100F55<<2) = 0x0314;
		SYS_REGOP(0x100F02<<2) &= ~0x0800;
		SYS_REGOP(0x100F02<<2) &= ~0x0400;
	}else if( 1==uart_idx ){ // 1==rs232_port
		SYS_REGOP(0x100F55<<2) = 0x0520;
		SYS_REGOP(0x100F02<<2) &= ~0x0800;
		SYS_REGOP(0x100F02<<2) |= 0x0400;
	}else{
		printk( "error: N/A Uart index\n" ) ;
	}
#endif

}

void MDrv_SYS_CPU_Sync(void)
{
#ifdef CONFIG_Titania3
    asm volatile (
        "sync;"
        );
#endif
}

void MDrv_SYS_Flush_Memory(void)
{
#ifdef CONFIG_Titania3
	Chip_Flush_Memory();
#endif
}

void MDrv_SYS_Read_Memory(void)
{
#ifdef CONFIG_Titania3
	Chip_Read_Memory();
#endif
}

unsigned long MDrv_SYS_GetSyetemTime(void)
{
    struct timespec ts;

    ts = current_kernel_time();

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void* MDrv_SYS_PA2NonCacheSeg( void* pAddrPA )
{
	if ((U32)pAddrPA < MIU1_MEM_BASE_ADR)
	{
		return (void*)(0xA0000000+((U32)pAddrPA));
	}
	else
	{
		return (void*)(0xC0000000+((U32)pAddrPA)-MIU1_MEM_BASE_ADR);
	}
}

void MDrv_SYS_ReloadAeon( U32 arg ){
	U32 i ;
	if( copy_from_user( &i, (U32*)arg, sizeof(i) ) )
		return;   // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
	if( RELOAD_AEON_STOP==i ){
		MDrv_SYS_DisableAeon() ;
		REG_ADDR(0x1033DE) = 0x01;
		aeon_msg[0] = 0 ;
		aeon_idx = 0 ;
	}else if( RELOAD_AEON_RESTART==i ){
		REG_ADDR(0x101E00) |= 0x02;
		MDrv_SYS_EnableAeon();
		msleep(200);
		printk( "AEON reloaded\n" ) ;
	}
}


U32 MDrv_SYS_Timer(U32 arg){
	U32 ret_timer ;

	// stop and capture timer value
#ifdef CONFIG_Titania3

	//get time
	ret_timer = SYS_REGOP((0x6080 + (0x5<< 2)));
	ret_timer = (ret_timer <<16);
	ret_timer += SYS_REGOP((0x6080 + (0x4<<2)));
	copy_to_user( (void *)arg, &ret_timer, sizeof(ret_timer) ) ;

	//set max
	SYS_REGOP((0x6080 + (0x2<<2)))=0xFFFF;
	SYS_REGOP((0x6080 + (0x3<<2)))=0xFFFF;

	// start timer
	SYS_REGOP(0x6080)=0x00;
	SYS_REGOP(0x6080)|=0x01;


#endif

#ifdef CONFIG_Titania2
	SYS_REGOP( (0x103C88<<1) ) = 0x0301 ;
	ret_timer = SYS_REGOP( (0x103C86<<1) ) ;
	ret_timer <<= 16 ;
	ret_timer += SYS_REGOP( (0x103C84<<1) ) ;

	copy_to_user( (void *)arg, &ret_timer, sizeof(ret_timer) ) ;

	// set max timer0 value
	SYS_REGOP( (0x103C80<<1) ) = 0xFFFF ;
	SYS_REGOP( (0x103C82<<1) ) = 0xFFFF ;

	// start timer
	SYS_REGOP( (0x103C88<<1) ) = 0x0302 ;
#endif

	return 0 ;
}



#if 0
int MDrv_SYS_GetMMAP(int type, unsigned int *addr, unsigned int *len)
{
	switch(type)
	{
		case E_SYS_MMAP_LINUX_BASE:
#ifdef LINUX_MEM_BASE_ADR
			*addr	= LINUX_MEM_BASE_ADR;
			*len	= LINUX_MEM_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_VD_3DCOMB:
#ifdef VD_3DCOMB_BASE_ADR
			*addr	= VD_3DCOMB_BASE_ADR;
			*len	= VD_3DCOMB_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MAD_BASE:
#ifdef MAD_BASE_BUFFER_ADR
			*addr	= MAD_BASE_BUFFER_ADR;
			*len	= MAD_BASE_BUFFER_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MVD_FB:
#ifdef MVD_FRAMEBUFFER_ADR
			*addr	= MVD_FRAMEBUFFER_ADR;
			*len	= MVD_FRAMEBUFFER_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MVD_BS:
#ifdef MVD_BITSTREAM_ADR
			*addr	= MVD_BITSTREAM_ADR;
			*len	= MVD_BITSTREAM_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_EMAC:
#ifdef EMAC_BUF_ADR
			*addr	= EMAC_BUF_ADR;
			*len	= EMAC_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_VE:
#ifdef VE_FRAMEBUFFER_ADR
			*addr	= VE_FRAMEBUFFER_ADR;
			*len	= VE_FRAMEBUFFER_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_SCALER_MENU_LOAD:
#ifdef SCALER_MENULOAD_BUF_ADR
			*addr	= SCALER_MENULOAD_BUF_ADR;
			*len	= SCALER_MENULOAD_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_SCALER_DNR_BUF:
#ifdef SCALER_DNR_BUF_ADR
			*addr	= SCALER_DNR_BUF_ADR;
			*len	= SCALER_DNR_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_RLD_BUF:
#ifdef RLD_BUF_ADR
			*addr	= RLD_BUF_ADR;
			*len	= RLD_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_TTX_BUF:
#ifdef TTX_BUF_ADR
			*addr	= TTX_BUF_ADR;
			*len	= TTX_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MPOOL:
#ifdef MPOOL_ADR
			*addr	= MPOOL_ADR;
			*len	= MPOOL_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_LINUX_2ND_MEM:
#ifdef LINUX_2ND_MEM_ADR
			*addr	= LINUX_2ND_MEM_ADR;
			*len	= LINUX_2ND_MEM_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_TSP:  // samuel, 20081107
#ifdef TSP_BUF_AVAILABLE
			*addr	= TSP_BUF_ADR;
			*len	= TSP_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_SVD:
#ifdef SVD_CPU_AVAILABLE
			*addr	= SVD_CPU_ADR;
			*len	= SVD_CPU_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MVD_SW:
#ifdef MVD_SW_AVAILABLE
			*addr	= MVD_SW_ADR;
			*len	= MVD_SW_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_SVD_ALL:
#ifdef SVD_CPU_AVAILABLE
			*addr	= SVD_CPU_ADR;
#if 0 // Alan.Chen for JPD ping-pong issue
			*len	= SVD_CPU_LEN + MVD_FRAMEBUFFER_LEN + MVD_BITSTREAM_LEN + JPD_OUTPUT_LEN;
#else
			*len    = SVD_CPU_LEN + MVD_FRAMEBUFFER_LEN + MVD_BITSTREAM_LEN + JPD_OUTPUT_LEN + MVD_SW_LEN;
#endif
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_AUDIO_CLIP_MEM:  // samuel, 20081107
#ifdef AUDIO_CLIP_AVAILABLE
			*addr	= AUDIO_CLIP_ADR;
			*len	= AUDIO_CLIP_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_POSD0_MEM:
#ifdef POSD0_AVAILABLE
			*addr	= POSD0_ADR;
			*len	= POSD0_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_POSD1_MEM:
#ifdef POSD1_AVAILABLE
			*addr	= POSD1_ADR;
			*len	= POSD1_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_BIN_MEM:
#ifdef BIN_MEM_AVAILABLE
			*addr	= BIN_MEM_ADR;
			*len	= BIN_MEM_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_JPD_MEM: // Samuel, 090108
#ifdef JPD_OUTPUT_AVAILABLE
			*addr	= JPD_OUTPUT_ADR;
			*len	= JPD_OUTPUT_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_PVR_DOWNLOAD_MEM: // StevenL, 090311
	        #ifdef PVR_DOWNLOAD_BUFFER_AVAILABLE
				*addr	= PVR_DOWNLOAD_BUFFER_ADR;
                *len	= PVR_DOWNLOAD_BUFFER_LEN;
			#else
				return false;
			#endif
			break;
		case E_SYS_MMAP_PVR_UPLOAD_MEM: // StevenL, 090311
	        #ifdef PVR_UPLOAD_BUFFER_AVAILABLE
				*addr	= PVR_UPLOAD_BUFFER_ADR;
                *len	= PVR_UPLOAD_BUFFER_LEN;
			#else
				return false;
			#endif
			break;
		case E_SYS_MMAP_PVR_THUMBNAIL_DECODE_MEM: // StevenL, 090311
	        #ifdef PVR_THUMBNAIL_DECODE_BUFFER_AVAILABLE
				*addr	= PVR_THUMBNAIL_DECODE_BUFFER_ADR;
                *len	= PVR_THUMBNAIL_DECODE_BUFFER_LEN;
			#else
				return false;
			#endif
			break;
		case E_SYS_MMAP_BT_POOL:
#ifdef BT_POOL_AVAILABLE
			*addr	= BT_POOL_ADR;
			*len	= BT_POOL_LEN;
#else
			return false;
#endif
			break;

#if defined(CONFIG_MSTAR_TITANIA_BD_T3_FPGA)
		case E_SYS_MMAP_FPGA_POOL_BASE:
#ifdef FPGA_POOL_BASE_BUFFER_ADR
			*addr	= FPGA_POOL_BASE_BUFFER_ADR;
			*len	= FPGA_POOL_BASE_BUFFER_LEN;
#else
			return false;
#endif
			break;
#endif

#if defined(CONFIG_MSTAR_TITANIA_BD_GP2_DEMO1)
		case E_SYS_MMAP_FPGA_POOL_BASE:
#ifdef FPGA_POOL_BASE_BUFFER_ADR
			*addr	= FPGA_POOL_BASE_BUFFER_ADR;
			*len	= FPGA_POOL_BASE_BUFFER_LEN;
#else
			return false;
#endif
			break;
#endif

#if defined(CONFIG_Titania3)
#if (!(MIU_DRAM_LEN < 0x10000000))
		case E_SYS_MMAP_M4VE:
			*addr   = M4VE_BUF_ADR;
			*len    = M4VE_BUF_LEN;
			break;

		case E_SYS_MMAP_JPG_OSD:
			*addr   = JPG_OSD_ADR;
			*len    = JPG_OSD_LEN;
			break;
#endif
#endif

		case E_SYS_MMAP_MIU1_BASE:
			*addr   = MIU1_MEM_BASE_ADR;
			*len    = MIU1_MEM_BASE_LEN;
			break;

		default:
			return false;
	}
	return true;
}

#endif


void MDrv_SYS_MMAP_Dump( void ){



	printk( "\n[System Memory Map]------------------\n" ) ;
	printk( "MIU_DRAM_LEN=0x%08X, %d, %dKB\n\n", MIU_DRAM_LEN, MIU_DRAM_LEN, MIU_DRAM_LEN/1024 ) ;

	printk( "LINUX_MEM_BASE_ADR=0x%08X, %d, %dKB\n", (int) LINUX_MEM_BASE_ADR,(int)LINUX_MEM_BASE_ADR, (int)LINUX_MEM_BASE_ADR/1024 ) ;
	printk( "LINUX_MEM_GAP_CHK=0x%08X, %d, %dKB\n", (int)LINUX_MEM_GAP_CHK, (int)LINUX_MEM_GAP_CHK, (int)LINUX_MEM_GAP_CHK/1024 ) ;
	printk( "LINUX_MEM_LEN=0x%08X, %d, %dKB\n", LINUX_MEM_LEN, LINUX_MEM_LEN, LINUX_MEM_LEN/1024 ) ;
	printk( "LINUX_MEM_END=0x%08X\n\n", (int)(LINUX_MEM_BASE_ADR+LINUX_MEM_LEN) );

	printk( "BIN_MEM_ADR=0x%08X, %d, %dKB\n", (int)BIN_MEM_ADR, (int)BIN_MEM_ADR,(int) BIN_MEM_ADR/1024 ) ;
	printk( "BIN_MEM_GAP_CHK=0x%08X, %d, %dKB\n", (int)BIN_MEM_GAP_CHK, (int)BIN_MEM_GAP_CHK, (int)BIN_MEM_GAP_CHK/1024 ) ;
	printk( "BIN_MEM_LEN=0x%08X, %d, %dKB\n", BIN_MEM_LEN, BIN_MEM_LEN, BIN_MEM_LEN/1024 ) ;
	printk( "BIN_MEM_END=0x%08X\n\n", (int)(BIN_MEM_ADR+BIN_MEM_LEN) );

	printk( "VE_FRAMEBUFFER_ADR=0x%08X, %d, %dKB\n", (int)VE_FRAMEBUFFER_ADR, (int)VE_FRAMEBUFFER_ADR, (int)VE_FRAMEBUFFER_ADR/1024 ) ;
	printk( "VE_FRAMEBUFFER_ADR_GAP_CHK=0x%08X, %d, %dKB\n", (int)VE_FRAMEBUFFER_ADR_GAP_CHK, (int)VE_FRAMEBUFFER_ADR_GAP_CHK, (int)VE_FRAMEBUFFER_ADR_GAP_CHK/1024 ) ;
	printk( "VE_FRAMEBUFFER_LEN=0x%08X, %d, %dKB\n", (int)VE_FRAMEBUFFER_LEN, (int)VE_FRAMEBUFFER_LEN, (int)VE_FRAMEBUFFER_LEN/1024 ) ;
	printk( "VE_FRAMEBUFFER_END=0x%08X\n\n", (int)(VE_FRAMEBUFFER_ADR+VE_FRAMEBUFFER_LEN) );

	printk( "SCALER_DNR_BUF_ADR=0x%08X, %d, %dKB\n", (int)SCALER_DNR_BUF_ADR,(int) SCALER_DNR_BUF_ADR, (int)SCALER_DNR_BUF_ADR/1024 ) ;
	printk( "SCALER_DNR_GAP_CHK=0x%08X, %d, %dKB\n", (int)SCALER_DNR_GAP_CHK, (int)SCALER_DNR_GAP_CHK,(int) SCALER_DNR_GAP_CHK/1024 ) ;
	printk( "SCALER_DNR_BUF_LEN_EXT=0x%08X, %d, %dKB\n",(int) SCALER_DNR_BUF_LEN_EXT, (int)SCALER_DNR_BUF_LEN_EXT, (int)SCALER_DNR_BUF_LEN_EXT/1024 ) ;
	printk( "SCALER_DNR_BUF_LEN=0x%08X, %d, %dKB\n", SCALER_DNR_BUF_LEN, SCALER_DNR_BUF_LEN, SCALER_DNR_BUF_LEN/1024 ) ;
	printk( "SCALER_DNR_BUF_END=0x%08X\n\n", (int)(SCALER_DNR_BUF_ADR+SCALER_DNR_BUF_LEN));

	printk( "SCALER_DNR_W_BARRIER_ADR=0x%08X, %d, %dKB\n", (int)SCALER_DNR_W_BARRIER_ADR, (int)SCALER_DNR_W_BARRIER_ADR, (int)SCALER_DNR_W_BARRIER_ADR/1024 ) ;
	printk( "SCALER_DNR_W_GAP_CHK=0x%08X, %d, %dKB\n", (int)SCALER_DNR_W_GAP_CHK, (int)SCALER_DNR_W_GAP_CHK, (int)SCALER_DNR_W_GAP_CHK/1024 ) ;
	printk( "SCALER_DNR_W_BARRIER_LEN=0x%08X, %d, %dKB\n", SCALER_DNR_W_BARRIER_LEN, SCALER_DNR_W_BARRIER_LEN, SCALER_DNR_W_BARRIER_LEN/1024 ) ;
	printk( "SCALER_DNR_W_BARRIER_END=0x%08X\n\n", (int)(SCALER_DNR_W_BARRIER_ADR+SCALER_DNR_W_BARRIER_LEN) );

	printk( "MAD_BASE_BUFFER_ADR=0x%08X, %d, %dKB\n",(int) MAD_BASE_BUFFER_ADR,(int) MAD_BASE_BUFFER_ADR, (int)MAD_BASE_BUFFER_ADR/1024 ) ;
	printk( "MAD_BASE_BUFFER_ADR_GAP_CHK=0x%08X, %d, %dKB\n", (int)MAD_BASE_BUFFER_ADR_GAP_CHK, (int)MAD_BASE_BUFFER_ADR_GAP_CHK, (int)MAD_BASE_BUFFER_ADR_GAP_CHK/1024 ) ;
	printk( "MAD_BASE_BUFFER_LEN=0x%08X, %d, %dKB\n", MAD_BASE_BUFFER_LEN, MAD_BASE_BUFFER_LEN, MAD_BASE_BUFFER_LEN/1024 ) ;
	printk( "MAD_BASE_BUFFER_END=0x%08X\n\n", (int)(MAD_BASE_BUFFER_ADR+MAD_BASE_BUFFER_LEN) );

	printk( "VD_3DCOMB_BASE_ADR=0x%08X, %d, %dKB\n", (int)VD_3DCOMB_BASE_ADR, (int)VD_3DCOMB_BASE_ADR, (int)VD_3DCOMB_BASE_ADR/1024 ) ;
	printk( "VD_3DCOMB_GAP_CHK=0x%08X, %d, %dKB\n", (int)VD_3DCOMB_GAP_CHK, (int)VD_3DCOMB_GAP_CHK, (int)VD_3DCOMB_GAP_CHK/1024 ) ;
	printk( "VD_3DCOMB_LEN=0x%08X, %d, %dKB\n", VD_3DCOMB_LEN, VD_3DCOMB_LEN, VD_3DCOMB_LEN/1024 ) ;
	printk( "VD_3DCOMB_END=0x%08X\n\n", (int)(VD_3DCOMB_BASE_ADR+VD_3DCOMB_LEN) );

#if 0
	printk( "RLD_BUF_ADR=0x%08X, %d, %dKB\n", (int)RLD_BUF_ADR, (int)RLD_BUF_ADR, (int)RLD_BUF_ADR/1024 ) ;
	printk( "RLD_BUF_GAP_CHK=0x%08X, %d, %dKB\n", (int)RLD_BUF_GAP_CHK, (int)RLD_BUF_GAP_CHK, (int)RLD_BUF_GAP_CHK/1024 ) ;
	printk( "RLD_BUF_LEN=0x%08X, %d, %dKB\n", RLD_BUF_LEN, RLD_BUF_LEN, RLD_BUF_LEN/1024 ) ;
	printk( "RLD_BUF_END=0x%08X\n\n", (int)(RLD_BUF_ADR+RLD_BUF_LEN) );
#endif

	printk( "TTX_BUF_ADR=0x%08X, %d, %dKB\n", (int)TTX_BUF_ADR, (int)TTX_BUF_ADR, (int)TTX_BUF_ADR/1024 ) ;
	printk( "TTX_BUF_GAP_CHK=0x%08X, %d, %dKB\n", (int)TTX_BUF_GAP_CHK,(int) TTX_BUF_GAP_CHK, (int)TTX_BUF_GAP_CHK/1024 ) ;
	printk( "TTX_BUF_LEN=0x%08X, %d, %dKB\n", TTX_BUF_LEN, TTX_BUF_LEN, TTX_BUF_LEN/1024 ) ;
	printk( "TTX_BUF_END=0x%08X\n\n", (int)(TTX_BUF_ADR+TTX_BUF_LEN) );

#if 0
	printk( "MVD_SW_ADR=0x%08X, %d, %dKB\n", (int)MVD_SW_ADR, (int)MVD_SW_ADR,(int) MVD_SW_ADR/1024 ) ;
	printk( "MVD_SW_ADR_GAP_CHK=0x%08X, %d, %dKB\n", (int)MVD_SW_ADR_GAP_CHK, (int)MVD_SW_ADR_GAP_CHK, (int)MVD_SW_ADR_GAP_CHK/1024 ) ;
	printk( "MVD_SW_LEN=0x%08X, %d, %dKB\n", (int)MVD_SW_LEN, (int)MVD_SW_LEN, (int)MVD_SW_LEN/1024 ) ;
	printk( "MPOOL_END=0x%08X\n\n", (int)(MVD_SW_ADR+MVD_SW_LEN) );
#endif

	printk( "MPOOL_ADR=0x%08X, %d, %dKB\n", (int)MPOOL_ADR, (int)MPOOL_ADR,(int) MPOOL_ADR/1024 ) ;
	printk( "MPOOL_GAP_CHK=0x%08X, %d, %dKB\n", (int)MPOOL_GAP_CHK, (int)MPOOL_GAP_CHK, (int)MPOOL_GAP_CHK/1024 ) ;
	printk( "MPOOL_LEN=0x%08X, %d, %dKB\n", (int)MPOOL_LEN, (int)MPOOL_LEN, (int)MPOOL_LEN/1024 ) ;
	printk( "MPOOL_END=0x%08X\n\n", (int)(MPOOL_ADR+MPOOL_LEN) );

	printk( "MVD_FRAMEBUFFER_ADR=0x%08X, %d, %dKB\n", (int)MVD_FRAMEBUFFER_ADR,(int) MVD_FRAMEBUFFER_ADR, (int)MVD_FRAMEBUFFER_ADR/1024 ) ;
	printk( "MVD_FRAMEBUFFER_ADR_GAP_CHK=0x%08X, %d, %dKB\n", (int)MVD_FRAMEBUFFER_ADR_GAP_CHK, (int)MVD_FRAMEBUFFER_ADR_GAP_CHK,(int) MVD_FRAMEBUFFER_ADR_GAP_CHK/1024 ) ;
	printk( "MVD_FRAMEBUFFER_LEN=0x%08X, %d, %dKB\n", MVD_FRAMEBUFFER_LEN, MVD_FRAMEBUFFER_LEN, MVD_FRAMEBUFFER_LEN/1024 ) ;
	printk( "MVD_FRAMEBUFFER_END=0x%08X\n\n", (int)(MVD_FRAMEBUFFER_ADR+MVD_FRAMEBUFFER_LEN) );

	printk( "MVD_BITSTREAM_ADR=0x%08X, %d, %dKB\n", (int)MVD_BITSTREAM_ADR, (int)MVD_BITSTREAM_ADR, (int)MVD_BITSTREAM_ADR/1024 ) ;
	printk( "MVD_BITSTREAM_ADR_GAP_CHK=0x%08X, %d, %dKB\n", (int)MVD_BITSTREAM_ADR_GAP_CHK, (int)MVD_BITSTREAM_ADR_GAP_CHK, (int)MVD_BITSTREAM_ADR_GAP_CHK/1024 ) ;
	printk( "MVD_BITSTREAM_LEN=0x%08X, %d, %dKB\n", MVD_BITSTREAM_LEN, MVD_BITSTREAM_LEN, MVD_BITSTREAM_LEN/1024 ) ;
	printk( "MVD_BITSTREAM_END=0x%08X\n\n", (int)(MVD_BITSTREAM_ADR+MVD_BITSTREAM_LEN) );

#if defined(CONFIG_Titania3)
#if (!(MIU_DRAM_LEN < 0x10000000))
	printk( "M4VE_BUF_ADR=0x%08X, %d, %dKB\n", (int)M4VE_BUF_ADR, (int)M4VE_BUF_ADR, (int)M4VE_BUF_ADR/1024 ) ;
	printk( "M4VE_BUF_ADR_GAP_CHK=0x%08X, %d, %dKB\n", (int)M4VE_BUF_ADR_GAP_CHK, (int)M4VE_BUF_ADR_GAP_CHK, (int)M4VE_BUF_ADR_GAP_CHK/1024 ) ;
	printk( "M4VE_BUF_LEN=0x%08X, %d, %dKB\n", M4VE_BUF_LEN, M4VE_BUF_LEN, M4VE_BUF_LEN/1024 ) ;
	printk( "MVD_BITSTREAM_END=0x%08X\n\n", (int)(M4VE_BUF_ADR+M4VE_BUF_LEN) );

	printk( "JPG_OSD_ADR=0x%08X, %d, %dKB\n", (int)JPG_OSD_ADR, (int)JPG_OSD_ADR, (int)JPG_OSD_ADR/1024 ) ;
	printk( "JPG_OSD_ADR_GAP_CHK=0x%08X, %d, %dKB\n", (int)JPG_OSD_ADR_GAP_CHK, (int)JPG_OSD_ADR_GAP_CHK, (int)JPG_OSD_ADR_GAP_CHK/1024 ) ;
	printk( "JPG_OSD_LEN=0x%08X, %d, %dKB\n", JPG_OSD_LEN, JPG_OSD_LEN, JPG_OSD_LEN/1024 ) ;
	printk( "MVD_BITSTREAM_END=0x%08X\n\n", (int)(JPG_OSD_ADR+JPG_OSD_LEN) );
#endif
#endif

}




unsigned int MDrv_SYS_GetKernelMemLen( void ){
	unsigned int a, b ;
	if( MDrv_SYS_GetMMAP(E_SYS_MMAP_LINUX_BASE, &a, &b) )
		return( b ) ;
	return( 0 ) ;
}
#ifdef TBD
/*
   20080818-T2-Release의 kernel 부분이 적용되기 전까지 처리안하도록 함. 임시로
   */

#endif

U32 MDrv_SYS_SetUartMode(U32 arg){

	if (copy_from_user(&g_in_SetUart_mode, (int __user*)arg, sizeof(int)))
	{
		return -EFAULT;
	}

	return( 0 ) ;
}

U32 MDrv_SYS_HotelMode(U32 arg){

	if (copy_from_user(&g_in_hotel_mode, (int __user*)arg, sizeof(int)))
	{
		return -EFAULT;
	}

	return( 0 ) ;
}

#define HOTEL_MODE_PRINTF_BUF   16
#define HOTEL_MODE_PRINTF_MAX   128

U32 MDrv_SYS_HotelModePrintf(U32 arg){
	int c, i ;
	unsigned char tmp[HOTEL_MODE_PRINTF_BUF] ;

	c = 0 ;
	while( 1 ){

		if (copy_from_user( tmp, (unsigned char*)(arg+(c*HOTEL_MODE_PRINTF_BUF)), HOTEL_MODE_PRINTF_BUF))
		{
			return -EFAULT;
		}

		for( i=0; i<HOTEL_MODE_PRINTF_BUF; i++ ){
			if( 0==tmp[i] ) break ;
			// char output
			*((volatile unsigned int*)(0xBF201300)) = tmp[i] ;
			//printk( "%c",tmp[i] );
		}
		if( i<HOTEL_MODE_PRINTF_BUF ) // encounter 0
			break ;

		c++ ;
		if( c>=HOTEL_MODE_PRINTF_MAX )
			break ;
		msleep(1) ;
	}

	return( 0 ) ;
}

extern int MDrv_fadvise( int fd ) ;
U32 MDrv_SYS_SetSeqFile(U32 arg)
{
	U32 fd;
	if (copy_from_user( &fd, (U32 *)arg, sizeof(U32)))
	{
		return -EFAULT;
	}

	if(MDrv_fadvise(fd)==0)
	{
		//printk("sys_fadvise64_64 OK\n");
		return 0;
	}
	else
	{
		//printk("sys_fadvise64_64 Fail\n");
		return -EFAULT;;
	}
}

//20090724 Terry, URSA ISP Load Code
#define SBIN_SIZE 65536
#define SBIN_START  0xBFC80000  //BFC00000+80000(512k) = BFC80000

U32 MDrv_SYS_GetSPI(U32 arg)
{
	unsigned char *SPI=(unsigned char *)SBIN_START;
	//  printk("MDRV MFC_BIN %X\n",arg);
	copy_to_user((void *)arg,(void *)SPI,SBIN_SIZE);

	return 0;
}


//20091028 Terry, MIU protect
//MS_BOOL MDrv_MIU_Protect(U8 u8Blockx,U8 *pu8ProtectId,U32 u32Start,U32 u32End,BOOL bSetFlag)
BOOL MDrv_SYS_MIU_Protect(U32 arg)
{        
	
	IO_SYS_MIU_PROTECT_t miup;	
 	copy_from_user( &miup,(IO_SYS_MIU_PROTECT_t __user *)arg, sizeof(IO_SYS_MIU_PROTECT_t));
	BOOL result=HAL_MIU_Protect(miup.u8Blockx,miup.pu8ProtectId,miup.u32Start,miup.u32End,miup.bSetFlag);
	return copy_to_user((BOOL __user *)arg,&result,sizeof(BOOL));
}



U32 MDrv_SYS_GetRev(U32 arg)
{
	U32 u32Value = Chip_Query_Rev();
	return copy_to_user((U32 __user *) arg, &u32Value, sizeof(U32));
}


//mail box crash protection 2009-11-06

static atomic_t _vdmcu_dsp_type = ATOMIC_INIT(VDMCU_DSP_UNKNOWN);

void MDrv_SYS_VDmcuSetType(VDMCU_DSP_TYPE_t type)
{
    atomic_set(&_vdmcu_dsp_type, type);
}

VDMCU_DSP_TYPE_t MDrv_SYS_VDmcuGetType(void)
{
    return atomic_read(&_vdmcu_dsp_type);
}


/*
//-------------------------------------------------------------------------------------------------
/// Set STC Synthesizer
/// @param u8StcId                  \b IN: Select STC0, STC1
/// @param u32StcCw                 \b IN: STC synthesizer control word [ u5.27 = (216/freq)*2^27 ]
/// @return TRUE - Success
///         FALSE - Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_System_SetStcSynth(MS_U8 u8StcId, MS_U32 u32StcCw)
{
MS_U32 u32OldIntr;

MsOS_DisableAllInterrupts(u32OldIntr);

_System_SetStcSynth(u8StcId, u32StcCw);

MsOS_RestoreAllInterrupts(u32OldIntr);

return TRUE;
}

//-------------------------------------------------------------------------------------------------
//[OBSOLETE]
/// Add control word to STC Synthesizer
/// @param u8StcId                  \b IN: Select STC0, STC1
/// @param s32Value                 \b IN: Value to add to STC synthesizer control word [ u5.27 = (216/freq)*2^27 ]\n
///                                        POSITIVE: slow down the STC
///                                        NEGATIVE: speed up the STC
/// @return TRUE - Success
///         FALSE - Failure
/// @note OBSOLETE
//[OBSOLETE]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_System_AddStcSynth(MS_U8 u8StcId, MS_S32 s32Value)
{
MS_BOOL    bRet = TRUE;
MS_U32     u32StcCw;
MS_U32     u32OldIntr;

MsOS_DisableAllInterrupts(u32OldIntr);

u32StcCw = (REG(REG_STC0_SYNC_CW_HI + (8*u8StcId)) << 16) | REG(REG_STC0_SYNC_CW_LO + (8*u8StcId));
u32StcCw = u32StcCw + s32Value;
if ((u32StcCw > MAX_STC_SYNTH) || (u32StcCw < MIN_STC_SYNTH))
{
bRet = FALSE;
}
else
{
_System_SetStcSynth(u8StcId, u32StcCw);
}

MsOS_RestoreAllInterrupts(u32OldIntr);

return bRet;
}


//-------------------------------------------------------------------------------------------------
/// Update control word to STC Synthesizer
/// @param u8StcId                  \b IN: Select STC0, STC1
/// @param StcSynthSrcType          \b IN: Select STC synthesizer control word
/// @return TRUE - Success
///         FALSE - Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_System_UpdStcSynth(MS_U8 u8StcId, StcSynthSrcType eSynthSrc)
{
MS_BOOL    bRet = TRUE;
MS_U32     u32StcSet;
MS_U32     u32OldIntr;

MsOS_DisableAllInterrupts(u32OldIntr);

if (eSynthSrc == E_STC_SYNTH_MCU)
{
	u32StcSet = (STC0_CW_SEL_MIPS | UPDATE_STC0_CW) << (1*u8StcId);
}
else
{
	u32StcSet = (STC0_CW_SEL_TSP | UPDATE_STC0_CW) << (1*u8StcId);
}

REG(REG_DCSYNC_MISC) = (REG(REG_DCSYNC_MISC) & ~((STC0_CW_SEL_TSP|UPDATE_STC0_CW)<<(1*u8StcId))) | u32StcSet;


MsOS_RestoreAllInterrupts(u32OldIntr);

return bRet;
}
*/

#if 0
//-------------------------------------------------------------------------------------------------
/// System initialzation
/// @return TRUE(Success), FALSE(Failure)
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_System_Init(void)
{
	ALL_INT_DISABLE();

	//[BUG]
	//[PATCH]
	//[NOTE] Change CPU to 123MHz be unstable after resetCPU to init CPU to 216MHz at BOOT
	TOP_REG(REG_TOP_MCU_USB_STC0) = (TOP_REG(REG_TOP_MCU_USB_STC0) & ~(TOP_MCU_CLK_MASK)) | TOP_MCU_CLK_DFT;
	TOP_REG(REG_TOP_MCU) = (TOP_REG(REG_TOP_MCU) & ~(TOP_CKG_MCU_MASK)) | TOP_CKG_MCU_SRC_123;
	TOP_REG(REG_TOP_MCU_USB_STC0) = (TOP_REG(REG_TOP_MCU_USB_STC0) & ~(TOP_MCU_CLK_MASK)) | TOP_MCU_CLK_SRC_MCUCLK;

	// Turn on output pad
	//[NOTE] Turn on output pad at boot may caused resetCPU hang
	TOP_REG(REG_TOP_PCI_PD_TEST) = (TOP_REG(REG_TOP_PCI_PD_TEST) & ~(TOP_TURN_OFF_PAD));

	ALL_INT_ENABLE();
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Get system information
/// @return @ref SYS_Info
//-------------------------------------------------------------------------------------------------
const SYS_Info* MDrv_System_GetInfo(void)
{
	return (const SYS_Info*)&sysInfo;
}

//-------------------------------------------------------------------------------------------------
/// Switch system output pad
/// @param  ePadType                \b IN:  type of switch pad
/// @return TRUE(Success), FALSE(Failure)
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_System_SwitchPad(SYS_PadType ePadType)
{
	MS_BOOL                bRet = TRUE;

	ALL_INT_DISABLE();

	switch (ePadType)
	{
		case E_SYS_PAD_MSD5010_SM2_IIC2:

			// SM2 ( CI_PAD )
			TOP_REG(REG_TOP_CR_CI_GPIO) &= ~(TOP_PAD_CI_ADLO_MASK);
			TOP_REG(REG_TOP_CR_CI_GPIO) |= (TOP_PAD_CI_ADLO_SM2);
			TOP_REG(REG_TOP_CR_CI_GPIO) &= ~(TOP_PAD_CI_ADHI_MASK);
			TOP_REG(REG_TOP_CR_CI_GPIO) |= (TOP_PAD_CI_ADHI_CCIR);              // Smartcard IC Vcc

			// IICM2 ( AU1_PAD )
			TOP_REG(REG_TOP_JTAG_AUD_SPI_GPIO_PWM) &= ~(TOP_PAD_AU1_MASK);
			TOP_REG(REG_TOP_JTAG_AUD_SPI_GPIO_PWM) |= (TOP_PAD_AU1_IICM1);

			break;

		case E_SYS_PAD_MSD5011_SM2_IIC2:

			// SM2 ( CI_PAD )
			TOP_REG(REG_TOP_CR_CI_GPIO) &= ~(TOP_PAD_CI_ADLO_MASK);
			TOP_REG(REG_TOP_CR_CI_GPIO) |= (TOP_PAD_CI_ADLO_SM2);
			TOP_REG(REG_TOP_CR_CI_GPIO) &= ~(TOP_PAD_CI_ADHI_MASK);
			TOP_REG(REG_TOP_CR_CI_GPIO) |= (TOP_PAD_CI_ADHI_CCIR);              // Smartcard IC Vcc

			// IICM2 ( AU1_PAD )
			TOP_REG(REG_TOP_JTAG_AUD_SPI_GPIO_PWM) &= ~(TOP_PAD_AU1_MASK);
			TOP_REG(REG_TOP_JTAG_AUD_SPI_GPIO_PWM) |= (TOP_PAD_AU1_IICM1);

			break;

		case E_SYS_PAD_MSD5015_GPIO:

			// GPIO_CI0_7 ( CI_PAD )
			TOP_REG(REG_TOP_CR_CI_GPIO) &= ~(TOP_PAD_CI_ADHI_MASK|TOP_PAD_CI_ADLO_MASK|TOP_PAD_CI_DATA_MASK);
			TOP_REG(REG_TOP_CR_CI_GPIO) |= (TOP_PAD_CI_ADHI_CCIR|TOP_PAD_CI_ADLO_GPIO|TOP_PAD_CI_DATA_TSI);

			// GPIO_CI8_9 ( CI_PAD )
			TOP_REG(REG_TOP_DVBC_HDMI_CI) &= ~(TOP_PAD_CI_CTL_MASK);
			TOP_REG(REG_TOP_DVBC_HDMI_CI) |= (TOP_PAD_CI_CTL_GPIO);

			// GPIO_PWM0_1 ( PWM_PAD )
			TOP_REG(REG_TOP_JTAG_AUD_SPI_GPIO_PWM) &= ~(TOP_PAD_PWM_MASK);
			TOP_REG(REG_TOP_JTAG_AUD_SPI_GPIO_PWM) |= (TOP_PAD_PWM_GPIO);

			break;

		case E_SYS_PAD_MSD5018_SM2:

			// SM2
			TOP_REG(REG_TOP_CR_CI_GPIO) = TOP_REG(REG_TOP_CR_CI_GPIO) & ~(TOP_PAD_GPIO_S_MASK);
			TOP_REG(REG_TOP_CR_CI_GPIO) = TOP_REG(REG_TOP_CR_CI_GPIO) | (TOP_PAD_GPIO_S_SM2);

			break;

		default:
			SYS_DBG(printf("%s Non supported PAD setting\n", __FUNCTION__));
			bRet = FALSE;
			break;
	}

	ALL_INT_ENABLE();
	return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Enable/Disable watchdog timer
/// @param  bEnable                 \b IN:  Enable/disable
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_System_WDTEnable(MS_BOOL bEnable)
{
	if (bEnable)
	{
		WDT_REG(REG_WDT_DISABLEWORD_L) = 0;
		WDT_REG(REG_WDT_DISABLEWORD_H) = 0;
	}
	else
	{
		WDT_REG(REG_WDT_DISABLEWORD_L) = 0xaa55;
		WDT_REG(REG_WDT_DISABLEWORD_H) = 0x55aa;
	}
}

//-------------------------------------------------------------------------------------------------
/// Clear watchdog timer
/// @return None
/// @note The monitor task should call this API regularly
//-------------------------------------------------------------------------------------------------
void MDrv_System_WDTClear(void)
{
	WDT_REG(REG_WDT_CLEAR_STATUS) = 0x0001;
}

//-------------------------------------------------------------------------------------------------
/// Query whether Watchdog timer was triggered last time and clear the last status
/// @return	TRUE(Yes), FALSE(No)
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_System_WDTLastStatus(void)
{
	MS_BOOL bRet;

	if (WDT_REG(REG_WDT_CLEAR_STATUS) & 0x0004)
	{
		bRet = TRUE;
	}
	else
	{
		bRet = FALSE;
	}

	WDT_REG(REG_WDT_CLEAR_STATUS) = 0x0002;

	return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Set tigger time of watchdog timer
/// @param  u32Ms                   \b IN:  msec (350~357564 ms at 12 MHz; 146~148958 ms at 28.8 MHz)
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_System_WDTSetTime(MS_U32 u32Ms)
{
	WDT_REG(REG_WDT_SETTIME) = (u32Ms * (XTAL_CLOCK_FREQ / 1000)) >> 22; //1 ~ 0x3FF
}

//-------------------------------------------------------------------------------------------------
/// Reset whole chip
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_System_ResetChip(void)
{
	WDT_REG(REG_WDT_PROGRAMKEY_L)   = 0x5678;
	WDT_REG(REG_WDT_PROGRAMKEY_H)   = 0x1234;
	WDT_REG(REG_WDT_ENABLEMCURESET) = 0;
	WDT_REG(REG_WDT_DISABLEWORD_L)  = 0;
	WDT_REG(REG_WDT_DISABLEWORD_H)  = 0;
	WDT_REG(REG_WDT_SETTIME)        = 0;
	while(1);
}

//-------------------------------------------------------------------------------------------------
/// Reset CPU only
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_System_ResetCPU(void)
{
	ALL_INT_DISABLE();

	TOP_REG(REG_TOP_RESET_CPU0) = 0x029F;
	TOP_REG(REG_TOP_RESET_CPU0) = 0x029F | TOP_RESET_CPU0;

	while(1);
}
#endif

