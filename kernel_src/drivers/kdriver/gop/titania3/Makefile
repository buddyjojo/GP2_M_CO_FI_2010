#
# Makefile for MStar GOP device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/gop
EXTRA_CFLAGS      +=  -Idrivers/mstar/gop/$(CONFIG_MSTAR_CHIP_NAME)


# specific options
EXTRA_CFLAGS      +=  -DRED_LION

# files
##dhjung LGE obj-$(CONFIG_MSTAR_GOP)             += mhal_gop.o  mhal_gop_interrupt.o
