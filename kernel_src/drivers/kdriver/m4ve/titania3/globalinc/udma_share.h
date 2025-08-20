#ifndef _UDMA_SHARE_H_
#define _UDMA_SHARE_H_

//#define _FPGA_
//#define _MSVC6_ //open when using VC6.0 platform
#include "M4VE_chip.h"
#ifdef _AEON_PLATFORM_
#ifdef _M4VE_T3_
#else
#include "MsTypes.h"
#endif
#endif

#ifdef _MIPS_PLATFORM_
#ifdef _M4VE_T3_
#include "mdrv_types.h"
#else
#include <sys/bsdtypes.h>
#endif
#endif //_MIPS_PLATFORM_
#include "Mscommon.h"

#if defined(_M4VE_BIG2_)&&defined(_MIPS_PLATFORM_)
#include "utility.h"
#define addr_log2phy(log_addr) MApi_UTL_HwMemAccessOffset((void *)log_addr)
//#define addr_phy2log(phy_addr) ((phy_addr)+MEM_LOGMAPPHY) //MApi_UTL_PhyMemRestore((CYG_ADDRESS)phy_addr, INTF_M4VE_MC_W)
#define addr_phy2log(phy_addr) (MApi_UTL_PhyMemRestore((CYG_ADDRESS)phy_addr, INTF_M4VE_MC_W) + CYGARC_KSEG_UNCACHED_BASE )
//#define addr_phy2log(phy_addr) MApi_UTL_PhyMemRestore((CYG_ADDRESS)phy_addr, INTF_M4VE_MC_W)

#else
#define addr_log2phy(log_addr) ((log_addr)-MEM_LOGMAPPHY)
#define addr_phy2log(phy_addr) ((phy_addr)+MEM_LOGMAPPHY)
#endif

/* Linear memory area descriptor */
typedef struct MEMMAP
{
    //U32 *miuPointer;
    U8  *miuPointer;
    U32 miuAddress;
    U32 size;
} MEMMAP_t;

typedef void (*proto_write)(U32 u32Address, U16 val);
typedef void (*proto_read) (U32 u32Address, U16 *val);

void MMAPInit(U32 buf_start, U32 buf_size);
long  MMAPMalloc(U32 size, MEMMAP_t * info);
int reg_scan(unsigned short *reg_mask, U32 num_reg, proto_write write_func, proto_read read_func);
#endif
