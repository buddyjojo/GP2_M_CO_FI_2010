#
# Makefile for MStar PWM device drivers.
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
EXTRA_CFLAGS      +=  -Idrivers/mstar/pwm
EXTRA_CFLAGS      +=  -Idrivers/mstar/pwm/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
#EXTRA_CFLAGS      +=  -Iinclude/asm-mips/titania
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
##dhjung LGE obj-$(CONFIG_MSTAR_PWM)             += mdrv_pwm.o
##dhjung LGE obj-$(CONFIG_MSTAR_PWM)             += mdrv_pwm_io.o
##dhjung LGE obj-$(CONFIG_MSTAR_PWM)             += $(CONFIG_MSTAR_CHIP_NAME)/

# dhjung LGE
obj-$(CONFIG_MSTAR_PWM)		+= mdrv-pwm.o
mdrv-pwm-objs				+= mdrv_pwm.o mdrv_pwm_io.o
mdrv-pwm-objs				+= $(CONFIG_MSTAR_CHIP_NAME)/mhal_pwm.o

# export header files
EXPORT_H_FILES    += mdrv_pwm_io.h
