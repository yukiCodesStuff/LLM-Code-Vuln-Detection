{
	int r = 1;

	vcpu->arch.l1tf_flush_l1d = true;
	switch (vcpu->arch.apf.host_apf_reason) {
	default:
		trace_kvm_page_fault(fault_address, error_code);

		if (kvm_event_needs_reinjection(vcpu))
			kvm_mmu_unprotect_page_virt(vcpu, fault_address);
		r = kvm_mmu_page_fault(vcpu, fault_address, error_code, insn,
				insn_len);
		break;
	case KVM_PV_REASON_PAGE_NOT_PRESENT:
		vcpu->arch.apf.host_apf_reason = 0;
		local_irq_disable();
		kvm_async_pf_task_wait(fault_address, 0);
		local_irq_enable();
		break;
	case KVM_PV_REASON_PAGE_READY:
		vcpu->arch.apf.host_apf_reason = 0;
		local_irq_disable();
		kvm_async_pf_task_wake(fault_address);
		local_irq_enable();
		break;
	}