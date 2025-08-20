/* T.J.Park Note : This file is similiar to fblin32.c included in the original MicroWindows 0.90,
   which supports a general memory screen device.
   If the frame buffer can be accessed such as normal memory block, these functions will work properly.

   But followings are different from the original fblin32.c
   - New APIs : DrawImage, CanDrawImage, DrawChar,...
   - Three drawing modes are clearly supported : COPY, CLEAR, and SRC_OVER
   - Improve the performance.
*/
#include "device.h"
#include "scr_gen32.h"

extern int			gr_mode;	/* drawing mode */
extern int			gr_alpha;	/* global alpha */
//extern int			gr_gradation_startpos; /* gradiation start pos, used in text drawing */
extern MWGRADATION gr_gradation; /* gradiation effect used in text drawing */
extern MWPIXELVAL	gr_xorcolor; /* xorcolr */

#define DIV255(value)		((((value)<<8)+(value)+0xff)>>16)

static unsigned char _UTFColorCovTab[68] = {
        // 0 : BG
        0x00,
        // 1~64 : FG
        0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40,
        0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80,
        0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc, 0xc0,
        0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc, 0xff,
        // 65 : Shadow
        0xff,
        // 66~67 : Smooth
        0xff, 0xff
};

void ScrGen32_DrawPixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
	unsigned long *pAddr, alphaValue;
	unsigned long srcA, srcR, srcG, srcB, dstA, dstR, dstG, dstB, rDst;

	pAddr = ((unsigned long *)(psd->addr)) + y * psd->linelen + x;
	if ((gr_mode!=MWMODE_XOR_FGBG)&&(gr_alpha < 0xff && gr_alpha >= 0)) {
		alphaValue = c>>24, alphaValue = DIV255(gr_alpha*alphaValue);
		c = (alphaValue<<24)|(c&0xffffff);
	}

	switch (gr_mode) {
		case MWMODE_COPY :
		default :
			*pAddr = c;
			break;

		case MWMODE_CLEAR :
			*pAddr = 0;
			break;

		case MWMODE_XOR_FGBG :
			*pAddr = (c ^ gr_xorcolor ^ *pAddr ) | 0xff000000;
			break;

		case MWMODE_SRC_OVER :
			srcA = (c>>24), srcR = (c>>16)&0xff, srcG = (c>>8)&0xff, srcB = c&0xff;
			if (srcA==0x00)
				break;

			dstA = (*pAddr>>24), dstR = (*pAddr>>16)&0xff, dstG = (*pAddr>>8)&0xff, dstB = *pAddr&0xff;
			if ((srcA==0xff)||(dstA==0)) {
				*pAddr = c;
				break;
			}

			srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

			rDst = 0xff-srcA;
			dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

			dstA = DIV255((srcA*0xff)+dstA*rDst);
			dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

			dstA = (dstA>0xff ? 0xff : dstA);
			dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

			*pAddr = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
			break;
	}

}

MWPIXELVAL ScrGen32_ReadPixel(PSD psd, MWCOORD x, MWCOORD y)
{
	return *(((unsigned long *)(psd->addr)) + y * psd->linelen + x);
}

static void ScrGen32_DrawHorzLineDefault(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
	unsigned long *pAddr, alphaValue;

	pAddr = ((unsigned long *)(psd->addr)) + y * psd->linelen + x1;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		alphaValue = c>>24, alphaValue = DIV255(gr_alpha*alphaValue);
		c = (alphaValue<<24)|(c&0xffffff);
	}

	for (; x1<=x2; x1++, pAddr++)
		*pAddr = c;
}

static void ScrGen32_DrawHorzLineXor(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
	unsigned long *pAddr;

	pAddr = ((unsigned long *)(psd->addr)) + y * psd->linelen + x1;

	for (; x1<=x2; x1++, pAddr++)
		*pAddr = (c ^ gr_xorcolor ^ *pAddr)|0xff000000;
}

static void ScrGen32_DrawHorzLineAlpha(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
	unsigned long *pAddr, alphaValue;
	unsigned long srcA, srcR, srcG, srcB, dst, dstA, dstR, dstG, dstB, calc, rDst;

	pAddr = ((unsigned long *)(psd->addr)) + y * psd->linelen + x1;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		alphaValue = c>>24, alphaValue = DIV255(gr_alpha*alphaValue);
		c = (alphaValue<<24)|(c&0xffffff);
	}

	srcA = (c>>24), srcR = (c>>16)&0xff, srcG = (c>>8)&0xff, srcB = c&0xff;
	if (srcA==0x00)
		return;

	srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;
	dst = calc =  ~(*pAddr);//0;
	for (; x1<=x2; x1++, pAddr++) {
		if (*pAddr==dst) {	// same pixels, same result
			*pAddr = calc;
			continue;
		}

		dstA = (*pAddr>>24), dstR = (*pAddr>>16)&0xff, dstG = (*pAddr>>8)&0xff, dstB = *pAddr&0xff;
		if ((srcA==0xff)||(dstA==0)) {
			*pAddr = c;
			continue;
		}

		rDst = 0xff-srcA;
		dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

		dstA = DIV255((srcA*0xff)+dstA*rDst);
		dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

		dstA = (dstA>0xff ? 0xff : dstA);
		dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

		dst = *pAddr, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
		*pAddr = calc;
	}
}

void ScrGen32_DrawHorzLine(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
	switch(gr_mode) {
		case MWMODE_COPY:
		default:
			ScrGen32_DrawHorzLineDefault(psd, x1, x2, y, c);
			break;

		case MWMODE_CLEAR:
			ScrGen32_DrawHorzLineDefault(psd, x1, x2, y, 0);
			break;

		case MWMODE_XOR_FGBG:
			ScrGen32_DrawHorzLineXor(psd, x1, x2, y, c);
			break;

		case MWMODE_SRC_OVER:
			if (gr_alpha < 0) {
				if ((c&0xff000000)==0xff000000)
					ScrGen32_DrawHorzLineDefault(psd, x1, x2, y, c);
				else if ((c&0xff000000)>0)
					ScrGen32_DrawHorzLineAlpha(psd, x1, x2, y, c);
			}
			else {
				if ((gr_alpha==0xff)&&((c&0xff000000)==0xff000000))
					ScrGen32_DrawHorzLineDefault(psd, x1, x2, y, c);
				else if ((gr_alpha>0)&&((c&0xff000000)>0))
					ScrGen32_DrawHorzLineAlpha(psd, x1, x2, y, c);
			}
			break;
	}
}

static void ScrGen32_DrawVertLineDefault(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
	unsigned long *pAddr, alphaValue;

	pAddr = ((unsigned long *)(psd->addr)) + y1 * psd->linelen + x;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		alphaValue = c>>24, alphaValue = DIV255(gr_alpha*alphaValue);
		c = (alphaValue<<24)|(c&0xffffff);
	}

	for (; y1<=y2; y1++, pAddr+=psd->linelen)
		*pAddr = c;
}

static void ScrGen32_DrawVertLineXor(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
	unsigned long *pAddr;

	pAddr = ((unsigned long *)(psd->addr)) + y1 * psd->linelen + x;

	for (; y1<=y2; y1++, pAddr+=psd->linelen)
		*pAddr = (c ^ gr_xorcolor ^ *pAddr)|0xff000000;
}

static void ScrGen32_DrawVertLineAlpha(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
	unsigned long *pAddr, alphaValue;
	unsigned long srcA, srcR, srcG, srcB, dst, dstA, dstR, dstG, dstB, calc, rDst;

	pAddr = ((unsigned long *)(psd->addr)) + y1 * psd->linelen + x;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		alphaValue = c>>24, alphaValue = DIV255(gr_alpha*alphaValue);
		c = (alphaValue<<24)|(c&0xffffff);
	}

	srcA = (c>>24), srcR = (c>>16)&0xff, srcG = (c>>8)&0xff, srcB = c&0xff;
	if (srcA==0x00)
		return;

	srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;
	dst = calc =  ~(*pAddr);//0;
	for (; y1<=y2; y1++, pAddr+=psd->linelen) {
		if (*pAddr==dst) {	// same pixels, same result
			*pAddr = calc;
			continue;
		}

		dstA = (*pAddr>>24), dstR = (*pAddr>>16)&0xff, dstG = (*pAddr>>8)&0xff, dstB = *pAddr&0xff;
		if ((srcA==0xff)||(dstA==0)) {
			*pAddr = c;
			continue;
		}

		rDst = 0xff-srcA;
		dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

		dstA = DIV255((srcA*0xff)+dstA*rDst);
		dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

		dstA = (dstA>0xff ? 0xff : dstA);
		dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

		dst = *pAddr, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
		*pAddr = calc;
	}
}

void ScrGen32_DrawVertLine(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
	switch(gr_mode) {
		case MWMODE_COPY:
		default:
			ScrGen32_DrawVertLineDefault(psd, x, y1, y2, c);
			break;

		case MWMODE_CLEAR:
			ScrGen32_DrawVertLineDefault(psd, x, y1, y2, 0);
			break;

		case MWMODE_XOR_FGBG:
			ScrGen32_DrawVertLineXor(psd, x, y1, y2, c);
			break;

		case MWMODE_SRC_OVER:
			if (gr_alpha < 0) {
				if ((c&0xff000000)==0xff000000)
					ScrGen32_DrawVertLineDefault(psd, x, y1, y2, c);
				else if ((c&0xff000000)>0)
					ScrGen32_DrawVertLineAlpha(psd, x, y1, y2, c);
			}
			else {
				if ((gr_alpha==0xff)&&((c&0xff000000)==0xff000000))
					ScrGen32_DrawVertLineDefault(psd, x, y1, y2, c);
				else if ((gr_alpha>0)&&((c&0xff000000)>0))
					ScrGen32_DrawVertLineAlpha(psd, x, y1, y2, c);
			}
			break;
	}
}

static void ScrGen32_FillRectDefault(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c)
{
	unsigned long *pAddr, *pa, alphaValue;
	int width, height;

	pAddr = ((unsigned long *)(psd->addr)) + y1 * psd->linelen + x1;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		alphaValue = c>>24, alphaValue = DIV255(gr_alpha*alphaValue);
		c = (alphaValue<<24)|(c&0xffffff);
	}

	for (height=y2-y1+1; height>0; height--, pAddr+=psd->linelen)
		for (width=x2-x1+1, pa=pAddr; width>0; width--, pa++)
			*pa = c;
}

static void ScrGen32_FillRectXor(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c)
{
	unsigned long *pAddr, *pa;
	int width, height;

	pAddr = ((unsigned long *)(psd->addr)) + y1 * psd->linelen + x1;

	for (height=y2-y1+1; height>0; height--, pAddr+=psd->linelen)
		for (width=x2-x1+1, pa=pAddr; width>0; width--, pa++)
			*pa = (c ^ gr_xorcolor ^ *pa)|0xff000000;
}

static void ScrGen32_FillRectAlpha(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c)
{
	unsigned long *pAddr, *pa, alphaValue;
	unsigned long srcA, srcR, srcG, srcB, dst, dstA, dstR, dstG, dstB, calc, rDst;
	int width, height;

	pAddr = ((unsigned long *)(psd->addr)) + y1 * psd->linelen + x1;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		alphaValue = c>>24, alphaValue = DIV255(gr_alpha*alphaValue);
		c = (alphaValue<<24)|(c&0xffffff);
	}

	srcA = (c>>24), srcR = (c>>16)&0xff, srcG = (c>>8)&0xff, srcB = c&0xff;
	if (srcA==0x00)
		return;

	srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;
	dst = calc = ~(*pAddr);//0;
	for (height=y2-y1+1; height>0; height--, pAddr+=psd->linelen)
		for (width=x2-x1+1, pa=pAddr; width>0; width--, pa++) {
			if (*pa==dst) {	// same pixels, same result
				*pa = calc;
				continue;
			}

			dstA = (*pa>>24), dstR = (*pa>>16)&0xff, dstG = (*pa>>8)&0xff, dstB = *pa&0xff;
			if ((srcA==0xff)||(dstA==0)) {
				*pa = c;
				continue;
			}

			rDst = 0xff-srcA;
			dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

			dstA = DIV255((srcA*0xff)+dstA*rDst);
			dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

			dstA = (dstA>0xff ? 0xff : dstA);
			dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

			dst = *pa, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
			*pa = calc;
		}
}

void ScrGen32_FillRect(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c)
{
	switch(gr_mode) {
		case MWMODE_COPY:
		default:
			ScrGen32_FillRectDefault(psd, x1, y1, x2, y2, c);
			break;

		case MWMODE_CLEAR:
			ScrGen32_FillRectDefault(psd, x1, y1, x2, y2, 0);
			break;

		case MWMODE_XOR_FGBG:
			ScrGen32_FillRectXor(psd, x1, y1, x2, y2, c);
			break;

		case MWMODE_SRC_OVER:
			if (gr_alpha < 0) {
				if ((c&0xff000000)==0xff000000)
					ScrGen32_FillRectDefault(psd, x1, y1, x2, y2, c);
				else if ((c&0xff000000)>0)
					ScrGen32_FillRectAlpha(psd, x1, y1, x2, y2, c);
			}
			else {
				if ((gr_alpha==0xff)&&((c&0xff000000)==0xff000000))
					ScrGen32_FillRectDefault(psd, x1, y1, x2, y2, c);
				else if ((gr_alpha>0)&&((c&0xff000000)>0))
					ScrGen32_FillRectAlpha(psd, x1, y1, x2, y2, c);
			}
			break;
	}
}

static void ScrGen32_BlitDefault(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int wi, hi, srcLineLength, dstLineLength;
	unsigned long alphaValue;

	srcLineLength = srcPsd->linelen;
	dstLineLength = dstPsd->linelen;
	pSrc = ((unsigned long *)(srcPsd->addr)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(dstPsd->addr)) + dstY * dstLineLength + dstX;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				alphaValue = *ps>>24, alphaValue = DIV255(gr_alpha*alphaValue);
				*pd = (alphaValue<<24)|(*ps&0xffffff);
			}
	}
	else {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++)
				*pd = *ps;
	}
}

static void ScrGen32_BlitXor(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int wi, hi, srcLineLength, dstLineLength;

	srcLineLength = srcPsd->linelen;
	dstLineLength = dstPsd->linelen;
	pSrc = ((unsigned long *)(srcPsd->addr)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(dstPsd->addr)) + dstY * dstLineLength + dstX;

	for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
		for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++)
			if (*ps&0xff000000)
				*pd = (gr_xorcolor ^ *ps ^ *pd) | 0xFF000000;
}

static void ScrGen32_BlitAlpha(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int wi, hi, srcLineLength, dstLineLength;
	unsigned long src, srcA, srcR, srcG, srcB, dst, dstA, dstR, dstG, dstB, calc, rDst;

	srcLineLength = srcPsd->linelen;
	dstLineLength = dstPsd->linelen;
	pSrc = ((unsigned long *)(srcPsd->addr)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(dstPsd->addr)) + dstY * dstLineLength + dstX;
	src = dst = calc = 0;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				if ((*ps==src)&&(*pd==dst)) {	// same pixels, same result
					*pd = calc;
					continue;
				}

				srcA = (*ps>>24), srcR = (*ps>>16)&0xff, srcG = (*ps>>8)&0xff, srcB = *ps&0xff;
				srcA = DIV255(gr_alpha*srcA);
				if (srcA==0x00)
					continue;

				dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
				if ((srcA==0xff)||(dstA==0)) {
					*pd = (srcA<<24)|(srcR<<16)|(srcG<<8)|srcB;//*ps;
					continue;
				}

				srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

				rDst = 0xff-srcA;
				dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

				dstA = DIV255((srcA*0xff)+dstA*rDst);
				dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				src = *ps, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
				*pd = calc;
			}
	}
	else {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				if ((*ps==src)&&(*pd==dst)) {	// same pixels, same result
					*pd = calc;
					continue;
				}

				srcA = (*ps>>24), srcR = (*ps>>16)&0xff, srcG = (*ps>>8)&0xff, srcB = *ps&0xff;
				if (srcA==0x00)
					continue;

				dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
				if ((srcA==0xff)||(dstA==0)) {
					*pd = *ps;
					continue;
				}

				srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

				rDst = 0xff-srcA;
				dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

				dstA = DIV255((srcA*0xff)+dstA*rDst);
				dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				src = *ps, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
				*pd = calc;
			}
	}
}

void ScrGen32_Blit(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY, long op)
{
	switch(op & MWROP_EXTENSION) {
		case MWROP_COPY:
		default:
			ScrGen32_BlitDefault(dstPsd, dstX, dstY, width, height, srcPsd, srcX, srcY);
			break;

		case MWROP_CLEAR:
			ScrGen32_FillRectDefault(dstPsd, dstX, dstY, dstX+width-1, dstY+height-1, 0);
			break;

		case MWROP_XOR_FGBG:
			ScrGen32_BlitXor(dstPsd, dstX, dstY, width, height, srcPsd, srcX, srcY);
			break;

		case MWROP_SRC_OVER:
			ScrGen32_BlitAlpha(dstPsd, dstX, dstY, width, height, srcPsd, srcX, srcY);
			break;
	}
}

PSD ScrGen32_AllocateMemGC(PSD psd)
{
	PSD	mempsd;

	/* if driver doesn't have blit, fail*/
	if((psd->flags & PSF_HAVEBLIT) == 0)
		return NULL;

	mempsd = malloc(sizeof(SCREENDEVICE));
	if (!mempsd)
		return NULL;

	/* copy passed device get initial values*/
	*mempsd = *psd;

	/* initialize*/
	mempsd->flags |= PSF_MEMORY;
	mempsd->flags &= ~PSF_SCREEN;
	mempsd->addr = NULL;

	return mempsd;
}

MWBOOL ScrGen32_MapMemGC(PSD memPsd, MWCOORD w, MWCOORD h, int planes, int bpp, int linelen,
	int size, void *pAddr)
{
	if(!(memPsd->flags & PSF_MEMORY))
		return FALSE;

	/* create mem psd w/h aligned with hw screen w/h*/
	memPsd->xres = w;
	memPsd->yres = h;
	memPsd->xvirtres = w;
	memPsd->yvirtres = h;
	memPsd->planes = planes;
	memPsd->bpp = bpp;
	memPsd->linelen = linelen;
	memPsd->size = size;
	memPsd->addr = pAddr;

	return 1;
}

void ScrGen32_FreeMemGC(PSD mempsd)
{
	if(!(mempsd->flags & PSF_MEMORY))
		return;

	/* note: mempsd->addr must be freed elsewhere*/

	free(mempsd);
}

static void ScrGen32_StretchBlitDefault(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD dstW,	MWCOORD dstH,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY, MWCOORD srcW, MWCOORD srcH)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int	wi, hi, srcLineLength, dstLineLength;
	int	rowPos, rowInc, colPos, colInc;
	unsigned long alphaValue;

	dstLineLength = dstPsd->linelen, srcLineLength = srcPsd->linelen;
	rowInc = (srcH << 16) / dstH, colInc = (srcW << 16) / dstW;

	pDst = (unsigned long*)dstPsd->addr + dstX + dstY * dstLineLength;
	pSrc = (unsigned long*)srcPsd->addr + srcX + srcY * srcLineLength;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		for (rowPos=0, hi=dstH; hi>0; hi--) {
			for (pd=pDst, ps=pSrc, colPos=0, wi=dstW; wi>0; wi--) {
				alphaValue = DIV255(gr_alpha * MW_SYS_GET_A(*ps));
				alphaValue = *ps>>24, alphaValue = DIV255(gr_alpha*alphaValue);
				*pd = (alphaValue<<24)|(*ps&0xffffff);

				pd++;
				for (colPos+=colInc; colPos>=0x10000; colPos-=0x10000, ps++) { ; }
			}

			pDst += dstLineLength;
			for (rowPos+=rowInc; rowPos>=0x10000; rowPos-=0x10000, pSrc+=srcLineLength) { ; }
		}
	}
	else {
		for (rowPos=0, hi=dstH; hi>0; hi--) {
			for (pd=pDst, ps=pSrc, colPos=0, wi=dstW; wi>0; wi--) {
				*pd = *ps;

				pd++;
				for (colPos+=colInc; colPos>=0x10000; colPos-=0x10000, ps++) { ; }
			}

			pDst += dstLineLength;
			for (rowPos+=rowInc; rowPos>=0x10000; rowPos-=0x10000, pSrc+=srcLineLength) { ; }
		}
	}
}

static void ScrGen32_StretchBlitXor(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD dstW,	MWCOORD dstH,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY, MWCOORD srcW, MWCOORD srcH)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int	wi, hi, srcLineLength, dstLineLength;
	int	rowPos, rowInc, colPos, colInc;

	dstLineLength = dstPsd->linelen;
	srcLineLength = srcPsd->linelen;

	rowInc = (srcH << 16) / dstH;
	colInc = (srcW << 16) / dstW;

	pDst = (unsigned long*)dstPsd->addr + dstX + dstY * dstLineLength;
	pSrc = (unsigned long*)srcPsd->addr + srcX + srcY * srcLineLength;

	for (rowPos=0, hi=dstH; hi>0; hi--) {
		for (pd=pDst, ps=pSrc, colPos=0, wi=dstW; wi>0; wi--) {
			if (*ps&0xff000000)
				*pd = (*ps ^ gr_xorcolor ^ *pd)|0xff000000;

			pd++;
			for (colPos+=colInc; colPos>=0x10000; colPos-=0x10000, ps++) { ; }
		}

		pDst += dstLineLength;
		for (rowPos+=rowInc; rowPos>=0x10000; rowPos-=0x10000, pSrc+=srcLineLength) { ; }
	}
}

static void ScrGen32_StretchBlitAlpha(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD dstW,	MWCOORD dstH,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY, MWCOORD srcW, MWCOORD srcH)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int	wi, hi, srcLineLength, dstLineLength;
	int	rowPos, rowInc, colPos, colInc;
	unsigned long src, srcA, srcR, srcG, srcB, dst, dstA, dstR, dstG, dstB, calc, rDst;

	dstLineLength = dstPsd->linelen, srcLineLength = srcPsd->linelen;
	rowInc = (srcH << 16) / dstH, colInc = (srcW << 16) / dstW;

	pDst = (unsigned long*)dstPsd->addr + dstX + dstY * dstLineLength;
	pSrc = (unsigned long*)srcPsd->addr + srcX + srcY * srcLineLength;
	src = dst = calc = 0;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		for (rowPos=0, hi=dstH; hi>0; hi--) {
			for (pd=pDst, ps=pSrc, colPos=0, wi=dstW; wi>0; wi--) {
				if ((*ps==src)&&(*pd==dst)) {	// same pixels, same result
					*pd = calc;
					goto ScrGen32_StretchBlitAlpha_Skip1;
				}

				srcA = (*ps>>24), srcR = (*ps>>16)&0xff, srcG = (*ps>>8)&0xff, srcB = *ps&0xff;
				srcA = DIV255(gr_alpha*srcA);
				if (srcA==0x00)
					goto ScrGen32_StretchBlitAlpha_Skip1;

				dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
				if ((srcA==0xff)||(dstA==0)) {
					*pd = (srcA<<24)|(srcR<<16)|(srcG<<8)|srcB;//*ps;
					goto ScrGen32_StretchBlitAlpha_Skip1;
				}

				srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

				rDst = 0xff-srcA;
				dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

				dstA = DIV255((srcA*0xff)+dstA*rDst);
				dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				src = *ps, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
				*pd = calc;

ScrGen32_StretchBlitAlpha_Skip1 :
				pd++;
				for (colPos+=colInc; colPos>=0x10000; colPos-=0x10000, ps++) { ; }
			}

			pDst += dstLineLength;
			for (rowPos+=rowInc; rowPos>=0x10000; rowPos-=0x10000, pSrc+=srcLineLength) { ; }
		}
	}
	else {
		for (rowPos=0, hi=dstH; hi>0; hi--) {
			for (pd=pDst, ps=pSrc, colPos=0, wi=dstW; wi>0; wi--) {
				if ((*ps==src)&&(*pd==dst)) {	// same pixels, same result
					*pd = calc;
					goto ScrGen32_StretchBlitAlpha_Skip2;
				}

				srcA = (*ps>>24), srcR = (*ps>>16)&0xff, srcG = (*ps>>8)&0xff, srcB = *ps&0xff;
				if (srcA==0x00)
					goto ScrGen32_StretchBlitAlpha_Skip2;

				dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
				if ((srcA==0xff)||(dstA==0)) {
					*pd = *ps;
					goto ScrGen32_StretchBlitAlpha_Skip2;
				}

				srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

				rDst = 0xff-srcA;
				dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

				dstA = DIV255((srcA*0xff)+dstA*rDst);
				dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				src = *ps, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
				*pd = calc;

ScrGen32_StretchBlitAlpha_Skip2 :
				pd++;
				for (colPos+=colInc; colPos>=0x10000; colPos-=0x10000, ps++) { ; }
			}

			pDst += dstLineLength;
			for (rowPos+=rowInc; rowPos>=0x10000; rowPos-=0x10000, pSrc+=srcLineLength) { ; }
		}
	}
}

void ScrGen32_StretchBlit(PSD dstPsd, MWCOORD dstX, MWCOORD dstY, MWCOORD dstW,	MWCOORD dstH,
	PSD srcPsd, MWCOORD srcX, MWCOORD srcY, MWCOORD srcW, MWCOORD srcH, long op)
{
	switch (op & MWROP_EXTENSION) {
		case MWROP_COPY:
		default:
			ScrGen32_StretchBlitDefault(dstPsd, dstX, dstY, dstW, dstH, srcPsd, srcX, srcY, srcW, srcH);
			break;

		case MWROP_CLEAR:
			ScrGen32_FillRectDefault(dstPsd, dstX, dstY, dstX + dstW - 1, dstY + dstH - 1, 0);
			break;

		case MWROP_XOR_FGBG:
			ScrGen32_StretchBlitXor(dstPsd, dstX, dstY, dstW, dstH, srcPsd, srcX, srcY, srcW, srcH);
			break;

		case MWROP_SRC_OVER:
			ScrGen32_StretchBlitAlpha(dstPsd, dstX, dstY, dstW, dstH, srcPsd, srcX, srcY, srcW, srcH);
			break;
	}
}

static void ScrGen32_Draw8BppImageDefault(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY, MWPIXELVAL *pConvTable)
{
	unsigned long *pDst, *pd, srcCol;
	unsigned char *pSrc, *ps;
	int wi, hi, srcLineLength, dstLineLength;
	unsigned long alphaValue;

	srcLineLength = pImage->pitch;
	dstLineLength = psd->linelen;
	pSrc = ((unsigned char *)(pImage->imagebits)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(psd->addr)) + dstY * dstLineLength + dstX;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				if (*ps==pImage->transcolor)
					continue;
				srcCol = pConvTable[*ps];
				alphaValue = srcCol>>24, alphaValue = DIV255(gr_alpha*alphaValue);
				*pd = (alphaValue<<24)|(srcCol&0xffffff);
			}
	}
	else {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				if (*ps==pImage->transcolor)
					continue;
				*pd = pConvTable[*ps];
			}
	}
}

static void ScrGen32_Draw8BppImageXor(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY, MWPIXELVAL *pConvTable)
{
	unsigned long *pDst, *pd;
	unsigned char *pSrc, *ps;
	int wi, hi, srcLineLength, dstLineLength;

	srcLineLength = pImage->pitch;
	dstLineLength = psd->linelen;
	pSrc = ((unsigned char *)(pImage->imagebits)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(psd->addr)) + dstY * dstLineLength + dstX;

	for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
		for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
			if (*ps==pImage->transcolor)
				continue;
			*pd = (gr_xorcolor ^ pConvTable[*ps] ^ *pd)|0xff000000;
		}
}

static void ScrGen32_Draw8BppImageWithGlobalAlpha(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY, MWPIXELVAL *pConvTable)
{
	unsigned long *pDst, *pd;
	unsigned char *pSrc, *ps;
	int wi, hi, srcLineLength, dstLineLength;
	unsigned long srcCol, src, srcA, srcR, srcG, srcB, dst, dstA, dstR, dstG, dstB, calc, rDst;

	srcLineLength = pImage->pitch;
	dstLineLength = psd->linelen;
	pSrc = ((unsigned char *)(pImage->imagebits)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(psd->addr)) + dstY * dstLineLength + dstX;
	src = dst = calc = 0;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				if (*ps==pImage->transcolor)
					continue;

				srcCol = pConvTable[*ps];
				if ((srcCol==src)&&(*pd==dst)) {	// same pixels, same result
					*pd = calc;
					continue;
				}

				srcA = (srcCol>>24), srcR = (srcCol>>16)&0xff, srcG = (srcCol>>8)&0xff, srcB = srcCol&0xff;
				srcA = DIV255(gr_alpha*srcA);
				if (srcA==0x00)
					continue;

				dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
				if ((srcA==0xff)||(dstA==0)) {
					*pd = srcCol;
					continue;
				}

				srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

				rDst = 0xff-srcA;
				dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

				dstA = DIV255((srcA*0xff)+dstA*rDst);
				dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				src = srcCol, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
				*pd = calc;
			}
	}
	else {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				if (*ps==pImage->transcolor)
					continue;

				srcCol = pConvTable[*ps];
				if ((srcCol==src)&&(*pd==dst)) {	// same pixels, same result
					*pd = calc;
					continue;
				}

				srcA = (srcCol>>24), srcR = (srcCol>>16)&0xff, srcG = (srcCol>>8)&0xff, srcB = srcCol&0xff;
				if (srcA==0x00)
					continue;

				dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
				if ((srcA==0xff)||(dstA==0)) {
					*pd = srcCol;
					continue;
				}

				srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

				rDst = 0xff-srcA;
				dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

				dstA = DIV255((srcA*0xff)+dstA*rDst);
				dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				src = srcCol, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
				*pd = calc;
			}
	}
}

void ScrGen32_Draw32BppImageGradation(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int wi, hi, srcLineLength, dstLineLength;
	unsigned long src, srcA, srcR, srcG, srcB, dst, dstA, dstR, dstG, dstB, calc, rDst;
	signed long ratio = 100;

	if(gr_gradation.length == 0) return;

	srcLineLength = pImage->pitch >> 2;
	dstLineLength = psd->linelen;
	pSrc = ((unsigned long *)(pImage->imagebits)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(psd->addr)) + dstY * dstLineLength + dstX;
	src = dst = calc = 0;

	for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)	{
		if(gr_gradation.direction == GRADATION_UP)
			ratio = (dstY + height - hi - gr_gradation.start_pos) * 100 / gr_gradation.length;
		else if(gr_gradation.direction == GRADATION_DOWN)
			ratio = (gr_gradation.length - (dstY + height - hi - gr_gradation.start_pos)) * 100 / gr_gradation.length;
			//ratio = (height - hi) * 100 / height;

		for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
			if(gr_gradation.direction == GRADATION_LEFT)
				ratio = (dstX + width - wi - gr_gradation.start_pos) * 100 / gr_gradation.length;
			else if(gr_gradation.direction == GRADATION_RIGHT)
				ratio = (gr_gradation.length - (dstX + width - wi - gr_gradation.start_pos)) * 100 / gr_gradation.length;

			if(ratio < 0)	ratio = 0;
			if(ratio > 100)	ratio = 100;

			if ((*ps==src)&&(*pd==dst)) {	// same pixels, same result
				*pd = calc;
				continue;
			}

			srcA = (*ps>>24), srcR = (*ps>>16)&0xff, srcG = (*ps>>8)&0xff, srcB = *ps&0xff;
			if (gr_alpha < 0xff && gr_alpha >= 0)
				srcA = DIV255(gr_alpha*srcA);
			if (srcA==0x00)
				continue;
			srcA = srcA * ratio / 100 * 8/10;	//gradation 효과를 약하게 나타내기 위해 (x 0.8)

			dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
			if ((srcA==0xff)||(dstA==0)) {
				*pd = *ps;
				continue;
			}

			srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

			rDst = 0xff-srcA;
			dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

			dstA = DIV255((srcA*0xff)+dstA*rDst);
			dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

			dstA = (dstA>0xff ? 0xff : dstA);
			dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

			src = *ps, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
			*pd = calc;
		}
	}
}

static void ScrGen32_Draw32BppImageDefault(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int wi, hi, srcLineLength, dstLineLength;
	unsigned long alphaValue;

	srcLineLength = pImage->pitch >> 2;
	dstLineLength = psd->linelen;
	pSrc = ((unsigned long *)(pImage->imagebits)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(psd->addr)) + dstY * dstLineLength + dstX;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				alphaValue = *ps>>24, alphaValue = DIV255(gr_alpha*alphaValue);
				*pd = (alphaValue<<24)|(*ps&0xffffff);
			}
	}
	else {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++)
				*pd = *ps;
	}
}

static void ScrGen32_Draw32BppImageXor(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int wi, hi, srcLineLength, dstLineLength;

	srcLineLength = pImage->pitch >> 2;
	dstLineLength = psd->linelen;
	pSrc = ((unsigned long *)(pImage->imagebits)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(psd->addr)) + dstY * dstLineLength + dstX;

	for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
		for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++)
			if (*ps&0xff000000)
				*pd = (gr_xorcolor ^ *ps ^ *pd) | 0xFF000000;
}

static void ScrGen32_Draw32BppImageAlpha(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY)
{
	unsigned long *pSrc, *ps, *pDst, *pd;
	int wi, hi, srcLineLength, dstLineLength;
	unsigned long src, srcA, srcR, srcG, srcB, dst, dstA, dstR, dstG, dstB, calc, rDst;

	srcLineLength = pImage->pitch >> 2;
	dstLineLength = psd->linelen;
	pSrc = ((unsigned long *)(pImage->imagebits)) + srcY * srcLineLength + srcX;
	pDst = ((unsigned long *)(psd->addr)) + dstY * dstLineLength + dstX;
	src = dst = calc = 0;

	if (gr_alpha < 0xff && gr_alpha >= 0) {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				if ((*ps==src)&&(*pd==dst)) {	// same pixels, same result
					*pd = calc;
					continue;
				}

				srcA = (*ps>>24), srcR = (*ps>>16)&0xff, srcG = (*ps>>8)&0xff, srcB = *ps&0xff;
				srcA = DIV255(gr_alpha*srcA);
				if (srcA==0x00)
					continue;

				dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
				if ((srcA==0xff)||(dstA==0)) {
					*pd = (srcA<<24)|(srcR<<16)|(srcG<<8)|srcB;//*ps;
					continue;
				}

				srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

				rDst = 0xff-srcA;
				dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

				dstA = DIV255((srcA*0xff)+dstA*rDst);
				dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				src = *ps, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
				*pd = calc;
			}
	}
	else {
		for (hi=height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			for (wi=width, ps=pSrc, pd=pDst; wi>0; wi--, ps++, pd++) {
				if ((*ps==src)&&(*pd==dst)) {	// same pixels, same result
					*pd = calc;
					continue;
				}

				srcA = (*ps>>24), srcR = (*ps>>16)&0xff, srcG = (*ps>>8)&0xff, srcB = *ps&0xff;
				if (srcA==0x00)
					continue;

				dstA = (*pd>>24), dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;
				if ((srcA==0xff)||(dstA==0)) {
					*pd = *ps;
					continue;
				}

				srcR = srcR*srcA, srcG = srcG*srcA, srcB = srcB*srcA;

				rDst = 0xff-srcA;
				dstR = DIV255((dstR*dstA)*rDst), dstG = DIV255((dstG*dstA)*rDst), dstB = DIV255((dstB*dstA)*rDst);

				dstA = DIV255((srcA*0xff)+dstA*rDst);
				dstR = (srcR+dstR)/dstA, dstG = (srcG+dstG)/dstA, dstB = (srcB+dstB)/dstA;

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				src = *ps, dst = *pd, calc = (dstA<<24)|(dstR<<16)|(dstG<<8)|dstB;
				*pd = calc;
			}
	}
}

void ScrGen32_DrawImage(PSD psd, MWCOORD dstX, MWCOORD dstY, MWCOORD width, MWCOORD height,
	PMWIMAGEHDR pImage, MWCOORD srcX, MWCOORD srcY, MWPIXELVAL *pConvTable)
{
	if(gr_gradation.direction > GRADATION_NONE && pImage->bpp == 32) {
		ScrGen32_Draw32BppImageGradation(psd, dstX, dstY, width, height, pImage, srcX, srcY);
		return;
	}

	switch(gr_mode) {
		case MWMODE_COPY:
		default:
			switch(pImage->bpp) {
				case 8:
					ScrGen32_Draw8BppImageDefault(psd, dstX, dstY, width, height, pImage, srcX, srcY, pConvTable);
					break;

				case 32:
					ScrGen32_Draw32BppImageDefault(psd, dstX, dstY, width, height, pImage, srcX, srcY);
					break;
			}
			break;

		case MWMODE_CLEAR:
			ScrGen32_FillRectDefault(psd, dstX, dstY, dstX + width - 1, dstY + height - 1, 0);
			break;

		case MWMODE_XOR_FGBG:
			switch(pImage->bpp) {
				case 8:
					ScrGen32_Draw8BppImageXor(psd, dstX, dstY, width, height, pImage, srcX, srcY, pConvTable);
					break;

				case 32:
					ScrGen32_Draw32BppImageXor(psd, dstX, dstY, width, height, pImage, srcX, srcY);
					break;
			}
			break;

		case MWMODE_SRC_OVER:
			/* We support source-over only for 32 Bpp ARGB format,
			   because we don't have any global alpha value yet.
			   and the 8 bpp image's palette contain only R, G, B data. */
			/* 2006.1.24 : We add the concept of global alpha. So 8bpp image and 24bpp image have alpha now! */
			switch(pImage->bpp) {
				case 8:
					if(gr_alpha == 0xFF || gr_alpha < 0)
						ScrGen32_Draw8BppImageDefault(psd, dstX, dstY, width, height, pImage, srcX, srcY, pConvTable);
					else if(gr_alpha > 0)
						ScrGen32_Draw8BppImageWithGlobalAlpha(psd, dstX, dstY, width, height, pImage, srcX, srcY, pConvTable);
					break;

				case 32:
					ScrGen32_Draw32BppImageAlpha(psd, dstX, dstY, width, height, pImage, srcX, srcY);
					break;
			}
			break;
	}
}

MWBOOL ScrGen32_CanDrawImage(PSD psd, PMWIMAGEHDR pimage)
{
	switch(pimage->bpp) {
		case 8:
			return TRUE;
		case 32:
			if(pimage->compression & MWIMAGE_RGB)
				return TRUE;
			break;
	}
	return FALSE;
}

static void ScrGen32_DrawCharFrom1BppGlyphDefault(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	int widthTemp;
	int heightTemp;

	unsigned long *pDestLine;
	unsigned long *pDest;

	unsigned char *glyphLineStart;
	unsigned char *curGlyphBytePtr;

	unsigned char	initGlyphMask;
	unsigned char	glyphMask;

	unsigned long srcA;
	unsigned long srcR;
	unsigned long srcG;
	unsigned long srcB;

	glyphLineStart = glyph + (glyphX >> 3)/* srcX / 8 */ + glyphLineLength * glyphY;
	initGlyphMask = 1 << (7 - (glyphX % 8));

	pDestLine = ((unsigned long*)(psd->addr)) + psd->linelen * dstY + dstX;
	heightTemp = glyphH;
	while(heightTemp--)
	{
		curGlyphBytePtr = glyphLineStart;
		glyphMask = initGlyphMask;

		pDest = pDestLine;
		widthTemp = glyphW;
		while(widthTemp--)
		{
			if(*curGlyphBytePtr & glyphMask)
			{
				if(gr_alpha == 0xff)
					*pDest = c;
				else {
					srcA = (gr_alpha >= 0) ? DIV255(gr_alpha * MW_SYS_GET_A(c)) : MW_SYS_GET_A(c);
					srcR = MW_SYS_GET_R(c);
					srcG = MW_SYS_GET_G(c);
					srcB = MW_SYS_GET_B(c);

					*pDest = MW_SYS_ARGB(srcA, srcR, srcG, srcB);
				}
			}

			/* Get the next glyph bit */
			if(glyphMask == 1)
			{
				curGlyphBytePtr++;
				glyphMask = 0x80; /* 1 << 7 */
			}
			else
			{
				glyphMask >>= 1;
			}

			pDest++;
		}
		glyphLineStart += glyphLineLength;
		pDestLine += psd->linelen;
	}
}

static void ScrGen32_DrawCharFrom1BppGlyphAlpha(PSD psd, int dstX, int dstY, unsigned char *glyph,
	int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	int widthTemp;
	int heightTemp;

	unsigned long *pDestLine;
	unsigned long *pDest;

	unsigned char *glyphLineStart;
	unsigned char *curGlyphBytePtr;

	unsigned char	initGlyphMask;
	unsigned char	glyphMask;

	unsigned long srcA;
	unsigned long srcR;
	unsigned long srcG;
	unsigned long srcB;
	unsigned long srcAMul;
	unsigned long rDst;

	unsigned long dstA;
	unsigned long dstR;
	unsigned long dstG;
	unsigned long dstB;

	unsigned long dst = 0;
	//unsigned long calc = c;
	unsigned long calc;

	srcA = (gr_alpha >= 0) ? DIV255(gr_alpha * MW_SYS_GET_A(c)) : MW_SYS_GET_A(c);
	srcR = MW_SYS_GET_R(c) * srcA;
	srcG = MW_SYS_GET_G(c) * srcA;
	srcB = MW_SYS_GET_B(c) * srcA;
	srcAMul = (srcA << 8);
	rDst = 0xFF - srcA;

	c = MW_SYS_ARGB(srcA, MW_SYS_GET_R(c), MW_SYS_GET_G(c), MW_SYS_GET_B(c));
	calc = c;

	glyphLineStart = glyph + (glyphX >> 3)/* glyphX / 8 */ + glyphLineLength * glyphY;
	initGlyphMask = 1 << (7 - (glyphX % 8));

	pDestLine = ((unsigned long*)(psd->addr)) + psd->linelen * dstY + dstX;
	heightTemp = glyphH;
	while(heightTemp--)
	{
		curGlyphBytePtr = glyphLineStart;
		glyphMask = initGlyphMask;

		pDest = pDestLine;
		widthTemp = glyphW;
		while(widthTemp--)
		{
			if(*curGlyphBytePtr & glyphMask)
			{
				if(*pDest == dst)
				{
					*pDest = calc;
				}
				else
				{
					dst = *pDest;
					dstA = MW_SYS_GET_A(dst);
					if(dstA == 0)
					{
						*pDest = calc = c;
					}
					else
					{
						dstR = ((MW_SYS_GET_R(dst) * dstA) >> 8) * rDst;
						dstG = ((MW_SYS_GET_G(dst) * dstA) >> 8) * rDst;
						dstB = ((MW_SYS_GET_B(dst) * dstA) >> 8) * rDst;

						dstA = (srcAMul + dstA * rDst) >> 8;
						dstR = (srcR + dstR) / dstA; /* dstA cannot be 0, because srcA is not 0 */
						dstG = (srcG + dstG) / dstA; /* dstA cannot be 0, because srcA is not 0 */
						dstB = (srcB + dstB) / dstA; /* dstA cannot be 0, because srcA is not 0 */

						dstA = dstA > 0xFF ? 0xFF : dstA;
						dstR = dstR > 0xFF ? 0xFF : dstR;
						dstG = dstG > 0xFF ? 0xFF : dstG;
						dstB = dstB > 0xFF ? 0xFF : dstB;

						*pDest = calc = MW_SYS_ARGB(dstA, dstR, dstG, dstB);
					}
				}
			}

			/* Get the next glyph bit */
			if(glyphMask == 1)
			{
				curGlyphBytePtr++;
				glyphMask = 0x80; /* 1 << 7 */
			}
			else
			{
				glyphMask >>= 1;
			}

			pDest++;
		}
		glyphLineStart += glyphLineLength;
		pDestLine += psd->linelen;
	}
}

static void ScrGen32_DrawCharFrom8BppGlyphDefault(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	int widthTemp;
	int heightTemp;

	unsigned long srcA;
	unsigned long srcR;
	unsigned long srcG;
	unsigned long srcB;

	unsigned long dst;

	unsigned long dstA;
	unsigned long dstR;
	unsigned long dstG;
	unsigned long dstB;

	unsigned long srcGlyph;
	unsigned long dstGlyph;

	unsigned char *glyphLineStart;
	unsigned char *curGlyphBytePtr;

	unsigned long *pDestLine;
	unsigned long *pDest;

	glyphLineStart = glyph + glyphX + glyphLineLength * glyphY;

	pDestLine = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	srcA = (gr_alpha < 0xff && gr_alpha >= 0) ? DIV255(gr_alpha * MW_SYS_GET_A(c)) : MW_SYS_GET_A(c);
	if(srcA > 0)
	{
		srcR = MW_SYS_GET_R(c);
		srcG = MW_SYS_GET_G(c);
		srcB = MW_SYS_GET_B(c);
	}
	else
	{
		srcR = srcG = srcB = 0;
	}

	c = MW_SYS_ARGB(srcA, srcR, srcG, srcB);

	heightTemp = glyphH;
	while(heightTemp--)
	{
		curGlyphBytePtr = glyphLineStart;
		pDest = pDestLine;
		widthTemp = glyphW;
		while(widthTemp--)
		{
			srcGlyph = *curGlyphBytePtr;
			if(srcGlyph == 126)
			{
				*pDest = c;
			}
			else if(srcGlyph > 0)
			{
				/* 0 ~ 126 ==> 0 ~ 256(not 255) */
				srcGlyph = (srcGlyph << 1) + (srcGlyph >> 5) + (srcGlyph >> 6);
				dstGlyph = 0x100 - srcGlyph;

				dst = *pDest;
				if(srcA == 0)
				{
					dstA = MW_SYS_GET_A(dst);
					dstR = MW_SYS_GET_R(dst);
					dstG = MW_SYS_GET_G(dst);
					dstB = MW_SYS_GET_B(dst);

					dstA = (dstA * dstGlyph) >> 8;
					dstR = (dstR * dstGlyph) >> 8;
					dstG = (dstG * dstGlyph) >> 8;
					dstB = (dstB * dstGlyph) >> 8;

					dstA = dstA > 0xFF ? 0xFF : dstA;
					dstR = dstR > 0xFF ? 0xFF : dstR;
					dstG = dstG > 0xFF ? 0xFF : dstG;
					dstB = dstB > 0xFF ? 0xFF : dstB;
				}
				else
				{
					dstA = MW_SYS_GET_A(dst);
					if(dstA == 0)
					{
						dstA = (srcA * srcGlyph) >> 8;
						//dstR = (srcR * srcGlyph) >> 8;
						//dstG = (srcG * srcGlyph) >> 8;
						//dstB = (srcB * srcGlyph) >> 8;
						dstR = srcR;
						dstG = srcG;
						dstB = srcB;

						dstA = dstA > 0xFF ? 0xFF : dstA;
						dstR = dstR > 0xFF ? 0xFF : dstR;
						dstG = dstG > 0xFF ? 0xFF : dstG;
						dstB = dstB > 0xFF ? 0xFF : dstB;
					}
					else
					{
						dstR = MW_SYS_GET_R(dst);
						dstG = MW_SYS_GET_G(dst);
						dstB = MW_SYS_GET_B(dst);

						dstA = (srcA * srcGlyph + dstA * dstGlyph) >> 8;
						dstR = (srcR * srcGlyph + dstR * dstGlyph) >> 8;
						dstG = (srcG * srcGlyph + dstG * dstGlyph) >> 8;
						dstB = (srcB * srcGlyph + dstB * dstGlyph) >> 8;

						dstA = dstA > 0xFF ? 0xFF : dstA;
						dstR = dstR > 0xFF ? 0xFF : dstR;
						dstG = dstG > 0xFF ? 0xFF : dstG;
						dstB = dstB > 0xFF ? 0xFF : dstB;
					}
				}

				if(dstA)
				{
					*pDest = MW_SYS_ARGB (dstA, dstR, dstG, dstB);
				}
				else
				{
					*pDest = 0;
				}

			}
			curGlyphBytePtr++;
			pDest++;

		}
		glyphLineStart += glyphLineLength;
		pDestLine += psd->linelen;
	}
}

static void ScrGen32_DrawCharFrom8BppGlyphGradation(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	int widthTemp;
	int heightTemp;

	unsigned long srcA;
	unsigned long srcR;
	unsigned long srcG;
	unsigned long srcB;

	unsigned long dst;

	unsigned long dstA;
	unsigned long dstR;
	unsigned long dstG;
	unsigned long dstB;

	unsigned long srcGlyph;
	unsigned long dstGlyph;

	unsigned char *glyphLineStart;
	unsigned char *curGlyphBytePtr;

	unsigned long *pDestLine;
	unsigned long *pDest;

	int ratio = 100;

	if(gr_gradation.length == 0) return;

	glyphLineStart = glyph + glyphX + glyphLineLength * glyphY;

	pDestLine = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	srcA = (gr_alpha < 0xff && gr_alpha >= 0) ? DIV255(gr_alpha * MW_SYS_GET_A(c)) : MW_SYS_GET_A(c);
	if(srcA > 0)
	{
		srcR = MW_SYS_GET_R(c);
		srcG = MW_SYS_GET_G(c);
		srcB = MW_SYS_GET_B(c);
	}
	else
	{
		srcR = srcG = srcB = 0;
	}

	c = MW_SYS_ARGB(srcA, srcR, srcG, srcB);

	heightTemp = glyphH;
	while(heightTemp--)
	{
		if(gr_gradation.direction == GRADATION_UP)
			ratio = (dstY + glyphH - heightTemp - gr_gradation.start_pos) * 100 / gr_gradation.length;
		else if(gr_gradation.direction == GRADATION_DOWN)
			ratio = (gr_gradation.length - (dstY + glyphH - heightTemp - gr_gradation.start_pos)) * 100 / gr_gradation.length;

		curGlyphBytePtr = glyphLineStart;
		pDest = pDestLine;
		widthTemp = glyphW;
		while(widthTemp--)
		{
			if(gr_gradation.direction == GRADATION_LEFT)
				ratio = (dstX + glyphW - widthTemp - gr_gradation.start_pos) * 100 / gr_gradation.length;
			else if(gr_gradation.direction == GRADATION_RIGHT)
				ratio = (gr_gradation.length - (dstX + glyphW - widthTemp - gr_gradation.start_pos)) * 100 / gr_gradation.length;

			if(ratio < 0)	ratio = 0;
			if(ratio > 100)	ratio = 100;

			srcGlyph = *curGlyphBytePtr;
			srcGlyph = (srcGlyph * ratio / 100) * 6/10;	//gradation 효과를 약하게 나타내기 위해 (x 0.6)
			if(srcGlyph == 126)
			{
				*pDest = c;
			}
			else if(srcGlyph > 0)
			{
				/* 0 ~ 126 ==> 0 ~ 256(not 255) */
				srcGlyph = (srcGlyph << 1) + (srcGlyph >> 5) + (srcGlyph >> 6);
				dstGlyph = 0x100 - srcGlyph;

				dst = *pDest;
				if(srcA == 0)
				{
					dstA = MW_SYS_GET_A(dst);
					dstR = MW_SYS_GET_R(dst);
					dstG = MW_SYS_GET_G(dst);
					dstB = MW_SYS_GET_B(dst);

					dstA = (dstA * dstGlyph) >> 8;
					dstR = (dstR * dstGlyph) >> 8;
					dstG = (dstG * dstGlyph) >> 8;
					dstB = (dstB * dstGlyph) >> 8;

					dstA = dstA > 0xFF ? 0xFF : dstA;
					dstR = dstR > 0xFF ? 0xFF : dstR;
					dstG = dstG > 0xFF ? 0xFF : dstG;
					dstB = dstB > 0xFF ? 0xFF : dstB;
				}
				else
				{
					dstA = MW_SYS_GET_A(dst);
					if(dstA == 0)
					{
						dstA = (srcA * srcGlyph) >> 8;
						//dstR = (srcR * srcGlyph) >> 8;
						//dstG = (srcG * srcGlyph) >> 8;
						//dstB = (srcB * srcGlyph) >> 8;
						dstR = srcR;
						dstG = srcG;
						dstB = srcB;

						dstA = dstA > 0xFF ? 0xFF : dstA;
						dstR = dstR > 0xFF ? 0xFF : dstR;
						dstG = dstG > 0xFF ? 0xFF : dstG;
						dstB = dstB > 0xFF ? 0xFF : dstB;
					}
					else
					{
						dstR = MW_SYS_GET_R(dst);
						dstG = MW_SYS_GET_G(dst);
						dstB = MW_SYS_GET_B(dst);

						dstA = (srcA * srcGlyph + dstA * dstGlyph) >> 8;
						dstR = (srcR * srcGlyph + dstR * dstGlyph) >> 8;
						dstG = (srcG * srcGlyph + dstG * dstGlyph) >> 8;
						dstB = (srcB * srcGlyph + dstB * dstGlyph) >> 8;

						dstA = dstA > 0xFF ? 0xFF : dstA;
						dstR = dstR > 0xFF ? 0xFF : dstR;
						dstG = dstG > 0xFF ? 0xFF : dstG;
						dstB = dstB > 0xFF ? 0xFF : dstB;
					}
				}

				if(dstA)
				{
					*pDest = MW_SYS_ARGB (dstA, dstR, dstG, dstB);
				}
				else
				{
					*pDest = 0;
				}

			}
			curGlyphBytePtr++;
			pDest++;

		}
		glyphLineStart += glyphLineLength;
		pDestLine += psd->linelen;
	}
}

static void ScrGen32_DrawCharFrom8BppGlyphXor(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	int widthTemp;
	int heightTemp;

	unsigned long dst;

	unsigned long srcA;
	unsigned long srcR;
	unsigned long srcG;
	unsigned long srcB;

	unsigned long dstA;
	unsigned long dstR;
	unsigned long dstG;
	unsigned long dstB;

	unsigned long srcGlyph;
	unsigned long dstGlyph;

	unsigned char *glyphLineStart;
	unsigned char *curGlyphBytePtr;

	unsigned long *pDestLine;
	unsigned long *pDest;
	unsigned long cTemp;

	glyphLineStart = glyph + glyphX + glyphLineLength * glyphY;
	pDestLine = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	heightTemp = glyphH;
	while(heightTemp--)
	{
		curGlyphBytePtr = glyphLineStart;
		pDest = pDestLine;
		widthTemp = glyphW;
		while(widthTemp--)
		{
			if(*curGlyphBytePtr > 0)
			{
				cTemp = (c ^ gr_xorcolor ^ *pDest) | 0xFF000000;
				srcA = (gr_alpha >= 0) ? DIV255(gr_alpha * MW_SYS_GET_A(cTemp)) : MW_SYS_GET_A(cTemp);
				srcR = DIV255(MW_SYS_GET_R(cTemp) * srcA);
				srcG = DIV255(MW_SYS_GET_G(cTemp) * srcA);
				srcB = DIV255(MW_SYS_GET_B(cTemp) * srcA);

				srcGlyph = *curGlyphBytePtr;
				/* 0 ~ 126 ==> 0 ~ 256(not 255) */
				srcGlyph = (srcGlyph << 1) + (srcGlyph >> 5) + (srcGlyph >> 6);

				dst = *pDest;
				dstA = MW_SYS_GET_A(dst);

				dstGlyph = 0x100 - ((srcGlyph * srcA) >> 8);

				dstA = (srcA * srcGlyph + dstA * dstGlyph) >> 8;

				if(dstA > 0)
				{
					dstR = (MW_SYS_GET_R(dst) * dstA) >> 8;
					dstG = (MW_SYS_GET_G(dst) * dstA) >> 8;
					dstB = (MW_SYS_GET_B(dst) * dstA) >> 8;

					dstR = (srcR * srcGlyph + dstR * dstGlyph) / dstA;
					dstG = (srcG * srcGlyph + dstG * dstGlyph) / dstA;
					dstB = (srcB * srcGlyph + dstB * dstGlyph) / dstA;

					dstA = dstA > 0xFF ? 0xFF : dstA;
					dstR = dstR > 0xFF ? 0xFF : dstR;
					dstG = dstG > 0xFF ? 0xFF : dstG;
					dstB = dstB > 0xFF ? 0xFF : dstB;

					*pDest = MW_SYS_ARGB (dstA, dstR, dstG, dstB);
					//*pDest = (gr_xorcolor ^ pTemp ^ *pDest) | 0xFF000000;
				}
				else
				{
					*pDest = 0;
				}
			}

			curGlyphBytePtr++;
			pDest++;

		}
		glyphLineStart += glyphLineLength;

		pDestLine += psd->linelen;
	}
}

static void ScrGen32_DrawCharFrom8BppGlyphAlpha(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	int widthTemp;
	int heightTemp;

	unsigned long dst;

	unsigned long srcA;
	unsigned long srcR;
	unsigned long srcG;
	unsigned long srcB;

	unsigned long dstA;
	unsigned long dstR;
	unsigned long dstG;
	unsigned long dstB;

	unsigned long srcGlyph;
	unsigned long dstGlyph;

	unsigned char *glyphLineStart;
	unsigned char *curGlyphBytePtr;

	unsigned long *pDestLine;
	unsigned long *pDest;

	glyphLineStart = glyph + glyphX + glyphLineLength * glyphY;

	pDestLine = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	srcA = (gr_alpha >= 0) ? DIV255(gr_alpha * MW_SYS_GET_A(c)) : MW_SYS_GET_A(c);
	srcR = DIV255(MW_SYS_GET_R(c) * srcA);
	srcG = DIV255(MW_SYS_GET_G(c) * srcA);
	srcB = DIV255(MW_SYS_GET_B(c) * srcA);

	heightTemp = glyphH;
	while(heightTemp--)
	{
		curGlyphBytePtr = glyphLineStart;
		pDest = pDestLine;
		widthTemp = glyphW;
		while(widthTemp--)
		{
			if(*curGlyphBytePtr > 0)
			{
				srcGlyph = *curGlyphBytePtr;
				/* 0 ~ 126 ==> 0 ~ 256(not 255) */
				srcGlyph = (srcGlyph << 1) + (srcGlyph >> 5) + (srcGlyph >> 6);
				dst = *pDest;
				dstA = MW_SYS_GET_A(dst);

				dstGlyph = 0x100 - ((srcGlyph * srcA) >> 8);

				dstA = (srcA * srcGlyph + dstA * dstGlyph) >> 8;

				if(dstA > 0)
				{
					dstR = (MW_SYS_GET_R(dst) * dstA) >> 8;
					dstG = (MW_SYS_GET_G(dst) * dstA) >> 8;
					dstB = (MW_SYS_GET_B(dst) * dstA) >> 8;

					dstR = (srcR * srcGlyph + dstR * dstGlyph) / dstA;
					dstG = (srcG * srcGlyph + dstG * dstGlyph) / dstA;
					dstB = (srcB * srcGlyph + dstB * dstGlyph) / dstA;

					dstA = dstA > 0xFF ? 0xFF : dstA;
					dstR = dstR > 0xFF ? 0xFF : dstR;
					dstG = dstG > 0xFF ? 0xFF : dstG;
					dstB = dstB > 0xFF ? 0xFF : dstB;

					*pDest = MW_SYS_ARGB (dstA, dstR, dstG, dstB);
				}
				else
				{
					*pDest = 0;
				}
			}

			curGlyphBytePtr++;
			pDest++;

		}
		glyphLineStart += glyphLineLength;
		pDestLine += psd->linelen;
	}
}

static void ScrGen32_DrawCharFrom6BppGlyphDefault(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	unsigned long srcA, srcR, srcG, srcB, dstA, dstR, dstG, dstB, src, dst, calc;
	unsigned long srcGlyph, dstGlyph;
	unsigned char *pGlyph, *pg;
	unsigned long *pDst, *pd;
	int wi, hi;

	pGlyph = glyph + glyphX + (glyphY-1) * glyphLineLength;

	pDst = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	srcA = (c>>24);
	if ((gr_alpha<0xff)&&(gr_alpha>=0))
		srcA = DIV255(gr_alpha*srcA);

	dst = src = calc = 0;
	if(srcA>0)
		srcR = (c>>16)&0xff, srcG = (c>>8)&0xff, srcB = c&0xff, c = (srcA<<24)|(c&0xffffff);
	else
		srcR = srcG = srcB = c =0;

	for (hi=glyphH; hi>0; hi--, pGlyph-=glyphLineLength, pDst+=psd->linelen) {
		for (wi=glyphW, pd=pDst, pg=pGlyph; wi>0; wi--, pg++, pd++) {
			srcGlyph = _UTFColorCovTab[*pg];

			if (srcGlyph == 0xff)
				*pd = c;
			else if (srcGlyph>0) {
				#if 0
				if ((src==srcGlyph)&&(dst==*pd)) {
					*pd = calc;
					continue;
				}
				#endif

				dstGlyph = 0xff - srcGlyph;

				dstA = (*pd>>24);
				dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;

				dstA = DIV255(srcA*srcGlyph+dstA*dstGlyph);
				dstR = DIV255(srcR*srcGlyph+dstR*dstGlyph), dstG = DIV255(srcG*srcGlyph+dstG*dstGlyph), dstB = DIV255(srcB*srcGlyph+dstB*dstGlyph);

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				if (dstA>0) {
					dst = *pd, src = srcGlyph;
					*pd = calc = ((dstA<<24)|(dstR<<16)|(dstG<<8)|dstB);
				}
				else
					dst = *pd, src = srcGlyph, *pd = calc = 0;
			}
		}
	}
}

static void ScrGen32_DrawCharFrom6BppGlyphGradation(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	unsigned long srcA, srcR, srcG, srcB, dstA, dstR, dstG, dstB, src, dst, calc;
	unsigned long srcGlyph, dstGlyph;
	unsigned char *pGlyph, *pg;
	unsigned long *pDst, *pd;
	int wi, hi;
	int ratio = 100;

	if(gr_gradation.length == 0) return;

	pGlyph = glyph + glyphX + (glyphY-1) * glyphLineLength;

	pDst = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	srcA = (c>>24);
	if ((gr_alpha<0xff)&&(gr_alpha>=0))
		srcA = DIV255(gr_alpha*srcA);

	dst = src = calc = 0;
	if(srcA>0)
		srcR = (c>>16)&0xff, srcG = (c>>8)&0xff, srcB = c&0xff, c = (srcA<<24)|(c&0xffffff);
	else
		srcR = srcG = srcB = c =0;

	for (hi=glyphH; hi>0; hi--, pGlyph-=glyphLineLength, pDst+=psd->linelen) {
		if(gr_gradation.direction == GRADATION_UP)
			ratio = (dstY + glyphH - hi - gr_gradation.start_pos) * 100 / gr_gradation.length;
		else if(gr_gradation.direction == GRADATION_DOWN)
			ratio = (gr_gradation.length - (dstY + glyphH - hi - gr_gradation.start_pos)) * 100 / gr_gradation.length;

		for (wi=glyphW, pd=pDst, pg=pGlyph; wi>0; wi--, pg++, pd++) {
			if(gr_gradation.direction == GRADATION_LEFT)
				ratio = (dstX + glyphW - wi - gr_gradation.start_pos) * 100 / gr_gradation.length;
			else if(gr_gradation.direction == GRADATION_RIGHT)
				ratio = (gr_gradation.length - (dstX + glyphW - wi - gr_gradation.start_pos)) * 100 / gr_gradation.length;

			if(ratio < 0)	ratio = 0;
			if(ratio > 100)	ratio = 100;

			srcGlyph = _UTFColorCovTab[*pg];
			srcGlyph = (srcGlyph * ratio / 100) * 6/10;	//gradation 효과를 약하게 나타내기 위해 (x 0.6)
			if (srcGlyph == 0xff)
				*pd = c;
			else if (srcGlyph>0) {
				#if 0
				if ((src==srcGlyph)&&(dst==*pd)) {
					*pd = calc;
					continue;
				}
				#endif

				dstGlyph = 0xff - srcGlyph;

				dstA = (*pd>>24);
				dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;

				dstA = DIV255(srcA*srcGlyph+dstA*dstGlyph);
				dstR = DIV255(srcR*srcGlyph+dstR*dstGlyph), dstG = DIV255(srcG*srcGlyph+dstG*dstGlyph), dstB = DIV255(srcB*srcGlyph+dstB*dstGlyph);

				dstA = (dstA>0xff ? 0xff : dstA);
				dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

				if (dstA>0) {
					dst = *pd, src = srcGlyph;
					*pd = calc = ((dstA<<24)|(dstR<<16)|(dstG<<8)|dstB);
				}
				else
					dst = *pd, src = srcGlyph, *pd = calc = 0;
			}
		}
	}
}
#if 0
static void ScrGen32_DrawCharFrom6BppGlyphXor(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	unsigned long srcA, srcR, srcG, srcB, dstA, dstR, dstG, dstB, src, dst, calc, colXor;
	unsigned long srcGlyph, dstGlyph;
	unsigned char *pGlyph, *pg;
	unsigned long *pDst, *pd;
	int wi, hi;

	pGlyph = glyph + glyphX + (glyphY-1) * glyphLineLength;

	pDst = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	dst = src = calc = 0;
	srcA = 0xff;

	for (hi=glyphH; hi>0; hi--, pGlyph-=glyphLineLength, pDst+=psd->linelen) {
		for (wi=glyphW, pd=pDst, pg=pGlyph; wi>0; wi--, pg++, pd++) {
			srcGlyph = _UTFColorCovTab[*pg];
			if (srcGlyph>0) {
				if ((src==srcGlyph)&&(dst==*pd)) {
					*pd = calc;
					continue;
				}

				colXor = (c^gr_xorcolor^*pd)|0xff000000;
				srcR = (colXor>>16)&0xff, srcG = (colXor>>8)&0xff, srcB = colXor&0xff;

				dstGlyph = 0xff - srcGlyph;
				dstA = (*pd>>24);
				dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;

				dstA = DIV255(srcA*srcGlyph+dstA*dstGlyph);
				if (dstA>0) {
					dstR = DIV255(srcR*srcGlyph+dstR*dstGlyph), dstG = DIV255(srcG*srcGlyph+dstG*dstGlyph), dstB = DIV255(srcB*srcGlyph+dstB*dstGlyph);

					dstA = (dstA>0xff ? 0xff : dstA);
					dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

					dst = *pd, src = srcGlyph;
					*pd = calc = ((dstA<<24)|(dstR<<16)|(dstG<<8)|dstB);
				}
				else
					dst = *pd, src = srcGlyph, *pd = calc = 0;
			}
		}
	}
}
#else
static void ScrGen32_DrawCharFrom6BppGlyphXor(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	unsigned long srcA, srcR, srcG, srcB, dstA, dstR, dstG, dstB, src, dst, calc, colXor;
	unsigned long srcGlyph, dstGlyph;
	unsigned char *pGlyph, *pg;
	unsigned long *pDst, *pd;
	int wi, hi;

	pGlyph = glyph + glyphX + (glyphY-1) * glyphLineLength;

	pDst = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	srcA = (c>>24);
	if ((gr_alpha<0xff)&&(gr_alpha>=0))
		srcA = DIV255(gr_alpha*srcA);

	dst = src = calc = 0;
	if (srcA>0)
		srcR = DIV255(((c>>16)&0xff)*srcA), srcG = DIV255(((c>>8)&0xff)*srcA), srcB = DIV255((c&0xff)*srcA);
	else
		srcR = srcG = srcB = c =0;

	for (hi=glyphH; hi>0; hi--, pGlyph-=glyphLineLength, pDst+=psd->linelen) {
		for (wi=glyphW, pd=pDst, pg=pGlyph; wi>0; wi--, pg++, pd++) {
			srcGlyph = _UTFColorCovTab[*pg];
			if (srcGlyph>0) {
				#if 0
				if ((src==srcGlyph)&&(dst==*pd)) {
					*pd = calc;
					continue;
				}
				#endif

				colXor = (c^gr_xorcolor^*pd)|0xff000000;
				srcR = (colXor>>16)&0xff, srcG = (colXor>>8)&0xff, srcB = colXor&0xff;

				dstGlyph = 0xff - ((srcGlyph*srcA)>>8);
				dstA = (*pd>>24);
				dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;

				dstA = (srcA*srcGlyph+dstA*dstGlyph)>>8;
				if (dstA>0) {
					dstR = DIV255(dstR*dstA), dstG = DIV255(dstG*dstA), dstB = DIV255(dstB*dstA);

					dstR = (srcR*srcGlyph+dstR*dstGlyph)/dstA, dstG = (srcG*srcGlyph+dstG*dstGlyph)/dstA, dstB = (srcB*srcGlyph+dstB*dstGlyph)/dstA;

					dstA = (dstA>0xff ? 0xff : dstA);
					dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

					dst = *pd, src = srcGlyph;
					*pd = calc = ((dstA<<24)|(dstR<<16)|(dstG<<8)|dstB);
				}
				else
					dst = *pd, src = srcGlyph, *pd = calc = 0;
			}
		}
	}
}
#endif

static void ScrGen32_DrawCharFrom6BppGlyphAlpha(PSD psd, int dstX, int dstY,
	unsigned char *glyph, int glyphLineLength, int glyphX, int glyphY, int glyphW, int glyphH, MWPIXELVAL c)
{
	unsigned long srcA, srcR, srcG, srcB, dstA, dstR, dstG, dstB, src, dst, calc;
	unsigned long srcGlyph, dstGlyph;
	unsigned char *pGlyph, *pg;
	unsigned long *pDst, *pd;
	int wi, hi;

	pGlyph = glyph + glyphX + (glyphY-1) * glyphLineLength;

	pDst = ((unsigned long *)(psd->addr)) + psd->linelen * dstY + dstX;

	srcA = (c>>24);
	if ((gr_alpha<0xff)&&(gr_alpha>=0))
		srcA = DIV255(gr_alpha*srcA);

	dst = src = calc = 0;
	if (srcA>0)
		srcR = DIV255(((c>>16)&0xff)*srcA), srcG = DIV255(((c>>8)&0xff)*srcA), srcB = DIV255((c&0xff)*srcA);
	else
		srcR = srcG = srcB = c =0;

	for (hi=glyphH; hi>0; hi--, pGlyph-=glyphLineLength, pDst+=psd->linelen) {
		for (wi=glyphW, pd=pDst, pg=pGlyph; wi>0; wi--, pg++, pd++) {
			srcGlyph = _UTFColorCovTab[*pg];
			if (srcGlyph == 0xff)
				*pd = c;
			else if (srcGlyph>0) {
				#if 0
				if ((src==srcGlyph)&&(dst==*pd)) {
					*pd = calc;
					continue;
				}
				#endif

				dstGlyph = 0xff - ((srcGlyph*srcA)>>8);
				dstA = (*pd>>24);
				dstR = (*pd>>16)&0xff, dstG = (*pd>>8)&0xff, dstB = *pd&0xff;

				dstA = (srcA*srcGlyph+dstA*dstGlyph)>>8;
				if (dstA>0) {
					dstR = DIV255(dstR*dstA), dstG = DIV255(dstG*dstA), dstB = DIV255(dstB*dstA);

					dstR = (srcR*srcGlyph+dstR*dstGlyph)/dstA, dstG = (srcG*srcGlyph+dstG*dstGlyph)/dstA, dstB = (srcB*srcGlyph+dstB*dstGlyph)/dstA;

					dstA = (dstA>0xff ? 0xff : dstA);
					dstR = (dstR>0xff ? 0xff : dstR), dstG = (dstG>0xff ? 0xff : dstG), dstB = (dstB>0xff ? 0xff : dstB);

					dst = *pd, src = srcGlyph;
					*pd = calc = ((dstA<<24)|(dstR<<16)|(dstG<<8)|dstB);
				}
				else
					dst = *pd, src = srcGlyph, *pd = calc = 0;
			}
		}
	}
}

void ScrGen32_DrawChar(PSD psd, MWCOORD dstX, MWCOORD dstY, int glyphBitCount,
	unsigned char *glyph, int glyphLineLength, MWCOORD glyphX, MWCOORD glyphY, MWCOORD glyphW, MWCOORD glyphH, MWPIXELVAL c)
{
	if(glyphBitCount == 6) {
		if(gr_gradation.direction > GRADATION_NONE) {
			ScrGen32_DrawCharFrom6BppGlyphGradation(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
			return;
		}

		switch(gr_mode) {
			case MWMODE_COPY:
			default:
				ScrGen32_DrawCharFrom6BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				break;

			case MWMODE_CLEAR:
				ScrGen32_DrawCharFrom6BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, 0);
				break;

			/* foryou add xor mode */
			case MWMODE_XOR_FGBG:
				ScrGen32_DrawCharFrom6BppGlyphXor(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				break;

			case MWMODE_SRC_OVER:
				if(gr_alpha < 0) {
					if(MW_SYS_GET_A(c) == 0xFF)
						ScrGen32_DrawCharFrom6BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
					else if(MW_SYS_GET_A(c) > 0)
						ScrGen32_DrawCharFrom6BppGlyphAlpha(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				}
				else if(gr_alpha > 0 && MW_SYS_GET_A(c) > 0) {
					if(gr_alpha == 0xFF && MW_SYS_GET_A(c) == 0xFF)
						ScrGen32_DrawCharFrom6BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
					else if(gr_alpha > 0 && MW_SYS_GET_A(c) > 0)
						ScrGen32_DrawCharFrom6BppGlyphAlpha(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				}
				break;
		}
	}
	else if(glyphBitCount == 8) {
		if(gr_gradation.direction > GRADATION_NONE) {
			ScrGen32_DrawCharFrom8BppGlyphGradation(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
			return;
		}

		switch(gr_mode) {
			case MWMODE_COPY:
			default:
				ScrGen32_DrawCharFrom8BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				break;

			case MWMODE_CLEAR:
				ScrGen32_DrawCharFrom8BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, 0);
				break;

			case MWMODE_XOR_FGBG:
				ScrGen32_DrawCharFrom8BppGlyphXor(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				break;

			case MWMODE_SRC_OVER:
				if(gr_alpha < 0) {
					if(MW_SYS_GET_A(c) == 0xFF)
						ScrGen32_DrawCharFrom8BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
					else if(MW_SYS_GET_A(c) > 0)
						ScrGen32_DrawCharFrom8BppGlyphAlpha(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				}
				else if(gr_alpha > 0 && MW_SYS_GET_A(c) > 0) {
					if(gr_alpha == 0xFF && MW_SYS_GET_A(c) == 0xFF)
						ScrGen32_DrawCharFrom8BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
					else if(gr_alpha > 0 && MW_SYS_GET_A(c) > 0)
						ScrGen32_DrawCharFrom8BppGlyphAlpha(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				}
				break;
		}
	}
	else if(glyphBitCount == 1) {
		switch(gr_mode) {
			case MWMODE_COPY:
			default:
				ScrGen32_DrawCharFrom1BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				break;

			case MWMODE_CLEAR:
				ScrGen32_DrawCharFrom1BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, 0);
				break;

			case MWMODE_SRC_OVER:
				if(gr_alpha < 0) {
					if(MW_SYS_GET_A(c) == 0xFF)
						ScrGen32_DrawCharFrom1BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
					else if(MW_SYS_GET_A(c) > 0)
						ScrGen32_DrawCharFrom1BppGlyphAlpha(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				}
				else {
					if(gr_alpha == 0xFF && MW_SYS_GET_A(c) == 0xFF)
						ScrGen32_DrawCharFrom1BppGlyphDefault(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
					else if(gr_alpha > 0 && MW_SYS_GET_A(c) > 0)
						ScrGen32_DrawCharFrom1BppGlyphAlpha(psd, dstX, dstY, glyph, glyphLineLength, glyphX, glyphY, glyphW, glyphH, c);
				}
				break;
		}
	}
}

/*
 * This stretchblit code was originally written for the TriMedia
 * VLIW CPU.  Therefore it uses RESTRICT pointers, and the special
 * one-assembler-opcode pseudo-functions SIGN and ABS.
 *
 * (The 'restrict' extension is in C99, so for a C99 compiler you
 * could "#define RESTRICT restrict" or put
 * "CFLAGS += -DRESTRICT=restrict" in the makefile).
 *
 * Compatibility definitions:
 */
#ifndef RESTRICT
#define RESTRICT
#endif
#ifndef SIGN
#define SIGN(x) (((x) > 0) ? 1 : (((x) == 0) ? 0 : -1))
#endif
#ifndef ABS
#define ABS(x) (((x) >= 0) ? (x) : -(x))
#endif

/* Blit a 32-bit image.
 * Can stretch the image by any X and/or Y scale factor.
 * Can flip the image in the X and/or Y axis.
 *
 * This is the faster version with no per-pixel multiply and a single
 * decision tree for the inner loop, by Jon.  Based on Alex's original
 * all-integer version.
 *
 * Paramaters:
 * srf              - Dest surface
 * dest_x_start
 * dest_y_start    - Top left corner of dest rectangle
 * width, height   - Size in dest co-ordinates.
 * x_denominator   - Denominator for source X value fractions.  Note that
 *                   this must be even, and all the numerators must also be
 *                   even, so we can easily divide by 2.
 * y_denominator   - Denominator for source Y value fractions.  Note that
 *                   this must be even, and all the numerators must also be
 *                   even, so we can easily divide by 2.
 * src_x_fraction  -
 * src_y_fraction  - Point in source that corresponds to the top left corner
 *                   of the pixel (dest_x_start, dest_y_start).  This is a
 *                   fraction - to get a float, divide by y_denominator.
 * x_step_fraction - X step in src for an "x++" step in dest.  May be negative
 *                   (for a flip).  Expressed as a fraction - divide it by
 *                   x_denominator for a float.
 * y_step_fraction - Y step in src for a  "y++" step in dest.  May be negative
 *                   (for a flip).  Expressed as a fraction - divide it by
 *                   y_denominator for a float.
 * image           - Source image.
 * op              - Raster operation
 */
void ScrGen32_StretchBlitEx(PSD dstPsd, PSD srcPsd, MWCOORD dest_x_start, MWCOORD dest_y_start, MWCOORD width, MWCOORD height,
	int x_denominator, int y_denominator, int src_x_fraction, int src_y_fraction, int x_step_fraction, int y_step_fraction, long op)
{
	/* Pointer to the current pixel in the source image */
	unsigned long *RESTRICT src_ptr;

	/* Pointer to x=xs1 on the next line in the source image */
	unsigned long *RESTRICT next_src_ptr;

	/* Pointer to the current pixel in the dest image */
	unsigned long *RESTRICT dest_ptr;

	/* Pointer to x=xd1 on the next line in the dest image */
	unsigned long *next_dest_ptr;

	/* Keep track of error in the source co-ordinates */
	int x_error;
	int y_error;

	/* 1-unit steps "forward" through the source image, as steps in the image
	 * byte array.
	 */
	int src_x_step_one;
	int src_y_step_one;

	/* normal steps "forward" through the source image, as steps in the image
	 * byte array.
	 */
	int src_x_step_normal;
	int src_y_step_normal;

	/* 1-unit steps "forward" through the source image, as steps in the image
	 * byte array.
	 */
	int x_error_step_normal;
	int y_error_step_normal;

	/* Countdown to the end of the destination image */
	int x_count;
	int y_count;

	/* Start position in source, in whole pixels */
	int src_x_start;
	int src_y_start;

	/* Error values for start X position in source */
	int x_error_start;

	/* 1-unit step down dest, in bytes. */
	int dest_y_step;

	/* source-over related */
	unsigned long srcA;
	unsigned long srcR;
	unsigned long srcG;
	unsigned long srcB;
	unsigned long rDst;

	unsigned long dstA;
	unsigned long dstR;
	unsigned long dstG;
	unsigned long dstB;

	unsigned long src = 0;
	unsigned long dst = 0;
	unsigned long calc = 0;


	/* We add half a dest pixel here so we're sampling from the middle of
	 * the dest pixel, not the top left corner.
	 */
	src_x_fraction += (x_step_fraction >> 1);
	src_y_fraction += (y_step_fraction >> 1);

	/* Seperate the whole part from the fractions.
	 *
	 * Also, We need to do lots of comparisons to see if error values are
	 * >= x_denominator.  So subtract an extra x_denominator for speed - then
	 * we can just check if it's >= 0.
	 */
	src_x_start = src_x_fraction / x_denominator;
	src_y_start = src_y_fraction / y_denominator;
	x_error_start = src_x_fraction - (src_x_start + 1) * x_denominator;
	y_error = src_y_fraction - (src_y_start + 1) * y_denominator;

	/* precalculate various deltas */

	src_x_step_normal = x_step_fraction / x_denominator;
	src_x_step_one = SIGN(x_step_fraction);
	x_error_step_normal =
		ABS(x_step_fraction) - ABS(src_x_step_normal) * x_denominator;

	src_y_step_normal = y_step_fraction / y_denominator;
	src_y_step_one = SIGN(y_step_fraction) * srcPsd->linelen;
	y_error_step_normal =
		ABS(y_step_fraction) - ABS(src_y_step_normal) * y_denominator;
	src_y_step_normal *= srcPsd->linelen;

	/* Pointer to the first source pixel */
	next_src_ptr =
		((unsigned long *) srcPsd->addr) +
		src_y_start * srcPsd->linelen + src_x_start;

	/* Cache the width of a scanline in dest */
	dest_y_step = dstPsd->linelen;

	/* Pointer to the first dest pixel */
	next_dest_ptr =
		((unsigned long *) dstPsd->addr) +
		(dest_y_start * dest_y_step) + dest_x_start;

	/*
	 * Note: The MWROP_SRC case below is a simple expansion of the
	 * default case.  It can be removed without significant speed
	 * penalty if you need to reduce code size.
	 *
	 * The MWROP_CLEAR case could be removed.  But it is a large
	 * speed increase for a small quantity of code.
	 */
	switch (op & MWROP_EXTENSION) {
	case MWROP_COPY:
	default:
		/* Benchmarking shows that this while loop is faster than the equivalent
		 * for loop: for(y_count=0; y_count<height; y_count++) { ... }
		 */
		y_count = height;
		while (y_count-- > 0) {
			src_ptr = next_src_ptr;
			dest_ptr = next_dest_ptr;

			x_error = x_error_start;

			x_count = width;
			while (x_count-- > 0) {
				/*dwp: global alpha*/
				if (gr_alpha == 0xff)
					*dest_ptr++ = *src_ptr;
				else {
					src = *src_ptr;
					srcA = (gr_alpha >= 0) ? DIV255(gr_alpha * MW_SYS_GET_A(src)) : MW_SYS_GET_A(src);
					srcR = MW_SYS_GET_R(src);
					srcG = MW_SYS_GET_G(src);
					srcB = MW_SYS_GET_B(src);

					*dest_ptr++ = MW_SYS_ARGB(srcA, srcR, srcG, srcB);
				}
				src_ptr += src_x_step_normal;
				x_error += x_error_step_normal;

				if (x_error >= 0) {
					src_ptr += src_x_step_one;
					x_error -= x_denominator;
				}
			}

			next_dest_ptr += dest_y_step;

			next_src_ptr += src_y_step_normal;
			y_error += y_error_step_normal;

			if (y_error >= 0) {
				next_src_ptr += src_y_step_one;
				y_error -= y_denominator;
			}
		}
		break;
	case MWROP_XOR_FGBG:
		y_count = height;
		while (y_count-- > 0) {
			src_ptr = next_src_ptr;
			dest_ptr = next_dest_ptr;

			x_error = x_error_start;

			x_count = width;
			while (x_count-- > 0) {
				if(MW_SYS_GET_A(*src_ptr))
					*dest_ptr = (gr_xorcolor ^ *src_ptr ^ *dest_ptr) | 0xFF000000;
				dest_ptr++;

				src_ptr += src_x_step_normal;
				x_error += x_error_step_normal;

				if (x_error >= 0) {
					src_ptr += src_x_step_one;
					x_error -= x_denominator;
				}
			}

			next_dest_ptr += dest_y_step;

			next_src_ptr += src_y_step_normal;
			y_error += y_error_step_normal;

			if (y_error >= 0) {
				next_src_ptr += src_y_step_one;
				y_error -= y_denominator;
			}
		}
		break;
	case MWROP_CLEAR:
		y_count = height;
		while (y_count-- > 0) {
			dest_ptr = next_dest_ptr;
			x_count = width;
			while (x_count-- > 0) {
				*dest_ptr++ = 0;
			}
			next_dest_ptr += dest_y_step;
		}
		break;

	case MWROP_SRC_OVER:
		y_count = height;
		while (y_count-- > 0) {
			src_ptr = next_src_ptr;
			dest_ptr = next_dest_ptr;

			x_error = x_error_start;

			x_count = width;
			while (x_count-- > 0) {
				if(*src_ptr == src && *dest_ptr == dst)
				{
					*dest_ptr = calc;
				}
				else
				{
					src = *src_ptr;
					dst = *dest_ptr;

					srcA = (gr_alpha >= 0) ? DIV255(gr_alpha * MW_SYS_GET_A(src)) : MW_SYS_GET_A(src);
					dstA = MW_SYS_GET_A(dst);
					if(srcA == 0xFF || dstA == 0x0)
					{
						//*dest_ptr = calc = src;
						*dest_ptr = calc = MW_SYS_ARGB(srcA, MW_SYS_GET_R(src), MW_SYS_GET_G(src), MW_SYS_GET_B(src));
					}
					else if(srcA == 0x0)
					{
						calc = dst;
					}
					else
					{
						rDst = 0xFF - srcA;

						srcR = MW_SYS_GET_R(src) * srcA;
						srcG = MW_SYS_GET_G(src) * srcA;
						srcB = MW_SYS_GET_B(src) * srcA;
						dstR = ((MW_SYS_GET_R(dst) * dstA) >> 8) * rDst;
						dstG = ((MW_SYS_GET_G(dst) * dstA) >> 8) * rDst;
						dstB = ((MW_SYS_GET_B(dst) * dstA) >> 8) * rDst;

						dstA = ((srcA<<8) + dstA * rDst) >> 8;
						dstR = (srcR + dstR) / dstA; /* dstA cannot be 0, because srcA is not 0 */
						dstG = (srcG + dstG) / dstA; /* dstA cannot be 0, because srcA is not 0 */
						dstB = (srcB + dstB) / dstA; /* dstA cannot be 0, because srcA is not 0 */

						dstA = dstA > 0xFF ? 0xFF : dstA;
						dstR = dstR > 0xFF ? 0xFF : dstR;
						dstG = dstG > 0xFF ? 0xFF : dstG;
						dstB = dstB > 0xFF ? 0xFF : dstB;

						*dest_ptr = calc = MW_SYS_ARGB(dstA, dstR, dstG, dstB);
					}
				}

				dest_ptr++;

				src_ptr += src_x_step_normal;
				x_error += x_error_step_normal;

				if (x_error >= 0) {
					src_ptr += src_x_step_one;
					x_error -= x_denominator;
				}
			}

			next_dest_ptr += dest_y_step;

			next_src_ptr += src_y_step_normal;
			y_error += y_error_step_normal;

			if (y_error >= 0) {
				next_src_ptr += src_y_step_one;
				y_error -= y_denominator;
			}
		}
		break;
	}
}


