#******************************************************************************#
#	DIGITAL TV RESEARCH LAB., LG ELECTRONICS INC., SEOUL, KOREA                #
#	Copyright(c) 1998-2007 by LG Electronics Inc.                              #
#                                                                              #
#   All rights are reserved.                                                   #
#   No part of this work covered by the copyright hereon may be                #
#   reproduced, stored in a retrieval system, in any form or                   #
#   by any means, electronic, mechanical, photocopying, recording              #
#   or otherwise, without the prior permission of LG Electronics.              #
#                                                                              #
#   ---------------------------------------------------------------------------#
#                                                                              #
#	FILE NAME	:	nanox.mk                                                   #
#	VERSION		:	1.0                                                        #
#	AUTHOR		:	Jackee, Lee(이재길, jackee@lge.com)                        #
#	DATE        :	2007/01/30                                                 #
#	DESCRIPTION	:	This file is to make indivisual sub modules.               #
#******************************************************************************#

SUBMOD_NAME		= nanox

ifeq ($(SUBMOD_NAME),)
$(error SUBMOD_NAME is not defined, please define this variable above)
endif

#==============================================================================
#	Include incs.mk from top most project directory
#==============================================================================
TOP_DIR		:= ../../..
include $(TOP_DIR)/incs.mk

#==============================================================================
#	Declare output archive name and location for intermediate files
#==============================================================================
OBJ_DIR		:= $(OBJ_DIR_BASE)
TGT_LIB		 = $(call cond_assign, YES, USE_LIB_AUTO_MAKE,$(LIB_DIR)/,$(MW_LIB_DIR)/)lib$(SUBMOD_NAME).a

#==============================================================================
#	List up source files to be compiled
#------------------------------------------------------------------------------
#	Append source file lists to $(SRCS).
#	It's not recommended to use full or absolute pathname in it.
#	If that is used, 'make' may be confused to find appropriate destination
#	directory for object files.
#	All source files will be searched according to below $(INC_SRCS_PATH).
#==============================================================================
SRCS			 =
SRCS			+= devarc.c
SRCS			+= devclip.c
SRCS			+= devdraw.c
SRCS			+= devfont.c
SRCS			+= devimage.c
SRCS			+= devkbd.c
SRCS			+= devlist.c
SRCS			+= devmouse.c
SRCS			+= devopen.c
SRCS			+= devpal1.c
SRCS			+= devpal2.c
SRCS			+= devpal4.c
SRCS			+= devpal8.c
SRCS			+= devpoly.c
SRCS			+= devrgn.c
SRCS			+= devrgn2.c
SRCS			+= devstipple.c
SRCS			+= error.c
SRCS			+= font_data_table.c
SRCS			+= ksc2unicode.c
SRCS			+= gb2unicode.c
SRCS			+= hk2unicode.c
SRCS			+= lgi.c
SRCS			+= kbd_null.c
SRCS			+= mou_null.c
SRCS			+= nxutil.c
SRCS			+= srvclip.c
SRCS			+= srvevent.c
SRCS			+= srvfunc.c
SRCS			+= srvmain.c
SRCS			+= srvutil.c
SRCS			+= arabic_contextual.c
SRCS			+= scr_reverse.c

ifeq ($(NANOX_PIXEL_DEPTH), 32BPP)
SRCS			+= scr_linux32bpp.c
SRCS			+= scr_gen32.c
endif

ifeq ($(NANOX_PIXEL_DEPTH), 16BPP)
SRCS			+= scr_linux16bpp.c
SRCS			+= scr_gen16.c
endif

SRCS			+= $(call cond_assign, 0, USE_TTF_ENGINE,     ,font_fontfusion.c)
SRCS			+= $(call cond_assign, 0, USE_UTF_ENGINE,     ,font_utf.c       )
SRCS			+= $(call cond_assign, 0, USE_UNITYPE_ENGINE, ,font_unitype.c   )

VPATH			:= engine drivers nanox

#==============================================================================
#	Include Path Definition
#------------------------------------------------------------------------------
#	Source and header files will be searched according to $(INC_SRCS_PATH)
#	If there are files with same name in this search path, the file which is
#	found first will be used.
#==============================================================================
INC_SRCH_PATH	 =
INC_SRCH_PATH	+= $(CMN_DIR)/include
INC_SRCH_PATH	+= $(MW_DIR)/include
INC_SRCH_PATH	+= $(DRV_DIR)/include
INC_SRCH_PATH	+= $(MW_DIR)/microwindows/src/include
INC_SRCH_PATH	+= $(MW_DIR)/microwindows/otherpkgs/zlib
INC_SRCH_PATH	+= $(MW_DIR)/microwindows/otherpkgs/LIBPNG-1.2.8
INC_SRCH_PATH	+= $(MW_DIR)/eme/jpeglib
INC_SRCH_PATH	+= $(MW_DIR)/fontfusion
INC_SRCH_PATH	+= $(MW_DIR)/unitype
INC_SRCH_PATH	+= $(MW_DIR)/utf

#==============================================================================
#   Module Dependent flags
#------------------------------------------------------------------------------
#   	Add module dependent definitions or flags to $(PER_MOD_FLAGS)
#==============================================================================
PER_MOD_CFLAGS	 =
PER_MOD_CFLAGS	+= -DLINUX
PER_MOD_CFLAGS	+= -DNONETWORK=1
PER_MOD_CFLAGS	+= -DTHREADSAFE=1
PER_MOD_CFLAGS	+= -DMW_NOSIGNALS
PER_MOD_CFLAGS	+= -DHAVE_PNG_SUPPORT=1
PER_MOD_CFLAGS	+= -DHAVE_LGI_SUPPORT=1
ifeq ($(NANOX_PIXEL_DEPTH), 16BPP)
PER_MOD_CFLAGS	+= -DHAVE_I16_SUPPORT=1
endif
ifneq ($(PLATFORM_TYPE), MSTAR_PLATFORM)
#	2008,11,27: MSTAR_PLATFORM에선 jpeg lib 제외
PER_MOD_CFLAGS	+= -DHAVE_JPEG_SUPPORT=1
endif
PER_MOD_CFLAGS	+= -DHAVE_GIF_SUPPORT=1
PER_MOD_CFLAGS	+= -DHAVE_FILEIO

ifeq ($(ENDIAN_TYPE), BIG_ENDIAN)
PER_MOD_CFLAGS	+= -DMW_CPU_BIG_ENDIAN=1
PER_MOD_CFLAGS	+= -DMW_CPU_LITTLE_ENDIAN=0
endif
ifeq ($(ENDIAN_TYPE), LITTLE_ENDIAN)
PER_MOD_CFLAGS	+= -DMW_CPU_BIG_ENDIAN=0
PER_MOD_CFLAGS	+= -DMW_CPU_LITTLE_ENDIAN=1
endif

ifeq ($(NANOX_PIXEL_DEPTH), 32BPP)
PER_MOD_CFLAGS	+= -D_32BPP_
endif

ifeq ($(NANOX_PIXEL_DEPTH), 16BPP)
PER_MOD_CFLAGS	+= -D_16BPP_
endif

#==============================================================================
#	Extract list of object files from list of source files
#==============================================================================
OBJS		 = $(foreach src, $(SRCS), $(OBJ_DIR)/$(src:.c=.o))

#==============================================================================
#	Default rule for this makefile
#==============================================================================
all : $(TGT_LIB)

#==============================================================================
#	Read global make rules from top most directory
#==============================================================================
include $(TOP_DIR)/rules.mk
