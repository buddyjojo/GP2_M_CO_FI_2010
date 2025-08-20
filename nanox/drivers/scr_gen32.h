#ifndef _SCR_GEN32_H_
#define _SCR_GEN32_H_

extern void ScrGen32_DrawPixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c);
extern MWPIXELVAL ScrGen32_ReadPixel(PSD psd, MWCOORD x, MWCOORD y);
extern void ScrGen32_DrawHorzLine(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c);
extern void ScrGen32_DrawVertLine(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c);
extern void ScrGen32_FillRect(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c);
extern void ScrGen32_Blit(PSD dstpsd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PSD srcpsd, MWCOORD srcX, MWCOORD srcY, long op);
extern PSD ScrGen32_AllocateMemGC(PSD psd);
extern MWBOOL ScrGen32_MapMemGC(PSD memPsd, MWCOORD w, MWCOORD h, int planes, int bpp, int linelen,
	int size, void *pAddr);
extern void ScrGen32_FreeMemGC(PSD mempsd);
extern void ScrGen32_StretchBlit(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD dstW,	MWCOORD dstH,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY, MWCOORD srcW, MWCOORD srcH, long op);
extern void ScrGen32_StretchBlitEx(PSD dstPsd, PSD srcPsd, MWCOORD dest_x_start, MWCOORD dest_y_start, MWCOORD width, MWCOORD height,
	int x_denominator, int y_denominator, int src_x_fraction, int src_y_fraction, int x_step_fraction, int y_step_fraction, long op);
extern void ScrGen32_DrawImage(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY, MWPIXELVAL *pConvTable);
extern MWBOOL ScrGen32_CanDrawImage(PSD psd, PMWIMAGEHDR pimage);
extern void ScrGen32_DrawChar(PSD psd, MWCOORD dstX, MWCOORD dstY, int glyphBitCount,
	unsigned char *glyph, int glyphLineLength, MWCOORD glyphX, MWCOORD glyphY, MWCOORD glyphW, MWCOORD glyphH, MWPIXELVAL c);

#endif
