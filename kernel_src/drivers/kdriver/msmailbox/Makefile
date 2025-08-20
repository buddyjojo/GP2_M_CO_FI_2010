#
# Makefile for MStar Mailbox device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/msmailbox
EXTRA_CFLAGS      +=  -Idrivers/mstar/msmailbox/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=

# files
##dhjung LGE obj-$(CONFIG_MSTAR_MSMAILBOX)	+=  mdrv_msmailbox_dev.o mdrv_msmailbox_interrupt.o mdrv_msmailbox_mbmgr.o $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_MSMAILBOX)	+= mdrv-msmailbox.o
mdrv-msmailbox-objs				+= mdrv_msmailbox_dev.o mdrv_msmailbox_interrupt.o mdrv_msmailbox_mbmgr.o
mdrv-msmailbox-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_msmailbox.o $(CONFIG_MSTAR_CHIP_NAME)/mhal_msmailbox_interrupt.o
