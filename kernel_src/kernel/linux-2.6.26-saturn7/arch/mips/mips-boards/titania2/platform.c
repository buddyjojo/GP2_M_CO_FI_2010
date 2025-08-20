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

//#include <linux/config.h>
#include <linux/autoconf.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/platform_device.h>

#include "chip_int.h"

#define UTMI_BASE_ADDRESS_START 0xbf807500
#define UTMI_BASE_ADDRESS_END 0xbf8075FC
#define USB_HOST20_ADDRESS_START 0xbf804800
#define USB_HOST20_ADDRESS_END 0xbf8049FC
//tony add for 2st EHCI
#define SECOND_UTMI_BASE_ADDRESS_START 0xbf807400
#define SECOND_UTMI_BASE_ADDRESS_END 0xbf8074FC
#define SECOND_USB_HOST20_ADDRESS_START 0xbf801A00
#define SECOND_USB_HOST20_ADDRESS_END 0xbf801BFC

static struct resource Mstar_usb_ehci_resources[] = {
	[0] = {
		.start		= UTMI_BASE_ADDRESS_START,
		.end		= UTMI_BASE_ADDRESS_START,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= USB_HOST20_ADDRESS_START,
		.end		= USB_HOST20_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= E_IRQ_UHC,
		.end		= E_IRQ_UHC,
		.flags		= IORESOURCE_IRQ,
	},
};
//tony add for 2st EHCI
static struct resource Second_Mstar_usb_ehci_resources[] = {
	[0] = {
		.start		= SECOND_UTMI_BASE_ADDRESS_START,
		.end		= SECOND_UTMI_BASE_ADDRESS_START,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= SECOND_USB_HOST20_ADDRESS_START,
		.end		= SECOND_USB_HOST20_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= E_IRQ_SECOND_UHC,
		.end		= E_IRQ_SECOND_UHC,
		.flags		= IORESOURCE_IRQ,
	},
};

/* The dmamask must be set for EHCI to work */
//static u64 ehci_dmamask = ~(u32)0;
//dhjung LGE
#if	( defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	|| \
	  defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1) )
static u64 ehci_dmamask = 0x3ffffff;	//tony add for limit DMA range
#else
static u64 ehci_dmamask = 0x7ffffff;	//tony add for limit DMA range
#endif

static struct platform_device Mstar_usb_ehci_device = {
	.name		= "Mstar-soc-ehci",
	.id		= 0,
	.dev = {
		.dma_mask			= &ehci_dmamask,
#if	( defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	|| \
	  defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1) )
		.coherent_dma_mask	= 0x3ffffff, //tony add for limit DMA range
#else
		.coherent_dma_mask	= 0x7ffffff, //tony add for limit DMA range
#endif
	},
	.num_resources	= ARRAY_SIZE(Mstar_usb_ehci_resources),
	.resource	= Mstar_usb_ehci_resources,
};
//tony add for 2st EHCI
static struct platform_device Second_Mstar_usb_ehci_device = {
	.name		= "Second-Mstar-soc-ehci",
	.id		= 1,
	.dev = {
		.dma_mask			= &ehci_dmamask,
#if	( defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	|| \
	  defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1) )
		.coherent_dma_mask	= 0x3ffffff, //tony add for limit DMA range
#else
		.coherent_dma_mask	= 0x7ffffff, //tony add for limit DMA range
#endif
	},
	.num_resources	= ARRAY_SIZE(Second_Mstar_usb_ehci_resources),
	.resource	= Second_Mstar_usb_ehci_resources,
};

//static struct platform_device *Mstar_platform_devices[] __initdata = {
static struct platform_device *Mstar_platform_devices[] = {
	&Mstar_usb_ehci_device,
	&Second_Mstar_usb_ehci_device,	 //tony add for 2st EHCI
};

int Mstar_platform_init(void)
{
	return platform_add_devices(Mstar_platform_devices, ARRAY_SIZE(Mstar_platform_devices));
}

arch_initcall(Mstar_platform_init);
