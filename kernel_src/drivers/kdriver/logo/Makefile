#
# Makefile for MStar System device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/logo
EXTRA_CFLAGS      +=  -Idrivers/mstar/logo/mdrvmfc
#EXTRA_CFLAGS      +=  -Idrivers/mstar/logo/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
EXTRA_CFLAGS      +=  

# files
# dhjung LGE
obj-$(CONFIG_MSTAR_SYSTEM)	+= mdrv-logo.o
mdrv-logo-objs	+= mdrv_logo.o
mdrv-logo-objs	+= mdrv_logo_scaler.o
mdrv-logo-objs	+= mdrv_mfc.o
mdrv-logo-objs	+= drv_extmicom.o
mdrv-logo-objs	+= mdrvmfc/mdrv_mfc_fb.o
mdrv-logo-objs	+= mdrvmfc/mdrv_mfc_film.o
mdrv-logo-objs	+= mdrvmfc/mdrv_mfc_init.o
mdrv-logo-objs	+= mdrvmfc/mdrv_mfc_mcu.o
mdrv-logo-objs	+= mdrvmfc/mdrv_mfc_panel.o
mdrv-logo-objs	+= mdrvmfc/mdrv_mfc_scalerip.o
mdrv-logo-objs	+= mdrvmfc/mdrv_mfc_scalerop.o
mdrv-logo-objs	+= mdrvmfc/pnl_lceall.o

# export header files
#EXPORT_H_FILES    += mdrv_logo_io.h
#EXPORT_H_FILES    += mdrv_logo_st.h
