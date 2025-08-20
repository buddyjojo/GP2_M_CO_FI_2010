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
/*
#include <stdio.h>
#include "mreg51.h"
#include "sysinfo.h"
#include "hwreg.h"
#include "drvsys.h"
#include "debug.h"
#include "drvgpio.h"
#include "chip/compat.h"
#include "drvGlobal.h"
*/
//#define //ASSERT(expr)    __//ASSERT(expr)

//-----------------------------------------------------------------
// 8-bit I2C IO expander for GPIO
//-----------------------------------------------------------------
#if (I2C_IO_EXPANDER_TYPE != I2C_IO_EXPANDER_NONE)

//#include "drviic.h"

#ifndef I2C_IO_EXPANDER_P0_IS_GPIO
    #define I2C_IO_EXPANDER_P0_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P1_IS_GPIO
    #define I2C_IO_EXPANDER_P1_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P2_IS_GPIO
    #define I2C_IO_EXPANDER_P2_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P3_IS_GPIO
    #define I2C_IO_EXPANDER_P3_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P4_IS_GPIO
    #define I2C_IO_EXPANDER_P4_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P5_IS_GPIO
    #define I2C_IO_EXPANDER_P5_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P6_IS_GPIO
    #define I2C_IO_EXPANDER_P6_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P7_IS_GPIO
    #define I2C_IO_EXPANDER_P7_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P8_IS_GPIO
    #define I2C_IO_EXPANDER_P9_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P8_IS_GPIO
    #define I2C_IO_EXPANDER_P9_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P9_IS_GPIO
    #define I2C_IO_EXPANDER_P9_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P10_IS_GPIO
    #define I2C_IO_EXPANDER_P10_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P11_IS_GPIO
    #define I2C_IO_EXPANDER_P11_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P12_IS_GPIO
    #define I2C_IO_EXPANDER_P12_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P13_IS_GPIO
    #define I2C_IO_EXPANDER_P13_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P14_IS_GPIO
    #define I2C_IO_EXPANDER_P14_IS_GPIO 0
#endif
#ifndef I2C_IO_EXPANDER_P15_IS_GPIO
    #define I2C_IO_EXPANDER_P15_IS_GPIO 0
#endif

#if 0
#define I2C_IO_OEN  ((I2C_IO_EXPANDER_P0_IS_GPIO <= GPIO_IN ? BIT0 : 0) | \
                     (I2C_IO_EXPANDER_P1_IS_GPIO <= GPIO_IN ? BIT1 : 0) | \
                     (I2C_IO_EXPANDER_P2_IS_GPIO <= GPIO_IN ? BIT2 : 0) | \
                     (I2C_IO_EXPANDER_P3_IS_GPIO <= GPIO_IN ? BIT3 : 0) | \
                     (I2C_IO_EXPANDER_P4_IS_GPIO <= GPIO_IN ? BIT4 : 0) | \
                     (I2C_IO_EXPANDER_P5_IS_GPIO <= GPIO_IN ? BIT5 : 0) | \
                     (I2C_IO_EXPANDER_P6_IS_GPIO <= GPIO_IN ? BIT6 : 0) | \
                     (I2C_IO_EXPANDER_P7_IS_GPIO <= GPIO_IN ? BIT7 : 0))

#define I2C_IO_OUT  ((I2C_IO_EXPANDER_P0_IS_GPIO == GPIO_OUT_HIGH ? BIT0 : 0) | \
                     (I2C_IO_EXPANDER_P1_IS_GPIO == GPIO_OUT_HIGH ? BIT1 : 0) | \
                     (I2C_IO_EXPANDER_P2_IS_GPIO == GPIO_OUT_HIGH ? BIT2 : 0) | \
                     (I2C_IO_EXPANDER_P3_IS_GPIO == GPIO_OUT_HIGH ? BIT3 : 0) | \
                     (I2C_IO_EXPANDER_P4_IS_GPIO == GPIO_OUT_HIGH ? BIT4 : 0) | \
                     (I2C_IO_EXPANDER_P5_IS_GPIO == GPIO_OUT_HIGH ? BIT5 : 0) | \
                     (I2C_IO_EXPANDER_P6_IS_GPIO == GPIO_OUT_HIGH ? BIT6 : 0) | \
                     (I2C_IO_EXPANDER_P7_IS_GPIO == GPIO_OUT_HIGH ? BIT7 : 0))
#endif

#define I2C_IO_OEN  1
#define I2C_IO_OUT  0
#define EXPAND_CH_ID  10


extern S32 MDrv_SW_IIC_Write(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf);
extern S32 MDrv_SW_IIC_Read(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf);

#if 1
B16 MDrv_IIC_WriteByte ( U8 u8SlaveID, U8 u8RegAddr, U8 u8Data )
{
/*
    IIC_Start();

    if(IIC_SendByte(u8SlaveID&0xFE)==FALSE)
        return FALSE;

    if(IIC_SendByte(u8RegAddr)==FALSE)
        return FALSE;

    if(IIC_SendByte(u8Data)==FALSE)
        return FALSE;

    IIC_Stop();
*/
    //MDrv_SW_IIC_Write(EXPAND_CH_ID, u8SlaveID, 0, 0, u8RegAddr, &u8Data);
    MDrv_SW_IIC_Write( EXPAND_CH_ID,u8SlaveID, 0, NULL, u8RegAddr, &u8Data );

    return TRUE;
}

B16 MDrv_IIC_ReadByte ( U8 u8SlaveID, U8 u8RegAddr, U8 *pu8Data )
{
    //MDrv_SW_IIC_Read( EXPAND_CH_ID,u8SlaveID, 0, NULL, u8RegAddr, &pu8Data );
    return TRUE;
}


#endif
#if (I2C_IO_EXPANDER_TYPE == I2C_IO_EXPANDER_PCA9557)
//-----------------------------------------------------------------
// PCA9557
// 8-bit I2C and SMBus I/O port with reset
//-----------------------------------------------------------------

//static unsigned char PCA9557_u8OUT = I2C_IO_OUT;
//static unsigned char PCA9557_u8OEN = I2C_IO_OEN;
static unsigned char PCA9557_u8OUT = 1;
static unsigned char PCA9557_u8OEN = 0;



void MDrv_ExGPIO_Init(void)
{
    U8 u8Loop;
//    U8 u8data;

    // Input Polarity Inversion
    for (u8Loop = 3; u8Loop > 0; u8Loop--)
    {
        //if (MDrv_IIC_WriteBytes(I2C_IO_EXPANDER_ADDR, 0, 0, 1, &PCF8574_u8OUT) != FALSE)
        if (MDrv_IIC_WriteByte(I2C_IO_EXPANDER_ADDR, 2, 0x00) != FALSE)
            break;
    }
    //ASSERT(u8Loop);

    // Initial Output Data
    for (u8Loop = 3; u8Loop > 0; u8Loop--)
        if (MDrv_IIC_WriteByte(I2C_IO_EXPANDER_ADDR, 1, I2C_IO_OUT) != FALSE)
            break;
    //ASSERT(u8Loop);

    // I/O direction
    for (u8Loop = 3; u8Loop > 0; u8Loop--)
        if (MDrv_IIC_WriteByte(I2C_IO_EXPANDER_ADDR, 3, I2C_IO_OEN) != FALSE)
            break;
    //ASSERT(u8Loop);
}

void MDrv_ExGPIO_Dir(U8 u8BitMask, B16 bIsInput)
{
    U8 u8Loop;
/*
    if (bIsInput)
        PCA9557_u8OEN |= u8BitMask;
    else
        PCA9557_u8OEN &= ~u8BitMask;
*/
    for (u8Loop = 3; u8Loop > 0; u8Loop--)
        if (MDrv_IIC_WriteByte(I2C_IO_EXPANDER_ADDR, 3, PCA9557_u8OEN) != FALSE)
            break;
    //ASSERT(u8Loop);
}

void MDrv_ExGPIO_Set(U8 u8BitMask, B16 bHigh)
{
    U8 u8Loop;
/*
    if (bHigh)
        PCA9557_u8OUT |= u8BitMask;
    else
        PCA9557_u8OUT &= ~u8BitMask;
*/
    for (u8Loop = 3; u8Loop > 0; u8Loop--)
        if (MDrv_IIC_WriteByte(I2C_IO_EXPANDER_ADDR, 1, PCA9557_u8OUT) != FALSE)
            break;
    //ASSERT(u8Loop);
}
/*
BIT MDrv_ExGPIO_Get(U8 u8BitMask)
{
    U8 u8Loop;
    U8 u8Input;

    u8Input = 0;

    for (u8Loop = 3; u8Loop > 0; u8Loop--)
        if (MDrv_IIC_ReadByte(I2C_IO_EXPANDER_ADDR, 0, &u8Input) != FALSE)
            break;
    ASSERT(u8Loop);

    return (BIT)(u8Input & u8BitMask);
}
*/

void MDrv_ExGPIO_Get(U8 u8BitMask)
{
    U8 u8Loop;
    U8 u8Input;

    u8Input = 0;

    for (u8Loop = 3; u8Loop > 0; u8Loop--)
        if (MDrv_IIC_ReadByte(I2C_IO_EXPANDER_ADDR, 0, &u8Input) != FALSE)
            break;
    //ASSERT(u8Loop);

   // return (BIT)(u8Input & u8BitMask);
}


#elif (I2C_IO_EXPANDER_TYPE == I2C_IO_EXPANDER_PCF8574)
//-----------------------------------------------------------------
// PCF8574
// Remote 8-bit I/O expander for I2C-bus
//-----------------------------------------------------------------

static U8 PCF8574_u8OUT = I2C_IO_OUT;

void MDrv_ExGPIO_Init(void)
{
    U8 u8Loop;

    for (u8Loop = 3; u8Loop > 0; u8Loop--)
    {
        if (MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 0, 0, 1, &PCF8574_u8OUT) != FALSE)
        {
            break;
        }
    }
    //ASSERT(u8Loop);
}

void MDrv_ExGPIO_Dir(U8 u8BitMask, B16 bIsInput)
{
    // nothing to do
    UNUSED(u8BitMask);
    UNUSED(bIsInput);
}

void MDrv_ExGPIO_Set(U8 u8BitMask, B16 bHigh)
{
    U8 u8Loop;

    if (bHigh)
        PCF8574_u8OUT |= u8BitMask;
    else
        PCF8574_u8OUT &= ~u8BitMask;

    for (u8Loop = 3; u8Loop > 0; u8Loop--)
    {
        if (MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 0, 0, 1, &PCF8574_u8OUT) != FALSE)
        {
            break;
        }
    }
    //ASSERT(u8Loop);
}

BIT MDrv_ExGPIO_Get(U8 u8BitMask)
{
    U8 u8Loop;
    U8 u8Input;

    u8Input = 0;

    for (u8Loop = 3; u8Loop > 0; u8Loop--)
        if (MDrv_SW_IIC_Read(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 0, 0, 1, &u8Input) != FALSE)
            break;
    //ASSERT(u8Loop);

    return (BIT)(u8Input & u8BitMask);
}
#elif (I2C_IO_EXPANDER_TYPE == I2C_IO_EXPANDER_MSG1016RC) //Milly¢º

#define I2C_IO_OEN0  ((I2C_IO_EXPANDER_P0_IS_GPIO <= GPIO_IN ? BIT0 : 0) | \
                     (I2C_IO_EXPANDER_P1_IS_GPIO <= GPIO_IN ? BIT1 : 0) | \
                     (I2C_IO_EXPANDER_P2_IS_GPIO <= GPIO_IN ? BIT2 : 0) | \
                     (I2C_IO_EXPANDER_P3_IS_GPIO <= GPIO_IN ? BIT3 : 0) | \
                     (I2C_IO_EXPANDER_P4_IS_GPIO <= GPIO_IN ? BIT4 : 0) | \
                     (I2C_IO_EXPANDER_P5_IS_GPIO <= GPIO_IN ? BIT5 : 0) | \
                     (I2C_IO_EXPANDER_P6_IS_GPIO <= GPIO_IN ? BIT6 : 0) | \
                     (I2C_IO_EXPANDER_P7_IS_GPIO <= GPIO_IN ? BIT7 : 0))

#define I2C_IO_OEN1  ((I2C_IO_EXPANDER_P8_IS_GPIO <= GPIO_IN ? BIT0 : 0) | \
                     (I2C_IO_EXPANDER_P9_IS_GPIO <= GPIO_IN ? BIT1 : 0) | \
                     (I2C_IO_EXPANDER_P10_IS_GPIO <= GPIO_IN ? BIT2 : 0) | \
                     (I2C_IO_EXPANDER_P11_IS_GPIO <= GPIO_IN ? BIT3 : 0) | \
                     (I2C_IO_EXPANDER_P12_IS_GPIO <= GPIO_IN ? BIT4 : 0) | \
                     (I2C_IO_EXPANDER_P13_IS_GPIO <= GPIO_IN ? BIT5 : 0) | \
                     (I2C_IO_EXPANDER_P14_IS_GPIO <= GPIO_IN ? BIT6 : 0) | \
                     (I2C_IO_EXPANDER_P15_IS_GPIO <= GPIO_IN ? BIT7 : 0))


#define I2C_IO_OUT0  ((I2C_IO_EXPANDER_P0_IS_GPIO == GPIO_OUT_HIGH ? BIT0 : 0) | \
                                  (I2C_IO_EXPANDER_P1_IS_GPIO == GPIO_OUT_HIGH ? BIT1 : 0) | \
                                  (I2C_IO_EXPANDER_P2_IS_GPIO == GPIO_OUT_HIGH ? BIT2 : 0) | \
                                  (I2C_IO_EXPANDER_P3_IS_GPIO == GPIO_OUT_HIGH ? BIT3 : 0) | \
                                  (I2C_IO_EXPANDER_P4_IS_GPIO == GPIO_OUT_HIGH ? BIT4 : 0) | \
                                  (I2C_IO_EXPANDER_P5_IS_GPIO == GPIO_OUT_HIGH ? BIT5 : 0) | \
                                  (I2C_IO_EXPANDER_P6_IS_GPIO == GPIO_OUT_HIGH ? BIT6 : 0) | \
                                  (I2C_IO_EXPANDER_P7_IS_GPIO == GPIO_OUT_HIGH ? BIT7 : 0) )

#define I2C_IO_OUT1  ((I2C_IO_EXPANDER_P8_IS_GPIO == GPIO_OUT_HIGH ? BIT0 : 0) | \
                                  (I2C_IO_EXPANDER_P9_IS_GPIO == GPIO_OUT_HIGH ? BIT1 : 0) | \
                                  (I2C_IO_EXPANDER_P10_IS_GPIO == GPIO_OUT_HIGH ? BIT2 : 0) | \
                                  (I2C_IO_EXPANDER_P11_IS_GPIO == GPIO_OUT_HIGH ? BIT3 : 0) | \
                                  (I2C_IO_EXPANDER_P12_IS_GPIO == GPIO_OUT_HIGH ? BIT4 : 0) | \
                                  (I2C_IO_EXPANDER_P13_IS_GPIO == GPIO_OUT_HIGH ? BIT5 : 0) | \
                                  (I2C_IO_EXPANDER_P14_IS_GPIO == GPIO_OUT_HIGH ? BIT6 : 0) | \
                                  (I2C_IO_EXPANDER_P15_IS_GPIO == GPIO_OUT_HIGH ? BIT7 : 0) )


//static U8 MSG1016RC_u8OUT0 = I2C_IO_OUT0;
//static U8 MSG1016RC_u8OUT1 = I2C_IO_OUT1;



void MDrv_ExGPIO_Init(void)
{
    //U8 u8Loop;

    U8 cIOExpander_cPWM[2];
    U8  cIOExpanderData[2];
    U8  cIOExpanderData_1[2];
    U8  cIOExpanderData_2[2];
    U8  cIOExpander_WReg_Addr0[3] = {0x10, 0x10,0x09};
    U8  cIOExpander_WReg_Addr1[3] = {0x10, 0x10,0x0A};
    U8  cIOExpander_CReg_Addr0[3]  = {0x10, 0x10,0x04};
    U8  cIOExpander_CReg_Addr1[3]  = {0x10, 0x10,0x05};
    U8  cIOExpander_dummy[3]         = {0x10, 0x10,0x01};
    U8  cIOExpander_Init_data[5]      = {0x53, 0x45, 0x52, 0x44, 0x42};
    U8 cIOExpander_PWM_Switch[3]  = {0x10, 0x10,0x03};

    //U8 WriteData0 , WriteData1;
    U8 ReadData[1] =0;

#if 0
    printf("I2C_IO_OUT0  = %bx\n", (U8) I2C_IO_OUT0);
    printf("I2C_IO_OUT1  = %bx\n", (U8) I2C_IO_OUT1);
    printf("I2C_IO_OEN0  = %bx\n",  (U8)I2C_IO_OEN0);
    printf("I2C_IO_OEN1  = %bx\n",  (U8)I2C_IO_OEN1);
#endif

    cIOExpander_cPWM[1] = 0x00;
    cIOExpander_cPWM[0] = 0x00;// reset

    cIOExpanderData_1[1] = 0x00;
    cIOExpanderData_1[0] = 0x37;

    cIOExpanderData_2[1] = 0x00;
    cIOExpanderData_2[0] = 0x35;


    // Disable I2C SERDB
    cIOExpanderData[1] = 0x00;
    cIOExpanderData[0] = 0x45;
    MDrv_SW_IIC_Write( EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 0, NULL, 1, cIOExpanderData );
    MDrv_SW_IIC_Write( EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR,  0, NULL, 5, cIOExpander_Init_data); // SERDB
    MDrv_SW_IIC_Write( EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 0, NULL, 1, cIOExpanderData_1 );    // use bus
    MDrv_SW_IIC_Write( EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 0, NULL, 1, cIOExpanderData_2 );    // wait cpu. stop microm in ioexpander

    cIOExpanderData[1] = 0x00;
    cIOExpanderData[0] = 0x00;

  //$ Initial Output Data
  // dummy
  MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 3, cIOExpander_dummy, 1, cIOExpanderData);
  //reset
  MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 3, cIOExpander_PWM_Switch, 1, cIOExpander_cPWM );
  //Switch to pwm
  cIOExpander_cPWM[1] = 0x00;
  cIOExpander_cPWM[0] = 0x41;
  MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 3, cIOExpander_PWM_Switch, 1, cIOExpander_cPWM );

  //(1)
  cIOExpanderData[1] = 0x00;
  cIOExpanderData[0] = (U8)I2C_IO_OUT0;
  MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR, 3, cIOExpander_WReg_Addr0, 1, cIOExpanderData);

  //(2)
  cIOExpanderData[1] = 0x00;
  cIOExpanderData[0] = (U8)I2C_IO_OUT1;
  MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR,3, cIOExpander_WReg_Addr1, 1, cIOExpanderData);

  //$ I/O direction
  //(1)
  cIOExpanderData[1] = 0x00;
  cIOExpanderData[0] = (U8 )I2C_IO_OEN0;
  MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR,3, &cIOExpander_CReg_Addr0, 1, cIOExpanderData);

  //(2)
  cIOExpanderData[1] = 0x00;
  cIOExpanderData[0] = (U8)I2C_IO_OEN1;
  MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR,3, &cIOExpander_CReg_Addr1, 1, cIOExpanderData);

}



void MDrv_ExGPIO1_Set(U8 u8BitMask, BOOLEAN bHigh)
{
    U8 cIOExpander_WReg_Addr1[3] = {0x10, 0x10,0x0A};
    U8 MSG1016RC_u8OUT                = I2C_IO_OUT1 ;

    if (bHigh)
        MSG1016RC_u8OUT |= u8BitMask;
    else
        MSG1016RC_u8OUT &= ~u8BitMask;

    //printf("GPUO_SET:out1 = %bx\n", MSG1016RC_u8OUT);
    MDrv_SW_IIC_Write(EXPAND_CH_ID,I2C_IO_EXPANDER_ADDR,3,
            cIOExpander_WReg_Addr1, 1, &MSG1016RC_u8OUT);

}
#endif
#endif  // !I2C_GPIO_EXPANDER

#if 0//(EXT_SUBMCU_TYPE != EXT_SUBMCU_NONE)

#if (EXT_SUBMCU_TYPE == EXT_SUBMCU_MSTAR_UA01)

#include "drvUART.h"

#define MCU_HDR             0xA5    // command header
#define MCU_RSP             0xCA    // response header

#define MCU_CMD_SET_GPIO    0x01
#define MCU_CMD_GET_GPIO    0x02
#define MCU_CMD_IR          0x03
#define MCU_CMD_KEY         0x04
#define MCU_CMD_CEC         0x05
#define MCU_CMD_STANDBY     0x06

void MDrv_SubGPIO_Set(U8 u8GpioId, B16 bHigh)
{
    U8 u8CkSum;

    putcharUART1(MCU_HDR);
    putcharUART1(MCU_CMD_SET_GPIO);     u8CkSum = MCU_CMD_SET_GPIO;
    putcharUART1(u8GpioId);             u8CkSum += u8GpioId;
    putcharUART1(bHigh);                u8CkSum += bHigh;

    putcharUART1(~u8CkSum);
}

BIT  MDrv_SubGPIO_Get(U8 u8GpioId)
{
    U16 u16Loop;
    U8 u8CkSum;
    U8 u8Char;

    putcharUART1(MCU_HDR);
    putcharUART1(MCU_CMD_GET_GPIO);     u8CkSum = MCU_CMD_GET_GPIO;
    putcharUART1(u8GpioId);             u8CkSum += u8GpioId;
    putcharUART1(~u8CkSum);

    u16Loop = 500;

    while ((!UART_GetChar(&u8Char) || u8Char != MCU_RSP) && u16Loop--)
        MDrv_Timer_Delayms(1);
    while (!UART_GetChar(&u8Char) && u16Loop--)
        MDrv_Timer_Delayms(1);
    while (!UART_GetChar(&u8CkSum) && u16Loop--)
        MDrv_Timer_Delayms(1);

    // if ((u8Char + u8CkSum) != 0)   error

    return u8Char;

}

#endif

#endif  // (EXT_SUBMCU_TYPE != EXT_SUBMCU_NONE)
