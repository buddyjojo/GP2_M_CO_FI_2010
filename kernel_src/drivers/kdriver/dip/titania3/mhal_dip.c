///////////////////////////////////////////////////////////////////////////////
//
//      File name: drvDIP.c
//      Company: MStarSemi Inc.
//
//      Description:  DIP driver implementation.
//                    1. NR part : noise reduction
//                    2. DI part : De-interlace
//
///////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------------
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/bitops.h>

#include <linux/spinlock.h>


#include <asm/atomic.h>

#include <linux/interrupt.h>
#include "chip_int.h"

#include "mdrv_types.h"
#include "mhal_dip.h"
#include "mhal_diprg.h"
#include "Board.h"

#define DIP_DEBUG
#ifdef  DIP_DEBUG

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                            BUG();\
                        }\
                    } while (0)

#define ASSERT(arg)                  assert((arg))
#define KDBG(x1, args...)            printk(KERN_WARNING x1, ## args)
#else
#define KDBG(x1)
#define ASSERT(arg)
#endif

#define DIP_DBG(x)          (x)

//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
/*
#define MAX_NR_BUFF_CNT     2
#define MAX_DATA_BUFF_CNT   16
#define MAX_BUFF_WIDTH      0x440   //1088
#define MAX_BUFF_HEIGHT     0x240   //576
#define SKIP_FIELD_NUMBER   60
#define NOT_SKIP_FIELD      1
*/

#if FRAME_CHECK
#define CHECK_LENGTH        8
#endif

/*
typedef enum
{
    E_SET_NR_BUFFER = BIT(0),
    E_SET_DI_BUFFER = BIT(1),
    E_SET_INPUT_TYPE = BIT(2),
    E_ENABLE_SNR = BIT(3),
    E_ENABLE_TNR = BIT(4),
    E_ENABLE_NR = BIT(5),
    E_ENABLE_DI = BIT(6)
}DIP_FLAG;
*/


//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------------
DEFINE_SPINLOCK(lock_dip);


DIP_FLAG u32DIPFlag = 0;
//NR setting
static U8 u8NrBuffCnt = 0;
static U32 u32NrDataBuff[MAX_NR_BUFF_CNT];
static U32 u32NrRatioBuff;
static U32 u32NrBuffWidth = 0;
static U32 u32NrBuffHeight = 0;
//DI setting
static U8 u8DiBuffCnt = 0;
dip_YUV420_start_t sDiDataBuff[MAX_DATA_BUFF_CNT];
static U32 u32DiBuffWidth = 0;
static U32 u32DiBuffHeight = 0;
static U8  u8HistOut[16];
static U8  u8HistDiff[16];
//DI output buffer control
//static U8  u8WriteField = 0;
U8  u8WriteField = 0;
static U8  u8DiBuffWritingIndex = 0;
volatile U16 u16DiBuffStatus = 0;// bit 0 is 0: DI Buffer 0 not ready; bit 0 is 1:DI Buffer 0 ready
static U32 u32DiBufferFieldCount = 0;
static U32 u32DiBufferFrameCount = 0;

static U8  u8DiBufferStartOutputFlag = FALSE;
//static U32 u8LastDiBufferReadyTime = 0;
U32 u8LastDiBufferReadyTime = 0;

U32 u32OverflowCount = 0;

#if ENABLE_DI_BUFFER_VERIFY
static U32 u32VerifyDiBufStartAddress;
static U8  u8VerifyDiBufCnt;
static U8  BufferIndex =0;
static U8* Y_TailBuf;
static U8* UV_TialBuf;
static U8* Y_SequencyBuf;
static U8* U_SequencyBuf;
static U8* V_SequencyBuf;
static U8* OutBuf;
static U8  DiBufferVerifyStart = 0;
#endif

#if DI_BUFFER_SKIP_CHECK
#define ERROR_INTERVAL_TIME 18
#define ChECK_NUMBER 300
static U32 u32DiBufferErrorFieldCount =0;
static U32 IntervalRecord[ChECK_NUMBER];
static U32 ErrorIntervalRecord[10];
static U32 EnableDiBufferTime =0;
static U32 FirstDiBufferDoneTime =0;
static U32 SecondDiBufferDoneTime =0;
#endif

#if MONITOR_INT_INTERVAL
U32 u32Time1 = 0;
U32 u32Time2 = 0;
U32 u32Time3 = 0;
U32 u32IntCount = 0;
#endif

#if NOT_SKIP_FIELD
BOOL bNotSkipField = FALSE;
#endif

#if FRAME_CHECK
U8 u8CheckWord[CHECK_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};
#endif

#define      _DIPENABLE          BIT0
#define      _DIPDISABLE         BIT1
#define      _DIPESETTING        BIT2
atomic_t     DIPEnabling = ATOMIC_INIT(_DIPDISABLE);


//----------------------------------------------------------------------------
// Local Function Prototypes
//----------------------------------------------------------------------------
#if FRAME_CHECK
BOOL MHal_DIP_SetIdentifyInfo(U8 u8DiIndex)
{
#if 0
    U8* pu8Start = NULL;

    if (u8DiIndex > u8DiBuffCnt)
    {
        return FALSE;
    }
    //KDBG("Set %d.\n", u8DiIndex);
    pu8Start = (U8*)(sDiDataBuff[u8DiIndex].u32YStart | 0xA0000000);
    memcpy(pu8Start, u8CheckWord, CHECK_LENGTH);
#endif
    return TRUE;
}


BOOL MHal_DIP_CheckIdentifyInfo(U8 u8DiIndex)
{
#if 0
    U8* pu8Start = NULL;
    U8 i;

    if (u8DiIndex > u8DiBuffCnt)
    {
        return FALSE;
    }

    pu8Start = (U8*)(sDiDataBuff[u8DiIndex].u32YStart | 0xA0000000);

    for (i=0; i<CHECK_LENGTH; i++)
    {
        if (pu8Start[i] != u8CheckWord[i])
        {
            return FALSE;
        }
    }
#endif
    return TRUE;
}
#else
BOOL MHal_DIP_SetIdentifyInfo(U8 u8DiIndex)
{
    UNUSED(u8DiIndex);
    return TRUE;
}


BOOL MHal_DIP_CheckIdentifyInfo(U8 u8DiIndex)
{
    UNUSED(u8DiIndex);
    return TRUE;
}

#endif

extern U32 pDIPInfo;

typedef irqreturn_t (*IsrHandler)(int irq, void *dev_id);
BOOL DIP_ISR_Control(BOOL bEnable, IsrHandler pCallback)
{
    static BOOL bISRstates = FALSE;

    if(bEnable)
    {
		if(FALSE == bISRstates) {
        if(request_irq(E_IRQ_DIP, pCallback, SA_INTERRUPT, "DIP_IRQ", NULL) != 0)// LGE drmyung 081013
        {
            KDBG("IRQ request failure\n");
            return FALSE;
        }
        bISRstates = TRUE;
		} else {
			disable_irq(E_IRQ_DIP);
			enable_irq(E_IRQ_DIP);
    }
    }
    else if(bEnable == FALSE)
    {
        // disable interrupt here
        // ...
        //
        free_irq(E_IRQ_DIP, NULL);
        bISRstates = FALSE;
    }
    return TRUE;
}


void MHal_DIP_ShowFieldInfo(void)
{
#if ENABLE_DI_BUFFER_VERIFY
    //printk("TO YUV\n");
    void MHal_DIP_TailYUV420ToSequenceYUVBlock1(void);


    _MHal_W1BM(DI_REG_MIU_CTRL, 0, BIT0);

    MHal_DIP_SetDIVerBuff(0);
    MHal_DIP_TailYUV420ToSequenceYUVBlock();
    MHal_DIP_SequenceYUVBlockToYUV422();


    _MHal_W1BM(DI_REG_MIU_CTRL, BIT0, BIT0);
#endif
/*
    U32 Time;

    Time = jiffies;
    KDBG("%d time %d\n",u32DiBufferFieldCount - u32LastDiBufferFieldCount,Time -u32LastDiBufferCountTime);
    u32LastDiBufferFieldCount=u32DiBufferFieldCount;
    u32LastDiBufferCountTime =Time;
*/

    //printk("NR REG[0x72] 0x%x\n", _MHal_R2B(NR_REG(0x72)));
    //KDBG("%d of %d \n",u32DiBufferErrorFieldCount,u32DiBufferFieldCount);
}

#if DI_BUFFER_SKIP_CHECK
#if 0
irqreturn_t DIP_HandleIsr(int irq, void *dev_id)
{
    U8  u8IntStatus = 0;
    U32 CurrentSystemTime = 0;
    U16 Interval = 0;
    //Avoid use KDBG in this function

    //Read int 19~16
    u8IntStatus = _MHal_R1B(NR_REG_IRQ_STATUS_19_16);
    if(u8IntStatus != 0)
    {
        if(u8IntStatus & REG_IRQ_DI_WR_MIU_DONE)
        {
                if( u8LastDiBufferReadyTime == 0 )
                {
                    u32DiBufferFieldCount++;
                    u8LastDiBufferReadyTime = jiffies;
                    FirstDiBufferDoneTime = u8LastDiBufferReadyTime ;
                }
                else
                {

                    CurrentSystemTime = jiffies;
                    Interval = CurrentSystemTime - u8LastDiBufferReadyTime;
                    u32DiBufferFieldCount += (Interval/16);
                    if( SecondDiBufferDoneTime ==0)
                        SecondDiBufferDoneTime =CurrentSystemTime;

                    //if( u32DiBufferFieldCount >= SKIP_FIELD_NUMBER)
                            //MDrv_WriteByteMask(DI_REG_MIU_CTRL, 0, (REG_DI_MIU_EN | REG_DI_MIU_WR_EN));

                    if( u32DiBufferFieldCount < ChECK_NUMBER )
                    {
                        IntervalRecord[u32DiBufferFieldCount] = Interval;
                    }

                    if( Interval > ERROR_INTERVAL_TIME )
                    {
                       u32DiBufferErrorFieldCount++;
                       ErrorIntervalRecord[u32DiBufferErrorFieldCount] = Interval;
                    }

                    u8LastDiBufferReadyTime = CurrentSystemTime;
                }

        }
        _MHal_W1B(NR_REG_IRQ_CLR_19_16, u8IntStatus);
        _MHal_W1B(NR_REG_IRQ_CLR_19_16, 0);
    }

    //MsOS_EnableInterrupt(E_IRQ_DISP_IPATH);
    return IRQ_HANDLED;
}
#endif
#else
#if FRAME_CHECK

#if MONITOR_INT_INTERVAL
static U8       g_DI_FinishIdx=0;
unsigned int   *g_pVerifyBuf;
#endif



extern void Chip_Flush_Memory(void);

irqreturn_t DIP_HandleIsr(int irq, void *dev_id)
{
    U8  u8IntStatus = 0;
    //U32 CurrentSystemTime = 0;
    //U16 Interval = 0;
    volatile    U32 *pInfo;
    U8 u8Index;
    static U8 prev_index = 0xFF;
    //Avoid use KDBG in this function
    #if MONITOR_INT_INTERVAL
    static  U32     info_index;
    #endif

    //Read int 19~16
    /*
        The IRQ status [15:0] for disp_ipath
        # [0] : the rising edge for NR Vsync output
        # [1] : the falling edge for NR Vsync output
        # [2] : the rising edge for NR Hsync output
        # [3] : the falling edge for NR Hsync output
        # [4] : the rising edge for NR Field output
        # [5] : the falling edge for NR Field output
        # [6] : No Vsync / Hsync as video source input
        # [7] : Abnormal Vertical Line Numbers as video source input
        # [8] : Abnormal Horizontal Pixel Numbers as video source input
        # [9] : NR Write Motion Ratio FIFO Full
        # [10] : NR Write Data FIFO Full
        # [11] : NR Read History Mode Motion Ratio FIFO Empty
        # [12] : NR Read Motion Ratio FIFO Empty
        # [13] : NR Read Data FIFO Empty
        # [14] : DI Read FIFO Empty
        # [15] : DI Write C FIFO Full
        The IRQ status [19:16] for disp_ipath
        # [16] : DI Write FIFO Full
        # [17]:  NR Write MIU Finish
        # [18]:  DI Write MIU Finish
        # [19]:  SNR Frost Algorithm Statistic Ready
        # [23:20]:  DI Write MIU Finish Index
        # [19:17] : reserved
    */
    u8IntStatus = _MHal_R1B(NR_REG_IRQ_STATUS_19_16);
    if(u8IntStatus != 0)
    {
        // DI Write MIU Finish
        if(u8IntStatus & REG_IRQ_DI_WR_MIU_DONE)
        {
            #if MONITOR_INT_INTERVAL


                *(unsigned int*)(g_pVerifyBuf+info_index) = (unsigned int)jiffies-u32Time1;
                u32Time1 = jiffies;
                info_index++;
                if(info_index == 1024*1024/4)
                {
                    info_index = 0;
                }
                /*
                u32IntCount++;
                if(u32IntCount==30)
                {
                    printk("TS %d %d\n", (unsigned int)jiffies, (unsigned int)jiffies-u32Time1);
                    u32IntCount = 0;
                }
                */


            #endif

            //u8Index = (U8)((u32DiBufferFrameCount+1)%u8DiBuffCnt);
            //u8Index_di = (U8)_MHal_R2B(DI_REG(0x5f));
            //g_DI_FinishIdx =(U8)((u32DiBufferFrameCount)%u8DiBuffCnt);
            u8Index = (U8)_MHal_R2B(DI_REG(0x5f));
            //printk("u8Index %x\n", u8Index);
            //if (MHal_DIP_CheckIdentifyInfo((u8Index))
            if(prev_index == u8Index)
            {

                //u16DiBuffStatus |= (1 << (u32DiBufferFrameCount%u8DiBuffCnt));
                u16DiBuffStatus |= (1 << (u8Index));
                u32DiBufferFrameCount++;
            }
            else
            {
                prev_index = u8Index;
            }
            pInfo = (U32*) pDIPInfo;
            *(pInfo) = (U32)u8Index;
            *(pInfo+1) = (U32)u16DiBuffStatus;
            *(pInfo+2) = (U32)u32DiBufferFrameCount;
            mb();
            Chip_Flush_Memory();
        }
        _MHal_W1B(NR_REG_IRQ_CLR_19_16, u8IntStatus);
        _MHal_W1B(NR_REG_IRQ_CLR_19_16, 0);
    }


    //MsOS_EnableInterrupt(E_IRQ_DISP_IPATH);
    return IRQ_HANDLED;
}

#else
#if 0
irqreturn_t DIP_HandleIsr(int irq, void *dev_id)
{
    U8  u8IntStatus = 0;
    U32 CurrentSystemTime = 0;
    U16 Interval = 0;
    //Avoid use KDBG in this function

    //Read int 19~16
    u8IntStatus = _MHal_R1B(NR_REG_IRQ_STATUS_19_16);
    if(u8IntStatus != 0)
    {

        if(u8IntStatus & REG_IRQ_DI_WR_MIU_DONE)
        {
#if MONITOR_INT_INTERVAL
            u32Time3 = u32Time2;
            u32Time2 = u32Time1;
            u32Time1 = jiffies;
            u32IntCount++;
#endif

#if NOT_SKIP_FIELD
            if (!bNotSkipField)
            {
                u8WriteField = 0;
                u8DiBufferStartOutputFlag = TRUE;
                u16DiBuffStatus = 0;
                u8LastDiBufferReadyTime = 0;
                bNotSkipField = TRUE;
            }
#endif

            u32DiBufferFieldCount++;
            if( u8DiBufferStartOutputFlag == FALSE ) //Skip the first SKIP_FIELD_NUMBER frame
            {
                if( u8LastDiBufferReadyTime == 0 )
                {
                    u8WriteField++;
                    u8LastDiBufferReadyTime = jiffies;
                }
                else
                {
                    CurrentSystemTime = jiffies;
                    Interval = CurrentSystemTime - u8LastDiBufferReadyTime;
                    u8LastDiBufferReadyTime = CurrentSystemTime;
                    //u8WriteField += ((Interval)/16); //60 field per 1000ms
                    u8WriteField += ((Interval)/15); //60 field per 1000ms

                    if( u8WriteField >= SKIP_FIELD_NUMBER )
                    {
                        u8WriteField = u8WriteField -SKIP_FIELD_NUMBER;
                        u8DiBufferStartOutputFlag = TRUE;
                        u16DiBuffStatus = 0;
                    }
                }

            }
            else// Start to Output DI Buffer
            {
                CurrentSystemTime = jiffies;
                Interval = CurrentSystemTime - u8LastDiBufferReadyTime;
#if NOT_SKIP_FIELD
                if (u8LastDiBufferReadyTime == 0)
                {
                    u8LastDiBufferReadyTime = CurrentSystemTime;
                    u8WriteField++;
                    Interval = 0;
                }
#endif
                if (Interval > 15)
                {
                    u8LastDiBufferReadyTime = CurrentSystemTime;
                    u8WriteField += ((Interval)/15);

                    if( u8WriteField <= 1 )
                    {
                    }
                    else if( u8WriteField == 2 )
                    {
                        u8WriteField =0;
                        u32DiBufferFrameCount++;

                        //Record buffer status
                        u16DiBuffStatus |= (U16)(0x01<<u8DiBuffWritingIndex);

                        u8DiBuffWritingIndex++;

                        if( u8DiBuffWritingIndex == u8DiBuffCnt )
                            u8DiBuffWritingIndex = 0;

                        //Check Next Buffer must be ready
                        if( u16DiBuffStatus & (0x01<<u8DiBuffWritingIndex) )
                        {
                            //KDBG("DIP Overflow.\n");
                            u32OverflowCount++;
                        }
                    }
                    else if( u8WriteField == 3 )
                    {
                        u8WriteField = 1;
                        u32DiBufferFrameCount++;

                        //Record buffer status
                        u16DiBuffStatus |= (U16)(0x01<<u8DiBuffWritingIndex);

                        u8DiBuffWritingIndex++;

                        if( u8DiBuffWritingIndex == u8DiBuffCnt )
                            u8DiBuffWritingIndex = 0;
                         //Check Next Buffer must be ready
                        if( u16DiBuffStatus & (0x01<<u8DiBuffWritingIndex) )
                        {
                            //KDBG("DIP Overflow.\n");
                            u32OverflowCount++;
                        }

                    }
                    else if( u8WriteField == 4 )
                    {
                        u8WriteField = 0;
                        u32DiBufferFrameCount += 2;

                        //Record buffer status
                        if( u8DiBuffWritingIndex == u8DiBuffCnt-1 )
                        {
                            u16DiBuffStatus |= (U16)(0x01<<u8DiBuffWritingIndex);
                            u16DiBuffStatus |= 0x01;

                            u8DiBuffWritingIndex = 1;
                        }
                        else
                        {
                            u16DiBuffStatus |= (U16)(0x03<<u8DiBuffWritingIndex);
                            u8DiBuffWritingIndex += 2;
                            if( u8DiBuffWritingIndex == u8DiBuffCnt )
                                u8DiBuffWritingIndex = 0;
                        }
                         //Check Next Buffer must be ready
                        if( u16DiBuffStatus & (0x01<<u8DiBuffWritingIndex) )
                        {
                            //KDBG("DIP Overflow.\n");
                            u32OverflowCount++;
                        }
                    }
                    else if( u8WriteField == 5 )
                    {
                        u8WriteField = 1;
                        u32DiBufferFrameCount += 2;

                        //Record buffer status
                        if( u8DiBuffWritingIndex == u8DiBuffCnt-1 )
                        {
                            u16DiBuffStatus |= (U16)(0x01<<u8DiBuffWritingIndex);
                            u16DiBuffStatus |= 0x01;

                            u8DiBuffWritingIndex = 1;
                        }
                        else
                        {
                            u16DiBuffStatus |= (U16)(0x03<<u8DiBuffWritingIndex);
                            u8DiBuffWritingIndex += 2;
                            if( u8DiBuffWritingIndex == u8DiBuffCnt )
                                u8DiBuffWritingIndex = 0;
                        }
                         //Check Next Buffer must be ready
                        if( u16DiBuffStatus & (0x01<<u8DiBuffWritingIndex) )
                        {
                            //KDBG("DIP Overflow.\n");
                            u32OverflowCount++;
                        }
                    }
                    else if( u8WriteField == 6 )
                    {
                        u8WriteField = 0;
                        u32DiBufferFrameCount += 3;

                        //Record buffer status
                        if( u8DiBuffWritingIndex == u8DiBuffCnt-2)
                        {
                            u16DiBuffStatus |= (U16)(0x03<<u8DiBuffWritingIndex);
                            u16DiBuffStatus |= 0x01;
                            u8DiBuffWritingIndex = 1;
                        }
                        else if( u8DiBuffWritingIndex == u8DiBuffCnt-1 )
                        {
                            u16DiBuffStatus |= (U16)(0x01<<u8DiBuffWritingIndex);
                            u16DiBuffStatus |= 0x03;

                            u8DiBuffWritingIndex = 2;
                        }
                        else
                        {
                            u16DiBuffStatus |= (U16)(0x07<<u8DiBuffWritingIndex);
                            u8DiBuffWritingIndex += 3;
                            if( u8DiBuffWritingIndex == u8DiBuffCnt )
                                u8DiBuffWritingIndex = 0;
                        }
                         //Check Next Buffer must be ready
                        if( u16DiBuffStatus & (0x01<<u8DiBuffWritingIndex) )
                        {
                            //KDBG("DIP Overflow.\n");
                            u32OverflowCount++;
                        }
                    }
                    else
                    {   //Should not be happen
                        //Wait to do
                        KDBG("DIP Error.u8WriteField > 6, %d.\n", Interval);
                        u8WriteField =0;
                    }
                }
            }
        }
        _MHal_W1B(NR_REG_IRQ_CLR_19_16, u8IntStatus);
        _MHal_W1B(NR_REG_IRQ_CLR_19_16, 0);
    }

//if( u32DiBufferFrameCount == 6 )
    //MDrv_WriteByteMask(DI_REG_MIU_CTRL, 0, (REG_DI_MIU_EN | REG_DI_MIU_WR_EN));

    //MsOS_EnableInterrupt(E_IRQ_DISP_IPATH);
    return IRQ_HANDLED;
}
#endif

#endif //FRAME_CHECK
#endif

//----------------------------------------------------------------------------
// Global Function
//----------------------------------------------------------------------------
#if FRAME_CHECK
DIP_ERRCODE MHal_DIP_GetDIOutputInfo(void)
{
    U8 u8Index = (U8)((u32DiBufferFrameCount+1)%u8DiBuffCnt);

    if (!MHal_DIP_CheckIdentifyInfo(u8Index))
    {
        u16DiBuffStatus |= (1 << (u32DiBufferFrameCount%u8DiBuffCnt));
        u32DiBufferFrameCount++;
    }
    return DIP_ERR_OK;
}
#endif

DIP_ERRCODE MHal_DIP_Init(BOOL bEnable)
{
    atomic_set(&DIPEnabling, _DIPDISABLE);

    u32DIPFlag = 0;
    if (bEnable)
    {
        // enable DIP clock (clk_disp_ipath clk)
        _MHal_W1BM(TOP_REG_DIP_CLK, 0x00, REG_DIP_CLK_DISABLE);
    }
    else
    {
        // disable DIP clock
        _MHal_W1BM(TOP_REG_DIP_CLK, 0x01, REG_DIP_CLK_DISABLE);
    }

    //_MHal_W1B(NR_REG_VD_SET_H, REG_NR_LOAD_REG);
    // [08   ] Disable Y blending SNR
    // [09   ] Disable C blending SNR
    // [12:14] YC444 { Y, Cb, Cr }
    // [15   ] Enable load double buffer registers
    //_MHal_W2BM(NR_REG(0x03), 0x8000, 0xFF00);

    //_MHal_W2B(NR_REG_CTRL_L, 0);
    //Disable NR process.
    _MHal_W2Bb(NR_REG(0x00), 0x0, 0x0);


    //_MHal_W1B(NR_REG_IN_CTRL_L, 0);
    // Software reset for NR
    _MHal_W2Bb(NR_REG(0x01), 0x0, 0x0);

    //Disable DI process.
    //_MHal_W1BM(DI_REG_MIU_CTRL, 0, (REG_DI_MIU_WR_EN| REG_DI_MIU_EN));

    // Disable MIF
    _MHal_W1BM(DI_REG_MIU_CTRL, 0, BIT0);


    // NR MIU mode (128bits)
    _MHal_W2BM(NR_REG(0x02), BIT8|BIT9, BIT8|BIT9);

    #if MONITOR_INT_INTERVAL
        g_pVerifyBuf = (U32)phys_to_virt((U32)MVD_SW_ADR);
        printk("VERIFY BUF %x\n", (unsigned int)g_pVerifyBuf);
    #endif

    return DIP_ERR_OK;
}

DIP_ERRCODE MHal_DIP_SetFrameInfo(U32 u32FrameWidth, U32 u32FrameHeight, BOOL bInterLace)
{
    DIP_DBG(KDBG("====MHal_DIP_SetFrameInfo====.\n"));
    DIP_DBG(KDBG("Frame width: %d, height: %d, Interlace: %d.\n", u32FrameWidth, u32FrameHeight, bInterLace));
#if 0
    //Horizontal start position for active window
    _MHal_W2B(NR_REG_ACTIVE_START_X, 0x00);

    //Vertical start position for active window
    _MHal_W2B(NR_REG_ACTIVE_START_Y, 0x07);

    //Horizontal width for active window
    _MHal_W2B(NR_REG_ACTIVE_WIDTH, (u32FrameWidth & 0x7FF)-1);
#else
    //Horizontal start position for active window
    //_MHal_W2B(NR_REG_ACTIVE_START_X, 0xA0);
    _MHal_W2B(NR_REG_ACTIVE_START_X, 0x7C);
    //Vertical start position for active window
    //_MHal_W2B(NR_REG_ACTIVE_START_Y, 0x09);
    _MHal_W2B(NR_REG_ACTIVE_START_Y, 0x07);
    //Horizontal width for active window
    //_MHal_W2B(NR_REG_ACTIVE_WIDTH, (624 & 0x7FF)-1);
    //_MHal_W2B(NR_REG_ACTIVE_WIDTH, (510 & 0x7FF)-1);
    _MHal_W2B(NR_REG_ACTIVE_WIDTH, (672 & 0x7FF)-1);
    //_MHal_W2B(NR_REG_ACTIVE_WIDTH, (u32FrameWidth & 0x7FF)-1);
#endif

    if (bInterLace)
    {
        _MHal_W2B(NR_REG_ACTIVE_HEIGHT_TOP, ((u32FrameHeight>>1)&0x3FF)-1);
        _MHal_W2B(NR_REG_ACTIVE_HEIGHT_BOT, ((u32FrameHeight>>1)&0x3FF)-1);
    }
    else
    {
        _MHal_W2B(NR_REG_ACTIVE_HEIGHT_TOP, (u32FrameHeight&0x3FF)-1);
        _MHal_W2B(NR_REG_ACTIVE_HEIGHT_BOT, 0);
    }

    return DIP_ERR_OK;
}

DIP_ERRCODE MHal_DIP_SetIntputMode(BOOL bVD)
{
    //U8 u8Temp = 0;
    DIP_DBG(KDBG("==== DIP Mode %d ====.\n", bVD));

    if (bVD)
    {
         //Set NR Input
        // CCIR input 16 bits, YC 444
        _MHal_W2BM(NR_REG(0x03), BIT2|BIT0, BIT2|BIT0);
        // HV sync reference : external BT601 HSync, BT601 VSync,
        // Interlace input
        _MHal_W2BM(NR_REG(0x01), (BIT2|BIT3), BITMASK(3:1));
        // Field reference : external BT601 Field
        _MHal_W2BM(NR_REG(0x01), BIT4, BITMASK(5:4));

        // Vsync polarity. (Default : Low)
        _MHal_W2BM(NR_REG(0x01), BIT6, BIT6);
        // Hsync polarity. (Default : Low)
        _MHal_W2BM(NR_REG(0x01), BIT7, BIT7);
        // Field polarity. (Default : Low)
        _MHal_W2BM(NR_REG(0x01), 0x00, BIT8);

#if 0
        _MHal_W1BM(NR_REG_IN_CTRL_L, u8Temp);


        u8Temp = (REG_NR_SOURCE_CCIR656);
        _MHal_W1B(NR_REG_IN_CTRL_L, u8Temp);

        u8Temp = 0;
        _MHal_W1B(NR_REG_IN_CTRL_H, u8Temp);

        u8Temp = (REG_NR_VD_CCIR_8BIT | REG_NR_YUV_ORDER_YCYC | REG_NR_VD_DDR_ORDER_POSITVE_MSB);
        _MHal_W1B(NR_REG_VD_SET_L, u8Temp);

        u8Temp = (REG_NR_LOAD_REG | REG_NR_BLEND_ROUND_Y | REG_NR_BLEND_ROUND_C);
        _MHal_W1B(NR_REG_VD_SET_H, u8Temp);

        //Set NR clock
        //RobertYang
        //u8Temp = (REG_NR_VD_CLK_180_PHASE | REG_NR_VD_CLK_DELAY_EN | REG_NR_EN_SNR_FROST_K_AUTO);
        u8Temp = 0;
        _MHal_W1B(NR_REG_CLK_L, u8Temp);
        _MHal_W1B(NR_REG_CLK_H, REG_NR_VD_CLK_DELAY_3UNIT);

        //Set NR output
        u8Temp = (REG_NR_OUT_VSYNC_HIGH_AVTIVE | REG_NR_OUT_HSYNC_HIGH_ACTIVE | REG_NR_OUT_TOP_FILED_HIGH_ACTIVE);
        _MHal_W1B(NR_REG_OUT_POLARITY, u8Temp);

        //Set DI
        u8Temp = (REG_DI_FMT_BLOCK | REG_DI_FMT_YC_FIELD_MODE | REG_DI_BLOCK_TOP);
        _MHal_W1B(DI_REG_MIU_CTRL, u8Temp);
        u8Temp = (REG_DI_FRAME_BASE_DI | REG_DI_MIU_WR_WHOLE_FRAME);
        _MHal_W1B(DI_REG_FRAME_BASE_SEL, u8Temp);
#endif

    }
    else
    {
        ASSERT(0);
#if 0
        //PAD input
        u8Temp = (REG_NR_SOUCE_PAD);
        // switch to PAD input
        _MHal_W1B(NR_REG_MLK_SET, u8Temp);
        //Set NR Input
        /*
         REG_NR_VSYNC_REF_PAD : refer to external PAD_BT601_VSYNC
         REG_NR_HSYNC_REF_PAD : refer to external PAD_BT601_HSYNC
         REG_NR_SOURCE_PAD_601_FIELD : refer to external PAD_BT601_FIELD
         REG_NR_VSYNC_HIGH_ACTIVE : Vsync, High active for Active Zone
         REG_NR_HSYNC_HIGH_ACTIVE : Hsync, High active for Active Zone \
        */
        u8Temp = (REG_NR_VSYNC_REF_PAD | REG_NR_HSYNC_REF_PAD | REG_NR_SOURCE_PAD_601_FIELD |
                  REG_NR_VSYNC_HIGH_ACTIVE | REG_NR_HSYNC_HIGH_ACTIVE);
        _MHal_W1B(NR_REG_IN_CTRL_L, u8Temp);

        /*
         REG_NR_TOP_FIELD_HIGH_ACTIVE : High active for Top Field
         REG_NR_VSTART_FRAME_END : Vertical Operation Start reference
                                    # 1'b0 : refer to frame start
                                    # 1'b1 : refer to frame end
         REG_NR_HSTART_LINE_END : Horizontal Operation Start reference
                                    # 1'b0 : refer to line start
                                    # 1'b1 : refer to line end
        */
        u8Temp = (REG_NR_TOP_FIELD_HIGH_ACTIVE | REG_NR_VSTART_FRAME_END | REG_NR_HSTART_LINE_END);
        _MHal_W1B(NR_REG_IN_CTRL_H, u8Temp);

        u8Temp = (REG_NR_VD_CCIR_DDR_4BIT | REG_NR_YUV_ORDER_YCYC | REG_NR_VD_DDR_ORDER_POSITVE_MSB);
        _MHal_W1B(NR_REG_VD_SET_L, u8Temp);

        u8Temp = (REG_NR_LOAD_REG | REG_NR_BLEND_ROUND_Y | REG_NR_BLEND_ROUND_C);
        _MHal_W1B(NR_REG_VD_SET_H, u8Temp);

        //Set NR clock
        u8Temp = (REG_NR_VD_CLK_180_PHASE | REG_NR_VD_CLK_DELAY_EN | REG_NR_EN_SNR_FROST_K_AUTO);
        _MHal_W1B(NR_REG_CLK_L, u8Temp);
        _MHal_W1B(NR_REG_CLK_H, REG_NR_VD_CLK_DELAY_3UNIT);

        //Set NR output
        u8Temp = (REG_NR_OUT_VSYNC_HIGH_AVTIVE | REG_NR_OUT_HSYNC_HIGH_ACTIVE | REG_NR_OUT_TOP_FILED_HIGH_ACTIVE);
        _MHal_W1B(NR_REG_OUT_POLARITY, u8Temp);

        //Set DI
        u8Temp = (REG_DI_FMT_BLOCK | REG_DI_FMT_YC_FIELD_MODE | REG_DI_BLOCK_TOP);
        _MHal_W1B(DI_REG_MIU_CTRL, u8Temp);
        u8Temp = (REG_DI_FRAME_BASE_DI | REG_DI_MIU_WR_WHOLE_FRAME);
        _MHal_W1B(DI_REG_FRAME_BASE_SEL, u8Temp);
#endif
    }
    return DIP_ERR_OK;
}

DIP_ERRCODE MHal_DIP_SetNRBuff(U8 u8BufCnt, U32 u32BufWidth, U32 u32BufHeight,
                               U32 u32BufStart, U32 u32BufEnd)
{
    U32 u32Temp = 0;
    U8 i;
    DIP_DBG(KDBG("====MHal_DIP_SetNRBuff====\n"));
    DIP_DBG(KDBG("Buffer count: %d, width: %d, height: %d.\n", u8BufCnt, u32BufWidth, u32BufHeight));
    DIP_DBG(KDBG("Buffer address start: 0x%x, end: 0x%x.\n", u32BufStart, u32BufEnd));

    // Auto gen buffer
    if(u8BufCnt == 0xFF)
    {
        #if 0
        u8BufCnt = 1;
        u32BufWidth = 720;
        u32BufHeight = 480;
        u32BufStart = (U32)phys_to_virt((U32)DIP_NR_BUF_ADR);
        u32BufEnd= (U32)phys_to_virt((U32)(DIP_NR_BUF_ADR+DIP_NR_BUF_LEN));
        #endif

        return  DIP_ERR_INVALID_BUFFER_CNT;
    }

    if (u8BufCnt > MAX_NR_BUFF_CNT)
    {
        return DIP_ERR_INVALID_BUFFER_CNT;
    }
    // T3 DIP need 16 bytes align.
    //if (u32BufStart % 8)
    if (u32BufStart % 16)
    {
        return DIP_ERR_INVALID_BUFFER_START;
    }
    if ((u32BufWidth % 16) || (u32BufWidth > MAX_BUFF_WIDTH))
    {
        return DIP_ERR_INVALID_BUFFER_WIDTH;
    }
    //if ((u32BufHeight % 16) || (u32BufHeight > MAX_BUFF_HEIGHT))
   if ((u32BufHeight > MAX_BUFF_HEIGHT))
    {
        return DIP_ERR_INVALID_BUFFER_HEIGHT;
    }
    u32Temp = u32BufWidth*u32BufHeight*(u8BufCnt*2+1);
    if (u32Temp > (u32BufEnd - u32BufStart))
    {
        return DIP_ERR_INVALID_BUFFER_SZIE;
    }

    // NR BUFFER
    // |--> NrRatioBuf
    // *
    // [NR Ration = u32BufWidth*u32BufHeight] [NR = u32BufWidth*u32BufHeight*2] [ ... ]
    //                                         *                                *
    //                                         |--> NrDataBuff[0]               |-->NrDataBuff[1]
    //

    //Initial data buffer count, buffer width, buffer count
    u8NrBuffCnt = u8BufCnt;
    u32NrBuffWidth = u32BufWidth;
    u32NrBuffHeight = u32BufHeight;
    //Initial ratio buffer address
    u32NrRatioBuff = u32BufStart;
    DIP_DBG(KDBG("Ratio Buff start: 0x%x.\n", u32NrRatioBuff));
    u32BufStart += u32BufWidth*u32BufHeight;// 1 Pixel 1 Byte in Motion Ratio
    //Initial data buffer address
    u32Temp = u32BufWidth*u32BufHeight*2;
    for (i=0; i<u8NrBuffCnt; i++)
    {
        u32NrDataBuff[i] = u32BufStart;
        u32BufStart += u32Temp;
        DIP_DBG(KDBG("NR buffer %d, start: 0x%x.\n", i, u32NrDataBuff[i]));
    }


    //MDrv_RIU_WriteU32(NR_REG_RATIO_START_0, u32NrRatioBuff>>3);

    //Set ratio buffer start & limitation (for T3 need 16 Bytes Align)
    _MHal_W2B(NR_REG_RATIO_START_0, ((u32NrRatioBuff&~0xE0000000)>>4)&0xFFFF);
    _MHal_W2B(NR_REG_RATIO_START_2, ((u32NrRatioBuff&~0xE0000000)>>4)>>16);

    //MDrv_RIU_WriteU32(NR_REG_RATIO_END_0, ((u32NrRatioBuff + u32BufWidth*u32BufHeight)>>3));
    _MHal_W2B(NR_REG_RATIO_END_0, (((u32NrRatioBuff + u32BufWidth*u32BufHeight)&~0xE0000000)>>4)&0xFFFF);
    _MHal_W2B(NR_REG_RATIO_END_2, (((u32NrRatioBuff + u32BufWidth*u32BufHeight)&~0xE0000000)>>4)>>16);

    //Set data buffer start & limitation
    //MDrv_RIU_WriteU32(NR_REG_DATA_START_0, u32NrDataBuff[0]>>3);
    _MHal_W2B(NR_REG_DATA_START_0, ((u32NrDataBuff[0]&~0xE0000000)>>4)&0xFFFF);
    _MHal_W2B(NR_REG_DATA_START_2, ((u32NrDataBuff[0]&~0xE0000000)>>4)>>16);


    //MDrv_RIU_WriteU32(NR_REG_DATA_END_0, ((u32NrDataBuff[0] + u32Temp)>>3));
    _MHal_W2B(NR_REG_DATA_END_0, (((u32NrDataBuff[0] + u32Temp)&~0xE0000000)>>4)&0xFFFF);
    _MHal_W2B(NR_REG_DATA_END_2, (((u32NrDataBuff[0] + u32Temp)&~0xE0000000)>>4)>>16);


    //If data buffer count > 1, set buffer offset and buffer count
    if (u8NrBuffCnt > 1)
    {
        _MHal_W1B(NR_REG_DATA_CNT, u8NrBuffCnt-1);
        //MDrv_RIU_WriteU32(NR_REG_DATA_SIZE_0, u32Temp>>2);//Unit is 4 byte
        _MHal_W2B(NR_REG_DATA_SIZE_0, (u32Temp>>2)&0xFFFF);//Unit is 4 byte
        _MHal_W2B(NR_REG_DATA_SIZE_2, (u32Temp>>2)>>16);//Unit is 4 byte
    }
    else
    {
        _MHal_W1B(NR_REG_DATA_CNT, 0);
        //MDrv_RIU_WriteU32(NR_REG_DATA_SIZE_0, 0);
        _MHal_W2B(NR_REG_DATA_SIZE_0, 0);
        _MHal_W2B(NR_REG_DATA_SIZE_2, 0);
    }

    //Set data & ratio buffer pitch
    _MHal_W2B(NR_REG_DATA_PITCH_L, (u32NrBuffWidth*2)>>4);//YUV422,16 byte alignment
    _MHal_W2B(NR_REG_RATIO_PITCH_L, u32NrBuffWidth>>4); // 1 Pixel 1 Byte in Motion Ratio

    //Set NR buffer flag
    u32DIPFlag |= E_SET_NR_BUFFER;

    return DIP_ERR_OK;
}

DIP_ERRCODE MHal_DIP_SetDIBuff(U8 u8BufCnt, U32 u32BufWidth, U32 u32BufHeight,
                               U32 u32BufStart, U32 u32BufEnd)
{
    U32 u32Temp = 0;
    U8 i;

    DIP_DBG(KDBG("====MHal_DIP_SetDIBuff====.\n"));
    DIP_DBG(KDBG("Buffer count: %d, width: %d, height: %d.\n", u8BufCnt, u32BufWidth, u32BufHeight));
    DIP_DBG(KDBG("Buffer address start: 0x%x, end: 0x%x.\n", u32BufStart, u32BufEnd));

    // Auto gen buffer
    if(u8BufCnt == 0xFF)
    {
        #if 0
        u8BufCnt = 5;
        u32BufWidth = 720;
        u32BufHeight = 480;
        u32BufStart = (U32)phys_to_virt((U32)DIP_DI_BUF_ADR);
        u32BufEnd= (U32)phys_to_virt((U32)(DIP_DI_BUF_ADR+DIP_DI_BUF_LEN));
        #endif
        return  DIP_ERR_INVALID_BUFFER_CNT;
    }

#if ENABLE_DI_BUFFER_VERIFY
    u32VerifyDiBufStartAddress = u32BufStart | 0xA0000000;
    u8VerifyDiBufCnt = u8BufCnt;
    DIP_DBG(KDBG("u32VerifyDiBufStartAddress: 0x%x, u8VerifyDiBufCnt: 0x%x.\n", u32VerifyDiBufStartAddress, u8VerifyDiBufCnt));
#endif

    if (u8BufCnt > MAX_DATA_BUFF_CNT-1)
    {
        return DIP_ERR_INVALID_BUFFER_CNT;
    }
    //if (u32BufStart % 8)
    if (u32BufStart % 16)
    {
        return DIP_ERR_INVALID_BUFFER_START;
    }
    if ((u32BufWidth % 16) || (u32BufWidth > MAX_BUFF_WIDTH))
    {
        return DIP_ERR_INVALID_BUFFER_WIDTH;
    }
    //if ((u32BufHeight % 16) || (u32BufHeight > MAX_BUFF_HEIGHT))
    if ((u32BufHeight > MAX_BUFF_HEIGHT))
    {
        return DIP_ERR_INVALID_BUFFER_HEIGHT;
    }
    u32Temp = (u32BufWidth*u32BufHeight*3*u8BufCnt)>>1;
    if (u32Temp > (u32BufEnd - u32BufStart))
    {
        return DIP_ERR_INVALID_BUFFER_SZIE;
    }

    // DI BUFFER
    // |--> sDiDataBuff[0]                                                      |--> sDiDataBuff[1]
    // *                                                                        *
    // [DI Y = u32BufWidth*u32BufHeight | DI UV= 0.5*u32BufWidth*u32BufHeight ] [ ... ]
    //

    //Initial data buffer count, buffer width, buffer count
    u8DiBuffCnt = u8BufCnt;
    u32DiBuffWidth = u32BufWidth;
    u32DiBuffHeight = u32BufHeight;
    //Initial YUV420 buffer address , 4Pixels 6 Bytes,1Pixel 1.5 Byte
    u32Temp = (u32BufWidth*u32BufHeight*3)>>1;
    for (i=0; i<u8DiBuffCnt; i++)
    {
        sDiDataBuff[i].u32YStart = u32BufStart;
        sDiDataBuff[i].u32UVStart = (u32BufStart + u32BufWidth*u32BufHeight); // Y size is u32BufWidth*u32BufHeight
        u32BufStart += u32Temp;
        #if FRAME_CHECK
        MHal_DIP_SetIdentifyInfo(i);
        #endif
        DIP_DBG(KDBG("DI buffer %d, Y star: 0x%x, UV strat: 0x%x.\n", i, sDiDataBuff[i].u32YStart, sDiDataBuff[i].u32UVStart));
    }

    //Set Ni Y & C buffer start (16 bytes align)
    //MDrv_RIU_WriteU32(DI_REG_FRAME_Y_START_0, sDiDataBuff[0].u32YStart>>3);
    _MHal_W2B(DI_REG_FRAME_Y_START_0, ((sDiDataBuff[0].u32YStart&~0xE0000000)>>3) & 0xFFFF);
    _MHal_W2B(DI_REG_FRAME_Y_START_2, ((sDiDataBuff[0].u32YStart&~0xE0000000)>>3)>>16);

    //MDrv_RIU_WriteU32(DI_REG_FRAME_C_START_0, sDiDataBuff[0].u32UVStart>>3);
    _MHal_W2B(DI_REG_FRAME_C_START_0, ((sDiDataBuff[0].u32UVStart&~0xE0000000)>>3)&0xFFFF);
    _MHal_W2B(DI_REG_FRAME_C_START_2, ((sDiDataBuff[0].u32UVStart&~0xE0000000)>>3)>>16);

    //Set Ni Y & C buffer end
    //MDrv_RIU_WriteU32(DI_REG_FRAME_Y_END_0, (sDiDataBuff[0].u32YStartu+(u32BufWidth*u32BufHeight))>>3);
    //MDrv_RIU_WriteU32(DI_REG_FRAME_C_END_0, (sDiDataBuff[0].u32UVStart+(u32BufWidth*u32BufHeight>>1)>>3);

    //Set Y & C buffer pitch
    //_MHal_W2B(DI_REG_FRAME_Y_PITCH, u32DiBuffWidth>>3);//u32DiBuffWidth 720 set 90 //Unit is 4 Byte
    //_MHal_W2B(DI_REG_FRAME_C_PITCH, u32DiBuffWidth>>3);//u32DiBuffWidth 720 set 90//Unit is 4 Byte

    _MHal_W2B(DI_REG_FRAME_Y_PITCH, u32DiBuffWidth>>3);//u32DiBuffWidth 720 set 90 //Unit is 4 Byte
    _MHal_W2B(DI_REG_FRAME_C_PITCH, u32DiBuffWidth>>3);//u32DiBuffWidth 720 set 90//Unit is 4 Byte


    //Set H Dimension ,Counter by pixel
    //Set V Dimension ,Between 2 vsync
    //_MHal_W2B(DI_REG_H_DIMENSION, 675);//720
    _MHal_W2B(DI_REG_H_DIMENSION, 672);//720
    //_MHal_W2B(DI_REG_H_DIMENSION, u32DiBuffWidth);//720
    //_MHal_W2B(DI_REG_V_DIMENSION, u32DiBuffHeight>>1);//240
    _MHal_W2B(DI_REG_V_DIMENSION, u32DiBuffHeight>>1);//240

    _MHal_W1BM(DI_REG_EXT_FB, REG_DI_INVERSE_FIELD | REG_DI_INVERSE_HSYNC | REG_DI_COUNT_FROM_1, REG_DI_INVERSE_FIELD | REG_DI_INVERSE_HSYNC | REG_DI_COUNT_FROM_1);
    //Set extern buffer offset and buffer count.
    if (u8DiBuffCnt > 1)
    {
        _MHal_W1BM(DI_REG_EXT_FB, (u8DiBuffCnt-1), REG_DI_EXT_FB_CNT_MASK);
        //MDrv_RIU_WriteU32(DI_REG_EXT_Y_OFFSET_0, (u32BufWidth*u32BufHeight*3/2)>>3);//Y start offset between two buffer
        _MHal_W2B(DI_REG_EXT_Y_OFFSET_0, ((u32BufWidth*u32BufHeight*3/2)>>3)&0xFFFF);//Y start offset between two buffer
        _MHal_W2B(DI_REG_EXT_Y_OFFSET_2, ((u32BufWidth*u32BufHeight*3/2)>>3)>>16);//Y start offset between two buffer

        //MDrv_RIU_WriteU32(DI_REG_EXT_C_OFFSET_0, (u32BufWidth*u32BufHeight*3/2)>>3);//C start offset between two buffer
        _MHal_W2B(DI_REG_EXT_C_OFFSET_0, ((u32BufWidth*u32BufHeight*3/2)>>3)&0xFFFF);//C start offset between two buffer
        _MHal_W2B(DI_REG_EXT_C_OFFSET_2, ((u32BufWidth*u32BufHeight*3/2)>>3)>>16);//C start offset between two buffer
    }
    else
    {
        _MHal_W1BM(DI_REG_EXT_FB, 0, REG_DI_EXT_FB_CNT_MASK);
        //MDrv_RIU_WriteU32(DI_REG_EXT_Y_OFFSET_0, 0);
        _MHal_W2B(DI_REG_EXT_Y_OFFSET_0, 0);
        _MHal_W2B(DI_REG_EXT_Y_OFFSET_2, 0);

        //MDrv_RIU_WriteU32(DI_REG_EXT_C_OFFSET_0, 0);
        _MHal_W2B(DI_REG_EXT_C_OFFSET_0, 0);
        _MHal_W2B(DI_REG_EXT_C_OFFSET_0, 0);
    }

    //Set DI buffer flag
    u32DIPFlag |= E_SET_DI_BUFFER;

    return DIP_ERR_OK;
}

U8* MHal_DIP_GetHistogramOut(void)
{
    U8 i = 0;
    U16 u16Tmp = 0;
    for (i=0; i<8; i++)
    {
        u16Tmp = _MHal_R2B(DI_REG_HIST_OUT_00 + i*2);
        u8HistOut[i*2] = (U8)(u16Tmp & 0xFF);
        u8HistOut[i*2+1] = (U8)((u16Tmp & 0xFF00)>>8);
    }
    return u8HistOut;
}

U8* MHal_DIP_GetHistogramDiff(void)
{
    U8 i = 0;
    U16 u16Tmp = 0;
    for (i=0; i<8; i++)
    {
        u16Tmp = _MHal_R2B(DI_REG_HIST_DIFF_00 + i*2);
        u8HistDiff[i*2] = (U8)(u16Tmp & 0xFF);
        u8HistDiff[i*2+1] = (U8)((u16Tmp & 0xFF00)>>8);
    }
    return u8HistDiff;
}

DIP_ERRCODE MHal_DIP_EnableNRandDI(BOOL bEnableNR, BOOL bEnableSNR, BOOL bEnableTNR, BOOL bEnableDI)
{
    U16 u16Tmp = 0;

    atomic_set(&DIPEnabling, _DIPESETTING);

    u8WriteField = 0;
    u32DiBufferFrameCount = 0;
    u32DiBufferFieldCount =0;
    u16DiBuffStatus =0;
    u8DiBuffWritingIndex =0;
    u8LastDiBufferReadyTime =0;
    u8DiBufferStartOutputFlag = FALSE;
/*
    //Mask un-used DIP int
    MDrv_WriteByte(NR_REG_IRQ_MASK_7_0, 0xFF);//Status Output 1:Close 0:Open
    MDrv_WriteByte(NR_REG_IRQ_MASK_15_8, 0xFF);
    MDrv_WriteByte(NR_REG_IRQ_MASK_FORCE_19_16, 0x0B);//open bit 18

    //Reister ISR
    DIP_ISR_Control(TRUE, DIP_HandleIsr);
*/

    if (bEnableNR)
    {
        if (bEnableSNR)
        {
            u32DIPFlag |= E_ENABLE_SNR;
            //MDrv_WriteByteMask(NR_REG_CTRL_L, (REG_NR_SNR_EN | REG_NR_SNR_LINE_BUFF_EN), (REG_NR_SNR_EN | REG_NR_SNR_LINE_BUFF_EN));
            u16Tmp |= (REG_NR_SNR_EN | REG_NR_SNR_LINE_BUFF_EN);
        }
        else
        {
            u32DIPFlag &= (~E_ENABLE_SNR);
            _MHal_W1BM(NR_REG_CTRL_L, 0, (REG_NR_SNR_EN | REG_NR_SNR_LINE_BUFF_EN));
        }
        if (bEnableTNR)
        {
            u32DIPFlag |= E_ENABLE_TNR;
            //MDrv_WriteByteMask(NR_REG_CTRL_L, (REG_NR_TNR_RD_DATA_EN | REG_NR_TNR_RD_RATIO_EN), (REG_NR_TNR_RD_DATA_EN | REG_NR_TNR_RD_RATIO_EN));
            //MDrv_WriteByteMask(NR_REG_CTRL_H, (REG_NR_TNR_ADJUST_RATIO_KY_EN | REG_NR_TNR_ADJUST_RATIO_KC_EN | REG_NR_TNR_EN),
            //    (REG_NR_TNR_ADJUST_RATIO_KY_EN | REG_NR_TNR_ADJUST_RATIO_KC_EN | REG_NR_TNR_EN));
            u16Tmp |= (REG_NR_TNR_RD_DATA_EN | REG_NR_TNR_RD_RATIO_EN);
            u16Tmp |= ((REG_NR_TNR_ADJUST_RATIO_KY_EN | REG_NR_TNR_ADJUST_RATIO_KC_EN | REG_NR_TNR_EN)<<8);
        }
        else
        {
            u32DIPFlag &= (~E_ENABLE_TNR);
            _MHal_W1BM(NR_REG_CTRL_L, 0, (REG_NR_TNR_RD_DATA_EN | REG_NR_TNR_RD_RATIO_EN));
            _MHal_W1BM(NR_REG_CTRL_H, 0, (REG_NR_TNR_ADJUST_RATIO_KY_EN | REG_NR_TNR_ADJUST_RATIO_KC_EN | REG_NR_TNR_EN));
        }
        u32DIPFlag |= E_ENABLE_NR;

        DIP_DBG(KDBG("Enable NR: 0x%x\n", u16Tmp));
/*
        MDrv_WriteByteMask(NR_REG_CTRL_H, (REG_NR_DATA_ACCESS_MIU_EN | REG_NR_RATIO_ACCESS_MIU_EN | REG_NR_MOTION_HISTORY_EN),
            (REG_NR_DATA_ACCESS_MIU_EN | REG_NR_RATIO_ACCESS_MIU_EN | REG_NR_MOTION_HISTORY_EN));
        DIP_DBG(KDBG("Enable NR 1 : 0x%x.\n", MDrv_Read2Byte(NR_REG_CTRL_L)));
        MDrv_WriteByteMask(NR_REG_CTRL_L, (REG_NR_MIU_WR_DATA_EN | REG_NR_MIU_WR_RATIO_EN | REG_NR_EN),
            (REG_NR_MIU_WR_DATA_EN | REG_NR_MIU_WR_RATIO_EN | REG_NR_EN));
        DIP_DBG(KDBG("Enable NR 2 : 0x%x.\n", MDrv_Read2Byte(NR_REG_CTRL_L)));
*/
        //RobertYang
        //u16Tmp |= ((REG_NR_DATA_ACCESS_MIU_EN | REG_NR_RATIO_ACCESS_MIU_EN | REG_NR_MOTION_HISTORY_EN)<<8);
        u16Tmp |= ((REG_NR_DATA_ACCESS_MIU_EN | REG_NR_RATIO_ACCESS_MIU_EN)<<8);
        u16Tmp |= (REG_NR_MIU_WR_DATA_EN | REG_NR_MIU_WR_RATIO_EN | REG_NR_EN);
        #if 0   // disable NR
            u16Tmp&=~REG_NR_EN;
        #endif
        DIP_DBG(KDBG("Enable NR 1: 0x%x\n", u16Tmp));
        _MHal_W2B(NR_REG_CTRL_L, u16Tmp);

        DIP_DBG(KDBG("Enable NR 2: 0x%x\n", _MHal_R2B(NR_REG_CTRL_L)));

        _MHal_W1BM(NR_REG_IN_CTRL_L, REG_NR_SW_RSTZ, REG_NR_SW_RSTZ);
        _MHal_W1B(NR_REG_IN_CTRL_L, 0xDD);

        u16Tmp = (0x0);
        DIP_DBG(KDBG("Enable NR-HSD Factor (L)%d\n", u16Tmp));
        _MHal_W2B(NR_HSD_REG(0x0A), u16Tmp);
        u16Tmp = (0x02<<4);
        DIP_DBG(KDBG("Enable NR-HSD Factor (H)%d\n", u16Tmp));
        _MHal_W2B(NR_HSD_REG(0x0B), u16Tmp);

        u16Tmp = BIT0|BIT1;
        DIP_DBG(KDBG("Enable NR-HSD\n"));
        _MHal_W2B(NR_HSD_REG(0x00), u16Tmp);

    }
    else
    {
        u32DIPFlag &= (~(E_ENABLE_SNR | E_ENABLE_TNR | E_ENABLE_NR));
        _MHal_W2B(NR_REG_CTRL_L, 0);
        _MHal_W1BM(NR_REG_IN_CTRL_L, 0, REG_NR_SW_RSTZ);
    }

    if (bEnableDI)
    {
        //_MHal_W1BM(DI_REG_MIU_CTRL, (REG_DI_MIU_EN | REG_DI_MIU_WR_EN | REG_DI_SWAP_YC), (REG_DI_MIU_EN | REG_DI_MIU_WR_EN | REG_DI_SWAP_YC));

        _MHal_W1BM(DI_REG_MIU_CTRL, BIT0, BIT0);
        _MHal_W2BM(DI_REG(0x0), 0xC000, 0xF000);
        _MHal_W2BM(DI_REG(0x1), 0x0080, 0x0080);

        u32DIPFlag |= E_ENABLE_DI;

        #if DI_BUFFER_SKIP_CHECK
            EnableDiBufferTime = jiffies;
            FirstDiBufferDoneTime=0;
            SecondDiBufferDoneTime =0;
        #endif

        #if ENABLE_DI_BUFFER_VERIFY
            DiBufferVerifyStart = 1;
        #endif
        #if DI_BUFFER_SKIP_CHECK
            for(u16Tmp=0;u16Tmp<ChECK_NUMBER;u16Tmp++)
            {
               IntervalRecord[u16Tmp] = 0;
            }
            for(u16Tmp=0;u16Tmp<10;u16Tmp++)
            {
                ErrorIntervalRecord[u16Tmp] = 0;
            }

            u32DiBufferFieldCount =0;
            u32DiBufferErrorFieldCount =0;
            u8LastDiBufferReadyTime = 0;
        #endif
    }
    else
    {
        //_MHal_W1BM(DI_REG_MIU_CTRL, 0, (REG_DI_MIU_EN | REG_DI_MIU_WR_EN));
        _MHal_W1BM(DI_REG_MIU_CTRL, 0, BIT0);
        _MHal_W2BM(DI_REG(0x00), 0xC000, 0xF000);
        u32DIPFlag &= (~E_ENABLE_DI);

        #if ENABLE_DI_BUFFER_VERIFY
            DiBufferVerifyStart = 0;
        #endif
        #if DI_BUFFER_SKIP_CHECK
            for(u16Tmp=0;u16Tmp<ChECK_NUMBER;u16Tmp++)
            {
                KDBG("IntervalRecord %d =%d \n",u16Tmp,IntervalRecord[u16Tmp]);
            }
            for(u16Tmp=0;u16Tmp<10;u16Tmp++)
            {
                KDBG("ErrorIntervalRecord %d =%d \n",u16Tmp,ErrorIntervalRecord[u16Tmp]);
            }
            KDBG("EnableDiBufferTime =%x \n",EnableDiBufferTime);
            KDBG("FirstDiBufferDoneTime =%x \n",FirstDiBufferDoneTime);
            KDBG("SecondDiBufferDoneTime =%x \n",SecondDiBufferDoneTime);
            KDBG("SKIP_FIELD_NUMBER =%d \n",SKIP_FIELD_NUMBER);
            KDBG("u32DiBufferErrorFieldCount =%d \n",u32DiBufferErrorFieldCount);
            KDBG("u32DiBufferFieldCount =%d \n",u32DiBufferFieldCount);
        #endif
    }

    if (bEnableNR && bEnableDI)
    {
        //Mask un-used DIP int
        _MHal_W1B(NR_REG_IRQ_MASK_7_0, REG_IRQ_MASK_ALL);//Status Output 1:Close 0:Open
        _MHal_W1B(NR_REG_IRQ_MASK_15_8, REG_IRQ_MASK_ALL);
        _MHal_W1B(NR_REG_IRQ_MASK_19_16, 0xFB);//open bit 17 18

        //Reister ISR
        DIP_ISR_Control(TRUE, DIP_HandleIsr);

        atomic_set(&DIPEnabling, _DIPENABLE);
    }
    else
    {
        atomic_set(&DIPEnabling, _DIPDISABLE);

        //Mask All DIP int
        _MHal_W1B(NR_REG_IRQ_MASK_7_0, REG_IRQ_MASK_ALL);
        _MHal_W1B(NR_REG_IRQ_MASK_15_8, REG_IRQ_MASK_ALL);
        _MHal_W1B(NR_REG_IRQ_MASK_19_16, 0xFF);

        //Un-reister ISR
        DIP_ISR_Control(FALSE, NULL);
    }

    return DIP_ERR_OK;
}

U8 MHal_DIP_GetDiBuffCount(void)
{
    return u8DiBuffCnt;
}

dip_DIbufinfo_t* MHal_DIP_GetDiBuffInfo(void)
{
    return (dip_DIbufinfo_t*)sDiDataBuff;
}

U16 MHal_DIP_GetDiBuffStatus(void)
{
    return u16DiBuffStatus;
}

DIP_ERRCODE MHal_DIP_ClearDiBuffStatus(U8 BufferIndex)// BufferIndex from 0 ~ (u8DiBuffCnt-1)
{
    unsigned long flags;

    spin_lock_irqsave(&lock_dip, flags);

    u16DiBuffStatus &= ~(0x01<<BufferIndex);

    spin_unlock_irqrestore(&lock_dip, flags);

#if FRAME_CHECK
    //MHal_DIP_SetIdentifyInfo(BufferIndex);
#endif
    return DIP_ERR_OK;
}

U32 MHal_DIP_GetDiBuffFrameCount(void)
{
    return u32DiBufferFrameCount;
}

/*****Below is for Test Only*****/

#if ENABLE_DI_BUFFER_VERIFY

void MHal_DIP_SetDIVerBuff(U32 u32BufStart)
{
    U32 i;
    u32BufStart = 0xA5A00000;

    OutBuf =(U8*)(u32BufStart);
    memset(OutBuf, 0x0, 0x200000);


    BufferIndex = 5;

    u32BufStart = 0xA5C00000;
    Y_SequencyBuf = (U8*)(u32BufStart + (720*480*2));
    U_SequencyBuf = (U8*)(u32BufStart + (720*480*2) + (720*480) );
    V_SequencyBuf = (U8*)(u32BufStart + (720*480*2) + (720*480)+(360*240) );

    // Green Y 0x72 U 0x48 V 0x3B
    // Red   Y 0x3B U 0x71 V 0xD0
    // Blue  Y 0x18 U 0xcD V 0x7A
    for( i=0;i<(720*480);i++)
        Y_SequencyBuf[i] = 0x80;
    for( i=0;i<(360*240);i++)
        U_SequencyBuf[i] = 0x36;
    for( i=0;i<(360*240);i++)
        V_SequencyBuf[i] = 0x22;
}

U8 MHal_DIP_GetDiBufferVerifyStart(void)
{
    return DiBufferVerifyStart;
}

//static U8 Yvalue = 0x72;
void MHal_DIP_SequenceYUVBlockToYUV422(void)
{
    U32 i,j;
    U32 InputPos,OutputPos;
    BOOL UVflag=TRUE;//U Output First // U:True V:False

    //MDrv_UART0_Switch(UART_SWITCH_AEON, TRUE);
    //KDBG("====MDrv_DIP_SequenceYUVBlockToYUV422====.\n");
    //MDrv_UART0_Switch(UART_SWITCH_51, TRUE);

#if 0
    Yvalue ++;
    if( Yvalue==1 )//Red
    {
        for( i=0;i<(720*480);i++)
            Y_SequencyBuf[i] = 0x3B;
        for( i=0;i<(360*240);i++)
            U_SequencyBuf[i] = 0x71;
        for( i=0;i<(360*240);i++)
            V_SequencyBuf[i] = 0xD0;
    }
    else if( Yvalue==2 )//Green
    {
        for( i=0;i<(720*480);i++)
            Y_SequencyBuf[i] = 0x72;
        for( i=0;i<(360*240);i++)
            U_SequencyBuf[i] = 0x3B;
        for( i=0;i<(360*240);i++)
            V_SequencyBuf[i] = 0x48;
    }
    else//Blue
    {   Yvalue =0;
        for( i=0;i<(720*480);i++)
            Y_SequencyBuf[i] = 0x18;
        for( i=0;i<(360*240);i++)
            U_SequencyBuf[i] = 0xCD;
        for( i=0;i<(360*240);i++)
            V_SequencyBuf[i] = 0x7A;
    }
#endif
    for( i=0; i<480 ;i++)
    {
        for( j=0; j<720*2 ;j++) //1Pixel 2Byte
        {
            OutputPos = (i*720*2)+j;
            if( j % 2 ==0 )//Y Ouput
            {
                InputPos = i*720+j/2;

                //OutBuf[OutputPos] = Y_SequencyBuf[InputPos];
                OutBuf[OutputPos] = Y_SequencyBuf[InputPos];
            }
            else//UV Output
            {
                //InputPos = (i/2*720/2)+(j/4+1);
                InputPos = ((i/2)*(360))+ (j/4);
                if( UVflag == TRUE )
                {
                    OutBuf[OutputPos] = U_SequencyBuf[InputPos];
                    //OutBuf[OutputPos] = 0x80;
                    UVflag = FALSE;
                }
                else
                {
                    OutBuf[OutputPos] = V_SequencyBuf[InputPos];
                    //OutBuf[OutputPos] = 0x80;
                    UVflag = TRUE;
                }
            }
        }
    }
}
void MHal_DIP_TailYUV420ToSequenceYUVBlock1(void)
{
    U32 sed, i, j;
    U32 Y_H_Block,Y_V_Block;

    U32 YLineAddr[720];
    U32 UVLineAddr[360];
    U32 VIndx[480];
    U32 VCIndx[240];
    U8* V_TialBuf;
    if( BufferIndex >= u8VerifyDiBufCnt )
        BufferIndex = 0;

    Y_TailBuf = (U8*)(u32VerifyDiBufStartAddress+((U32)BufferIndex*(720*480*3/2)));
    UV_TialBuf = (U8*)((u32VerifyDiBufStartAddress+720*480)+((U32)BufferIndex*(720*480*3/2)));
    V_TialBuf = (U8*)((u32VerifyDiBufStartAddress+720*480)+360*240 + ((U32)BufferIndex*(720*480*3/2)));


    sed = 0x0;
    for(i=0; i<360; i++)
    {
        YLineAddr[i*2] = sed;
        sed = sed + 0x08;
        YLineAddr[i*2+1] = sed;
        sed = sed + 0x08;
    }

    sed = 0x0;
    for(i=0; i<180; i++)
    {
        UVLineAddr[i*2] = sed;
        sed = sed + 0x08;
        UVLineAddr[i*2+1] = sed;
        sed = sed + 0x08;
    }


    sed = 0x0;
    for(i=0; i<480/8; i++)
    {
        VIndx[i*8+0] = sed;
        VIndx[i*8+1] = sed+1;
        VIndx[i*8+2] = sed+2;
        VIndx[i*8+3] = sed+3;
        VIndx[i*8+4] = sed+4;
        VIndx[i*8+5] = sed+5;
        VIndx[i*8+6] = sed+6;
        VIndx[i*8+7] = sed+7;
        sed = sed + 0x08*2;
    }

    sed = 0x0;
    for(i=0; i<240/8; i++)
    {
        VCIndx[i*8+0] = sed;
        VCIndx[i*8+1] = sed+1;
        VCIndx[i*8+2] = sed+2;
        VCIndx[i*8+3] = sed+3;
        VCIndx[i*8+4] = sed+4;
        VCIndx[i*8+5] = sed+5;
        VCIndx[i*8+6] = sed+6;
        VCIndx[i*8+7] = sed+7;
        sed = sed + 0x8;
    }

    //BufferIndex++;
    Y_H_Block = 720;
    Y_V_Block = (480);
    for(j=0;j<Y_V_Block;j++)
    {
        for(i=0;i<Y_H_Block;i++)
        {
            Y_SequencyBuf[i+720*j] = Y_TailBuf[YLineAddr[i]+VIndx[j]];
        }
    }
#if 0
    Y_H_Block = 360;
    Y_V_Block = 240;
    for(j=0;j<Y_V_Block;j++)
    {
        for(i=0;i<Y_H_Block;i++)
        {
            U_SequencyBuf[i+360*j] = UV_TialBuf[UVLineAddr[i]+VCIndx[j]];
        }
    }

    Y_H_Block = 360;
    Y_V_Block = 240;
    for(j=0;j<Y_V_Block;j++)
    {
        for(i=0;i<Y_H_Block;i++)
        {
            V_SequencyBuf[i+360*j] = V_TialBuf[UVLineAddr[i]+VCIndx[j]];
        }
    }
#endif
}

void MHal_DIP_TailYUV420ToSequenceYUVBlock(void)
{
    U32 i,j,sub_i,sub_j;
    U32 Y_H_Block,Y_V_Block;
    U32 UV_H_Block,UV_V_Block;
    U32 BlockStartPos=0,SequencePos=0,TailPos=0;

    if( BufferIndex >= u8VerifyDiBufCnt )
        BufferIndex = 0;

    Y_TailBuf = (U8*)(u32VerifyDiBufStartAddress+((U32)BufferIndex*(720*480*3/2)));
    UV_TialBuf = (U8*)((u32VerifyDiBufStartAddress+720*480)+((U32)BufferIndex*(720*480*3/2)));

    //BufferIndex++;

    Y_H_Block = (720/16);
    Y_V_Block = (480/16);
    TailPos=0;

    for(i=0;i<Y_V_Block;i++)
        for(j=0;j<Y_H_Block;j++)
    {
        {
            //Y First Block Size 8*8 0f 16*16
            BlockStartPos = i*(Y_H_Block*256)+(j*16);
            for(sub_i=0;sub_i<8;sub_i++)
            {
                SequencePos = BlockStartPos + sub_i*720;
                for(sub_j=0;sub_j<8;sub_j++)
                {
                    Y_SequencyBuf[SequencePos+sub_j] = Y_TailBuf[TailPos];
                    TailPos++;
                }
            }

            //Y Second Block Size 8*8 0f 16*16
            BlockStartPos = i*(Y_H_Block*256)+(j*16)+8;
            for(sub_i=0;sub_i<8;sub_i++)
            {
                SequencePos = BlockStartPos + sub_i*720;
                for(sub_j=0;sub_j<8;sub_j++)
                {
                    Y_SequencyBuf[SequencePos+sub_j] = Y_TailBuf[TailPos];
                    TailPos++;
                }
            }

            //Y Third Block Size 8*8 0f 16*16
            BlockStartPos = (i*Y_H_Block*256)+(j*16)+(720*8);
            for(sub_i=0;sub_i<8;sub_i++)
            {
                SequencePos = BlockStartPos + sub_i*720;
                for(sub_j=0;sub_j<8;sub_j++)
                {
                    Y_SequencyBuf[SequencePos+sub_j] = Y_TailBuf[TailPos];
                    TailPos++;
                }
            }

            //Y Fourth Block Size 8*8 0f 16*16
            BlockStartPos = (i*Y_H_Block*256)+(j*16)+(720*8)+8;
            for(sub_i=0;sub_i<8;sub_i++)
            {
                SequencePos = BlockStartPos + sub_i*720;
                for(sub_j=0;sub_j<8;sub_j++)
                {
                    Y_SequencyBuf[SequencePos+sub_j] = Y_TailBuf[TailPos];
                    TailPos++;
                }
            }
        }//for(j=0;j<Y_H_Block;j++)
    }//for(i=0;i<Y_V_Block;i++)


    UV_H_Block = (360/8);
    UV_V_Block = (240/8);

    TailPos=0;
    for(i=0;i<UV_V_Block;i++)
        for(j=0;j<UV_H_Block;j++)
    {
        {
            BlockStartPos = i*(UV_H_Block*64)  + (j*8);

            for(sub_i=0;sub_i<8;sub_i++)//UV Block Size is 8*8
            {
                SequencePos = BlockStartPos + sub_i*360;
                for(sub_j=0;sub_j<8;sub_j++)
                {
                    U_SequencyBuf[SequencePos+sub_j] = UV_TialBuf[TailPos];
                    TailPos++;
                }

            }

            BlockStartPos = i*(UV_H_Block*64)  + (j*8);

            for(sub_i=0;sub_i<8;sub_i++)//UV Block Size is 8*8
            {
                SequencePos = BlockStartPos + sub_i*360;
                for(sub_j=0;sub_j<8;sub_j++)
                {
                    V_SequencyBuf[SequencePos+sub_j] = UV_TialBuf[TailPos];
                    TailPos++;
                }

            }
        }
    }
}

#endif//#if ENABLE_DI_BUFFER_VERIFY

