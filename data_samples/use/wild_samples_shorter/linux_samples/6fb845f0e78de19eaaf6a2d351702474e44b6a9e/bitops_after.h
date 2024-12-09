		: "ir" (1UL << bit), GCC_OFF_SMALL_ASM() (*m));
#if defined(CONFIG_CPU_MIPSR2) || defined(CONFIG_CPU_MIPSR6)
	} else if (kernel_uses_llsc && __builtin_constant_p(bit)) {
		loongson_llsc_mb();
		do {
			__asm__ __volatile__(
			"	" __LL "%0, %1		# set_bit	\n"
			"	" __INS "%0, %3, %2, 1			\n"
		} while (unlikely(!temp));
#endif /* CONFIG_CPU_MIPSR2 || CONFIG_CPU_MIPSR6 */
	} else if (kernel_uses_llsc) {
		loongson_llsc_mb();
		do {
			__asm__ __volatile__(
			"	.set	push				\n"
			"	.set	"MIPS_ISA_ARCH_LEVEL"		\n"
		: "ir" (~(1UL << bit)));
#if defined(CONFIG_CPU_MIPSR2) || defined(CONFIG_CPU_MIPSR6)
	} else if (kernel_uses_llsc && __builtin_constant_p(bit)) {
		loongson_llsc_mb();
		do {
			__asm__ __volatile__(
			"	" __LL "%0, %1		# clear_bit	\n"
			"	" __INS "%0, $0, %2, 1			\n"
		} while (unlikely(!temp));
#endif /* CONFIG_CPU_MIPSR2 || CONFIG_CPU_MIPSR6 */
	} else if (kernel_uses_llsc) {
		loongson_llsc_mb();
		do {
			__asm__ __volatile__(
			"	.set	push				\n"
			"	.set	"MIPS_ISA_ARCH_LEVEL"		\n"
		unsigned long *m = ((unsigned long *) addr) + (nr >> SZLONG_LOG);
		unsigned long temp;

		loongson_llsc_mb();
		do {
			__asm__ __volatile__(
			"	.set	push				\n"
			"	.set	"MIPS_ISA_ARCH_LEVEL"		\n"