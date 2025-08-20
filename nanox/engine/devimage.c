/*
 * Copyright (c) 2000, 2001, 2003 Greg Haerr <greg@censoft.com>
 * Portions Copyright (c) 2000 Martin Jolicoeur <martinj@visuaide.com>
 * Portions Copyright (c) 2000 Alex Holden <alex@linuxhacker.org>
 * Portions Copyright (c) Independant JPEG group (ijg)
 *
 * Image load/cache/resize/display routines
 *
 * If FASTJPEG is defined, JPEG images are decoded to
 * a 256 color standardized palette (mwstdpal8). Otherwise,
 * the images are decoded depending on their output
 * components (usually 24bpp).
 *
 * GIF, BMP, JPEG, PPM, PGM, PBM, PNG, XPM and TIFF formats are supported.
 * JHC:  Instead of working with a file, we work with a buffer
 *       (either provided by the user or through mmap).  This
 *	 improves speed, and provides a mechanism by which the
 *	 client can send image data directly to the engine
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "device.h"
#include "swap.h"
#if HAVE_MMAP
#include <sys/mman.h>
#endif

#if MW_FEATURE_IMAGES /* whole file */

extern	int 	dbgprint(const char*, ... );
extern	void 	OSA_PrintStack(void);

/* cached image list*/
typedef struct {
	MWLIST		link;		/* link list*/
	int		id;		/* image id*/
	PMWIMAGEHDR	pimage;		/* image data*/
	PSD		psd;		/* FIXME shouldn't need this*/
} IMAGEITEM, *PIMAGEITEM;

static MWLISTHEAD imagehead;		/* global image list*/
static int nextimageid = 1;

typedef struct {  /* structure for reading images from buffer   */
	unsigned char *start;	/* The pointer to the beginning of the buffer */
	unsigned long offset;	/* The current offset within the buffer       */
	unsigned long size;	/* The total size of the buffer               */
} buffer_t;


static void ComputePitch(int bpp, int width, int *pitch, int *bytesperpixel);
#if defined(HAVE_BMP_SUPPORT)
static int  LoadBMP(buffer_t *src, PMWIMAGEHDR pimage);
#endif
#if defined(HAVE_JPEG_SUPPORT)
static int  LoadJPEG(buffer_t *src, PMWIMAGEHDR pimage, PSD psd,
		MWBOOL fast_grayscale);
#endif
#if defined(HAVE_PNG_SUPPORT)
static int  LoadPNG(buffer_t *src, PMWIMAGEHDR pimage);
static int  LoadPNGOnlyHW(buffer_t * src, PMWIMAGEHDR pimage);
#endif
#if defined(HAVE_GIF_SUPPORT)
static int IsGIF(buffer_t *src);
static int LoadGIFEx(buffer_t *src, PMWIMAGEHDR *ppimages, int maxImageCount);
#endif
#if defined(HAVE_PNM_SUPPORT)
static int LoadPNM(buffer_t *src, PMWIMAGEHDR pimage);
#endif
#if defined(HAVE_XPM_SUPPORT)
static int LoadXPM(buffer_t *src, PMWIMAGEHDR pimage, PSD psd);
#endif
#if defined(HAVE_TIFF_SUPPORT)
static int LoadTIFF(char *path, PMWIMAGEHDR pimage);
#endif
#if defined(HAVE_LGI_SUPPORT)
static int LoadLGI(buffer_t * src, PMWIMAGEHDR pimage);
#endif

#if defined(HAVE_I16_SUPPORT)
static int LoadI16(buffer_t * src, PMWIMAGEHDR pImage);
#endif

#if (MWPIXEL_FORMAT == MWPF_TRUECOLOR1555)
static MWBOOL Convert32bppTo16bppARGB1555(PMWIMAGEHDR pImage);
static MWBOOL Convert24bppTo16bppARGB1555(PMWIMAGEHDR pImage);
static void ReverseImage16(PMWIMAGEHDR pImage);
#endif
#if (MWPIXEL_FORMAT == MWPF_TRUECOLOR8888)
static MWBOOL Convert16bppTo32bppARGB8888(PMWIMAGEHDR pImage);
static MWBOOL Convert24bppTo32bppARGB8888(PMWIMAGEHDR pImage);
static void Convert32bppEndian(PMWIMAGEHDR pImage);
static void ReverseImage32(PMWIMAGEHDR pImage);
#endif

/*
 * Buffered input functions to replace stdio functions
 */
static void
binit(buffer_t *buffer, void *startdata, int size)
{
	buffer->start = startdata;
	buffer->size = size;
	buffer->offset = 0;
}

static long
bseek(buffer_t *buffer, long offset, int whence)
{
	long new;

	switch(whence) {
	case SEEK_SET:
		if (offset >= buffer->size || offset < 0)
			return -1L;
		buffer->offset = offset;
		break;

	case SEEK_CUR:
		new = buffer->offset + offset;
		if (new >= buffer->size || new < 0)
			return -1L;
		buffer->offset = new;
		break;

	case SEEK_END:
		new = buffer->size - 1 + offset;
		if (new >= buffer->size || new < 0)
			return -1L;
		buffer->offset = new;
		break;

	default:
		return -1L;
	}
	return buffer->offset;
}

static int
bread(buffer_t *buffer, void *dest, unsigned long size)
{
	unsigned long copysize;

	if (buffer->offset == buffer->size)
		return 0;	/* EOF*/

	if (buffer->offset + size > buffer->size)
		copysize = buffer->size - buffer->offset;
	else copysize = size;

	memcpy(dest, buffer->start + buffer->offset, copysize);

	buffer->offset += copysize;
	return copysize;
}

#if defined(HAVE_BMP_SUPPORT)||defined(HAVE_PNM_SUPPORT)||defined(HAVE_XPM_SUPPORT)
static int
bgetc(buffer_t *buffer)
{
	if (buffer->offset == buffer->size)
		return EOF;
	return buffer->start[buffer->offset++];
}

static char *
bgets(buffer_t *buffer, char *dest, unsigned int size)
{
	int i,o;
	unsigned int copysize = size - 1;

	if (buffer->offset == buffer->size)
		return 0;

	if (buffer->offset + copysize > buffer->size)
		copysize = buffer->size - buffer->offset;

	for(o=0, i=buffer->offset; i < buffer->offset + copysize; i++, o++) {
		if ((dest[o] = buffer->start[i]) == '\n')
			break;
	}

	buffer->offset = i + 1;
	dest[o + 1] = 0;

	return dest;
}

static int
beof(buffer_t *buffer)
{
	return (buffer->offset == buffer->size);
}
#endif

/*
 * Image decoding and display
 * NOTE: This routine and APIs will change in subsequent releases.
 *
 * Decodes and loads a graphics file, then resizes to width/height,
 * then displays image at x, y
 * If width/height == -1, don't resize, use image size.
 * Clipping is not currently supported, just stretch/shrink to fit.
 *
 */

static int GdDecodeImage(PSD psd, buffer_t *src, char *path, int flags);

/**
 * Load an image from a memory buffer.
 *
 * @param psd Screen device.
 * @param buffer The buffer containing the image data.
 * @param size The size of the buffer.
 * @param flags If nonzero, JPEG images will be loaded as grayscale.  Yuck!
 */
int
GdLoadImageFromBuffer(PSD psd, void *buffer, int size, int flags)
{
	buffer_t src;

	binit(&src, buffer, size);
	return GdDecodeImage(psd, &src, NULL, flags);
}

static void
GdDecodeImageEx(PSD psd, buffer_t * src, char *path, MW_LOAD_IMAGE_EX_RESULT *pResult);

/**
 * This function is added to support animated GIF.
 * Unlike GdLoadImageFromBuffer(), this function may retrieve multiple images if the input buffer contains
 * an animated GIF.
 * The caller of this function may handle the multiple images to show an animation.
 * The delay time between the images is also necessary. For this, MWIMAGEHDR contains following new members.
 * - delaytime
 * - inputflag
 * - disposal
 * Refer to the GIF documentation for the description of the above values.
 */
void
GdLoadImageEx(PSD psd, void *buffer, int size, MW_LOAD_IMAGE_EX_RESULT *pResult)
{
	buffer_t src;

	binit(&src, buffer, size);
	GdDecodeImageEx(psd, &src, NULL, pResult);
}


void
GdDrawImageFromRawDataBuffer(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
	MWCOORD height, void *buffer, MWCOORD buffer_width, MWCOORD buffer_height)
{
	int id;
//	buffer_t src;
	PMWIMAGEHDR pimage;
	PIMAGEITEM	pItem;
#if 0
	unsigned long *pSrc, *pDst;
	int hi, srcLineLength, dstLineLength;
#endif

//	binit(&src, buffer, size);
//	id = GdDecodeImage(psd, &src, NULL, flags);
	{
		/* allocate image struct*/
		pimage = (PMWIMAGEHDR)NANOX_MALLOC(sizeof(MWIMAGEHDR));
		if (!pimage) {
			NANOX_FREE(pimage);
			id = 0;
			goto done;
		}

		memset(pimage, 0, sizeof(MWIMAGEHDR));
		pimage->width = buffer_width;
		pimage->height = buffer_height;
		pimage->planes = 1;
		pimage->bpp = 32;
		ComputePitch(pimage->bpp, pimage->width, &pimage->pitch, &pimage->bytesperpixel);
		pimage->compression = MWIMAGE_RGB;	/* RGB not BGR order*/
		pimage->transcolor = -1L;
		pimage->palette = NULL;
		pimage->palsize = 0;
		//pimage->imagebits = buffer;
		pimage->imagebits = NANOX_ScrAllocateFrame(pimage->bpp, pimage->width, pimage->height, pimage->pitch * pimage->height, &pimage->pitch);
		if(!pimage->imagebits) {
			NANOX_FREE(pimage);
			id = 0;
			goto done;
		}

#if 1
		memcpy(pimage->imagebits, buffer, pimage->pitch * pimage->height);
#else
		srcLineLength = buffer_width;
		dstLineLength = pimage->pitch>>2;
		pSrc = (unsigned long *)buffer;
		pDst = (unsigned long *)pimage->imagebits;

		for (hi=pimage->height; hi>0; hi--, pSrc+=srcLineLength, pDst+=dstLineLength)
			memcpy(pDst, pSrc, pimage->width<<2);
#endif

#if 0//MW_CPU_LITTLE_ENDIAN
		Convert32bppEndian(pimage);
#endif

		/* allocate id*/
		pItem = GdItemNew(IMAGEITEM);
		if (!pItem) {
			NANOX_FREE(pimage);
			id = 0;
			goto done;
		}
		pItem->id = nextimageid++;
		pItem->pimage = pimage;
		pItem->psd = psd;
		GdListAdd(&imagehead, &pItem->link);

		id = pItem->id;
	}

done:
	if (id) {
		GdDrawRawDataImageToFit(psd, x, y, width, height, id);

		GdFreeImage(id);
	}
}


/**
 * Draw an image from a memory buffer.
 *
 * @param psd Drawing surface.
 * @param x X destination co-ordinate.
 * @param y Y destination co-ordinate.
 * @param width If >=0, the image will be scaled to this width.
 * If <0, the image will not be scaled horiziontally.
 * @param height If >=0, the image will be scaled to this height.
 * If <0, the image will not be scaled vertically.
 * @param buffer The buffer containing the image data.
 * @param size The size of the buffer.
 * @param flags If nonzero, JPEG images will be loaded as grayscale.  Yuck!
 */
void
GdDrawImageFromBuffer(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
	MWCOORD height, void *buffer, int size, int flags)
{
	int id;
	buffer_t src;

	binit(&src, buffer, size);

	id = GdDecodeImage(psd, &src, NULL, flags);

	if (id) {
		GdDrawImageToFit(psd, x, y, width, height, id);
		GdFreeImage(id);
	}
}

#if defined(HAVE_FILEIO)
/**
 * Draw an image from a file.
 *
 * @param psd Drawing surface.
 * @param x X destination co-ordinate.
 * @param y Y destination co-ordinate.
 * @param width If >=0, the image will be scaled to this width.
 * If <0, the image will not be scaled horiziontally.
 * @param height If >=0, the image will be scaled to this height.
 * If <0, the image will not be scaled vertically.
 * @param path The file containing the image data.
 * @param flags If nonzero, JPEG images will be loaded as grayscale.  Yuck!
 */
void
GdDrawImageFromFile(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
	MWCOORD height, char *path, int flags)
{
	int	id;

	id = GdLoadImageFromFile(psd, path, flags);
	if (id) {
		GdDrawImageToFit(psd, x, y, width, height, id);
		GdFreeImage(id);
	}
}
#endif /* defined(HAVE_FILEIO) */

#if defined(HAVE_FILEIO)
/**
 * Load an image from a file.
 *
 * @param psd Drawing surface.
 * @param path The file containing the image data.
 * @param flags If nonzero, JPEG images will be loaded as grayscale.  Yuck!
 */
int
GdLoadImageFromFile(PSD psd, char *path, int flags)
{
  int fd, id;
  struct stat s;
  void *buffer = 0;
  buffer_t src;

#ifdef WIN32
  fd = open(path, O_RDONLY | O_BINARY);
#else
  fd = open(path, O_RDONLY, 0);
#endif

  if (fd < 0 || fstat(fd, &s) < 0) {
    EPRINTF("GdLoadImageFromFile: can't open image: %s\n", path);
    return(0);
  }

#if HAVE_MMAP
  buffer = mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (!buffer) {
    EPRINTF("GdLoadImageFromFile: Couldn't map image %s\n", path);
    close(fd);
    return(0);
  }
#else
  buffer = NANOX_MALLOC(s.st_size);
  if (!buffer) {
     EPRINTF("GdLoadImageFromFile: Couldn't load image %s\n", path);
     close(fd);
     return(0);
  }

  if (read(fd, buffer, s.st_size) != s.st_size) {
    EPRINTF("GdLoadImageFromFile: Couldn't load image %s\n", path);
    close(fd);
    return(0);
  }
#endif

  binit(&src, buffer, s.st_size);
  id = GdDecodeImage(psd, &src, path, flags);

#if HAVE_MMAP
  munmap(buffer, s.st_size);
#else
  NANOX_FREE(buffer);
#endif

  close(fd);
  return(id);
}
#endif /* defined(HAVE_FILEIO) */

/*
 * GdDecodeImage:
 * @psd: Drawing surface.
 * @src: The image data.
 * @flags: If nonzero, JPEG images will be loaded as grayscale.  Yuck!
 *
 * Load an image.
 */

static int
GdDecodeImage(PSD psd, buffer_t * src, char *path, int flags)
{
	int         loadOK, fReverse;
	PMWIMAGEHDR pimage;
	PIMAGEITEM  pItem;
	MWBOOL     bConvert = TRUE;
	MWBOOL     bPNGbyHW_flag = 0;

	fReverse = (flags&MWIMAGE_REVERSE ? 1 : 0);
	flags &= (~MWIMAGE_REVERSE);	//

#if defined(HAVE_GIF_SUPPORT) /* T.J.Park : To support animated GIF, the function definition is changed */
	/* pimage is allocated in LoadGIFEx(), unlike other image loading functions */
	if (LoadGIFEx(src, &pimage, 1) > 0)
		goto done;
#endif

	loadOK = 0;
	/* allocate image struct*/
	pimage = (PMWIMAGEHDR)NANOX_MALLOC(sizeof(MWIMAGEHDR));
	if (!pimage)
	{
		dbgprint("^r^ ----------- sys mem not enough!!!! line:%d in %s------------", __LINE__, __FUNCTION__);
		return 0;
	}

	memset(pimage, 0, sizeof(MWIMAGEHDR));
	pimage->transcolor = -1L;

#if defined(HAVE_TIFF_SUPPORT)
	/* must be first... no buffer support yet*/
	if (path)
		loadOK = LoadTIFF(path, pimage);
#endif

#if defined(HAVE_I16_SUPPORT)
	if (loadOK == 0)
		loadOK = LoadI16(src, pimage);
#endif

#if defined(HAVE_PNG_SUPPORT)
	if (loadOK == 0)
	{
		#if 1
		//First, try HW decoding
		loadOK = LoadPNGOnlyHW(src, pimage);
		#else
		//Skip HW decoding (the same to the legacy SW decoding)
		loadOK = 0;
		#endif
		if(loadOK == 1)
		{
			bPNGbyHW_flag = 1;
		}
		else
		{
			//try SW decoding
			bPNGbyHW_flag = 0;
			loadOK = LoadPNG(src, pimage);
		}
	}
#endif

#if defined(HAVE_LGI_SUPPORT)
	if (loadOK == 0)
		loadOK = LoadLGI(src, pimage);
#endif
#if defined(HAVE_BMP_SUPPORT)
	if (loadOK == 0)
		loadOK = LoadBMP(src, pimage);
#endif
#if defined(HAVE_JPEG_SUPPORT)
	if (loadOK == 0)
		loadOK = LoadJPEG(src, pimage, psd, flags);
#endif
#if defined(HAVE_PNM_SUPPORT)
	if (loadOK == 0)
		loadOK = LoadPNM(src, pimage);
#endif
#if defined(HAVE_XPM_SUPPORT)
	if (loadOK == 0)
		loadOK = LoadXPM(src, pimage, psd);
#endif

	if(loadOK == 1) {
		pimage->log_screen_width = pimage->width;
		pimage->log_screen_height = pimage->height;
#if (MWPIXEL_FORMAT == MWPF_TRUECOLOR1555)
		switch (pimage->bpp) {
			//case 16 :	// Endian Conversion.
			//	break;
			case 24 :
				bConvert = Convert24bppTo16bppARGB1555(pimage);
				break;
			case 32 :
				bConvert = Convert32bppTo16bppARGB1555(pimage);
				break;
		}
		if(!bConvert)
		{
			EPRINTF("[%s][%d] image bpp convert error\n", __FUNCTION__, __LINE__);
			goto err;		/* image loading error*/
		}
		if (fReverse)
			ReverseImage16(pimage);
#endif
#if (MWPIXEL_FORMAT == MWPF_TRUECOLOR8888)
		if(bPNGbyHW_flag == 0)
		{
			switch (pimage->bpp) {
				case 16 :
					bConvert = Convert16bppTo32bppARGB8888(pimage);
					break;
				case 24 :
					bConvert = Convert24bppTo32bppARGB8888(pimage);
					break;
#if MW_CPU_LITTLE_ENDIAN
				case 32 :	// Endian Conversion.
					EPRINTF("Convert32bppEndian Skip!");
					Convert32bppEndian(pimage);
					break;
		}
#endif
		}
		if(!bConvert)
		{
			EPRINTF("[%s][%d] image bpp convert error\n", __FUNCTION__, __LINE__);
			goto err;		/* image loading error*/
		}
		if (fReverse)
		{
			EPRINTF("ReverseImage32 Skip!");
			ReverseImage32(pimage);
		}
#endif
	}
	else {
		EPRINTF("GdLoadImageFromFile: unknown image type\n");
		goto err;		/* image loading error*/
	}

#if defined(HAVE_GIF_SUPPORT)
done:
#endif

	/* allocate id*/
	pItem = GdItemNew(IMAGEITEM);
	if (!pItem)
		goto err;
	pItem->id = nextimageid++;
	pItem->pimage = pimage;
	pItem->psd = psd;
	GdListAdd(&imagehead, &pItem->link);

	return pItem->id;

err:
	NANOX_FREE(pimage);
	return 0;			/* image loading error*/
}

#if (MWPIXEL_FORMAT == MWPF_TRUECOLOR1555)
/* This function is only for 16bpp ARG1555 (stonekim 20070516) */
static MWBOOL Convert32bppTo16bppARGB1555(PMWIMAGEHDR pImage)
{
	int wi, hi, pitch16;
	unsigned long *pB32;
	unsigned short *pB16;
	MWUCHAR *pImageBits16, *pBitmap32, *pBitmap16;

	pitch16 = pImage->width<<1;
#if 0
	pImageBits16 = NANOX_MALLOC(pitch16*pImage->height);
#else
	pImageBits16 = NANOX_ScrAllocateFrame(16, pImage->width, pImage->height, pitch16*pImage->height, &pitch16);
#endif
	if (pImageBits16==NULL)
	{
		EPRINTF("[%s][%d]: no memory \n", __FUNCTION__, __LINE__);
		return FALSE;	// Fail to convert
	}

	pBitmap32 = pImage->imagebits;
	pBitmap16 = pImageBits16;
	for (hi=pImage->height; hi>0; hi--, pBitmap32+=pImage->pitch, pBitmap16+=pitch16)
		for (wi=pImage->width, pB32 = (unsigned long *)pBitmap32, pB16 = (unsigned short *)pBitmap16; wi>0; wi--, pB32++, pB16++) {
			if (*pB32&0xff000000) {
				*pB16 = ((*pB32&0xf8) >> 3)|((*pB32&0xf800) >> 6)|((*pB32&0xf80000) >> 9);
				if (*pB16==0)
					*pB16 = 1;
				*pB16 |= ((*pB32&0xff000000)==0xff000000 ? 0x0000 : 0x8000);
			}
			else
				*pB16 = 0;
		}

	pImage->bpp = 16;
	pImage->bytesperpixel = 2;
	pImage->pitch = pitch16;
#if 0
	NANOX_FREE(pImage->imagebits);
#else
	NANOX_ScrFreeFrame(pImage->imagebits);
#endif
	pImage->imagebits = pImageBits16;
	return TRUE;
}

/* This function is only for 16bpp ARG1555 */
static MWBOOL Convert24bppTo16bppARGB1555(PMWIMAGEHDR pImage)
{
	int wi, hi, pitch16;
	unsigned char *pB24;
	unsigned short *pB16;
	unsigned long c32;
	MWUCHAR *pImageBits16, *pBitmap24, *pBitmap16;

	pitch16 = pImage->width<<1;
#if 0
	pImageBits16 = NANOX_MALLOC(pitch16*pImage->height);
#else
	pImageBits16 = NANOX_ScrAllocateFrame(16, pImage->width, pImage->height, pitch16*pImage->height, &pitch16);
#endif
	if (pImageBits16==NULL)
	{
		EPRINTF("[%s][%d]: no memory \n", __FUNCTION__, __LINE__);
		return FALSE;	// Fail to convert
	}
	pBitmap24 = pImage->imagebits;
	pBitmap16 = pImageBits16;
	for (hi=pImage->height; hi>0; hi--, pBitmap24+=pImage->pitch, pBitmap16+=pitch16)
		for (wi=pImage->width, pB24 = (unsigned char *)pBitmap24, pB16 = (unsigned short *)pBitmap16; wi>0; wi--, pB24+=3, pB16++) {
			c32 = 0xff00|*pB24, c32 <<= 8, c32 |= *(pB24+1), c32 <<= 8, c32 |= *(pB24+2);
			if (c32&0xff000000) {
				*pB16 = ((c32&0xf8) >> 3)|((c32&0xf800) >> 6)|((c32&0xf80000) >> 9);
				if (*pB16==0)
					*pB16 = 1;
				*pB16 |= ((c32&0xff000000)==0xff000000 ? 0x0000 : 0x8000);
			}
			else
				*pB16 = 0;
		}

	pImage->bpp = 16;
	pImage->bytesperpixel = 2;
	pImage->pitch = pitch16;
#if 0
	NANOX_FREE(pImage->imagebits);
#else
	NANOX_ScrFreeFrame(pImage->imagebits);
#endif
	pImage->imagebits = pImageBits16;
	return TRUE;
}

static void ReverseImage16(PMWIMAGEHDR pImage)
{
	int wi, hi, pitch16;
	unsigned short *pB16, *pRB16, temp;
	MWUCHAR *pBitmap16;

	pBitmap16 = pImage->imagebits;
	pitch16 = pImage->pitch;
	for (hi=pImage->height; hi>0; hi--, pBitmap16+=pitch16) {
		pB16 = (unsigned short *)pBitmap16;
		pRB16 = (unsigned short *)pBitmap16+pImage->width-1;
		for (wi=((pImage->width+1)>>1); wi>0; wi--, pB16++, pRB16--)
			temp = *pB16, *pB16 = *pRB16, *pRB16 = temp;
	}
}
#endif

#if (MWPIXEL_FORMAT == MWPF_TRUECOLOR8888)
/* This function is only for 32bpp ARG8888 (stonekim 20070516) */
static MWBOOL Convert16bppTo32bppARGB8888(PMWIMAGEHDR pImage)
{
	extern unsigned long scrVosdAlphaValue;	// scr_linux32bpp.c : for chnage ARGB1555 to ARGB8888 : 20080515 stonekim
	int wi, hi, pitch32;
	unsigned long *pB32;
	unsigned short *pB16;
	MWUCHAR *pImageBits32, *pBitmap32, *pBitmap16;

	pitch32 = pImage->width<<2;
#if 0
	pImageBits32 = NANOX_MALLOC(pitch32*pImage->height);
#else
	pImageBits32 = NANOX_ScrAllocateFrame(32, pImage->width, pImage->height, pitch32*pImage->height, &pitch32);
#endif
	if (pImageBits32==NULL)
	{
		EPRINTF("[%s][%d]: no memory \n", __FUNCTION__, __LINE__);
		return FALSE;	// Fail to convert
	}

	pBitmap16 = pImage->imagebits;
	pBitmap32 = pImageBits32;
	for (hi=pImage->height; hi>0; hi--, pBitmap16+=pImage->pitch, pBitmap32+=pitch32)
		for (wi=pImage->width, pB32 = (unsigned long *)pBitmap32, pB16 = (unsigned short *)pBitmap16; wi>0; wi--, pB32++, pB16++) {
			if (*pB16)
				*pB32 = ((*pB16&0x7c00)<<9)|((*pB16&0x03e0)<<6)|((*pB16&0x001f)<<3)|((*pB16&0x8000) ? scrVosdAlphaValue : 0xff000000);
			else
				*pB32 = 0;
		}

	pImage->bpp = 32;
	pImage->bytesperpixel = 4;
	pImage->pitch = pitch32;
#if 0
	NANOX_FREE(pImage->imagebits);
#else
	NANOX_ScrFreeFrame(pImage->imagebits);
#endif
	pImage->imagebits = pImageBits32;
	return TRUE;
}

static MWBOOL Convert24bppTo32bppARGB8888(PMWIMAGEHDR pImage)
{
	int wi, hi, pitch32;
	unsigned char *pB24;
	unsigned long *pB32;
	MWUCHAR *pImageBits32, *pBitmap24, *pBitmap32;

	pitch32 = pImage->width<<2;
#if 0
	pImageBits32 = NANOX_MALLOC(pitch32*pImage->height);
#else
	pImageBits32 = NANOX_ScrAllocateFrame(32, pImage->width, pImage->height, pitch32*pImage->height, &pitch32);
#endif
	if (pImageBits32==NULL)
	{
		EPRINTF("[%s][%d]: no memory \n", __FUNCTION__, __LINE__);
		return FALSE;	// Fail to convert
	}

	pBitmap24 = pImage->imagebits;
	pBitmap32 = pImageBits32;
	for (hi=pImage->height; hi>0; hi--, pBitmap24+=pImage->pitch, pBitmap32+=pitch32)
		for (wi=pImage->width, pB24 = (unsigned char *)pBitmap24, pB32 = (unsigned long *)pBitmap32; wi>0; wi--, pB24+=3, pB32++)
			*pB32 = 0xff00, *pB32 |= *pB24, *pB32 <<= 8, *pB32 |= *(pB24+1), *pB32 <<= 8, *pB32 |= *(pB24+2);

	pImage->bpp = 32;
	pImage->bytesperpixel = 4;
	pImage->pitch = pitch32;
#if 0
	NANOX_FREE(pImage->imagebits);
#else
	NANOX_ScrFreeFrame(pImage->imagebits);
#endif
	pImage->imagebits = pImageBits32;
	return TRUE;
}

static void Convert32bppEndian(PMWIMAGEHDR pImage)
{
	int wi, hi, pitch32;
	unsigned long *pB32, temp;
	MWUCHAR *pBitmap32;

	pBitmap32 = pImage->imagebits;
	pitch32 = pImage->pitch;
	for (hi=pImage->height; hi>0; hi--, pBitmap32+=pitch32)
		for (wi=pImage->width, pB32 = (unsigned long *)pBitmap32; wi>0; wi--, pB32++)
			temp = *pB32, *pB32 = (temp>>24)|((temp>>8)&0xff00)|((temp<<8)&0xff0000)|(temp<<24);
}

static void ReverseImage32(PMWIMAGEHDR pImage)
{
	int wi, hi, pitch32;
	unsigned long *pB32, *pRB32, temp;
	MWUCHAR *pBitmap32;

	pBitmap32 = pImage->imagebits;
	pitch32 = pImage->pitch;
	for (hi=pImage->height; hi>0; hi--, pBitmap32+=pitch32) {
		pB32 = (unsigned long *)pBitmap32;
		pRB32 = (unsigned long *)pBitmap32+pImage->width-1;
		for (wi=(pImage->width>>1); wi>0; wi--, pB32++, pRB32--)
			temp = *pB32, *pB32 = *pRB32, *pRB32 = temp;
	}
}

#endif

#if defined(HAVE_I16_SUPPORT)
static int LoadI16(buffer_t * src, PMWIMAGEHDR pImage)
{
	#define QUEUE_SIZE			256
	#define QUEUE_MASK			0xff
	#define MAX_LENGTH			0x3f
	#define MAX_LENGTH_OVER256	0x1f
	#define PUSH_QUEUE(x)	Queue[QueueIndex]=x,QueueIndex=(QueueIndex+1)&QUEUE_MASK

	unsigned char Queue[QUEUE_SIZE];
	int QueueIndex;

	int dest_size, i, j, qi, x, tx, paddingLength;
	unsigned char length;
	unsigned short *pDest, *pd, c, numOfColors, width, height;
	unsigned char *pSource, *pColorValues;

	// Init pSource
	pSource = src->start;

	// Identifier
	if ((*pSource!='1')||(*(pSource+1)!='6')) {
		return 0;
	}

	// Get Info
	width = *(pSource+2), width<<=8, width |= *(pSource+3);
	height = *(pSource+4), height<<=8, height |= *(pSource+5);
	numOfColors = *(pSource+6), numOfColors<<=8, numOfColors |= *(pSource+7);

	// Init pImage
	pImage->bpp = 16;
	pImage->bytesperpixel = 2;
	pImage->width = width;
	pImage->height = height;
	pImage->planes = 1;
	pImage->compression = MWIMAGE_RGB;
	pImage->pitch = (width<<1);
#if 0
	if (!(pImage->imagebits = NANOX_MALLOC(pImage->pitch * pImage->height)))
		return 0;
#else
	if (!(pImage->imagebits = NANOX_ScrAllocateFrame(16, pImage->width, pImage->height, pImage->pitch * pImage->height, &pImage->pitch)))
		return 0;
#endif
	pDest = (unsigned short *)pImage->imagebits;

	// Color Values
	pColorValues = pSource+8;

	// Compressed Area
	pSource += (8+numOfColors*2);

	// Queue Reset
	dest_size = ((unsigned long)(*pSource)<<24)|((unsigned long)(*(pSource+1))<<16)|
	  ((unsigned long)(*(pSource+2))<<8)|(unsigned long)*(pSource+3);
	pSource += sizeof(unsigned long);

	memset(Queue, 0, QUEUE_SIZE);
	QueueIndex = 0;

	i = 0, x = 0, paddingLength = (pImage->pitch>>1) - width;
	while (i<dest_size) {
		switch (*pSource & 0xc0) {
			case 0x00 :		//	1. Continueous
				length = *pSource & 0x3f, i += length;
				pSource++, c = *(pColorValues+*pSource*2), c<<=8, c |= *(pColorValues+*pSource*2+1), pSource++;
				for (j=0; j<length; j++, pDest++) {
					*pDest = c;
					x++;
					if (x==width)
						pDest += paddingLength, x = 0;
				}
				break;
			case 0x40 :		//	2. None Continueous
				length = *pSource & 0x3f, i += length;
				pSource++;
				for (j=0; j<length; j++, pDest++, pSource++) {
					*pDest = *(pColorValues+*pSource*2), *pDest<<=8, *pDest |= *(pColorValues+*pSource*2+1);
					PUSH_QUEUE(*pSource);
					x++;
					if (x==width)
						pDest += paddingLength, x = 0;
				}
				break;
			case 0x80 :		//	3. Patternning : 64 Level Patternning
				length = *pSource & 0x3f, i += length;
				pSource++, qi = *pSource, pSource++;
				for (j=0, pd=pDest, tx=x; j<length; j++, pDest++, qi=(qi+1)&QUEUE_MASK) {
					*pDest = Queue[qi];
					x++;
					if (x==width)
						pDest += paddingLength, x = 0;
				}
				for (j=0; j<length; j++, pd++) {
					PUSH_QUEUE((unsigned char)*pd);
					c = *(pColorValues+*pd*2), c<<=8, c |= *(pColorValues+*pd*2+1);
					*pd = c;
					tx++;
					if (tx==width)
						pd += paddingLength, tx = 0;
				}
				break;
			case 0xc0 :		//	4. Over 255 Index
				if (*pSource & 0x20) {	// Over 0x1fff Index
					length = *pSource & 0x1f, i += length;
					pSource++;
					for (j=0; j<length; j++, pDest++) {
						*pDest = *pSource, *pDest <<= 8, pSource++, *pDest |= *pSource, pSource++;
						x++;
						if (x==width)
							pDest += paddingLength, x = 0;
					}
				}
				else {	// Over 255 under 0x2000 Index
					*pDest = *pSource & 0x1f, *pDest <<= 8, pSource++, *pDest |= *pSource;
					c = *(pColorValues+*pDest*2), c<<=8, c |= *(pColorValues+*pDest*2+1);
					*pDest = c, pSource++, pDest++, i++;
					x++;
					if (x==width)
						pDest += paddingLength, x = 0;
				}
				break;
		}
	}

	return 1;
}
#endif
/* This function is added to support animated GIF. */
static void
GdDecodeImageEx(PSD psd, buffer_t * src, char *path, MW_LOAD_IMAGE_EX_RESULT *pResult)
{
	if(pResult == NULL)
		return;

#if defined(HAVE_GIF_SUPPORT)
	if(IsGIF(src))
	{
		PMWIMAGEHDR pimage[MAX_LOAD_IMAGE_EX];
		PIMAGEITEM  pItem;
		int nLoadedImageCount;
		int i;

		bseek(src, 0, SEEK_SET);
		nLoadedImageCount = LoadGIFEx(src, pimage, MAX_LOAD_IMAGE_EX);

		/* Register all the loaded image */
		pResult->count = 0;
		for(i = 0;i < nLoadedImageCount;i++)
		{
			/* allocate id*/
			pItem = GdItemNew(IMAGEITEM);
			if (pItem)
			{
				pResult->images[pResult->count++] = pItem->id = nextimageid++;
				pItem->pimage = pimage[i];
				pItem->psd = psd;
				GdListAdd(&imagehead, &pItem->link);
			}
			else
			{
				if(pimage[i]->imagebits) {
#if 0
					NANOX_FREE(pimage[i]->imagebits);
#else
					NANOX_ScrFreeFrame(pimage[i]->imagebits);
#endif
				}
				if(pimage[i]->palette)
					NANOX_FREE(pimage[i]->palette);
				NANOX_FREE(pimage[i]);
			}
		}
	}
	else
#endif
	{
		int id;

		bseek(src, 0, SEEK_SET);
		id = GdDecodeImage(psd, src, path, 0);
		if(id)
		{
			pResult->count = 1;
			pResult->images[0] = id;
		}
		else
		{
			pResult->count = 0;
		}
	}
}

static PIMAGEITEM
findimage(int id)
{
	PMWLIST		p;
	PIMAGEITEM	pimagelist;

	for (p=imagehead.head; p; p=p->next) {
		pimagelist = GdItemAddr(p, IMAGEITEM, link);
		if (pimagelist->id == id)
		{
			if(pimagelist->pimage == NULL)
			{
				dbgprint("^R^ [Warn] item(id=%d) has NULL pimage in %s\n\n\n------------------------------\n", id, __FUNCTION__);
				//OSA_PrintStack();
				//continue; //return NULL
			}
			return pimagelist;
		}
	}
	return NULL;
}

/**
 * Extended version of GdDrawImageToFit, written by T.J.Park.
 * This function allows the caller to specify the image area to be drawn,
 * while GdDrawImageToFit always draws the whole image.
 *
 * @param psd Drawing surface.
 * @param x X destination co-ordinate.
 * @param y Y destination co-ordinate.
 * @param width If >=0, the image will be scaled to this width.
 * If <0, the image will not be scaled horiziontally.
 * @param height If >=0, the image will be scaled to this height.
 * If <0, the image will not be scaled vertically.
 * @param id Image to draw.
 * @param srcX X source(image) co-ordinate.
 * @param srcY Y source(image) co-ordinate.
 * @param srcWidth width of the source(image).
 * @param srcHeight height of the source(image).
 */
void
GdDrawImageToFitEx(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
	int id, MWCOORD srcX, MWCOORD srcY, MWCOORD srcWidth, MWCOORD srcHeight)
{
	PIMAGEITEM	pItem;
	PMWIMAGEHDR	pimage;

	if ((width==0)||(height==0)||(srcWidth==0)||(srcHeight==0))
		return;

	pItem = findimage(id);
	if (!pItem)
		return;
	pimage = pItem->pimage;

	/*
	 * Display image, possibly stretch/shrink to resize
	 */
	if(srcX < 0)
		srcX = 0;
	if(srcX >= pimage->width)
		return;

	if(srcY < 0)
		srcY = 0;
	if(srcY >= pimage->height)
		return;

	if(srcWidth < 0)
		srcWidth = pimage->width;
	if(srcX + srcWidth > pimage->width)
		srcWidth = pimage->width - srcX;
	if(srcHeight < 0)
		srcHeight = pimage->height;
	if(srcY + srcHeight > pimage->height)
		srcHeight = pimage->height - srcY;

	if (height < 0)
		height = srcHeight;
	if (width < 0)
		width = srcWidth;

	if (height != srcHeight || width != srcWidth) {
		MWCLIPRECT	rcSrc;
		MWCLIPRECT	rcDst;
		MWIMAGEHDR	image2;

		/* create similar image, different width/height*/

		image2.width = width;
		image2.height = height;
		image2.planes = pimage->planes;
		image2.bpp = pimage->bpp;
		ComputePitch(pimage->bpp, width, &image2.pitch,
			&image2.bytesperpixel);
		image2.compression = pimage->compression;
		image2.palsize = pimage->palsize;
		image2.palette = pimage->palette;	/* already allocated*/
		image2.transcolor = pimage->transcolor;
#if 0
		if( (image2.imagebits = NANOX_MALLOC(image2.pitch*height)) == NULL) {
			EPRINTF("GdDrawImageToFit: no memory\n");
			return;
		}
#else
		if( (image2.imagebits = NANOX_ScrAllocateFrame(image2.bpp, width, height, image2.pitch*height, &image2.pitch)) == NULL) {
			EPRINTF("GdDrawImageToFit: no memory\n");
			return;
		}
#endif
		rcSrc.x = srcX;
		rcSrc.y = srcY;
		rcSrc.width = srcWidth;
		rcSrc.height = srcHeight;

		rcDst.x = 0;
		rcDst.y = 0;
		rcDst.width = width;
		rcDst.height = height;

		/* Stretch some part of source to destination rectangle*/
		GdStretchImage(pimage, &rcSrc, &image2, &rcDst);
		GdDrawImage(psd, x, y, &image2);
#if 0
		NANOX_FREE(image2.imagebits);
#else
		NANOX_ScrFreeFrame(image2.imagebits);
#endif
	} else
		GdDrawImageEx(psd, x, y, pimage, srcX, srcY, srcWidth, srcHeight);
}

/**
 * Draw an image.
 *
 * @param psd Drawing surface.
 * @param x X destination co-ordinate.
 * @param y Y destination co-ordinate.
 * @param width If >=0, the image will be scaled to this width.
 * If <0, the image will not be scaled horiziontally.
 * @param height If >=0, the image will be scaled to this height.
 * If <0, the image will not be scaled vertically.
 * @param id Image to draw.
 */
void
GdDrawImageToFit(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
	int id)
{
	PIMAGEITEM	pItem;
	PMWIMAGEHDR	pimage;

	if ((width==0)||(height==0))
		return;

	pItem = findimage(id);
	if (!pItem)
		return;
	pimage = pItem->pimage;

	if((pimage->xoffset > 0 && pimage->log_screen_width > pimage->width)
	|| (pimage->yoffset > 0 && pimage->log_screen_height > pimage->height))
	{ /* GIF image with offset. This may be an animated GIF. */
		if(height < 0)
			height = pimage->log_screen_height;
		if(width < 0)
			width = pimage->log_screen_width;

		if(height != pimage->log_screen_height || width != pimage->log_screen_width)
		{ /* We need to stretch it */
			MWCOORD image_w;
			MWCOORD image_h;
			MWCOORD x_offset;
			MWCOORD y_offset;

			/* Calculate the width/height of the image, relative to the GIF screen width/height */
			image_w = pimage->width * width / pimage->log_screen_width;
			image_h = pimage->height * height / pimage->log_screen_height;
			/* Calcuate the x/y offset of the image, relative to the GIF screen left/top position */
			x_offset = pimage->xoffset * width / pimage->log_screen_width;
			y_offset = pimage->yoffset * height / pimage->log_screen_height;

			width = image_w;
			height = image_h;
			x += x_offset;
			y += y_offset;
		}
		else
		{
			width = pimage->width;
			height = pimage->height;
			x += pimage->xoffset;
			y += pimage->yoffset;
		}
	}
	else
	{ /* Normal Image : PNG, JPEG, Simple GIF, ... */
		if (height < 0)
			height = pimage->height;
		if (width < 0)
			width = pimage->width;
	}


	/*
	 * Display image, possibly stretch/shrink to resize
	 */

	if (height != pimage->height || width != pimage->width) {
		MWCLIPRECT	rcDst;
		MWIMAGEHDR	image2;

		/* create similar image, different width/height*/

		image2.width = width;
		image2.height = height;
		image2.planes = pimage->planes;
		image2.bpp = pimage->bpp;
		ComputePitch(pimage->bpp, width, &image2.pitch,
			&image2.bytesperpixel);
		image2.compression = pimage->compression;
		image2.palsize = pimage->palsize;
		image2.palette = pimage->palette;	/* already allocated*/
		image2.transcolor = pimage->transcolor;
#if 0
		if( (image2.imagebits = NANOX_MALLOC(image2.pitch*height)) == NULL) {
			EPRINTF("GdDrawImageToFit: no memory\n");
			return;
		}
#else
		if( (image2.imagebits = NANOX_ScrAllocateFrame(image2.bpp, width, height, image2.pitch*height, &image2.pitch)) == NULL) {
			EPRINTF("GdDrawImageToFit: no memory\n");
			return;
		}
#endif
		rcDst.x = 0;
		rcDst.y = 0;
		rcDst.width = width;
		rcDst.height = height;

		/* Stretch full source to destination rectangle*/
		GdStretchImage(pimage, NULL, &image2, &rcDst);
		GdDrawImage(psd, x, y, &image2);
#if 0
		NANOX_FREE(image2.imagebits);
#else
		NANOX_ScrFreeFrame(image2.imagebits);
#endif
	} else
		GdDrawImage(psd, x, y, pimage);
}



void
GdDrawRawDataImageToFit(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
	int id)
{
	PIMAGEITEM	pItem;
	PMWIMAGEHDR	pimage;

	if ((width==0)||(height==0))
		return;

	pItem = findimage(id);
	if (!pItem)
		return;
	pimage = pItem->pimage;

	{ /* Normal Image : PNG, JPEG, Simple GIF, ... */
		if (height < 0)
			height = pimage->height;
		if (width < 0)
			width = pimage->width;
	}


	/*
	 * Display image, possibly stretch/shrink to resize
	 */

	if (height != pimage->height || width != pimage->width) {
		MWCLIPRECT	rcDst;
		MWIMAGEHDR	image2;

		/* create similar image, different width/height*/

		image2.width = width;
		image2.height = height;
		image2.planes = pimage->planes;
		image2.bpp = pimage->bpp;
		ComputePitch(pimage->bpp, width, &image2.pitch,
			&image2.bytesperpixel);
		image2.compression = pimage->compression;
		image2.palsize = pimage->palsize;
		image2.palette = pimage->palette;	/* already allocated*/
		image2.transcolor = pimage->transcolor;
#if 0
		if( (image2.imagebits = NANOX_MALLOC(image2.pitch*height)) == NULL) {
			EPRINTF("GdDrawImageToFit: no memory\n");
			return;
		}
#else
		if( (image2.imagebits = NANOX_ScrAllocateFrame(image2.bpp, width, height, image2.pitch*height, &image2.pitch)) == NULL) {
			EPRINTF("GdDrawImageToFit: no memory\n");
			return;
		}
#endif
		rcDst.x = 0;
		rcDst.y = 0;
		rcDst.width = width;
		rcDst.height = height;

		/* Stretch full source to destination rectangle*/
		if (NANOX_ScrStretchMemoryFrame(&image2, rcDst.x, rcDst.y, rcDst.width, rcDst.height,
									pimage, 0, 0, pimage->width, pimage->height) < 0)
		{
			GdStretchImage(pimage, NULL, &image2, &rcDst);
		}
		GdDrawImage(psd, x, y, &image2);
#if 0
		NANOX_FREE(image2.imagebits);
#else
		NANOX_ScrFreeFrame(image2.imagebits);
#endif
	} else
		GdDrawImage(psd, x, y, pimage);
}



/**
 * Destroy an image.
 *
 * @param id Image to free.
 */
void
GdFreeImage(int id)
{
	PIMAGEITEM	pItem;
	PMWIMAGEHDR	pimage;

	pItem = findimage(id);
	if (pItem) {
		GdListRemove(&imagehead, &pItem->link);
		pimage = pItem->pimage;

		/* delete image bits*/
		if(pimage->imagebits) {
#if 0
			NANOX_FREE(pimage->imagebits);
#else
			NANOX_ScrFreeFrame(pimage->imagebits);
#endif
		}
		if(pimage->palette)
			NANOX_FREE(pimage->palette);

		if(pimage)
			NANOX_FREE(pimage);
		GdItemFree(pItem);
	}
}

/**
 * Get information about an image.
 *
 * @param id Image to query.
 * @param pii Destination for image information.
 * @return TRUE on success, FALSE on error.
 */
MWBOOL
GdGetImageInfo(int id, PMWIMAGEINFO pii)
{
	PMWIMAGEHDR	pimage;
	PIMAGEITEM	pItem;
	int		i;

	pItem = findimage(id);
	if (!pItem) {
		memset(pii, 0, sizeof(*pii));
		return FALSE;
	}

	if (!pItem->pimage) {
		dbgprint("^R^ [Warn] item(id=%d) has NULL pimage in %s\n\n\n------------------------------\n", id, __FUNCTION__);
		OSA_PrintStack();
		memset(pii, 0, sizeof(*pii));
		return FALSE;
	}
	pimage = pItem->pimage;
	pii->id = id;
	pii->width = pimage->width;
	pii->height = pimage->height;
	pii->planes = pimage->planes;
	pii->bpp = pimage->bpp;
	pii->pitch = pimage->pitch;
	pii->bytesperpixel = pimage->bytesperpixel;
	pii->compression = pimage->compression;
	/* For animated GIF */
	pii->log_screen_width = pimage->log_screen_width;
	pii->log_screen_height = pimage->log_screen_height;
	pii->xoffset = pimage->xoffset;
	pii->yoffset = pimage->yoffset;
	pii->delaytime = pimage->delaytime;
	pii->inputflag = pimage->inputflag;
	pii->disposal = pimage->disposal;
	/* End of animated GIF specific */
	pii->palsize = pimage->palsize;
	if (pimage->palsize) {
		if (pimage->palette) {
			for (i=0; i<pimage->palsize; ++i)
				pii->palette[i] = pimage->palette[i];
		} else {
			/* FIXME handle jpeg's without palette*/
			GdGetPalette(pItem->psd, 0, pimage->palsize,
				pii->palette);
		}
	}
	pii->imagebits = pimage->imagebits;
	return TRUE;
}

#define PIX2BYTES(n)	(((n)+7)/8)
/*
 * compute image line size and bytes per pixel
 * from bits per pixel and width
 */
static void
ComputePitch(int bpp, int width, int *pitch, int *bytesperpixel)
{
	int	linesize;
	int	bytespp = 1;

	if(bpp == 1)
		linesize = PIX2BYTES(width);
	else if(bpp <= 4)
		linesize = PIX2BYTES(width<<2);
	else if(bpp <= 8)
		linesize = width;
	else if(bpp <= 16) {
		linesize = width * 2;
		bytespp = 2;
	} else if(bpp <= 24) {
		linesize = width * 3;
		bytespp = 3;
	} else {
		linesize = width * 4;
		bytespp = 4;
	}

	/* rows are DWORD right aligned*/
	*pitch = (linesize + 3) & ~3;
	*bytesperpixel = bytespp;
}

/*
 * Copyright (c) 2000, 2001, 2003, 2006 Greg Haerr <greg@censoft.com>
 *
 * StretchImage - Resize an image
 *
 * Major portions from SDL Simple DirectMedia Layer by Sam Lantinga
 * Copyright (C) 1997, 1998, 1999, 2000  Sam Lantinga <slouken@devolution.com>
 * This a stretch blit implementation based on ideas given to me by
 * Tomasz Cejner - thanks! :)
 */

#define DEFINE_COPY_ROW(name, type)					\
static void name(type *src, int src_w, type *dst, int dst_w)		\
{									\
	int i;								\
	int pos, inc;							\
	type pixel = 0;							\
									\
	pos = 0x10000;							\
	inc = (src_w << 16) / dst_w;					\
	for ( i=dst_w; i>0; --i ) {					\
		while ( pos >= 0x10000L ) {				\
			pixel = *src++;					\
			pos -= 0x10000L;				\
		}							\
		*dst++ = pixel;						\
		pos += inc;						\
	}								\
}

DEFINE_COPY_ROW(copy_row1, unsigned char)
DEFINE_COPY_ROW(copy_row2, unsigned short)
DEFINE_COPY_ROW(copy_row4, unsigned long)

static void copy_row3(unsigned char *src, int src_w, unsigned char *dst,
	int dst_w)
{
	int i;
	int pos, inc;
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
		while ( pos >= 0x10000L ) {
			b = *src++;
			g = *src++;
			r = *src++;
			pos -= 0x10000L;
		}
		*dst++ = b;
		*dst++ = g;
		*dst++ = r;
		pos += inc;
	}
}

/**
 * Perform a stretch blit between two image structs of the same format.
 *
 * @param src Source image.
 * @param srcrect Source rectangle.
 * @param dst Destination image.
 * @param dstrect Destination rectangle.
 */
void
GdStretchImage(PMWIMAGEHDR src, MWCLIPRECT *srcrect, PMWIMAGEHDR dst,
	MWCLIPRECT *dstrect)
{
	int pos, inc;
	int bytesperpixel;
	int dst_maxrow;
	int src_row, dst_row;
	MWUCHAR *srcp = 0;
	MWUCHAR *dstp;
	MWCLIPRECT full_src;
	MWCLIPRECT full_dst;

	if ( src->bytesperpixel != dst->bytesperpixel ) {
		EPRINTF("GdStretchImage: bytesperpixel mismatch\n");
		return;
	}

	/* Verify the blit rectangles */
	if ( srcrect ) {
		if ( (srcrect->x < 0) || (srcrect->y < 0) ||
		     ((srcrect->x+srcrect->width) > src->width) ||
		     ((srcrect->y+srcrect->height) > src->height) ) {
			EPRINTF("GdStretchImage: invalid source rect\n");
			return;
		}
	} else {
		full_src.x = 0;
		full_src.y = 0;
		full_src.width = src->width;
		full_src.height = src->height;
		srcrect = &full_src;
	}
	if ( dstrect ) {
		/* if stretching to nothing, return*/
		if (!dstrect->width || !dstrect->height)
			return;
		if ( (dstrect->x < 0) || (dstrect->y < 0) ||
		     ((dstrect->x+dstrect->width) > dst->width) ||
		     ((dstrect->y+dstrect->height) > dst->height) ) {
			EPRINTF("GdStretchImage: invalid dest rect\n");
			return;
		}
	} else {
		full_dst.x = 0;
		full_dst.y = 0;
		full_dst.width = dst->width;
		full_dst.height = dst->height;
		dstrect = &full_dst;
	}

	/* Set up the data... */
	pos = 0x10000;
	inc = (srcrect->height << 16) / dstrect->height;
	src_row = srcrect->y;
	dst_row = dstrect->y;
	bytesperpixel = dst->bytesperpixel;

	/* Perform the stretch blit */
	for ( dst_maxrow = dst_row+dstrect->height; dst_row<dst_maxrow;
								++dst_row ) {
		dstp = (MWUCHAR *)dst->imagebits + (dst_row*dst->pitch)
				    + (dstrect->x*bytesperpixel);
		while ( pos >= 0x10000L ) {
			srcp = (MWUCHAR *)src->imagebits + (src_row*src->pitch)
				    + (srcrect->x*bytesperpixel);
			++src_row;
			pos -= 0x10000L;
		}

		switch (bytesperpixel) {
		case 1:
			copy_row1(srcp, srcrect->width, dstp, dstrect->width);
			break;
		case 2:
			copy_row2((unsigned short *)srcp, srcrect->width,
				(unsigned short *)dstp, dstrect->width);
			break;
		case 3:
			copy_row3(srcp, srcrect->width, dstp, dstrect->width);
			break;
		case 4:
			copy_row4((unsigned long *)srcp, srcrect->width,
				(unsigned long *)dstp, dstrect->width);
			break;
		}

		pos += inc;
	}
}

#if defined(HAVE_JPEG_SUPPORT)
/* eme jpeg library를 사용하도록 수정 20060808 */
typedef unsigned int UINT16;
typedef unsigned char UINT8;
#include "jpeglib.h"
/*
 * JPEG decompression routine
 *
 * JPEG support must be enabled (see README.txt in contrib/jpeg)
 *
 * SOME FINE POINTS: (from libjpeg)
 * In the below code, we ignored the return value of jpeg_read_scanlines,
 * which is the number of scanlines actually read.  We could get away with
 * this because we asked for only one line at a time and we weren't using
 * a suspending data source.  See libjpeg.doc for more info.
 *
 * We cheated a bit by calling alloc_sarray() after jpeg_start_decompress();
 * we should have done it beforehand to ensure that the space would be
 * counted against the JPEG max_memory setting.  In some systems the above
 * code would risk an out-of-memory error.  However, in general we don't
 * know the output image dimensions before jpeg_start_decompress(), unless we
 * call jpeg_calc_output_dimensions().  See libjpeg.doc for more about this.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must emit data bottom-to-top,
 * you can use one of the virtual arrays provided by the JPEG memory manager
 * to invert the data.  See wrbmp.c for an example.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.doc.
 */
static buffer_t *inptr;

static void
init_source(j_decompress_ptr cinfo)
{
	cinfo->src->next_input_byte = inptr->start;
	cinfo->src->bytes_in_buffer = inptr->size;
}

static void
fill_input_buffer(j_decompress_ptr cinfo)
{
	return;
}

static void
skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	if (num_bytes >= inptr->size)
		return;
	cinfo->src->next_input_byte += num_bytes;
	cinfo->src->bytes_in_buffer -= num_bytes;
}

static boolean
resync_to_restart(j_decompress_ptr cinfo, int desired)
{
	return jpeg_resync_to_restart(cinfo, desired);
}

static void
term_source(j_decompress_ptr cinfo)
{
	return;
}

static int
LoadJPEG(buffer_t * src, PMWIMAGEHDR pimage, PSD psd, MWBOOL fast_grayscale)
{
	int i;
	int ret = 2;		/* image load error */
	unsigned char magic[8];
	struct jpeg_source_mgr smgr;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
#if FASTJPEG
	extern MWPALENTRY mwstdpal8[256];
#else
	MWPALENTRY palette[256];
#endif

	/* first determine if JPEG file since decoder will error if not */
	bseek(src, 0, SEEK_SET);
	if (!bread(src, magic, 2))
		return 0;
	if (magic[0] != 0xFF || magic[1] != 0xD8)
		return 0;	/* not JPEG image */

	bread(src, magic, 8);
	if (strncmp(magic+4, "JFIF", 4) != 0 && strncmp(magic+4, "Exif", 4) != 0 && strncmp(magic+4, "WANG",4) != 0)
		return 0;   /* not JPEG image */

	bseek(src, -1, SEEK_END);
	if (!bread(src, magic, 2))
		return 0;

	if (magic[0] != 0xFF || magic[1] != 0xD9)
	{
		dbgprint("LoadJPEG: EOI is broken (End of Image)\n");
		return 0;	/* not JPEG image EOI */
	}

	bread(src, 0, SEEK_SET);
	pimage->imagebits = NULL;
	pimage->palette = NULL;

	/* Step 1: allocate and initialize JPEG decompression object */
	/* We set up the normal JPEG error routines. */
	cinfo.err = jpeg_std_error(&jerr);

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2:  Setup the source manager */
	smgr.init_source = (void *) init_source;
	smgr.fill_input_buffer = (void *) fill_input_buffer;
	smgr.skip_input_data = (void *) skip_input_data;
	smgr.resync_to_restart = (void *) resync_to_restart;
	smgr.term_source = (void *) term_source;
	cinfo.src = &smgr;
	inptr = src;

	/* Step 2: specify data source (eg, a file) */
	/* jpeg_stdio_src (&cinfo, fp); */

	/* Step 3: read file parameters with jpeg_read_header() */
	jpeg_read_header(&cinfo, TRUE);
	/* Step 4: set parameters for decompression */
	cinfo.out_color_space = fast_grayscale? JCS_GRAYSCALE: JCS_RGB;
	cinfo.quantize_colors = FALSE;

#if FASTJPEG
	goto fastjpeg;
#endif
	if (!fast_grayscale) {
		if (psd->pixtype == MWPF_PALETTE) {
#if FASTJPEG
fastjpeg:
#endif
			cinfo.quantize_colors = TRUE;
#if FASTJPEG
			cinfo.actual_number_of_colors = 256;
#else
			/* Get system palette */
			cinfo.actual_number_of_colors =
				GdGetPalette(psd, 0, psd->ncolors, palette);
#endif

			/* Allocate jpeg colormap space */
			cinfo.colormap = (*cinfo.mem->alloc_sarray)
				((j_common_ptr) &cinfo, JPOOL_IMAGE,
			       	(JDIMENSION)cinfo.actual_number_of_colors,
				(JDIMENSION)3);

			/* Set colormap from system palette */
			for(i = 0; i < cinfo.actual_number_of_colors; ++i) {
#if FASTJPEG
				cinfo.colormap[0][i] = mwstdpal8[i].r;
				cinfo.colormap[1][i] = mwstdpal8[i].g;
				cinfo.colormap[2][i] = mwstdpal8[i].b;
#else
				cinfo.colormap[0][i] = palette[i].r;
				cinfo.colormap[1][i] = palette[i].g;
				cinfo.colormap[2][i] = palette[i].b;
#endif
			}
		}
	} else {
		/* Grayscale output asked */
		cinfo.quantize_colors = TRUE;
		cinfo.out_color_space = JCS_GRAYSCALE;
		cinfo.desired_number_of_colors = 256;
	}
	jpeg_calc_output_dimensions(&cinfo);

	pimage->width = cinfo.output_width;
	pimage->height = cinfo.output_height;
	pimage->planes = 1;
#if FASTJPEG
	pimage->bpp = 8;
#else
	pimage->bpp = (fast_grayscale || psd->pixtype == MWPF_PALETTE)?
		8: cinfo.output_components*8;
#endif
	ComputePitch(pimage->bpp, pimage->width, &pimage->pitch,
		&pimage->bytesperpixel);
	pimage->compression = MWIMAGE_RGB;	/* RGB not BGR order*/
	pimage->palsize = (pimage->bpp == 8)? 256: 0;
#if 0
	pimage->imagebits = NANOX_MALLOC(pimage->pitch * pimage->height);
#else
	pimage->imagebits = NANOX_ScrAllocateFrame(pimage->bpp, pimage->width, pimage->height, pimage->pitch * pimage->height, &pimage->pitch);
#endif
	if(!pimage->imagebits)
		goto err;
	pimage->palette = NULL;

	if(pimage->bpp <= 8) {
		pimage->palette = NANOX_MALLOC(256*sizeof(MWPALENTRY));
		if(!pimage->palette)
			goto err;
		if (fast_grayscale) {
			for (i=0; i<256; ++i) {
				MWPALENTRY pe;
				/* FIXME could use static palette here*/
				pe.r = pe.g = pe.b = i, pe._padding = 0;
				pimage->palette[i] = pe;
			}
		} else {
#if FASTJPEG
			/* FASTJPEG case only, normal uses hw palette*/
			for (i=0; i<256; ++i)
				pimage->palette[i] = mwstdpal8[i];
#endif
		}
	}

	/* Step 5: Start decompressor */
	jpeg_start_decompress (&cinfo);

	/* Step 6: while (scan lines remain to be read) */
	while(cinfo.output_scanline < cinfo.output_height) {
		JSAMPROW rowptr[1];
		rowptr[0] = (JSAMPROW)(pimage->imagebits +
			cinfo.output_scanline * pimage->pitch);
		jpeg_read_scanlines (&cinfo, rowptr, 1);
	}
	ret = 1;

err:
	/* Step 7: Finish decompression */
	jpeg_finish_decompress (&cinfo);

	/* Step 8: Release JPEG decompression object */
	jpeg_destroy_decompress (&cinfo);

	/* May want to check to see whether any corrupt-data
	 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	 */
	return ret;
}
#endif /* defined(HAVE_JPEG_SUPPORT)*/

#if defined(HAVE_PNG_SUPPORT)
#include <png.h>
/* png_jmpbuf() macro is not defined prior to libpng-1.0.6*/
#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr)	((png_ptr)->jmpbuf)
#endif
/*
 * Load a PNG file.
 * Currently for simplicity we get the PNG library to convert the file to
 * 24 bit RGB format with no alpha channel information even if we could
 * potentially store the image more efficiently by taking note of the image
 * type and depth and acting accordingly. Similarly, > 8 bits per channel,
 * gamma correction, etc. are not supported.
 */

/* This is a quick user defined function to read from the buffer instead of from the file pointer */
static void
png_read_buffer(png_structp pstruct, png_bytep pointer, png_size_t size)
{
	bread(pstruct->io_ptr, pointer, size);
}



static int
LoadPNGOnlyHW(buffer_t * src, PMWIMAGEHDR pimage)
{
	unsigned char hdr[8];//, **rows;
	png_structp state;
	png_infop pnginfo;
	png_uint_32 width, height;
	int bit_depth, colourtype;//, i;

	bseek(src, 0L, SEEK_SET);

	if(bread(src, hdr, 8) != 8) return 0;

	if(png_sig_cmp(hdr, 0, 8)) return 0;

	if(!(state = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
						NULL, NULL))) goto nomem;

	if(!(pnginfo = png_create_info_struct(state))) {
		png_destroy_read_struct(&state, NULL, NULL);
		goto nomem;
	}

	/* Set up the input function */
	png_set_read_fn(state, src, png_read_buffer);
	/* png_init_io(state, src); */

	png_set_sig_bytes(state, 8);
	png_read_info(state, pnginfo);
	png_get_IHDR(state, pnginfo, &width, &height, &bit_depth, &colourtype,
							NULL, NULL, NULL);

	pimage->width = width;
	pimage->height = height;

#if 1 /* T.J.Park Note : Our implementation needs 32 bpp with alpha channel. */
	/* if png image has alpha channel, the output image format becomes 32 bpp by png_set_expand(). */
	if(colourtype & PNG_COLOR_MASK_PALETTE)
	{ /* Indexed color */
		if((pnginfo->valid & PNG_INFO_tRNS)
		&& pnginfo->num_trans > 0)
		{
			pimage->bpp = 32; /* Include alpha channel. see png_info.trans. */
			/* In this case, we don't need to care about color keying
			   because we convert all the paletted PNG images to true color. */
		}
		else
		{
			pimage->bpp = 24;
		}
	}
	else
	{ /* True color */
		if(colourtype & PNG_COLOR_MASK_ALPHA)
		{
			pimage->bpp = 32;
			/* Because we have alpha channel, I think it will be OK even if we ignore the color key. */
		}
		else
		{
			pimage->bpp = 24;
			/* If the input image does not include alpha channel, png_info.trans_values may be useful.
			   In this case, I use png_info.trans_values for color keying. */
			/* The order of pimage->transcolor should be 0xAARRGGBB. This is not depending on the system's endian. */
			if(pnginfo->valid & PNG_INFO_tRNS)
			{ /* Though each pixel does not have alpha channel, there is a color key info in this image. */
				if(colourtype & PNG_COLOR_MASK_COLOR)
				{ /* true color(16 bit or 24 bit) */
					if(bit_depth == 16)
					{ /* Each color component is described in 16 bit,
						so pnginfo->trans_values may also have 16 bit length, but I am not sure. */
						/* Make each 8 bit RGB component, and compose transcolor from them. */
						pimage->transcolor = ((((long)(pnginfo->trans_values.red)) >> 8) << 16)
											|((((long)(pnginfo->trans_values.green)) >> 8) << 8)
											|((((long)(pnginfo->trans_values.blue)) >> 8));
					}
					else
					{
						pimage->transcolor = (((long)(pnginfo->trans_values.red)) << 16)
											|(((long)(pnginfo->trans_values.green)) << 8)
											|((long)(pnginfo->trans_values.blue));
					}

				}
				else
				{ /* gray */
					long gray;
					/* I didn't know exactly how to support the color key in gray format. */
					/* I checked png_do_gray_rgb(), and see how to control the gray files.
					   It seems that there are only 8 bit and 4 bit gray files.
					   This code does not consider 2 bit or 1 bit gray files. Am I correct??? */
					switch(bit_depth)
					{
						case 16:
							gray = pnginfo->trans_values.gray >> 8;
							break;
						case 8:
							gray = pnginfo->trans_values.gray & 0x00FF;
							break;
						case 4:
						default:
							gray = pnginfo->trans_values.gray & 0x000F;
							gray |= (gray << 4);
							break;
					}
					pimage->transcolor = (gray) | (gray << 8) | (gray << 16);
				}
			}
		}
	}
#else /* Original MicroWindows has converted all formats to 24 bpp. */
	pimage->bpp = 24;
#endif
#if 1
	// TODO: support HW decoding for other format!
	if(pimage->bpp != 32)
	{
		// Currently, Only 32BPP format is supported!
		png_destroy_read_struct(&state, &pnginfo, NULL);
		goto decode_fail;
	}
#endif
	pimage->planes = 1;
	ComputePitch(pimage->bpp, pimage->width, &pimage->pitch,
						&pimage->bytesperpixel);
	/* PNG's output is always RGB(or ARGB) in byte-order because we call png_set_expand() and png_set_gray_to_rgb(). */
	pimage->compression = MWIMAGE_RGB;//MWIMAGE_ALPHA_CHANNEL  MWIMAGE_RGB

	if(!(pimage->imagebits = NANOX_DecodePNGImgByHW(src->start, src->size, pimage->width, pimage->height)))
	{
		//dbgprint("^y^ NANOX_DecodePNGImgByHW() Failed pimage->imagebits : %d \n",pimage->imagebits);
		png_destroy_read_struct(&state, &pnginfo, NULL);
		goto decode_fail;
	}

	//Format of decoded buffer (pimage->imagebits) is the same to ARGB8888.
	//Note that this depends on how to implement API related to HW decoding.
	pimage->bpp=32;

	png_destroy_read_struct(&state, &pnginfo, NULL);
	return 1;


nomem:
	EPRINTF("LoadPNGOnlyHW: Out of memory\n");
	return 2;

decode_fail:
	EPRINTF("LoadPNGOnlyHW: HW Decoding for PNG Failed!!!\n");
	return 3;

}

static int
LoadPNG(buffer_t * src, PMWIMAGEHDR pimage)
{
	unsigned char hdr[8], **rows;
	png_structp state;
	png_infop pnginfo;
	png_uint_32 width, height;
	int bit_depth, colourtype, i;

	bseek(src, 0L, SEEK_SET);

	if(bread(src, hdr, 8) != 8) return 0;

	if(png_sig_cmp(hdr, 0, 8)) return 0;

	if(!(state = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
						NULL, NULL))) goto nomem;

	if(!(pnginfo = png_create_info_struct(state))) {
		png_destroy_read_struct(&state, NULL, NULL);
		goto nomem;
	}

	/* Set up the input function */
	png_set_read_fn(state, src, png_read_buffer);
	/* png_init_io(state, src); */

	png_set_sig_bytes(state, 8);
	png_read_info(state, pnginfo);
	png_get_IHDR(state, pnginfo, &width, &height, &bit_depth, &colourtype,
							NULL, NULL, NULL);

	pimage->width = width;
	pimage->height = height;

#if 1 /* T.J.Park Note : Our implementation needs 32 bpp with alpha channel. */
	/* if png image has alpha channel, the output image format becomes 32 bpp by png_set_expand(). */
	if(colourtype & PNG_COLOR_MASK_PALETTE)
	{ /* Indexed color */
		if((pnginfo->valid & PNG_INFO_tRNS)
		&& pnginfo->num_trans > 0)
		{
			pimage->bpp = 32; /* Include alpha channel. see png_info.trans. */
			/* In this case, we don't need to care about color keying
			   because we convert all the paletted PNG images to true color. */
		}
		else
		{
			pimage->bpp = 24;
		}
	}
	else
	{ /* True color */
		if(colourtype & PNG_COLOR_MASK_ALPHA)
		{
			pimage->bpp = 32;
			/* Because we have alpha channel, I think it will be OK even if we ignore the color key. */
		}
		else
		{
			pimage->bpp = 24;
			/* If the input image does not include alpha channel, png_info.trans_values may be useful.
			   In this case, I use png_info.trans_values for color keying. */
			/* The order of pimage->transcolor should be 0xAARRGGBB. This is not depending on the system's endian. */
			if(pnginfo->valid & PNG_INFO_tRNS)
			{ /* Though each pixel does not have alpha channel, there is a color key info in this image. */
				if(colourtype & PNG_COLOR_MASK_COLOR)
				{ /* true color(16 bit or 24 bit) */
					if(bit_depth == 16)
					{ /* Each color component is described in 16 bit,
						so pnginfo->trans_values may also have 16 bit length, but I am not sure. */
						/* Make each 8 bit RGB component, and compose transcolor from them. */
						pimage->transcolor = ((((long)(pnginfo->trans_values.red)) >> 8) << 16)
											|((((long)(pnginfo->trans_values.green)) >> 8) << 8)
											|((((long)(pnginfo->trans_values.blue)) >> 8));
					}
					else
					{
						pimage->transcolor = (((long)(pnginfo->trans_values.red)) << 16)
											|(((long)(pnginfo->trans_values.green)) << 8)
											|((long)(pnginfo->trans_values.blue));
					}

				}
				else
				{ /* gray */
					long gray;
					/* I didn't know exactly how to support the color key in gray format. */
					/* I checked png_do_gray_rgb(), and see how to control the gray files.
					   It seems that there are only 8 bit and 4 bit gray files.
					   This code does not consider 2 bit or 1 bit gray files. Am I correct??? */
					switch(bit_depth)
					{
						case 16:
							gray = pnginfo->trans_values.gray >> 8;
							break;
						case 8:
							gray = pnginfo->trans_values.gray & 0x00FF;
							break;
						case 4:
						default:
							gray = pnginfo->trans_values.gray & 0x000F;
							gray |= (gray << 4);
							break;
					}
					pimage->transcolor = (gray) | (gray << 8) | (gray << 16);
				}
			}
		}
	}
#else /* Original MicroWindows has converted all formats to 24 bpp. */
	pimage->bpp = 24;
#endif

	pimage->planes = 1;
	ComputePitch(pimage->bpp, pimage->width, &pimage->pitch,
						&pimage->bytesperpixel);

	/* PNG's output is always RGB(or ARGB) in byte-order because we call png_set_expand() and png_set_gray_to_rgb(). */
	pimage->compression = MWIMAGE_RGB;

#if 0
    if(!(pimage->imagebits = NANOX_MALLOC(pimage->pitch * pimage->height)))
	{
		png_destroy_read_struct(&state, &pnginfo, NULL);
		goto nomem;
    }
#else
    if(!(pimage->imagebits = NANOX_ScrAllocateFrame(pimage->bpp, pimage->width, pimage->height, pimage->pitch * pimage->height, &pimage->pitch)))
	{
		png_destroy_read_struct(&state, &pnginfo, NULL);
		goto nomem;
    }
#endif

	if(!(rows = NANOX_MALLOC(pimage->height * sizeof(unsigned char *))))
	{
		png_destroy_read_struct(&state, &pnginfo, NULL);
		goto nomem;
    }

	if(setjmp(png_jmpbuf(state))) {
		NANOX_FREE(rows);
		png_destroy_read_struct(&state, &pnginfo, NULL);
		return 2;
	}

	for(i = 0; i < pimage->height; i++)
		rows[i] = pimage->imagebits + (i * pimage->pitch);

	png_set_expand(state);
	if(bit_depth == 16)
		png_set_strip_16(state);

#if 1
	if(pimage->bpp == 32)
		png_set_swap_alpha(state);	/* re-arrange into 0xAARRGGBB. If we do not, the order is 0xRRGGBBAA */
	else
		png_set_strip_alpha(state);
#else
	if(colourtype & PNG_COLOR_MASK_ALPHA)
		png_set_strip_alpha(state); /* original MW ignored alpha channel */
#endif

	if(colourtype == PNG_COLOR_TYPE_GRAY ||	colourtype == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(state);

	/* Now let the PNG decoder make the output */
	png_read_image(state, rows);

	png_read_end(state, NULL);
	/* OK. Done. */

	NANOX_FREE(rows);
	png_destroy_read_struct(&state, &pnginfo, NULL);

	return 1;

nomem:
	EPRINTF("LoadPNG: Out of memory\n");
	return 2;
}
#endif /* defined(HAVE_PNG_SUPPORT)*/

#if defined(HAVE_LGI_SUPPORT)
#include "lgi.h"

static int
LoadLGI(buffer_t * src, PMWIMAGEHDR pimage)
{
	LGI_T lgi;

	if(!LGI_PrepareDecoding(&lgi, src->start, src->size))
		return 0;

	memset(pimage, 0, sizeof(MWIMAGEHDR));

	pimage->bpp = lgi.bits_per_pixel_org;
	pimage->bytesperpixel = lgi.bytes_per_pixel_org;
	pimage->compression = MWIMAGE_RGB;
	pimage->planes = 1;
	pimage->width = lgi.width;
	pimage->height = lgi.height;
	pimage->pitch = pimage->width * pimage->bytesperpixel;
	pimage->transcolor = -1;

	if(lgi.format & LGIF_INDEXED)
	{
		int i;
		unsigned char *pPalette;

		if(lgi.palette_count == 0 || lgi.palette_block_size == 0)
			goto err;

		pimage->palsize = lgi.palette_count;
		pimage->palette = NANOX_MALLOC(256 * sizeof(MWPALENTRY));
		if(pimage->palette == NULL)
			goto err;

		pPalette = lgi.palette_entries;
		switch(lgi.format & 0x7F)
		{
			case LGIF_ARGB8888:
			case LGIF_RGB0888:
				for(i = 0;i < pimage->palsize;i++)
				{
					pimage->palette[i]._padding = *pPalette++;
					pimage->palette[i].r = *pPalette++;
					pimage->palette[i].g = *pPalette++;
					pimage->palette[i].b = *pPalette++;
				}
				break;
			case LGIF_RGB888:
				for(i = 0;i < pimage->palsize;i++)
				{
					pimage->palette[i]._padding = 0;
					pimage->palette[i].r = *pPalette++;
					pimage->palette[i].g = *pPalette++;
					pimage->palette[i].b = *pPalette++;
				}
				break;
			case LGIF_ARGB4444:
				for(i = 0;i < pimage->palsize;i++)
				{
					pimage->palette[i]._padding = ((*pPalette) & 0xF0) >> 4;
					pimage->palette[i].r = (*pPalette++) & 0x0F;
					pimage->palette[i].g = ((*pPalette) & 0xF0) >> 4;
					pimage->palette[i].b = (*pPalette++) & 0x0F;
				}
				break;
			case LGIF_W_RGB555:
				for(i = 0;i < pimage->palsize;i++)
				{
					pimage->palette[i]._padding = (pPalette[0] & 0x80) >> 7;
					pimage->palette[i].r = (pPalette[0] & 0x7C) << 1;
					pimage->palette[i].g = ((pPalette[0] & 0x03) << 6) | ((pPalette[1] & 0xFE) >> 2);
					pimage->palette[i].b = (pPalette[1] & 0x1F) << 3;
					pPalette += 2;
				}
				break;
			case LGIF_RGB555:
				for(i = 0;i < pimage->palsize;i++)
				{
					pimage->palette[i]._padding = 0;
					pimage->palette[i].r = (pPalette[0] & 0x7C) << 1;
					pimage->palette[i].g = ((pPalette[0] & 0x03) << 6) | ((pPalette[1] & 0xFE) >> 2);
					pimage->palette[i].b = (pPalette[1] & 0x1F) << 3;
					pPalette += 2;
				}
				break;
			case LGIF_RGB565:
				for(i = 0;i < pimage->palsize;i++)
				{
					pimage->palette[i]._padding = 0;
					pimage->palette[i].r = pPalette[0] & 0xF8;
					pimage->palette[i].g = ((pPalette[0] & 0x07) << 5) | ((pPalette[1] & 0xE0) >> 3);
					pimage->palette[i].b = (pPalette[1] & 0x1F) << 3;
					pPalette += 2;
				}
				break;
			default:
				goto err;
		}
	}

#if 0
	pimage->imagebits = NANOX_MALLOC(pimage->pitch * pimage->height);
#else
	pimage->imagebits = NANOX_ScrAllocateFrame(pimage->bpp, pimage->width, pimage->height, pimage->pitch * pimage->height, &pimage->pitch);
#endif

	if(!LGI_DoDecoding(&lgi, pimage->imagebits, pimage->pitch * pimage->height))
		goto err;

	return 1;
err:
	if(pimage->palette)
		NANOX_FREE(pimage->palette);
	if(pimage->imagebits) {
#if 0
		NANOX_FREE(pimage->imagebits);
#else
		NANOX_ScrFreeFrame(pimage->imagebits);
#endif
	}
	return 2;
}
#endif

#if defined(HAVE_BMP_SUPPORT)
/* BMP stuff*/
#define BI_RGB		0L
#define BI_RLE8		1L
#define BI_RLE4		2L
#define BI_BITFIELDS	3L

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef long		LONG;

typedef struct {
	/* BITMAPFILEHEADER*/
	BYTE	bfType[2];
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
} BMPFILEHEAD;

#define FILEHEADSIZE 14

/* windows style*/
typedef struct {
	/* BITMAPINFOHEADER*/
	DWORD	BiSize;
	DWORD	BiWidth;
	DWORD	BiHeight;
	WORD	BiPlanes;
	WORD	BiBitCount;
	DWORD	BiCompression;
	DWORD	BiSizeImage;
	DWORD	BiXpelsPerMeter;
	DWORD	BiYpelsPerMeter;
	DWORD	BiClrUsed;
	DWORD	BiClrImportant;
} BMPINFOHEAD;

#define INFOHEADSIZE 40

/* os/2 style*/
typedef struct {
	/* BITMAPCOREHEADER*/
	DWORD	bcSize;
	WORD	bcWidth;
	WORD	bcHeight;
	WORD	bcPlanes;
	WORD	bcBitCount;
} BMPCOREHEAD;

#define COREHEADSIZE 12

static int	DecodeRLE8(MWUCHAR *buf, buffer_t *src);
static int	DecodeRLE4(MWUCHAR *buf, buffer_t *src);
static void	put4(int b);

/*
 * BMP decoding routine
 */

/* Changed by JHC to allow a buffer instead of a filename */

static int
LoadBMP(buffer_t *src, PMWIMAGEHDR pimage)
{
	int		h, i, compression;
	int		headsize;
	MWUCHAR		*imagebits;
	BMPFILEHEAD	bmpf;
	BMPINFOHEAD	bmpi;
	BMPCOREHEAD	bmpc;
	MWUCHAR 	headbuffer[INFOHEADSIZE];

	bseek(src, 0, SEEK_SET);

	pimage->imagebits = NULL;
	pimage->palette = NULL;

	/* read BMP file header*/
	if (bread(src, &headbuffer, FILEHEADSIZE) != FILEHEADSIZE)
	  return(0);

	bmpf.bfType[0] = headbuffer[0];
	bmpf.bfType[1] = headbuffer[1];

	/* Is it really a bmp file ? */
	if (*(WORD*)&bmpf.bfType[0] != wswap(0x4D42)) /* 'BM' */
		return 0;	/* not bmp image*/

	/*bmpf.bfSize = dwswap(dwread(&headbuffer[2]));*/
	bmpf.bfOffBits = dwswap(dwread(&headbuffer[10]));

	/* Read remaining header size */
	if (bread(src,&headsize,sizeof(DWORD)) != sizeof(DWORD))
		return 0;	/* not bmp image*/
	headsize = dwswap(headsize);

	/* might be windows or os/2 header */
	if(headsize == COREHEADSIZE) {

		/* read os/2 header */
		if(bread(src, &headbuffer, COREHEADSIZE-sizeof(DWORD)) !=
			COREHEADSIZE-sizeof(DWORD))
				return 0;	/* not bmp image*/

		/* Get data */
		bmpc.bcWidth = wswap(*(WORD*)&headbuffer[0]);
		bmpc.bcHeight = wswap(*(WORD*)&headbuffer[2]);
		bmpc.bcPlanes = wswap(*(WORD*)&headbuffer[4]);
		bmpc.bcBitCount = wswap(*(WORD*)&headbuffer[6]);

		pimage->width = (int)bmpc.bcWidth;
		pimage->height = (int)bmpc.bcHeight;
		pimage->bpp = bmpc.bcBitCount;
		if (pimage->bpp <= 8)
			pimage->palsize = 1 << pimage->bpp;
		else pimage->palsize = 0;
		compression = BI_RGB;
	} else {
		/* read windows header */
		if(bread(src, &headbuffer, INFOHEADSIZE-sizeof(DWORD)) !=
			INFOHEADSIZE-sizeof(DWORD))
				return 0;	/* not bmp image*/

		/* Get data */
		bmpi.BiWidth = dwswap(*(DWORD*)&headbuffer[0]);
		bmpi.BiHeight = dwswap(*(DWORD*)&headbuffer[4]);
		bmpi.BiPlanes = wswap(*(WORD*)&headbuffer[8]);
		bmpi.BiBitCount = wswap(*(WORD*)&headbuffer[10]);
		bmpi.BiCompression = dwswap(*(DWORD*)&headbuffer[12]);
		bmpi.BiSizeImage = dwswap(*(DWORD*)&headbuffer[16]);
		bmpi.BiXpelsPerMeter = dwswap(*(DWORD*)&headbuffer[20]);
		bmpi.BiYpelsPerMeter = dwswap(*(DWORD*)&headbuffer[24]);
		bmpi.BiClrUsed = dwswap(*(DWORD*)&headbuffer[28]);
		bmpi.BiClrImportant = dwswap(*(DWORD*)&headbuffer[32]);

		pimage->width = (int)bmpi.BiWidth;
		pimage->height = (int)bmpi.BiHeight;
		pimage->bpp = bmpi.BiBitCount;
		pimage->palsize = (int)bmpi.BiClrUsed;
		if (pimage->palsize > 256)
			pimage->palsize = 0;
		else if(pimage->palsize == 0 && pimage->bpp <= 8)
			pimage->palsize = 1 << pimage->bpp;
		compression = bmpi.BiCompression;
	}
	pimage->compression = MWIMAGE_BGR;	/* right side up, BGR order*/
	pimage->planes = 1;

	/* currently only 1, 4, 8 and 24 bpp bitmaps*/
	if(pimage->bpp > 8 && pimage->bpp != 24) {
		EPRINTF("LoadBMP: image bpp not 1, 4, 8 or 24\n");
		return 2;	/* image loading error*/
	}

	/* compute byte line size and bytes per pixel*/
	ComputePitch(pimage->bpp, pimage->width, &pimage->pitch,
		&pimage->bytesperpixel);

	/* Allocate image */
#if 0
	if( (pimage->imagebits = NANOX_MALLOC(pimage->pitch*pimage->height)) == NULL)
		goto err;
#else
	if( (pimage->imagebits = NANOX_ScrAllocateFrame(pimage->bpp, pimage->width, pimage->height,pimage->pitch*pimage->height, &pimage->pitch)) == NULL)
		goto err;
#endif
	if( (pimage->palette = NANOX_MALLOC(256*sizeof(MWPALENTRY))) == NULL)
		goto err;

	/* get colormap*/
	if(pimage->bpp <= 8) {
		for(i=0; i<pimage->palsize; i++) {
			pimage->palette[i].b = bgetc(src);
			pimage->palette[i].g = bgetc(src);
			pimage->palette[i].r = bgetc(src);
			if(headsize != COREHEADSIZE)
				bgetc(src);
		}
	}

	/* decode image data*/
	bseek(src, bmpf.bfOffBits, SEEK_SET);

	h = pimage->height;
	/* For every row ... */
	while (--h >= 0) {
		/* turn image rightside up*/
		imagebits = pimage->imagebits + h*pimage->pitch;

		/* Get row data from file */
		if(compression == BI_RLE8) {
			if(!DecodeRLE8(imagebits, src))
				break;
		} else if(compression == BI_RLE4) {
			if(!DecodeRLE4(imagebits, src))
				break;
		} else {
			if(bread(src, imagebits, pimage->pitch) !=
				pimage->pitch)
					goto err;
		}
	}
	return 1;		/* bmp image ok*/

err:
	EPRINTF("LoadBMP: image loading error\n");
	if(pimage->imagebits) {
#if 0
		NANOX_FREE(pimage->imagebits);
#else
		NANOX_ScrFreeFrame(pimage->imagebits);
#endif
	}
	if(pimage->palette)
		NANOX_FREE(pimage->palette);
	return 2;		/* bmp image error*/
}

/*
 * Decode one line of RLE8, return 0 when done with all bitmap data
 */
static int
DecodeRLE8(MWUCHAR *buf, buffer_t *src)
{
	int		c, n;
	MWUCHAR *	p = buf;

	for( ;;) {
	  switch( n = bgetc(src)) {
	  case EOF:
	    return( 0);
	  case 0:			/* 0 = escape*/
	    switch( n = bgetc(src)) {
	    case 0: 	/* 0 0 = end of current scan line*/
	      return( 1);
	    case 1:		/* 0 1 = end of data*/
	      return( 1);
	    case 2:		/* 0 2 xx yy delta mode NOT SUPPORTED*/
	      (void)bgetc(src);
	      (void)bgetc(src);
	      continue;
	    default:	/* 0 3..255 xx nn uncompressed data*/
	      for( c=0; c<n; c++)
		*p++ = bgetc(src);
	      if( n & 1)
		(void)bgetc(src);
	      continue;
	    }
	  default:
	    c = bgetc(src);
	    while( n--)
	      *p++ = c;
	    continue;
	  }
	}
}

/*
 * Decode one line of RLE4, return 0 when done with all bitmap data
 */
static MWUCHAR *p;
static int	once;

static void
put4(int b)
{
	static int	last;

	last = (last << 4) | b;
	if( ++once == 2) {
		*p++ = last;
		once = 0;
	}
}

static int
DecodeRLE4(MWUCHAR *buf, buffer_t *src)
{
	int		c, n, c1, c2;

	p = buf;
	once = 0;
	c1 = 0;

	for( ;;) {
	  switch( n = bgetc(src)) {
	  case EOF:
	    return( 0);
	  case 0:			/* 0 = escape*/
	    switch( n = bgetc(src)) {
	    case 0: 	/* 0 0 = end of current scan line*/
	      if( once)
		put4( 0);
	      return( 1);
	    case 1:		/* 0 1 = end of data*/
	      if( once)
		put4( 0);
	      return( 1);
	    case 2:		/* 0 2 xx yy delta mode NOT SUPPORTED*/
	      (void)bgetc(src);
	      (void)bgetc(src);
	      continue;
	    default:	/* 0 3..255 xx nn uncompressed data*/
	      c2 = (n+3) & ~3;
	      for( c=0; c<c2; c++) {
		if( (c & 1) == 0)
		  c1 = bgetc(src);
		if( c < n)
		  put4( (c1 >> 4) & 0x0f);
		c1 <<= 4;
	      }
	      continue;
	    }
	  default:
	    c = bgetc(src);
	    c1 = (c >> 4) & 0x0f;
	    c2 = c & 0x0f;
	    for( c=0; c<n; c++)
	      put4( (c&1)? c2: c1);
	    continue;
	  }
	}
}
#endif /* defined(HAVE_BMP_SUPPORT)*/

#if 0
void print_image(PMWIMAGEHDR image)
{
	int i;

	DPRINTF("Image:\n\n");
	DPRINTF("height: %d\n", image->height);
	DPRINTF("width: %d\n", image->width);
	DPRINTF("planes: %d\n", image->planes);
	DPRINTF("bpp: %d\n", image->bpp);
	DPRINTF("compression: %d\n", image->compression);
	DPRINTF("palsize: %d\n", image->palsize);

	for (i=0;i<image->palsize;i++)
		DPRINTF("palette: %d, %d, %d\n", image->palette[i].r,
			image->palette[i].g, image->palette[i].b);

	for(i=0;i<(image->width*image->height);i++)
		DPRINTF("imagebits: %d\n", image->imagebits[i]);
}
#endif

#if defined(HAVE_GIF_SUPPORT)
/* Code for GIF decoding has been adapted from XPaint:                   */
/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, 1991, 1993 David Koblas.			       | */
/* | Copyright 1996 Torsten Martinsen.				       | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.	       | */
/* +-------------------------------------------------------------------+ */
/* Portions Copyright (C) 1999  Sam Lantinga*/
/* Adapted for use in SDL by Sam Lantinga -- 7/20/98 */
/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* GIF stuff*/
/*
 * GIF decoding routine
 */
#define	MAXCOLORMAPSIZE		256
#define	MAX_LWZ_BITS		12
#define INTERLACE		0x40
#define LOCALCOLORMAP		0x80

#define CM_RED		0
#define CM_GREEN	1
#define CM_BLUE		2

#define BitSet(byte, bit)	(((byte) & (bit)) == (bit))
#define	ReadOK(src,buffer,len)	bread(src, buffer, len)
#define LM_to_uint(a,b)		(((b)<<8)|(a))

struct {
    unsigned int Width;
    unsigned int Height;
    unsigned char ColorMap[3][MAXCOLORMAPSIZE];
    unsigned int BitPixel;
    unsigned int ColorResolution;
    unsigned int Background;
    unsigned int AspectRatio;
    int GrayScale;
} GifScreen;

static struct {
    int transparent;
    int delayTime;
    int inputFlag;
    int disposal;
} Gif89;

static int ReadColorMap(buffer_t *src, int number,
		unsigned char buffer[3][MAXCOLORMAPSIZE], int *flag);
static int DoExtension(buffer_t *src, int label);
static int GetDataBlock(buffer_t *src, unsigned char *buf);
static int GetCode(buffer_t *src, int code_size, int flag);
static int LWZReadByte(buffer_t *src, int flag, int input_code_size);
static PMWIMAGEHDR ReadImage(buffer_t* src
		  , int x, int y, int width, int height
		  , int cmapSize, unsigned char cmap[3][MAXCOLORMAPSIZE]
		  , int gray, int interlace, int ignore);


static int
IsGIF(buffer_t *src)
{
	unsigned char buf[6];
	unsigned char *temp;

    bseek(src, 0, SEEK_SET);
	temp = buf;

    if (!ReadOK(src, temp, 6))
        return 0;		/* not gif image*/
    if (strncmp((char *) temp, "GIF", 3) != 0)
        return 0;
	temp += 3;
    if (strncmp(temp, "87a", 3) != 0 && strncmp(temp, "89a", 3) != 0) {
		EPRINTF("IsGIF: GIF version number not 87a or 89a\n");
        return 0;		/* image loading error*/
    }

	return 1; /* OK. This is a GIF 87a or 89a image. */
}

static int
LoadGIFEx(buffer_t *src, PMWIMAGEHDR *ppimages, int maxImageCount)
{
	PMWIMAGEHDR pimage;

    unsigned char buf[32];
    unsigned char c;
    unsigned char localColorMap[3][MAXCOLORMAPSIZE];
    int grayScale;
    int useGlobalColormap;
    int bitPixel;
    int imageCount;
    char version[4];

    bseek(src, 0, SEEK_SET);

    if (!ReadOK(src, buf, 6))
        return 0;		/* not gif image*/
    if (strncmp((char *) buf, "GIF", 3) != 0)
        return 0;
    strncpy(version, (char *) buf + 3, 3);
    version[3] = '\0';

    if (strcmp(version, "87a") != 0 && strcmp(version, "89a") != 0)
	{
		EPRINTF("LoadGIFEx: GIF version number not 87a or 89a\n");
        return 0;		/* image loading error*/
    }

    if (!ReadOK(src, buf, 7))
	{
		EPRINTF("LoadGIFEx: bad screen descriptor\n");
        return 0;		/* image loading error*/
    }
    GifScreen.Width = LM_to_uint(buf[0], buf[1]);
    GifScreen.Height = LM_to_uint(buf[2], buf[3]);
    GifScreen.BitPixel = 2 << (buf[4] & 0x07);
    GifScreen.ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
    GifScreen.Background = buf[5];
    GifScreen.AspectRatio = buf[6];

    if (BitSet(buf[4], LOCALCOLORMAP))
	{ /* Global Colormap */
		if (ReadColorMap(src, GifScreen.BitPixel, GifScreen.ColorMap,
				 &GifScreen.GrayScale))
		{
			EPRINTF("LoadGIFEx: bad global colormap\n");
			return 0;		/* image loading error*/
		}
    }

	/* Initialize the Gif89 extension information. This info may exist per image */
	Gif89.transparent = -1;
	Gif89.delayTime = -1;
	Gif89.inputFlag = -1;
	Gif89.disposal = 0;

	imageCount = 0;
    while(1)
	{
		if (!ReadOK(src, &c, 1))
		{
			EPRINTF("LoadGIFEx: EOF on image data\n");
			break;
		}

		if (c == ';')
		{ /* GIF terminator */
			if (imageCount < 1)
			{
				EPRINTF("LoadGIFEx: no image\n");
				break;
			}
		}

		if (c == '!')
		{ /* Extension */
			if (!ReadOK(src, &c, 1))
			{
				EPRINTF("LoadGIF: EOF on extension function code\n");
				break;
			}
			DoExtension(src, c);
			/* Now we have a GIF89 info */
			continue;
		}

		if (c != ',')
		{ /* Not a valid start character */
			continue;
		}

		if (!ReadOK(src, buf, 9))
		{
			EPRINTF("LoadGIF: bad image size\n");
			break;
		}
		/* Now we have an image descriptor */

		useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);

		bitPixel = 1 << ((buf[8] & 0x07) + 1);

		if (!useGlobalColormap)
		{
			if (ReadColorMap(src, bitPixel, localColorMap, &grayScale))
			{
				EPRINTF("LoadGIF: bad local colormap\n");
				break;
			}
			pimage = ReadImage(src
			                 , LM_to_uint(buf[0], buf[1]) /* x-offset */
							 , LM_to_uint(buf[2], buf[3]) /* y-offset */
			                 , LM_to_uint(buf[4], buf[5]) /* width */
							 , LM_to_uint(buf[6], buf[7]) /* height */
							 , bitPixel
							 , localColorMap
							 , grayScale
							 , BitSet(buf[8], INTERLACE)
							 , 0);
		}
		else
		{
			pimage = ReadImage(src
			                 , LM_to_uint(buf[0], buf[1]) /* x-offset */
							 , LM_to_uint(buf[2], buf[3]) /* y-offset */
							 , LM_to_uint(buf[4], buf[5]) /* width */
							 , LM_to_uint(buf[6], buf[7]) /* height */
							 , GifScreen.BitPixel
							 , GifScreen.ColorMap
							 , GifScreen.GrayScale
							 , BitSet(buf[8], INTERLACE)
							 , 0);
		}

		if(pimage == NULL)
			break;

		/* Add it to the image list */
		ppimages[imageCount++] = pimage;
		if(imageCount >= maxImageCount)
			break;
    }

	return imageCount;
}

static int
ReadColorMap(buffer_t *src, int number, unsigned char buffer[3][MAXCOLORMAPSIZE],
    int *gray)
{
    int i;
    unsigned char rgb[3];
    int flag;

    flag = TRUE;

    for (i = 0; i < number; ++i) {
	if (!ReadOK(src, rgb, sizeof(rgb)))
	    return 1;
	buffer[CM_RED][i] = rgb[0];
	buffer[CM_GREEN][i] = rgb[1];
	buffer[CM_BLUE][i] = rgb[2];
	flag &= (rgb[0] == rgb[1] && rgb[1] == rgb[2]);
    }

#if 0
    if (flag)
	*gray = (number == 2) ? PBM_TYPE : PGM_TYPE;
    else
	*gray = PPM_TYPE;
#else
    *gray = 0;
#endif

    return FALSE;
}

static int
DoExtension(buffer_t *src, int label)
{
    static unsigned char buf[256];

    switch (label) {
    case 0x01:			/* Plain Text Extension */
	break;
    case 0xff:			/* Application Extension */
	break;
    case 0xfe:			/* Comment Extension */
	while (GetDataBlock(src, (unsigned char *) buf) != 0);
	return FALSE;
    case 0xf9:			/* Graphic Control Extension */
	GetDataBlock(src, (unsigned char *) buf);
	Gif89.disposal = (buf[0] >> 2) & 0x7;
	Gif89.inputFlag = (buf[0] >> 1) & 0x1;
	Gif89.delayTime = LM_to_uint(buf[1], buf[2]);
	if ((buf[0] & 0x1) != 0)
	    Gif89.transparent = buf[3];

	while (GetDataBlock(src, (unsigned char *) buf) != 0);
	return FALSE;
    default:
	break;
    }

    while (GetDataBlock(src, (unsigned char *) buf) != 0);

    return FALSE;
}

static int ZeroDataBlock = FALSE;

static int
GetDataBlock(buffer_t *src, unsigned char *buf)
{
    unsigned char count;

    if (!ReadOK(src, &count, 1))
	return -1;
    ZeroDataBlock = count == 0;

    if ((count != 0) && (!ReadOK(src, buf, count)))
	return -1;
    return count;
}

static int
GetCode(buffer_t *src, int code_size, int flag)
{
    static unsigned char buf[280];
    static int curbit, lastbit, done, last_byte;
    int i, j, ret;
    unsigned char count;

    if (flag) {
	curbit = 0;
	lastbit = 0;
	done = FALSE;
	return 0;
    }
    if ((curbit + code_size) >= lastbit) {
	if (done) {
	    if (curbit >= lastbit)
		EPRINTF("LoadGIF: bad decode\n");
	    return -1;
	}
	buf[0] = buf[last_byte - 2];
	buf[1] = buf[last_byte - 1];

	if ((count = GetDataBlock(src, &buf[2])) == 0)
	    done = TRUE;

	last_byte = 2 + count;
	curbit = (curbit - lastbit) + 16;
	lastbit = (2 + count) * 8;
    }
    ret = 0;
    for (i = curbit, j = 0; j < code_size; ++i, ++j)
	ret |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}

static int
LWZReadByte(buffer_t *src, int flag, int input_code_size)
{
    int code, incode;
    register int i;
    static int fresh = FALSE;
    static int code_size, set_code_size;
    static int max_code, max_code_size;
    static int firstcode, oldcode;
    static int clear_code, end_code;
    static int table[2][(1 << MAX_LWZ_BITS)];
    static int stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;

    if (flag) {
	set_code_size = input_code_size;
	code_size = set_code_size + 1;
	clear_code = 1 << set_code_size;
	end_code = clear_code + 1;
	max_code_size = 2 * clear_code;
	max_code = clear_code + 2;

	GetCode(src, 0, TRUE);

	fresh = TRUE;

	for (i = 0; i < clear_code; ++i) {
	    table[0][i] = 0;
	    table[1][i] = i;
	}
	for (; i < (1 << MAX_LWZ_BITS); ++i)
	    table[0][i] = table[1][0] = 0;

	sp = stack;

	return 0;
    } else if (fresh) {
	fresh = FALSE;
	do {
	    firstcode = oldcode = GetCode(src, code_size, FALSE);
	} while (firstcode == clear_code);
	return firstcode;
    }
    if (sp > stack)
	return *--sp;

    while ((code = GetCode(src, code_size, FALSE)) >= 0) {
	if (code == clear_code) {
	    for (i = 0; i < clear_code; ++i) {
		table[0][i] = 0;
		table[1][i] = i;
	    }
	    for (; i < (1 << MAX_LWZ_BITS); ++i)
		table[0][i] = table[1][i] = 0;
	    code_size = set_code_size + 1;
	    max_code_size = 2 * clear_code;
	    max_code = clear_code + 2;
	    sp = stack;
	    firstcode = oldcode = GetCode(src, code_size, FALSE);
	    return firstcode;
	} else if (code == end_code) {
	    int count;
	    unsigned char buf[260];

	    if (ZeroDataBlock)
		return -2;

	    while ((count = GetDataBlock(src, buf)) > 0);

	    if (count != 0) {
		/*
		 * EPRINTF("missing EOD in data stream (common occurence)");
		 */
	    }
	    return -2;
	}
	incode = code;

	if (code >= max_code) {
	    *sp++ = firstcode;
	    code = oldcode;
	}
	while (code >= clear_code) {
	    *sp++ = table[1][code];
	    if (code == table[0][code])
		EPRINTF("LoadGIF: circular table entry\n");
	    code = table[0][code];
	}

	*sp++ = firstcode = table[1][code];

	if ((code = max_code) < (1 << MAX_LWZ_BITS)) {
	    table[0][code] = oldcode;
	    table[1][code] = firstcode;
	    ++max_code;
	    if ((max_code >= max_code_size) &&
		(max_code_size < (1 << MAX_LWZ_BITS))) {
		max_code_size *= 2;
		++code_size;
	    }
	}
	oldcode = incode;

	if (sp > stack)
	    return *--sp;
    }
    return code;
}

static PMWIMAGEHDR
ReadImage(buffer_t* src
		  , int x, int y, int width, int height
		  , int cmapSize, unsigned char cmap[3][MAXCOLORMAPSIZE]
		  , int gray, int interlace, int ignore)
{
	PMWIMAGEHDR pimage;
    unsigned char c;
    int i, v;
    int xpos = 0, ypos = 0, pass = 0;

    /*
     *	Initialize the compression routines
     */
    if (!ReadOK(src, &c, 1))
	{
		EPRINTF("LoadGIF: EOF on image data\n");
		return NULL;
    }

	if (LWZReadByte(src, TRUE, c) < 0)
	{
		EPRINTF("LoadGIF: error reading image\n");
		return NULL;
    }

    /*
     *	If this is an "uninteresting picture" ignore it.
     */
    if (ignore)
	{
		while (LWZReadByte(src, FALSE, c) >= 0);

		return NULL;
    }

	pimage = (PMWIMAGEHDR)NANOX_MALLOC(sizeof(MWIMAGEHDR));
	if(!pimage)
	{
		return NULL;
	}

    /*image = ImageNewCmap(len, height, cmapSize);*/
	pimage->log_screen_width = GifScreen.Width;
	pimage->log_screen_height = GifScreen.Height;
	pimage->xoffset = x;
	pimage->yoffset = y;
    pimage->width = width;
    pimage->height = height;
    pimage->planes = 1;
    pimage->bpp = 8;
    ComputePitch(8, width, &pimage->pitch, &pimage->bytesperpixel);
    pimage->compression = 0;
    /* set transparent color, if any*/
    pimage->transcolor = Gif89.transparent;
	pimage->delaytime = Gif89.delayTime * 10; /* 1/100 seconds ==> milliseconds */
	pimage->inputflag = Gif89.inputFlag;
	pimage->disposal = Gif89.disposal;

	pimage->palsize = cmapSize;
    pimage->palette = NANOX_MALLOC(256*sizeof(MWPALENTRY));
	if(pimage->palette == NULL)
	{
		NANOX_FREE(pimage);
	    return NULL;
	}
#if 0
	pimage->imagebits = NANOX_MALLOC(height*pimage->pitch);
#else
	pimage->imagebits = NANOX_ScrAllocateFrame(pimage->bpp, width, height, height*pimage->pitch, &pimage->pitch);
#endif
	if(pimage->imagebits == NULL)
	{
		NANOX_FREE(pimage->palette);
		NANOX_FREE(pimage);
	    return NULL;
	}


    for (i = 0; i < cmapSize; i++)
	{
	/*ImageSetCmap(image, i, cmap[CM_RED][i],
		     cmap[CM_GREEN][i], cmap[CM_BLUE][i]);*/
		pimage->palette[i].r = cmap[CM_RED][i];
		pimage->palette[i].g = cmap[CM_GREEN][i];
		pimage->palette[i].b = cmap[CM_BLUE][i];
    }

    while ((v = LWZReadByte(src, FALSE, c)) >= 0)
	{
		pimage->imagebits[ypos * pimage->pitch + xpos] = v;

		++xpos;
		if (xpos == width)
		{
			xpos = 0;
			if (interlace)
			{
				switch (pass)
				{
					case 0:
					case 1:
						ypos += 8;
						break;
					case 2:
						ypos += 4;
						break;
					case 3:
						ypos += 2;
						break;
				}

				if (ypos >= height)
				{
					++pass;
					switch (pass)
					{
						case 1:
							ypos = 4;
							break;
						case 2:
							ypos = 2;
							break;
						case 3:
							ypos = 1;
							break;
						default:
							goto fini;
					}
				}
			}
			else
			{
				++ypos;
			}
		}

		if (ypos >= height)
			break;
    }

fini:
    return pimage;
}
#endif /* defined(HAVE_GIF_SUPPORT)*/

#if defined(HAVE_PNM_SUPPORT)
enum {
	PNM_TYPE_NOTPNM,
	PNM_TYPE_PBM,
	PNM_TYPE_PGM,
	PNM_TYPE_PPM
};
static int LoadPNM(buffer_t *src, PMWIMAGEHDR pimage)
{
	char buf[256], *p;
	int type = PNM_TYPE_NOTPNM, binary = 0, gothdrs = 0, scale = 0;
	int ch, x = 0, y = 0, i, n, mask, col1, col2, col3;

	bseek(src, 0L, SEEK_SET);

	if(!bgets(src,buf, 4)) return 0;

	if(!strcmp("P1\n", buf)) type = PNM_TYPE_PBM;
	else if(!strcmp("P2\n", buf)) type = PNM_TYPE_PGM;
	else if(!strcmp("P3\n", buf)) type = PNM_TYPE_PPM;
	else if(!strcmp("P4\n", buf)) {
		type = PNM_TYPE_PBM;
		binary = 1;
	}
	else if(!strcmp("P5\n", buf)) {
		type = PNM_TYPE_PGM;
		binary = 1;
	}
	else if(!strcmp("P6\n", buf)) {
		type = PNM_TYPE_PPM;
		binary = 1;
	}

	if(type == PNM_TYPE_NOTPNM) return 0;

	n = 0;
	while((p = bgets(src, buf, 256))) {
		if(*buf == '#') continue;
		if(type == PNM_TYPE_PBM) {
			if(sscanf(buf, "%i %i", &pimage->width,
					&pimage->height) == 2) {
				pimage->bpp = 1;
				gothdrs = 1;
				if(!(pimage->palette = NANOX_MALLOC(
						sizeof(MWPALENTRY) * 2))) {
					EPRINTF("Out of memory\n");
					return 2;
				}
				pimage->palsize = 2;
				pimage->palette[0].r = 0xff;
				pimage->palette[0].g = 0xff;
				pimage->palette[0].b = 0xff;
				pimage->palette[1].r = 0;
				pimage->palette[1].g = 0;
				pimage->palette[1].b = 0;
			}
			break;
		}
		if((type == PNM_TYPE_PGM) || (type == PNM_TYPE_PPM)) {
			if(!n++) {
				if(sscanf(buf, "%i %i", &pimage->width,
					&pimage->height) != 2) break;
			} else {
				if(sscanf(buf, "%i", &i) != 1) break;
				pimage->bpp = 24;
				if(i > 255) {
					EPRINTF("LoadPNM: PPM files must be "
						"24bpp\n");
					return 2;
				}
				for(scale = 7, n = 2; scale; scale--, n *= 2)
					if(i < n) break;
				gothdrs = 1;
				break;
			}
		}
	}

	if(!gothdrs) {
		EPRINTF("LoadPNM: bad image headers\n");
		if(pimage->palette) NANOX_FREE(pimage->palette);
		return 2;
	}

	pimage->planes = 1;
	ComputePitch(pimage->bpp, pimage->width, &pimage->pitch,
						&pimage->bytesperpixel);
	pimage->compression = MWIMAGE_RGB;
#if 0
	if(!(pimage->imagebits = NANOX_MALLOC(pimage->pitch * pimage->height))) {
#else
	if(!(pimage->imagebits = NANOX_ScrAllocateFrame(pimage->bpp, pimage->width, pimage->height, pimage->pitch * pimage->height, &pimage->pitch))) {
#endif
		EPRINTF("LoadPNM: couldn't allocate memory for image\n");
		if(pimage->palette) NANOX_FREE(pimage->palette);
		return 2;
	}

	p = pimage->imagebits;

	if(type == PNM_TYPE_PBM) {
		if(binary) {
			x = 0;
			y = 0;
			while((ch = bgetc(src)) != EOF) {
				for(i = 0; i < 8; i++) {
					mask = 0x80 >> i;
					if(ch & mask) *p |= mask;
					else *p &= ~mask;
					if(++x == pimage->width) {
						if(++y == pimage->height)
							return 1;
						p = pimage->imagebits - 1 +
							(y * pimage->pitch);
						x = 0;
						break;
					}
				}
				p++;
			}
		} else {
			n = 0;
			while((ch = bgetc(src)) != EOF) {
				if(isspace(ch)) continue;
				mask = 0x80 >> n;
				if(ch == '1') *p |= mask;
				else if(ch == '0') *p &= ~mask;
				else goto baddata;
				if(++n == 8) {
					n = 0;
					p++;
				}
				if(++x == pimage->width) {
					if(++y == pimage->height)
						return 1;
					p = pimage->imagebits +
						(y * pimage->pitch);
					n = 0;
					x = 0;
				}
			}
		}
	} else {
		while(1) {
			if(type == PNM_TYPE_PGM) {
				if(binary) {
					if((ch = bgetc(src)) == EOF)
						goto baddata;
				} else {
				  /*if(fscanf(fp, "%i", &ch) != 1)*/
						goto baddata;
				}
				*p++ = ch << scale;
				*p++ = ch << scale;
				*p++ = ch << scale;
			} else {
				if(binary) {
					if(((col1 = bgetc(src)) == EOF) ||
					 	((col2 = bgetc(src)) == EOF) ||
					 	((col3 = bgetc(src)) == EOF))
						goto baddata;
				} else {
				  /*if(fscanf(fp, "%i %i %i", &col1, &col2, &col3) != 3)*/
						goto baddata;
				}
				*p++ = col1 << scale;
				*p++ = col2 << scale;
				*p++ = col3 << scale;
			}
			if(++x == pimage->width) {
				if(++y == pimage->height) return 1;
				p = pimage->imagebits + (y * pimage->pitch);
				x = 0;
			}
		}
	}

baddata:
	EPRINTF("LoadPNM: bad image data\n");
#if 0
	NANOX_FREE(pimage->imagebits);
#else
	NANOX_ScrFreeFrame(pimage->imagebits);
#endif
	if(pimage->palette) NANOX_FREE(pimage->palette);
	return 2;
}
#endif /* defined(HAVE_PNM_SUPPORT) */

#if defined(HAVE_XPM_SUPPORT)
struct xpm_cmap {
  char mapstr[3];
  long palette_entry;
  long color;
  struct xpm_cmap *next;
};


static long XPM_parse_color(char *color)
{
  /* This will parse the string into a color value of some sort */

  if (color[0] != '#')
    {
      if (!strcmp(color, "None"))
	return(-1); /* Transparent */
      else
	return(0); /* If its an X color, then we bail */
    }
  else
    {
      /* This is ugly! */

      char *sptr = color + 1;
      char rstr[5], gstr[5], bstr[5];
      long r,g,b;

      switch(strlen(sptr))
	{
	case 6:
	  return(strtol(sptr, NULL, 16));

	case 9: /* RRRGGGBBB */
	  strncpy(rstr, sptr, 3);
	  strncpy(gstr, sptr + 3, 3);
	  strncpy(bstr, sptr + 6, 3);

	  rstr[3] = 0;
	  gstr[3] = 0;
	  bstr[3] = 0;

	  r = strtol(rstr, NULL, 16) >> 4;
	  g = strtol(gstr, NULL, 16) >> 4;
	  b = strtol(bstr, NULL, 16) >> 4;

	  return( (long) ( r << 16 | g << 8 | b));

	case 12:
	  strncpy(rstr, sptr, 4);
	  strncpy(gstr, sptr + 4, 4);
	  strncpy(bstr, sptr + 8, 4);

	  rstr[4] = 0;
	  gstr[4] = 0;
	  bstr[4] = 0;

	  r = strtol(rstr, NULL, 16) >> 8;
	  g = strtol(gstr, NULL, 16) >> 8;
	  b = strtol(bstr, NULL, 16) >> 8;

	  return( (long) ( (r & 0xFF) << 16 | (g & 0xFF) << 8 | (b & 0xFF)));
	}
    }

  return(0);
}

/* A series of status indicators that let us know whats going on */
/* It could be an enum if you want */

#define LOAD_HEADER 1
#define LOAD_COLORS 2
#define LOAD_PALETTE 3
#define LOAD_PIXELS 4
#define LOAD_DONE 5

/* The magic that "should" indicate an XPM (does it really?) */
#define XPM_MAGIC "/* XPM */"
#define XPM_TRANSCOLOR 0x01000000

static int LoadXPM(buffer_t *src, PMWIMAGEHDR pimage, PSD psd)
{
  struct xpm_cmap *colorheap = 0;  /* A "heap" of color structs */
  struct xpm_cmap *colormap[256];  /* A quick hash of 256 spots for colors */

  unsigned char *imageptr = 0;

  MWSCREENINFO sinfo;

  char xline[300];
  char dline[300];

  char *c;
  int a;

  int col, row, colors, cpp;
  int in_color = 0;
  int read_xline = 0;

  int status = LOAD_HEADER;

  /* Very first thing, get the screen info */
  GdGetScreenInfo(psd, &sinfo);

  for(a = 0; a < 256; a++)
    colormap[a] = 0;

  pimage->imagebits = NULL;
  pimage->palette = NULL;

  /* Start over at the beginning with the file */
  bseek(src, 0, SEEK_SET);

  bgets(src, xline, 300);

  /* Chop the EOL */
  xline[strlen(xline) - 1] = 0;

  /* Check the magic */
  if (strncmp(xline, XPM_MAGIC, sizeof(XPM_MAGIC))) return(0);

  while(!beof(src))
    {
      /* Get the next line from the file */
      bgets(src,xline, 300);
      xline[strlen(xline) - 1] = 0;

      /* Check it out */
      if (xline[0] == '/' && xline[1] == '*') /* Comment */
	continue;

      if (xline[0] != '\"')
	continue;

      /* remove the quotes from the line */
      for(c = xline + 1, a = 0; *c != '\"' && *c != 0; c++, a++)
	dline[a] = *c;

      dline[a] = 0;

      /* Is it the header? */
      if (status == LOAD_HEADER)
	{
	  sscanf(dline, "%i %i %i %i", &col, &row, &colors, &cpp);

	  pimage->width = col;
	  pimage->height = row;
	  pimage->planes = 1;

	  if (sinfo.bpp <= 8)
	    {
	      pimage->bpp = sinfo.bpp;
	      pimage->compression = 0;
	      pimage->transcolor = -1;
	    }
	  else
	    {
	      pimage->bpp = 32;
	      pimage->transcolor = XPM_TRANSCOLOR;
	      pimage->compression = MWIMAGE_BGR;
	    }

	  pimage->palsize = colors;

	  ComputePitch(pimage->bpp, col, &pimage->pitch, &pimage->bytesperpixel);

#if 0
	  pimage->imagebits = NANOX_MALLOC(pimage->pitch * pimage->height);
#else
	  pimage->imagebits = NANOX_ScrAllocateFrame(pimage->bpp, pimage->width, pimage->height, pimage->pitch * pimage->height, &pimage->pitch);
#endif
	  imageptr = (unsigned char *) pimage->imagebits;

	  /* Allocate enough room for all the colors */
	  colorheap = (struct xpm_cmap *) NANOX_MALLOC(colors * sizeof(struct xpm_cmap));

	  /* Allocate the palette space (if required) */

	  if (sinfo.bpp <= 8)
	      pimage->palette = NANOX_MALLOC(256*sizeof(MWPALENTRY));

	  if (!colorheap)
	    {
	      EPRINTF("Couldn't allocate any memory for the colors\n");
	      return(0);
	    }

	  status = LOAD_COLORS;
	  in_color = 0;
	  continue;
	}

      /* Are we in load colors? */
      if (status == LOAD_COLORS)
	{
	  struct xpm_cmap *n;

	  char tstr[5];
	  char cstr[256];

	  unsigned char m;

	  c = dline;

	  /* Go at at least 1 charater, and then count until we have
	     two spaces in a row */

	  strncpy(tstr, c, cpp);

	  c += cpp;
	  for(; *c == '\t' || *c == ' '; c++); /* Skip over whitespace */

	  /* FIXME: We assume that a 'c' follows.  What if it doesn't? */
	  c +=2;

	  tstr[cpp] = 0;

	  /* Now we put it into the array for easy lookup   */
	  /* We base it off the first charater, even though */
	  /* there may be up to 4                           */

	  m = tstr[0];

	  if (colormap[m])
	    {
	      n = colormap[m];

	      while(n->next) n = n->next;
	      n->next = &colorheap[in_color];
	      n = n->next;
	    }
	  else
	    {
	      colormap[m] = &colorheap[in_color];
	      n = colormap[m];
	    }

	  n->next = 0;

	  /* Record the string */
	  strncpy(n->mapstr, tstr, cpp);
	  n->mapstr[cpp] = 0;

	  /* Now record the palette entry */
	  n->palette_entry = (long) in_color;

	  /* This is the color */
	  sscanf(c, "%65535s", cstr);

	  /* Turn it into a real value */
	  n->color = XPM_parse_color(cstr);

	  /* If we are in palette mode, then we need to */
	  /* load the palette (duh..) */

	  if (sinfo.bpp <= 8)
	    {
	      if (n->color == -1)
		{
		  pimage->transcolor = in_color;
		  n->color = -1;
		}

	      pimage->palette[in_color].r = (n->color >> 16) & 0xFF;
	      pimage->palette[in_color].g = (n->color >> 8) & 0xFF;
	      pimage->palette[in_color].b = n->color & 0xFF;
	    }
	  else
	    {
	      if (n->color == -1) {
		n->color = XPM_TRANSCOLOR;
	      }
	    }

	  if (++in_color == colors)
	    {
	      read_xline = 0;
	      status = LOAD_PIXELS;
	    }

	  continue;
	}

      if (status == LOAD_PIXELS)
      {
	int bytecount = 0;
	int bitcount = 0;
	long dwordcolor = 0;
	int i;
	char pxlstr[3];

	c = dline;

	while(*c)
	  {
	    unsigned char z = 0;

	    if (cpp == 1)
	      {
		z = *c;

		if (!colormap[z])
		  {
		    EPRINTF("No color entry for (%c)\n", z);
		    return(0);
		  }

		if (sinfo.bpp <= 8)
		  dwordcolor = (long) colormap[z]->palette_entry;
		else
		  dwordcolor = colormap[z]->color;

		c++;
	      }
	    else
	      {
		struct xpm_cmap *n;

		/* We grab the largest possible, and then compare */

		strncpy(pxlstr, c, cpp);
		z = pxlstr[0];

		if (!colormap[z])
		  {
		    EPRINTF("No color entry for (%s)\n", pxlstr);
		    return(0);
		  }

		n = colormap[z];

		while(n)
		  {
		    if (!strncmp(n->mapstr, pxlstr, cpp))
		      break;

		    n = n->next;
		  }

		if (!n)
		  {
		    EPRINTF("No color found for (%s)\n", pxlstr);
		    return(0);
		  }

		if (sinfo.bpp <= 8)
		  dwordcolor = (long) n->palette_entry;
		else
		  dwordcolor = n->color;
		c += cpp;
	      }

	    /*
	     * This ugly thing is needed to ensure that we
	     * work well in all modes.
	     */
	    switch(sinfo.bpp)
	      {
	      case 2:
		if (bitcount == 0)
		  imageptr[0] = 0;

		imageptr[0] |= (dwordcolor & 0x3) << (4 - bitcount);
		bitcount++;

		if (bitcount == 4)
		  {
		    imageptr++;
		    bytecount += pimage->bytesperpixel;
		    bitcount = 0;
		  }

		break;

	      case 4:
		if (bitcount == 0)
		  imageptr[0] = 0;

		imageptr[0] |= (dwordcolor & 0xF) << (2 - bitcount);
		bitcount++;

		if (bitcount == 2)
		  {
		    imageptr++;
		    bytecount += pimage->bytesperpixel;
		    bitcount = 0;
		  }

		break;

	      case 8:
	      case 16:
	      case 24:
	      case 32:

		for(i = 0; i < pimage->bytesperpixel; i++)
		  imageptr[i] = (dwordcolor >> (8 * i)) & 0xFF;

		imageptr += pimage->bytesperpixel;
		bytecount += pimage->bytesperpixel;
		break;

#ifdef NOTUSED
	      case 8:
		imageptr[0] = (unsigned char) (dwordcolor & 0xFF);
		imageptr += pimage->bytesperpixel;
		bytecount += pimage->bytesperpixel;
		break;

	      case 16:
	      case 24:
	      case 32:
		imageptr[0] = (unsigned char) (dwordcolor >> 24) & 0xFF;
		imageptr[1] = (unsigned char) (dwordcolor >> 16) & 0xFF;
		imageptr[2] = (unsigned char) (dwordcolor >> 8) & 0xFF;
		imageptr[3] = (unsigned char) (dwordcolor & 0xFF);
		imageptr += pimage->bytesperpixel;
		bytecount += pimage->bytesperpixel;
		break;
#endif
	      }
	  }

	/* Pad to the end of the line */
	if (bytecount < pimage->pitch)
	  for(i = 0; i < (pimage->pitch - bytecount); i++)
	    *imageptr++ = 0x00;

	read_xline++;

	if (read_xline == row)
	  status = LOAD_DONE;

	continue;
      }
    }

  NANOX_FREE(colorheap);

  if (status != LOAD_DONE)
    return(-1);
  return(1);
}
#endif /* defined(HAVE_XPM_SUPPORT)*/

#if defined(HAVE_TIFF_SUPPORT)
#include <tiffio.h>

static int
LoadTIFF(char *path, PMWIMAGEHDR pimage)
{
	TIFF 	*tif;
	int	w, h;
	long	size;

	tif = TIFFOpen(path, "r");
	if (!tif)
		return 0;

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	size = w * h;
	pimage->width = w;
	pimage->height = h;
	pimage->bpp = 32;
	pimage->pitch = w * sizeof(uint32);
	pimage->bytesperpixel = 4;
	pimage->planes = 1;
	pimage->palsize = 0;
	pimage->palette = NULL;

	/* upside down, RGB order (with alpha)*/
	pimage->compression = MWIMAGE_RGB;

	/* Allocate image */
	if ((pimage->imagebits = NANOX_MALLOC(size * sizeof(uint32))) == NULL)
		goto err;

	TIFFReadRGBAImage(tif, pimage->width, pimage->height,
		(uint32 *)pimage->imagebits, 0);

#if 0
	{
		/* FIXME alpha channel should be blended with destination*/
		int i;
		uint32	*rgba;
		uint32	rgba_r, rgba_g, rgba_b, rgba_a;
		rgba = (uint32 *)pimage->imagebits;
		for (i = 0; i < size; ++i, ++rgba) {
			if ((rgba_a = TIFFGetA(*rgba) + 1) == 256)
				continue;
			rgba_r = (TIFFGetR(*rgba) * rgba_a)>>8;
			rgba_g = (TIFFGetG(*rgba) * rgba_a)>>8;
			rgba_b = (TIFFGetB(*rgba) * rgba_a)>>8;
			*rgba = 0xff000000|(rgba_b<<16)|(rgba_g<<8)|(rgba_r);
		}
	}
#endif
	TIFFClose(tif);

	/* T.J.Park Note : Convert upside-down image to top-to-bottom. This is not tested yet, because we don't use TIFF. */
	if(pimage->width > 0 && pimage->height > 1)
	{
		uint32 *line = NANOX_MALLOC(pimage->pitch);

		NANOX_FREE(line);
	}
	return 1;

err:
	EPRINTF("LoadTIFF: image loading error\n");
	if (tif)
		TIFFClose(tif);
	if(pimage->imagebits) {
#if 0
		NANOX_FREE(pimage->imagebits);
#else
		NANOX_ScrFreeFrame(pimage->imagebits);
#endif
	}
	if(pimage->palette)
		NANOX_FREE(pimage->palette);
	return 2;		/* image error*/
}
#endif /* defined(HAVE_TIFF_SUPPORT)*/

#endif /* MW_FEATURE_IMAGES - whole file */
