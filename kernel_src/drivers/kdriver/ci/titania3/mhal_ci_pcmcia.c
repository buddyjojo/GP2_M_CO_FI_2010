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

///////////////////////////////////////////////////////////////////////////////
///
/// file    mhal_ci_pcmcia.c
/// @brief  PCMCIA Device Driver of CI Physical Layer
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////

#define  _DRVCI_PCMCIA_C_

/*****************************************************************************/
/*                       Header Files                                        */
/*****************************************************************************/
#include <linux/sched.h>
#include <linux/kthread.h>
#include <asm/semaphore.h>
#include "mdrv_ci.h"

/*****************************************************************************/
/*                       Define                                              */
/*****************************************************************************/
#define PCMCIA_SLEEP_INTERVAL               50//ms
#define PCMCIA_CARD_DETECT_TIME             1000//ms
#define PCMCIA_CARD_DETECT_TIMES            (PCMCIA_CARD_DETECT_TIME/PCMCIA_SLEEP_INTERVAL)

#define CI_INIT                             0x00
#define CI_CARDINSERTED                     0x01
#define CI_CARDREMOVED                      0x02
#define CI_CARDINIT                         0x04
#define CI_CONNECTED                        0x08
//#define CI_LINK                           0x10
//#define CI_SESSION                        0x20
//#define CI_APP                            0x40
#define CI_IDLE                             0x80

/* CISTPL_VERS_1 */
#define TPLLV1_MAJOR_VAL                    0x05
#define TPLLV1_MINOR_VAL                    0x00
#if 0
#define STCI_STR                            "DVB_CI_V"
#endif

/*****************************************************************************/
/*                       Macro                                               */
/*****************************************************************************/
#define DRV_CI_PCMCIA_DEBUG_ENABLE          0

#if DRV_CI_PCMCIA_DEBUG_ENABLE
#define DRV_CI_PCMCIA_DEBUG(fmt, args...)   printk("" fmt, ## args)
#else
#define DRV_CI_PCMCIA_DEBUG(fmt, args...)
#endif

/*****************************************************************************/
/*                       Global Variables                                    */
/*****************************************************************************/
extern struct semaphore PfModeSem;
#if PCMCIA_IRQ_ENABLE
static BOOL _gbPCMCIA_IrqStatus  = FALSE;
#endif

/*****************************************************************************/
/*                       Local Variables                                     */
/*****************************************************************************/
static B16 _gbCardAInsideCI;
static B16 _gbBypassModeCI;
static PCMCIA_INFO_t *_pPCMCIA_Info;
static PCMCIA_HANDLE _hPCMCIA;

/****************************************************************************/
/*                       Local Functions                                    */
/****************************************************************************/
static void _MDrv_CI_PCMCIA_GetSemaphore(void);
static void _MDrv_CI_PCMCIA_ReleaseSemaphore(void);
static void _MDrv_CI_PCMCIA_ReadReg(U8 u8Addr, U8 *pu8Value);
static void _MDrv_CI_PCMCIA_WriteReg(U8 u8Addr, U8 u8Value);

static void _MDrv_CI_PCMCIA_GetSemaphore(void)
{
    volatile U8 *pu8Reg;

    down(&PfModeSem);

    // 0x1EDE BIT4 REG_PF_MODE: O for PCMAIA; 1 for PF
    pu8Reg = (U8 *)(REG_MIPS_BASE + 0x1EDE * 2);
    *pu8Reg &= 0xEF;
}

static void _MDrv_CI_PCMCIA_ReleaseSemaphore(void)
{
    volatile U8 *pu8Reg;
    
    // 0x1EDE BIT4 REG_PF_MODE: O for PCMAIA; 1 for PF
    pu8Reg = (U8 *)(REG_MIPS_BASE + 0x1EDE * 2);
    *pu8Reg |= 0x10;

    up(&PfModeSem);
}

static void _MDrv_CI_PCMCIA_ReadReg(U8 u8Addr, U8 *pu8Value)
{
    volatile U8 *pu8Reg;

    if (u8Addr & 0x01)
        u8Addr = (u8Addr - 1) * 2 + 1;
    else
        u8Addr = (u8Addr) * 2;

    pu8Reg = (U8 *)CI_REG(u8Addr);
    *pu8Value = *pu8Reg;
}

static void _MDrv_CI_PCMCIA_WriteReg(U8 u8Addr, U8 u8Value)
{
    volatile U8 *pu8Reg;

    if (u8Addr & 0x01)
        u8Addr = (u8Addr - 1) * 2 + 1;
    else
        u8Addr = (u8Addr) * 2;

    pu8Reg = (U8 *)CI_REG(u8Addr);
    *pu8Reg = u8Value;
}

void MDrv_CI_PCMCIA_SwitchPower(U8 u8OnOff)
{
    volatile U8 *pu8Reg;

    if (u8OnOff)
    {
        /* PCMCIA Power on. */
        pu8Reg = (U8 *)(REG_MIPS_BASE + (0x1E86 * 2));
        *pu8Reg |= BIT3;
    }
    else
    {
        /* PCMCIA Power off. */
        pu8Reg = (U8 *)(REG_MIPS_BASE + (0x1E86 * 2));
        *pu8Reg &= (~BIT3);
    }
}
void MDrv_CI_PCMCIA_ISR(void)
{
    U8 u8Value;
    static U8 u8Count;
    //volatile U8 *pu8Reg;

    _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA_DONE_BUS_IDLE, &u8Value);

    if (u8Value & BIT2)
    { // Card Out.
        if (FALSE != _gbCardAInsideCI)
        {
            u8Count = 0;
            _gbCardAInsideCI = FALSE;
            {
                /* PCMCIA Power off. */
               // pu8Reg = (U8 *)(REG_MIPS_BASE + (0x1E86 * 2));
               // *pu8Reg &= (~BIT3);
                MDrv_CI_PCMCIA_SwitchPower(0);
            }
        }
    }
    else
    { // Card In.
        if (FALSE == _gbCardAInsideCI)
        {
            if (PCMCIA_CARD_DETECT_TIMES == u8Count)
            {
                _gbCardAInsideCI = TRUE;
                {
                    /* PCMCIA Power on. */
                   // pu8Reg = (U8 *)(REG_MIPS_BASE + (0x1E86 * 2));
                   // *pu8Reg |= BIT3;
                   MDrv_CI_PCMCIA_SwitchPower(1);
                }
                //wgkwak in LGE
                //Normally remove msleep in ISR
                MDrv_CI_PCMCIA_HwRstPCMCIA(PCMCIA_RESETDELAY_ZERO);
            }
            else
                u8Count++;
        }
    }
}

//wgkwak in LGE
void MDrv_CI_PCMCIA_HwRstPCMCIA(unsigned int resetDelay)
{
    U8 u8Value;

    _MDrv_CI_PCMCIA_ReadReg(REG_MODULE_VCC_OOB, &u8Value);
    u8Value &= 0xD8;
    u8Value |= 0x05;    // module reset to slot A - [BIT2]1: RESET = HIGH
    _MDrv_CI_PCMCIA_WriteReg(REG_MODULE_VCC_OOB, u8Value);
    msleep(10);         // MUST...for HW reset
    u8Value &= 0xD8;
    u8Value |= 0x01;    // module reset to slot A - [BIT2]0: RESET = LOW
    _MDrv_CI_PCMCIA_WriteReg(REG_MODULE_VCC_OOB, u8Value);

    //wgkwak in LGE
    //msleep(2000);     // MUST...for HW reset with S5 MIPS platform (Conax TechniCrypt CXV needs 2000ms)
    
    _MDrv_CI_PCMCIA_WriteReg( REG_INT_MASK_CLEAR, 0x7F );   // Mask all PCMCIA IRQ.
    
    msleep(resetDelay);
}

B16 MDrv_CI_PCMCIA_IsModuleStillPlugged(void)
{
    MDrv_CI_PCMCIA_ISR();
    
    return (_gbCardAInsideCI);
}

void MDrv_CI_PCMCIA_EnableTSRouting(PCMCIA_HANDLE hSlot, B16 fEnable)
{
    hSlot = hSlot;
    fEnable = fEnable;
}

//! This function is called to write the byte bData into the card IO memory at address wAddr.
void MDrv_CI_PCMCIA_WriteIOMem(PCMCIA_HANDLE hSlot, U16 wAddr, U8 bData)
{
    U8 u8value;
    U16 i;

    hSlot = hSlot;
    // select attribute memory write, low byte
    _MDrv_CI_PCMCIA_WriteReg(REG_PCM_MEM_IO_CMD, IO_WRITE);

    _MDrv_CI_PCMCIA_GetSemaphore();
    {
        // write address
        _MDrv_CI_PCMCIA_WriteReg(REG_ADDR1, ( wAddr >> 8 ));
        _MDrv_CI_PCMCIA_WriteReg(REG_ADDR0, wAddr );
        // write data
        _MDrv_CI_PCMCIA_WriteReg(REG_WRITE_DATA, bData);

        // fire command
        _MDrv_CI_PCMCIA_WriteReg(REG_FIRE_READ_DATA_CLEAR, 1);
    }
    _MDrv_CI_PCMCIA_ReleaseSemaphore();

    //polling if fire is done
    for (i = 0; i < 20000; i++)
    {
        _MDrv_CI_PCMCIA_ReadReg(REG_FIRE_READ_DATA_CLEAR, &u8value);

        if (!(u8value & 0x01))
        {
            break;
        }
        msleep(1);
    }

    // polling if bus is idle
    for (i = 0; i < 20000; i++)
    {
        _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA_DONE_BUS_IDLE, &u8value);

        if (u8value & 0x02)
        {
            break;
        }
        msleep(1);
    }

    //printk("Write Type %bx, Addr %x, value %bx\n", u8AccessType, Addr, u8data);
}

//! This function is read one byte of from the card IO memory at address wAddr.
U8 MDrv_CI_PCMCIA_ReadIOMem(PCMCIA_HANDLE hSlot, U16 wAddr)
{
    U8 u8value, u8mem = 0x00;
    U16 i;

    hSlot = hSlot;
    // select attribute memory read, low byte
    _MDrv_CI_PCMCIA_WriteReg(REG_PCM_MEM_IO_CMD, IO_READ);

    _MDrv_CI_PCMCIA_GetSemaphore();
    {
        // read address
        _MDrv_CI_PCMCIA_WriteReg(REG_ADDR1, ( wAddr >> 8 ));
        _MDrv_CI_PCMCIA_WriteReg(REG_ADDR0, wAddr);

        // fire command
        _MDrv_CI_PCMCIA_WriteReg(REG_FIRE_READ_DATA_CLEAR, 1);
    }
    _MDrv_CI_PCMCIA_ReleaseSemaphore();

    //polling if fire is done
    for (i = 0; i < 20000; i++)
    {
        _MDrv_CI_PCMCIA_ReadReg(REG_FIRE_READ_DATA_CLEAR, &u8value);

        if (!( u8value & 0x01 ))
        {
            break;
        }
        msleep(1);
    }

    //polling if data ready
    for (i = 0; i < 20000; i++)
    {
        _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA_DONE_BUS_IDLE, &u8value);

        if (u8value & 0x01)
        {
            _MDrv_CI_PCMCIA_GetSemaphore();
            {
                _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA, &u8mem);
            }
            _MDrv_CI_PCMCIA_ReleaseSemaphore();
            break;
        }
        msleep(1);
    }
    // clean stat_rd done
    _MDrv_CI_PCMCIA_WriteReg(REG_FIRE_READ_DATA_CLEAR, 2);

    // polling if bus is idle
    for (i = 0; i < 20000; i++)
    {
        _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA_DONE_BUS_IDLE, &u8value);

        if ((u8value & 0x03) == 0x0002)
        {
            break;
        }
        msleep(1);
    }

    //printk("Read Addr %x, value %bx\n", wAddr, u8mem);
    return u8mem;
}

void MDrv_CI_PCMCIA_WriteAttribMem(PCMCIA_HANDLE hSlot, U16 wAddr, U8 bData)
{
    U8 u8value;
    U16 i;

    hSlot = hSlot;
    // select attribute memory write, low byte
    _MDrv_CI_PCMCIA_WriteReg(REG_PCM_MEM_IO_CMD, ATTRIBMEMORY_WRITE );

    _MDrv_CI_PCMCIA_GetSemaphore();
    {
        // write address
        _MDrv_CI_PCMCIA_WriteReg(REG_ADDR1, (wAddr >> 8));
        _MDrv_CI_PCMCIA_WriteReg(REG_ADDR0, wAddr);

        // write data
        _MDrv_CI_PCMCIA_WriteReg(REG_WRITE_DATA, bData);

        // fire command
        _MDrv_CI_PCMCIA_WriteReg(REG_FIRE_READ_DATA_CLEAR, 1);
    }
    _MDrv_CI_PCMCIA_ReleaseSemaphore();

    //polling if fire is done
    for (i = 0; i < 20000; i++)
    {
        _MDrv_CI_PCMCIA_ReadReg(REG_FIRE_READ_DATA_CLEAR, &u8value);

        if (!(u8value & 0x01))
        {
            break;
        }
        msleep(1);
    }

    // polling if bus is idle
    for (i = 0; i < 20000; i++)
    {
        _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA_DONE_BUS_IDLE, &u8value);

        if (u8value & 0x02)
        {
            break;
        }
        msleep(1);
    }

    //printf("Write Type %bx, Addr %x, value %bx\n", u8AccessType, Addr, u8data);
    return;
}

void MDrv_CI_PCMCIA_ReadAttribMem(U16 Addr, U8 *pDest)
{
    int i, j;
    U8 u8value;

    // CIS readout with 8Bit I/O accesses
    // requires that we read only every second
    // byte. (The result of reading the even addresses does not seem to work on most modules)

    for (i = 0; i < MAX_CIS_SIZE; i++)
    {
        //pDest[i]=pAddr[2*i];
        // select attribute memory read, low byte
        _MDrv_CI_PCMCIA_WriteReg(REG_PCM_MEM_IO_CMD, ATTRIBMEMORY_READ);

        _MDrv_CI_PCMCIA_GetSemaphore();
        {
            // read address
            _MDrv_CI_PCMCIA_WriteReg(REG_ADDR1, ((Addr + 2 * i) >> 8));
            _MDrv_CI_PCMCIA_WriteReg(REG_ADDR0, (Addr + 2 * i));

            // fire command
            _MDrv_CI_PCMCIA_WriteReg(REG_FIRE_READ_DATA_CLEAR, 1);
        }
        _MDrv_CI_PCMCIA_ReleaseSemaphore();

        //polling if fire is done
        for (j = 0; j < 20000; j++)
        {
            _MDrv_CI_PCMCIA_ReadReg(REG_FIRE_READ_DATA_CLEAR, &u8value);

            if (!(u8value & 0x01))
            {
                break;
            }
            msleep(1);
        }

        //polling if data ready
        for (j = 0; j < 20000; j++)
        {
            _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA_DONE_BUS_IDLE, &u8value);

            if (u8value & 0x01)
            {
                _MDrv_CI_PCMCIA_GetSemaphore();
                {
                    _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA, pDest + i);
                }
                _MDrv_CI_PCMCIA_ReleaseSemaphore();
                break;
            }
            msleep(1);
        }

        // clean stat_rd done
        _MDrv_CI_PCMCIA_WriteReg(REG_FIRE_READ_DATA_CLEAR, 2);

        // polling if bus is idle
        for (j = 0; j < 20000; j++)
        {
            _MDrv_CI_PCMCIA_ReadReg(REG_READ_DATA_DONE_BUS_IDLE, &u8value);

            if ((u8value & 0x03) == 0x0002)
            {
                break;
            }
            msleep(1);
        }

        //printk("Read Type %bx, Addr %x, value %bx\n", u8AccessType, Addr, u8mem);
    }
#if DRV_CI_PCMCIA_DEBUG_ENABLE
    /* Dump CIS. */
    DRV_CI_PCMCIA_DEBUG("PCMCIA CIS:\r\n");
    for (i = 0; i < MAX_CIS_SIZE; i++)
    {
        DRV_CI_PCMCIA_DEBUG("%02X ", *( pDest + i ));
        if ( 0 == ( ( i + 1 ) & 0x0007 ) )
            DRV_CI_PCMCIA_DEBUG("\r\n");
    }
    DRV_CI_PCMCIA_DEBUG("\r\n");
#endif
}

// -------------------------------------------------------------
B16 MDrv_CI_PCMCIA_ParseAttribMem(const U8 *pAttribMem, U16 dwLen, PCMCIA_INFO_t *pInfo)
{
    U8 bTag;
    U8 bTagLen;
    S32 i;//,len;
    U8 FieldSize;
    U8 LastIndex;
    U8 MaskByte;
    U8 SubTagByte;
    U8 SubTagLen;
    U16 STCI_IFN;
    char CiDetStr[20];
    B16 fDefault;
    B16 fInterface;
    U8 bFeature;
#if 0
    B16 bSTCI_STR = FALSE;
#endif
    memset(pInfo, 0x00, sizeof(PCMCIA_INFO_t));
    do
    {
        bTag = pAttribMem[0];
        bTagLen = pAttribMem[1];
        if ((U16)bTagLen + 2 > dwLen)
        {
            return FALSE;
        }

        dwLen -= (bTagLen + 2);
        //dwLen = dwLen-    (U16)bTagLen-2;
        DRV_CI_PCMCIA_DEBUG("Parse_PCMCIA_AttribMem:bTag=0x%x dwLen=%d=============\n", bTag, dwLen);
        for (i = 0; i < bTagLen; i++)
        {
            DRV_CI_PCMCIA_DEBUG("%02X ", pAttribMem[2 + i]);
        }
        DRV_CI_PCMCIA_DEBUG("\n");
        DRV_CI_PCMCIA_DEBUG("=============\n");
        switch (bTag)
        {
            case 0x00:
                DRV_CI_PCMCIA_DEBUG("Parse_PCMCIA_AttribMem case 0x00\n");
                break;//_asm{ int 3  };

            case 0x01:
                DRV_CI_PCMCIA_DEBUG("CISTPL_DEVICE\r\n");
                // Memory type
                // Access speed
                // Memory Size
                break;
            case 0x13:
                DRV_CI_PCMCIA_DEBUG("CISTPL_LINKTARGET\r\n");
                // "CIS"
                break;
            case 0x15:
                /* en50221 P.76: CISTPL_VERS_1: TPLLV1_MAJOR = 05h; TPLLV1_MINOR = 00h; */
                if (TPLLV1_MAJOR_VAL != pAttribMem[2] || TPLLV1_MINOR_VAL != pAttribMem[3])
                    return FALSE;

                #if STORE_CISVERS1_INFO
                pInfo->wPCMCIAStdRev = (U16)pAttribMem[2] << 8 | (U16)pAttribMem[3];
                DRV_CI_PCMCIA_DEBUG("CISTPL_VERS_1: pInfo->wPCMCIAStdRev = %04X\n", pInfo->wPCMCIAStdRev);
                pInfo->dwValidFlags |= PCMCIAINFO_VERS1_VALID;
                i = 4;
                if (bTagLen < 2)
                {
                    break;
                } // error
                memcpy( pInfo->pszManufacturerName, pAttribMem + i, min( bTagLen + 2 - i, MAX_PCMCIA_STRLEN - 1 ) );
                pInfo->pszManufacturerName[min( bTagLen + 2 - i, MAX_PCMCIA_STRLEN - 1 )] = 0;
                i += strlen( ( char* )pAttribMem + i ) + 1;
                if ( i < bTagLen + 2 )
                {
                    memcpy( pInfo->pszProductName, pAttribMem + i, min( bTagLen + 2 - i, MAX_PCMCIA_STRLEN - 1 ) );
                    pInfo->pszProductName[min( bTagLen + 2 - i, MAX_PCMCIA_STRLEN - 1 )] = 0;
                    i += strlen( ( char* )pAttribMem + i ) + 1;                   
                }
                if ( i < bTagLen + 2 )
                {
                    memcpy( pInfo->pszProductInfo1, pAttribMem + i, min( bTagLen + 2 - i, MAX_PCMCIA_STRLEN - 1 ) );
                    pInfo->pszProductInfo1[min( bTagLen + 2 - i, MAX_PCMCIA_STRLEN - 1 )] = 0;
                    i += strlen( ( char* )pAttribMem + i ) + 1;
                }
                if ( i < bTagLen + 2 )
                {
                    memcpy( pInfo->pszProductInfo2, pAttribMem + i, min( bTagLen + 2 - i, MAX_PCMCIA_STRLEN - 1 ) );
                    pInfo->pszProductInfo2[min( bTagLen + 2 - i, MAX_PCMCIA_STRLEN - 1 )] = 0;
                }

                    /* $compatible[ciplus=1]$  */
                {
                    U8 u8aCI_PLUS_STRING[] = "$compatible[ciplus=1]$";
                    U8 u8aCI_PLUS_STRING2[] = "$COMPATIBLE[CIPLUS=1]$";
                    if ( 0 == memcmp( pInfo->pszProductInfo1, u8aCI_PLUS_STRING, sizeof( u8aCI_PLUS_STRING ) ) )
                    {
                        pInfo->bCI_PLUS = 1;
                    }
                    else if( 0 == memcmp( pInfo->pszProductInfo1, u8aCI_PLUS_STRING2, sizeof( u8aCI_PLUS_STRING2 ) ) )
                    {
                        pInfo->bCI_PLUS = 1;                                         
                    }
                    else
                    {
                        pInfo->bCI_PLUS = 0;                                         
                    }
                } 
                    
                //DRV_CI_PCMCIA_DEBUG(("CISTPL_VERS1, SpecVer %04X\r\nManufacturer:%s\r\nProduct: %s,%s %s\r\n",pInfo->wPCMCIAStdRev,pInfo->pszManufacturerName,pInfo->pszProductName,pInfo->pszProductInfo1,pInfo->pszProductInfo2 ));
                break;
                #endif
            case 0x17:
                //DRV_CI_PCMCIA_DEBUG(("CISTPL_DEVICE_A\r\n"));
                // Like 0x01... for device(s) in attribute memory
                // Memory type
                // Access speed
                // Memory Size
                break;
            case 0x1a:
                DRV_CI_PCMCIA_DEBUG("CISTPL_CONFIG\r\n");
                /*
                {
                    U8 bNumAddrBytes;
                    U8 bNumConfigRegs;
                    U8 bLastIndex;
                    bNumAddrBytes = (pAttribMem[2]&0x03)+1;
                    bNumConfigRegs = ((pAttribMem[2]>>2)&0x0F)+1;
                    bLastIndex = pAttribMem[3]&0x3F;
                    for(i=0;i<bNumAddrBytes;i++)
                    {
                        pInfo->ConfigOffset = pInfo->ConfigOffset<<8;
                        pInfo->ConfigOffset |= pAttribMem[4+i];
                        DRV_CI_PCMCIA_DEBUG(("ConfigOffset %04X\r\n",pInfo->ConfigOffset));
                    }
                    i++;
                }
                */
                FieldSize = pAttribMem[2];
                LastIndex = pAttribMem[3];
                if (FieldSize == 0)
                {
                    pInfo->ConfigOffset = pAttribMem[4];
                }
                else if (FieldSize == 1)
                {
                    pInfo->ConfigOffset = (U32)pAttribMem[5] << 8 | (U32)pAttribMem[4];
                }
                DRV_CI_PCMCIA_DEBUG("ConfigOffset 0x%x\r\n", pInfo->ConfigOffset);
                MaskByte = pAttribMem[5 + FieldSize];
                SubTagByte = pAttribMem[6 + FieldSize];
                SubTagLen = pAttribMem[7 + FieldSize];
                STCI_IFN = (U16)pAttribMem[9 + FieldSize] << 8 | (U16)pAttribMem[8 + FieldSize];
                memcpy(CiDetStr, pAttribMem + 10 + FieldSize, min((U32)(SubTagLen - 2), sizeof(CiDetStr) - 1));
                CiDetStr[min((U32)(SubTagLen - 2), sizeof( CiDetStr ) - 1 )] = 0;
#if 0
                if (0 == strncmp(CiDetStr, STCI_STR, strlen(STCI_STR)))
                    bSTCI_STR = TRUE;
#else
                //wgkwak in LGE
                if((STCI_IFN == 0x0241)&&(!strncmp(CiDetStr,"DVB_CI_V1.00",strlen("DVB_CI_V1.00"))))    pInfo->bCAMIsInserted = 1;
#endif
                //DRV_CI_PCMCIA_DEBUG("FieldSize %d, LastIndex %d MaskByte %02X SubTag %02X ID %02X %s\r\n", FieldSize, LastIndex, MaskByte, SubTagByte, STCI_IFN, CiDetStr);
                break;
            case 0x1b:
                DRV_CI_PCMCIA_DEBUG("CISTPL_CFTABLE_ENTRY\r\n");
                {
                    U8 bIF;
                    U8 bFlags;
                    int j;
                    pInfo->Config[pInfo->bNumConfigs].bConfigIndex = pAttribMem[2] & 0x3F;
                    DRV_CI_PCMCIA_DEBUG("Config Entry value 0x%X\n",pInfo->Config[pInfo->bNumConfigs].bConfigIndex);
                    if (pAttribMem[2] & 0x40)
                    {
                        ;
                    } // Default
                    if (pAttribMem[2] & 0x80)
                    {
                        bIF = pAttribMem[3];
                        i = 4;
                    }
                    else
                    {
                        i = 3;
                    }
                    bFlags = pAttribMem[i];
                    i++;
                    if (bFlags & (BIT0|BIT1))
                    {
                        U8 bPowerBits = pAttribMem[i];
                        //DRV_CI_PCMCIA_DEBUG(("PowerDesc %02X\r\n", bPowerBits));
                        i++;
                        for (j = 0; j < 7; j++)
                        {
                            if ((bPowerBits >> j) & 0x01)
                            {
                                i++;
                            }
                            while (pAttribMem[i] & 0x80)
                            {
                                i++;
                            } // extension byte
                        }
                    }
                    if (bFlags & BIT2)
                    {
                        //DRV_CI_PCMCIA_DEBUG(("TimingDesc %02X\r\n", pAttribMem[i]));
                        i++;
                    }
                    if (bFlags & BIT3)//BIT3 IO
                    {
                        if (pAttribMem[i] & 0x80)
                        {
                            U8 bAddrBytes;
                            U8 bLenBytes;
                            U8 bSize[4] =
                            {
                                0, 1, 2, 4
                            };
                            U8 bNumDesc;
                            U32 dwEALen = 0;
                            U32 dwEAAddr = 0;
                            bNumDesc = pAttribMem[i + 1] & 0x0F;
                            bAddrBytes = bSize[( pAttribMem[i + 1] >> 4 ) & 0x03];
                            bLenBytes = bSize[( pAttribMem[i + 1] >> 6 ) & 0x03];
                            //DRV_CI_PCMCIA_DEBUG(("EADesc %02X %d %d %d\r\n", pAttribMem[i + 1], bNumDesc, bAddrBytes, bLenBytes));
                            i += 2;
                            switch (bAddrBytes)
                            {
                                case 1:
                                    dwEAAddr = pAttribMem[i];
                                    break;
                                case 2:
                                    dwEAAddr = (U32)pAttribMem[i] | (U32)pAttribMem[i + 1] << 8;
                                    break;
                                case 4:
                                    dwEAAddr = (U32)pAttribMem[i] | (U32)pAttribMem[i + 1] << 8 | (U32)pAttribMem[i + 2] << 16 | (U32)pAttribMem[i + 3] << 24;
                                    break;
                            }
                            pInfo->Config[pInfo->bNumConfigs].dwEAAddr = dwEAAddr;
                            i += bLenBytes;
                            switch (bLenBytes)
                            {
                                case 1:
                                    dwEALen = pAttribMem[i];
                                    break;
                                case 2:
                                    dwEALen = (U32)pAttribMem[i] | (U32)pAttribMem[i + 1] << 8;
                                    break;
                                case 4:
                                    dwEALen = (U32)pAttribMem[i] | (U32)pAttribMem[i + 1] << 8 | (U32)pAttribMem[i + 2] << 16 | (U32)pAttribMem[i + 3] << 24;
                                    break;
                            }
                            pInfo->Config[pInfo->bNumConfigs].dwEALen = dwEALen;
                            i += bAddrBytes;
                            //DRV_CI_PCMCIA_DEBUG(("Addr %04X Len %04X", dwEAAddr, dwEALen));
                        }
                        else
                        {
                            i++;
                        }
                    }
                    if (bFlags & BIT4)//BIT4 IRQ
                    {
                        DRV_CI_PCMCIA_DEBUG("IrqDesc\r\n");
                        pInfo->Config[pInfo->bNumConfigs].bIRQDesc1 = pAttribMem[i];
                        if ((pAttribMem[i] & BIT5) && !(pAttribMem[i] & (~BIT5)))
                        {
                            pInfo->bINT = ENABLE;  
                        }

                        if (pAttribMem[i] & BIT4)
                        {
                            pInfo->Config[pInfo->bNumConfigs].wIRQData = (U16)pAttribMem[i] << 8 | (U16)pAttribMem[i + 1];
                            i += 2;
                        }
                        i++;
                    }
                    if (bFlags & 0x60)
                    {
                        DRV_CI_PCMCIA_DEBUG("MemoryDesc\r\n");
                        i++;
                    }
                    if (bFlags & 0x80)
                    {
                        DRV_CI_PCMCIA_DEBUG("MixedDesc\r\n");
                        i++;
                    }


                    while (i < (bTagLen + 2))
                    {
                        DRV_CI_PCMCIA_DEBUG("SubTag 0x%02X %d %d\n", pAttribMem[i], i, bTagLen);
                        if (pAttribMem[i] == 0xc0)
                        {
                            if (strcmp((char *)pAttribMem + i + 2, "DVB_HOST") == 0)
                            {
                                pInfo->Config[pInfo->bNumConfigs].fCITagsPresent |= 0x01;
                            }
                            DRV_CI_PCMCIA_DEBUG("%s\r\n", pAttribMem + i + 2);
                        }
                        if (pAttribMem[i] == 0xc1)
                        {
                            if (strcmp((char *)pAttribMem + i + 2, "DVB_CI_MODULE") == 0)
                            {
                                pInfo->Config[pInfo->bNumConfigs].fCITagsPresent |= 0x02;
                            }
                            DRV_CI_PCMCIA_DEBUG("%s\r\n", pAttribMem + i + 2);
                        }
                        i += pAttribMem[i + 1] + 2;
                    }

                    pInfo->bNumConfigs++;
                }
                fDefault = pAttribMem[2] >> 7;
                fInterface = (pAttribMem[2] >> 6) & 0x01;
                //DRV_CI_PCMCIA_DEBUG("(default)");
                if (fInterface)
                {
                    //DRV_CI_PCMCIA_DEBUG("IF %02X ", pAttribMem[3]);
                }
                bFeature = pAttribMem[3 + fInterface];
                #if 0
                DRV_CI_PCMCIA_DEBUG("\r\n");

                for (i = 0; i < bTagLen; i++)
                {
                    DRV_CI_PCMCIA_DEBUG("%02X ", pAttribMem[2 + i]);
                }
                DRV_CI_PCMCIA_DEBUG("\r\n");
                for (i = 0; i < bTagLen; i++)
                {
                    DRV_CI_PCMCIA_DEBUG("%c  ", pAttribMem[2 + i]);
                }
                DRV_CI_PCMCIA_DEBUG("\r\n");
                #endif
                break;
            case 0x1c:
                DRV_CI_PCMCIA_DEBUG("CISTPL_DEVICE_OC\r\n");
                break;
            case 0x1D:
                DRV_CI_PCMCIA_DEBUG("CISTPL_DEVICE_OA\r\n");
                break;
            case 0x20:
                DRV_CI_PCMCIA_DEBUG("CISTPL_MANFID\r\n");
                pInfo->dwValidFlags |= PCMCIAINFO_MANID_VALID;
                pInfo->wManufacturerId = (U16)pAttribMem[2] << 8 | (U16)pAttribMem[3];
                pInfo->wCardID = (U16)pAttribMem[4] << 8 | (U16)pAttribMem[5];
                DRV_CI_PCMCIA_DEBUG("Manufacturer code %04X Product Code %04X\r\n", pInfo->wManufacturerId, pInfo->wCardID);
                break;
            case 0x21:
                DRV_CI_PCMCIA_DEBUG("CISTPL_FUNCID\r\n");
                pInfo->dwValidFlags |= PCMCIAINFO_FUNCID_VALID;
                pInfo->FuncType = ( PCMCIA_FUNCTYPE )pAttribMem[2];
                pInfo->bFuncIDSysInfo = pAttribMem[3];
                break;
            case 0x40:
                // CISTPL_VERS2
                DRV_CI_PCMCIA_DEBUG("Parse_PCMCIA_AttribMem case 0x40\n");
                break;
            default:
                #if 0
                DRV_CI_PCMCIA_DEBUG("Parse_PCMCIA_AttribMem case else\n");
                DRV_CI_PCMCIA_DEBUG("Tag %02X, Len %d\r\n", bTag, bTagLen);
                for (i = 0; i < bTagLen; i++)
                {
                    DRV_CI_PCMCIA_DEBUG("%02X ", pAttribMem[2 + i]);
                }
                DRV_CI_PCMCIA_DEBUG("\r\n");
                for (i = 0; i < bTagLen; i++)
                {
                    DRV_CI_PCMCIA_DEBUG("%c", pAttribMem[2 + i]);
                }
                DRV_CI_PCMCIA_DEBUG("\r\n");
                #endif
                break;
        }
        pAttribMem += (2 + (U16)bTagLen);
    }
    while (bTag != 0x14 && bTag != 0xFF && dwLen);
#if 0
    if (bSTCI_STR)
        return TRUE;
    else
        return FALSE;
#else
    return TRUE;
#endif
}

B16 MDrv_CI_PCMCIA_ResetInterface(void)
{
    U8 ret;
    
    msleep(1000);
    gu8CI_Command |= CICOMMAND_RESET;                                               // Up RS
    MDrv_CI_PCMCIA_WriteIOMem(_hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command);
    ret = MDrv_CI_L2_WaitForCIBit(_hPCMCIA, CISTATUS_FREE);
    gu8CI_Command &= ( ~CICOMMAND_RESET );                                          // DOWN RS
    MDrv_CI_PCMCIA_WriteIOMem(_hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command);
    
    return ret;
}

void MDrv_CI_PCMCIA_SwitchBypassMode(B16 Mode)
{
    if (Mode)
        _gbBypassModeCI = TRUE;
    else
        _gbBypassModeCI = FALSE;
}

static int _MDrv_PCMCIA_KThread(void *data)
{
    while (1)
    {
        MDrv_CI_PCMCIA_ISR();
        msleep(PCMCIA_SLEEP_INTERVAL);
    }

    return 0;
}

void _MDrv_PCMCIA_InitVariables(PCMCIA_INFO_t *_PCMCIA_Info)
{
    volatile U8 *pu8Reg;

    /* Power */
    // 0x2508[15:0] = 0 (clear power down)
    pu8Reg = (U8 *)(REG_MIPS_BASE + (0x2508 * 2));
    *pu8Reg = 0x00;
    *(++pu8Reg) = 0x00;
    // 0x250C[15:0] = 0 (clear power down clocks)
    pu8Reg = (U8 *)(REG_MIPS_BASE + (0x250C * 2));
    *pu8Reg = 0x00;
    *(++pu8Reg) = 0x00;
#if 0
    /* MPLL power on */
    // 0x2514[7:0] = 0x24
    pu8Reg = (U8 *)(REG_MIPS_BASE + (0x2514 * 2));
    *pu8Reg = 0x24;
    // 0x2510[15:0] = 0x0605
    pu8Reg = (U8 *)(REG_MIPS_BASE + (0x2510 * 2));
    *pu8Reg = 0x05;
    *(++pu8Reg) = 0x06;
    // 0x2516[15:0] = 0 (MPLL div clocks enable)
    pu8Reg = (U8 *)(REG_MIPS_BASE + (0x2516 * 2));
    *pu8Reg = 0x00;
    *(++pu8Reg) = 0x00;
    // 0x25C0[15:0] = 0x0
    pu8Reg = (U8 *)(REG_MIPS_BASE + (0x25C0 * 2));
    *pu8Reg = 0x00;
    *(++pu8Reg) = 0x00;
#endif
    /* System Board Initialization */
    // 0x1E31[3:0] = 0 (reg_ckg_pcm: 27Mhz)
    pu8Reg = (U8 *)(REG_MIPS_BASE + ((0x1E31 - 1) * 2 + 1));
    *pu8Reg &= 0xF0;
#ifndef    S5_LG_DEMO_BD_2
//    2008,05,08: changed for NEW MSTAR BOARD(no FE)    from dean only for S5_LG_DEMO_BOARD_2
    // 0x1EA1[7] = 0 (reg_allpad_in)
    pu8Reg = (U8 *)(REG_MIPS_BASE + ((0x1EA1 - 1) * 2 + 1));
    *pu8Reg &= (~BIT7);
#endif
    // 0x1EDC: depends on package (MSD3102EQ: 0กั55, MSD3108EV: 0xB5)
    pu8Reg = (U8 *)(REG_MIPS_BASE + (0x1EDC * 2));
    *pu8Reg = 0xB5;

    // When checking pcmcia functions, check 0x1EE0[5:0] = 0 and 0x1EE5[6:1] = 0
    pu8Reg = (U8 *)(REG_MIPS_BASE + (0x1EE0 * 2));
    *pu8Reg &= 0xC0;
    pu8Reg = (U8 *)(REG_MIPS_BASE + ((0x1EE5 - 1) * 2 + 1));
    *pu8Reg &= 0x81;

    _pPCMCIA_Info = _PCMCIA_Info;
    _hPCMCIA = (PCMCIA_HANDLE)_PCMCIA_Info;

    _gbCardAInsideCI = FALSE;
    _gbBypassModeCI = FALSE;
}

void MDrv_PCMCIA_Init(PCMCIA_INFO_t *_PCMCIA_Info)
{
    _MDrv_PCMCIA_InitVariables(_PCMCIA_Info);
    kernel_thread(_MDrv_PCMCIA_KThread, _pPCMCIA_Info, CLONE_KERNEL);
}

#if PCMCIA_IRQ_ENABLE
void MDrv_PCMCIA_Enable_Interrupt( BOOL bEnable )
{
    volatile U8 u8Reg;

    MDrv_PCMCIA_Set_InterruptStatus( FALSE );
    
    if ( ENABLE == bEnable )
    {
        /* Unmask bit2 for IRQ from CICAM. */
        _MDrv_CI_PCMCIA_ReadReg( REG_INT_MASK_CLEAR, (U8 *)&u8Reg );
        u8Reg &= ( ~BIT2 );
        _MDrv_CI_PCMCIA_WriteReg( REG_INT_MASK_CLEAR, u8Reg );
    }
    else
    {
        /* Mask bit2 for IRQ from CICAM. */
        _MDrv_CI_PCMCIA_ReadReg( REG_INT_MASK_CLEAR, (U8 *)&u8Reg );
        u8Reg |= BIT2;
        _MDrv_CI_PCMCIA_WriteReg( REG_INT_MASK_CLEAR, u8Reg );
    }
}

void MDrv_PCMCIA_Clear_Interrupt( void )
{
    volatile U8 u8Reg;

    _MDrv_CI_PCMCIA_ReadReg(REG_INT_MASK_CLEAR1, (U8 *)&u8Reg);
    u8Reg |= BIT1;
    _MDrv_CI_PCMCIA_WriteReg(REG_INT_MASK_CLEAR1, u8Reg);
    u8Reg &= ( ~BIT1 );
    _MDrv_CI_PCMCIA_WriteReg(REG_INT_MASK_CLEAR1, u8Reg);
}

void MDrv_PCMCIA_Set_InterruptStatus( BOOL Status )
{
    _gbPCMCIA_IrqStatus = Status;
}

BOOL MDrv_PCMCIA_Get_InterruptStatus( void )
{
    return _gbPCMCIA_IrqStatus;
}
#endif

#undef  _DRVCI_PCMCIA_C_
