#
# Makefile for MStar SPI device drivers.
#

# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/common
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/spi
#EXTRA_CFLAGS      +=  -Idrivers/mstar/spi/$(CONFIG_MSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DRED_LION

# files
obj-$(CONFIG_MSTAR_SPI)             += mdrv-spi.o 
#obj-y             += $(CONFIG_MSTAR_CHIP_NAME)/
mdrv-spi-objs                       += mdrv_spi.o
mdrv-spi-objs                       += mdrv_spi_io.o

