#include "mdrv_types.h"
#include "mdrv_mpif_io_uboot.h"
#include "mdrv_mpif_st.h"
#include "mdrv_mpif.h"

extern void	printf(const char *fmt, ...);
#define printk printf

#define MPIF_WARNING(fmt, args...)       printk(KERN_WARNING "[MPIF][%06d]     " fmt, __LINE__, ## args)
#define MPIF_PRINT(fmt, args...)         printk("[MPIF][%06d]     " fmt, __LINE__, ## args)

#define OPT_IO_DGB 0
#if OPT_IO_DGB
#define MPIF_IO_DEBUG(fmt, args...)           printk("\033[47;34m[IO][%05d] " fmt "\033[0m", __LINE__, ## args)
#else
#define MPIF_IO_DEBUG(fmt, args...)
#endif

static void* memcpy(void* dest,const void* src, int count)
{
    char *tmp = (char *) dest, *s = (char *) src;

    while (count--)
        *tmp++ = *s++;

    return dest;
}


static U32 copy_from_user(void *dst, void *src, int size)
{
    memcpy(dst, src, size);
    return 0;
}

static U32 copy_to_user(void *dst, void *src, int size)
{
    memcpy(dst, src, size);
    return 0;
}

int ioctl(int inode, unsigned int cmd, void *parg)
{
    unsigned long arg = (unsigned long)parg;
    switch (cmd)
    {
        case IOCTL_MPIF_INIT:
        {
            MPIF_INIT_PARAM param;
            U32 err;
            err = copy_from_user(&param,(MPIF_INIT_PARAM*)arg,sizeof(MPIF_INIT_PARAM));
            if (err != 0)
                return -EFAULT;

            MPIF_IO_DEBUG("IOCTL_MPIF_INIT[CK:%u, TR:%u, WC:%u\n", param.u8clk, param.u8trc, param.u8wc);
            MDrv_MPIF_Init(param.u8clk, param.u8trc, param.u8wc);
            break;
        }
        case IOCTL_MPIF_INIT_SPIF:
        {
            U8 u8slaveid;
			//printf("IOCTL_MPIF_INIT_SPIF ~~ \n");
            if (copy_from_user(&u8slaveid,(MPIF_INIT_PARAM*)arg,sizeof(U8)))
                return -EFAULT;

            MPIF_IO_DEBUG("IOCTL_MPIF_INIT_SPIF[ID:%u]\n", u8slaveid);
            MDrv_MPIF_InitSPIF(u8slaveid);
            break;
        }
        case IOCTL_MPIF_1A:
        {
            MPIF_PARAM param;
            U8 u8data;
        
            if (copy_from_user(&param,(MPIF_PARAM*)arg,sizeof(MPIF_PARAM)))
                return -EFAULT;
        
            u8data = (U8)param.data;
            param.bRet = MDrv_MPIF_LC1A_RW(param.u8bWite, param.slaveid, param.addr, &u8data);
            param.data = (U32)u8data;
            MPIF_IO_DEBUG("IOCTL_MPIF_1A[W:%u, ID:%u A:0x%x D:0x%x]\n", 
                param.u8bWite, param.slaveid, param.addr, u8data);

            if (copy_to_user((MPIF_PARAM*)arg, (MPIF_PARAM*)&param, sizeof(MPIF_PARAM)))
                return -EFAULT;
        
            break;
        }
        case IOCTL_MPIF_2A:
        {
            MPIF_PARAM param;
            U16 u16data;
        
            if (copy_from_user(&param,(MPIF_PARAM*)arg,sizeof(MPIF_PARAM)))
                return -EFAULT;
        
            u16data = (U16)param.data;
            param.bRet = MDrv_MPIF_LC2A_RW(param.u8bWite, param.slaveid, param.addr, &u16data);
            param.data = (U16)u16data;
            MPIF_IO_DEBUG("IOCTL_MPIF_1A[W:%u, ID:%u A:0x%x D:0x%x]\n", 
                param.u8bWite, param.slaveid, param.addr, param.data);

            if (copy_to_user((MPIF_PARAM*)arg, (MPIF_PARAM*)&param, sizeof(MPIF_PARAM)))
                return -EFAULT;
        
            break;
        }
        case IOCTL_MPIF_SET_CMDDATA_WIDTH:
        {
            MPIF_BUS_PARAM param;
            BOOL ret;
            MPIF_IO_DEBUG("IOCTL_MPIF_SET_CMDDATA_WIDTH\n");
            if (copy_from_user(&param,(MPIF_BUS_PARAM*)arg,sizeof(MPIF_BUS_PARAM)))
                return -EFAULT;

            ret = MDrv_MPIF_SetCmdDataWidth(param.u8slaveid, param.u8cmdwidth, param.u8datawidth);

            if (copy_to_user((BOOL*)arg, &ret, sizeof(BOOL)))
                return -EFAULT;
            break;
        }
        default:
            MPIF_PRINT("mpif ioctl: unknown command, <%d>\n", cmd);
            return -ENOTTY;
    }

    return 0;

}

