
#ifndef _MDRV_BDMA_IO_H_
#define _MDRV_BDMA_IO_H_

/* Use 'B' as magic number */
#define BDMA_IOC_MAGIC              'B'

#define BDMA_IOC_INIT               _IO(BDMA_IOC_MAGIC, 0)
#define BDMA_IOC_RESET              _IO(BDMA_IOC_MAGIC, 1)
#define BDMA_IOC_COPY               _IOW(BDMA_IOC_MAGIC, 2, BDMA_PARA_t)
#define BDMA_IOC_COPY_MEM2MEM       _IOW(BDMA_IOC_MAGIC, 3, BDMA_PARA_t)
#define BDMA_IOC_COPY_FLASH2DRAM    _IOW(BDMA_IOC_MAGIC, 4, BDMA_PARA_t)
#define BDMA_IOC_SEARCH             _IOWR(BDMA_IOC_MAGIC, 5, BDMA_PARA_t)
#define BDMA_IOC_SEARCH_DRAM        _IOWR(BDMA_IOC_MAGIC, 6, BDMA_PARA_t)


#define BDMA_IOC_MAXNR              7

#endif

