#
# Makefile for MStar H264 device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/h264
EXTRA_CFLAGS      +=  -Idrivers/mstar/system
EXTRA_CFLAGS      +=  -Idrivers/mstar/h264/$(CONFIG_MSTAR_CHIP_NAME)
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
#EXTRA_CFLAGS      +=  -DTITANIA
#EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
#obj-y             += mhal_h264.o mhal_h264_interrupt.o
## obj-$(CONFIG_MSTAR_H264) 				+= mhal_h264.o
##dhjung LGE obj-$(CONFIG_MSTAR_H264)		+= mhal_h264.o