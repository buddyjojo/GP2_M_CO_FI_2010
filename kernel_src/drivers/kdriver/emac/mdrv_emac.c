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
/// @file    devEMAC.c
/// @brief  EMAC Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/init.h>
#include <linux/autoconf.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "mdrv_types.h"
#include "mst_platform.h"
#include "chip_setup.h"
#include "chip_int.h"
#include "mhal_chiptop_reg.h"
#include "mhal_emac.h"
#include "mdrv_emac.h"


#include "mst_devid.h"
#include "mdrv_probe.h"

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
#define EMAC_RX_TMR         (0)
#define EMAC_LINK_TMR       (1)

#define EMAC_CHECK_LINK_TIME (HZ)
//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
unsigned char phyaddr = 0;

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
static struct timer_list EMAC_timer, Link_timer;
static struct net_device *emac_dev;
//-------------------------------------------------------------------------------------------------
//  EMAC Function
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_timer_callback( unsigned long value );

//-------------------------------------------------------------------------------------------------
// PHY MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Access the PHY to determine the current Link speed and Mode, and update the
// MAC accordingly.
// If no link or auto-negotiation is busy, then no changes are made.
// Returns: 0 : OK
//          -1 : No link
//          -2 : AutoNegotiation still in progress
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_update_linkspeed( struct net_device* dev )
{
    u32 bmsr, bmcr, LocPtrA;
    u32 speed, duplex;

    // Link status is latched, so read twice to get current value //
    MHal_EMAC_read_phy(phyaddr, MII_BMSR, &bmsr );
    MHal_EMAC_read_phy( phyaddr, MII_BMSR, &bmsr );
    if ( !( bmsr & BMSR_LSTATUS ) )
    {
        return -1;
    }             //no link //

    MHal_EMAC_read_phy( phyaddr, MII_BMCR, &bmcr );

    if ( bmcr & BMCR_ANENABLE )
    {
        //AutoNegotiation is enabled //
        if ( !( bmsr & BMSR_ANEGCOMPLETE ) )
        {
            printk("==> AutoNegotiation still in progress\n");
            return -2;
        }        // auto-negotitation in progress //

        MHal_EMAC_read_phy( phyaddr, MII_LPA, &LocPtrA );
        if ( ( LocPtrA & LPA_100FULL ) || ( LocPtrA & LPA_100HALF ) )
        {
            speed = SPEED_100;
        }
        else
        {
            speed = SPEED_10;
        }
        if ( ( LocPtrA & LPA_100FULL ) || ( LocPtrA & LPA_10FULL ) )
        {
            duplex = DUPLEX_FULL;
        }
        else
        {
            duplex = DUPLEX_HALF;
        }
    }
    else
    {
        speed = ( bmcr & BMCR_SPEED100 ) ? SPEED_100 : SPEED_10;
        duplex = ( bmcr & BMCR_FULLDPLX ) ? DUPLEX_FULL : DUPLEX_HALF;
    }

    // Update the MAC //
    MHal_EMAC_update_speed_duplex( speed, duplex );
    return 0;
}

//-------------------------------------------------------------------------------------------------
//Program the hardware MAC address from dev->dev_addr.
//-------------------------------------------------------------------------------------------------
void MDev_EMAC_update_mac_address( struct net_device *dev)
{
   u32 value;
   value = (dev->dev_addr[3] << 24) | (dev->dev_addr[2] << 16) | (dev->dev_addr[1] << 8) |(dev->dev_addr[0]);
   MHal_EMAC_Write_SA1L(value);
   value = (dev->dev_addr[5] << 8) | (dev->dev_addr[4]);
   MHal_EMAC_Write_SA1H(value);
}

//-------------------------------------------------------------------------------------------------
// ADDRESS MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Set the ethernet MAC address in dev->dev_addr
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_get_mac_address( struct net_device* dev )
{
    char addr[6];
    u32 HiAddr, LoAddr;

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = MHal_EMAC_get_SA1H_addr();
    LoAddr = MHal_EMAC_get_SA1L_addr();

    addr[0] = ( LoAddr & 0xff );
    addr[1] = ( LoAddr & 0xff00 ) >> 8;
    addr[2] = ( LoAddr & 0xff0000 ) >> 16;
    addr[3] = ( LoAddr & 0xff000000 ) >> 24;
    addr[4] = ( HiAddr & 0xff );
    addr[5] = ( HiAddr & 0xff00 ) >> 8;

    if ( is_valid_ether_addr( addr ) )
    {
        memcpy( dev->dev_addr, &addr, 6 );
        return;
    }
    // Check if bootloader set address in Specific-Address 2 //
    HiAddr = MHal_EMAC_get_SA2H_addr();
    LoAddr = MHal_EMAC_get_SA2L_addr();
    addr[0] = ( LoAddr & 0xff );
    addr[1] = ( LoAddr & 0xff00 ) >> 8;
    addr[2] = ( LoAddr & 0xff0000 ) >> 16;
    addr[3] = ( LoAddr & 0xff000000 ) >> 24;
    addr[4] = ( HiAddr & 0xff );
    addr[5] = ( HiAddr & 0xff00 ) >> 8;

    if ( is_valid_ether_addr( addr ) )
    {
        memcpy( dev->dev_addr, &addr, 6 );
        return;
    }
}

#ifdef URANUS_ETHER_ADDR_CONFIGURABLE
//-------------------------------------------------------------------------------------------------
// Store the new hardware address in dev->dev_addr, and update the MAC.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_set_mac_address( struct net_device* dev, void* addr )
{
    struct sockaddr *address = addr;

    if ( !is_valid_ether_addr( address->sa_data ) )
    {
        return -EADDRNOTAVAIL;
    }

    memcpy( dev->dev_addr, address->sa_data, dev->addr_len );
    MDev_EMAC_update_mac_address( dev );
    return 0;
}
#endif

//-------------------------------------------------------------------------------------------------
// Add multicast addresses to the internal multicast-hash table.
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_sethashtable( struct net_device* dev )
{
    struct dev_mc_list* curr;
    unsigned char mc_filter[2];
    u32 i, bitnr;

    mc_filter[0] = mc_filter[1] = 0;

    curr = dev->mc_list;
    for ( i = 0; i < dev->mc_count; i++, curr = curr->next )
    {
        if ( !curr )
        {
            break;
        }             // unexpected end of list //

        bitnr = ether_crc( ETH_ALEN, curr->dmi_addr ) >> 26;
        mc_filter[bitnr >> 5] |= 1 << ( bitnr & 31 );
    }

    MHal_EMAC_update_HSH( mc_filter[1], mc_filter[0] );
}

//-------------------------------------------------------------------------------------------------
//Enable/Disable promiscuous and multicast modes.
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_set_rx_mode( struct net_device* dev )
{
    u32 uRegVal;
    uRegVal = MHal_EMAC_Read_CFG();
    if ( dev->flags & IFF_PROMISC )
    {
        // Enable promiscuous mode //
        uRegVal |= EMAC_CAF;
    }
    else if ( dev->flags & ( ~IFF_PROMISC ) )
    {
        // Disable promiscuous mode //
        uRegVal &= ~EMAC_CAF;
    }
    MHal_EMAC_Write_CFG( uRegVal );

    if ( dev->flags & IFF_ALLMULTI )
    {
        // Enable all multicast mode //
        MHal_EMAC_update_HSH( 0xFF, 0xFF );
        uRegVal |= EMAC_MTI;
    }
    else if ( dev->mc_count > 0 )
    {
        // Enable specific multicasts//
        MDev_EMAC_sethashtable( dev );
        uRegVal |= EMAC_MTI;
    }
    else if ( dev->flags & ( ~IFF_ALLMULTI ) )
    {
        // Disable all multicast mode//
        MHal_EMAC_update_HSH( 0, 0 );
        uRegVal &= ~EMAC_MTI;
    }
    MHal_EMAC_Write_CFG( uRegVal );
}
//-------------------------------------------------------------------------------------------------
// IOCTL
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable/Disable MDIO
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_mdio_read( struct net_device* dev,
                                int phy_id,
                                int location )
{
    u32 value;
    MHal_EMAC_read_phy( phy_id, location, &value );
    return value;
}

static void MDev_EMAC_mdio_write( struct net_device* dev,
                                  int phy_id,
                                  int location,
                                  int value )
{
    MHal_EMAC_write_phy( phy_id, location, value );
}

//-------------------------------------------------------------------------------------------------
//ethtool support.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_ethtool_ioctl( struct net_device* dev, void* useraddr )
{
    struct EMAC_private* LocPtr = ( struct EMAC_private* ) dev->priv;
    u32 ethcmd;
    int res = 0;

    if ( copy_from_user( &ethcmd, useraddr, sizeof( ethcmd ) ) )
    {
        return -EFAULT;
    }

    spin_lock_irq( &LocPtr->lock );

    switch ( ethcmd )
    {
      case ETHTOOL_GSET:
        {
            struct ethtool_cmd ecmd =
            {
              ETHTOOL_GSET
            };
            res = mii_ethtool_gset( &LocPtr->mii, &ecmd );
            if ( LocPtr->phy_media == PORT_FIBRE )
            {
                //override media type since mii.c doesn't know //
                ecmd.supported = SUPPORTED_FIBRE;
                ecmd.port = PORT_FIBRE;
            }
            if ( copy_to_user( useraddr, &ecmd, sizeof( ecmd ) ) )
            {
                res = -EFAULT;
            }
            break;
        }
      case ETHTOOL_SSET:
        {
            struct ethtool_cmd ecmd;
            if ( copy_from_user( &ecmd, useraddr, sizeof( ecmd ) ) )
            {
                res = -EFAULT;
            }
            else
            {
                res = mii_ethtool_sset( &LocPtr->mii, &ecmd );
            }
            break;
        }
      case ETHTOOL_NWAY_RST:
        {
            res = mii_nway_restart( &LocPtr->mii );
            break;
        }
      case ETHTOOL_GLINK:
        {
            struct ethtool_value edata =
            {
              ETHTOOL_GLINK
            };
            edata.data = mii_link_ok( &LocPtr->mii );
            if ( copy_to_user( useraddr, &edata, sizeof( edata ) ) )
            {
                res = -EFAULT;
            }
            break;
        }
      default:
        res = -EOPNOTSUPP;
    }
    spin_unlock_irq( &LocPtr->lock );
    return res;
}

//-------------------------------------------------------------------------------------------------
// User-space ioctl interface.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_ioctl( struct net_device* dev, struct ifreq* rq, int cmd )
{
    int     result;
    PROBE_IO_ENTRY(MDRV_MAJOR_EMAC, _IOC_NR(cmd));

    switch ( cmd )
    {
      case SIOCETHTOOL:
        result = MDev_EMAC_ethtool_ioctl( dev, ( void * ) rq->ifr_data );
        PROBE_IO_EXIT(MDRV_MAJOR_EMAC, _IOC_NR(cmd));
        return result;
      default:
       PROBE_IO_EXIT(MDRV_MAJOR_EMAC, _IOC_NR(cmd));
        return -EOPNOTSUPP;
    }
}
//-------------------------------------------------------------------------------------------------
// MAC
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//Initialize and start the Receiver and Transmit subsystems
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_start( struct net_device* dev )
{
    struct EMAC_private* LocPtr = ( struct EMAC_private* ) dev->priv;
    struct recv_desc_bufs* dlist, * dlist_phys;
#ifndef SOFTWARE_DESCRIPTOR
    int i;
#endif
    u32 uRegVal;

    dlist = LocPtr->dlist;
    dlist_phys = LocPtr->dlist_phys;
#ifdef SOFTWARE_DESCRIPTOR
    dlist->descriptors[MAX_RX_DESCR - 1].addr |= EMAC_DESC_WRAP;
#else
    for ( i = 0; i < MAX_RX_DESCR; i++ )
    {
        dlist->descriptors[i].addr = 0;
        dlist->descriptors[i].size = 0;
    }
    // Set the Wrap bit on the last descriptor //
    dlist->descriptors[MAX_RX_DESCR - 1].addr = EMAC_DESC_WRAP;
#endif //#ifndef SOFTWARE_DESCRIPTOR
    // set offset of read and write pointers in the receive circular buffer //
    uRegVal = MHal_EMAC_Read_BUFF();
    uRegVal = ( RX_BUFFER_BASE | RX_BUFFER_SEL );
    MHal_EMAC_Write_BUFF( uRegVal );
    MHal_EMAC_Write_RDPTR( 0 );
    MHal_EMAC_Write_WRPTR( 0 );

    // Program address of descriptor list in Rx Buffer Queue register //
    uRegVal = ( ( EMAC_REG ) & dlist_phys->descriptors );
    MHal_EMAC_Write_RBQP( uRegVal );

    //Reset buffer index//
    LocPtr->rxBuffIndex = 0;

    // Enable Receive and Transmit //
    uRegVal = MHal_EMAC_Read_CTL();
    uRegVal |= ( EMAC_RE | EMAC_TE );
    MHal_EMAC_Write_CTL( uRegVal );
}

//-------------------------------------------------------------------------------------------------
// Open the ethernet interface
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_open( struct net_device* dev )
{
    struct EMAC_private* LocPtr = ( struct EMAC_private* ) dev->priv;
    u32 uRegVal;
    if ( !is_valid_ether_addr( dev->dev_addr ) )
    {
        return -EADDRNOTAVAIL;
    }

    //ato  EMAC_SYS->PMC_PCER = 1 << EMAC_ID_EMAC;    //Re-enable Peripheral clock //
    uRegVal = MHal_EMAC_Read_CTL();
    uRegVal |= EMAC_CSR;
    MHal_EMAC_Write_CTL( uRegVal );
    // Enable PHY interrupt //
    MHal_EMAC_enable_phyirq();

    // Enable MAC interrupts //
#ifndef INT_DELAY
    uRegVal = EMAC_INT_RCOM |
              EMAC_INT_RBNA |
              EMAC_INT_TUND |
              EMAC_INT_RTRY |
              EMAC_INT_TCOM |
              EMAC_INT_ROVR |
              EMAC_INT_HRESP;
    MHal_EMAC_Write_IER( uRegVal );
#endif
    // Determine current link speed //
    spin_lock_irq( &LocPtr->lock );
    ( void ) MDev_EMAC_update_linkspeed( dev );
    spin_unlock_irq( &LocPtr->lock );

    MDev_EMAC_start( dev );
    netif_start_queue( dev );

    init_timer( &Link_timer );
    Link_timer.data = EMAC_LINK_TMR;
    Link_timer.function = MDev_EMAC_timer_callback;
    Link_timer.expires = jiffies + EMAC_CHECK_LINK_TIME;
    add_timer(&Link_timer);
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Close the interface
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_close( struct net_device* dev )
{
    u32 uRegVal;
    //Disable Receiver and Transmitter //
    uRegVal = MHal_EMAC_Read_CTL();
    uRegVal &= ~( EMAC_TE | EMAC_RE );
    MHal_EMAC_Write_CTL( uRegVal );
    // Disable PHY interrupt //
    MHal_EMAC_disable_phyirq();

    //Disable MAC interrupts //
    uRegVal = EMAC_INT_RCOM |
              EMAC_INT_RBNA |
              EMAC_INT_TUND |
              EMAC_INT_RTRY |
              EMAC_INT_TCOM |
              EMAC_INT_ROVR |
              EMAC_INT_HRESP;
    MHal_EMAC_Write_IDR( uRegVal );
    netif_stop_queue( dev );
    del_timer(&Link_timer);
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Update the current statistics from the internal statistics registers.
//-------------------------------------------------------------------------------------------------
static struct net_device_stats* MDev_EMAC_stats( struct net_device* dev )
{
    struct EMAC_private* LocPtr = ( struct EMAC_private* ) dev->priv;
    int ale, lenerr, seqe, lcol, ecol;

    if ( netif_running( dev ) )
    {
        LocPtr->stats.rx_packets += MHal_EMAC_Read_OK();                /* Good frames received */
        ale = MHal_EMAC_Read_ALE();
        LocPtr->stats.rx_frame_errors += ale;                              /* Alignment errors */
        lenerr = MHal_EMAC_Read_ELR();
        LocPtr->stats.rx_length_errors += lenerr;                         /* Excessive Length or Undersize Frame error */
        seqe = MHal_EMAC_Read_SEQE();
        LocPtr->stats.rx_crc_errors += seqe;                                /* CRC error */
        LocPtr->stats.rx_fifo_errors += MHal_EMAC_Read_ROVR();
        LocPtr->stats.rx_errors += ale +
                                   lenerr +
                                   seqe +
                                   MHal_EMAC_Read_SE() +
                                   MHal_EMAC_Read_RJB();
        LocPtr->stats.tx_packets += MHal_EMAC_Read_FRA();              /* Frames successfully transmitted */
        LocPtr->stats.tx_fifo_errors += MHal_EMAC_Read_TUE();         /* Transmit FIFO underruns */
        LocPtr->stats.tx_carrier_errors += MHal_EMAC_Read_CSE();     /* Carrier Sense errors */
        LocPtr->stats.tx_heartbeat_errors += MHal_EMAC_Read_SQEE(); /* Heartbeat error */
        lcol = MHal_EMAC_Read_LCOL();
        ecol = MHal_EMAC_Read_ECOL();
        LocPtr->stats.tx_window_errors += lcol;                            /* Late collisions */
        LocPtr->stats.tx_aborted_errors += ecol;                          /* 16 collisions */
        LocPtr->stats.collisions += MHal_EMAC_Read_SCOL() +
                                    MHal_EMAC_Read_MCOL() +
                                    lcol +
                                    ecol;
    }
    return &LocPtr->stats;
}

static void MDev_EMAC_retry(struct net_device* dev )
{
    struct EMAC_private* LocPtr = ( struct EMAC_private* ) dev->priv;

    //Set address of the data in the Transmit Address register //
    MHal_EMAC_Write_TAR(LocPtr->txpkt);
    // Set length of the packet in the Transmit Control register //
    MHal_EMAC_Write_TCR( LocPtr->skb_length );
    dev->trans_start = jiffies;
    LocPtr->retx_count++;
    printk("Retry count:%u\n", LocPtr->retx_count);
}

//-------------------------------------------------------------------------------------------------
//Transmit packet.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_tx( struct sk_buff* skb, struct net_device* dev )
{
    struct EMAC_private* LocPtr = ( struct EMAC_private* ) dev->priv;

    //flush write buffer data
    Chip_Flush_Memory();

#ifndef TX_SKB_PTR
    LocPtr->txpkt = TX_SKB_BASE + RAM_ADDR_BASE;
    memcpy( ( void * ) LocPtr->txpkt, skb->data, skb->len );
    if ( MHal_EMAC_Read_TSR() & EMAC_BNQ )
    {
        netif_stop_queue( dev );
        // Store packet information (to free when Tx completed) //
        LocPtr->skb = skb;
        LocPtr->skb_length = ( int ) skb->len;
        LocPtr->stats.tx_bytes += skb->len;
        //Set address of the data in the Transmit Address register //
        MHal_EMAC_Write_TAR(LocPtr->txpkt);
        // Set length of the packet in the Transmit Control register //
        MHal_EMAC_Write_TCR( LocPtr->skb_length );
        dev->trans_start = jiffies;
    }
#else
    u32 tx_abso_addr = dma_map_single( NULL,
                                       skb->data,
                                       skb->len,
                                       DMA_TO_DEVICE );
    if ( MHal_EMAC_Read_TSR() & EMAC_BNQ )
    {
        netif_stop_queue( dev );
        // Store packet information (to free when Tx completed) //
        LocPtr->skb = skb;
        Tx_SkbAddr = skb;
        LocPtr->skb_length = ( int ) skb->len;
        LocPtr->stats.tx_bytes += skb->len;
        //Set address of the data in the Transmit Address register //
        MHal_EMAC_Write_TAR( ( u32 ) tx_abso_addr );
        LocPtr->txpkt = tx_abso_addr;
        // Set length of the packet in the Transmit Control register //
        MHal_EMAC_Write_TCR( LocPtr->skb_length );
        dev->trans_start = jiffies;
    }
#endif
    else
    {
        printk( "EMAC TX Function  TRUE \n " );
        return 1;            // if we return anything but zero, dev.c:1055 calls kfree_skb(skb)
        //on this skb, he also reports -ENETDOWN and printk's, so either
        //we free and return(0) or don't free and return 1 //
    }
    return 0;
}

u32 MDev_EMAC_ReadRam32( u32 uRamAddr, u32 xoffset )
{
    return ( *( ( u32 * ) ( ( char * ) uRamAddr + xoffset ) ) );
}

//-------------------------------------------------------------------------------------------------
// Extract received frame from buffer descriptors and sent to upper layers.
// (Called from interrupt context)
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_rx( struct net_device* dev )
{
    struct EMAC_private *LocPtr = ( struct EMAC_private* ) dev->priv;
    struct recv_desc_bufs *dlist;
    unsigned char *p_recv;
    u32 pktlen;
    u32 uRegVal = 0;
    u32 retval = 0;
#ifdef SOFTWARE_DESCRIPTOR
    u32 RBQP_offset;
#else
    struct sk_buff* skb;
    u32 wrap_bit;
#endif

#ifndef INT_DELAY
    int count = 0;
#endif

    dlist = LocPtr->dlist ;
    //flush write buffer data
    Chip_Flush_Memory();
    //flush read buffer data
    ((volatile unsigned int *)0xA0010000)[0];

    // If any Ownership bit is 1, frame received.
    while ( ( dlist->descriptors[LocPtr->rxBuffIndex].addr ) & EMAC_DESC_DONE )
    {
#ifdef SOFTWARE_DESCRIPTOR
        p_recv = ( char * )
                 ( ( dlist->descriptors[LocPtr->rxBuffIndex].addr ) & ~( EMAC_DESC_DONE |
                                                                         EMAC_DESC_WRAP ) );
        pktlen = dlist->descriptors[LocPtr->rxBuffIndex].size & 0x7ff;     /* Length of frame including FCS */

        // the frame is not splitted in two parts //
        //memcpy(skb_put(rx_skb[LocPtr->rxBuffIndex], pktlen), p_recv, pktlen);
        skb_put( rx_skb[LocPtr->rxBuffIndex], pktlen );
        // update consumer pointer//
        rx_skb[LocPtr->rxBuffIndex]->dev = dev;
        rx_skb[LocPtr->rxBuffIndex]->protocol = eth_type_trans( rx_skb[LocPtr->rxBuffIndex],
                                                                dev );
        rx_skb[LocPtr->rxBuffIndex]->len = pktlen;
        dev->last_rx = jiffies;
        LocPtr->stats.rx_bytes += pktlen;
        retval = netif_rx( rx_skb[LocPtr->rxBuffIndex] );

        rx_skb[LocPtr->rxBuffIndex] = alloc_skb( SOFTWARE_DESC_LEN,
                                                 GFP_ATOMIC );

        rx_abso_addr[LocPtr->rxBuffIndex] = ( u32 )
                                            rx_skb[LocPtr->rxBuffIndex]->data;//(volatile u32) dma_alloc_coherent(NULL, (SOFTWARE_DESC_LEN*RBQP_LENG), &dmaaddr ,0);//dma_alloc_coherent;//(u32)rx_skb[idxRBQP];//(u32)skb_put(rx_skb[idxRBQP], SOFTWARE_DESC_LEN);//dma_map_single(NULL, rx_skb[idxRBQP]->data, SOFTWARE_DESC_LEN, DMA_FROM_DEVICE);

        RBQP_offset = LocPtr->rxBuffIndex * 8;
        if ( LocPtr->rxBuffIndex < ( MAX_RX_DESCR - 1 ) )
        {
            MHal_EMAC_WritRam32( RAM_ADDR_BASE,
                                 RBQP_BASE + RBQP_offset,
                                 rx_abso_addr[LocPtr->rxBuffIndex] );
        }
        else
        {
            MHal_EMAC_WritRam32( RAM_ADDR_BASE,
                                 RBQP_BASE +
                                 RBQP_offset,
                                 ( rx_abso_addr[LocPtr->rxBuffIndex] +
                                   EMAC_DESC_WRAP ) );
        }

        if ( dlist->descriptors[LocPtr->rxBuffIndex].size & EMAC_MULTICAST )
        {
            LocPtr->stats.multicast++;
        }
        dlist->descriptors[LocPtr->rxBuffIndex].addr &= ~EMAC_DESC_DONE;  /* reset ownership bit */

        //wrap after last buffer //
        LocPtr->rxBuffIndex++;
        if ( LocPtr->rxBuffIndex == MAX_RX_DESCR )
        {
            LocPtr->rxBuffIndex = 0;
        }

        uRegVal = ( u32 ) rx_skb[LocPtr->rxBuffIndex];
        MHal_EMAC_Write_RDPTR( uRegVal );
#else
        p_recv = ( char * )
                 ( ( ( ( dlist->descriptors[LocPtr->rxBuffIndex].addr ) & 0x0FFFFFFF ) |
                     RAM_ADDR_BASE ) & ~( EMAC_DESC_DONE | EMAC_DESC_WRAP ) );
        pktlen = dlist->descriptors[LocPtr->rxBuffIndex].size & 0x7ff;     /* Length of frame including FCS */

        skb = alloc_skb( pktlen + 6, GFP_ATOMIC );

        if ( skb != NULL )
        {
            skb_reserve( skb, 2 );

            if ( ( unsigned char * ) ( dlist->recv_buf + RX_BUFFER_SIZE ) >
                 ( p_recv + ROUND_SUP_4( pktlen ) ) )
            {
                // the frame is not splitted in two parts //
                memcpy( skb_put( skb, pktlen ), p_recv, pktlen );
                // update consumer pointer//
                uRegVal = MHal_EMAC_Read_RDPTR();
                uRegVal += ROUND_SUP_4( pktlen );
                MHal_EMAC_Write_RDPTR( uRegVal );
            }
            else
            {
                //the frame is splitted in two parts //
                int firstPacketPartSize = ( unsigned char* ) dlist->recv_buf +
                                          RX_BUFFER_SIZE -
                                          p_recv;
                int secondPacketSize = ( ROUND_SUP_4( pktlen ) -
                                         firstPacketPartSize );

                memcpy( skb_put( skb, firstPacketPartSize ),
                        p_recv,
                        firstPacketPartSize );
                memcpy( skb_put( skb, secondPacketSize ),
                        ( unsigned char * ) dlist->recv_buf,
                        secondPacketSize );
                // update consumer pointer and toggle the wrap bit //
                wrap_bit = ( MHal_EMAC_Read_RDPTR() & EMAC_WRAP_R ) ^ EMAC_WRAP_R;
                MHal_EMAC_Write_RDPTR( secondPacketSize | wrap_bit );
            }
            skb->dev = dev;
            skb->protocol = eth_type_trans( skb, dev );
            skb->len = pktlen;
            dev->last_rx = jiffies;
            LocPtr->stats.rx_bytes += pktlen;
            retval = netif_rx( skb );
        }
        else
        {
            LocPtr->stats.rx_dropped += 1;
        }

        if ( dlist->descriptors[LocPtr->rxBuffIndex].size & EMAC_MULTICAST )
        {
            LocPtr->stats.multicast++;
        }
        dlist->descriptors[LocPtr->rxBuffIndex].addr &= ~EMAC_DESC_DONE;  /* reset ownership bit */
        //wrap after last buffer //
        LocPtr->rxBuffIndex++;
        if ( LocPtr->rxBuffIndex == MAX_RX_DESCR )
        {
            LocPtr->rxBuffIndex = 0;
        }
#endif
#ifdef INT_DELAY
        if ( ThisUVE.flagRBNA == 0 )
        {
            xReceiveNum--;
            if ( xReceiveNum == 0 )
            {
                return 0;
            }
        }
#else
        if ( retval != 0 )
        {
            uRegVal = MHal_EMAC_Read_IDR();
            uRegVal |= ( EMAC_INT_RCOM | EMAC_INT_RBNA );
            MHal_EMAC_Write_IDR( uRegVal );
            EMAC_timer.expires = jiffies + 10;
            add_timer( &EMAC_timer );
            return 1;
        }

        if ( ++count > 5 )
        {
            return 0;
        }
#endif//#ifdef INT_DELAY
    }
#ifdef INT_DELAY
    xReceiveNum = 0;
    ThisUVE.flagRBNA = 0;
#endif
    return 0;
}

//-------------------------------------------------------------------------------------------------
//MAC interrupt handler
//-------------------------------------------------------------------------------------------------
#ifdef INT_DELAY
irqreturn_t MDev_EMAC_interrupt( int irq, void* dev_id )//irqreturn_t
{
    struct net_device* dev = ( struct net_device* ) dev_id;
    struct EMAC_private* LocPtr = ( struct EMAC_private* ) dev->priv;
    u32 intstatus = 0;
    u32 xReceiveFlag = 0;

    //MAC Interrupt Status register indicates what interrupts are pending.
    //It is automatically cleared once read.
    xoffsetValue = MHal_EMAC_Read_JULIAN_0108() & 0x0000FFFF;
    xReceiveNum += xoffsetValue & 0xFF;

    if ( xoffsetValue & 0x8000 )
    {
        xReceiveFlag = 1;
    }
    ThisUVE.flagRBNA = 0;
    while ( ( xReceiveFlag == 1 ) ||
            ( xReceiveNum != 0 ) ||
            ( intstatus = MHal_EMAC_Read_ISR() & ~MHal_EMAC_Read_IMR() ) )
    {
        if ( intstatus & EMAC_INT_RBNA )
        {
            LocPtr->stats.rx_dropped ++;
            ThisUVE.flagRBNA = 1;
            xReceiveFlag = 1;
        }

        // Transmit complete //
        if ( intstatus & EMAC_INT_TCOM )
        {
            // The TCOM bit is set even if the transmission failed. //
            if ( intstatus & ( EMAC_INT_TUND | EMAC_INT_RTRY ) )
            {
                LocPtr->stats.tx_errors += 1;
                if ( intstatus & EMAC_INT_RTRY )
                {
                    printk( "%s: Transmit RTRY error\n", dev->name );
                }

                if ( intstatus & EMAC_INT_TUND )
                {
                    if (5 > LocPtr->retx_count)
                    {
                        MDev_EMAC_retry(dev);
                        LocPtr->retx_count++;
                        printk("Tx TUND Retry: %u\n", LocPtr->retx_count);
                        goto end;
                    }
                    else
                    printk( "%s: Transmit TUND error\n", dev->name );
                }
            }
            else
                LocPtr->retx_count = 0;

            dev_kfree_skb_irq(LocPtr->skb );
            netif_wake_queue( dev );
        }

        if ( intstatus & EMAC_INT_DONE )
        {
            ThisUVE.flagISR_INT_DONE = 0x01;
        }
        //Overrun //
        if ( intstatus & EMAC_INT_ROVR )
        {
            LocPtr->stats.rx_dropped ++;
            printk( "%s: ROVR error\n", dev->name );
        }
        if ( xReceiveNum != 0 )
        {
            xReceiveFlag = 1;
        }
        // Receive complete //
        if ( xReceiveFlag == 1 )
        {
            xReceiveFlag = 0;
            MDev_EMAC_rx( dev );
        }
    }

end:
    //quit_int:
    return IRQ_HANDLED;
}
#else //#ifdef INT_DELAY
irqreturn_t MDev_EMAC_interrupt( int irq, void* dev_id )
{
    struct net_device* dev = ( struct net_device* ) dev_id;
    struct EMAC_private* LocPtr = ( struct EMAC_private* ) dev->priv;
    unsigned long intstatus;

    while ( ( intstatus = MHal_EMAC_Read_ISR() & ~MHal_EMAC_Read_IMR() ) )
    {
        //RX buffer not available//
        if ( intstatus & EMAC_INT_RBNA )
        {
            LocPtr->stats.rx_dropped ++;
        }
        // Receive complete //
        if ( intstatus & EMAC_INT_RCOM )
        {
            if ( MDev_EMAC_rx( dev ) )
            {
                goto quit_int;
            }
        }
        // Transmit complete //
        if ( intstatus & EMAC_INT_TCOM )
        {
            // The TCOM bit is set even if the transmission failed. //
            if ( intstatus & ( EMAC_INT_TUND | EMAC_INT_RTRY ) )
            {
                LocPtr->stats.tx_errors += 1;
                if ( intstatus & EMAC_INT_TUND )
                {
                    printk( "%s: Transmit TUND error\n", dev->name );
                }
                if ( intstatus & EMAC_INT_RTRY )
                {
                    printk( "%s: Transmit RTRY error\n", dev->name );
                }
            }
            netif_wake_queue( dev );
        }

        if ( intstatus & EMAC_INT_DONE )
        {
            ThisUVE.flagISR_INT_DONE = 0x01;
        }
        //Overrun //
        if ( intstatus & EMAC_INT_ROVR )
        {
            MDev_EMAC_rx( dev );
            LocPtr->stats.rx_dropped ++;
        }
    }
    quit_int:
    return IRQ_HANDLED;
}
#endif

//-------------------------------------------------------------------------------------------------
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
void MDev_EMAC_HW_init( void )
{
    u32 word_ETH_CTL = 0x00000000;
    u32 word_ETH_CFG = 0x00000000;
    u32 uJulian104Value = 0;
    u32 uNegPhyVal = 0;
#ifdef SOFTWARE_DESCRIPTOR
    u32 idxRBQP = 0;
    u32 RBQP_offset = 0;
#endif
    // (20071026_CHARLES) Disable TX, RX and MDIO:    (If RX still enabled, the RX buffer will be overwrited)
    MHal_EMAC_Write_CTL( word_ETH_CTL );
    // Init RX --------------------------------------------------------------
    memset( ( u8 * ) RAM_ADDR_BASE + RX_BUFFER_BASE, 0x00, RX_BUFFER_SIZE );

    MHal_EMAC_Write_BUFF( RX_BUFFER_BASE | RX_BUFFER_SEL );
    MHal_EMAC_Write_RDPTR( 0x00000000 );
    MHal_EMAC_Write_WRPTR( 0x00000000 );
    // Initialize "Receive Buffer Queue Pointer"
    MHal_EMAC_Write_RBQP( RBQP_BASE );
    // Initialize Receive Buffer Descriptors
    memset( ( u8 * ) RAM_ADDR_BASE + RBQP_BASE, 0x00, 0x2000 );          // Clear for max(8*1024)bytes (max:1024 descriptors)
    MHal_EMAC_WritRam32( RAM_ADDR_BASE,
                         ( RBQP_BASE + RBQP_SIZE - 0x08 ),
                         0x00000002 );                 // (n-1) : Wrap = 1

#ifdef INT_DELAY
    //Reg_rx_frame_cyc[15:8] -0xFF range 1~255
    //Reg_rx_frame_num[7:0]  -0x05 receive frames per INT.
    //0x80 Enable interrupt delay mode.
    //register 0x104 receive counter need to modify smaller for ping
    //Modify bigger(need small than 8) for throughput
    uJulian104Value = JULIAN_104_VAL;//0xFF050080;//receive counter : 0x5
    MHal_EMAC_Write_JULIAN_0104( uJulian104Value );
    MHal_EMAC_Write_IER( 0x000004B5 );
#else
    // Enable Interrupts ----------------------------------------------------
    MHal_EMAC_Write_JULIAN_0104( 0x00000000 );
    MHal_EMAC_Write_IER( 0x0000FFFF );
#endif
    // Set MAC address ------------------------------------------------------
    MHal_EMAC_Write_SA1_MAC_Address( ThisBCE.sa1[0],
                                     ThisBCE.sa1[1],
                                     ThisBCE.sa1[2],
                                     ThisBCE.sa1[3],
                                     ThisBCE.sa1[4],
                                     ThisBCE.sa1[5] );
    MHal_EMAC_Write_SA2_MAC_Address( ThisBCE.sa2[0],
                                     ThisBCE.sa2[1],
                                     ThisBCE.sa2[2],
                                     ThisBCE.sa2[3],
                                     ThisBCE.sa2[4],
                                     ThisBCE.sa2[5] );
    MHal_EMAC_Write_SA3_MAC_Address( ThisBCE.sa3[0],
                                     ThisBCE.sa3[1],
                                     ThisBCE.sa3[2],
                                     ThisBCE.sa3[3],
                                     ThisBCE.sa3[4],
                                     ThisBCE.sa3[5] );
    MHal_EMAC_Write_SA4_MAC_Address( ThisBCE.sa4[0],
                                     ThisBCE.sa4[1],
                                     ThisBCE.sa4[2],
                                     ThisBCE.sa4[3],
                                     ThisBCE.sa4[4],
                                     ThisBCE.sa4[5] );

#ifdef SOFTWARE_DESCRIPTOR
#ifdef RX_CHECKSUM
    uJulian104Value = uJulian104Value |
                      ( CHECKSUM_ENABLE | SOFTWARE_DESCRIPTOR_ENABLE );
#else
    uJulian104Value = uJulian104Value | SOFTWARE_DESCRIPTOR_ENABLE;
#endif

    MHal_EMAC_Write_JULIAN_0104( uJulian104Value );
    for ( idxRBQP = 0; idxRBQP < RBQP_LENG; idxRBQP++ )
    {
#ifdef SOFTWARE_DESCRIPTOR
        rx_skb[idxRBQP] = alloc_skb( SOFTWARE_DESC_LEN, GFP_ATOMIC );

        rx_abso_addr[idxRBQP] = ( u32 ) rx_skb[idxRBQP]->data;//(volatile u32) dma_alloc_coherent(NULL, (SOFTWARE_DESC_LEN*RBQP_LENG), &dmaaddr ,0);//dma_alloc_coherent;//(u32)rx_skb[idxRBQP];//(u32)skb_put(rx_skb[idxRBQP], SOFTWARE_DESC_LEN);//dma_map_single(NULL, rx_skb[idxRBQP]->data, SOFTWARE_DESC_LEN, DMA_FROM_DEVICE);
        RBQP_offset = idxRBQP * 8;
        if ( idxRBQP < ( RBQP_LENG - 1 ) )
        {
            MHal_EMAC_WritRam32( RAM_ADDR_BASE,
                                 RBQP_BASE + RBQP_offset,
                                 rx_abso_addr[idxRBQP] );
        }
        else
        {
            MHal_EMAC_WritRam32( RAM_ADDR_BASE,
                                 RBQP_BASE + RBQP_offset,
                                 ( rx_abso_addr[idxRBQP] + EMAC_DESC_WRAP ) );
        }
#else
        RBQP_offset = idxRBQP * 8;
        if ( idxRBQP < ( RBQP_LENG - 1 ) )
        {
            MHal_EMAC_WritRam32( RAM_ADDR_BASE,
                                 RBQP_BASE +
                                 RBQP_offset,
                                 ( RX_BUFFER_BASE +
                                   SOFTWARE_DESC_LEN * idxRBQP ) );
        }
        else
        {
            MHal_EMAC_WritRam32( RAM_ADDR_BASE,
                                 RBQP_BASE +
                                 RBQP_offset,
                                 ( RX_BUFFER_BASE +
                                   SOFTWARE_DESC_LEN * idxRBQP +
                                   EMAC_DESC_WRAP ) );
        }
#endif
    }
#endif //#ifdef SOFTWARE_DESCRIPTOR
    MHal_EMAC_Write_MAN( 0x50028000 | ( phyaddr << PHY_ADDR_OFFSET ) );
    // IMPORTANT: Run NegotiationPHY() before writing REG_ETH_CFG.
    uNegPhyVal = MHal_EMAC_NegotiationPHY();
    if ( uNegPhyVal == 0x01 )
    {
        ThisUVE.flagMacTxPermit = 0x01;
        ThisBCE.duplex = 1;
    }
    else if ( uNegPhyVal == 0x02 )
    {
        ThisUVE.flagMacTxPermit = 0x01;
        ThisBCE.duplex = 2;
    }

    // ETH_CFG Register -----------------------------------------------------
    word_ETH_CFG = 0x00000800;          // Init: CLK = 0x2
    // (20070808) IMPORTANT: REG_ETH_CFG:bit1(FD), 1:TX will halt running RX frame, 0:TX will not halt running RX frame.
    // If always set FD=0, no CRC error will occur. But throughput maybe need re-evaluate.
    // IMPORTANT: (20070809) NO_MANUAL_SET_DUPLEX : The real duplex is returned by "negotiation"
    if ( ThisBCE.speed == EMAC_SPEED_100 )
    {
//dhjung LGE
//		printk("100M\n");
        word_ETH_CFG |= 0x00000001;
    }
    if ( ThisBCE.duplex == 2 )
    {
//dhjung LGE
//		printk("Full-Duplex\n");
        word_ETH_CFG |= 0x00000002;
    }
    if ( ThisBCE.cam == 1 )
    {
        word_ETH_CFG |= 0x00000200;
    }
    if ( ThisBCE.rcv_bcast == 0 )
    {
        word_ETH_CFG |= 0x00000020;
    }
    if ( ThisBCE.rlf == 1 )
    {
        word_ETH_CFG |= 0x00000100;
    }

    MHal_EMAC_Write_CFG( word_ETH_CFG );

    // ETH_CTL Register -----------------------------------------------------
    word_ETH_CTL = 0x0000000C;                                  // Enable transmit and receive : TE + RE = 0x0C (Disable MDIO)
    if ( ThisBCE.wes == 1 )
    {
        word_ETH_CTL |= 0x00000080;
    }

    MHal_EMAC_Write_CTL( word_ETH_CTL );
    MHal_EMAC_Write_JULIAN_0100( JULIAN_100_VAL );
    ThisUVE.flagPowerOn = 1;
    ThisUVE.initedEMAC = 1;
}

//-------------------------------------------------------------------------------------------------
// EMAC init Variable
//-------------------------------------------------------------------------------------------------
static u32 MDev_EMAC_VarInit( void )
{
    //temporary and fix after Alan fix SYS MMAP
    u32 alloRAM_ADDR_BASE = 0, alloRAM_SIZE = 0;

    if (false == MDrv_SYS_GetMMAP( E_SYS_MMAP_EMAC, &alloRAM_ADDR_BASE, &alloRAM_SIZE))
    {
        dma_addr_t dmaaddr;

        // dma_alloc_coherent
        alloRAM_ADDR_BASE = ( volatile u32 )dma_alloc_coherent( NULL,
                                EMAC_ABSO_MEM_SIZE,
                                &dmaaddr,
                                0);

        if ( !alloRAM_ADDR_BASE )
        {
            printk( "EMAC init fail (Gat base address fail) !!!\n" );
            return 0;
        }
    }
    else
        alloRAM_ADDR_BASE += EMAC_ABSO_MEM_BASE;    // noncacheable

    memset( ( u32 * ) alloRAM_ADDR_BASE, 0x00, EMAC_ABSO_MEM_SIZE );
    RAM_ADDR_BASE = ( alloRAM_ADDR_BASE & 0xFFFFC000 ) + RX_BUFFER_SIZE;    // IMPORTANT: Let lowest 14 bits as zero.
    RX_BUFFER_BASE += RAM_ADDR_BASE - EMAC_ABSO_MEM_BASE;
    RBQP_BASE += RAM_ADDR_BASE - EMAC_ABSO_MEM_BASE;
    TX_BUFFER_BASE += RAM_ADDR_BASE - EMAC_ABSO_MEM_BASE;
    RX_FRAME_ADDR += RAM_ADDR_BASE - EMAC_ABSO_MEM_BASE;
    RAM_ADDR_BASE = EMAC_ABSO_MEM_BASE;  // IMPORTANT_TRICK_20070512
    TX_SKB_BASE = TX_BUFFER_BASE + MAX_RX_DESCR * sizeof( struct rbf_t );

    memset( &ThisBCE, 0x00, sizeof( BasicConfigEMAC ) );
    memset( &ThisUVE, 0x00, sizeof( UtilityVarsEMAC ) );

    ThisBCE.wes = 0;                          // 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    ThisBCE.duplex = 2;                          // 1:Half-duplex, 2:Full-duplex
    ThisBCE.cam = 0;                      // 0:No CAM, 1:Yes
    ThisBCE.rcv_bcast = 0;                        // 0:No, 1:Yes
    ThisBCE.rlf = 0;                      // 0:No, 1:Yes receive long frame(1522)
    ThisBCE.rcv_bcast = 1;
    ThisBCE.speed = EMAC_SPEED_100;
    ThisBCE.sa1[0] = MY_MAC[0];
    ThisBCE.sa1[1] = MY_MAC[1];
    ThisBCE.sa1[2] = MY_MAC[2];
    ThisBCE.sa1[3] = MY_MAC[3];
    ThisBCE.sa1[4] = MY_MAC[4];
    ThisBCE.sa1[5] = MY_MAC[5];
    return alloRAM_ADDR_BASE;//TRUE;
}

//-------------------------------------------------------------------------------------------------
// Initialize the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------
static unsigned int u32EMACIntRegister;
static int MDev_EMAC_setup( struct net_device* dev, unsigned long phy_type )
{
    struct EMAC_private* LocPtr;

    static int already_initialized = 0;
    dma_addr_t dmaaddr;
    u32 val;
    u32 RetAddr;
    if ( already_initialized )
    {
        return FALSE;
    }

    RetAddr = MDev_EMAC_VarInit();

    if ( !RetAddr )
    {
        return FALSE;
    }

    LocPtr = ( struct EMAC_private * ) RetAddr;//alloRAM_ADDR_BASE;

    if ( LocPtr == NULL )
    {
        free_irq( dev->irq, dev );
        return -ENOMEM;
    }

    dev->base_addr = ( long ) REG_ADDR_BASE;
    MDev_EMAC_HW_init();
    dev->irq = E_IRQ_EMAC;

    //Install the interrupt handler //
    //Notes: Modify linux/kernel/irq/manage.c  /* interrupt.h */
    if(0 == u32EMACIntRegister) {
    if ( request_irq( dev->irq,
                      MDev_EMAC_interrupt,
                      SA_INTERRUPT,
                      dev->name,
	                      dev ) ) {
        return -EBUSY;
    }
		u32EMACIntRegister = 1;
    } else {
		disable_irq(dev->irq);
    	enable_irq(dev->irq);
    }

    dev->priv = LocPtr;

    // Allocate memory for DMA Receive descriptors //
    LocPtr->dlist_phys = LocPtr->dlist = ( struct recv_desc_bufs * )
                         ( RX_BUFFER_BASE + RAM_ADDR_BASE );

    if ( LocPtr->dlist == NULL )
    {
        dma_free_noncoherent( dev->priv, EMAC_ABSO_MEM_SIZE, &dmaaddr, 0 );//kfree (dev->priv);
        free_irq( dev->irq, dev );
        return -ENOMEM;
    }

    spin_lock_init( &LocPtr->lock );
    ether_setup( dev );

    dev->open = MDev_EMAC_open;
    dev->stop = MDev_EMAC_close;
    dev->hard_start_xmit = MDev_EMAC_tx;
    dev->get_stats = MDev_EMAC_stats;
    dev->set_multicast_list = MDev_EMAC_set_rx_mode;
    dev->do_ioctl = MDev_EMAC_ioctl;
    dev->set_mac_address = MDev_EMAC_set_mac_address;
    dev->tx_queue_len = EMAC_MAX_TX_QUEUE;

    MDev_EMAC_get_mac_address( dev );     // Get ethernet address and store it in dev->dev_addr //
    MDev_EMAC_update_mac_address( dev ); // Program ethernet address into MAC //
    spin_lock_irq( &LocPtr->lock );
    MHal_EMAC_enable_mdi();

    MHal_EMAC_read_phy( phyaddr, MII_USCR_REG, &val );
    if ( ( val & ( 1 << 10 ) ) == 0 )    // DSCR bit 10 is 0 -- fiber mode //
    {
        LocPtr->phy_media = PORT_FIBRE;
    }
    spin_unlock_irq( &LocPtr->lock );

    //Support for ethtool //
    LocPtr->mii.dev = dev;
    LocPtr->mii.mdio_read = MDev_EMAC_mdio_read;
    LocPtr->mii.mdio_write = MDev_EMAC_mdio_write;

    already_initialized = 1;
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Detect MAC and PHY and perform initialization
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_probe( struct net_device* dev )
{
    int detected = -1;

    /* Read the PHY ID registers - try all addresses */
    detected = MDev_EMAC_setup( dev, MII_URANUS_ID );
    return detected;
}

//-------------------------------------------------------------------------------------------------
// EMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_timer_callback( unsigned long value )
{
    int ret = 0;

#ifndef INT_DELAY
    if (EMAC_RX_TMR == value)
        MHal_EMAC_timer_callback( value );
#endif

    ret = MDev_EMAC_update_linkspeed(emac_dev);
    if (0 == ret)
    {
        if ( !ThisBCE.connected )
        {
            ThisBCE.connected = 1;
            netif_carrier_on(emac_dev);
        }
    }
    else if (-1 == ret )    //no link
    {
        ThisBCE.connected = 0;
        netif_carrier_off( emac_dev );
    }

    Link_timer.expires = jiffies + EMAC_CHECK_LINK_TIME;
    add_timer(&Link_timer);
}

//-------------------------------------------------------------------------------------------------
// EMAC MACADDR Setup
//-------------------------------------------------------------------------------------------------

#define MACADDR_FORMAT "XX:XX:XX:XX:XX:XX"

static int __init macaddr_auto_config_setup( char* addrs )
{
    if ( strlen( addrs ) == strlen( MACADDR_FORMAT ) &&
         ':' == addrs[2] &&
         ':' == addrs[5] &&
         ':' == addrs[8] &&
         ':' == addrs[11] &&
         ':' == addrs[14] )
    {
        addrs[2] = '\0';
        addrs[5] = '\0';
        addrs[8] = '\0';
        addrs[11] = '\0';
        addrs[14] = '\0';

        MY_MAC[0] = ( u8 ) simple_strtoul( &( addrs[0] ), NULL, 16 );
        MY_MAC[1] = ( u8 ) simple_strtoul( &( addrs[3] ), NULL, 16 );
        MY_MAC[2] = ( u8 ) simple_strtoul( &( addrs[6] ), NULL, 16 );
        MY_MAC[3] = ( u8 ) simple_strtoul( &( addrs[9] ), NULL, 16 );
        MY_MAC[4] = ( u8 ) simple_strtoul( &( addrs[12] ), NULL, 16 );
        MY_MAC[5] = ( u8 ) simple_strtoul( &( addrs[15] ), NULL, 16 );

        /* set back to ':' or the environment variable would be destoried */ // REVIEW: this coding style is dangerous
        addrs[2] = ':';
        addrs[5] = ':';
        addrs[8] = ':';
        addrs[11] = ':';
        addrs[14] = ':';
    }

    return 1;
}

__setup( "macaddr=", macaddr_auto_config_setup );

//-------------------------------------------------------------------------------------------------
// Clock management
//-------------------------------------------------------------------------------------------------
void MDev_EMAC_PowerOn(void)
{
    MHal_EMAC_Power_On_Clk();
}

void MDev_EMAC_PowerOff(void)
{
    MHal_EMAC_Power_Off_Clk();
}

//-------------------------------------------------------------------------------------------------
// EMAC init module
//-------------------------------------------------------------------------------------------------
//dhjung LGE : void -> int
static int MDev_EMAC_ScanPHYAddr(void)
{
    unsigned char addr = 0;
    u32 value = 0;

    MHal_EMAC_enable_mdi();
    do
    {
        MHal_EMAC_read_phy(addr, MII_BMSR, &value);
        if (0 != value && 0x0000FFFF != value)
        {
//dhjung LGE
            printk("PHY Addr : %u\n", addr);
            break;
        }
    }while(++addr && addr < 32);
    MHal_EMAC_disable_mdi();
    phyaddr = addr;

//dhjung LGE
	if (value == 0 || value == 0x0000FFFF)
		return -1;
	else
		return 0;
}

//static int __init MDev_EMAC_init_module( void )
static int MDev_EMAC_init_module( void )
{
    MDev_EMAC_PowerOn();
//dhjung LGE
//	strcpy( emac_dev->name, "EMAC" );

    init_timer( &EMAC_timer );
    EMAC_timer.data = EMAC_RX_TMR;
    EMAC_timer.function = MDev_EMAC_timer_callback;
    EMAC_timer.expires = jiffies;

//dhjung LGE
    if (MDev_EMAC_ScanPHYAddr() < 0)
		goto ret;

    emac_dev = alloc_etherdev( 0 );
    if ( !emac_dev )
    {
        printk( KERN_ERR "No EMAC dev mem!\n" );
        return -ENOMEM;
    }
    if ( !MDev_EMAC_probe( emac_dev ) )
        return register_netdev( emac_dev );

    free_netdev( emac_dev );
    emac_dev = 0;
    printk( KERN_ERR "Init EMAC error!\n" );

//dhjung LGE
ret :
    return -1;
}
//-------------------------------------------------------------------------------------------------
// EMAC exit module
//-------------------------------------------------------------------------------------------------
static void __exit MDev_EMAC_exit_module( void )
{
    if ( emac_dev )
    {
#ifndef INT_DELAY
        del_timer(&EMAC_timer);
#endif

        unregister_netdev( emac_dev );
        free_netdev( emac_dev );
    }
}

//module_init( MDev_EMAC_init_module );
user_initcall( MDev_EMAC_init_module );
module_exit( MDev_EMAC_exit_module );

MODULE_AUTHOR( "MSTAR" );
MODULE_DESCRIPTION( "EMAC Ethernet driver" );
MODULE_LICENSE( "MSTAR" );
