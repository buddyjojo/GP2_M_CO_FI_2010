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

#ifndef _MS_MMAP_128MB_128MB_H_
#define _MS_MMAP_128MB_128MB_H_

#define JPEG_HW_PATCH       1

// Memory alignment
#define MemAlignUnit                64UL
#define MemAlign(n, unit)           ((((n)+(unit)-1)/(unit))*(unit))

#define MIU_DRAM_LEN				(0x18000000)

//----------------------------------------------------------------------
// MIU 0
//----------------------------------------------------------------------
#define MIU0_MEM_BASE_ADR           0x00000000
#define MIU0_MEM_BASE_LEN           0x8000000

// Linux kernel space
#define LINUX_MEM_AVAILABLE			(MIU0_MEM_BASE_ADR)
#define LINUX_MEM_BASE_ADR 			(LINUX_MEM_AVAILABLE)
#define LINUX_MEM_GAP_CHK  			(LINUX_MEM_BASE_ADR-LINUX_MEM_AVAILABLE)
#define LINUX_MEM_LEN				(0xBC0000)	// 12MB - 256K

// for MVD and TSP binary dhjung LGE
#define BIN_MEM_AVAILABLE      		(LINUX_MEM_BASE_ADR + LINUX_MEM_LEN)
#define BIN_MEM_ADR                 MemAlign(BIN_MEM_AVAILABLE, 4096)
#define BIN_MEM_GAP_CHK        		(BIN_MEM_ADR-BIN_MEM_AVAILABLE)
#define BIN_MEM_LEN    				0x40000     // 256K

// must start at nnn0000 - 512k alignment   --[Dean] be careful allocate this big alignment area
#define MAD_BASE_AVAILABLE         	(BIN_MEM_ADR+BIN_MEM_LEN)
#define MAD_BASE_BUFFER_ADR        	MemAlign(MAD_BASE_AVAILABLE, 0x80000)
#define MAD_BASE_BUFFER_ADR_GAP_CHK	(MAD_BASE_BUFFER_ADR-MAD_BASE_AVAILABLE)
#define MAD_BASE_BUFFER_LEN        	0x280000	//2.5M // 3MB -> 2.5MB, samuel, 20081105


// For Maximum is 1920x1088x3x2 about 12MB in HDMI and YPbPr
// Let it overwrite EVENTDB and MHEG5 buffer(These buffers is DTV only)
#define SCALER_DNR_AVAILABLE  		(MAD_BASE_BUFFER_ADR+MAD_BASE_BUFFER_LEN)
#define SCALER_DNR_BUF_ADR	  		MemAlign(SCALER_DNR_AVAILABLE, 16)
#define SCALER_DNR_GAP_CHK	  		(SCALER_DNR_BUF_ADR-SCALER_DNR_AVAILABLE)
#define SCALER_DNR_BUF_LEN	  		0x1000000-80 //16MB-80byte  //0xC00000  //12582912	// 12MB

#define SCALER_DNR_W_AVAILABLE		(SCALER_DNR_BUF_ADR + SCALER_DNR_BUF_LEN)
#define SCALER_DNR_W_BARRIER_ADR	(SCALER_DNR_W_AVAILABLE)
#define SCALER_DNR_W_GAP_CHK		(SCALER_DNR_W_BARRIER_ADR-SCALER_DNR_W_AVAILABLE)
#define SCALER_DNR_W_BARRIER_LEN	80//16		// DNR submit 2 64-bit data before compare limit
#define SCALER_DNR_BUF_LEN_EXT		((((896UL-736UL)*3+0x0F) & ~0x0F) * 581UL *2) // the output size of VD will be 848 * 581

//[20090920 Shawn] MM I/O
#define SC_BUF_AVAILABLE           (SCALER_DNR_W_BARRIER_ADR + SCALER_DNR_W_BARRIER_LEN)
#define SC_BUF_ADR                 MemAlign(SC_BUF_AVAILABLE, 4096)
#define SC_ADR_GAP_CHK             (SC_BUF_ADR-SC_BUF_AVAILABLE)
#define SC_BUF_LEN                  0x2000 // 1024*12bit/8 = 1536 Bytes,
                                           // 1536*3(entry)= 4608 = 0x1200 (Bytes)
                                           // allocate 8k for SC

//======================================================================
// Can not add any buffer here (in between VE_FRAMEBUFFER_ADR and SCALER_DNR_BUF_ADR)
// Because USB/OAD download use the buffer from VE_FRAMEBUFFER_ADR to the end of SCALER_DNR_BUF_ADR
//======================================================================

//#define EMAC_BUF_AVAILABLE			(MAD_BASE_BUFFER_ADR+MAD_BASE_BUFFER_LEN)
#define EMAC_BUF_AVAILABLE			(SC_BUF_ADR+SC_BUF_LEN)
#define EMAC_BUF_ADR				MemAlign(EMAC_BUF_AVAILABLE, 8)
#define EMAC_BUF_GAP_CHK			(EMAC_BUF_ADR-EMAC_BUF_AVAILABLE)
#define EMAC_BUF_LEN				0x100000 // 1MB

#define SVD_CPU_AVAILABLE  			(EMAC_BUF_ADR + EMAC_BUF_LEN) //(MPOOL_ADR + MPOOL_LEN) // (0x8000000)
#define SVD_CPU_ADR        			MemAlign(SVD_CPU_AVAILABLE, 2048)
#define SVD_CPU_ADR_GAP_CHK			(SVD_CPU_ADR-SVD_CPU_AVAILABLE)
#define SVD_CPU_LEN        			0x100000    //1024KB

#define MVD_SW_AVAILABLE     		(SVD_CPU_ADR + SVD_CPU_LEN)
#define MVD_SW_ADR           		MemAlign(MVD_SW_AVAILABLE, 8)
#define MVD_SW_ADR_GAP_CHK   		(MVD_SW_ADR-MVD_SW_AVAILABLE)
#define MVD_SW_LEN           		0xB00000//0x900000//0xB00000	//11MB

// MVD share memory  is in the first 64K of MVD_SW_ADR
// It can't co-buffer with VE+TTX
#define MVD_SW_SHARE_MEM_ADR        MVD_SW_ADR
#define MVD_SW_SHARE_MEM_LEN        0x10000

// TSP buffer, TSP extract from mpool, samuel, 20081105
#define TSP_BUF_AVAILABLE			(MVD_SW_ADR + MVD_SW_LEN)
#define TSP_BUF_ADR 			    MemAlign(TSP_BUF_AVAILABLE, 32)
#define TSP_BUF_GAP_CHK  			(TSP_BUF_ADR-TSP_BUF_AVAILABLE)
#define TSP_BUF_LEN      			(0x800000) //8MB

#define VE_FRAMEBUFFER_AVAILABLE  	(MVD_SW_SHARE_MEM_ADR + MVD_SW_SHARE_MEM_LEN) // (SVD_CPU_ADR + SVD_CPU_LEN)
#define VE_FRAMEBUFFER_ADR		  	MemAlign(VE_FRAMEBUFFER_AVAILABLE, 8)
#define VE_FRAMEBUFFER_ADR_GAP_CHK	(VE_FRAMEBUFFER_ADR-VE_FRAMEBUFFER_AVAILABLE)
#define VE_FRAMEBUFFER_LEN			(0x32A000UL + 0x2800) 	// 720*576*2*2 + 10KByte(ttx insertion)

#define TTX_BUF_AVAILABLE			(VE_FRAMEBUFFER_ADR + VE_FRAMEBUFFER_LEN)
#define TTX_BUF_ADR					MemAlign(TTX_BUF_AVAILABLE, 4096)
#define TTX_BUF_GAP_CHK				(TTX_BUF_ADR-TTX_BUF_AVAILABLE)
#define TTX_BUF_LEN					0x40000  // 200KB is enough, but MVD co-buf here for callback data, so need 256KB, samuel, 20081105

#define MPOOL_AVAILABLE				(TSP_BUF_ADR + TSP_BUF_LEN)
#define MPOOL_ADR					MemAlign(MPOOL_AVAILABLE, 4096)
#define MPOOL_GAP_CHK				(MPOOL_ADR-MPOOL_AVAILABLE)
#define MPOOL_LEN				    (28*0x100000)

#define MAILBOX_AVAILABLE           (MPOOL_ADR + MPOOL_LEN) //  (AUDIO_CLIP_AVAILABLE + AUDIO_CLIP_LEN)
#define MAILBOX_ADR           	    MemAlign(MAILBOX_AVAILABLE, 8)
#define MAILBOX_GAP_CHK   	        (MAILBOX_ADR-MAILBOX_AVAILABLE)
#define MAILBOX_LEN           	    0xA000 // 32 KB

#define DIP_AVAILABLE               (MAILBOX_ADR + MAILBOX_LEN)
#define DIP_ADR                     MemAlign(DIP_AVAILABLE, 8)
#define DIP_ADR_GAP_CHK             (DIP_ADR-DIP_AVAILABLE)
#define DIP_LEN                     0x1000 // 4k bytes

#define PVR_DOWNLOAD_BUFFER_AVAILABLE  (DIP_ADR + DIP_LEN)
#define PVR_DOWNLOAD_BUFFER_ADR        MemAlign(PVR_DOWNLOAD_BUFFER_AVAILABLE, 4096)
#define PVR_DOWNLOAD_BUFFER_GAP_CHK    (PVR_DOWNLOAD_BUFFER_ADR-PVR_DOWNLOAD_BUFFER_AVAILABLE)
#define PVR_DOWNLOAD_BUFFER_LEN        0x480000 // 384K*12 = 4608K = 4.5MB

#define PVR_UPLOAD_BUFFER_AVAILABLE   (PVR_DOWNLOAD_BUFFER_ADR + PVR_DOWNLOAD_BUFFER_LEN)
#define PVR_UPLOAD_BUFFER_ADR         MemAlign(PVR_UPLOAD_BUFFER_AVAILABLE, 4096)
#define PVR_UPLOAD_BUFFER_GAP_CHK     (PVR_UPLOAD_BUFFER_ADR-PVR_UPLOAD_BUFFER_AVAILABLE)
#define PVR_UPLOAD_BUFFER_LEN         0x120000 // 384K*3  = 1152K

// dhjung LGE
#define LINUX_2ND_MEM_AVAILABLE		(PVR_UPLOAD_BUFFER_ADR + PVR_UPLOAD_BUFFER_LEN)
#define LINUX_2ND_MEM_ADDR			MemAlign(LINUX_2ND_MEM_AVAILABLE, 32)
#define LINUX_2ND_MEM_LEN			(MIU0_MEM_BASE_LEN - LINUX_2ND_MEM_ADDR)

//----------------------------------------------------------------------
// MIU 1
//----------------------------------------------------------------------
#define MIU1_MEM_BASE_ADR           0x10000000
#define MIU1_MEM_BASE_LEN           0x8000000

// VD_3DCOMB (co-buf with MVD_FRAMEBUFFER ), move to miu1, samuel, 20081105
#define VD_3DCOMB_AVAILABLE			(MIU1_MEM_BASE_ADR)
#define VD_3DCOMB_BASE_ADR 			MemAlign(VD_3DCOMB_AVAILABLE, 16)
#define VD_3DCOMB_GAP_CHK  			(VD_3DCOMB_BASE_ADR-VD_3DCOMB_AVAILABLE)
#define VD_3DCOMB_LEN      			(0x700000) //(0x400000) //4MB

// need 512 byte alignment SD
#define MVD_FRAMEBUFFER_AVAILABLE   (MIU1_MEM_BASE_ADR)
#define MVD_FRAMEBUFFER_ADR         MemAlign(MVD_FRAMEBUFFER_AVAILABLE, 2048)
#define MVD_FRAMEBUFFER_ADR_GAP_CHK (MVD_FRAMEBUFFER_ADR-MVD_FRAMEBUFFER_AVAILABLE)
#define MVD_FRAMEBUFFER_LEN			0x1B00000	//27MB: Dean

// need 8 byte alignment
#define MVD_BITSTREAM_AVAILABLE		(MVD_FRAMEBUFFER_ADR + MVD_FRAMEBUFFER_LEN)
#define MVD_BITSTREAM_ADR			MemAlign(MVD_BITSTREAM_AVAILABLE, 8)
#define MVD_BITSTREAM_ADR_GAP_CHK	(MVD_BITSTREAM_ADR-MVD_BITSTREAM_AVAILABLE)
#define MVD_BITSTREAM_LEN        	0x400000	//6MB->4MB     20081028 samuel

#define JPD_OUTPUT_AVAILABLE     	(MVD_BITSTREAM_ADR + MVD_BITSTREAM_LEN) //(MPOOL_ADR + MPOOL_LEN)
#define JPD_OUTPUT_ADR           	MemAlign(JPD_OUTPUT_AVAILABLE, 8)
#define JPD_OUTPUT_ADR_GAP_CHK   	(JPD_OUTPUT_ADR-JPD_OUTPUT_AVAILABLE)
#if JPEG_HW_PATCH
#define JPD_OUTPUT_LEN           	0xC00000 //  for Jpeg HW patch , 2M + 10M (patch)
#else
#define JPD_OUTPUT_LEN           	0x200000 // not used -> 0x200000    20081028 samue
#endif

//dhjung LGE
#define PVR_THUMBNAIL_DECODE_BUFFER_AVAILABLE   (JPD_OUTPUT_ADR + JPD_OUTPUT_LEN)
#define PVR_THUMBNAIL_DECODE_BUFFER_ADR         MemAlign(PVR_THUMBNAIL_DECODE_BUFFER_AVAILABLE, 8)
#define PVR_THUMBNAIL_DECODE_BUFFER_GAP_CHK     (PVR_THUMBNAIL_DECODE_BUFFER_ADR-PVR_THUMBNAIL_DECODE_BUFFER_AVAILABLE)
#define PVR_THUMBNAIL_DECODE_BUFFER_LEN         0x900000 // 9MB*/

// Samuel, move to MIU1 to save space in MIU0
#define AUDIO_CLIP_AVAILABLE        (PVR_THUMBNAIL_DECODE_BUFFER_ADR + PVR_THUMBNAIL_DECODE_BUFFER_LEN)
#define AUDIO_CLIP_ADR        	    MemAlign(AUDIO_CLIP_AVAILABLE, 8)
#define AUDIO_CLIP_ADR_GAP_CHK 	    (AUDIO_CLIP_ADR-AUDIO_CLIP_AVAILABLE)
#define AUDIO_CLIP_LEN           	0x400000

#define M4VE_BUF_AVAILABLE          (AUDIO_CLIP_ADR + AUDIO_CLIP_LEN)
#define M4VE_BUF_ADR                MemAlign(M4VE_BUF_AVAILABLE, 8)
#define M4VE_BUF_ADR_GAP_CHK        (M4VE_BUF_ADR-M4VE_BUF_AVAILABLE)
#define M4VE_BUF_LEN                0x280000    //2.5MB

// cobuffer with many buffers
#define JPG_OSD_AVAILABLE           (JPD_OUTPUT_ADR + JPD_OUTPUT_LEN) //(PVR_THUMBNAIL_DECODE_BUFFER_ADR + PVR_THUMBNAIL_DECODE_BUFFER_LEN)
#define JPG_OSD_ADR                 MemAlign(JPG_OSD_AVAILABLE, 8)
#define JPG_OSD_ADR_GAP_CHK        (JPG_OSD_ADR-JPG_OSD_AVAILABLE)
#if JPEG_HW_PATCH
#define JPG_OSD_LEN                0xF00000  // 13MB+2M(patch)
#else
#define JPG_OSD_LEN                0xD00000  // 13MB
#endif

#define POSD0_AVAILABLE            (M4VE_BUF_ADR + M4VE_BUF_LEN)
#define POSD0_ADR           	    MemAlign(POSD0_AVAILABLE, 8)
#define POSD0_ADR_GAP_CHK   	    (POSD0_ADR-POSD0_AVAILABLE)
#define POSD0_LEN                   0x410000

#define POSD1_AVAILABLE             (POSD0_ADR + POSD0_LEN)
#define POSD1_ADR           	    MemAlign(POSD1_AVAILABLE, 8)
#define POSD1_ADR_GAP_CHK   	    (POSD1_ADR-POSD1_AVAILABLE)
#define POSD1_LEN           	    0 // not used

#define MIU1_POOL_AVAILABLE         (POSD1_ADR + POSD1_LEN)
#define MIU1_POOL_ADR           	MemAlign(MIU1_POOL_AVAILABLE, 8)
#define MIU1_POOL_ADR_GAP_CHK   	(MIU1_POOL_ADR-MIU1_POOL_AVAILABLE)
#define MIU1_POOL_LEN           	(20*0x100000)

#define BB_AVAILABLE                (MIU1_POOL_ADR+MIU1_POOL_LEN)
#define BB_ADR           	        MemAlign(BB_AVAILABLE, 8)
#define BB_ADR_GAP_CHK   	        (BB_ADR-BB_AVAILABLE)
#define BB_LEN           	        (0x20000)

#define DEBUG_AVAILABLE                (BB_ADR+BB_LEN)
#define DEBUG_ADR                      MemAlign(DEBUG_AVAILABLE, 8)
#define DEBUG_ADR_GAP_CHK              (DEBUG_ADR-DEBUG_AVAILABLE)
#define DEBUG_LEN                      (0x20000)

// Samuel, move from MIU0 to MIU1
//     BT_POOL added(dreamer@lge.com 2009,01,12)
//     (It's the private memory of BT module(BT POOL))
#define BT_POOL_AVAILABLE           (DEBUG_ADR+DEBUG_LEN)
#define BT_POOL_ADR             	MemAlign(BT_POOL_AVAILABLE, 32)
#define BT_POOL_ADR_GAP_CHK     	(BT_POOL_ADR-BT_POOL_AVAILABLE)
#define BT_POOL_LEN             	0x400000

#define APVR_BUF_AVAILABLE         (BT_POOL_ADR+BT_POOL_LEN)
#define APVR_BUF_ADR        	    MemAlign(APVR_BUF_AVAILABLE, 8)
#define APVR_BUF_ADR_GAP_CHK 	    (APVR_BUF_ADR-APVR_BUF_AVAILABLE)
#define APVR_BUF_LEN             	0x800000

#define MIU1_DUMMY_AVAILABLE		(APVR_BUF_ADR+APVR_BUF_LEN)
#define MIU1_DUMMY_ADR				MemAlign(MIU1_DUMMY_AVAILABLE, 32)
#define MIU1_DUMMY_ADR_GAP_CHK		(MIU1_DUMMY_ADR-MIU1_DUMMY_AVAILABLE)
#define MIU1_DUMMY_LEN         	    (MIU_DRAM_LEN-MIU1_DUMMY_ADR)

#if ((MIU1_DUMMY_ADR+MIU1_DUMMY_LEN)>MIU_DRAM_LEN)
#error your-mmap-over-dram-size
#endif


#endif
