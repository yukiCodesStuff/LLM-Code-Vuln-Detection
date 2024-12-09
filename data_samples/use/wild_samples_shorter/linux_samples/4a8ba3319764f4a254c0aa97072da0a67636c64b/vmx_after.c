	memset(vmx_msr_bitmap_legacy, 0xff, PAGE_SIZE);
	memset(vmx_msr_bitmap_longmode, 0xff, PAGE_SIZE);

	if (setup_vmcs_config(&vmcs_config) < 0) {
		r = -EIO;
		goto out7;
	}

	if (boot_cpu_has(X86_FEATURE_NX))
		kvm_enable_efer_bits(EFER_NX);

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

	if (enable_ept) {
		kvm_mmu_set_mask_ptes(0ull,
			(enable_ept_ad_bits) ? VMX_EPT_ACCESS_BIT : 0ull,
			(enable_ept_ad_bits) ? VMX_EPT_DIRTY_BIT : 0ull,
			0ull, VMX_EPT_EXECUTABLE_MASK);
		ept_set_mmio_spte_mask();
		kvm_enable_tdp();
	} else
		kvm_disable_tdp();

	update_ple_window_actual_max();

	return alloc_kvm_area();

out7:
	free_page((unsigned long)vmx_vmwrite_bitmap);