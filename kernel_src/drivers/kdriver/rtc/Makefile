#
# Makefile for MStar RTC device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/rtc
EXTRA_CFLAGS      +=  -Idrivers/mstar/rtc/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
#EXTRA_CFLAGS      +=  -Iinclude/asm-mips/titania
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_RTC)             += mdrv_rtc.o
##dhjung LGE obj-$(CONFIG_MSTAR_RTC)             += mdrv_rtc_io.o
##dhjung LGE obj-$(CONFIG_MSTAR_RTC)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_RTC)	+= mdrv-rtc.o
mdrv-rtc-objs			+= mdrv_rtc.o mdrv_rtc_io.o
mdrv-rtc-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_rtc.o

# export header files
EXPORT_H_FILES    += mdrv_rtc_io.h
