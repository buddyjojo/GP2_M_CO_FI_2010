#
# Makefile for MStar MPIF device drivers.
#

# general options
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/mpif
EXTRA_CFLAGS      +=  -Idrivers/mstar/system
EXTRA_CFLAGS      +=  -Idrivers/mstar/mpif/titania
#EXTRA_CFLAGS      +=  -Idrivers/mstar/common/titania
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iarch/mips/mips-boards/$(CONFIG_MSTAR_CHIP_NAME)/board

# specific options
#EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-y             += mhal_mpif.o