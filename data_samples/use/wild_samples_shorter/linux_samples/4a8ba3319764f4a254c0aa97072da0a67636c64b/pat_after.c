	      PAT(4, WB) | PAT(5, WC) | PAT(6, UC_MINUS) | PAT(7, UC);

	/* Boot CPU check */
	if (!boot_pat_state) {
		rdmsrl(MSR_IA32_CR_PAT, boot_pat_state);
		if (!boot_pat_state) {
			pat_disable("PAT read returns always zero, disabled.");
			return;
		}
	}

	wrmsrl(MSR_IA32_CR_PAT, pat);

	if (boot_cpu)