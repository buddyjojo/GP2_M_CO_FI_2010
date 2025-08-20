////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (!¡±MStar Confidential Information!¡L) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file   drvSerFlash.c
/// @brief  Serial Flash Driver Interface (MXIC MX25L3205, MX25L6405)
///                                       (EON EN25B16, EN25B64)
///                                       (SPANSION S25FL064A)
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drvSerFlash.h"            // SERFLASH_TYPE
#include "regSerFlash.h"
#include <../board/GP2_DEMO1/regCHIP.h>
#include <common.h>

// !!! Uranus Serial Flash Notes: !!!
//  - The clock of DMA & Read via XIU operations must be < 3*CPU clock
//  - The clock of DMA & Read via XIU operations are determined by only REG_ISP_CLK_SRC; other operations by REG_ISP_CLK_SRC only
//  - DMA program can't run on DRAM, but in flash ONLY
//  - DMA from SPI to DRAM => size/DRAM start/DRAM end must be 8-B aligned

#define SERFLASH_TYPE_NONE          0
//#define SERFLASH_TYPE_PM25LV040     1
//#define SERFLASH_TYPE_SST25LF040    2
//#define SERFLASH_TYPE_ST25P32       3
//#define SERFLASH_TYPE_MX25L1605     4
#define SERFLASH_TYPE_MX25L3205     5                                   // MXIC
#define SERFLASH_TYPE_MX25L6405     6                                   // MXIC
#define SERFLASH_TYPE_S25FL064A     10                                  // SPANSION
#define SERFLASH_TYPE_EN25B16       20
#define SERFLASH_TYPE_EN25B64       21


#define SERFLASH_TYPE SERFLASH_TYPE_MX25L3205
//#define SERFLASH_TYPE SERFLASH_TYPE_MX25L6405
//#define SERFLASH_TYPE SERFLASH_TYPE_EN25B16
//#define SERFLASH_TYPE SERFLASH_TYPE_EN25B64
//#define SERFLASH_TYPE SERFLASH_TYPE_S25FL064A


//#ifndef SERFLASH_TYPE
//#error "SERFLASH_TYPE not defined!"
//#endif

//#if (SERFLASH_TYPE == SERFLASH_TYPE_MX25L3205) || (SERFLASH_TYPE == SERFLASH_TYPE_MX25L6405)
//  ||(SERFLASH_TYPE == SERFLASH_TYPE_EN25B16  ) || (SERFLASH_TYPE == SERFLASH_TYPE_EN25B64) //2008/07/19 Nick
//  ||(SERFLASH_TYPE == SERFLASH_TYPE_S25FL064A)
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#if   (SERFLASH_TYPE == SERFLASH_TYPE_MX25L3205)
    #define NUMBER_OF_SERFLASH_SECTORS  (1024)
    #define SERFLASH_SECTOR_SIZE        (4*1024)
#elif (SERFLASH_TYPE == SERFLASH_TYPE_MX25L6405)
    #define NUMBER_OF_SERFLASH_SECTORS  (2024)
    #define SERFLASH_SECTOR_SIZE        (4*1024)
#endif

#if (SERFLASH_TYPE == SERFLASH_TYPE_EN25B16)
    #define NUMBER_OF_SERFLASH_SECTORS  (32)
    #define SERFLASH_SECTOR_SIZE        (64*1024)
#elif ((SERFLASH_TYPE == SERFLASH_TYPE_EN25B64)||(SERFLASH_TYPE == SERFLASH_TYPE_S25FL064A))
    #define NUMBER_OF_SERFLASH_SECTORS  128 //(32 * 4)
    #define SERFLASH_SECTOR_SIZE        (64*1024)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
//serial flash mutex wait time
#define SERFLASH_MUTEX_WAIT_TIME    	3000
#define SERFLASH_SAFETY_FACTOR      	10

#define SER_FLASH_TIME(_stamp, _msec)  	{ volatile int i; for (i=0; i<100; i++); _stamp =0xFFFFFFF;}
#define SER_FLASH_EXPIRE(_stamp)       	( !(--_stamp) )
#define SER_FLASH_INIT()               	{ }
#define SER_FLASH_ENTRY()              	{ }
#define SER_FLASH_RETURN()             	{ }
#define SER_FLASH_YIELD_TASK()         	{ }

#define LOU16(u32Val)  					( (MS_U16)(u32Val) )
#define HIU16(u32Val)  					( (MS_U16)((u32Val) >> 16) )

#define SPI_READ_ADDRESS 				(0x87000000)

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
MS_S32 _s32SerFlash_Mutex;

static SerFlashInfo _SerFlashInfo =
{
    1,
    (NUMBER_OF_SERFLASH_SECTORS*SERFLASH_SECTOR_SIZE),
    NUMBER_OF_SERFLASH_SECTORS,
    SERFLASH_SECTOR_SIZE
};


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
#ifdef MS_DEBUG
#define DEBUG_SER_FLASH(x)          (x)
#else
#define DEBUG_SER_FLASH(x)          //(x)
#endif

////2008/07/19 Nick Modify start
//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// Get system information
/// @return Chip Revision ID
//-------------------------------------------------------------------------------------------------
MS_U8 MDrv_SYS_GetChipRev(void)
{
    MS_U8 u8Revision;
    u8Revision = (TOP_REG(REG_TOP_CHIP_VERSION) & CHIP_REVISION_MASK) >> CHIP_REVISION_SHFT;
    return u8Revision;
}

//-------------------------------------------------------------------------------------------------
// Wait for SPI Write Cmd Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_SerFlash_WaitWriteCmdRdy(void)
{
#if 1
    BOOL bRet = FALSE;
    MS_U32 u32Timer;

    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*30)//2008/07/19 Nick mark
    do {
        if ( (ISP_REG16(REG_ISP_SPI_WR_CMDRDY) & 0x1) == 0x1 )
        {
            bRet = TRUE;
            break;
        }

    } while (!SER_FLASH_EXPIRE(u32Timer));

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(printf("Wait for SPI Write Cmd Ready fails!\n"));
    }

    return bRet;
#else
    BOOL bRet = FALSE;

    do
    {
        if ((ISP_REG16(REG_ISP_SPI_WR_CMDRDY) & 0x0001) == 0x0001)
        {
            bRet = TRUE;
            break;
        }
    } while (1); // REVIEW: add timeout <-???

    return bRet;
#endif

}

//-------------------------------------------------------------------------------------------------
// Wait for SPI Write Data Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_SerFlash_WaitWriteDataRdy(void)
{
#if 0
    BOOL bRet = FALSE;
    MS_U32 u32Timer;

    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*30) //2008/07/19 Nick mark
    do {
        if ( (ISP_REG16(REG_ISP_SPI_WR_DATARDY) & 0x1) == 0x1 )
        {
            bRet = TRUE;
            break;
        }
    } while (!SER_FLASH_EXPIRE(u32Timer));

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(printf("Wait for SPI Write Data Ready fails!\n"));
    }

    return bRet;
#else
    BOOL bRet = FALSE;
    do
    {
        if ((ISP_REG16(REG_ISP_SPI_WR_DATARDY) & 0x0001) == 0x0001)
        {
            bRet = TRUE;
            break;
        }
    } while (1); // REVIEW: add timeout <-???

    return bRet;
#endif
}

//-------------------------------------------------------------------------------------------------
// Wait for SPI Read Data Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_SerFlash_WaitReadDataRdy(void)
{
#if 0
    BOOL bRet = FALSE;
    MS_U32 u32Timer;

    SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*30) //2008/07/19 Nick mark
    do {
        if ( (ISP_REG16(REG_ISP_SPI_RD_DATARDY) & 0x1) == 0x1 )
        {
            bRet = TRUE;
            break;
        }
    } while (!SER_FLASH_EXPIRE(u32Timer));

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(printf("Wait for SPI Read Data Ready fails!\n"));
    }

    return bRet;
#else
    BOOL bRet = FALSE;

    do
    {
        if ((ISP_REG16(REG_ISP_SPI_RD_DATARDY) & 0x0001) == 0x0001)
        {
            bRet = TRUE;
            break;
        }
    } while (1); // REVIEW: add timeout <-???

    return bRet;
#endif

}

//-------------------------------------------------------------------------------------------------
// Wait for Write/Erase to be done
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_SerFlash_WaitWriteDone(void)
{
#if 0
    BOOL bRet = FALSE;
    MS_U32 u32Timer;

    //2008/07/19 Nick add start
    #if(SERFLASH_TYPE == SERFLASH_TYPE_MX25L3205) || (SERFLASH_TYPE == SERFLASH_TYPE_MX25L6405)
       SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*50*1000)//2008/07/19 Nick mark

    #elif((SERFLASH_TYPE == SERFLASH_TYPE_EN25B16  ) || (SERFLASH_TYPE == SERFLASH_TYPE_EN25B64)
        ||(SERFLASH_TYPE == SERFLASH_TYPE_S25FL064A))
       SER_FLASH_TIME(u32Timer, SERFLASH_SAFETY_FACTOR*384*1000)   //max (chip erase = 384s, sector erase = 3s, page program = 3ms)
    #endif
    //2008/07/19 Nick add end
    do {
        if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
        {
            printf("Wait Write cmd ready fail!\n"); //2008/07/22 Nick
            break;
        }
        ISP_REG16(REG_ISP_SPI_COMMAND) = 0x05; //RDSR

        ISP_REG16(REG_ISP_SPI_RDREQ) = 0x01; //SPI read request
        if ( _MDrv_SerFlash_WaitReadDataRdy() == FALSE )
        {
            break;
        }

        if ( (ISP_REG16(REG_ISP_SPI_RDATA) & 0x1) == 0 ) //WIP =0 write done
        {
            bRet = TRUE;
            break;
        }

        ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis
        SER_FLASH_YIELD_TASK();
    } while (!SER_FLASH_EXPIRE(u32Timer));

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(printf("Wait for Write to be done fails!\n"));
    }

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis

    return bRet;
#else
    BOOL bRet = FALSE;

    if (_MDrv_SerFlash_WaitWriteCmdRdy() == FALSE)
    {
        return FALSE;
    }

    do
    {
        ISP_REG16(REG_ISP_SPI_COMMAND) = CMD_READ_STATUS_REG;   // RDSR

        ISP_REG16(REG_ISP_SPI_RDREQ) = 0x01;                    // SPI read request

        if (_MDrv_SerFlash_WaitReadDataRdy() == FALSE)
        {
            break;
        }

        if (0 == (ISP_REG16(REG_ISP_SPI_RDATA) & 0x01))       // WIP == 0, write done
        {
            bRet = TRUE;
            break;
        }
    } while (1); // REVIEW: add timeout <-???

    if (bRet == FALSE)
    {
        printf("ERROR: wait for write to be done fail !!!\n");
    }

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; // SPI CEB dis

    return bRet;
#endif
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/// Get system information
/// @return Chip Revision ID
//-------------------------------------------------------------------------------------------------
MS_U8 MDrv_SerFlash_GetChipRev(void)
{
    MS_U8 u8Revision;
    u8Revision = (TOP_REG(REG_TOP_CHIP_VERSION) & CHIP_REVISION_MASK) >> CHIP_REVISION_SHFT;
    return u8Revision;
}

//-------------------------------------------------------------------------------------------------
/// Get the information of Serial Flash
/// @param  pFlashInfo \b OUT: the buffer to store flash information
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_SerFlash_GetInfo(SerFlashInfo *pFlashInfo)
{
    memcpy(pFlashInfo, &_SerFlashInfo, sizeof(SerFlashInfo));
}
//-------------------------------------------------------------------------------------------------
/// Initialize Serial Flash
/// @return None
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
void MDrv_SerFlash_Init(void)
{
	printf("  spi init... \n");
    //Create Serial Flash Mutex
    //_s32SerFlash_Mutex = MsOS_CreateMutex(E_MSOS_FIFO, "Mutex SerFlash");
    // SER_FLASH_INIT();
    //Note: Adjust clock source & freq according to the specific flash spec
    //1. The clock of DMA & Read via XIU
    PIU_REG16(REG_PIU_SPI_CLK_SRC) &= ~(1<<5); //Xtal

    PIU_REG16(REG_PIU_SPI_CLK_SRC)  = (PIU_REG16(REG_PIU_SPI_CLK_SRC) & ~0x1F) | 0x08; //36 Mhz
    PIU_REG16(REG_PIU_SPI_CLK_SRC) |= (1<<5); //clk_sel

    //2. The clock of other operations
    ISP_REG16(REG_ISP_SPI_CLKDIV) = (1<<6); //cpu clock / div8
    ISP_REG16(REG_ISP_CHIP_SEL) = SFSH_CHIP_SEL_XIU | 0x01; // chip_sel, [0]:FLASH0, [1]:FLASH1
    #if(SERFLASH_TYPE == SERFLASH_TYPE_MX25L3205) || (SERFLASH_TYPE == SERFLASH_TYPE_MX25L6405)
        ISP_REG16(REG_ISP_DEV_SEL) = 0;   // PMC flash: MX flash is similar to PMC flash, except RDID command
    #elif((SERFLASH_TYPE == SERFLASH_TYPE_EN25B16  ) || (SERFLASH_TYPE == SERFLASH_TYPE_EN25B64)
       ||(SERFLASH_TYPE == SERFLASH_TYPE_S25FL064A))
        ISP_REG16(REG_ISP_DEV_SEL) = 2;   //ST flash: SPANSION flash is the same as ST flash
    #endif
    *((volatile MS_U32*)(0xBF201658)) = 0x00;

    //ISP_REG16(REG_ISP_DEV_SEL) = 0; //test
    ISP_REG16(REG_ISP_SPI_ENDIAN) = 0; //little-endian
    printf("    spi init done! \n");
}

//-------------------------------------------------------------------------------------------------
/// Erase all sectors in Serial Flash
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_EraseChip(void)
{
    BOOL bRet = FALSE;
	printf("  spi erase all chip...\n");


    SER_FLASH_ENTRY();

    ISP_REG16(REG_ISP_PASSWORD) = 0xAAAA; //enable ISP

    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
    {
        goto MDrv_SerFlash_EraseChip_return;
    }
    ISP_REG16(REG_ISP_SPI_COMMAND) = 0x06; //WREN


    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
    {
        goto MDrv_SerFlash_EraseChip_return;
    }
    ISP_REG16(REG_ISP_SPI_COMMAND) = 0xC7; //CHIP_ERASE

    bRet = _MDrv_SerFlash_WaitWriteDone();

    if(bRet==TRUE)
        printf("    spi erase all chip Done!\n");
    else
        printf("    spi erase all chip fail!\n");

MDrv_SerFlash_EraseChip_return:

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis

    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP

    SER_FLASH_RETURN();

    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Erase certain sectors in Serial Flash
/// @param  u32StartSec    \b IN: start sector
/// @param  u32EndSec      \b IN: end sector
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_EraseSec(MS_U32 u32StartSec, MS_U32 u32EndSec)
{
    //HW doesn't support Sector Erase command on MX flash now; use trigger mode instead.
    BOOL bRet = FALSE;
    MS_U32 u32I;

    SER_FLASH_ENTRY();

    ISP_REG16(REG_ISP_PASSWORD) = 0xAAAA; //enable ISP

    for( u32I=u32StartSec; u32I<=u32EndSec; u32I++)
    {
        #if(SERFLASH_TYPE == SERFLASH_TYPE_MX25L3205) || (SERFLASH_TYPE == SERFLASH_TYPE_MX25L6405)
        {
            if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
            {
                goto MDrv_SerFlash_EraseSec_return;
            }
            ISP_REG16(REG_ISP_SPI_COMMAND) = 0x06; //WREN

            if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
            {
                goto MDrv_SerFlash_EraseSec_return;
            }
            //SFSH_RIU_REG16(REG_SFSH_SPI_COMMAND) = 0x20; //SECTOR_ERASE

            ISP_REG16(REG_ISP_TRIGGER_MODE) = 0x3333; //enable trigger mode

            ISP_REG16(REG_ISP_SPI_WDATA) = 0x20; //SECTOR_ERASE
            if ( _MDrv_SerFlash_WaitWriteDataRdy() == FALSE )
            {
                printf("Erase sec wait write data ready fail");
                goto MDrv_SerFlash_EraseSec_return;
            }

            ISP_REG16(REG_ISP_SPI_WDATA) = HIU16(u32I*SERFLASH_SECTOR_SIZE) & 0xFF;
            if ( _MDrv_SerFlash_WaitWriteDataRdy() == FALSE )
            {
                printf("Erase sec wait write data ready fail");
                goto MDrv_SerFlash_EraseSec_return;
            }

            ISP_REG16(REG_ISP_SPI_WDATA) = LOU16(u32I*SERFLASH_SECTOR_SIZE) >>8;
            if ( _MDrv_SerFlash_WaitWriteDataRdy() == FALSE )
            {
                printf("Erase sec wait write data ready fail");
                goto MDrv_SerFlash_EraseSec_return;
            }

            ISP_REG16(REG_ISP_SPI_WDATA) = LOU16(u32I*SERFLASH_SECTOR_SIZE) & 0xFF;
            if ( _MDrv_SerFlash_WaitWriteDataRdy() == FALSE )
            {
                printf("Erase sec wait write data ready fail");
                goto MDrv_SerFlash_EraseSec_return;
            }

            ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis

            ISP_REG16(REG_ISP_TRIGGER_MODE) = 0x2222; //disable trigger mode

            bRet = _MDrv_SerFlash_WaitWriteDone();
            if ( bRet == FALSE )
            {
                printf("Erase sec wait write done fail");
                goto MDrv_SerFlash_EraseSec_return;
            }

            ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis
        }

        #elif(SERFLASH_TYPE == SERFLASH_TYPE_EN25B16  ) || (SERFLASH_TYPE == SERFLASH_TYPE_EN25B64)
        {
            // Treat first small sectors to be one 64K sector
            if (u32I == 0)
            {   // 2*4K, 1*8K, 1*16K, 1*32K
                static MS_U16 small_sec_size[] = { 0x0, 0x1000, 0x2000, 0x4000, 0x8000 };
                int i;
                for (i = 0; i < sizeof(small_sec_size) / sizeof(MS_U16); i++)
                {
                    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
                    {
                        goto MDrv_SerFlash_EraseSec_return;
                    }
                    ISP_REG16(REG_ISP_SPI_COMMAND) = 0x06; //WREN

                    ISP_REG16(REG_ISP_SPI_ADDR_L) = LOU16(small_sec_size[i]);
                    ISP_REG16(REG_ISP_SPI_ADDR_H) = HIU16(small_sec_size[i]);

                    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
                    {
                        printf("Erase sec wait write cmd ready fail");
                        goto MDrv_SerFlash_EraseSec_return;
                    }
                    ISP_REG16(REG_ISP_SPI_COMMAND) = 0xD8; //SECTOR_ERASE

                    bRet = _MDrv_SerFlash_WaitWriteDone();
                    if ( bRet == FALSE )
                    {
                        printf("Erase sec wait write done fail");
                        goto MDrv_SerFlash_EraseSec_return;
                    }
                }
            }
            else
            {   // 31*64K
                if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
                {
                    goto MDrv_SerFlash_EraseSec_return;
                }
                ISP_REG16(REG_ISP_SPI_COMMAND) = 0x06; //WREN

                ISP_REG16(REG_ISP_SPI_ADDR_L) = LOU16(u32I*SERFLASH_SECTOR_SIZE);
                ISP_REG16(REG_ISP_SPI_ADDR_H) = HIU16(u32I*SERFLASH_SECTOR_SIZE);

                if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
                {
                    goto MDrv_SerFlash_EraseSec_return;
                }
                ISP_REG16(REG_ISP_SPI_COMMAND) = 0xD8; //SECTOR_ERASE

                bRet = _MDrv_SerFlash_WaitWriteDone();
                if ( bRet == FALSE )
                {
                    goto MDrv_SerFlash_EraseSec_return;
                }
            }
        }
        if(SERFLASH_TYPE == SERFLASH_TYPE_S25FL064A)
        {
            if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
            {
                goto MDrv_SerFlash_EraseSec_return;
            }
            ISP_REG16(REG_ISP_SPI_COMMAND) = 0x06; //WREN

            ISP_REG16(REG_ISP_SPI_ADDR_L) = LOU16(u32I*SERFLASH_SECTOR_SIZE);
            ISP_REG16(REG_ISP_SPI_ADDR_H) = HIU16(u32I*SERFLASH_SECTOR_SIZE);

            if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
            {
                goto MDrv_SerFlash_EraseSec_return;
            }
            ISP_REG16(REG_ISP_SPI_COMMAND) = 0xD8; //SECTOR_ERASE

            bRet = _MDrv_SerFlash_WaitWriteDone();
            if ( bRet == FALSE )
            {
                printf("Erase sec wait write done fail");
                goto MDrv_SerFlash_EraseSec_return;
            }
        }
	#endif
    }

MDrv_SerFlash_EraseSec_return:

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis

    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP

    SER_FLASH_RETURN();

    return bRet;
}


//-------------------------------------------------------------------------------------------------
/// Write data to Serial Flash
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b IN: data to be written
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_Write(MS_U32 u32Addr, MS_U8 *pu8Data, MS_U32 u32Size)
{
    BOOL bRet = FALSE;
    MS_U32 u32I;
    MS_U32 u32Rem, u32WriteBytes;
    //MS_U32 u32Status=0;
    //ASSERT( u32Addr+u32Size <= _SerFlashInfo.u32TotalSize);
    //ASSERT( u32Addr%4 ==0 );
    //ASSERT( u32Size%4 ==0 );
	printf("  spi write... \n");


    SER_FLASH_ENTRY();

    ISP_REG16(REG_ISP_PASSWORD) = 0xAAAA; //enable ISP

 //   MDrv_SerFlash_WriteProtect(0);
 //   printf("MDrv_SerFlash_Write 0001\n");
 //   u32Status = *((volatile MS_U32*)(REG_ISP_BASE+REG_ISP_SPI_WR_CMDRDY*4));
 //   printf("MDrv_SerFlash_Write write cmd ready %x\n",u32Status);

#define SERFLASH_PAGE_SIZE  256

    u32Rem = u32Addr % SERFLASH_PAGE_SIZE;

    if(u32Rem)
    {

        u32WriteBytes = SERFLASH_PAGE_SIZE - u32Rem;
        if(u32Size < u32WriteBytes)
        {
            u32WriteBytes = u32Size;
        }
        if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
        {
            goto MDrv_SerFlash_Write_return;
        }
        ISP_REG16(REG_ISP_SPI_COMMAND) = 0x06; //WREN

        ISP_REG16(REG_ISP_SPI_ADDR_L) = LOU16(u32Addr);
        ISP_REG16(REG_ISP_SPI_ADDR_H) = HIU16(u32Addr);
        if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
        {
            goto MDrv_SerFlash_Write_return;
        }
        ISP_REG16(REG_ISP_SPI_COMMAND) = 0x02; //PAGE_PROG
        for ( u32I=0; u32I<u32WriteBytes; u32I++)
        {
            ISP_REG16(REG_ISP_SPI_WDATA) = pu8Data[u32I];

            if ( _MDrv_SerFlash_WaitWriteDataRdy() == FALSE )
            {
                goto MDrv_SerFlash_Write_return;
            }
        }

        ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis

        bRet = _MDrv_SerFlash_WaitWriteDone();

        if ( bRet == TRUE )
        {
            u32Addr += u32WriteBytes;
            pu8Data += u32WriteBytes;
            u32Size -= u32WriteBytes;
        }
        else
        {
            goto MDrv_SerFlash_Write_return;
        }
    }
    while(u32Size)
    {
        if( u32Size > SERFLASH_PAGE_SIZE)
        {
            u32WriteBytes = SERFLASH_PAGE_SIZE; //write EEPROM_WRITE_BYTES_MAX bytes one time
        }
        else
        {
            u32WriteBytes = u32Size;
        }

        if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
        {
            goto MDrv_SerFlash_Write_return;
        }
        ISP_REG16(REG_ISP_SPI_COMMAND) = 0x06; //WREN
        ISP_REG16(REG_ISP_SPI_ADDR_L) = LOU16(u32Addr);
        ISP_REG16(REG_ISP_SPI_ADDR_H) = HIU16(u32Addr);
        if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
        {
            goto MDrv_SerFlash_Write_return;
        }
        ISP_REG16(REG_ISP_SPI_COMMAND) = 0x02; //PAGE_PROG
        for ( u32I=0; u32I<u32WriteBytes; u32I++)
        {
            ISP_REG16(REG_ISP_SPI_WDATA) = pu8Data[u32I];
            if ( _MDrv_SerFlash_WaitWriteDataRdy() == FALSE )
            {
                goto MDrv_SerFlash_Write_return;
            }
        }

        ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis
        bRet = _MDrv_SerFlash_WaitWriteDone();
        if ( bRet == TRUE )
        {
            u32Addr += u32WriteBytes;
            pu8Data += u32WriteBytes;
            u32Size -= u32WriteBytes;
        }
        else
        {
            printf("write cmd wait write done fail");
            goto MDrv_SerFlash_Write_return;
        }
    }

MDrv_SerFlash_Write_return:

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis
    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP
    SER_FLASH_RETURN();
	printf("    spi write done! \n");
    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Read data from Serial Flash
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b OUT: data ptr to store the read data
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_Read(MS_U32 u32Addr, MS_U8 *pu8Data, MS_U32 u32Size)
{
    BOOL bRet = FALSE;
    MS_U32 u32I;

    //ASSERT( u32Addr+u32Size <= _SerFlashInfo.u32TotalSize);
    //ASSERT( u32Addr%4 ==0 );
    //ASSERT( u32Size%4 ==0 );

    SER_FLASH_ENTRY();

#if 0 //def READ_SERFLASH_READ_BY_RIU //slower than read via XIU

    ISP_REG16(REG_ISP_PASSWORD) = 0xAAAA; //enable ISP

    ISP_REG16(REG_ISP_SPI_ADDR_L) = LOU16(u32Addr);
    ISP_REG16(REG_ISP_SPI_ADDR_H) = HIU16(u32Addr);

    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
    {
        goto MDrv_SerFlash_Read_return;
    }
    ISP_REG16(REG_ISP_SPI_COMMAND) = 0x03; //READ //0x0B fast Read : HW doesn't support now

    for ( u32I=0; u32I<u32Size; u32I++)
    {
        ISP_REG16(REG_ISP_SPI_RDREQ) = 0x01; //SPI read request

        if ( _MDrv_SerFlash_WaitReadDataRdy() == FALSE )
        {
            goto MDrv_SerFlash_Read_return;
        }

        pu8Data[u32I] = ISP_REG16(REG_ISP_SPI_RDATA);
    }

    bRet = TRUE;

MDrv_SerFlash_Read_return:

#else //via XIU (FAST_READ)
    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP

    for( u32I=0; u32I<u32Size; u32I+=4)
    {
        //only indirect mode
        MS_U32 u32Value = SFSH_XIU_REG32((u32Addr+u32I)/4);
        //FixMe: if u32Addr / u32Size is not 4-B aligned
        /*
        pu8Data[u32I+0] = ( u32Value >> 24) & 0xFF;
        pu8Data[u32I+1] = ( u32Value >> 16) & 0xFF;
        pu8Data[u32I+2] = ( u32Value >> 8) & 0xFF;
        pu8Data[u32I+3] = ( u32Value >> 0) & 0xFF;
        */
        pu8Data[u32I+0] = ( u32Value >> 0) & 0xFF;
        pu8Data[u32I+1] = ( u32Value >> 8) & 0xFF;
        pu8Data[u32I+2] = ( u32Value >> 16) & 0xFF;
        pu8Data[u32I+3] = ( u32Value >> 24) & 0xFF;
    }

    bRet = TRUE;
#endif

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis
    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP
    SER_FLASH_RETURN();
    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Protect blocks in Serial Flash
/// @param  bEnable \b IN: TRUE/FALSE: enable/disable protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_WriteProtect(BOOL bEnable)
{
//Note: Temporarily don't call this function until MSTV_Tool ready
#if 1
    BOOL bRet = FALSE;

    SER_FLASH_ENTRY();

    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP
    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
    {
        goto MDrv_SerFlash_WriteProtect_return;
    }

    ISP_REG16(REG_ISP_PASSWORD) = 0xAAAA; //enable ISP
    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
    {
        goto MDrv_SerFlash_WriteProtect_return;
    }
    ISP_REG16(REG_ISP_SPI_COMMAND) = 0x06; //WREN

    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
    {
        goto MDrv_SerFlash_WriteProtect_return;
    }
    ISP_REG16(REG_ISP_SPI_COMMAND) = 0x01; //WRSR
    if (bEnable)
    {
        #if(SERFLASH_TYPE == SERFLASH_TYPE_MX25L3205) || (SERFLASH_TYPE == SERFLASH_TYPE_MX25L6405)
            ISP_REG16(REG_ISP_SPI_WDATA) = 0xf<<2;  //[5:2] protect blocks
        #elif((SERFLASH_TYPE == SERFLASH_TYPE_EN25B16  ) || (SERFLASH_TYPE == SERFLASH_TYPE_EN25B64)
            ||(SERFLASH_TYPE == SERFLASH_TYPE_S25FL064A))
            ISP_REG16(REG_ISP_SPI_WDATA) = 0x7<<2;  //[4:2] protect blocks
       #endif
    }
    else
    {
        #if(SERFLASH_TYPE == SERFLASH_TYPE_MX25L3205) || (SERFLASH_TYPE == SERFLASH_TYPE_MX25L6405)
            ISP_REG16(REG_ISP_SPI_WDATA) = 0x0<<2;  //[5:2] protect blocks
       #elif((SERFLASH_TYPE == SERFLASH_TYPE_EN25B16  ) || (SERFLASH_TYPE == SERFLASH_TYPE_EN25B64)
            ||(SERFLASH_TYPE == SERFLASH_TYPE_S25FL064A))
            ISP_REG16(REG_ISP_SPI_WDATA) = 0x0<<2;  //[4:2] protect blocks
       #endif
    }
    if ( _MDrv_SerFlash_WaitWriteDataRdy() == FALSE )
    {
        goto MDrv_SerFlash_WriteProtect_return;
    }
    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis
    bRet = _MDrv_SerFlash_WaitWriteDone();
    bRet = TRUE;

MDrv_SerFlash_WriteProtect_return:
    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis
    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP
    SER_FLASH_RETURN();

    return bRet;
#else
    return TRUE;
#endif
}

//-------------------------------------------------------------------------------------------------
/// Read ID from Serial Flash
/// @param  pu8Data \b OUT: data ptr to store the read ID
/// @param  u32Size \b IN: size in Bytes
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_ReadID(MS_U8 *pu8Data, MS_U32 u32Size)
{
    //HW doesn't support ReadID on MX flash; use trigger mode instead.
    BOOL bRet = FALSE;
    MS_U32 u32I;

    ISP_REG16(REG_ISP_PASSWORD) = 0xAAAA; //enable ISP

    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
    {
        goto MDrv_SerFlash_ReadID_return;
    }

    ISP_REG16(REG_ISP_TRIGGER_MODE) = 0x3333; //enable trigger mode

    ISP_REG16(REG_ISP_SPI_WDATA) = 0x9F; //RDID
    if ( _MDrv_SerFlash_WaitWriteDataRdy() == FALSE )
    {
        goto MDrv_SerFlash_ReadID_return;
    }

    for ( u32I=0; u32I<u32Size; u32I++)
    {
        ISP_REG16(REG_ISP_SPI_RDREQ) = 0x01; //SPI read request

        if ( _MDrv_SerFlash_WaitReadDataRdy() == FALSE )
        {
            goto MDrv_SerFlash_ReadID_return;
        }

        pu8Data[u32I] = ISP_REG16(REG_ISP_SPI_RDATA);
    }
    bRet = TRUE;

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis

    ISP_REG16(REG_ISP_TRIGGER_MODE) = 0x2222; //disable trigger mode


MDrv_SerFlash_ReadID_return:

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis

    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP

    SER_FLASH_RETURN();

    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Read data from Serial Flash to DRAM in DMA mode
/// @param  u32FlashStart \b IN: src start address in flash (0 ~ flash size-1)
/// @param  u32DRAMStart  \b IN: dst start address in DRAM (16B-aligned) (0 ~ DRAM size-1)
/// @param  u32Size       \b IN: size in Bytes (8B-aligned) (>=8)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_DMA(MS_U32 u32FlashStart, MS_U32 u32DRAMStart, MS_U32 u32Size)
{
    BOOL bRet = FALSE;
    MS_U32 u32Timer;
    MS_U32 u32Timeout;

    u32Timeout = SERFLASH_SAFETY_FACTOR*u32Size/(108*1000/4/8);

    SER_FLASH_ENTRY();

    ISP_REG16(REG_ISP_CHIP_SEL) |= SFSH_CHIP_SEL_RIU;   //For DMA only
    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP

    PIU_REG16(REG_PIU_DMA_SIZE_L) = LOU16(u32Size);
    PIU_REG16(REG_PIU_DMA_SIZE_H) = HIU16(u32Size);
    PIU_REG16(REG_PIU_DMA_DRAMSTART_L) = LOU16(u32DRAMStart);
    PIU_REG16(REG_PIU_DMA_DRAMSTART_H) = HIU16(u32DRAMStart);
    PIU_REG16(REG_PIU_DMA_SPISTART_L) = LOU16(u32FlashStart);
    PIU_REG16(REG_PIU_DMA_SPISTART_H) = HIU16(u32FlashStart);
    PIU_REG16(REG_PIU_DMA_CMD) = PIU_DMA_CMD_LE | PIU_DMA_CMD_FIRE; //trigger

    // Wait for DMA to be done
    SER_FLASH_TIME(u32Timer, u32Timeout)
    do {
        if ( (PIU_REG16(REG_PIU_DMA_STATUS) & 0x02) == 0x2 ) //finished
        {
            bRet = TRUE;
            break;
        }
        SER_FLASH_YIELD_TASK();
    } while (!SER_FLASH_EXPIRE(u32Timer));

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(printf("DMA timeout!\n"));
    }

    ISP_REG16(REG_ISP_CHIP_SEL) &= ~SFSH_CHIP_SEL_RIU;  //For DMA only
    SER_FLASH_RETURN();

    return bRet;
}

//------- ------------------------------------------------------------------------------------------
/// Read Status Register in Serial Flash
/// @param  pu8StatusReg \b OUT: ptr to Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_ReadStatusReg(MS_U8 *pu8StatusReg)
{
    BOOL bRet = FALSE;

    SER_FLASH_ENTRY();

    ISP_REG16(REG_ISP_PASSWORD) = 0xAAAA; //enable ISP

    if ( _MDrv_SerFlash_WaitWriteCmdRdy() == FALSE )
    {
        goto MDrv_SerFlash_ReadStatusReg_return;
    }
    ISP_REG16(REG_ISP_SPI_COMMAND) = 0x05; //RDSR

    ISP_REG16(REG_ISP_SPI_RDREQ) = 0x01; //SPI read request

    if ( _MDrv_SerFlash_WaitReadDataRdy() == FALSE )
    {
        goto MDrv_SerFlash_ReadStatusReg_return;
    }

    *pu8StatusReg = ISP_REG16(REG_ISP_SPI_RDATA);

    bRet = TRUE;

MDrv_SerFlash_ReadStatusReg_return:

    ISP_REG16(REG_ISP_SPI_CECLR) = 1; //SPI CEB dis

    ISP_REG16(REG_ISP_PASSWORD) = 0x5555; //disable ISP

    SER_FLASH_RETURN();

    return bRet;
}

u8 SWU_SPI_Update(unsigned char* pImg_start, u32 size)
{
	printf("< SPI boot downloading started > \n");

	MDrv_SerFlash_Init();
	MDrv_SerFlash_EraseChip();
	MDrv_SerFlash_Write(0, pImg_start, size);
	MDrv_SerFlash_Read(0, (MS_U8 *)SPI_READ_ADDRESS, size);
	if(memcmp(pImg_start, (unsigned char*)SPI_READ_ADDRESS, size))
	{
		printf("  Verify is failed... \n");
		return (-1);
	}

	printf("  Verify is OK... \n");
	printf("< SPI boot downloading ended > \n");

	return 0;
}
