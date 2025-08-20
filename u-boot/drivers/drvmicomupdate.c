/******************************************************************************

  LGE. DTV RESEARCH LABORATORY
  COPYRIGHT(c) LGE CO.,LTD. 2006. SEOUL, KOREA.
  All rights are reserved.

 ******************************************************************************/

/** @file drvmicomupdate.c
 *
 *	This c file defines the DDI functions related to MICOM Interface
 *
 *	@author 	redcloud00(redcloud00@lge.com)
 *	@version	0.1
 *	@date	2009.08.10
 *	@note
 *	@see
 */


/*------------------------------------------------------------------------
 *	file inclusions
 *-----------------------------------------------------------------------*/
#include <common.h>
#include <command.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <exports.h>
#include <mtdinfo.h>
#include <micom.h>
#include <malloc.h>

/*------------------------------------------------------------------------
 *	constant definitions
 *-----------------------------------------------------------------------*/
#ifndef INVALID_COMMAND
#define INVALID_COMMAND							0xff
#endif

#define RETRY_COUNT								5
#define BUF_SIZE								256
#define MAX_MICOM_IMAGE_SIZE					0x20000

#define DL_DONE									0
#define DL_BADCHAR								1
#define DL_CONT									2

#define MYSON_MICOM_ID 							0x94

#define BLOCK_COUNT								31		// NEC - 32KB (0 ~ 31)
#define ONE_BLOCK_SIZE							1024	// NEC - 1KB
#define ONETIME_WRITE_SIZE						64		// NEC - 64 bytes

/*------------------------------------------------------------------------
 *	macro function definitions
 *-----------------------------------------------------------------------*/
#define	MICOM_DEBUG
#ifdef	MICOM_DEBUG
#define micom_print(fmt, args...)	printf(fmt, ##args)
#else
#define micom_print(fmt, args...)
#endif

#define I2C_UPDATE_MICOM(a, b, c, d, e)				MDrv_SW_IIC_Write(EXT_MICOM_CH, EXT_MICOM_DEVID, 0, 0, c, d)
#define NEC_VER_SIZE 4

extern char * strtok(char * s,const char * ct);
#define strtok_r( _s, _sep, _lasts ) \
        ( *(_lasts) = strtok( (_s), (_sep) ) )



/*------------------------------------------------------------------------
 *	type definitions
 *-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *	extern global variables and function prototypes declarations
 *-----------------------------------------------------------------------*/


//extern OSA_TASK_T UCOMTaskId;
extern unsigned char gDebugMicomDownload;

/*------------------------------------------------------------------------
 *	global variables and function prototypes declarations
 *-----------------------------------------------------------------------*/


/*------------------------------------------------------------------------
 *	static variables and function prototypes declarations
 *-----------------------------------------------------------------------*/
static unsigned char *_gpImageBuf = NULL;


/**
 * To convert  hex to bin.
 *
 * @param c [IN] input character
 * @return integer value
 * @see none
 * @author Jung Kyong Jo
 */
static int _SWU_HexToInt (char c)
{
	if ((c >= '0') && (c <= '9'))
		return (c - '0');

	if ((c >= 'A') && (c <= 'F'))
		return (c - 'A' + 10);

	return (0);
}

/**
 * To calculate check-sum.
 *
 * @param buf [IN] input buf
 * @param length [IN] buf length
 * @return check-sum value
 * @see none
 * @author Jung Dong Ho
 */
UINT8 _SWU_ChkSum(UINT8 *buf, UINT8 length)
{
	UINT8 i, work = 0;

	for(i = 0; i < length; i++)
		work += buf[i];

	work = ~work + 1;

	return work;
}

/**
 * To exit the isp mode.
 *
 * @param void
 * @return void
 * @see none
 * @author Jung Kyong Jo
 */
 #if 0
void SWU_MICOM_ExitISPMode (void)
{
	char tmpBuf[BUF_SIZE];

	memset (tmpBuf, 0x0, BUF_SIZE);
	// Wake up Target
	micom_print ("Wake up Target : ");
	tmpBuf[0] = 0x48, tmpBuf[1] = 0x00;
	if (I2C_UPDATE_MICOM (0, 0, 2, tmpBuf, 3) < 0)
		return;
}
#endif
/**
 * To convert hex to bin consecutively.
 *
 * @param pRecBuf [IN] receive buf
 * @param pLen [OUT] hex length
 * @param pDataSize [OUT] data size
 * @return count
 * @see none
 * @author Jung Kyong Jo
 */
int SWU_MICOM_ConvertHexToBin (char *pRecBuf, int *pLen, int *pDataSize)
{
	int		i, hexLength, j;
	int		s;
	unsigned short	hexAddr;
	unsigned char	*pi, recType;
	static unsigned char _crcCheck = 0;
	static unsigned char _crcVal;

	// Get Hex Length
	hexLength = (_SWU_HexToInt (pRecBuf[1]) << 4), hexLength |= _SWU_HexToInt (pRecBuf[2]);
	_crcCheck += hexLength;

	// Get Data Address
	hexAddr  = (_SWU_HexToInt (pRecBuf[3]) << 12), hexAddr |= (_SWU_HexToInt (pRecBuf[4]) << 8);
	hexAddr |= (_SWU_HexToInt (pRecBuf[5]) <<  4), hexAddr |= _SWU_HexToInt (pRecBuf[6]);
	_crcCheck += (hexAddr & 0xFF), _crcCheck += (hexAddr >> 8);

	// Get Record Type
	recType = (_SWU_HexToInt (pRecBuf[7]) << 4), recType |= _SWU_HexToInt (pRecBuf[8]);
	_crcCheck += recType;

	//Get  Data Size
	if (*pDataSize < (hexAddr + hexLength))
		*pDataSize = hexAddr + hexLength;

	// Get Hex Data;
	for (i = 0, j = 9, pi = _gpImageBuf + hexAddr; i < hexLength; i++, pi++)
	{
		*pi = (_SWU_HexToInt (pRecBuf[j]) << 4); j++;
		*pi |= _SWU_HexToInt (pRecBuf[j]); j++;

		_crcCheck += *pi;
	}

	// Check CRC
	_crcVal = (_SWU_HexToInt (pRecBuf[j]) << 4), _crcVal |= _SWU_HexToInt (pRecBuf[j + 1]);
	_crcCheck += _crcVal;

	micom_print (".");
	//micom_print ("recType = 0x%x  hexaddr = 0x%x",recType,hexAddr);

	if (recType == 0x01 && hexAddr == 0x0000)
	{
		s = DL_DONE;
		micom_print ("\n\tData Size = %04x\n", *pDataSize);
		micom_print ("\tTransfer Completed\n");

	}
	else if (_crcCheck)
	{
		s = DL_BADCHAR;
		micom_print ("\tCRC Fail !!: %d\n", _crcCheck);
	}
	else
		s = DL_CONT;

	*pLen = hexLength;
	return (s);
}

/**
 * To update NEC micom image.
 *
 * @param pakOrgSize [IN] the size of the micom pak file
 * @param dataSize [IN] the offset of the micom binary
 * @return if succeeded, return SWU_OK. else return SWU_FAIL
 * @see Micom structure
 * @see Total flash size : 32KB
 * @see Boot & Self Block : 4KB (0~3), Boot & Self Backup Block : 4KB (4~7)
 * @see User1 Block : 4KB (8~11), User2 Block : 20KB (12~31)
 * @see Command Type : ERASE(0x0), FMODE(0x1), WRITE(0x2), DATAVERIFY(0x3)
 * @see Command Type : GETERROR(0x4), IVERIFY(0x5), UMODE(0x6), CTEST(0x7)
 * @see Command Type : EEPROM(0x8), BCHECK(0x9), SWAP(0xa), GETINFO(0xb)
 * @see Command Type : RESET(0xc), READY(0xda)
 * @author Jung Dong Ho
 */
u8 SWU_MICOM_UpdateNecMICOM (u32 pakOrgSize, u32 dataSize)
{
	int		i;
	int		j;
	int		k, pt;
	int		retryIndex;
	int		romAddress;
	unsigned char	tmpBuf[BUF_SIZE * 2], *pCurPtr;
	U32		errcnt=0;

	micom_print ("Update MICOM : NEC\n");


	//////////////////////////////////
	// 1. Flash Download Mode Ready
	// [comand, length, enable, check sum]
	//////////////////////////////////
	micom_print ("\t## Flash Download Mode Setting ##\n");
	tmpBuf[0] = 0xda, tmpBuf[1] = 0x04;
	tmpBuf[2] = 0x01, tmpBuf[3] = _SWU_ChkSum(tmpBuf, 3);
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++ )
	{
		if (I2C_UPDATE_MICOM (0, 0, 4, tmpBuf, 1) < 0)
		{
			micom_print ("Flash Download Mode Setting failed.. \n");
			micom_print ("Retry Flash Download Mode Setting %d\n", retryIndex + 1);
			continue;
		}

		break;
	}

	if (retryIndex == RETRY_COUNT)
	{
		micom_print ("Flash Download Mode Setting failed..\n");

		goto Error;
	}
	else
		micom_print ("Flash Download Mode Setting Okay. : %d\n", retryIndex);

	udelay(50000);

	//////////////////////////////////
	// 2. Flash Download Mode Check
	// [comand, length, bank, check sum]
	//////////////////////////////////
	micom_print ("\t## Flash Download Mode Check ##\n");
	tmpBuf[0] = 0x01, tmpBuf[1] = 0x04;
	tmpBuf[2] = 0x00, tmpBuf[3] = _SWU_ChkSum(tmpBuf, 3);
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++ )
	{
		if (I2C_UPDATE_MICOM (0, 0, 4, tmpBuf, 1) < 0)
		{
			micom_print ("Flash Download Mode Check failed.. 1\n");
			micom_print ("Retry Flash Download Mode Check %d\n", retryIndex + 1);
			continue;
		}

		break;
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("Flash Download Mode Check failed..\n");
		goto Error;
	}
	else
		micom_print ("Flash Download Mode Check Okay.\n");

	udelay(50000);


	//////////////////////////////////
	// 3. New Boot Area Erase
	// [comand, length, block num, bank, check sum]
	//////////////////////////////////
	micom_print ("\t## Boot Area Erase ##\n");
	tmpBuf[0] = 0x00, tmpBuf[1] = 0x05;
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		for (j=4; j<8; j++)
		{
			micom_print ("Boot Area Erase #%d.\n", j);
			tmpBuf[2] = j;
			tmpBuf[3] = 0;
			tmpBuf[4] = _SWU_ChkSum(tmpBuf, 4);

			if (I2C_UPDATE_MICOM (0, 0, 5, tmpBuf, 1) < 0)
			{
				micom_print ("Boot Area Erase failed.. \n");
				micom_print ("Retry erase %d\n", retryIndex + 1);
				errcnt++;
				//continue;
			}

			udelay(50000);
		}

		if(errcnt>0)
		{
			micom_print (" errcnt = %d \n", errcnt);
			errcnt = 0;
			continue;
		}
		else
		{			
			break;
		}	
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("Flash Erase failed..\n");
		goto Error;
	}
	else
		micom_print ("Flash Erase Okay.\n");

	udelay(300000); //more that 300msec
	//////////////////////////////////
	// 4. New Boot Area Write
	// [comand, length, bank, address, data, check sum]
	//////////////////////////////////
	micom_print ("\t## New Boot Area Write ##\n");
	tmpBuf[0] = 0x02, tmpBuf[1] = 0x46;
	tmpBuf[2] = 0;
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		pt = 0; 						// Hex 파일의 처음부터 복사

		for (i=4; i<8; i++)
		{
			micom_print ("Flash Write Boot Area #%d.\n", i);

			romAddress = i * ONE_BLOCK_SIZE;	// 기입처 ROM 주소 산출

			for (j=0; j<ONE_BLOCK_SIZE / ONETIME_WRITE_SIZE; j++)
			{
				tmpBuf[3] = (UINT8) (romAddress >> 8);
				tmpBuf[4] = (UINT8) (romAddress >> 0);

				for(k = 0, pCurPtr = _gpImageBuf; k < ONETIME_WRITE_SIZE; k++)
				{
					tmpBuf[k+5] = pCurPtr[pt];		// 송신 버퍼에 기입해 데이터를 세트
					pt++;
				}
				tmpBuf[k+5] = _SWU_ChkSum(tmpBuf, 69);

				if (I2C_UPDATE_MICOM (0, 0, 70, tmpBuf, 1) < 0)
				{
					micom_print ("Write MICOM failed.. \n");
					micom_print ("Retry write %d\n", retryIndex + 1);
					errcnt++;
					//continue;
				}

				romAddress += ONETIME_WRITE_SIZE;

				udelay(5000);
			}

		}

		if(errcnt>0)
		{
			micom_print (" errcnt = %d \n", errcnt);
			errcnt = 0;
			continue;
		}
		else
		{			
			break;
		}	
	}


	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("New Boot  Area Write failed..\n");
		goto Error;
	}
	else
		micom_print ("New Boot  Area Write Okay.\n");

	udelay(500000);
//_____________________________________________________________
	//////////////////////////////////
	// 5. User1 Area Erase
	// [comand, length, block num, bank, check sum]
	//////////////////////////////////
	micom_print ("\t## User1 Area Erase ##\n");
	tmpBuf[0] = 0x00, tmpBuf[1] = 0x05;
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		for (j=8; j<12; j++)
		{
			micom_print ("User1 Area Erase #%d.\n", j);
			tmpBuf[2] = j;
			tmpBuf[3] = 0;
			tmpBuf[4] = _SWU_ChkSum(tmpBuf, 4);

			if (I2C_UPDATE_MICOM (0, 0, 5, tmpBuf, 1) < 0)
			{
				micom_print ("User1 Area Erase failed.. \n");
				micom_print ("Retry erase %d\n", retryIndex + 1);
				errcnt++;
				//continue;
			}

			udelay(500000);
		}

		if(errcnt>0)
		{
			micom_print (" errcnt = %d \n", errcnt);
			errcnt = 0;
			continue;
		}
		else
		{			
			break;
		}	
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("Flash Erase failed..\n");
		goto Error;
	}
	else
		micom_print ("Flash Erase Okay.\n");

	udelay(500000);
//_______________________________________________________________
	//////////////////////////////////
	// 6. User1 Area Write
	// [comand, length, bank, address, data, check sum]
	//////////////////////////////////
	micom_print ("\t## User1 Area Write ##\n");
	tmpBuf[0] = 0x02, tmpBuf[1] = 0x46;
	tmpBuf[2] = 0;
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		for (i=8; i<12; i++)
		{
			micom_print ("Flash Write User1 Area #%d.\n", i);

			romAddress = i * ONE_BLOCK_SIZE;	// 기입처 ROM 주소 산출

			for (j=0; j<ONE_BLOCK_SIZE / ONETIME_WRITE_SIZE; j++)
			{
				tmpBuf[3] = (UINT8) (romAddress >> 8);
				tmpBuf[4] = (UINT8) (romAddress >> 0);

				pt = romAddress;				// Hex 파일과 동일한 주소에 복사

				for(k = 0, pCurPtr = _gpImageBuf; k < ONETIME_WRITE_SIZE; k++)
				{
					tmpBuf[k+5] = pCurPtr[pt]; 	// 송신 버퍼에 기입해 데이터를 세트
					pt++;
				}
				tmpBuf[k+5] = _SWU_ChkSum(tmpBuf, 69);

				if (I2C_UPDATE_MICOM (0, 0, 70, tmpBuf, 1) < 0)
				{
					micom_print ("User1 Area Write failed.. \n");
					micom_print ("Retry write %d\n", retryIndex + 1);
					errcnt++;
					//continue;
				}

				romAddress += ONETIME_WRITE_SIZE;

				udelay(5000);
			}

		}

		if(errcnt>0)
		{
			micom_print (" errcnt = %d \n", errcnt);
			errcnt = 0;
			continue;
		}
		else
		{
			break;
		}	
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("User1 Area Write failed..\n");
		goto Error;
	}
	else
		micom_print ("User1 Area Write Okay.\n");

	udelay(50000);

//_____________________________________________________________
	//////////////////////////////////
	// 7. User2 Area Erase
	// [comand, length, block num, bank, check sum]
	//////////////////////////////////
	micom_print ("\t## User2 Area Erase ##\n");
	tmpBuf[0] = 0x00, tmpBuf[1] = 0x05;
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		for (j=12; j<32; j++)
		{
			micom_print ("User2 Area Erase #%d.\n", j);
			tmpBuf[2] = j;
			tmpBuf[3] = 0;
			tmpBuf[4] = _SWU_ChkSum(tmpBuf, 4);

			if (I2C_UPDATE_MICOM (0, 0, 5, tmpBuf, 1) < 0)
			{
				micom_print ("User2 Area Erase failed.. \n");
				micom_print ("Retry erase %d\n", retryIndex + 1);
				errcnt++;
				//continue;
			}
			udelay(500000);
		}

		if(errcnt>0)
		{
			micom_print (" errcnt = %d \n", errcnt);
			errcnt = 0;
			continue;
		}
		else
		{
			break;
		}	
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("Flash Erase failed..\n");
		goto Error;
	}
	else
		micom_print ("Flash Erase Okay.\n");

	udelay(50000);
//_____________________________________________________________

	//////////////////////////////////
	// 8. User2 Area Write
	// [comand, length, bank, address, data, check sum]
	//////////////////////////////////
	micom_print ("\t## User2 Area Write ##\n");
	tmpBuf[0] = 0x02, tmpBuf[1] = 0x46;
	tmpBuf[2] = 0;
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		for (i=12; i<32; i++)
		{
			micom_print ("Flash Write User2 Area #%d.\n", i);

			romAddress = i * ONE_BLOCK_SIZE;	// 기입처 ROM 주소 산출

			for (j=0; j<ONE_BLOCK_SIZE / ONETIME_WRITE_SIZE; j++)
			{
				tmpBuf[3] = (UINT8) (romAddress >> 8);
				tmpBuf[4] = (UINT8) (romAddress >> 0);

				pt = romAddress;				// Hex 파일과 동일한 주소에 복사

				for(k = 0, pCurPtr = _gpImageBuf; k < ONETIME_WRITE_SIZE; k++)
				{
					tmpBuf[k+5] = pCurPtr[pt]; 	// 송신 버퍼에 기입해 데이터를 세트
					pt++;
				}
				tmpBuf[k+5] = _SWU_ChkSum(tmpBuf, 69);

				if (I2C_UPDATE_MICOM (0, 0, 70, tmpBuf, 1) < 0)
				{
					micom_print ("User2 Area Write failed.. \n");
					micom_print ("Retry write %d\n", retryIndex + 1);
					errcnt++;
					//continue;
				}

				romAddress += ONETIME_WRITE_SIZE;

				udelay(5000);
			}

		}

		if(errcnt>0)
		{
			micom_print (" errcnt = %d \n", errcnt);
			errcnt = 0;
			continue;
		}
		else
		{
			break;
		}	
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("User2 Area Write failed..\n");
		goto Error;
	}
	else
		micom_print ("User2 Area Write Okay.\n");

	udelay(50000);

	//////////////////////////////////
	// 9. Flash Verify
	// [comand, length, block, bank, check sum]
	//////////////////////////////////
	micom_print ("\t## Flash Verify ##\n");
	tmpBuf[0] = 0x05, tmpBuf[1] = 0x05;
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		for (j=4; j<32; j++)
		{
			micom_print ("Flash Verify  #%d.\n", j);
			tmpBuf[2] = j;
			tmpBuf[3] = 0;
			tmpBuf[4] = _SWU_ChkSum(tmpBuf, 4);

			if (I2C_UPDATE_MICOM (0, 0, 5, tmpBuf, 1) < 0)
			{
				micom_print ("Flash Verify failed.. \n");
				micom_print ("Retry Flash Verify %d\n", retryIndex + 1);
				errcnt++;
				//continue;
			}
			udelay(50000);
		}

		if(errcnt>0)
		{
			micom_print (" errcnt = %d \n", errcnt);
			errcnt = 0;
			continue;
		}
		else
		{
			break;
		}	
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("Flash Verify failed..\n");
		goto Error;
	}
	else
		micom_print ("Flash Verify Okay.\n");

	udelay(50000);

	//////////////////////////////////
	// 10. Boot Swap
	// [comand, length, dummy, check sum]
	//////////////////////////////////
	micom_print ("\t## Boot Swap ##\n");
	tmpBuf[0] = 0x0a, tmpBuf[1] = 0x04;
	tmpBuf[2] = 0x00, tmpBuf[3] = _SWU_ChkSum(tmpBuf, 3);
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		if (I2C_UPDATE_MICOM (0, 0, 4, tmpBuf, 1) < 0) // IFREN High
		{
			micom_print ("Boot Swap failed.. \n");
			micom_print ("Retry Boot Swap %d\n", retryIndex + 1);
			continue;
		}

		break;
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("Boot Swap failed..\n");
		goto Error;
	}
	else
		micom_print ("Boot Swap Okay.\n");

	udelay(500000);

	//////////////////////////////////
	// 11. Flash Reset
	// [comand, length, bummy, check sum]
	//////////////////////////////////

	micom_print ("\t## Reset Flash ##\n");
	tmpBuf[0] = 0x0c, tmpBuf[1] = 0x04;
	tmpBuf[2] = 0x00; tmpBuf[3] = _SWU_ChkSum(tmpBuf, 3);
	for (retryIndex = 0; retryIndex < RETRY_COUNT; retryIndex++)
	{
		if (I2C_UPDATE_MICOM (0, 0, 4, tmpBuf, 1) < 0)
		{
			micom_print ("Reset MICOM failed.. \n");
			micom_print ("Retry reset %d\n", retryIndex + 1);
			continue;
		}

		break;
	}

	if (retryIndex == RETRY_COUNT )
	{
		micom_print ("Flash Reset failed..\n");
		goto Error;
	}
	else
		micom_print ("Flash Reset Okay.\n");

	udelay(50000);

	micom_print ("Micom Update Success!!!!!!\n");

	return (0);

Error :

	micom_print ("Micom Update Error!!!!!!\n");

	return (-1);
}


u8 SWU_MICOM_UpdateMICOMImage (char* pImg_start, u32 pakOrgSize)
{
	char	*pHexData = NULL;
	int		dataSize=0, readSize=0, len, binSize=0, tReadSize=0;
	int		status = DL_CONT;
	char	*pCurPtr, *pTmpPtr, *pLast;
	u8	retVal = -1;

	pHexData = pImg_start;
	pCurPtr = pImg_start + pakOrgSize;

	_gpImageBuf = (unsigned char *)malloc (MAX_MICOM_IMAGE_SIZE);
	if (!_gpImageBuf)
	{
		micom_print ("Cannot allocate memory for image data!!\n");
		free (pHexData);
		return (-1);
	}

	micom_print ("Converting Hex data to Binary...\n");

	pTmpPtr = pHexData;
	pCurPtr = strtok_r (pTmpPtr, "\n", (char **)&pLast);

	if (!pCurPtr)
	{
		micom_print ("illegal pointer ...\n");
		goto EndTask;
	}

	//pCurPtr = strtok_r (NULL, "\n", (char **)&pLast);// for 1st line parsing...1st line is not needed

	while (pCurPtr && (status == DL_CONT) && (tReadSize < pakOrgSize))
	{
		readSize   = strlen (pCurPtr);
		tReadSize += readSize;

		//micom_print ("read data : %s\n readSize : %d\n", pCurPtr, readSize);
		status = SWU_MICOM_ConvertHexToBin ((char *)pCurPtr, &len, &dataSize);

		if (status == DL_BADCHAR)
			goto EndTask;

		binSize += len;

		pCurPtr = strtok_r (NULL, "\n", (char **)&pLast);
	}

	micom_print ("\nBinary size : %d , dataSize : %d\n", binSize, dataSize);
	retVal = SWU_MICOM_UpdateNecMICOM(pakOrgSize, dataSize);

EndTask :

	if (pHexData)
		free (pHexData);
	if (retVal == 0)
		micom_print ("\nMicom Update OK.\n");
	else
		micom_print ("\nMicom Update Fail.\n");

	return (retVal);

}

u8 SWU_MICOM_BinUpdate(char* pImg_start, u32 size)
{
	MICOM_VERSION_T micomVerNecT;
	char	micomVerNec[10] = { 0 , };
//	char	micomVerApp[10] = { 0 , };

	MICOM_GetVersion(&micomVerNecT);

	micomVerNec[0] = micomVerNecT.number[1];
	micomVerNec[1] = micomVerNecT.number[3];
	micomVerNec[2] = micomVerNecT.number[4];
	micomVerNec[3] = micomVerNecT.number[6];
	micomVerNec[4] = '0';

	micom_print("micomVerNec = %s \n", micomVerNec);

#if 0
	memcpy(micomVerApp,pImg_start,NEC_VER_SIZE);
	micom_print("micomVerApp = %s\n", micomVerApp);

	if(strlen(micomVerApp) != NEC_VER_SIZE && strlen(micomVerNec) != NEC_VER_SIZE)
	{
		micom_print("Cannot find micom version..\n");
		return -1;
	}
	if( strcmp(micomVerApp, micomVerNec) <= 0)
	{
		micom_print("Your application micom version is not new one.\n");
		micom_print("No micom update.\n");
		return -1;
	}
#endif
	if(pImg_start[0] != ':' && pImg_start[4] == ':')
	{
		micom_print("Micom binary have Version Field [ v%c.%c%c.%c ] \n",
			pImg_start[0] , pImg_start[1] , pImg_start[2] , pImg_start[3] );
		pImg_start += 4;
	}
	else
	{
		micom_print("Micom binary have no Version Field. \n");
	}
		
	if (SWU_MICOM_UpdateMICOMImage(pImg_start, size) != 0)
	{
		micom_print ( "SWU_MICOM_UpdateMICOMImage() failed\n");
		return -1;
	}
	micom_print( "SWU_MICOM_UpdateMICOMImage() Success\n");
	return 0;
}


