#
# Makefile for MStar Close Caption device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# drmyung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/cc
EXTRA_CFLAGS      +=  -Idrivers/mstar/cc/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=

# files
# drmyung LGE obj-$(CONFIG_MSTAR_CC)             += mdrv_cc.o $(CONFIG_MSTAR_CHIP_NAME)/

# drmyung LGE
obj-$(CONFIG_MSTAR_CC)	+= mdrv-cc.o
mdrv-cc-objs			+= mdrv_cc.o
mdrv-cc-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_cc.o
