#
# Makefile for MStar SAR HAL drivers.
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

# files
##dhjung LGE obj-$(CONFIG_MSTAR_SAR)             += mhal_sar.o
