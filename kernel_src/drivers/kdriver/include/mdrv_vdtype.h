#ifndef __MDRV_VDTYPE_H__
#define __MDRV_VDTYPE_H__

/* Video Source Type */
typedef enum
{
    E_SOURCE_INVALID               =   0x00,               ///< Video source Invalid
    E_SOURCE_ATV                   =   0x01,               ///< Video source ATV
    E_SOURCE_CVBS1                 =   0x02,               ///< Video source CVBS 1
    E_SOURCE_CVBS2                 =   0x03,               ///< Video source CVBS 2
    E_SOURCE_CVBS3                 =   0x04,               ///< Video source CVBS 2
    E_SOURCE_SVIDEO1               =   0x05,               ///< Video source SVIDEO 1
    E_SOURCE_SVIDEO2               =   0x06,               ///< Video source SVIDEO 2
    E_SOURCE_SCART1                =   0x07,               ///< Video source SCART 1
    E_SOURCE_SCART2                =   0x08,               ///< Video source SCART 2
    E_SOURCE_THROUGH_3DCOMB        =   0x50,               ///< Video source through 3D Comb
    E_SOURCE_THROUGH_3DCOMB_ATV    =   (E_SOURCE_THROUGH_3DCOMB | E_SOURCE_ATV),   ///< Video source through 3D Comb ATV
    E_SOURCE_THROUGH_3DCOMB_CVBS1  =   (E_SOURCE_THROUGH_3DCOMB | E_SOURCE_CVBS1),///< Video source through 3D Comb CVBS1
    E_SOURCE_THROUGH_3DCOMB_CVBS2  =   (E_SOURCE_THROUGH_3DCOMB | E_SOURCE_CVBS2),///< Video source through 3D Comb CVBS2
    E_SOURCE_THROUGH_3DCOMB_SCART1 =   (E_SOURCE_THROUGH_3DCOMB | E_SOURCE_SCART1),///< Video source through 3D Comb SCART1
    E_SOURCE_THROUGH_3DCOMB_SCART2 =   (E_SOURCE_THROUGH_3DCOMB | E_SOURCE_SCART2)///< Video source through 3D Comb SCART2
} VD_VIDEOSOURCE_TYPE;

/* Video Standard type */
typedef enum
{
    E_STANDARD_PAL_BGHI        = 0x00,        ///< Video standard PAL BGHI
    E_STANDARD_NTSC_M          = 0x01,        ///< Video standard NTSC M
    E_STANDARD_SECAM           = 0x02,        ///< Video standard SECAM
    E_STANDARD_NTSC_44         = 0x03,        ///< Video standard  NTSC 44
    E_STANDARD_PAL_M           = 0x04,        ///< Video standard  PAL M
    E_STANDARD_PAL_N           = 0x05,        ///< Video standard  PAL N
    E_STANDARD_PAL_60          = 0x06,        ///< Video standard PAL 60
    E_STANDARD_NOTSTANDARD     = 0x07        ///< NOT Video standard
} VD_VIDEOSTANDARD_TYPE;


#endif


