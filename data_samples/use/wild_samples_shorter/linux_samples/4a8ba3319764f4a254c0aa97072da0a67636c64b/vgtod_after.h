
	/*
	 * Load per CPU data from GDT.  LSL is faster than RDTSCP and
	 * works on all CPUs.  This is volatile so that it orders
	 * correctly wrt barrier() and to keep gcc from cleverly
	 * hoisting it out of the calling function.
	 */
	asm volatile ("lsl %1,%0" : "=r" (p) : "r" (__PER_CPU_SEG));

	return p;
}
