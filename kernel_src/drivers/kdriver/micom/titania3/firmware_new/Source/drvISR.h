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
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_ISR_H_
#define _DRV_ISR_H_

/******************************************************************************/
// Interrupt priority groups
//          Highest     Middle      Lowest
// Group0   EX0         EX7         SERIAL1
// Group1   TIMER0      EX2         EX8
// Group2   EX1         EX3         EX9
// Group3   TIMER1      EX4         EX10
// Group4   SERIAL0     EX5         EX11
// Group5   TIMER2      EX6         EX12
//
// Interrupt priority levels
// 0    Lowest
// 1
// 2
// 3    Highest
/******************************************************************************/
#if 0
#ifndef DISABLE_INTERRUPT_PRIORITY
#define DISABLE_INTERRUPT_PRIORITY		0
#endif

#if DISABLE_INTERRUPT_PRIORITY
#define INT_GRP0_PRIO       0
#define INT_GRP1_PRIO       0
#define INT_GRP2_PRIO       0
#define INT_GRP3_PRIO       0
#define INT_GRP4_PRIO       0
#define INT_GRP5_PRIO       0
#else
// customized for priority of group 0 ~ 5
#define INT_GRP0_PRIO       2
#define INT_GRP1_PRIO       2
#define INT_GRP2_PRIO       3   // Now EX1 must be 3, sec isr use it
#define INT_GRP3_PRIO       1
#define INT_GRP4_PRIO       1
#define INT_GRP5_PRIO       1
#endif
#else
// customized for priority of group 0 ~ 5
#define INT_GRP0_PRIO       1
#define INT_GRP1_PRIO       1
#define INT_GRP2_PRIO       1   // Now EX1 must be 3, sec isr use it
#define INT_GRP3_PRIO       1
#define INT_GRP4_PRIO       1
#define INT_GRP5_PRIO       1

#endif

// customized for register bank of priority 0 ~ 3
#define INT_PRIO0_REGBANK   0
#define INT_PRIO1_REGBANK   1
#define INT_PRIO2_REGBANK   2
#define INT_PRIO3_REGBANK   3

//
// Following data is from R8051XC design spec, not customizable value
//

//      interrupt      keil C number   source   description
#define INT_EX0             0       // ie0      External interrupt 0
#define INT_TIMER0          1       // tf0      Timer 0 overflow
#define INT_EX1             2       // ie1      External interrupt 1
#define INT_TIMER1          3       // tf1      Timer1 overflow
#define INT_SERIAL0         4       // riti0    Serial port 0 interrupt
#define INT_TIMER2          5       // tfexf2   Timer2 interrupt
#define INT_EX7             8       // iex7     External interrupt 7
#define INT_EX2             9       // iex2     External interrupt 2
#define INT_EX3             10      // iex3     External interrupt 3
#define INT_EX4             11      // iex4     External interrupt 4
#define INT_EX5             12      // iex5     External interrupt 5
#define INT_EX6             13      // iex6     External interrupt 6
#define INT_SERIAL1         16      // riti1    Serial port 1 interrupt
#define INT_EX8             17      // iex8     External interrupt 8
#define INT_EX9             18      // iex9     External interrupt 9
#define INT_EX10            19      // iex10    External interrupt 10
#define INT_EX11            20      // iex11    External interrupt 11
#define INT_EX12            21      // iex12    External interrupt 12

// interrupt to group mapping
#define INT_GROUP_EX0       0
#define INT_GROUP_EX7       0
#define INT_GROUP_SERIAL1   0
#define INT_GROUP_TIMER0    1
#define INT_GROUP_EX2       1
#define INT_GROUP_EX8       1
#define INT_GROUP_EX1       2
#define INT_GROUP_EX3       2
#define INT_GROUP_EX9       2
#define INT_GROUP_TIMER1    3
#define INT_GROUP_EX4       3
#define INT_GROUP_EX10      3
#define INT_GROUP_SERIAL0   4
#define INT_GROUP_EX5       4
#define INT_GROUP_EX11      4
#define INT_GROUP_TIMER2    5
#define INT_GROUP_EX6       5
#define INT_GROUP_EX12      5

// get group of irq
#define INT_GROUP( irq )    CONCAT( INT_GROUP_, irq )

// get priority of irq
#define INT_PRIO( irq )     CONCAT( INT_GRP, CONCAT( INT_GROUP( irq ), _PRIO ) )

// get default register bank of irq
#define INT_REGBANK( irq )  CONCAT( INT_PRIO, CONCAT( INT_PRIO( irq ), _REGBANK ) )

#endif /* _DRV_ISR_H_ */
