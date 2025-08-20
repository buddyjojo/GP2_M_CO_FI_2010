#
# Makefile for MStar Scaler HAL drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/scaler
EXTRA_CFLAGS      +=  -Idrivers/mstar/scaler/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=

# files
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mhal_scaler.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mhal_lpll.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mhal_mod.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mhal_adc.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mhal_hdmi.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mhal_ddc2bi.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mhal_auto.o
