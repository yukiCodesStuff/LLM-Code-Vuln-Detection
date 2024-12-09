	if (IS_ENABLED(CONFIG_RANDOMIZE_BASE))
		return true;

	/* Defer to CPU feature registers */
	return !cpuid_feature_extract_unsigned_field(pfr0,
						     ID_AA64PFR0_CSV3_SHIFT);
}