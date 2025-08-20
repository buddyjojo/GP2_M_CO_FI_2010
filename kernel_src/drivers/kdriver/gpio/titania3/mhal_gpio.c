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

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <linux/autoconf.h>
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

#include "mhal_gpio.h"
#include "mhal_gpio_reg.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
U8 GPIO99=1;//1:PCM_GPIO0   0:NOR_GPIO15
U8 GPIO101=1;//1:PCM_GPIO2   0:NOR_GPIO14
U8 GPIO102=1;//1:PCM_GPIO3   0:NOR_GPIO13
U8 GPIO103=1;//1:PCM_GPIO4   0:NOR_GPIO12
U8 GPIO104=1;//1:PCM_GPIO5   0:NOR_GPIO11
U8 GPIO106=1;//1:PCM_GPIO7   0:NOR_GPIO10
U8 GPIO107=1;//1:PCM_GPIO8   0:NOR_GPIO9
U8 GPIO108=1;//1:PCM_GPIO9   0:NOR_GPIO8

U8 GPIO110=1;//1:PCM_GPIO11   0:NOR_GPIO7
U8 GPIO112=1;//1:PCM_GPIO13   0:NOR_GPIO6
U8 GPIO114=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO116=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO117=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO118=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO119=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO120=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO121=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO122=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO124=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO125=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO126=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO127=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO128=1;//1:PCM_GPIO15   0:NOR_GPIO5

U8 GPIO136=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO137=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO138=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO139=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO140=1;//1:PCM_GPIO15   0:NOR_GPIO5
U8 GPIO141=1;//1:PCM_GPIO15   0:NOR_GPIO5
//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//the functions of this section set to initialize
void MHal_GPIO_Init(void)
{
    MHal_GPIO_REG(REG_ALL_PAD_IN) &= ~BIT7;
}


void MHal_Test_In_Mode_Set(void)
{
    MHal_GPIO_REG(REG_TEST_MODE)  &= ~(BIT2|BIT1|BIT0);
}


void MHal_Test_Out_Mode_Set(void)
{
    MHal_GPIO_REG(REG_TEST_MODE)  &= ~(BIT6|BIT5|BIT4);
}

void MHal_DSPEJTAG_Mode_Set(void)
{
    MHal_GPIO_REG(REG_DSPEJTAG_MODE)  &= ~(BIT2|BIT1|BIT0);
}

void MHal_FOURTHUART_Mode_Set(void)
{
    MHal_GPIO_REG(REG_FOURTHUART_MODE)  &= ~(BIT7|BIT6);
}

//balup_090907
void MHal_THIRDUART_Mode_Set(void)
{
    MHal_GPIO_REG(REG_THIRDUART_MODE)  &= ~(BIT3|BIT2);
}

void MHal_PDTraceCtrl_Set(void)
{
    MHal_GPIO_REG(REG_PDTRACERCTRL_MODE)  &= ~(BIT1|BIT0);
}

void MHal_FASTUART_Mode_Set(void)
{
    MHal_GPIO_REG(REG_FASTUART_MODE)  &= ~(BIT6|BIT5|BIT4);
}
//end balup


void MHal_SAR_GPIO_Switch(void) //switch sar to gpio
{
    MHal_GPIO_REG(REG_SAR_GPIO) &= ~(BIT4|BIT3|BIT2|BIT1|BIT0);
}


void MHal_GPIO_PM_SPI_CZ_Set(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CZ_SET) |= BIT6;
}

void MHal_GPIO_PM_SPI_CK_Set(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CK_SET) |= BIT3;
}
void MHal_GPIO_PM_SPI_DI_Set(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DI_SET) |= BIT3;
}

void MHal_GPIO_PM_SPI_DO_Set(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DO_SET) |= BIT3;
}

void MHal_GPIO_IRIN_Set(void)
{
    MHal_GPIO_REG(REG_IRIN_SET) |= BIT4;
}

void MHal_GPIO_CEC_Set(void)
{
    MHal_GPIO_REG(REG_CEC_SET) |= BIT6;

}

void MHal_GPIO_GPIO_PM0_Set(void)
{
}

void MHal_GPIO_GPIO_PM1_Set(void)
{
}
void MHal_GPIO_GPIO_PM2_Set(void)
{
}

void MHal_GPIO_GPIO_PM3_Set(void)
{
}

void MHal_GPIO_GPIO_PM4_Set(void)
{

    MHal_GPIO_REG(REG_GPIO_PM4_SET1) = 0xbe;
    MHal_GPIO_REG(REG_GPIO_PM4_SET2) = 0xba;
}

void MHal_GPIO_GPIO_PM5_Set(void)
{
}

void MHal_GPIO_GPIO_PM6_Set(void)
{
    MHal_GPIO_REG(REG_GPIO_PM6_SET) |= BIT0;

}

void MHal_GPIO_GPIO_PM7_Set(void)
{
}

void MHal_GPIO_GPIO_PM8_Set(void)
{
}

void MHal_GPIO_GPIO_PM9_Set(void)
{
}

void MHal_GPIO_GPIO_PM10_Set(void)
{
    MHal_GPIO_REG(REG_GPIO_PM10_SET) &= ~BIT1;

}

void MHal_GPIO_GPIO_PM11_Set(void)
{
}

void MHal_GPIO_GPIO_PM12_Set(void)
{
}

void MHal_GPIO_HOTPLUGA_Set(void)
{
}


void MHal_GPIO_HOTPLUGB_Set(void)
{
}

void MHal_GPIO_HOTPLUGC_Set(void)
{
}

void MHal_GPIO_HOTPLUGD_Set(void)
{
}


void MHal_GPIO_DDCDA_CK_Set(void)
{
    MHal_GPIO_REG(REG_DDCDA_CK_SET) |= BIT7;

}

void MHal_GPIO_DDCDA_DA_Set(void)
{
    MHal_GPIO_REG(REG_DDCDA_DA_SET) |= BIT7;

}

void MHal_GPIO_DDCDB_CK_Set(void)
{
    MHal_GPIO_REG(REG_DDCDB_DA_SET) |= BIT7;

}

void MHal_GPIO_DDCDB_DA_Set(void)
{
    MHal_GPIO_REG(REG_DDCDB_CK_SET) |= BIT7;

}

void MHal_GPIO_DDCDC_CK_Set(void)
{
    MHal_GPIO_REG(REG_DDCDC_CK_SET) |= BIT7;
}

void MHal_GPIO_DDCDC_DA_Set(void)
{
    MHal_GPIO_REG(REG_DDCDC_DA_SET) |= BIT7;
}

void MHal_GPIO_DDCDD_CK_Set(void)
{
    MHal_GPIO_REG(REG_DDCDD_CK_SET) |= BIT7;
}

void MHal_GPIO_DDCDD_DA_Set(void)
{
    MHal_GPIO_REG(REG_DDCDD_DA_SET) |= BIT7;

}

void MHal_GPIO_SAR0_Set(void)
{
    MHal_SAR_GPIO_Switch();

    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();


}

void MHal_GPIO_SAR1_Set(void)
{
    MHal_SAR_GPIO_Switch();
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();

}

void MHal_GPIO_SAR2_Set(void)
{
    MHal_SAR_GPIO_Switch();
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();
}

void MHal_GPIO_SAR3_Set(void)
{
    MHal_SAR_GPIO_Switch();
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();
}

void MHal_GPIO_SAR4_Set(void)
{
    MHal_SAR_GPIO_Switch();
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();
}

void MHal_GPIO_GPIO0_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();
    MHal_FOURTHUART_Mode_Set();
    MHal_GPIO_REG(REG_GPIO0_SET4) &= ~BIT4;

}

void MHal_GPIO_GPIO1_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();
    MHal_FOURTHUART_Mode_Set();
    MHal_GPIO_REG(REG_GPIO0_SET4) &= ~BIT5;

}

void MHal_GPIO_GPIO2_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();

}

void MHal_GPIO_GPIO3_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();

}

void MHal_GPIO_GPIO4_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
}

void MHal_GPIO_GPIO5_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();

}

void MHal_GPIO_GPIO6_Set(void)
{

    MHal_GPIO_REG(REG_GPIO6_SET1) &= ~(BIT6|BIT5|BIT4|BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_GPIO6_SET2) &= ~BIT7;

}

void MHal_GPIO_GPIO7_Set(void)
{
    MHal_GPIO_REG(REG_GPIO7_SET1) &= ~(BIT6|BIT5|BIT4|BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_GPIO7_SET2) &= ~(BIT7|BIT6);

}

void MHal_GPIO_GPIO8_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
}

void MHal_GPIO_GPIO9_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
}

void MHal_GPIO_GPIO10_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_GPIO10_SET2) &= ~(BIT5|BIT4);

}

void MHal_GPIO_GPIO11_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_GPIO11_SET2) &= ~(BIT5|BIT4);

}

void MHal_GPIO_GPIO12_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();

    MHal_GPIO_REG(REG_GPIO12_SET3) &= ~(BIT3|BIT2);

}
void MHal_GPIO_GPIO13_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();

    MHal_GPIO_REG(REG_GPIO13_SET3) &= ~(BIT3|BIT2);

}
void MHal_GPIO_GPIO14_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_GPIO14_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_GPIO14_SET3) &= ~(BIT6);
}
void MHal_GPIO_GPIO15_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_GPIO14_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_GPIO14_SET3) &= ~(BIT6);

}
void MHal_GPIO_GPIO16_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PDTRACECTRL_SET) &= ~(BIT1|BIT0);

}
void MHal_GPIO_GPIO17_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PDTRACECTRL_SET) &= ~(BIT1|BIT0);

}
void MHal_GPIO_GPIO18_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PDTRACECTRL_SET) &= ~(BIT1|BIT0);

}

void MHal_GPIO_GPIO19_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PDTRACECTRL_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_GPIO20_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PDTRACECTRL_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_GPIO21_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PDTRACECTRL_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_GPIO22_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
}

void MHal_GPIO_GPIO23_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
}


void MHal_GPIO_GPIO24_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_GPIO24_SET2) &= ~(BIT3|BIT2);

}
void MHal_GPIO_GPIO25_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_GPIO25_SET2) &= ~(BIT3|BIT2);

}

void MHal_GPIO_GPIO26_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_GPIO26_SET2) &= ~(BIT3|BIT2);

}
void MHal_GPIO_GPIO27_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_GPIO27_SET2) &= ~(BIT3|BIT2);

}

void MHal_GPIO_UART_RX2_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();
    MHal_GPIO_REG(REG_UART_RX2_SET3) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_UART_RX2_SET4) &= ~(BIT0);

}
void MHal_GPIO_UART_TX2_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();
    MHal_GPIO_REG(REG_UART_TX2_SET3) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_UART_TX2_SET4) &= ~(BIT0);

}

void MHal_GPIO_PWM0_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PWM0_SET2) |= (BIT0);
}
void MHal_GPIO_PWM1_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PWM1_SET2) |= (BIT1);

}
void MHal_GPIO_PWM2_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PWM2_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_PWM2_SET3) |= (BIT2);
}


void MHal_GPIO_PWM3_Set(void)
{
    MHal_GPIO_REG(REG_PWM3_SET) |= (BIT3);
}


void MHal_GPIO_PWM4_Set(void)
{
    MHal_GPIO_REG(REG_PWM4_SET) |= (BIT4);
}

void MHal_GPIO_DDCR_DA_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_DDCR_DA_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_DDCR_DA_SET3) |= (BIT7);

}

void MHal_GPIO_DDCR_CK_Set(void)
{
    //MHal_Test_Out_Mode_Set();

    MHal_GPIO_REG(REG_DDCR_CK_SET1)  &= ~(BIT6|BIT5|BIT4);
    MHal_GPIO_REG(REG_DDCR_CK_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_DDCR_CK_SET3) |= (BIT7);

}


void MHal_GPIO_TGPIO0_Set(void)
{
    MHal_GPIO_REG(REG_TGPIO0_SET1) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_TGPIO0_SET2) &= ~(BIT6);;

}


void MHal_GPIO_TGPIO1_Set(void)
{
    MHal_GPIO_REG(REG_TGPIO1_SET) &= ~(BIT7|BIT6);

}


void MHal_GPIO_TGPIO2_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TGPIO2_SET2) &= ~(BIT1|BIT0);


}


void MHal_GPIO_TGPIO3_Set(void)
{
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TGPIO3_SET2) &= ~(BIT1|BIT0);


}


void MHal_GPIO_TS0_D0_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_TS0_D0_SET2) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_TS0_D0_SET3) &= ~(BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_TS0_D0_SET4) |= (BIT2);

}


void MHal_GPIO_TS0_D1_Set(void)
{
    MHal_GPIO_REG(REG_TS0_D1_SET) |= (BIT1);

}


void MHal_GPIO_TS0_D2_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_TS0_D2_SET2) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_TS0_D2_SET3) &= ~(BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_TS0_D2_SET4) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_TS0_D2_SET5) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_TS0_D2_SET6) |= (BIT1);

}


void MHal_GPIO_TS0_D3_Set(void)
{
    MHal_GPIO_REG(REG_TS0_D3_SET) |= (BIT1);

}
void MHal_GPIO_TS0_D4_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_TS0_D4_SET2) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_TS0_D4_SET3) &= ~(BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_TS0_D4_SET4) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_TS0_D4_SET5) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_TS0_D4_SET6) |= (BIT1);

}


void MHal_GPIO_TS0_D5_Set(void)
{
    MHal_GPIO_REG(REG_TS0_D5_SET) |= (BIT1);

}


void MHal_GPIO_TS0_D6_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_TS0_D6_SET2) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_TS0_D6_SET3) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_TS0_D6_SET4) |= (BIT1);
}



void MHal_GPIO_TS0_D7_Set(void)
{
    MHal_GPIO_REG(REG_TS0_D7_SET) |= (BIT1);

}


void MHal_GPIO_TS0_VLD_Set(void)
{

    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_TS0_VLD_SET2) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_TS0_VLD_SET3) |= (BIT2);
}


void MHal_GPIO_TS0_SYNC_Set(void)
{
    MHal_GPIO_REG(REG_TS0_SYNC_SET) |= (BIT2);
}



void MHal_GPIO_TS0_CLK_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_DSPEJTAG_Mode_Set();
    MHal_GPIO_REG(REG_TS0_CLK_SET3) |= (BIT2);

}

void MHal_GPIO_TS1_D0_Set(void)
{
    MHal_GPIO_REG(REG_TS1_D0_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_D0_SET2) |= (BIT3);

}
void MHal_GPIO_TS1_D1_Set(void)
{
    MHal_GPIO_REG(REG_TS1_D1_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_D1_SET2) |= (BIT3);

}


void MHal_GPIO_TS1_D2_Set(void)
{
    MHal_GPIO_REG(REG_TS1_D2_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_D2_SET2) |= (BIT3);

}

void MHal_GPIO_TS1_D3_Set(void)
{
    MHal_GPIO_REG(REG_TS1_D3_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_D3_SET2) |= (BIT3);

}

void MHal_GPIO_TS1_D4_Set(void)
{
    MHal_GPIO_REG(REG_TS1_D4_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_D4_SET2) |= (BIT3);

}


void MHal_GPIO_TS1_D5_Set(void)
{
    MHal_GPIO_REG(REG_TS1_D5_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_D5_SET2) |= (BIT3);

}


void MHal_GPIO_TS1_D6_Set(void)
{
    MHal_GPIO_REG(REG_TS1_D6_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_D6_SET2) |= (BIT3);

}


void MHal_GPIO_TS1_D7_Set(void)
{
    MHal_GPIO_REG(REG_TS1_D7_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_D7_SET2) |= (BIT3);

}


void MHal_GPIO_TS1_VLD_Set(void)
{
    MHal_GPIO_REG(REG_TS1_VLD_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_VLD_SET2) |= (BIT3);

}


void MHal_GPIO_TS1_SYNC_Set(void)
{
    MHal_GPIO_REG(REG_TS1_SYNC_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_SYNC_SET2) |= (BIT3);

}


void MHal_GPIO_TS1_CLK_Set(void)
{
    MHal_GPIO_REG(REG_TS1_CLK_SET1) &= ~(BIT4);
    MHal_GPIO_REG(REG_TS1_CLK_SET2) |= (BIT3);

}


void MHal_GPIO_PCM_A4_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_A4_SET2) &= ~(BIT4);

    if(GPIO99)
    {
        //PCM_GPIO0
        MHal_GPIO_REG(REG_PCM_A4_SET3) |= (BIT0);

    }
    else
    {
        //NOR_GPIO15
        MHal_GPIO_REG(REG_PCM_A4_SET3) &= ~(BIT0);
        MHal_GPIO_REG(REG_PCM_A4_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A4_SET5) |= (BIT3);



    }

}



void MHal_GPIO_PCM_WAIT_N_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_WAIT_N_SET2) |= (BIT0);

}
void MHal_GPIO_PCM_A5_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_A5_SET2) &= ~(BIT4);

    if(GPIO101)
    {
        //PCM_GPIO2
        MHal_GPIO_REG(REG_PCM_A5_SET3) |= (BIT1);

    }
    else
    {
        //NOR_GPIO14
        MHal_GPIO_REG(REG_PCM_A5_SET3) &= ~(BIT1);
        MHal_GPIO_REG(REG_PCM_A5_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A5_SET5) |= (BIT3);

    }

}


void MHal_GPIO_PCM_A6_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_A6_SET2) &= ~(BIT4);

    if(GPIO102)
    {
        //PCM_GPIO3
        MHal_GPIO_REG(REG_PCM_A6_SET3) |= (BIT1);

    }
    else
    {
        //NOR_GPIO13
        MHal_GPIO_REG(REG_PCM_A6_SET3) &= ~(BIT1);
        MHal_GPIO_REG(REG_PCM_A6_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A6_SET5) |= (BIT3);

    }

}



void MHal_GPIO_PCM_A7_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_A7_SET2) &= ~(BIT4);

    if(GPIO103)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A7_SET3) |= (BIT1);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A7_SET3) &= ~(BIT1);
        MHal_GPIO_REG(REG_PCM_A7_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A7_SET5) |= (BIT3);

    }

}


void MHal_GPIO_PCM_A12_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_A12_SET2) &= ~(BIT4);

    if(GPIO104)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A12_SET3) |= (BIT1);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A12_SET3) &= ~(BIT1);
        MHal_GPIO_REG(REG_PCM_A12_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A12_SET5) |= (BIT3);

    }

}



void MHal_GPIO_PCM_IRQA_N_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_IRQA_N_SET2) |= (BIT1);

}

void MHal_GPIO_PCM_A14_Set(void)
{
    MHal_Test_In_Mode_Set();

    if(GPIO106)
    {
        MHal_GPIO_REG(REG_PCM_A14_SET2) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A14_SET2) &= ~(BIT5);
        MHal_GPIO_REG(REG_PCM_A14_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A14_SET4) |= (BIT3);

    }

}


void MHal_GPIO_PCM_A13_Set(void)
{
    MHal_Test_In_Mode_Set();

    if(GPIO107)
    {
        MHal_GPIO_REG(REG_PCM_A13_SET2) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A13_SET2) &= ~(BIT5);
        MHal_GPIO_REG(REG_PCM_A13_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A13_SET4) |= (BIT3);

    }

}

void MHal_GPIO_PCM_A8_Set(void)
{
    MHal_Test_In_Mode_Set();

    if(GPIO108)
    {
        MHal_GPIO_REG(REG_PCM_A8_SET2) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_SET2) &= ~(BIT5);
        MHal_GPIO_REG(REG_PCM_A8_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A8_SET4) |= (BIT3);

    }

}



void MHal_GPIO_PCM_IOWR_N_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_IOWR_N_SET2) |= (BIT5);

}


void MHal_GPIO_PCM_A9_Set(void)
{
    MHal_Test_In_Mode_Set();

    if(GPIO110)
    {
        MHal_GPIO_REG(REG_PCM_A8_SET2) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_SET2) &= ~(BIT5);
        MHal_GPIO_REG(REG_PCM_A8_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A8_SET4) |= (BIT3);

    }

}



void MHal_GPIO_PCM_IORD_N_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_IORD_N_SET2) |= (BIT5);

}



void MHal_GPIO_PCM_A11_Set(void)
{
    MHal_Test_In_Mode_Set();

    if(GPIO112)
    {
        MHal_GPIO_REG(REG_PCM_A11_SET2) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A11_SET2) &= ~(BIT5);
        MHal_GPIO_REG(REG_PCM_A11_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A11_SET4) |= (BIT3);

    }

}


void MHal_GPIO_PCM_OE_N_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_OE_N_SET2) |= (BIT5);

}


void MHal_GPIO_PCM_A10_Set(void)
{
    MHal_Test_In_Mode_Set();

    if(GPIO114)
    {
        MHal_GPIO_REG(REG_PCM_A10_SET2) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A10_SET2) &= ~(BIT5);
        MHal_GPIO_REG(REG_PCM_A10_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A10_SET4) |= (BIT3);

    }


}
void MHal_GPIO_PCM_CE_N_Set(void)
{


    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_CE_N_SET2) &= ~(BIT3|BIT2);
    MHal_DSPEJTAG_Mode_Set();
    MHal_GPIO_REG(REG_PCM_CE_N_SET4) |= (BIT5);

}


void MHal_GPIO_PCM_D7_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_D7_SET2) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_PCM_D7_SET3) &= ~(BIT2|BIT1|BIT0);

    if(GPIO116)
    {
        MHal_GPIO_REG(REG_PCM_D7_SET4) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D7_SET4) &= ~(BIT5);
        MHal_GPIO_REG(REG_PCM_D7_SET5) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_D7_SET6) |= (BIT3);

    }

}


void MHal_GPIO_PCM_D6_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_D6_SET2) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_PCM_D6_SET3) &= ~(BIT2|BIT1|BIT0);

    if(GPIO117)
    {
        MHal_GPIO_REG(REG_PCM_D6_SET4) |= (BIT2);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D6_SET4) &= ~(BIT2);
        MHal_GPIO_REG(REG_PCM_D6_SET5) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_D6_SET6) |= (BIT3);

    }

}



void MHal_GPIO_PCM_D5_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM_D5_SET2) &= ~(BIT3|BIT2);
    MHal_GPIO_REG(REG_PCM_D5_SET3) &= ~(BIT2|BIT1|BIT0);

    if(GPIO118)
    {
        MHal_GPIO_REG(REG_PCM_D5_SET4) |= (BIT2);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D5_SET4) &= ~(BIT2);
        MHal_GPIO_REG(REG_PCM_D5_SET5) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_D5_SET6) |= (BIT3);

    }

}


void MHal_GPIO_PCM_D4_Set(void)
{
    MHal_Test_In_Mode_Set();

    if(GPIO119)
    {
        MHal_GPIO_REG(REG_PCM_D4_SET2) |= (BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D4_SET2) &= ~(BIT4);
        MHal_GPIO_REG(REG_PCM_D4_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_D4_SET4) |= (BIT3);

    }

}


void MHal_GPIO_PCM_D3_Set(void)
{
    MHal_Test_In_Mode_Set();

    if(GPIO120)
    {
        MHal_GPIO_REG(REG_PCM_D3_SET2) |= (BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D3_SET2) &= ~(BIT4);
        MHal_GPIO_REG(REG_PCM_D3_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_D3_SET4) |= (BIT3);

    }

}


void MHal_GPIO_PCM_A3_Set(void)
{
    MHal_GPIO_REG(REG_PCM_A3_SET1) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_PCM_A3_SET2) &= ~(BIT4);
    if(GPIO121)
    {
        MHal_GPIO_REG(REG_PCM_A3_SET3) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_SET3) &= ~(BIT3);
        MHal_GPIO_REG(REG_PCM_A3_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A3_SET5) |= (BIT3);

    }

}



void MHal_GPIO_PCM_A2_Set(void)
{

    MHal_Test_Out_Mode_Set();


    MHal_GPIO_REG(REG_PCM_A2_SET1) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_PCM_A2_SET2) &= ~(BIT4);
    MHal_GPIO_REG(REG_PCM_A2_SET5) &= ~(BIT1|BIT0);

    if(GPIO122)
    {
        MHal_GPIO_REG(REG_PCM_A2_SET3) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_SET3) &= ~(BIT3);
        MHal_GPIO_REG(REG_PCM_A3_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A3_SET5) |= (BIT3);

    }

}


void MHal_GPIO_PCM_REG_N_Set(void)
{
    MHal_GPIO_REG(REG_PCM_REG_N_SET) |= (BIT3);
}

void MHal_GPIO_PCM_A1_Set(void)
{

    MHal_Test_Out_Mode_Set();


    MHal_GPIO_REG(REG_PCM_A1_SET1) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_PCM_A1_SET2) &= ~(BIT4);
    MHal_GPIO_REG(REG_PCM_A1_SET5) &= ~(BIT1|BIT0);

    if(GPIO124)
    {
        MHal_GPIO_REG(REG_PCM_A1_SET3) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A1_SET3) &= ~(BIT3);
        MHal_GPIO_REG(REG_PCM_A1_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A1_SET5) |= (BIT3);

    }

}




void MHal_GPIO_PCM_A0_Set(void)
{


    MHal_GPIO_REG(REG_PCM_A0_SET1) &= ~(BIT4);

    if(GPIO125)
    {
        MHal_GPIO_REG(REG_PCM_A0_SET2) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A0_SET2) &= ~(BIT3);
        MHal_GPIO_REG(REG_PCM_A0_SET3) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_A0_SET4) |= (BIT3);

    }

}



void MHal_GPIO_PCM_D0_Set(void)
{

    MHal_Test_Out_Mode_Set();


    MHal_GPIO_REG(REG_PCM_D0_SET2) &= ~(BIT1|BIT0);

    if(GPIO126)
    {
        MHal_GPIO_REG(REG_PCM_A1_SET3) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A1_SET3) &= ~(BIT3);
        MHal_GPIO_REG(REG_PCM_A1_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_D0_SET2) |= (BIT3);

    }

}


void MHal_GPIO_PCM_D1_Set(void)
{

    MHal_GPIO_REG(REG_PCM_D1_SET1) |= (BIT3);

    if(GPIO127)
    {
        MHal_GPIO_REG(REG_PCM_D1_SET1) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D1_SET1) &= ~(BIT3);
        MHal_GPIO_REG(REG_PCM_D1_SET2) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_D1_SET3) |= (BIT3);

    }

}


void MHal_GPIO_PCM_D2_Set(void)
{

    MHal_Test_Out_Mode_Set();


    MHal_GPIO_REG(REG_PCM_D2_SET2) &= ~(BIT1|BIT0);

    if(GPIO128)
    {
        MHal_GPIO_REG(REG_PCM_D2_SET3) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D2_SET3) &= ~(BIT3);
        MHal_GPIO_REG(REG_PCM_D2_SET4) &= ~(BIT1|BIT0);
        MHal_GPIO_REG(REG_PCM_D2_SET2) |= (BIT3);

    }

}



void MHal_GPIO_PCM_RESET_Set(void)
{

    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_PCM_RESET_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_PCM_RESET_SET3) |= ~(BIT3|BIT2);

}



void MHal_GPIO_PCM_CD_N_Set(void)
{

    MHal_GPIO_REG(REG_PCM_CD_N_SET) |= ~(BIT3|BIT2);

}
void MHal_GPIO_PCM2_CE_N_Set(void)
{

    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PCM2_CE_N_SET2) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_PCM2_CE_N_SET3) |= (BIT5|BIT4);

}



void MHal_GPIO_PCM2_IRQA_N_Set(void)
{

    MHal_GPIO_REG(REG_PCM2_IRQA_N_SET1) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_PCM2_IRQA_N_SET2) |= (BIT7|BIT6);

}



void MHal_GPIO_PCM2_WAIT_N_Set(void)
{

    MHal_GPIO_REG(REG_PCM2_WAIT_N_SET) |= (BIT7|BIT6);

}



void MHal_GPIO_PCM2_RESET_Set(void)
{

    MHal_GPIO_REG(REG_PCM2_RESET_SET) |= (BIT7);
    MHal_GPIO_REG(REG_PCM2_RESET_SET) &= ~(BIT6);

}



void MHal_GPIO_PCM2_CD_N_Set(void)
{

    MHal_GPIO_REG(REG_PCM2_CD_N_SET) |= (BIT7);
    MHal_GPIO_REG(REG_PCM2_CD_N_SET) &= ~(BIT6);

}



void MHal_GPIO_PF_AD15_Set(void)
{

    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PF_AD15_SET2) &= ~(BIT4);
    MHal_GPIO_REG(REG_PF_AD15_SET3) &= ~(BIT1|BIT0);
    if(GPIO136)
    {
        MHal_GPIO_REG(REG_PF_AD15_SET3) |= (BIT3);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_AD15_SET3) &= ~(BIT3);
        MHal_GPIO_REG(REG_PF_AD15_SET3) |= (BIT3);

    }

}



void MHal_GPIO_PF_CE0Z_Set(void)
{

    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PF_CE0Z_SET2) &= ~(BIT4);
    MHal_GPIO_REG(REG_PF_CE0Z_SET3) &= ~(BIT1|BIT0);
    if(GPIO137)
    {
        MHal_GPIO_REG(REG_PF_CE0Z_SET3) |= (BIT3);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE0Z_SET3) &= ~(BIT3);
        MHal_GPIO_REG(REG_PF_CE0Z_SET3) |= (BIT3);

    }

}


void MHal_GPIO_PF_CE1Z_Set(void)
{

    MHal_GPIO_REG(REG_PF_CE1Z_SET2) &= ~(BIT4);
    if(GPIO138)
    {
        MHal_GPIO_REG(REG_PF_CE1Z_SET3) |= (BIT4);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE1Z_SET3) &= ~(BIT4);
        MHal_GPIO_REG(REG_PF_CE1Z_SET3) |= (BIT3);

    }

}


void MHal_GPIO_PF_OEZ_Set(void)
{


    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_PF_OEZ_SET2) &= ~(BIT4);
    MHal_GPIO_REG(REG_PF_OEZ_SET3) &= ~(BIT1|BIT0);
    if(GPIO139)
    {
        MHal_GPIO_REG(REG_PF_OEZ_SET3) |= (BIT4);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_OEZ_SET3) &= ~(BIT4);
        MHal_GPIO_REG(REG_PF_OEZ_SET3) |= (BIT3);

    }


}



void MHal_GPIO_PF_WEZ_Set(void)
{

    MHal_GPIO_REG(REG_PF_WEZ_SET1) &= ~(BIT4);
    if(GPIO140)
    {
        MHal_GPIO_REG(REG_PF_WEZ_SET2) |= (BIT4);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_WEZ_SET2) &= ~(BIT4);
        MHal_GPIO_REG(REG_PF_WEZ_SET2) |= (BIT3);

    }

}



void MHal_GPIO_PF_ALE_Set(void)
{

    MHal_GPIO_REG(REG_PF_ALE_SET1) &= ~(BIT4);
    if(GPIO141)
    {
        MHal_GPIO_REG(REG_PF_ALE_SET2) |= (BIT4);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_ALE_SET2) &= ~(BIT4);
        MHal_GPIO_REG(REG_PF_ALE_SET2) |= (BIT3);

    }

}



void MHal_GPIO_F_RBZ_Set(void)
{

    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_F_RBZ_SET2) &= ~(BIT4);
    MHal_GPIO_REG(REG_F_RBZ_SET3) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_F_RBZ_SET3) |= ~(BIT5);

}


void MHal_GPIO_TCON0_Set(void)
{
// shjang_090904 
//#if (USE_TCON_PANEL == 0)
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON0_SET2) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_TCON0_SET3) &= ~(BIT7);
    MHal_GPIO_REG(REG_TCON0_SET4) &= ~(BIT0);
    MHal_GPIO_REG(REG_TCON0_SET3) |= (BIT0);
//#endif
}


//balup_090907
void MHal_GPIO_GPIO48_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
	MHal_DSPEJTAG_Mode_Set();
  	//MHal_THIRDUART_Mode_Set();
}

void MHal_GPIO_GPIO49_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
	MHal_DSPEJTAG_Mode_Set();
  	//MHal_THIRDUART_Mode_Set();
}

void MHal_GPIO_GPIO52_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
	MHal_PDTraceCtrl_Set();
}


void MHal_GPIO_GPIO53_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
	MHal_PDTraceCtrl_Set();
}

void MHal_GPIO_GPIO47_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
	MHal_FASTUART_Mode_Set();
}



void MHal_GPIO_TCON1_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON1_SET2) |= (BIT0);
}


void MHal_GPIO_TCON2_Set(void)
{
// shjang_090904 
//#if (USE_TCON_PANEL == 0)
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON2_SET2) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_TCON2_SET3) &= ~(BIT7);
    MHal_GPIO_REG(REG_TCON2_SET4) &= ~(BIT7);
    MHal_GPIO_REG(REG_TCON2_SET3) |= (BIT0);
//#endif

}



void MHal_GPIO_TCON3_Set(void)
{

    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON3_SET2) |= (BIT0);
}


void MHal_GPIO_TCON4_Set(void)
{
// shjang_090904 
//#if (USE_TCON_PANEL == 0)
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON4_SET4) &= ~(BIT7);
    MHal_GPIO_REG(REG_TCON4_SET3) &= ~(BIT7);
    MHal_GPIO_REG(REG_TCON4_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_TCON4_SET3) |= (BIT1);
//#endif
}



void MHal_GPIO_TCON5_Set(void)
{

    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON5_SET2) |= (BIT1);
}


void MHal_GPIO_TCON6_Set(void)
{
// shjang_090904
//#if (USE_TCON_PANEL == 0)
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON6_SET4) &= ~(BIT7);
    MHal_GPIO_REG(REG_TCON6_SET3) &= ~(BIT7);
    MHal_GPIO_REG(REG_TCON6_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_TCON6_SET3) |= (BIT1);
//#endif
}

void MHal_GPIO_TCON7_Set(void)
{

    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON7_SET2) |= (BIT1);
}


void MHal_GPIO_TCON8_Set(void)
{
// shjang_090904 
//#if (USE_TCON_PANEL == 0)
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON8_SET2) |= (BIT2);
//#endif
}


void MHal_GPIO_TCON9_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON9_SET2) |= (BIT2);
}


void MHal_GPIO_TCON10_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON10_SET2) |= (BIT2);
}



void MHal_GPIO_TCON11_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON11_SET2) |= (BIT2);
}


void MHal_GPIO_TCON12_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON12_SET2) |= (BIT2);
}


void MHal_GPIO_TCON13_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON13_SET2) |= (BIT2);
}



void MHal_GPIO_TCON14_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON14_SET2) |= (BIT2);
}


void MHal_GPIO_TCON15_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON15_SET2) |= (BIT3);
}


void MHal_GPIO_TCON16_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON16_SET2) |= (BIT3);
}


void MHal_GPIO_TCON17_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON17_SET2) |= (BIT3);
}


void MHal_GPIO_TCON18_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON18_SET2) |= (BIT3);
}


void MHal_GPIO_TCON19_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON19_SET2) |= (BIT3);
}



void MHal_GPIO_TCON20_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON20_SET2) |= (BIT3);
}


void MHal_GPIO_TCON21_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_TCON21_SET2) |= (BIT3);
}



void MHal_GPIO_ET_COL_Set(void)
{

    MHal_Test_Out_Mode_Set();
    MHal_GPIO_REG(REG_ET_COL_SET2) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_ET_COL_SET3) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_COL_SET3) |= (BIT2);

}


void MHal_GPIO_ET_TXD1_Set(void)
{

    MHal_GPIO_REG(REG_ET_TXD1_SET1) &= ~(BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_TXD1_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_TXD1_SET2) |= (BIT2);

}


void MHal_GPIO_ET_TXD0_Set(void)
{

    MHal_GPIO_REG(REG_ET_TXD0_SET1) &= ~(BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_TXD0_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_TXD0_SET2) |= (BIT2);

}



void MHal_GPIO_ET_TX_EN_Set(void)
{

    MHal_GPIO_REG(REG_ET_TX_EN_SET1) &= ~(BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_TX_EN_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_TX_EN_SET2) |= (BIT2);

}


void MHal_GPIO_ET_TX_CLK_Set(void)
{
    MHal_GPIO_REG(REG_ET_TX_CLK_SET1) &= ~(BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_TX_CLK_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_TX_CLK_SET2) |= (BIT2);

}


void MHal_GPIO_ET_RXD0_Set(void)
{
    MHal_GPIO_REG(REG_ET_RXD0_SET) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_RXD0_SET) |= (BIT2);
}

void MHal_GPIO_ET_RXD1_Set(void)
{
    MHal_GPIO_REG(REG_ET_RXD1_SET) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_RXD1_SET) |= (BIT2);
}

void MHal_GPIO_ET_MDC_Set(void)
{
    MHal_GPIO_REG(REG_ET_MDC_SET) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_MDC_SET) |= (BIT2);
}


void MHal_GPIO_ET_EDIO_Set(void)
{
    MHal_GPIO_REG(REG_ET_EDIO_SET) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_ET_EDIO_SET) |= (BIT2);
}


void MHal_GPIO_I2S_IN_WS_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();

    MHal_GPIO_REG(REG_I2S_IN_WS_SET2) &= ~(BIT5|BIT4);
    MHal_GPIO_REG(REG_I2S_IN_WS_SET3) |= (BIT5);
    MHal_GPIO_REG(REG_I2S_IN_WS_SET3) &=~ (BIT6);

}


void MHal_GPIO_I2S_IN_BCK_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_GPIO_REG(REG_I2S_IN_BCK_SET2) |= (BIT5);

}



void MHal_GPIO_I2S_IN_SD_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();

    MHal_GPIO_REG(REG_I2S_IN_SD_SET2) &= ~(BIT5|BIT4);
    MHal_GPIO_REG(REG_I2S_IN_SD_SET3) |= (BIT5);
    MHal_GPIO_REG(REG_I2S_IN_WS_SET3) &=~ (BIT6);

}


void MHal_GPIO_SPDIF_IN_Set(void)
{
    MHal_GPIO_REG(REG_SPDIF_IN_SET) |= (BIT6);
}


void MHal_GPIO_SPDIF_OUT_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();

    MHal_GPIO_REG(REG_SPDIF_OUT_SET2) &= ~(BIT5|BIT4);
    MHal_GPIO_REG(REG_SPDIF_OUT_SET3) |= (BIT5);

}


void MHal_GPIO_I2S_OUT_MCK_Set(void)
{
    MHal_Test_Out_Mode_Set();

    MHal_GPIO_REG(REG_I2S_OUT_BCK_SET2) |= (BIT6);

}


void MHal_GPIO_I2S_OUT_WS_Set(void)
{
    MHal_Test_In_Mode_Set();
    MHal_Test_Out_Mode_Set();

    MHal_GPIO_REG(REG_I2S_OUT_WS_SET2) |= (BIT6);

}


void MHal_GPIO_I2S_OUT_BCK_Set(void)
{
    MHal_Test_Out_Mode_Set();

    MHal_GPIO_REG(REG_I2S_OUT_BCK_SET2) |= (BIT6);

}


void MHal_GPIO_I2S_OUT_SD_Set(void)
{
    MHal_Test_Out_Mode_Set();

    MHal_GPIO_REG(REG_I2S_OUT_SD_SET2) |= (BIT6);

}



void MHal_GPIO_I2S_OUT_SD1_Set(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD1_SET) |= (BIT6);

}



void MHal_GPIO_I2S_OUT_SD2_Set(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD2_SET) |= (BIT6);

}



void MHal_GPIO_I2S_OUT_SD3_Set(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD3_SET) |= (BIT6);

}


void MHal_GPIO_B_ODD_0_Set(void)
{

    MHal_GPIO_REG(REG_B_ODD_0_SET) &= ~(BIT3|BIT2);

}



void MHal_GPIO_B_ODD_1_Set(void)
{

    MHal_GPIO_REG(REG_B_ODD_1_SET) &= ~(BIT3|BIT2);

}



void MHal_GPIO_B_ODD_2_Set(void)
{

    MHal_GPIO_REG(REG_B_ODD_2_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_B_ODD_3_Set(void)
{

    MHal_GPIO_REG(REG_B_ODD_3_SET) &= ~(BIT1|BIT0);

}





void MHal_GPIO_B_ODD_4_Set(void)
{

    MHal_GPIO_REG(REG_B_ODD_4_SET) &= ~(BIT7|BIT6);

}



void MHal_GPIO_B_ODD_5_Set(void)
{

    MHal_GPIO_REG(REG_B_ODD_5_SET) &= ~(BIT7|BIT6);

}

void MHal_GPIO_B_ODD_6_Set(void)
{

    MHal_GPIO_REG(REG_B_ODD_6_SET) &= ~(BIT5|BIT4);

}

void MHal_GPIO_B_ODD_7_Set(void)
{

    MHal_GPIO_REG(REG_B_ODD_7_SET) &= ~(BIT5|BIT4);

}

void MHal_GPIO_G_ODD_0_Set(void)
{

    MHal_GPIO_REG(REG_G_ODD_0_SET) &= ~(BIT3|BIT2);

}



void MHal_GPIO_G_ODD_1_Set(void)
{

    MHal_GPIO_REG(REG_G_ODD_1_SET) &= ~(BIT3|BIT2);

}



void MHal_GPIO_G_ODD_2_Set(void)
{

    MHal_GPIO_REG(REG_G_ODD_2_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_G_ODD_3_Set(void)
{

    MHal_GPIO_REG(REG_G_ODD_3_SET) &= ~(BIT1|BIT0);

}





void MHal_GPIO_G_ODD_4_Set(void)
{

    MHal_GPIO_REG(REG_G_ODD_4_SET) &= ~(BIT7|BIT6);

}



void MHal_GPIO_G_ODD_5_Set(void)
{

    MHal_GPIO_REG(REG_G_ODD_5_SET) &= ~(BIT7|BIT6);

}

void MHal_GPIO_G_ODD_6_Set(void)
{

    MHal_GPIO_REG(REG_G_ODD_6_SET) &= ~(BIT5|BIT4);

}

void MHal_GPIO_G_ODD_7_Set(void)
{

    MHal_GPIO_REG(REG_G_ODD_7_SET) &= ~(BIT5|BIT4);

}

void MHal_GPIO_R_ODD_0_Set(void)
{

    MHal_GPIO_REG(REG_R_ODD_0_SET) &= ~(BIT3|BIT2);

}



void MHal_GPIO_R_ODD_1_Set(void)
{

    MHal_GPIO_REG(REG_R_ODD_1_SET) &= ~(BIT3|BIT2);

}



void MHal_GPIO_R_ODD_2_Set(void)
{

    MHal_GPIO_REG(REG_R_ODD_2_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_R_ODD_3_Set(void)
{

    MHal_GPIO_REG(REG_R_ODD_3_SET) &= ~(BIT1|BIT0);

}





void MHal_GPIO_R_ODD_4_Set(void)
{

    MHal_GPIO_REG(REG_R_ODD_4_SET) &= ~(BIT7|BIT6);

}



void MHal_GPIO_R_ODD_5_Set(void)
{

    MHal_GPIO_REG(REG_R_ODD_5_SET) &= ~(BIT7|BIT6);

}

void MHal_GPIO_R_ODD_6_Set(void)
{

    MHal_GPIO_REG(REG_R_ODD_6_SET) &= ~(BIT5|BIT4);

}

void MHal_GPIO_R_ODD_7_Set(void)
{

    MHal_GPIO_REG(REG_R_ODD_7_SET) &= ~(BIT5|BIT4);

}

void MHal_GPIO_mini_LVDS_0_Set(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_0_SET) &= ~(BIT3|BIT2);

}



void MHal_GPIO_mini_LVDS_1_Set(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_1_SET) &= ~(BIT3|BIT2);

}



void MHal_GPIO_mini_LVDS_2_Set(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_2_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_mini_LVDS_3_Set(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_3_SET) &= ~(BIT1|BIT0);

}





void MHal_GPIO_mini_LVDS_4_Set(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_4_SET) &= ~(BIT7|BIT6);

}



void MHal_GPIO_mini_LVDS_5_Set(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_5_SET) &= ~(BIT7|BIT6);

}

void MHal_GPIO_mini_LVDS_6_Set(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_6_SET) &= ~(BIT5|BIT4);

}

void MHal_GPIO_mini_LVDS_7_Set(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_7_SET) &= ~(BIT5|BIT4);

}

void MHal_GPIO_LCK_Set(void)
{

    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LCK_SET) &= ~(BIT3|BIT2);

}


void MHal_GPIO_LDE_Set(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;
    MHal_GPIO_REG(REG_LDE_SET) &= ~(BIT3|BIT2);

}


void MHal_GPIO_LHSYNC_Set(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LHSYNC_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_LVSYNC_Set(void)
{

    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LVSYNC_SET) &= ~(BIT1|BIT0);

}


void MHal_GPIO_PCM_WE_N_Set(void)
{
    MHal_Test_In_Mode_Set();

    MHal_GPIO_REG(REG_PCM_WE_N_SET2) |= (BIT5);

}


//this section set to make output


void MHal_GPIO_PM_SPI_CZ_Oen(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CZ_OEN) &= ~BIT4;
}

void MHal_GPIO_PM_SPI_CK_Oen(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CK_OEN) &= ~BIT5;
}
void MHal_GPIO_PM_SPI_DI_Oen(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DI_OEN) &= ~BIT6;
}

void MHal_GPIO_PM_SPI_DO_Oen(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DO_OEN) &= ~BIT7;
}

void MHal_GPIO_IRIN_Oen(void)
{
    MHal_GPIO_REG(REG_IRIN_OEN) &= ~BIT0;
}

void MHal_GPIO_CEC_Oen(void)
{
    MHal_GPIO_REG(REG_CEC_OEN) &= ~BIT2;

}

void MHal_GPIO_GPIO_PM0_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM0_OEN) &= ~BIT0;

}

void MHal_GPIO_GPIO_PM1_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM1_OEN) &= ~BIT1;

}

void MHal_GPIO_GPIO_PM2_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM2_OEN) &= ~BIT2;

}

void MHal_GPIO_GPIO_PM3_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM3_OEN) &= ~BIT3;

}

void MHal_GPIO_GPIO_PM4_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM4_OEN) &= ~BIT4;
}

void MHal_GPIO_GPIO_PM5_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM5_OEN) &= ~BIT5;

}

void MHal_GPIO_GPIO_PM6_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM6_OEN) &= ~BIT6;

}

void MHal_GPIO_GPIO_PM7_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM7_OEN) &= ~BIT7;

}

void MHal_GPIO_GPIO_PM8_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM8_OEN) &= ~BIT0;

}

void MHal_GPIO_GPIO_PM9_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM9_OEN) &= ~BIT1;

}

void MHal_GPIO_GPIO_PM10_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM10_OEN) &= ~BIT2;

}

void MHal_GPIO_GPIO_PM11_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM11_OEN) &= ~BIT3;

}

void MHal_GPIO_GPIO_PM12_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO_PM12_OEN) &= ~BIT4;

}

void MHal_GPIO_HOTPLUGA_Oen(void)
{
    MHal_GPIO_REG(REG_HOTPLUGA_OEN) &= ~BIT0;

}


void MHal_GPIO_HOTPLUGB_Oen(void)
{
    MHal_GPIO_REG(REG_HOTPLUGB_OEN) &= ~BIT1;

}

void MHal_GPIO_HOTPLUGC_Oen(void)
{
    MHal_GPIO_REG(REG_HOTPLUGC_OEN) &= ~BIT2;

}

void MHal_GPIO_HOTPLUGD_Oen(void)
{
    MHal_GPIO_REG(REG_HOTPLUGD_OEN) &= ~BIT3;

}


void MHal_GPIO_DDCDA_CK_Oen(void)
{
    MHal_GPIO_REG(REG_DDCDA_CK_OEN) &= ~BIT1;

}

void MHal_GPIO_DDCDA_DA_Oen(void)
{
    MHal_GPIO_REG(REG_DDCDA_DA_OEN) &= ~BIT5;

}

void MHal_GPIO_DDCDB_CK_Oen(void)
{
    MHal_GPIO_REG(REG_DDCDB_DA_OEN) &= ~BIT1;

}

void MHal_GPIO_DDCDB_DA_Oen(void)
{
    MHal_GPIO_REG(REG_DDCDB_CK_OEN) &= ~BIT5;

}

void MHal_GPIO_DDCDC_CK_Oen(void)
{
    MHal_GPIO_REG(REG_DDCDC_CK_OEN) &= ~BIT1;
}

void MHal_GPIO_DDCDC_DA_Oen(void)
{
    MHal_GPIO_REG(REG_DDCDC_DA_OEN)  &= ~BIT5;
}

void MHal_GPIO_DDCDD_CK_Oen(void)
{
    MHal_GPIO_REG(REG_DDCDD_CK_OEN) &= ~BIT1;
}

void MHal_GPIO_DDCDD_DA_Oen(void)
{
    MHal_GPIO_REG(REG_DDCDD_DA_OEN) &= ~BIT5;

}

void MHal_GPIO_SAR0_Oen(void)
{
    MHal_GPIO_REG(REG_SAR0_OEN) &= ~BIT0;

}

void MHal_GPIO_SAR1_Oen(void)
{
    MHal_GPIO_REG(REG_SAR1_OEN) &= ~BIT1;

}

void MHal_GPIO_SAR2_Oen(void)
{
    MHal_GPIO_REG(REG_SAR2_OEN) &= ~BIT2;
}

void MHal_GPIO_SAR3_Oen(void)
{
    MHal_GPIO_REG(REG_SAR3_OEN) &= ~BIT3;
}

void MHal_GPIO_SAR4_Oen(void)
{
    MHal_GPIO_REG(REG_SAR4_OEN) &= ~BIT4;
}

void MHal_GPIO_GPIO0_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO0_OEN) &= ~BIT0;

}

void MHal_GPIO_GPIO1_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO1_OEN) &= ~BIT1;

}

void MHal_GPIO_GPIO2_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO2_OEN) &= ~BIT2;

}

void MHal_GPIO_GPIO3_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO3_OEN) &= ~BIT3;

}

void MHal_GPIO_GPIO4_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO4_OEN) &= ~BIT4;
}

void MHal_GPIO_GPIO5_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO5_OEN) &= ~BIT5;
}

void MHal_GPIO_GPIO6_Oen(void)
{

    MHal_GPIO_REG(REG_GPIO6_OEN) &= ~BIT6;

}

void MHal_GPIO_GPIO7_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO7_OEN) &= ~BIT7;

}

void MHal_GPIO_GPIO8_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO8_OEN) &= ~BIT0;
}

void MHal_GPIO_GPIO9_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO9_OEN) &= ~BIT1;
}

void MHal_GPIO_GPIO10_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO10_OEN) &= ~BIT2;

}

void MHal_GPIO_GPIO11_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO11_OEN) &= ~BIT3;

}

void MHal_GPIO_GPIO12_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO12_OEN) &= ~BIT4;

}
void MHal_GPIO_GPIO13_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO13_OEN) &= ~BIT5;

}
void MHal_GPIO_GPIO14_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO14_OEN) &= ~BIT6;
}
void MHal_GPIO_GPIO15_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO15_OEN) &= ~BIT7;

}
void MHal_GPIO_GPIO16_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO16_OEN) &= ~BIT0;

}
void MHal_GPIO_GPIO17_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO17_OEN) &= ~BIT1;

}
void MHal_GPIO_GPIO18_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO18_OEN) &= ~BIT2;

}

void MHal_GPIO_GPIO19_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO19_OEN) &= ~BIT3;

}


void MHal_GPIO_GPIO20_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO20_OEN) &= ~BIT4;

}


void MHal_GPIO_GPIO21_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO21_OEN) &= ~BIT5;

}


void MHal_GPIO_GPIO22_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO22_OEN) &= ~BIT6;
}

void MHal_GPIO_GPIO23_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO23_OEN) &= ~BIT7;
}


void MHal_GPIO_GPIO24_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO24_OEN) &= ~BIT0;

}
void MHal_GPIO_GPIO25_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO25_OEN) &= ~BIT1;

}

void MHal_GPIO_GPIO26_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO26_OEN) &= ~BIT2;

}
void MHal_GPIO_GPIO27_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO27_OEN) &= ~BIT3;

}

void MHal_GPIO_UART_RX2_Oen(void)
{
    MHal_GPIO_REG(REG_UART_RX2_OEN) &= ~BIT4;

}
void MHal_GPIO_UART_TX2_Oen(void)
{
    MHal_GPIO_REG(REG_UART_TX2_OEN) &= ~BIT5;

}

void MHal_GPIO_PWM0_Oen(void)
{
    MHal_GPIO_REG(REG_PWM0_OEN) &= ~BIT6;
}
void MHal_GPIO_PWM1_Oen(void)
{
    MHal_GPIO_REG(REG_PWM1_OEN) &= ~BIT7;

}
void MHal_GPIO_PWM2_Oen(void)
{
    MHal_GPIO_REG(REG_PWM2_OEN) &= ~BIT0;
}


void MHal_GPIO_PWM3_Oen(void)
{
    MHal_GPIO_REG(REG_PWM3_OEN) &= ~BIT1;
}


void MHal_GPIO_PWM4_Oen(void)
{
    MHal_GPIO_REG(REG_PWM4_OEN) &= ~BIT2;
}

void MHal_GPIO_DDCR_DA_Oen(void)
{
    MHal_GPIO_REG(REG_DDCR_DA_OEN) &= ~BIT3;

}

void MHal_GPIO_DDCR_CK_Oen(void)
{
    MHal_GPIO_REG(REG_DDCR_CK_OEN) &= ~BIT4;

}


void MHal_GPIO_TGPIO0_Oen(void)
{
    MHal_GPIO_REG(REG_TGPIO0_OEN) &= ~BIT5;

}


void MHal_GPIO_TGPIO1_Oen(void)
{
    MHal_GPIO_REG(REG_TGPIO1_OEN) &= ~BIT6;

}


void MHal_GPIO_TGPIO2_Oen(void)
{
    MHal_GPIO_REG(REG_TGPIO2_OEN)&= ~BIT7;
}


void MHal_GPIO_TGPIO3_Oen(void)
{
    MHal_GPIO_REG(REG_TGPIO3_OEN) &= ~BIT0;
}


void MHal_GPIO_TS0_D0_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_D0_OEN) &= ~BIT0;

}


void MHal_GPIO_TS0_D1_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_D1_OEN) &= ~BIT1;

}


void MHal_GPIO_TS0_D2_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_D2_OEN) &= ~BIT2;

}


void MHal_GPIO_TS0_D3_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_D3_OEN) &= ~BIT3;

}
void MHal_GPIO_TS0_D4_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_D4_OEN) &= ~BIT4;

}


void MHal_GPIO_TS0_D5_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_D5_OEN) &= ~BIT5;

}


void MHal_GPIO_TS0_D6_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_D6_OEN) &= ~BIT6;
}



void MHal_GPIO_TS0_D7_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_D7_OEN) &= ~BIT7;

}


void MHal_GPIO_TS0_VLD_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_VLD_OEN) &= ~BIT0;
}


void MHal_GPIO_TS0_SYNC_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_SYNC_OEN) &= ~BIT1;
}



void MHal_GPIO_TS0_CLK_Oen(void)
{
    MHal_GPIO_REG(REG_TS0_CLK_OEN) &= ~BIT2;
}

void MHal_GPIO_TS1_D0_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_D0_OEN) &= ~(BIT0);

}
void MHal_GPIO_TS1_D1_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_D1_OEN) &= ~(BIT1);

}


void MHal_GPIO_TS1_D2_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_D2_OEN) &= ~(BIT2);

}

void MHal_GPIO_TS1_D3_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_D3_OEN) &= ~(BIT3);

}

void MHal_GPIO_TS1_D4_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_D4_OEN) &= ~(BIT4);

}


void MHal_GPIO_TS1_D5_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_D5_OEN) &= ~(BIT5);

}


void MHal_GPIO_TS1_D6_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_D6_OEN) &= ~(BIT6);

}


void MHal_GPIO_TS1_D7_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_D7_OEN) &= ~(BIT7);

}


void MHal_GPIO_TS1_VLD_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_VLD_OEN) &= ~(BIT0);

}


void MHal_GPIO_TS1_SYNC_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_SYNC_OEN) &= ~(BIT1);
}


void MHal_GPIO_TS1_CLK_Oen(void)
{
    MHal_GPIO_REG(REG_TS1_CLK_OEN) &= ~(BIT2);

}


void MHal_GPIO_PCM_A4_Oen(void)
{

    if(GPIO99)
    {
        //PCM_GPIO0
        MHal_GPIO_REG(REG_PCM_A4_OEN1) &= ~(BIT0);

    }
    else
    {
        //NOR_GPIO15
        MHal_GPIO_REG(REG_PCM_A4_OEN2) &= ~(BIT7);



    }

}



void MHal_GPIO_PCM_WAIT_N_Oen(void)
{
    MHal_GPIO_REG(REG_PCM_WAIT_N_OEN) &= ~(BIT1);

}
void MHal_GPIO_PCM_A5_Oen(void)
{

    if(GPIO101)
    {
        //PCM_GPIO2
        MHal_GPIO_REG(REG_PCM_A5_OEN1) &= ~(BIT2);

    }
    else
    {
        //NOR_GPIO14
        MHal_GPIO_REG(REG_PCM_A5_OEN2) &= ~(BIT6);

    }

}


void MHal_GPIO_PCM_A6_Oen(void)
{

    if(GPIO102)
    {
        //PCM_GPIO3
        MHal_GPIO_REG(REG_PCM_A6_OEN1) &= ~(BIT3);

    }
    else
    {
        //NOR_GPIO13
        MHal_GPIO_REG(REG_PCM_A6_OEN2) &= ~(BIT5);

    }

}



void MHal_GPIO_PCM_A7_Oen(void)
{

    if(GPIO103)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A7_OEN1)  &= ~(BIT4);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A7_OEN2) &= ~(BIT4);

    }

}


void MHal_GPIO_PCM_A12_Oen(void)
{

    if(GPIO104)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A12_OEN1) &= ~(BIT5);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A12_OEN2) &= ~(BIT3);

    }

}



void MHal_GPIO_PCM_IRQA_N_Oen(void)
{
    MHal_GPIO_REG(REG_PCM_IRQA_N_OEN) &= ~(BIT6);

}

void MHal_GPIO_PCM_A14_Oen(void)
{

    if(GPIO106)
    {
        MHal_GPIO_REG(REG_PCM_A14_OEN1) &= ~(BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A14_OEN2) &= ~(BIT2);

    }

}


void MHal_GPIO_PCM_A13_Oen(void)
{

    if(GPIO107)
    {
        MHal_GPIO_REG(REG_PCM_A13_OEN1) &= ~(BIT0);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A13_OEN2)  &= ~(BIT1);

    }

}

void MHal_GPIO_PCM_A8_Oen(void)
{

    if(GPIO108)
    {
        MHal_GPIO_REG(REG_PCM_A8_OEN1) &= ~(BIT1);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_OEN2) &= ~(BIT0);

    }

}



void MHal_GPIO_PCM_IOWR_N_Oen(void)
{
    MHal_GPIO_REG(REG_PCM_IOWR_N_OEN) &= ~(BIT2);

}


void MHal_GPIO_PCM_A9_Oen(void)
{

    if(GPIO110)
    {
        MHal_GPIO_REG(REG_PCM_A8_OEN1) &= ~(BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_OEN2) &= ~(BIT7);

    }

}



void MHal_GPIO_PCM_IORD_N_Oen(void)
{
    MHal_GPIO_REG(REG_PCM_IORD_N_OEN) &= ~(BIT4);

}



void MHal_GPIO_PCM_A11_Oen(void)
{

    if(GPIO112)
    {
        MHal_GPIO_REG(REG_PCM_A11_OEN2) &= ~(BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A11_OEN2) &= ~(BIT6);

    }

}


void MHal_GPIO_PCM_OE_N_Oen(void)
{
    MHal_GPIO_REG(REG_PCM_OE_N_OEN) &= ~(BIT6);

}


void MHal_GPIO_PCM_A10_Oen(void)
{

    if(GPIO114)
    {
        MHal_GPIO_REG(REG_PCM_A10_OEN1) &= ~(BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A10_OEN2) &= ~(BIT5);
    }


}
void MHal_GPIO_PCM_CE_N_Oen(void)
{

    MHal_GPIO_REG(REG_PCM_CE_N_OEN) &= ~(BIT0);

}


void MHal_GPIO_PCM_D7_Oen(void)
{

    if(GPIO116)
    {
        MHal_GPIO_REG(REG_PCM_D7_OEN1) &= ~(BIT1);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D7_OEN2) &= ~(BIT4);
    }

}


void MHal_GPIO_PCM_D6_Oen(void)
{

    if(GPIO117)
    {
        MHal_GPIO_REG(REG_PCM_D6_OEN1) &= ~(BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D6_OEN2) &= ~(BIT3);
    }

}



void MHal_GPIO_PCM_D5_Oen(void)
{

    if(GPIO118)
    {
        MHal_GPIO_REG(REG_PCM_D5_OEN1) &= ~(BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D5_OEN2) &= ~(BIT2);

    }

}


void MHal_GPIO_PCM_D4_Oen(void)
{

    if(GPIO119)
    {
        MHal_GPIO_REG(REG_PCM_D4_OEN1) &= ~(BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D4_OEN2) &= ~(BIT1);
    }

}


void MHal_GPIO_PCM_D3_Oen(void)
{

    if(GPIO120)
    {
        MHal_GPIO_REG(REG_PCM_D3_OEN1)  &= ~(BIT6);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D3_OEN2) &= ~(BIT0);
    }

}


void MHal_GPIO_PCM_A3_Oen(void)
{
    if(GPIO121)
    {
        MHal_GPIO_REG(REG_PCM_A3_OEN1) &= ~(BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_OEN2) &= ~(BIT6);

    }

}



void MHal_GPIO_PCM_A2_Oen(void)
{


    if(GPIO122)
    {
        MHal_GPIO_REG(REG_PCM_A2_OEN1) &= ~(BIT0);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_OEN2) &= ~(BIT7);
    }

}


void MHal_GPIO_PCM_REG_N_Oen(void)
{
    MHal_GPIO_REG(REG_PCM_REG_N_OEN) &= ~(BIT1);
}

void MHal_GPIO_PCM_A1_Oen(void)
{

    if(GPIO124)
    {
        MHal_GPIO_REG(REG_PCM_A1_OEN1) &= ~(BIT2);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A1_OEN2) &= ~(BIT0);

    }

}




void MHal_GPIO_PCM_A0_Oen(void)
{

    if(GPIO125)
    {
        MHal_GPIO_REG(REG_PCM_A0_OEN1) &= ~(BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A0_OEN2) &= ~(BIT1);

    }

}



void MHal_GPIO_PCM_D0_Oen(void)
{


    if(GPIO126)
    {
        MHal_GPIO_REG(REG_PCM_A1_OEN1) &= ~(BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D0_OEN2) &= ~(BIT2);

    }

}


void MHal_GPIO_PCM_D1_Oen(void)
{


    if(GPIO127)
    {
        MHal_GPIO_REG(REG_PCM_D1_OEN1) &= ~(BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D1_OEN2) &= ~(BIT3);

    }

}


void MHal_GPIO_PCM_D2_Oen(void)
{


    if(GPIO128)
    {
        MHal_GPIO_REG(REG_PCM_D2_OEN1) &= ~(BIT6);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D2_OEN2)  &= ~(BIT4);

    }

}



void MHal_GPIO_PCM_RESET_Oen(void)
{

    MHal_GPIO_REG(REG_PCM_RESET_OEN) &= ~(BIT7);

}



void MHal_GPIO_PCM_CD_N_Oen(void)
{

    MHal_GPIO_REG(REG_PCM_CD_N_OEN) &= ~(BIT0);

}
void MHal_GPIO_PCM2_CE_N_Oen(void)
{

    MHal_GPIO_REG(REG_PCM2_CE_N_OEN)&= ~(BIT0);

}



void MHal_GPIO_PCM2_IRQA_N_Oen(void)
{

    MHal_GPIO_REG(REG_PCM2_IRQA_N_OEN)&= ~(BIT1);

}



void MHal_GPIO_PCM2_WAIT_N_Oen(void)
{

    MHal_GPIO_REG(REG_PCM2_WAIT_N_OEN) &= ~(BIT2);

}



void MHal_GPIO_PCM2_RESET_Oen(void)
{

    MHal_GPIO_REG(REG_PCM2_RESET_OEN) &= ~(BIT3);

}



void MHal_GPIO_PCM2_CD_N_Oen(void)
{

    MHal_GPIO_REG(REG_PCM2_CD_N_OEN) &= ~(BIT4);

}



void MHal_GPIO_PF_AD15_Oen(void)
{

    if(GPIO136)
    {
        MHal_GPIO_REG(REG_PF_AD15_OEN1) &= ~(BIT0);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_AD15_OEN2) &= ~(BIT0);

    }

}



void MHal_GPIO_PF_CE0Z_Oen(void)
{

    if(GPIO137)
    {
        MHal_GPIO_REG(REG_PF_CE0Z_OEN1) &= ~(BIT1);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE0Z_OEN2) &= ~(BIT1);

    }

}


void MHal_GPIO_PF_CE1Z_Oen(void)
{

    if(GPIO138)
    {
        MHal_GPIO_REG(REG_PF_CE1Z_OEN1) &= ~(BIT2);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE1Z_OEN2) &= ~(BIT2);
    }

}


void MHal_GPIO_PF_OEZ_Oen(void)
{

    if(GPIO139)
    {
        MHal_GPIO_REG(REG_PF_OEZ_OEN1)  &= ~(BIT3);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_OEZ_OEN2)  &= ~(BIT3);
    }

}



void MHal_GPIO_PF_WEZ_Oen(void)
{

    if(GPIO140)
    {
        MHal_GPIO_REG(REG_PF_WEZ_OEN1) &= ~(BIT4);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_WEZ_OEN2) &= ~(BIT4);

    }

}



void MHal_GPIO_PF_ALE_Oen(void)
{

    if(GPIO141)
    {
        MHal_GPIO_REG(REG_PF_ALE_OEN1) &= ~(BIT5);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_ALE_OEN2) &= ~(BIT5);

    }

}



void MHal_GPIO_F_RBZ_Oen(void)
{
    MHal_GPIO_REG(REG_F_RBZ_OEN) &= ~(BIT6);
}

//balup_090907

void MHal_GPIO_GPIO48_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO48_OEN) &= ~(BIT4);
}

void MHal_GPIO_GPIO49_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO48_OEN) &= ~(BIT5);
}

void MHal_GPIO_GPIO53_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO53_OEN) &= ~(BIT1);
}

void MHal_GPIO_GPIO52_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO53_OEN) &= ~(BIT0);
}

void MHal_GPIO_GPIO47_Oen(void)
{
    MHal_GPIO_REG(REG_GPIO53_OEN) &= ~(BIT3);
}





void MHal_GPIO_TCON0_Oen(void)
{
    MHal_GPIO_REG(REG_TCON0_OEN) &= ~(BIT0);
}



void MHal_GPIO_TCON1_Oen(void)
{
    MHal_GPIO_REG(REG_TCON1_OEN) &= ~(BIT1);
}


void MHal_GPIO_TCON2_Oen(void)
{
    MHal_GPIO_REG(REG_TCON2_OEN) &= ~(BIT2);
}



void MHal_GPIO_TCON3_Oen(void)
{
    MHal_GPIO_REG(REG_TCON3_OEN) &= ~(BIT3);
}


void MHal_GPIO_TCON4_Oen(void)
{
    MHal_GPIO_REG(REG_TCON4_OEN) &= ~(BIT4);
}



void MHal_GPIO_TCON5_Oen(void)
{
    MHal_GPIO_REG(REG_TCON5_OEN) &= ~(BIT5);
}


void MHal_GPIO_TCON6_Oen(void)
{
    MHal_GPIO_REG(REG_TCON6_OEN) &= ~(BIT6);
}

void MHal_GPIO_TCON7_Oen(void)
{
    MHal_GPIO_REG(REG_TCON7_OEN)  &= ~(BIT7);
}


void MHal_GPIO_TCON8_Oen(void)
{
    MHal_GPIO_REG(REG_TCON8_OEN) &= ~(BIT0);
}


void MHal_GPIO_TCON9_Oen(void)
{
    MHal_GPIO_REG(REG_TCON9_OEN) &= ~(BIT1);
}


void MHal_GPIO_TCON10_Oen(void)
{
    MHal_GPIO_REG(REG_TCON10_OEN) &= ~(BIT2);
}



void MHal_GPIO_TCON11_Oen(void)
{
    MHal_GPIO_REG(REG_TCON11_OEN) &= ~(BIT3);
}


void MHal_GPIO_TCON12_Oen(void)
{
    MHal_GPIO_REG(REG_TCON12_OEN) &= ~(BIT4);
}


void MHal_GPIO_TCON13_Oen(void)
{
    MHal_GPIO_REG(REG_TCON13_OEN) &= ~(BIT5);
}



void MHal_GPIO_TCON14_Oen(void)
{
    MHal_GPIO_REG(REG_TCON14_OEN) &= ~(BIT6);
}


void MHal_GPIO_TCON15_Oen(void)
{
    MHal_GPIO_REG(REG_TCON15_OEN) &= ~(BIT7);
}


void MHal_GPIO_TCON16_Oen(void)
{
    MHal_GPIO_REG(REG_TCON16_OEN) &= ~(BIT0);
}


void MHal_GPIO_TCON17_Oen(void)
{
    MHal_GPIO_REG(REG_TCON17_OEN)  &= ~(BIT1);
}


void MHal_GPIO_TCON18_Oen(void)
{
    MHal_GPIO_REG(REG_TCON18_OEN)  &= ~(BIT2);
}


void MHal_GPIO_TCON19_Oen(void)
{
    MHal_GPIO_REG(REG_TCON19_OEN)  &= ~(BIT3);
}



void MHal_GPIO_TCON20_Oen(void)
{
    MHal_GPIO_REG(REG_TCON20_OEN) &= ~(BIT4);
}


void MHal_GPIO_TCON21_Oen(void)
{
    MHal_GPIO_REG(REG_TCON21_OEN)  &= ~(BIT5);
}



void MHal_GPIO_ET_COL_Oen(void)
{

    MHal_GPIO_REG(REG_ET_COL_OEN)  &= ~(BIT0);

}


void MHal_GPIO_ET_TXD1_Oen(void)
{

    MHal_GPIO_REG(REG_ET_TXD1_OEN) &= ~(BIT1);

}


void MHal_GPIO_ET_TXD0_Oen(void)
{

    MHal_GPIO_REG(REG_ET_TXD0_OEN) &= ~(BIT2);


}



void MHal_GPIO_ET_TX_EN_Oen(void)
{

    MHal_GPIO_REG(REG_ET_TX_EN_OEN) &= ~(BIT3);

}


void MHal_GPIO_ET_TX_CLK_Oen(void)
{
    MHal_GPIO_REG(REG_ET_TX_CLK_OEN) &= ~(BIT4);

}


void MHal_GPIO_ET_RXD0_Oen(void)
{
    MHal_GPIO_REG(REG_ET_RXD0_OEN) &= ~(BIT5);
}

void MHal_GPIO_ET_RXD1_Oen(void)
{
    MHal_GPIO_REG(REG_ET_RXD1_OEN) &= ~(BIT6);
}

void MHal_GPIO_ET_MDC_Oen(void)
{
    MHal_GPIO_REG(REG_ET_MDC_OEN) &= ~(BIT7);
}


void MHal_GPIO_ET_EDIO_Oen(void)
{
    MHal_GPIO_REG(REG_ET_EDIO_OEN) &= ~(BIT0);
}


void MHal_GPIO_I2S_IN_WS_Oen(void)
{

    MHal_GPIO_REG(REG_I2S_IN_WS_OEN) &= ~(BIT0);

}


void MHal_GPIO_I2S_IN_BCK_Oen(void)
{
    MHal_GPIO_REG(REG_I2S_IN_BCK_OEN)  &= ~(BIT1);

}



void MHal_GPIO_I2S_IN_SD_Oen(void)
{

    MHal_GPIO_REG(REG_I2S_IN_SD_OEN) &= ~(BIT2);

}


void MHal_GPIO_SPDIF_IN_Oen(void)
{
    MHal_GPIO_REG(REG_SPDIF_IN_OEN) &= ~(BIT3);
}


void MHal_GPIO_SPDIF_OUT_Oen(void)
{

    MHal_GPIO_REG(REG_SPDIF_OUT_OEN) &= ~(BIT4);

}


void MHal_GPIO_I2S_OUT_MCK_Oen(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_MCK_OEN)&= ~(BIT5);

}


void MHal_GPIO_I2S_OUT_WS_Oen(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_WS_OEN) &= ~(BIT6);

}


void MHal_GPIO_I2S_OUT_BCK_Oen(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_BCK_OEN) &= ~(BIT7);

}


void MHal_GPIO_I2S_OUT_SD_Oen(void)
{
    MHal_GPIO_REG(REG_I2S_OUT_SD_OEN) &= ~(BIT0);
}



void MHal_GPIO_I2S_OUT_SD1_Oen(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD1_OEN) &= ~(BIT1);

}



void MHal_GPIO_I2S_OUT_SD2_Oen(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD2_OEN) &= ~(BIT2);

}



void MHal_GPIO_I2S_OUT_SD3_Oen(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD3_OEN) &= ~(BIT3);

}


void MHal_GPIO_B_ODD_0_Oen(void)
{

    MHal_GPIO_REG(REG_B_ODD_0_OEN) &= ~(BIT3);

}



void MHal_GPIO_B_ODD_1_Oen(void)
{

    MHal_GPIO_REG(REG_B_ODD_1_OEN) &= ~(BIT2);

}



void MHal_GPIO_B_ODD_2_Oen(void)
{

    MHal_GPIO_REG(REG_B_ODD_2_OEN) &= ~(BIT1);

}


void MHal_GPIO_B_ODD_3_Oen(void)
{

    MHal_GPIO_REG(REG_B_ODD_3_OEN) &= ~(BIT0);

}





void MHal_GPIO_B_ODD_4_Oen(void)
{

    MHal_GPIO_REG(REG_B_ODD_4_OEN) &= ~(BIT7);

}



void MHal_GPIO_B_ODD_5_Oen(void)
{

    MHal_GPIO_REG(REG_B_ODD_5_OEN) &= ~(BIT6);

}

void MHal_GPIO_B_ODD_6_Oen(void)
{

    MHal_GPIO_REG(REG_B_ODD_6_OEN) &= ~(BIT5);

}

void MHal_GPIO_B_ODD_7_Oen(void)
{

    MHal_GPIO_REG(REG_B_ODD_7_OEN) &= ~(BIT4);
}

void MHal_GPIO_G_ODD_0_Oen(void)
{

    MHal_GPIO_REG(REG_G_ODD_0_OEN) &= ~(BIT3);

}



void MHal_GPIO_G_ODD_1_Oen(void)
{

    MHal_GPIO_REG(REG_G_ODD_1_OEN) &= ~(BIT2);

}



void MHal_GPIO_G_ODD_2_Oen(void)
{

    MHal_GPIO_REG(REG_G_ODD_2_OEN) &= ~(BIT1);

}


void MHal_GPIO_G_ODD_3_Oen(void)
{

    MHal_GPIO_REG(REG_G_ODD_3_OEN) &= ~(BIT0);

}





void MHal_GPIO_G_ODD_4_Oen(void)
{

    MHal_GPIO_REG(REG_G_ODD_4_OEN) &= ~(BIT7);

}



void MHal_GPIO_G_ODD_5_Oen(void)
{

    MHal_GPIO_REG(REG_G_ODD_5_OEN) &= ~(BIT6);

}

void MHal_GPIO_G_ODD_6_Oen(void)
{

    MHal_GPIO_REG(REG_G_ODD_6_OEN) &= ~(BIT5);

}

void MHal_GPIO_G_ODD_7_Oen(void)
{

    MHal_GPIO_REG(REG_G_ODD_7_OEN)&= ~(BIT4);

}

void MHal_GPIO_R_ODD_0_Oen(void)
{

    MHal_GPIO_REG(REG_R_ODD_0_OEN) &= ~(BIT3);

}



void MHal_GPIO_R_ODD_1_Oen(void)
{

    MHal_GPIO_REG(REG_R_ODD_1_OEN) &= ~(BIT2);

}



void MHal_GPIO_R_ODD_2_Oen(void)
{

    MHal_GPIO_REG(REG_R_ODD_2_OEN) &= ~(BIT1);

}


void MHal_GPIO_R_ODD_3_Oen(void)
{

    MHal_GPIO_REG(REG_R_ODD_3_OEN) &= ~(BIT0);

}





void MHal_GPIO_R_ODD_4_Oen(void)
{

    MHal_GPIO_REG(REG_R_ODD_4_OEN) &= ~(BIT7);

}



void MHal_GPIO_R_ODD_5_Oen(void)
{

    MHal_GPIO_REG(REG_R_ODD_5_OEN) &= ~(BIT6);

}

void MHal_GPIO_R_ODD_6_Oen(void)
{

    MHal_GPIO_REG(REG_R_ODD_6_OEN) &= ~(BIT5);

}

void MHal_GPIO_R_ODD_7_Oen(void)
{

    MHal_GPIO_REG(REG_R_ODD_7_OEN) &= ~(BIT4);

}

void MHal_GPIO_mini_LVDS_0_Oen(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_0_OEN) &= ~(BIT3);

}



void MHal_GPIO_mini_LVDS_1_Oen(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_1_OEN) &= ~(BIT2);

}



void MHal_GPIO_mini_LVDS_2_Oen(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_2_OEN) &= ~(BIT1);

}


void MHal_GPIO_mini_LVDS_3_Oen(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_3_OEN)&= ~(BIT0);

}





void MHal_GPIO_mini_LVDS_4_Oen(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_4_OEN) &= ~(BIT7);

}



void MHal_GPIO_mini_LVDS_5_Oen(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_5_OEN) &= ~(BIT6);

}

void MHal_GPIO_mini_LVDS_6_Oen(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_6_OEN) &= ~(BIT5);

}

void MHal_GPIO_mini_LVDS_7_Oen(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_7_OEN) &= ~(BIT4);

}

void MHal_GPIO_LCK_Oen(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LCK_OEN) &= ~(BIT3);

}


void MHal_GPIO_LDE_Oen(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LDE_OEN) &= ~(BIT2);
}


void MHal_GPIO_LHSYNC_Oen(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LHSYNC_OEN) &= ~(BIT1);

}


void MHal_GPIO_LVSYNC_Oen(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LVSYNC_OEN) &= ~(BIT0);

}


void MHal_GPIO_PCM_WE_N_Oen(void)
{
    MHal_GPIO_REG(REG_PCM_WE_N_OEN) &= ~(BIT2);
}




// this section set to disable output

void MHal_GPIO_PM_SPI_CZ_Odn(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CZ_OEN) |= BIT4;
}

void MHal_GPIO_PM_SPI_CK_Odn(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CK_OEN) |= BIT5;
}
void MHal_GPIO_PM_SPI_DI_Odn(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DI_OEN) |= BIT6;
}

void MHal_GPIO_PM_SPI_DO_Odn(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DO_OEN) |= BIT7;
}

void MHal_GPIO_IRIN_Odn(void)
{
    MHal_GPIO_REG(REG_IRIN_OEN) |= BIT0;
}

void MHal_GPIO_CEC_Odn(void)
{
    MHal_GPIO_REG(REG_CEC_OEN) |= BIT2;

}

void MHal_GPIO_GPIO_PM0_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM0_OEN) |= BIT0;

}

void MHal_GPIO_GPIO_PM1_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM1_OEN) |= BIT1;

}

void MHal_GPIO_GPIO_PM2_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM2_OEN) |= BIT2;

}

void MHal_GPIO_GPIO_PM3_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM3_OEN) |= BIT3;

}

void MHal_GPIO_GPIO_PM4_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM4_OEN) |= BIT4;
}

void MHal_GPIO_GPIO_PM5_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM5_OEN) |= BIT5;

}

void MHal_GPIO_GPIO_PM6_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM6_OEN) |= BIT6;

}

void MHal_GPIO_GPIO_PM7_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM7_OEN) |= BIT7;

}

void MHal_GPIO_GPIO_PM8_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM8_OEN) |= BIT0;

}

void MHal_GPIO_GPIO_PM9_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM9_OEN) |= BIT1;

}

void MHal_GPIO_GPIO_PM10_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM10_OEN) |= BIT2;

}

void MHal_GPIO_GPIO_PM11_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM11_OEN) |= BIT3;

}

void MHal_GPIO_GPIO_PM12_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO_PM12_OEN) |= BIT4;

}

void MHal_GPIO_HOTPLUGA_Odn(void)
{
    MHal_GPIO_REG(REG_HOTPLUGA_OEN) |= BIT0;

}


void MHal_GPIO_HOTPLUGB_Odn(void)
{
    MHal_GPIO_REG(REG_HOTPLUGB_OEN) |= BIT1;

}

void MHal_GPIO_HOTPLUGC_Odn(void)
{
    MHal_GPIO_REG(REG_HOTPLUGC_OEN) |= BIT2;

}

void MHal_GPIO_HOTPLUGD_Odn(void)
{
    MHal_GPIO_REG(REG_HOTPLUGD_OEN) |= BIT3;

}


void MHal_GPIO_DDCDA_CK_Odn(void)
{
    MHal_GPIO_REG(REG_DDCDA_CK_OEN) |= BIT1;

}

void MHal_GPIO_DDCDA_DA_Odn(void)
{
    MHal_GPIO_REG(REG_DDCDA_DA_OEN) |= BIT5;

}

void MHal_GPIO_DDCDB_CK_Odn(void)
{
    MHal_GPIO_REG(REG_DDCDB_DA_OEN) |= BIT1;

}

void MHal_GPIO_DDCDB_DA_Odn(void)
{
    MHal_GPIO_REG(REG_DDCDB_CK_OEN) |= BIT5;

}

void MHal_GPIO_DDCDC_CK_Odn(void)
{
    MHal_GPIO_REG(REG_DDCDC_CK_OEN) |= BIT1;
}

void MHal_GPIO_DDCDC_DA_Odn(void)
{
    MHal_GPIO_REG(REG_DDCDC_DA_OEN)  |= BIT5;
}

void MHal_GPIO_DDCDD_CK_Odn(void)
{
    MHal_GPIO_REG(REG_DDCDD_CK_OEN) |= BIT1;
}

void MHal_GPIO_DDCDD_DA_Odn(void)
{
    MHal_GPIO_REG(REG_DDCDD_DA_OEN) |= BIT5;

}

void MHal_GPIO_SAR0_Odn(void)
{
    MHal_GPIO_REG(REG_SAR0_OEN) |= BIT0;

}

void MHal_GPIO_SAR1_Odn(void)
{
    MHal_GPIO_REG(REG_SAR1_OEN) |= BIT1;

}

void MHal_GPIO_SAR2_Odn(void)
{
    MHal_GPIO_REG(REG_SAR2_OEN) |= BIT2;
}

void MHal_GPIO_SAR3_Odn(void)
{
    MHal_GPIO_REG(REG_SAR3_OEN) |= BIT3;
}

void MHal_GPIO_SAR4_Odn(void)
{
    MHal_GPIO_REG(REG_SAR4_OEN) |= BIT4;
}

void MHal_GPIO_GPIO0_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO0_OEN) |= BIT0;

}

void MHal_GPIO_GPIO1_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO1_OEN) |= BIT1;

}

void MHal_GPIO_GPIO2_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO2_OEN) |= BIT2;

}

void MHal_GPIO_GPIO3_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO3_OEN) |= BIT3;

}

void MHal_GPIO_GPIO4_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO4_OEN) |= BIT4;
}

void MHal_GPIO_GPIO5_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO5_OEN) |= BIT5;
}

void MHal_GPIO_GPIO6_Odn(void)
{

    MHal_GPIO_REG(REG_GPIO6_OEN) |= BIT6;

}

void MHal_GPIO_GPIO7_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO7_OEN) |= BIT7;

}

void MHal_GPIO_GPIO8_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO8_OEN) |= BIT0;
}

void MHal_GPIO_GPIO9_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO9_OEN) |= BIT1;
}

void MHal_GPIO_GPIO10_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO10_OEN) |= BIT2;

}

void MHal_GPIO_GPIO11_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO11_OEN) |= BIT3;

}

void MHal_GPIO_GPIO12_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO12_OEN) |= BIT4;

}
void MHal_GPIO_GPIO13_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO13_OEN) |= BIT5;

}
void MHal_GPIO_GPIO14_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO14_OEN) |= BIT6;
}
void MHal_GPIO_GPIO15_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO15_OEN) |= BIT7;

}
void MHal_GPIO_GPIO16_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO16_OEN) |= BIT0;

}
void MHal_GPIO_GPIO17_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO17_OEN) |= BIT1;

}
void MHal_GPIO_GPIO18_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO18_OEN) |= BIT2;

}

void MHal_GPIO_GPIO19_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO19_OEN) |= BIT3;

}


void MHal_GPIO_GPIO20_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO20_OEN) |= BIT4;

}


void MHal_GPIO_GPIO21_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO21_OEN) |= BIT5;

}


void MHal_GPIO_GPIO22_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO22_OEN) |= BIT6;
}

void MHal_GPIO_GPIO23_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO23_OEN) |= BIT7;
}


void MHal_GPIO_GPIO24_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO24_OEN) |= BIT0;

}
void MHal_GPIO_GPIO25_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO25_OEN) |= BIT1;

}

void MHal_GPIO_GPIO26_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO26_OEN) |= BIT2;

}
void MHal_GPIO_GPIO27_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO27_OEN) |= BIT3;

}

void MHal_GPIO_UART_RX2_Odn(void)
{
    MHal_GPIO_REG(REG_UART_RX2_OEN) |= BIT4;

}
void MHal_GPIO_UART_TX2_Odn(void)
{
    MHal_GPIO_REG(REG_UART_TX2_OEN) |= BIT5;

}

void MHal_GPIO_PWM0_Odn(void)
{
    MHal_GPIO_REG(REG_PWM0_OEN) |= BIT6;
}
void MHal_GPIO_PWM1_Odn(void)
{
    MHal_GPIO_REG(REG_PWM1_OEN) |= BIT7;

}
void MHal_GPIO_PWM2_Odn(void)
{
    MHal_GPIO_REG(REG_PWM2_OEN) |= BIT0;
}


void MHal_GPIO_PWM3_Odn(void)
{
    MHal_GPIO_REG(REG_PWM3_OEN) |= BIT1;
}


void MHal_GPIO_PWM4_Odn(void)
{
    MHal_GPIO_REG(REG_PWM4_OEN) |= BIT2;
}

void MHal_GPIO_DDCR_DA_Odn(void)
{
    MHal_GPIO_REG(REG_DDCR_DA_OEN) |= BIT3;

}

void MHal_GPIO_DDCR_CK_Odn(void)
{
    MHal_GPIO_REG(REG_DDCR_CK_OEN) |= BIT4;

}


void MHal_GPIO_TGPIO0_Odn(void)
{
    MHal_GPIO_REG(REG_TGPIO0_OEN) |= BIT5;

}


void MHal_GPIO_TGPIO1_Odn(void)
{
    MHal_GPIO_REG(REG_TGPIO1_OEN) |= BIT6;

}


void MHal_GPIO_TGPIO2_Odn(void)
{
    MHal_GPIO_REG(REG_TGPIO2_OEN)|= BIT7;
}


void MHal_GPIO_TGPIO3_Odn(void)
{
    MHal_GPIO_REG(REG_TGPIO3_OEN) |= BIT0;
}


void MHal_GPIO_TS0_D0_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_D0_OEN) |= BIT0;

}


void MHal_GPIO_TS0_D1_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_D1_OEN) |= BIT1;

}


void MHal_GPIO_TS0_D2_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_D2_OEN) |= BIT2;

}


void MHal_GPIO_TS0_D3_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_D3_OEN) |= BIT3;

}
void MHal_GPIO_TS0_D4_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_D4_OEN) |= BIT4;

}


void MHal_GPIO_TS0_D5_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_D5_OEN) |= BIT5;

}


void MHal_GPIO_TS0_D6_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_D6_OEN) |= BIT6;
}



void MHal_GPIO_TS0_D7_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_D7_OEN) |= BIT7;

}


void MHal_GPIO_TS0_VLD_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_VLD_OEN) |= BIT0;
}


void MHal_GPIO_TS0_SYNC_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_SYNC_OEN) |= BIT1;
}



void MHal_GPIO_TS0_CLK_Odn(void)
{
    MHal_GPIO_REG(REG_TS0_CLK_OEN) |= BIT2;
}

void MHal_GPIO_TS1_D0_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_D0_OEN) |= (BIT0);

}
void MHal_GPIO_TS1_D1_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_D1_OEN) |= (BIT1);

}


void MHal_GPIO_TS1_D2_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_D2_OEN) |= (BIT2);

}

void MHal_GPIO_TS1_D3_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_D3_OEN) |= (BIT3);

}

void MHal_GPIO_TS1_D4_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_D4_OEN) |= (BIT4);

}


void MHal_GPIO_TS1_D5_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_D5_OEN) |= (BIT5);

}


void MHal_GPIO_TS1_D6_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_D6_OEN) |= (BIT6);

}


void MHal_GPIO_TS1_D7_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_D7_OEN) |= (BIT7);

}


void MHal_GPIO_TS1_VLD_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_VLD_OEN) |= (BIT0);

}


void MHal_GPIO_TS1_SYNC_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_SYNC_OEN) |= (BIT1);
}


void MHal_GPIO_TS1_CLK_Odn(void)
{
    MHal_GPIO_REG(REG_TS1_CLK_OEN) |= (BIT2);

}


void MHal_GPIO_PCM_A4_Odn(void)
{

    if(GPIO99)
    {
        //PCM_GPIO0
        MHal_GPIO_REG(REG_PCM_A4_OEN1) |= (BIT0);

    }
    else
    {
        //NOR_GPIO15
        MHal_GPIO_REG(REG_PCM_A4_OEN2) |= (BIT7);



    }

}



void MHal_GPIO_PCM_WAIT_N_Odn(void)
{
    MHal_GPIO_REG(REG_PCM_WAIT_N_OEN) |= (BIT1);

}
void MHal_GPIO_PCM_A5_Odn(void)
{

    if(GPIO101)
    {
        //PCM_GPIO2
        MHal_GPIO_REG(REG_PCM_A5_OEN1) |= (BIT2);

    }
    else
    {
        //NOR_GPIO14
        MHal_GPIO_REG(REG_PCM_A5_OEN2) |= (BIT6);

    }

}


void MHal_GPIO_PCM_A6_Odn(void)
{

    if(GPIO102)
    {
        //PCM_GPIO3
        MHal_GPIO_REG(REG_PCM_A6_OEN1) |= (BIT3);

    }
    else
    {
        //NOR_GPIO13
        MHal_GPIO_REG(REG_PCM_A6_OEN2) |= (BIT5);

    }

}



void MHal_GPIO_PCM_A7_Odn(void)
{

    if(GPIO103)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A7_OEN1)  |= (BIT4);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A7_OEN2) |= (BIT4);

    }

}


void MHal_GPIO_PCM_A12_Odn(void)
{

    if(GPIO104)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A12_OEN1) |= (BIT5);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A12_OEN2) |= (BIT3);

    }

}



void MHal_GPIO_PCM_IRQA_N_Odn(void)
{
    MHal_GPIO_REG(REG_PCM_IRQA_N_OEN) |= (BIT6);

}

void MHal_GPIO_PCM_A14_Odn(void)
{

    if(GPIO106)
    {
        MHal_GPIO_REG(REG_PCM_A14_OEN1) |= (BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A14_OEN2) |= (BIT2);

    }

}


void MHal_GPIO_PCM_A13_Odn(void)
{

    if(GPIO107)
    {
        MHal_GPIO_REG(REG_PCM_A13_OEN1) |= (BIT0);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A13_OEN2)  |= (BIT1);

    }

}

void MHal_GPIO_PCM_A8_Odn(void)
{

    if(GPIO108)
    {
        MHal_GPIO_REG(REG_PCM_A8_OEN1) |= (BIT1);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_OEN2) |= (BIT0);

    }

}



void MHal_GPIO_PCM_IOWR_N_Odn(void)
{
    MHal_GPIO_REG(REG_PCM_IOWR_N_OEN) |= (BIT2);

}


void MHal_GPIO_PCM_A9_Odn(void)
{

    if(GPIO110)
    {
        MHal_GPIO_REG(REG_PCM_A8_OEN1) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_OEN2) |= (BIT7);

    }

}



void MHal_GPIO_PCM_IORD_N_Odn(void)
{
    MHal_GPIO_REG(REG_PCM_IORD_N_OEN) |= (BIT4);

}



void MHal_GPIO_PCM_A11_Odn(void)
{

    if(GPIO112)
    {
        MHal_GPIO_REG(REG_PCM_A11_OEN2) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A11_OEN2) |= (BIT6);

    }

}


void MHal_GPIO_PCM_OE_N_Odn(void)
{
    MHal_GPIO_REG(REG_PCM_OE_N_OEN) |= (BIT6);

}


void MHal_GPIO_PCM_A10_Odn(void)
{

    if(GPIO114)
    {
        MHal_GPIO_REG(REG_PCM_A10_OEN1) |= (BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A10_OEN2) |= (BIT5);
    }


}
void MHal_GPIO_PCM_CE_N_Odn(void)
{

    MHal_GPIO_REG(REG_PCM_CE_N_OEN) |= (BIT0);

}


void MHal_GPIO_PCM_D7_Odn(void)
{

    if(GPIO116)
    {
        MHal_GPIO_REG(REG_PCM_D7_OEN1) |= (BIT1);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D7_OEN2) |= (BIT4);
    }

}


void MHal_GPIO_PCM_D6_Odn(void)
{

    if(GPIO117)
    {
        MHal_GPIO_REG(REG_PCM_D6_OEN1) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D6_OEN2) |= (BIT3);
    }

}



void MHal_GPIO_PCM_D5_Odn(void)
{

    if(GPIO118)
    {
        MHal_GPIO_REG(REG_PCM_D5_OEN1) |= (BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D5_OEN2) |= (BIT2);

    }

}


void MHal_GPIO_PCM_D4_Odn(void)
{

    if(GPIO119)
    {
        MHal_GPIO_REG(REG_PCM_D4_OEN1) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D4_OEN2) |= (BIT1);
    }

}


void MHal_GPIO_PCM_D3_Odn(void)
{

    if(GPIO120)
    {
        MHal_GPIO_REG(REG_PCM_D3_OEN1)  |= (BIT6);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D3_OEN2) |= (BIT0);
    }

}


void MHal_GPIO_PCM_A3_Odn(void)
{
    if(GPIO121)
    {
        MHal_GPIO_REG(REG_PCM_A3_OEN1) |= (BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_OEN2) |= (BIT6);

    }

}



void MHal_GPIO_PCM_A2_Odn(void)
{


    if(GPIO122)
    {
        MHal_GPIO_REG(REG_PCM_A2_OEN1) |= (BIT0);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_OEN2) |= (BIT7);
    }

}


void MHal_GPIO_PCM_REG_N_Odn(void)
{
    MHal_GPIO_REG(REG_PCM_REG_N_OEN) |= (BIT1);
}

void MHal_GPIO_PCM_A1_Odn(void)
{

    if(GPIO124)
    {
        MHal_GPIO_REG(REG_PCM_A1_OEN1) |= (BIT2);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A1_OEN2) |= (BIT0);

    }

}




void MHal_GPIO_PCM_A0_Odn(void)
{

    if(GPIO125)
    {
        MHal_GPIO_REG(REG_PCM_A0_OEN1) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A0_OEN2) |= (BIT1);

    }

}



void MHal_GPIO_PCM_D0_Odn(void)
{


    if(GPIO126)
    {
        MHal_GPIO_REG(REG_PCM_A1_OEN1) |= (BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D0_OEN2) |= (BIT2);

    }

}


void MHal_GPIO_PCM_D1_Odn(void)
{


    if(GPIO127)
    {
        MHal_GPIO_REG(REG_PCM_D1_OEN1) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D1_OEN2) |= (BIT3);

    }

}


void MHal_GPIO_PCM_D2_Odn(void)
{


    if(GPIO128)
    {
        MHal_GPIO_REG(REG_PCM_D2_OEN1) |= (BIT6);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D2_OEN2)  |= (BIT4);

    }

}



void MHal_GPIO_PCM_RESET_Odn(void)
{

    MHal_GPIO_REG(REG_PCM_RESET_OEN) |= (BIT7);

}



void MHal_GPIO_PCM_CD_N_Odn(void)
{

    MHal_GPIO_REG(REG_PCM_CD_N_OEN) |= (BIT0);

}
void MHal_GPIO_PCM2_CE_N_Odn(void)
{

    MHal_GPIO_REG(REG_PCM2_CE_N_OEN)|= (BIT0);

}



void MHal_GPIO_PCM2_IRQA_N_Odn(void)
{

    MHal_GPIO_REG(REG_PCM2_IRQA_N_OEN)|= (BIT1);

}



void MHal_GPIO_PCM2_WAIT_N_Odn(void)
{

    MHal_GPIO_REG(REG_PCM2_WAIT_N_OEN) |= (BIT2);

}



void MHal_GPIO_PCM2_RESET_Odn(void)
{

    MHal_GPIO_REG(REG_PCM2_RESET_OEN) |= (BIT3);

}



void MHal_GPIO_PCM2_CD_N_Odn(void)
{

    MHal_GPIO_REG(REG_PCM2_CD_N_OEN) |= (BIT4);

}



void MHal_GPIO_PF_AD15_Odn(void)
{

    if(GPIO136)
    {
        MHal_GPIO_REG(REG_PF_AD15_OEN1) |= (BIT0);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_AD15_OEN2) |= (BIT0);

    }

}



void MHal_GPIO_PF_CE0Z_Odn(void)
{

    if(GPIO137)
    {
        MHal_GPIO_REG(REG_PF_CE0Z_OEN1) |= (BIT1);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE0Z_OEN2) |= (BIT1);

    }

}


void MHal_GPIO_PF_CE1Z_Odn(void)
{

    if(GPIO138)
    {
        MHal_GPIO_REG(REG_PF_CE1Z_OEN1) |= (BIT2);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE1Z_OEN2) |= (BIT2);
    }

}


void MHal_GPIO_PF_OEZ_Odn(void)
{

    if(GPIO139)
    {
        MHal_GPIO_REG(REG_PF_OEZ_OEN1)  |= (BIT3);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_OEZ_OEN2)  |= (BIT3);
    }

}



void MHal_GPIO_PF_WEZ_Odn(void)
{

    if(GPIO140)
    {
        MHal_GPIO_REG(REG_PF_WEZ_OEN1) |= (BIT4);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_WEZ_OEN2) |= (BIT4);

    }

}



void MHal_GPIO_PF_ALE_Odn(void)
{

    if(GPIO141)
    {
        MHal_GPIO_REG(REG_PF_ALE_OEN1) |= (BIT5);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_ALE_OEN2) |= (BIT5);

    }

}



void MHal_GPIO_F_RBZ_Odn(void)
{
    MHal_GPIO_REG(REG_F_RBZ_OEN) |= (BIT6);
}

//balup_090907
void MHal_GPIO_GPIO48_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO48_OEN) |= (BIT4);
}

void MHal_GPIO_GPIO49_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO48_OEN) |= (BIT5);
}

void MHal_GPIO_GPIO53_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO53_OEN) |= (BIT1);
}

void MHal_GPIO_GPIO52_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO53_OEN) |= (BIT0);
}

void MHal_GPIO_GPIO47_Odn(void)
{
    MHal_GPIO_REG(REG_GPIO48_OEN) |= (BIT3);
}






void MHal_GPIO_TCON0_Odn(void)
{
    MHal_GPIO_REG(REG_TCON0_OEN) |= (BIT0);
}



void MHal_GPIO_TCON1_Odn(void)
{
    MHal_GPIO_REG(REG_TCON1_OEN) |= (BIT1);
}


void MHal_GPIO_TCON2_Odn(void)
{
    MHal_GPIO_REG(REG_TCON2_OEN) |= (BIT2);
}



void MHal_GPIO_TCON3_Odn(void)
{
    MHal_GPIO_REG(REG_TCON3_OEN) |= (BIT3);
}


void MHal_GPIO_TCON4_Odn(void)
{
    MHal_GPIO_REG(REG_TCON4_OEN) |= (BIT4);
}



void MHal_GPIO_TCON5_Odn(void)
{
    MHal_GPIO_REG(REG_TCON5_OEN) |= (BIT5);
}


void MHal_GPIO_TCON6_Odn(void)
{
    MHal_GPIO_REG(REG_TCON6_OEN) |= (BIT6);
}

void MHal_GPIO_TCON7_Odn(void)
{
    MHal_GPIO_REG(REG_TCON7_OEN)  |= (BIT7);
}


void MHal_GPIO_TCON8_Odn(void)
{
    MHal_GPIO_REG(REG_TCON8_OEN) |= (BIT0);
}


void MHal_GPIO_TCON9_Odn(void)
{
    MHal_GPIO_REG(REG_TCON9_OEN) |= (BIT1);
}


void MHal_GPIO_TCON10_Odn(void)
{
    MHal_GPIO_REG(REG_TCON10_OEN) |= (BIT2);
}



void MHal_GPIO_TCON11_Odn(void)
{
    MHal_GPIO_REG(REG_TCON11_OEN) |= (BIT3);
}


void MHal_GPIO_TCON12_Odn(void)
{
    MHal_GPIO_REG(REG_TCON12_OEN) |= (BIT4);
}


void MHal_GPIO_TCON13_Odn(void)
{
    MHal_GPIO_REG(REG_TCON13_OEN) |= (BIT5);
}



void MHal_GPIO_TCON14_Odn(void)
{
    MHal_GPIO_REG(REG_TCON14_OEN) |= (BIT6);
}


void MHal_GPIO_TCON15_Odn(void)
{
    MHal_GPIO_REG(REG_TCON15_OEN) |= (BIT7);
}


void MHal_GPIO_TCON16_Odn(void)
{
    MHal_GPIO_REG(REG_TCON16_OEN) |= (BIT0);
}


void MHal_GPIO_TCON17_Odn(void)
{
    MHal_GPIO_REG(REG_TCON17_OEN)  |= (BIT1);
}


void MHal_GPIO_TCON18_Odn(void)
{
    MHal_GPIO_REG(REG_TCON18_OEN)  |= (BIT2);
}


void MHal_GPIO_TCON19_Odn(void)
{
    MHal_GPIO_REG(REG_TCON19_OEN)  |= (BIT3);
}



void MHal_GPIO_TCON20_Odn(void)
{
    MHal_GPIO_REG(REG_TCON20_OEN) |= (BIT4);
}


void MHal_GPIO_TCON21_Odn(void)
{
    MHal_GPIO_REG(REG_TCON21_OEN)  |= (BIT5);
}



void MHal_GPIO_ET_COL_Odn(void)
{

    MHal_GPIO_REG(REG_ET_COL_OEN)  |= (BIT0);

}


void MHal_GPIO_ET_TXD1_Odn(void)
{

    MHal_GPIO_REG(REG_ET_TXD1_OEN) |= (BIT1);

}


void MHal_GPIO_ET_TXD0_Odn(void)
{

    MHal_GPIO_REG(REG_ET_TXD0_OEN) |= (BIT2);


}



void MHal_GPIO_ET_TX_EN_Odn(void)
{

    MHal_GPIO_REG(REG_ET_TX_EN_OEN) |= (BIT3);

}


void MHal_GPIO_ET_TX_CLK_Odn(void)
{
    MHal_GPIO_REG(REG_ET_TX_CLK_OEN) |= (BIT4);

}


void MHal_GPIO_ET_RXD0_Odn(void)
{
    MHal_GPIO_REG(REG_ET_RXD0_OEN) |= (BIT5);
}

void MHal_GPIO_ET_RXD1_Odn(void)
{
    MHal_GPIO_REG(REG_ET_RXD1_OEN) |= (BIT6);
}

void MHal_GPIO_ET_MDC_Odn(void)
{
    MHal_GPIO_REG(REG_ET_MDC_OEN) |= (BIT7);
}


void MHal_GPIO_ET_EDIO_Odn(void)
{
    MHal_GPIO_REG(REG_ET_EDIO_OEN) |= (BIT0);
}


void MHal_GPIO_I2S_IN_WS_Odn(void)
{

    MHal_GPIO_REG(REG_I2S_IN_WS_OEN) |= (BIT0);

}


void MHal_GPIO_I2S_IN_BCK_Odn(void)
{
    MHal_GPIO_REG(REG_I2S_IN_BCK_OEN)  |= (BIT1);

}



void MHal_GPIO_I2S_IN_SD_Odn(void)
{

    MHal_GPIO_REG(REG_I2S_IN_SD_OEN) |= (BIT2);

}


void MHal_GPIO_SPDIF_IN_Odn(void)
{
    MHal_GPIO_REG(REG_SPDIF_IN_OEN) |= (BIT3);
}


void MHal_GPIO_SPDIF_OUT_Odn(void)
{

    MHal_GPIO_REG(REG_SPDIF_OUT_OEN) |= (BIT4);

}


void MHal_GPIO_I2S_OUT_MCK_Odn(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_MCK_OEN)|= (BIT5);

}


void MHal_GPIO_I2S_OUT_WS_Odn(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_WS_OEN) |= (BIT6);

}


void MHal_GPIO_I2S_OUT_BCK_Odn(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_BCK_OEN) |= (BIT7);

}


void MHal_GPIO_I2S_OUT_SD_Odn(void)
{
    MHal_GPIO_REG(REG_I2S_OUT_SD_OEN) |= (BIT0);
}



void MHal_GPIO_I2S_OUT_SD1_Odn(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD1_OEN) |= (BIT1);

}



void MHal_GPIO_I2S_OUT_SD2_Odn(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD2_OEN) |= (BIT2);

}



void MHal_GPIO_I2S_OUT_SD3_Odn(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD3_OEN) |= (BIT3);

}


void MHal_GPIO_B_ODD_0_Odn(void)
{

    MHal_GPIO_REG(REG_B_ODD_0_OEN) |= (BIT3);

}



void MHal_GPIO_B_ODD_1_Odn(void)
{

    MHal_GPIO_REG(REG_B_ODD_1_OEN) |= (BIT2);

}



void MHal_GPIO_B_ODD_2_Odn(void)
{

    MHal_GPIO_REG(REG_B_ODD_2_OEN) |= (BIT1);

}


void MHal_GPIO_B_ODD_3_Odn(void)
{

    MHal_GPIO_REG(REG_B_ODD_3_OEN) |= (BIT0);

}





void MHal_GPIO_B_ODD_4_Odn(void)
{

    MHal_GPIO_REG(REG_B_ODD_4_OEN) |= (BIT7);

}



void MHal_GPIO_B_ODD_5_Odn(void)
{

    MHal_GPIO_REG(REG_B_ODD_5_OEN) |= (BIT6);

}

void MHal_GPIO_B_ODD_6_Odn(void)
{

    MHal_GPIO_REG(REG_B_ODD_6_OEN) |= (BIT5);

}

void MHal_GPIO_B_ODD_7_Odn(void)
{

    MHal_GPIO_REG(REG_B_ODD_7_OEN) |= (BIT4);
}

void MHal_GPIO_G_ODD_0_Odn(void)
{

    MHal_GPIO_REG(REG_G_ODD_0_OEN) |= (BIT3);

}



void MHal_GPIO_G_ODD_1_Odn(void)
{

    MHal_GPIO_REG(REG_G_ODD_1_OEN) |= (BIT2);

}



void MHal_GPIO_G_ODD_2_Odn(void)
{

    MHal_GPIO_REG(REG_G_ODD_2_OEN) |= (BIT1);

}


void MHal_GPIO_G_ODD_3_Odn(void)
{

    MHal_GPIO_REG(REG_G_ODD_3_OEN) |= (BIT0);

}





void MHal_GPIO_G_ODD_4_Odn(void)
{

    MHal_GPIO_REG(REG_G_ODD_4_OEN) |= (BIT7);

}



void MHal_GPIO_G_ODD_5_Odn(void)
{

    MHal_GPIO_REG(REG_G_ODD_5_OEN) |= (BIT6);

}

void MHal_GPIO_G_ODD_6_Odn(void)
{

    MHal_GPIO_REG(REG_G_ODD_6_OEN) |= (BIT5);

}

void MHal_GPIO_G_ODD_7_Odn(void)
{

    MHal_GPIO_REG(REG_G_ODD_7_OEN)|= (BIT4);

}

void MHal_GPIO_R_ODD_0_Odn(void)
{

    MHal_GPIO_REG(REG_R_ODD_0_OEN) |= (BIT3);

}



void MHal_GPIO_R_ODD_1_Odn(void)
{

    MHal_GPIO_REG(REG_R_ODD_1_OEN) |= (BIT2);

}



void MHal_GPIO_R_ODD_2_Odn(void)
{

    MHal_GPIO_REG(REG_R_ODD_2_OEN) |= (BIT1);

}


void MHal_GPIO_R_ODD_3_Odn(void)
{

    MHal_GPIO_REG(REG_R_ODD_3_OEN) |= (BIT0);

}





void MHal_GPIO_R_ODD_4_Odn(void)
{

    MHal_GPIO_REG(REG_R_ODD_4_OEN) |= (BIT7);

}



void MHal_GPIO_R_ODD_5_Odn(void)
{

    MHal_GPIO_REG(REG_R_ODD_5_OEN) |= (BIT6);

}

void MHal_GPIO_R_ODD_6_Odn(void)
{

    MHal_GPIO_REG(REG_R_ODD_6_OEN) |= (BIT5);

}

void MHal_GPIO_R_ODD_7_Odn(void)
{

    MHal_GPIO_REG(REG_R_ODD_7_OEN) |= (BIT4);

}

void MHal_GPIO_mini_LVDS_0_Odn(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_0_OEN) |= (BIT3);

}



void MHal_GPIO_mini_LVDS_1_Odn(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_1_OEN) |= (BIT2);

}



void MHal_GPIO_mini_LVDS_2_Odn(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_2_OEN) |= (BIT1);

}


void MHal_GPIO_mini_LVDS_3_Odn(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_3_OEN)|= (BIT0);

}





void MHal_GPIO_mini_LVDS_4_Odn(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_4_OEN) |= (BIT7);

}



void MHal_GPIO_mini_LVDS_5_Odn(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_5_OEN) |= (BIT6);

}

void MHal_GPIO_mini_LVDS_6_Odn(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_6_OEN) |= (BIT5);

}

void MHal_GPIO_mini_LVDS_7_Odn(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_7_OEN) |= (BIT4);

}

void MHal_GPIO_LCK_Odn(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LCK_OEN) |= (BIT3);

}


void MHal_GPIO_LDE_Odn(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LDE_OEN) |= (BIT2);
}


void MHal_GPIO_LHSYNC_Odn(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LHSYNC_OEN) |= (BIT1);

}


void MHal_GPIO_LVSYNC_Odn(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LVSYNC_OEN) |= (BIT0);

}


void MHal_GPIO_PCM_WE_N_Odn(void)
{
    MHal_GPIO_REG(REG_PCM_WE_N_OEN) |= (BIT2);
}



//unctions of this section set to get input

U8 MHal_GPIO_PM_SPI_CZ_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PM_SPI_CZ_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PM_SPI_CK_In(void)
{

    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_PM_SPI_CK_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);
    return (u8RetGPIO &= BIT0);
}
U8 MHal_GPIO_PM_SPI_DI_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PM_SPI_DI_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PM_SPI_DO_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PM_SPI_DO_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_IRIN_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_IRIN_IN) ;
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_CEC_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_CEC_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM0_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM0_IN) ;

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM1_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM1_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM2_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM2_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM3_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM3_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM4_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM4_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM5_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM5_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM6_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM6_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM7_In(void)
{
        U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM7_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM8_In(void)
{
        U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM8_IN) ;

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM9_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM9_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM10_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM10_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM11_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM11_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO_PM12_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO_PM12_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_HOTPLUGA_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_HOTPLUGA_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_HOTPLUGB_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_HOTPLUGB_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_HOTPLUGC_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_HOTPLUGC_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_HOTPLUGD_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_HOTPLUGD_IN) ;
    u8RetGPIO = (u8RetGPIO>>0);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_DDCDA_CK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCDA_CK_IN) ;
    u8RetGPIO = (u8RetGPIO>>0);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCDA_DA_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCDA_DA_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCDB_CK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCDB_DA_IN) ;
    u8RetGPIO = (u8RetGPIO>>0);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCDB_DA_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCDB_CK_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCDC_CK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCDC_CK_IN) ;
    u8RetGPIO = (u8RetGPIO>>0);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCDC_DA_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCDC_DA_IN)  ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCDD_CK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCDD_CK_IN) ;
    u8RetGPIO = (u8RetGPIO>>0);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCDD_DA_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCDD_DA_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_SAR0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_SAR0_IN) ;

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_SAR1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_SAR1_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_SAR2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_SAR2_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_SAR3_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_SAR3_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_SAR4_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_SAR4_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO0_IN) ;

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO1_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO2_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO3_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO3_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO4_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO4_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO5_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO5_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO6_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_GPIO6_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO7_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO7_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO8_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO8_IN) ;
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO9_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO9_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO10_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO10_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO11_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO11_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO12_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO12_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO13_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO13_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}
U8 MHal_GPIO_GPIO14_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO14_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}
U8 MHal_GPIO_GPIO15_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO15_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}
U8 MHal_GPIO_GPIO16_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO16_IN) ;

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO17_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO17_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}
U8 MHal_GPIO_GPIO18_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO18_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO19_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO19_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_GPIO20_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO20_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_GPIO21_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO21_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_GPIO22_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO22_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO23_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO23_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_GPIO24_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO24_IN) ;

    return (u8RetGPIO &= BIT0);
}
U8 MHal_GPIO_GPIO25_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO25_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO26_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO26_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO27_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO27_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_UART_RX2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_UART_RX2_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}
U8 MHal_GPIO_UART_TX2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_UART_TX2_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PWM0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PWM0_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PWM1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PWM1_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PWM2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PWM2_IN) ;
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PWM3_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PWM3_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PWM4_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PWM4_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCR_DA_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCR_DA_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_DDCR_CK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_DDCR_CK_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TGPIO0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TGPIO0_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TGPIO1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TGPIO1_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TGPIO2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TGPIO2_IN);
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TGPIO3_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TGPIO3_IN) ;
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS0_D0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_D0_IN) ;

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS0_D1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_D1_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS0_D2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_D2_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS0_D3_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_D3_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}
U8 MHal_GPIO_TS0_D4_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_D4_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS0_D5_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_D5_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS0_D6_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_D6_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_TS0_D7_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_D7_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS0_VLD_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_VLD_IN) ;
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS0_SYNC_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_SYNC_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_TS0_CLK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS0_CLK_IN) ;
     u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_TS1_D0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_D0_IN) ;
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_TS1_D1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_D1_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS1_D2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_D2_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_TS1_D3_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_D3_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_TS1_D4_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_D4_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS1_D5_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_D5_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS1_D6_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_D6_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS1_D7_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_D7_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS1_VLD_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_VLD_IN) ;
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS1_SYNC_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_SYNC_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TS1_CLK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TS1_CLK_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_A4_In(void)
{
    U8 u8RetGPIO;

    if(GPIO99)
    {
        //PCM_GPIO0
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A4_IN1) ;

    }
    else
    {
        //NOR_GPIO15
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A4_IN2) ;
        u8RetGPIO = (u8RetGPIO>>7);
    }

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM_WAIT_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM_WAIT_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PCM_A5_In(void)
{
    U8 u8RetGPIO;

    if(GPIO101)
    {        //PCM_GPIO2

        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A5_IN1) ;
        u8RetGPIO = (u8RetGPIO>>2);
    }
    else
    {
        //NOR_GPIO14
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A5_IN2) ;
        u8RetGPIO = (u8RetGPIO>>6);
    }
    return (u8RetGPIO &= BIT0);
}




U8 MHal_GPIO_PCM_A6_In(void)
{
    U8 u8RetGPIO;

    if(GPIO102)
    {
        //PCM_GPIO3
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A6_IN1) ;
        u8RetGPIO = (u8RetGPIO>>3);

    }
    else
    {
        //NOR_GPIO13
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A6_IN2) ;
        u8RetGPIO = (u8RetGPIO>>5);
    }
    return (u8RetGPIO &= BIT0);
}




U8 MHal_GPIO_PCM_A7_In(void)
{
    U8 u8RetGPIO;

    if(GPIO103)
    {
        //PCM_GPIO4
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A7_IN1)  ;
        u8RetGPIO = (u8RetGPIO>>4);

    }
    else
    {
        //NOR_GPIO12
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A7_IN2) ;
        u8RetGPIO = (u8RetGPIO>>4);
    }
    return (u8RetGPIO &= BIT0);
}




U8 MHal_GPIO_PCM_A12_In(void)
{
    U8 u8RetGPIO;

    if(GPIO104)
    {
        //PCM_GPIO4
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A12_IN1) ;
        u8RetGPIO = (u8RetGPIO>>5);
    }
    else
    {        //NOR_GPIO12
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A12_IN2) ;
        u8RetGPIO = (u8RetGPIO>>3);
    }
    return (u8RetGPIO &= BIT0);
}





U8 MHal_GPIO_PCM_IRQA_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM_IRQA_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PCM_A14_In(void)
{
    U8 u8RetGPIO;

    if(GPIO106)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A14_IN1) ;
        u8RetGPIO = (u8RetGPIO>>7);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A14_IN2) ;
        u8RetGPIO = (u8RetGPIO>>2);
    }
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM_A13_In(void)
{
    U8 u8RetGPIO;

    if(GPIO107)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A13_IN1) ;
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A13_IN2)  ;
        u8RetGPIO = (u8RetGPIO>>1);
    }
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM_A8_In(void)
{
    U8 u8RetGPIO;

    if(GPIO108)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A8_IN1) ;
        u8RetGPIO = (u8RetGPIO>>1);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A8_IN2) ;
        u8RetGPIO = (u8RetGPIO>>0);
    }
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_IOWR_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM_IOWR_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_A9_In(void)
{
    U8 u8RetGPIO;

    if(GPIO110)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A8_IN1) ;
        u8RetGPIO = (u8RetGPIO>>3);
    }
    else
    {

        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A8_IN2) ;
        u8RetGPIO = (u8RetGPIO>>7);
    }
    return (u8RetGPIO &= BIT0);
}




U8 MHal_GPIO_PCM_IORD_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM_IORD_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM_A11_In(void)
{
    U8 u8RetGPIO;

    if(GPIO112)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A11_IN2) ;
        u8RetGPIO = (u8RetGPIO>>5);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A11_IN2) ;
        u8RetGPIO = (u8RetGPIO>>6);
    }
    return (u8RetGPIO &= BIT0);
}




U8 MHal_GPIO_PCM_OE_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM_OE_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_A10_In(void)
{
    U8 u8RetGPIO;

    if(GPIO114)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A10_IN1) ;
        u8RetGPIO = (u8RetGPIO>>7);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A10_IN2) ;
        u8RetGPIO = (u8RetGPIO>>5);
    }
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_CE_N_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_PCM_CE_N_IN) ;
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_D7_In(void)
{
    U8 u8RetGPIO;

    if(GPIO116)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D7_IN1) ;
        u8RetGPIO = (u8RetGPIO>>1);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D7_IN2) ;
        u8RetGPIO = (u8RetGPIO>>4);
    }
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_D6_In(void)
{
    U8 u8RetGPIO;

    if(GPIO117)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D6_IN1) ;
        u8RetGPIO = (u8RetGPIO>>3);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D6_IN2) ;
        u8RetGPIO = (u8RetGPIO>>3);
    }
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM_D5_In(void)
{
    U8 u8RetGPIO;

    if(GPIO118)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D5_IN1) ;
        u8RetGPIO = (u8RetGPIO>>4);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D5_IN2) ;
        u8RetGPIO = (u8RetGPIO>>2);
    }
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_D4_In(void)
{
    U8 u8RetGPIO;
    if(GPIO119)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D4_IN1) ;
        u8RetGPIO = (u8RetGPIO>>5);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D4_IN2) ;
        u8RetGPIO = (u8RetGPIO>>1);
    }
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM_D3_In(void)
{
    U8 u8RetGPIO;

    if(GPIO120)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D3_IN1)  ;
        u8RetGPIO = (u8RetGPIO>>6);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D3_IN2) ;
    }

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_A3_In(void)
{
    U8 u8RetGPIO;
    if(GPIO121)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A3_IN1) ;
        u8RetGPIO = (u8RetGPIO>>7);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A3_IN2) ;
        u8RetGPIO = (u8RetGPIO>>6);
    }
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_A2_In(void)
{
    U8 u8RetGPIO;
    if(GPIO122)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A2_IN1) ;
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A3_IN2) ;
        u8RetGPIO = (u8RetGPIO>>7);
    }

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_REG_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM_REG_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PCM_A1_In(void)
{
    U8 u8RetGPIO;

    if(GPIO124)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A1_IN1) ;
        u8RetGPIO = (u8RetGPIO>>2);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A1_IN2) ;
    }

    return (u8RetGPIO &= BIT0);
}




U8 MHal_GPIO_PCM_A0_In(void)
{
    U8 u8RetGPIO;

    if(GPIO125)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A0_IN1) ;
        u8RetGPIO = (u8RetGPIO>>3);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A0_IN2) ;
        u8RetGPIO = (u8RetGPIO>>1);
    }
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_D0_In(void)
{
    U8 u8RetGPIO;


    if(GPIO126)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_A1_IN1) ;
        u8RetGPIO = (u8RetGPIO>>4);

        return (u8RetGPIO &= BIT0);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D0_IN2) ;
        u8RetGPIO = (u8RetGPIO>>2);

        return (u8RetGPIO &= BIT0);
    }

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_D1_In(void)
{
    U8 u8RetGPIO;
    if(GPIO127)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D1_IN1) ;
        u8RetGPIO = (u8RetGPIO>>5);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D1_IN2) ;
        u8RetGPIO = (u8RetGPIO>>3);
    }
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PCM_D2_In(void)
{
    U8 u8RetGPIO;

    if(GPIO128)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D2_IN1) ;
        u8RetGPIO = (u8RetGPIO>>6);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PCM_D2_IN2)  ;
        u8RetGPIO = (u8RetGPIO>>4);
    }
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM_RESET_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_PCM_RESET_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM_CD_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM_CD_N_IN) ;
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_PCM2_CE_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM2_CE_N_IN);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM2_IRQA_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM2_IRQA_N_IN);
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM2_WAIT_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM2_WAIT_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM2_RESET_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM2_RESET_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PCM2_CD_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM2_CD_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PF_AD15_In(void)
{
    U8 u8RetGPIO;

    if(GPIO136)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_AD15_IN1) ;
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_AD15_IN2) ;
    }
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PF_CE0Z_In(void)
{
    U8 u8RetGPIO;
    if(GPIO137)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_CE0Z_IN1) ;
        u8RetGPIO = (u8RetGPIO>>1);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_CE0Z_IN2) ;
        u8RetGPIO = (u8RetGPIO>>1);
    }
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PF_CE1Z_In(void)
{
    U8 u8RetGPIO;

    if(GPIO138)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_CE1Z_IN1) ;
        u8RetGPIO = (u8RetGPIO>>2);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_CE1Z_IN2) ;
        u8RetGPIO = (u8RetGPIO>>2);
    }
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PF_OEZ_In(void)
{
    U8 u8RetGPIO;

    if(GPIO139)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_OEZ_IN1)  ;
        u8RetGPIO = (u8RetGPIO>>3);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_OEZ_IN2)  ;
        u8RetGPIO = (u8RetGPIO>>3);
    }

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_PF_WEZ_In(void)
{
    U8 u8RetGPIO;

    if(GPIO140)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_WEZ_IN1) ;
        u8RetGPIO = (u8RetGPIO>>4);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_WEZ_IN2) ;
        u8RetGPIO = (u8RetGPIO>>4);
    }
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PF_ALE_In(void)
{
    U8 u8RetGPIO;

    if(GPIO141)
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_ALE_IN1) ;
        u8RetGPIO = (u8RetGPIO>>5);
    }
    else
    {
        u8RetGPIO = MHal_GPIO_REG(REG_PF_ALE_IN2) ;
        u8RetGPIO = (u8RetGPIO>>5);
    }
    return (u8RetGPIO &= BIT0);
}




U8 MHal_GPIO_F_RBZ_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_F_RBZ_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON0_IN) ;
    return (u8RetGPIO &= BIT0);
}


//balup_090907

U8 MHal_GPIO_GPIO48_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO48_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO49_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO48_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_GPIO53_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO53_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);		
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO52_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO53_IN) ;
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_GPIO47_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_GPIO48_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);	
    return (u8RetGPIO &= BIT0);
}







U8 MHal_GPIO_TCON1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON1_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON2_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON2_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_TCON3_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON3_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON4_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON4_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_TCON5_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON5_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON6_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON6_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_TCON7_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON7_IN)  ;
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON8_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON8_IN) ;
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON9_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON9_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON10_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON10_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_TCON11_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON11_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON12_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON12_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON13_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON13_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_TCON14_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON14_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON15_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON15_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON16_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON16_IN) ;
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON17_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON17_IN)  ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON18_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON18_IN)  ;
    u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON19_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON19_IN)  ;
    u8RetGPIO = (u8RetGPIO>>3);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_TCON20_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON20_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_TCON21_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_TCON21_IN)  ;
     u8RetGPIO = (u8RetGPIO>>5);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_ET_COL_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_ET_COL_IN)  ;

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_ET_TXD1_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_ET_TXD1_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_ET_TXD0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_ET_TXD0_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_ET_TX_EN_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_ET_TX_EN_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_ET_TX_CLK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_ET_TX_CLK_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_ET_RXD0_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_ET_RXD0_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_ET_RXD1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_ET_RXD1_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_ET_MDC_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_ET_MDC_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_ET_EDIO_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_ET_EDIO_IN) ;
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_I2S_IN_WS_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_I2S_IN_WS_IN) ;

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_I2S_IN_BCK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_I2S_IN_BCK_IN)  ;
    u8RetGPIO = (u8RetGPIO>>1);
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_I2S_IN_SD_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_I2S_IN_SD_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_SPDIF_IN_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_SPDIF_IN_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_SPDIF_OUT_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_SPDIF_OUT_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_I2S_OUT_MCK_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_I2S_OUT_MCK_IN);
    u8RetGPIO = (u8RetGPIO>>5);

return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_I2S_OUT_WS_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_I2S_OUT_WS_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_I2S_OUT_BCK_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_I2S_OUT_BCK_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_I2S_OUT_SD_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_I2S_OUT_SD_IN) ;
    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_I2S_OUT_SD1_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_I2S_OUT_SD1_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_I2S_OUT_SD2_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_I2S_OUT_SD2_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_I2S_OUT_SD3_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_I2S_OUT_SD3_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_B_ODD_0_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_B_ODD_0_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_B_ODD_1_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_B_ODD_1_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_B_ODD_2_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_B_ODD_2_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_B_ODD_3_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_B_ODD_3_IN) ;

    return (u8RetGPIO &= BIT0);
}





U8 MHal_GPIO_B_ODD_4_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_B_ODD_4_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_B_ODD_5_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_B_ODD_5_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_B_ODD_6_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_B_ODD_6_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_B_ODD_7_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_B_ODD_7_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);
    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_G_ODD_0_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_G_ODD_0_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_G_ODD_1_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_G_ODD_1_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_G_ODD_2_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_G_ODD_2_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_G_ODD_3_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_G_ODD_3_IN) ;

    return (u8RetGPIO &= BIT0);
}





U8 MHal_GPIO_G_ODD_4_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_G_ODD_4_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_G_ODD_5_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_G_ODD_5_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_G_ODD_6_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_G_ODD_6_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_G_ODD_7_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_G_ODD_7_IN);
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_R_ODD_0_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_R_ODD_0_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_R_ODD_1_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_R_ODD_1_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_R_ODD_2_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_R_ODD_2_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_R_ODD_3_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_R_ODD_3_IN) ;

return (u8RetGPIO &= BIT0);
}





U8 MHal_GPIO_R_ODD_4_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_R_ODD_4_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_R_ODD_5_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_R_ODD_5_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_R_ODD_6_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_R_ODD_6_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_R_ODD_7_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_R_ODD_7_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_mini_LVDS_0_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_mini_LVDS_0_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_mini_LVDS_1_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_mini_LVDS_1_IN) ;
     u8RetGPIO = (u8RetGPIO>>2);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_mini_LVDS_2_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_mini_LVDS_2_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_mini_LVDS_3_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_mini_LVDS_3_IN);

    return (u8RetGPIO &= BIT0);
}





U8 MHal_GPIO_mini_LVDS_4_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_mini_LVDS_4_IN) ;
    u8RetGPIO = (u8RetGPIO>>7);

    return (u8RetGPIO &= BIT0);
}



U8 MHal_GPIO_mini_LVDS_5_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_mini_LVDS_5_IN) ;
    u8RetGPIO = (u8RetGPIO>>6);

return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_mini_LVDS_6_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_mini_LVDS_6_IN) ;
    u8RetGPIO = (u8RetGPIO>>5);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_mini_LVDS_7_In(void)
{
    U8 u8RetGPIO;

    u8RetGPIO = MHal_GPIO_REG(REG_mini_LVDS_7_IN) ;
    u8RetGPIO = (u8RetGPIO>>4);

    return (u8RetGPIO &= BIT0);
}

U8 MHal_GPIO_LCK_In(void)
{
    U8 u8RetGPIO;
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    u8RetGPIO = MHal_GPIO_REG(REG_LCK_IN) ;
    u8RetGPIO = (u8RetGPIO>>3);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_LDE_In(void)
{
    U8 u8RetGPIO;
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    u8RetGPIO = MHal_GPIO_REG(REG_LDE_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);
    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_LHSYNC_In(void)
{
    U8 u8RetGPIO;
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    u8RetGPIO = MHal_GPIO_REG(REG_LHSYNC_IN) ;
    u8RetGPIO = (u8RetGPIO>>1);

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_LVSYNC_In(void)
{
    U8 u8RetGPIO;
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    u8RetGPIO = MHal_GPIO_REG(REG_LVSYNC_IN) ;

    return (u8RetGPIO &= BIT0);
}


U8 MHal_GPIO_PCM_WE_N_In(void)
{
    U8 u8RetGPIO;
    u8RetGPIO = MHal_GPIO_REG(REG_PCM_WE_N_IN) ;
    u8RetGPIO = (u8RetGPIO>>2);
return (u8RetGPIO &= BIT0);
}


//unctions of this section set to output high

void MHal_GPIO_PM_SPI_CZ_High(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CZ_OUT) |= BIT4;
}

void MHal_GPIO_PM_SPI_CK_High(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CK_OUT) |= BIT5;
}
void MHal_GPIO_PM_SPI_DI_High(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DI_OUT) |= BIT6;
}

void MHal_GPIO_PM_SPI_DO_High(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DO_OUT) |= BIT7;
}

void MHal_GPIO_IRIN_High(void)
{
    MHal_GPIO_REG(REG_IRIN_OUT) |= BIT0;
}

void MHal_GPIO_CEC_High(void)
{
    MHal_GPIO_REG(REG_CEC_OUT) |= BIT2;

}

void MHal_GPIO_GPIO_PM0_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM0_OUT) |= BIT0;

}

void MHal_GPIO_GPIO_PM1_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM1_OUT) |= BIT1;

}

void MHal_GPIO_GPIO_PM2_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM2_OUT) |= BIT2;

}

void MHal_GPIO_GPIO_PM3_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM3_OUT) |= BIT3;

}

void MHal_GPIO_GPIO_PM4_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM4_OUT) |= BIT4;
}

void MHal_GPIO_GPIO_PM5_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM5_OUT) |= BIT5;

}

void MHal_GPIO_GPIO_PM6_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM6_OUT) |= BIT6;

}

void MHal_GPIO_GPIO_PM7_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM7_OUT) |= BIT7;

}

void MHal_GPIO_GPIO_PM8_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM8_OUT) |= BIT0;

}

void MHal_GPIO_GPIO_PM9_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM9_OUT) |= BIT1;

}

void MHal_GPIO_GPIO_PM10_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM10_OUT) |= BIT2;

}

void MHal_GPIO_GPIO_PM11_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM11_OUT) |= BIT3;

}

void MHal_GPIO_GPIO_PM12_High(void)
{
    MHal_GPIO_REG(REG_GPIO_PM12_OUT) |= BIT4;

}

void MHal_GPIO_HOTPLUGA_High(void)
{
    MHal_GPIO_REG(REG_HOTPLUGA_OUT) |= BIT4;

}


void MHal_GPIO_HOTPLUGB_High(void)
{
    MHal_GPIO_REG(REG_HOTPLUGB_OUT) |= BIT5;

}

void MHal_GPIO_HOTPLUGC_High(void)
{
    MHal_GPIO_REG(REG_HOTPLUGC_OUT) |= BIT6;

}

void MHal_GPIO_HOTPLUGD_High(void)
{
    MHal_GPIO_REG(REG_HOTPLUGD_OUT) |= BIT7;

}


void MHal_GPIO_DDCDA_CK_High(void)
{
    MHal_GPIO_REG(REG_DDCDA_CK_OUT) |= BIT2;

}

void MHal_GPIO_DDCDA_DA_High(void)
{
    MHal_GPIO_REG(REG_DDCDA_DA_OUT) |= BIT6;

}

void MHal_GPIO_DDCDB_CK_High(void)
{
    MHal_GPIO_REG(REG_DDCDB_DA_OUT) |= BIT2;

}

void MHal_GPIO_DDCDB_DA_High(void)
{
    MHal_GPIO_REG(REG_DDCDB_CK_OUT) |= BIT6;

}

void MHal_GPIO_DDCDC_CK_High(void)
{
    MHal_GPIO_REG(REG_DDCDC_CK_OUT) |= BIT2;
}

void MHal_GPIO_DDCDC_DA_High(void)
{
    MHal_GPIO_REG(REG_DDCDC_DA_OUT) |= BIT6;
}

void MHal_GPIO_DDCDD_CK_High(void)
{
    MHal_GPIO_REG(REG_DDCDD_CK_OUT) |= BIT2;
}

void MHal_GPIO_DDCDD_DA_High(void)
{
    MHal_GPIO_REG(REG_DDCDD_DA_OUT) |= BIT6;

}

void MHal_GPIO_SAR0_High(void)
{
    MHal_GPIO_REG(REG_SAR0_OUT) |= BIT0;

}

void MHal_GPIO_SAR1_High(void)
{
    MHal_GPIO_REG(REG_SAR1_OUT) |= BIT1;

}

void MHal_GPIO_SAR2_High(void)
{
    MHal_GPIO_REG(REG_SAR2_OUT) |= BIT2;
}

void MHal_GPIO_SAR3_High(void)
{
    MHal_GPIO_REG(REG_SAR3_OUT) |= BIT3;
}

void MHal_GPIO_SAR4_High(void)
{
    MHal_GPIO_REG(REG_SAR4_OUT) |= BIT4;
}

void MHal_GPIO_GPIO0_High(void)
{
    MHal_GPIO_REG(REG_GPIO0_OUT) |= BIT0;

}

void MHal_GPIO_GPIO1_High(void)
{
    MHal_GPIO_REG(REG_GPIO1_OUT) |= BIT1;

}

void MHal_GPIO_GPIO2_High(void)
{
    MHal_GPIO_REG(REG_GPIO2_OUT) |= BIT2;

}

void MHal_GPIO_GPIO3_High(void)
{
    MHal_GPIO_REG(REG_GPIO3_OUT) |= BIT3;

}

void MHal_GPIO_GPIO4_High(void)
{
    MHal_GPIO_REG(REG_GPIO4_OUT) |= BIT4;
}

void MHal_GPIO_GPIO5_High(void)
{
    MHal_GPIO_REG(REG_GPIO5_OUT) |= BIT5;
}

void MHal_GPIO_GPIO6_High(void)
{

    MHal_GPIO_REG(REG_GPIO6_OUT) |= BIT6;

}

void MHal_GPIO_GPIO7_High(void)
{
    MHal_GPIO_REG(REG_GPIO7_OUT) |= BIT7;

}

void MHal_GPIO_GPIO8_High(void)
{
    MHal_GPIO_REG(REG_GPIO8_OUT) |= BIT0;
}

void MHal_GPIO_GPIO9_High(void)
{
    MHal_GPIO_REG(REG_GPIO9_OUT) |= BIT1;
}

void MHal_GPIO_GPIO10_High(void)
{
    MHal_GPIO_REG(REG_GPIO10_OUT) |= BIT2;

}

void MHal_GPIO_GPIO11_High(void)
{
    MHal_GPIO_REG(REG_GPIO11_OUT) |= BIT3;

}

void MHal_GPIO_GPIO12_High(void)
{
    MHal_GPIO_REG(REG_GPIO12_OUT) |= BIT4;

}
void MHal_GPIO_GPIO13_High(void)
{
    MHal_GPIO_REG(REG_GPIO13_OUT) |= BIT5;

}
void MHal_GPIO_GPIO14_High(void)
{
    MHal_GPIO_REG(REG_GPIO14_OUT) |= BIT6;
}
void MHal_GPIO_GPIO15_High(void)
{
    MHal_GPIO_REG(REG_GPIO15_OUT) |= BIT7;

}
void MHal_GPIO_GPIO16_High(void)
{
    MHal_GPIO_REG(REG_GPIO16_OUT) |= BIT0;

}
void MHal_GPIO_GPIO17_High(void)
{
    MHal_GPIO_REG(REG_GPIO17_OUT) |= BIT1;

}
void MHal_GPIO_GPIO18_High(void)
{
    MHal_GPIO_REG(REG_GPIO18_OUT) |= BIT2;

}

void MHal_GPIO_GPIO19_High(void)
{
    MHal_GPIO_REG(REG_GPIO19_OUT) |= BIT3;

}


void MHal_GPIO_GPIO20_High(void)
{
    MHal_GPIO_REG(REG_GPIO20_OUT) |= BIT4;

}


void MHal_GPIO_GPIO21_High(void)
{
    MHal_GPIO_REG(REG_GPIO21_OUT) |= BIT5;

}


void MHal_GPIO_GPIO22_High(void)
{
    MHal_GPIO_REG(REG_GPIO22_OUT) |= BIT6;
}

void MHal_GPIO_GPIO23_High(void)
{
    MHal_GPIO_REG(REG_GPIO23_OUT) |= BIT7;
}


void MHal_GPIO_GPIO24_High(void)
{
    MHal_GPIO_REG(REG_GPIO24_OUT) |= BIT0;

}
void MHal_GPIO_GPIO25_High(void)
{
    MHal_GPIO_REG(REG_GPIO25_OUT) |= BIT1;

}

void MHal_GPIO_GPIO26_High(void)
{
    MHal_GPIO_REG(REG_GPIO26_OUT) |= BIT2;

}
void MHal_GPIO_GPIO27_High(void)
{
    MHal_GPIO_REG(REG_GPIO27_OUT) |= BIT3;

}

void MHal_GPIO_UART_RX2_High(void)
{
    MHal_GPIO_REG(REG_UART_RX2_OUT) |= BIT4;

}
void MHal_GPIO_UART_TX2_High(void)
{
    MHal_GPIO_REG(REG_UART_TX2_OUT) |= BIT5;

}

void MHal_GPIO_PWM0_High(void)
{
    MHal_GPIO_REG(REG_PWM0_OUT) |= BIT6;
}
void MHal_GPIO_PWM1_High(void)
{
    MHal_GPIO_REG(REG_PWM1_OUT) |= BIT7;

}
void MHal_GPIO_PWM2_High(void)
{
    MHal_GPIO_REG(REG_PWM2_OUT) |= BIT0;
}


void MHal_GPIO_PWM3_High(void)
{
    MHal_GPIO_REG(REG_PWM3_OUT) |= BIT1;
}


void MHal_GPIO_PWM4_High(void)
{
    MHal_GPIO_REG(REG_PWM4_OUT) |= BIT2;
}

void MHal_GPIO_DDCR_DA_High(void)
{
    MHal_GPIO_REG(REG_DDCR_DA_OUT) |= BIT3;

}

void MHal_GPIO_DDCR_CK_High(void)
{
    MHal_GPIO_REG(REG_DDCR_CK_OUT) |= BIT4;

}


void MHal_GPIO_TGPIO0_High(void)
{
    MHal_GPIO_REG(REG_TGPIO0_OUT) |= BIT5;

}


void MHal_GPIO_TGPIO1_High(void)
{
    MHal_GPIO_REG(REG_TGPIO1_OUT) |= BIT6;

}


void MHal_GPIO_TGPIO2_High(void)
{
    MHal_GPIO_REG(REG_TGPIO2_OUT)|= BIT7;
}


void MHal_GPIO_TGPIO3_High(void)
{
    MHal_GPIO_REG(REG_TGPIO3_OUT) |= BIT0;
}


void MHal_GPIO_TS0_D0_High(void)
{
    MHal_GPIO_REG(REG_TS0_D0_OUT) |= BIT0;

}


void MHal_GPIO_TS0_D1_High(void)
{
    MHal_GPIO_REG(REG_TS0_D1_OUT) |= BIT1;

}


void MHal_GPIO_TS0_D2_High(void)
{
    MHal_GPIO_REG(REG_TS0_D2_OUT) |= BIT2;

}


void MHal_GPIO_TS0_D3_High(void)
{
    MHal_GPIO_REG(REG_TS0_D3_OUT) |= BIT3;

}
void MHal_GPIO_TS0_D4_High(void)
{
    MHal_GPIO_REG(REG_TS0_D4_OUT) |= BIT4;

}


void MHal_GPIO_TS0_D5_High(void)
{
    MHal_GPIO_REG(REG_TS0_D5_OUT) |= BIT5;

}


void MHal_GPIO_TS0_D6_High(void)
{
    MHal_GPIO_REG(REG_TS0_D6_OUT) |= BIT6;
}



void MHal_GPIO_TS0_D7_High(void)
{
    MHal_GPIO_REG(REG_TS0_D7_OUT) |= BIT7;

}


void MHal_GPIO_TS0_VLD_High(void)
{
    MHal_GPIO_REG(REG_TS0_VLD_OUT) |= BIT0;
}


void MHal_GPIO_TS0_SYNC_High(void)
{
    MHal_GPIO_REG(REG_TS0_SYNC_OUT) |= BIT1;
}



void MHal_GPIO_TS0_CLK_High(void)
{
    MHal_GPIO_REG(REG_TS0_CLK_OUT) |= BIT2;
}

void MHal_GPIO_TS1_D0_High(void)
{
    MHal_GPIO_REG(REG_TS1_D0_OUT) |= (BIT0);

}
void MHal_GPIO_TS1_D1_High(void)
{
    MHal_GPIO_REG(REG_TS1_D1_OUT) |= (BIT1);

}


void MHal_GPIO_TS1_D2_High(void)
{
    MHal_GPIO_REG(REG_TS1_D2_OUT) |= (BIT2);

}

void MHal_GPIO_TS1_D3_High(void)
{
    MHal_GPIO_REG(REG_TS1_D3_OUT) |= (BIT3);

}

void MHal_GPIO_TS1_D4_High(void)
{
    MHal_GPIO_REG(REG_TS1_D4_OUT) |= (BIT4);

}


void MHal_GPIO_TS1_D5_High(void)
{
    MHal_GPIO_REG(REG_TS1_D5_OUT) |= (BIT5);

}


void MHal_GPIO_TS1_D6_High(void)
{
    MHal_GPIO_REG(REG_TS1_D6_OUT) |= (BIT6);

}


void MHal_GPIO_TS1_D7_High(void)
{
    MHal_GPIO_REG(REG_TS1_D7_OUT) |= (BIT7);

}


void MHal_GPIO_TS1_VLD_High(void)
{
    MHal_GPIO_REG(REG_TS1_VLD_OUT) |= (BIT0);

}


void MHal_GPIO_TS1_SYNC_High(void)
{
    MHal_GPIO_REG(REG_TS1_SYNC_OUT) |= (BIT1);
}


void MHal_GPIO_TS1_CLK_High(void)
{
    MHal_GPIO_REG(REG_TS1_CLK_OUT) |= (BIT2);

}


void MHal_GPIO_PCM_A4_High(void)
{

    if(GPIO99)
    {
        //PCM_GPIO0
        MHal_GPIO_REG(REG_PCM_A4_OUT1) |= (BIT0);

    }
    else
    {
        //NOR_GPIO15
        MHal_GPIO_REG(REG_PCM_A4_OUT2) |= (BIT7);



    }

}



void MHal_GPIO_PCM_WAIT_N_High(void)
{
    MHal_GPIO_REG(REG_PCM_WAIT_N_OUT) |= (BIT1);

}
void MHal_GPIO_PCM_A5_High(void)
{

    if(GPIO101)
    {
        //PCM_GPIO2
        MHal_GPIO_REG(REG_PCM_A5_OUT1) |= (BIT2);

    }
    else
    {
        //NOR_GPIO14
        MHal_GPIO_REG(REG_PCM_A5_OUT2) |= (BIT6);

    }

}


void MHal_GPIO_PCM_A6_High(void)
{

    if(GPIO102)
    {
        //PCM_GPIO3
        MHal_GPIO_REG(REG_PCM_A6_OUT1) |= (BIT3);

    }
    else
    {
        //NOR_GPIO13
        MHal_GPIO_REG(REG_PCM_A6_OUT2) |= (BIT5);

    }

}



void MHal_GPIO_PCM_A7_High(void)
{

    if(GPIO103)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A7_OUT1)  |= (BIT4);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A7_OUT2) |= (BIT4);

    }

}


void MHal_GPIO_PCM_A12_High(void)
{

    if(GPIO104)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A12_OUT1) |= (BIT5);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A12_OUT2) |= (BIT3);

    }

}



void MHal_GPIO_PCM_IRQA_N_High(void)
{
    MHal_GPIO_REG(REG_PCM_IRQA_N_OUT) |= (BIT6);

}

void MHal_GPIO_PCM_A14_High(void)
{

    if(GPIO106)
    {
        MHal_GPIO_REG(REG_PCM_A14_OUT1) |= (BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A14_OUT2) |= (BIT2);

    }

}


void MHal_GPIO_PCM_A13_High(void)
{

    if(GPIO107)
    {
        MHal_GPIO_REG(REG_PCM_A13_OUT1) |= (BIT0);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A13_OUT2)  |= (BIT1);

    }

}

void MHal_GPIO_PCM_A8_High(void)
{

    if(GPIO108)
    {
        MHal_GPIO_REG(REG_PCM_A8_OUT1) |= (BIT1);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_OUT2) |= (BIT0);

    }

}



void MHal_GPIO_PCM_IOWR_N_High(void)
{
    MHal_GPIO_REG(REG_PCM_IOWR_N_OUT) |= (BIT2);

}


void MHal_GPIO_PCM_A9_High(void)
{

    if(GPIO110)
    {
        MHal_GPIO_REG(REG_PCM_A8_OUT1) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_OUT2) |= (BIT7);

    }

}



void MHal_GPIO_PCM_IORD_N_High(void)
{
    MHal_GPIO_REG(REG_PCM_IORD_N_OUT) |= (BIT4);

}



void MHal_GPIO_PCM_A11_High(void)
{

    if(GPIO112)
    {
        MHal_GPIO_REG(REG_PCM_A11_OUT2) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A11_OUT2) |= (BIT6);

    }

}


void MHal_GPIO_PCM_OE_N_High(void)
{
    MHal_GPIO_REG(REG_PCM_OE_N_OUT) |= (BIT6);

}


void MHal_GPIO_PCM_A10_High(void)
{

    if(GPIO114)
    {
        MHal_GPIO_REG(REG_PCM_A10_OUT1) |= (BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A10_OUT2) |= (BIT5);
    }


}
void MHal_GPIO_PCM_CE_N_High(void)
{

    MHal_GPIO_REG(REG_PCM_CE_N_OUT) |= (BIT0);

}


void MHal_GPIO_PCM_D7_High(void)
{

    if(GPIO116)
    {
        MHal_GPIO_REG(REG_PCM_D7_OUT1) |= (BIT1);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D7_OUT2) |= (BIT4);
    }

}


void MHal_GPIO_PCM_D6_High(void)
{

    if(GPIO117)
    {
        MHal_GPIO_REG(REG_PCM_D6_OUT1) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D6_OUT2) |= (BIT3);
    }

}



void MHal_GPIO_PCM_D5_High(void)
{

    if(GPIO118)
    {
        MHal_GPIO_REG(REG_PCM_D5_OUT1) |= (BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D5_OUT2) |= (BIT2);

    }

}


void MHal_GPIO_PCM_D4_High(void)
{

    if(GPIO119)
    {
        MHal_GPIO_REG(REG_PCM_D4_OUT1) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D4_OUT2) |= (BIT1);
    }

}


void MHal_GPIO_PCM_D3_High(void)
{

    if(GPIO120)
    {
        MHal_GPIO_REG(REG_PCM_D3_OUT1)  |= (BIT6);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D3_OUT2) |= (BIT0);
    }

}


void MHal_GPIO_PCM_A3_High(void)
{
    if(GPIO121)
    {
        MHal_GPIO_REG(REG_PCM_A3_OUT1) |= (BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_OUT2) |= (BIT6);

    }

}



void MHal_GPIO_PCM_A2_High(void)
{


    if(GPIO122)
    {
        MHal_GPIO_REG(REG_PCM_A2_OUT1) |= (BIT0);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_OUT2) |= (BIT7);
    }

}


void MHal_GPIO_PCM_REG_N_High(void)
{
    MHal_GPIO_REG(REG_PCM_REG_N_OUT) |= (BIT1);
}

void MHal_GPIO_PCM_A1_High(void)
{

    if(GPIO124)
    {
        MHal_GPIO_REG(REG_PCM_A1_OUT1) |= (BIT2);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A1_OUT2) |= (BIT0);

    }

}




void MHal_GPIO_PCM_A0_High(void)
{

    if(GPIO125)
    {
        MHal_GPIO_REG(REG_PCM_A0_OUT1) |= (BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A0_OUT2) |= (BIT1);

    }

}



void MHal_GPIO_PCM_D0_High(void)
{


    if(GPIO126)
    {
        MHal_GPIO_REG(REG_PCM_A1_OUT1) |= (BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D0_OUT2) |= (BIT2);

    }

}


void MHal_GPIO_PCM_D1_High(void)
{


    if(GPIO127)
    {
        MHal_GPIO_REG(REG_PCM_D1_OUT1) |= (BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D1_OUT2) |= (BIT3);

    }

}


void MHal_GPIO_PCM_D2_High(void)
{


    if(GPIO128)
    {
        MHal_GPIO_REG(REG_PCM_D2_OUT1) |= (BIT6);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D2_OUT2)  |= (BIT4);

    }

}



void MHal_GPIO_PCM_RESET_High(void)
{

    MHal_GPIO_REG(REG_PCM_RESET_OUT) |= (BIT7);

}



void MHal_GPIO_PCM_CD_N_High(void)
{

    MHal_GPIO_REG(REG_PCM_CD_N_OUT) |= (BIT0);

}
void MHal_GPIO_PCM2_CE_N_High(void)
{

    MHal_GPIO_REG(REG_PCM2_CE_N_OUT)|= (BIT0);

}



void MHal_GPIO_PCM2_IRQA_N_High(void)
{

    MHal_GPIO_REG(REG_PCM2_IRQA_N_OUT)|= (BIT1);

}



void MHal_GPIO_PCM2_WAIT_N_High(void)
{

    MHal_GPIO_REG(REG_PCM2_WAIT_N_OUT) |= (BIT2);

}



void MHal_GPIO_PCM2_RESET_High(void)
{

    MHal_GPIO_REG(REG_PCM2_RESET_OUT) |= (BIT3);

}



void MHal_GPIO_PCM2_CD_N_High(void)
{

    MHal_GPIO_REG(REG_PCM2_CD_N_OUT) |= (BIT4);

}



void MHal_GPIO_PF_AD15_High(void)
{

    if(GPIO136)
    {
        MHal_GPIO_REG(REG_PF_AD15_OUT1) |= (BIT0);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_AD15_OUT2) |= (BIT0);

    }

}



void MHal_GPIO_PF_CE0Z_High(void)
{

    if(GPIO137)
    {
        MHal_GPIO_REG(REG_PF_CE0Z_OUT1) |= (BIT1);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE0Z_OUT2) |= (BIT1);

    }

}


void MHal_GPIO_PF_CE1Z_High(void)
{

    if(GPIO138)
    {
        MHal_GPIO_REG(REG_PF_CE1Z_OUT1) |= (BIT2);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE1Z_OUT2) |= (BIT2);
    }

}


void MHal_GPIO_PF_OEZ_High(void)
{

    if(GPIO139)
    {
        MHal_GPIO_REG(REG_PF_OEZ_OUT1)  |= (BIT3);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_OEZ_OUT2)  |= (BIT3);
    }

}



void MHal_GPIO_PF_WEZ_High(void)
{

    if(GPIO140)
    {
        MHal_GPIO_REG(REG_PF_WEZ_OUT1) |= (BIT4);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_WEZ_OUT2) |= (BIT4);

    }

}



void MHal_GPIO_PF_ALE_High(void)
{

    if(GPIO141)
    {
        MHal_GPIO_REG(REG_PF_ALE_OUT1) |= (BIT5);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_ALE_OUT2) |= (BIT5);

    }

}



void MHal_GPIO_F_RBZ_High(void)
{
    MHal_GPIO_REG(REG_F_RBZ_OUT) |= (BIT6);
}

//balup_090907

void MHal_GPIO_GPIO48_High(void)
{
    MHal_GPIO_REG(REG_GPIO48_OUT) |= (BIT4);
}

void MHal_GPIO_GPIO49_High(void)
{
    MHal_GPIO_REG(REG_GPIO48_OUT) |= (BIT5);
}

void MHal_GPIO_GPIO53_High(void)
{
    MHal_GPIO_REG(REG_GPIO53_OUT) |= (BIT1);
}

void MHal_GPIO_GPIO52_High(void)
{
    MHal_GPIO_REG(REG_GPIO53_OUT) |= (BIT0);
}

void MHal_GPIO_GPIO47_High(void)
{
    MHal_GPIO_REG(REG_GPIO48_OUT) |= (BIT3);
}





void MHal_GPIO_TCON0_High(void)
{
    MHal_GPIO_REG(REG_TCON0_OUT) |= (BIT0);
}



void MHal_GPIO_TCON1_High(void)
{
    MHal_GPIO_REG(REG_TCON1_OUT) |= (BIT1);
}


void MHal_GPIO_TCON2_High(void)
{
    MHal_GPIO_REG(REG_TCON2_OUT) |= (BIT2);
}



void MHal_GPIO_TCON3_High(void)
{
    MHal_GPIO_REG(REG_TCON3_OUT) |= (BIT3);
}


void MHal_GPIO_TCON4_High(void)
{
    MHal_GPIO_REG(REG_TCON4_OUT) |= (BIT4);
}



void MHal_GPIO_TCON5_High(void)
{
    MHal_GPIO_REG(REG_TCON5_OUT) |= (BIT5);
}


void MHal_GPIO_TCON6_High(void)
{
    MHal_GPIO_REG(REG_TCON6_OUT) |= (BIT6);
}

void MHal_GPIO_TCON7_High(void)
{
    MHal_GPIO_REG(REG_TCON7_OUT)  |= (BIT7);
}


void MHal_GPIO_TCON8_High(void)
{
    MHal_GPIO_REG(REG_TCON8_OUT) |= (BIT0);
}


void MHal_GPIO_TCON9_High(void)
{
    MHal_GPIO_REG(REG_TCON9_OUT) |= (BIT1);
}


void MHal_GPIO_TCON10_High(void)
{
    MHal_GPIO_REG(REG_TCON10_OUT) |= (BIT2);
}



void MHal_GPIO_TCON11_High(void)
{
    MHal_GPIO_REG(REG_TCON11_OUT) |= (BIT3);
}


void MHal_GPIO_TCON12_High(void)
{
    MHal_GPIO_REG(REG_TCON12_OUT) |= (BIT4);
}


void MHal_GPIO_TCON13_High(void)
{
    MHal_GPIO_REG(REG_TCON13_OUT) |= (BIT5);
}



void MHal_GPIO_TCON14_High(void)
{
    MHal_GPIO_REG(REG_TCON14_OUT) |= (BIT6);
}


void MHal_GPIO_TCON15_High(void)
{
    MHal_GPIO_REG(REG_TCON15_OUT) |= (BIT7);
}


void MHal_GPIO_TCON16_High(void)
{
    MHal_GPIO_REG(REG_TCON16_OUT) |= (BIT0);
}


void MHal_GPIO_TCON17_High(void)
{
    MHal_GPIO_REG(REG_TCON17_OUT)  |= (BIT1);
}


void MHal_GPIO_TCON18_High(void)
{
    MHal_GPIO_REG(REG_TCON18_OUT)  |= (BIT2);
}


void MHal_GPIO_TCON19_High(void)
{
    MHal_GPIO_REG(REG_TCON19_OUT)  |= (BIT3);
}



void MHal_GPIO_TCON20_High(void)
{
    MHal_GPIO_REG(REG_TCON20_OUT) |= (BIT4);
}


void MHal_GPIO_TCON21_High(void)
{
    MHal_GPIO_REG(REG_TCON21_OUT)  |= (BIT5);
}



void MHal_GPIO_ET_COL_High(void)
{

    MHal_GPIO_REG(REG_ET_COL_OUT)  |= (BIT0);

}


void MHal_GPIO_ET_TXD1_High(void)
{

    MHal_GPIO_REG(REG_ET_TXD1_OUT) |= (BIT1);

}


void MHal_GPIO_ET_TXD0_High(void)
{

    MHal_GPIO_REG(REG_ET_TXD0_OUT) |= (BIT2);


}



void MHal_GPIO_ET_TX_EN_High(void)
{

    MHal_GPIO_REG(REG_ET_TX_EN_OUT) |= (BIT3);

}


void MHal_GPIO_ET_TX_CLK_High(void)
{
    MHal_GPIO_REG(REG_ET_TX_CLK_OUT) |= (BIT4);

}


void MHal_GPIO_ET_RXD0_High(void)
{
    MHal_GPIO_REG(REG_ET_RXD0_OUT) |= (BIT5);
}

void MHal_GPIO_ET_RXD1_High(void)
{
    MHal_GPIO_REG(REG_ET_RXD1_OUT) |= (BIT6);
}

void MHal_GPIO_ET_MDC_High(void)
{
    MHal_GPIO_REG(REG_ET_MDC_OUT) |= (BIT7);
}


void MHal_GPIO_ET_EDIO_High(void)
{
    MHal_GPIO_REG(REG_ET_EDIO_OUT) |= (BIT0);
}


void MHal_GPIO_I2S_IN_WS_High(void)
{

    MHal_GPIO_REG(REG_I2S_IN_WS_OUT) |= (BIT0);

}


void MHal_GPIO_I2S_IN_BCK_High(void)
{
    MHal_GPIO_REG(REG_I2S_IN_BCK_OUT)  |= (BIT1);

}



void MHal_GPIO_I2S_IN_SD_High(void)
{

    MHal_GPIO_REG(REG_I2S_IN_SD_OUT) |= (BIT2);

}


void MHal_GPIO_SPDIF_IN_High(void)
{
    MHal_GPIO_REG(REG_SPDIF_IN_OUT) |= (BIT3);
}


void MHal_GPIO_SPDIF_OUT_High(void)
{

    MHal_GPIO_REG(REG_SPDIF_OUT_OUT) |= (BIT4);

}


void MHal_GPIO_I2S_OUT_MCK_High(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_MCK_OUT)|= (BIT5);

}


void MHal_GPIO_I2S_OUT_WS_High(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_WS_OUT) |= (BIT6);

}


void MHal_GPIO_I2S_OUT_BCK_High(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_BCK_OUT) |= (BIT7);

}


void MHal_GPIO_I2S_OUT_SD_High(void)
{
    MHal_GPIO_REG(REG_I2S_OUT_SD_OUT) |= (BIT0);
}



void MHal_GPIO_I2S_OUT_SD1_High(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD1_OUT) |= (BIT1);

}



void MHal_GPIO_I2S_OUT_SD2_High(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD2_OUT) |= (BIT2);

}



void MHal_GPIO_I2S_OUT_SD3_High(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD3_OUT) |= (BIT3);

}


void MHal_GPIO_B_ODD_0_High(void)
{

    MHal_GPIO_REG(REG_B_ODD_0_OUT) |= (BIT3);

}



void MHal_GPIO_B_ODD_1_High(void)
{

    MHal_GPIO_REG(REG_B_ODD_1_OUT) |= (BIT2);

}



void MHal_GPIO_B_ODD_2_High(void)
{

    MHal_GPIO_REG(REG_B_ODD_2_OUT) |= (BIT1);

}


void MHal_GPIO_B_ODD_3_High(void)
{

    MHal_GPIO_REG(REG_B_ODD_3_OUT) |= (BIT0);

}





void MHal_GPIO_B_ODD_4_High(void)
{

    MHal_GPIO_REG(REG_B_ODD_4_OUT) |= (BIT7);

}



void MHal_GPIO_B_ODD_5_High(void)
{

    MHal_GPIO_REG(REG_B_ODD_5_OUT) |= (BIT6);

}

void MHal_GPIO_B_ODD_6_High(void)
{

    MHal_GPIO_REG(REG_B_ODD_6_OUT) |= (BIT5);

}

void MHal_GPIO_B_ODD_7_High(void)
{

    MHal_GPIO_REG(REG_B_ODD_7_OUT) |= (BIT4);
}

void MHal_GPIO_G_ODD_0_High(void)
{

    MHal_GPIO_REG(REG_G_ODD_0_OUT) |= (BIT3);

}



void MHal_GPIO_G_ODD_1_High(void)
{

    MHal_GPIO_REG(REG_G_ODD_1_OUT) |= (BIT2);

}



void MHal_GPIO_G_ODD_2_High(void)
{

    MHal_GPIO_REG(REG_G_ODD_2_OUT) |= (BIT1);

}


void MHal_GPIO_G_ODD_3_High(void)
{

    MHal_GPIO_REG(REG_G_ODD_3_OUT) |= (BIT0);

}





void MHal_GPIO_G_ODD_4_High(void)
{

    MHal_GPIO_REG(REG_G_ODD_4_OUT) |= (BIT7);

}



void MHal_GPIO_G_ODD_5_High(void)
{

    MHal_GPIO_REG(REG_G_ODD_5_OUT) |= (BIT6);

}

void MHal_GPIO_G_ODD_6_High(void)
{

    MHal_GPIO_REG(REG_G_ODD_6_OUT) |= (BIT5);

}

void MHal_GPIO_G_ODD_7_High(void)
{

    MHal_GPIO_REG(REG_G_ODD_7_OUT)|= (BIT4);

}

void MHal_GPIO_R_ODD_0_High(void)
{

    MHal_GPIO_REG(REG_R_ODD_0_OUT) |= (BIT3);

}



void MHal_GPIO_R_ODD_1_High(void)
{

    MHal_GPIO_REG(REG_R_ODD_1_OUT) |= (BIT2);

}



void MHal_GPIO_R_ODD_2_High(void)
{

    MHal_GPIO_REG(REG_R_ODD_2_OUT) |= (BIT1);

}


void MHal_GPIO_R_ODD_3_High(void)
{

    MHal_GPIO_REG(REG_R_ODD_3_OUT) |= (BIT0);

}





void MHal_GPIO_R_ODD_4_High(void)
{

    MHal_GPIO_REG(REG_R_ODD_4_OUT) |= (BIT7);

}



void MHal_GPIO_R_ODD_5_High(void)
{

    MHal_GPIO_REG(REG_R_ODD_5_OUT) |= (BIT6);

}

void MHal_GPIO_R_ODD_6_High(void)
{

    MHal_GPIO_REG(REG_R_ODD_6_OUT) |= (BIT5);

}

void MHal_GPIO_R_ODD_7_High(void)
{

    MHal_GPIO_REG(REG_R_ODD_7_OUT) |= (BIT4);

}

void MHal_GPIO_mini_LVDS_0_High(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_0_OUT) |= (BIT3);

}



void MHal_GPIO_mini_LVDS_1_High(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_1_OUT) |= (BIT2);

}



void MHal_GPIO_mini_LVDS_2_High(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_2_OUT) |= (BIT1);

}


void MHal_GPIO_mini_LVDS_3_High(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_3_OUT)|= (BIT0);

}





void MHal_GPIO_mini_LVDS_4_High(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_4_OUT) |= (BIT7);

}



void MHal_GPIO_mini_LVDS_5_High(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_5_OUT) |= (BIT6);

}

void MHal_GPIO_mini_LVDS_6_High(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_6_OUT) |= (BIT5);

}

void MHal_GPIO_mini_LVDS_7_High(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_7_OUT) |= (BIT4);

}

void MHal_GPIO_LCK_High(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LCK_OUT) |= (BIT3);

}


void MHal_GPIO_LDE_High(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LDE_OUT) |= (BIT2);
}


void MHal_GPIO_LHSYNC_High(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LHSYNC_OUT) |= (BIT1);

}


void MHal_GPIO_LVSYNC_High(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LVSYNC_OUT) |= (BIT0);

}


void MHal_GPIO_PCM_WE_N_High(void)
{
    MHal_GPIO_REG(REG_PCM_WE_N_OUT) |= (BIT2);
}

//unctions of this section set to output low

void MHal_GPIO_PM_SPI_CZ_Low(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CZ_OUT) &= ~BIT4;
}

void MHal_GPIO_PM_SPI_CK_Low(void)
{
    MHal_GPIO_REG(REG_PM_SPI_CK_OUT) &= ~BIT5;
}
void MHal_GPIO_PM_SPI_DI_Low(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DI_OUT) &= ~BIT6;
}

void MHal_GPIO_PM_SPI_DO_Low(void)
{
    MHal_GPIO_REG(REG_PM_SPI_DO_OUT) &= ~BIT7;
}

void MHal_GPIO_IRIN_Low(void)
{
    MHal_GPIO_REG(REG_IRIN_OUT) &= ~BIT0;
}

void MHal_GPIO_CEC_Low(void)
{
    MHal_GPIO_REG(REG_CEC_OUT) &= ~BIT2;

}

void MHal_GPIO_GPIO_PM0_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM0_OUT) &= ~BIT0;

}

void MHal_GPIO_GPIO_PM1_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM1_OUT) &= ~BIT1;

}

void MHal_GPIO_GPIO_PM2_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM2_OUT) &= ~BIT2;

}

void MHal_GPIO_GPIO_PM3_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM3_OUT) &= ~BIT3;

}

void MHal_GPIO_GPIO_PM4_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM4_OUT) &= ~BIT4;
}

void MHal_GPIO_GPIO_PM5_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM5_OUT) &= ~BIT5;

}

void MHal_GPIO_GPIO_PM6_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM6_OUT) &= ~BIT6;

}

void MHal_GPIO_GPIO_PM7_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM7_OUT) &= ~BIT7;

}

void MHal_GPIO_GPIO_PM8_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM8_OUT) &= ~BIT0;

}

void MHal_GPIO_GPIO_PM9_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM9_OUT) &= ~BIT1;

}

void MHal_GPIO_GPIO_PM10_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM10_OUT) &= ~BIT2;

}

void MHal_GPIO_GPIO_PM11_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM11_OUT) &= ~BIT3;

}

void MHal_GPIO_GPIO_PM12_Low(void)
{
    MHal_GPIO_REG(REG_GPIO_PM12_OUT) &= ~BIT4;

}

void MHal_GPIO_HOTPLUGA_Low(void)
{
    MHal_GPIO_REG(REG_HOTPLUGA_OUT) &= ~BIT4;

}


void MHal_GPIO_HOTPLUGB_Low(void)
{
    MHal_GPIO_REG(REG_HOTPLUGB_OUT) &= ~BIT5;

}

void MHal_GPIO_HOTPLUGC_Low(void)
{
    MHal_GPIO_REG(REG_HOTPLUGC_OUT) &= ~BIT6;

}

void MHal_GPIO_HOTPLUGD_Low(void)
{
    MHal_GPIO_REG(REG_HOTPLUGD_OUT) &= ~BIT7;

}


void MHal_GPIO_DDCDA_CK_Low(void)
{
    MHal_GPIO_REG(REG_DDCDA_CK_OUT) &= ~BIT2;

}

void MHal_GPIO_DDCDA_DA_Low(void)
{
    MHal_GPIO_REG(REG_DDCDA_DA_OUT) &= ~BIT6;

}

void MHal_GPIO_DDCDB_CK_Low(void)
{
    MHal_GPIO_REG(REG_DDCDB_DA_OUT) &= ~BIT2;

}

void MHal_GPIO_DDCDB_DA_Low(void)
{
    MHal_GPIO_REG(REG_DDCDB_CK_OUT) &= ~BIT6;

}

void MHal_GPIO_DDCDC_CK_Low(void)
{
    MHal_GPIO_REG(REG_DDCDC_CK_OUT) &= ~BIT2;
}

void MHal_GPIO_DDCDC_DA_Low(void)
{
    MHal_GPIO_REG(REG_DDCDC_DA_OUT)  &= ~BIT6;
}

void MHal_GPIO_DDCDD_CK_Low(void)
{
    MHal_GPIO_REG(REG_DDCDD_CK_OUT) &= ~BIT2;
}

void MHal_GPIO_DDCDD_DA_Low(void)
{
    MHal_GPIO_REG(REG_DDCDD_DA_OUT) &= ~BIT6;

}

void MHal_GPIO_SAR0_Low(void)
{
    MHal_GPIO_REG(REG_SAR0_OUT) &= ~BIT0;

}

void MHal_GPIO_SAR1_Low(void)
{
    MHal_GPIO_REG(REG_SAR1_OUT) &= ~BIT1;

}

void MHal_GPIO_SAR2_Low(void)
{
    MHal_GPIO_REG(REG_SAR2_OUT) &= ~BIT2;
}

void MHal_GPIO_SAR3_Low(void)
{
    MHal_GPIO_REG(REG_SAR3_OUT) &= ~BIT3;
}

void MHal_GPIO_SAR4_Low(void)
{
    MHal_GPIO_REG(REG_SAR4_OUT) &= ~BIT4;
}

void MHal_GPIO_GPIO0_Low(void)
{
    MHal_GPIO_REG(REG_GPIO0_OUT) &= ~BIT0;

}

void MHal_GPIO_GPIO1_Low(void)
{
    MHal_GPIO_REG(REG_GPIO1_OUT) &= ~BIT1;

}

void MHal_GPIO_GPIO2_Low(void)
{
    MHal_GPIO_REG(REG_GPIO2_OUT) &= ~BIT2;

}

void MHal_GPIO_GPIO3_Low(void)
{
    MHal_GPIO_REG(REG_GPIO3_OUT) &= ~BIT3;

}

void MHal_GPIO_GPIO4_Low(void)
{
    MHal_GPIO_REG(REG_GPIO4_OUT) &= ~BIT4;
}

void MHal_GPIO_GPIO5_Low(void)
{
    MHal_GPIO_REG(REG_GPIO5_OUT) &= ~BIT5;
}

void MHal_GPIO_GPIO6_Low(void)
{

    MHal_GPIO_REG(REG_GPIO6_OUT) &= ~BIT6;

}

void MHal_GPIO_GPIO7_Low(void)
{
    MHal_GPIO_REG(REG_GPIO7_OUT) &= ~BIT7;

}

void MHal_GPIO_GPIO8_Low(void)
{
    MHal_GPIO_REG(REG_GPIO8_OUT) &= ~BIT0;
}

void MHal_GPIO_GPIO9_Low(void)
{
    MHal_GPIO_REG(REG_GPIO9_OUT) &= ~BIT1;
}

void MHal_GPIO_GPIO10_Low(void)
{
    MHal_GPIO_REG(REG_GPIO10_OUT) &= ~BIT2;

}

void MHal_GPIO_GPIO11_Low(void)
{
    MHal_GPIO_REG(REG_GPIO11_OUT) &= ~BIT3;

}

void MHal_GPIO_GPIO12_Low(void)
{
    MHal_GPIO_REG(REG_GPIO12_OUT) &= ~BIT4;

}
void MHal_GPIO_GPIO13_Low(void)
{
    MHal_GPIO_REG(REG_GPIO13_OUT) &= ~BIT5;

}
void MHal_GPIO_GPIO14_Low(void)
{
    MHal_GPIO_REG(REG_GPIO14_OUT) &= ~BIT6;
}
void MHal_GPIO_GPIO15_Low(void)
{
    MHal_GPIO_REG(REG_GPIO15_OUT) &= ~BIT7;

}
void MHal_GPIO_GPIO16_Low(void)
{
    MHal_GPIO_REG(REG_GPIO16_OUT) &= ~BIT0;

}
void MHal_GPIO_GPIO17_Low(void)
{
    MHal_GPIO_REG(REG_GPIO17_OUT) &= ~BIT1;

}
void MHal_GPIO_GPIO18_Low(void)
{
    MHal_GPIO_REG(REG_GPIO18_OUT) &= ~BIT2;

}

void MHal_GPIO_GPIO19_Low(void)
{
    MHal_GPIO_REG(REG_GPIO19_OUT) &= ~BIT3;

}


void MHal_GPIO_GPIO20_Low(void)
{
    MHal_GPIO_REG(REG_GPIO20_OUT) &= ~BIT4;

}


void MHal_GPIO_GPIO21_Low(void)
{
    MHal_GPIO_REG(REG_GPIO21_OUT) &= ~BIT5;

}


void MHal_GPIO_GPIO22_Low(void)
{
    MHal_GPIO_REG(REG_GPIO22_OUT) &= ~BIT6;
}

void MHal_GPIO_GPIO23_Low(void)
{
    MHal_GPIO_REG(REG_GPIO23_OUT) &= ~BIT7;
}


void MHal_GPIO_GPIO24_Low(void)
{
    MHal_GPIO_REG(REG_GPIO24_OUT) &= ~BIT0;

}
void MHal_GPIO_GPIO25_Low(void)
{
    MHal_GPIO_REG(REG_GPIO25_OUT) &= ~BIT1;

}

void MHal_GPIO_GPIO26_Low(void)
{
    MHal_GPIO_REG(REG_GPIO26_OUT) &= ~BIT2;

}
void MHal_GPIO_GPIO27_Low(void)
{
    MHal_GPIO_REG(REG_GPIO27_OUT) &= ~BIT3;

}

void MHal_GPIO_UART_RX2_Low(void)
{
    MHal_GPIO_REG(REG_UART_RX2_OUT) &= ~BIT4;

}
void MHal_GPIO_UART_TX2_Low(void)
{
    MHal_GPIO_REG(REG_UART_TX2_OUT) &= ~BIT5;

}

void MHal_GPIO_PWM0_Low(void)
{
    MHal_GPIO_REG(REG_PWM0_OUT) &= ~BIT6;
}
void MHal_GPIO_PWM1_Low(void)
{
    MHal_GPIO_REG(REG_PWM1_OUT) &= ~BIT7;

}
void MHal_GPIO_PWM2_Low(void)
{
    MHal_GPIO_REG(REG_PWM2_OUT) &= ~BIT0;
}


void MHal_GPIO_PWM3_Low(void)
{
    MHal_GPIO_REG(REG_PWM3_OUT) &= ~BIT1;
}


void MHal_GPIO_PWM4_Low(void)
{
    MHal_GPIO_REG(REG_PWM4_OUT) &= ~BIT2;
}

void MHal_GPIO_DDCR_DA_Low(void)
{
    MHal_GPIO_REG(REG_DDCR_DA_OUT) &= ~BIT3;

}

void MHal_GPIO_DDCR_CK_Low(void)
{
    MHal_GPIO_REG(REG_DDCR_CK_OUT) &= ~BIT4;

}


void MHal_GPIO_TGPIO0_Low(void)
{
    MHal_GPIO_REG(REG_TGPIO0_OUT) &= ~BIT5;

}


void MHal_GPIO_TGPIO1_Low(void)
{
    MHal_GPIO_REG(REG_TGPIO1_OUT) &= ~BIT6;

}


void MHal_GPIO_TGPIO2_Low(void)
{
    MHal_GPIO_REG(REG_TGPIO2_OUT)&= ~BIT7;
}


void MHal_GPIO_TGPIO3_Low(void)
{
    MHal_GPIO_REG(REG_TGPIO3_OUT) &= ~BIT0;
}


void MHal_GPIO_TS0_D0_Low(void)
{
    MHal_GPIO_REG(REG_TS0_D0_OUT) &= ~BIT0;

}


void MHal_GPIO_TS0_D1_Low(void)
{
    MHal_GPIO_REG(REG_TS0_D1_OUT) &= ~BIT1;

}


void MHal_GPIO_TS0_D2_Low(void)
{
    MHal_GPIO_REG(REG_TS0_D2_OUT) &= ~BIT2;

}


void MHal_GPIO_TS0_D3_Low(void)
{
    MHal_GPIO_REG(REG_TS0_D3_OUT) &= ~BIT3;

}
void MHal_GPIO_TS0_D4_Low(void)
{
    MHal_GPIO_REG(REG_TS0_D4_OUT) &= ~BIT4;

}


void MHal_GPIO_TS0_D5_Low(void)
{
    MHal_GPIO_REG(REG_TS0_D5_OUT) &= ~BIT5;

}


void MHal_GPIO_TS0_D6_Low(void)
{
    MHal_GPIO_REG(REG_TS0_D6_OUT) &= ~BIT6;
}



void MHal_GPIO_TS0_D7_Low(void)
{
    MHal_GPIO_REG(REG_TS0_D7_OUT) &= ~BIT7;

}


void MHal_GPIO_TS0_VLD_Low(void)
{
    MHal_GPIO_REG(REG_TS0_VLD_OUT) &= ~BIT0;
}


void MHal_GPIO_TS0_SYNC_Low(void)
{
    MHal_GPIO_REG(REG_TS0_SYNC_OUT) &= ~BIT1;
}



void MHal_GPIO_TS0_CLK_Low(void)
{
    MHal_GPIO_REG(REG_TS0_CLK_OUT) &= ~BIT2;
}

void MHal_GPIO_TS1_D0_Low(void)
{
    MHal_GPIO_REG(REG_TS1_D0_OUT) &= ~(BIT0);

}
void MHal_GPIO_TS1_D1_Low(void)
{
    MHal_GPIO_REG(REG_TS1_D1_OUT) &= ~(BIT1);

}


void MHal_GPIO_TS1_D2_Low(void)
{
    MHal_GPIO_REG(REG_TS1_D2_OUT) &= ~(BIT2);

}

void MHal_GPIO_TS1_D3_Low(void)
{
    MHal_GPIO_REG(REG_TS1_D3_OUT) &= ~(BIT3);

}

void MHal_GPIO_TS1_D4_Low(void)
{
    MHal_GPIO_REG(REG_TS1_D4_OUT) &= ~(BIT4);

}


void MHal_GPIO_TS1_D5_Low(void)
{
    MHal_GPIO_REG(REG_TS1_D5_OUT) &= ~(BIT5);

}


void MHal_GPIO_TS1_D6_Low(void)
{
    MHal_GPIO_REG(REG_TS1_D6_OUT) &= ~(BIT6);

}


void MHal_GPIO_TS1_D7_Low(void)
{
    MHal_GPIO_REG(REG_TS1_D7_OUT) &= ~(BIT7);

}


void MHal_GPIO_TS1_VLD_Low(void)
{
    MHal_GPIO_REG(REG_TS1_VLD_OUT) &= ~(BIT0);

}


void MHal_GPIO_TS1_SYNC_Low(void)
{
    MHal_GPIO_REG(REG_TS1_SYNC_OUT) &= ~(BIT1);
}


void MHal_GPIO_TS1_CLK_Low(void)
{
    MHal_GPIO_REG(REG_TS1_CLK_OUT) &= ~(BIT2);

}


void MHal_GPIO_PCM_A4_Low(void)
{

    if(GPIO99)
    {
        //PCM_GPIO0
        MHal_GPIO_REG(REG_PCM_A4_OUT1) &= ~(BIT0);

    }
    else
    {
        //NOR_GPIO15
        MHal_GPIO_REG(REG_PCM_A4_OUT2) &= ~(BIT7);



    }

}



void MHal_GPIO_PCM_WAIT_N_Low(void)
{
    MHal_GPIO_REG(REG_PCM_WAIT_N_OUT) &= ~(BIT1);

}
void MHal_GPIO_PCM_A5_Low(void)
{

    if(GPIO101)
    {
        //PCM_GPIO2
        MHal_GPIO_REG(REG_PCM_A5_OUT1) &= ~(BIT2);

    }
    else
    {
        //NOR_GPIO14
        MHal_GPIO_REG(REG_PCM_A5_OUT2) &= ~(BIT6);

    }

}


void MHal_GPIO_PCM_A6_Low(void)
{

    if(GPIO102)
    {
        //PCM_GPIO3
        MHal_GPIO_REG(REG_PCM_A6_OUT1) &= ~(BIT3);

    }
    else
    {
        //NOR_GPIO13
        MHal_GPIO_REG(REG_PCM_A6_OUT2) &= ~(BIT5);

    }

}



void MHal_GPIO_PCM_A7_Low(void)
{

    if(GPIO103)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A7_OUT1)  &= ~(BIT4);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A7_OUT2) &= ~(BIT4);

    }

}


void MHal_GPIO_PCM_A12_Low(void)
{

    if(GPIO104)
    {
        //PCM_GPIO4
        MHal_GPIO_REG(REG_PCM_A12_OUT1) &= ~(BIT5);

    }
    else
    {
        //NOR_GPIO12
        MHal_GPIO_REG(REG_PCM_A12_OUT2) &= ~(BIT3);

    }

}



void MHal_GPIO_PCM_IRQA_N_Low(void)
{
    MHal_GPIO_REG(REG_PCM_IRQA_N_OUT) &= ~(BIT6);

}

void MHal_GPIO_PCM_A14_Low(void)
{

    if(GPIO106)
    {
        MHal_GPIO_REG(REG_PCM_A14_OUT1) &= ~(BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A14_OUT2) &= ~(BIT2);

    }

}


void MHal_GPIO_PCM_A13_Low(void)
{

    if(GPIO107)
    {
        MHal_GPIO_REG(REG_PCM_A13_OUT1) &= ~(BIT0);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A13_OUT2)  &= ~(BIT1);

    }

}

void MHal_GPIO_PCM_A8_Low(void)
{

    if(GPIO108)
    {
        MHal_GPIO_REG(REG_PCM_A8_OUT1) &= ~(BIT1);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_OUT2) &= ~(BIT0);

    }

}



void MHal_GPIO_PCM_IOWR_N_Low(void)
{
    MHal_GPIO_REG(REG_PCM_IOWR_N_OUT) &= ~(BIT2);

}


void MHal_GPIO_PCM_A9_Low(void)
{

    if(GPIO110)
    {
        MHal_GPIO_REG(REG_PCM_A8_OUT1) &= ~(BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A8_OUT2) &= ~(BIT7);

    }

}



void MHal_GPIO_PCM_IORD_N_Low(void)
{
    MHal_GPIO_REG(REG_PCM_IORD_N_OUT) &= ~(BIT4);

}



void MHal_GPIO_PCM_A11_Low(void)
{

    if(GPIO112)
    {
        MHal_GPIO_REG(REG_PCM_A11_OUT2) &= ~(BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A11_OUT2) &= ~(BIT6);

    }

}


void MHal_GPIO_PCM_OE_N_Low(void)
{
    MHal_GPIO_REG(REG_PCM_OE_N_OUT) &= ~(BIT6);

}


void MHal_GPIO_PCM_A10_Low(void)
{

    if(GPIO114)
    {
        MHal_GPIO_REG(REG_PCM_A10_OUT1) &= ~(BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A10_OUT2) &= ~(BIT5);
    }


}
void MHal_GPIO_PCM_CE_N_Low(void)
{

    MHal_GPIO_REG(REG_PCM_CE_N_OUT) &= ~(BIT0);

}


void MHal_GPIO_PCM_D7_Low(void)
{

    if(GPIO116)
    {
        MHal_GPIO_REG(REG_PCM_D7_OUT1) &= ~(BIT1);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D7_OUT2) &= ~(BIT4);
    }

}


void MHal_GPIO_PCM_D6_Low(void)
{

    if(GPIO117)
    {
        MHal_GPIO_REG(REG_PCM_D6_OUT1) &= ~(BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D6_OUT2) &= ~(BIT3);
    }

}



void MHal_GPIO_PCM_D5_Low(void)
{

    if(GPIO118)
    {
        MHal_GPIO_REG(REG_PCM_D5_OUT1) &= ~(BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D5_OUT2) &= ~(BIT2);

    }

}


void MHal_GPIO_PCM_D4_Low(void)
{

    if(GPIO119)
    {
        MHal_GPIO_REG(REG_PCM_D4_OUT1) &= ~(BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D4_OUT2) &= ~(BIT1);
    }

}


void MHal_GPIO_PCM_D3_Low(void)
{

    if(GPIO120)
    {
        MHal_GPIO_REG(REG_PCM_D3_OUT1)  &= ~(BIT6);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D3_OUT2) &= ~(BIT0);
    }

}


void MHal_GPIO_PCM_A3_Low(void)
{
    if(GPIO121)
    {
        MHal_GPIO_REG(REG_PCM_A3_OUT1) &= ~(BIT7);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_OUT2) &= ~(BIT6);

    }

}



void MHal_GPIO_PCM_A2_Low(void)
{


    if(GPIO122)
    {
        MHal_GPIO_REG(REG_PCM_A2_OUT1) &= ~(BIT0);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A3_OUT2) &= ~(BIT7);
    }

}


void MHal_GPIO_PCM_REG_N_Low(void)
{
    MHal_GPIO_REG(REG_PCM_REG_N_OUT) &= ~(BIT1);
}

void MHal_GPIO_PCM_A1_Low(void)
{

    if(GPIO124)
    {
        MHal_GPIO_REG(REG_PCM_A1_OUT1) &= ~(BIT2);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A1_OUT2) &= ~(BIT0);

    }

}




void MHal_GPIO_PCM_A0_Low(void)
{

    if(GPIO125)
    {
        MHal_GPIO_REG(REG_PCM_A0_OUT1) &= ~(BIT3);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_A0_OUT2) &= ~(BIT1);

    }

}



void MHal_GPIO_PCM_D0_Low(void)
{


    if(GPIO126)
    {
        MHal_GPIO_REG(REG_PCM_A1_OUT1) &= ~(BIT4);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D0_OUT2) &= ~(BIT2);

    }

}


void MHal_GPIO_PCM_D1_Low(void)
{


    if(GPIO127)
    {
        MHal_GPIO_REG(REG_PCM_D1_OUT1) &= ~(BIT5);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D1_OUT2) &= ~(BIT3);

    }

}


void MHal_GPIO_PCM_D2_Low(void)
{


    if(GPIO128)
    {
        MHal_GPIO_REG(REG_PCM_D2_OUT1) &= ~(BIT6);

    }
    else
    {
        MHal_GPIO_REG(REG_PCM_D2_OUT2)  &= ~(BIT4);

    }

}



void MHal_GPIO_PCM_RESET_Low(void)
{

    MHal_GPIO_REG(REG_PCM_RESET_OUT) &= ~(BIT7);

}



void MHal_GPIO_PCM_CD_N_Low(void)
{

    MHal_GPIO_REG(REG_PCM_CD_N_OUT) &= ~(BIT0);

}
void MHal_GPIO_PCM2_CE_N_Low(void)
{

    MHal_GPIO_REG(REG_PCM2_CE_N_OUT)&= ~(BIT0);

}



void MHal_GPIO_PCM2_IRQA_N_Low(void)
{

    MHal_GPIO_REG(REG_PCM2_IRQA_N_OUT)&= ~(BIT1);

}



void MHal_GPIO_PCM2_WAIT_N_Low(void)
{

    MHal_GPIO_REG(REG_PCM2_WAIT_N_OUT) &= ~(BIT2);

}



void MHal_GPIO_PCM2_RESET_Low(void)
{

    MHal_GPIO_REG(REG_PCM2_RESET_OUT) &= ~(BIT3);

}



void MHal_GPIO_PCM2_CD_N_Low(void)
{

    MHal_GPIO_REG(REG_PCM2_CD_N_OUT) &= ~(BIT4);

}



void MHal_GPIO_PF_AD15_Low(void)
{

    if(GPIO136)
    {
        MHal_GPIO_REG(REG_PF_AD15_OUT1) &= ~(BIT0);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_AD15_OUT2) &= ~(BIT0);

    }

}



void MHal_GPIO_PF_CE0Z_Low(void)
{

    if(GPIO137)
    {
        MHal_GPIO_REG(REG_PF_CE0Z_OUT1) &= ~(BIT1);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE0Z_OUT2) &= ~(BIT1);

    }

}


void MHal_GPIO_PF_CE1Z_Low(void)
{

    if(GPIO138)
    {
        MHal_GPIO_REG(REG_PF_CE1Z_OUT1) &= ~(BIT2);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_CE1Z_OUT2) &= ~(BIT2);
    }

}


void MHal_GPIO_PF_OEZ_Low(void)
{

    if(GPIO139)
    {
        MHal_GPIO_REG(REG_PF_OEZ_OUT1)  &= ~(BIT3);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_OEZ_OUT2)  &= ~(BIT3);
    }

}



void MHal_GPIO_PF_WEZ_Low(void)
{

    if(GPIO140)
    {
        MHal_GPIO_REG(REG_PF_WEZ_OUT1) &= ~(BIT4);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_WEZ_OUT2) &= ~(BIT4);

    }

}



void MHal_GPIO_PF_ALE_Low(void)
{

    if(GPIO141)
    {
        MHal_GPIO_REG(REG_PF_ALE_OUT1) &= ~(BIT5);
    }
    else
    {
        MHal_GPIO_REG(REG_PF_ALE_OUT2) &= ~(BIT5);

    }

}



void MHal_GPIO_F_RBZ_Low(void)
{
    MHal_GPIO_REG(REG_F_RBZ_OUT) &= ~(BIT6);
}


//balup_090907

void MHal_GPIO_GPIO48_Low(void)
{
    MHal_GPIO_REG(REG_GPIO48_OUT) &= ~(BIT4);
}

void MHal_GPIO_GPIO49_Low(void)
{
    MHal_GPIO_REG(REG_GPIO48_OUT) &= ~(BIT5);
}

void MHal_GPIO_GPIO53_Low(void)
{
    MHal_GPIO_REG(REG_GPIO53_OUT) &= ~(BIT1);
}

void MHal_GPIO_GPIO52_Low(void)
{
    MHal_GPIO_REG(REG_GPIO53_OUT) &= ~(BIT0);
}

void MHal_GPIO_GPIO47_Low(void)
{
    MHal_GPIO_REG(REG_GPIO48_OUT) &= ~(BIT3);
}



void MHal_GPIO_TCON0_Low(void)
{
    MHal_GPIO_REG(REG_TCON0_OUT) &= ~(BIT0);
}



void MHal_GPIO_TCON1_Low(void)
{
    MHal_GPIO_REG(REG_TCON1_OUT) &= ~(BIT1);
}


void MHal_GPIO_TCON2_Low(void)
{
    MHal_GPIO_REG(REG_TCON2_OUT) &= ~(BIT2);
}



void MHal_GPIO_TCON3_Low(void)
{
    MHal_GPIO_REG(REG_TCON3_OUT) &= ~(BIT3);
}


void MHal_GPIO_TCON4_Low(void)
{
    MHal_GPIO_REG(REG_TCON4_OUT) &= ~(BIT4);
}



void MHal_GPIO_TCON5_Low(void)
{
    MHal_GPIO_REG(REG_TCON5_OUT) &= ~(BIT5);
}


void MHal_GPIO_TCON6_Low(void)
{
    MHal_GPIO_REG(REG_TCON6_OUT) &= ~(BIT6);
}

void MHal_GPIO_TCON7_Low(void)
{
    MHal_GPIO_REG(REG_TCON7_OUT)  &= ~(BIT7);
}


void MHal_GPIO_TCON8_Low(void)
{
    MHal_GPIO_REG(REG_TCON8_OUT) &= ~(BIT0);
}


void MHal_GPIO_TCON9_Low(void)
{
    MHal_GPIO_REG(REG_TCON9_OUT) &= ~(BIT1);
}


void MHal_GPIO_TCON10_Low(void)
{
    MHal_GPIO_REG(REG_TCON10_OUT) &= ~(BIT2);
}



void MHal_GPIO_TCON11_Low(void)
{
    MHal_GPIO_REG(REG_TCON11_OUT) &= ~(BIT3);
}


void MHal_GPIO_TCON12_Low(void)
{
    MHal_GPIO_REG(REG_TCON12_OUT) &= ~(BIT4);
}


void MHal_GPIO_TCON13_Low(void)
{
    MHal_GPIO_REG(REG_TCON13_OUT) &= ~(BIT5);
}



void MHal_GPIO_TCON14_Low(void)
{
    MHal_GPIO_REG(REG_TCON14_OUT) &= ~(BIT6);
}


void MHal_GPIO_TCON15_Low(void)
{
    MHal_GPIO_REG(REG_TCON15_OUT) &= ~(BIT7);
}


void MHal_GPIO_TCON16_Low(void)
{
    MHal_GPIO_REG(REG_TCON16_OUT) &= ~(BIT0);
}


void MHal_GPIO_TCON17_Low(void)
{
    MHal_GPIO_REG(REG_TCON17_OUT)  &= ~(BIT1);
}


void MHal_GPIO_TCON18_Low(void)
{
    MHal_GPIO_REG(REG_TCON18_OUT)  &= ~(BIT2);
}


void MHal_GPIO_TCON19_Low(void)
{
    MHal_GPIO_REG(REG_TCON19_OUT)  &= ~(BIT3);
}



void MHal_GPIO_TCON20_Low(void)
{
    MHal_GPIO_REG(REG_TCON20_OUT) &= ~(BIT4);
}


void MHal_GPIO_TCON21_Low(void)
{
    MHal_GPIO_REG(REG_TCON21_OUT)  &= ~(BIT5);
}



void MHal_GPIO_ET_COL_Low(void)
{

    MHal_GPIO_REG(REG_ET_COL_OUT)  &= ~(BIT0);

}


void MHal_GPIO_ET_TXD1_Low(void)
{

    MHal_GPIO_REG(REG_ET_TXD1_OUT) &= ~(BIT1);

}


void MHal_GPIO_ET_TXD0_Low(void)
{

    MHal_GPIO_REG(REG_ET_TXD0_OUT) &= ~(BIT2);


}



void MHal_GPIO_ET_TX_EN_Low(void)
{

    MHal_GPIO_REG(REG_ET_TX_EN_OUT) &= ~(BIT3);

}


void MHal_GPIO_ET_TX_CLK_Low(void)
{
    MHal_GPIO_REG(REG_ET_TX_CLK_OUT) &= ~(BIT4);

}


void MHal_GPIO_ET_RXD0_Low(void)
{
    MHal_GPIO_REG(REG_ET_RXD0_OUT) &= ~(BIT5);
}

void MHal_GPIO_ET_RXD1_Low(void)
{
    MHal_GPIO_REG(REG_ET_RXD1_OUT) &= ~(BIT6);
}

void MHal_GPIO_ET_MDC_Low(void)
{
    MHal_GPIO_REG(REG_ET_MDC_OUT) &= ~(BIT7);
}


void MHal_GPIO_ET_EDIO_Low(void)
{
    MHal_GPIO_REG(REG_ET_EDIO_OUT) &= ~(BIT0);
}


void MHal_GPIO_I2S_IN_WS_Low(void)
{

    MHal_GPIO_REG(REG_I2S_IN_WS_OUT) &= ~(BIT0);

}


void MHal_GPIO_I2S_IN_BCK_Low(void)
{
    MHal_GPIO_REG(REG_I2S_IN_BCK_OUT)  &= ~(BIT1);

}



void MHal_GPIO_I2S_IN_SD_Low(void)
{

    MHal_GPIO_REG(REG_I2S_IN_SD_OUT) &= ~(BIT2);

}


void MHal_GPIO_SPDIF_IN_Low(void)
{
    MHal_GPIO_REG(REG_SPDIF_IN_OUT) &= ~(BIT3);
}


void MHal_GPIO_SPDIF_OUT_Low(void)
{

    MHal_GPIO_REG(REG_SPDIF_OUT_OUT) &= ~(BIT4);

}


void MHal_GPIO_I2S_OUT_MCK_Low(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_MCK_OUT)&= ~(BIT5);

}


void MHal_GPIO_I2S_OUT_WS_Low(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_WS_OUT) &= ~(BIT6);

}


void MHal_GPIO_I2S_OUT_BCK_Low(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_BCK_OUT) &= ~(BIT7);

}


void MHal_GPIO_I2S_OUT_SD_Low(void)
{
    MHal_GPIO_REG(REG_I2S_OUT_SD_OUT) &= ~(BIT0);
}



void MHal_GPIO_I2S_OUT_SD1_Low(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD1_OUT) &= ~(BIT1);

}



void MHal_GPIO_I2S_OUT_SD2_Low(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD2_OUT) &= ~(BIT2);

}



void MHal_GPIO_I2S_OUT_SD3_Low(void)
{

    MHal_GPIO_REG(REG_I2S_OUT_SD3_OUT) &= ~(BIT3);

}


void MHal_GPIO_B_ODD_0_Low(void)
{

    MHal_GPIO_REG(REG_B_ODD_0_OUT) &= ~(BIT3);

}



void MHal_GPIO_B_ODD_1_Low(void)
{

    MHal_GPIO_REG(REG_B_ODD_1_OUT) &= ~(BIT2);

}



void MHal_GPIO_B_ODD_2_Low(void)
{

    MHal_GPIO_REG(REG_B_ODD_2_OUT) &= ~(BIT1);

}


void MHal_GPIO_B_ODD_3_Low(void)
{

    MHal_GPIO_REG(REG_B_ODD_3_OUT) &= ~(BIT0);

}





void MHal_GPIO_B_ODD_4_Low(void)
{

    MHal_GPIO_REG(REG_B_ODD_4_OUT) &= ~(BIT7);

}



void MHal_GPIO_B_ODD_5_Low(void)
{

    MHal_GPIO_REG(REG_B_ODD_5_OUT) &= ~(BIT6);

}

void MHal_GPIO_B_ODD_6_Low(void)
{

    MHal_GPIO_REG(REG_B_ODD_6_OUT) &= ~(BIT5);

}

void MHal_GPIO_B_ODD_7_Low(void)
{

    MHal_GPIO_REG(REG_B_ODD_7_OUT) &= ~(BIT4);
}

void MHal_GPIO_G_ODD_0_Low(void)
{

    MHal_GPIO_REG(REG_G_ODD_0_OUT) &= ~(BIT3);

}



void MHal_GPIO_G_ODD_1_Low(void)
{

    MHal_GPIO_REG(REG_G_ODD_1_OUT) &= ~(BIT2);

}



void MHal_GPIO_G_ODD_2_Low(void)
{

    MHal_GPIO_REG(REG_G_ODD_2_OUT) &= ~(BIT1);

}


void MHal_GPIO_G_ODD_3_Low(void)
{

    MHal_GPIO_REG(REG_G_ODD_3_OUT) &= ~(BIT0);

}





void MHal_GPIO_G_ODD_4_Low(void)
{

    MHal_GPIO_REG(REG_G_ODD_4_OUT) &= ~(BIT7);

}



void MHal_GPIO_G_ODD_5_Low(void)
{

    MHal_GPIO_REG(REG_G_ODD_5_OUT) &= ~(BIT6);

}

void MHal_GPIO_G_ODD_6_Low(void)
{

    MHal_GPIO_REG(REG_G_ODD_6_OUT) &= ~(BIT5);

}

void MHal_GPIO_G_ODD_7_Low(void)
{

    MHal_GPIO_REG(REG_G_ODD_7_OUT)&= ~(BIT4);

}

void MHal_GPIO_R_ODD_0_Low(void)
{

    MHal_GPIO_REG(REG_R_ODD_0_OUT) &= ~(BIT3);

}



void MHal_GPIO_R_ODD_1_Low(void)
{

    MHal_GPIO_REG(REG_R_ODD_1_OUT) &= ~(BIT2);

}



void MHal_GPIO_R_ODD_2_Low(void)
{

    MHal_GPIO_REG(REG_R_ODD_2_OUT) &= ~(BIT1);

}


void MHal_GPIO_R_ODD_3_Low(void)
{

    MHal_GPIO_REG(REG_R_ODD_3_OUT) &= ~(BIT0);

}





void MHal_GPIO_R_ODD_4_Low(void)
{

    MHal_GPIO_REG(REG_R_ODD_4_OUT) &= ~(BIT7);

}



void MHal_GPIO_R_ODD_5_Low(void)
{

    MHal_GPIO_REG(REG_R_ODD_5_OUT) &= ~(BIT6);

}

void MHal_GPIO_R_ODD_6_Low(void)
{

    MHal_GPIO_REG(REG_R_ODD_6_OUT) &= ~(BIT5);

}

void MHal_GPIO_R_ODD_7_Low(void)
{

    MHal_GPIO_REG(REG_R_ODD_7_OUT) &= ~(BIT4);

}

void MHal_GPIO_mini_LVDS_0_Low(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_0_OUT) &= ~(BIT3);

}



void MHal_GPIO_mini_LVDS_1_Low(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_1_OUT) &= ~(BIT2);

}



void MHal_GPIO_mini_LVDS_2_Low(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_2_OUT) &= ~(BIT1);

}


void MHal_GPIO_mini_LVDS_3_Low(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_3_OUT)&= ~(BIT0);

}





void MHal_GPIO_mini_LVDS_4_Low(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_4_OUT) &= ~(BIT7);

}



void MHal_GPIO_mini_LVDS_5_Low(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_5_OUT) &= ~(BIT6);

}

void MHal_GPIO_mini_LVDS_6_Low(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_6_OUT) &= ~(BIT5);

}

void MHal_GPIO_mini_LVDS_7_Low(void)
{

    MHal_GPIO_REG(REG_mini_LVDS_7_OUT) &= ~(BIT4);

}

void MHal_GPIO_LCK_Low(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LCK_OUT) &= ~(BIT3);

}


void MHal_GPIO_LDE_Low(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LDE_OUT) &= ~(BIT2);
}


void MHal_GPIO_LHSYNC_Low(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LHSYNC_OUT) &= ~(BIT1);

}


void MHal_GPIO_LVSYNC_Low(void)
{
    MHal_GPIO_REG(REG_LVDS_BANK) = 0;

    MHal_GPIO_REG(REG_LVSYNC_OUT) &= ~(BIT0);

}


void MHal_GPIO_PCM_WE_N_Low(void)
{
    MHal_GPIO_REG(REG_PCM_WE_N_OUT) &= ~(BIT2);
}


