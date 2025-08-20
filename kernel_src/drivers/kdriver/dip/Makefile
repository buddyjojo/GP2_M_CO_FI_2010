#
# Makefile for MStar Video Decoder device drivers.
#

ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board
EXTRA_CFLAGS      +=  -Idrivers/mstar/dip/$(CONFIG_MSTAR_CHIP_NAME)

EXTRA_AFLAGS        = -Idrivers/mstar/dip

# specific options
EXTRA_CFLAGS      +=

# dhjung LGE
obj-$(CONFIG_MSTAR_DIP)	+= mdrv-dip.o
mdrv-dip-objs			+= mdrv_dip.o
mdrv-dip-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_dip.o

