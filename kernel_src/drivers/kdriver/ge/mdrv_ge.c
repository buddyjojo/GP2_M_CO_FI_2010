////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    devGE.c
/// @author MStar Semiconductor Inc.
/// @brief  GE Device Driver
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/module.h>
#include <linux/fs.h>    // for MKDEV()
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "mdrv_types.h"
#include "mst_devid.h"
#include "mhal_ge.h"
#include "mdrv_ge.h"

#include "mdrv_probe.h"

static int MDrv_GE_Open(struct inode *inode, struct file *filp);
static int MDrv_GE_Release(struct inode *inode, struct file *filp);
static int MDrv_GE_IOCtl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

//------------------------------------------------------------------------------
// structure
//------------------------------------------------------------------------------
struct MS_GE_dev {
    struct cdev cdev;     /* Char device structure      */
};

struct file_operations _ge_fops = {
    .owner =    THIS_MODULE,
    .ioctl =    MDrv_GE_IOCtl,
    .open =     MDrv_GE_Open,
    .release =  MDrv_GE_Release,
};

// -----------------------------------------------------------------------------
// Global variable
// -----------------------------------------------------------------------------
int _GE_Major =    MDRV_MAJOR_GE;//MDEV_MAJOR_GE;
int _GE_Minor =    MDRV_MINOR_GE; //MDEV_MINOR_GE;
int _GE_Num_Devs = MDRV_GE_NR_DEVS; /* number of bare devGE devices */

struct MS_GE_dev *_pGE_Devices; /* allocated in scull_init_module */

// -----------------------------------------------------------------------------
// Local function
// -----------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                     GE & PE Function
//------------------------------------------------------------------------------

void MDrv_GE_Init(unsigned long arg)
{
    MHal_GE_Init();
}

void MDrv_GE_PowerOff(unsigned long arg)
{
    MHal_GE_PowerOff();
}

int MDrv_GE_Set_Power_Off(unsigned long arg)
{
    U8 u8OnOffFlag;
	
    if (copy_from_user(&u8OnOffFlag, ((U8 __user *)arg), sizeof(U8)))
        return EFAULT;

    MHal_GE_Set_Power_Off(u8OnOffFlag);
		return 0;
}

int MDrv_GE_ScreenCopy(unsigned long arg)
{
    GE_BLOCK geSrcBlk, geDstBlk;

    if (copy_from_user(&geSrcBlk, ((MS_GE_SCREEN_COPY __user *)arg)->pSrcBlk, sizeof(GE_BLOCK)))
        return EFAULT;

    if (copy_from_user(&geDstBlk, ((MS_GE_SCREEN_COPY __user *)arg)->pDstBlk, sizeof(GE_BLOCK)))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_ScreenCopy(&geSrcBlk, &geDstBlk))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_DrawBitmap(unsigned long arg)
{
    BMPHANDLE geBmpHandle;
    GE_DRAW_BMP_INFO geDrawBmpInfo;

    if (copy_from_user(&geBmpHandle, &(((MS_GE_DRAW_BITMAP __user *)arg)->Bmp_Handle), sizeof(BMPHANDLE)))
        return EFAULT;

    if (copy_from_user(&geDrawBmpInfo, ((MS_GE_DRAW_BITMAP __user *)arg)->pDrawBmpInfo, sizeof(GE_DRAW_BMP_INFO)))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_DrawBitmap(geBmpHandle, &geDrawBmpInfo))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GE_GetFontInfo(unsigned long arg)
{
    FONTHANDLE font_handle;
    GE_FONT_INFO font_info;

    if (copy_from_user(&font_handle, &(((MS_GE_GET_FONTINFO __user *)arg)->Font_Handle), sizeof(FONTHANDLE)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_GetFontInfo(font_handle, &font_info))
    {
        return copy_to_user(((MS_GE_GET_FONTINFO __user *)arg)->pFontInfo, &font_info, sizeof(GE_FONT_INFO));
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GE_GetBitmapInfo(unsigned long arg)
{
    BMPHANDLE bmp_handle;
    GE_BITMAP_INFO bmp_info;

    if (copy_from_user(&bmp_handle, &(((MS_GE_GET_BITMAPINFO __user *)arg)->Bmp_Handle), sizeof(BMPHANDLE)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_GetBitmapInfo(bmp_handle, &bmp_info))
    {
        return copy_to_user(((MS_GE_GET_BITMAPINFO __user *)arg)->pBmpInfo, &bmp_info, sizeof(GE_BITMAP_INFO));
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_GetFrameBufferInfo(unsigned long arg)
{
    U32 u32Width;
    U32 u32Height;
    U32 u32Pitch;
    U32 u32FbFmt;
    U32 u32Addr;

    if (GESTATUS_SUCCESS == MHal_GE_GetFrameBufferInfo(&u32Width, &u32Height, &u32Pitch, &u32FbFmt, &u32Addr))
    {
        if(__put_user(u32Width, &(((MS_GE_GET_FRAMEBUFFERINFO __user *)arg)->u32Width)))
            return EFAULT;  // check the return value of __put_user/__get_user (dreamer@lge.com)
        if(__put_user(u32Height, &(((MS_GE_GET_FRAMEBUFFERINFO __user *)arg)->u32Height)))
            return EFAULT;  // check the return value of __put_user/__get_user (dreamer@lge.com)
        if(__put_user(u32Pitch, &(((MS_GE_GET_FRAMEBUFFERINFO __user *)arg)->u32Pitch)))
            return EFAULT;  // check the return value of __put_user/__get_user (dreamer@lge.com)
        if(__put_user(u32FbFmt, &(((MS_GE_GET_FRAMEBUFFERINFO __user *)arg)->u32FbFmt)))
            return EFAULT;  // check the return value of __put_user/__get_user (dreamer@lge.com)
        if(__put_user(u32Addr, &(((MS_GE_GET_FRAMEBUFFERINFO __user *)arg)->u32Addr)))
            return EFAULT;

        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_DrawLine(unsigned long arg)
{
    GE_DRAW_LINE_INFO geDrawLineInfo;

    if (copy_from_user(&geDrawLineInfo, ((GE_DRAW_LINE_INFO __user *)arg), sizeof(GE_DRAW_LINE_INFO)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_DrawLine(&(geDrawLineInfo)))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GE_DrawOval(unsigned long arg)
{
    GE_OVAL_FILL_INFO geOval;

    if (copy_from_user(&geOval, ((GE_OVAL_FILL_INFO __user *)arg), sizeof(GE_OVAL_FILL_INFO)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_DrawOval(&geOval))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}

int  MDrv_GE_LoadBitmap(unsigned long arg)
{
    MS_GE_LOAD_BITMAP geLoadBitmap;

    if (copy_from_user(&geLoadBitmap, ((MS_GE_LOAD_BITMAP __user *)arg), sizeof(MS_GE_LOAD_BITMAP)))
        return EFAULT;

    geLoadBitmap.hBmpHandle = MHal_GE_LoadBitmap(geLoadBitmap.u32Addr, geLoadBitmap.u32Len,
        geLoadBitmap.u32Width, geLoadBitmap.u32Height, geLoadBitmap.enBufferFmt);

    if (ERR_HANDLE != geLoadBitmap.hBmpHandle)

    {
        return copy_to_user(&(((MS_GE_LOAD_BITMAP __user *)arg)->hBmpHandle), &geLoadBitmap.hBmpHandle, sizeof(BMPHANDLE));
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int  MDrv_GE_LoadFont(unsigned long arg)
{
    U32 u32Addr;
    U32 u32Len;
    U32 u32Width;
    U32 u32Height;
    GE_GLYPH_BBOX geBBox;
    GE_Buffer_Format geBufferFmt;
    FONTHANDLE hFontHandle;
    GE_GLYPH_BBOX *pGlyphBBox;

    if (__get_user(u32Addr, &(((MS_GE_LOAD_FONT __user *)arg)->u32Addr)))
        return EFAULT;
    if (__get_user(u32Len, &(((MS_GE_LOAD_FONT __user *)arg)->u32Len)))
        return EFAULT;
    if (__get_user(u32Width, &(((MS_GE_LOAD_FONT __user *)arg)->u32Width)))
        return EFAULT;
    if (__get_user(u32Height, &(((MS_GE_LOAD_FONT __user *)arg)->u32Height)))
        return EFAULT;

    if (NULL == ((MS_GE_LOAD_FONT __user *)arg)->pGlyphBBox)
    {
        pGlyphBBox = NULL;
    }
    else
    {
        if (copy_from_user(&geBBox, ((MS_GE_LOAD_FONT __user *)arg)->pGlyphBBox, sizeof(GE_GLYPH_BBOX)))
        {
            return EFAULT;
        }
        pGlyphBBox = &geBBox;
    }

    if (copy_from_user(&geBufferFmt, &(((MS_GE_LOAD_FONT __user *)arg)->enBufferFmt), sizeof(GE_Buffer_Format)))
        return EFAULT;

    hFontHandle = MHal_GE_LoadFont(u32Addr, u32Len, u32Width, u32Height, pGlyphBBox, geBufferFmt);

    if (ERR_HANDLE != hFontHandle)
    {
        return copy_to_user(&(((MS_GE_LOAD_FONT __user *)arg)->hFontHandle), &hFontHandle, sizeof(FONTHANDLE));
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}

int MDrv_GE_FreeBitmap(unsigned long arg)
{
    BMPHANDLE geBmpHandle;

    if (copy_from_user(&geBmpHandle, ((BMPHANDLE __user *)arg), sizeof(BMPHANDLE)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_FreeBitmap(geBmpHandle))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}


int MDrv_GE_FreeFont(unsigned long arg)
{
    FONTHANDLE geFontHandle;

    if (copy_from_user(&geFontHandle, ((FONTHANDLE __user *)arg), sizeof(FONTHANDLE)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_FreeFont(geFontHandle))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}

int MDrv_GE_RectFill(unsigned long arg)
{
    GE_RECT_FILL_INFO geRectFillInfo;

    if (copy_from_user(&geRectFillInfo, ((GE_RECT_FILL_INFO __user *)arg), sizeof(GE_RECT_FILL_INFO)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_RectFill(&geRectFillInfo))
    {
        //printk("MDrv_GE_RectFill Success\n");
        return 0;
    }
    else
    {
        printk("MDrv_GE_RectFill Fail\n");
        // The return value is not so appropriate.
        return EFAULT;
    }

}

int MDrv_GE_SetClip(unsigned long arg)
{
    GE_POINT_t geV0, geV1;

    if (copy_from_user(&geV0, ((MS_GE_SET_CLIP __user *)arg)->pPoint0, sizeof(GE_POINT_t)))
        return EFAULT;

    if (copy_from_user(&geV1, ((MS_GE_SET_CLIP __user *)arg)->pPoint1, sizeof(GE_POINT_t)))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_SetClip(&geV0, &geV1))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}


int MDrv_GE_SetItalic(unsigned long arg)
{
    MS_GE_SET_ITALIC geSetItalic;

    if (copy_from_user(&geSetItalic, (MS_GE_SET_ITALIC __user *)arg, sizeof(MS_GE_SET_ITALIC)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetItalic(geSetItalic.blEnable, geSetItalic.u8Init_line, geSetItalic.u8Init_dis, geSetItalic.u8Delta))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}


int MDrv_GE_SetDither(unsigned long arg)
{
    U16  blDither;

    if (__get_user(blDither, (U16 __user *)arg))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetDither(blDither))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetSrcBufferInfo(unsigned long arg)
{
    GE_BUFFER_INFO geBufferInfo;
    U32 u32OffsetofByte;

    if (copy_from_user(&geBufferInfo, ((MS_GE_SET_BUFFERINFO __user *)arg)->pBufferInfo, sizeof(GE_BUFFER_INFO)))
        return EFAULT;
    if (__get_user(u32OffsetofByte, &(((MS_GE_SET_BUFFERINFO __user *)arg)->u32OffsetofByte)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetSrcBufferInfo(&geBufferInfo, u32OffsetofByte))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}

int MDrv_GE_SetDstBufferInfo(unsigned long arg)
{
    GE_BUFFER_INFO geBufferInfo;
    U32 u32OffsetofByte;

    if (copy_from_user(&geBufferInfo, ((MS_GE_SET_BUFFERINFO __user *)arg)->pBufferInfo, sizeof(GE_BUFFER_INFO)))
        return EFAULT;
    if (__get_user(u32OffsetofByte, &(((MS_GE_SET_BUFFERINFO __user *)arg)->u32OffsetofByte)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetDstBufferInfo(&geBufferInfo, u32OffsetofByte))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}


int MDrv_GE_SetNearestMode(unsigned long arg)
{
    U16  bNearestMode;

    if (__get_user(bNearestMode, (U16 __user *)arg))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetNearestMode(bNearestMode))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GE_SetMirror(unsigned long arg)
{
    MS_GE_SET_MIRROR  geSetMirror;

    if (copy_from_user(&geSetMirror, (MS_GE_SET_MIRROR __user *)arg, sizeof(MS_GE_SET_MIRROR)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetMirror(geSetMirror.blIsMirrorX, geSetMirror.blIsMirrorY))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GE_SetROP2(unsigned long arg)
{
    MS_GE_SET_ROP2  geSetROP2;

    if (copy_from_user(&geSetROP2, (MS_GE_SET_ROP2 __user *)arg, sizeof(MS_GE_SET_ROP2)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetROP2(geSetROP2.blEnable, geSetROP2.eRopMode))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetRotate(unsigned long arg)
{
    GEROTATE_ANGLE  geRotateAngle;

    if (__get_user(geRotateAngle, (GEROTATE_ANGLE __user *)arg))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_SetRotate(geRotateAngle))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetSrcColorKey(unsigned long arg)
{
    U16 blEnable;
    GE_COLOR_KEY_MODE enColorKeyMode;
    GE_Buffer_Format enBufferFmt;
    U32 u32PS_Color;
    U32 u32GE_Color;

    if (__get_user(blEnable, &((MS_GE_SET_COLORKEY __user *)arg)->blEnable))
        return EFAULT;

    if (__get_user(enColorKeyMode, &((MS_GE_SET_COLORKEY __user *)arg)->eOPMode))
        return EFAULT;

    if (__get_user(enBufferFmt, &((MS_GE_SET_COLORKEY __user *)arg)->enBufferFmt))
        return EFAULT;

    if (__get_user(u32PS_Color, ((MS_GE_SET_COLORKEY __user *)arg)->pu32PS_Color))
        return EFAULT;

    if (__get_user(u32GE_Color, ((MS_GE_SET_COLORKEY __user *)arg)->pu32GE_Color))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetSrcColorKey(blEnable, enColorKeyMode, enBufferFmt, &u32PS_Color, &u32GE_Color))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetDstColorKey(unsigned long arg)
{
    U16 blEnable;
    GE_COLOR_KEY_MODE enColorKeyMode;
    GE_Buffer_Format enBufferFmt;
    U32 u32PS_Color;
    U32 u32GE_Color;

    if (__get_user(blEnable, &((MS_GE_SET_COLORKEY __user *)arg)->blEnable))
        return EFAULT;

    if (__get_user(enColorKeyMode, &((MS_GE_SET_COLORKEY __user *)arg)->eOPMode))
        return EFAULT;

    if (__get_user(enBufferFmt, &((MS_GE_SET_COLORKEY __user *)arg)->enBufferFmt))
        return EFAULT;

    if (__get_user(u32PS_Color, ((MS_GE_SET_COLORKEY __user *)arg)->pu32PS_Color))
        return EFAULT;

    if (__get_user(u32GE_Color, ((MS_GE_SET_COLORKEY __user *)arg)->pu32GE_Color))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetDstColorKey(blEnable, enColorKeyMode, enBufferFmt, &u32PS_Color, &u32GE_Color))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_TextOut(unsigned long arg)
{
    FONTHANDLE geFontHandle;
    U8         *pu8Index = NULL;
    U32        u32StrWidth;
    U16        u16CharNum;
    GE_TEXT_OUT_INFO gdTextOutInfo;
    U8         u8ReturnVal;

    if (copy_from_user(&geFontHandle, &(((MS_GE_TEXTOUT __user *)arg)->Font_Handle), sizeof(FONTHANDLE)))
    {
        u8ReturnVal = EFAULT;
        goto TEXTOUT_EXIT;
    }

    if (__get_user(u16CharNum, &(((MS_GE_TEXTOUT __user *)arg)->u16CharNum)))
    {
        u8ReturnVal = EFAULT;
        goto TEXTOUT_EXIT;
    }

    if (__get_user(u32StrWidth, &(((MS_GE_TEXTOUT __user *)arg)->u32StrWidth)))
    {
        u8ReturnVal = EFAULT;
        goto TEXTOUT_EXIT;
    }

    if (copy_from_user(&gdTextOutInfo, ((MS_GE_TEXTOUT __user *)arg)->pTextOutInfo, sizeof(GE_TEXT_OUT_INFO)))
    {
        u8ReturnVal = EFAULT;
        goto TEXTOUT_EXIT;
    }

    pu8Index = (U8*) kmalloc(u16CharNum * u32StrWidth, GFP_KERNEL);

    if (pu8Index && copy_from_user(pu8Index, ((MS_GE_TEXTOUT __user *)arg)->pu8Index, u16CharNum * u32StrWidth))
    {
        u8ReturnVal = EFAULT;
        goto TEXTOUT_EXIT;
    }

    if (GESTATUS_SUCCESS == MHal_GE_TextOut(geFontHandle, pu8Index, u32StrWidth, &gdTextOutInfo))
    {
        u8ReturnVal = 0;
    }
    else
    {
        u8ReturnVal = EFAULT;
    }

TEXTOUT_EXIT:
    if (NULL != pu8Index)
    {
        kfree(pu8Index);
    }
    return u8ReturnVal;

}



int MDrv_GE_QueryTextDispLength(unsigned long arg)
{
    FONTHANDLE geFontHandle;
    U8         u8Index;
    U32        u32StrWidth;
    GE_TEXT_OUT_INFO gdTextOutInfo;
    U32        u32DispLength;


    if (copy_from_user(&geFontHandle, &(((MS_GE_QUERY_TEXT_DISPLENGTH __user *)arg)->Font_Handle), sizeof(FONTHANDLE)))
        return EFAULT;
    if (__get_user(u8Index, ((MS_GE_QUERY_TEXT_DISPLENGTH __user *)arg)->pu8Index))
        return EFAULT;
    if (__get_user(u32StrWidth, &(((MS_GE_QUERY_TEXT_DISPLENGTH __user *)arg)->u32StrWidth)))
        return EFAULT;
    if (copy_from_user(&gdTextOutInfo, ((MS_GE_QUERY_TEXT_DISPLENGTH __user *)arg)->pTextOutInfo, sizeof(GE_TEXT_OUT_INFO)))
        return EFAULT;
    if (__get_user(u32DispLength, ((MS_GE_QUERY_TEXT_DISPLENGTH __user *)arg)->pu32DispLength))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_QueryTextDispLength(geFontHandle, &u8Index, u32StrWidth, &gdTextOutInfo, &u32DispLength))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}



int MDrv_GE_CharacterOut(unsigned long arg)
{
    GE_CHAR_INFO  geCharInfo;
    GE_TEXT_OUT_INFO geTextOutInfo;


    if (copy_from_user(&geCharInfo, ((MS_GE_CHARACTEROUT __user *)arg)->pGECharInfo, sizeof(GE_CHAR_INFO)))
        return EFAULT;

    if (copy_from_user(&geTextOutInfo, ((MS_GE_CHARACTEROUT __user *)arg)->pGETextOutInfo, sizeof(GE_TEXT_OUT_INFO)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_CharacterOut(&geCharInfo, &geTextOutInfo))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetAlphaSrcFrom(unsigned long arg)
{
    GE_ALPHA_SRC_FROM eMode;

    if (__get_user(eMode, (GE_ALPHA_SRC_FROM __user *)arg))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_SetAlphaSrcFrom(eMode))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetAlphaBlending(unsigned long arg)
{
    MS_GE_SET_ALPHABLENDING geSetAlphaBlending;

    if (copy_from_user(&geSetAlphaBlending, (MS_GE_SET_ALPHABLENDING __user *)arg, sizeof(MS_GE_SET_ALPHABLENDING)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetAlphaBlending(geSetAlphaBlending.enBlendCoef, geSetAlphaBlending.u8BlendFactor))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_EnableAlphaBlending(unsigned long arg)
{
    U16 blEnable;

    if (__get_user(blEnable, (U16 __user *)arg))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_EnableAlphaBlending(blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_Line_Pattern_Reset(unsigned long arg)
{
    if (GESTATUS_SUCCESS == MHal_GE_Line_Pattern_Reset())
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_Set_Line_Pattern(unsigned long arg)
{
    U16 blEnable;
    U8   u8LinePattern;
    U8   u8RepeatFactor;

    if (__get_user(blEnable, &(((MS_GE_SET_LINEPATTERN __user *)arg)->blEnable)))
        return EFAULT;
    if (__get_user(u8LinePattern, &(((MS_GE_SET_LINEPATTERN __user *)arg)->u8LinePattern)))
        return EFAULT;
    if (__get_user(u8RepeatFactor, &(((MS_GE_SET_LINEPATTERN __user *)arg)->u8RepeatFactor)))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_Set_Line_Pattern(blEnable, u8LinePattern, u8RepeatFactor))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_BitBlt(unsigned long arg)
{
    GE_DRAW_RECT geDrawRect;
    U32 u32DrawFlag;

    if (copy_from_user(&geDrawRect, ((MS_GE_SET_BITBLT __user *)arg)->pGEDrawRect, sizeof(GE_DRAW_RECT)))
        return EFAULT;
    if (__get_user(u32DrawFlag, &(((MS_GE_SET_BITBLT __user *)arg)->u32DrawFlag)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_BitBlt(&geDrawRect, u32DrawFlag))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_BitBltEx(unsigned long arg)
{
    GE_DRAW_RECT geDrawRect;
    U32 u32DrawFlag;
    GE_SCALE_INFO geScaleInfo;

    if (copy_from_user(&geDrawRect, ((MS_GE_SET_BITBLTEX __user *)arg)->pDrawRect, sizeof(GE_DRAW_RECT)))
        return EFAULT;
    if (__get_user(u32DrawFlag, &(((MS_GE_SET_BITBLTEX __user *)arg)->u32DrawFlag)))
        return EFAULT;
    if (copy_from_user(&geScaleInfo, ((MS_GE_SET_BITBLTEX __user *)arg)->pScaleInfo, sizeof(GE_SCALE_INFO)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_BitBltEx(&geDrawRect, u32DrawFlag, &geScaleInfo))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_TrapezoidBitBlt(unsigned long arg)
{
    GE_TRAPEZOID_INFO geTrapezoidInfo;
    U32 u32DrawFlag;

    if (copy_from_user(&geTrapezoidInfo, ((MS_GE_SET_TRAPEZOID_BITBLT __user *)arg)->pGETrapezoidInfo, sizeof(GE_TRAPEZOID_INFO)))
        return EFAULT;
    if (__get_user(u32DrawFlag, &(((MS_GE_SET_TRAPEZOID_BITBLT __user *)arg)->u32DrawFlag)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_TrapezoidBitBlt(&geTrapezoidInfo, u32DrawFlag))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_GetScaleBltInfo(unsigned long arg)
{
    GE_DRAW_RECT geDrawRect;
    GE_BLOCK  geSrcBlk;
    GE_BLOCK  geDstBlk;
    GE_SCALE_INFO geScaleInfo;

    if (copy_from_user(&geDrawRect, ((MS_GE_GET_SCALEBLTINFO __user *)arg)->pDrawRect, sizeof(GE_DRAW_RECT)))
        return EFAULT;
    if (copy_from_user(&geSrcBlk, ((MS_GE_GET_SCALEBLTINFO __user *)arg)->pSrcBlk, sizeof(GE_BLOCK)))
        return EFAULT;
    if (copy_from_user(&geDstBlk, ((MS_GE_GET_SCALEBLTINFO __user *)arg)->pDstBlk, sizeof(GE_BLOCK)))
        return EFAULT;
    if (copy_from_user(&geScaleInfo, ((MS_GE_GET_SCALEBLTINFO __user *)arg)->pScaleInfo, sizeof(GE_SCALE_INFO)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_GetScaleBltInfo(&geDrawRect, &geSrcBlk, &geDstBlk, &geScaleInfo))
    {   // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        if( copy_to_user(((MS_GE_GET_SCALEBLTINFO __user *)arg)->pSrcBlk, &geSrcBlk, sizeof(GE_BLOCK)))
            return EFAULT;
        if( copy_to_user(((MS_GE_GET_SCALEBLTINFO __user *)arg)->pDstBlk, &geDstBlk, sizeof(GE_BLOCK)))
            return EFAULT;
        if( copy_to_user(((MS_GE_GET_SCALEBLTINFO __user *)arg)->pScaleInfo, &geScaleInfo, sizeof(GE_SCALE_INFO)))
            return EFAULT;

        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetIntensity(unsigned long arg)
{
    U32 u32ID;
    GE_Buffer_Format enBufferFmt;
    U32 u32Color;

    if (__get_user(u32ID, &(((MS_GE_SET_INTENSITY __user *)arg)->u32ID)))
        return EFAULT;
    if (__get_user(enBufferFmt, &(((MS_GE_SET_INTENSITY __user *)arg)->enBufferFmt)))
        return EFAULT;
    if (__get_user(u32Color, ((MS_GE_SET_INTENSITY __user *)arg)->pu32Color))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetIntensity(u32ID, enBufferFmt, &u32Color))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_CreateBuffer(unsigned long arg)
{
#if 0
    MS_GE_CREATE_BUFFER stCreateBuffer;

    if (copy_from_user(&stCreateBuffer, (MS_GE_CREATE_BUFFER __user *)arg, sizeof(MS_GE_CREATE_BUFFER)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_CreateBuffer(stCreateBuffer.u32Addr, stCreateBuffer.u32Width, stCreateBuffer.u32Height, stCreateBuffer.geBufFmt))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
#else
    printk("MHal_GE_CreateBuffer() is obsoleted. Need to allocate by yourself.\n");
    return EFAULT;
#endif
}


int MDrv_GE_DeleteBuffer(unsigned long arg)
{
#if 0
    GE_BUFFER_INFO geBufferInfo;

    if (copy_from_user(&geBufferInfo, (GE_BUFFER_INFO __user *)arg, sizeof(GE_BUFFER_INFO)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_DeleteBuffer(&geBufferInfo))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
#else
    printk("MHal_GE_CreateBuffer() is obsoleted. Need to allocate by yourself.\n");
    return EFAULT;
#endif
}

int MDrv_GE_BeginDraw(unsigned long arg)
{
    if (GESTATUS_SUCCESS == MHal_GE_BeginDraw())
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GE_EndDraw(unsigned long arg)
{

    if (GESTATUS_SUCCESS == MHal_GE_EndDraw())
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_YUV_Set(unsigned long arg)
{
    GE_YUV_INFO geYUVInfo;
    if (copy_from_user(&geYUVInfo, (GE_BUFFER_INFO __user *)arg, sizeof(geYUVInfo)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_YUV_Set(&geYUVInfo))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_YUV_Get(unsigned long arg)
{
    GE_YUV_INFO geYUVInfo;

    if (GESTATUS_SUCCESS == MHal_GE_YUV_Get(&geYUVInfo))
    {
        return copy_to_user(( GE_BUFFER_INFO __user *)arg, &geYUVInfo, sizeof(GE_BUFFER_INFO));
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_InitBufferInfo(unsigned long arg)
{
    GE_BUFFER_INFO stBufInfo;

    U32 u32Addr;
    U32 u32Width;
    U32 u32Height;
    GE_Buffer_Format enBufferFmt;

    if (__get_user(u32Addr, &(((MS_GE_INIT_BUFFERINFO __user *)arg)->u32Addr)))
        return EFAULT;
    if (__get_user(u32Width, &(((MS_GE_INIT_BUFFERINFO __user *)arg)->u32Width)))
        return EFAULT;
    if (__get_user(u32Height, &(((MS_GE_INIT_BUFFERINFO __user *)arg)->u32Height)))
        return EFAULT;
    if (__get_user(enBufferFmt, &(((MS_GE_INIT_BUFFERINFO __user *)arg)->enBufferFmt)))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_InitBufferInfo(&stBufInfo, u32Addr, u32Width, u32Height, enBufferFmt))
    {
        return copy_to_user(((MS_GE_INIT_BUFFERINFO __user *)arg)->pBufInfo, &stBufInfo, sizeof(GE_BUFFER_INFO));
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_EnableVCmdQueue(unsigned long arg)
{
    U16  blEnable_VCMDQ;

    if (__get_user(blEnable_VCMDQ, (U16 __user *)arg))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_EnableVCmdQueue(blEnable_VCMDQ))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetVCmdBuffer(unsigned long arg)
{
    MS_GE_SET_VCMD_BUFFER stVCMD_Buf;

    if (copy_from_user(&stVCMD_Buf, (MS_GE_SET_VCMD_BUFFER __user *)arg, sizeof(MS_GE_SET_ITALIC)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetVCmdBuffer(stVCMD_Buf.u32Addr, stVCMD_Buf.enBufSize))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GE_SetVCmd_W_Thread(unsigned long arg)
{
    U8 u8WThread;

    if (__get_user(u8WThread, (U8 __user *)arg))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_SetVCmd_W_Thread(u8WThread))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetVCmd_R_Thread(unsigned long arg)
{
    U8 u8WThread;

    if (__get_user(u8WThread, (U8 __user *)arg))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_SetVCmd_R_Thread(u8WThread))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_EnableDSTAC(unsigned long arg)
{
    U16 blEnable;

    if (__get_user(blEnable, (U8 __user *)arg))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_EnableDSTAC(blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_Set_DSTAC_Mode(unsigned long arg)
{
    U16 blEnable;

    if (__get_user(blEnable, (U8 __user *)arg))
        return EFAULT;


    if (GESTATUS_SUCCESS == MHal_GE_Set_DSTAC_Mode(blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_Set_DSTAC_Thread(unsigned long arg)
{
    MS_GE_SET_DSTAC_THREAD stDSTAC_Thread;

    if (copy_from_user(&stDSTAC_Thread, (MS_GE_SET_VCMD_BUFFER __user *)arg, sizeof(MS_GE_SET_DSTAC_THREAD)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_Set_DSTAC_Thread(stDSTAC_Thread.u8LoThread, stDSTAC_Thread.u8HiThread))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_SetROP8(unsigned long arg)
{
    MS_GE_SET_ROP8  stSetROP8;

    if (copy_from_user(&stSetROP8, (MS_GE_SET_ROP8 __user *)arg, sizeof(MS_GE_SET_ROP8)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetROP8(stSetROP8.blEnable, stSetROP8.eRopMode, stSetROP8.u8ConstAlpha))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GE_SetFramePtr(unsigned long arg)
{

    GE_BUFFER_INFO inarg ;

    if (copy_from_user(&inarg, (GE_BUFFER_INFO __user *)arg, sizeof(GE_BUFFER_INFO)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_SetFramePtr((inarg.u32Width>>8), (inarg.u32Width&0xFF), inarg.u32Addr))
    {
        return 0;
}
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GE_Palette_Set(unsigned long arg)
{
    GePaletteEntry *pGePalArray;
    U32 u32PalStart;
    U32 u32PalEnd;
    U32 u32PalSize;
//    GePalType ePalType;
    int iRet = 0;

    if (__get_user(u32PalStart, &((MS_GE_SET_PALETTE __user *)arg)->u32PalStart))
        iRet = EFAULT;
    if (__get_user(u32PalEnd, &((MS_GE_SET_PALETTE __user *)arg)->u32PalEnd))
        iRet = EFAULT;
//    if (__get_user(ePalType, &((MS_GE_SET_PALETTE __user *)arg)->enPalType))
//        iRet = EFAULT;

    u32PalSize = u32PalEnd - u32PalStart + 1;

    pGePalArray = kmalloc(sizeof(GePaletteEntry) * u32PalSize, GFP_KERNEL);

    if (pGePalArray)
    {
        if (copy_from_user(pGePalArray, ((MS_GE_SET_PALETTE __user *)arg)->pPalArray, sizeof(GePaletteEntry)*u32PalSize))
        {
            //iRet = EFAULT;
            kfree(pGePalArray); // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
            return EFAULT;
        }

        if (MHal_GE_Palette_Set(pGePalArray, u32PalStart, u32PalEnd))
        {
            iRet = EFAULT;
        }

        kfree(pGePalArray);
    }
    else
    {
        iRet = EFAULT;
    }

    return iRet;

}

int MDrv_GE_WaitGEFinish(unsigned long arg)
{
    MS_GE_WAIT_GE gewait;

    if (copy_from_user(&gewait, (MS_GE_WAIT_GE __user *)arg, sizeof(MS_GE_WAIT_GE)))
        return EFAULT;

    if (GESTATUS_SUCCESS == MHal_GE_WaitGEFinish(gewait.u8CMDCount))
    {
        gewait.result = TRUE;
        copy_to_user(&(((MS_GE_WAIT_GE __user *)arg)->result), &(gewait.result), sizeof(U8));        
        return 0;
    }
    else
    {
        gewait.result = FALSE;
        copy_to_user(&(((MS_GE_WAIT_GE __user *)arg)->result), &(gewait.result), sizeof(U8));        
        return EFAULT;
    }
}

///////////////////////////////

// -----------------------------------------------------------------------------
// Device Methods
// -----------------------------------------------------------------------------

/*
 * Set up the char_dev structure for this device.
 */
static void MDrv_GE_Setup_CDev(struct MS_GE_dev *dev, int index)
{
    int err, devno = MKDEV(_GE_Major, _GE_Minor);

    cdev_init(&dev->cdev, &_ge_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &_ge_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    /* Fail gracefully if need be */
    if (err)
        printk(KERN_NOTICE "Error %d adding drvGE %d", err, index);
}

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void MDrv_GE_CleanUp_Module(void)
{
    int i;
    dev_t devno = MKDEV(_GE_Major, _GE_Minor);

    /* Get rid of our char dev entries */
    if (_pGE_Devices) {
        for (i = 0; i < _GE_Num_Devs; i++) {
            cdev_del(&_pGE_Devices[i].cdev);
        }
        kfree(_pGE_Devices);
    }

    /* cleanup_module is never called if registering failed */
    unregister_chrdev_region(devno, _GE_Num_Devs);

}

/*
 * Open and close
 */

int MDrv_GE_Open(struct inode *inode, struct file *filp)
{
    struct MS_GE_dev *dev; /* device information */

    dev = container_of(inode->i_cdev, struct MS_GE_dev, cdev);
    filp->private_data = dev; /* for other methods */

    printk("GE opens successfully\n");

    return 0;          /* success */
}

int MDrv_GE_Release(struct inode *inode, struct file *filp)
{
    printk("GE closes successfully\n");

    return 0;
}

extern void dummyRegWrite( void ) ;
extern void outGE_WaitAvailableCMDQueue(void);

int MDrv_GE_IOCtl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
    int err = 0;
    int retval = 0;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (_IOC_TYPE(cmd) != MDRV_GE_IOC_MAGIC) {
        printk("IOCtl Type Error!!! (Cmd=%x)\n",cmd);
        return -ENOTTY;
    }
    if (_IOC_NR(cmd) > MDRV_GE_IOC_MAXNR) {
        printk("IOCtl NR Error!!! (Cmd=%x)\n",cmd);
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err) {
        printk("IOCtl Error!!! (cmd=%x)\n",cmd);
        return -EFAULT;
    }

    PROBE_IO_ENTRY(MDRV_MAJOR_GE, _IOC_NR(cmd));

    //printk("GE IOCTL %lx\n",cmd);
    switch(cmd) {
      case MDRV_GE_IOC_INIT:
        MDrv_GE_Init(arg);
        break;

      case MDRV_GE_IOC_POWER_OFF:
        MDrv_GE_PowerOff(arg);
        break;

      case MDRV_GE_IOC_SCREEN_COPY:
        retval = MDrv_GE_ScreenCopy(arg);
        break;

      case MDRV_GE_IOC_DRAW_BITMAP:
        retval = MDrv_GE_DrawBitmap(arg);
        break;

      case MDRV_GE_IOC_GET_FONTINFO:
        retval = MDrv_GE_GetFontInfo(arg);
        break;

      case MDRV_GE_IOC_GET_BITMAPINFO:
        retval = MDrv_GE_GetBitmapInfo(arg);
        break;

      case MDRV_GE_IOC_GET_FRAMEBUFFERINFO:
        retval = MDrv_GE_GetFrameBufferInfo(arg);
        break;

      case MDRV_GE_IOC_DRAW_LINE:
        retval = MDrv_GE_DrawLine(arg);
        break;

      case MDRV_GE_IOC_DRAW_OVAL:
        retval = MDrv_GE_DrawOval(arg);
        break;

      case MDRV_GE_IOC_LOAD_BITMAP:
        retval = MDrv_GE_LoadBitmap(arg);
        break;

      case MDRV_GE_IOC_LOAD_FONT:
        retval = MDrv_GE_LoadFont(arg);
        break;

      case MDRV_GE_IOC_FREE_BITMAP:
          MDrv_GE_FreeBitmap(arg);
        break;

      case MDRV_GE_IOC_FREE_FONT:
          MDrv_GE_FreeFont(arg);
        break;

      case MDRV_GE_IOC_RECT_FILL:
          MDrv_GE_RectFill(arg);
          //printk("rect fill finish\n");
        break;

      case MDRV_GE_IOC_SET_CLIP:
          MDrv_GE_SetClip(arg);
        break;

      case MDRV_GE_IOC_SET_ITALIC:
          MDrv_GE_SetItalic(arg);
        break;

      case MDRV_GE_IOC_SET_DITHER:
          MDrv_GE_SetDither(arg);
        break;

      case MDRV_GE_IOC_SET_SRCBUFFERINO:
          MDrv_GE_SetSrcBufferInfo(arg);
        break;

      case MDRV_GE_IOC_SET_DSTBUFFERINO:
          MDrv_GE_SetDstBufferInfo(arg);
        break;

      case MDRV_GE_IOC_SET_NEARESTMODE:
          MDrv_GE_SetNearestMode(arg);
        break;

      case MDRV_GE_IOC_SET_MIRROR:
          MDrv_GE_SetMirror(arg);
        break;

      case MDRV_GE_IOC_SET_ROP2:
          MDrv_GE_SetROP2(arg);
        break;

      case MDRV_GE_IOC_SET_ROTATE:
          MDrv_GE_SetRotate(arg);
        break;

      case MDRV_GE_IOC_SET_SRCCOLORKEY:
          MDrv_GE_SetSrcColorKey(arg);
        break;

      case MDRV_GE_IOC_SET_DSTCOLORKEY:
          MDrv_GE_SetDstColorKey(arg);
        break;

      case MDRV_GE_IOC_TEXTOUT:
          MDrv_GE_TextOut(arg);
        break;

      case MDRV_GE_IOC_QUERY_TEXT_DISPLENGTH:
          MDrv_GE_QueryTextDispLength(arg);
        break;

      case MDRV_GE_IOC_CHARACTEROUT:
          MDrv_GE_CharacterOut(arg);
        break;

      case MDRV_GE_IOC_SET_ALPHASRCFROM:
          MDrv_GE_SetAlphaSrcFrom(arg);
        break;

      case MDRV_GE_IOC_SET_ALPHABLENDING:
          MDrv_GE_SetAlphaBlending(arg);
        break;

      case MDRV_GE_IOC_ENABLE_ALPHABLENDING:
          MDrv_GE_EnableAlphaBlending(arg);
        break;

      case MDRV_GE_IOC_LINEPATTERN_RESET:
          MDrv_GE_Line_Pattern_Reset(arg);
        break;

      case MDRV_GE_IOC_SET_LINEPATTERN:
          MDrv_GE_Set_Line_Pattern(arg);
        break;

      case MDRV_GE_IOC_BITBLT:
          MDrv_GE_BitBlt(arg);
        break;

      case MDRV_GE_IOC_BITBLTEX:
          MDrv_GE_BitBltEx(arg);
        break;

      case MDRV_GE_IOC_TRAPEZOID_BITBLT:
          MDrv_GE_TrapezoidBitBlt(arg);
        break;
        
      case MDRV_GE_IOC_Get_SCALEBLTINFO:
          MDrv_GE_GetScaleBltInfo(arg);
        break;

      case MDRV_GE_IOC_SET_INTENSITY:
          MDrv_GE_SetIntensity(arg);
        break;

      case MDRV_GE_IOC_CREATE_BUFFER:
          MDrv_GE_CreateBuffer(arg);
        break;

      case MDRV_GE_IOC_DELETE_BUFFER:
          MDrv_GE_DeleteBuffer(arg);
        break;

      case MDRV_GE_IOC_BEGIN_DRAW:
          MDrv_GE_BeginDraw(arg);
        break;

      case MDRV_GE_IOC_END_DRAW:
          MDrv_GE_EndDraw(arg);
        break;

      case MDRV_GE_IOC_SET_YUV:
          MDrv_GE_YUV_Set(arg);
        break;

      case MDRV_GE_IOC_GET_YUV:
          MDrv_GE_YUV_Get(arg);
        break;

      case MDRV_GE_IOC_INIT_BUFFERINFO:
          MDrv_GE_InitBufferInfo(arg);
        break;

      case MDRV_GE_IOC_ENABLE_VCMDQ:
          MDrv_GE_EnableVCmdQueue(arg);
        break;

      case MDRV_GE_IOC_SET_VCMD_BUF:
          MDrv_GE_SetVCmdBuffer(arg);
        break;

      case MDRV_GE_IOC_SET_VCMD_W_TH:
          MDrv_GE_SetVCmd_W_Thread(arg);
        break;

      case MDRV_GE_IOC_SET_VCMD_R_TH:
          MDrv_GE_SetVCmd_R_Thread(arg);
        break;

      case MDRV_GE_IOC_ENABLE_DSTAC:
          MDrv_GE_EnableDSTAC(arg);
        break;

      case MDRV_GE_IOC_SET_DSTAC_MODE:
          MDrv_GE_Set_DSTAC_Mode(arg);
        break;

      case MDRV_GE_IOC_SET_DSTAC_TH:
          MDrv_GE_Set_DSTAC_Thread(arg);
        break;

      case MDRV_GE_IOC_SET_ROP8:
          MDrv_GE_SetROP8(arg);
        break;

      case MDRV_GE_IOC_SET_FRAME_PTR:
          MDrv_GE_SetFramePtr(arg);
        break;

///// LGE : 20080517 //////////
      case MDRV_GE_IOC_SET_PALETTE:
          MDrv_GE_Palette_Set(arg);
        break;
///////////////////////////////

      case MDRV_GE_IOC_WAIT_GE_FINISH:
          MDrv_GE_WaitGEFinish(arg);
        break;
        
      case MDRV_GE_IOC_SET_POWER_OFF:
        MDrv_GE_Set_Power_Off(arg);
        break;
        
      default:  /* redundant, as cmd was checked against MAXNR */
        printk("Unknow IOCTL %x\n",cmd);
        PROBE_IO_EXIT(MDRV_MAJOR_GE, _IOC_NR(cmd));
        return -ENOTTY;
    }

    outGE_WaitAvailableCMDQueue();

    dummyRegWrite() ;


    PROBE_IO_EXIT(MDRV_MAJOR_GE, _IOC_NR(cmd));

    return retval;

}




static int __init MDrv_GE_Module_Init(void)
{
    int result, i;
    dev_t dev_Number = 0;

    printk("GE driver inits\n");

   /*
    * Get a range of minor numbers to work with, asking for a dynamic
    * major unless directed otherwise at load time.
    */

    if (_GE_Major) {
        dev_Number = MKDEV(_GE_Major, _GE_Minor);
        result = register_chrdev_region(dev_Number, _GE_Num_Devs, "drvGE");
    } else {
        result = alloc_chrdev_region(&dev_Number, _GE_Minor, _GE_Num_Devs, "drvGE");
        _GE_Major = MAJOR(dev_Number);
    }
    if (result < 0) {
        printk("drvGE: can't get major %d\n", _GE_Major);
        return result;
    }

    /*
     * allocate the devices -- we can't have them static, as the number
     * can be specified at load time
     */
    _pGE_Devices = kmalloc(_GE_Num_Devs * sizeof(struct MS_GE_dev), GFP_KERNEL);
    if (!_pGE_Devices) {
        result = -ENOMEM;
        goto fail;  /* Make this more graceful */
    }
    memset(_pGE_Devices, 0, _GE_Num_Devs * sizeof(struct MS_GE_dev));

    /* Initialize each device. */
    for (i = 0; i < _GE_Num_Devs; i++) {
        MDrv_GE_Setup_CDev(&_pGE_Devices[i], i);
    }

    // Maybe need to move to MHal_GE_Module_Init
    MHal_GE_Init();

    return 0;

  fail:
    MDrv_GE_CleanUp_Module();

    return result;

}

static void __exit MDrv_GE_Module_Exit(void)
{
    dev_t dev_Number = MKDEV(_GE_Major, _GE_Minor);

    /* cleanup_module is never called if registering failed */
    unregister_chrdev_region(dev_Number, _GE_Num_Devs);

    printk("GE driver exits\n");
}




module_init(MDrv_GE_Module_Init);
module_exit(MDrv_GE_Module_Exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("GE driver");
MODULE_LICENSE("MSTAR");
