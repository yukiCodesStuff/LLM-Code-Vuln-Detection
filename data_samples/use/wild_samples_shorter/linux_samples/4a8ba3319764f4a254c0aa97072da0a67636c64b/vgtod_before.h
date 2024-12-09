
	/*
	 * Load per CPU data from GDT.  LSL is faster than RDTSCP and
	 * works on all CPUs.
	 */
	asm("lsl %1,%0" : "=r" (p) : "r" (__PER_CPU_SEG));

	return p;
}
