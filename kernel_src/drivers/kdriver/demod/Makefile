#
# Makefile for MStar Demodulator device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/demod
EXTRA_CFLAGS      +=  -Idrivers/mstar/demod/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
# EXTRA_CFLAGS      +=  -DTITANIA
# EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_DEMOD)             += mdrv_demod.o
##dhjung LGE obj-$(CONFIG_MSTAR_DEMOD)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_DEMOD)		+= mdrv-demod.o
mdrv-demod-objs 				+= mdrv_demod.o
mdrv-demod-objs	                += $(CONFIG_MSTAR_CHIP_NAME)/mhal_demod.o
