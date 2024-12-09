struct stat64 {
	unsigned long long	st_dev;
	unsigned char	__pad0[4];
#define STAT64_HAS_BROKEN_ST_INO
	unsigned long	__st_ino;

	unsigned int	st_mode;
	unsigned int	st_nlink;

	unsigned long	st_uid;
	unsigned long	st_gid;

	unsigned long long	st_rdev;
	unsigned char	__pad3[4];

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
#endif
	unsigned long	st_atime;
	unsigned long	st_atime_nsec;

	unsigned long	st_mtime;
	unsigned long	st_mtime_nsec;

	unsigned long	st_ctime;
	unsigned long	st_ctime_nsec;

	unsigned long long	st_ino;
};

#endif  /* _ASM_M32R_STAT_H */