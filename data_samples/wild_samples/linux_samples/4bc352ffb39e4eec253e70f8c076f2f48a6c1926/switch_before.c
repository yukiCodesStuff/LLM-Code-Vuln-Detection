	do {
		/* Jump in the fire! */
		exit_code = __guest_enter(vcpu, host_ctxt);

		/* And we're baaack! */
	} while (fixup_guest_exit(vcpu, &exit_code));

	if (cpus_have_const_cap(ARM64_HARDEN_BP_POST_GUEST_EXIT)) {
		u32 midr = read_cpuid_id();

		/* Apply BTAC predictors mitigation to all Falkor chips */
		if (((midr & MIDR_CPU_MODEL_MASK) == MIDR_QCOM_FALKOR) ||
		    ((midr & MIDR_CPU_MODEL_MASK) == MIDR_QCOM_FALKOR_V1)) {
			__qcom_hyp_sanitize_btac_predictors();
		}
	}