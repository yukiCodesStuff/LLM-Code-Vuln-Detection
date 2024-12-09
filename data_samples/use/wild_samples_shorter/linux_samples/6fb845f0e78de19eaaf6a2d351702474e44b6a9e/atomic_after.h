	if (kernel_uses_llsc) {						      \
		int temp;						      \
									      \
		loongson_llsc_mb();					      \
		__asm__ __volatile__(					      \
		"	.set	push					\n"   \
		"	.set	"MIPS_ISA_LEVEL"			\n"   \
		"1:	ll	%0, %1		# atomic_" #op "	\n"   \
	if (kernel_uses_llsc) {						      \
		int temp;						      \
									      \
		loongson_llsc_mb();					      \
		__asm__ __volatile__(					      \
		"	.set	push					\n"   \
		"	.set	"MIPS_ISA_LEVEL"			\n"   \
		"1:	ll	%1, %2		# atomic_" #op "_return	\n"   \
	if (kernel_uses_llsc) {						      \
		int temp;						      \
									      \
		loongson_llsc_mb();					      \
		__asm__ __volatile__(					      \
		"	.set	push					\n"   \
		"	.set	"MIPS_ISA_LEVEL"			\n"   \
		"1:	ll	%1, %2		# atomic_fetch_" #op "	\n"   \
	if (kernel_uses_llsc) {						      \
		long temp;						      \
									      \
		loongson_llsc_mb();					      \
		__asm__ __volatile__(					      \
		"	.set	push					\n"   \
		"	.set	"MIPS_ISA_LEVEL"			\n"   \
		"1:	lld	%0, %1		# atomic64_" #op "	\n"   \
	if (kernel_uses_llsc) {						      \
		long temp;						      \
									      \
		loongson_llsc_mb();					      \
		__asm__ __volatile__(					      \
		"	.set	push					\n"   \
		"	.set	"MIPS_ISA_LEVEL"			\n"   \
		"1:	lld	%1, %2		# atomic64_" #op "_return\n"  \
	if (kernel_uses_llsc) {						      \
		long temp;						      \
									      \
		loongson_llsc_mb();					      \
		__asm__ __volatile__(					      \
		"	.set	push					\n"   \
		"	.set	"MIPS_ISA_LEVEL"			\n"   \
		"1:	lld	%1, %2		# atomic64_fetch_" #op "\n"   \