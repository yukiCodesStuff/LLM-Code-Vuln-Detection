long compat_sys_fallocate(int fd, int mode,
			  u32 offset_lo, u32 offset_hi,
			  u32 len_lo, u32 len_hi);

/* Assembly trampoline to avoid clobbering r0. */
long _compat_sys_rt_sigreturn(void);
