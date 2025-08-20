#
# Makefile for MStar CI device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/ci
EXTRA_CFLAGS      +=  -Idrivers/mstar/ci/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DRED_LION

# files
#obj-y             += mhal_ci_l2.o
#obj-y             += mhal_ci_pcmcia.o
##dhjung LGE obj-$(CONFIG_MSTAR_CI)		+= mhal_ci_l2.o
##dhjung LGE obj-$(CONFIG_MSTAR_CI)		+= mhal_ci_pcmcia.o
