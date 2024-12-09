{
	return PERF_SAMPLE_REGS_ABI_32;
}
#else /* CONFIG_X86_64 */
#define REG_NOSUPPORT ((1ULL << PERF_REG_X86_DS) | \
		       (1ULL << PERF_REG_X86_ES) | \
		       (1ULL << PERF_REG_X86_FS) | \
	else
		return PERF_SAMPLE_REGS_ABI_64;
}
#endif /* CONFIG_X86_32 */