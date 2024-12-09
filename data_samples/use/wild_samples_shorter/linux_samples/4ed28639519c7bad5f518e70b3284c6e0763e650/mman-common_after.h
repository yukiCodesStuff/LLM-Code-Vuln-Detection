#else
# define MAP_UNINITIALIZED 0x0		/* Don't support this flag */
#endif

/* 0x0100 - 0x80000 flags are defined in asm-generic/mman.h */
#define MAP_FIXED_NOREPLACE	0x100000	/* MAP_FIXED which doesn't unmap underlying mapping */

/*
 * Flags for mlock
 */