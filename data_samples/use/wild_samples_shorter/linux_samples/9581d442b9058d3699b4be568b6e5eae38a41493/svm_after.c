	sync_lapic_to_cr8(vcpu);

	save_host_msrs(vcpu);
	savesegment(fs, fs_selector);
	savesegment(gs, gs_selector);
	ldt_selector = kvm_read_ldt();
	svm->vmcb->save.cr2 = vcpu->arch.cr2;
	/* required for live migration with NPT */
	if (npt_enabled)
	vcpu->arch.regs[VCPU_REGS_RSP] = svm->vmcb->save.rsp;
	vcpu->arch.regs[VCPU_REGS_RIP] = svm->vmcb->save.rip;

	load_host_msrs(vcpu);
	loadsegment(fs, fs_selector);
#ifdef CONFIG_X86_64
	load_gs_index(gs_selector);
	wrmsrl(MSR_KERNEL_GS_BASE, current->thread.gs);
#else
	loadsegment(gs, gs_selector);
#endif
	kvm_load_ldt(ldt_selector);

	reload_tss(vcpu);

	local_irq_disable();