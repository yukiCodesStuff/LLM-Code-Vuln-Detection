
	vmx_switch_vmcs(vcpu, &vmx->vmcs01);

	/*
	 * If IBRS is advertised to the vCPU, KVM must flush the indirect
	 * branch predictors when transitioning from L2 to L1, as L1 expects
	 * hardware (KVM in this case) to provide separate predictor modes.
	 * Bare metal isolates VMX root (host) from VMX non-root (guest), but
	 * doesn't isolate different VMCSs, i.e. in this case, doesn't provide
	 * separate modes for L2 vs L1.
	 */
	if (guest_cpuid_has(vcpu, X86_FEATURE_SPEC_CTRL))
		indirect_branch_prediction_barrier();

	/* Update any VMCS fields that might have changed while L2 ran */
	vmcs_write32(VM_EXIT_MSR_LOAD_COUNT, vmx->msr_autoload.host.nr);
	vmcs_write32(VM_ENTRY_MSR_LOAD_COUNT, vmx->msr_autoload.guest.nr);
	vmcs_write64(TSC_OFFSET, vcpu->arch.tsc_offset);