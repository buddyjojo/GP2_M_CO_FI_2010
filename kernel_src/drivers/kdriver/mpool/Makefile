#
# Makefile for MStar Malloc device drivers.
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
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/mpool
EXTRA_CFLAGS      +=  -Idrivers/mstar/mpool/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  

# files
##dhjung LGE obj-$(CONFIG_MSTAR_MPOOL)             += mdrv_mpool.o

# dhjung LGE
obj-$(CONFIG_MSTAR_MPOOL)	+= mdrv-mpool.o
mdrv-mpool-objs				+= mdrv_mpool.o

# export header files
#EXPORT_H_FILES    += mdrv_mpool_io.h
#EXPORT_H_FILES    += mdrv_mpool_st.h
