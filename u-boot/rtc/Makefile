#
# (C) Copyright 2001-2006
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

#CFLAGS += -DDEBUG

LIB		 = $(obj)librtc.a

COBJS	 = date.o
COBJS	+= bf533_rtc.o
COBJS	+= ds12887.o
COBJS	+= ds1302.o
COBJS	+= ds1306.o
COBJS	+= ds1307.o
COBJS	+= ds1337.o
COBJS	+= ds1374.o
COBJS	+= ds1556.o
COBJS	+= ds164x.o
COBJS	+= ds174x.o
COBJS	+= m41t11.o
COBJS	+= max6900.o
COBJS	+= m48t35ax.o
COBJS	+= mc146818.o
COBJS	+= mk48t59.o
COBJS	+= mpc5xxx.o
COBJS	+= mpc8xx.o
COBJS	+= pcf8563.o
COBJS	+= s3c24x0_rtc.o
COBJS	+= rs5c372.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	@$(CCDV) $(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
