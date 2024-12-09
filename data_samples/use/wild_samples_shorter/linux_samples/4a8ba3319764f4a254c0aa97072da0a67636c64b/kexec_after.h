extern void reserve_crashkernel(void);
extern void machine_kexec_mask_interrupts(void);

static inline bool kdump_in_progress(void)
{
	return crashing_cpu >= 0;
}

#else /* !CONFIG_KEXEC */
static inline void crash_kexec_secondary(struct pt_regs *regs) { }

static inline int overlaps_crashkernel(unsigned long start, unsigned long size)
	return 0;
}

static inline bool kdump_in_progress(void)
{
	return false;
}

#endif /* CONFIG_KEXEC */
#endif /* ! __ASSEMBLY__ */
#endif /* __KERNEL__ */
#endif /* _ASM_POWERPC_KEXEC_H */