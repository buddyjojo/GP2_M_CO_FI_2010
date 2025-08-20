#============================================================================#
#                                                                            #
#   LGE. DTV RESEARCH LABORATORY                                             #
#   COPYRIGHT(c) LGE CO.,LTD. 2005. SEOUL, KOREA.                            #
#   All rights are reserved.                                                 #
#   No part of this work covered by the copyright hereon may be              #
#   reproduced, stored in a retrieval system, in any form or                 #
#   by any means, electronic, mechanical, photocopying, recording            #
#   or otherwise, without the prior permission of LG Electronics.            #
#                                                                            #
#----------------------------------------------------------------------------#
#                                                                            #
#   MicroWindows Makefile                                                    #
#                                                                            #
#   Author : T.J.Park (with help of C.S.Kim)                                 #
#   Date   : 2005. 9. 5                                                      #
#   Date   : 2007. 3. 2 (for RUKH2.0 by stonekim)                            #
#   Date   : 2007. 3. 20 (for 16bpp ARGB1555 by stonekim)                    #
#                                                                            #
#============================================================================#

#=============================================================================
#	Config
#=============================================================================

PIXEL_DEPTH	=	32BPP
#PIXEL_DEPTH		=	16BPP


PRJ_DIR		= ../../..
ARCHIVE_DIR	= $(PWD)

#-----------------------------------------------------------------------------
#	External Rules Definition
#-----------------------------------------------------------------------------
include $(PRJ_DIR)/platform.mk
include $(PRJ_DIR)/tools.mk
include $(PRJ_DIR)/model.mk


#=============================================================================
#	Tools definition
#=============================================================================

CROSS	?=	mips2_fp_be-		# use default when "CROSS" is not defined yet.

CC		=	@$(CCDV) $(CROSS)gcc
AR		=	$(CROSS)ar

CP		=	cp
RM		=	rm -f
MV		=	mv -f
MKDIR	=	mkdir -p
RMDIR	=	rm -r -f
TOUCH	=	touch
ECHO	=	echo

#=============================================================================
#	Directory definition
#=============================================================================
ARCHIVE_DIR		=	$(CURDIR)
ZLIB_DIR		=	$(ARCHIVE_DIR)/../otherpkgs/zlib
PNG_DIR			=	$(ARCHIVE_DIR)/../otherpkgs/LIBPNG-1.2.8
#JPEG_DIR		=	$(ARCHIVE_DIR)/../otherpkgs/jpeg-6b
#JPEG_DIR		=	$(ARCHIVE_DIR)/../../eme/JpegLib
FONTFUSION_DIR	=	$(ARCHIVE_DIR)/../../fontfusion
UTF_DIR			=	$(ARCHIVE_DIR)/../../utf

#-----------------------------------------------------------------------------
#	External Rules Definition
#-----------------------------------------------------------------------------


#=============================================================================
#	Source Definition (Add or Remove your local files)
#=============================================================================
SRCS	=   $(ARCHIVE_DIR)/engine/devarc.c \
			$(ARCHIVE_DIR)/engine/devclip.c \
			$(ARCHIVE_DIR)/engine/devdraw.c \
			$(ARCHIVE_DIR)/engine/devfont.c \
			$(ARCHIVE_DIR)/engine/devimage.c \
			$(ARCHIVE_DIR)/engine/devkbd.c \
			$(ARCHIVE_DIR)/engine/devlist.c \
			$(ARCHIVE_DIR)/engine/devmouse.c \
			$(ARCHIVE_DIR)/engine/devopen.c \
			$(ARCHIVE_DIR)/engine/devpal1.c \
			$(ARCHIVE_DIR)/engine/devpal2.c \
			$(ARCHIVE_DIR)/engine/devpal4.c \
			$(ARCHIVE_DIR)/engine/devpal8.c \
			$(ARCHIVE_DIR)/engine/devpoly.c \
			$(ARCHIVE_DIR)/engine/devrgn.c \
			$(ARCHIVE_DIR)/engine/devrgn2.c \
			$(ARCHIVE_DIR)/engine/devstipple.c \
			$(ARCHIVE_DIR)/engine/error.c \
			$(ARCHIVE_DIR)/engine/font_data_table.c \
			$(ARCHIVE_DIR)/engine/ksc2unicode.c \
			$(ARCHIVE_DIR)/engine/hk2unicode.c \
			$(ARCHIVE_DIR)/engine/lgi.c \
			$(ARCHIVE_DIR)/drivers/kbd_null.c \
			$(ARCHIVE_DIR)/drivers/mou_null.c \
			$(ARCHIVE_DIR)/nanox/nxutil.c \
			$(ARCHIVE_DIR)/nanox/srvclip.c \
			$(ARCHIVE_DIR)/nanox/srvevent.c \
			$(ARCHIVE_DIR)/nanox/srvfunc.c \
			$(ARCHIVE_DIR)/nanox/srvmain.c \
			$(ARCHIVE_DIR)/nanox/srvutil.c

ifeq ($(PIXEL_DEPTH), 32BPP)
SRCS	+=	$(ARCHIVE_DIR)/drivers/scr_linux_rukh.c \
			$(ARCHIVE_DIR)/drivers/scr_gen32.c
endif

ifeq ($(PIXEL_DEPTH), 16BPP)
#SRCS	+=	$(ARCHIVE_DIR)/drivers/scr_linux_rukh16bpp.c \
#			$(ARCHIVE_DIR)/drivers/scr_gen16.c
SRCS	+=	$(ARCHIVE_DIR)/drivers/scr_linux_chaplin16bpp.c \
			$(ARCHIVE_DIR)/drivers/scr_gen16.c
endif

ifeq ($(FONT_ENGINE_TYPE), FONTFUSION)
SRCS	+=	$(ARCHIVE_DIR)/engine/font_fontfusion.c
endif

ifeq ($(FONT_ENGINE_TYPE), JUNGLESYSTEM)
SRCS	+=	$(ARCHIVE_DIR)/engine/font_utf.c
endif


#=============================================================================
#	Object Name - Auto Generation Rules
#=============================================================================
OBJS	=	$(SRCS:.c=.o)



#=============================================================================
#	Tools definition
#=============================================================================
INCLUDES	=	-I. -I$(ARCHIVE_DIR)/include				\
				-I$(FONTFUSION_DIR)						\
				-I$(UTF_DIR)							\
				-I$(PNG_DIR)							\
				-I$(ZLIB_DIR)
#				-I$(JPEG_DIR)

#USER_DEFS	=	-DLINUX -DNONETWORK=1 -DTHREADSAFE=1 -DMW_NOSIGNALS \
#				-DHAVE_PNG_SUPPORT=1 -DHAVE_GIF_SUPPORT=1 -DHAVE_JPEG_SUPPORT=1 -DHAVE_LGI_SUPPORT=1 \
#				-DHAVE_FILEIO -DMW_CPU_BIG_ENDIAN=1

USER_DEFS	=	-DLINUX -DNONETWORK=1 -DTHREADSAFE=1 -DMW_NOSIGNALS \
				-DHAVE_PNG_SUPPORT=1 -DHAVE_LGI_SUPPORT=1 \
				-DHAVE_FILEIO -DMW_CPU_BIG_ENDIAN=1

ifeq ($(FONT_ENGINE_TYPE), FONTFUSION)
USER_DEFS	+=	-DHAVE_FONTFUSION_SUPPORT=1
USER_DEFS	+=	-DHAVE_UTF_SUPPORT=0
endif

ifeq ($(FONT_ENGINE_TYPE), JUNGLESYSTEM)
USER_DEFS	+=	-DHAVE_FONTFUSION_SUPPORT=0
USER_DEFS	+=	-DHAVE_UTF_SUPPORT=1
endif

CFLAGS	=	-mips32r2 -mtune=r4600 \
			-g -O2 -c \
			-Wall -Winline -Wstrict-prototypes -Wundef\
			-Wwrite-strings -Wno-comment -fsigned-char \
			-std=gnu99 -D_REENTRANT -D_GNU_SOURCE \
			$(INCLUDES) $(USER_DEFS)

ifeq ($(PIXEL_DEPTH), 32BPP)
CFLAGS	+=	-D_32BPP_
endif

ifeq ($(PIXEL_DEPTH), 16BPP)
CFLAGS	+=	-D_16BPP_
endif


#=============================================================================
#	Top-Level Rule : Making Target
#=============================================================================

LIBNAME	= libnanox.a

$(LIBNAME) : $(OBJS)
	$(ECHO) Building $(LIBNAME) archives
	$(AR) -rcsu $@ $(OBJS)
	$(ECHO) Successfully Completed
	cp $(LIBNAME) ../../libs


clean :
	@$(ECHO) Cleaning all object, images ...
	@$(RM) $(OBJS)
	@$(RM) $(LIBNAME)
	@$(ECHO) Succesfully Completed ...


%.o: %.c
	@$(CC) $(CFLAGS) $< -o $@

