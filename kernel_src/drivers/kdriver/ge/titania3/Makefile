#
# Makefile for MStar GE device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/ge

# specific options
EXTRA_CFLAGS      +=  -DRED_LION

# files
##dhjung LGE obj-$(CONFIG_MSTAR_GE)             += mhal_ge.o
##dhjung LGE obj-$(CONFIG_MSTAR_GE)             += mhal_ge_fp.o
