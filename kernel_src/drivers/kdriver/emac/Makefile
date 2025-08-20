#
# Makefile for MStar EMAC device drivers.
#

ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# general options
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/emac
EXTRA_CFLAGS      +=  -Idrivers/mstar/emac/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DRED_LION

# files
#obj-$(CONFIG_MSTAR_EMAC)             += mdrv_emac.o
# dhjung LGE
obj-$(CONFIG_MSTAR_EMAC)	+= mdrv-emac.o
mdrv-emac-objs			+= mdrv_emac.o

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
mdrv-emac-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_emac.o
