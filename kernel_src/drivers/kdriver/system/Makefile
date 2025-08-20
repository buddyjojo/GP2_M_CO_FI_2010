#
# Makefile for MStar System device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/system
EXTRA_CFLAGS      +=  -Idrivers/mstar/system/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=

# files
##dhjung LGE obj-$(CONFIG_MSTAR_SYSTEM)		+= mdrv_system_io.o
##dhjung LGE obj-$(CONFIG_MSTAR_SYSTEM)		+= mdrv_system.o
#obj-$(CONFIG_MSTAR_SYSTEM)            		+= $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_SYSTEM)	+= mdrv-system.o
#mdrv-system-objs	+= mdrv_system.o mdrv_system_io.o mdrv_bandwidth.o
mdrv-system-objs	+= mdrv_system.o mdrv_system_io.o
mdrv-system-objs        += $(CONFIG_MSTAR_CHIP_NAME)/mhal_MIU.o

# export header files
EXPORT_H_FILES    += mdrv_system_io.h
EXPORT_H_FILES    += mdrv_system_st.h
