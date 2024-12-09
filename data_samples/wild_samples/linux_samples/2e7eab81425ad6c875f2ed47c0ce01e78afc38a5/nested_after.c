	} else {
		/*
		 * The only expected VM-instruction error is "VM entry with
		 * invalid control field(s)." Anything else indicates a
		 * problem with L0.  And we should never get here with a
		 * VMFail of any type if early consistency checks are enabled.
		 */
		WARN_ON_ONCE(vmcs_read32(VM_INSTRUCTION_ERROR) !=
			     VMXERR_ENTRY_INVALID_CONTROL_FIELD);
		WARN_ON_ONCE(nested_early_check);
	}

	/*
	 * Drop events/exceptions that were queued for re-injection to L2
	 * (picked up via vmx_complete_interrupts()), as well as exceptions
	 * that were pending for L2.  Note, this must NOT be hoisted above
	 * prepare_vmcs12(), events/exceptions queued for re-injection need to
	 * be captured in vmcs12 (see vmcs12_save_pending_event()).
	 */
	vcpu->arch.nmi_injected = false;
	kvm_clear_exception_queue(vcpu);
	kvm_clear_interrupt_queue(vcpu);

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
	if (kvm_caps.has_tsc_control)
		vmcs_write64(TSC_MULTIPLIER, vcpu->arch.tsc_scaling_ratio);

	if (vmx->nested.l1_tpr_threshold != -1)
		vmcs_write32(TPR_THRESHOLD, vmx->nested.l1_tpr_threshold);

	if (vmx->nested.change_vmcs01_virtual_apic_mode) {
		vmx->nested.change_vmcs01_virtual_apic_mode = false;
		vmx_set_virtual_apic_mode(vcpu);
	}