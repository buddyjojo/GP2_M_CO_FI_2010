#
# Makefile for MStar Teletext device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/ttx
EXTRA_CFLAGS      +=  -Idrivers/mstar/ttx/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=

# files
# drmyung LGE obj-$(CONFIG_MSTAR_TTX)             += mdrv_ttx.o mdrv_ttx_interrupt.o $(CONFIG_MSTAR_CHIP_NAME)/

# drmyung LGE
obj-$(CONFIG_MSTAR_TTX)	+= mdrv-ttx.o
mdrv-ttx-objs			+= mdrv_ttx.o mdrv_ttx_interrupt.o
mdrv-ttx-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_ttx.o
