 * adapt the usual convention.
 */

long compat_sys_truncate64(char __user *filename, u32 dummy, u32 low, u32 high)
{
	return sys_truncate(filename, ((loff_t)high << 32) | low);
}

long compat_sys_ftruncate64(unsigned int fd, u32 dummy, u32 low, u32 high)
{
	return sys_ftruncate(fd, ((loff_t)high << 32) | low);
}

long compat_sys_pread64(unsigned int fd, char __user *ubuf, size_t count,
			u32 dummy, u32 low, u32 high)
{
	return sys_pread64(fd, ubuf, count, ((loff_t)high << 32) | low);
}

long compat_sys_pwrite64(unsigned int fd, char __user *ubuf, size_t count,
			 u32 dummy, u32 low, u32 high)
{
	return sys_pwrite64(fd, ubuf, count, ((loff_t)high << 32) | low);
}

long compat_sys_lookup_dcookie(u32 low, u32 high, char __user *buf, size_t len)
{
	return sys_lookup_dcookie(((loff_t)high << 32) | low, buf, len);
}

long compat_sys_sync_file_range2(int fd, unsigned int flags,
				 u32 offset_lo, u32 offset_hi,
				 u32 nbytes_lo, u32 nbytes_hi)
{
	return sys_sync_file_range(fd, ((loff_t)offset_hi << 32) | offset_lo,
				   ((loff_t)nbytes_hi << 32) | nbytes_lo,
				   flags);
}

long compat_sys_fallocate(int fd, int mode,
			  u32 offset_lo, u32 offset_hi,
			  u32 len_lo, u32 len_hi)
{
	return sys_fallocate(fd, mode, ((loff_t)offset_hi << 32) | offset_lo,
			     ((loff_t)len_hi << 32) | len_lo);
}

/* Provide the compat syscall number to call mapping. */
#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (call),

/* See comments in sys.c */
#define compat_sys_fadvise64_64 sys32_fadvise64_64
#define compat_sys_readahead sys32_readahead

/* Call the assembly trampolines where necessary. */
#define compat_sys_rt_sigreturn _compat_sys_rt_sigreturn
#define sys_clone _sys_clone