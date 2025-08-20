#
# Makefile for MStar NAND device drivers.
#
# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS				+=  -Idrivers/mstar/include
EXTRA_CFLAGS				+=  -Idrivers/mstar/nand
EXTRA_CFLAGS				+=  -Idrivers/mstar/nand/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS				+=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)

obj-$(CONFIG_MSTAR_NAND)	+= mdrv-nand.o
mdrv-nand-objs				+= mdrv_nand.o
mdrv-nand-objs				+= mdrv_nand_io.o
mdrv-nand-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/drvNAND.o
