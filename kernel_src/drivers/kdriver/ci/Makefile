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
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/ci
EXTRA_CFLAGS      +=  -Idrivers/mstar/ci/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
#EXTRA_CFLAGS      +=  -Iinclude/asm-mips/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -DRED_LION

# files
##dhjung LGE obj-$(CONFIG_MSTAR_CI)   += mdrv_ci.o
##dhjung LGE obj-$(CONFIG_MSTAR_CI)   += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_CI)	+= mdrv-ci.o
mdrv-ci-objs			+= mdrv_ci.o
mdrv-ci-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_ci_l2.o
mdrv-ci-objs			+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_ci_pcmcia.o
