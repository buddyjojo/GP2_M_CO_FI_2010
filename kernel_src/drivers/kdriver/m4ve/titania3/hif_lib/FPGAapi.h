#ifndef _FPGAAPI_H_
#define _FPGAAPI_H_

#define RET_OK 1
#define _UDMA_
//#define _HIF_

#ifdef _UDMA_
//#include "UDMAapi.h"
#define FPGA_Connect         UDMA_Connect
#define FPGA_RIURead16       UDMA_RIURead16
#define FPGA_RIUWrite16      UDMA_RIUWrite16
#define FPGA_MIURead         UDMAOP_DMAGet
#define FPGA_MIUWrite        UDMAOP_DMAPut
#define FPGA_MIUReadFile     UDMAOP_DMAGetFile
#define FPGA_MIUWriteFile    UDMAOP_DMAPutFile
#elif defined(_HIF_)
#include "HIFapi.h"
#define FPGA_Connect         HIF_Connect
#define FPGA_RIURead16       HIF_RIURead16
#define FPGA_RIUWrite16      HIF_RIUWrite16
#define FPGA_MIURead         HIF_MIURead
#define FPGA_MIUWrite        HIF_MIUWrite
#define FPGA_MIUReadFile     HIF_DMAGetFile
#define FPGA_MIUWriteFile    HIF_DMAPutFile
BOOL HIF_DMAGetFile_slow(char* filename, DWORD addr, int size, int endian, BYTE* pbuf);
#else
#error "you have to define _UDMA_ or _HIF_!!!"
#endif

#endif