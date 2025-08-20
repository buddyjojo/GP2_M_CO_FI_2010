#
# Makefile for the kernel MICOM char device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# EXTRA_CFLAGS += -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
# EXTRA_CFLAGS += -Idrivers/mstar/common
EXTRA_CFLAGS  = -Idrivers/mstar/include
EXTRA_CFLAGS += -Idrivers/mstar/micom
EXTRA_CFLAGS += -Idrivers/mstar/micom/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS += -Iinclude/asm-mips/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS += -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS += -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

##dhjung LGE obj-$(CONFIG_MSTAR_MICOM) += mdrv_micom_io.o mdrv_micom.o

# dhjung LGE
obj-$(CONFIG_MSTAR_MICOM)	+= mdrv-micom.o
mdrv-micom-objs				+= mdrv_micom.o mdrv_micom_io.o
mdrv-micom-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_micom.o
