{
	return current_thread_info()->status & TS_COMPAT;
}

extern int compat_setup_rt_frame(int sig, struct k_sigaction *ka,
				 siginfo_t *info, sigset_t *set,
				 struct pt_regs *regs);

/* Compat syscalls. */
struct compat_siginfo;
struct compat_sigaltstack;
long compat_sys_rt_sigreturn(void);
long compat_sys_truncate64(char __user *filename, u32 dummy, u32 low, u32 high);
long compat_sys_ftruncate64(unsigned int fd, u32 dummy, u32 low, u32 high);
long compat_sys_pread64(unsigned int fd, char __user *ubuf, size_t count,
			u32 dummy, u32 low, u32 high);
long compat_sys_pwrite64(unsigned int fd, char __user *ubuf, size_t count,
			 u32 dummy, u32 low, u32 high);
long compat_sys_lookup_dcookie(u32 low, u32 high, char __user *buf, size_t len);
long compat_sys_sync_file_range2(int fd, unsigned int flags,
				 u32 offset_lo, u32 offset_hi,
				 u32 nbytes_lo, u32 nbytes_hi);
long compat_sys_fallocate(int fd, int mode,
			  u32 offset_lo, u32 offset_hi,
			  u32 len_lo, u32 len_hi);
long compat_sys_llseek(unsigned int fd, unsigned int offset_high,
		       unsigned int offset_low, loff_t __user * result,
		       unsigned int origin);

/* Assembly trampoline to avoid clobbering r0. */
long _compat_sys_rt_sigreturn(void);

#endif /* _ASM_TILE_COMPAT_H */