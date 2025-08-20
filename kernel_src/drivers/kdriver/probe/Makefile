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
EXTRA_CFLAGS      +=  -Idrivers/mstar/probe/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)


EXTRA_AFLAGS        = -Idrivers/mstar/probe

# specific options
EXTRA_CFLAGS      +=

# dhjung LGE
obj-$(CONFIG_MSTAR_PROBE)	+= mdrv-probe.o
mdrv-probe-objs			+= mdrv_probe.o

