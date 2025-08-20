////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_cec.c
/// @brief  CEC(Consumer Electronics Control) Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <linux/autoconf.h>
//#include <linux/undefconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "Board.h"
#include "mst_devid.h"
#include "mdrv_types.h"

#include "mdrv_cec.h"
//#include "mhal_cec_reg.h"
#include "chip_int.h"
//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define CEC_REG(addr)                   (*(volatile U32 *)(addr))
#define CEC_REG_8(addr)                 (*(volatile U8 *)(addr))
#define MDrv_WriteByteMask( u16Reg, u8Value, u8Mask )    \
    (CEC_REG_8(u16Reg) = (CEC_REG_8(u16Reg) & ~(u8Mask)) | ((u8Value) & (u8Mask)))
#define CEC_PRINT(fmt, args...)         //printk("[CEC][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_CEC_DEVICE_COUNT     1


#define CEC_DEBUG
#ifdef RTC_DEBUG
#define DEBUG_CEC(x) (x)
#else
#define DEBUG_CEC(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
extern void MDrv_CEC_RxChkBuf(void) ;
extern U8 _MDrv_HeaderSwap(U8 value);
extern U8 _MDrv_CecSendFrame(U8 header, U8 opcode, U8* operand, U8 len);
//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------

static CEC_INFO_LIST_t _CECInfo;
static CEC_ERROR_CODE _ErrorCode = E_CEC_FEATURE_ABORT;
static U8   _u8CecResponse = 0xFF;
static U8   _u8CecResponse2 = 0xFF;
static U8  _u8MsgID;
static U8	_u8AckResult;//080627_yongs
static U8  _u8CecEnable=1;//080910_yongs

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static irqreturn_t _MDrv_HDMI_ISR(int irq, void *dev_id)
{
#if 0
    U8  u8IntStatus;

	u8IntStatus = CEC_REG_8(REG_IRQ_FINAL_STATUS);

    printk("HDMI_ISR status 0x%x\n", u8IntStatus);

	if ( u8IntStatus & BIT4 )   // 31: ADCDVI2RIU
    {
        //if(g_HDMI_ISR_Callback)//victor 20081229, DVI+HDCP snow noise patch
            //g_HDMI_ISR_Callback();

	  // HDMI irq clear
	      CEC_REG(REG_HDMI_INT_CLEAR) = 0x2000;
    	  CEC_REG(REG_HDMI_INT_CLEAR) = 0;
    }

    if ( u8IntStatus & BIT4 )   // 31: ADCDVI2RIU
#endif
    {
        MDrv_CEC_RxChkBuf();     // save the buffer for receiving a CEC message.
        _u8CecResponse= 0xA2; //for LG, CP_READ_HDMI_CEC_DATA(0XA2)
    }

    return IRQ_HANDLED;
}

void _MDrv_InitVariable(void)
{
    U8 i, j;

    _CECInfo.CecFifoIdxS = 0;
    _CECInfo.CecFifoIdxE = 0;
    _CECInfo.bCecMsgCnt = 0;
    _CECInfo.fCecInitFinish = FALSE;
    _CECInfo.MyLogicalAddress = E_LA_TV;       //TV
    _CECInfo.MyPhysicalAddress[0] = 0x00;      //default (0,0,0,0) for TV
    _CECInfo.MyPhysicalAddress[1] = 0x00;
    _CECInfo.MyDeviceType = E_DEVICE_TYPE_TV;    //TV device
    _CECInfo.MyPowerStatus  = E_MSG_PWRSTA_STANDBY2ON;

    _CECInfo.ActiveLogicalAddress = 0;
    _CECInfo.ActiveDeviceType = E_DEVICE_TYPE_RESERVED;
    _CECInfo.ActivePowerStatus = E_MSG_PWRSTA_STANDBY;
    _CECInfo.ActivePhysicalAddress[0] = 0x00;
    _CECInfo.ActivePhysicalAddress[1] = 0x00;
    _CECInfo.ActiveDeviceCECVersion = CEC_VERSION_13a;

    for(i=0;i<CEC_FIFO_CNT;i++)
    {
        _CECInfo.CecRxBuf[i].ucLength = 0;
        for(j=0;j<16;j++)
            _CECInfo.CecRxBuf[i].tRxData[j]= 0;
    }
    for(i=0;i<15;i++)
    {
        _CECInfo.CecDevicesExisted[i] = FALSE;
    }
}

U8 _MDrv_CecSendFrame(U8 header, U8 opcode, U8* operand, U8 len)
{
    U8 i, cnt, *ptr, res;

    // clear CEC TX INT status
    CEC_REG_8(REG_HDMI_INT_CLEAR + 1) = 0x0E; //for REG_HDMI_INT_CLEAR high byte
    CEC_REG_8(REG_HDMI_INT_CLEAR + 1) = 0x00; //for REG_HDMI_INT_CLEAR high byte

    CEC_REG_8(REG_HDMI_CEC_TX_DATA0) = header; //for REG_HDMI_CEC_TX_DATA0 low byte
    CEC_REG_8(REG_HDMI_CEC_TX_DATA0 + 1) = opcode; //for REG_HDMI_CEC_TX_DATA0 high byte

//    CEC_PRINT("\r\n/********  CEC Tx **********/\r\n");
//    CEC_PRINT("CEC Tx FIFO= 0x%2x\n", (U8)header);
//    CEC_PRINT("opcode= 0x%2x\n", (U8)opcode);

    ptr=operand;
    for(i=0;i<len;i++)
    {
        CEC_REG_8(REG_HDMI_CEC_TX_DATA1 + ((i*2) - (i%2)))= *(ptr+i); //???
//        CEC_PRINT(" operand=0x%2x\n", *(operand+i));
    }
//    MICOM_PRINT("\r\n/**************************/\r\n");

    // CEC transmit length
    if(header==_MDrv_HeaderSwap(header))
        CEC_REG_8(REG_HDMI_CEC_CONFIG2) = 0;                  //polling message
    else
        CEC_REG_8(REG_HDMI_CEC_CONFIG2) = (len+1);

    //The total time,
    //(1). successful, 4.5 ms + 10 * 2.4 ms * N = 4.5 ms + 24 * N
    //              = 28.5 ms (1), or 52.5 ms (2), ....
    //(2). NAK,        (4.5 ms + 10 * 2.4 ms) * 1 + (4.5 ms + 10 * 2.4 ms +7.2 ms(3 bit time)) * retry (3)
    //              = 28.5 + 35.2 * 3 = 133.6 ms

    cnt=0;

    do
    {
        msleep(2);

        if(cnt++>=100)
            break;
    } while((CEC_REG_8(REG_HDMI_INT_STATUS + 1) & 0x0E) == 0); //for REG_HDMI_INT_STATUS high byte

    res = (CEC_REG_8(REG_HDMI_INT_STATUS + 1) & 0x0E) ;

    if(cnt>=100)
        res |= E_CEC_SYSTEM_BUSY;

    // clear CEC TX INT status
    CEC_REG_8(REG_HDMI_INT_CLEAR + 1) = 0x0E;     //for REG_HDMI_INT_CLEAR high byte
    CEC_REG_8(REG_HDMI_INT_CLEAR + 1) = 0x00;     //for REG_HDMI_INT_CLEAR high byte

    return res;
}

void _MDrv_CecFindMyAddress(void)
{
    MDrv_WriteByteMask((REG_HDMI_CEC_CONFIG4), _CECInfo.MyLogicalAddress<<4,  0xF0);    //for REG_HDMI_CEC_CONFIG3 high byte
    CEC_PRINT("\r\nMy logical address=%2x\r\n", _CECInfo.MyLogicalAddress);
}

U8 _MDrv_HeaderSwap(U8 value)
{
    return(((value&0x0f)<<4)+((value&0xf0)>>4));
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
static unsigned int u32AESDMAIntRegister;
void MDrv_CEC_Init(void)
{
    U16 reg_val;
    int result;

    _MDrv_InitVariable();

    // Adcdvi irq clear control.
    CEC_REG_8(ADC_ATOP_REG_BASE + (0x71<<2) + 1) = 0xFF;
    CEC_REG_8(ADC_ATOP_REG_BASE + (0x71<<2) + 1) = 0;
    // Adcdvi irq mask control - disable all adcdvi irq.
    CEC_REG_8(ADC_ATOP_REG_BASE + (0x70<<2) + 1) = 0xFF;

    // HDMI irq clear
    CEC_REG(REG_HDMI_INT_CLEAR) = 0xFFFF;
    CEC_REG(REG_HDMI_INT_CLEAR) = 0;
    // HDMI irq mask control -only enable CEC rx irq
    // LGE earnest 2009/01/24 		Tuner init time is slower because of this interrupt in PDP. We must move init routine after starting HDMI.
    CEC_REG(REG_HDMI_INT_MASK) = 0xFEFF; //thchen 20090117 //MICOM_REG(REG_HDMI_INT_MASK) = 0xFEFF;

    // CPU irq mask
    //CEC_REG_8(EX0_INT_MASK + 0x35) = 0x7F;   //thchen 20090117

    // CEC pad
    //reg_val=CEC_REG(CHIP_REG_BASE + (0x50<<2));
    // Neptune
    //CEC_REG(CHIP_REG_BASE + (0x50<<2)) = ((reg_val & 0xE7FF) | BIT11); // BIT12:BIT11=01
    //CEC_REG(CHIP_REG_BASE + (0x50<<2)) = (reg_val | BIT11 | BIT12); // BIT12:BIT11=11
    //CEC_REG(CHIP_REG_BASE + (0x50<<2)) = (reg_val | BIT11 & ~BIT12); // BIT12:BIT11=10
    //CEC_REG(CHIP_REG_BASE + (0x50<<2)) = (reg_val & ~BIT11 & ~BIT12); // BIT12:BIT11=00

	//printk("=================\n");
	//printk("=======CEC INIT 2======\n");
	//printk("=================\n");
    // CEC Clock not gating
    reg_val=CEC_REG_8(REG_HDMI_CEC_CONFIG3);
    CEC_REG_8(REG_HDMI_CEC_CONFIG3) = reg_val& ~BIT7;

    reg_val=(MST_XTAL_CLOCK_HZ%100000l)*0.00016+0.5;
    CEC_REG(REG_HDMI_CEC_CONFIG4 + 1) = ((reg_val<<8)+MST_XTAL_CLOCK_HZ/100000l);
    // Aaron Lin
    //test
    //CEC_REG_8(REG_HDMI_CEC_CONFIG4 + 4) = (((reg_val<<8)+MST_XTAL_CLOCK_HZ/100000l)>>8);

    //reg_val=MICOM_REG_8(REG_HDMI_CEC_CONFIG2 + 1)&(~0x07);
    //CEC_REG_8(REG_HDMI_CEC_CONFIG2 + 1) = (reg_val|RETRY_CNT);
    CEC_REG_8(REG_HDMI_CEC_CONFIG2 + 1) = (0x10|RETRY_CNT);
    //reg_val=(FrameInterval<<8)|(BusFreeTime<<4)|(ReTxInterval);
    // Aaron Lin
    //CEC_REG_8(REG_HDMI_CEC_CONFIG3 + 1) = reg_val;
    CEC_REG_8(REG_HDMI_CEC_CONFIG3 + 1) = (BusFreeTime<<4)|(ReTxInterval);
	CEC_REG_8(REG_HDMI_CEC_CONFIG4) = FrameInterval;
    //for test
    //CEC_REG_8(REG_HDMI_CEC_CONFIG3 + 4) = (reg_val>>8);
    CEC_REG_8(REG_HDMI_CEC_STATUS1) = 0x07;

    _MDrv_CecFindMyAddress();       //logical address for this TV.
	//printk("LOW : 0x%x\n",CEC_REG_8(REG_HDMI_CEC_CONFIG3 + 1));
	//printk("HIGH : 0x%x\n",CEC_REG_8(REG_HDMI_CEC_CONFIG4));
	//printk("CEC status : 0x%x\n",CEC_REG_8(REG_HDMI_CEC_STATUS1));
    _CECInfo.fCecInitFinish = TRUE;
    _CECInfo.MyPowerStatus  = E_MSG_PWRSTA_ON;
    CEC_PRINT("Complete CEC Initiation!!\n",0);

    if(0 == u32AESDMAIntRegister) {
    //start HDMI ISR
    result = request_irq(E_IRQH_EXP_HDMI, _MDrv_HDMI_ISR, SA_INTERRUPT, "HDMI", NULL);
	    if(0 != result) {
        printk("[CEC] request IRQ %d failed\n", E_IRQH_EXP_HDMI);
			return -EBUSY;
    }

	    u32AESDMAIntRegister = 1;
	} else {
		disable_irq(E_IRQH_EXP_HDMI);
    	enable_irq(E_IRQH_EXP_HDMI);
    }

    //enable_irq(E_FIQH_HDCP);
    // CPU irq mask
    //CEC_REG_8(EX0_INT_MASK + 0x35) = 0x7F;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_CEC_ChkDevs()
//  [Description]
//                  Driver layer: Use to check the existed CEC devices currently
//  [Arguments]:
//
//  [Return]:
//
//**************************************************************************
void MDrv_CEC_ChkDevs(CEC_INFO_LIST_t *pCEC_Info)
{
    U8 i, res;

    CEC_PRINT("\r\n Existed CEC device \r\n", 0);
    for (i=E_LA_TV; i<E_LA_UNREGISTERED; i++)
    {
        res = _MDrv_CecSendFrame( ((_CECInfo.MyLogicalAddress<<4)&0xF0) |(i&0x0F), 0x00, &i, 0);
        if(res&E_CEC_TX_SUCCESS)
        {
            _CECInfo.CecDevicesExisted[i] = TRUE;
            _CECInfo.ActiveLogicalAddress = i;
            CEC_PRINT("\r\n DEVICE ID= %2x \r\n", i);
        }
        else
            _CECInfo.CecDevicesExisted[i] = FALSE;
    }

    *pCEC_Info = _CECInfo;
}

//**************************************************************************
//  [Function Name]:
//                   MDrv_CEC_RxChkBuf()
//  [Description]
//                   Driver layer: Use to retrieve CEC message and store into CEC Rx buffer
//  [Arguments]:
//
//  [Return]:
//
//**************************************************************************
void MDrv_CEC_RxChkBuf(void)  //don't place print message in this function
{
    U8 i;

    if(CEC_REG_8(REG_HDMI_INT_STATUS + 1) & BIT0)
    {

        _CECInfo.CecRxBuf[_CECInfo.CecFifoIdxE].ucLength = (CEC_REG_8(REG_HDMI_CEC_STATUS1 + 1)& 0x0F)+1;

        if(_CECInfo.CecRxBuf[_CECInfo.CecFifoIdxE].ucLength>1)            //1: polling message, not needed to handle
        {
            for(i = 0 ; i < _CECInfo.CecRxBuf[_CECInfo.CecFifoIdxE].ucLength ; i++)
            {
                _CECInfo.CecRxBuf[_CECInfo.CecFifoIdxE].tRxData[i] = CEC_REG_8( (REG_HDMI_CEC_RX_DATA0) + ((i*2) - (i%2)) );
            }
            if( _CECInfo.CecRxBuf[_CECInfo.CecFifoIdxE].tRxData[0] != _MDrv_HeaderSwap(_CECInfo.CecRxBuf[_CECInfo.CecFifoIdxE].tRxData[0]) )
            {
                _CECInfo.bCecMsgCnt++;
                _CECInfo.CecFifoIdxE=((++_CECInfo.CecFifoIdxE)>=CEC_FIFO_CNT)? 0 : _CECInfo.CecFifoIdxE;
            }
        }

        // clear RX INT status
        CEC_REG_8(REG_HDMI_INT_CLEAR + 1) = BIT0;
        CEC_REG_8(REG_HDMI_INT_CLEAR + 1) = 0;
        // clear RX NACK status
        // Aaron Lin
        //CEC_REG_8(REG_HDMI_CEC_STATUS1) = BIT0;
        CEC_REG_8(REG_HDMI_CEC_STATUS1) = BIT2;
    }
}

void MDrv_CEC_RxApi(CEC_INFO_LIST_t *pCEC_Info)
{
    *pCEC_Info = _CECInfo;
    _u8CecResponse= 0xFF; //for LG, reset to 0xFF
}

//**************************************************************************
//  [Function Name]:
//                   MDrv_CEC_TxApi()
//  [Description]
//                   Driver layer: Use to send CEC message
//  [Arguments]:
//                   dst_address: destination logical address
//                   msg:            CEC message
//                   operand_ptr: message parameters
//                   len:              parameter length
//  [Return]:
//                   error_code: return status
//**************************************************************************
void MDrv_CEC_TxApi(CEC_TX_INFO_t* pCEC_TxInfo)
{
    U8 res, header;
    CEC_DEVICELA dst_address = pCEC_TxInfo->Dst_Addr;
    CEC_MSGLIST msg = pCEC_TxInfo->Msg;
    U8* operand_ptr = pCEC_TxInfo->Operand;
    U8 len = pCEC_TxInfo->Len;
    _ErrorCode = E_CEC_FEATURE_ABORT;

	if( pCEC_TxInfo->u8Cmd == CP_WRITE_CECM_READY )
	{
		return;
	}
	else if( pCEC_TxInfo->u8Cmd == CP_WRITE_CECM_SETMODE)
	{
		if(dst_address == 1)
		{
		//	printk("==== CEC Enable\n",0);
			_u8CecEnable = 1;
		}
		else
		{
		//	printk("==== CEC Disable\n",0);
			_u8CecEnable = 0;
		}

		return;
	}

	if(!_u8CecEnable)
		return;

	//printk("==== send cec data ==== \n",0);

    header = ((_CECInfo.MyLogicalAddress<<4)&0xF0)|(dst_address&0x0F);

    res=_MDrv_CecSendFrame(header, msg, operand_ptr, len)+1;

    if(res&E_CEC_TX_SUCCESS)
        _ErrorCode = E_CEC_TX_SUCCESS;
    else if(res&E_CEC_RF)
        _ErrorCode = E_CEC_RF;
    else if(res&E_CEC_LOST_ABT)
        _ErrorCode = E_CEC_LOST_ABT;
    else if(res&E_CEC_SYSTEM_BUSY)
        _ErrorCode = E_CEC_SYSTEM_BUSY;

    	_u8CecResponse2= 0xA3; //for LG, CP_SLAVTX_CEC_RESLUT(0XA3)
    //return error_code;
    //CEC_PRINT("MDrv_MICOM_CecTxApi ErrorCode =%2x\n", _ErrorCode);
    #if 1 //080627_yongs
	_u8MsgID = pCEC_TxInfo->u8MsgID;
	if(_ErrorCode == E_CEC_TX_SUCCESS && dst_address != 0x0F)
	{
		//printk("k:ack \n",0);
		_u8AckResult = 1;
	}
	else
	{
		//printk("k:no ack \n",0);
		_u8AckResult = 0;
	}
	#else
	pCEC_TxInfo->CecMsg = _ErrorCode;
	#endif

   // pCEC_TxInfo->ErrorCode = _ErrorCode; //080627_yongs
}

void MDrv_CEC_GetResult(CEC_TX_INFO_t* pCEC_TxInfo)
{
	#if 1 //080627_yongs
	pCEC_TxInfo->ErrorCode= _u8AckResult;
	#else
	pCEC_TxInfo->CecMsg = _ErrorCode;
	#endif
	pCEC_TxInfo->u8MsgID = _u8MsgID;

	_u8CecResponse2= 0xFF; //for LG, reset to 0xFF
}

U16 MDrv_CEC_Response(void)
{
	U16 _u16CecResponse = ((U16)(_u8CecResponse << 8) & 0xff00 ) | _u8CecResponse2 ;
	return _u16CecResponse;

}

