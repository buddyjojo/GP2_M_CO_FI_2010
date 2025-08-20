#
# Makefile for MStar GPIO device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/gpio
EXTRA_CFLAGS      +=  -Idrivers/mstar/gpio/$(CONFIG_MSTAR_CHIP_NAME)

# dhjung LGE
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common/$(CONFIG_MSTAR_CHIP_NAME)
#EXTRA_CFLAGS      +=  -Idrivers/mstar/8051lite

# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_GPIO)             += mdrv_gpio.o
##dhjung LGE obj-$(CONFIG_MSTAR_GPIO)             += mdrv_gpio_io.o
##dhjung LGE obj-$(CONFIG_MSTAR_GPIO)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_GPIO)	+= mdrv-gpio.o
mdrv-gpio-objs				+= mdrv_gpio.o mdrv_gpio_io.o
mdrv-gpio-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_gpio.o

#export header files
#EXPORT_H_FILES    += mdrv_gpio_io.h
