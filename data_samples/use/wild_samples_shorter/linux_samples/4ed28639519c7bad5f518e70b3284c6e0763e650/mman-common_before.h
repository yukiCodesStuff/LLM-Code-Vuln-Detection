#else
# define MAP_UNINITIALIZED 0x0		/* Don't support this flag */
#endif
#define MAP_FIXED_NOREPLACE	0x80000	/* MAP_FIXED which doesn't unmap underlying mapping */

/*
 * Flags for mlock
 */