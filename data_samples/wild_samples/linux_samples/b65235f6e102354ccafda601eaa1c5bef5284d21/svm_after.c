{
	int i;

	if (intercept == svm->x2avic_msrs_intercepted)
		return;

	if (!x2avic_enabled)
		return;

	for (i = 0; i < MAX_DIRECT_ACCESS_MSRS; i++) {
		int index = direct_access_msrs[i].index;

		if ((index < APIC_BASE_MSR) ||
		    (index > APIC_BASE_MSR + 0xff))
			continue;
		set_msr_interception(&svm->vcpu, svm->msrpm, index,
				     !intercept, !intercept);
	}