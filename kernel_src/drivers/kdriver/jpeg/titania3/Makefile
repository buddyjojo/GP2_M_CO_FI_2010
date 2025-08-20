#
# Makefile for MStar JPEG device drivers.
#

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

# specific options
EXTRA_CFLAGS      +=

# files
##dhjung LGE obj-$(CONFIG_MSTAR_JPEG)             += mhal_jpd.o
