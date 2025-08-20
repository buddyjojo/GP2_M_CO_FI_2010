#ifndef _MADP_MFC_SPI_H_
#define _MADP_MFC_SPI_H_

#include "mdrv_types.h"
//#define MFC_IIC_CHANNEL_ID 0x02
#define _BIT0			(0x0001)
#define _BIT1			(0x0002)
#define _BIT2			(0x0004)
#define _BIT3			(0x0008)
#define _BIT4			(0x0010)
#define _BIT5			(0x0020)
#define _BIT6			(0x0040)
#define _BIT7			(0x0080)
#define _BIT8			(0x0100)
#define _BIT9			(0x0200)
#define _BIT10			(0x0400)
#define _BIT11			(0x0800)
#define _BIT12			(0x1000)
#define _BIT13			(0x2000)
#define _BIT14			(0x4000)
#define _BIT15			(0x8000)

//------------------------------------------------------------------------------
// SPI Definition
//------------------------------------------------------------------------------
#define MFC_SPI_SLAVE_ADDR 	0x94
enum
{
	SpiEnter = 0x23,
	SpiReset = 0x24,
};

// Status register[7:0]
#define RDY    _BIT0
#define WEN    _BIT1
#define BP0    _BIT2
#define BP1    _BIT3
#define AAI    _BIT6
#define WPEN   _BIT7

enum
{
	FV_ATMEL = 0x1F,
	FV_ST    = 0x20,
	FV_PMC   = 0x9D,
	FV_SST   = 0xBF,
	FV_MXIC  = 0xC2,
	FV_NUM   = 0xFF
};

typedef struct
{
  U8  ManufacturerID;
  U8  ProductID;

  U16 ProgramCKS; // check sum
  U16 VerifyCKS; // check sum
} _FLASHTYPE;

U8 spiRDSR(void);
void spiWaitReady(U8 u8Flag);
void spiWREN(BOOL bCtrl); // Set write enable latch
void spiEWSR(void);
void spiWRSR(U8 u8ID, U8 u8Cmd);
void spiRDID(_FLASHTYPE* Flash);
void spiBlockErase(U8 u8ID, U16 wPage, U8 ucAddr);
U16 spiAAIWriteByte(U16 wPage, U8 u8Start, U16 u16Len, U8 *pBuf);
U16 spiPageWriteByte(U16 wPage, U8 u8Start, U8 u8End, U8 *pBuf);
void spiPageReadByte(U16 wPage, U8 ucStart, U8 ucEnd, U8 *pBuf);

#endif
