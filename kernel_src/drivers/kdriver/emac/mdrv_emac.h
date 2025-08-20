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
/// @file   EMAC.h
/// @author MStar Semiconductor Inc.
/// @brief  EMAC Driver Interface
///////////////////////////////////////////////////////////////////////////////////////////////////


// -----------------------------------------------------------------------------
// Linux EMAC.h define start
// -----------------------------------------------------------------------------

#ifndef __DRV_EMAC_H_
#define __DRV_EMAC_H_

//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
#define USE_TASK                            1           // 1:Yes, 0:No
#define SOFTWARE_DESC_LEN                   0x600

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#if (!USE_TASK) // MEM_BASE_ADJUSTMENT ......................................
u32 RAM_ADDR_BASE                     = 0xA0000000
u32 RX_BUFFER_BASE                    = 0x00000000     // ==0xA0000000 ~~ 0xA0004000 (Max: 16 KB)
u32 RBQP_BASE                         = RX_BUFFER_SIZE //0x00004000 		// ==0xA0004000 ~~ 0xA0005FFF for MAX 1024 descriptors
u32 TX_BUFFER_BASE                    = (RX_BUFFER_SIZE+RBQP_SIZE)//0x00006000 	    // ==0xA0006000 ~~ ????????
u32 TX_SKB_BASE                       = TX_BUFFER_BASE+0x100//0x00006100
u32 RX_FRAME_ADDR                     = TX_SKB_BASE+0x600//0x00007000 	    // Software COPY&STORE one RX frame. Size is not defined.
#else // The memory allocation for TASK.
//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------
u32 RAM_ADDR_BASE = 0x00000000;     // After init, RAM_ADDR_BASE = EMAC_ABSO_MEM_BASE
u32 RX_BUFFER_BASE = 0x00000000;     // IMPORTANT: lowest 14 bits as zero.
u32 RBQP_BASE = RX_BUFFER_SIZE;//0x00004000;     // IMPORTANT: lowest 13 bits as zero.
u32 TX_BUFFER_BASE = ( RX_BUFFER_SIZE + RBQP_SIZE );//0x00006000;
u32 TX_SKB_BASE = ( RX_BUFFER_SIZE + RBQP_SIZE + 0x600 );//0x00006100;
u32 RX_FRAME_ADDR = ( RX_BUFFER_SIZE + RBQP_SIZE + 0xC00 );//0x00007000;
#endif //^MEM_BASE_ADJUSTMENT ...............................................

u8 MY_DEV[16] =
{
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};
u8 MY_MAC[6] =
{
  0x00, 0x30, 0x1B, 0xBA, 0x02, 0xDB
};
u8 PC_MAC[6] =
{
  0x00, 0x21, 0x86, 0x5C, 0xC8, 0x65
};

#ifdef INT_DELAY
u32 xoffsetValue, xReceiveNum;
#endif

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
struct rbf_t
{
    unsigned int addr;
    unsigned long size;
};

struct recv_desc_bufs
{
    char recv_buf[RX_BUFFER_SIZE];                /* must be on MAX_RBUFF_SZ boundary */
    struct rbf_t descriptors[MAX_RX_DESCR];     /* must be on sizeof (rbf_t) boundary */
};

struct EMAC_private
{
    struct net_device_stats stats;
    struct mii_if_info mii;         /* ethtool support */

    /* PHY */
    unsigned long phy_type;         /* type of PHY (PHY_ID) */
    spinlock_t lock;            /* lock for MDI interface */
    short phy_media;            /* media interface type */

    /* Transmit */
    struct sk_buff* skb;            /* holds skb until xmit interrupt completes */
    dma_addr_t skb_physaddr;        /* phys addr from pci_map_single */
    int skb_length;             /* saved skb length for pci_unmap_single */
    unsigned char retx_count;       /* resend count of tx */
    unsigned int txpkt;                 /* previous tx packet pointer */
    /* Receive */
    int rxBuffIndex;            /* index into receive descriptor list */
    struct recv_desc_bufs* dlist;       /* descriptor list address */
    struct recv_desc_bufs* dlist_phys;  /* descriptor list physical address */
};

#define ROUND_SUP_4(x) (((x)+3)&~3)

struct eth_drv_sgX
{
    u32 buf;
    u32 len;
};

struct _BasicConfigEMAC
{
    u8 connected;          // 0:No, 1:Yes    <== (20070515) Wait for Julian's reply
    u8 speed;               // 10:10Mbps, 100:100Mbps
    // ETH_CTL Register:
    u8 wes;             // 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    // ETH_CFG Register:
    u8 duplex;              // 1:Half-duplex, 2:Full-duplex
    u8 cam;                // 0:No CAM, 1:Yes
    u8 rcv_bcast;       // 0:No, 1:Yes
    u8 rlf;                // 0:No, 1:Yes receive long frame(1522)
    // MAC Address:
    u8 sa1[6];              // Specific Addr 1 (MAC Address)
    u8 sa2[6];              // Specific Addr 2
    u8 sa3[6];              // Specific Addr 3
    u8 sa4[6];              // Specific Addr 4
};
typedef struct _BasicConfigEMAC BasicConfigEMAC;

struct _UtilityVarsEMAC
{
    u32 cntChkINTCounter;
    u32 readIdxRBQP;        // Reset = 0x00000000
    u32 rxOneFrameAddr;     // Reset = 0x00000000 (Store the Addr of "ReadONE_RX_Frame")
    // Statistics Counters : (accumulated)
    u32 cntREG_ETH_FRA;
    u32 cntREG_ETH_SCOL;
    u32 cntREG_ETH_MCOL;
    u32 cntREG_ETH_OK;
    u32 cntREG_ETH_SEQE;
    u32 cntREG_ETH_ALE;
    u32 cntREG_ETH_DTE;
    u32 cntREG_ETH_LCOL;
    u32 cntREG_ETH_ECOL;
    u32 cntREG_ETH_TUE;
    u32 cntREG_ETH_CSE;
    u32 cntREG_ETH_RE;
    u32 cntREG_ETH_ROVR;
    u32 cntREG_ETH_SE;
    u32 cntREG_ETH_ELR;
    u32 cntREG_ETH_RJB;
    u32 cntREG_ETH_USF;
    u32 cntREG_ETH_SQEE;
    // Interrupt Counter :
    u32 cntHRESP;           // Reset = 0x0000
    u32 cntROVR;            // Reset = 0x0000
    u32 cntLINK;            // Reset = 0x0000
    u32 cntTIDLE;           // Reset = 0x0000
    u32 cntTCOM;            // Reset = 0x0000
    u32 cntTBRE;            // Reset = 0x0000
    u32 cntRTRY;            // Reset = 0x0000
    u32 cntTUND;            // Reset = 0x0000
    u32 cntTOVR;            // Reset = 0x0000
    u32 cntRBNA;            // Reset = 0x0000
    u32 cntRCOM;            // Reset = 0x0000
    u32 cntDONE;            // Reset = 0x0000
    // Flags:
    u8 flagMacTxPermit;    // 0:No,1:Permitted.  Initialize as "permitted"
    u8 flagISR_INT_RCOM;
    u8 flagISR_INT_RBNA;
    u8 flagISR_INT_DONE;
    u8 flagPowerOn;        // 0:Poweroff, 1:Poweron
    u8 initedEMAC;         // 0:Not initialized, 1:Initialized.
    u8 flagRBNA;
    // Misc Counter:
    u32 cntRxFrames;        // Reset = 0x00000000 (Counter of RX frames,no matter it's me or not)
    u32 cntReadONE_RX;      // Counter for ReadONE_RX_Frame
    u32 cntCase20070806;
    u32 cntChkToTransmit;
    // Misc Variables:
    u32 mainThreadTasks;    // (20071029_CHARLES) b0=Poweroff,b1=Poweron
};
typedef struct _UtilityVarsEMAC UtilityVarsEMAC;

BasicConfigEMAC ThisBCE;
UtilityVarsEMAC ThisUVE;

typedef volatile unsigned int EMAC_REG;

struct sk_buff* Tx_SkbAddr;


#ifdef TESTING
extern void EMAC_TEST_All( void );
#endif

struct sk_buff* rx_skb[MAX_RX_DESCR];
u32 rx_abso_addr[MAX_RX_DESCR];
#endif

void MDev_EMAC_PowerOn(void);
void MDev_EMAC_PowerOff(void);

// -----------------------------------------------------------------------------
// Linux EMAC.h End
// -----------------------------------------------------------------------------


