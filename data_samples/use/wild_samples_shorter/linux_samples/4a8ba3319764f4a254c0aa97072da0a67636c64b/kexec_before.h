extern void reserve_crashkernel(void);
extern void machine_kexec_mask_interrupts(void);

#else /* !CONFIG_KEXEC */
static inline void crash_kexec_secondary(struct pt_regs *regs) { }

static inline int overlaps_crashkernel(unsigned long start, unsigned long size)
	return 0;
}

#endif /* CONFIG_KEXEC */
#endif /* ! __ASSEMBLY__ */
#endif /* __KERNEL__ */
#endif /* _ASM_POWERPC_KEXEC_H */