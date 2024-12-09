		  "i" (-EFAULT)						\
		: "memory");						\
	} else if (cpu_has_llsc) {					\
		loongson_llsc_mb();					\
		__asm__ __volatile__(					\
		"	.set	push				\n"	\
		"	.set	noat				\n"	\
		"	.set	push				\n"	\
		  "i" (-EFAULT)
		: "memory");
	} else if (cpu_has_llsc) {
		loongson_llsc_mb();
		__asm__ __volatile__(
		"# futex_atomic_cmpxchg_inatomic			\n"
		"	.set	push					\n"
		"	.set	noat					\n"
		: GCC_OFF_SMALL_ASM() (*uaddr), "Jr" (oldval), "Jr" (newval),
		  "i" (-EFAULT)
		: "memory");
		loongson_llsc_mb();
	} else
		return -ENOSYS;

	*uval = val;