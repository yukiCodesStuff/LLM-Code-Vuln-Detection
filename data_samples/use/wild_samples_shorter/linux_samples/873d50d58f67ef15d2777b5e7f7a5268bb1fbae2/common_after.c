	cr4_clear_bits(X86_CR4_UMIP);
}

DEFINE_STATIC_KEY_FALSE_RO(cr_pinning);
EXPORT_SYMBOL(cr_pinning);
unsigned long cr4_pinned_bits __ro_after_init;
EXPORT_SYMBOL(cr4_pinned_bits);

/*
 * Once CPU feature detection is finished (and boot params have been
 * parsed), record any of the sensitive CR bits that are set, and
 * enable CR pinning.
 */
static void __init setup_cr_pinning(void)
{
	unsigned long mask;

	mask = (X86_CR4_SMEP | X86_CR4_SMAP | X86_CR4_UMIP);
	cr4_pinned_bits = this_cpu_read(cpu_tlbstate.cr4) & mask;
	static_key_enable(&cr_pinning.key);
}

/*
 * Protection Keys are not available in 32-bit mode.
 */
static bool pku_disabled;
	enable_sep_cpu();
#endif
	cpu_detect_tlb(&boot_cpu_data);
	setup_cr_pinning();
}

void identify_secondary_cpu(struct cpuinfo_x86 *c)
{