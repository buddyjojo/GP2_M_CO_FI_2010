//by dhjung LGE
#include <malloc.h>
#include <common.h>
#include <command.h>
#include <asm/types.h>
#include <nvm.h>

#define	DEBUG_INVALID_COMMAND	(0xFFFFFFFFUL)


#define DEFAULT_PRINT_MASK		{					\
	0,											\
	{ {{0,},0}, {{0,},0}, {{0,},0}, {{0,},0},	\
		{{0,},0}, {{0,},0}, {{0,},0}, {{0,},0}	\
	},											\
}
/******************************************************************************
  ���� �� ���� (Global Type Definitions)
 ******************************************************************************/

/** System DB Default Value ���� �� global ���� ���� */
SYS_DB_T gSysNvmDB =
{
	0,
	DEFAULT_PRINT_MASK,
	0,		/* PDP panel use time */

	0xFF,	/* internal micom: powerOnMode 	*/
	0xFF,	/* internal micom: powerState 	*/
	0,		/* internal micom: powerOnStatus(LST)  	*/
	1,		/* internal micom: set id  	*/
	0,		/* internal micom: key operating mode(key locked) */
	0,		/* internal micom: irLocked 	(FALSE) */
	0,		/* internal micom: localLocked	(FALSE) */
	0,		/* internal micom: reserverd */
	0,		/* internal micom: debug counter */
	0,		/* FRC auto download or not */
	{50, 33, 33, 50, 50, 25, 50, 50, 50, 50}, /* panel powerseq */
	{0, 0x80, 0xff, 20, 100, 10, 35,120,100}, /* panel pwm */
	0,
	1,
	1,
	0,		/*Vcom Pgamma value checksum */
};

TOOLOPTION_DB_T gToolOptionDB =
{
	4,
	{(INCH_42<<12)|(TOOL_LD45<<5)|(MODULE_LGD<<2)|(0<<1)|0},
	{0},
	{56454},
	{0},
	{0},
	{0},	/* Area Option */
	{'G', 'L', 'O', 'B' ,'A' ,'L', '-' ,'P' , 'L', 'A', 'T', '2', '\0'}	//090212 odrie20 Model Name�� eeprom�� �����ϰ� �ʱ� data�� inch, tool�� �������� ���鵵�� ��
};

static int			bDebugStatusRead;

typedef enum{
	FRC_OPT_SEL,
	FRC_OPT1_SEL,
	LVDS_OPT_SEL,
	DDRSIZE_OPT_SEL,
	PANEL_RES_OPT_SEL,
	GIP_OPT_SEL,
	OLED_SEL,
	MICOM_HW_OPT_SEL,
}HW_OPT_T;


/*==============================================================================
  External function prototypes
  ==============================================================================*/

/*==============================================================================
  Local Functions
  ==============================================================================*/
static UINT32	_NVM_CheckState (UINT16 offset, UINT8 cData);
static int		_NVM_ReadNvram (UINT32 offset, UINT32 nData, UINT8 *pRxBuf);
static int		_NVM_WriteNvram (UINT32 offset, UINT32 nData, UINT8 *pTxBuf);
static int		_dbgHexDecimalInput (const char *pStr, unsigned int *pVal);
#ifdef _NOT_USED_
static int		_dbgDecimalInput (const char *pStr, unsigned int *pVal);
#endif
static void		_NVM_PrintMap (void);

/******************************************************************************
  Hex Decimal Input
 ******************************************************************************/
static int _dbgHexDecimalInput (const char *pStr, unsigned int *pVal)
{
	char inStr[128];

	printf ("%s: 0x", pStr);
	gets (&inStr[0]);

	*pVal = simple_strtoul (inStr, (char **) NULL, 16);

	printf ("\n");

	return 0;
}

#ifdef _NOT_USED_
/******************************************************************************
  Decimal Input
 ******************************************************************************/
static int _dbgDecimalInput (const char *pStr, unsigned int *pVal)
{
	char inStr[128];

	printf ("%s: ", pStr);
	gets (&inStr[0]);

	*pVal = simple_strtoul(inStr, (char**)NULL, 10);

	printf ("\n");

	return 0;
}
#endif

static void _NVM_PrintMap (void)
{
	printf("\n\n");
	printf("================================================\n");
	printf("NVRAM Size: %d bytes\n", NVRAM_SIZE);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"SYS"   ,	SYS_DB_BASE       	,	SYS_DB_SIZE       	,	SYS_DB_SIZE 	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"ANA"   ,	ANA_DB_BASE        	,	ANA_DB_SIZE        	,	ANA_DB_SIZE  	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"TOOL"	,	TOOL_OPTION_DB_BASE ,	TOOL_OPTION_DB_SIZE ,	TOOL_OPTION_DB_SIZE  );
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"FACTO" ,	FACTORY_DB_BASE     ,	FACTORY_DB_SIZE     ,	FACTORY_DB_SIZE 	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"UI"    ,	UI_DB_BASE        	,	UI_DB_SIZE        	,	UI_DB_SIZE  	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"UI EX" ,	UI_EXPERT_DB_BASE   ,	UI_EXPERT_DB_SIZE   ,	UI_EXPERT_DB_SIZE	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"CH" 	,	CH_DB_BASE     		,	CH_DB_SIZE     		,	CH_DB_SIZE	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"BT"    ,	BT_DB_BASE        	,	BT_DB_SIZE        	,	BT_DB_SIZE  	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"EMP" 	,	EMP_DB_BASE     	,	EMP_DB_SIZE     	,	EMP_DB_SIZE	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"ACAP" 	,	ACAP_DB_BASE     	,	ACAP_DB_SIZE     	,	ACAP_DB_SIZE	);
//	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"THX" 	,	THX_DB_BASE     	,	THX_DB_SIZE     	,	THX_DB_SIZE	);
//	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"MICOM" ,	MICOM_DB_BASE     	,	MICOM_DB_SIZE     	,	MICOM_DB_SIZE	);
//	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"FREE" 	,	FREE_DB_BASE     	,	FREE_DB_SIZE     	,	FREE_DB_SIZE	);
	printf("%6s: offset(0x%05x) size(%5d/0x%04x)\n",	"FREE" 	,	TOTAL_DB_SIZE     	,	NVMDRV_TOTAL_SIZE	- 1     ,	NVMDRV_TOTAL_SIZE	- 1	);

	printf("================================================\n");
}

static UINT32 _NVM_CheckState (UINT16 offset, UINT8 cData)
{
	/*==================================================================================================
	  Chattering test�� Ch.�� ��ȯ�Ǵ� ������ ���� (LCD PVR ������ ������ ����)

	  ��   ��
	  EEPROM�� data�� write�� ���, EEPROM�� write cycle time(max. 5ms) ���� CPU�� read/write ��û�� �������� �ʴ´�.
	  �׷��� ���� code structure������ EEPROM�� I2C write ���� 20usec ������ delay ���� read �Ǵ� write operation�� �ϵ��� �Ǿ� ����.
	  Power ON sequence ���� EEPROM�� write operation ���� �ٷ� read operation�� request�� ���,
	  no ACK �� EEPROM�� SCL�� low state�� ����δ� ������ �߻��Ͽ�, ���� read operation�� error�� �߻���.
	  �̷��� error�� Channel map �� current channel data�� loading(read from EEPROM)�ϴ� operation�� ������ �� �߻��Ѵٸ�, ���� ���� ������ �߻���.

	  ��   å
	  ���� EEPROM write ������ [data write ? write�� data�� �ٽ� read ? read fail�� �߻��� ��� �ٽ� write]�ϴ� algorithm�� �����ϰ� ����.
	  1���� write�� data�� �ٽ� read�Ͽ� Ȯ���ϴ� ������ �����ϰ�, 20usec�� delay ���� I2C read/write operation�� �� ��� no ACK �� SCL low state ������ �߻���.
	  2���� 300 usec�� delay ���� I2C read/write operation�� �� ��� �������� ����.
	  EEPROM(24C512)�� datasheet �� ���� write cycle time(5msec)���� ū 6.5msec�� delay ���� I2C read/write operation�� �� ���
	  no ACK �� SCL low ������ �߻����� �ʰ� ������.
	  ==================================================================================================*/

	udelay(7000); /* 7ms delay */

	return 0;
}

static int _NVM_ReadNvram  (UINT32 offset, UINT32 nData, UINT8 *pRxBuf)
{
	UINT32	retVal;
	UINT32	i;

	UINT32	tmpVal;
	UINT8	subAddr[2];

	UINT16	headPageOffset;
	UINT32	headPageLength;
	UINT8	*pHeadPageRxBuf;

	UINT16	midPageOffset;
	UINT32	midPageLength;
	UINT32	midPageIterate;
	UINT8	*pMidPageRxBuf;

	UINT16	tailPageOffset;
	UINT32	tailPageLength;
	UINT8	*pTailPageRxBuf;

	/*------------------------------------------------------------
	 *	Receive Data from MVRAM
	 -------------------------------------------------------------*/

	/* Head Page Constants */
	headPageOffset = offset;
	tmpVal = (NVMDRV_PAGE_SIZE - (offset % NVMDRV_PAGE_SIZE)) % NVMDRV_PAGE_SIZE;
	if (tmpVal > nData) {
		headPageLength = nData;
	} else {
		headPageLength = tmpVal;
	}
	pHeadPageRxBuf = pRxBuf;

	/* Mid Page Constants */
	midPageOffset  = headPageOffset + headPageLength;
	midPageLength  = NVMDRV_PAGE_SIZE;
	midPageIterate = (nData - headPageLength) / NVMDRV_PAGE_SIZE;
	pMidPageRxBuf  = pHeadPageRxBuf + headPageLength;

	/* Tail Page Constants */
	tailPageOffset = midPageOffset + midPageLength * midPageIterate;
	tailPageLength = nData - headPageLength - (midPageLength * midPageIterate);
	pTailPageRxBuf = pMidPageRxBuf + midPageLength * midPageIterate;

	/* Read NVRAM */
	if (headPageLength>0) {
		subAddr[0] = (UINT8)(headPageOffset >> 8);
		subAddr[1] = (UINT8)(headPageOffset & 0xff);
		retVal = I2cReadNVM ( NVM_ID, 2, subAddr, headPageLength, pHeadPageRxBuf, 0);
		if (retVal < 0) {
			printf ("NVRAM Read Error : offset %x, nData %x, pRxBuf %x\n", offset, nData, pRxBuf);
			return -1;
		}
	}

	for (i=0; i<midPageIterate; i++) {
		subAddr[0] = (UINT8)(midPageOffset >> 8);
		subAddr[1] = (UINT8)(midPageOffset & 0xff);
		retVal = I2cReadNVM ( NVM_ID, 2, subAddr, midPageLength, pMidPageRxBuf + i*midPageLength, 0);

		if (retVal < 0) {
			printf ("NVRAM Read Error : offset %x, nData %x, pRxBuf %x\n", offset, nData, pRxBuf);
			return -1;
		}

		midPageOffset = midPageOffset + NVMDRV_PAGE_SIZE;
	}

	if (tailPageLength>0) {
		subAddr[0] = (UINT8)(tailPageOffset >> 8);
		subAddr[1] = (UINT8)(tailPageOffset & 0xff);
		retVal = I2cReadNVM ( NVM_ID, 2, subAddr, tailPageLength, pTailPageRxBuf, 0);
		if (retVal < 0) {
			printf ("NVRAM Read Error : offset %x, nData %x, pRxBuf %x\n", offset, nData, pRxBuf);
			return -1;
		}
	}

	/*------------------------------------------------------------
	 *	Report Result
	 -------------------------------------------------------------*/
	return 0;
}

static int _NVM_WriteNvram   (UINT32 offset, UINT32 nData, UINT8 *pTxBuf)
{
	UINT32	retVal;
	UINT32	i;

	UINT32	tmpVal;
	UINT8	subAddr[2];

	UINT16	headPageOffset;
	UINT32	headPageLength;
	UINT8	*pHeadPageTxBuf;

	UINT16	midPageOffset;
	UINT32	midPageLength;
	UINT32	midPageIterate;
	UINT8	*pMidPageTxBuf;

	UINT16	tailPageOffset;
	UINT32	tailPageLength;
	UINT8	*pTailPageTxBuf;

	/*------------------------------------------------------------
	 *	Write Data to NVRAM
	 -------------------------------------------------------------*/

	/* Head Page Constants */
	headPageOffset = offset;
	tmpVal = (NVMDRV_PAGE_SIZE - (offset % NVMDRV_PAGE_SIZE)) % NVMDRV_PAGE_SIZE;
	if (tmpVal > nData) {
		headPageLength = nData;
	} else {
		headPageLength = tmpVal;
	}
	pHeadPageTxBuf = pTxBuf;

	/* Mid Page Constants */
	midPageOffset  = headPageOffset + headPageLength;
	midPageLength  = NVMDRV_PAGE_SIZE;
	midPageIterate = (nData - headPageLength) / NVMDRV_PAGE_SIZE;
	pMidPageTxBuf  = pHeadPageTxBuf + headPageLength;

	/* Tail Page Constants */
	tailPageOffset = midPageOffset + midPageLength * midPageIterate;
	tailPageLength = nData - headPageLength - (midPageLength * midPageIterate);
	pTailPageTxBuf = pMidPageTxBuf + midPageLength * midPageIterate;

	/* Write NVRAM */
	if (headPageLength>0) {
		subAddr[0] = (UINT8)(headPageOffset >> 8);
		subAddr[1] = (UINT8)(headPageOffset & 0xff);
		retVal = I2cWriteNVM ( NVM_ID, 2, subAddr, headPageLength, pHeadPageTxBuf, 0);
		if (retVal < 0) {
			printf ("NVRAM Write Error  1 : offset %x, nData %x, pTxBuf %x\n", offset, nData, pTxBuf);
			return -1;
		}

		retVal = _NVM_CheckState (headPageOffset,*pHeadPageTxBuf);
		if (retVal < 0) {
			printf ("NVRAM Write Error  2 : offset %x, nData %x, pTxBuf %x\n", offset, nData, pTxBuf);
			return -1;
		}
	}

	for (i=0; i<midPageIterate; i++) {
		subAddr[0] = (UINT8)(midPageOffset >> 8);
		subAddr[1] = (UINT8)(midPageOffset & 0xff);
		retVal = I2cWriteNVM ( NVM_ID, 2, subAddr, midPageLength, pMidPageTxBuf + i*midPageLength, 0);
		if (retVal < 0) {
			printf ("NVRAM Write Error  3 : offset %x, nData %x, pTxBuf %x\n", offset, nData, pTxBuf);
			return -1;
		}

		retVal = _NVM_CheckState (midPageOffset,*(pMidPageTxBuf + i*midPageLength));
		if (retVal < 0) {
			printf ("NVRAM Write Error  4 : offset %x, nData %x, pTxBuf %x\n", offset, nData, pTxBuf);
			return -1;
		}

		midPageOffset = midPageOffset + NVMDRV_PAGE_SIZE;
	}

	if (tailPageLength>0) {
		subAddr[0] = (UINT8)(tailPageOffset >> 8);
		subAddr[1] = (UINT8)(tailPageOffset & 0xff);
		retVal = I2cWriteNVM ( NVM_ID, 2, subAddr, tailPageLength, pTailPageTxBuf, 0);
		if (retVal < 0) {
			printf ("NVRAM Write Error 5 : offset %x, nData %x, pTxBuf %x\n", offset, nData, pTxBuf);
			return -1;
		}

		retVal = _NVM_CheckState (tailPageOffset, *pTailPageTxBuf);

		if (retVal < 0) {
			printf ("NVRAM Write Error 6 : offset %x, nData %x, pTxBuf %x\n", offset, nData, pTxBuf);
			return -1;
		}
	}

	/*------------------------------------------------------------
	 *	Report Result
	 -------------------------------------------------------------*/
	return 0;
}

int DDI_NVM_Read  (UINT32 offset, UINT32 nData, UINT8 *pRxBuf)
{
	int	retVal;

	/* Input parameter check */
	if (((offset+nData) > NVMDRV_TOTAL_SIZE) || (pRxBuf == NULL))
	{
		printf ("DDI_NVM_Read Error - Wrong Size: offset 0x%x, nData 0x%x, pRxBuf 0x%x\n", offset, nData, (UINT32) pRxBuf);
		return -1;
	}

	/* Read from nvram */
	retVal = _NVM_ReadNvram (offset, nData, pRxBuf);
	if (retVal < 0)
	{
		printf ("DDI_NVM_Read Error - Wrong Read : offset 0x%x, nData 0x%x, pRxBuf 0x%x\n", offset, nData, (UINT32) pRxBuf);
		return -1;
	}

	return retVal;
}

int DDI_NVM_Write	(UINT32 offset, UINT32 nData, UINT8 *pTxBuf)
{
	int	retVal;

	/* Input parameter check */
	if (((offset+nData) > NVMDRV_TOTAL_SIZE) || (pTxBuf == NULL))
	{
		printf ("DDI_NVM_Write Error - Wrong Size : offset 0x%x, nData 0x%x, pTxBuf 0x%x\n", offset, nData, (UINT32) pTxBuf);
		return -1;
	}

	/* Write to nvram */
	retVal = _NVM_WriteNvram (offset, nData, pTxBuf);
	if (retVal < 0)
	{
		printf ("DDI_NVM_Write Error - Wrong Write : offset 0x%x, nData 0x%x, pTxBuf 0x%x\n", offset, nData, (UINT32) pTxBuf);
		return -1;
	}
	return retVal;
}

int DDI_NVM_ReadOne(UINT32 offset, UINT8 *value)
{
	UINT32 retVal;

	retVal = DDI_NVM_Read(offset, 1, value);

	return retVal;
}

int	DDI_NVM_WriteOne(UINT32 offset, UINT8 value)
{
	UINT32 retVal;

	retVal = DDI_NVM_Write(offset, 1, &value);

	return retVal;
}

void DDI_NVM_GetToolOpt(void)
{
	UINT32 					toolOptvalid;
	static int				bReadFlag = 0;

	if (bReadFlag == 0)
	{
		DDI_NVM_Read(TOOL_OPTION_DB_BASE, sizeof(gToolOptionDB.validMark), (UINT8 *)&(toolOptvalid));
		if(toolOptvalid != 0xffffffff)
			DDI_NVM_Read(TOOL_OPTION_DB_BASE, sizeof(gToolOptionDB), (UINT8 *)(&gToolOptionDB));

		bReadFlag = 1;
	}
#if 0
	printf("\n");
	printf("\ngToolOptionDB.nToolOption1=%d",gToolOptionDB.nToolOption1);
	printf("\ngToolOptionDB.nToolOption1.flags.nModuleVer=%d",gToolOptionDB.nToolOption1.flags.nModuleVer);
	printf("\ngToolOptionDB.nToolOption1.flags.eModelModuleType=%d",gToolOptionDB.nToolOption1.flags.eModelModuleType);
	printf("\ngToolOptionDB.nToolOption1.flags.eModelToolType=%d",gToolOptionDB.nToolOption1.flags.eModelToolType);
	printf("\ngToolOptionDB.nToolOption1.flags.eModelInchType=%d",gToolOptionDB.nToolOption1.flags.eModelInchType);
	printf("\n");
	printf("\ngToolOptionDB.nToolOption2=%d",gToolOptionDB.nToolOption2);
	printf("\ngToolOptionDB.nToolOption2.flags.nUSBCount=%d",gToolOptionDB.nToolOption2.flags.nUSBCount);
	printf("\ngToolOptionDB.nToolOption2.flags.nRGBCount=%d",gToolOptionDB.nToolOption2.flags.nRGBCount);
	printf("\ngToolOptionDB.nToolOption2.flags.nAVPosition=%d",gToolOptionDB.nToolOption2.flags.nAVPosition);
	printf("\ngToolOptionDB.nToolOption2.flags.nRCA_AVCount=%d",gToolOptionDB.nToolOption2.flags.nRCA_AVCount);
	printf("\ngToolOptionDB.nToolOption2.flags.nSCARTCount=%d",gToolOptionDB.nToolOption2.flags.nSCARTCount);
	printf("\ngToolOptionDB.nToolOption2.flags.bSupportComponentSwap=%d",gToolOptionDB.nToolOption2.flags.bSupportComponentSwap);
	printf("\ngToolOptionDB.nToolOption2.flags.bCompAVCommon=%d",gToolOptionDB.nToolOption2.flags.bCompAVCommon);
	printf("\ngToolOptionDB.nToolOption2.flags.eCompPosition=%d",gToolOptionDB.nToolOption2.flags.eCompPosition);
	printf("\ngToolOptionDB.nToolOption2.flags.nCOMPCount=%d",gToolOptionDB.nToolOption2.flags.nCOMPCount);
	printf("\ngToolOptionDB.nToolOption2.flags.nHDMIPosition=%d",gToolOptionDB.nToolOption2.flags.nHDMIPosition);
	printf("\ngToolOptionDB.nToolOption2.flags.eHDMISwitchIC=%d",gToolOptionDB.nToolOption2.flags.eHDMISwitchIC);
	printf("\ngToolOptionDB.nToolOption2.flags.nHDMICount=%d",gToolOptionDB.nToolOption2.flags.nHDMICount);
	printf("\n");
	printf("\ngToolOptionDB.nToolOption3=%d",gToolOptionDB.nToolOption3);
	printf("\ngToolOptionDB.nToolOption3.flags.bInstantBootEnabled=%d",gToolOptionDB.nToolOption3.flags.bInstantBootEnabled);
	printf("\ngToolOptionDB.nToolOption3.flags.bDVRReady=%d",gToolOptionDB.nToolOption3.flags.bDVRReady);
	printf("\ngToolOptionDB.nToolOption3.flags.bBootLogo=%d",gToolOptionDB.nToolOption3.flags.bBootLogo);
	printf("\ngToolOptionDB.nToolOption3.flags.bWirelessReady=%d",gToolOptionDB.nToolOption3.flags.bWirelessReady);
	printf("\ngToolOptionDB.nToolOption3.flags.eBackLight=%d",gToolOptionDB.nToolOption3.flags.eBackLight);
	printf("\ngToolOptionDB.nToolOption3.flags.eAudioAmp=%d",gToolOptionDB.nToolOption3.flags.eAudioAmp);
	printf("\ngToolOptionDB.nToolOption3.flags.bEManual=%d",gToolOptionDB.nToolOption3.flags.bEManual);
	printf("\ngToolOptionDB.nToolOption3.flags.bHeadphone=%d",gToolOptionDB.nToolOption3.flags.bHeadphone);
	printf("\ngToolOptionDB.nToolOption3.flags.bDigitalEye=%d",gToolOptionDB.nToolOption3.flags.bDigitalEye);
	printf("\ngToolOptionDB.nToolOption3.flags.bBluetooth=%d",gToolOptionDB.nToolOption3.flags.bBluetooth);
	printf("\ngToolOptionDB.nToolOption3.flags.bDivx=%d",gToolOptionDB.nToolOption3.flags.bDivx);
	printf("\ngToolOptionDB.nToolOption3.flags.bEMF=%d",gToolOptionDB.nToolOption3.flags.bEMF);
	printf("\n");
	printf("\ngToolOptionDB.nToolOption4=%d",gToolOptionDB.nToolOption4);
	printf("\ngToolOptionDB.nToolOption4.flags.bSupportEPA=%d",gToolOptionDB.nToolOption4.flags.bSupportEPA);
	printf("\ngToolOptionDB.nToolOption4.flags.bCIFS=%d",gToolOptionDB.nToolOption4.flags.bCIFS);
	printf("\ngToolOptionDB.nToolOption4.flags.bISF=%d",gToolOptionDB.nToolOption4.flags.bISF);
	printf("\ngToolOptionDB.nToolOption4.flags.bPictureWizard=%d",gToolOptionDB.nToolOption4.flags.bPictureWizard);
	printf("\ngToolOptionDB.nToolOption4.flags.bTHXMediaDirector=%d",gToolOptionDB.nToolOption4.flags.bTHXMediaDirector);
	printf("\ngToolOptionDB.nToolOption4.flags.bDLNA=%d",gToolOptionDB.nToolOption4.flags.bDLNA);
	printf("\ngToolOptionDB.nToolOption4.flags.bTHX=%d",gToolOptionDB.nToolOption4.flags.bTHX);
	printf("\ngToolOptionDB.nToolOption4.flags.bLocalDimming=%d",gToolOptionDB.nToolOption4.flags.bLocalDimming);
	printf("\ngToolOptionDB.nToolOption4.flags.eAnalogDemod=%d",gToolOptionDB.nToolOption4.flags.eAnalogDemod);
	printf("\ngToolOptionDB.nToolOption4.flags.eDigitalDemod=%d",gToolOptionDB.nToolOption4.flags.eDigitalDemod);
	printf("\n");
	printf("\ngToolOptionDB.nToolOption5=%d",gToolOptionDB.nToolOption5);
	printf("\ngToolOptionDB.nToolOption5.flags.bPowerBoardType=%d",gToolOptionDB.nToolOption5.flags.bPowerBoardType);
	printf("\ngToolOptionDB.nToolOption5.flags.bNetcastService=%d",gToolOptionDB.nToolOption5.flags.bNetcastService);
	printf("\ngToolOptionDB.nToolOption5.flags.bOrangeService=%d",gToolOptionDB.nToolOption5.flags.bOrangeService);
	printf("\ngToolOptionDB.nToolOption5.flags.bMirrorMode=%d",gToolOptionDB.nToolOption5.flags.bMirrorMode);
	printf("\ngToolOptionDB.nToolOption5.flags.nUSBHubLevel=%d",gToolOptionDB.nToolOption5.flags.nUSBHubLevel);
	printf("\ngToolOptionDB.nToolOption5.flags.bTVLink=%d",gToolOptionDB.nToolOption5.flags.bTVLink);
	printf("\ngToolOptionDB.nToolOption5.flags.bChBrowser=%d",gToolOptionDB.nToolOption5.flags.bChBrowser);
	printf("\ngToolOptionDB.nToolOption5.flags.bMotionRemocon=%d",gToolOptionDB.nToolOption5.flags.bMotionRemocon);
	printf("\ngToolOptionDB.nToolOption5.flags.bSupportSkype=%d",gToolOptionDB.nToolOption5.flags.bSupportSkype);
	printf("\ngToolOptionDB.nToolOption5.flags.bSupportWiFi=%d",gToolOptionDB.nToolOption5.flags.bSupportWiFi);
	printf("\ngToolOptionDB.nToolOption5.flags.nHDMIPortSwapOrder=%d",gToolOptionDB.nToolOption5.flags.nHDMIPortSwapOrder);
	printf("\n");
	printf("\n");


#endif

	return ;
}

extern U8 Splash_GetHWoption(HW_OPT_T option_mask);
int DDI_GetModelOption(void)
{
 	if(!Splash_GetHWoption(MICOM_HW_OPT_SEL) ) return 0;	//lcd
	else 										 return 1;	//pdp
}

int OSA_MD_GetDbgMode(void)
{
	uint32_t	nvmData;

	if (!bDebugStatusRead)
	{
		DDI_NVM_Read(SYS_DB_BASE , sizeof(gSysNvmDB.validMark), (UINT8 *)&(nvmData));
		if(nvmData != 0xffffffff)
		{
			DDI_NVM_Read(SYS_DB_BASE + (UINT32)&(gSysNvmDB.nDebugStatus) - (UINT32)&gSysNvmDB,			\
				sizeof(gSysNvmDB.nDebugStatus), (UINT8 *)&(gSysNvmDB.nDebugStatus));
		}
		else
		{
			gSysNvmDB.nDebugStatus = RELEASE_LEVEL ;
		}
		bDebugStatusRead =1;
	}
	return (int)gSysNvmDB.nDebugStatus;
}

void OSA_MD_SetDbgMode(UINT8	nDebugStatus)
{
	uint32_t	nvmData;

	gSysNvmDB.nDebugStatus = nDebugStatus ;

	DDI_NVM_Read(SYS_DB_BASE , sizeof(gSysNvmDB.validMark), (UINT8 *)&(nvmData));
	if(nvmData != 0xffffffff)
	{
		DDI_NVM_Write(SYS_DB_BASE + (UINT32)&(gSysNvmDB.nDebugStatus) - (UINT32)&gSysNvmDB,			\
			sizeof(gSysNvmDB.nDebugStatus), (UINT8 *)&(gSysNvmDB.nDebugStatus));
	}
}


int do_nvm_dbg (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) // should be used integer type function
{
	UINT32	command;
	UINT32	addr;
	UINT32	data;
	UINT32	size;
	UINT32	i;
	UINT8	*pBuf;

	/* command processing */
	do
	{
		_NVM_PrintMap();
		printf ("\n\n========== [Nvram Debug Menu] ============");
		printf ("\n");
		printf ("\n      0x01 :  Read Byte");
		printf ("\n      0x02 :  Write Byte");
		printf ("\n      0x10 :  Clear NVRAM");
		printf ("\n      0x11 :  Fill Block");
		printf ("\n      0x12 :  Read Block");
		printf ("\n      0x13 :  Write Block");
		printf ("\n      0xFF :  Exit From This Menu");
		printf ("\n");
		printf ("\n==========================================");
		printf ("\n\n");

		_dbgHexDecimalInput ("Enter command", &command);
		printf ("\n\n");

		switch (command)
		{
			case 0x01 :
				pBuf = (UINT8 *) malloc (4);
				_dbgHexDecimalInput ("Addr", &addr);
				DDI_NVM_Read (addr, 1, pBuf);
				printf ("Data : 0x%x\n", pBuf[0]);
				free (pBuf);
				break;

			case 0x02 :
				pBuf = (UINT8 *) malloc (4);
				_dbgHexDecimalInput ("Addr", &addr);
				_dbgHexDecimalInput ("Data", &data);
				pBuf[0] = (UINT8) data;
				DDI_NVM_Write (addr, 1, pBuf);
				free (pBuf);
				break;

			case 0x10 :
				pBuf = (UINT8 *) malloc (NVMDRV_TOTAL_SIZE);
				memset(pBuf, 0xff, NVMDRV_TOTAL_SIZE);
				DDI_NVM_Write (0, NVMDRV_TOTAL_SIZE, pBuf);
				free (pBuf);
				break;

			case 0x11 :
				_dbgHexDecimalInput ("Addr", &addr);
				_dbgHexDecimalInput ("Size", &size);
				_dbgHexDecimalInput ("Data", &data);
				pBuf = (UINT8 *) malloc (size);
				memset (pBuf, (UINT8)data, size);
				DDI_NVM_Write (addr, size, pBuf);
				free (pBuf);
				break;

			case 0x12 :
				_dbgHexDecimalInput ("Addr", &addr);
				_dbgHexDecimalInput ("Size", &size);
				pBuf = (UINT8 *) malloc (size);
				DDI_NVM_Read (addr, size, pBuf);
				for (i=0; i<size; i++)
				{
					printf ("Data[0x%x] : 0x%x\n", addr+i, pBuf[i]);
				}
				free (pBuf);
				break;

			case 0x13 :
				_dbgHexDecimalInput ("Addr", &addr);
				_dbgHexDecimalInput ("Size", &size);
				pBuf = (UINT8 *) malloc (size);
				for (i=0; i<size; i++)
				{
					printf ("Data[0x%x]", addr+i);
					_dbgHexDecimalInput ("", &data);
					pBuf[i] = (UINT8) data;
				}
				DDI_NVM_Write (addr, size, pBuf);
				free (pBuf);
				break;

			case 0xFF :
				/* Exit this menu */
				command = DEBUG_INVALID_COMMAND;
				break;

			default :
				break;
		}
		bDebugStatusRead = 0;

	} while (command != DEBUG_INVALID_COMMAND);

	return 0;
}
#ifdef INCLUDE_ADDI_LG_CMDS
U_BOOT_CMD(
	nvmdbg,	1,	0,	do_nvm_dbg,
	"nvmdbg\t- eeprom test progrm\n",
);
#endif
