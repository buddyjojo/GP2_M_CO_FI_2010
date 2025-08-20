#
# Makefile for MStar Byte DMA device drivers.
#

ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/bdma
EXTRA_CFLAGS      +=  -Idrivers/mstar/bdma/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
# EXTRA_CFLAGS      +=  -DTITANIA
# EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_MSTAR_BDMA)		+= mdrv-bdma.o
mdrv-bdma-objs 				    += mdrv_bdma.o
mdrv-bdma-objs	                += $(CONFIG_MSTAR_CHIP_NAME)/mhal_bdma.o
