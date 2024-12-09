	long long	st_size;
	unsigned long	st_blksize;

#if defined(__BIG_ENDIAN)
	unsigned long	__pad4;		/* future possible st_blocks high bits */
	unsigned long	st_blocks;	/* Number 512-byte blocks allocated. */
#elif defined(__LITTLE_ENDIAN)
	unsigned long	st_blocks;	/* Number 512-byte blocks allocated. */
	unsigned long	__pad4;		/* future possible st_blocks high bits */
#else
#error no endian defined