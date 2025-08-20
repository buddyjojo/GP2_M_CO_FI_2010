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

#define _MSAPI_JPEG_MEMORY_C_

#include <linux/mm.h>
#include "mdrv_jpeg_memory.h"



struct __mem__
{
    struct __mem__  *next;    /* single-linked list */
    U32	  len;      /* length of following block */
};

typedef struct __mem__	 __memt__;
typedef __memt__  *__memp__;

#define	HLEN	(sizeof(__memt__))
#define MIN_BLOCK	(HLEN * 4)


#define AVAIL	(__mem_avail__[0])

#define MIN_POOL_SIZE	(HLEN * 10)


__memt__  __mem_avail__ [2] =
{
    { NULL, 0 },	/* HEAD for the available block list */
    { NULL, 0 },	/* UNUSED but necessary so free doesn't join HEAD or ROVER with the pool */
};

BOOL MDrv_JPEG_init_mempool (void *pool, U32 size)
{
    if (size < MIN_POOL_SIZE)
        return FALSE; 					/* FAILURE */

    if (pool == NULL)
    {
        pool = (void *)1;
        size--;
    }

    memset(pool, 0, size);

    AVAIL.next = (struct __mem__  *)pool;
    AVAIL.len  = size;

    (AVAIL.next)->next = NULL;
    (AVAIL.next)->len  = size - HLEN;

    return TRUE;   					/* SUCCESS */
}

void *MDrv_JPEG_malloc (U32 size)
{
    __memp__ q;			/* ptr to free block */
    __memp__ p;			/* q->next */
    U32 k;			/* space remaining in the allocated block */
    q = &AVAIL;

    while (1)
    {
        if ((p = q->next) == NULL)
        {
            return (NULL);				/* FAILURE */
        }

        if (p->len >= size)
            break;

        q = p;
    }

    k = p->len - size;		/* calc. remaining bytes in block */

    if (k < MIN_BLOCK)		/* rem. bytes too small for new block */
    {
        q->next = p->next;
        return (&p[1]);				/* SUCCESS */
    }

    k -= HLEN;
    p->len = k;

    q = (__memp__ ) (((char *) (&p [1])) + k);
    q->len = size;
    q->next = NULL;

    return (&q[1]);					/* SUCCESS */
}

void MDrv_JPEG_free (void *memp)
{
    __memp__ q;		/* ptr to free block */
    __memp__ p;		/* q->next */
    __memp__ p0;		/* block to free */

    if ((memp == NULL) || (AVAIL.len == 0))
        return;

    p0 = (__memp__) memp;
    p0 = &p0 [-1];		/* get address of header */

    q = &AVAIL;

    while (1)
    {
        p = q->next;

        if ((p == NULL) || (p > (__memp__) memp))
            break;

        q = p;
    }

    if ((p != NULL) && ((((char *)memp) + p0->len) == (char *)p))
    {
        p0->len += p->len + HLEN;
        p0->next = p->next;
    }
    else
    {
        p0->next = p;
    }

    if ((((char *)q) + q->len + HLEN) == (char *)p0)
    {
        q->len += p0->len + HLEN;
        q->next = p0->next;
    }
    else
    {
        q->next = p0;
    }
}

#undef _MSAPI_JPEG_MEMORY_C_

