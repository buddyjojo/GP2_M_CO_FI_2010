#
# (C) Copyright 2004-2006
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

LIB		 = $(obj)libcommon.a

AOBJS	 =

 COBJS	 = main.o
#COBJS	+= ACEX1K.o
#COBJS	+= altera.o
#COBJS	+= bedbug.o
#COBJS	+= circbuf.o
#COBJS	+= cmd_ace.o
#COBJS	+= cmd_autoscript.o
#COBJS	+= cmd_bdinfo.o
#COBJS	+= cmd_bedbug.o
#COBJS	+= cmd_bmp.o
 COBJS	+= cmd_boot.o
 COBJS	+= cmd_bootm.o
 COBJS	+= cmd_cache.o
#COBJS	+= cmd_console.o
#COBJS	+= cmd_date.o
#COBJS	+= cmd_dcr.o
#COBJS	+= cmd_diag.o
#COBJS	+= cmd_display.o
#COBJS	+= cmd_doc.o
#COBJS	+= cmd_dtt.o
#COBJS	+= cmd_eeprom.o
#COBJS	+= cmd_elf.o
#COBJS	+= cmd_ext2.o
#COBJS	+= cmd_fat.o
#COBJS	+= cmd_fdc.o
#COBJS	+= cmd_fdos.o
#COBJS	+= cmd_flash.o
#COBJS	+= cmd_fpga.o
 COBJS	+= cmd_i2c.o
#COBJS	+= cmd_ide.o
#COBJS	+= cmd_immap.o
#COBJS	+= cmd_itest.o
#COBJS	+= cmd_jffs2.o
#COBJS	+= cmd_load.o
#COBJS	+= cmd_log.o
 COBJS	+= cmd_mem.o
#COBJS	+= cmd_mii.o
#COBJS	+= cmd_misc.o
#COBJS	+= cmd_mmc.o
 COBJS	+= cmd_nand.o
 COBJS	+= cmd_net.o
 COBJS	+= cmd_nvedit.o
#COBJS	+= cmd_pci.o
#COBJS	+= cmd_pcmcia.o
#COBJS	+= cmd_portio.o
#COBJS	+= cmd_reginfo.o
#COBJS	+= cmd_reiser.o
#COBJS	+= cmd_scsi.o
#COBJS	+= cmd_spi.o
#COBJS	+= cmd_universe.o
 COBJS	+= cmd_usb.o
#COBJS	+= cmd_vfd.o
 COBJS	+= command.o
 COBJS	+= console.o
 COBJS	+= devices.o
 COBJS	+= dlmalloc.o
#COBJS	+= docecc.o
#COBJS	+= environment.o
 COBJS	+= env_common.o
 COBJS	+= env_nand.o
#COBJS	+= env_dataflash.o
#COBJS	+= env_flash.o
#COBJS	+= env_eeprom.o
#COBJS	+= env_nvram.o
#COBJS	+= env_nowhere.o
 COBJS	+= exports.o
#COBJS	+= flash.o
#COBJS	+= fpga.o
#COBJS	+= ft_build.o
#COBJS	+= hush.o
#COBJS	+= kgdb.o
#COBJS	+= lcd.o
 COBJS	+= lists.o
#COBJS	+= lynxkdi.o
#COBJS	+= memsize.o
#COBJS	+= miiphybb.o
#COBJS	+= miiphyutil.o
#COBJS	+= s_record.o
#COBJS	+= serial.o
#COBJS	+= soft_i2c.o
#COBJS	+= soft_spi.o
#COBJS	+= spartan2.o
#COBJS	+= spartan3.o
COBJS	+= usb.o
#COBJS	+= usb_kbd.o
#COBJS	+= usb_storage.o
#COBJS	+= virtex2.o
#COBJS	+= xilinx.o
#COBJS	+= crc16.o
#COBJS	+= xyzModem.o
#COBJS	+= cmd_mac.o
COBJS	+= cmd_splash.o
COBJS	+= cmd_emac.o
COBJS	+= cmd_miu.o
#COBJS	+= cmd_micom.o

SRCS	:= $(AOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS))

CPPFLAGS += -I..

all:	$(LIB) $(AOBJS)

$(LIB): $(obj).depend $(OBJS)
	@$(CCDV) $(AR) $(ARFLAGS) $@ $(OBJS)

$(obj)environment.o: $(src)environment.c $(obj)../tools/envcrc
	@$(CCDV) $(CC) $(AFLAGS) -Wa,--no-warn \
		-DENV_CRC=$(shell $(obj)../tools/envcrc) \
		-c -o $@ $(src)environment.c

$(obj)../tools/envcrc:
	$(MAKE) -C ../tools

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
