#
# Makefile for MStar Video Decoder device drivers.
#

ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/str/$(CONFIG_MSTAR_CHIP_NAME)

EXTRA_AFLAGS        = -Idrivers/mstar/str

# specific options
EXTRA_CFLAGS      +=

# dhjung LGE
obj-$(CONFIG_MSTAR_STR)	+= mdrv-str.o
mdrv-str-objs			+= mdrv_str.o mdrv_str_sleep.o asm_common.o asm_cache.o asm_tlb.o



