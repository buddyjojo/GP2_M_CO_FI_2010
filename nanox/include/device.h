#ifndef _DEVICE_H
#define _DEVICE_H
/*
 * Copyright (c) 1999, 2000, 2001, 2002, 2003 Greg Haerr <greg@censoft.com>
 * Portions Copyright (c) 2002 Koninklijke Philips Electronics
 *
 * Engine-level Screen, Mouse and Keyboard device driver API's and types
 *
 * Contents of this file are not for general export
 */
#include "mwtypes.h"				/* public export typedefs*/
#include "mwsystem.h"

/* Changeable limits and options*/
#define ALPHABLEND	1			/* =1 to include blending code*/
#define DYNAMICREGIONS	1		/* =1 to use MWCLIPREGIONS*/
#define HAVEFLOAT	1			/* =1 incl float, GdArcAngle*/
#define POLYREGIONS	1			/* =1 includes polygon regions*/
#define ANIMATEPALETTE	0		/* =1 animated palette test*/

#if HAVE_FONTFUSION_SUPPORT | HAVE_UTF_SUPPORT | HAVE_UNITYPE_SUPPORT
#define FONTMAPPER  0           /* =1 for Morten's font mapper*/
#define FONT_INSTALLER 1		/* =1 for T.J.Park's font installer */
#else
#define FONTMAPPER	0			/* =1 for Morten's font mapper*/
#define FONT_INSTALLER 0		/* =1 for T.J.Park's font installer */
#endif

#define USE_ALLOCA	0			/* =1 when alloca() is available */
#define FASTJPEG	0			/* KDH ,  =1 for temp quick jpeg 8bpp*/
#define HAVE_MMAP   0      		/* =1 to use mmap if available*/

#define MAX_OSD_LAYER	2		/* Support multiple OSD. You must implement device drivers for each OSD layer. */
extern int scrNumberOfLayers;	/* the number of layers to be used */

#define MW_FEATURE_IMAGES 1		/* =1 to enable GdLoadImage* / GdDrawImage* */
//#define MW_FEATURE_IMAGES 0		/* platform doesn't support images*/

//#define MW_FEATURE_TIMERS 1		/* =1 to include MWTIMER support */
#define MW_FEATURE_TIMERS 0		/* Other platforms do not support timers yet */

/* determine compiler capability for handling EPRINTF/DPRINTF macros*/
#ifndef MW_FEATURE_GDERROR
#define MW_FEATURE_GDERROR	0		/* use fprintf instead of GdError*/
//#define MW_FEATURE_GDERROR	1		/* use GdError for errors*/
#endif /* MW_FEATURE_GDERROR*/

#if MW_FEATURE_GDERROR
#define EPRINTF			GdError		/* error output*/
#if DEBUG
#define DPRINTF			GdError		/* debug output*/
#else
#define DPRINTF			GdErrorNull	/* no debug output*/
#endif
#else

/* GCC compiler-only macro magic to save space */
#include <stdio.h>    /* For stderr */
#ifndef _EMUL_WIN
//#define EPRINTF(str, args...)   fprintf(stderr, str, ##args)  /* error output*/
#define EPRINTF(str, args...)	/* no debug output*/
//#define DPRINTF(str, args...)   fprintf(stderr, str, ##args)  /* debug output*/
#define DPRINTF(str, ...)	/* no debug output*/
#else
#define EPRINTF
#define DPRINTF
#endif
#endif /* MW_FEATURE_GDERROR*/

/* To avoid overflow : For Java AWT test */
//#define SAFE_PLUS(x, width)		((((x) > 0) && ((width) > 0) && ((x) + (width) < 0)) ? (0x7FFFFFFF) : ((x) + (width)))
#define SAFE_PLUS(x, width)     ((((x) > 0) && ((width) > 0) && ((unsigned int)(x) + (unsigned int)(width) > 0x7FFFFFFF)) ? (0x7FFFFFFF) : ((x) + (width)))

/* Which low-level psd->DrawArea routines to include. */
#define MW_FEATURE_PSDOP_COPY                   0
#if HAVE_FONTFUSION_SUPPORT | HAVE_UTF_SUPPORT | HAVE_UNITYPE_SUPPORT
#define MW_FEATURE_PSDOP_ALPHAMAP               1
#else
#define MW_FEATURE_PSDOP_ALPHAMAP               0
#endif
#define MW_FEATURE_PSDOP_ALPHACOL               0
#define MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST 0
#define MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST 0

/* max char height/width must be >= 16 and a multiple of sizeof(MWIMAGEBITS)*/
#define MAX_CHAR_HEIGHT	128			/* maximum text bitmap height*/
#define MAX_CHAR_WIDTH	128			/* maximum text bitmap width*/
#define	MIN_MWCOORD	((MWCOORD) -32768)	/* minimum coordinate value */
#define	MAX_MWCOORD	((MWCOORD) 32767)	/* maximum coordinate value */
#define	MAX_CLIPRECTS 	200			/* max clip rects (obsolete)*/

/* Override some of the above defines, for features which are required
 * for the Microwindows FreeType 2 font driver
 */
#ifdef HAVE_FREETYPE_2_SUPPORT
#undef  MW_FEATURE_PSDOP_ALPHACOL
#define MW_FEATURE_PSDOP_ALPHACOL 1
#undef  MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST
#define MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST 1
#endif

/* Sanity check: VTSWITCH involves a timer. */
//#if VTSWITCH && !MW_FEATURE_TIMERS
//#error VTSWITCH depends on MW_FEATURE_TIMERS - disable VTSWITCH in config or enable MW_FEATURE_TIMERS in this file
//#endif

typedef struct _mwscreendevice *PSD;

/* builtin C-based proportional/fixed font structure*/
typedef struct {
	char *		name;		/* font name*/
	int		maxwidth;	/* max width in pixels*/
	unsigned int	height;		/* height in pixels*/
	int		ascent;		/* ascent (baseline) height*/
	int		firstchar;	/* first character in bitmap*/
	int		size;		/* font size in characters*/
	MWIMAGEBITS *	bits;		/* 16-bit right-padded bitmap data*/
	unsigned long *offset;		/* 256 offsets into bitmap data*/
	unsigned char *	width;		/* 256 character widths or 0 if fixed*/
	int		defaultchar;	/* default char (not glyph index)*/
	long		bits_size;	/* # words of MWIMAGEBITS bits*/
} MWCFONT, *PMWCFONT;

/* draw procs associated with fonts.  Strings are [re]packed using defencoding*/
typedef struct {
	MWTEXTFLAGS	encoding;	/* routines expect this encoding*/
	MWBOOL	(*GetFontInfo)(PMWFONT pfont, PMWFONTINFO pfontinfo);
	void 	(*GetTextSize)(PMWFONT pfont, const void *text, int cc,
			MWTEXTFLAGS flags, MWCOORD *pwidth, MWCOORD *pheight,
			MWCOORD *pbase);
	void	(*GetTextBits)(PMWFONT pfont, int ch, const MWIMAGEBITS **retmap,
			MWCOORD *pwidth, MWCOORD *pheight, MWCOORD *pbase);
	void	(*DestroyFont)(PMWFONT pfont);
	void	(*DrawText)(PMWFONT pfont, PSD psd, MWCOORD x, MWCOORD y,
			const void *str, int count, MWTEXTFLAGS flags);
	void    (*SetFontSize)(PMWFONT pfont, MWCOORD fontsize);
	void    (*SetFontRotation)(PMWFONT pfont, int tenthdegrees);
	void    (*SetFontAttr)(PMWFONT pfont, int setflags, int clrflags);
	PMWFONT (*Duplicate) (PMWFONT psrcfont, MWCOORD fontsize);
} MWFONTPROCS, *PMWFONTPROCS;

/* new multi-renderer font struct*/
typedef struct _mwfont {		/* common hdr for all font structures*/
	PMWFONTPROCS	fontprocs;	/* font-specific rendering routines*/
	MWCOORD		fontsize;	/* font height in pixels*/
	int		fontrotation;	/* font rotation*/
	int		fontattr;	/* font attributes: kerning/antialias*/
	/* font-specific rendering data here*/
} MWFONT;

/* builtin core font struct*/
typedef struct {
	PMWFONTPROCS	fontprocs;	/* common hdr*/
	MWCOORD		fontsize;
	int		fontrotation;
	int		fontattr;

	char *		name;		/* Microwindows font name*/
	PMWCFONT	cfont;		/* builtin font data*/
} MWCOREFONT, *PMWCOREFONT;


/* This structure is used to pass parameters into the low
 * level device driver functions.
 */
typedef struct {
	MWCOORD dstx, dsty, dstw, dsth, dst_linelen;
	MWCOORD srcx, srcy, src_linelen;
	void *pixels, *misc;
	MWPIXELVAL bg_color, fg_color;
	int gr_usebg;
	MWPIXELVAL colorval;
	unsigned char is1bitfont;	/* indicates 1bit mono bitmap font or not */
} driver_gc_t;

/* Operations for the Blitter/Area functions */

#if MW_FEATURE_PSDOP_COPY

/*
 * FIXME Not Documented - see drivers/fblin16.c
 */
#define PSDOP_COPY	0

/*
 * FIXME Not Documented - see drivers/fblin16.c
 */
#define PSDOP_COPYALL	1

/*
 * FIXME Not Documented - see drivers/fblin16.c
 */
#define PSDOP_COPYTRANS 2

#endif /* MW_FEATURE_PSDOP_COPY */

#if MW_FEATURE_PSDOP_ALPHAMAP
/*
 * Copy an image to screen, using an alpha map.
 * Params:
 * dstx, dsty  - Destination for top left of image
 * dstw, dsth  - Image size
 * srcx, srcy  - Start co-ordinates in source image
 * src_linelen - Source image stride, in pixels
 * pixels      - Image to copy from.  Format: same color model as display.
 * misc        - Alpha map.  Format: ADDR8, entries
 *               are alpha values in range 0-255.
 * gr_usebg    - Ignored.  FIXME If set, should blend to bg_color.
 * bg_color    - Ignored.  FIXME Should be used if gr_usebg is set.
 * fg_color    - Ignored.
 */
#define PSDOP_ALPHAMAP	3
#endif /* MW_FEATURE_PSDOP_ALPHAMAP */

#if MW_FEATURE_PSDOP_ALPHACOL
/*
 * Draws an alpha map to screen (e.g. an anti-aliased font).
 * Params:
 * dstx, dsty  - Destination for top left of image
 * dstw, dsth  - Image size
 * srcx, srcy  - Start co-ordinates in source alpha map
 * src_linelen - Source image stride, in pixels
 * misc        - Alpha map.  Format: ADDR8, entries
 *               are alpha values in range 0-255.
 * fg_color    - The color to draw in, in the display format.
 * gr_usebg    - Ignored.  FIXME If set, should blend to bg_color.
 * bg_color    - Ignored.  FIXME Should be used if gr_usebg is set.
 * pixels      - Ignored.
 */
#define PSDOP_ALPHACOL	4
#endif /* MW_FEATURE_PSDOP_ALPHACOL */

#if MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST
/*
 * Draws a mono bitmap to screen (e.g. a mono font).
 * This variant takes the bitmap as an array of bytes,
 * where the Least Significant Bit in each byte is
 * used to set the left-most of the eight pixels
 * controlled by that byte.  I.e:
 *
 * [ 1 1 1 1 0 0 0 1 ] == 0x8F
 *
 * Params:
 * dstx, dsty  - Destination for top left of image
 * dstw, dsth  - Image size
 * srcx, srcy  - Start co-ordinates in source alpha map
 * src_linelen - Source image stride, in pixels
 * pixels      - The bitmap.  Format: ADDR8, LSB is drawn first.
 * fg_color    - The color to draw "1" bits in, in the display format.
 * bg_color    - The color to draw "0" bits in, in the display format.
 * gr_usebg    - If zero, then "0" bits are transparent.  If nonzero,
 *               then "0" bits are bg_color.
 */
#define PSDOP_BITMAP_BYTES_LSB_FIRST	5
#endif /* MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST */

#if MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST
/*
 * Draws a mono bitmap to screen (e.g. a mono font).
 * This variant takes the bitmap as an array of bytes,
 * where the Most Significant Bit in each byte is
 * used to set the left-most of the eight pixels
 * controlled by that byte.  I.e:
 *
 * [ 1 1 1 1 0 0 0 1 ] == 0xF1
 *
 * Params:
 * dstx, dsty  - Destination for top left of image
 * dstw, dsth  - Image size
 * srcx, srcy  - Start co-ordinates in source alpha map
 * src_linelen - Source image stride, in pixels
 * pixels      - The bitmap.  Format: ADDR8, MSB is drawn first.
 * fg_color    - The color to draw "1" bits in, in the display format.
 * bg_color    - The color to draw "0" bits in, in the display format.
 * gr_usebg    - If zero, then "0" bits are transparent.  If nonzero,
 *               then "0" bits are bg_color.
 */
#define PSDOP_BITMAP_BYTES_MSB_FIRST	6
#endif /* MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST */

/* common blitter parameter structure*/
typedef struct {
	PSD		dstpsd;		/* dst drawable*/
	MWCOORD		dstx, dsty;	/* dst x,y,w,h*/
	MWCOORD		dstw, dsth;
	MWCOORD		srcx, srcy;	/* src x,y*/
	MWCOORD		srcw, srch;	/* src w,h if stretchblit*/
	PSD		srcpsd;		/* src drawable*/
	unsigned long 	rop;		/* raster opcode*/
	PSD		alphachan;	/* alpha chan for MWROP_BLENDCHANNEL*/
	MWPIXELVAL	fgcolor;	/* fg/bg color for MWROP_BLENDFGBG*/
	MWPIXELVAL	bgcolor;
	MWPIXELVAL	transcolor;	/* trans color for MWROP_SRCTRANSCOPY*/
} MWBLITARGS, *PMWBLITARGS;

/* screen subdriver entry points: one required for each draw function*/
/* NOTE: currently used for fb driver only*/
typedef struct {
	int	 (*Init)(PSD psd);
	void 	 (*DrawPixel)(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c);
	MWPIXELVAL (*ReadPixel)(PSD psd, MWCOORD x, MWCOORD y);
	void 	 (*DrawHorzLine)(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y,
			MWPIXELVAL c);
	void	 (*DrawVertLine)(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2,
			MWPIXELVAL c);
	void	 (*FillRect)(PSD psd,MWCOORD x1,MWCOORD y1,MWCOORD x2,
			MWCOORD y2,MWPIXELVAL c);
	void	 (*Blit)(PSD destpsd, MWCOORD destx, MWCOORD desty, MWCOORD w,
			MWCOORD h,PSD srcpsd,MWCOORD srcx,MWCOORD srcy,long op);
	void	 (*DrawArea)(PSD psd, driver_gc_t *gc, int op);

	/* Note: StretchBlit() is deprecated, callers should prefer
	 * StretchBlitEx() if available.  Drivers should just
	 * implement StretchBlitEx().
	 */
	void	 (*StretchBlit)(PSD destpsd, MWCOORD destx, MWCOORD desty,
			MWCOORD dstw, MWCOORD dsth, PSD srcpsd, MWCOORD srcx,
			MWCOORD srcy, MWCOORD srcw, MWCOORD srch, long op);
	void 	 (*StretchBlitEx) (PSD dstpsd, PSD srcpsd,
			MWCOORD dest_x_start, int dest_y_start,
			MWCOORD width, int height,
			int x_denominator, int y_denominator,
			int src_x_fraction, int src_y_fraction,
			int x_step_fraction, int y_step_fraction, long op);
} SUBDRIVER, *PSUBDRIVER;

/*
 * Interface to Screen Device Driver
 * This structure is also allocated for memory (offscreen) drawing and blitting.
 */
typedef struct _mwscreendevice {
	MWCOORD	xres;		/* X screen res (real) */
	MWCOORD	yres;		/* Y screen res (real) */
	MWCOORD	xvirtres;	/* X drawing res (will be flipped in portrait mode) */
	MWCOORD	yvirtres;	/* Y drawing res (will be flipped in portrait mode) */
	int	planes;		/* # planes*/
	int	bpp;		/* # bpp*/
	int	linelen;	/* line length in bytes for bpp 1,2,4,8*/
				/* line length in pixels for bpp 16, 24, 32*/
	int	size;		/* size of memory allocated*/
	long	ncolors;	/* # screen colors*/
	int	pixtype;	/* format of pixel value*/
	int	flags;		/* device flags*/
	void *	addr;		/* address of memory allocated (memdc or fb)*/

	int layer;

	PSD	(*Open)(PSD psd, int layer);
	void	(*Close)(PSD psd);
	void	(*GetScreenInfo)(PSD psd,PMWSCREENINFO psi);
	void	(*SetPalette)(PSD psd,int first,int count,MWPALENTRY *pal);
	void	(*DrawPixel)(PSD psd,MWCOORD x,MWCOORD y,MWPIXELVAL c);
	MWPIXELVAL (*ReadPixel)(PSD psd,MWCOORD x,MWCOORD y);
	void	(*DrawHorzLine)(PSD psd,MWCOORD x1,MWCOORD x2,MWCOORD y,
			MWPIXELVAL c);
	void	(*DrawVertLine)(PSD psd,MWCOORD x,MWCOORD y1,MWCOORD y2,
			MWPIXELVAL c);
	void	(*FillRect)(PSD psd,MWCOORD x1,MWCOORD y1,MWCOORD x2,MWCOORD y2,
			MWPIXELVAL c);
	PMWCOREFONT builtin_fonts;

	/* *void (*DrawText)(PSD psd,MWCOORD x,MWCOORD y,const MWUCHAR *str,
			int count, MWPIXELVAL fg, PMWFONT pfont);***/

	void	(*Blit)(PSD destpsd,MWCOORD destx,MWCOORD desty,MWCOORD w,
			MWCOORD h,PSD srcpsd,MWCOORD srcx,MWCOORD srcy,long op);
	void	(*PreSelect)(PSD psd);
	void	(*DrawArea)(PSD psd, driver_gc_t *gc, int op);
	int	(*SetIOPermissions)(PSD psd);
	PSD	(*AllocateMemGC)(PSD psd);
	MWBOOL	(*MapMemGC)(PSD mempsd,MWCOORD w,MWCOORD h,int planes,int bpp,
			int linelen,int size,void *addr);
	void	(*FreeMemGC)(PSD mempsd);
	void	(*StretchBlit)(PSD destpsd,MWCOORD destx,MWCOORD desty,
			MWCOORD destw,MWCOORD desth,PSD srcpsd,MWCOORD srcx,
			MWCOORD srcy,MWCOORD srcw,MWCOORD srch,long op);
	void	(*SetPortrait)(PSD psd,int portraitmode);
	int	portrait;	 /* screen portrait mode*/
	PSUBDRIVER orgsubdriver; /* original subdriver for portrait modes*/
	void 	(*StretchBlitEx) (PSD dstpsd, PSD srcpsd,
			MWCOORD dest_x_start, MWCOORD dest_y_start,
			MWCOORD width, MWCOORD height,
			int x_denominator, int y_denominator,
			int src_x_fraction, int src_y_fraction,
			int x_step_fraction, int y_step_fraction,
			long op);

	/* Update() copies the back-buffer image into the primary screen.
	   To use this back-buffer feature, psd must have PSF_HAVE_OFFSCREEN option. */
	void	(*Update) (PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD width, MWCOORD height);

	/* DrawImage copies the given image(located in the memory) to the given PSD.
	   This function does not need to be able to draw all image formats, instead,
	   you need to implement this function only for the image formats which is supported by the screen device.
	   For example, if the given screen device has 32 bit ARGB format, then you may implement DrawImage()
	   only for 32 bit ARGB format.
	   Before calling DrawImage(), the caller must check whether the given image format is supported by calling CanDrawImage(). */
	void	(*DrawImage)(PSD psd, MWCOORD destX, MWCOORD destY, MWCOORD width, MWCOORD height
								, PMWIMAGEHDR pimage, MWCOORD srcX, MWCOORD srcY, MWPIXELVAL *convtable);
	MWBOOL	(*CanDrawImage)(PSD psd, PMWIMAGEHDR pimage);

	/* Implement DrawChar() only when the screen device supports it. */
	void	(*DrawChar)(PSD psd, MWCOORD dstX, MWCOORD dstY
					, int glyphBitCount
					, unsigned char *glyph, int glyphLineLength, MWCOORD glyphX, MWCOORD glyphY
					, MWCOORD glyphW, MWCOORD glyphH
					, MWPIXELVAL c);

	/* Implement this if you need to use GrSetScreenRes() which supports changing of screen resolution in run-time */
	void	(*SetScreenRes)(PSD psd, MWCOORD width, MWCOORD height);
} SCREENDEVICE;

/* PSD flags*/
#define	PSF_SCREEN		0x0001	/* screen device*/
#define PSF_MEMORY		0x0002	/* memory device*/
#define PSF_HAVEBLIT		0x0004	/* have bitblit*/
#define PSF_HAVEOP_COPY		0x0008	/* psd->DrawArea can do area copy*/
#define PSF_ADDRMALLOC		0x0010	/* psd->addr was malloc'd*/
#define PSF_ADDRSHAREDMEM	0x0020	/* psd->addr is shared memory*/

/* T.J.Park Note : We are using back-buffer(or off-screen, whatever you call) in order to avoid flickering.
                   All graphics primitives works in the back buffer,
                   and also we support another function, GrCopyOffScreen().
				   I added a new flag so that the Microwindows can check whether the given
				   screen device has off-screen. See GrCopyOffScreen(). */
#define PSF_HAVE_OFFSCREEN	0x0040	/* the screen device has the back-buffer. Must be used with PSF_SCREEN. */

/* Interface to Mouse Device Driver*/
typedef struct _mousedevice {
	int	(*Open)(struct _mousedevice *);
	void	(*Close)(void);
	int	(*GetButtonInfo)(void);
	void	(*GetDefaultAccel)(int *pscale,int *pthresh);
	int	(*Read)(MWCOORD *dx,MWCOORD *dy,MWCOORD *dz,int *bp);
	int	(*Poll)(void);	/* not required if have select()*/
        int     flags;		/* raw, normal, transform flags*/
} MOUSEDEVICE;

#define MOUSE_NORMAL		0x0000	/* mouse in normal mode*/
#define MOUSE_RAW		0x0001	/* mouse in raw mode*/
#define MOUSE_TRANSFORM		0x0002	/* perform transform*/

/* Interface to Keyboard Device Driver*/
typedef struct _kbddevice {
	int  (*Open)(struct _kbddevice *pkd);
	void (*Close)(void);
	void (*GetModifierInfo)(MWKEYMOD *modifiers, MWKEYMOD *curmodifiers);
	int  (*Read)(MWKEY *buf,MWKEYMOD *modifiers,MWSCANCODE *scancode);
	int  (*Poll)(void);		/* not required if have select()*/
} KBDDEVICE;


/* Clip areas*/
#define CLIP_VISIBLE		0
#define CLIP_INVISIBLE		1
#define CLIP_PARTIAL		2

/* static clip rectangle: drawing allowed if point within rectangle*/
typedef struct {
	MWCOORD 	x;		/* x coordinate of top left corner */
	MWCOORD 	y;		/* y coordinate of top left corner */
	MWCOORD 	width;		/* width of rectangle */
	MWCOORD 	height;		/* height of rectangle */
} MWCLIPRECT;

typedef struct {
	MWCOORD	width;
	MWCOORD	height;
	PSD	psd;
} MWTILE;

typedef struct {
	int	start_pos;
	int	length;
	int	direction;
} MWGRADATION;

#ifndef TRUE
#define TRUE			1
#endif
#ifndef FALSE
#define FALSE			0
#endif

#define	MWMIN(a,b)		((a) < (b) ? (a) : (b))
#define	MWMAX(a,b) 		((a) > (b) ? (a) : (b))

/* color and palette defines*/
#define RGBDEF(r,g,b)	{r, g, b}

/* FIXME : drhong : Put the alpha value 0xff. Because we are using 32 bit color format, the previous
version returns all color with alpha channel 0. There must be more consideration about
palettized color system with alpha channel. Temporarily we put the alpha value 0xff. */
#if	0
#define GETPALENTRY(pal,index) ((unsigned long)(pal[index].r |\
				(pal[index].g << 8) | (pal[index].b << 16)))
#else
#define GETPALENTRY(pal,index) ((unsigned long)(pal[index].r |\
				(pal[index].g << 8) | (pal[index].b << 16) | 0xff000000))
#endif
/*#define GETPALENTRY(pal,index) ((*(unsigned long *)&pal[index])&0x00ffffff)*/

#define REDVALUE(rgb)	((rgb) & 0xff)
#define GREENVALUE(rgb) (((rgb) >> 8) & 0xff)
#define BLUEVALUE(rgb)	(((rgb) >> 16) & 0xff)

/* Truecolor color conversion and extraction macros*/
/*
 * Conversion from 8-bit RGB components to MWPIXELVAL
 */
/* create 32 bit 8/8/8/8 format pixel (0xAARRGGBB) from RGB triplet*/
#define RGB2PIXEL8888(r,g,b)	\
	(0xFF000000UL | ((r) << 16) | ((g) << 8) | (b))

/* create 32 bit 8/8/8/8 format pixel (0xAARRGGBB) from ARGB triplet*/
#define ARGB2PIXEL8888(a,r,g,b)	\
	(((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

/* create 24 bit 8/8/8 format pixel (0x00RRGGBB) from RGB triplet*/
#define RGB2PIXEL888(r,g,b)	\
	(((r) << 16) | ((g) << 8) | (b))

/* create 16 bit 5/6/5 format pixel from RGB triplet */
#define RGB2PIXEL565(r,g,b)	\
	((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

/* create 16 bit 5/5/5 format pixel from RGB triplet */
#define RGB2PIXEL555(r,g,b)	\
	((((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3))

/* create 8 bit 3/3/2 format pixel from RGB triplet*/
#define RGB2PIXEL332(r,g,b)	\
	(((r) & 0xe0) | (((g) & 0xe0) >> 3) | (((b) & 0xc0) >> 6))

/* create 16 bit 1/5/5/5 format pixel from RGB triplet : added 2007.03 */
#define ARGB2PIXEL1555(a,r,g,b)	\
	((a) ? (((a==0xff) ? 0x0001 : 0x8001) | (((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3)) : 0)
//#define ARGB2PIXEL1555(a,r,g,b)	\
//	((a) ? (((a==0xff) ? 0x0001 : 0x0001) | (((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3)) : 0)


/*
 * Conversion from MWCOLORVAL to MWPIXELVAL
 */
/* create 32 bit 8/8/8/8 format pixel from ARGB colorval (0xAABBGGRR)*/
/* In this format, alpha is preserved. */
#define COLOR2PIXEL8888(c)	\
	((((c) & 0xff) << 16) | ((c) & 0xff00ff00ul) | (((c) & 0xff0000) >> 16))

/* create 24 bit 8/8/8 format pixel from RGB colorval (0x00BBGGRR)*/
/* In this format, alpha is ignored. */
#define COLOR2PIXEL888(c)	\
	((((c) & 0xff) << 16) | ((c) & 0xff00) | (((c) & 0xff0000) >> 16))

/* create 16 bit 5/6/5 format pixel from RGB colorval (0x00BBGGRR)*/
/* In this format, alpha is ignored. */
#define COLOR2PIXEL565(c)	\
	((((c) & 0xf8) << 8) | (((c) & 0xfc00) >> 5) | (((c) & 0xf80000) >> 19))

/* create 16 bit 5/5/5 format pixel from RGB colorval (0x00BBGGRR)*/
/* In this format, alpha is ignored. */
#define COLOR2PIXEL555(c)	\
	((((c) & 0xf8) << 7) | (((c) & 0xf800) >> 6) | (((c) & 0xf80000) >> 19))

/* create 8 bit 3/3/2 format pixel from RGB colorval (0x00BBGGRR)*/
/* In this format, alpha is ignored. */
#define COLOR2PIXEL332(c)	\
	(((c) & 0xe0) | (((c) & 0xe000) >> 11) | (((c) & 0xc00000) >> 22))

/* create 16 bit 1/5/5/5 format pixel from RGB colorval (0xAABBGGRR) : added 2007.03 */
/* In this format */
#define COLOR2PIXEL1555(c)	\
	((((c)&0xff000000)==0xff000000 ? 0x0000 : 0x8000) | (((c) & 0xf8) << 7) | (((c) & 0xf800) >> 6) | (((c) & 0xf80000) >> 19))
//#define COLOR2PIXEL1555(c)	\
//	(((c)&0xff000000 ? ((((c)&0xff000000)==0xff000000 ? 0x0001 : 0x0001) | (((c) & 0xf8) << 7) | (((c) & 0xf800) >> 6) | (((c) & 0xf80000) >> 19)) : 0))


/*
 * Conversion from MWPIXELVAL to red, green or blue components
 */
/* return 8/8/8/8 bit a, r, g or b component of 32 bit pixelval*/
#define PIXEL8888ALPHA(pixelval)	(((pixelval) >> 24) & 0xff)
#define PIXEL8888RED(pixelval)  	(((pixelval) >> 16) & 0xff)
#define PIXEL8888GREEN(pixelval)	(((pixelval) >> 8) & 0xff)
#define PIXEL8888BLUE(pixelval) 	((pixelval) & 0xff)

/* return 8/8/8 bit r, g or b component of 24 bit pixelval*/
#define PIXEL888RED(pixelval)		(((pixelval) >> 16) & 0xff)
#define PIXEL888GREEN(pixelval)		(((pixelval) >> 8) & 0xff)
#define PIXEL888BLUE(pixelval)		((pixelval) & 0xff)

/* return 5/6/5 bit r, g or b component of 16 bit pixelval*/
#define PIXEL565RED(pixelval)		(((pixelval) >> 11) & 0x1f)
#define PIXEL565GREEN(pixelval)		(((pixelval) >> 5) & 0x3f)
#define PIXEL565BLUE(pixelval)		((pixelval) & 0x1f)

/* return 5/5/5 bit r, g or b component of 16 bit pixelval*/
#define PIXEL555RED(pixelval)		(((pixelval) >> 10) & 0x1f)
#define PIXEL555GREEN(pixelval)		(((pixelval) >> 5) & 0x1f)
#define PIXEL555BLUE(pixelval)		((pixelval) & 0x1f)

/* return 3/3/2 bit r, g or b component of 8 bit pixelval*/
#define PIXEL332RED(pixelval)		(((pixelval) >> 5) & 0x07)
#define PIXEL332GREEN(pixelval)		(((pixelval) >> 2) & 0x07)
#define PIXEL332BLUE(pixelval)		((pixelval) & 0x03)

/* return 1/5/5/5 bit r, g or b component of 16 bit pixelval : added 2007.03. */
#define PIXEL1555ALPHA(pixelval)	((pixelval) & 0x8000 ? 0xff : 0)
#define PIXEL1555RED(pixelval)		(((pixelval) >> 10) & 0x1f)
#define PIXEL1555GREEN(pixelval)	(((pixelval) >> 5) & 0x1f)
#define PIXEL1555BLUE(pixelval)		((pixelval) & 0x1f)

/*
 * Conversion from MWPIXELVAL to normal 8-bit red, green or blue components
 */
/* return 8/8/8/8 bit a, r, g or b component of 32 bit pixelval*/
#define PIXEL8888ALPHA8(pixelval)		(((pixelval) >> 24) & 0xff)
#define PIXEL8888RED8(pixelval)		(((pixelval) >> 16) & 0xff)
#define PIXEL8888GREEN8(pixelval)		(((pixelval) >> 8) & 0xff)
#define PIXEL8888BLUE8(pixelval)		((pixelval) & 0xff)

/* return 8 bit r, g or b component of 8/8/8 24 bit pixelval*/
#define PIXEL888RED8(pixelval)          (((pixelval) >> 16) & 0xff)
#define PIXEL888GREEN8(pixelval)        (((pixelval) >> 8) & 0xff)
#define PIXEL888BLUE8(pixelval)         ((pixelval) & 0xff)

/* return 8 bit r, g or b component of 5/6/5 16 bit pixelval*/
#define PIXEL565RED8(pixelval)          (((pixelval) >> 8) & 0xf8)
#define PIXEL565GREEN8(pixelval)        (((pixelval) >> 3) & 0xfc)
#define PIXEL565BLUE8(pixelval)         (((pixelval) << 3) & 0xf8)

/* return 8 bit r, g or b component of 5/5/5 16 bit pixelval*/
#define PIXEL555RED8(pixelval)          (((pixelval) >> 7) & 0xf8)
#define PIXEL555GREEN8(pixelval)        (((pixelval) >> 2) & 0xf8)
#define PIXEL555BLUE8(pixelval)         (((pixelval) << 3) & 0xf8)

/* return 8 bit r, g or b component of 3/3/2 8 bit pixelval*/
#define PIXEL332RED8(pixelval)          ( (pixelval)       & 0xe0)
#define PIXEL332GREEN8(pixelval)        (((pixelval) << 3) & 0xe0)
#define PIXEL332BLUE8(pixelval)         (((pixelval) << 6) & 0xc0)

/* return 8 bit r, g or b component of 1/5/5/5 16 bit pixelval : added 2007.03. */
#define PIXEL1555ALPHA8(pixelval)		((pixelval)&0x8000 ? 0xff : 0)
#define PIXEL1555RED8(pixelval)         (((pixelval) >> 7) & 0xf8)
#define PIXEL1555GREEN8(pixelval)       (((pixelval) >> 2) & 0xf8)
#define PIXEL1555BLUE8(pixelval)        (((pixelval) << 3) & 0xf8)


/*
 * Conversion from MWPIXELVAL to *32-bit* red, green or blue components
 * (i.e. PIXEL888RED32(x) == (PIXEL888RED8(x) << 24).  These macros optimize
 * out the extra shift.)
 */
/* return 32 bit a, r, g or b component of 8/8/8/8 32 bit pixelval*/
#define PIXEL8888ALPHA32(pixelval)        ( ((unsigned long)(pixelval))        & 0xff000000UL)
#define PIXEL8888RED32(pixelval)          ((((unsigned long)(pixelval)) <<  8) & 0xff000000UL)
#define PIXEL8888GREEN32(pixelval)        ((((unsigned long)(pixelval)) << 16) & 0xff000000UL)
#define PIXEL8888BLUE32(pixelval)         ((((unsigned long)(pixelval)) << 24) & 0xff000000UL)

/* return 32 bit r, g or b component of 8/8/8 24 bit pixelval*/
#define PIXEL888RED32(pixelval)          ((((unsigned long)(pixelval)) <<  8) & 0xff000000UL)
#define PIXEL888GREEN32(pixelval)        ((((unsigned long)(pixelval)) << 16) & 0xff000000UL)
#define PIXEL888BLUE32(pixelval)         ((((unsigned long)(pixelval)) << 24) & 0xff000000UL)

/* return 32 bit r, g or b component of 5/6/5 16 bit pixelval*/
#define PIXEL565RED32(pixelval)          ((((unsigned long)(pixelval)) << 16) & 0xf8000000UL)
#define PIXEL565GREEN32(pixelval)        ((((unsigned long)(pixelval)) << 21) & 0xfc000000UL)
#define PIXEL565BLUE32(pixelval)         ((((unsigned long)(pixelval)) << 27) & 0xf8000000UL)

/* return 32 bit r, g or b component of 5/5/5 16 bit pixelval*/
#define PIXEL555RED32(pixelval)          ((((unsigned long)(pixelval)) << 17) & 0xf8000000UL)
#define PIXEL555GREEN32(pixelval)        ((((unsigned long)(pixelval)) << 22) & 0xf8000000UL)
#define PIXEL555BLUE32(pixelval)         ((((unsigned long)(pixelval)) << 27) & 0xf8000000UL)

/* return 32 bit r, g or b component of 3/3/2 8 bit pixelval*/
#define PIXEL332RED32(pixelval)          ((((unsigned long)(pixelval)) << 24) & 0xe0000000UL)
#define PIXEL332GREEN32(pixelval)        ((((unsigned long)(pixelval)) << 27) & 0xe0000000UL)
#define PIXEL332BLUE32(pixelval)         ((((unsigned long)(pixelval)) << 30) & 0xc0000000UL)

/* return 32 bit r, g or b component of 1/5/5/5 16 bit pixelval : added 2007.03. */
#define PIXEL1555ALPHA32(pixelval)        ((pixelval)&0x8000 ? 0xff000000UL : 0)
#define PIXEL1555RED32(pixelval)          ((((unsigned long)(pixelval)) << 17) & 0xf8000000UL)
#define PIXEL1555GREEN32(pixelval)        ((((unsigned long)(pixelval)) << 22) & 0xf8000000UL)
#define PIXEL1555BLUE32(pixelval)         ((((unsigned long)(pixelval)) << 27) & 0xf8000000UL)



/*
 * Conversion from MWPIXELVAL to MWCOLORVAL
 */
/* create RGB colorval (0xAABBGGRR) from 8/8/8/8 format pixel*/
#define PIXEL8888TOCOLORVAL(p)	\
	((((p) & 0xff0000ul) >> 16) | ((p) & 0xff00ff00ul) | (((p) & 0xffu) << 16) | 0xff000000ul)

/* create RGB colorval (0xAABBGGRR) from 8/8/8 format pixel*/
#define PIXEL888TOCOLORVAL(p)	\
	(0xff000000ul | (((p) & 0xff0000ul) >> 16) | ((p) & 0xff00u) | (((p) & 0xffu) << 16) | 0xff000000ul)

/* create RGB colorval (0xAABBGGRR) from 5/6/5 format pixel*/
#define PIXEL565TOCOLORVAL(p)	\
	(0xff000000ul | (((p) & 0xf800u) >> 8) | (((p) & 0x07e0u) << 5) | (((p) & 0x1ful) << 19) | 0xff000000ul)

#define PIXEL555TOCOLORVAL(p)	\
	(0xff000000ul | (((p) & 0x7c00u) >> 7) | (((p) & 0x03e0u) << 6) | (((p) & 0x1ful) << 19) | 0xff000000ul)

/* create RGB colorval (0xAABBGGRR) from 3/3/2 format pixel*/
#define PIXEL332TOCOLORVAL(p)	\
	(0xff000000ul | (((p) & 0xe0u)) | (((p) & 0x1cu) << 11) | (((p) & 0x03ul) << 19) | 0xff000000ul)

#define PIXEL1555TOCOLORVAL(p)	\
	((((p) & 0x7c00u) >> 7) | (((p) & 0x03e0u) << 6) | (((p) & 0x1ful) << 19) | 0xff000000ul)



#if MWPIXEL_FORMAT == MWPF_TRUECOLOR8888
#define RGB2PIXEL(r,g,b)	RGB2PIXEL8888(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL8888(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL8888TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL8888RED(p)
#define PIXEL2GREEN(p)		PIXEL8888GREEN(p)
#define PIXEL2BLUE(p)		PIXEL8888BLUE(p)
#endif

#if (MWPIXEL_FORMAT == MWPF_TRUECOLOR888) || (MWPIXEL_FORMAT == MWPF_TRUECOLOR0888)
#define RGB2PIXEL(r,g,b)	RGB2PIXEL888(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL888(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL888TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL888RED(p)
#define PIXEL2GREEN(p)		PIXEL888GREEN(p)
#define PIXEL2BLUE(p)		PIXEL888BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR565
#define RGB2PIXEL(r,g,b)	RGB2PIXEL565(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL565(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL565TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL565RED(p)
#define PIXEL2GREEN(p)		PIXEL565GREEN(p)
#define PIXEL2BLUE(p)		PIXEL565BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR555
#define RGB2PIXEL(r,g,b)	RGB2PIXEL555(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL555(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL555TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL555RED(p)
#define PIXEL2GREEN(p)		PIXEL555GREEN(p)
#define PIXEL2BLUE(p)		PIXEL555BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR332
#define RGB2PIXEL(r,g,b)	RGB2PIXEL332(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL332(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL332TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL332RED(p)
#define PIXEL2GREEN(p)		PIXEL332GREEN(p)
#define PIXEL2BLUE(p)		PIXEL332BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR1555
#define RGB2PIXEL(r,g,b)	ARGB2PIXEL1555(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL1555(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL1555TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL1555RED(p)
#define PIXEL2GREEN(p)		PIXEL1555GREEN(p)
#define PIXEL2BLUE(p)		PIXEL1555BLUE(p)
#endif



#if 0000
/* colors assumed in first 16 palette entries*/
/* note: don't use palette indices if the palette may
 * be reloaded.  Use the RGB values instead.
 */
#define BLACK		PALINDEX(0)		/*   0,   0,   0*/
#define BLUE		PALINDEX(1)
#define GREEN		PALINDEX(2)
#define CYAN		PALINDEX(3)
#define RED			PALINDEX(4)
#define MAGENTA		PALINDEX(5)
#define BROWN		PALINDEX(6)
#define LTGRAY		PALINDEX(7)		/* 192, 192, 192*/
#define GRAY		PALINDEX(8)		/* 128, 128, 128*/
#define LTBLUE		PALINDEX(9)
#define LTGREEN		PALINDEX(10)
#define LTCYAN		PALINDEX(11)
#define LTRED		PALINDEX(12)
#define LTMAGENTA	PALINDEX(13)
#define YELLOW		PALINDEX(14)
#define WHITE		PALINDEX(15)		/* 255, 255, 255*/
#endif

/* GdMakePaletteConversionTable bLoadType types*/
#define LOADPALETTE		1	/* load image palette into system palette*/
#define MERGEPALETTE	2	/* merge image palette into system palette*/

/* entry points*/

#define NANOX_MALLOC(size)			malloc((size))
#define NANOX_FREE(addr)			free((addr))
#define NANOX_CALLOC(num,size)		calloc((num), (size))

/* devdraw.c*/
PSD	GdOpenScreen(int layer);

/* Added for back-buffer operation. See devdraw.c */
void GdCopyOffscreen(MWCLIPREGION *rgn);
void GdCopyOffscreenEx(int layer, MWCLIPREGION *rgn);

void	GdCloseScreen(PSD psd);
int	GdSetPortraitMode(PSD psd, int portraitmode);
int	GdSetMode(int mode);
int GdSetGlobalAlpha(int alpha);
int GdSetGradationOption(MWGRADATION gradationOption);
MWBOOL	GdSetUseBackground(MWBOOL flag);
MWPIXELVAL GdSetForegroundPixelVal(PSD psd, MWPIXELVAL fg);
MWPIXELVAL GdSetBackgroundPixelVal(PSD psd, MWPIXELVAL bg);
MWPIXELVAL GdSetForegroundColor(PSD psd, MWCOLORVAL fg);
MWPIXELVAL GdSetXorColor(PSD psd, MWCOLORVAL xorcolor); /* added by foryou */
MWPIXELVAL GdSetBackgroundColor(PSD psd, MWCOLORVAL bg);

void	GdResetPalette(void);
void	GdSetPalette(PSD psd,int first, int count, MWPALENTRY *palette);
int	GdGetPalette(PSD psd,int first, int count, MWPALENTRY *palette);
MWCOLORVAL GdGetColorRGB(PSD psd, MWPIXELVAL pixel);
MWPIXELVAL GdFindColor(PSD psd, MWCOLORVAL c);
MWPIXELVAL GdFindNearestColor(MWPALENTRY *pal, int size, MWCOLORVAL cr);
int	GdCaptureScreen(char *path);
void	GdGetScreenInfo(PSD psd,PMWSCREENINFO psi);
void	GdSetScreenRes(PSD psd, int width, int height);
void	GdPoint(PSD psd,MWCOORD x, MWCOORD y);
MWBOOL GdClipLine(MWCOORD *px1, MWCOORD *py1, MWCOORD *px2, MWCOORD *py2
				  ,MWCOORD xmin, MWCOORD ymin, MWCOORD xmax, MWCOORD ymax);
void	GdLine(PSD psd,MWCOORD x1,MWCOORD y1,MWCOORD x2,MWCOORD y2,
		MWBOOL bDrawLastPoint);
void	GdRect(PSD psd,MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height);
void	GdFillRect(PSD psd,MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height);
void	GdBitmap(PSD psd,MWCOORD x,MWCOORD y,MWCOORD width,MWCOORD height,
		const MWIMAGEBITS *imagebits);
MWBOOL	GdColorInPalette(MWCOLORVAL cr,MWPALENTRY *palette,int palsize);
void	GdMakePaletteConversionTable(PSD psd,MWPALENTRY *palette,int palsize,
		MWPIXELVAL *convtable,int fLoadType);
void	GdDrawImage(PSD psd,MWCOORD x, MWCOORD y, PMWIMAGEHDR pimage);
void	GdDrawImageEx(PSD psd, MWCOORD destX, MWCOORD destY
		, PMWIMAGEHDR pimage, MWCOORD srcX, MWCOORD srcY, MWCOORD srcWidth, MWCOORD srcHeight);
void	GdPoly(PSD psd,int count, MWPOINT *points);
void	GdFillPoly(PSD psd,int count, MWPOINT *points);
void	GdReadArea(PSD psd,MWCOORD x,MWCOORD y,MWCOORD width,MWCOORD height,
		MWPIXELVAL *pixels);
void	GdArea(PSD psd,MWCOORD x,MWCOORD y,MWCOORD width,MWCOORD height,
		void *pixels, int pixtype);
void	GdDrawAreaInternal(PSD psd, driver_gc_t * gc, int op);
void	GdTranslateArea(MWCOORD width, MWCOORD height, void *in, int inpixtype,
		MWCOORD inpitch, void *out, int outpixtype, int outpitch);
void	GdCopyArea(PSD psd,MWCOORD srcx,MWCOORD srcy,MWCOORD width,
		MWCOORD height, MWCOORD destx, MWCOORD desty);
void	GdBlit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD width,
		MWCOORD height,PSD srcpsd,MWCOORD srcx,MWCOORD srcy,long rop);
void	GdStretchBlit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD dstw,
		MWCOORD dsth, PSD srcpsd, MWCOORD srcx, MWCOORD srcy,
		MWCOORD srcw, MWCOORD srch, long rop);
void	GdStretchBlitEx(PSD dstpsd, MWCOORD d1_x, MWCOORD d1_y, MWCOORD d2_x,
		MWCOORD d2_y, PSD srcpsd, MWCOORD s1_x, MWCOORD s1_y,
		MWCOORD s2_x, MWCOORD s2_y, long rop);
int	GdCalcMemGCAlloc(PSD psd, unsigned int width, unsigned int height,
		int planes, int bpp, int *size, int *linelen);
void	drawbitmap(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
		const MWIMAGEBITS *imagebits);
void	drawpoint(PSD psd, MWCOORD x, MWCOORD y);
void	drawrow(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y);
extern SCREENDEVICE scrdev[MAX_OSD_LAYER];


/* devarc.c*/
MWBOOL GdClipArc(MWCOORD *px, MWCOORD *py, MWCOORD *prx, MWCOORD *pry
				, MWCOORD xmin, MWCOORD ymin, MWCOORD xmax, MWCOORD ymax);
/* requires float*/
void	GdArcAngle(PSD psd, MWCOORD x0, MWCOORD y0, MWCOORD rx, MWCOORD ry,
		MWCOORD angle1, MWCOORD angle2, int type);
/* integer only*/
void	GdArc(PSD psd, MWCOORD x0, MWCOORD y0, MWCOORD rx, MWCOORD ry,
		MWCOORD ax, MWCOORD ay, MWCOORD bx, MWCOORD by, int type);
void	GdEllipse(PSD psd,MWCOORD x, MWCOORD y, MWCOORD rx, MWCOORD ry,
		MWBOOL fill);

/* devfont.c*/
void	GdClearFontList(void);
int	GdAddFont(char *fndry, char *family, char *fontname, PMWLOGFONT lf,
		  unsigned int flags);
PMWFONT	GdSetFont(PMWFONT pfont);
PMWFONT GdCreateFont(PSD psd, const char *name, MWCOORD height,
		const PMWLOGFONT plogfont);
MWCOORD	GdSetFontSize(PMWFONT pfont, MWCOORD fontsize);
void GdGetFontList(MWFONTLIST ***list, int *num);
void GdFreeFontList(MWFONTLIST ***list, int num);
int	GdSetFontRotation(PMWFONT pfont, int tenthdegrees);
int	GdSetFontAttr(PMWFONT pfont, int setflags, int clrflags);
void	GdDestroyFont(PMWFONT pfont);
MWBOOL	GdGetFontInfo(PMWFONT pfont, PMWFONTINFO pfontinfo);
int	GdConvertEncoding(const void *istr, MWTEXTFLAGS iflags, int cc, void *ostr,
		MWTEXTFLAGS oflags);

int GdAll2GBK(const void *istr, void *ostr);//Add @Henry @20080401
int GdUnicodeConvertGBK(const void *istr, void*ostr);	//Add @Henry @20080401


void	GdGetTextSize(PMWFONT pfont, const void *str, int cc, MWCOORD *pwidth,
		MWCOORD *pheight, MWCOORD *pbase, MWTEXTFLAGS flags);
int	GdGetTextSizeEx(PMWFONT pfont, const void *str, int cc,
		int nMaxExtent, int *lpnFit, int *alpDx, MWCOORD *pwidth,
		MWCOORD *pheight, MWCOORD *pbase, MWTEXTFLAGS flags);
void	GdText(PSD psd,MWCOORD x,MWCOORD y,const void *str,int count,
		MWTEXTFLAGS flags);
PMWFONT	GdCreateFontFromBuffer(PSD psd, const unsigned char *buffer,
		unsigned length, const char *format, MWCOORD height);
PMWFONT	GdDuplicateFont(PSD psd, PMWFONT psrcfont, MWCOORD fontsize);

/* devclip1.c*/
void 	GdSetClipRects(PSD psd,int count,MWCLIPRECT *table);
MWBOOL	GdClipPoint(PSD psd,MWCOORD x,MWCOORD y);
int	GdClipArea(PSD psd,MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2);
extern MWCOORD clipminx, clipminy, clipmaxx, clipmaxy;

/* devclip2.c*/
void	GdSetClipRegion(PSD psd, MWCLIPREGION *reg);

/* devrgn.c - multi-rectangle region entry points*/
MWBOOL GdPtInRegion(MWCLIPREGION *rgn, MWCOORD x, MWCOORD y);
int    GdRectInRegion(MWCLIPREGION *rgn, const MWRECT *rect);
MWBOOL GdEqualRegion(MWCLIPREGION *r1, MWCLIPREGION *r2);
MWBOOL GdEmptyRegion(MWCLIPREGION *rgn);
MWCLIPREGION *GdAllocRegion(void);
MWCLIPREGION *GdAllocRectRegion(MWCOORD left,MWCOORD top,MWCOORD right,MWCOORD bottom);
MWCLIPREGION *GdAllocRectRegionIndirect(MWRECT *prc);
void GdSetRectRegion(MWCLIPREGION *rgn, MWCOORD left, MWCOORD top,
		MWCOORD right, MWCOORD bottom);
void GdSetRectRegionIndirect(MWCLIPREGION *rgn, MWRECT *prc);
void GdDestroyRegion(MWCLIPREGION *rgn);
void GdOffsetRegion(MWCLIPREGION *rgn, MWCOORD x, MWCOORD y);
int  GdGetRegionBox(MWCLIPREGION *rgn, MWRECT *prc);
void GdUnionRectWithRegion(const MWRECT *rect, MWCLIPREGION *rgn);
void GdSubtractRectFromRegion(const MWRECT *rect, MWCLIPREGION *rgn);
void GdCopyRegion(MWCLIPREGION *d, MWCLIPREGION *s);
void GdIntersectRegion(MWCLIPREGION *d, MWCLIPREGION *s1, MWCLIPREGION *s2);
void GdUnionRegion(MWCLIPREGION *d, MWCLIPREGION *s1, MWCLIPREGION *s2);
void GdSubtractRegion(MWCLIPREGION *d, MWCLIPREGION *s1, MWCLIPREGION *s2);
void GdXorRegion(MWCLIPREGION *d, MWCLIPREGION *s1, MWCLIPREGION *s2);
MWCLIPREGION *GdAllocBitmapRegion(MWIMAGEBITS *bitmap, MWCOORD width, MWCOORD height);

/* devrgn2.c*/
MWCLIPREGION *GdAllocPolygonRegion(MWPOINT *points, int count, int mode);
MWCLIPREGION *GdAllocPolyPolygonRegion(MWPOINT *points, int *count,
		int nbpolygons, int mode);

/* devmouse.c*/
int	GdOpenMouse(void);
void	GdCloseMouse(void);
void	GdGetButtonInfo(int *buttons);
void	GdRestrictMouse(MWCOORD newminx,MWCOORD newminy,MWCOORD newmaxx,
		MWCOORD newmaxy);
void	GdSetAccelMouse(int newthresh, int newscale);
void	GdMoveMouse(MWCOORD newx, MWCOORD newy);
int	GdReadMouse(MWCOORD *px, MWCOORD *py, int *pb);
void	GdMoveCursor(MWCOORD x, MWCOORD y);
MWBOOL	GdGetCursorPos(MWCOORD *px, MWCOORD *py);
void	GdSetCursor(PMWCURSOR pcursor);
int 	GdShowCursor(PSD psd);
int 	GdHideCursor(PSD psd);
void	GdCheckCursor(PSD psd,MWCOORD x1,MWCOORD y1,MWCOORD x2,MWCOORD y2);
void 	GdFixCursor(PSD psd);
void    GdSetTransform(MWTRANSFORM *);

extern MOUSEDEVICE mousedev;

/* devkbd.c*/
int  	GdOpenKeyboard(void);
void 	GdCloseKeyboard(void);
void 	GdGetModifierInfo(MWKEYMOD *modifiers, MWKEYMOD *curmodifiers);
int  	GdReadKeyboard(MWKEY *buf, MWKEYMOD *modifiers, MWSCANCODE *scancode);
extern KBDDEVICE kbddev;


/* devimage.c */
#if MW_FEATURE_IMAGES
int		GdLoadImageFromBuffer(PSD psd, void *buffer, int size, int flags);

/* New function, added by T.J.Park */
void	GdLoadImageEx(PSD psd, void *buffer, int size, MW_LOAD_IMAGE_EX_RESULT *pResult);

void	GdDrawImageFromRawDataBuffer(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
		 MWCOORD height, void *buffer, MWCOORD buffer_width, MWCOORD buffer_height);


void	GdDrawImageFromBuffer(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
		 MWCOORD height, void *buffer, int size, int flags);
void	GdDrawImageFromFile(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
		MWCOORD height, char *path, int flags);
int		GdLoadImageFromFile(PSD psd, char *path, int flags);
void	GdDrawImageToFit(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
		MWCOORD height, int id);
void	GdDrawImageToFitEx(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
		int id, MWCOORD srcX, MWCOORD srcY, MWCOORD srcWidth, MWCOORD srcHeight);

void	GdDrawRawDataImageToFit(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height, int id);
void	GdFreeImage(int id);
MWBOOL	GdGetImageInfo(int id, PMWIMAGEINFO pii);
void	GdStretchImage(PMWIMAGEHDR src, MWCLIPRECT *srcrect, PMWIMAGEHDR dst,
		MWCLIPRECT *dstrect);
#endif /* MW_FEATURE_IMAGES */

/* devlist.c*/
/* field offset*/
#define MWITEM_OFFSET(type, field)    ((long)&(((type *)0)->field))

void * 	GdItemAlloc(unsigned int size);
void	GdListAdd(PMWLISTHEAD pHead,PMWLIST pItem);
void	GdListInsert(PMWLISTHEAD pHead,PMWLIST pItem);
void	GdListRemove(PMWLISTHEAD pHead,PMWLIST pItem);
#define GdItemNew(type)	((type *)GdItemAlloc(sizeof(type)))
#define GdItemFree(ptr)	free((void *)ptr)

/* devstipple.c */
void	GdSetDash(unsigned long *mask, int *count);
void	GdSetStippleBitmap(MWIMAGEBITS *stipple, MWCOORD width, MWCOORD height);
void	GdSetTSOffset(int xoff, int yoff);
int	GdSetFillMode(int mode);
void	GdSetTilePixmap(PSD src, MWCOORD width, MWCOORD height);
void	ts_drawpoint(PSD psd, MWCOORD x, MWCOORD y);
void	ts_drawrow(PSD psd, MWCOORD x1, MWCOORD x2,  MWCOORD y);
void	ts_fillrect(PSD psd, MWCOORD x, MWCOORD y, MWCOORD w, MWCOORD h);
void	set_ts_origin(int x, int y);

/* return base item address from list ptr*/
#define GdItemAddr(p,type,list)	((type *)((long)p - MWITEM_OFFSET(type,list)))

#if MW_FEATURE_TIMERS
#include <sys/time.h>

typedef void (*MWTIMERCB)(void *);

#define  MWTIMER_ONESHOT         0
#define  MWTIMER_PERIODIC        1

typedef struct mw_timer MWTIMER;
struct mw_timer {
	struct timeval	timeout;
	MWTIMERCB	callback;
	void		*arg;
	MWTIMER		*next;
	MWTIMER		*prev;
    int         type;     /* MWTIMER_ONESHOT or MWTIMER_PERIODIC */
    MWTIMEOUT   period;
};

MWTIMER		*GdAddTimer(MWTIMEOUT timeout, MWTIMERCB callback, void *arg);
MWTIMER         *GdAddPeriodicTimer(MWTIMEOUT timeout, MWTIMERCB callback, void *arg);
void		GdDestroyTimer(MWTIMER *timer);
MWTIMER		*GdFindTimer(void *arg);
MWBOOL		GdGetNextTimeout(struct timeval *tv, MWTIMEOUT timeout);
MWBOOL		GdTimeout(void);

#endif /* MW_FEATURE_TIMERS */

/* error.c*/
int	GdError(const char *format, ...);
int	GdErrorNull(const char *format, ...);  /* doesn't print msgs */

#if USE_ALLOCA
/* alloca() is available, so use it for better performance */
#define ALLOCA(size)	alloca(size)
#define FREEA(pmem)
#else
/* no alloca(), so use malloc()/free() instead */
#define ALLOCA(size)	malloc(size)
#define FREEA(pmem)	free(pmem)
#endif

/**
 * <B>LGE-Specific</B> : Frame Memory.
 */
extern void *NANOX_ScrAllocateFrame(int bpp, int width, int height, int size, int *pStride);
extern void NANOX_ScrFreeFrame(void *pFrame);

extern int NANOX_ScrStretchMemoryFrame(PMWIMAGEHDR pDst, MWCOORD dstX, MWCOORD dstY, MWCOORD dstW, MWCOORD dstH,
									PMWIMAGEHDR pSrc, MWCOORD srcX, MWCOORD srcY, MWCOORD srcW, MWCOORD srcH);


#endif /*_DEVICE_H*/
