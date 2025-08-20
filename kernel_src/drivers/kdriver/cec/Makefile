#
# Makefile for MStar CEC device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/cec
EXTRA_CFLAGS      +=  -Idrivers/mstar/cec/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
#EXTRA_CFLAGS      +=  -Iinclude/asm-mips/titania
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_CEC)             += mdrv_cec.o
##dhjung LGE obj-$(CONFIG_MSTAR_CEC)             += mdrv_cec_io.o
##dhjung LGE obj-$(CONFIG_MSTAR_CEC)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_CEC)	+= mdrv-cec.o
mdrv-cec-objs			+= mdrv_cec.o mdrv_cec_io.o
#mdrv-cec-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_cec.o

# export header files
EXPORT_H_FILES    += mdrv_cec_io.h
