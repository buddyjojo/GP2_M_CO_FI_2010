#
# Makefile for MStar IIC HAL drivers.
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

# files
##dhjung LGE obj-$(CONFIG_MSTAR_IIC)             += mhal_iic.o
