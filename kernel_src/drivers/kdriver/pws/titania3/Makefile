#
# Makefile for MStar EMAC device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/demod/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/pws
EXTRA_CFLAGS      +=  -Idrivers/mstar/pws/$(CONFIG_MSTAR_CHIP_NAME)


# specific options
EXTRA_CFLAGS      +=  -DRED_LION




