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
/// file    mmap_64mb_64mb.h
/// @brief  Memory mapping for 64MB+64MB RAM
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MS_MMAP_64MB_64MB_H_
#define _MS_MMAP_64MB_64MB_H_

// Memory alignment
#define MemAlignUnit     							64UL
#define MemAlign(n, unit)							((((n)+(unit)-1)/(unit))*(unit))

#if	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_256MDDR_BOARD)
#define MIU_DRAM_LEN								(0x10000000)
#else
#define MIU_DRAM_LEN								(0xC000000)
#endif

//----------------------------------------------------------------------
// MIU 0
//----------------------------------------------------------------------
// Linux kernel space
#define LINUX_MEM_AVAILABLE							0x00000000UL
#define LINUX_MEM_BASE_ADR 							(LINUX_MEM_AVAILABLE)
#define LINUX_MEM_GAP_CHK  							(LINUX_MEM_BASE_ADR-LINUX_MEM_AVAILABLE)
//dhjung LGE
#if 1

#define LINUX_MEM_LEN								(0xBC0000) // 12MB - 256K

// for MVD and TSP binary dhjung LGE
#define BIN_MEM_AVAILABLE   						(LINUX_MEM_BASE_ADR + LINUX_MEM_LEN)
#define BIN_MEM_ADR         						MemAlign(BIN_MEM_AVAILABLE, 4096)
#define BIN_MEM_GAP_CHK     						(BIN_MEM_ADR-BIN_MEM_AVAILABLE)
#define BIN_MEM_LEN    								0x40000

// must start at nnn0000 - 512k alignment   --[Dean] be careful allocate this big alignment area
#define MAD_BASE_AVAILABLE 							(BIN_MEM_ADR+BIN_MEM_LEN)
#define MAD_BASE_BUFFER_ADR							MemAlign(MAD_BASE_AVAILABLE, 0x80000)
#define MAD_BASE_BUFFER_ADR_GAP_CHK	(MAD_BASE_BUFFER_ADR-MAD_BASE_AVAILABLE)
#define MAD_BASE_BUFFER_LEN        					0x280000 //2.5M // 3MB -> 2.5MB, samuel, 20081105

#else

#define LINUX_MEM_LEN								(0xC00000) // 12MB

// must start at nnn0000 - 512k alignment   --[Dean] be careful allocate this big alignment area
#define MAD_BASE_AVAILABLE 							(LINUX_MEM_BASE_ADR+LINUX_MEM_LEN)
#define MAD_BASE_BUFFER_ADR							MemAlign(MAD_BASE_AVAILABLE, 0x80000)
#define MAD_BASE_BUFFER_ADR_GAP_CHK	(MAD_BASE_BUFFER_ADR-MAD_BASE_AVAILABLE)
#define MAD_BASE_BUFFER_LEN							0x280000 //2.5M  // 3MB -> 2.5MB, samuel, 20081105

#endif

//======================================================================
// Can not add any buffer here (in between VE_FRAMEBUFFER_ADR and SCALER_DNR_BUF_ADR)
// Because USB/OAD download use the buffer from VE_FRAMEBUFFER_ADR to the end of SCALER_DNR_BUF_ADR
//======================================================================

// For Maximum is 1920x1088x3x2 about 12MB in HDMI and YPbPr
// Let it overwrite EVENTDB and MHEG5 buffer(These buffers is DTV only)
#define SCALER_DNR_AVAILABLE  						(MAD_BASE_AVAILABLE + MAD_BASE_BUFFER_LEN)
#define SCALER_DNR_BUF_ADR	  						MemAlign(SCALER_DNR_AVAILABLE, 8)
#define SCALER_DNR_GAP_CHK	  						(SCALER_DNR_BUF_ADR-SCALER_DNR_AVAILABLE)
#define SCALER_DNR_BUF_LEN	  						0x1000000-80 //16MB-80byte  //0xC00000  //12582912	// 12MB

#define SCALER_DNR_W_AVAILABLE						(SCALER_DNR_BUF_ADR + SCALER_DNR_BUF_LEN)
#define SCALER_DNR_W_BARRIER_ADR					(SCALER_DNR_W_AVAILABLE)
#define SCALER_DNR_W_GAP_CHK						(SCALER_DNR_W_BARRIER_ADR-SCALER_DNR_W_AVAILABLE)
#define SCALER_DNR_W_BARRIER_LEN					80//16		// DNR submit 2 64-bit data before compare limit
#define SCALER_DNR_BUF_LEN_EXT						((((896UL-736UL)*3+0x0F) & ~0x0F) * 581UL *2) // the output size of VD will be 848 * 581

// no use in ATSC
#define RLD_BUF_AVAILABLE							(SCALER_DNR_W_BARRIER_ADR+SCALER_DNR_W_BARRIER_LEN)
#define RLD_BUF_ADR									MemAlign(RLD_BUF_AVAILABLE, 8)
#define RLD_BUF_GAP_CHK								(RLD_BUF_ADR-RLD_BUF_AVAILABLE)
#define RLD_BUF_LEN									0xDD000//905216

// TSP buffer, TSP extract from mpool, samuel, 20081105
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define TSP_BUF_AVAILABLE							(SCALER_DNR_W_BARRIER_ADR+SCALER_DNR_W_BARRIER_LEN)
#else
#define TSP_BUF_AVAILABLE							(RLD_BUF_ADR + RLD_BUF_LEN)
#endif
#define TSP_BUF_ADR 	 							MemAlign(TSP_BUF_AVAILABLE, 32)
#define TSP_BUF_GAP_CHK  							(TSP_BUF_ADR-TSP_BUF_AVAILABLE)
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define TSP_BUF_LEN									(0x300000) //5MB => 3MB
#else
#define TSP_BUF_LEN									(0x800000) //8MB
#endif

//#define VE_FRAMEBUFFER_AVAILABLE					(MVD_BITSTREAM_ADR + MVD_BITSTREAM_LEN)
#define VE_FRAMEBUFFER_AVAILABLE  					(TSP_BUF_ADR+TSP_BUF_LEN)
#define VE_FRAMEBUFFER_ADR		  					MemAlign(VE_FRAMEBUFFER_AVAILABLE, 8)
#define VE_FRAMEBUFFER_ADR_GAP_CHK					(VE_FRAMEBUFFER_ADR-VE_FRAMEBUFFER_AVAILABLE)
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define VE_FRAMEBUFFER_LEN							0 // samuel,20081112, not used in ATSC
#else
#define VE_FRAMEBUFFER_LEN							(0x195000UL + 0x2800) // 720*576*2*2 + 10KByte(ttx insertion)
#endif


#define TTX_BUF_AVAILABLE							(VE_FRAMEBUFFER_ADR + VE_FRAMEBUFFER_LEN)
#define TTX_BUF_ADR									MemAlign(TTX_BUF_AVAILABLE, 4096)
#define TTX_BUF_GAP_CHK								(TTX_BUF_ADR-TTX_BUF_AVAILABLE)
#define TTX_BUF_LEN									0x40000  // 200KB is enough, but MVD co-buf here for callback data, so need 256KB, samuel, 20081105

// no use in ATSC
#define MVD_SW_AVAILABLE   							(RLD_BUF_ADR + RLD_BUF_LEN)
#define MVD_SW_ADR         							MemAlign(MVD_SW_AVAILABLE, 8)
#define MVD_SW_ADR_GAP_CHK 							(MVD_SW_ADR-MVD_SW_AVAILABLE)
#define MVD_SW_LEN         							0xB00000 //0x900000//0xB00000	//11MB

#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
// in ATSC, just skip MVD_SW (no need to play movie)
#define MPOOL_AVAILABLE								(TTX_BUF_ADR + TTX_BUF_LEN)
#else
#define MPOOL_AVAILABLE								(MVD_SW_ADR + MVD_SW_LEN)
#endif
#define MPOOL_ADR	 								MemAlign(MPOOL_AVAILABLE, 4096)
#define MPOOL_GAP_CHK								(MPOOL_ADR-MPOOL_AVAILABLE)
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define MPOOL_LEN									(0x4000000 - MPOOL_ADR - 0x1000000 - 0x400000 /*released from POSD0*/)
#else
#if 1 // temporariliy changed by streamerj, 090527
#define MPOOL_LEN									(0x8000000 - MPOOL_ADR - 0x3A00000 - 0x400000 + 0x200000 /*released from POSD0*/)
#else
#define MPOOL_LEN									(0x8000000 - MPOOL_ADR - 0x3A00000 - 0x400000 + 0x200000 + 0x800000 /*released from POSD0*/)
#endif
#endif

// dhjung LGE
#define LINUX_2ND_MEM_AVAILABLE		(MPOOL_ADR + MPOOL_LEN)
#define LINUX_2ND_MEM_ADDR			MemAlign(LINUX_2ND_MEM_AVAILABLE, 32)
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define LINUX_2ND_MEM_LEN							(0x4000000 - LINUX_2ND_MEM_ADDR)
#else
#define LINUX_2ND_MEM_LEN							(0x8000000 - LINUX_2ND_MEM_ADDR)
#endif

//----------------------------------------------------------------------
// MIU 1
//----------------------------------------------------------------------
#define SVD_CPU_AVAILABLE  							(0x8000000)
#define SVD_CPU_ADR        							MemAlign(SVD_CPU_AVAILABLE, 2048)
#define SVD_CPU_ADR_GAP_CHK							(SVD_CPU_ADR-SVD_CPU_AVAILABLE)
#define SVD_CPU_LEN        							0x100000 //1024KB

// VD_3DCOMB (co-buf with MVD_FRAMEBUFFER ), move to miu1, samuel, 20081105
#define VD_3DCOMB_AVAILABLE							(SVD_CPU_ADR + SVD_CPU_LEN)
#define VD_3DCOMB_BASE_ADR 							MemAlign(VD_3DCOMB_AVAILABLE, 8)
#define VD_3DCOMB_GAP_CHK  							(VD_3DCOMB_BASE_ADR-VD_3DCOMB_AVAILABLE)
#define VD_3DCOMB_LEN      							(0x400000) //4MB

// need 512 byte alignment SD
#define MVD_FRAMEBUFFER_AVAILABLE					(SVD_CPU_ADR + SVD_CPU_LEN)
#define MVD_FRAMEBUFFER_ADR      					MemAlign(MVD_FRAMEBUFFER_AVAILABLE, 2048)
#define MVD_FRAMEBUFFER_ADR_GAP_CHK (MVD_FRAMEBUFFER_ADR-MVD_FRAMEBUFFER_AVAILABLE)
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define MVD_FRAMEBUFFER_LEN							0xE00000 //14MB
#else
#define MVD_FRAMEBUFFER_LEN							0x1B00000 //27MB: Dean
#endif

// need 8 byte alignment
#define MVD_BITSTREAM_AVAILABLE						(MVD_FRAMEBUFFER_ADR + MVD_FRAMEBUFFER_LEN)
#define MVD_BITSTREAM_ADR							MemAlign(MVD_BITSTREAM_AVAILABLE, 8)
#define MVD_BITSTREAM_ADR_GAP_CHK					(MVD_BITSTREAM_ADR-MVD_BITSTREAM_AVAILABLE)
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define MVD_BITSTREAM_LEN 							0x400000 //4MB
#else
#define MVD_BITSTREAM_LEN 							0x400000 //6MB->4MB     20081028 samuel
#endif

#define JPD_OUTPUT_AVAILABLE  						(MVD_BITSTREAM_ADR + MVD_BITSTREAM_LEN)
#define JPD_OUTPUT_ADR        						MemAlign(JPD_OUTPUT_AVAILABLE, 8)
#define JPD_OUTPUT_ADR_GAP_CHK						(JPD_OUTPUT_ADR-JPD_OUTPUT_AVAILABLE)
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define JPD_OUTPUT_LEN								0x400000 //4MB
#else
#define JPD_OUTPUT_LEN								0x200000 // not used -> 0x200000    20081028 samuel
#endif

#define AUDIO_CLIP_AVAILABLE  						(JPD_OUTPUT_ADR + JPD_OUTPUT_LEN)
#define AUDIO_CLIP_ADR        						MemAlign(AUDIO_CLIP_AVAILABLE, 8)
#define AUDIO_CLIP_ADR_GAP_CHK						(AUDIO_CLIP_ADR-AUDIO_CLIP_AVAILABLE)
#define AUDIO_CLIP_LEN        						0xA00000 // 10MB

#define PVR_DOWNLOAD_BUFFER_AVAILABLE				(JPD_OUTPUT_ADR + JPD_OUTPUT_LEN)
#define PVR_DOWNLOAD_BUFFER_ADR      				MemAlign(PVR_DOWNLOAD_BUFFER_AVAILABLE, 8)   // for Window1 alignment
#define PVR_DOWNLOAD_BUFFER_GAP_CHK  				(PVR_DOWNLOAD_BUFFER_ADR-PVR_DOWNLOAD_BUFFER_AVAILABLE)
#define PVR_DOWNLOAD_BUFFER_LEN      				0x60000 // 384K

#define PVR_DOWNLOAD_STREAM_BUFFER_AVAILABLE		(PVR_DOWNLOAD_BUFFER_ADR + PVR_DOWNLOAD_BUFFER_LEN)
#define PVR_DOWNLOAD_STREAM_BUFFER_ADR      		MemAlign(PVR_DOWNLOAD_STREAM_BUFFER_AVAILABLE, 8)   // for Window1 alignment
#define PVR_DOWNLOAD_STREAM_BUFFER_GAP_CHK  		(PVR_DOWNLOAD_STREAM_BUFFER_ADR-PVR_DOWNLOAD_STREAM_BUFFER_AVAILABLE)
#define PVR_DOWNLOAD_STREAM_BUFFER_LEN      		0x18000 // 96K

#define PVR_UPLOAD_BUFFER_AVAILABLE					(PVR_DOWNLOAD_STREAM_BUFFER_ADR + PVR_DOWNLOAD_STREAM_BUFFER_LEN)
#define PVR_UPLOAD_BUFFER_ADR      					MemAlign(PVR_UPLOAD_BUFFER_AVAILABLE, 8)
#define PVR_UPLOAD_BUFFER_GAP_CHK  					(PVR_UPLOAD_BUFFER_ADR-PVR_UPLOAD_BUFFER_AVAILABLE)
#define PVR_UPLOAD_BUFFER_LEN      					0x60000 // 384K

#define PVR_UPLOAD_STREAM_BUFFER_AVAILABLE			(PVR_UPLOAD_BUFFER_ADR + PVR_UPLOAD_BUFFER_LEN)
#define PVR_UPLOAD_STREAM_BUFFER_ADR      			MemAlign(PVR_UPLOAD_STREAM_BUFFER_AVAILABLE, 8)   // for Window1 alignment
#define PVR_UPLOAD_STREAM_BUFFER_GAP_CHK  			(PVR_UPLOAD_STREAM_BUFFER_ADR-PVR_UPLOAD_STREAM_BUFFER_AVAILABLE)
#define PVR_UPLOAD_STREAM_BUFFER_LEN      			0x18000 // 96K

#define PVR_THUMBNAIL_DECODE_BUFFER_AVAILABLE		(PVR_UPLOAD_STREAM_BUFFER_ADR + PVR_UPLOAD_STREAM_BUFFER_LEN)
#define PVR_THUMBNAIL_DECODE_BUFFER_ADR      		MemAlign(PVR_THUMBNAIL_DECODE_BUFFER_AVAILABLE, 8)
#define PVR_THUMBNAIL_DECODE_BUFFER_GAP_CHK  		(PVR_THUMBNAIL_DECODE_BUFFER_ADR-PVR_THUMBNAIL_DECODE_BUFFER_AVAILABLE)
#define PVR_THUMBNAIL_DECODE_BUFFER_LEN      		0x900000 // 9MB

#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define MAILBOX_AVAILABLE							(JPD_OUTPUT_ADR + JPD_OUTPUT_LEN)
#else
#define MAILBOX_AVAILABLE							(AUDIO_CLIP_AVAILABLE + AUDIO_CLIP_LEN)
#endif
#define MAILBOX_ADR      							MemAlign(MAILBOX_AVAILABLE, 8)
#define MAILBOX_GAP_CHK  							(MAILBOX_ADR-MAILBOX_AVAILABLE)
#define MAILBOX_LEN      							0xA000 // 32 KB

#define POSD0_AVAILABLE  							(MAILBOX_ADR + MAILBOX_LEN)
#define POSD0_ADR        							MemAlign(POSD0_AVAILABLE, 8)
#define POSD0_ADR_GAP_CHK							(POSD0_ADR-POSD0_AVAILABLE)
#define POSD0_LEN        							0x410000

#define POSD1_AVAILABLE  							(POSD0_ADR + POSD0_LEN)
#define POSD1_ADR        							MemAlign(POSD1_AVAILABLE, 8)
#define POSD1_ADR_GAP_CHK							(POSD1_ADR-POSD1_AVAILABLE)
#define POSD1_LEN        							0 // not used

//	BT_POOL added(dreamer@lge.com 2009,01,12)
//	(It's the private memory of BT module(BT POOL))
#if 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define BT_POOL_AVAILABLE  							(POSD1_ADR + POSD1_LEN)
#define BT_POOL_ADR        							MemAlign(BT_POOL_AVAILABLE, 32)
#define BT_POOL_ADR_GAP_CHK							(BT_POOL_ADR-BT_POOL_AVAILABLE)
#define BT_POOL_LEN        							0 // not used
#else
#define BT_POOL_AVAILABLE  							(POSD1_ADR + POSD1_LEN)
#define BT_POOL_ADR        							MemAlign(BT_POOL_AVAILABLE, 32)
#define BT_POOL_ADR_GAP_CHK							(BT_POOL_ADR-BT_POOL_AVAILABLE)
#if 1 // temporariliy changed by streamerj, 090527
#define BT_POOL_LEN									0x800000 // 8MB
#else
#define BT_POOL_LEN									0x000000 // 8MB
#endif
#endif

#define MIU1_DUMMY_AVAILABLE  						(BT_POOL_ADR + BT_POOL_LEN)
#define MIU1_DUMMY_ADR		  						MemAlign(MIU1_DUMMY_AVAILABLE, 32)
#define MIU1_DUMMY_ADR_GAP_CHK						(MIU1_DUMMY_ADR-MIU1_DUMMY_AVAILABLE)
#define MIU1_DUMMY_LEN        						(MIU_DRAM_LEN-MIU1_DUMMY_ADR)

#if ((MIU1_DUMMY_ADR+MIU1_DUMMY_LEN)>MIU_DRAM_LEN)
#error your-mmap-over-dram-size
#endif

#endif
