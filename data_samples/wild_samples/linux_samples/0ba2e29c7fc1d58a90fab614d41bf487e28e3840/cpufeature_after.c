	if (__kpti_forced) {
		pr_info_once("kernel page table isolation forced %s by command line option\n",
			     __kpti_forced > 0 ? "ON" : "OFF");
		return __kpti_forced > 0;
	}

	/* Useful for KASLR robustness */
	if (IS_ENABLED(CONFIG_RANDOMIZE_BASE))
		return true;

	/* Don't force KPTI for CPUs that are not vulnerable */
	switch (read_cpuid_id() & MIDR_CPU_MODEL_MASK) {
	case MIDR_CAVIUM_THUNDERX2:
	case MIDR_BRCM_VULCAN:
		return false;
	}