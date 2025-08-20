//by redcloud00
//#include <common.h>
//#include <command.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <drv_extmicom.h>

S32	MICOM_GetVersion (MICOM_VERSION_T *pVersion)
{
	S32				retVal;
	U8			cmd;
	U8			data[7];

	cmd = CP_READ_MICOM_VERSION;

	retVal = MICOM_READ_COMMAND ( cmd, 7, data );
	if (retVal > 0)
	{
		printk("MICOM_GetVersion \n");
		printk("data[0] = %d, data[1] = %d, data[2] = %d, data[3] = %d, data[4] = %d \n",data[0],data[1],data[2],data[3],data[4]);
		pVersion->number[0] = data[0];
		pVersion->number[1] = data[1];
		pVersion->number[2] = data[2];
		pVersion->number[3] = data[3];
		pVersion->number[4] = data[4];
		pVersion->number[5] = data[5];
		pVersion->number[6] = data[6];
		pVersion->number[7] = 0;
	}

	return( retVal );
}


S32 MICOM_GetHWoption (U8 *pHWption)
{
	S32			retVal;
	U8		cmd;
	U8		data[1];

	/* Send HWoption read command */
	cmd = CP_READ_MICOM_HWOPTION;

	retVal = MICOM_READ_COMMAND ( cmd, 1, data );
	if(retVal > 0)
	{
		*pHWption = data[0];
		//printf("pHWption = 0x%x \n",*pHWption);
	}

	return( retVal );
}

S32 MICOM_GetRTCTime (TIME_T *pTime)
{
	S32			retVal;
	U8		cmd;
	U8		data[8];

	/* Send RTC read command */
	cmd = CP_READ_RTC_YMDHMS;

	retVal = MICOM_READ_COMMAND ( cmd, 7, data );
	if(retVal > 0)
	{
		pTime->year		= (U16)data[0]*100 + data[1];
		pTime->month  	= data[2];
		pTime->day	 	= data[3];
		pTime->hour	 	= data[4];
		pTime->minute 	= data[5];
		pTime->second 	= data[6];

		if (pTime->hour == 24)
		{
			/* Host에서는 MICOM과 달리 24시 대신 0시를 사용함 */
			pTime->hour = 0;
		}
	}

	return( retVal );
}

S32	MICOM_TurnOnInv(void)
{
	S32		retVal;
	U8	data[1];

	data[0] = CP_WRITE_INVERT_OR_VAVS_ON;
	retVal = MICOM_WRITE_COMMAND ( (U8*)&data, 1);

	return( retVal );
}

S32	MICOM_VerifyInverterOn(U8 *pRData)
{
	S32			retVal;
	U8		cmd;
	U8		data[1];

	/* Send Verity Inv On read command */
	cmd = CP_READ_MICOM_INV_ONOFF;

	retVal = MICOM_READ_COMMAND ( cmd, 1, data );
	if(retVal > 0)
	{
		*pRData = data[0];
		//printf("Inv On response = 0x%x \n",*pRData);
	}

	return( retVal );
}

S32	MICOM_TurnOnPanelOn(void)
{
	S32		retVal;
	U8	data[2];
	
	data[0] = CP_WRITE_PANEL_ONOFF;
	data[1] = 1; //panel on
	retVal = MICOM_WRITE_COMMAND ( (U8*)&data, 2);

	return( retVal );
}

S32	MICOM_VerifyPanelOn(U8 *pRData)
{
	S32			retVal;
	U8		cmd;
	U8		data[1];

	/* Send Verity Panel On read command */
	cmd = CP_READ_MICOM_PANEL_ONOFF;

	retVal = MICOM_READ_COMMAND ( cmd, 1, data );
	if(retVal > 0)
	{
		*pRData = data[0];
		//printf("Panel On response = 0x%x \n",*pRData);
	}

	return( retVal );
}


