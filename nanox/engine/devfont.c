/*
 * Copyright (c) 2000, 2002, 2003 Greg Haerr <greg@censoft.com>
 * Portions Copyright (c) 2002 by Koninklijke Philips Electronics N.V.
 */
/**
 * These routines do the necessary range checking, clipping, and cursor
 * overwriting checks, and then call the lower level device dependent
 * routines to actually do the drawing.  The lower level routines are
 * only called when it is known that all the pixels to be drawn are
 * within the device area and are visible.
 *
 *
 * Device-independent font and text drawing routines
 */
/*#define NDEBUG*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device.h"
#include "devfont.h"
#include "branches.h"

#if HAVE_FONTFUSION_SUPPORT
#include "font_fontfusion.h"
#endif
#if HAVE_UTF_SUPPORT
#include "font_utf.h"
#endif
#if HAVE_UNITYPE_SUPPORT
#include "font_unitype.h"
#endif

#if 0
#include "../drivers/genfont.h"
#endif

/* See ksc2unicode.c */
extern unsigned short ksc2uni(unsigned short CODE);

/* See gb2uicode.c */ //@Added by Henry@20080401
extern unsigned short gb2uni(unsigned short CODE);
extern unsigned short uni2gb(unsigned short CODE);

/* See hk2uicode.c */ //@Added by Henry@20080510
extern unsigned short hk2uni(unsigned short CODE);
extern unsigned short uni2hk(unsigned short CODE);

/**
 * The current font.
 */
PMWFONT	gr_pfont;

/* temp extern decls*/
extern MWPIXELVAL gr_foreground;
extern MWPIXELVAL gr_background;
extern MWBOOL gr_usebg;

void corefont_drawtext(PMWFONT pfont, PSD psd, MWCOORD x, MWCOORD y,
		const void *text, int cc, MWTEXTFLAGS flags);
static int  utf8_to_utf16(const unsigned char *utf8, int cc,
		unsigned short *unicode16);

/**
 * Set the font for future calls.
 *
 * @param pfont The font to use.  If NULL, the font is not changed.
 * @return The old font.
 */
PMWFONT
GdSetFont(PMWFONT pfont)
{
	PMWFONT	oldfont = gr_pfont;

#if 1
	/* T.J.Park Note : pfont may be NULL because we don't have any default font.
	   Also, I checked what happens if gr_pfont is destroyed by GrDestroyFont().
	   In that case, the system is crashed because of NULL pointer access.
	   So, I put the following updates.
	   1. In GrDestroyFont() : If gr_pfont is about to be destroyed, then call GdSetFont(NULL).
	   2. In GdText() and GdGetTextSize() : If the given font is NULL, then do nothing.
	*/
	gr_pfont = pfont;
#else
	/* Original version kept the previous font setting. */
	if (pfont)
		gr_pfont = pfont;
#endif
	return oldfont;
}

/**
 * Select a font, based on various parameters.
 * If plogfont is specified, name and height parms are ignored
 * and instead used from MWLOGFONT.
 *
 * If height is 0, match based on passed name, trying
 * builtins first for speed, then other font renderers.
 * If not found, return 0.  If height=0 is used for
 * scalable font renderers, a call to GdSetFontSize with
 * requested height must occur before rendering.
 *
 * If height not 0, perform same match based on name,
 * but return builtin font best match from height if
 * not otherwise found.
 *
 * @param psd      The screen that this font will be displayed on.
 * @param name     The name of the font.  May be NULL.  Ignored if
 *                 plogfont is specified.
 * @param height   The height of the font in pixels.  Ignored if
 *                 plogfont is specified.
 * @param plogfont A structure describing the font, or NULL.
 * @return         A new font, or NULL on error.
 */
#if HAVE_FONTFUSION_SUPPORT | HAVE_UTF_SUPPORT | HAVE_UNITYPE_SUPPORT
PMWFONT
GdCreateFont(PSD psd, const char *name, MWCOORD height, const PMWLOGFONT plogfont)
{
	MWCOORD		width;
	int			fontattr;
	PMWFONT     pfont;
	char        fontname[MWLF_FACESIZE];

	/* T.J.Park Note :

	When HAVE_FONTFUSION_SUPPORT is defined,
	1. No system font is supported.
	2. FONT_MAPPER is not used.
	3. All font-creation requests are sending to FontFusion.

	In addition, I added a new feature called "Font Installer", which can allow installing(or registering)
	a raw font data	into the system. This can be done by calling GrInstallFont().

	So, we look up the given font name in the "installed font list", and if we can't find it
	, then we think the given name is a file name, and search the file.
	*/

	fontattr = 0;
	/* if plogfont not specified, use name and height*/
	if (!plogfont)
	{
		if(!name)
		{
			return NULL;
		}
		else
		{
			strcpy(fontname, name);
		}

		/*if no logfont, anyway, set following attributes forcely*/
		fontattr |= MWTF_ANTIALIAS;
		fontattr |= MWTF_KERNING;

		width = 0; /* this means width is same as height */
	}
	else
	{
		strcpy(fontname, plogfont->lfFaceName);

		height = plogfont->lfHeight;

		if (plogfont->lfUnderline)
			fontattr = MWTF_UNDERLINE;
		if (plogfont->lfItalic)
			fontattr |= MWTF_ITALIC;
		if (plogfont->lfWeight == MWLF_WEIGHT_BOLD)
			fontattr |= MWTF_BOLD;
		if (plogfont->lfQuality)
			fontattr |= MWTF_ANTIALIAS;
		if (plogfont->lfKerning)
			fontattr |= MWTF_KERNING;

		width = plogfont->lfWidth;
	}

	height = abs(height);

	pfont = NULL;

#if HAVE_UNITYPE_SUPPORT	//091015 unitype -> ttf와 utf를 모두 지원
	if ((strcmp(&fontname[strlen(fontname)-3], "TTF")==0)||(strcmp(&fontname[strlen(fontname)-3], "ttf")==0)
			|| (strcmp(&fontname[strlen(fontname)-3], "UTF")==0)||(strcmp(&fontname[strlen(fontname)-3], "utf")==0)
		 	|| (strcmp(&fontname[strlen(fontname)-3], "PFR")==0)||(strcmp(&fontname[strlen(fontname)-3], "pfr")==0))//unitype engine은 UTF/TTF/PFR 모두 지원	
		pfont = UNITYPE_CreateFont(fontname, height, fontattr, width);
#else

#if HAVE_FONTFUSION_SUPPORT
	if ((strcmp(&fontname[strlen(fontname)-3], "TTF")==0)||(strcmp(&fontname[strlen(fontname)-3], "ttf")==0))
		pfont = fontfusion_createfont(fontname, height,	fontattr, width);
#endif

#if HAVE_UTF_SUPPORT
	if ((strcmp(&fontname[strlen(fontname)-3], "UTF")==0)||(strcmp(&fontname[strlen(fontname)-3], "utf")==0))
		pfont = UTF_CreateFont(fontname, height, fontattr, width);
#endif

#endif
	return pfont;
}


#else
PMWFONT
GdCreateFont(PSD psd, const char *name, MWCOORD height,	const PMWLOGFONT plogfont)
{
	int 		i;
	int		fontht;
	int		fontno;
 	int		fontclass;
	int		fontattr = 0;
	PMWFONT		pfont;
	PMWCOREFONT	pf = psd->builtin_fonts;
	MWFONTINFO	fontinfo;
	MWSCREENINFO 	scrinfo;
	const char *	fontname;
#if !FONTMAPPER
	char 		fontmapper_fontname[MWLF_FACESIZE + 1];
#endif

	GdGetScreenInfo(psd, &scrinfo);

	/* if plogfont not specified, use passed name, height and any class*/
	if (!plogfont) {
		/* if name not specified, use first builtin*/
		if (!name || !name[0])
			name = pf->name;
		fontname = name;
		fontclass = MWLF_CLASS_ANY;
	} else {
		/* otherwise, use MWLOGFONT name, height and class*/
#if FONTMAPPER
		fontname = NULL; /* Paranoia */
 		fontclass = select_font(plogfont, &fontname);
#else
		/* Copy the name from plogfont->lfFaceName to fontmapper_fontname
		 * Note that it may not be NUL terminated in the source string,
		 * so we're careful to NUL terminate it here.
		 */
		strncpy(fontmapper_fontname, plogfont->lfFaceName, MWLF_FACESIZE);
		fontmapper_fontname[MWLF_FACESIZE] = '\0';
		fontname = fontmapper_fontname;
		if (!fontname[0])	/* if name not specified, use first builtin*/
			fontname = pf->name;
		fontclass = plogfont->lfClass;
#endif
		height = plogfont->lfHeight;
		if (plogfont->lfUnderline)
			fontattr = MWTF_UNDERLINE;
	}
	height = abs(height);

	/* check builtin fonts first for speed*/
 	if (!height && (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_BUILTIN)) {
  		for(i = 0; i < scrinfo.fonts; ++i) {
 			if(!strcmpi(pf[i].name, fontname)) {
  				pf[i].fontsize = pf[i].cfont->height;
				pf[i].fontattr = fontattr;
DPRINTF("createfont: (height == 0) found builtin font %s (%d)\n", fontname, i);
  				return (PMWFONT)&pf[i];
  			}
  		}
		/*
		 * Specified height=0 and no builtin font matched name.
		 * if not font found with other renderer, no font
		 * will be loaded, and 0 returned.
		 *
		 * We used to return the first builtin font.  If a font
		 * return needs to be guaranteed, specify a non-zero
		 * height, and the closest builtin font to the specified
		 * height will always be returned.
		 */
  	}

	/* try to load font (regardless of height) using other renderers*/

#ifdef HAVE_FNT_SUPPORT
	if (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_FNT) {
		pfont = (PMWFONT) fnt_createfont(fontname, height, fontattr);
		if (pfont) {
			DPRINTF("fnt_createfont: using font %s\n", fontname);
			return(pfont);
		}
		EPRINTF("fnt_createfont: %s,%d not found\n", fontname, height);
	}
#endif

#ifdef HAVE_PCF_SUPPORT
	if (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_PCF) {
		pfont = (PMWFONT) pcf_createfont(fontname, height, fontattr);
		if (pfont) {
			DPRINTF("pcf_createfont: using font %s\n", fontname);
			return(pfont);
		}
		EPRINTF("pcf_createfont: %s,%d not found\n", fontname, height);
	}
#endif

#if HAVE_FREETYPE_SUPPORT
 	if (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_FREETYPE) {
		if (freetype_init(psd)) {
			/* FIXME auto antialias for height > 14 for kaffe*/
			if (plogfont && plogfont->lfHeight > 14 &&
				plogfont->lfQuality)
					fontattr |= MWTF_ANTIALIAS;

			pfont = (PMWFONT)freetype_createfont(fontname, height,
					fontattr);
			if(pfont) {
				/* FIXME kaffe kluge*/
				pfont->fontattr |= MWTF_FREETYPE;
				return pfont;
			}
 			EPRINTF("freetype_createfont: %s,%d not found\n",
				fontname, height);
		}
  	}
#endif

#if HAVE_FREETYPE_2_SUPPORT
 	if (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_FREETYPE) {
		if (freetype2_init(psd)) {
			/* FIXME auto antialias for height > 14 for kaffe*/
			if (plogfont && plogfont->lfHeight > 14 &&
				plogfont->lfQuality)
					fontattr |= MWTF_ANTIALIAS;

			pfont = (PMWFONT)freetype2_createfont(fontname, height,
					fontattr);
			if(pfont) {
				/* FIXME kaffe kluge*/
				pfont->fontattr |= MWTF_FREETYPE;
				return pfont;
			}
			EPRINTF("freetype2_createfont: %s,%d not found\n",
				fontname, height);
		}
  	}
#endif

#if HAVE_T1LIB_SUPPORT
	if (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_T1LIB) {
		if (t1lib_init(psd)) {
			pfont = (PMWFONT)t1lib_createfont(fontname, height,
					fontattr);
			if(pfont)
				return pfont;
			EPRINTF("t1lib_createfont: %s,%d not found\n",
				fontname, height);
		}
  	}
#endif

#if HAVE_HZK_SUPPORT
	if (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_HZK) {
		/* Make sure the library is initialized */
		if (hzk_init(psd)) {
			pfont = (PMWFONT)hzk_createfont(fontname, height, fontattr);
			if(pfont)
				return pfont;
			EPRINTF("hzk_createfont: %s,%d not found\n", fontname, height);
		}
	}
#endif

#if HAVE_EUCJP_SUPPORT
 	if (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_MGL) {
		pfont = (PMWFONT)eucjp_createfont(fontname, height, fontattr);
		if (pfont) {
			DPRINTF("eujcp_createfont: using font %s\n", fontname);
			return pfont;
		}
		EPRINTF("eucjp_createfont: %s,%d not found\n", fontname, height);
	}
#endif

	/*
	 * No font yet found.  If height specified, we'll return
	 * a builtin font.  Otherwise 0 will be returned.
	 */
 	if (height != 0 && (fontclass == MWLF_CLASS_ANY || fontclass == MWLF_CLASS_BUILTIN)) {
		/* find builtin font closest in height*/
		fontno = 0;
		height = abs(height);
		fontht = MAX_MWCOORD;
		for(i = 0; i < scrinfo.fonts; ++i) {
			pfont = (PMWFONT)&pf[i];
			GdGetFontInfo(pfont, &fontinfo);
			if(fontht > abs(height-fontinfo.height)) {
				fontno = i;
				fontht = abs(height-fontinfo.height);
			}
		}
		pf[fontno].fontsize = pf[fontno].cfont->height;
		pf[fontno].fontattr = fontattr;
DPRINTF("createfont: (height != 0) using builtin font %s (%d)\n", pf[fontno].name, fontno);
		return (PMWFONT)&pf[fontno];
	}

	/* no font matched: don't load font, return 0*/
DPRINTF("createfont: no font found, returning NULL\n");
	return 0;
}
#endif /* HAVE_FONTFUSION_SUPPORT */

/**
 * Set the size of a font.
 *
 * @param pfont    The font to modify.
 * @param fontsize The new size.
 * @return         The old size.
 */
MWCOORD
GdSetFontSize(PMWFONT pfont, MWCOORD fontsize)
{
	MWCOORD oldfontsize = pfont->fontsize;
	pfont->fontsize = fontsize;

	if (pfont->fontprocs->SetFontSize)
	    pfont->fontprocs->SetFontSize(pfont, fontsize);

	return oldfontsize;
}

/**
 * Set the rotation angle of a font.
 *
 * @param pfont        The font to modify.
 * @param tenthdegrees The new rotation angle, in tenths of degrees.
 * @return             The old rotation angle, in tenths of degrees.
 */
int
GdSetFontRotation(PMWFONT pfont, int tenthdegrees)
{
	MWCOORD oldrotation = pfont->fontrotation;
	pfont->fontrotation = tenthdegrees;

	if (pfont->fontprocs->SetFontRotation)
	    pfont->fontprocs->SetFontRotation(pfont, tenthdegrees);

	return oldrotation;
}


/**
 * Set/reset font attributes (MWTF_KERNING, MWTF_ANTIALIAS)
 * for a font.
 *
 * @param pfont    The font to modify.
 * @param setflags The flags to set.  Overrides clrflags.
 * @param clrflags The flags to clear.
 * @return         The old font attributes.
 */
int
GdSetFontAttr(PMWFONT pfont, int setflags, int clrflags)
{
	MWCOORD	oldattr = pfont->fontattr;

	pfont->fontattr &= ~clrflags;
	pfont->fontattr |= setflags;

	if (pfont->fontprocs->SetFontAttr)
	    pfont->fontprocs->SetFontAttr(pfont, setflags, clrflags);

	return oldattr;
}

/**
 * Unload and deallocate a font.  Do not use the font once it has been
 * destroyed.
 *
 * @param pfont The font to deallocate.
 */
void
GdDestroyFont(PMWFONT pfont)
{
	if (pfont->fontprocs->DestroyFont)
		pfont->fontprocs->DestroyFont(pfont);
}

/**
 * Return information about a specified font.
 *
 * @param pfont The font to query.
 * @param pfontinfo Recieves the result of the query
 * @return TRUE on success, FALSE on error.
 */
MWBOOL
GdGetFontInfo(PMWFONT pfont, PMWFONTINFO pfontinfo)
{
	if(!pfont || !pfont->fontprocs->GetFontInfo)
		return FALSE;

	return pfont->fontprocs->GetFontInfo(pfont, pfontinfo);
}

/**
 * Draws text onto a drawing surface (e.g. the screen or a double-buffer).
 * Uses the current font, current foreground color, and possibly the
 * current background color.  Applies clipping if necessary.
 * The background color is only drawn if the gr_usebg flag is set.
 *
 * @param psd   The destination drawing surface.  Non-NULL.
 * @param x     The X co-ordinate to draw the text.
 * @param y     The Y co-ordinate to draw the text.  The flags specify
 *              whether this is the top (MWTF_TOP), bottom (MWTF_BOTTOM),
 *              or baseline (MWTF_BASELINE) of the text.
 * @param str   The string to display.  Non-NULL.
 * @param cc    The length of str.  For Asian DBCS encodings, this is
 *              specified in bytes.  For all other encodings such as ASCII,
 *              UTF8 and UC16, it is specified in characters.  For ASCII
 *              and DBCS encodings, this may be set to -1, and the length
 *              will be calculated automatically.
 * @param flags Flags specifying the encoding of str and the position of the
 *              text.  Specifying the vertical position is mandatory.
 *              The encoding of str defaults to ASCII if not specified.
 */

#if HAVE_FONTFUSION_SUPPORT | HAVE_UTF_SUPPORT | HAVE_UNITYPE_SUPPORT
void
GdText(PSD psd, MWCOORD x, MWCOORD y, const void *str, int cc, MWTEXTFLAGS flags)
{
	const void *    text;
	unsigned long   buf[512];
	int     defencoding;

	if(gr_pfont == NULL)
		return;

	defencoding = gr_pfont->fontprocs->encoding;

	/* convert encoding if required*/
	if((flags & MWTF_PACKMASK) != defencoding) {
		cc = GdConvertEncoding(str, flags, cc, buf, defencoding);
		flags &= ~MWTF_PACKMASK;
		flags |= defencoding;
		text = buf;
	}
	else
		text = str;

	if(cc == -1 && (flags & MWTF_PACKMASK) == MWTF_ASCII)
		cc = strlen((char *)str);

	if(cc <= 0 || !gr_pfont->fontprocs->DrawText)
		return;
	/* draw text string*/
	gr_pfont->fontprocs->DrawText(gr_pfont, psd, x, y, text, cc, flags);
}
#else
void
GdText(PSD psd, MWCOORD x, MWCOORD y, const void *str, int cc,MWTEXTFLAGS flags)
{
	const void *	text;
	int		defencoding = gr_pfont->fontprocs->encoding;
	int		force_uc16 = 0;
	unsigned long	buf[256];

	/*
	 * DBCS encoding is handled a little special: if the selected
	 * font is a builtin, then we'll force a conversion to UC16
	 * rather than converting to the renderer specification.  This is
	 * because we allow DBCS-encoded strings to draw using the
	 * specially-compiled-in font if the character is not ASCII.
	 * This is specially handled in corefont_drawtext below.
	 *
	 * If the font is not builtin, then the drawtext routine must handle
	 * all glyph output, including ASCII.
	 */
	if (flags & MWTF_DBCSMASK) {
		/* force double-byte sequences to UC16 if builtin font only*/
		if (gr_pfont->fontprocs->GetTextBits == gen_gettextbits &&
		    gr_pfont->fontprocs->DrawText == corefont_drawtext) {
			defencoding = MWTF_UC16;
			force_uc16 = 1;
		}
	}

	/* convert encoding if required*/
	if((flags & (MWTF_PACKMASK|MWTF_DBCSMASK)) != defencoding) {
		cc = GdConvertEncoding(str, flags, cc, buf, defencoding);
		flags &= ~MWTF_PACKMASK;	/* keep DBCS bits for drawtext*/
		flags |= defencoding;
		text = buf;
	} else text = str;

	/* use strlen for char count when ascii or dbcs*/
	if(cc == -1 && (flags & MWTF_PACKMASK) == MWTF_ASCII)
		cc = strlen((char *)str);

	if(cc <= 0 || !gr_pfont->fontprocs->DrawText)
		return;

	/* draw text string, DBCS flags may still be set*/
	if (!force_uc16)	/* remove DBCS flags if not needed*/
		flags &= ~MWTF_DBCSMASK;
	gr_pfont->fontprocs->DrawText(gr_pfont, psd, x, y, text, cc, flags);
}
#endif /* HAVE_FONTFUSION_SUPPORT */

/*
 * Draw ascii text using COREFONT type font.
 */
#if HAVE_FONTFUSION_SUPPORT | HAVE_UTF_SUPPORT | HAVE_UNITYPE_SUPPORT
void
corefont_drawtext(PMWFONT pfont, PSD psd, MWCOORD x, MWCOORD y,
    const void *text, int cc, MWTEXTFLAGS flags)
{
	/* T.J.Park Note : Do nothing in this function, because we don't use it
	when HAVE_FONTFUSION_SUPPORT is defined. */
}
#else
void
corefont_drawtext(PMWFONT pfont, PSD psd, MWCOORD x, MWCOORD y,
	const void *text, int cc, MWTEXTFLAGS flags)
{
	const unsigned char *str = text;
	const unsigned short *istr = text;
	MWCOORD		width;			/* width of text area */
	MWCOORD 	height;			/* height of text area */
	MWCOORD		base;			/* baseline of text*/
	MWCOORD		startx, starty;
	const MWIMAGEBITS *bitmap;		/* bitmap for characters */
	MWBOOL		bgstate;
	int		clip;

	if (flags & MWTF_DBCSMASK)
		dbcs_gettextsize(pfont, istr, cc, flags, &width, &height, &base);
	else pfont->fontprocs->GetTextSize(pfont, str, cc, flags, &width, &height, &base);

	if (flags & MWTF_BASELINE)
		y -= base;
	else if (flags & MWTF_BOTTOM)
		y -= (height - 1);
	startx = x;
	starty = y + base;
	bgstate = gr_usebg;

	switch (clip = GdClipArea(psd, x, y, x + width - 1, y + height - 1)) {
	case CLIP_VISIBLE:
		/* clear background once for all characters*/
		if (gr_usebg)
			psd->FillRect(psd, x, y, x + width - 1, y + height - 1,
				gr_background);

		/* FIXME if we had a low-level text drawer, plug in here:
		psd->DrawText(psd, x, y, str, cc, gr_foreground, pfont);
		GdFixCursor(psd);
		return;
		*/

		/* save state for combined routine below*/
		bgstate = gr_usebg;
		gr_usebg = FALSE;
		break;

	case CLIP_INVISIBLE:
		return;
	}

	/* Get the bitmap for each character individually, and then display
	 * them possibly using clipping for each one.
	 */

	/*
	 * If the string was marked as DBCS, then we've forced the conversion
	 * to UC16 in GdText.  Here we special-case the non-ASCII values and
	 * get the bitmaps from the specially-compiled-in font.  Otherwise,
	 * we draw them using the normal pfont->fontprocs->GetTextBits.
	 */
	while (--cc >= 0 && x < psd->xvirtres) {
		if (flags & MWTF_DBCSMASK)
			dbcs_gettextbits(pfont, *istr++, flags, &bitmap, &width,
				&height, &base);
		else pfont->fontprocs->GetTextBits(pfont, *str++, &bitmap, &width,
			&height, &base);


		if (clip == CLIP_VISIBLE)
			drawbitmap(psd, x, y, width, height, bitmap);
		else
			GdBitmap(psd, x, y, width, height, bitmap);
		x += width;
	}

	if (pfont->fontattr & MWTF_UNDERLINE)
		GdLine(psd, startx, starty, x, starty, FALSE);

	/* restore background draw state*/
	gr_usebg = bgstate;

	GdFixCursor(psd);
}
#endif /* HAVE_FONTFUSION_SUPPORT */

#if HAVE_FNT_SUPPORT | HAVE_PCF_SUPPORT
/*
 * Draw MWTF_UC16 text using COREFONT type font.
 */
void
gen16_drawtext(PMWFONT pfont, PSD psd, MWCOORD x, MWCOORD y,
	const void *text, int cc, MWTEXTFLAGS flags)
{
	const unsigned short *str = text;
	MWCOORD		width;			/* width of text area */
	MWCOORD		height;			/* height of text area */
	MWCOORD		base;			/* baseline of text */
	MWCOORD		startx, starty;
	const MWIMAGEBITS *bitmap;		/* bitmap for characters */
	MWBOOL		bgstate;
	int		clip;

	pfont->fontprocs->GetTextSize(pfont, str, cc, flags, &width, &height, &base);

	if (flags & MWTF_BASELINE)
		y -= base;
	else if (flags & MWTF_BOTTOM)
		y -= (height - 1);
	startx = x;
	starty = y + base;
	bgstate = gr_usebg;

	switch (clip = GdClipArea(psd, x, y, x + width - 1, y + height - 1)) {
	case CLIP_VISIBLE:
		/* clear background once for all characters*/
		if (gr_usebg)
			psd->FillRect(psd, x, y, x + width - 1, y + height - 1,
				gr_background);

		/* FIXME if we had a low-level text drawer, plug in here:
		psd->DrawText(psd, x, y, str, cc, gr_foreground, pfont);
		GdFixCursor(psd);
		return;
		*/

		/* save state for combined routine below*/
		bgstate = gr_usebg;
		gr_usebg = FALSE;
		break;

	case CLIP_INVISIBLE:
		return;
	}

	/* Get the bitmap for each character individually, and then display
	 * them using clipping for each one.
	 */
	while (--cc >= 0 && x < psd->xvirtres) {
		unsigned int ch = *str++;
		pfont->fontprocs->GetTextBits(pfont, ch, &bitmap, &width,
			&height, &base);

		if (clip == CLIP_VISIBLE)
			drawbitmap(psd, x, y, width, height, bitmap);
		else
			GdBitmap(psd, x, y, width, height, bitmap);
		x += width;
	}

	if (pfont->fontattr & MWTF_UNDERLINE)
		GdLine(psd, startx, starty, x, starty, FALSE);

	/* restore background draw state*/
	gr_usebg = bgstate;

	GdFixCursor(psd);
}
#endif /* HAVE_FNT_SUPPORT | HAVE_PCF_SUPPORT*/

#if HAVE_T1LIB_SUPPORT | HAVE_FREETYPE_SUPPORT
/*
 * Produce blend table from src and dst based on passed alpha table
 * Used because we don't quite yet have GdArea with alphablending,
 * so we pre-blend fg/bg colors for fade effect.
 */
void
alphablend(PSD psd, OUTPIXELVAL *out, MWPIXELVAL src, MWPIXELVAL dst,
	unsigned char *alpha, int count)
{
	unsigned int	a, d;
	unsigned char	r, g, b;
	MWCOLORVAL	palsrc, paldst;
	extern MWPALENTRY gr_palette[256];

	while (--count >= 0) {
	    a = *alpha++;

#define BITS(pixel,shift,mask)	(((pixel)>>shift)&(mask))
	    if(a == 0)
		*out++ = dst;
	    else if(a == 255)
		*out++ = src;
	    else
		switch(psd->pixtype) {
	        case MWPF_TRUECOLOR0888:
	        case MWPF_TRUECOLOR888:
		    d = BITS(dst, 16, 0xff);
		    r = (unsigned char)(((BITS(src, 16, 0xff) - d)*a)>>8) + d;
		    d = BITS(dst, 8, 0xff);
		    g = (unsigned char)(((BITS(src, 8, 0xff) - d)*a)>>8) + d;
		    d = BITS(dst, 0, 0xff);
		    b = (unsigned char)(((BITS(src, 0, 0xff) - d)*a)>>8) + d;
		    *out++ = (r << 16) | (g << 8) | b;
		    break;

	        case MWPF_TRUECOLOR565:
		    d = BITS(dst, 11, 0x1f);
		    r = (unsigned char)(((BITS(src, 11, 0x1f) - d)*a)>>8) + d;
		    d = BITS(dst, 5, 0x3f);
		    g = (unsigned char)(((BITS(src, 5, 0x3f) - d)*a)>>8) + d;
		    d = BITS(dst, 0, 0x1f);
		    b = (unsigned char)(((BITS(src, 0, 0x1f) - d)*a)>>8) + d;
		    *out++ = (r << 11) | (g << 5) | b;
		    break;

	        case MWPF_TRUECOLOR555:
		    d = BITS(dst, 10, 0x1f);
		    r = (unsigned char)(((BITS(src, 10, 0x1f) - d)*a)>>8) + d;
		    d = BITS(dst, 5, 0x1f);
		    g = (unsigned char)(((BITS(src, 5, 0x1f) - d)*a)>>8) + d;
		    d = BITS(dst, 0, 0x1f);
		    b = (unsigned char)(((BITS(src, 0, 0x1f) - d)*a)>>8) + d;
		    *out++ = (r << 10) | (g << 5) | b;
		    break;

	        case MWPF_TRUECOLOR332:
		    d = BITS(dst, 5, 0x07);
		    r = (unsigned char)(((BITS(src, 5, 0x07) - d)*a)>>8) + d;
		    d = BITS(dst, 2, 0x07);
		    g = (unsigned char)(((BITS(src, 2, 0x07) - d)*a)>>8) + d;
		    d = BITS(dst, 0, 0x03);
		    b = (unsigned char)(((BITS(src, 0, 0x03) - d)*a)>>8) + d;
		    *out++ = (r << 5) | (g << 2) | b;
		    break;

	        case MWPF_PALETTE:
		    /* reverse lookup palette entry for blend ;-)*/
		    palsrc = GETPALENTRY(gr_palette, src);
		    paldst = GETPALENTRY(gr_palette, dst);
		    d = REDVALUE(paldst);
		    r = (unsigned char)(((REDVALUE(palsrc) - d)*a)>>8) + d;
		    d = GREENVALUE(paldst);
		    g = (unsigned char)(((GREENVALUE(palsrc) - d)*a)>>8) + d;
		    d = BLUEVALUE(paldst);
		    b = (unsigned char)(((BLUEVALUE(palsrc) - d)*a)>>8) + d;
		    *out++ = GdFindNearestColor(gr_palette, (int)psd->ncolors,
				MWRGB(r, g, b));
		    break;
	  	}
	}
}
#endif /*HAVE_T1LIB_SUPPORT | HAVE_FREETYPE_SUPPORT*/

#if !HAVE_FREETYPE_SUPPORT
int
GdGetTextSizeEx(PMWFONT pfont, const void *str, int cc,int nMaxExtent,
	int* lpnFit, int* alpDx,MWCOORD *pwidth,MWCOORD *pheight,
	MWCOORD *pbase, MWTEXTFLAGS flags)
{
	*pwidth = *pheight = *pbase = 0;
	return 0;
}

void
GdFreeFontList(MWFONTLIST ***fonts, int n)
{
}

void
GdGetFontList(MWFONTLIST ***fonts, int *numfonts)
{
	*numfonts = -1;
}
#endif /* !HAVE_FREETYPE_SUPPORT*/

/**
 * Convert text from one encoding to another.
 * Input cc and returned cc is character count, not bytes.
 * Return < 0 on error or can't translate.
 *
 * @param istr   Input string.
 * @param iflags Encoding of istr, as MWTF_xxx constants.
 * @param cc     The length of istr.  For Asian DBCS encodings, this is
 *               specified in bytes.  For all other encodings such as ASCII,
 *               UTF8 and UC16, it is specified in characters.  For ASCII
 *               and DBCS encodings, this may be set to -1, and the length
 *               will be calculated automatically.
 * @param ostr   Output string.
 * @param oflags Encoding of ostr, as MWTF_xxx constants.
 * @return       Number of characters (not bytes) converted.
 */
int
GdConvertEncoding(const void *istr, MWTEXTFLAGS iflags, int cc, void *ostr,
	MWTEXTFLAGS oflags)
{
	const unsigned char 	*istr8;
	const unsigned short 	*istr16;
	const unsigned long		*istr32;
	unsigned char 			*ostr8;
	unsigned short 			*ostr16;
	unsigned long			*ostr32;
	unsigned int			ch;
	int						icc;
	unsigned short			buf16[512];


	iflags &= MWTF_PACKMASK|MWTF_DBCSMASK;
	oflags &= MWTF_PACKMASK|MWTF_DBCSMASK;

	/* allow -1 for len with ascii or dbcs*/
	if(cc == -1 && (iflags == MWTF_ASCII))
		cc = strlen((char *)istr);

	/* first check for utf8 input encoding*/
	if(iflags == MWTF_UTF8) {
		/* we've only got uc16 now so convert to uc16...*/
		cc = utf8_to_utf16((unsigned char *)istr, cc,
			oflags==MWTF_UC16?(unsigned short*) ostr: buf16);

		if(oflags == MWTF_UC16 || cc < 0)
			return cc;

		/* will decode again to requested format (probably ascii)*/
		iflags = MWTF_UC16;
		istr = buf16;
	}

#if HAVE_HZK_SUPPORT
	if(iflags == MWTF_UC16 && oflags == MWTF_ASCII) {
		/* only support uc16 convert to ascii now...*/
		cc = UC16_to_GB( istr, cc, ostr);
		return cc;
	}
#endif

	icc = cc;
	cc = 0;
	istr8 = istr;
	istr16 = istr;
	istr32 = istr;
	ostr8 = ostr;
	ostr16 = ostr;
	ostr32 = ostr;


	/* Convert between formats.  Note that there's no error
	 * checking here yet.
	 */


	while(--icc >= 0) {
		switch(iflags) {
		default:
			ch = *istr8++;
			break;
		case MWTF_UC16:
			ch = *istr16++;
			break;
		case MWTF_XCHAR2B:
			ch = *istr8++ << 8;
			ch |= *istr8++;
			break;
		case MWTF_UC32:
			ch = *istr32++;
			break;
		#if	(SYS_DVB)
		case MWTF_DBCS_BIG5:	/* Chinese BIG5*/
			ch = *istr8++;
			if (ch >= 0xA1 && ch <= 0xF9 && icc &&
			    ((*istr8 >= 0x40 && *istr8 <= 0x7E) ||
			     (*istr8 >= 0xA1 && *istr8 <= 0xFE))) {
				ch = (ch << 8) | *istr8++;
				--icc;
			}
			break;
		case MWTF_DBCS_EUCCN:	/* Chinese EUCCN (GB2312+0x80)*/ //@Henry: Chinese GBK
			ch = *istr8++;
			if (ch >= 0x81 && ch <= 0xFE && icc &&
			    *istr8 >= 0x40 && *istr8 <= 0xFE) {
				ch = (ch << 8) | *istr8++;
				--icc;
				/*@Henry :make it unicode*/
				ch = gb2uni((unsigned short) ch);
			}
			break;
		#endif	/* (SYS_DVB) */
		case MWTF_DBCS_EUCKR:	/* Korean EUCKR (KSC5601+0x80)*/
			ch = *istr8++;
			if (ch >= 0xA1 && ch <= 0xFE && icc &&
			    *istr8 >= 0xA1 && *istr8 <= 0xFE) {
				ch = (ch << 8) | *istr8++;
				--icc;

				/* T.J.Park Note : make it unicode. */
				ch = ksc2uni((unsigned short) ch);
			}
			break;
		case MWTF_DBCS_EUCJP:	/* Japanese EUCJP*/
			ch = *istr8++;
			if (ch >= 0xA1 && ch <= 0xFE && icc &&
			    *istr8 >= 0xA1 && *istr8 <= 0xFE) {
				ch = (ch << 8) | *istr8++;
				--icc;
			}
			break;
		case MWTF_DBCS_JIS:	/* Japanese JISX0213*/
			ch = *istr8++;
			if (icc && (
			    (ch >= 0xA1 && ch <= 0xFE && *istr8 >= 0xA1 && *istr8 <= 0xFE)
			    ||
			    (((ch >= 0x81 && ch <= 0x9F) || (ch >= 0xE0 && ch <= 0xEF)) &&
			     (*istr8 >= 0x40 && *istr8 <= 0xFC && *istr8 != 0x7F))
			            )) {
					ch = (ch << 8) | *istr8++;
					--icc;
			}

			break;
		}

		switch(oflags) {
		default:
			*ostr8++ = (unsigned char)ch;
			break;
		case MWTF_UC16:
			*ostr16++ = (unsigned short)ch;
			break;
		case MWTF_XCHAR2B:
			*ostr8++ = (unsigned char)(ch >> 8);
			*ostr8++ = (unsigned char)ch;
			break;
		case MWTF_UC32:
			*ostr32++ = ch;
			break;
		}
		++cc;
	}
	return cc;
}

/**
 * Gets the size of some text in a specified font.
 *
 * @param pfont   The font to measure.  Non-NULL.
 * @param str     The string to measure.  Non-NULL.
 * @param cc      The length of str.  For Asian DBCS encodings, this is
 *                specified in bytes.  For all other encodings such as ASCII,
 *                UTF8 and UC16, it is specified in characters.  For ASCII
 *                and DBCS encodings, this may be set to -1, and the length
 *                will be calculated automatically.
 * @param pwidth  On return, holds the width of the text.
 * @param pheight On return, holds the height of the text.
 * @param pbase   On return, holds the baseline of the text.
 * @param flags   Flags specifying the encoding of str and the position of the
 *                text.  Specifying the vertical position is mandatory.
 *                The encoding of str defaults to ASCII if not specified.
 */
#if HAVE_FONTFUSION_SUPPORT | HAVE_UTF_SUPPORT | HAVE_UNITYPE_SUPPORT
void
GdGetTextSize(PMWFONT pfont, const void *str, int cc, MWCOORD *pwidth,
    MWCOORD *pheight, MWCOORD *pbase, MWTEXTFLAGS flags)
{
	const void *    text;
	unsigned long   buf[256];
	int     defencoding;

	if(pfont == NULL)
	{
		*pwidth = 0;
		*pheight = 0;
		*pbase = 0;
		return;
	}

	defencoding = pfont->fontprocs->encoding;

    /* convert encoding if required*/
	if((flags & MWTF_PACKMASK) != defencoding) {
		cc = GdConvertEncoding(str, flags, cc, buf, defencoding);
		flags &= ~MWTF_PACKMASK;
		flags |= defencoding;
		text = buf;
	} else text = str;

	if(cc == -1 && (flags & MWTF_PACKMASK) == MWTF_ASCII)
		cc = strlen((char *)str);

	if(cc <= 0 || !pfont->fontprocs->GetTextSize) {
		*pwidth = *pheight = *pbase = 0;
		return;
	}

    /* calc height and width of string*/
	pfont->fontprocs->GetTextSize(pfont, text, cc, flags, pwidth, pheight, pbase);
}

#else
void
GdGetTextSize(PMWFONT pfont, const void *str, int cc, MWCOORD *pwidth,
	MWCOORD *pheight, MWCOORD *pbase, MWTEXTFLAGS flags)
{
	const void *	text;
	MWTEXTFLAGS	defencoding = pfont->fontprocs->encoding;
	int		force_uc16 = 0;
	unsigned long	buf[256];

	/* DBCS handled specially: see comment in GdText*/
	if (flags & MWTF_DBCSMASK) {
		/* force double-byte sequences to UC16 if builtin font only*/
		if (pfont->fontprocs->GetTextBits == gen_gettextbits &&
		    pfont->fontprocs->DrawText == corefont_drawtext) {
			defencoding = MWTF_UC16;
			force_uc16 = 1;
		}
	}

	/* convert encoding if required*/
	if((flags & (MWTF_PACKMASK|MWTF_DBCSMASK)) != defencoding) {
		cc = GdConvertEncoding(str, flags, cc, buf, defencoding);
		flags &= ~MWTF_PACKMASK; /* keep DBCS bits for gettextsize*/
		flags |= defencoding;
		text = buf;
	} else text = str;

	/* use strlen for char count when ascii or dbcs*/
	if(cc == -1 && (flags & MWTF_PACKMASK) == MWTF_ASCII)
		cc = strlen((char *)str);

	if(cc <= 0 || !pfont->fontprocs->GetTextSize) {
		*pwidth = *pheight = *pbase = 0;
		return;
	}

	/* calc height and width of string*/
	if (force_uc16)		/* if UC16 conversion forced, string is DBCS*/
		dbcs_gettextsize(pfont, text, cc, flags, pwidth, pheight, pbase);
	else pfont->fontprocs->GetTextSize(pfont, text, cc, flags, pwidth, pheight, pbase);
}
#endif /* HAVE_FONTFUSION_SUPPORT */

#if HAVE_FREETYPE_2_SUPPORT
/**
 * Create a new font, from a buffer.
 *
 * @param psd    Drawing surface.
 * @param buffer The data to create the font from.  This should be an
 *               in-memory copy of a font file.
 * @param length The length of the buffer, in bytes.
 * @param format Buffer format, or NULL or "" to auto-detect.
 *               Currently unused, since only FreeType 2 fonts are
 *               currently supported, and FreeType 2 always
 *               autodetects.
 * @param height The font height in pixels.
 * @return       New font, or NULL on error.
 */
PMWFONT
GdCreateFontFromBuffer(PSD psd, const unsigned char *buffer,
		       unsigned length, const char *format, MWCOORD height)
{
	PMWFONT pfont = NULL;

	//assert(buffer);

	/* EPRINTF("Nano-X: Font magic = '%c%c%c%c' @ GdCreateFontFromBuffer\n",
	 * (char) buffer[0], (char) buffer[1], (char) buffer[2], (char) buffer[3]);
	 */

	/*
	 * If we had multiple font drivers, we'd have to do select one
	 * based on 'format' here.  (Suggestion: 'format' is the file
	 * extension - e.g. TTF, PFR, ...)
	 */

	if (freetype2_init(psd)) {
		pfont = (PMWFONT)freetype2_createfontfrombuffer(buffer, length, height);
	}
	if (!pfont)
		EPRINTF("GdCreateFontFromBuffer: create failed.\n");

	return pfont;
}

/**
 * Create a new font, which is a copy of an old font.
 *
 * @param psd      Drawing surface.
 * @param psrcfont Font to copy from.
 * @param fontsize Size of new font, or 0 for unchanged.
 * @return         New font.
 */
PMWFONT
GdDuplicateFont(PSD psd, PMWFONT psrcfont, MWCOORD fontsize)
{
	//assert(psd);
	//assert(psrcfont);

	if (psrcfont->fontprocs->Duplicate)
		return psrcfont->fontprocs->Duplicate(psrcfont, fontsize);

	return psrcfont;
}
#endif /*HAVE_FREETYPE_2_SUPPORT*/

/**
 * UTF-8 to UTF-16 conversion.  Surrogates are handeled properly, e.g.
 * a single 4-byte UTF-8 character is encoded into a surrogate pair.
 * On the other hand, if the UTF-8 string contains surrogate values, this
 * is considered an error and returned as such.
 *
 * The destination array must be able to hold as many Unicode-16 characters
 * as there are ASCII characters in the UTF-8 string.  This in case all UTF-8
 * characters are ASCII characters.  No more will be needed.
 *
 * This function will also accept Java's variant of UTF-8.  This encodes
 * U+0000 as two characters rather than one, so the UTF-8 does not contain
 * any zeroes.
 *
 * @author Copyright (c) 2000 Morten Rolland, Screen Media
 *
 * @param utf8      Input string in UTF8 format.
 * @param cc        Number of bytes to convert.
 * @param unicode16 Destination buffer.
 * @return          Number of characters converted, or -1 if input is not
 *                  valid UTF8.
 */
static int
utf8_to_utf16(const unsigned char *utf8, int cc, unsigned short *unicode16)
{
	int count = 0;
	unsigned char c0, c1;
	unsigned long scalar;

	while(--cc >= 0) {
		c0 = *utf8++;
		/*DPRINTF("Trying: %02x\n",c0);*/

		if ( c0 < 0x80 ) {
			/* Plain ASCII character, simple translation :-) */
			*unicode16++ = c0;
			count++;
			continue;
		}

		if ( (c0 & 0xc0) == 0x80 )
			/* Illegal; starts with 10xxxxxx */
			return -1;

		/* c0 must be 11xxxxxx if we get here => at least 2 bytes */
		scalar = c0;
		if(--cc < 0)
			return -1;
		c1 = *utf8++;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x20) ) {
			/* Two bytes UTF-8 */
			if ( (scalar != 0) && (scalar < 0x80) )
				return -1;	/* Overlong encoding */
			*unicode16++ = scalar & 0x7ff;
			count++;
			continue;
		}

		/* c0 must be 111xxxxx if we get here => at least 3 bytes */
		if(--cc < 0)
			return -1;
		c1 = *utf8++;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x10) ) {
			/*DPRINTF("####\n");*/
			/* Three bytes UTF-8 */
			if ( scalar < 0x800 )
				return -1;	/* Overlong encoding */
			if ( scalar >= 0xd800 && scalar < 0xe000 )
				return -1;	/* UTF-16 high/low halfs */
			*unicode16++ = scalar & 0xffff;
			count++;
			continue;
		}

		/* c0 must be 1111xxxx if we get here => at least 4 bytes */
		c1 = *utf8++;
		if(--cc < 0)
			return -1;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x08) ) {
			/* Four bytes UTF-8, needs encoding as surrogates */
			if ( scalar < 0x10000 )
				return -1;	/* Overlong encoding */
			scalar -= 0x10000;
			*unicode16++ = ((scalar >> 10) & 0x3ff) + 0xd800;
			*unicode16++ = (scalar & 0x3ff) + 0xdc00;
			count += 2;
			continue;
		}

		return -1;	/* No support for more than four byte UTF-8 */
	}
	return count;
}

#if	(SYS_DVB)
/**
 * @Created by Henry
 * Convert text from UNICODE to GBK.
 * Return < 0 on error or can't translate.
 * @20070714
 * @param istr   UNICODE.
 * @param ostr   GBK.
 * @return       Number of bytes converted.
 */
int
GdUnicodeConvertGBK(const void *istr, void*ostr)
{
	const unsigned char 	*istr8;
	unsigned char 		*ostr8;
	unsigned char			ch1,ch2;
	unsigned int			ch;
	int					icc = 0;


	istr8 = istr;
	ostr8 = ostr;

	while(1)
	{
		ch1 = *istr8++;
		ch2 = *istr8++;

		if(ch1 == 0x00 && ch2 == 0x00) //end
		{
			break;
		}
		else
		{
			if(ch1 == 0x00)	//ASCII
			{
				*ostr8++ = ch2;
				icc ++;
			}
			else
			{
				ch = ch1;
				ch = (ch << 8) & 0xFF00;
				ch = ch | ch2;
				ch = uni2gb((unsigned short) ch);
				*ostr8++ = ((ch & 0xFF00)>>8);
				*ostr8++ = (ch & 0x00FF);
				icc += 2;
			}

		}


	}
	return icc;
}
#endif	/* (SYS_DVB) */

/**
 * @Created by jskim
 * Convert text from One Byte Format to GBK.
 * Return < 0 on error or can't translate.
 * @20070829
 * @param istr   One Bye Format.
 * @param ostr   GBK.
 * @return       Number of bytes converted.
 */
int
OneByteFormConvertGBK(const unsigned char *istr, unsigned char *ostr)
{
	const unsigned char 	*istr8;
	unsigned char 		*ostr8;
	unsigned char		ch1;
	int			icc = 0;


	istr8 = istr;
	ostr8 = ostr;

	while(1)
	{
		ch1 = *istr8++;

		if(ch1 == 0x00) //end
		{
			break;
		}
		else
		{
			if( (ch1 >= 0x20) && (ch1 <= 0x7e) )
			{
				*ostr8++ = ch1;
				icc ++;
			}
			else	// Now not changeable character
			{
				*ostr8++ = 0x2A;  // '*'
				icc ++;
			}
		}


	}
	return icc;


}

#if	(SYS_DVB)
/**
 * Make all input string data to GBK
 * Return < 0 on error or can't translate.
 *
 * @param istr   Input string.
 * @param ostr   Output string.
 * @return       Number of bytes. Not include 0x00
 *			-1 means out of GBK
 */
 //Add by Henry
int GdAll2GBK(const void *istr, void *ostr)
{
	const unsigned char *istr8;
	unsigned char 		*ostr8;
	unsigned int		ch;
	int					icc = 0;

	istr8 = istr;
	ostr8 = ostr;
	ch = *istr8;
	switch(ch)
	{

		case 0x01:	// ISO/IEC 8859-5
		case 0x02:	// ISO/IEC 8859-6
		case 0x03:	// ISO/IEC 8859-7
		case 0x04:	// ISO/IEC 8859-8
		case 0x05:	// ISO/IEC 8859-9
		{
			istr8 +=1;	//skip one byte
			icc = OneByteFormConvertGBK(istr8,ostr8);
			break;
		}
		case 0x13:	// GB-2312-1980
		{
			istr8 +=1;	//skip one byte
			strcpy(ostr8,istr8);
			icc = strlen(ostr8);
			break;
		}
		case 0x10:	// ISO/IEC 8859
		{
			istr8 +=3;	//skip three bytes
			icc = OneByteFormConvertGBK(istr8,ostr8);
			break;
		}
		case 0x11:	// ISO/IEC 10646-1 (UNICODE)
		{
			istr8 +=1;
			icc = GdUnicodeConvertGBK(istr8,ostr8);
			break;
		}
		case 0x12:	// KSC 5601-1987
		{
			return -1;
		}
		default:
		{
			if((ch == 0x00) || ((ch >= 0x06) && (ch<=0x0F)) || ((ch >= 0x14) && (ch<=0x1F)) )
			{
				return -1;	//Reserved
			}
			else if((ch >= 0x20)&&(ch <= 0xFF)) //ASCII and no the header byte
			{
				// 방송사가 Table Header 정보 없이 GBK Format Text를 보내고 있어 변경하여 대응함.
				// 기존 Default Table Text가 전송되는 경우 특수 Text 등이 잘못 표시될 수 있게 됨.
				//icc = OneByteFormConvertGBK(istr8,ostr8);

				strcpy(ostr8,istr8);
				icc = strlen(ostr8);
			}

			break;
		}

	}

	return icc;

}
#endif	/* (SYS_DVB) */

unsigned short convert_JisToUnicode(unsigned short text)
{
	if(text == 0x4FA0)
		return 0x4FE0;
	else if(text == 0x4FE3)
		return 0x4FC1;
	else if(text == 0x5024)
		return 0x503C;
	else if(text == 0x5036)
		return 0x4FF1;
	else if(text == 0x5078)
		return 0x5077;
	else if(text == 0x5185)
		return 0x5167;
	else if(text == 0x5239)
		return 0x524E;
	else if(text == 0x5265)
		return 0x525D;
	else if(text == 0x5449)
		return 0x5433;
	else if(text == 0x5451)
		return 0x541E;
	else if(text == 0x5516)
		return 0x555E;
	else if(text == 0x559E)
		return 0x5527;
	else if(text == 0x5618)
		return 0x5653;
	else if(text == 0x56A2)
		return 0x56CA;
	else if(text == 0x5897)
		return 0x589E;
	else if(text == 0x5910)
		return 0x657B;
	else if(text == 0x5A2F)
		return 0x5A1B;
	else if(text == 0x5C61)
		return 0x5C62;
	else if(text == 0x5DD3)
		return 0x5DD4;
	else if(text == 0x5DE3)
		return 0x5DE2;
	else if(text == 0x5E47)
		return 0x5E6B;
	else if(text == 0x5F66)
		return 0x5F65;
	else if(text == 0x5FB3)
		return 0x5FB7;
	else if(text == 0x60A6)
		return 0x6085;
	else if(text == 0x6238)
		return 0x6236;
	else if(text == 0x629B)
		return 0x62CB;
	else if(text == 0x63B2)
		return 0x63ED;
	else if(text == 0x63B4)
		return 0x6451;
	else if(text == 0x63BB)
		return 0x6414;
	else if(text == 0x6483)
		return 0x64CA;
	else if(text == 0x6505)
		return 0x6522;
	else if(text == 0x6669)
		return 0x665A;
	else if(text == 0x66A6)
		return 0x66C6;
	else if(text == 0x66C1)
		return 0x66A8;
	else if(text == 0x6863)
		return 0x6A94;
	else if(text == 0x6A2A)
		return 0x6A6B;
	else if(text == 0x6B69)
		return 0x6B65;
	else if(text == 0x6B73)
		return 0x6B72;
	else if(text == 0x6BCE)
		return 0x6BCF;
	else if(text == 0x6D99)
		return 0x6DDA;
	else if(text == 0x6D9C)
		return 0x7006;
	else if(text == 0x6E07)
		return 0x6E34;
	else if(text == 0x6E09)
		return 0x6D89;
	else if(text == 0x6E8C)
		return 0x6F51;
	else if(text == 0x6F11)
		return 0x6E89;
	else if(text == 0x7114)
		return 0x7130;
	else if(text == 0x72B6)
		return 0x72C0;
	else if(text == 0x7523)
		return 0x7522;
	else if(text == 0x75E9)
		return 0x7626;
	else if(text == 0x7977)
		return 0x79B1;
	else if(text == 0x7A0E)
		return 0x7A05;
	else if(text == 0x7BAA)
		return 0x7C1E;
	else if(text == 0x7CA4)
		return 0x7CB5;
	else if(text == 0x7D4B)
		return 0x7E8A;
	else if(text == 0x7D76)
		return 0x7D55;
	else if(text == 0x7DD1)
		return 0x7DA0;
	else if(text == 0x7E01)
		return 0x7DE3;
	else if(text == 0x7E4B)
		return 0x7E6B;
	else if(text == 0x7E66)
		return 0x7E48;
	else if(text == 0x7E89)
		return 0x7E98;
	else if(text == 0x8131)
		return 0x812B;
	else if(text == 0x83B1)
		return 0x840A;
	else if(text == 0x848B)
		return 0x8523;
	else if(text == 0x85AB)
		return 0x85B0;
	else if(text == 0x865A)
		return 0x865B;
	else if(text == 0x8749)
		return 0x87EC;
	else if(text == 0x874B)
		return 0x881F;
	else if(text == 0x8AAC)
		return 0x8AAA;
	else if(text == 0x8EAF)
		return 0x8EC0;
	else if(text == 0x8EB1)
		return 0x8EB2;
	else if(text == 0x90F7)
		return 0x9109;
	else if(text == 0x9197)
		return 0x91B1;
	else if(text == 0x91A4)
		return 0x91AC;
	else if(text == 0x9332)
		return 0x9304;
	else if(text == 0x95B2)
		return 0x95B1;
	else if(text == 0x982C)
		return 0x9830;
	else if(text == 0x983C)
		return 0x8CF4;
	else if(text == 0x983D)
		return 0x9839;
	else if(text == 0x9A28)
		return 0x9A52;
	else if(text == 0x9D0E)
		return 0x9DD7;
	else if(text == 0x9DC4)
		return 0x96DE;
	else if(text == 0x9E78)
		return 0x9E7C;
	else if(text == 0x9EB9)
		return 0x9EB4;
	else if(text == 0x9EBA)
		return 0x9EB5;
	else if(text == 0x9ED2)
		return 0x9ED1;

	return text;
}
