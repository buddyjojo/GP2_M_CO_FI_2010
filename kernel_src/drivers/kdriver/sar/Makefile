#
# Makefile for MStar SAR device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/sar
EXTRA_CFLAGS      +=  -Idrivers/mstar/sar/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
#EXTRA_CFLAGS      +=  -Iinclude/asm-mips/titania
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_SAR)             += mdrv_sar.o
##dhjung LGE obj-$(CONFIG_MSTAR_SAR)             += mdrv_sar_io.o
##dhjung LGE obj-$(CONFIG_MSTAR_SAR)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_SAR)	+= mdrv-sar.o
mdrv-sar-objs			+= mdrv_sar.o mdrv_sar_io.o
mdrv-sar-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_sar.o

# export header files
EXPORT_H_FILES    += mdrv_sar_io.h
