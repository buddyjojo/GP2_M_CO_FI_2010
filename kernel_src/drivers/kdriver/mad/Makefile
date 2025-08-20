#
# Makefile for MStar MAD device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/mad/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/gpio
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

EXTRA_CFLAGS      +=  -Idrivers/mstar/mad/dspCode_d
EXTRA_CFLAGS      +=  -Idrivers/mstar/mad/dspCode_s
EXTRA_CFLAGS      +=  -Idrivers/mstar/mad/dspCode_adv
ifeq ("$(CONFIG_MSTAR_CHIP_NAME)","titania2")
EXTRA_CFLAGS      += -DAUDIO_T2
endif
# specific options
EXTRA_CFLAGS      +=

# files
##dhjung LGE obj-$(CONFIG_MSTAR_MAD)             += $(BOARD_CHOICE)/
##dhjung LGE obj-$(CONFIG_MSTAR_MAD)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_MAD)		+= mdrv-mad.o
ifeq ("$(CONFIG_MSTAR_CHIP_NAME)","titania2")
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mdrv_mad_io.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mdrv_mad_common.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mdrv_mad_dvb.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mdrv_mad_process.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mdrv_mad_sif.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mdrv_semaphore.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mdrv_mad_dvb2.o
else
mdrv-mad-objs				+= mdrv_mad_io.o
mdrv-mad-objs				+= mdrv_mad_common.o
mdrv-mad-objs				+= mdrv_mad_dvb.o
mdrv-mad-objs				+= mdrv_mad_process.o
mdrv-mad-objs				+= mdrv_mad_sif.o
mdrv-mad-objs				+= mdrv_semaphore.o
mdrv-mad-objs				+= mdrv_mad_dvb2.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_mad_audsp.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_mad_reg.o
mdrv-mad-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_semaphore.o
endif
