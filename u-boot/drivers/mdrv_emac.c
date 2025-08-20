
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvEMAC.c
/// @brief  u-boot EMAC Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------#include "e1000.h"

#include "mdrv_emac.h"

u8 MY_MAC[6] = { 0x00, 0x30, 0x1B, 0xBA, 0x02, 0xDB };
unsigned char phy_id = 0;

#define PHY_REG_STATUS	    (1)
#define EMAC_DBG_TFTP       (0)
#define EMAC_DBG_MSG(x)     //x
#if defined(CONFIG_GP2_DEMO1)
//@FIXME after RTK rework PHY circuit
#define RTK_OUI_MSB         (0x1C)
#endif
#if (EMAC_DBG_TFTP)
s32 rxpre_seq = 0, txpre_seq = 0;
#endif

#define BIT0	                    0x00000001
#define BIT1	                    0x00000002
#define BIT2	                    0x00000004
#define BIT3	                    0x00000008
#define BIT4	                    0x00000010
#define BIT5	                    0x00000020
#define BIT6	                    0x00000040
#define BIT7	                    0x00000080
#define BIT8	                    0x00000100
#define BIT9	                    0x00000200
#define BIT10	                    0x00000400
#define BIT11	                    0x00000800
#define BIT12	                    0x00001000
#define BIT13	                    0x00002000
#define BIT14	                    0x00004000
#define BIT15  	                    0x00008000

//GPIO reset
#define REG_CHIP_BASE              	0xBF203C00
#define REG_MIPS_BASE              	0xBF000000//Use 8 bit addressing
#define MHal_GPIO_REG(addr)    		(*(volatile U8*)(REG_MIPS_BASE + (((addr) & ~1)<<1) + (addr & 1)))

//-------------------------------------------------------------------------------------------------
//Initialize and start the Receiver and Transmit subsystems
//-------------------------------------------------------------------------------------------------
static void MDrv_EMAC_start (struct eth_device *nic)
{
	struct emac_hw *hw = (struct emac_hw *) nic->priv;
	struct recv_desc_bufs *dlist, *dlist_phys;

	u32 uRegVal,i;
#ifdef SOFTWARE_DESCRIPTOR
	u32 uJulian104Value = 0, RBQP_offset;
#endif

	dlist = hw->dlist;
	dlist_phys = hw->dlist_phys;
#ifdef SOFTWARE_DESCRIPTOR
	// dlist->descriptors[MAX_RX_DESCR - 1].addr |= EMAC_DESC_WRAP;

#ifdef CHECKSUM
	uJulian104Value=uJulian104Value|(CHECKSUM_ENABLE|SOFTWARE_DESCRIPTOR_ENABLE);
#else
	uJulian104Value=uJulian104Value|SOFTWARE_DESCRIPTOR_ENABLE;
#endif

	//  MHal_EMAC_Write_JULIAN_0104(0xFF050081);

	for(i=0; i<RBQP_LENG;i++)
	{
		RBQP_offset = i * 8;
		if(i<(RBQP_LENG-1))
		{
			MHal_EMAC_WritRam32(RAM_ADDR_BASE,RBQP_BASE + RBQP_offset, (RX_BUFFER_BASE+0x600*i));
		}
		else
		{
			MHal_EMAC_WritRam32(RAM_ADDR_BASE,RBQP_BASE + RBQP_offset, (RX_BUFFER_BASE+0x600*i+EMAC_DESC_WRAP));
		}
		// printf("RX_BUFFER is %x \n",(RX_BUFFER_BASE+0x600*idxRBQP));
	}
	Chip_Flush_Memory();
	MHal_EMAC_Write_RBQP(RBQP_BASE);
	MHal_EMAC_Write_JULIAN_0100(0x0000F007);
	MHal_EMAC_Write_JULIAN_0104(uJulian104Value);//Disable interrupt delay

#else
	for(i = 0; i < MAX_RX_DESCR; i++) {
		dlist->descriptors[i].addr = 0;
		dlist->descriptors[i].size = 0;
	}
	// Set the Wrap bit on the last descriptor //
	dlist->descriptors[MAX_RX_DESCR - 1].addr = EMAC_DESC_WRAP;
#endif //#ifndef SOFTWARE_DESCRIPTOR

	// set offset of read and write pointers in the receive circular buffer //
	uRegVal = MHal_EMAC_Read_BUFF();
	uRegVal = (RX_BUFFER_BASE|RX_BUFFER_SEL);
	MHal_EMAC_Write_BUFF(uRegVal);
	MHal_EMAC_Write_RDPTR(0);
	MHal_EMAC_Write_WRPTR(0);

	// Program address of descriptor list in Rx Buffer Queue register //
	uRegVal = ((EMAC_REG) & dlist_phys->descriptors);
	MHal_EMAC_Write_RBQP(uRegVal);

	//Reset buffer index//
	hw->rxBuffIndex = 0;

	// Enable Receive and Transmit //
	uRegVal = MHal_EMAC_Read_CTL();
	uRegVal |= (EMAC_RE | EMAC_TE);
	MHal_EMAC_Write_CTL(uRegVal);
}

//-------------------------------------------------------------------------------------------------
// Open the ethernet interface
//-------------------------------------------------------------------------------------------------
static int MDrv_EMAC_open (struct eth_device *nic, bd_t * bis)
{
	// struct emac_hw *hw = (struct emac_hw *) nic->priv;
	u32 uRegVal;

	//ato  EMAC_SYS->PMC_PCER = 1 << EMAC_ID_EMAC;   //Re-enable Peripheral clock //
	uRegVal = MHal_EMAC_Read_CTL();
	uRegVal |= EMAC_CSR;
	MHal_EMAC_Write_CTL(uRegVal);
	// Enable PHY interrupt //
	MHal_EMAC_enable_phyirq ();

	// Enable MAC interrupts //
	uRegVal = EMAC_INT_RCOM |EMAC_INT_RBNA
		| EMAC_INT_TUND | EMAC_INT_RTRY | EMAC_INT_TCOM
		| EMAC_INT_ROVR | EMAC_INT_HRESP;
	MHal_EMAC_Write_IER(uRegVal);

	MDrv_EMAC_start (nic);
	return 0;
}

#if (EMAC_DBG_TFTP)
static void MDrv_EMAC_DumpMem(u32 addr, u32 len)
{
	u8 *ptr = (u8 *)addr;
	u32 i;

	printf("\n ===== Dump %lx =====\n", ptr);
	i = (u32)ptr % 0x10;
	if (i)
	{
		len += i;
		ptr -= i;
	}
	for (i=0; i<len;i++, ptr++)
	{
		if ((u32)ptr%0x10 ==0)
			printf("%lx: ", ptr);
		if (*ptr < 0x10)
			printf("0%x ", *ptr);
		else
			printf("%x ", *ptr);
		if ((u32)ptr%0x10 == 0x0f)
			printf("\n");
	}
}
#endif

#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_GP2_DEMO1)
static u8 bc[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xfF };
static u8 *MDrv_EMAC_CheckShift(u8 *addr)
{
	if (0 != memcmp((void *)bc, (void *)addr, 0x06))
	{
		if (0 != memcmp(MY_MAC, addr, 0x06))
			return (addr+4);
	}
	return addr;
}
#endif
/**************************************************************************
  POLL - Wait for a frame
 ***************************************************************************/
static int MDrv_EMAC_rx(struct eth_device *nic)
{
	struct emac_hw *hw = (struct emac_hw *) nic->priv;
	struct recv_desc_bufs *dlist;
	unsigned char *p_recv;

	u32 pktlen;
	u32 uRegVal=0;
	u32 wrap_bit;
#if (EMAC_DBG_TFTP)
	u8 type = 0;
	u16 block = 0;
#endif

	dlist = hw->dlist ;
	// If any Ownership bit is 1, frame received.
	flush_cache((ulong)dlist ,sizeof(dlist));//2008/12/23 Nick
	//flush read buffer data
	uRegVal = ((volatile u32 *)0xA001000)[0];
	uRegVal = ((volatile u32 *)0xC001000)[0];

#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
	Chip_Flush_Memory();
#endif
	while ( (dlist->descriptors[hw->rxBuffIndex].addr )& EMAC_DESC_DONE)
	{
		p_recv = (unsigned char *) ((((dlist->descriptors[hw->rxBuffIndex].addr)&0x0FFFFFFF) |RAM_ADDR_BASE) &~(EMAC_DESC_DONE | EMAC_DESC_WRAP));
		pktlen = (u32)dlist->descriptors[hw->rxBuffIndex].size & 0x7ff;    /* Length of frame including FCS */


#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_GP2_DEMO1)
		//@FIXME after T3 ECO
		p_recv = MDrv_EMAC_CheckShift(p_recv);
#endif
#if (EMAC_DBG_TFTP)
		block = *(p_recv+0x2d) | (*(p_recv+0x2c) << 8);
		type = *(p_recv+0x2b);
		if (3 == type && block >0)
		{
			if (block == rxpre_seq)
			{
				EMAC_DBG_MSG(printf("xxxxxx Rx tftp Packet:%lx Type:%u block:%u again!!!\n", p_recv, type, block));
			}
			else
				rxpre_seq++;
		}
		else if (4 == type)
		{
			EMAC_DBG_MSG(printf("Rx tftp ACK Packet:%lx Type:%u block:%u\n", p_recv, type, block));
		}
		else if (6 == type)
		{
			EMAC_DBG_MSG(printf("\nRx tftp Opt ACK Packet:%lx Type:%u\n", p_recv, type));
			EMAC_DBG_MSG(printf("Ready to tftp transaction!!!!!!!\n"));
		}
		else if (block != (rxpre_seq+1) && rxpre_seq > 0)
		{
			EMAC_DBG_MSG(printf("\nError tftp packet block:%u rsr:%x tsr:%x\n", (rxpre_seq+1), MHal_EMAC_Read_RSR(), MHal_EMAC_Read_TSR()));
			EMAC_DBG_MSG(printf("\ndescriptor:%lx size:%lx\n", &(dlist->descriptors[hw->rxBuffIndex]), pktlen));
			//MDrv_EMAC_DumpMem(p_recv, 0x40);
			//MDev_EMAC_stats();
		}
#endif

		flush_cache((ulong)p_recv,pktlen);
#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
		Chip_Flush_Memory();
#endif
		if ((unsigned char *) (dlist->recv_buf + RX_BUFFER_SIZE) >
				(p_recv + ROUND_SUP_4 (pktlen)))
		{
			// the frame is not splitted in two parts //
			memcpy(packet, p_recv, pktlen);
			// update consumer pointer//
			uRegVal = MHal_EMAC_Read_RDPTR();
			uRegVal += ROUND_SUP_4(pktlen);
			MHal_EMAC_Write_RDPTR(uRegVal);
		}
		else
		{
			//the frame is splitted in two parts //
			int firstPacketPartSize =
				(unsigned char *) dlist->recv_buf + RX_BUFFER_SIZE - p_recv;
			int secondPacketSize =(ROUND_SUP_4(pktlen) - firstPacketPartSize);
			memcpy((unsigned char *)packet, p_recv, firstPacketPartSize);
			memcpy((unsigned char *)(packet+firstPacketPartSize), (unsigned char *) dlist->recv_buf, secondPacketSize);

			// update consumer pointer and toggle the wrap bit //
			wrap_bit = (MHal_EMAC_Read_RDPTR() & EMAC_WRAP_R) ^ EMAC_WRAP_R;
			MHal_EMAC_Write_RDPTR(secondPacketSize | wrap_bit);
		}
		flush_cache((ulong)packet,pktlen);
#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
		Chip_Flush_Memory();
#endif
		NetReceive((uchar *)packet, pktlen);

		flush_cache((ulong)packet,pktlen);
#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
		Chip_Flush_Memory();
#endif
		dlist->descriptors[hw->rxBuffIndex].addr  &= ~EMAC_DESC_DONE;  /* reset ownership bit */
		//wrap after last buffer //
		hw->rxBuffIndex++;
		if (hw->rxBuffIndex == MAX_RX_DESCR)
		{
			hw->rxBuffIndex = 0;
		}
	}
	return 1;
}

/**************************************************************************
  TRANSMIT - Transmit a frame
 ***************************************************************************/
static int MDrv_EMAC_tx(struct eth_device *nic, volatile void *packet, int length)
{
	u32 tmp = 0;
#if (EMAC_DBG_TFTP)
	u8 *ptr = (u8 *)packet, type;
	u16 block = 0;
#endif
	while (length < 60)         //padding to 60 bytes
	{
		*((u8*)packet+length)=0;
		length++;
	}
	flush_cache((ulong)packet,length);
	//flush read buffer data
	tmp = ((volatile u32 *)0xA001000)[0];
	tmp = ((volatile u32 *)0xC001000)[0];


#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
	Chip_Flush_Memory();
#endif
#if (EMAC_DBG_TFTP)
	type = *(ptr+0x2b);
	block = *(ptr+0x2d) | (*(ptr+0x2c) << 8);
	if (1 == type)
	{
		EMAC_DBG_MSG(printf("\nTx tftp Request Type:%u\n", ptr, type));
	}
	else if (3 == type)
	{
		EMAC_DBG_MSG(printf("Tx tftp Packet:%lx Type:%u block:%u\n", ptr, type, block));
	}
	else if (4 == type && block >0)
	{
		if (block == txpre_seq)
			EMAC_DBG_MSG(printf("xxxxxxx Tx tftp ACK Packet:%lx Type:%u block:%u again!!!!\n", ptr, type, block));
		else
			block++;
	}
#endif
	MHal_EMAC_Write_TAR((u32)packet);
	// Set length of the packet in the Transmit Control register //
	MHal_EMAC_Write_TCR(length);

	return 1;
}

//-------------------------------------------------------------------------------------------------
// Close the interface
//-------------------------------------------------------------------------------------------------
void MDrv_EMAC_close(struct eth_device *nic)
{
	u32 uRegVal;
	//Disable Receiver and Transmitter //
	uRegVal = MHal_EMAC_Read_CTL();
	uRegVal &= ~(EMAC_TE | EMAC_RE);
	MHal_EMAC_Write_CTL(uRegVal);
	// Disable PHY interrupt //
	MHal_EMAC_disable_phyirq ();

	//Disable MAC interrupts //
	uRegVal = EMAC_INT_RCOM | EMAC_INT_RBNA
		| EMAC_INT_TUND | EMAC_INT_RTRY | EMAC_INT_TCOM
		| EMAC_INT_ROVR | EMAC_INT_HRESP;
	MHal_EMAC_Write_IDR(uRegVal);
	netif_stop_queue (nic);
}

void MDev_EMAC_stats(void)
{
	printf("Rx Ok:%u\n", MHal_EMAC_Read_OK());       /* Good frames received */
	printf("ALE:%u\n", MHal_EMAC_Read_ALE());
	printf("ELR:%u\n", MHal_EMAC_Read_ELR());        /* Excessive Length or Undersize Frame error */
	printf("SEQE:%u\n", MHal_EMAC_Read_SEQE());      /* Excessive Length or Undersize Frame error */
	printf("ROVR:%u\n", MHal_EMAC_Read_ROVR());      //rx fifo error
	printf("SE:%u\n", MHal_EMAC_Read_SE());
	printf("RJB:%u\n", MHal_EMAC_Read_RJB());

	printf("Tx Ok:%u\n", MHal_EMAC_Read_FRA());
	printf("TUE:%u\n", MHal_EMAC_Read_TUE());        /* Transmit FIFO underruns */
	printf("CSE:%u\n", MHal_EMAC_Read_CSE());      /* Carrier Sense errors */
	printf("SQEE:%u\n", MHal_EMAC_Read_SQEE());      /* Heartbeat error */
}

void MDrv_EMAC_PhyAddrScan(void)
{
	U32 i;
	U32 word_ETH_MAN  = 0x00000000;

	MHal_EMAC_Write_CTL(0x00000010 | MHal_EMAC_Read_CTL());
	ThisUVE.flagISR_INT_DONE = 0x00;

	word_ETH_MAN = MHal_EMAC_Read_MAN()&0xffff;
	//dhjung LGE
	//printf("phy [%d] = %x\n", phy_id, word_ETH_MAN);
	if((0xffff != word_ETH_MAN)&&(0x0000 != word_ETH_MAN))
	{
		return;
	}

	for (phy_id = 0; phy_id < 32; phy_id++)
	{
		word_ETH_MAN = (PHY_LOW_HIGH<<PHY_LOW_HIGH_SHIFT) | (PHY_READ_OP<<PHY_RW_SHIFT) |
			(phy_id<<PHY_ADDR_SHIFT) | (PHY_REG_STATUS<<PHY_REG_SHIFT) |
			(PHY_CODE<<PHY_CODE_SHIFT) | (0x0);

		MHal_EMAC_Write_MAN(word_ETH_MAN);
		for(i=0;i<1000;i++) // Wait for management interrupt done
		{
			if(ThisUVE.flagISR_INT_DONE == 0x01) {
				ThisUVE.flagISR_INT_DONE = 0x00;

				break;
			}
			udelay(10);
		}
		word_ETH_MAN = MHal_EMAC_Read_MAN()&0xffff;
		//dhjung LGE
		//printf("phy [%d] = %x\n", phy_id, word_ETH_MAN);
		if((0xffff != word_ETH_MAN)&&(0x0000 != word_ETH_MAN))
		{
			return;
		}
	}
}

//-------------------------------------------------------------------------------------------------
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
static int MDrv_EMAC_HW_init(struct eth_device *nic)
{
	u32 word_ETH_CTL = 0x00000000;
	u32 word_ETH_CFG = 0x00000000;
	u32 uJulian104Value=0;
	u32 uNegPhyVal=0;
	u32 word_ETH_MAN =0x57828000; //default

#ifdef SOFTWARE_DESCRIPTOR
	u32 idxRBQP=0;
	u32 RBQP_offset=0;
#endif
	// (20071026_CHARLES) Disable TX, RX and MDIO:   (If RX still enabled, the RX buffer will be overwrited)
	MHal_EMAC_Write_CTL(word_ETH_CTL);
	// Init RX --------------------------------------------------------------
	memset((u8*)RAM_ADDR_BASE + RX_BUFFER_BASE,0x00,RX_BUFFER_SIZE);

	MHal_EMAC_Write_BUFF(RX_BUFFER_BASE|RX_BUFFER_SEL);
	MHal_EMAC_Write_RDPTR(0x00000000);
	MHal_EMAC_Write_WRPTR(0x00000000);
	// Initialize "Receive Buffer Queue Pointer"
	MHal_EMAC_Write_RBQP(RBQP_BASE);
	// Initialize Receive Buffer Descriptors
	//    memset((u8*)RAM_ADDR_BASE+RBQP_BASE,0x00,0x2000);	    // Clear for max(8*1024)bytes (max:1024 descriptors)
	memset((u8*)RAM_ADDR_BASE+RBQP_BASE,0x00,RBQP_SIZE);
	MHal_EMAC_WritRam32(RAM_ADDR_BASE,(RBQP_BASE+RBQP_SIZE-0x08),0x00000002);             // (n-1) : Wrap = 1

	// Enable Interrupts ----------------------------------------------------
	//uJulian104Value=0x00000000;
	//MHal_EMAC_Write_JULIAN_0104(uJulian104Value);
	MHal_EMAC_Write_IER(0x0000FFFF);

	//dhjung LGE
	//printf("MAC Address %02X:%02X:%02X:%02X:%02X:%02X\n",ThisBCE.sa1[0],ThisBCE.sa1[1],
	//			ThisBCE.sa1[2],ThisBCE.sa1[3],ThisBCE.sa1[4],ThisBCE.sa1[5]);
	// Set MAC address ------------------------------------------------------
	MHal_EMAC_Write_SA1_MAC_Address(ThisBCE.sa1[0],ThisBCE.sa1[1],ThisBCE.sa1[2],ThisBCE.sa1[3],ThisBCE.sa1[4],ThisBCE.sa1[5]);
	MHal_EMAC_Write_SA2_MAC_Address(ThisBCE.sa2[0],ThisBCE.sa2[1],ThisBCE.sa2[2],ThisBCE.sa2[3],ThisBCE.sa2[4],ThisBCE.sa2[5]);
	MHal_EMAC_Write_SA3_MAC_Address(ThisBCE.sa3[0],ThisBCE.sa3[1],ThisBCE.sa3[2],ThisBCE.sa3[3],ThisBCE.sa3[4],ThisBCE.sa3[5]);
	MHal_EMAC_Write_SA4_MAC_Address(ThisBCE.sa4[0],ThisBCE.sa4[1],ThisBCE.sa4[2],ThisBCE.sa4[3],ThisBCE.sa4[4],ThisBCE.sa4[5]);

#ifdef SOFTWARE_DESCRIPTOR
#ifdef CHECKSUM
	uJulian104Value=uJulian104Value|(CHECKSUM_ENABLE|SOFTWARE_DESCRIPTOR_ENABLE);
#else
	uJulian104Value=uJulian104Value|SOFTWARE_DESCRIPTOR_ENABLE;
#endif
	//  MHal_EMAC_Write_JULIAN_0104(0xFF050081);

	for(idxRBQP=0; idxRBQP<RBQP_LENG;idxRBQP++)
	{
		RBQP_offset = idxRBQP * 8;
		if(idxRBQP<(RBQP_LENG-1))
		{
			MHal_EMAC_WritRam32(RAM_ADDR_BASE,RBQP_BASE + RBQP_offset, (RX_BUFFER_BASE+0x600*idxRBQP));
		}
		else
		{
			MHal_EMAC_WritRam32(RAM_ADDR_BASE,RBQP_BASE + RBQP_offset, (RX_BUFFER_BASE+0x600*idxRBQP+EMAC_DESC_WRAP));
		}
		// printf("RX_BUFFER is %x \n",(RX_BUFFER_BASE+0x600*idxRBQP));
	}
	Chip_Flush_Memory();
	MHal_EMAC_Write_RBQP(RBQP_BASE);
	MHal_EMAC_Write_JULIAN_0100(0x0000F007);
	MHal_EMAC_Write_JULIAN_0104(uJulian104Value);//Disable interrupt delay
#else
#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
	MHal_EMAC_Write_JULIAN_0100(0x0000F007);
	MHal_EMAC_Write_JULIAN_0104(0x00);
#else
	uJulian104Value=0x00000000;
	MHal_EMAC_Write_JULIAN_0104(uJulian104Value);
	MHal_EMAC_Write_JULIAN_0100(0x00000107);
#endif
#endif //#ifdef SOFTWARE_DESCRIPTOR

	//dhjung LGE
	//printf("ETH_MAN=0x%X\n",word_ETH_MAN);//debug
	MHal_EMAC_Write_MAN(word_ETH_MAN);
	// IMPORTANT: Run NegotiationPHY() before writing REG_ETH_CFG.
	uNegPhyVal = MHal_EMAC_NegotiationPHY();
	if(uNegPhyVal==0x01)
	{
		ThisUVE.flagMacTxPermit = 0x01;
		ThisBCE.duplex = 1;
	}
	else if(uNegPhyVal==0x02)
	{
		ThisUVE.flagMacTxPermit = 0x01;
		ThisBCE.duplex = 2;
	}

	// ETH_CFG Register -----------------------------------------------------
	word_ETH_CFG = (0x00000800|EMAC_FD|EMAC_SPD);		// Init: CLK = 0x2

	MHal_EMAC_Write_CFG(word_ETH_CFG);
	// ETH_CTL Register -----------------------------------------------------
	word_ETH_CTL = 0x000001C;  // Enable transmit and receive : TE + RE + MDIO = 0x1C
	if(ThisBCE.wes == 1) word_ETH_CTL |= 0x00000080;
	MHal_EMAC_Write_CTL(word_ETH_CTL);
	//MHal_EMAC_Write_JULIAN_0100(0x00000107);

	ThisUVE.flagPowerOn = 1;
	ThisUVE.initedEMAC  = 1;

	return 1;
}

//-------------------------------------------------------------------------------------------------
// EMAC init Variable
//-------------------------------------------------------------------------------------------------
static volatile U8 emac_var[EMAC_ABSO_MEM_SIZE];
u32 MDrv_EMAC_VarInit(void)
{
	u32 alloRAM_ADDR_BASE;
	int i;
	char *s, *e;

	alloRAM_ADDR_BASE = (u32)emac_var;

	flush_cache((ulong)alloRAM_ADDR_BASE ,EMAC_ABSO_MEM_SIZE);//2008/12/23 Nick
	memset((u32 *)alloRAM_ADDR_BASE,0x00,EMAC_ABSO_MEM_SIZE);
	RAM_ADDR_BASE    = (alloRAM_ADDR_BASE & 0xFFFFC000) + RX_BUFFER_SIZE;   // IMPORTANT: Let lowest 14 bits as zero.

	RX_BUFFER_BASE	+= RAM_ADDR_BASE - EMAC_ABSO_PHY_BASE;
	RBQP_BASE		+= RAM_ADDR_BASE - EMAC_ABSO_PHY_BASE;
	TX_BUFFER_BASE	+= RAM_ADDR_BASE - EMAC_ABSO_PHY_BASE;
	RAM_ADDR_BASE    = EMAC_ABSO_MEM_BASE;  // IMPORTANT_TRICK_20070512

	//dhjung LGE
	//printf("Rx buff:%lx\nRBQP:%lx\nRam Base:%lx\n", RX_BUFFER_BASE, RBQP_BASE, RAM_ADDR_BASE);
	memset(&ThisBCE,0x00,sizeof(BasicConfigEMAC));
	memset(&ThisUVE,0x00,sizeof(UtilityVarsEMAC));

	ThisBCE.wes         = 0;				// 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
	ThisBCE.duplex      = 2;				// 1:Half-duplex, 2:Full-duplex
	ThisBCE.cam         = 0;                // 0:No CAM, 1:Yes
	ThisBCE.rcv_bcast   = 0;      			// 0:No, 1:Yes
	ThisBCE.rlf         = 0;                // 0:No, 1:Yes receive long frame(1522)
	ThisBCE.rcv_bcast   = 1;
	ThisBCE.speed       = EMAC_SPEED_100;

	s = getenv ("ethaddr");
	if (s)
	{
		for (i = 0; i < 6; ++i)
		{
			ThisBCE.sa1[i] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s)
			{
				s = (*e) ? e + 1 : e;
			}
		}
	}
	else
	{
		ThisBCE.sa1[0]      = MY_MAC[0];
		ThisBCE.sa1[1]      = MY_MAC[1];
		ThisBCE.sa1[2]      = MY_MAC[2];
		ThisBCE.sa1[3]      = MY_MAC[3];
		ThisBCE.sa1[4]      = MY_MAC[4];
		ThisBCE.sa1[5]      = MY_MAC[5];
	}

	return alloRAM_ADDR_BASE;
}

/**************************************************************************
  PROBE - Look for an adapter, this routine's visible to the outside
  You should omit the last argument struct pci_device * for a non-PCI NIC
 ***************************************************************************/
struct eth_device nic_device;
struct emac_hw hw_device;
int initialized=FALSE;
int MDrv_EMAC_initialize(bd_t * bis)
{
	struct eth_device *nic = NULL;
	struct emac_hw *hw = NULL;
	char *phy_id_string = NULL;

	if ((phy_id_string = getenv ("PHY_ID")) != NULL)
	{
		phy_id = phy_id_string ? (int)simple_strtol(phy_id_string, NULL, 10) : 0;
	}

	if(initialized)
	{
		MDrv_EMAC_PhyAddrScan();
#if (EMAC_DBG_TFTP)
		rxpre_seq = txpre_seq = 0;
#endif
		return -1;
	}
	//dhjung LGE
	//printf("phy_id=%d\n", phy_id);

	MDrv_EMAC_VarInit();
	nic=&nic_device;
	hw=&hw_device;

	hw->dlist_phys = hw->dlist = (struct recv_desc_bufs *) (RX_BUFFER_BASE + RAM_ADDR_BASE);

	//dhjung LGE
	//printf("gpio reset phy\n");

	//Reset Emac PHY
	MHal_GPIO_REG(0x101ea1) &= ~BIT7;
	MHal_GPIO_REG(0x101e24) &= ~(BIT6|BIT5|BIT4);
	MHal_GPIO_REG(0x1422) &= ~(BIT4|BIT3|BIT2|BIT1|BIT0);
	MHal_GPIO_REG(0x101e07) &= ~(BIT2|BIT1|BIT0);

	MHal_GPIO_REG(0x1423) |= BIT3;
	//set out
	MHal_GPIO_REG(0x1423)  &= ~BIT3;
	//set output low
	MHal_GPIO_REG(0x1424) &= ~BIT3;
	mdelay(20);
	//set output high
	MHal_GPIO_REG(0x1424) |= BIT3;
	//dhjung LGE : should be retained to '1' for somewhile
	mdelay(200);

	MHal_EMAC_Power_On_Clk();
	MDrv_EMAC_PhyAddrScan();

#if defined(CONFIG_GP2_DEMO1)
	{
		u32 val = 0;
		MHal_EMAC_read_phy(phy_id, 2, &val);
		if (RTK_OUI_MSB == val)
		{
			//dhjung LGE
			//printf("Force change RTK PHY Reg 25 to RMII mode:");
			MHal_EMAC_read_phy(phy_id, 25, &val);
			//printf("%x -> ", val);
			MHal_EMAC_write_phy(phy_id, 25, 0x400);
			MHal_EMAC_read_phy(phy_id, 25, &val);
			//printf("%x\n", val);
		}
	}
#endif
	nic->priv = hw;
	MDrv_EMAC_HW_init(nic);

	/* EMAC HW init */
	nic->init = MDrv_EMAC_open;
	nic->recv = MDrv_EMAC_rx;
	nic->send = MDrv_EMAC_tx;
	nic->halt = MDrv_EMAC_close;
	//dhjung LGE
	strcpy(nic->name, "emac");
	memcpy(nic->enetaddr, MY_MAC, sizeof(MY_MAC));

	eth_register(nic);
	initialized = 1;

	return 0;
}
