#ifndef EHCI_USB_H
#define EHCI_USB_H

#define USB_DELAY(_msec)		wait_ms(_msec)
#define KSEG02KSEG1(addr)		((void *)((UINT32)(addr)|0x20000000))  //cached -> unchched
#define KSEG12KSEG0(addr)		((void *)((UINT32)(addr)&~0x20000000)) //unchched -> cached

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS   		0x0080
#define RH_CLEAR_FEATURE		0x0100
#define RH_SET_FEATURE  		0x0300
#define RH_SET_ADDRESS			0x0500
#define RH_GET_DESCRIPTOR		0x0680
#define RH_SET_DESCRIPTOR       0x0700
#define RH_GET_CONFIGURATION	0x0880
#define RH_SET_CONFIGURATION	0x0900
#define RH_GET_STATE            0x0280
#define RH_GET_INTERFACE        0x0A80
#define RH_SET_INTERFACE        0x0B00
#define RH_SYNC_FRAME           0x0C80

#endif
