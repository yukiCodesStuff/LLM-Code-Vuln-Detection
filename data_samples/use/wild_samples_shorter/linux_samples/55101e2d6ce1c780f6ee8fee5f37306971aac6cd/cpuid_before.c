	const u32 kvm_supported_word9_x86_features =
		F(FSGSBASE) | F(BMI1) | F(HLE) | F(AVX2) | F(SMEP) |
		F(BMI2) | F(ERMS) | f_invpcid | F(RTM) | f_mpx | F(RDSEED) |
		F(ADX);

	/* all calls to cpuid_count() should be made on the same cpu */
	get_cpu();
