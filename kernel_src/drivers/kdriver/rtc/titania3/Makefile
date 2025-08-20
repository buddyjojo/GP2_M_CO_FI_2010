#
# Makefile for MStar RTC HAL drivers.
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

# files
##dhjung LGE obj-$(CONFIG_MSTAR_RTC)             += mhal_rtc.o
