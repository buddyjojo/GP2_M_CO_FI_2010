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
/// @file   usb_lan.c
/// @brief  usb lan Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <common.h>

#include <net.h>
#include "usb.h"
#include "usb_lan.h"

extern int rtl8150_init(struct eth_device *nic);
extern int pegasus_init(struct eth_device *nic);

struct eth_device *gnic = NULL;
typedef int  (*__entry) (struct eth_device*);

struct init_info {
	unsigned short idVendor;
	unsigned short idProduct;
	__entry init;
};

struct init_info ctrlTbl[] =
{
	{VENDOR_ID_REALTEK, PRODUCT_ID_RTL8150, rtl8150_init}
	,{VENDOR_ID_ADMTEK, PRODUCT_ID_ADM8515, pegasus_init}
};

#define USBLAN_MAX_INFOTBL  (sizeof(ctrlTbl)/sizeof(struct init_info))

static int lookup_ctrlTbl(u16 idVendor, u16 idProduct)
{
	u8 idx = 0;

	for(; idx < USBLAN_MAX_INFOTBL; idx++)
	{
		if (idVendor == ctrlTbl[idx].idVendor
				&& idProduct == ctrlTbl[idx].idProduct)
			break;
	}
	return idx;
}

void usb_lan_release(void)
{
	if (gnic) {
		free(gnic);
	}
}

/**************************************************************************
  PROBE - Look for an adapter, this routine's visible to the outside
  You should omit the last argument struct pci_device * for a non-PCI NIC
 ***************************************************************************/
int
usb_lan_initialize(struct usb_device *udev)
{
	struct usb_lan_hw *hw = NULL;
	struct eth_device *nic = NULL;

	DEBUGFUNC();

	nic = (struct eth_device *) malloc(sizeof (struct eth_device));
	if (!nic) {
		return -ENOMEM;
	}
	hw = (struct usb_lan_hw *) malloc(sizeof (struct usb_lan_hw));
	if (!hw) {
		return -ENOMEM;
	}

	gnic = nic;
	nic->udev = udev;
	nic->priv = hw;

	//dhjung LGE
	//printf("vendor:%x product:%x\n",nic->udev->descriptor.idVendor, nic->udev->descriptor.idProduct);
	hw->tblIdx = lookup_ctrlTbl(nic->udev->descriptor.idVendor, nic->udev->descriptor.idProduct);
	if (USBLAN_MAX_INFOTBL == hw->tblIdx)
	{
		printf("Can't find usb lan dev!!\n");
		return -ENXIO;
	}

	USB_LAN_DBG("Go to fxp:%x\n", ctrlTbl[hw->tblIdx].init);
	//printf cause fxp crash
	//ctrlTbl[hw->tblIdx].init(nic);
	if (hw->tblIdx)
		pegasus_init(nic);
	else
		rtl8150_init(nic);

	eth_register(nic);

	uEMAC_start = 0;		//2008/12/05 Nick
	uUSB_LAN_start = 1;		//2008/12/05 Nick

	return 1;
}
