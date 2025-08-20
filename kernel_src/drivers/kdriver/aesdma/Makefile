#
# Makefile for MStar AESDMA device drivers.
#
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/aesdma
EXTRA_CFLAGS      +=  -Idrivers/mstar/aesdma/$(CONFIG_MSTAR_CHIP_NAME)



# specific options
EXTRA_CFLAGS      +=  -DRED_LION
# files
obj-$(CONFIG_MSTAR_AESDMA)	+= mdrv-aesdma.o
mdrv-aesdma-objs            += mdrv_aesdma.o mdrv_aesdma_io.o
mdrv-aesdma-objs		    += $(CONFIG_MSTAR_CHIP_NAME)/mhal_aesdma.o
