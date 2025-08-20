#
# Makefile for MStar VE device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/ve
EXTRA_CFLAGS      +=  -Idrivers/mstar/ve/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=

# files
#obj-y             += mhal_ve.o
## obj-$(CONFIG_MSTAR_VE)				+= mhal_ve.o
##dhjung LGE obj-$(CONFIG_MSTAR_VE)		+= mhal_ve.o

#export header files
EXPORT_H_FILES    +=

