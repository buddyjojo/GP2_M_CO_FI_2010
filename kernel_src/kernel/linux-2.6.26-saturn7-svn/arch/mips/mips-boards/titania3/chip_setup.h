#ifndef _CHIP_SETUP_H_
#define _CHIP_SETUP_H_

#define	REG_MIPS_BASE				0xBF200000
#define MIPS_MIU0_BASE				(0xA0000000)
#define MIPS_MIU1_BASE				(0xC0000000)
#define RAWUART_MAX_SIZE            4800//204800
//dhjung LGE
#undef	CONFIG_UART0

/// Memory mapping type
typedef enum
{
    E_SYS_MMAP_LINUX_BASE,
    E_SYS_MMAP_VD_3DCOMB,
    E_SYS_MMAP_MAD_BASE,
    E_SYS_MMAP_MVD_FB,
    E_SYS_MMAP_MVD_BS,
    E_SYS_MMAP_EMAC,
    E_SYS_MMAP_VE,
    E_SYS_MMAP_SCALER_DNR_BUF,
    E_SYS_MMAP_TTX_BUF,
    E_SYS_MMAP_SC_BUF,
    E_SYS_MMAP_MPOOL,
    E_SYS_MMAP_LINUX_2ND_MEM,
    E_SYS_MMAP_SVD,
    E_SYS_MMAP_MVD_SW,
    E_SYS_MMAP_SVD_ALL,
    E_SYS_MMAP_POSD0_MEM,
    E_SYS_MMAP_POSD1_MEM,
    E_SYS_MMAP_TSP, // samuel, 20081107
    E_SYS_MMAP_AUDIO_CLIP_MEM, // samuel, 20081107
	E_SYS_MMAP_BIN_MEM,
	E_SYS_MMAP_JPD_MEM, // Samuel, 20090108
	E_SYS_MMAP_BT_POOL,	// dreamer@lge.com, 20090112
    E_SYS_MMAP_M4VE,
    E_SYS_MMAP_JPG_OSD,
    E_SYS_MMAP_PVR_DOWNLOAD_MEM, // StevenL, 20090311
    E_SYS_MMAP_PVR_UPLOAD_MEM, // StevenL, 20090311
    E_SYS_MMAP_PVR_THUMBNAIL_DECODE_MEM, // StevenL, 20090311
    E_SYS_MMAP_MIU1_BASE,
    E_SYS_MMAP_APVR_BASE,
    E_SYS_MMAP_MIU1_POOL,
    E_SYS_MMAP_OD_MSB_BUFFER,
	E_SYS_MMAP_OD_LSB_BUFFER,
    E_SYS_MMAP_BB,
    E_SYS_MMAP_DEBUG,
    E_SYS_MMAP_FPGA_POOL_BASE,
    E_SYS_MMAP_NUMBER,
} SYS_Memory_Mapping;

//dhjung LGE
typedef enum
{
	E_SYS_MMAP_256MB,
	E_SYS_MMAP_512MB
} SYS_Memory_Size;

extern void Chip_Flush_Memory(void);
extern void Chip_Read_Memory(void) ;
extern unsigned int Chip_Query_MIPS_CLK(void);
extern unsigned int Chip_Query_Rev(void);
extern int MDrv_SYS_GetMMAP(int type, unsigned int *addr, unsigned int *len);
extern unsigned int mpool_userspace_base ;
#endif
