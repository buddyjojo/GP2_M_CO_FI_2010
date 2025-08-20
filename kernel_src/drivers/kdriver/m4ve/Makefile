#
# Makefile for MStar TSP device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/m4ve
EXTRA_CFLAGS      +=  -Idrivers/mstar/m4ve/$(CONFIG_MSTAR_CHIP_NAME)/Aeon
EXTRA_CFLAGS      +=  -Idrivers/mstar/m4ve/$(CONFIG_MSTAR_CHIP_NAME)/globalinc
EXTRA_CFLAGS      +=  -Idrivers/mstar/m4ve/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_TSP)             += mdrv_m4ve.o
##dhjung LGE obj-$(CONFIG_MSTAR_TSP)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_M4VE)		+= mdrv-m4ve.o
mdrv-m4ve-objs 				+= mdrv_m4ve.o
mdrv-m4ve-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/Aeon/CheckSum.o
mdrv-m4ve-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/Aeon/cvbr_rc.o
mdrv-m4ve-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/Aeon/drvM4VE.o
mdrv-m4ve-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/Aeon/plugin_single.o
mdrv-m4ve-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/Aeon/putHeader.o
mdrv-m4ve-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/Aeon/udma_share.o
mdrv-m4ve-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/Aeon/yuvgen.o
#mdrv-tsp-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_tsp.o
