#
# Makefile for the kernel IrDA char device drivers.
#

# dhjung LGE
ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif

# dhjung LGE
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS += -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS	+= -Idrivers/mstar/ir
EXTRA_CFLAGS	+= -Idrivers/mstar/ir/$(CONFIG_MSTAR_CHIP_NAME)
# EXTRA_CFLAGS += -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board
# specific options
#EXTRA_CFLAGS      +=  -Iinclude/asm-mips/titania
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

##dhjung LGE obj-$(CONFIG_MSTAR_IR) += mdrv_ir_io.o mdrv_ir.o

# dhjung LGE
obj-$(CONFIG_MSTAR_IR)	+= mdrv-ir.o
mdrv-ir-objs			+= mdrv_ir.o mdrv_ir_io.o
#export header files
EXPORT_H_FILES    += mdrv_ir_io.h
