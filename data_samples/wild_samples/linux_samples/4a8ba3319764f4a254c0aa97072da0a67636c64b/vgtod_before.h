{
	unsigned int p;

	/*
	 * Load per CPU data from GDT.  LSL is faster than RDTSCP and
	 * works on all CPUs.
	 */
	asm("lsl %1,%0" : "=r" (p) : "r" (__PER_CPU_SEG));

	return p;
}

#endif /* CONFIG_X86_64 */

#endif /* _ASM_X86_VGTOD_H */