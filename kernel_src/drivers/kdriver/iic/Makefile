#
# Makefile for MStar IIC device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/iic
EXTRA_CFLAGS      +=  -Idrivers/mstar/iic/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
#EXTRA_CFLAGS      +=  -Iinclude/asm-mips/titania
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_IIC)             += mdrv_iic.o
##dhjung LGE obj-$(CONFIG_MSTAR_IIC)             += mdrv_iic_io.o
##dhjung LGE obj-$(CONFIG_MSTAR_IIC)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_IIC)	+= mdrv-iic.o
mdrv-iic-objs			+= mdrv_iic.o mdrv_iic_io.o
mdrv-iic-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_iic.o

# export header files
EXPORT_H_FILES    += mdrv_iic_io.h
