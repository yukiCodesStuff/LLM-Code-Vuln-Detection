{
	u32 func_id = smccc_get_function(vcpu);
	u32 val = PSCI_RET_NOT_SUPPORTED;
	u32 feature;

	switch (func_id) {
	case ARM_SMCCC_VERSION_FUNC_ID:
		val = ARM_SMCCC_VERSION_1_1;
		break;
	case ARM_SMCCC_ARCH_FEATURES_FUNC_ID:
		feature = smccc_get_arg1(vcpu);
		switch(feature) {
		case ARM_SMCCC_ARCH_WORKAROUND_1:
			if (kvm_arm_harden_branch_predictor())
				val = 0;
			break;
		}
		break;
	default:
		return kvm_psci_call(vcpu);
	}