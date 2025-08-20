#
# Makefile for MStar JPEG device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

#DEBUG SYMBOL
#EXTRA_CFLAGS      +=  -gdwarf-2
#EXTRA_CFLAGS      +=  -g
#EXTRA_CFLAGS      +=  -O0

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/jpeg
EXTRA_CFLAGS      +=  -Idrivers/mstar/jpeg/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)

# dhjung LGE
#EXTRA_CFLAGS      +=  -Idrivers/mstar/system
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_JPEG)             += mdrv_jpeg_io.o mdrv_jpeg_decoder.o mdrv_jpeg_memory.o $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_JPEG)	+= mdrv-jpeg.o
mdrv-jpeg-objs			+= mdrv_jpeg_io.o
mdrv-jpeg-objs			+= mdrv_jpeg_decoder.o
mdrv-jpeg-objs			+= mdrv_jpeg_memory.o
mdrv-jpeg-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_jpd.o
