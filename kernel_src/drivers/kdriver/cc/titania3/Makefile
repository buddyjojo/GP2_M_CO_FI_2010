#
# Makefile for MStar Close Caption device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/cc
EXTRA_CFLAGS      +=  -Idrivers/mstar/cc/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=

# files
## obj-$(CONFIG_MSTAR_CC)             += drv_scaler_io.o
## obj-$(CONFIG_MSTAR_CC)             += hal_vd.o
## drmyung LGE obj-$(CONFIG_MSTAR_CC)             += mhal_cc.o
