#
# Makefile for MStar PWS device drivers.
#
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/demod/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/pws
EXTRA_CFLAGS      +=  -Idrivers/mstar/pws/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DRED_LION
# files
obj-$(CONFIG_MSTAR_PWS)	    += mdrv-pws.o
mdrv-pws-objs               += mdrv_pws.o mdrv_pws_io.o
#mdrv-pws-objs               += mdrv_pws.o
mdrv-pws-objs		        += $(CONFIG_MSTAR_CHIP_NAME)/mhal_pws.o
