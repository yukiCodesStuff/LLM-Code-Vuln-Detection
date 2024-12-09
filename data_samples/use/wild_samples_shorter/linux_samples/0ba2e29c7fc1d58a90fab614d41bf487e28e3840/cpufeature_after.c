	if (IS_ENABLED(CONFIG_RANDOMIZE_BASE))
		return true;

	/* Don't force KPTI for CPUs that are not vulnerable */
	switch (read_cpuid_id() & MIDR_CPU_MODEL_MASK) {
	case MIDR_CAVIUM_THUNDERX2:
	case MIDR_BRCM_VULCAN:
		return false;
	}

	/* Defer to CPU feature registers */
	return !cpuid_feature_extract_unsigned_field(pfr0,
						     ID_AA64PFR0_CSV3_SHIFT);
}