#ifndef _MDRV_M4VE_H_
#define _MDRV_M4VE_H_

#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
#include "mdrv_types.h"
#include <linux/module.h>

#include <linux/fs.h>    // for MKDEV()

#include <linux/cdev.h>
#elif defined(_M4VE_BIG2_)&&defined(_MIPS_PLATFORM_)
#include <sys/bsdtypes.h>
#include "pthread_map.h"
#else //defined(_WIN32)
#include <semaphore.h>
#include "pthread.h"
#endif

#include "mdrv_m4ve_io.h"
#include "mdrv_m4ve_st.h"

#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
struct M4VE_Dev {
	struct cdev cdev;	  /* Char device structure		*/
};

//S32 MDrv_M4VE_Ioctl(struct inode *inode, struct file *filp,
//                 unsigned int cmd, unsigned long arg);
#else
//S32 MDrv_M4VE_Ioctl(int m4ve_fd, unsigned int cmd, unsigned long arg);
#endif
#endif

