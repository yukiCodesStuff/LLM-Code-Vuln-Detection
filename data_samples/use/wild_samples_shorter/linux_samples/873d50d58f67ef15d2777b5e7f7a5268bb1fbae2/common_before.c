	cr4_clear_bits(X86_CR4_UMIP);
}

/*
 * Protection Keys are not available in 32-bit mode.
 */
static bool pku_disabled;
	enable_sep_cpu();
#endif
	cpu_detect_tlb(&boot_cpu_data);
}

void identify_secondary_cpu(struct cpuinfo_x86 *c)
{