#
# Makefile for MStar Teletext device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/ttx
EXTRA_CFLAGS      +=  -Idrivers/mstar/ttx/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=

# files
## obj-$(CONFIG_MSTAR_TTX)          += drv_scaler_io.o
## obj-$(CONFIG_MSTAR_TTX)          += hal_vd.o
## drmyung LGE obj-$(CONFIG_MSTAR_TTX)             += mhal_ttx.o
