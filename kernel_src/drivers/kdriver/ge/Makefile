#
# Makefile for MStar GE device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/ge
EXTRA_CFLAGS      +=  -Idrivers/mstar/ge/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DRED_LION

# files
##dhjung LGE obj-$(CONFIG_MSTAR_GE)             += mdrv_ge.o
##dhjung LGE obj-$(CONFIG_MSTAR_GE)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_GE)	+= mdrv-ge.o
mdrv-ge-objs			+= mdrv_ge.o
mdrv-ge-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_ge.o $(CONFIG_MSTAR_CHIP_NAME)/mhal_ge_fp.o
