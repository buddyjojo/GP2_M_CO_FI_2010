/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This code is based on linux driver for sl811hs chip, source at
 * drivers/usb/host/sl811.c:
 *
 * SL811 Host Controller Interface driver for USB.
 *
 * Copyright (c) 2003/06, Courage Co., Ltd.
 *
 * Based on:
 *	1.uhci.c by Linus Torvalds, Johannes Erdfelt, Randy Dunlap,
 *	  Georg Acher, Deti Fliegl, Thomas Sailer, Roman Weissgaerber,
 *	  Adam Richter, Gregory P. Smith;
 *	2.Original SL811 driver (hc_sl811.o) by Pei Liu <pbl@cypress.com>
 *	3.Rewrited as sl811.o by Yin Aihua <yinah:couragetech.com.cn>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#ifdef  CONFIG_USB_EHCI
#include <usb.h>
#include <asm-mips/io.h>
#include "ehci_usb.h"

#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
#define HW_BASE			0xbf200000
#else
#define HW_BASE			0xbf800000
#endif

static int root_hub_devnum = 0;
static struct usb_port_status rh_status = { 0 };/* root hub port status */

#define mdelay(n)		({unsigned long msec=(n); while (msec--) udelay(1000);})
#ifdef SL811_DEBUG
#define PDEBUG(level, fmt, args...) \
	if (debug >= (level)) printf("[%s:%d] " fmt, \
			__PRETTY_FUNCTION__, __LINE__ , ## args)
#else
#define PDEBUG(level, fmt, args...) do {} while(0)
#endif

#if 0
static int titania_rh_submit_urb(struct usb_device *usb_dev, unsigned long pipe,
		void *data, int buf_len, struct devrequest *cmd);
#endif
extern int UsbPortSelect;

static void Titania_usb_init(void)
{
	//===========================================================================
#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_GP2_DEMO1)
	writeb(0x0A, (void*) (0xbf000000+0x100700*2)); // Disable MAC initial suspend, Reset UHC
	writeb(0x28, (void*) (0xbf000000+0x100700*2)); // Release UHC reset, enable UHC XIU function

	writeb(readb((void*)0xbf000000+(0x103A80+0x3C)*2) | 0x01, (void*) (0xbf000000+(0x103A80+0x3C)*2)); // set CA_START as 1
	mdelay(10);

	writeb(readb((void*)0xbf000000+(0x103A80+0x3C)*2) & ~0x01, (void*) (0xbf000000+(0x103A80+0x3C)*2)); // release CA_START

	while ((readb((void*)0xbf000000+(0x103A80+0x3C)*2) & 0x02) == 0);	// polling bit <1> (CA_END)

	writeb(readb((void*)0xbf000000+(0x100700+0x02)*2) & ~0x03, (void*) (0xbf000000+(0x100700+0x02)*2)); //UHC select enable
	writeb(readb((void*)0xbf000000+(0x100700+0x02)*2) | 0x01, (void*) (0xbf000000+(0x100700+0x02)*2)); //UHC select enable

	writeb(readb((void*)0xbf000000+(0x102400+0x40)*2) & ~0x10, (void*) (0xbf000000+(0x102400+0x40)*2)); //0: VBUS On.
	mdelay(1);	// delay 1ms

	writeb(readb((void*)0xbf000000+(0x102400+0x40)*2) | 0x08, (void*) (0xbf000000+(0x102400+0x40)*2)); // Active HIGH
	mdelay(1);	// delay 1ms

	writeb(readb((void*)0xbf000000+(0x103a80+0x06)*2) | 0x40, (void*) (0xbf000000+(0x103a80+0x06)*2)); //CDR stage select
	writeb(readb((void*)0xbf000000+(0x103a80+0x06)*2) & ~0x20, (void*) (0xbf000000+(0x103a80+0x06)*2)); //CDR stage select

	writeb(readb((void*)0xbf000000+(0x103a80+0x03)*2-1) | 0x28, (void*) (0xbf000000+(0x103a80+0x03)*2-1)); //Disconnect window select
	writeb(readb((void*)0xbf000000+(0x103a80+0x03)*2-1) & 0xef, (void*) (0xbf000000+(0x103a80+0x03)*2-1)); //Disconnect window select
	writeb(readb((void*)0xbf000000+(0x103a80+0x07)*2-1) & 0xfd, (void*) (0xbf000000+(0x103a80+0x07)*2-1)); //Disable improved CDR
	writeb(readb((void*)0xbf000000+(0x103a80+0x09)*2-1) |0x81, (void*) (0xbf000000+(0x103a80+0x09)*2-1)); // UTMI RX anti-dead-loc, ISI effect improvement
	writeb(readb((void*)0xbf000000+(0x103a80+0x0b)*2-1) |0x80, (void*) (0xbf000000+(0x103a80+0x0b)*2-1)); // TX timing select latch path
	writeb(readb((void*)0xbf000000+(0x103a80+0x15)*2-1) |0x20, (void*) (0xbf000000+(0x103a80+0x15)*2-1)); // Chirp signal source select

	writeb(readb((void*)0xbf000000+(0x103a80+0x2c)*2) |0x10, (void*) (0xbf000000+(0x103a80+0x2c)*2));
	writeb(readb((void*)0xbf000000+(0x103a80+0x2d)*2-1) |0x02, (void*) (0xbf000000+(0x103a80+0x2d)*2-1));
	writeb(readb((void*)0xbf000000+(0x103a80+0x2f)*2-1) |0x81, (void*) (0xbf000000+(0x103a80+0x2f)*2-1));
#else
	writeb(0x00,HW_BASE+0x4b80);
	//XBYTE[0x3aac]|=0x04;						//Select current switch from DVI
	writeb(readb(HW_BASE+0x7558)|0x04,HW_BASE+0x7558);
	//XBYTE[0x3a86]|=0x04;						//HSTXIEN
	writeb(readb(HW_BASE+0x750c)|0x04,HW_BASE+0x750c);
	//XBYTE[0x3a80]=0x0;							//disable power down over write
	//XBYTE[0x3a81]=0x0;							//disable power down over write high byte
	writeb(0x00,HW_BASE+0x7500);
	writeb(0x00,HW_BASE+0x7501);
	//XBYTE[0x0700]|=0x28;						//disable init suspend state
	writeb(readb(HW_BASE+0xe00)|0x28,HW_BASE+0xe00);
	//XBYTE[0x0702]|=0x01;						//UHC select enable
	writeb(readb(HW_BASE+0xe04)|0x01,HW_BASE+0xe04);
	//XBYTE[0x2434] |= 0x40;
	writeb(readb(HW_BASE+0x4868)|0x40,HW_BASE+0x4868);
	//MDrv_Timer_Delayms(2);
	mdelay(2);
	//XBYTE[0x2440] &=~0x10;						//EHCI Vbsu turn on
	writeb(readb(HW_BASE+0x4880)&~0x10,HW_BASE+0x4880);
	mdelay(2);
	//===========================================================================
	//UHC_XBYTE(0x34)&=0xBF;					//set suspend
	writeb(readb(HW_BASE+0x4868)&0xbf,HW_BASE+0x4868);
	//UHC_XBYTE(0x34)|=0x40;						//clr suspend
	writeb(readb(HW_BASE+0x4868)|0x40,HW_BASE+0x4868);
	//MDrv_Timer_Delayms(2);
	mdelay(2);
	//XBYTE[UTMIBaseAddr+0x00]|=0x01;			// override mode enable for power down control
	writeb(readb(HW_BASE+0x7500)|0x01,HW_BASE+0x7500);
	//XBYTE[UTMIBaseAddr+0x01]|=0x40;			// enable IREF power down
	writeb(readb(HW_BASE+0x7501)|0x40,HW_BASE+0x7501);
	//XBYTE[UTMIBaseAddr+0x01]|=0x02;			// enable PLL power down
	writeb(readb(HW_BASE+0x7501)|0x02,HW_BASE+0x7501);
	//XBYTE[UTMIBaseAddr+0x01]&=0xFD;			// disable PLL power down
	writeb(readb(HW_BASE+0x7501)&0xfd,HW_BASE+0x7501);
	mdelay(1);
	writeb(readb(HW_BASE+0x7500)&0xfe,HW_BASE+0x7500);	//disable override mode for power down control
	//===========================================================================
	writeb(readb(HW_BASE+0x750c)|0x44,HW_BASE+0x750c); 	//Force HS TX current enable and CDR stage select
	writeb(readb(HW_BASE+0x750c)|0x03,HW_BASE+0x750c);	//reset UTMI
	writeb(readb(HW_BASE+0x750c)&0xfc,HW_BASE+0x750c);
	mdelay(1);
	writeb(readb(HW_BASE+0x7551)|0x08,HW_BASE+0x7551);	//disable full speed retime
	writeb(0xa8,HW_BASE+0x7505);
#if 1
	//XBYTE[UTMIBaseAddr+0x07]|=0x02;
	writeb(readb(HW_BASE+0x750d)&0xfd,HW_BASE+0x750d);
	writeb(readb(HW_BASE+0x7558)|0xc5,HW_BASE+0x7558);
	//XBYTE[UTMIBaseAddr+0x2d]=0x3b;				//enable TX common mode,
	writeb(0x3b,HW_BASE+0x7559);
	//XBYTE[UTMIBaseAddr+0x2f]|=0x0e;			//preemsis
	writeb(readb(HW_BASE+0x755d)|0x0e,HW_BASE+0x755d);
#else
	//XBYTE[UTMIBaseAddr+0x2c]|=0x01;
	writeb(readb(HW_BASE+0x7558)|0x01,HW_BASE+0x7558);
	//XBYTE[UTMIBaseAddr+0x2d]=0x38;         //disable TX common mode
	writeb(0x38,HW_BASE+0x7559);
#endif
	writeb(readb(HW_BASE+0x7525)|0x02,HW_BASE+0x7525);	//ISI improvement
	writeb(readb(HW_BASE+0x7511)|0xe0,HW_BASE+0x7511);	//patch low tempture,FL meta issue and enable new FL RX engin
	writeb(readb(HW_BASE+0x754d)&0xf3,HW_BASE+0x754d);
	writeb(readb(HW_BASE+0x754d)|0x08,HW_BASE+0x754d);
	writeb(readb(HW_BASE+0x7554)&0xf0,HW_BASE+0x7554);
	writeb(readb(HW_BASE+0x7529)|0x20,HW_BASE+0x7529);
	writeb(readb(HW_BASE+0x4880)&~0x10,HW_BASE+0x4880);
	writeb(readb(HW_BASE+0x4868)|0x0c,HW_BASE+0x4868);
#endif
	//===========================================================================
}

void Titania_usb_init_port1(void)
{
#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_GP2_DEMO1)
	writeb(0x0A, (void*) (0xbf000000+0x100780*2)); // Disable MAC initial suspend, Reset UHC
	writeb(0x28, (void*) (0xbf000000+0x100780*2)); // Release UHC reset, enable UHC XIU function

	writeb(readb((void*)0xbf000000+(0x103a00+0x3C)*2) | 0x01, (void*) (0xbf000000+(0x103a00+0x3C)*2)); // set CA_START as 1
	mdelay(10);

	writeb(readb((void*)0xbf000000+(0x103a00+0x3C)*2) & ~0x01, (void*) (0xbf000000+(0x103a00+0x3C)*2)); // release CA_START

	while ((readb((void*)0xbf000000+(0x103a00+0x3C)*2) & 0x02) == 0);	// polling bit <1> (CA_END)

	writeb(readb((void*)0xbf000000+(0x100780+0x02)*2) & ~0x03, (void*) (0xbf000000+(0x100780+0x02)*2)); //UHC select enable
	writeb(readb((void*)0xbf000000+(0x100780+0x02)*2) | 0x01, (void*) (0xbf000000+(0x100780+0x02)*2)); //UHC select enable

	writeb(readb((void*)0xbf000000+(0x100D00+0x40)*2) & ~0x10, (void*) (0xbf000000+(0x100D00+0x40)*2)); //0: VBUS On.
	mdelay(1);	// delay 1ms

	writeb(readb((void*)0xbf000000+(0x100D00+0x40)*2) | 0x08, (void*) (0xbf000000+(0x100D00+0x40)*2)); // Active HIGH
	mdelay(1);	// delay 1ms

	writeb(readb((void*)0xbf000000+(0x103a00+0x06)*2) | 0x40, (void*) (0xbf000000+(0x103a00+0x06)*2)); //CDR stage select
	writeb(readb((void*)0xbf000000+(0x103a00+0x06)*2) & ~0x20, (void*) (0xbf000000+(0x103a00+0x06)*2)); //CDR stage select

	writeb(readb((void*)0xbf000000+(0x103a00+0x03)*2-1) | 0x28, (void*) (0xbf000000+(0x103a00+0x03)*2-1)); //Disconnect window select
	writeb(readb((void*)0xbf000000+(0x103a00+0x03)*2-1) & 0xef, (void*) (0xbf000000+(0x103a00+0x03)*2-1)); //Disconnect window select
	writeb(readb((void*)0xbf000000+(0x103a00+0x07)*2-1) & 0xfd, (void*) (0xbf000000+(0x103a00+0x07)*2-1)); //Disable improved CDR
	writeb(readb((void*)0xbf000000+(0x103a00+0x09)*2-1) |0x81, (void*) (0xbf000000+(0x103a00+0x09)*2-1)); // UTMI RX anti-dead-loc, ISI effect improvement
	writeb(readb((void*)0xbf000000+(0x103a00+0x0b)*2-1) |0x80, (void*) (0xbf000000+(0x103a00+0x0b)*2-1)); // TX timing select latch path
	writeb(readb((void*)0xbf000000+(0x103a00+0x15)*2-1) |0x20, (void*) (0xbf000000+(0x103a00+0x15)*2-1)); // Chirp signal source select

	writeb(readb((void*)0xbf000000+(0x103a00+0x2c)*2) |0x10, (void*) (0xbf000000+(0x103a00+0x2c)*2));
	writeb(readb((void*)0xbf000000+(0x103a00+0x2d)*2-1) |0x02, (void*) (0xbf000000+(0x103a00+0x2d)*2-1));
	writeb(readb((void*)0xbf000000+(0x103a00+0x2f)*2-1) |0x81, (void*) (0xbf000000+(0x103a00+0x2f)*2-1));
#else
	writeb(0x00,HW_BASE+0x4b80);
	//open port 1
	//  XBYTE[0x3a2c]|=0x04; //Select current switch from DVI
	writeb(readb(HW_BASE+0x7458)|0x04,HW_BASE+0x7458);

	//XBYTE[0x3a06]|=0x04; //HSTXIEN
	writeb(readb(HW_BASE+0x740c)|0x04,HW_BASE+0x740c);
	//    XBYTE[0x3a00]=0x0;   //disable power down over write
	//    XBYTE[0x3a01]=0x0;   //disable power down over write high byte
	writeb(0x00,HW_BASE+0x7400);
	writeb(0x00,HW_BASE+0x7401);
	//    XBYTE[0x0780]|=0x28; //disable init suspend state
	writeb(readb(HW_BASE+0x0f00)|0x28,HW_BASE+0x0f00);
	mdelay(2);
	writeb(readb(HW_BASE+0x0f04)|0x01,HW_BASE+0x0f04);
	//    XBYTE[0x0782]|=0x01;  //UHC select enable
	mdelay(2);
	// Power On USB
	// XBYTE[0x0d34] |= 0x40;
	writeb(readb(HW_BASE+0x1a68)|0x40,HW_BASE+0x1a68);
	mdelay(2);
	writeb(readb(HW_BASE+0x1a80)&~0x10,HW_BASE+0x1a80);
	//==============usb init part=============================================================

	//UHC_XBYTE(0x34)&=0xBF;                            //set suspend
	writeb(readb(HW_BASE+0x1a68)&0xbf,HW_BASE+0x1a68);
	//UHC_XBYTE(0x34)|=0x40;                            //clr suspend
	writeb(readb(HW_BASE+0x1a68)|0x40,HW_BASE+0x1a68);
	//MDrv_Timer_Delayms(2);
	mdelay(2);
	//XBYTE[UTMIBaseAddr+0x00]|=0x01;                   // override mode enable for power down control

	writeb(readb(HW_BASE+0x7400)|0x01,HW_BASE+0x7400);
	//XBYTE[UTMIBaseAddr+0x01]|=0x40;                   // enable IREF power down

	writeb(readb(HW_BASE+0x7401)|0x40,HW_BASE+0x7401);

	//XBYTE[UTMIBaseAddr+0x01]|=0x02;                   // enable PLL power down
	writeb(readb(HW_BASE+0x7401)|0x02,HW_BASE+0x7401);

	//XBYTE[UTMIBaseAddr+0x01]&=0xFD;                   // disable PLL power down
	writeb(readb(HW_BASE+0x7401)&0xfd,HW_BASE+0x7401);
	mdelay(1);
	writeb(readb(HW_BASE+0x7400)&0xfe,HW_BASE+0x7400);          //disable override mode for power down control

	//===========================================================================

	writeb(readb(HW_BASE+0x740c)|0x44,HW_BASE+0x740c);   //Force HS TX current enable and CDR stage select
	writeb(readb(HW_BASE+0x740c)|0x03,HW_BASE+0x740c);   //reset UTMI
	writeb(readb(HW_BASE+0x740c)&0xfc,HW_BASE+0x740c);
	mdelay(1);
	writeb(readb(HW_BASE+0x7451)|0x08,HW_BASE+0x7451);   //disable full speed retime
	writeb(0xa8,HW_BASE+0x7405);

#if 1
	//XBYTE[UTMIBaseAddr+0x07]|=0x02;

	writeb(readb(HW_BASE+0x740d)&0xfd,HW_BASE+0x740d);
	writeb(readb(HW_BASE+0x7458)|0xc5,HW_BASE+0x7458);
	//XBYTE[UTMIBaseAddr+0x2d]=0x3b;                                       //enable TX common mode,

	writeb(0x3b,HW_BASE+0x7459);
	//XBYTE[UTMIBaseAddr+0x2f]|=0x0e;                            //preemsis

	writeb(readb(HW_BASE+0x745d)|0x0e,HW_BASE+0x745d);

#else
	//XBYTE[UTMIBaseAddr+0x2c]|=0x01;

	writeb(readb(HW_BASE+0x7558)|0x01,HW_BASE+0x7558);

	//XBYTE[UTMIBaseAddr+0x2d]=0x38;         //disable TX common mode

	writeb(0x38,HW_BASE+0x7559);
#endif
	writeb(readb(HW_BASE+0x7425)|0x02,HW_BASE+0x7425);  //ISI improvement
	writeb(readb(HW_BASE+0x7411)|0xe0,HW_BASE+0x7411);   //patch low tempture,FL meta issue and enable new FL RX engin
	writeb(readb(HW_BASE+0x744d)&0xf3,HW_BASE+0x744d);
	writeb(readb(HW_BASE+0x744d)|0x08,HW_BASE+0x744d);
	writeb(readb(HW_BASE+0x7454)&0xf0,HW_BASE+0x7454);
	writeb(readb(HW_BASE+0x7429)|0x20,HW_BASE+0x7429);
	writeb(readb(HW_BASE+0x1a80)&~0x10,HW_BASE+0x1a80);
	writeb(readb(HW_BASE+0x1a68)|0x0c,HW_BASE+0x1a68);
#endif
	//===========================================================================
}

extern int Usb_host_Init(void);
extern int Usb_host_Init2(void);

int usb_lowlevel_init(void)
{
	int speed;
	root_hub_devnum = 0;

	if (UsbPortSelect==0)
	{
		Titania_usb_init();
		speed=Usb_host_Init();
	}
	else
	{
		Titania_usb_init_port1();
		speed=Usb_host_Init2();
	}
#if 1				//do we really it????
	//dhjung LGE
	if (speed == -1)
		return -1;
	else if (speed == 1)	//low speed
		rh_status.wPortStatus |= USB_PORT_STAT_CONNECTION | USB_PORT_STAT_LOW_SPEED;
	else
	{
		rh_status.wPortStatus |= USB_PORT_STAT_CONNECTION;
		rh_status.wPortStatus &= ~USB_PORT_STAT_LOW_SPEED;
	}
	rh_status.wPortChange |= USB_PORT_STAT_C_CONNECTION;
#endif
	return 0;
}

extern void MDrv_UsbClose(void);
extern void MDrv_UsbClose2(void);
extern void usb_lan_release(void);

int usb_lowlevel_stop(int p)
{
	if (p == 0xff)                 //use UsbPortSelect
		p=UsbPortSelect;

	usb_lan_release();

	if (p == 0)
		MDrv_UsbClose();
	else
		MDrv_UsbClose2();
	return 0;
}

extern unsigned char Send_Receive_Bulk_Data(void *buffer,int len,int dir_out);
extern unsigned char Send_Receive_Bulk_Data2(void *buffer,int len,int dir_out);

extern unsigned long  gUsbStatus,gUsbStatusP1;
extern int gTotalBytes,gTotalBytesP1;
int usb_bulk_transfer_in(struct usb_device *dev, void *data, int len,int *transdata)
{
	*transdata=0;
	if (UsbPortSelect==0)
	{
		if (Send_Receive_Bulk_Data(data,len,0)>0)
		{
			dev->status=gUsbStatus;
			return -1;
		}
		*transdata=len-gTotalBytes;
	}
	else
	{
		if (Send_Receive_Bulk_Data2(data,len,0)>0)
		{
			dev->status=gUsbStatusP1;
			return -1;
		}
		*transdata=len-gTotalBytesP1;
	}
	if(dev->status==0)
		return 0;
	else
		return -1;
}

int usb_bulk_transfer_out(struct usb_device *dev, void *data, int len)
{
	if (UsbPortSelect==0)
	{
		if (Send_Receive_Bulk_Data(data,len,1)>0)
		{
			dev->status=gUsbStatus;
			return -1;
		}
	}
	else
	{
		if (Send_Receive_Bulk_Data2(data,len,1)>0)
		{
			dev->status=gUsbStatusP1;
			return -1;
		}
	}
	if(dev->status==0)
		return 0;
	else
		return -1;
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int len)
{
	int dir_out = usb_pipeout(pipe);
	int done	= 0;

	dev->status = 0;

	if (UsbPortSelect==0)
	{
		if (Send_Receive_Bulk_Data(buffer,len,dir_out)>0)
		{
			dev->status=gUsbStatus;
			return -1;
		}
	}
	else
	{
		if (Send_Receive_Bulk_Data2(buffer,len,dir_out)>0)
		{
			dev->status=gUsbStatusP1;
			return -1;
		}
	}
	dev->act_len = done;

	return 0;
}

extern unsigned char flib_Host20_Issue_Control (unsigned char bEdNum, unsigned char* pbCmd, unsigned short hwDataSize, unsigned char* pbData);
extern unsigned char flib_Host20_Issue_Control2 (unsigned char bEdNum, unsigned char* pbCmd, unsigned short hwDataSize, unsigned char* pbData);
int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		int len,struct devrequest *setup)
{
	int done		= 0;
	dev->status = 0;

	if (UsbPortSelect==0)
	{

		if ( flib_Host20_Issue_Control (dev->devnum,(unsigned char*) setup,len,(unsigned char*) buffer) > 0)
		{
			dev->status=-1;
		}
	}
	else
	{
		if ( flib_Host20_Issue_Control2(dev->devnum,(unsigned char*) setup,len,(unsigned char*) buffer) > 0)
		{
			dev->status=-1;
		}
	}

	/* status phase */
	dev->act_len = len;

	return done;
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int len, int interval)
{
	return -1;
}

//dhjung LGE
#if 0
/*
 * SL811 Virtual Root Hub
 */
/* Device descriptor */
static __u8 sl811_rh_dev_des[] =
{
	0x12,	    /*	__u8  bLength; */
	0x01,	    /*	__u8  bDescriptorType; Device */
	0x10,	    /*	__u16 bcdUSB; v1.1 */
	0x01,
	0x09,	    /*	__u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*	__u8  bDeviceSubClass; */
	0x00,	    /*	__u8  bDeviceProtocol; */
	0x08,	    /*	__u8  bMaxPacketSize0; 8 Bytes */
	0x00,	    /*	__u16 idVendor; */
	0x00,
	0x00,	    /*	__u16 idProduct; */
	0x00,
	0x00,	    /*	__u16 bcdDevice; */
	0x00,
	0x00,	    /*	__u8  iManufacturer; */
	0x02,	    /*	__u8  iProduct; */
	0x01,	    /*	__u8  iSerialNumber; */
	0x01	    /*	__u8  bNumConfigurations; */
};

/* Configuration descriptor */
static __u8 sl811_rh_config_des[] =
{
	0x09,	    /*	__u8  bLength; */
	0x02,	    /*	__u8  bDescriptorType; Configuration */
	0x19,	    /*	__u16 wTotalLength; */
	0x00,
	0x01,	    /*	__u8  bNumInterfaces; */
	0x01,	    /*	__u8  bConfigurationValue; */
	0x00,	    /*	__u8  iConfiguration; */
	0x40,	    /*	__u8  bmAttributes;
					Bit 7: Bus-powered, 6: Self-powered, 5 Remote-wakwup,
					4..0: resvd */
	0x00,	    /*	__u8  MaxPower; */

	/* interface */
	0x09,	    /*	__u8  if_bLength; */
	0x04,	    /*	__u8  if_bDescriptorType; Interface */
	0x00,	    /*	__u8  if_bInterfaceNumber; */
	0x00,	    /*	__u8  if_bAlternateSetting; */
	0x01,	    /*	__u8  if_bNumEndpoints; */
	0x09,	    /*	__u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,	    /*	__u8  if_bInterfaceSubClass; */
	0x00,	    /*	__u8  if_bInterfaceProtocol; */
	0x00,	    /*	__u8  if_iInterface; */

	/* endpoint */
	0x07,	    /*	__u8  ep_bLength; */
	0x05,	    /*	__u8  ep_bDescriptorType; Endpoint */
	0x81,	    /*	__u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,	    /*	__u8  ep_bmAttributes; Interrupt */
	0x08,	    /*	__u16 ep_wMaxPacketSize; */
	0x00,
	0xff	    /*	__u8  ep_bInterval; 255 ms */
};

/* root hub class descriptor*/
static __u8 sl811_rh_hub_des[] =
{
	0x09,			/*  __u8  bLength; */
	0x29,			/*  __u8  bDescriptorType; Hub-descriptor */
	0x01,			/*  __u8  bNbrPorts; */
	0x00,			/* __u16  wHubCharacteristics; */
	0x00,
	0x50,			/*  __u8  bPwrOn2pwrGood; 2ms */
	0x00,			/*  __u8  bHubContrCurrent; 0 mA */
	0xfc,			/*  __u8  DeviceRemovable; *** 7 Ports max *** */
	0xff			/*  __u8  PortPwrCtrlMask; *** 7 ports max *** */
};

/*
 * helper routine for returning string descriptors in UTF-16LE
 * input can actually be ISO-8859-1; ASCII is its 7-bit subset
 */
static int ascii2utf (char *s, u8 *utf, int utfmax)
{
	int retval;

	for (retval = 0; *s && utfmax > 1; utfmax -= 2, retval += 2) {
		*utf++ = *s++;
		*utf++ = 0;
	}
	return retval;
}

/*
 * root_hub_string is used by each host controller's root hub code,
 * so that they're identified consistently throughout the system.
 */
static int usb_root_hub_string (int id, int serial, char *type, __u8 *data, int len)
{
	char buf [30];

	/* assert (len > (2 * (sizeof (buf) + 1)));
	   assert (strlen (type) <= 8);*/

	/* language ids */
	if (id == 0) {
		*data++ = 4; *data++ = 3;	/* 4 bytes data */
		*data++ = 0; *data++ = 0;	/* some language id */
		return 4;

		/* serial number */
	} else if (id == 1) {
		sprintf (buf, "%#x", serial);

		/* product description */
	} else if (id == 2) {
		sprintf (buf, "USB %s Root Hub", type);

		/* id 3 == vendor description */

		/* unsupported IDs --> "stall" */
	} else
		return 0;

	ascii2utf (buf, data + 2, len - 2);
	data [0] = 2 + strlen(buf) * 2;
	data [1] = 3;
	return data [0];
}

/* helper macro */
#define OK(x)	len = (x); break

/*
 * This function handles all USB request to the the virtual root hub
 */
static int titania_rh_submit_urb(struct usb_device *usb_dev, unsigned long pipe,
		void *data, int buf_len, struct devrequest *cmd)
{
	__u8 data_buf[16];
	__u8 *bufp = data_buf;
	int len = 0;
	int status = 0;

	__u16 bmRType_bReq;
	__u16 wValue;
	__u16 wIndex;
	__u16 wLength;

	if (usb_pipeint(pipe)) {
		PDEBUG(0, "interrupt transfer unimplemented!\n");
		return 0;
	}

	bmRType_bReq  = cmd->requesttype | (cmd->request << 8);
	wValue	      = le16_to_cpu (cmd->value);
	wIndex	      = le16_to_cpu (cmd->index);
	wLength	      = le16_to_cpu (cmd->length);

	PDEBUG(5, "submit rh urb, req = %d(%x) val = %#x index = %#x len=%d\n",
			bmRType_bReq, bmRType_bReq, wValue, wIndex, wLength);

	/* 	Request Destination:
		without flags: Device,
		USB_RECIP_INTERFACE: interface,
		USB_RECIP_ENDPOINT: endpoint,
		USB_TYPE_CLASS means HUB here,
		USB_RECIP_OTHER | USB_TYPE_CLASS  almost ever means HUB_PORT here
	*/
	switch (bmRType_bReq) {
		case RH_GET_STATUS:
			*(__u16 *)bufp = cpu_to_le16(1);
			OK(2);

		case RH_GET_STATUS | USB_RECIP_INTERFACE:
			*(__u16 *)bufp = cpu_to_le16(0);
			OK(2);

		case RH_GET_STATUS | USB_RECIP_ENDPOINT:
			*(__u16 *)bufp = cpu_to_le16(0);
			OK(2);

		case RH_GET_STATUS | USB_TYPE_CLASS:
			*(__u32 *)bufp = cpu_to_le32(0);
			OK(4);

		case RH_GET_STATUS | USB_RECIP_OTHER | USB_TYPE_CLASS:
			*(__u32 *)bufp = cpu_to_le32(rh_status.wPortChange<<16 | rh_status.wPortStatus);
			OK(4);

		case RH_CLEAR_FEATURE | USB_RECIP_ENDPOINT:
			switch (wValue) {
				case 1:
					OK(0);
			}
			break;

		case RH_CLEAR_FEATURE | USB_TYPE_CLASS:
			switch (wValue) {
				case C_HUB_LOCAL_POWER:
					OK(0);

				case C_HUB_OVER_CURRENT:
					OK(0);
			}
			break;

		case RH_CLEAR_FEATURE | USB_RECIP_OTHER | USB_TYPE_CLASS:
			switch (wValue) {
				case USB_PORT_FEAT_ENABLE:
					rh_status.wPortStatus &= ~USB_PORT_STAT_ENABLE;
					OK(0);

				case USB_PORT_FEAT_SUSPEND:
					rh_status.wPortStatus &= ~USB_PORT_STAT_SUSPEND;
					OK(0);

				case USB_PORT_FEAT_POWER:
					rh_status.wPortStatus &= ~USB_PORT_STAT_POWER;
					OK(0);

				case USB_PORT_FEAT_C_CONNECTION:
					rh_status.wPortChange &= ~USB_PORT_STAT_C_CONNECTION;
					OK(0);

				case USB_PORT_FEAT_C_ENABLE:
					rh_status.wPortChange &= ~USB_PORT_STAT_C_ENABLE;
					OK(0);

				case USB_PORT_FEAT_C_SUSPEND:
					rh_status.wPortChange &= ~USB_PORT_STAT_C_SUSPEND;
					OK(0);

				case USB_PORT_FEAT_C_OVER_CURRENT:
					rh_status.wPortChange &= ~USB_PORT_STAT_C_OVERCURRENT;
					OK(0);

				case USB_PORT_FEAT_C_RESET:
					rh_status.wPortChange &= ~USB_PORT_STAT_C_RESET;
					OK(0);
			}
			break;

		case RH_SET_FEATURE | USB_RECIP_OTHER | USB_TYPE_CLASS:
			switch (wValue) {
				case USB_PORT_FEAT_SUSPEND:
					rh_status.wPortStatus |= USB_PORT_STAT_SUSPEND;
					OK(0);

				case USB_PORT_FEAT_RESET:
					rh_status.wPortStatus |= USB_PORT_STAT_RESET;
					rh_status.wPortChange = 0;
					rh_status.wPortChange |= USB_PORT_STAT_C_RESET;
					rh_status.wPortStatus &= ~USB_PORT_STAT_RESET;
					rh_status.wPortStatus |= USB_PORT_STAT_ENABLE;
					OK(0);

				case USB_PORT_FEAT_POWER:
					rh_status.wPortStatus |= USB_PORT_STAT_POWER;
					OK(0);

				case USB_PORT_FEAT_ENABLE:
					rh_status.wPortStatus |= USB_PORT_STAT_ENABLE;
					OK(0);
			}
			break;

		case RH_SET_ADDRESS:
			root_hub_devnum = wValue;
			OK(0);

		case RH_GET_DESCRIPTOR:
			switch ((wValue & 0xff00) >> 8) {
				case USB_DT_DEVICE:
					len = sizeof(sl811_rh_dev_des);
					bufp = sl811_rh_dev_des;
					OK(len);

				case USB_DT_CONFIG:
					len = sizeof(sl811_rh_config_des);
					bufp = sl811_rh_config_des;
					OK(len);

				case USB_DT_STRING:
					len = usb_root_hub_string(wValue & 0xff, (int)(long)0,	"SL811HS", data, wLength);
					if (len > 0) {
						bufp = data;
						OK(len);
					}

				default:
					status = -32;
			}
			break;

		case RH_GET_DESCRIPTOR | USB_TYPE_CLASS:
			len = sizeof(sl811_rh_hub_des);
			bufp = sl811_rh_hub_des;
			OK(len);

		case RH_GET_CONFIGURATION:
			bufp[0] = 0x01;
			OK(1);

		case RH_SET_CONFIGURATION:
			OK(0);

		default:
			PDEBUG(1, "unsupported root hub command\n");
			status = -32;
	}

	len = min(len, buf_len);
	if (data != bufp)
		memcpy(data, bufp, len);

	PDEBUG(5, "len = %d, status = %d\n", len, status);

	usb_dev->status = status;
	usb_dev->act_len = len;

	return status == 0 ? len : status;
}
#endif

#endif	/* CONFIG_USB_SL811HS */
