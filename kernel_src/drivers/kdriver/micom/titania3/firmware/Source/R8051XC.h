#ifndef _R8051XC_H_
#define _R8051XC_H_

#define ISR_TYPE_R8051XC    1

// BYTE REGISTER DEFINE
sfr  P0         = 0x80;
sfr  P1         = 0x90;
sfr  P2         = 0xA0;
sfr  P3         = 0xB0;
sfr  PSW        = 0xD0;
sfr  ACC        = 0xE0;
sfr  B          = 0xF0;

sfr  SP         = 0x81;
sfr  DPL        = 0x82;
sfr  DPH        = 0x83;
sfr  DPL1       = 0x84;
sfr  DPH1       = 0x85;
sfr  WDTREL     = 0x86;
sfr  PCON       = 0x87;
sfr  TCON       = 0x88;
sfr  TMOD       = 0x89;
sfr  TL0        = 0x8A;
sfr  TL1        = 0x8B;
sfr  TH0        = 0x8C;
sfr  TH1        = 0x8D;
sfr  CKCON      = 0x8E;

sfr  DPS        = 0x92;
sfr  DPC        = 0x93;
sfr  PAGESEL    = 0x94;
sfr  S0CON      = 0x98;
sfr  S0BUF      = 0x99;
sfr  S1CON      = 0x9B;
sfr  S1BUF      = 0x9C;
sfr  S1RELL     = 0x9D;

sfr  DMAS0      = 0xA1;
sfr  DMAS1      = 0xA2;
sfr  DMAS2      = 0xA3;
sfr  DMAT0      = 0xA4;
sfr  DMAT1      = 0xA5;
sfr  DMAT2      = 0xA6;

sfr  S0RELL     = 0xAA;

sfr  DMAC0      = 0xB1;
sfr  DMAC1      = 0xB2;
sfr  DMAC2      = 0xB3;
sfr  DMASEL     = 0xB4;
sfr  DMAM0      = 0xB5;
sfr  DMAM1      = 0xB6;
sfr  S0RELH     = 0xBA;
sfr  S1RELH     = 0xBB;

sfr  CCEN       = 0xC1;
sfr  CCL1       = 0xC2;
sfr  CCH1       = 0xC3;
sfr  CCL2       = 0xC4;
sfr  CCH2       = 0xC5;
sfr  CCL3       = 0xC6;
sfr  CCH3       = 0xC7;
sfr  T2CON      = 0xC8;
sfr  CRCL       = 0xCA;
sfr  CRCH       = 0xCB;
sfr  TL2        = 0xCC;
sfr  TH2        = 0xCD;

sfr  I2C2DAT    = 0xD2;
sfr  I2C2ADR    = 0xD3;
sfr  I2C2CON    = 0xD4;
sfr  I2C2STA    = 0xD5;
sfr  ADCON      = 0xD8;
sfr  I2CDAT     = 0xDA;
sfr  I2CADR     = 0xDB;
sfr  I2CCON     = 0xDC;
sfr  I2CSTA     = 0xDD;

sfr  SPSTA      = 0xE1;
sfr  SPCON      = 0xE2;
sfr  SPDAT      = 0xE3;
sfr  SPSSN      = 0xE4;
sfr  MD0        = 0xE9;
sfr  MD1        = 0xEA;
sfr  MD2        = 0xEB;
sfr  MD3        = 0xEC;
sfr  MD4        = 0xED;
sfr  MD5        = 0xEE;
sfr  ARCON      = 0xEF;

sfr  SRST       = 0xF7;

#if ISR_TYPE_R8051XC

sfr  IEN0       = 0xA8;
sfr  IEN1       = 0xB8;
sfr  IEN2       = 0x9A;
sfr  IP0        = 0xA9;
sfr  IP1        = 0xB9;
sfr  IRCON      = 0xC0;
sfr  IRCON2     = 0xBF;

// IEN0::0xA8
sbit EAL        = 0xAF;
sbit WDT        = 0xAE;
sbit ET2        = 0xAD;
sbit ES0        = 0xAC;
sbit ET1        = 0xAB;
sbit EX1        = 0xAA;
sbit ET0        = 0xA9;
sbit EX0        = 0xA8;

// IEN1::0xB8
sbit EXEN2      = 0xBF;
sbit SWDT       = 0xBE;
sbit EX6        = 0xBD;
sbit EX5        = 0xBC;
sbit EX4        = 0xBB;
sbit EX3        = 0xBA;
sbit EX2        = 0xB9;
sbit EX7        = 0xB8;

// IRCON::0xC0
sbit EXF2       = 0xC7;
sbit TF2        = 0xC6;
sbit IEX6       = 0xC5;
sbit IEX5       = 0xC4;
sbit IEX4       = 0xC3;
sbit IEX3       = 0xC2;
sbit IEX2       = 0xC1;
sbit IEX7       = 0xC0;

#else   // ISR_TYPE_8051

sfr  IP         = 0xB8;

/*  IP   */
sbit PT2        = 0xBD;
sbit PS         = 0xBC;
sbit PT1        = 0xBB;
sbit PX1        = 0xBA;
sbit PT0        = 0xB9;
sbit PX0        = 0xB8;

#endif


//===========================
// BIT REGISTER DEFINE
// PSW::0xD0
sbit CY         = 0xD7;
sbit AC         = 0xD6;
sbit F0         = 0xD5;
sbit RS1        = 0xD4;
sbit RS0        = 0xD3;
sbit OV         = 0xD2;
sbit F1         = 0xD1;
sbit UD         = 0xD1;     // alias of F1
sbit P          = 0xD0;

// TCON::0x88
sbit TF1        = 0x8F;
sbit TR1        = 0x8E;
sbit TF0        = 0x8D;
sbit TR0        = 0x8C;
sbit IE1        = 0x8B;
sbit IT1        = 0x8A;
sbit IE0        = 0x89;
sbit IT0        = 0x88;

// T2CON::0xC8
sbit T2PS       = 0xCF;
sbit I3FR       = 0xCE;
sbit I2FR       = 0xCD;
sbit T2R1       = 0xCC;
sbit T2R0       = 0xCB;
sbit T2CM       = 0xCA;
sbit T2I1       = 0xC9;
sbit T2I0       = 0xC8;

// S0CON::0x98
sbit SM0        = 0x9F;
sbit SM1        = 0x9E;
sbit SM20       = 0x9D;
sbit REN0       = 0x9C;
sbit TB80       = 0x9B;
sbit RB80       = 0x9A;
sbit TI0        = 0x99;
sbit RI0        = 0x98;

/*  P3  */
sbit RD         = 0xB7;
sbit WR         = 0xB6;
sbit T1         = 0xB5;
sbit T0         = 0xB4;
sbit INT1       = 0xB3;
sbit INT0       = 0xB2;
sbit TXD        = 0xB1;
sbit RXD        = 0xB0;

/*  SCON1  */
sbit SM0_1      = 0xC7;
sbit SM1_1      = 0xC6;
sbit SM2_1      = 0xC5;
sbit REN_1      = 0xC4;
sbit TB8_1      = 0xC3;
sbit RB8_1      = 0xC2;
sbit TI_1       = 0xC1;
sbit RI_1       = 0xC0;

/*  WDTCON  */
sbit WDTEN      = 0xD9;
sbit WDTRST     = 0xD8;

// P0::0x80
sbit P0_0       = 0x80;
sbit P0_1       = 0x81;
sbit P0_2       = 0x82;
sbit P0_3       = 0x83;
sbit P0_4       = 0x84;
sbit P0_5       = 0x85;
sbit P0_6       = 0x86;
sbit P0_7       = 0x87;

// P1::0x90
sbit P1_0       = 0x90;
sbit P1_1       = 0x91;
sbit P1_2       = 0x92;
sbit P1_3       = 0x93;
sbit P1_4       = 0x94;
sbit P1_5       = 0x95;
sbit P1_6       = 0x96;
sbit P1_7       = 0x97;

// P2::0xA0
sbit P2_0       = 0xA0;
sbit P2_1       = 0xA1;
sbit P2_2       = 0xA2;
sbit P2_3       = 0xA3;
sbit P2_4       = 0xA4;
sbit P2_5       = 0xA5;
sbit P2_6       = 0xA6;
sbit P2_7       = 0xA7;

// P3::0xB0
sbit P3_0       = 0xB0;
sbit P3_1       = 0xB1;
sbit P3_2       = 0xB2;
sbit P3_3       = 0xB3;
sbit P3_4       = 0xB4;
sbit P3_5       = 0xB5;
sbit P3_6       = 0xB6;
sbit P3_7       = 0xB7;

#endif
