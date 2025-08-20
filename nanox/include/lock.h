/*
 * Critical section locking definitions for Microwindows
 * Copyright (c) 2002, 2003 by Greg Haerr <greg@censoft.com>
 *
 * The current implementation uses pthreads included in libc
 *
 * It's currently required that any locking mechanism
 * allow multiple locks on the same thread (ie. recursive calls)
 * This is necessary since routines nest calls on
 * LOCK(&nxGlobalLock). (nanox/client.c and nanox/nxproto.c)
 */
#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef THREADSAFE
#define THREADSAFE 0
#endif

#ifndef THREADSAFE_WIN32
#define THREADSAFE_WIN32 0
#endif

#ifndef THREADSAFE_VXWORKS
#define THREADSAFE_VXWORKS 0
#endif

#if THREADSAFE
#if defined(WIN32)
#define THREADSAFE_WIN32	1	/* use win32 threadsafe routines */
#elif defined(VXWORKS)
#define THREADSAFE_VXWORKS	1	/* use vxworks task-safe routines */
#else
#define THREADSAFE_LINUX	1	/* use linux threadsafe routines */
#endif
#endif

#if THREADSAFE_WIN32
#include <windows.h>
typedef HANDLE	MWMUTEX;
#define LOCK_DECLARE(name)	MWMUTEX name = INVALID_HANDLE_VALUE
#define LOCK_EXTERN(name)	extern MWMUTEX name
#define LOCK_INIT(m)		m = CreateMutex(NULL, FALSE, NULL)
#define LOCK_FREE(m)		{CloseHandle(m); m = INVALID_HANDLE_VALUE;}
#define LOCK(m)				((m) != INVALID_HANDLE_VALUE) ? WaitForSingleObject(m, INFINITE) : ((void) 0)
#define UNLOCK(m)			((m) != INVALID_HANDLE_VALUE) ? ReleaseMutex(m) : ((void) 0)
#endif

#if THREADSAFE_VXWORKS
#include <semLib.h>
typedef SEM_ID	MWMUTEX;
#define LOCK_DECLARE(name)	SEM_ID name = NULL
#define LOCK_EXTERN(name)	extern MWMUTEX name
#define LOCK_INIT(m)		m = semMCreate (SEM_Q_PRIORITY | SEM_INVERSION_SAFE)
#define LOCK_FREE(m)		{semDelete (m); m = NULL;}
#define LOCK(m)				((m) != NULL) ? semTake (m, WAIT_FOREVER) : ((void) 0)
#define UNLOCK(m)			((m) != NULL) ? semGive(m) : ((void) 0)
#endif
/*
 * Linux critical section locking definitions
 */
#if THREADSAFE_LINUX
#ifndef __USE_GNU
#define __USE_GNU		/* define _NP routines*/
#endif
#ifdef USE_OSADAP
#undef USE_OSADAP
#define USE_OSADAP 0
#endif
#if (USE_OSADAP > 0)
#include "common.h"
#include "osa/osadap.h"

#define OSA_UNINIT_LOCK_VALUE	(-1)
typedef SID_TYPE		MWMUTEX;
#else
#include <pthread.h>
typedef pthread_mutex_t	MWMUTEX;
#endif
#if !defined(__CYGWIN__)
/*
 * This definition doesn't require explicit initialization and -lpthread
 *
 * It uses a common (but non-standard) pthreads extension.
 */

#if (USE_OSADAP > 0)
#define LOCK_DECLARE(name)
#define LOCK_INIT(m)
#else
#define LOCK_DECLARE(name)	MWMUTEX name = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#define LOCK_INIT(m)
#endif
#else
/*
 * This definition requires adding -lpthreads to link all Nano-X applications
 * which isn't required if LOCK_DECLARE is used as above:  The pthread entry
 * points pthread_mutex_lock/unlock are included in the standard C library, but
 * pthread_mutex_init is not.  If this is not the case with your library,
 * include these routines, and add -lpthreads to your applications link line.
 */
#define LOCK_DECLARE(name)	MWMUTEX name
#if 1
/*
 * Use portable version.
 *
 * Note: Older libraries may not have these UNIX98 functions.  You may need
 * to use the old non-portable function name (see below).
 */
#define LOCK_INIT(m)	\
	{ \
	pthread_mutexattr_t attr; \
	pthread_mutexattr_init(&attr); \
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
	pthread_mutex_init((m), &attr); \
	}
#else
/* Use old non-portable function name */
#define LOCK_INIT(m)	\
	{ \
	pthread_mutexattr_t attr; \
	pthread_mutexattr_init(&attr); \
	pthread_mutexattr_setkind_np(&attr, PTHREAD_MUTEX_RECURSIVE_NP); \
	pthread_mutex_init((m), &attr); \
	}
#endif
#endif

#define LOCK_EXTERN(name)	extern MWMUTEX name
#define LOCK_FREE(m)		pthread_mutex_destroy(m)
#define LOCK(m)			pthread_mutex_lock(m)
#define UNLOCK(m)		pthread_mutex_unlock(m)
#endif /* THREADSAFE_LINUX*/

/* no locking support - dummy macros*/
#if !THREADSAFE
typedef int		MWMUTEX;

#define LOCK_DECLARE(name)	MWMUTEX name
#define LOCK_EXTERN(name)	extern MWMUTEX name
#define LOCK_INIT(m)
#define LOCK_FREE(m)
#define LOCK(m)
#define UNLOCK(m)
#endif /* !THREADSAFE*/

#endif	// _LOCK_H_

