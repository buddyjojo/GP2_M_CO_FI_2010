#
# Makefile for MStar H264 device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common
#EXTRA_CFLAGS     +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/h264
EXTRA_CFLAGS      +=  -Idrivers/mstar/h264/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
#EXTRA_CFLAGS      +=  -DTITANIA
#EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
ifeq ("$(CONFIG_MSTAR_CHIP_NAME)","titania3")
##obj-y             += mdrv_h264.o $(CONFIG_MSTAR_CHIP_NAME)/
##dhjung LGE obj-$(CONFIG_MSTAR_H264)             +=  mdrv_h264.o $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_H264)	+= mdrv-h264.o
mdrv-h264-objs				+= mdrv_h264.o

CONFIG_MSTAR_HVD_DRIVER_TYPE = UTOPIA_HVD_DRIVER
EXTRA_CFLAGS				+= -DUTOPIA_HVD_DRIVER
EXTRA_CFLAGS				+= -DREDLION_LINUX_KERNEL_ENVI
EXTRA_CFLAGS				+= -DCHIP_T3
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
# this is for normal case .ko
mdrv-h264-y					+= $(CONFIG_MSTAR_CHIP_NAME)/libHVD_normal.lib
else
# this is for flash,epk case
mdrv-h264-y					+= $(CONFIG_MSTAR_CHIP_NAME)/libHVD_flash.lib
endif

mdrv-h264-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_hvd.o
mdrv-h264-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_hvd_interrupt.o
# debug use
#EXTRA_CFLAGS				+= -Idrivers/mstar/h264/$(CONFIG_MSTAR_CHIP_NAME)/utopia/mxlib/drv/hvd
##EXTRA_CFLAGS				+= -Idrivers/mstar/h264/$(CONFIG_MSTAR_CHIP_NAME)/utopia/mxlib/hal/t2/svd
#EXTRA_CFLAGS				+= -Idrivers/mstar/h264/$(CONFIG_MSTAR_CHIP_NAME)/utopia/mxlib/hal/t3/vpu
#EXTRA_CFLAGS				+= -Idrivers/mstar/h264/$(CONFIG_MSTAR_CHIP_NAME)/utopia/mxlib/hal/t3/hvd
#mdrv-h264-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/utopia/mxlib/drv/hvd/drvHVD.o
##mdrv-h264-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/utopia/mxlib/hal/t2/svd/halSVD.o
#mdrv-h264-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/utopia/mxlib/hal/t3/vpu/halVPU.o
#mdrv-h264-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/utopia/mxlib/hal/t3/hvd/halHVD.o

#mdrv-h264-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_h264.o
#mdrv-h264-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_h264_interrupt.o
#mips2_fp_le-ar -cru libHVD_flash.lib drvHVD.o halHVD.o halVPU.o
#mips2_fp_le-ar -cru libHVD_normal.lib drvHVD.o halHVD.o halVPU.o
endif
