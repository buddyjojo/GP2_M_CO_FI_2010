////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¨MStar Confidential Information〃) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#include <linux/platform_device.h>
#include "ehci.h"

#if 0	//tony
static int ehci_mstar_reinit(struct ehci_hcd *ehci)
{
	int	retval;

	retval = ehci_init(ehci);
	ehci_port_power(ehci, 0);

	return retval;
}
#endif

#if 0
static const struct hc_driver ehci_driver = {
	.description =		hcd_name,

	/*
	 * generic hardware linkage
	 */
	.irq =			ehci_irq,
	.flags =		HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 */
	.reset =		ehci_hc_reset,
	.start =		ehci_start,
#ifdef	CONFIG_PM
	.suspend =		ehci_suspend,
	.resume =		ehci_resume,
#endif
	.stop =			ehci_stop,

	/*
	 * memory lifecycle (except per-request)
	 */
	.hcd_alloc =		ehci_hcd_alloc,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ehci_urb_enqueue,
	.urb_dequeue =		ehci_urb_dequeue,
	.endpoint_disable =	ehci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ehci_hub_status_data,
	.hub_control =		ehci_hub_control,
	.hub_suspend =		ehci_hub_suspend,
	.hub_resume =		ehci_hub_resume,
};
#endif

static const struct hc_driver ehci_mstar_hc_driver = {
	.description = hcd_name,
	.product_desc = "Mstar EHCI",
	.hcd_priv_size = sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq = ehci_irq,
	.flags = HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 *
	 * FIXME -- ehci_init() doesn't do enough here.
	 * See ehci-ppc-soc for a complete implementation.
	 */
#if 1	//tony
	.reset = ehci_init,
#else
	.reset = ehci_mstar_reinit,
#endif
	.start = ehci_run,
	.stop = ehci_stop,
	.shutdown = ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue = ehci_urb_enqueue,
	.urb_dequeue = ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number = ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data = ehci_hub_status_data,
	.hub_control = ehci_hub_control,
	.bus_suspend = ehci_bus_suspend,
	.bus_resume = ehci_bus_resume,
	.relinquish_port = ehci_relinquish_port,
	.port_handed_over = ehci_port_handed_over,
};

#ifdef CONFIG_Triton
static void uranus_start_ehc(struct platform_device *dev)
{
	u8 tmp=0;
	//===========================================================================
	//writeb(0x00,0xbf804b80);
	//writeb(0x00,0xbf804b80);
	//XBYTE[0x3aac]|=0x04;							//Select current switch from DVI
	//writeb(readb(0xbf807558)|0x04,0xbf807558);
	//writeb(readb(0xbf806258)|0x04,0xbf806258);	//bit2=0
	//XBYTE[0x3a86]|=0x04;							//HSTXIEN
	//writeb(readb(0xbf80750c)|0x04,0xbf80750c);
	writeb(readb(0xbf80620c)|0x04,0xbf80620c);		//
	//XBYTE[0x3a80]=0x0;							//disable power down over write
	//XBYTE[0x3a81]=0x0;							//disable power down over write high byte
	//writeb(0x00,0xbf807500);
	writeb(0x00,0xbf806200);
	//writeb(0x00,0xbf807501);
	writeb(0x00,0xbf806201);

	//XBYTE[0x0700]|=0x28;							//disable init suspend state
	writeb(readb(0xbf805e00)|0x28,0xbf805e00);		//usbc disable init suspend state
	//XBYTE[0x0702]|=0x01;					  		//UHC select enable
	writeb(readb(0xbf805e04)|0x01,0xbf805e04);		//usbc UHC select enable

	//XBYTE[0x2434] |= 0x40;
	//writeb(readb(0xbf804868)|0x40,0xbf804868);
	writeb(readb(0xbf805c68)|0x40,0xbf805c68);
	//MDrv_Timer_Delayms(2);
	mdelay(2);
	//XBYTE[0x2440] &=~0x10;						//EHCI Vbsu turn on
	//writeb(readb(0xbf804880)&~0x10,0xbf804880);
	writeb(readb(0xbf805c80)&~0x10,0xbf805c80);
	mdelay(2);
	//===========================================================================
	//UHC_XBYTE(0x34)&=0xBF;						//set suspend
	//writeb(readb(0xbf804868)&0xbf,0xbf804868);
	writeb(readb(0xbf805c68)&0xbf,0xbf805c68);
	//UHC_XBYTE(0x34)|=0x40;						//clr suspend
	//writeb(readb(0xbf804868)|0x40,0xbf804868);
	writeb(readb(0xbf805c68)|0x40,0xbf805c68);
	//MDrv_Timer_Delayms(2);
	mdelay(2);
	//XBYTE[UTMIBaseAddr+0x00]|=0x01;			// override mode enable for power down control
	//writeb(readb(0xbf807500)|0x01,0xbf807500);
	writeb(readb(0xbf806200)|0x01,0xbf806200);
	//XBYTE[UTMIBaseAddr+0x01]|=0x40;			// enable IREF power down
	//writeb(readb(0xbf807501)|0x40,0xbf807501);
	writeb(readb(0xbf806201)|0x40,0xbf806201);
	//XBYTE[UTMIBaseAddr+0x01]|=0x02;			// enable PLL power down
	//writeb(readb(0xbf807501)|0x02,0xbf807501);
	writeb(readb(0xbf806201)|0x02,0xbf806201);
	//XBYTE[UTMIBaseAddr+0x01]&=0xFD;			// disable PLL power down
	//writeb(readb(0xbf807501)&0xfd,0xbf807501);
	writeb(readb(0xbf806201)&0xfd,0xbf806201);

	writeb(readb(0xbf805c20)|0x01,0xbf805c20);	//HC Reset
	mdelay(1);
	//writeb(readb(0xbf807500)&0xfe,0xbf807500);//disable override mode for power down control
	writeb(readb(0xbf806200)&0xfe,0xbf806200);	//disable override mode for power down control
	//===========================================================================
	//writeb(readb(0xbf80750c)|0x44,0xbf80750c);//Force HS TX current enable and CDR stage select
	writeb(readb(0xbf80620c)|0x44,0xbf80620c); 	//Force HS TX current enable and CDR stage select
	//writeb(readb(0xbf80750c)|0x03,0xbf80750c);//reset UTMI
	writeb(readb(0xbf80620c)|0x03,0xbf80620c);	//reset UTMI
	//writeb(readb(0xbf80750c)&0xfc,0xbf80750c);
	writeb(readb(0xbf80620c)&0xfc,0xbf80620c);
	mdelay(1);
	//writeb(readb(0xbf807551)|0x08,0xbf807551);//disable full speed retime
	writeb(0x08,0xbf806251);					//disable full speed retime
	//writeb(0xa8,0xbf807505);
	writeb(0xa8,0xbf806205);
#if 1
	//XBYTE[UTMIBaseAddr+0x07]|=0x02;
	//writeb(readb(0xbf80750d)&0xfd,0xbf80750d);
	writeb(readb(0xbf80620d)&0xfd,0xbf80620d);
	//writeb(readb(0xbf807558)|0xc5,0xbf807558);
	writeb(readb(0xbf806258)|0xc1,0xbf806258);	//for triton
	//XBYTE[UTMIBaseAddr+0x2d]=0x3b;			//enable TX common mode,
	//writeb(0x3b,0xbf807559);
	writeb(0x3b,0xbf806259);
	//XBYTE[UTMIBaseAddr+0x2f]|=0x0e;			//preemsis
	//writeb(readb(0xbf80755d)|0x0e,0xbf80755d);
	writeb(readb(0xbf80625d)|0x0e,0xbf80625d);
#else
	//XBYTE[UTMIBaseAddr+0x2c]|=0x01;
	writeb(readb(0xbf807558)|0x01,0xbf807558);
	//XBYTE[UTMIBaseAddr+0x2d]=0x38;         	//disable TX common mode
	writeb(0x38,0xbf807559);
#endif
	//writeb(readb(0xbf807525)|0x02,0xbf807525);//ISI improvement
	writeb(readb(0xbf806225)|0x02,0xbf806225);	//ISI improvement
	//writeb(readb(0xbf807511)|0xe0,0xbf807511);//patch low tempture,FL meta issue and enable new FL RX engin
	writeb(readb(0xbf806211)|0xe0,0xbf806211);	//patch low tempture,FL meta issue and enable new FL RX engin
	//writeb(readb(0xbf80754d)&0xf3,0xbf80754d);
	writeb(readb(0xbf80624d)&0xf3,0xbf80624d);
	//writeb(readb(0xbf80754d)|0x08,0xbf80754d);
	writeb(readb(0xbf80624d)|0x08,0xbf80624d);
	//writeb(readb(0xbf807554)&0xf0,0xbf807554);
	writeb(readb(0xbf806254)&0xf0,0xbf806254);
	//writeb(readb(0xbf807529)|0x20,0xbf807529);
	writeb(readb(0xbf806229)|0x20,0xbf806229);
	//writeb(readb(0xbf804880)&~0x10,0xbf804880);
	writeb(readb(0xbf805c80)&~0x10,0xbf805c80);

	//writeb(readb(0xbf804868)|0x08,0xbf804868);
	writeb(readb(0xbf805c68)|0x08,0xbf805c68);
	//writeb(readb(0xbf804868)&0xfb,0xbf804868);
	writeb(readb(0xbf805c68)&0xfb,0xbf805c68);
#if 0
	writeb(readb(0xbf804868) & 0xf3,0xbf804868);
#endif
	writeb(readb(0xbf806211) | 0x08,0xbf806211);//anti dead lock for HS
	tmp=readb(0xbf806258);
	//===========================================================================
}
#else

/*
 * 이걸 실행하지 않으면, H.264 채널에서 부팅시 no-signal 발생함.
 */
void uranus_disable_power_down(void)
{
	writeb(0x00,(void*)0xbf207500);								//disable power down over write
	writeb(0x00,(void*)0xbf207501);								//disable power down over write high byte
}

static void uranus_start_ehc1(struct platform_device *dev)
{
	printk("uranus_start_ehc1 start\n");

#if (_USB_T3_U01_PATCH_EHCI == 1)
	writeb(readb((void*)0xbf000000+0x100b3c*2) & ~0x03, (void*) (0xbf000000+0x100b3c*2)); //select ddl_pll0
	writeb(0, (void*) (0xbf000000+0x100128*2)); //disable xiu timeout channel:uhc0 and uhc1
	writeb(0, (void*) (0xbf000000+0x100100*2)); //disable xiu time out mechanism
	writeb(0x02, (void*) (0xbf000000+0x10012c*2)); //set xiu pre time
	writeb(0x0A, (void*) (0xbf000000+0x10012d*2-1)); //set xiu recovery time
	writeb(readb((void*)0xbf000000+0x100125*2-1) | 0x10, (void*) (0xbf000000+0x100125*2-1)); //mask UHC0 xiu ack
	writeb(readb((void*)0xbf000000+0x100124*2) | 0x10, (void*) (0xbf000000+0x100124*2)); //mask UHC1 xiu ack
#endif

#if 0	//Duplication to the bootloader
	writeb(0x0a, (void*) (0xbf000000+0x100700*2)); // Disable MAC initial suspend, Reset UHC
	writeb(0x28, (void*) (0xbf000000+0x100700*2)); // Release UHC reset, enable UHC and OTG XIU function

	writeb(readb((void*)0xbf000000+(0x103a80+0x22)*2) | 0xE0, (void*) (0xbf000000+(0x103a80+0x22)*2)); // Set PLL_TEST[23:21] for enable 480MHz clock
	writeb(readb((void*)0xbf000000+(0x103a80+0x20)*2) | 0x02, (void*) (0xbf000000+(0x103a80+0x20)*2)); // Set PLL_TEST[1] for PLL multiplier 20X
	writeb(readb((void*)0xbf000000+(0x103a80+0x09)*2-1) & ~0x08, (void*) (0xbf000000+(0x103a80+0x09)*2-1)); // Disable force_pll_on
	writeb(readb((void*)0xbf000000+(0x103a80+0x08)*2) & ~0x80, (void*) (0xbf000000+(0x103a80+0x08)*2)); // Enable band-gap current
	writeb(0xC3, (void*) (0xbf000000+0x103a80*2)); // reg_pdn: bit<15>, bit <2> ref_pdn
	mdelay(1);	// delay 1ms

	writeb(0x69, (void*) (0xbf000000+(0x103a80+0x01)*2-1)); // Turn on UPLL, reg_pdn: bit<9>
	mdelay(2);	// delay 2ms

	writeb(0x01, (void*) (0xbf000000+0x103a80*2)); // Turn all (including hs_current) use override mode
	writeb(0, (void*) (0xbf000000+(0x103a80+0x01)*2-1)); // Turn on UPLL, reg_pdn: bit<9>
#endif

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
#if (_USB_T3_U01_PATCH_EHCI == 1)
	writeb(0, (void*) (0xbf000000+(0x102400+0x80)*2));
	writeb(readb((void*)0xbf000000+(0x102400+0x40)*2) & ~0x10, (void*) (0xbf000000+(0x102400+0x40)*2)); //0: VBUS On.
	mdelay(1);	// delay 1ms
	writeb(0, (void*) (0xbf000000+(0x102400+0x80)*2));
	writeb(readb((void*)0xbf000000+(0x102400+0x40)*2) & ~0x10, (void*) (0xbf000000+(0x102400+0x40)*2)); //0: VBUS On.
	mdelay(1);	// delay 1ms
	writeb(0, (void*) (0xbf000000+(0x102400+0x80)*2));
#endif

	writeb(readb((void*)0xbf000000+(0x102400+0x40)*2) | 0x08, (void*) (0xbf000000+(0x102400+0x40)*2)); // Active HIGH
	mdelay(1);	// delay 1ms
#if (_USB_T3_U01_PATCH_EHCI == 1)
	writeb(0, (void*) (0xbf000000+(0x102400+0x80)*2));
	writeb(readb((void*)0xbf000000+(0x102400+0x40)*2) | 0x08, (void*) (0xbf000000+(0x102400+0x40)*2)); // Active HIGH
	mdelay(1);	// delay 1ms
	writeb(0, (void*) (0xbf000000+(0x102400+0x80)*2));
	writeb(readb((void*)0xbf000000+(0x102400+0x40)*2) | 0x08, (void*) (0xbf000000+(0x102400+0x40)*2)); // Active HIGH
	mdelay(1);	// delay 1ms
	writeb(0, (void*) (0xbf000000+(0x102400+0x80)*2));
#endif

	writeb(readb((void*)0xbf000000+(0x103a80+0x06)*2) | 0x40, (void*) (0xbf000000+(0x103a80+0x06)*2)); //reg_tx_force_hs_current_enable

	writeb(readb((void*)0xbf000000+(0x103a80+0x03)*2-1) | 0x28, (void*) (0xbf000000+(0x103a80+0x03)*2-1)); //Disconnect window select
	writeb(readb((void*)0xbf000000+(0x103a80+0x03)*2-1) & 0xef, (void*) (0xbf000000+(0x103a80+0x03)*2-1)); //Disconnect window select

	writeb(readb((void*)0xbf000000+(0x103a80+0x07)*2-1) & 0xfd, (void*) (0xbf000000+(0x103a80+0x07)*2-1)); //Disable improved CDR
	writeb(readb((void*)0xbf000000+(0x103a80+0x09)*2-1) |0x81, (void*) (0xbf000000+(0x103a80+0x09)*2-1)); // UTMI RX anti-dead-loc, ISI effect improvement
	writeb(readb((void*)0xbf000000+(0x103a80+0x0b)*2-1) |0x80, (void*) (0xbf000000+(0x103a80+0x0b)*2-1)); // TX timing select latch path
	writeb(readb((void*)0xbf000000+(0x103a80+0x15)*2-1) |0x20, (void*) (0xbf000000+(0x103a80+0x15)*2-1)); // Chirp signal source select

	writeb(readb((void*)0xbf000000+(0x103a80+0x2c)*2) |0x10, (void*) (0xbf000000+(0x103a80+0x2c)*2));
	writeb(readb((void*)0xbf000000+(0x103a80+0x2d)*2-1) |0x02, (void*) (0xbf000000+(0x103a80+0x2d)*2-1));
	writeb(readb((void*)0xbf000000+(0x103a80+0x2f)*2-1) |0x81, (void*) (0xbf000000+(0x103a80+0x2f)*2-1));
}

//-------------------------------------------------------------------------------------------------------
//tony Second EHCI H.W Initial
void uranus_start_ehc2(struct platform_device *dev)
{
	printk("uranus_start_ehc2 start\n");

#if (_USB_T3_U01_PATCH_EHCI == 1)
	writeb(readb((void*)0xbf000000+0x100b3c*2) & ~0x03, (void*) (0xbf000000+0x100b3c*2)); //select ddl_pll0
	writeb(0, (void*) (0xbf000000+0x100128*2)); //disable xiu timeout channel:uhc0 and uhc1
	writeb(0, (void*) (0xbf000000+0x100100*2)); //disable xiu time out mechanism
	writeb(0x02, (void*) (0xbf000000+0x10012c*2)); //set xiu pre time
	writeb(0x0A, (void*) (0xbf000000+0x10012d*2-1)); //set xiu recovery time
	writeb(readb((void*)0xbf000000+0x100125*2-1) | 0x10, (void*) (0xbf000000+0x100125*2-1)); //mask UHC0 xiu ack
	writeb(readb((void*)0xbf000000+0x100124*2) | 0x10, (void*) (0xbf000000+0x100124*2)); //mask UHC1 xiu ack
#endif

#if 0	//Duplication to the bootloader
	writeb(0x0a, (void*) (0xbf000000+0x100780*2)); // Disable MAC initial suspend, Reset UHC
	writeb(0x28, (void*) (0xbf000000+0x100780*2)); // Release UHC reset, enable UHC and OTG XIU function

	writeb(readb((void*)0xbf000000+(0x103a00+0x22)*2) | 0xE0, (void*) (0xbf000000+(0x103a00+0x22)*2)); // Set PLL_TEST[23:21] for enable 480MHz clock
	writeb(readb((void*)0xbf000000+(0x103a00+0x20)*2) | 0x02, (void*) (0xbf000000+(0x103a00+0x20)*2)); // Set PLL_TEST[1] for PLL multiplier 20X
	writeb(readb((void*)0xbf000000+(0x103a00+0x09)*2-1) & ~0x08, (void*) (0xbf000000+(0x103a00+0x09)*2-1)); // Disable force_pll_on
	writeb(readb((void*)0xbf000000+(0x103a00+0x08)*2) & ~0x80, (void*) (0xbf000000+(0x103a00+0x08)*2)); // Enable band-gap current
	writeb(0xC3, (void*) (0xbf000000+0x103a00*2)); // reg_pdn: bit<15>, bit <2> ref_pdn
	mdelay(1);	// delay 1ms

	writeb(0x69, (void*) (0xbf000000+(0x103a00+0x01)*2-1)); // Turn on UPLL, reg_pdn: bit<9>
	mdelay(2);	// delay 2ms

	writeb(0x01, (void*) (0xbf000000+0x103a00*2)); // Turn all (including hs_current) use override mode
	writeb(0, (void*) (0xbf000000+(0x103a00+0x01)*2-1)); // Turn on UPLL, reg_pdn: bit<9>
#endif

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
#if (_USB_T3_U01_PATCH_EHCI == 1)
	writeb(0, (void*) (0xbf000000+(0x100D00+0x80)*2));
	writeb(readb((void*)0xbf000000+(0x100D00+0x40)*2) & ~0x10, (void*) (0xbf000000+(0x100D00+0x40)*2)); //0: VBUS On.
	mdelay(1);	// delay 1ms
	writeb(0, (void*) (0xbf000000+(0x100D00+0x80)*2));
	writeb(readb((void*)0xbf000000+(0x100D00+0x40)*2) & ~0x10, (void*) (0xbf000000+(0x100D00+0x40)*2)); //0: VBUS On.
	mdelay(1);	// delay 1ms
	writeb(0, (void*) (0xbf000000+(0x100D00+0x80)*2));
#endif

	writeb(readb((void*)0xbf000000+(0x100D00+0x40)*2) | 0x08, (void*) (0xbf000000+(0x100D00+0x40)*2)); // Active HIGH
	mdelay(1);	// delay 1ms
#if (_USB_T3_U01_PATCH_EHCI == 1)
	writeb(0, (void*) (0xbf000000+(0x100D00+0x80)*2));
	writeb(readb((void*)0xbf000000+(0x100D00+0x40)*2) | 0x08, (void*) (0xbf000000+(0x100D00+0x40)*2)); // Active HIGH
	mdelay(1);	// delay 1ms
	writeb(0, (void*) (0xbf000000+(0x100D00+0x80)*2));
	writeb(readb((void*)0xbf000000+(0x100D00+0x40)*2) | 0x08, (void*) (0xbf000000+(0x100D00+0x40)*2)); // Active HIGH
	mdelay(1);	// delay 1ms
	writeb(0, (void*) (0xbf000000+(0x100D00+0x80)*2));
#endif
	writeb(readb((void*)0xbf000000+(0x103a00+0x06)*2) | 0x40, (void*) (0xbf000000+(0x103a00+0x06)*2)); //reg_tx_force_hs_current_enable

	writeb(readb((void*)0xbf000000+(0x103a00+0x03)*2-1) | 0x28, (void*) (0xbf000000+(0x103a00+0x03)*2-1)); //Disconnect window select
	writeb(readb((void*)0xbf000000+(0x103a00+0x03)*2-1) & 0xef, (void*) (0xbf000000+(0x103a00+0x03)*2-1)); //Disconnect window select

	writeb(readb((void*)0xbf000000+(0x103a00+0x07)*2-1) & 0xfd, (void*) (0xbf000000+(0x103a00+0x07)*2-1)); //Disable improved CDR
	writeb(readb((void*)0xbf000000+(0x103a00+0x09)*2-1) |0x81, (void*) (0xbf000000+(0x103a00+0x09)*2-1)); // UTMI RX anti-dead-loc, ISI effect improvement
	writeb(readb((void*)0xbf000000+(0x103a00+0x0b)*2-1) |0x80, (void*) (0xbf000000+(0x103a00+0x0b)*2-1)); // TX timing select latch path
	writeb(readb((void*)0xbf000000+(0x103a00+0x15)*2-1) |0x20, (void*) (0xbf000000+(0x103a00+0x15)*2-1)); // Chirp signal source select

	writeb(readb((void*)0xbf000000+(0x103a00+0x2c)*2) |0x10, (void*) (0xbf000000+(0x103a00+0x2c)*2));
	writeb(readb((void*)0xbf000000+(0x103a00+0x2d)*2-1) |0x02, (void*) (0xbf000000+(0x103a00+0x2d)*2-1));
	writeb(readb((void*)0xbf000000+(0x103a00+0x2f)*2-1) |0x81, (void*) (0xbf000000+(0x103a00+0x2f)*2-1));
}
#endif

static void uranus_stop_ehc(struct platform_device *dev)
{

}

/*-------------------------------------------------------------------------*/

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */

/**
 * usb_ehci_au1xxx_probe - initialize Au1xxx-based HCDs
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 */
int usb_ehci_mstar_probe(const struct hc_driver *driver,
		struct usb_hcd **hcd_out, struct platform_device *dev)
{
	int retval;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;

	//tony---------------------------------------
	if( 0==memcmp(dev->name,"Mstar-soc-ehci",14) )
	{
		printk("Mstar-soc-ehci H.W init\n");
		uranus_start_ehc1(dev);
	}
	else if( 0==memcmp(dev->name,"Second-Mstar-soc-ehci",21) )
	{
		printk("Second-Mstar-soc-ehci H.W init\n");
		uranus_start_ehc2(dev);
	}
	//-------------------------------------------
	//
	if (dev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
	}
	hcd = usb_create_hcd(driver, &dev->dev, "mstar");
	if (!hcd)
		return -ENOMEM;
	hcd->rsrc_start = dev->resource[1].start;
	hcd->rsrc_len = dev->resource[1].end - dev->resource[1].start + 0;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		pr_debug("request_mem_region failed");
		retval = -EBUSY;
		goto err1;
	}

	//hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);	//tony
	hcd->regs = (void *)(u32)(hcd->rsrc_start);	        	//tony
	if (!hcd->regs) {
		pr_debug("ioremap failed");
		retval = -ENOMEM;
		goto err2;
	}

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	//ehci->regs = hcd->regs + HC_LENGTH(readl(&ehci->caps->hc_capbase));
#if (_USB_T3_U01_PATCH_EHCI == 1)
	ehci->regs = hcd->regs + 0x10; //USB T3_U01 patch
#else
	ehci->regs = hcd->regs + HC_LENGTH(ehci_readl(0, &ehci->caps->hc_capbase));
#endif

	//printk("\nDean: [%s] ehci->regs: 0x%x\n", __FILE__, (unsigned int)ehci->regs);
	/* cache this readonly data; minimize chip reads */
#if (_USB_T3_U01_PATCH_EHCI == 1)
	ehci->hcs_params = (ehci_readl(0, &ehci->caps->hcs_params) & 0xF0) | 1; //USB T3_U01 patch
#else
	ehci->hcs_params = ehci_readl(0, &ehci->caps->hcs_params);
#endif

	/* ehci_hcd_init(hcd_to_ehci(hcd)); */
	retval = usb_add_hcd(hcd, dev->resource[2].start, SA_INTERRUPT);//tony
	//usb_add_hcd(hcd, dev->resource[2].start, IRQF_DISABLED | IRQF_SHARED);
	if (retval == 0)
		return retval;

	uranus_stop_ehc(dev);
	//iounmap(hcd->regs);		//tony
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err1:
	usb_put_hcd(hcd);
	return retval;
}

/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

/**
 * usb_ehci_hcd_au1xxx_remove - shutdown processing for Au1xxx-based HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_ehci_hcd_au1xxx_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 *
 */
void usb_ehci_mstar_remove(struct usb_hcd *hcd, struct platform_device *dev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
	uranus_stop_ehc(dev);
}

/*-------------------------------------------------------------------------*/

static int ehci_hcd_mstar_drv_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd = NULL;
	int ret;

	pr_debug("In ehci_hcd_mstar_drv_probe\n");

	if (usb_disabled())
		return -ENODEV;

	/* FIXME we only want one one probe() not two */
	ret = usb_ehci_mstar_probe(&ehci_mstar_hc_driver, &hcd, pdev);
	return ret;
}

static int ehci_hcd_mstar_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	/* FIXME we only want one one remove() not two */
	usb_ehci_mstar_remove(hcd, pdev);
	return 0;
}

/*-------------------------------------------------------------------------*/

static struct platform_driver ehci_hcd_mstar_driver = {
	.probe 		= ehci_hcd_mstar_drv_probe,
	.remove 	= ehci_hcd_mstar_drv_remove,
//	.suspend	= ehci_hcd_uranus_drv_suspend,
//	.resume		= ehci_hcd_uranus_drv_resume,
	.driver = {
		.name	= "Mstar-soc-ehci",
//		.bus	= &platform_bus_type,
	}
};


static struct platform_driver second_ehci_hcd_mstar_driver = {
	.probe 		= ehci_hcd_mstar_drv_probe,
	.remove 	= ehci_hcd_mstar_drv_remove,
//	.suspend	= ehci_hcd_uranus_drv_suspend,
//	.resume		= ehci_hcd_uranus_drv_resume,
	.driver = {
		.name 	= "Second-Mstar-soc-ehci",
//		.bus	= &platform_bus_type,
	}
};
