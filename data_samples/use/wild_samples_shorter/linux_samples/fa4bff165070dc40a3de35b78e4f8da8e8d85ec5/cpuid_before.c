	/* cpuid 7.0.edx*/
	const u32 kvm_cpuid_7_0_edx_x86_features =
		F(AVX512_4VNNIW) | F(AVX512_4FMAPS) | F(SPEC_CTRL) |
		F(SPEC_CTRL_SSBD) | F(ARCH_CAPABILITIES) | F(INTEL_STIBP);

	/* all calls to cpuid_count() should be made on the same cpu */
	get_cpu();
