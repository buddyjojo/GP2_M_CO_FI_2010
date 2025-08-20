#ifndef __MDRV_VD_H__
#define __MDRV_VD_H__

#include "hal_vd_types.h"


// use 'v' as magic number
#define VD_IOCTL_MAGIC                          'v'
#define IOCTL_VD_INIT                           _IOW(VD_IOCTL_MAGIC, 0, U32)
#define IOCTL_VD_GET_CURRENT_VIDEO_STANDARD     _IOR(VD_IOCTL_MAGIC, 1, U32)
#define IOCTL_VD_THREAD_HALT                    _IOW(VD_IOCTL_MAGIC, 2, U32)
#define IOCTL_VD_THREAD_START                   _IOW(VD_IOCTL_MAGIC, 3, U32)
#define IOCTL_VD_WAIT_EVENT                     _IOR(VD_IOCTL_MAGIC, 4, U32)
#define IOCTL_VD_GET_CURRENT_VIDEO_SOURCE       _IOR(VD_IOCTL_MAGIC, 5, U32)
#define IOCTL_VD_GET_STATES                     _IOR(VD_IOCTL_MAGIC, 6, U32)
#define IOCTL_VD_SET_VIDEO_SOURCE               _IOW(VD_IOCTL_MAGIC, 7, U32)
#define IOCTL_VD_MCU_RUN                        _IOW(VD_IOCTL_MAGIC, 8, U32)
#define IOCTL_VD_GET_WINDOWS                    _IOR(VD_IOCTL_MAGIC, 9, U32)
#define IOCTL_VD_GET_HV_TOTAL                   _IOR(VD_IOCTL_MAGIC,10, U32)
#define IOCTL_VD_HSYN_DETECTION                 _IOW(VD_IOCTL_MAGIC,11, U32)
#define IOCTL_VD_CHECKSTATES                    _IOR(VD_IOCTL_MAGIC,12, U32)
#define IOCTL_VD_GET_WINDOWS_SVIDEO             _IOR(VD_IOCTL_MAGIC,13, U32)
#define IOCTL_VD_RESET_AGC                      _IOW(VD_IOCTL_MAGIC,14, U32)
/* this ioctl speed 3D COMB up */
#define IOCTL_VD_3DSPEEDUP                      _IOW(VD_IOCTL_MAGIC,15, U32)
/* this ioctl enable or disable 3D COMB. */
#define IOCTL_VD_3DCOMB                         _IOW(VD_IOCTL_MAGIC,16, U32)
//	added to change the initial value of color kill bound(dreamer@lge.com)
#define IOCTL_VD_INIT_COLOR_KILL_BOUND          _IOW(VD_IOCTL_MAGIC,17, U32)
#define IOCTL_VD_LOADDSP                        _IOW(VD_IOCTL_MAGIC,18, U32)//DSP branch
// Set Analog Color System - Start 090309.
#define IOCTL_VD_SET_ANALOG_COLOR_SYSTEM        _IOW(VD_IOCTL_MAGIC,19, U32)
#define IOCTL_VD_GET_ANALOG_STD_SYSTEM          _IOR(VD_IOCTL_MAGIC,20, U32)
// Set Analog Color System - End   090309.

#define IOCTL_VD_EXIT               			_IOW(VD_IOCTL_MAGIC, 21, U32)  //20090826EL

// shjang_091020
#define IOCTL_VD_SWING_THRESHOLD				_IOW(VD_IOCTL_MAGIC,22, U32)
// 20091020 BY
#define IOCTL_VD_POWER_ON                       _IOW(VD_IOCTL_MAGIC, 23, U32)
#define IOCTL_VD_POWER_OFF                      _IOW(VD_IOCTL_MAGIC, 24, U32)
#define IOCTL_VD_EXTERNAL_LOADDSP				_IOW(VD_IOCTL_MAGIC, 25, U32)

//lachesis_100118
#define IOCTL_VD_SET_ADAPTIVE_SLICE				_IOW(VD_IOCTL_MAGIC,26, unsigned int)
#define IOCTL_VD_SET_ADAPTIVE_TRACKING          _IOW(VD_IOCTL_MAGIC,27, unsigned int)

// shjang_100322 manual H pll speed control
#define IOCTL_VD_HPLL_TRACKING					_IOW(VD_IOCTL_MAGIC, 28, U32)

#define IOCTL_VD_SET_HSYNC_WIDTH_DET	 		_IOW(VD_IOCTL_MAGIC,29, unsigned int)

#endif


