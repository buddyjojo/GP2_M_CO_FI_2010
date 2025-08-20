#ifndef _DRV_MVOP_H_
#define _DRV_MVOP_H_
#include <linux/module.h>

#include <linux/fs.h>    // for MKDEV()

#include <linux/cdev.h>
#include "mdrv_mvd.h"
#include "mdrv_mvop_io.h"
#include "mdrv_mvop_st.h"




struct MVOP_Dev {
	struct cdev cdev;	  /* Char device structure		*/
};


S32 MDrv_MVOP_Ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg);
S32 MDrv_MVOP_Open(struct inode *inode, struct file *filp);
S32 MDrv_MVOP_Release(struct inode *inode, struct file *filp);



#endif     
