#
# Makefile for MStar Scaler device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS	+=  -Idrivers/mstar/scaler
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS	+=  -Idrivers/mstar/include
EXTRA_CFLAGS	+=  -Idrivers/mstar/8051lite
EXTRA_CFLAGS      +=  -Idrivers/mstar/gpio
EXTRA_CFLAGS	+=  -Idrivers/mstar/scaler/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=

# files
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mdrv_scaler_io.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mdrv_scaler.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mdrv_adc.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mdrv_hdmi.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mdrv_mace.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mdrv_scaler_pcmode.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += mdrv_auto.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += QualityMode.o
##dhjung LGE obj-$(CONFIG_MSTAR_SCALER)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_SCALER)	+= mdrv-scaler.o
mdrv-scaler-objs			+= mdrv_scaler.o mdrv_scaler_io.o mdrv_adc.o mdrv_hdmi.o mdrv_bandwidth.o
mdrv-scaler-objs			+= mdrv_mace.o mdrv_scaler_pcmode.o mdrv_auto.o
mdrv-scaler-objs			+= mdrv_qmap.o
mdrv-scaler-objs			+= mdrv_tcon.o
mdrv-scaler-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_scaler.o
mdrv-scaler-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_lpll.o
mdrv-scaler-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_mod.o
mdrv-scaler-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_adc.o
mdrv-scaler-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_hdmi.o
mdrv-scaler-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_auto.o
mdrv-scaler-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_tcon.o
mdrv-scaler-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_od.o

# export header files
#EXPORT_H_FILES    += mdrv_scaler_io.h
#EXPORT_H_FILES    += mdrv_scaler_st.h
#EXPORT_H_FILES    += mdrv_mod_st.h
#EXPORT_H_FILES    += mdrv_scaler_pcmode.h
