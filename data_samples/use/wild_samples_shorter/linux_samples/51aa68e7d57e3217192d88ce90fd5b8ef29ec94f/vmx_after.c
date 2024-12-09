	if (exec_control & CPU_BASED_TPR_SHADOW) {
		vmcs_write64(VIRTUAL_APIC_PAGE_ADDR, -1ull);
		vmcs_write32(TPR_THRESHOLD, vmcs12->tpr_threshold);
	} else {
#ifdef CONFIG_X86_64
		exec_control |= CPU_BASED_CR8_LOAD_EXITING |
				CPU_BASED_CR8_STORE_EXITING;
#endif
	}

	/*
	 * Merging of IO bitmap not currently supported.