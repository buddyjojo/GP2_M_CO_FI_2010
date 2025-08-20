/******************************************************************************

	LGE. DTV RESEARCH LABORATORY
	COPYRIGHT(c) LGE CO.,LTD. 2006. SEOUL, KOREA.
	All rights are reserved.

******************************************************************************/

/** @file nvm.h
 *
 *	This c file defines the DDI functions related to MICOM Interface
 *
 *	@author 	redcloud00(redcloud00@lge.com)
 *	@version	0.1
 *	@date	2009.07.24
 *	@note
 *	@see
 */


#ifndef _MICOM_H_
#define _MICOM_H_
/*
#include <asm/types.h>
#include "../drivers/titania_hw_i2c.h"
*/
#include "mdrv_types.h"


#undef  INCLUDE_INTERNAL_MICOM_CASE

/*********************************************************************
	매크로 상수 정의(Macro Definitions)
**********************************************************************/
extern	S32 MDrv_SW_IIC_Read(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf);
extern	S32 MDrv_SW_IIC_Write(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf);


// ============= MICOM - CPU Communication Protocols ================================

// ============= MICOM Write Protocols =================
#ifdef INCLUDE_INTERNAL_MICOM_CASE
#define		CP_WRITE_POWER_CONTROL				0x26
#else
#define		CP_WRITE_POWER_CONTROL				0x25
#endif
#define		CP_WRITE_EEPROM_READ_READY			0x28
#define		CP_WRITE_EEPROM_WRITE				0x29

#define		CP_WRITE_INVERT_OR_VAVS_ON			0x36
#define		CP_WRITE_INVERT_OR_VAVS_OFF			0x37
#define		CP_WRITE_PANEL_ONOFF				0x3A

#define		CP_WRITE_DISP_ENABLE_ON				0x3E
#define		CP_WRITE_DISP_ENABLE_OFF			0x3F


// ============= MICOM Read Protocols =================
#define		CP_READ_RTC_YMDHMS					0x80
#define		CP_READ_POWERON_MODE				0x9A
#define		CP_READ_MICOM_VERSION				0xA1
#define		CP_READ_MICOM_HWOPTION				0xA6

#define		CP_READ_MICOM_INV_ONOFF				0xD0
#define		CP_READ_MICOM_PANEL_ONOFF			0xD1





/******************************************************************************
    타입  정의    (Type Definitions)
******************************************************************************/

typedef struct{
	int year;   /* years since 1900 */
	int month;  /* months since January [0, 11] */
	int day;    /* day of the month [1, 31] */
	int hour;   /* hours since midnight [0, 23] */
	int minute; /* minutes after the hour [0, 59] */
	int second; /* seconds after the minute [0, 59] */
}TIME_T;

typedef	struct
{
	U8	number[6];	/**< version number	*/
}	MICOM_VERSION_T;

/******************************************************************************
    매크로 함수 정의    (Macro Definitions)
******************************************************************************/

#define EXT_MICOM_DEVID				0x52
#define EXT_MICOM_CH				7

#ifndef	INCLUDE_INTERNAL_MICOM_CASE
#define MICOM_READ_COMMAND( cmd, size, pIn )	I2C_MICOM_READ (EXT_MICOM_CH, EXT_MICOM_DEVID, &cmd, (size), (pIn) , 3)
#define	MICOM_WRITE_COMMAND( pOut, size )   	I2C_MICOM_WRITE(EXT_MICOM_CH, EXT_MICOM_DEVID,    0, (size), (pOut), 3)
#else
#define MICOM_READ_COMMAND( cmd, size, pIn )	DDI_UCOM_ReadCommand ( cmd, (size), (pIn) )
#define	MICOM_WRITE_COMMAND( pOut, size )   	DDI_UCOM_WriteCommand( (pOut), (size) )
#endif

#define	I2C_MICOM_READ(a, b, c, d, e, f)		MDrv_SW_IIC_Read(a, b, 1, c, d, e)
#define	I2C_MICOM_WRITE(a, b, c, d, e, f)		MDrv_SW_IIC_Write(a, b, 0, 0, d, e) 

extern S32 MICOM_GetVersion (MICOM_VERSION_T *pVersion);
extern S32 MICOM_GetRTCTime (TIME_T *pTime);
extern S32 MICOM_TurnOnInv(void);
extern S32 MICOM_TurnOnPanelOn(void);
extern S32 MICOM_GetHWoption (U8 *pHWption);
extern S32	MICOM_VerifyInverterOn(U8 *pRData);
extern S32	MICOM_VerifyPanelOn(U8 *pRData);


#endif  /* End of _MICOM_H_ */
