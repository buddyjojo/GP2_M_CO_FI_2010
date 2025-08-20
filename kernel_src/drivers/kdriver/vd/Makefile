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
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/vd
EXTRA_CFLAGS      +=  -Idrivers/mstar/vd/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=

# files
## obj-$(CONFIG_MSTAR_VD)				+= drv_scaler_io.o
# obj-$(CONFIG_MSTAR_VD) 				+= hal_vd.o
##dhjung LGE obj-$(CONFIG_MSTAR_VD)		+= mdrv_vd.o mdrv_vd_states_thread.o $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_VD)	+= mdrv-vd.o
mdrv-vd-objs			+= mdrv_vd.o mdrv_vd_states_thread.o
mdrv-vd-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/hal_vd.o
mdrv-vd-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/hal_vd_adc.o
mdrv-vd-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/vd_dsp.o
mdrv-vd-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/vd_dsp_for_NTSC.o

EXPORT_H_FILES    += mdrv_vdtype.h
