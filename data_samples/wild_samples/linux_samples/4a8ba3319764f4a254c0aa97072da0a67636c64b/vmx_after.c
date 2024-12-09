{
	int r = -ENOMEM, i, msr;

	rdmsrl_safe(MSR_EFER, &host_efer);

	for (i = 0; i < ARRAY_SIZE(vmx_msr_index); ++i)
		kvm_define_shared_msr(i, vmx_msr_index[i]);

	vmx_io_bitmap_a = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_io_bitmap_a)
		return r;

	vmx_io_bitmap_b = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_io_bitmap_b)
		goto out;

	vmx_msr_bitmap_legacy = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_msr_bitmap_legacy)
		goto out1;

	vmx_msr_bitmap_legacy_x2apic =
				(unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_msr_bitmap_legacy_x2apic)
		goto out2;

	vmx_msr_bitmap_longmode = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_msr_bitmap_longmode)
		goto out3;

	vmx_msr_bitmap_longmode_x2apic =
				(unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_msr_bitmap_longmode_x2apic)
		goto out4;
	vmx_vmread_bitmap = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_vmread_bitmap)
		goto out5;

	vmx_vmwrite_bitmap = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_vmwrite_bitmap)
		goto out6;

	memset(vmx_vmread_bitmap, 0xff, PAGE_SIZE);
	memset(vmx_vmwrite_bitmap, 0xff, PAGE_SIZE);

	/*
	 * Allow direct access to the PC debug port (it is often used for I/O
	 * delays, but the vmexits simply slow things down).
	 */
	memset(vmx_io_bitmap_a, 0xff, PAGE_SIZE);
	clear_bit(0x80, vmx_io_bitmap_a);

	memset(vmx_io_bitmap_b, 0xff, PAGE_SIZE);

	memset(vmx_msr_bitmap_legacy, 0xff, PAGE_SIZE);
	memset(vmx_msr_bitmap_longmode, 0xff, PAGE_SIZE);

	if (setup_vmcs_config(&vmcs_config) < 0) {
		r = -EIO;
		goto out7;
	}
	else {
		kvm_x86_ops->hwapic_irr_update = NULL;
		kvm_x86_ops->deliver_posted_interrupt = NULL;
		kvm_x86_ops->sync_pir_to_irr = vmx_sync_pir_to_irr_dummy;
	}

	if (nested)
		nested_vmx_setup_ctls_msrs();

	vmx_disable_intercept_for_msr(MSR_FS_BASE, false);
	vmx_disable_intercept_for_msr(MSR_GS_BASE, false);
	vmx_disable_intercept_for_msr(MSR_KERNEL_GS_BASE, true);
	vmx_disable_intercept_for_msr(MSR_IA32_SYSENTER_CS, false);
	vmx_disable_intercept_for_msr(MSR_IA32_SYSENTER_ESP, false);
	vmx_disable_intercept_for_msr(MSR_IA32_SYSENTER_EIP, false);
	vmx_disable_intercept_for_msr(MSR_IA32_BNDCFGS, true);

	memcpy(vmx_msr_bitmap_legacy_x2apic,
			vmx_msr_bitmap_legacy, PAGE_SIZE);
	memcpy(vmx_msr_bitmap_longmode_x2apic,
			vmx_msr_bitmap_longmode, PAGE_SIZE);

	if (enable_apicv) {
		for (msr = 0x800; msr <= 0x8ff; msr++)
			vmx_disable_intercept_msr_read_x2apic(msr);

		/* According SDM, in x2apic mode, the whole id reg is used.
		 * But in KVM, it only use the highest eight bits. Need to
		 * intercept it */
		vmx_enable_intercept_msr_read_x2apic(0x802);
		/* TMCCT */
		vmx_enable_intercept_msr_read_x2apic(0x839);
		/* TPR */
		vmx_disable_intercept_msr_write_x2apic(0x808);
		/* EOI */
		vmx_disable_intercept_msr_write_x2apic(0x80b);
		/* SELF-IPI */
		vmx_disable_intercept_msr_write_x2apic(0x83f);
	}