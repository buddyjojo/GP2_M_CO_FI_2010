/*
 * cachectl.h -- defines for MIPS cache control system calls
 *
 * Copyright (C) 1994, 1995, 1996 by Ralf Baechle
 */
#ifndef	__ASM_MIPS_CACHECTL
#define	__ASM_MIPS_CACHECTL

/*
 * Options for cacheflush system call
 */
#define	ICACHE	(1<<0)		/* flush instruction cache        */
#define	DCACHE	(1<<1)		/* writeback and flush data cache */
#define	BCACHE	(ICACHE|DCACHE)	/* flush both caches              */

/*
 * Caching modes for the cachectl(2) call
 *
 * cachectl(2) is currently not supported and returns ENOSYS.
 */
#define CACHEABLE	0	/* make pages cacheable */
#define UNCACHEABLE	1	/* make pages uncacheable */

#define cache32_unroll32(base,op)					\
	__asm__ __volatile__(						\
	"	.set push					\n"	\
	"	.set noreorder					\n"	\
	"	.set mips3					\n"	\
	"	cache %1, 0x000(%0); cache %1, 0x020(%0)	\n"	\
	"	cache %1, 0x040(%0); cache %1, 0x060(%0)	\n"	\
	"	cache %1, 0x080(%0); cache %1, 0x0a0(%0)	\n"	\
	"	cache %1, 0x0c0(%0); cache %1, 0x0e0(%0)	\n"	\
	"	cache %1, 0x100(%0); cache %1, 0x120(%0)	\n"	\
	"	cache %1, 0x140(%0); cache %1, 0x160(%0)	\n"	\
	"	cache %1, 0x180(%0); cache %1, 0x1a0(%0)	\n"	\
	"	cache %1, 0x1c0(%0); cache %1, 0x1e0(%0)	\n"	\
	"	cache %1, 0x200(%0); cache %1, 0x220(%0)	\n"	\
	"	cache %1, 0x240(%0); cache %1, 0x260(%0)	\n"	\
	"	cache %1, 0x280(%0); cache %1, 0x2a0(%0)	\n"	\
	"	cache %1, 0x2c0(%0); cache %1, 0x2e0(%0)	\n"	\
	"	cache %1, 0x300(%0); cache %1, 0x320(%0)	\n"	\
	"	cache %1, 0x340(%0); cache %1, 0x360(%0)	\n"	\
	"	cache %1, 0x380(%0); cache %1, 0x3a0(%0)	\n"	\
	"	cache %1, 0x3c0(%0); cache %1, 0x3e0(%0)	\n"	\
	"	.set pop					\n"	\
		:							\
		: "r" (base),						\
		  "i" (op));

#define JUMP_TO_ALIGN(order) \
	__asm__ __volatile__( \
		"b\t1f\n\t" \
		".align\t" #order "\n\t" \
		"1:\n\t" \
		)
#define CACHE32_UNROLL32_ALIGN	JUMP_TO_ALIGN(10) /* 32 * 32 = 1024 */
#define CACHE32_UNROLL32_ALIGN2	JUMP_TO_ALIGN(11)

#endif	/* __ASM_MIPS_CACHECTL */
