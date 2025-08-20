#
# Makefile for MStar RLD device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/rld
EXTRA_CFLAGS      +=  -Idrivers/mstar/rld/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=

# files
##dhjung LGE obj-$(CONFIG_MSTAR_RLD)             += mdrv_rld.o
##dhjung LGE obj-$(CONFIG_MSTAR_RLD)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_RLD)	+= mdrv-rld.o
mdrv-rld-objs			+= mdrv_rld.o
mdrv-rld-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_rld.o

# export header files
#EXPORT_H_FILES    += mdrv_rld_io.h
#EXPORT_H_FILES    += mdrv_rld_st.h
