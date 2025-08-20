#ifndef	_NANO_X_H
#define	_NANO_X_H
/* Copyright (c) 1999, 2000, 2001, 2002, 2003 Greg Haerr <greg@censoft.com>
 * Portions Copyright (c) 2002 by Koninklijke Philips Electronics N.V.
 * Copyright (c) 2000 Alex Holden <alex@linuxhacker.org>
 * Copyright (c) 1991 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 *
 * Nano-X public definition header file:  user applications should
 * include only this header file.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "mwtypes.h"			/* exported engine MW* types*/

/*
 * The following typedefs are inherited from the Microwindows
 * engine layer.
 */
typedef MWCOORD 		GR_COORD;			/** coordinate value */
typedef MWCOORD 		GR_SIZE;			/** size value */
typedef MWCOLORVAL 		GR_COLOR;			/** full color value */
typedef MWPIXELVAL 		GR_PIXELVAL;		/** hw pixel value */
typedef MWIMAGEBITS 	GR_BITMAP;			/** bitmap unit */
typedef MWUCHAR 		GR_CHAR;			/** filename, window title */

/**
 * Text encoding flags or text alignment flags.
 *
 * In GrText, one of the encoding flags and one of the alignment
 * flags shall be combinated and used.
 *
 * In GrGetGCTextSize, only one of the encoding flags shall be
 * used. Alignment flags shall be ignored.
 *
 *
 * <TABLE>
 * Alignment flags		Description
 * ---------------		--------------------------------------------------
 * GR_TFTOP				y parameter of the function means the top line of
 *						 the text
 * GR_TFBASELINE		y parameter of the function means the baseline
 *						 position of the text
 * GR_TFBOTTOM			y parameter of the function means the bottom line
 *						 of the text
 * </TABLE>
 *
 * <TABLE>
 * Encoding flags		Description
 * --------------		-------------------------------------------------------
 * GR_TFASCII			Each character has one-byte length, and the code set is
 * 						 ASCII.
 * GR_TFUTF8			Universal transformation format. Each character has
 *						 one-byte length, and the code set is Unicode.
 * GR_TFUC16			Each character is a 16 bit short integer, and the code
 *						 set is Unicode.
 * GR_TFUC32			Each character is a 32 bit integer, and the code set is
 *						 Unicode.
 * GR_TFXCHAR2B			X11 big endian PCF. Each character has two-byte length
 *						 in byte-order. <p>
 *						 This is different from GR_TFUC16, because GR_TFUC16 is
 *						 just a two-byte integer. In big-endian system, the conversion
 *						 rule will be exactly the same, though the code set is different.
 * GR_TFKSC5601			<B>LGE-Specific</B> : each character has one or two-byte
 *						 length depending on the first byte of the character.
 *						 The code set is kSC5601.
 * </TABLE>
 */
typedef MWTEXTFLAGS		GR_TEXTFLAGS;

typedef MWKEY	 		GR_KEY;				/** keystroke value */
typedef MWSCANCODE		GR_SCANCODE;		/** oem keystroke scancode value */
typedef MWKEYMOD		GR_KEYMOD;			/** keystroke modifiers */

/**
 * Screen Information. Used by GrGetScreenInfo().
 *
 * This structure has the following members. <p>
 * <B>LGE-Specific</B> : Only rows, cols, bpp and pixtype are used.
 *
 *
 * - rows : number of rows on screen, i.e, the height, described by the number of pixels.
 * - cols : number of columns on screen, i.e, the width, described by the number of pixels.
 * - xdpcm : dots per centimeter in x direction.
 *           <B>LGE-Specific</B> : This value is not used.
 * - ydpcm : dots per centimeter in y direction.
 *           <B>LGE-Specific</B> : This value is not used.
 * - planes : hw number of planes(?).
 *           <B>T.J.Park Note</B> : I don't know what this is.
 *             Researching of other drivers tells me that this value is set to 4 in VGA/EGA systems
 *             , and 1 in other systems.
 *             In our implementation, the returned value shall be always 1.
 * - bpp : the bits per pixel.
 * - ncolors : number of colors supported.
 *           <B>T.J.Park Note</B> : In 32 bpp system, this value shall be -1.
 *              We need 33 bit to describe the number of colors!!!
 * - fonts : number of built-in fonts.
 *           <B>LGE-Specific</B> : This value is always 0, because we don't define any built-in font.
 * - buttons : buttons which are implemented.
 *           <B>LGE-Specific</B> : This value is always 0.
 * - modifiers : modifiers which are implemented.
 *           <B>T.J.Park Note</B> : I don't know the meaning of this value. The returned value may be undefined.
 * - pixtype : format of pixel value. One of the following values.
 *
 * <TABLE>
 * MWPF_PALETTE         pixel is packed 8 bits 1, 4 or 8 pal index
 * MWPF_TRUECOLOR0888   pixel is packed 32 bits 8/8/8 truecolor
 * MWPF_TRUECOLOR888    pixel is packed 24 bits 8/8/8 truecolor
 * MWPF_TRUECOLOR565    pixel is packed 16 bits 5/6/5 truecolor
 * MWPF_TRUECOLOR555    pixel is packed 16 bits 5/5/5 truecolor
 * MWPF_TRUECOLOR332    pixel is packed 8 bits 3/3/2 truecolor
 * MWPF_TRUECOLOR8888   pixel is packed 32 bits 8/8/8/8 truecolor with alpha
 * </TABLE>
 *
 * - portrait : current portrait mode.
 *              <B>LGE-Specific</B> : This value is always 0.
 * - fbdriver : true if running mwin fb screen driver.
 *              <B>LGE-Specific</B> : This value is not used.
 * - rmask : red mask bits in pixel.
 *              <B>LGE-Specific</B> : This value is not used.
 * - gmask : green mask bits in pixel.
 *              <B>LGE-Specific</B> : This value is not used.
 * - bmask : blue mask bits in pixel.
 *              <B>LGE-Specific</B> : This value is not used.
 * - xpos : current x mouse position.
 *              <B>LGE-Specific</B> : This value is not used because we don't use mouse driver.
 * - ypos : current y mouse position.
 *              <B>LGE-Specific</B> : This value is not used because we don't use mouse driver.
 * - vs_width : virtual screen width.
 *              <B>T.J.Park Note</B> : I don't know the meaning of this value at all.
 * - vs_height : virtual screen height.
 *              <B>T.J.Park Note</B> : I don't know the meaning of this value at all.
 * - ws_width : workspace width.
 *              <B>T.J.Park Note</B> : It seems that this value exists in order to
 *                 support the task bar and the work-space in PC system.
 * - ws_height : workspace height.
 *              <B>T.J.Park Note</B> : It seems that this value exists in order to
 *                 support the task bar and the work-space in PC system.
 */
typedef MWSCREENINFO	GR_SCREEN_INFO;

typedef MWWINDOWFBINFO	GR_WINDOW_FB_INFO;	/** direct client-mapped window info */
typedef MWFONTINFO		GR_FONT_INFO;		/** font information */

/**GR_IMAGE_INFO
  * Image Information. Used by GrGetImageInfo().
  *
  *
  *
  * <B>T.J.Park Note</B> : It is very interesting that this
  * structure does not contain a 'pixtype' member, though the
  * GR_SCREEN_INFO does. Moreover, any function of the
  * MicroWindows does not handle 16 bpp image. Why? After
  * researching the image-drawing functions of devdraw.c, I found
  * the followings.
  *
  *
  *
  *     * All 1 bpp, 4 bpp, and 8 bpp images are considered as
  *       indexed-color formats.
  *     * All 16 bpp images are converted to one of 24 bpp or 32
  *       bpp.
  *     * All images containing alpha channel are converted to 32
  *       bpp ARGB or ABGR, whatever the original format is. This is
  *       true even if the original image's format is an indexed color
  *       format.
  *
  * So the returned format of the image can be checked as
  * followings.
  *
  *
  *
  *     * If 'bpp' is less that or equal to 8, then the format is
  *       an indexed color format.
  *     * If 'bpp' is 24, then the format is RGB or BGR,
  *       depending on the 'compression' field.
  *     * If 'bpp' is 32, then the format is ARGB or ABGR,
  *       depending on the 'compression' field.
  *
  * \Note that the 'RGB' or 'BGR' order means a byte order, not
  * an integer order.
  *
  * This structure has the following members.
  *
  *
  *
  *     * id : image id
  *     * width : image width in pixels
  *     * height : image height in pixels
  *     * planes : number of image planes. This must be 1.
  *     * bpp : bits per pixel (1, 4, 8, 24, or 32, except 16.).
  *     * pitch : bytes per line
  *     * bytesperpixel : bytes per pixel
  *     * compression : compression algorithm. One of MWIMAGE_BGR, MWIMAGE_RGB, or MWIMAGE_ALPHA_CHANNEL
  * <TABLE>
  * MWIMAGE_BGR             compression flag: BGR byte order
  * MWIMAGE_RGB             compression flag: RGB not BGR bytes
  * MWIMAGE_ALPHA_CHANNEL   compression flag: 32-bit w/alpha.<P>
  *                          <B>LGE-Specific</B> : This mode is not used.<P>
  *                          All 32bpp formats are considered as ARGB or ABGR
  *                          format.
  * </TABLE>
  *
  *     * log_screen_width : <B>LGE-Specific</B> : This value is added for animated GIF. See GrLoadImageEx() for more information.
  *       This value is the logical screen height of a GIF image, which may be larger than the actual image width/height.
  *     * log_screen_height : <B>LGE-Specific</B> : This value is added for animated GIF. See GrLoadImageEx() for more information.
  *       This value is the logical screen width of a GIF image, which may be larger than the actual image width/height.
  *     * xoffset : <B>LGE-Specific</B> : This value is added for animated GIF. See GrLoadImageEx() for more information.
  *       This value is the x-offset of one of the images included in a GIF image. This value is relative to the left-top position of the logical screen of GIF.
  *     * yoffset : <B>LGE-Specific</B> : This value is added for animated GIF. See GrLoadImageEx() for more information.
  *       This value is the y-offset of one of the images included in a GIF image. This value is relative to the left-top position of the logical screen of GIF.
  *     * delaytime : <B>LGE-Specific</B> : This value is added for animated GIF. See GrLoadImageEx() for more information.
  *       This value is the delay time until drawing the next image.
  *       This value is specified by millisecond, though GIF89 defines it as 1/100 seconds.
  *     * inputflag : <B>LGE-Specific</B> : This value is added for animated GIF. See GrLoadImageEx() for more information.
  *       This value indicates whether or not user input is expected before continuing.
  *       If the flag is set, processing will continue when user input is entered.
  *       The nature of the user input is determined by the application (Carriage Return, Mouse Button Click, etc.).
  *     * disposal : <B>LGE-Specific</B> : This value is added for animated GIF. See GrLoadImageEx() for more information.
  *       This value indicates the way in which the graphic is to be treated after being displayed.
  * <TABLE>
  * 0         No disposal specified. The decoder is not required to take any action.
  * 1         Do not dispose. The graphic is to be left in place.
  * 2         Restore to background color. The area used by the graphic must be restored to the background color.
  * 3         Restore to previous. The decoder is required to restore the area overwritten by the graphic with what was there prior to rendering the graphic.
  * 4~7       To be defined.
  * </TABLE>
  *
  *
  *     * palsize : palette size
  *     * palette[256] : palette
  *     * imagebits : <B>LGE-Specific</B> : the pointer of the
  *       image buffer.
  *
*/
typedef MWIMAGEINFO		GR_IMAGE_INFO;

/**
 * Image Header for multicolor image representation. Used by GrDrawImageBits().
 * This structure has the following members. See GR_IMAGE_INFO for more information about each field.
 *
 *
 * - width : image width in pixels <p>
 * - height : image height in pixels <p>
 * - planes : number of image planes. This value must be 1. <p>
 * - bpp : bits per pixel (1, 4, 8, 24 or 32, except 16 bpp) <p>
 * - pitch : bytes per line <p>
 * - bytesperpixel : bytes per pixel <p>
 * - compression : compression algorithm. One of MWIMAGE_BGR, MWIMAGE_RGB, or MWIMAGE_ALPHA_CHANNEL <p>
 * <TABLE>
 * MWIMAGE_BGR				compression flag: BGR byte order
 * MWIMAGE_RGB				compression flag: RGB not BGR bytes
 * MWIMAGE_ALPHA_CHANNEL	compression flag: 32-bit w/alpha. <B>LGE-Specific</B> : This mode is not used. All 32bpp formats are considered as ARGB or ABGR format.
 * </TABLE>
 * - palsize : palette size <p>
 * - transcolor : transparent color or -1 if none <p>
 * - palette : palette. If true color is used, the value will be NULL. <p>
 * - imagebits : image bits (dword right aligned) <p>
 */
typedef MWIMAGEHDR		GR_IMAGE_HDR;

/**
 * Structure for output of GrLoadImageEx().
 *
 * - count : image count. If GrLoadImageEx() fails, this value shall be 0.
 * - images : the array of image IDs. The size of the array is defined as MAX_LOAD_IMAGE_EX.
 */
typedef MW_LOAD_IMAGE_EX_RESULT		GR_LOAD_IMAGE_EX_RESULT;

/**
 * Logical font descriptor, defined similiarly to Win32. Used by GrCreateFont().
 *
 * This structure has the following members. <p>
 *
 * - lfHeight : desired height in pixels. This value must be specified. <p>
 * - lfWidth : If 0, it is considered as same value as lfHeight. (<B>LGE-Specific</B> : This value is described in percentage(0 ~ 200), not in pixels) <p>
 * - lfEscapement : rotation in tenths of degree. <B>LGE-Specific</B> : Not supported. <p>
 * - lfOrientation : not used. <p>
 * - lfWeight : font weight. For example, you can make a bold font with this value. (<B>LGE-Specific</B> : Only MWLF_WEIGHT_REGULAR and MWLF_WEIGHT_BOLD are available.) <p>
 * - lfPitchValue : font pitch value. <B>LGE-Specific</B> : Not supported. <p>
 * - lfItalic : 1 for italic <p>
 * - lfUnderline : 1 for underline <p>
 * - lfStrikeOut : not used <p>
 * - lfCharSet : font character set. (<B>LGE-Specific</B> : This value is ignored.) <p>
 * - lfOutPrecision : font type selection. (<B>LGE-Specific</B> : This value is ignored.) <p>
 * - lfClipPrecision : not used <p>
 * - lfQuality : 1 for grey-scaling, 0 for monochrome. <p>
 * - lfPitchAndFamily : not used <p>
 * - lfClass : font class (renderer). (<B>LGE-Specific</B> : This value is ignored.) <p>
 * - lfPitch : font pitch. Only used by FONTMAPPER when enabled. <p>
 * - lfRoman : 1 for Roman letters (upright). Only used by FONTMAPPER when enabled. <p>
 * - lfSerif : 1 for Serifed font. Only used by FONTMAPPER when enabled. <p>
 * - lfSansSerif : 1 for Sans-serif font. Only used by FONTMAPPER when enabled. <p>
 * - lfModern : 1 for Modern font. Only used by FONTMAPPER when enabled. <p>
 * - lfMonospace : 1 for Monospaced font. Only used by FONTMAPPER when enabled. <p>
 * - lfProportional : 1 for Proportional font. Only used by FONTMAPPER when enabled. <p>
 * - lfOblique : 1 for Oblique (kind of Italic). Only used by FONTMAPPER when enabled. <p>
 * - lfSmallCaps : 1 for small caps. Only used by FONTMAPPER when enabled. <p>
 * - lfKerning : 1 for kerning. <p>
 * - lfFaceName[MWLF_FACESIZE] : the full path name for a font file, or a font face name. <p>
 *
 *
 * <B>LGE-Specific</B> : To help the setting of this structure, following macros have been newly included in mwtypes.h.
 * <TABLE>
 * MWLF_Clear(plogfont)				Clears the given GR_LOGFONT and set all the members as default values.
 * MWLF_SetBold(plogfont)			Set MWLF_WEIGHT_BOLD to the lfWeight member of the given GR_LOGFONT.
 * MWLF_SetRegular(plogfont)		Set MWLF_WEIGHT_REGULAR to the lfWeight member of the given GR_LOGFONT.
 * MWLF_SetItalics(plogfont)		Set italic to the lfItalic member of the given GR_LOGFONT
 * MWLF_SetKerning(plogfont)		Set kerning to the lfKerning member of the given GR_LOGFONT
 * MWLF_NoAntialias(plogfont)		Clear the lfQuality flag of the given GR_LOGFONT
 * MWLF_Width(plogfont, value)		Set the width of the given GR_LOGFONT, specified by percentage, ranged from 50 ~ 200 %.
 * MWLF_Height(plogfont, value)		Set the height of the given GR_LOGFONT, specified by the pixel count.
 * MWLF_Name(plogfont, name)		Set the font name of the given GR_LOGFONT. The name may be either a file name or an installed font name.
 * </TABLE>
 */
typedef MWLOGFONT		GR_LOGFONT;
typedef MWPALENTRY		GR_PALENTRY;		/** palette entry */
typedef MWPOINT			GR_POINT;			/** definition of a point */
typedef MWTIMEOUT		GR_TIMEOUT;			/** timeout value */
typedef MWFONTLIST		GR_FONTLIST;		/** list of fonts */
typedef MWKBINFO		GR_KBINFO;			/** keyboard information  */
typedef MWSTIPPLE       GR_STIPPLE;     	/** Stipple information   */
typedef MWTRANSFORM     GR_TRANSFORM;   	/** Transform information */

/* Basic typedefs. */
typedef int 			GR_COUNT;			/** number of items */
typedef unsigned char	GR_CHAR_WIDTH;		/** width of character */
typedef unsigned int	GR_ID;				/** resource ids */
typedef GR_ID			GR_DRAW_ID;			/** drawable id */
typedef GR_DRAW_ID		GR_WINDOW_ID;		/** window or pixmap id */
typedef GR_ID			GR_GC_ID;			/** graphics context id */
typedef GR_ID			GR_REGION_ID;		/** region id */
typedef GR_ID			GR_FONT_ID;			/** font id */
typedef GR_ID			GR_IMAGE_ID;		/** image id */
typedef GR_ID			GR_TIMER_ID;		/** timer id */
typedef GR_ID			GR_CURSOR_ID;		/** cursor id */
typedef unsigned short	GR_BOOL;			/** boolean value */
typedef int				GR_ERROR;			/** error types */
typedef int				GR_EVENT_TYPE;		/** event types */
typedef int				GR_UPDATE_TYPE;		/** window update types */
typedef unsigned long	GR_EVENT_MASK;		/** event masks */
typedef char			GR_FUNC_NAME[25];	/** function name */

/**
 * Window property flags.
 * <TABLE>
 * GR_WM_PROPS_NOBACKGROUND			Don't draw window background
 * GR_WM_PROPS_NOFOCUS				Don't set focus to this window
 * GR_WM_PROPS_NOMOVE				Don't let user move window
 * GR_WM_PROPS_NORAISE				Don't let user raise window
 * GR_WM_PROPS_NODECORATE			Don't redecorate window
 * GR_WM_PROPS_NOAUTOMOVE			Don't move window on 1st map
 * GR_WM_PROPS_NOAUTORESIZE			Don't resize window on 1st map
 * GR_WM_PROPS_TRANSPARENT			<B>LGE-Specific</B> : Make the window look transparent, i.e, not a rectangle. Or make the window translucent with source-over.
 * GR_WM_PROPS_APPWINDOW			Leave appearance to WM
 * GR_WM_PROPS_APPMASK				Appearance mask
 * GR_WM_PROPS_BORDER				Single line border
 * GR_WM_PROPS_APPFRAME				3D app frame (overrides border)
 * GR_WM_PROPS_CAPTION				Title bar
 * GR_WM_PROPS_CLOSEBOX				Close box
 * GR_WM_PROPS_MAXIMIZE				Application is maximized
 * </TABLE>
 */
typedef unsigned long	GR_WM_PROPS;
typedef unsigned long	GR_SERIALNO;		/** Selection request ID number */
typedef unsigned short	GR_MIMETYPE;		/** Index into mime type list */
typedef unsigned long	GR_LENGTH;			/** Length of a block of data */
typedef unsigned int	GR_BUTTON;			/** mouse button value */

/** Nano-X rectangle, different from MWRECT */
typedef struct {
	GR_COORD x;			/** upper left x coordinate */
	GR_COORD y;			/** upper left y coordinate */
	GR_SIZE  width;		/** rectangle width */
	GR_SIZE  height;	/** rectangle height */
} GR_RECT;

/* The root window id. T.J.Park added GR_ROOT_WINDOW2_ID for multiple OSD layers. */
#define	GR_ROOT_WINDOW_ID	((GR_WINDOW_ID) 1)	/** The ID of the root window for OSD Layer 1. If you have only one OSD, this is the only root window. */
#define GR_ROOT_WINDOW2_ID	((GR_WINDOW_ID) 2)	/** The ID of the root window for OSD Layer 2, if you have. */
/* Add more GR_ROOT_WINDOW00_ID if you have multiple OSD layers */

/* GR_COLOR color constructor */
#define GR_RGB(r,g,b)			MWRGB(r,g,b)
#define GR_ARGB(a,r,g,b)		MWARGB(a,r,g,b)

/* Drawing modes for GrSetGCMode */
#define	GR_MODE_COPY			MWMODE_COPY			/** src */
#define	GR_MODE_SET				MWMODE_COPY			/** obsolete, use GR_MODE_COPY */
#define	GR_MODE_XOR				MWMODE_XOR			/** src ^ dst */
#define	GR_MODE_OR				MWMODE_OR			/** src | dst */
#define	GR_MODE_AND				MWMODE_AND			/** src & dst */
#define	GR_MODE_CLEAR 			MWMODE_CLEAR		/** 0 */
#define	GR_MODE_SETTO1			MWMODE_SETTO1		/** 11111111 */ /* will be GR_MODE_SET*/
#define	GR_MODE_EQUIV			MWMODE_EQUIV		/** ~(src ^ dst) */
#define	GR_MODE_NOR				MWMODE_NOR			/** ~(src | dst) */
#define	GR_MODE_NAND			MWMODE_NAND			/** ~(src & dst) */
#define	GR_MODE_INVERT			MWMODE_INVERT		/** ~dst */
#define	GR_MODE_COPYINVERTED	MWMODE_COPYINVERTED	/** ~src */
#define	GR_MODE_ORINVERTED		MWMODE_ORINVERTED	/** ~src | dst */
#define	GR_MODE_ANDINVERTED		MWMODE_ANDINVERTED	/** ~src & dst */
#define GR_MODE_ORREVERSE		MWMODE_ORREVERSE	/** src | ~dst */
#define	GR_MODE_ANDREVERSE		MWMODE_ANDREVERSE	/** src & ~dst */
#define	GR_MODE_NOOP			MWMODE_NOOP			/** dst */
#define GR_MODE_SRC_OVER		MWMODE_SRC_OVER		/** blend src and dst based on alpha channel by porter-duff algorithm */

#define GR_MODE_DRAWMASK	0x00FF
#define GR_MODE_EXCLUDECHILDREN	0x0100				/* exclude children on clip*/

/* Line modes */
#define GR_LINE_SOLID           MWLINE_SOLID
#define GR_LINE_ONOFF_DASH      MWLINE_ONOFF_DASH

#define GR_FILL_SOLID           MWFILL_SOLID
#define GR_FILL_STIPPLE         MWFILL_STIPPLE
#define GR_FILL_OPAQUE_STIPPLE  MWFILL_OPAQUE_STIPPLE
#define GR_FILL_TILE            MWFILL_TILE

/* Polygon regions*/
#define GR_POLY_EVENODD			MWPOLY_EVENODD
#define GR_POLY_WINDING			MWPOLY_WINDING

/* builtin font std names*/
#define GR_FONT_SYSTEM_VAR		MWFONT_SYSTEM_VAR
#define GR_FONT_SYSTEM_FIXED	MWFONT_SYSTEM_FIXED
#define GR_FONT_GUI_VAR			MWFONT_GUI_VAR		/* deprecated*/
#define GR_FONT_OEM_FIXED		MWFONT_OEM_FIXED	/* deprecated*/

/* GrText/GrGetGCTextSize encoding flags*/
#define GR_TFASCII				MWTF_ASCII
#define GR_TFUTF8				MWTF_UTF8
#define GR_TFUC16				MWTF_UC16
#define GR_TFUC32				MWTF_UC32
#define GR_TFXCHAR2B			MWTF_XCHAR2B
#define GR_SIDEBUG				MWTF_SIDEBUG
#define GR_TFUC					GR_TFUC32	/* 현 system에서 unicode draw 시 string을 저장하는 wchar_t의 size에 따라
												unicode type 설정함(User는 설정할 필요 없음). 2byte: GR_TFUC16, 4byte:GR_TFUC32 */

/* <B>T.J.Park Note</B> : Though there was a conversion rule for ksc5601, no definition existed in API level.
   Now I add the API level definition for korean models. */
#define GR_TFKSC5601			MWTF_DBCS_EUCKR

#define GR_TFPACKMASK			MWTF_PACKMASK

/* GrText alignment flags */
#define GR_TFTOP				MWTF_TOP
#define GR_TFBASELINE			MWTF_BASELINE
#define GR_TFBOTTOM				MWTF_BOTTOM

/* GrText prop. flags */
#define GR_SKIP_DINGBAT			MWTF_SKIP_DINGBAT

/* Draw Text prop. flags*/
#define GR_GRADATION_UP				MWTF_GRADATION_UP
#define GR_GRADATION_DOWN			MWTF_GRADATION_DOWN
#define GR_GRADATION_LEFT			MWTF_GRADATION_LEFT
#define GR_GRADATION_RIGHT			MWTF_GRADATION_RIGHT
#define GR_GRADATIONMASK				MWTF_GRADATIONMASK
#define GR_LIGHT_GRADATION			MWTF_LIGHT_GRADATION

/* GrSetFontAttr flags*/
#define GR_TFKERNING			MWTF_KERNING
#define GR_TFANTIALIAS			MWTF_ANTIALIAS
#define GR_TFUNDERLINE			MWTF_UNDERLINE

/* GrArc, GrArcAngle types*/
#define GR_ARC					MWARC			/** arc only */
#define GR_ARCOUTLINE			MWARCOUTLINE	/** arc + outline */
#define GR_PIE					MWPIE			/** pie (filled) */

/* GrSetWindowRegion types*/
#define GR_WINDOW_BOUNDING_MASK	0	/** outer border*/
#define GR_WINDOW_CLIP_MASK	1		/** inner border*/

/* Booleans */
#define	GR_FALSE		0
#define	GR_TRUE			1

/* Loadable Image support definition */
#define GR_IMAGE_MAX_SIZE	(-1)

/* Button flags */
#define	GR_BUTTON_R			MWBUTTON_R 	/** right button */
#define	GR_BUTTON_M			MWBUTTON_M	/** middle button */
#define	GR_BUTTON_L			MWBUTTON_L	/** left button */
#define	GR_BUTTON_ANY		(MWBUTTON_R|MWBUTTON_M|MWBUTTON_L) /** any */

/* GrSetBackgroundPixmap flags */
#define GR_BACKGROUND_TILE		0	/** Tile across the window */
#define GR_BACKGROUND_CENTER	1	/** Draw in center of window */
#define GR_BACKGROUND_TOPLEFT	2	/** Draw at top left of window */
#define GR_BACKGROUND_STRETCH	4	/** Stretch image to fit window*/
#define GR_BACKGROUND_TRANS		8	/** Don't fill in gaps */

/* GrNewPixmapFromData flags*/
#define GR_BMDATA_BYTEREVERSE	01	/** byte-reverse bitmap data*/
#define GR_BMDATA_BYTESWAP		02	/** byte-swap bitmap data*/

#if 0 /* don't define unimp'd flags*/
/* Window property flags */
#define GR_WM_PROP_NORESIZE		0x04	/* don't let user resize window */
#define GR_WM_PROP_NOICONISE	0x08	/* don't let user iconise window */
#define GR_WM_PROP_NOWINMENU	0x10	/* don't display a window menu button */
#define GR_WM_PROP_NOROLLUP		0x20	/* don't let user roll window up */
#define GR_WM_PROP_ONTOP		0x200	/* try to keep window always on top */
#define GR_WM_PROP_STICKY		0x400	/* keep window after desktop change */
#define GR_WM_PROP_DND			0x2000	/* accept drag and drop icons */
#endif

/* Window properties*/
#define GR_WM_PROPS_NOBACKGROUND	0x00000001L /** Don't draw window background */
#define GR_WM_PROPS_NOFOCUS	 		0x00000002L /** Don't set focus to this window */
#define GR_WM_PROPS_NOMOVE	 		0x00000004L /** Don't let user move window */
#define GR_WM_PROPS_NORAISE			0x00000008L /** Don't let user raise window */
#define GR_WM_PROPS_NODECORATE		0x00000010L /** Don't redecorate window */
#define GR_WM_PROPS_NOAUTOMOVE		0x00000020L /** Don't move window on 1st map */
#define GR_WM_PROPS_NOAUTORESIZE	0x00000040L /** Don't resize window on 1st map */
/* Added by T.J.Park */
#define GR_WM_PROPS_TRANSPARENT		0x00000080L /** Make the window look transparent, i.e, not a rectangle. */

/* default decoration style*/
#define GR_WM_PROPS_APPWINDOW		0x00000000L	/** Leave appearance to WM */
#define GR_WM_PROPS_APPMASK			0xF0000000L	/** Appearance mask */
#define GR_WM_PROPS_BORDER			0x80000000L	/** Single line border */
#define GR_WM_PROPS_APPFRAME		0x40000000L	/** 3D app frame (overrides border) */
#define GR_WM_PROPS_CAPTION			0x20000000L /** Title bar */
#define GR_WM_PROPS_CLOSEBOX		0x10000000L /** Close box */
#define GR_WM_PROPS_MAXIMIZE		0x08000000L /** Application is maximized */

/* Flags for indicating valid bits in GrSetWMProperties call*/
#define GR_WM_FLAGS_PROPS			0x0001		/** Properties*/
#define GR_WM_FLAGS_TITLE			0x0002		/** Title*/
#define GR_WM_FLAGS_BACKGROUND		0x0004		/** Background color*/
#define GR_WM_FLAGS_BORDERSIZE		0x0008		/** Border size*/
#define GR_WM_FLAGS_BORDERCOLOR		0x0010		/** Border color*/
#define GR_WM_FLAGS_MODELESS			0x0020		/** test */
#define GR_WM_FLAGS_DRAWFOCUSEDLAST			0x0040		/** test */

/* NOTE: this struct must be hand-packed to a DWORD boundary for nxproto.h*/

/**
 * Window manager properties used by the GrGetWMProperties()/GrSetWMProperties() calls.
 */
typedef struct {
  GR_WM_PROPS flags;		/** Which properties valid in struct for set. Combination of GR_WM_FLAGS_PROPS, GR_WM_FLAGS_TITLE, GR_WM_FLAGS_BACKGROUND, GR_WM_FLAGS_BORDERSIZE, or GR_WM_FLAGS_BORDERCOLOR. */
  GR_WM_PROPS props;		/** Window property bits. Combination of GR_WM_PROPS values. */
  GR_CHAR *title;			/** Window title */
  GR_COLOR background;		/** Window background color */
  GR_SIZE bordersize;		/** Window border size */
  GR_COLOR bordercolor;		/** Window border color */
} GR_WM_PROPERTIES;

/**
 * Window properties returned by the GrGetWindowInfo() call.
 */
typedef struct {
  GR_WINDOW_ID wid;			/** window id (or 0 if no such window) */
  GR_WINDOW_ID parent;		/** parent window id */
  GR_WINDOW_ID child;		/** first child window id (or 0) */
  GR_WINDOW_ID sibling;		/** next sibling window id (or 0) */
  GR_BOOL inputonly;		/** TRUE if window is input only */
  GR_BOOL mapped;			/** TRUE if window is mapped */
  GR_BOOL realized;			/** TRUE if window is mapped and visible */
  GR_COORD x;				/** parent-relative x position of window */
  GR_COORD y;				/** parent-relative  y position of window */
  GR_SIZE width;			/** width of window */
  GR_SIZE height;			/** height of window */
  GR_SIZE bordersize;		/** size of border */
  GR_COLOR bordercolor;		/** color of border */
  GR_COLOR background;		/** background color */
  GR_EVENT_MASK eventmask;	/** current event mask for this client */
  GR_WM_PROPS props;		/** window properties */
  GR_CURSOR_ID cursor;		/** cursor id*/
  unsigned long processid;	/** process id of owner*/
} GR_WINDOW_INFO;

/**
 * Graphics context properties returned by the GrGetGCInfo() call.
 */
typedef struct {
  GR_GC_ID gcid;			/** GC id (or 0 if no such GC) */
  int mode;					/** drawing mode which was set by GrSetGCMode(). */
  int alpha;				/** global alpha value which was set by GrSetGCAlpha(). */
  GR_COLOR xorcolor;			/** xor color which was set by GrSetGCXorColor() added by foryou*/
  GR_REGION_ID region;		/** ID of the clipping region which was set GrSetGCRegion(). */
  int xoff;					/** x offset of the clipping region, which was set by GrSetGCClipOrigin(). */
  int yoff;					/** y offset of the clipping region, which was set by GrSetGCClipOrigin(). */
  GR_FONT_ID font;			/** ID of the font which was set by GrSetGCFont(). */
  GR_COLOR foreground;		/** foreground RGB color or pixel value which was set by GrSetGCForeground(). */
  GR_COLOR background;		/** background RGB color or pixel value which was set by GrSetGCBackground(). */
  GR_BOOL fgispixelval;		/** TRUE if 'foreground' is actually a GR_PIXELVAL, FALSE if 'foreground' is a GR_COLOR. See GrSetGCForegroundPixelVal(). */
  GR_BOOL bgispixelval;		/** TRUE if 'background' is actually a GR_PIXELVAL, FALSE if 'background' is a GR_COLOR. See GrSetGCBackgroundPixelVal(). */
  GR_BOOL usebackground;	/** TRUE if 'background' is used when drawing a text or image. FALSE to ignore 'background'. See GrSetGCUseBackground(). <p>
							 * <B>LGE-Specific</B> : In the original MicroWindows 0.90, usebackground flag have a bug when the foreground and background color are exactly same value,
							 * and the bug still exists. In the current implementation, usebackground is totally ignored on GrText(), so you don't need to call GrSetGCUseBackground().
							 * If you want to draw a background of a text, first get the extents of the text(GrGetGCTextSize), and then call GrFillRect() before calling GrText(). */

  GR_BOOL exposure;			/** TRUE if you want to receive exposure events on GrCopyArea(). FALSE otherwise. See GrSetGCGraphicsExposure(). */
} GR_GC_INFO;

/**
 * color palette
 */
typedef struct {
  GR_COUNT count;			/** number of valid entries */
  GR_PALENTRY palette[256];	/** palette */
} GR_PALETTE;

/** Calibration data passed to GrCalcTransform */
typedef struct {
  int xres;				/** X resolution of the screen */
  int yres;				/** Y resolution of the screen */
  int minx;				/** min raw X value */
  int miny;				/** min raw Y values */
  int maxx;				/** max raw X value */
  int maxy;				/** max raw Y value */
  GR_BOOL xswap;		/** true if the x component should be swapped */
  GR_BOOL yswap;		/** true if the y component should be swapped */
} GR_CAL_DATA;

/* Error codes */
#define	GR_ERROR_BAD_WINDOW_ID			1
#define	GR_ERROR_BAD_GC_ID				2
#define	GR_ERROR_BAD_CURSOR_SIZE		3
#define	GR_ERROR_MALLOC_FAILED			4
#define	GR_ERROR_BAD_WINDOW_SIZE		5
#define	GR_ERROR_KEYBOARD_ERROR			6
#define	GR_ERROR_MOUSE_ERROR			7
#define	GR_ERROR_INPUT_ONLY_WINDOW		8
#define	GR_ERROR_ILLEGAL_ON_ROOT_WINDOW	9
#define	GR_ERROR_TOO_MUCH_CLIPPING		10
#define	GR_ERROR_SCREEN_ERROR			11
#define	GR_ERROR_UNMAPPED_FOCUS_WINDOW	12
#define	GR_ERROR_BAD_DRAWING_MODE		13
#define GR_ERROR_BAD_LINE_ATTRIBUTE     14
#define GR_ERROR_BAD_FILL_MODE          15
#define GR_ERROR_BAD_REGION_ID			16

/* Event types.
 * Mouse motion is generated for every motion of the mouse, and is used to
 * track the entire history of the mouse (many events and lots of overhead).
 * Mouse position ignores the history of the motion, and only reports the
 * latest position of the mouse by only queuing the latest such event for
 * any single client (good for rubber-banding).
 */
#define	GR_EVENT_TYPE_ERROR				(-1)
#define	GR_EVENT_TYPE_NONE				0
#define	GR_EVENT_TYPE_EXPOSURE			1
#define	GR_EVENT_TYPE_BUTTON_DOWN		2
#define	GR_EVENT_TYPE_BUTTON_UP			3
#define	GR_EVENT_TYPE_MOUSE_ENTER		4
#define	GR_EVENT_TYPE_MOUSE_EXIT		5
#define	GR_EVENT_TYPE_MOUSE_MOTION		6
#define	GR_EVENT_TYPE_MOUSE_POSITION	7
#define	GR_EVENT_TYPE_KEY_DOWN			8
#define	GR_EVENT_TYPE_KEY_UP			9
#define	GR_EVENT_TYPE_FOCUS_IN			10
#define	GR_EVENT_TYPE_FOCUS_OUT			11
#define GR_EVENT_TYPE_FDINPUT			12
#define GR_EVENT_TYPE_UPDATE			13
#define GR_EVENT_TYPE_CHLD_UPDATE		14
#define GR_EVENT_TYPE_CLOSE_REQ			15
#define GR_EVENT_TYPE_TIMEOUT			16
#define GR_EVENT_TYPE_SCREENSAVER		17
#define GR_EVENT_TYPE_CLIENT_DATA_REQ	18
#define GR_EVENT_TYPE_CLIENT_DATA		19
#define GR_EVENT_TYPE_SELECTION_CHANGED 20
#define GR_EVENT_TYPE_TIMER             21
#define GR_EVENT_TYPE_PORTRAIT_CHANGED  22

/* Event masks */
#define	GR_EVENTMASK(n)					(((GR_EVENT_MASK) 1) << (n))

#define	GR_EVENT_MASK_NONE				GR_EVENTMASK(GR_EVENT_TYPE_NONE)
#define	GR_EVENT_MASK_ERROR				0x80000000L
#define	GR_EVENT_MASK_EXPOSURE			GR_EVENTMASK(GR_EVENT_TYPE_EXPOSURE)
#define	GR_EVENT_MASK_BUTTON_DOWN		GR_EVENTMASK(GR_EVENT_TYPE_BUTTON_DOWN)
#define	GR_EVENT_MASK_BUTTON_UP			GR_EVENTMASK(GR_EVENT_TYPE_BUTTON_UP)
#define	GR_EVENT_MASK_MOUSE_ENTER		GR_EVENTMASK(GR_EVENT_TYPE_MOUSE_ENTER)
#define	GR_EVENT_MASK_MOUSE_EXIT		GR_EVENTMASK(GR_EVENT_TYPE_MOUSE_EXIT)
#define	GR_EVENT_MASK_MOUSE_MOTION		GR_EVENTMASK(GR_EVENT_TYPE_MOUSE_MOTION)
#define	GR_EVENT_MASK_MOUSE_POSITION	GR_EVENTMASK(GR_EVENT_TYPE_MOUSE_POSITION)
#define	GR_EVENT_MASK_KEY_DOWN			GR_EVENTMASK(GR_EVENT_TYPE_KEY_DOWN)
#define	GR_EVENT_MASK_KEY_UP			GR_EVENTMASK(GR_EVENT_TYPE_KEY_UP)
#define	GR_EVENT_MASK_FOCUS_IN			GR_EVENTMASK(GR_EVENT_TYPE_FOCUS_IN)
#define	GR_EVENT_MASK_FOCUS_OUT			GR_EVENTMASK(GR_EVENT_TYPE_FOCUS_OUT)
#define	GR_EVENT_MASK_FDINPUT			GR_EVENTMASK(GR_EVENT_TYPE_FDINPUT)
#define	GR_EVENT_MASK_UPDATE			GR_EVENTMASK(GR_EVENT_TYPE_UPDATE)
#define	GR_EVENT_MASK_CHLD_UPDATE		GR_EVENTMASK(GR_EVENT_TYPE_CHLD_UPDATE)
#define	GR_EVENT_MASK_CLOSE_REQ			GR_EVENTMASK(GR_EVENT_TYPE_CLOSE_REQ)
#define	GR_EVENT_MASK_TIMEOUT			GR_EVENTMASK(GR_EVENT_TYPE_TIMEOUT)
#define GR_EVENT_MASK_SCREENSAVER		GR_EVENTMASK(GR_EVENT_TYPE_SCREENSAVER)
#define GR_EVENT_MASK_CLIENT_DATA_REQ	GR_EVENTMASK(GR_EVENT_TYPE_CLIENT_DATA_REQ)
#define GR_EVENT_MASK_CLIENT_DATA		GR_EVENTMASK(GR_EVENT_TYPE_CLIENT_DATA)
#define GR_EVENT_MASK_SELECTION_CHANGED GR_EVENTMASK(GR_EVENT_TYPE_SELECTION_CHANGED)
#define GR_EVENT_MASK_TIMER             GR_EVENTMASK(GR_EVENT_TYPE_TIMER)
#define GR_EVENT_MASK_PORTRAIT_CHANGED  GR_EVENTMASK(GR_EVENT_TYPE_PORTRAIT_CHANGED)
/* Event mask does not affect GR_EVENT_TYPE_HOTKEY_DOWN and
 * GR_EVENT_TYPE_HOTKEY_UP, hence no masks for those events. */

#define	GR_EVENT_MASK_ALL		((GR_EVENT_MASK) -1L)

/* update event types */
#define GR_UPDATE_MAP			1
#define GR_UPDATE_UNMAP			2
#define GR_UPDATE_MOVE			3
#define GR_UPDATE_SIZE			4
#define GR_UPDATE_UNMAPTEMP		5	/* unmap during window move/resize*/
#define GR_UPDATE_ACTIVATE		6	/* toplevel window [de]activate*/
#define GR_UPDATE_DESTROY		7
#define GR_UPDATE_REPARENT      8

/**
 * Event for errors detected by the server.
 * These events are not delivered to GrGetNextEvent, but instead call
 * the user supplied error handling function.  Only the first one of
 * these errors at a time is saved for delivery to the client since
 * there is not much to be done about errors anyway except complain
 * and exit.
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_FUNC_NAME name;		/** function name which failed */
  GR_ERROR code;			/** error code */
  GR_ID id;					/** resource id (maybe useless) */
} GR_EVENT_ERROR;

/**
 * Event for a mouse button pressed down or released.
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_WINDOW_ID wid;			/** window id event delivered to */
  GR_WINDOW_ID subwid;		/** sub-window id (pointer was in) */
  GR_COORD rootx;			/** root window x coordinate */
  GR_COORD rooty;			/** root window y coordinate */
  GR_COORD x;				/** window x coordinate of mouse */
  GR_COORD y;				/** window y coordinate of mouse */
  GR_BUTTON buttons;		/** current state of all buttons */
  GR_BUTTON changebuttons;	/** buttons which went down or up */
  GR_KEYMOD modifiers;		/** modifiers (MWKMOD_SHIFT, etc)*/
  GR_TIMEOUT time;			/** tickcount time value*/
} GR_EVENT_BUTTON;

/**
 * Event for a keystroke typed for the window with has focus.
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_WINDOW_ID wid;			/** window id event delived to */
  GR_WINDOW_ID subwid;		/** sub-window id (pointer was in) */
  GR_COORD rootx;			/** root window x coordinate */
  GR_COORD rooty;			/** root window y coordinate */
  GR_COORD x;				/** window x coordinate of mouse */
  GR_COORD y;				/** window y coordinate of mouse */
  GR_BUTTON buttons;		/** current state of buttons */
  GR_KEYMOD modifiers;		/** modifiers (MWKMOD_SHIFT, etc)*/
  GR_KEY ch;				/** 16-bit unicode key value, MWKEY_xxx */
  GR_SCANCODE scancode;		/** OEM scancode value if available*/
  GR_BOOL hotkey;			/** TRUE if generated from GrGrabKey(GR_GRAB_HOTKEY_x) */
} GR_EVENT_KEYSTROKE;

/**
 * Event for exposure for a region of a window.
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_WINDOW_ID wid;			/** window id */
  GR_COORD x;				/** window x coordinate of exposure */
  GR_COORD y;				/** window y coordinate of exposure */
  GR_SIZE width;			/** width of exposure */
  GR_SIZE height;			/** height of exposure */
} GR_EVENT_EXPOSURE;

/**
 * General events for focus in or focus out for a window, or mouse enter
 * or mouse exit from a window, or window unmapping or mapping, etc.
 * Server portrait mode changes are also sent using this event to
 * all windows that request it.
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_WINDOW_ID wid;			/** window id */
  GR_WINDOW_ID otherid;		/** new/old focus id for focus events*/
} GR_EVENT_GENERAL;

/**
 * Events for mouse motion or mouse position.
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_WINDOW_ID wid;			/** window id event delivered to */
  GR_WINDOW_ID subwid;		/** sub-window id (pointer was in) */
  GR_COORD rootx;			/** root window x coordinate */
  GR_COORD rooty;			/** root window y coordinate */
  GR_COORD x;				/** window x coordinate of mouse */
  GR_COORD y;				/** window y coordinate of mouse */
  GR_BUTTON buttons;		/** current state of buttons */
  GR_KEYMOD modifiers;		/** modifiers (MWKMOD_SHIFT, etc)*/
} GR_EVENT_MOUSE;

/**
 * GrRegisterInput() event.
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  int		fd;				/** input file descriptor*/
} GR_EVENT_FDINPUT;

/**
 * GR_EVENT_TYPE_UPDATE
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_WINDOW_ID wid;			/** select window id*/
  GR_WINDOW_ID subwid;		/** update window id (=wid for UPDATE event)*/
  GR_COORD x;				/** new window x coordinate */
  GR_COORD y;				/** new window y coordinate */
  GR_SIZE width;			/** new width */
  GR_SIZE height;			/** new height */
  GR_UPDATE_TYPE utype;		/** update_type */
} GR_EVENT_UPDATE;

/**
 * GR_EVENT_TYPE_SCREENSAVER
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_BOOL activate;			/** true = activate, false = deactivate */
} GR_EVENT_SCREENSAVER;

/**
 * GR_EVENT_TYPE_CLIENT_DATA_REQ
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_WINDOW_ID wid;			/** ID of requested window */
  GR_WINDOW_ID rid;			/** ID of window to send data to */
  GR_SERIALNO serial;		/** Serial number of transaction */
  GR_MIMETYPE mimetype;		/** Type to supply data as */
} GR_EVENT_CLIENT_DATA_REQ;

/**
 * GR_EVENT_TYPE_CLIENT_DATA
 */
typedef struct {
  GR_EVENT_TYPE type;		/** event type */
  GR_WINDOW_ID wid;			/** ID of window data is destined for */
  GR_WINDOW_ID rid;			/** ID of window data is from */
  GR_SERIALNO serial;		/** Serial number of transaction */
  unsigned long len;		/** Total length of data */
  unsigned long datalen;	/** Length of following data */
  void *data;				/** Pointer to data (filled in on client side) */
} GR_EVENT_CLIENT_DATA;

/**
 * GR_EVENT_TYPE_SELECTION_CHANGED
 */
typedef struct {
  GR_EVENT_TYPE type;		/**< event type */
  GR_WINDOW_ID new_owner;	/**< ID of new selection owner */
} GR_EVENT_SELECTION_CHANGED;

/**
 * GR_EVENT_TYPE_TIMER
 */
typedef struct {
  GR_EVENT_TYPE  type;		/**< event type, GR_EVENT_TYPE_TIMER */
  GR_WINDOW_ID   wid;		/**< ID of window timer is destined for */
  GR_TIMER_ID    tid;		/**< ID of expired timer */
} GR_EVENT_TIMER;

/**
 * Union of all possible event structures.
 * This is the structure returned by GrGetNextEvent() and similar routines.
 */
typedef union {
  GR_EVENT_TYPE type;			/**< event type */
  GR_EVENT_ERROR error;			/**< error event */
  GR_EVENT_GENERAL general;		/**< general window events */
  GR_EVENT_BUTTON button;		/**< button events */
  GR_EVENT_KEYSTROKE keystroke;		/**< keystroke events */
  GR_EVENT_EXPOSURE exposure;		/**< exposure events */
  GR_EVENT_MOUSE mouse;			/**< mouse motion events */
  GR_EVENT_FDINPUT fdinput;		/**< fd input events*/
  GR_EVENT_UPDATE update;		/**< window update events */
  GR_EVENT_SCREENSAVER screensaver; 	/**< Screen saver events */
  GR_EVENT_CLIENT_DATA_REQ clientdatareq; /**< Request for client data events */
  GR_EVENT_CLIENT_DATA clientdata;	  /**< Client data events */
  GR_EVENT_SELECTION_CHANGED selectionchanged; /**< Selection owner changed */
  GR_EVENT_TIMER timer;                 /**< Timer events */
} GR_EVENT;

typedef void (*GR_FNCALLBACKEVENT)(GR_EVENT *);

/* GR_BITMAP macros*/
/* size of GR_BITMAP image in words*/
#define	GR_BITMAP_SIZE(width, height)	MWIMAGE_SIZE(width, height)
#define	GR_BITMAPBITS			MWIMAGE_BITSPERIMAGE
#define	GR_BITVALUE(n)			MWIMAGE_BITVALUE(n)
#define	GR_FIRSTBIT			MWIMAGE_FIRSTBIT
#define	GR_NEXTBIT(m)			MWIMAGE_NEXTBIT(m)
#define	GR_TESTBIT(m)			MWIMAGE_TESTBIT(m)
#define	GR_SHIFTBIT(m)			MWIMAGE_SHIFTBIT(m)

/* GrGrabKey() types. */

/**
 * Key reservation type for GrGrabKey() - a key is reserved exclusively,
 * and hotkey events are sent regardless of focus.
 *
 * Hotkey events are sent to the client that reserved the key.  The window
 * ID passed to the GrGrabKey() call is passed as the source window.
 *
 * This type of reservation is useful for implementing a "main menu" key
 * or similar hotkeys.
 *
 * This can be used to implement the MHP method
 * org.dvb.event.EventManager.addUserEventListener(listener,client,events).
 *
 * @see GrGrabKey()
 * @see GrUngrabKey()
 */
#define GR_GRAB_HOTKEY_EXCLUSIVE        0

/**
 * Key reservation type for GrGrabKey() - hotkey events are sent when a key
 * is pressed, regardless of focus.  This is not an exclusive reservation,
 * so the app that has the focus will get a normal key event.
 *
 * Hotkey events are sent to the client that reserved the key.  The window
 * ID passed to the GrGrabKey() call is passed as the source window.
 *
 * Note that because this is not an exclusive grab, it does not stop
 * other applications from grabbing the same key (using #GR_GRAB_HOTKEY
 * or any other grab mode).  If an application has an exclusive grab on
 * a key, then any grabs of type #GR_GRAB_HOTKEY will be ignored when
 * dispatching that key event.
 *
 * This can be used to implement the MHP method
 * org.dvb.event.EventManager.addUserEventListener(listener,events).
 *
 * @see GrGrabKey()
 * @see GrUngrabKey()
 */
#define GR_GRAB_HOTKEY                  1

/**
 * Key reservation type for GrGrabKey() - a key is reserved exclusively,
 * and normal key events are sent if the specified window has focus.
 *
 * This stops other applications from getting events on the specified key.
 *
 * For example, an application could use this to reserve the number
 * keys before asking the user for a PIN, to prevent other applications
 * stealing the PIN using #GR_GRAB_TYPE_HOTKEY.  (Note that this assumes
 * the applications are running in a controlled environment, such as
 * Java, so they can only interact with the platform in limited ways).
 *
 * This can be used to implement the MHP method
 * org.dvb.event.EventManager.addExclusiveAccessToAWTEvent(client,events).
 *
 * @see GrGrabKey()
 * @see GrUngrabKey()
 */
#define GR_GRAB_EXCLUSIVE               2

/**
 * Key reservation type for GrGrabKey() - a key is reserved exclusively,
 * and normal key events are sent if the specified window has focus,
 * or the mouse pointer is over the window.
 *
 * This stops other applications from getting events on the specified key.
 *
 * This is for compatibility with the first GrGrabKey() API, which only
 * supported this kind of reservation.
 *
 * @see GrGrabKey()
 * @see GrUngrabKey()
 */
#define GR_GRAB_EXCLUSIVE_MOUSE       3

/**
 * Highest legal value of any GR_GRAB_xxx constant.  (Lowest legal value
 * must be 0).
 *
 * @internal
 */
#define GR_GRAB_MAX                     GR_GRAB_EXCLUSIVE_MOUSE

/* GrGetSysColor colors*/
/* desktop background*/
#define GR_COLOR_DESKTOP           0

/* caption colors*/
#define GR_COLOR_ACTIVECAPTION     1
#define GR_COLOR_ACTIVECAPTIONTEXT 2
#define GR_COLOR_INACTIVECAPTION   3
#define GR_COLOR_INACTIVECAPTIONTEXT 4

/* 3d border shades*/
#define GR_COLOR_WINDOWFRAME       5
#define GR_COLOR_BTNSHADOW         6
#define GR_COLOR_3DLIGHT           7
#define GR_COLOR_BTNHIGHLIGHT      8

/* top level application window backgrounds/text*/
#define GR_COLOR_APPWINDOW         9
#define GR_COLOR_APPTEXT           10

/* button control backgrounds/text (usually same as app window colors)*/
#define GR_COLOR_BTNFACE           11
#define GR_COLOR_BTNTEXT           12

/* edit/listbox control backgrounds/text, selected highlights*/
#define GR_COLOR_WINDOW            13
#define GR_COLOR_WINDOWTEXT        14
#define GR_COLOR_HIGHLIGHT         15
#define GR_COLOR_HIGHLIGHTTEXT     16
#define GR_COLOR_GRAYTEXT          17

/* menu backgrounds/text*/
#define GR_COLOR_MENUTEXT          18
#define GR_COLOR_MENU              19

/**
 * Error strings per error number
 */
#define GR_ERROR_STRINGS		\
	"",				\
	"Bad window id: %d\n",		\
	"Bad graphics context: %d\n",	\
	"Bad cursor size\n",		\
	"Out of server memory\n",	\
	"Bad window size: %d\n",	\
	"Keyboard error\n",		\
	"Mouse error\n",		\
	"Input only window: %d\n",	\
	"Illegal on root window: %d\n",	\
	"Clipping overflow\n",		\
	"Screen error\n",		\
	"Unmapped focus window: %d\n",	\
	"Bad drawing mode gc: %d\n",    \
        "Bad line attribute gc: %d\n",  \
        "Bad fill mode gc: %d\n",       \
	"Bad region id: %d\n",

extern char *nxErrorStrings[];

//Added by Henry @20080401
int GrAll2GBK(const void *istr, void *ostr);

/******************************************************************************/
/************************* Public graphics routines. **************************/
/******************************************************************************/


/**
 * Clear the event buffer. This function is only used in client-server model.
 *
 * This function will do nothing when NONETWORK is defined,
 *
 * <B>LGE-Specific</B> : We don't use this function.
 */
void	GrFlush(void);

/**
 * In client-server model, this function connects the client to the Nano-X server. <p>
 * In non clinent-server model(when NONETWORK is defined), this function opens the screen device and creates the root window. <p>
 *
 *
 * <B>LGE-Specific</B> : This function is called only once when the system starts. <p>
 * If multiple screen devices are supported, this function opens all the available screen devices and creates multiple
 * root windows, each for the screen device.
 * See MAX_OSD_LAYERS defined in device.h, and if you need multiple screen devices, change the MAX_OSD_LAYERS, and
 * implement each screen device drivers of scrdev[] array.
 */
int		GrOpen(void);
void	GrSetNumOfLayers(int numOfLayers);	// for ACAP

/**
 * <B>LGE-Specific</B> : GrBeginPaint() and GrEndPaint() are not defined in the original MicroWindows 0.90.
 *
 *
 * The purpose of these functions are to support more efficient and correct synchronization of windows
 * , especially when the windows are created in various tasks(threads).
 *
 *
 * These functions are similiar to GetDC() or ReleaseDC() of Win32( or uglBatchStart() and uglBatchEnd() of WindML)
 * , and support a critical section between multiple graphic primitives call.
 *
 *
 * GrBeginPaint() opens the critical section for a series of graphics primitives call.
 */
void GrBeginPaint(void);

/**
 * <B>LGE-Specific</B> : This function closes the critical section which was opened by GrBeginPaint().
 *
 *
 * For more information, refer to GrBeginPaint().
 */
void GrEndPaint(void);

/**
 * <B>LGE-Specific</B> : This function opens the critical section.
 */
void GrServerLock(void);

/**
 * <B>LGE-Specific</B> : This function closes the critical section.
 */
void GrServerUnlock(void);

/**
 * <B>LGE-Specific</B> : GrCopyOffscreen() and GrCopyOffscreenEx() support system-level back-buffer.
 * These functions are not defined in the original MicroWindows 0.90.
 *
 *
 * The system-level back-buffer(off-screen) is an image buffer whose format and resolution are exactly same
 * as the target screen. Through this feature, the application does not need to make a back buffer for each window
 * in order to remove flickering. System-level back buffer makes the programming easier.
 *
 *
 * For this feature, a graphic driver function, Update() is newly added in mwscreendevice structure.
 * If the system does not need the system-level back-buffer, you don't need to implement the Update() function in your scrdev.
 * If Update() is not defined in the given screen device, this function will do nothing.
 *
 *
 * GrCopyOffscreen() and GrCopyOffscreenEx() require a region ID(not a simple rectangle), so you can blit any shape of area of
 * off-screen(back-buffer)'s image into the screen.
 *
 *
 * GrCopyOffscreen() copies the off-screen of the first screen device into the screen.
 * If you have multiple screen devices, call GrCopyOffScreenEx() instead of GrCopyOffscreen().
 * Note that the following two function calls make the exactly same result.
 * <PRE>
 * GrCopyOffscreen(region);
 * GrCopyOffscreenEx(0, region);
 * </PRE>
 *
 *
 * Also note that the given region object will be automatically reset in this function call
 * (i.e, after this function call, the region object will be 'empty')
 * , so you don't need to clear the region after this function call.
 *
 * @param rid [IN] the ID of the region indicating the area to be copied from off-screen to screen.
 */
void GrCopyOffscreen(GR_REGION_ID rid);

/**
 * Cache font.
 * Refer to UI_TOOLKIT_CacheTextSize()
 */
void GrCacheFont(GR_GC_ID gc, void *str, int count);

/**
 * <B>LGE-Specific</B> : An advanced version of GrCopyOffscreen() to support multiple screen devices.
 *
 * For more information, refer to GrCopyOffscreen().
 *
 * You can implement and use multiple screens, even when there is only one 'physical' screen device available.
 *
 * The only thing necessary is the blit operation between memory blocks(which is called generally as 'surfaces').
 * The blit operation must be able to do alpha-blending and stretching.
 *
 * The resolutions of the screen devices may be different, but the pixel formats must be same.
 *
 * @param layer [IN] 0 for the first OSD, 1 for the second OSD, etc. This value must be smaller than MAX_OSD_LAYER
 * @param rid [IN] the ID of the regin indicating the area to be copied from off-screen to screen.
 */
void GrCopyOffscreenEx(int layer, GR_REGION_ID rid);

/**
 * Closes the screen device(s).
 *
 * Note that this function does not free(deallocate) all the resources allocated by Nano-X.
 */
void GrClose(void);

/**
 * Suspends the calling thread or process during the given milliseconds.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 *
 * @param msecs the interval during which the calling thread or process will be suspended.
 */
void GrDelay(GR_TIMEOUT msecs);

/**
 * Retrieves the information of the first screen devices.
 * If you have multiple screen devices, use GrGetScreenInfoEx() instead of this function.
 *
 * <B>T.J.Park Note</B> : Not all the values are correct. At this time, only rows, cols, and bpp have the exact values.
 *
 * pixtype will also be an useful value, but the definitions do not cover all the possible pixel formats. <p>
 * (for example, 16 bpp W_RGB(1:5:5:5) format is not defined in mwtypes.h).
 *
 * In the current implementation, pixtype is also 0.
 *
 * All other values are undefined or meaningless.
 *
 * @param sip [OUT] the pointer of the GR_SCREEN_INFO where the return values are saved.
 */
void GrGetScreenInfo(GR_SCREEN_INFO *sip);

/**
 * <B>LGE-Specific</B> : This function is not defined in the original MicroWindows 0.90.
 *
 * This function is added in order to support multiple screen devices. <p>
 * About multiple screen devices, refer to GrOpen(), GrCopyOffscreen(), and GrCopyOffscreenEx().
 *
 * @param layer [IN] 0 for the first OSD, 1 for the second OSD, etc. This value must be smaller than MAX_OSD_LAYER.
 */
void
GrGetScreenInfoEx(int layer, GR_SCREEN_INFO *sip);

/**
 * <B>LGE-Specific</B> : This function is not defined in the original MicroWindows 0.90.
 *
 * This function is added in order to support changing of the screen width/height at run time.
 * If you use multiple screen devices, use GrSetScreenResolutionEx instead.
 *
 * This function changes the width/height of the OSD layer 0 to the specified width/height.
 * The resolution of the root window is also changed by this function.
 *
 * @param width [IN] the new width of the screen 0.
 * @param height [IN] the new height of the screen 0.
 */
void
GrSetScreenResolution(int width, int height);

/**
 * <B>LGE-Specific</B> : This function is not defined in the original MicroWindows 0.90.
 *
 * This function is added in order to support changing of the screen width/height at run time.
 *
 * This function changes the width/height of the specified OSD layer to the specified width/height.
 * The resolution of the root window is also changed by this function.
 *
 * @param layer [IN] 0 for the first OSD, 1 for the second OSD, etc. This value must be smaller than MAX_OSD_LAYER.
 * @param width [IN] the new width of the screen[layer].
 * @param height [IN] the new height of the screen[layer].
 */
void
GrSetScreenResolutionEx(int layer, int width, int height);

/**
 * This function retrieves the system-defined color of the given palette index.
 * The system-defined colors are defined and used for a windows' common shape, for example, the title-bar color
 * or the window border color.
 *
 *
 * <B>LGE-Specific</B> : All windows are created without title-bar and border, so this function is not tested at all.
 *
 *
 * @param index [IN] One of the following values. <p>
 * - GR_COLOR_DESKTOP <p>
 * - GR_COLOR_ACTIVECAPTION <p>
 * - GR_COLOR_ACTIVECAPTIONTEXT <p>
 * - GR_COLOR_INACTIVECAPTION <p>
 * - GR_COLOR_INACTIVECAPTIONTEXT <p>
 * - GR_COLOR_WINDOWFRAME <p>
 * - GR_COLOR_BTNSHADOW <p>
 * - GR_COLOR_3DLIGHT <p>
 * - GR_COLOR_BTNHIGHLIGHT <p>
 * - GR_COLOR_APPWINDOW <p>
 * - GR_COLOR_APPTEXT <p>
 * - GR_COLOR_BTNFACE <p>
 * - GR_COLOR_BTNTEXT <p>
 * - GR_COLOR_WINDOW <p>
 * - GR_COLOR_WINDOWTEXT <p>
 * - GR_COLOR_HIGHLIGHT <p>
 * - GR_COLOR_HIGHLIGHTTEXT <p>
 * - GR_COLOR_GRAYTEXT <p>
 * - GR_COLOR_MENUTEXT <p>
 * - GR_COLOR_MENU <p>
 * @return The color of the given index.
 */
GR_COLOR GrGetSysColor(int index);

/**
 * This function creates a new window.
 *
 *
 * <B>LGE-Specific</B> : The bordersize shall be always 0, and background/border color shall be ignored.
 * In order to ignore the background color, GrNewWindowEx() may be more useful, though you can use
 * GrNewWindow() and GrSetWMProperties().
 *
 *
 * If you have multiple screen devices, multiple root windows are also available. <p>
 * For example, if you have two screen devices, you can create a window under GR_ROOT_WINDOW2_ID
 * , not only under GR_ROOT_WINDOW_ID. <p>
 * If you have three screen devices, define more GR_ROOT_WINDOWXX_ID in nano-X.h
 *
 *
 * In order to support multiple root windows, the ID of a normal window is allocated from 20, not 2.
 * This means that you can define up to 19 root windows.
 *
 *
 * @param parent [IN] the ID of the parent window. If the parent is the root window, use GR_ROOT_WINDOW_ID(or GR_ROOT_WINDOW2_ID).
 * @param x [IN] the x-coordinate of the window's left-top position, described by the parent window's coordinates.
 * @param y [IN] the y-coordinate of the window's left-top position, described by the parent window's coordinates.
 * @param width [IN] the width of the window.
 * @param height [IN] the height of the window.
 * @param bordersize [IN] the width of the border. (<B>LGE-Specific</B> : This value must be 0).
 * @param background [IN] the background color. If GR_WM_PROPS_NOBACKGROUND is set for this window, this color will be ignored.
 * @param bordercolor [IN] the border color. If bordersize is 0, this value will be ignored.
 * @return the ID of the window. (<B>LGE-Specific</B> : This value is always bigger than 20).
 */
GR_WINDOW_ID GrNewWindow(GR_WINDOW_ID parent, GR_COORD x, GR_COORD y,
			GR_SIZE width, GR_SIZE height, GR_SIZE bordersize,
			GR_COLOR background, GR_COLOR bordercolor);

/**
 * This function creates a new pixmap. Note that a pixmap is a graphics object("Drawable" in the term of X-window).
 * You can call any graphics primitives for a pixmap, because pixmap is exactly same as the window from the point of
 * graphics primitives's view.
 * Using pixmap, you can create a back-buffer for each window, and copy the pixmap's image into a window, and vice versa.
 * The memory of a pixmap is allocated by the graphics driver, if the last parameter 'pixels' is NULL.
 * If 'pixels' is not NULL, the 'pixels' is used for the pixmap's image buffer.
 *
 * Destroying of a pixmap is exactly same as the window. Just call GrDestroyWindow().
 *
 * @param width [IN] the width of the pixmap.
 * @param height [IN] the height of the pixmap.
 * @param pixels [IN] the pointer of the pixmap image. If this is NULL, the pixmap's image buffer is allocated by the graphics driver.
 * Note that the pixel type of this parameter must be same as the target screen device. No format conversion is done in this function.
 */
GR_WINDOW_ID GrNewPixmap(GR_SIZE width, GR_SIZE height, void *pixels);

/**
 * An advanced version of GrNewPixmap to support multiple screen devices.
 *
 * <B>LGE-Specific</B> : The original MicroWindows 0.90 does not include this function. This function is added in order to support multiple
 * screen devices. Refer to GrOpen(), GrCopyOffscreen(), and GrCopyOffscreenEx() for more information about multiple screen devices.
 *
 * @param width [IN] the width of the pixmap
 * @param height [IN] the height of the pixmap
 * @pixels [IN] the pointer of the pixmap image. If this parameter is NULL, the pixmap's image buffer is allocated by the graphics driver.
 * Note that the pixel type of this parameter must be same as the target screen device. No conversion routine is included in this function.
 * @param layer [IN] 0 for the first OSD, 1 for the second OSD, etc. This value must be smaller than MAX_OSD_LAYER
 */
GR_WINDOW_ID GrNewPixmapEx(GR_SIZE width, GR_SIZE height, void *pixels, int layer);

/**
 * Allocate a new input-only window which is a child of the specified window.
 * Such a window is invisible, cannot be drawn into, and is only used to
 * return events.  The window inherits the cursor of the parent window.
 * The window is owned by the current client.
 *
 * <B>LGE-Specific</B> : This function is not used at all.
 *
 * @param parent [IN] the ID of the parent window.
 * @param x [IN] the x-coordinates of the window, described in the parent window's domain.
 * @param y [IN] the y-coordinates of the window, described in the parent window's domain.
 * @param width [IN] the width of the window.
 * @param height [IN] the height of the window.
 */
GR_WINDOW_ID GrNewInputWindow(GR_WINDOW_ID parent, GR_COORD x, GR_COORD y,
				GR_SIZE width, GR_SIZE height);

/**
 * Destroys a window or a pixmap.
 *
 * @param wid [IN] the ID of the "Drawable" object to be destroyed.
 */
void GrDestroyWindow(GR_WINDOW_ID wid);

/**
 * Creats a new graphics context.
 *
 * @return the ID of the created GC.
 */
GR_GC_ID GrNewGC(void);

/**
 * Duplicates the given graphics context.
 *
 * @param gc [IN] the ID of the GC to be copied from.
 * @return the ID of the GC created from the given GC. The contents of the newly created GC is same as the original one.
 */
GR_GC_ID GrCopyGC(GR_GC_ID gc);

/**
 * Retrieves the information of a graphics context.
 *
 * @param gc [IN] the ID of the GC.
 * @param gcip [IN] the pointer of GR_GC_INFO where the returned value will be saved in.
 */
void		GrGetGCInfo(GR_GC_ID gc, GR_GC_INFO *gcip);

/**
 * Destroys the GC object.
 *
 * @param gc [IN] the ID of the GC.
 */
void		GrDestroyGC(GR_GC_ID gc);

/**
 * Creates a new empty region.
 */
GR_REGION_ID	GrNewRegion(void);

/**
 * Creates a new region from the given 'monochrome' image.
 *
 * In order to use this function, you must define DYNAMICREGIONS as 1 in device.h.
 * If DYNAMICREGIONS is 0, this function will do nothing.
 *
 *
 * <B>T.J.Park Note</B> : The linelength of the given monochrome image is byte-aligned.
 *                 This function is not tested in our project.
 *
 *
 * @param bitmap [IN] the pointer of monochrome image
 * @param width [IN] the width of the monochrome image
 * @param height [IN] the height of the monochrome image
 */
GR_REGION_ID	GrNewBitmapRegion(GR_BITMAP *bitmap, GR_SIZE width,
			GR_SIZE height);

/**
 * Creates a new region from the given polygon(an array of points).
 * In order to use this function, you must define POLYREGIONS as 1 in device.h.
 * If POLYREGIONS is 0, this function will do nothing.
 *
 *
 * <B>T.J.Park Note</B> : This function is not tested in our project.
 *                 Regarding the first parameter 'mode', I don't know what it means.
 *                 There is no description about the 'mode' in the source code.
 *                 It seems that it is related to the way how the each scan line is converted to a region.
 *
 *
 * @param mode [IN] One of GR_POLY_EVENODD or GR_POLY_WINDING.
 * @param count [IN] the number of the points.
 * @param points [IN] the pointer of the points array.
 */
GR_REGION_ID	GrNewPolygonRegion(int mode, GR_COUNT count, GR_POINT *points);

/**
 * Destroys a region.
 *
 * @param region [IN] the ID of the region.
 */
void		GrDestroyRegion(GR_REGION_ID region);

/**
 * Union a rectangle with the given region.
 *
 * @param region [IN] the ID of the region.
 * @param rect [IN] the pointer of the rectangle to be added into the region.
 *                  Note that this parameter can not be NULL. There is no validity check about it.
 */
void		GrUnionRectWithRegion(GR_REGION_ID region, GR_RECT *rect);

/**
 * Union two region, and save the result in another region.
 *
 * @param dst_rgn [OUT] the ID of the destination region. This is the result.
 * @param src_rgn1 [IN] the ID of the first source region.
 * @param src_rgn2 [IN] the ID of the second source region.
 */
void		GrUnionRegion(GR_REGION_ID dst_rgn, GR_REGION_ID src_rgn1,
			GR_REGION_ID src_rgn2);

/**
 * Intersect two region, and save the result in another region.
 *
 * @param dst_rgn [OUT] the ID of the destination region. This is the result.
 * @param src_rgn1 [IN] the ID of the first source region.
 * @param src_rgn2 [IN] the ID of the second source region.
 */
void		GrIntersectRegion(GR_REGION_ID dst_rgn, GR_REGION_ID src_rgn1,
			GR_REGION_ID src_rgn2);
/**
 * Subtract src_rgn2 from src_rgn1, and save the result in dst_rgn.
 *
 * @param dst_rgn [OUT] the ID of the destination region. This is the result.
 * @param src_rgn1 [IN] the ID of the first source region. This is the minuend.
 * @param src_rgn2 [IN] the ID of the second source region. This is the subtrahend.
 */
void		GrSubtractRegion(GR_REGION_ID dst_rgn, GR_REGION_ID src_rgn1,
			GR_REGION_ID src_rgn2);

/**
 * Do XOR-operation with two source regions, and save the result in another region.
 *
 * @param dst_rgn [OUT] the ID of the destination region. This is the result.
 * @param src_rgn1 [IN] the ID of the first source region.
 * @param src_rgn2 [IN] the ID of the second source region.
 */
void		GrXorRegion(GR_REGION_ID dst_rgn, GR_REGION_ID src_rgn1,
			GR_REGION_ID src_rgn2);

/**
 * Set the given region into the given graphics context, in other words, set a clipping region.
 * All graphics primitives using this GC shall be clipped after this function call.
 *
 * @param gc [IN] the ID of the GC.
 * @param region [IN] the ID of the clipping region.
 */
void		GrSetGCRegion(GR_GC_ID gc, GR_REGION_ID region);

/**
 * Set the x/y offset of the user defined clip region. The clip region(if it is set to the GC),
 * will be used as if it is moved by (x,y). For example, if you set a clip rectangle's left-top corner at (10, 10)
 * when the clip origin is (20, 20), then it is the same result as you set a clip rectangle's left-top corner at (30, 30)
 * when the clip origin is (0, 0).
 *
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-offset of the clip region's origin.
 * @param y [IN] the y-offset of the clip region's origin.
 */
void		GrSetGCClipOrigin(GR_GC_ID gc, int x, int y);

/**
 * Checks whether the given position exists in a region.
 *
 * @param region [IN] the ID of the region.
 * @param x [IN] the x-coordinate of the point.
 * @param y [IN] the y-coordinate of the point.
 * @return GR_TRUE if the point exists in the region. GR_FALSE otherwise.
 */
GR_BOOL		GrPointInRegion(GR_REGION_ID region, GR_COORD x, GR_COORD y);

/**
 * Checks whether the given rectangle exists in a region.
 *
 * @param region [IN] the ID of the region.
 * @param x [IN] the x-coordinate of the left-top corner of the rectangle.
 * @param y [IN] the y-coordinate of the left-top corner of the rectangle.
 * @param w [IN] the width of the rectangle.
 * @param h [IN] the height of the rectangle.
 * @return One of the followings.
 * <TABLE>
 * MWRECT_OUT		rectangle not in region
 * MWRECT_ALLIN		rectangle all in region
 * MWRECT_PARTIN	rectangle partly in region
 * </TABLE>
 */
int		GrRectInRegion(GR_REGION_ID region, GR_COORD x, GR_COORD y,
			GR_COORD w, GR_COORD h);

/**
 * Checks whether the given region is empty.
 *
 * @param region [IN] the ID of the region.
 * @return GR_TRUE if the region is empty. GR_FALSE otherwise.
 */
GR_BOOL		GrEmptyRegion(GR_REGION_ID region);

/**
 * Checks whether the given two regions are exactly same.
 *
 * @param rgn1 [IN] the ID of the first region.
 * @param rgn2 [IN] the ID of the second region.
 * @return GR_TRUE if the two regions are same. GR_FALSE otherwise.
 */
GR_BOOL		GrEqualRegion(GR_REGION_ID rgn1, GR_REGION_ID rgn2);

/**
 * Moves a region by the given offset.
 *
 * @param region [IN] the ID of the region.
 * @param dx [IN] the x-offset by which the region is moved horizontally.
 * @param dy [IN] the y-offset by which the region is moved vertically.
 */
void		GrOffsetRegion(GR_REGION_ID region, GR_SIZE dx, GR_SIZE dy);

/**
 * Retrieves the bounding box of a region. The returned rect must cover the whole region, and is the smallest.
 *
 * @param region [IN] the ID of the region.
 * @param rect [OUT] the pointer of rectangle where the returned rectangle will be saved in. This can not be NULL.
 * @return The type of the region. One of the followings.
 * <TABLE>
 * MWREGION_NULL		empty region.
 * MWREGION_SIMPLE		rectangular region.
 * MWREGION_COMPLEX		non-rectangular region.
 * </TABLE>
 */
int		GrGetRegionBox(GR_REGION_ID region, GR_RECT *rect);

/**
 * Maps a window into the window tree, i.e, makes a window(and its children) visible.
 *
 * @param wid [IN] the ID of the window. If this value is a pixmap ID, this function will do nothing.
 */
void		GrMapWindow(GR_WINDOW_ID wid);

/**
 * Unmaps a window from the window tree, i.e, makes a window(and its children) invisible.
 *
 * @param wid [IN] the ID of the window. If this value is a pixmap ID, this function will do nothing.
 */
void		GrUnmapWindow(GR_WINDOW_ID wid);

/**
 * Raise a window to the highest level among its siblings.
 *
 *
 * <B>T.J.Park Note</B> : MicroWindows does not support a function to raise or lower one level.
 *                 It may be necessary.
 *
 *
 * @param wid [IN] the ID of the window.
 */
void		GrRaiseWindow(GR_WINDOW_ID wid);

/**
 * Lower a window to the lowest level among its siblings.
 *
 *
 * <B>T.J.Park Note</B> : MicroWindows does not support a function to raise or lower one level.
 *                 It may be necessary.
 *
 *
 * @param wid [IN] the ID of the window.
 */
void		GrLowerWindow(GR_WINDOW_ID wid);

/**
 * Moves a window to the specified position.
 *
 * @param wid [IN] the ID of the window.
 * @param x [IN] the x-coordinate of the position where the window moves to. This value is specified in the parent window's coorinates.
 * @param y [IN] the y-coordinate of the position where the window moves to. This value is specified in the parent window's coorinates.
 */
void		GrMoveWindow(GR_WINDOW_ID wid, GR_COORD x, GR_COORD y);

/**
 * Changes the width or height of a window.
 *
 * @param wid [IN] the ID of the window.
 * @param width [IN] the new width
 * @param height [IN] the new height
 */
void		GrResizeWindow(GR_WINDOW_ID wid, GR_SIZE width, GR_SIZE height);

/**
 * Changes the parent of a window.
 *
 * @param wid [IN] the ID of the window.
 * @param pwid [IN] the ID of the new parent window.
 * @param x [IN] the x-coordinate of the window in the new parent window's coordinate.
 * @param y [IN] the y-coordinate of the window in the new parent window's coordinate.
 */
void		GrReparentWindow(GR_WINDOW_ID wid, GR_WINDOW_ID pwid,
			GR_COORD x, GR_COORD y);

GR_WINDOW_ID GrFindVisibleWindow(GR_COORD x, GR_COORD y); // kwonyouk

/**
 * Retrieves the information of a window.
 *
 * @param wid [IN] the ID of the window
 * @param infoptr [OUT] the pointer of GR_WINDOW_INFO where the returned value will be saved in. This can not be NULL.
 */
void		GrGetWindowInfo(GR_WINDOW_ID wid, GR_WINDOW_INFO *infoptr);

/**
 * Set the properties of a window.
 *
 * <B>LGE-Specific</B> : GR_WM_PROPS_TRANSPARENT is newly added in the window properties.
 *
 * The original MicroWindows 0.90 does not support a 'transparent' window.
 *
 * If a window is 'transparent', then windows below it at the position where the original window was
 * initially placed are not obscured and show through, i.e, a transparent window is not
 * clipped off when its siblings and its parent are repainted. Instead, an exposure event is sent to the calling client.
 * Through this way, a transparent and even translucent window(through GR_MODE_SRC_OVER) can be implemented.
 * When using a 'transparent' window, the client must handle the exposure event, and the screen device driver must
 * support the system level back-buffer in order to avoid flickering.
 *
 * @param wid [IN] the ID of the window
 * @param props [IN] the pointer of GR_WM_PROPERTIES. This can not be NULL.
 */
void		GrSetWMProperties(GR_WINDOW_ID wid, GR_WM_PROPERTIES *props);

/**
 * Get the properties of a window.
 *
 * @param wid [IN] the ID of the window
 * @param props [OUT] the pointer of GR_WM_PROPERTIES where the returned value will be saved to. This can not be NULL.
 */
void		GrGetWMProperties(GR_WINDOW_ID wid, GR_WM_PROPERTIES *props);

/**
 * Creates a font from the given parameters.
 *
 * You can create a font in two ways, <p>
 * - First, you can set 'plogfont' to NULL. Instead, the name and the height parameter will be used to create a font.
 *   In this case, all the font options other than height and font name are set to default.
 * - Second, you can also use 'plogfont' to describe the font more precisely. For example, you can specify the width different
 *   from the height, choose whether the font is grey-scaled or monochrome, or even choose whether the kerning is used.
 *
 * <B>LGE-Specific</B> : In the current implementation, FONTMAPPER is not used. Instead, we support FONT_INSTALLER
 * and another function for installing a font data into the system.
 *
 * See GrInstallFont() and GrUninstallFont() for more information.
 *
 * The name of the font specified by the first parameter of GrCreateFont() or the name field of GR_LOGFONT structure
 * , may be one of the followings.
 *
 * - name of a font file if the system supports a file system(i.e, if HAVE_FILEIO is enabled).
 * - a font face name if the system supports a font installer.
 *
 * The typical way of font creation in this LGE version is as followings.
 *
 * <PRE>
 * // First, you must install a font into the system. Here, pFontData and nFontData contains the font data of 'Times New Roman'.
 * // You can set any font name in the first parameter, 'font name', regardless of what the actual font data is.
 * // Note the font name is the key when the system searches a font data.
 * GrInstallFont("Times New Roman", pFontData, nFontDataSize);
 * fontId1 = GrCreateFont("Times New Roman", 30, NULL); // Create a font whose height is 30, with the installed font data.
 * fontId2 = GrCreateFont("Times New Roman", 20, NULL); // Create another font whose height is 20, with the same font data.
 *
 * GrSetGCFont(gc, fontId1); // Set the font 1
 * GrText(...); // Draw a text
 * GrSetGCFont(gc, fontId2); // Set the font 2
 * GrText(...); // Draw a text
 *
 * GrDestroyFont(fontId1);
 * GrDestroyFont(fontId2);
 * GrUninstallFont("Times New Roman");
 * // Actually you don't need to call GrDestroyFont() before calling GrUninstallFont(),
 * // because GrUninstallFont() automatically destroys all font objects based on the font data.
 * </PRE>
 *
 * When searching a font data, the given font name is first considered as a installed font name. If the function doesn't find
 * any font data with that name, then, it tries to open a file with that name(must be described in full path).
 *
 * @param name [IN] Ignored if plogfont is not NULL. This string can be either a file name or an installed font name.
 * @param height [IN] Ignored if plogfont is not NULL. This value is the height of the font, specified by pixel count.
 * @param plogfont [IN] If this value is NULL, you must set appropriate values to 'name' and 'height' parameters. See GR_LOGFONT for more information.
 */
GR_FONT_ID	GrCreateFont(GR_CHAR *name, GR_COORD height,
			GR_LOGFONT *plogfont);

/**
 * <B>LGE-Specific</B> : We don't use FONTMAPPER. Instead we use FONT_INSTALLER.
 *
 * GrInstallFont and GrUninstallFont support installing a font, and are defined only when FONT_INSTALLER is enabled.
 * After installing a font, you can create a font using the installed font data. This method is useful because <p>
 * - this method works, even if your system does not support a file system. The font data may be included in compile time,
 * , or may be gathered from a network in run time. Of course, you can load the font data from a file system with your own method. <p>
 * - this method is efficient. There is only one copy of the font data in the system memory, even if you create multiple
 * font objects using the font data. <p>
 *
 *
 * Note that this function does not allocate another buffer, nor copy the given font data into another buffer.
 * This means you must not free nor corrupt the font data until you uninstall the font, and must free the font data
 * after uninstalling it.
 *
 * @param name [IN] the font face name. You can use whatever name you want, but the name must be unique for the font management system.
 * @param data [IN] the pointer of the font data.
 * @param length [IN] the size of the font data in bytes.
 */
void GrInstallFont(GR_CHAR *name, unsigned char *data, int length);

/**
 * <B>LGE-Specific</B> : See GrInstallFont() for more information.
 *
 * After you uninstall a font data from the system, you must free the font data if it is allocated.
 *
 * Here is an example.
 *
 * <PRE>
 * unsigned char *pData = malloc(length);
 *
 * // prepare the font data. For example, you may gather the font data from a network.
 *
 * // Now install the font data into the system
 * GrInstallFont("MyFontData", pData, length);
 *
 * // Create a font and use it....Do not corrupt pData until it is unsintalled.
 *
 * // When you don't need the font data any more, uninstall it.
 * GrUninstallFont("MyFontData");
 *
 * // You must free the font data.
 * free(pData);
 * </PRE>
 *
 * @param name [IN] the font face name.
 */
void GrUninstallFont(GR_CHAR *name);

/**
 * Creates a font from a given font data. This function is defined only when HAVE_FREETYPE_2_SUPPORT is enabled.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 */
GR_FONT_ID	GrCreateFontFromBuffer(const void *buffer, unsigned length,
			const char *format, GR_COORD height);

/**
 * Duplicate a font object to another one. You can specified the height of the new font.
 * This function is defined only when HAVE_FREETYPE_2_SUPPORT is enabled.
 *
 * <B>T.J.Park Note</B> : We don't use this function.
 */
GR_FONT_ID	GrCopyFont(GR_FONT_ID fontid, GR_COORD height);

/**
 * Retrieves all the fonts from the system. This function is defined only when HAVE_FREETYPE_SUPPORT is enabled.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 */
void		GrGetFontList(GR_FONTLIST ***fonts, int *numfonts);

/**
 * Retrieves all the fonts from the system. This function is defined only when HAVE_FREETYPE_SUPPORT is enabled.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 */
void		GrFreeFontList(GR_FONTLIST ***fonts, int numfonts);

/**
 * Changes the height of the given font object.
 *
 * <B>LGE-Specific</B> : This function is not implemented. The behaviour is undefined. Do not call this function.
 */
void		GrSetFontSize(GR_FONT_ID fontid, GR_COORD size);

/**
 * Rotate the given font object.
 *
 * <B>LGE-Specific</B> : This function is not implemented. The behaviour is undefined. Do not call this function.
 */
void		GrSetFontRotation(GR_FONT_ID fontid, int tenthsdegrees);

/**
 * Rotate the given font object.
 *
 * <B>LGE-Specific</B> : This function is not implemented. The behaviour is undefined. Do not call this function.
 */
void		GrSetFontAttr(GR_FONT_ID fontid, int setflags, int clrflags);

/**
 * Set a new pitch for the given font object.
 *
 * <B>LGE-Specific</B> : This function is not implemented. The behaviour is undefined. Do not call this function.
 */
void		GrSetFontPitch(GR_FONT_ID fontid, int pitchval);

/**
 * Destroys a font object from the system.
 *
 * @param fontid [IN] the ID of the font object.
 */
void		GrDestroyFont(GR_FONT_ID fontid);

/**
 * Retrieves information of the given font object.
 *
 * <B>LGE-Specific</B> : Basically this function works, but the implementation is incomplete and very inefficient.
 * If you want to get a text size, do not use this function. Instead, call GrGetGCTextSize().
 *
 * @param fontid [IN] the ID of the font object.
 */
void		GrGetFontInfo(GR_FONT_ID font, GR_FONT_INFO *fip);

/**
 * Set if the current font is UTF. It's Used for font position adjustment when UTF/TTF font informations are different.
 */
void GrSetIsUTFFont(int isUTF);
/**
 * Get if the current font is UTF.
 */
int GrGetIsUTFFont(void);

/**
 * Get the window ID which has the focus.
 *
 * <B>LGE-Specific</B> : Do not use this function. Regarding the input focus, you must implement your own management system.
 * Note that we don't use the keyboard or mouse driver of MicroWindows.
 */
GR_WINDOW_ID	GrGetFocus(void);

/**
 * Set a new focused window.
 *
 * <B>LGE-Specific</B> : Do not use this function. Regarding key focus, you must implement your own management system.
 * Note that we don't use the keyboard or mouse driver of MicroWindows.
 */
void		GrSetFocus(GR_WINDOW_ID wid);

/**
 * Clear the specified area of a window and possibly make an exposure event.
 * This sets the area window to its background color or pixmap.  If the
 * exposeflag is nonzero, then this also creates an exposure event for the
 * window.
 *
 * <B>LGE-Specific</B> : Do not use this function. Instead call GrFillRect() for your window background.
 */
void		GrClearArea(GR_WINDOW_ID wid, GR_COORD x, GR_COORD y,
			GR_SIZE width, GR_SIZE height, GR_BOOL exposeflag);

/**
 * Select events for a window for this client.
 * The events are a bitmask for the events desired.
 *
 *
 * <B>LGE-Specific</B> : Use only GR_EVENT_MASK_EXPOSURE to receive GR_EVENT_TYPE_EXPOSURE.
 *
 * @param wid [IN] the ID of the window
 * @param eventmask [IN] the bitmask for the events desired.
 */
void		GrSelectEvents(GR_WINDOW_ID wid, GR_EVENT_MASK eventmask);

/**
 * Return the next waiting event for a client, or wait for one if there
 * is none yet.  The event is copied into the specified structure, and
 * then is moved from the event queue to the free event queue.  If there
 * is an error event waiting, it is delivered before any other events.
 *
 *
 * <B>LGE-Specific</B> : Only GR_EVENT_TYPE_EXPOSURE is used. And, this
 * function does not wait for an event, because the waiting function(GsSelect()) does not work
 * in all operating system(select() must be available to do that).
 *
 * In order to make it portable in all operating systems, call GrPeekEvent() to check the event queue,
 * and if the return value is non-zero, then call this function.
 *
 * @param ep [OUT] the pointer of GR_EVENT structure.
 */
void		GrGetNextEvent(GR_EVENT *ep);

/**
 * Fills in the specified event structure with a copy of the next event on the
 * queue that matches the type parameters passed and removes it from the queue.
 * If 'block' is GR_TRUE, the call will block until a matching event is found.
 * Otherwise, only the local queue is searched, and an event type of
 * GR_EVENT_TYPE_NONE is returned if the match is not found.
 *
 *
 * <B>LGE-Specific</B> : This function is defined only when NONETWORK is disbbled, i.e, in client-server model.
 *
 * @param wid     Window id for which to check events. 0 means no window.
 * @param mask    Event mask of events for which to check. 0 means no check for mask.
 * @param update  Update event subtype when event mask is GR_EVENT_MASK_UPDATE.
 * @param ep      Pointer to the GR_EVENT structure to return the event in.
 * @param block   Specifies whether or not to block, GR_TRUE blocks, GR_FALSE does not.
 * @return        GR_EVENT_TYPE if an event was returned, or GR_EVENT_TYPE_NONE
 *                if no events match.
 */
int			GrGetTypedEvent(GR_WINDOW_ID wid, GR_EVENT_MASK mask,
			GR_UPDATE_TYPE update, GR_EVENT *ep, GR_BOOL block);

typedef GR_BOOL (*GR_TYPED_EVENT_CALLBACK)(GR_WINDOW_ID, GR_EVENT_MASK,
			GR_UPDATE_TYPE, GR_EVENT *, void *);

/**
 * The specified callback function is called with the passed event type parameters
 * for each event on the queue, until the callback function CheckFunction
 * returns GR_TRUE.  The event is then removed from the queue and returned.
 * If 'block' is GR_TRUE, the call will block until a matching event is found.
 * Otherwise, only the local queue is searched, and an event type of
 * GR_EVENT_TYPE_NONE is returned if the match is not found.
 *
 *
 * <B>LGE-Specific</B> : This function is defined only when NONETWORK is disabled, i.e, in client-server model.
 *
 * @param wid     Window id for which to check events. 0 means no window.
 * @param mask    Event mask of events for which to check. 0 means no check for mask.
 * @param update  Update event subtype when event mask is GR_EVENT_MASK_UPDATE.
 * @param ep      Pointer to the GR_EVENT structure to return the event in.
 * @param block   Specifies whether or not to block, GR_TRUE blocks, GR_FALSE does not.
 * @param matchfn Specifies the callback function called for matching.
 * @param arg     A programmer-specified argument passed to the callback function.
 * @return        GR_EVENT_TYPE if an event was returned, or GR_EVENT_TYPE_NONE
 *                if no events match.
 */
int			GrGetTypedEventPred(GR_WINDOW_ID wid, GR_EVENT_MASK mask,
			GR_UPDATE_TYPE update, GR_EVENT * ep, GR_BOOL block,
			GR_TYPED_EVENT_CALLBACK matchfn, void *arg);

/**
 * Return the next event from the event queue, or
 * wait for a new one if one is not ready.  If timeout
 * is nonzero, return timeout event if time elapsed.
 *
 * <B>LGE-Specific</B> : This function does not work if the system does not support select() function(For example, VxWorks).
 * Do not call this function. Instead use GrGetNextEvent() for compatibility.
 */
void		GrGetNextEventTimeout(GR_EVENT *ep, GR_TIMEOUT timeout);

/**
 * Return the next event from the event queue if one is ready.
 * If one is not ready, then the type GR_EVENT_TYPE_NONE is returned.
 * If it is an error event, then a user-specified routine is called
 * if it was defined, otherwise we clean up and exit.
 */
void		GrCheckNextEvent(GR_EVENT *ep);

/**
 * Peek at the event queue for the current client to see if there are any
 * outstanding events.  Returns the event at the head of the queue, or
 * else a null event type.  The event is still left in the queue, however.
 *
 * @param ep [OUT] the pointer of the GR_EVENT where the returned event will be saved in. This can not be NULL.
 * @return Non-zero if there are more that one event waiting in the queue. Zero otherwise.
 */
int			GrPeekEvent(GR_EVENT *ep);

/**
 * Wait until an event is available for a client, and then peek at it.
 */
void		GrPeekWaitEvent(GR_EVENT *ep);

/**
 * Draw a line in the specified drawable using the specified graphics context.
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x1 [IN] the x-coordinate of the start point
 * @param y1 [IN] the y-coordinate of the start point
 * @param x2 [IN] the x-coordinate of the end point
 * @param y2 [IN] the y-coordinate of the end point
 */
void		GrLine(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x1, GR_COORD y1,
			GR_COORD x2, GR_COORD y2);
/**
 * Draw a point in the specified drawable using the specified
 * graphics context.
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-coordinate of the point
 * @param y [IN] the y-coordinate of the point
 */
void		GrPoint(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y);

/**
 * Draw points in the specified drawable using the specified
 * graphics context.
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param count [IN] the number of the points
 * @param pointtable [IN] the pointer of the point array. This can not be NULL.
 */
void		GrPoints(GR_DRAW_ID id, GR_GC_ID gc, GR_COUNT count,
			GR_POINT *pointtable);

/**
 * Draw the boundary of a rectangle in the specified drawable using the
 * specified graphics context.
 *
 *
 * NOTE: this function draws a rectangle 1 pixel wider and higher
 * than Xlib's XDrawRectangle().
 *
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-coordinate of the left-top corner of the rectangle.
 * @param y [IN] the y-coordinate of the left-top corner of the rectangle.
 * @param width [IN] the width of the rectangle.
 * @param height [IN] the height of the rectangle.
 */
void		GrRect(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE width, GR_SIZE height);

/**
 * Fill a rectangle in the specified drawable using the specified
 * graphics context.
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-coordinate of the left-top corner of the rectangle.
 * @param y [IN] the y-coordinate of the left-top corner of the rectangle.
 * @param width [IN] the width of the rectangle.
 * @param height [IN] the height of the rectangle.
 */
void		GrFillRect(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE width, GR_SIZE height);

/**
 * Draw a polygon in the specified drawable using the specified
 * graphics context.  The polygon is only complete if the first
 * point is repeated at the end.
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param count [IN] the number of the points
 * @param pointtable [IN] the pointer of the point array. This can not be NULL.
 */
void		GrPoly(GR_DRAW_ID id, GR_GC_ID gc, GR_COUNT count,
			GR_POINT *pointtable);

/**
 * Draw a filled polygon in the specified drawable using the specified
 * graphics context.  The last point may be a duplicate of the first
 * point, but this is not required.
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param count [IN] the number of the points
 * @param pointtable [IN] the pointer of the point array. This can not be NULL.
 */
void		GrFillPoly(GR_DRAW_ID id, GR_GC_ID gc, GR_COUNT count,
			GR_POINT *pointtable);

/**
 * Draw the boundary of an ellipse in the specified drawable with
 * the specified graphics context. This function uses only integer operation.
 *
 * <B>T.J.Park Note</B> : GrEllipse, GrFillEllipse, and GrArc are implemented
 * by 1 base function which is using only integer operation.
 * Because of the way how the function is implemented, there is a restriction about the radius(rx and ry).
 * The radius must be smaller than 1024. If the radius is larger than or equal to 1024
 * , the result is undefined.
 * So, I added a check routine to truncate the radius, if the radius is larger
 * than or equal to 1024.
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-coordinate of the center.
 * @param y [IN] the y-coordinate of the center.
 * @param rx [IN] the radius in x direction.
 * @param ry [IN] the radius in y direction.
 */
void		GrEllipse(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE rx, GR_SIZE ry);

/**
 * Fill an ellipse in the specified drawable using the specified
 * graphics context. This function uses only integer operation.
 *
 * <B>T.J.Park Note</B> : GrEllipse, GrFillEllipse, and GrArc are implemented
 * by 1 base function which is using only integer operation.
 * Because of the way how the function is implemented, there is a restriction about the radius(rx and ry).
 * The radius must be smaller than 1024. If the radius is larger than or equal to 1024
 * , the result is undefined.
 * So, I added a check routine to truncate the radius, if the radius is larger
 * than or equal to 1024.
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-coordinate of the center.
 * @param y [IN] the y-coordinate of the center.
 * @param rx [IN] the radius in x direction.
 * @param ry [IN] the radius in y direction.
 */

void		GrFillEllipse(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x,
			GR_COORD y, GR_SIZE rx, GR_SIZE ry);
/**
 * Draw an arc, pie or ellipse in the specified drawable using
 * the specified graphics context. This function uses only
 * integer operation.
 *
 * The last parameter, 'type' is one of the following values.
 * <TABLE>
 * GR_ARC          Draw only the arc, i.e, the radiuses will not be
 *                  drawn.
 * GR_ARCOUTLINE   Draw the arc and the radiuses.
 * GR_PIE          Fill the arc, such as a pie chart.
 * </TABLE>
 *
 *
 * <B>T.J.Park Note</B> : To understand the meaning of (ax,ay)
 * and (bx,by), draw an infinite line bound for (ax, ay) from
 * the center. Then, the line means the start line of the arc.
 * As the same way, draw an infinite line bound for (bx, by)
 * from the center. This is the end line of the arc. You don't
 * need to specify the start or end point exactly on the
 * circumference of the arc. The only requirement is that the
 * points must be on the 'lines'. Also, if the start line and
 * the end line are the same line, then the result is a full
 * ellipse. See the picture below.
 *
 * <IMAGE arc>
 *
 * Also, note that GrArc has a bug when ay or by is same as y.
 * In that case, the expected result may be the exactly
 * horizontal line, but the result is not exactly horizontal.
 *
 * <B>T.J.Park Note</B> : GrEllipse, GrFillEllipse, and GrArc
 * are implemented by 1 base function which is using only
 * integer operation. Because of the way how the function is
 * implemented, there is a restriction about the radius(rx and
 * ry). The radius must be smaller than 1024. If the radius is
 * larger than or equal to 1024 , the result is undefined. So, I
 * added a check routine to truncate the radius, if the radius
 * is larger than or equal to 1024.
 *
 *
 * @param id    [IN] the ID of the drawable object, i.e, either
 *              window or pixmap.
 * @param gc    [IN] the ID of the GC.
 * @param x     [IN] the x\-cooridinate of the center
 * @param y     [IN] the y\-cooridinate of the center
 * @param rx    [IN] the radius of arc in x direction.
 * @param ry    [IN] the radius of arc in y direction.
 * @param ax    [IN] the x\-coordinate of the start position of the
 *              arc
 * @param ay    [IN] the y\-coordinate of the start position of the
 *              arc
 * @param bx    [IN] the x\-coordinate of the end position of the
 *              arc
 * @param by    [IN] the y\-coordinate of the end position of the
 *              arc
 * @param type  [IN] One of GR_ARC, GR_ARCOUTLINE, or GR_PIE. See
 *              \Description for more information.
 */
void		GrArc(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE rx, GR_SIZE ry, GR_COORD ax, GR_COORD ay,
			GR_COORD bx, GR_COORD by, int type);

/**
 * Draw an arc or pie in the specified drawable using
 * the specified graphics context.  This function requires floating point operation.
 * Because GrArc uses only integer operation, it is faster.
 * To use this function, HAVEFLOAT must be defined.
 *
 * The last parameter, type is one of the following values.
 * <TABLE>
 * GR_ARC			Draw only the arc, i.e, the radiuses will not be drawn.
 * GR_ARCOUTLINE	Draw the arc and the radiuses.
 * GR_PIE			Fill the arc, such as a pie chart.
 * </TABLE>
 *
 * @param id [IN] the ID of the drawable object, i.e, either window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-cooridinate of the center
 * @param y [IN] the y-cooridinate of the center
 * @param rx [IN] the radius of arc in x direction.
 * @param ry [IN] the radius of arc in y direction.
 * @param angle1 [IN] the start of arc.  In 64ths of a degree, anticlockwise from the +x axis.
 * @param angle2 [IN] th end of arc.  In 64ths of a degree, anticlockwise from the +x axis.
 * @param type [IN] One of GR_ARC, GR_ARCOUTLINE, or GR_PIE. See Description for more information.
 */
void		GrArcAngle(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE rx, GR_SIZE ry, GR_COORD angle1,
			GR_COORD angle2, int type);

/**
 * Set the foreground color in a graphics context from a passed RGB color value.
 * You can use GR_RGB or GR_ARGB macro to specify the color.
 *
 * <PRE>
 * GrSetGCForeground(gc, GR_ARGB(0x80,0xff,0x0,0x0)); // Set the foregournd color as translucent red.
 * </PRE>
 *
 * @param gc [IN] the ID of the GC
 * @param foreground [IN] the foreground color, specified by 0xAABBGGRR. This value is independent of the system color format.
 */
void		GrSetGCForeground(GR_GC_ID gc, GR_COLOR foreground);

/**
 * Set the foreground color in a graphics context from a passed pixel value.
 *
 * @param gc [IN] the ID of the GC
 * @param foreground [IN] the foreground color, specified by pixel value. This value is depending on what the system color format is.
 *                        For example, if the system color format is 8 bit indexed color, then this value is a 8 bit index.
 */
void		GrSetGCForegroundPixelVal(GR_GC_ID gc, GR_PIXELVAL foreground);

/**
 * Set the background color in a graphics context from a passed RGB color value.
 * You can use GR_RGB or GR_ARGB macro to specify the color.
 *
 * <PRE>
 * GrSetGCBackground(gc, GR_ARGB(0x80,0xff,0x0,0x0)); // Set the backgournd color as translucent red.
 * </PRE>
 *
 * <B>T.J.Park Note</B> : Background color is rarely used. The only typical case is when drawing text.
 *                 Also, in order to use background, you must call GrSetGCUseBackground().
 *                 Most operations are using only foreground color(even in GrFillRect).
 *
 * <B>LGE-Specific</B> : Because background color has a problem in case that the foreground and background color is exactly same,
 *                the current implementation totally ignores the background color whatever the use_background flag is.
 *
 * @param gc [IN] the ID of the GC
 * @param foreground [IN] the background color, specified by 0xAABBGGRR. This value is independent of the system color format.
 */
void		GrSetGCBackground(GR_GC_ID gc, GR_COLOR background);

/**
 * Set the background color in a graphics context from a passed pixel value.
 *
 * <B>T.J.Park Note</B> : Background color is rarely used. The only typical case is when drawing text.
 *                 Also, in order to use background, you must call GrSetGCUseBackground().
 *
 * <B>LGE-Specific</B> : Because background color has a problem if the foreground and background color is exactly same,
 *                the current implementation totally ignores the background color whatever the use_background flag is.
 *
 * @param gc [IN] the ID of the GC
 * @param foreground [IN] the foreground color, specified by pixel value. This value is depending on what the system color format is.
 *                        For example, if the system color format is 8 bit indexed color, then this value is a 8 bit index.
 */
void		GrSetGCBackgroundPixelVal(GR_GC_ID gc, GR_PIXELVAL background);

/**
 * Set whether or not the background color is drawn in bitmaps and text.
 *
 * <B>LGE-Specific</B> : Because background color has a problem if the foreground and background color is exactly same,
 *                the current implementation totally ignores the background color whatever the use_background flag is.
 *
 * @param gc [IN] the ID of the GC.
 * @param flag [IN] GR_TRUE to use the background color. GR_FALSE otherwise.
 */
void		GrSetGCUseBackground(GR_GC_ID gc, GR_BOOL flag);

/**
 * Set the drawing mode in a graphics context.
 *
 * Here are the all the possible modes.
 * <TABLE>
 * GR_MODE_COPY				src
 * GR_MODE_SET				obsolete, use GR_MODE_COPY
 * GR_MODE_XOR				src ^ dst
 * GR_MODE_OR				src | dst
 * GR_MODE_AND				src & dst
 * GR_MODE_CLEAR			0
 * GR_MODE_SETTO1			11111111. will be GR_MODE_SET
 * GR_MODE_EQUIV			~(src ^ dst)
 * GR_MODE_NOR				~(src | dst)
 * GR_MODE_NAND				~(src & dst)
 * GR_MODE_INVERT			~dst
 * GR_MODE_COPYINVERTED		~src
 * GR_MODE_ORINVERTED		~src | dst
 * GR_MODE_ANDINVERTED		~src & dst
 * GR_MODE_ORREVERSE		src | ~dst
 * GR_MODE_ANDREVERSE		src & ~dst
 * GR_MODE_NOOP				dst
 * GR_MODE_SRC_OVER			blend src and dst based on alpha channel by porter-duff algorithm
 * </TABLE>
 *
 * <B>LGE-Specific</B> : Only GR_MODE_COPY, GR_MODE_CLEAR, and GR_MODE_SRC_OVER are implemented. Other operations are not supported.
 *
 * @param gc [IN] the ID of the GC
 * @param mode [IN] the drawing mode. See Description for more information.
 */
void		GrSetGCMode(GR_GC_ID gc, int mode);

/**
 * <B>LGE-Specific</B> : This function is newly added in order to support global alpha of MHP/OCAP.
 * Global alpha value overrides the alpha channel of each pixel when the drawing mode is GR_MODE_SRC_OVER.
 * If this value is negative, then the global alpha will not be used when drawing.
 *
 * @param gc [IN] the ID of the GC
 * @param alpha [IN] the global alpha value. If this value is greater than 255, then the global alpha is simply set as 255.
 */
void		GrSetGCAlpha(GR_GC_ID gc, int alpha);

/*
 * Set the gradiation option. If the user wants to draw text in gradiation mode, use this value.
 *
 * @param gc [IN] the ID of the GC
 * @param startpos [IN] the start point of gradation
 * @param length [IN] the length of gradation
 * @param direction [IN] gradation direction: up/down/left/right.
 */
void GrSetGCGradationOption(GR_GC_ID gc, int startpos, int length, int direction);

/* foryou add xor color */
/**
 * <B>LGE-Specific</B> : This function is newly added in order to support xorcolor of MHP/OCAP.
 * @param gc [IN] the ID of the GC
 * @param alpha [IN] the xor color value.
 */
void	GrSetGCXor(GR_GC_ID gc, GR_COLOR xorcolor);
/**
 * Set the attributes of the line.
 *
 * Here are the possible values of linestyle.
 * <TABLE>
 * GR_LINE_SOLID		solid line style
 * GR_LINE_ONOFF_DASH	dash line style
 * </TABLE>
 *
 * @param gc [IN] the ID of the GC
 * @param linestyle [IN] the line style. See Description for more information.
 */
void		GrSetGCLineAttributes(GR_GC_ID gc, int linestyle);

/**
 * Set the dash mode. A series of numbers are passed indicating the on / off state.
 * For example { 3, 1 } indicates 3 on and 1 off.
 *
 * <B>T.J.Park Note</B> : I don't know the exact meaning of this function.
 */
void		GrSetGCDash(GR_GC_ID gc, char *dashes, int count);

/**
 * Set the fill mode.
 *
 * Here are the possible values of fillmode.
 * <TABLE>
 * GR_FILL_SOLID
 * GR_FILL_STIPPLE
 * GR_FILL_OPAQUE_STIPPLE
 * GR_FILL_TILE
 * </TABLE>
 *
 * @param gc [IN] the ID of the GC
 * @param fillmode [IN] the fill mode. See Description for more information.
 */
void		GrSetGCFillMode(GR_GC_ID gc, int fillmode);

/**
 * Set the stipple bitmap which is used for stipple-mode filling.
 *
 * @param gc [IN] the ID of the GC
 * @param bitmap [IN] the pointer of the stipple bitmap.
 * @param width [IN] the width of the bitmap
 * @param height [IN] the height of the bitmap
 */
void		GrSetGCStipple(GR_GC_ID gc, GR_BITMAP * bitmap, GR_SIZE width, GR_SIZE height);

/**
 * Set the tile drawable which is used for tile-mode filling.
 *
 * @param gc [IN] the ID of the GC
 * @param pixmap [IN] the ID of the drawable, i.e, window or pixmap.
 * @param width [IN] the width of the drawable.
 * @param height [IN] the height of the drawable.
 */
void		GrSetGCTile(GR_GC_ID gc, GR_WINDOW_ID pixmap, GR_SIZE width, GR_SIZE height);

/**
 * This sets the stipple offset to the specified offset.
 *
 * @param gc [IN] the ID of the GC
 * @param x [IN] the x offset of the stipple.
 * @param y [IN] the y offset of the stipple.
 */
void		GrSetGCTSOffset(GR_GC_ID gc, GR_COORD x, GR_COORD y);

/**
 * Sets whether or not the exposure event is sent on GrCopyArea().
 *
 * @param gc [IN] the ID of the GC.
 * @param exposure [IN] GR_TRUE for receiving exposure event after you call GrCopyArea(). GR_FALSE otherwise.
 */
void		GrSetGCGraphicsExposure(GR_GC_ID gc, GR_BOOL exposure);

/**
 * Sets the font in a graphics context.
 * Regarding font, refer to GrCreateFont for more information.
 *
 * @param gc [IN] the ID of the GC.
 * @param font [IN] the ID of the font.
 */
void		GrSetGCFont(GR_GC_ID gc, GR_FONT_ID font);

/**
 * Retrieves width, height and the baseline of the text.
 *
 * <B>T.J.Park Note</B> : Unlike GrText, this function is not affected by the alignment flag(GR_TFTOP
 *                 , GR_TFBASELINE, or GR_TFBOTTOM), because it just calculate the extent of the text.
 *                 But, the text encoding flag(GR_TFASCII, GR_TFUTF8, GR_TFUC16, etc) is necessary.
 *
 * About the baseline : the returned baseline is defined as the length from the top to the baseline. <p>
 * So, ascending = baseline, <p>
 * and, descending = height - baseline.
 *
 * @param gc [IN] the ID of the GC
 * @param str [IN] the pointer of the text. The size of each character is depending on the encoding flags.
 * @param count [IN] the count of the character.
 * @param flags [IN] the encoding rule. See GR_TEXTFLAGS for more information.
 * @param retwidth [OUT] the pointer of the width to be returned. This can not be NULL.
 * @param retheight [OUT] the pointer of the height to be returned. This can not be NULL.
 * @param retbase [OUT] the pointer of the baseline to be returned. This can not be NULL.
 */
void		GrGetGCTextSize(GR_GC_ID gc, void *str, int count,
			GR_TEXTFLAGS flags, GR_SIZE *retwidth,
			GR_SIZE *retheight,GR_SIZE *retbase);

/**
 * Read the color values from the specified rectangular area of the
 * specified drawable into a supplied buffer.  If the drawable is a
 * window which is obscured by other windows, then the returned values
 * will include the values from the covering windows.  Regions outside
 * of the screen boundaries, or unmapped windows will return black.
 *
 * <B>T.J.Park Note</B> : The format of the returned image is depending on the system color format.
 * Also, note that GR_PIXELVAL is 0xAARRGGBB order if the sysytem color format is 24bpp RGB or 32bpp ARGB,
 * unlike the GR_COLOR whose order is 0xAABBGGRR.
 *
 * @param id [IN] the ID of the drawable, i.e, window or pixmap.
 * @param x [IN] the x-coordinate of the left-top corner of the area to be read.
 * @param y [IN] the y-coordinate of the left-top corner of the area to be read.
 * @param width [IN] the width of the area to be read.
 * @param height [IN] the height of the area to be read.
 * @param pixels [OUT] the pointer of the pixels to be returend. This can not be NULL and the memory must be large enough to contain the image.
 */
void		GrReadArea(GR_DRAW_ID id, GR_COORD x, GR_COORD y, GR_SIZE width,
			GR_SIZE height, GR_PIXELVAL *pixels);

/**
 * Draw a rectangular area in the specified drawable using the specified
 * graphics context.  This differs from rectangle drawing in that the
 * color values for each pixel in the rectangle are specified.
 * The color table is indexed row by row.
 *
 * The possible values of pixtype are as followings.
 * <TABLE>
 * MWPF_RGB				pixel is GR_COLOR type whose order is 0xAABBGGRR.
 * MWPF_PALETTE			pixel is packed 8 bits 1, 4 or 8 pal index.
 * MWPF_TRUECOLOR0888	pixel is packed 32 bits 8/8/8 truecolor
 * MWPF_TRUECOLOR888	pixel is packed 24 bits 8/8/8 truecolor
 * MWPF_TRUECOLOR565	pixel is packed 16 bits 5/6/5 truecolor
 * MWPF_TRUECOLOR555	pixel is packed 16 bits 5/5/5 truecolor
 * MWPF_TRUECOLOR332	pixel is packed 8 bits 3/3/2 truecolor
 * MWPF_TRUECOLOR8888	pixel is packed 32 bits 8/8/8/8 truecolor with alpha
 * </TABLE>
 *
 * <B>T.J.Park Note</B> : Though this function can be used for drawing an image, it is not a good idea.
 * This function is very inefficient because of color format conversion, and the current implementation
 * does not support any HW accelation. Because some HW supports the color format conversion on blitting,
 * the performace may be improved if we rewrote this function( even in the current implementation, you can
 * find the vestige about that effort, but it is commented out at the current time. ).
 * For better performance, use GrDrawImageToFit for drawing an image, or use GrCopyArea for blitting.
 *
 * @param id [IN] the ID of the drawable, i.e, window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-coordinate of the left-top corner of the area.
 * @param y [IN] the y-coordinate of the left-top corner of the area.
 * @param width [IN] the width of the area.
 * @param height [IN] the height of the area.
 * @param pixels [IN] the pixels to be drawn. This can not be NULL.
 * @param pixtype [IN] the pixel type. See Description for more information.
 */
void		GrArea(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE width,GR_SIZE height,void *pixels,int pixtype);

/**
 * Copy(blit) a rectangle from one drawable to another or the same.
 *
 * Possible values of op are as followings.
 * <TABLE>
 * MWROP_USE_GC_MODE	Use the GC mode which is set by GrSetGCMode(). For example, if the GC mode is GR_MODE_SRC_OVER, then the blit operation shall be MWROP_SRC_OVER.
 * MWROP_COPY			src
 * MWROP_XOR			src ^ dst
 * MWROP_OR				src | dst
 * MWROP_AND			src & dst
 * MWROP_CLEAR			0
 * MWROP_SET			obsolete, use MWROP_COPY
 * MWROP_EQUIV			~(src ^ dst)
 * MWROP_NOR			~(src | dst)
 * MWROP_NAND			~(src & dst)
 * MWROP_INVERT			~dst
 * MWROP_COPYINVERTED	~src
 * MWROP_ORINVERTED		~src | dst
 * MWROP_ANDINVERTED	~src & dst
 * MWROP_ORREVERSE		src | ~dst
 * MWROP_ANDREVERSE		src & ~dst
 * MWROP_NOOP			dst
 * MWROP_XOR_FGBG		fg ^ bg
 * MWROP_SRC			same as MWROP_COPY
 * MWROP_DST			same as MWROP_NOOP
 * MWROP_SRC_OVER		blend src and dst based on alpha channel by Porter-Duff algorithm.
 * MWROP_DST_OVER		blend dst and src based on alpha channel by Porter-Duff algorithm.
 * MWROP_SRC_IN			Porter-Duff source-in algorithm.
 * MWROP_DST_IN			Porter-Duff dest-in algorithm.
 * MWROP_SRC_OUT		Porter-Duff source-out algorithm.
 * MWROP_DST_OUT		Porter-Duff dest-out algorithm.
 * </TABLE>
 *
 * <B>LGE-Specific</B> : There are many MWROP_XXX options defined in mwtypes.h, but only the following options are available now. <p>
 * - MWROP_COPY <p>
 * - MWROP_CLEAR <p>
 * - MWROP_SRC_OVER <p>
 * - MWROP_USE_GC_MODE <p>
 *
 * To understand the Porter-Duff operations(MWROP_SRC_OVER etc), see http://java.sun.com/docs/books/tutorial/2d/display/compositing.html.
 *
 * @param id [IN] the ID of the destination drawable(window or pixmap).
 * @param gc [IN] the ID of the GC
 * @param x [IN] the x-coordinate of the left-top corner of the destination area.
 * @param y [IN] the y-coordinate of the left-top corner of the destination area.
 * @param width [IN] the width of the destination area.
 * @param height [IN] the height of the destination area.
 * @param srcid [IN] the ID of the source drawable(window or pixmap).
 * @param srcx [IN] the x-coordinate of the left-top corner of the source area.
 * @param srcy [IN] the y-coordinate of the left-top corner of the source area.
 * @param op [IN] options based on src/dst binary operations. See Description for more information.
 */
void		GrCopyArea(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE width, GR_SIZE height, GR_DRAW_ID srcid,
			GR_COORD srcx, GR_COORD srcy, unsigned long op);

/**
 * Copy and stretch a rectangle, but does not support flipping the image.
 * The parameters are very different from GrStretchAreaEx() though it looks very similiar at the first glance.
 * GrStretchAreaEx()'s positioning is done by two points, while GrStretchArea()'s parameters are (x,y) and (width, height).
 * If the stretching is done by software, GrStretchAreaEx() is faster than GrStretchArea().
 * If the stretching is done by hardware, GrStretchArea() is faster and the quality may be far better.
 * There is no version of GrStretchAreaEx() implemented by HW accelation.
 *
 * <B>LGE-Specific</B> : GrStretchArea() of the original MicroWindows 0.90 is now GrStretchAreaEx() in this LGE version.
 *
 * To understand the Porter-Duff operations(MWROP_SRC_OVER etc), see http://java.sun.com/docs/books/tutorial/2d/display/compositing.html.
 *
 * @param dstid [IN] the ID of the destination drawable(window or pixmap).
 * @param gc [IN] the ID of the GC
 * @param dx [IN] the x-coordinate of the left-top corner of the destination area.
 * @param dy [IN] the y-coordinate of the left-top corner of the destination area.
 * @param dw [IN] the width of the destination area.
 * @param dh [IN] the height of the destination area.
 * @param srcid [IN] the ID of the source drawable(window or pixmap).
 * @param sx [IN] the x-coordinate of the left-top corner of the source area.
 * @param sy [IN] the y-coordinate of the left-top corner of the source area.
 * @param sw [IN] the width of the source area.
 * @param sh [IN] the height of the source area.
 * @param op [IN] options based on src/dst binary operations. See GrCopyArea() for details.
 */
void		GrStretchArea(GR_DRAW_ID dstid, GR_GC_ID gc, GR_COORD dx,
			GR_COORD dy, GR_COORD dw, GR_COORD dh,
			GR_DRAW_ID srcid, GR_COORD sx, GR_COORD sy,
			GR_COORD sw, GR_COORD sh, unsigned long op);

/**
 * Copy and stretch a rectangle, and also supports flipping the image.
 * Paramaters are the coordinates of two points in the source, and
 * two corresponding points in the destination.  The image is scaled
 * and flipped as needed to make the two points correspond.
 * The top-left corner is drawn, the bottom right one isn't
 * , i.e, (0,0)-(2,2) specifies a 2x2 rectangle consisting of the points
 * at (0,0), (0,1), (1,0), (1,1).  It does not include the points where x=2 or y=2.
 *
 * This function can flip the source image. The flipping rule is as followings.
 * - if sx2 - sx1 and dx2 - dx1 have the same sign(i.e, both positive or both negative), then there is no horizontal flipping.
 * - if sx2 - sx1 and dx2 - dx1 have different sign(i.e, one of them is positive, the other is negative), then horizontal flipping occurs.
 * - Same rule for vertical flipping.
 *
 * Note that overlapping of source and destination is not supported, and the result is undefined.
 *
 * <B>LGE-Specific</B> : The original name of this function was GrStretchArea(). In this LGE version, GrStretchArea()
 * is a more simple function, which does not support flipping. But GrStretchArea() may be faster if HW is available, and
 * may be grey-scaled.
 *
 * There are many MWROP_XXX options defined in mwtypes.h, but only the following options are available now. <p>
 * - MWROP_COPY <p>
 * - MWROP_CLEAR <p>
 * - MWROP_SRC_OVER <p>
 * - MWROP_USE_GC_MODE <p>
 * Also, because this function can do stretching and flipping in one loop, it is very difficult to use HW for stretching.
 * In the current implementation, both the stretching and flipping are done by software, and the stretching is not grey-scaled.
 * If you want to use HW scaler, use GrStretchArea() instead.
 *
 * To understand the Porter-Duff operations(MWROP_SRC_OVER etc), see http://java.sun.com/docs/books/tutorial/2d/display/compositing.html.
 *
 * @param dstid [IN] the ID of the destination drawable(window or pixmap).
 * @param gc [IN] the ID of the GC
 * @param dx1 [IN] the x-coordinate of the first corner of the destination. This need not be the left-top corner.
 * @param dy1 [IN] the y-coordinate of the first corner of the destination. This need not be the left-top corner.
 * @param dx2 [IN] the x-coordinate of the second corner of the destination. This need not be the right-bottom corner.
 * @param dy2 [IN] the y-coordinate of the second corner of the destination. This need not be the right-bottom corner.
 * @param srcid [IN] the ID of the source drawable(window or pixmap).
 * @param sx1 [IN] the x-coordinate of the first corner of the source. This need not be the left-top corner.
 * @param sy1 [IN] the y-coordinate of the first corner of the source. This need not be the left-top corner.
 * @param sx2 [IN] the x-coordinate of the second corner of the source. This need not be the right-bottom corner.
 * @param sy2 [IN] the y-coordinate of the second corner of the source. This need not be the right-bottom corner.
 * @param op [IN] options based on src/dst binary operations. See GrCopyArea() for details.
 */
void		GrStretchAreaEx(GR_DRAW_ID dstid, GR_GC_ID gc, GR_COORD dx1,
			GR_COORD dy1, GR_COORD dx2, GR_COORD dy2,
			GR_DRAW_ID srcid, GR_COORD sx1, GR_COORD sy1,
			GR_COORD sx2, GR_COORD sy2, unsigned long op);
/**
 * Draw a rectangular area in the specified drawable using the specified
 * graphics, as determined by the specified 1 bpp monochrome bitmap.  This differs from
 * rectangle drawing in that the rectangle is drawn using the foreground
 * color and possibly the background color as determined by the bitmap.
 * Each row of bits is aligned to the next bitmap word boundary (so there
 * is padding at the end of the row).  The background bit values are only
 * written if the usebackground flag is set in the GC.
 *
 * @param id [IN] the ID of the drawable, i.e, window or pixmap.
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-coordinate of the left-top corner of the destination area.
 * @param y [IN] the y-coordinate of the right-bottom corner of the destination area.
 * @param width [IN] the width of the destination area(same as the width of the source bitmap)
 * @param height [IN] the height of the destination area(same as the height of the source bitmap)
 * @param imagebits [IN] the pointer of the 1 bpp monochrome image. This can not be NULL.
 */
void		GrBitmap(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE width, GR_SIZE height, GR_BITMAP *imagebits);

/**
 * Draw a multicolor image at x, y. This is similiar to GrDrawImageToFit, but the source image
 * is specified by a GR_IMAGE_HDR, not by a loaded image ID(GR_IMAGE_ID).
 * Also, this function does not support scaling, so the destination width and height are not
 * specified by the parameters.
 *
 * The compression flag of GR_IMAGE_HDR may be one of the followings
 * <TABLE>
 * MWIMAGE_BGR				compression flag: BGR byte order
 * MWIMAGE_RGB				compression flag: RGB not BGR bytes
 * MWIMAGE_ALPHA_CHANNEL	compression flag: 32-bit w/alpha. <B>LGE-Specific</B> : This mode is not used. All 32bpp formats are considered as ARGB or ABGR format.
 * </TABLE>
 *
 * @param id [IN] the ID of the drawable, i.e, window or pixmap
 * @param gc [IN] the ID of the GC.
 * @param x [IN] the x-coordinate of the left-top corner of the destination area.
 * @param y [IN] the y-coordinate of the left-top corner of the destination area.
 * @param pimage [IN] the pointer of GR_IMAGE_HDR structure.
 */
void		GrDrawImageBits(GR_DRAW_ID id,GR_GC_ID gc,GR_COORD x,GR_COORD y,
			GR_IMAGE_HDR *pimage);

/**
 * Load an image file from the file system and display it at the specified coordinates.
 * This function is defined only when HAVE_FILEIO is enabled.
 *
 * <B>LGE-Specific</B> : Three types of images are supported now. GIF, PNG, and JPEG.
 *                Note that MicroWindows does not retrieve 16 bpp decoded image, instead, it converts all 16 bpp images to 24 or 32 bpp.
 *                If the image contains alpha channel, the returned image's format is 32 bpp ARGB or ABGR, regardless of the original image format.
 *                Currently, PNG is the only format which can support alpha channel, so all PNG images containing alpha channel shall be converted to 32 bpp.
 *
 * <B>T.J.Park Note</B> : There is an issue about animated GIF. An animated GIF may contain mutiple image planes.
 *                 MicroWindows does not support this case, and retrieves only the first image of the GIF data.
 *
 * @param id [IN] the ID of the drawable, i.e, window or pixmap
 * @param gc [IN] the ID of the GC
 * @param x [IN] the x-coordinate of the left-top corner of the destination area.
 * @param y [IN] the y-coordinate of the left-top corner of the destination area.
 * @param width [IN] the width of the destination area.
 *                   If this value is minus, the destination width is determined by the source(image) width.
 *                   If this value is positive and different from the image's width, then the image will be scaled up or down.
 * @param height [IN] the height of the destination area.
 *                    If this value is minus, the destination height is determined by the source(image) height.
 *                    If this value is positive and different from the image's height, then the image will be scaled up or down.
 * @param path [IN] the full path of the image file. This can not be NULL.
 * @param flags [IN] If non-zero, the JPEG image will be loaded as greyscale. For other image format, this parameter will be ignored.
 */
void		GrDrawImageFromFile(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x,
			GR_COORD y, GR_SIZE width, GR_SIZE height,
			char *path, int flags);

/**
 * Load an image file from the file system, and retrieve the ID of the loaded image.
 * This function is defined only when HAVE_FILEIO is enabled.
 *
 * <B>LGE-Specific</B> : Three types of images are supported now. GIF, PNG, and JPEG.
 *                Note that MicroWindows does not retrieve 16 bpp decoded image, instead, it converts all 16 bpp images to 24 or 32 bpp.
 *                If the image contains alpha channel, the returned image's format is 32 bpp ARGB or ABGR, regardless of the original image format.
 *                Currently, PNG is the only format which can support alpha channel, so all PNG images containing alpha channel shall be converted to 32 bpp.
 *
 * <B>T.J.Park Note</B> : There is an issue about animated GIF. An animated GIF may contain mutiple image planes.
 *                 MicroWindows does not support this case, and retrieves only the first image of the GIF data.
 *
 * @param path [IN] the full path of the image file. This can not be NULL.
 * @param flags [IN] If non-zero, the JPEG image will be loaded as greyscale. For other image format, this parameter will be ignored.
 * @return the ID of the loaded image. If fails, it returns 0.
 */
GR_IMAGE_ID	GrLoadImageFromFile(char *path, int flags);

/**
 * Draw an image from the given buffer, and display it at the specified coordinates.
 *
 * @param id [IN] the ID of the drawable, i.e, window or pixmap
 * @param gc [IN] the ID of the GC
 * @param x [IN] the x-coordinate of the left-top corner of the destination area.
 * @param y [IN] the y-coordinate of the left-top corner of the destination area.
 * @param width [IN] the width of the destination area.
 *                   If this value is minus, the destination width is determined by the source(image) width.
 *                   If this value is positive and different from the image's width, then the image will be scaled up or down.
 * @param height [IN] the height of the destination area.
 *                    If this value is minus, the destination height is determined by the source(image) height.
 *                    If this value is positive and different from the image's height, then the image will be scaled up or down.
 * @param buffer [IN] the pointer of the buffer which contains the image data. This can not be NULL.
 * @param buffer_width [IN] the width of the source buffer area.
 * @param buffer_height [IN] the height of the source buffer area.
 */
void		GrDrawImageFromRawDataBuffer(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			GR_SIZE width, GR_SIZE height, void *buffer, GR_SIZE buffer_width, GR_SIZE buffer_height);

/**
 * Load an image from the given buffer, and display it at the specified coordinates.
 *
 * <B>LGE-Specific</B> : Three types of images are supported now. GIF, PNG, and JPEG.
 *                Note that MicroWindows does not retrieve 16 bpp decoded image, instead, it converts all 16 bpp images to 24 or 32 bpp.
 *                If the image contains alpha channel, the returned image's format is 32 bpp ARGB or ABGR, regardless of the original image format.
 *                Currently, PNG is the only format which can support alpha channel, so all PNG images containing alpha channel shall be converted to 32 bpp.
 *
 * <B>T.J.Park Note</B> : There is an issue about animated GIF. An animated GIF may contain mutiple image planes.
 *                 MicroWindows does not support this case, and retrieves only the first image of the GIF data.
 *
 * @param id [IN] the ID of the drawable, i.e, window or pixmap
 * @param gc [IN] the ID of the GC
 * @param x [IN] the x-coordinate of the left-top corner of the destination area.
 * @param y [IN] the y-coordinate of the left-top corner of the destination area.
 * @param width [IN] the width of the destination area.
 *                   If this value is minus, the destination width is determined by the source(image) width.
 *                   If this value is positive and different from the image's width, then the image will be scaled up or down.
 * @param height [IN] the height of the destination area.
 *                    If this value is minus, the destination height is determined by the source(image) height.
 *                    If this value is positive and different from the image's height, then the image will be scaled up or down.
 * @param buffer [IN] the pointer of the buffer which contains the image data. This can not be NULL.
 * @param size [IN] the size of the buffer.
 * @param flags [IN] If non-zero, the JPEG image will be loaded as greyscale. For other image format, this parameter will be ignored.
 */
void		GrDrawImageFromBuffer(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x,
			GR_COORD y, GR_SIZE width, GR_SIZE height,
			void *buffer, int size, int flags);

/**
 * Load an image from the given buffer, and retrieve the ID of the loaded image.
 *
 * <B>LGE-Specific</B> : Three types of images are supported now. GIF, PNG, and JPEG.
 *                Note that MicroWindows does not retrieve 16 bpp decoded image, instead, it converts all 16 bpp images to 24 or 32 bpp.
 *                If the image contains alpha channel, the returned image's format is 32 bpp ARGB or ABGR, regardless of the original image format.
 *                Currently, PNG is the only format which can support alpha channel, so all PNG images containing alpha channel shall be converted to 32 bpp.
 *
 * <B>T.J.Park Note</B> : There is an issue about animated GIF. An animated GIF may contain mutiple image planes.
 *                 MicroWindows does not support this case, and retrieves only the first image of the GIF data.
 *
 * @param buffer [IN] the pointer of the buffer which contains the image data. This can not be NULL.
 * @param size [IN] the size of the buffer.
 * @param flags [IN] If non-zero, the JPEG image will be loaded as greyscale. For other image format, this parameter will be ignored.
 * @return the ID of the loaded image. If fails, it returns 0.
 */
GR_IMAGE_ID	GrLoadImageFromBuffer(void *buffer, int size, int flags);


/**
 * <B>LGE-Specific</B> : This function is newly added in order to support animated GIF.
 * This function is different from GrLoadImageFromBuffer(), in that this function may load multiple images from
 * one buffer, if the buffer contains an animated GIF.
 * The caller can be aware that the returned image is an animated GIF through checking pResult->count.
 * If pResult->count is 0, it means that the loading failed.
 * If pResult->count is 1, it means the loaded image is just a image.
 * If pResult->count is greater than 1, it means the loaded image is an animated GIF.
 *
 * It is the caller's responsibility to perform the "animation" if multiple image ids are returned.
 * In this case, the caller must check the logical width/height, x/y offset, delay time, user input flag,
 * and disposal flag, which are specified in GIF specification.
 * All the values are also newly added in GR_IMAGE_INFO, which you can get from GrGetImageInfo().
 * Refer to GIF specification for the exact meaning of the values.
 *
 * Because this function may return multiple image ids, the caller must free all the returned images, by calling
 * multiple call of GrFreeImage().
 *
 * @param buffer [IN] the pointer of the buffer which contains the image data. This can not be NULL.
 * @param size [IN] the size of the buffer.
 * @param pResult [OUT] the pointer of GR_LOAD_IMAGE_EX_RESULT, where the loaded image ids will be saved.
 */
void GrLoadImageEx(void *buffer, int size, GR_LOAD_IMAGE_EX_RESULT *pResult);

/**
 * Draw an loaded image at the specified coordinates.
 *
 * @param id [IN] the ID of the drawable, window or pixmap
 * @param gc [IN] the ID of the GC
 * @param x [IN] the x-coordinate of left-top corner of the destination area.
 * @param y [IN] the y-coordinate of left-top corner of the destination area.
 * @param width [IN] the width of the destination area.
 *                   If this value is minus, the destination width is determined by the source(image) width.
 *                   If this value is positive and different from the image's width, then the image will be scaled up or down.
 * @param height [IN] the height of the destination area.
 *                    If this value is minus, the destination height is determined by the source(image) height.
 *                    If this value is positive and different from the image's height, then the image will be scaled up or down.
 * @param imageid [IN] the ID of the loaded image
 */
void		GrDrawImageToFit(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x,
			GR_COORD y, GR_SIZE width, GR_SIZE height,
			GR_IMAGE_ID imageid);

/** <B>LGE-Specific</B> : This function is added to support scissoring and stretching of (a part of) an image.
	This is a JavaVM requirement. Note that the following two function calls make exactly same result.
	<PRE>
	GrDrawImageToFix(wid, gc, 100, 100, -1, -1, imageId);
	GrDrawImageToFixEx(wid, gc, 100, 100, -1, -1, imageId, 0, 0, -1, -1);
	</PRE>

	@param id [IN] the ID of window or pixmap where imageid will be drawn to.
	@param gc [IN] the ID of GC.
	@param x [IN] the x-coordinate of the destination window or pixmap.
	@param y [IN] the y-coordinate of the destination window or pixmap.
	@param width [IN] the width of the destination area.
					If this value is minus, the destination width is determined by the source(image) width.
	@param height [IN] the height of the destination area.
					If this value is minus, the destination height is determined by the source(image) height.
	@param imageid [IN] the ID of image.
	@param imageX [IN] the x-coordinate of the image.
					If this value is bigger than the image width minus 1, then this function does nothing.
					If this value is smaller than 0, it is treated as 0 (horizontal start position of the image).
	@param imageY [IN] the y-coordinate of the image.
					If this value is bigger than the image height minus 1, then this function does nothing.
					If this vlaue is smaller than 0, it is treated as 0 (vertical start position of the image).
	@param imageWidth [IN] the width of the image to be drawn.
					If this value is smaller than 0, it is treated as the whole image width.
					If imageX plus imageWidth is bigger than the whole image width, then it is truncated.
	@param imageHeight [IN] the height of the image to be drawn.
					If this value is smaller than 0, it is treated as the whoel image height.
					If imageY plus imageHeight is bigger than the whole image height, then it is truncated.
*/
void		GrDrawImageToFitEx(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y
						, GR_SIZE width, GR_SIZE height
						, GR_IMAGE_ID imageid, GR_COORD imageX, GR_COORD imageY
						, GR_COORD imageWidth, GR_COORD imageHeight);

/**
 * Free a loaded image.
 *
 * @param id [IN] the ID of the loaded image.
 */
void		GrFreeImage(GR_IMAGE_ID id);

/**
 * Retrieve the information about the given image.
 *
 * <B>LGE-Specific</B> : The original MicroWindows 0.90 does not return the image data pointer.
 *                We add a new member, imagebits, in order to read the image data quickly.
 *                Be careful if you write data into the imagebits.
 *                Also, the retrieved imagebits pointer shall not be accessed in client-server model.
 *
 * @param id [IN] the ID of the loaded image
 * @param iip [OUT] the pointer of GR_IMAGE_INFO structure where the returned information will be saved in. This can not be NULL.
 */
void		GrGetImageInfo(GR_IMAGE_ID id, GR_IMAGE_INFO *iip);

/**
 * Draw the given text at the specified coordinates.
 *
 * @param id [IN] the ID of the drawable, i.e, window or pixmap
 * @param gc [IN] the ID of the GC
 * @param x [IN] the x-coordinate of the leftmost position of the text.
 * @param y [IN] the y-coordinate of the text. The exact meaning is defined by the alignment flags.
 * @param str [IN] the pointer of the text. The size of each character is depending on the encoding flags.
 * @param count [IN] the count of the character.
 * @param flags [IN] combination of alignment and encoding rule. See GR_TEXTFLAGS for more information.
 */
void		GrText(GR_DRAW_ID id, GR_GC_ID gc, GR_COORD x, GR_COORD y,
			void *str, GR_COUNT count, GR_TEXTFLAGS flags);

/**
 * Create a new server-based cursor resource.
 *
 * <B>LGE-Specific</B> : We don't use this feature. The mouse driver is empty.
 */
GR_CURSOR_ID	GrNewCursor(GR_SIZE width, GR_SIZE height, GR_COORD hotx,
			GR_COORD hoty, GR_COLOR foreground, GR_COLOR background,
			GR_BITMAP *fgbitmap, GR_BITMAP *bgbitmap);

/**
 * Destroy a server-based cursor resource.
 *
 * <B>LGE-Specific</B> : We don't use this feature. The mouse driver is empty.
 */
void		GrDestroyCursor(GR_CURSOR_ID cid);

/**
 * Specify a cursor for a window.
 * This cursor will only be used within that window, and by default
 * for its new children.  If the cursor is currently within this
 * window, it will be changed to the new one immediately.
 * If the new cursor id is 0, revert to the root window cursor.
 *
 * <B>LGE-Specific</B> : We don't use this feature. The mouse driver is empty.
 */
void		GrSetWindowCursor(GR_WINDOW_ID wid, GR_CURSOR_ID cid);

/**
 * Sets the bounding region of the specified window, not
 * to be confused with a GC clip region.  The bounding region
 * is used to implement non-rectangular windows.
 * A window is defined by two regions: the bounding region
 * and the clip region.  The bounding region defines the area
 * within the parent window that the window will occupy, including
 * border.  The clip region is the subset of the bounding region
 * that is available for subwindows and graphics.  The area between
 * the bounding region and the clip region is defined to be the
 * border of the window.
 * Currently, only the window bounding region is implemented.
 *
 * <B>LGE-Specific</B> : To implement a non-rectangular(transparent) window or translucent window,
 *                GR_WM_PROPS_TRANSPARENT is more useful. See GrNewWindowEx() or GrSetWMProperties() for more information.
 *
 * @param wid [IN] the ID of the window.
 * @param rid [IN] region id
 * @param type [IN] region type. <B>T.J.Park Note</B> : This value is not used now.
 */
void		GrSetWindowRegion(GR_WINDOW_ID wid, GR_REGION_ID rid, int type);

/**
 * Move the cursor to the specified absolute screen coordinates.
 * The coordinates are that of the defined hot spot of the cursor.
 * The cursor's appearance is changed to that defined for the window
 * in which the cursor is moved to.  In addition, mouse enter, mouse
 * exit, focus in, and focus out events are generated if necessary.
 * The current mouse location is also changed.
 *
 * <B>LGE-Specific</B> : We don't use this feature. The mouse driver is empty.
 */
void		GrMoveCursor(GR_COORD x, GR_COORD y);

/**
 * Return the system palette entries of the first screen device.
 * Before calling this function, the pal->count must be set.
 *
 * @param pal [OUT] the pointer of the palette. This can not be NULL.
 */
void		GrGetSystemPalette(GR_PALETTE *pal);

/**
 * Return the system palette entries of the specified screen device.
 * Before calling this function, the pal->count must be set.
 *
 * <B>LGE-Specific</B> : This function is not defined in the original MicroWindows 0.90.
 * This function is added in order to support multiple screen devices. <p>
 * About multiple screen devices, refer to GrOpen(), GrCopyOffscreen(), and GrCopyOffscreenEx().
 *
 * @param layer [IN] 0 for the first OSD, 1 for the second OSD, etc. This value must be smaller than MAX_OSD_LAYER.
 * @param pal [OUT] the pointer of the palette. This can not be NULL.
 */
void		GrGetSystemPaletteEx(int layer, GR_PALETTE *pal);

/**
 * Set the system palette entries of the first screen device.
 *
 * @param first [IN] the first index of the entry to set
 * @param pal [IN] the pointer of the palette. This can not be NULL.
 */
void		GrSetSystemPalette(GR_COUNT first, GR_PALETTE *pal);

/**
 * Set the system palette entries of the specified screen device.
 *
 * <B>LGE-Specific</B> : This function is not defined in the original MicroWindows 0.90.
 * This function is added in order to support multiple screen devices. <p>
 * About multiple screen devices, refer to GrOpen(), GrCopyOffscreen(), and GrCopyOffscreenEx().
 *
 * @param layer [IN] 0 for the first OSD, 1 for the second OSD, etc. This value must be smaller than MAX_OSD_LAYER.
 * @param first [IN] the first index of the entry to set
 * @param pal [IN] the pointer of the palette. This can not be NULL.
 */
void		GrSetSystemPaletteEx(int layer, GR_COUNT first, GR_PALETTE *pal);

/**
 * Convert a system-independent color to a system dependent color.
 *
 * @param c [IN] a system-independent color. 0xAABBGGRR.
 * @param retpixel [OUT] pointer of the system-dependent color to be returned. This can not be NULL.
 */
void		GrFindColor(GR_COLOR c, GR_PIXELVAL *retpixel);

/**
 * Convert a system-independent color to a system dependent color, associated with the specified screen device.
 *
 * <B>LGE-Specific</B> : This function is not defined in the original MicroWindows 0.90.
 * This function is added in order to support multiple screen devices. <p>
 * About multiple screen devices, refer to GrOpen(), GrCopyOffscreen(), and GrCopyOffscreenEx().
 *
 * @param layer [IN] 0 for the first OSD, 1 for the second OSD, etc. This value must be smaller than MAX_OSD_LAYER.
 * @param c [IN] a system-independent color. 0xAABBGGRR.
 * @param retpixel [OUT] pointer of the system-dependent color to be returned. This can not be NULL.
 */
void		GrFindColorEx(int layer, GR_COLOR c, GR_PIXELVAL *retpixel);

/**
 * Requests a shared memory area of the specified size to use for transferring
 * command arguments. This is faster but less portable than the standard BSD
 * sockets method of communication (and of course will only work if the client
 * and server are on the same machine). Apart from the initial allocation of
 * the area using this call, the use of shared memory is completely
 * transparent. Additionally, if the allocation fails we silently and
 * automatically fall back on socket communication. It is safe to call this
 * function even if shared memory support is not compiled in; it will simply
 * do nothing.
 *
 * - todo FIXME: how does the user decide what size of shared memory area to allocate?
 *
 * <B>LGE-Specific</B> : This function works only in client-server model. Also this function is not defined in all operating systems.
 *
 * @param shmsize  the size of the shared memory area to allocate
 */
void		GrReqShmCmds(long shmsize);

/**
 * Generate a mouse event(including the button click).
 *
 * The third parameter, button, is one of the followings.
 * <TABLE>
 * GR_BUTTON_R			right button
 * GR_BUTTON_M			middle button
 * GR_BUTTON_L			left button
 * </TABLE>
 *
 * <B>LGE-Specific</B> : We don't use this feature. The mouse driver is empty.
 *
 * @param x [IN] the x-coordinate of the mouse pointer
 * @param y [IN] the y-coordinate of the mouse pointer
 * @param button [IN] See Description.
 * @param visible [IN] If 0, no cursor state changes. If 1, the cursor will be visible. Otherwise(not 0 and not 1), the cursor will be hidden.
 */
void		GrInjectPointerEvent(MWCOORD x, MWCOORD y,
			int button, int visible);

/**
 * Generate a keyboard event.
 *
 * <B>LGE-Specific</B> : We don't use this feature. The keyboard driver is empty.
 */
void		GrInjectKeyboardEvent(GR_WINDOW_ID wid, GR_KEY keyvalue,
			GR_KEYMOD modifiers, GR_SCANCODE scancode,
			GR_BOOL pressed);

/**
 * Generate a close event to the client who owns the given window.
 *
 * <B>LGE-Specific</B> : Because all windows are created with no border and no title bar, this function will not be used.
 *
 * @param wid [IN] the ID of the window.
 */
void		GrCloseWindow(GR_WINDOW_ID wid);

/**
 * Forcibly kill the connection to the client who owns the given window.
 *
 * <B>LGE-Specific</B> : Because we don't use client-server model, this function will not be used.
 */
void		GrKillWindow(GR_WINDOW_ID wid);

/**
 * Set the time-out of the screen saver.
 *
 * <B>LGE-Specific</B> : We don't use this feature.
 */
void		GrSetScreenSaverTimeout(GR_TIMEOUT timeout);

/**
 * Sets the current selection (otherwise known as the clipboard) ownership
 * to the specified window. Specifying an owner of 0 disowns the selection.
 * The typelist argument is a list of mime types (seperated by space
 * characters) which the window is able to supply the data as. At least one
 * type must be specified unless you are disowning the selection (typically
 * text/plain for plain ASCII text or text/uri-list for a filename).
 *
 * The window which owns the current selection must be prepared to handle
 * SELECTION_LOST events (received when another window takes ownership of the
 * selection) and CLIENT_DATA_REQ events (received when a client wishes to
 * retreive the selection data).
 *
 * <B>LGE-Specific</B> : We don't use this function.
 *
 * @param wid [IN] the ID of the window to set the selection owner to
 * @param typelist [IN] list of mime types selection data can be supplied as
 */
void		GrSetSelectionOwner(GR_WINDOW_ID wid, GR_CHAR *typelist);

/**
 * Finds the window which currently owns the selection and returns its ID,
 * or 0 if no window currently owns the selection. A pointer to the list of
 * mime types the selection owner is capable of supplying is placed in the
 * pointer specified by the typelist argument. The typelist is null terminated,
 * and the fields are seperated by space characters. It is the callers
 * responsibility to free the typelist string, as it is allocated dynamically.
 * If the allocation fails, it will be set to a NULL pointer, so remember to
 * check the value of it before using it.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 *
 * @param typelist [IN] pointer used to return the list of available mime types
 * @return the ID of the window which currently owns the selection, or 0
 */
GR_WINDOW_ID	GrGetSelectionOwner(GR_CHAR **typelist);

/**
 * Sends a CLIENT_DATA_REQ event to the specified window. Used for requesting
 * both selection and "drag and drop" data. The mimetype argument specifies
 * the format of the data you would like to receive, as an index into the list
 * returned by GrGetSelectionOwner (the first type in the list is index 0).
 * The server makes no guarantees as to when, or even if, the client will
 * reply to the request. If the client does reply, the reply will take the
 * form of one or more CLIENT_DATA events. The request serial number is
 * typically a unique ID which the client can assign to a request in order for
 * it to be able to keep track of transfers (CLIENT_DATA events contain the
 * same number in the sid field). Remember to free the data field of the
 * CLIENT_DATA events as they are dynamically allocated. Also note that if
 * the allocation fails the data field will be set to NULL, so you should
 * check the value before using it.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 *
 * @param wid  the ID of the window requesting the data
 * @param rid  the ID of the window to request the data from
 * @param serial  the serial number of the request
 * @param mimetype  the number of the desired mime type to request
 */
void		GrRequestClientData(GR_WINDOW_ID wid, GR_WINDOW_ID rid,
			GR_SERIALNO serial, GR_MIMETYPE mimetype);

/**
 * Used as the response to a CLIENT_DATA_REQ event. Sends the specified data
 * of the specified length to the specified window using the specified source
 * window ID and transfer serial number. Any fragmentation of the data into
 * multiple CLIENT_DATA events which is required is handled automatically.
 * The serial number should always be set to the value supplied by the
 * CLIENT_DATA_REQ event. The thislen parameter is used internally to split
 * the data up into packets. It should be set to the same value as the len
 * parameter.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 *
 * @param wid     The ID of the window sending the data.
 * @param did     The ID of the destination window.
 * @param serial  The serial number of the request.
 * @param len     Number of bytes of data to transfer.
 * @param thislen Number of bytes in this packet.
 * @param data    Pointer to the data to transfer.
 */
void		GrSendClientData(GR_WINDOW_ID wid, GR_WINDOW_ID did,
			GR_SERIALNO serial, GR_LENGTH len, GR_LENGTH thislen,
			void *data);

/**
 * Generate a beep sound.
 *
 * <B>LGE-Specific</B> : Not all the operating systems support this function. We don't use this function.
 */
void		GrBell(void);

/**
 * Set a window's background pixmap.  Note that this doesn't
 * cause a screen refresh, use GrClearWindow if required.
 *
 * The flags is one of the following values.
 * <TABLE>
 * GR_BACKGROUND_TILE			Tile across the window
 * GR_BACKGROUND_CENTER			Draw in center of window
 * GR_BACKGROUND_TOPLEFT		Draw at top left of window
 * GR_BACKGROUND_STRETCH		Stretch image to fit window
 * GR_BACKGROUND_TRANS			Don't fill in gaps
 * </TABLE>
 *
 * <B>LGE-Specific</B> : We don't use this function. All windows are created with no background.
 *
 * @param wid [IN] the ID of the window.
 * @param pixmap [IN] the background pixmap.
 * @param flags [IN] See Description.
 */
void		GrSetBackgroundPixmap(GR_WINDOW_ID wid, GR_WINDOW_ID pixmap,
			int flags);

/**
 * Returns the current information for the mouse pointer.
 *
 * <B>LGE-Specific</B> : We don't use this function. The mouse driver is empty.
 *
 * @param mwin	Window the mouse is current in
 * @param x	Current X pos of mouse (from root)
 * @param y	Current Y pos of mouse (from root)
 * @param bmask Current button mask
 */
void		GrQueryPointer(GR_WINDOW_ID *mwin, GR_COORD *x, GR_COORD *y,
			GR_BUTTON *bmask);
/**
 * Return window parent and list of children.
 * Caller must free() children list after use.
 *
 * <B>LGE-Specific</B> : This function assumes the memory is allocated by malloc().
 *                But, it may not be true in LGE version. We may use partition in order to
 *                avoid fragmentation. It is recommended that you use the GrGetWindowInfo()
 *                to get the child and the parent window info.
 *
 * @param wid [IN] the ID of the window.
 * @param parentid [OUT] the pointer of the parent window to be returned.
 * @param children [OUT] the pointer of the pointer of the children list to be returned. The memory will be allocated by the Nano-X.
 * @param nchildren [OUT] the pointer of the children count to be returned.
 */
void		GrQueryTree(GR_WINDOW_ID wid, GR_WINDOW_ID *parentid,
			GR_WINDOW_ID **children, GR_COUNT *nchildren);

/**
 * Grab a key for a specific window.
 *
 * <B>LGE-Specific</B> : We don't use this function. The keyboard driver is empty.
 *
 * @param id Window to send event to.
 * @param key MWKEY value.
 * @param type The type of reservation to make.  Valid values are
 *             #GR_GRAB_HOTKEY_EXCLUSIVE,
 *             #GR_GRAB_HOTKEY,
 *             #GR_GRAB_EXCLUSIVE and
 *             #GR_GRAB_EXCLUSIVE_MOUSE.
 * @return #GR_TRUE on success, #GR_FALSE on error.
 */
GR_BOOL         GrGrabKey(GR_WINDOW_ID wid, GR_KEY key, int type);

/**
 * Ungrab a key for a specific window.
 *
 * <B>LGE-Specific</B> : We don't use this function. The keyboard driver is empty.
 *
 * @param id window to stop key grab.
 * @param key MWKEY value.
 */
void            GrUngrabKey(GR_WINDOW_ID wid, GR_KEY key);

/**
 * Creates a Nano-X timer with the specified period.
 * NOTE: There is a potential for more GR_TIMER_EVENTS to be queued
 * in the connection between the Nano-X server and client.  The client
 * should be able to handle late arriving GR_TIMER_EVENTs.
 *
 * <B>LGE-Specific</B> : Do not use this function. Use your own timer. This function may not
 * work in all operating systems.
 *
 * @param wid the ID of the window to use as a destination for GR_TIMER_EVENT
 *	events that result from this timer.
 * @param period the timer period in milliseconds
 * @return the ID of the newly created timer, or 0 if failure.
 */
GR_TIMER_ID	GrCreateTimer(GR_WINDOW_ID wid, GR_TIMEOUT period);

/**
 * Destroys a timer previously created with GrCreateTimer().
 *
 * <B>LGE-Specific</B> : Do not use this function. Use your own timer. This function may not
 * work in all operating systems.
 *
 * @param tid the ID of the timer to destroy
 */
void		GrDestroyTimer(GR_TIMER_ID tid);

/**
 * Sets new server portrait mode and redraws all windows.
 *
 * <B>LGE-Specific</B> : Portrait mode is supported for PDA. We don't use this feature.
 *
 * @param portraitmode New portrait mode.
 */
void		GrSetPortraitMode(int portraitmode);

/**
 * Register the specified file descriptor to return an event
 * when input is ready.
 *
 * <B>LGE-Specific</B> : We don't use this function. The exact meaning is unknown.
 */
void		GrRegisterInput(int fd);

/**
 * Unregister the specified file descriptor.
 *
 * <B>LGE-Specific</B> : We don't use this function. The exact meaning is unknown.
 */
void		GrUnregisterInput(int fd);

/**
 * Make a infinite loop for Nano-X, repeatedly calling GrGetNextEvent().
 *
 * <B>LGE-Specific</B> : We don't use this function.
 */
void		GrMainLoop(GR_FNCALLBACKEVENT fncb);

/**
 * Set an error handling routine, which will be called on any errors from
 * the server (when events are asked for by the client).  If zero is given,
 * then errors will be returned as regular events.
 * Returns the previous error handler.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 */
GR_FNCALLBACKEVENT GrSetErrorHandler(GR_FNCALLBACKEVENT fncb);
void		GrDefaultErrorHandler(GR_EVENT *ep);

/**
 * Prepare for the client to call select().  Asks the server to send the next
 * event but does not wait around for it to arrive.  Initializes the
 * specified fd_set structure with the client/server socket descriptor and any
 * previously registered external file descriptors.  Also compares the current
 * contents of maxfd, the client/server socket descriptor, and the previously
 * registered external file descriptors, and returns the highest of them in
 * maxfd.
 *
 * Usually used in conjunction with GrServiceSelect().
 *
 * This function is available with client/server only.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 *
 * Note that in a multithreaded client, the application must ensure that
 * no Nano-X calls are made between the calls to GrPrepareSelect() and
 * GrServiceSelect(), else there will be race conditions.
 *
 * @param maxfd  Pointer to a variable which the highest in use fd will be
 *               written to.  Must contain a valid value on input - will only
 *               be overwritten if the new value is higher than the old
 *               value.
 * @param rfdset Pointer to the file descriptor set structure to use.  Must
 *               be valid on input - file descriptors will be added to this
 *               set without clearing the previous contents.
 */
void		GrPrepareSelect(int *maxfd,void *rfdset);

/**
 * Handles events after the client has done a select() call.
 *
 * Calls the specified callback function is an event has arrived, or if
 * there is data waiting on an external fd specified by GrRegisterInput().
 *
 * Used by GrMainLoop().
 *
 * This function is available with client/server only.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 *
 * @param rfdset Pointer to the file descriptor set containing those file
 *               descriptors that are ready for reading.
 * @param fncb   Pointer to the function to call when an event needs handling.
 */
void		GrServiceSelect(void *rfdset, GR_FNCALLBACKEVENT fncb);

/**
 * Returns the current length of the client side queue.
 *
 * This function is available with client/server only.
 *
 * <B>LGE-Specific</B> : We don't use this function.
 *
 * @return The current length of the client side queue.
 */
int			GrQueueLength(void);

/**
 * This passes transform data to the mouse input engine.
 *
 * <B>LGE-Specific</B> : We don't use this function. The mouse driver is empty.
 *
 * @param trans A GR_TRANSFORM structure that contains the transform data
 *              for the filter, or NULL to disable.
 */
void		GrSetTransform(GR_TRANSFORM *);

/* nxtransform.c - mouse utility routines (requires floating point)*/

/**
 * <B>LGE-Specific</B> : We don't use this function. The mouse driver is empty.
 */
int			GrCalcTransform(GR_CAL_DATA *, GR_TRANSFORM *);

/**
 * <B>LGE-Specific</B> : We don't use this function. The mouse driver is empty.
 */
int			GrLoadTransformData(char *filename, GR_TRANSFORM *);

/**
 * <B>LGE-Specific</B> : We don't use this function. The mouse driver is empty.
 */
int			GrSaveTransformData(GR_TRANSFORM *, char *filename);

/* nxutil.c - utility routines*/

/**
 * An advanced version of GrNewWindow(). You can set the window properties and the title text.
 *
 * <B>LGE-Specific</B> : The bordersize shall be always 0, and background and border color shall be ignored.
 * In order to ignore the background color, you must call GrNewWindowEx() instead of GrNewWindow(),
 * and put the first parameter as GR_WM_PROPS_NOBACKGROUND.
 * Also, if you have multiple screen devices, multiple root windows are also available. For example,
 * if you have two screen devices, you can create a window under GR_ROOT_WINDOW2_ID, not only under GR_ROOT_WINDOW_ID.
 * If you have three screen devices, define more GR_ROOT_WINDOWXX_ID in nano-X.h
 *
 * In order to support multiple root windows, the ID of the normal windows starts from 20, not 2.
 * This means that you can define up to 19 root windows.
 *
 * <B>LGE-Specific</B> : GR_WM_PROPS_TRANSPARENT is newly added in the window properties. The original MicroWindows 0.90 does not
 * support a 'transparent' window. If a window is 'transparent', then windows below it at the position
 * where the original window was initially placed are not obscured and show through, i.e, a transparent window is not
 * clipped off when its siblings and its parent are repainted. Instead, an exposure event is sent to the calling client.
 * Through this way, a transparent or translucent window(through GR_MODE_SRC_OVER) can be implemented.
 * When using a 'transparent' window, the client must handle the exposure event, and the screen device driver must
 * support the system level back-buffer in order to avoid flickering.
 *
 * @param props [IN] the window properties. See GR_WM_PROPS for more information.
 * @param title [IN] the text of the title string.
 * @param parent [IN] the ID of the parent window. If the parent is the root window, use GR_ROOT_WINDOW_ID(or GR_ROOT_WINDOW2_ID).
 * @param x [IN] the x-coordinate of the window's left-top position, described in the parent window's domain.
 * @param y [IN] the y-coordinate of the window's left-top position, described in the parent window's domain.
 * @param width [IN] the width of the window.
 * @param height [IN] the height of the window.
 * @param bordersize [IN] the width of the border. (<B>LGE-Specific</B> : This value must be 0).
 * @param background [IN] the background color. If GR_WM_PROPS_NOBACKGROUND is set for this window, this color will be ignored.
 * @param bordercolor [IN] the border color. If bordersize is 0, this value will be ignored.
 * @return the ID of the window. (<B>LGE-Specific</B> : This value is always bigger than 20).
 */
GR_WINDOW_ID	GrNewWindowEx(GR_WM_PROPS props, GR_CHAR *title,
			GR_WINDOW_ID parent, GR_COORD x, GR_COORD y,
			GR_SIZE width, GR_SIZE height, GR_COLOR background);
/**
 * Draw an array of lines.
 *
 * <B>T.J.Park Note</B> : I don't know the difference between this function and GrPoly.
 *
 * @param w [IN] the ID of the window.
 * @param gc [IN] the ID of the GC.
 * @param points [IN] the pointer of the point array. This can not be NULL.
 * @param count [IN] the number of the points.
 */
void		GrDrawLines(GR_DRAW_ID w, GR_GC_ID gc, GR_POINT *points,
			GR_COUNT count);

/**
 * Create a GdBitmap-compatible bitmap (16-bit short array) from data bits flags specify input format.
 * Caller must free return buffer.
 *
 * Currently only works if width/height < bits_width/bits_height.
 */
GR_BITMAP *	GrNewBitmapFromData(GR_SIZE width, GR_SIZE height, GR_SIZE bits_width,
			GR_SIZE bits_height, void *bits, int flags);

/**
 * Create a new pixmap and initialize from bitmap data and fg/bg colors.
 */
GR_WINDOW_ID    GrNewPixmapFromData(GR_SIZE width, GR_SIZE height,
			GR_COLOR foreground, GR_COLOR background, void * bits,
			int flags);
/**
 * Create a bitmap from a specified pixmap
 * This function may not be needed if Microwindows implemented a depth-1 pixmap.
 */
GR_BITMAP *	GrNewBitmapFromPixmap(GR_WINDOW_ID pixmap, int x, int y, GR_SIZE width,
			GR_SIZE height);

/**
 * Create a region from a monochrome pixmap
 */
GR_REGION_ID	GrNewRegionFromPixmap(GR_WINDOW_ID src, MWCOORD x, MWCOORD y,
			GR_SIZE width, GR_SIZE height);

/* direct client-side framebuffer mapping routines*/

/**
 * Map framebuffer address into client memory.
 *
 * <B>LGE-Specific</B> : This function is not defined in this version.
 *
 * @return Pointer to start of framebuffer,
 * or NULL if framebuffer not directly accessible by client.
 */
unsigned char * GrOpenClientFramebuffer(void);

/**
 * Unmap framebuffer, if mapped.
 *
 * <B>LGE-Specific</B> : This function is not defined in this version.
 */
void		GrCloseClientFramebuffer(void);

/**
 * Return client-side mapped framebuffer info for
 * passed window.  If not running framebuffer, the
 * physpixel and winpixel members will be NULL, and
 * everything else correct.
 *
 * <B>LGE-Specific</B> : This function is not defined in this version.
 *
 * @param wid    Window to query
 * @param fbinfo Structure to store results.
 */
void		GrGetWindowFBInfo(GR_WINDOW_ID wid, GR_WINDOW_FB_INFO *fbinfo);

/*
 * Retrofit routine.  Use GrNewCursor and GrSetWindowCursor for new code.
 */
GR_CURSOR_ID	GrSetCursor(GR_WINDOW_ID wid, GR_SIZE width, GR_SIZE height,
			GR_COORD hotx, GR_COORD hoty, GR_COLOR foreground,
			GR_COLOR background, GR_BITMAP *fbbitmap,
			GR_BITMAP *bgbitmap);

#define GrSetBorderColor		GrSetWindowBorderColor	/* retrofit*/
#define GrClearWindow(wid,exposeflag)	GrClearArea(wid,0,0,0,0,exposeflag) /* retrofit*/

/* useful function macros*/
#define GrSetWindowBackgroundColor(wid,color) \
		{	GR_WM_PROPERTIES props;	\
			props.flags = GR_WM_FLAGS_BACKGROUND; \
			props.background = color; \
			GrSetWMProperties(wid, &props); \
		}
#define GrSetWindowBorderSize(wid,width) \
		{	GR_WM_PROPERTIES props;	\
			props.flags = GR_WM_FLAGS_BORDERSIZE; \
			props.bordersize = width; \
			GrSetWMProperties(wid, &props); \
		}
#define GrSetWindowBorderColor(wid,color) \
		{	GR_WM_PROPERTIES props;	\
			props.flags = GR_WM_FLAGS_BORDERCOLOR; \
			props.bordercolor = color; \
			GrSetWMProperties(wid, &props); \
		}
#define GrSetWindowTitle(wid,name) \
		{	GR_WM_PROPERTIES props;	\
			props.flags = GR_WM_FLAGS_TITLE; \
			props.title = (GR_CHAR *)name; \
			GrSetWMProperties(wid, &props); \
		}

#ifdef __cplusplus
}
#endif

/* RTEMS requires rtems_main()*/
#if __rtems__
#define main	rtems_main
#endif

/* client side event queue (client.c local)*/
typedef struct event_list EVENT_LIST;
struct event_list {
	EVENT_LIST *	next;
	GR_EVENT	event;
};

/* queued request buffer (nxproto.c local)*/
typedef struct {
	unsigned char *bufptr;		/* next unused buffer location*/
	unsigned char *bufmax;		/* max buffer location*/
	unsigned char *buffer;		/* request buffer*/
} REQBUF;

#ifdef __ECOS
#include <sys/select.h>
#include <cyg/kernel/kapi.h>
/*
 * In a single process, multi-threaded environment, we need to keep
 * all static data of shared code in a structure, with a pointer to
 * the structure to be stored in thread-local storage
 */
typedef struct {                                /* Init to: */
    int                 _nxSocket;              /*  -1 */
    MWMUTEX	 	_nxGlobalLock;
    int                 _storedevent;           /* 0 */
    GR_EVENT            _storedevent_data;      /* no init(0) */
    int                 _regfdmax;              /* -1 */
    fd_set		_regfdset;		/* FD_ZERO */
    GR_FNCALLBACKEVENT  _GrErrorFunc;           /* GrDefaultErrorHandler */
    REQBUF              _reqbuf;
    EVENT_LIST          *_evlist;
} ecos_nanox_client_data;

extern int     ecos_nanox_client_data_index;

#define ACCESS_PER_THREAD_DATA()                                        \
    ecos_nanox_client_data *data = (ecos_nanox_client_data*)            \
        cyg_thread_get_data((cyg_ucount32)ecos_nanox_client_data_index);

#define INIT_PER_THREAD_DATA()                                                  \
    {                                                                           \
        ecos_nanox_client_data *dptr = malloc(sizeof(ecos_nanox_client_data));  \
        ecos_nanox_client_data_index = data;                                    \
        dptr->_nxSocket = -1;                                                   \
	dptr->nxGlobalLock = 0;
        dptr->_storedevent = 0;                                                 \
        dptr->_regfdmax = -1;                                                   \
        FD_ZERO(&dptr->_regfdset);                                              \
        dptr->_GrErrorFunc = GrDefaultErrorHandler;                             \
        dptr->_reqbuf.bufptr = NULL;                                            \
        dptr->_reqbuf.bufmax = NULL;                                            \
        dptr->_reqbuf.buffer = NULL;                                            \
        dptr->_evlist = NULL;                                                   \
        cyg_thread_set_data(ecos_nanox_client_data_index,(CYG_ADDRWORD)dptr);   \
    }

#define nxSocket                (data->_nxSocket)
#define nxGlobalLock            (data->_nxGlobalLock)
#define storedevent             (data->_storedevent)
#define storedevent_data        (data->_storedevent_data)
#define regfdmax                (data->_regfdmax)
#define regfdset                (data->_regfdset)
#define ErrorFunc               (data->_GrErrorFunc)
#define reqbuf                  (data->_reqbuf)
#define evlist                  (data->_evlist)

#else
#define ACCESS_PER_THREAD_DATA()
#endif

/**
 * <B>LGE-Specific</B> : This function opens OSD Update mutex.
 */
extern void NANOX_ScrLock(void);
/**
 * <B>LGE-Specific</B> : This function closes OSD Update mutex.
 */
extern void NANOX_ScrUnlock(void);

/**
 * <B>LGE-Specific</B> : This function closes OSD Update mutex.
 * Reverse Mode (Arabic)
 */
extern void NANOX_ScreenReverseOn(void);
extern void NANOX_ScreenReverseOff(void);
extern int NANOX_IsScreenReversed(void);

#endif /* _NANO_X_H*/
