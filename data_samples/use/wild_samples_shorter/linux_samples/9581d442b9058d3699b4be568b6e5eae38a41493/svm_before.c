	sync_lapic_to_cr8(vcpu);

	save_host_msrs(vcpu);
	fs_selector = kvm_read_fs();
	gs_selector = kvm_read_gs();
	ldt_selector = kvm_read_ldt();
	svm->vmcb->save.cr2 = vcpu->arch.cr2;
	/* required for live migration with NPT */
	if (npt_enabled)
	vcpu->arch.regs[VCPU_REGS_RSP] = svm->vmcb->save.rsp;
	vcpu->arch.regs[VCPU_REGS_RIP] = svm->vmcb->save.rip;

	kvm_load_fs(fs_selector);
	kvm_load_gs(gs_selector);
	kvm_load_ldt(ldt_selector);
	load_host_msrs(vcpu);

	reload_tss(vcpu);

	local_irq_disable();