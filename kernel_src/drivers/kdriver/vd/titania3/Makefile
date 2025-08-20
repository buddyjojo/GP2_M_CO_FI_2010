#
# Makefile for MStar Video Decoder device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/vd
EXTRA_CFLAGS      +=  -Idrivers/mstar/vd/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=

# files
## obj-$(CONFIG_MSTAR_VD)				+= drv_scaler_io.o
## obj-$(CONFIG_MSTAR_VD)				+= hal_vd.o
##dhjung LGE obj-$(CONFIG_MSTAR_VD)		+= hal_vd.o hal_vd_adc.o vd_dsp.o
