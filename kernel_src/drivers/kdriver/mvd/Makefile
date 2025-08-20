#
# Makefile for MStar MVD device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/mvd
EXTRA_CFLAGS      +=  -Idrivers/mstar/mvd/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# dhjung LGE
#EXTRA_CFLAGS      +=  -Idrivers/mstar/system
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX
EXTRA_CFLAGS      +=  -DCHIP_T3
EXTRA_CFLAGS      +=  -DREDLION_LINUX_KERNEL_ENVI

ifeq ($(MAKE_TYPE), MODULE_STANDALONE) # this is for normal case .ko
mdrv-mvd-y                    += $(CONFIG_MSTAR_CHIP_NAME)/../../h264/titania3/libHVD_normal.lib
else # this is for flash, epk case
endif


# files
##dhjung LGE obj-$(CONFIG_MSTAR_MVD)             += mdrv_mvd.o $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_MVD)	+= mdrv-mvd.o
mdrv-mvd-objs			+= mdrv_mvd.o
mdrv-mvd-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_mvd.o
mdrv-mvd-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_mvd_interrupt.o
mdrv-mvd-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/drvMVD.o
mdrv-mvd-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/halMVD.o

