#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG -DDEBUG

LIB		 = $(obj)libdrivers.a

 COBJS	 =
#COBJS	+= cfb_console.o
#COBJS	+= cfi_flash.o
#COBJS	+= dataflash.o
#COBJS	+= netconsole.o
#COBJS	+= serial.o
#COBJS	+= serial_uranus.o
 COBJS	+= serial_titania.o
#COBJS	+= usbdcore.o
#COBJS	+= usbtty.o
 COBJS	+= drvusbmain.o
 COBJS	+= drvusbmain2.o
 COBJS	+= drvhostlib.o
 COBJS	+= drvhostlib2.o
 COBJS	+= ehci_usb.o
 COBJS	+= usb_lan.o
 COBJS	+= pegasus.o
 COBJS	+= rtl8150.o
 COBJS	+= drvNAND.o
 COBJS	+= titania_hw_i2c.o
#COBJS  += titania_sw_i2c.o
#COBJS  += mdrv_image.o
#COBJS  += drvMicom.o
 COBJS  += mdrv_emac.o
 COBJS  += mhal_emac.o
 COBJS  += spiflash.o
 COBJS  += mdrv_pwm.o
 COBJS  += mdrv_pm.o
#COBJS  += drvGlobal.o
 COBJS  += drvmicomupdate.o
# COBJS	+= mdrv_mfc.o
 COBJS	+= mdrv_scaler.o
 COBJS	+= mdrv_tcon.o
 COBJS	+= mdrv_tcon_tbl.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	@$(CCDV) $(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
