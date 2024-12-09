{
	unsigned long *msr_bitmap;

	if (is_guest_mode(vcpu))
		msr_bitmap = vmx_msr_bitmap_nested;
	else if (cpu_has_secondary_exec_ctrls() &&
		 (vmcs_read32(SECONDARY_VM_EXEC_CONTROL) &
		  SECONDARY_EXEC_VIRTUALIZE_X2APIC_MODE)) {
		if (is_long_mode(vcpu))
			msr_bitmap = vmx_msr_bitmap_longmode_x2apic;
		else
			msr_bitmap = vmx_msr_bitmap_legacy_x2apic;
	} else {
		if (is_long_mode(vcpu))
			msr_bitmap = vmx_msr_bitmap_longmode;
		else
			msr_bitmap = vmx_msr_bitmap_legacy;
	}

	vmcs_write64(MSR_BITMAP, __pa(msr_bitmap));
}
{
	struct vcpu_vmx *vmx = to_vmx(vcpu);

	vmcs_write32(PIN_BASED_VM_EXEC_CONTROL, vmx_pin_based_exec_ctrl(vmx));
	if (cpu_has_secondary_exec_ctrls()) {
		if (kvm_vcpu_apicv_active(vcpu))
			vmcs_set_bits(SECONDARY_VM_EXEC_CONTROL,
				      SECONDARY_EXEC_APIC_REGISTER_VIRT |
				      SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY);
		else
			vmcs_clear_bits(SECONDARY_VM_EXEC_CONTROL,
					SECONDARY_EXEC_APIC_REGISTER_VIRT |
					SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY);
	}
	if (cpu_has_vmx_tsc_scaling()) {
		kvm_has_tsc_control = true;
		kvm_max_tsc_scaling_ratio = KVM_VMX_TSC_MULTIPLIER_MAX;
		kvm_tsc_scaling_ratio_frac_bits = 48;
	}

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

	set_bit(0, vmx_vpid_bitmap); /* 0 is reserved for host */

	for (msr = 0x800; msr <= 0x8ff; msr++)
		vmx_disable_intercept_msr_read_x2apic(msr);

	/* According SDM, in x2apic mode, the whole id reg is used.  But in
	 * KVM, it only use the highest eight bits. Need to intercept it */
	vmx_enable_intercept_msr_read_x2apic(0x802);
	/* TMCCT */
	vmx_enable_intercept_msr_read_x2apic(0x839);
	/* TPR */
	vmx_disable_intercept_msr_write_x2apic(0x808);
	/* EOI */
	vmx_disable_intercept_msr_write_x2apic(0x80b);
	/* SELF-IPI */
	vmx_disable_intercept_msr_write_x2apic(0x83f);

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

	/*
	 * Only enable PML when hardware supports PML feature, and both EPT
	 * and EPT A/D bit features are enabled -- PML depends on them to work.
	 */
	if (!enable_ept || !enable_ept_ad_bits || !cpu_has_vmx_pml())
		enable_pml = 0;

	if (!enable_pml) {
		kvm_x86_ops->slot_enable_log_dirty = NULL;
		kvm_x86_ops->slot_disable_log_dirty = NULL;
		kvm_x86_ops->flush_log_dirty = NULL;
		kvm_x86_ops->enable_log_dirty_pt_masked = NULL;
	}

	kvm_set_posted_intr_wakeup_handler(wakeup_handler);

	return alloc_kvm_area();

out8:
	free_page((unsigned long)vmx_vmwrite_bitmap);
out7:
	free_page((unsigned long)vmx_vmread_bitmap);
out6:
	if (nested)
		free_page((unsigned long)vmx_msr_bitmap_nested);
out5:
	free_page((unsigned long)vmx_msr_bitmap_longmode_x2apic);
out4:
	free_page((unsigned long)vmx_msr_bitmap_longmode);
out3:
	free_page((unsigned long)vmx_msr_bitmap_legacy_x2apic);
out2:
	free_page((unsigned long)vmx_msr_bitmap_legacy);
out1:
	free_page((unsigned long)vmx_io_bitmap_b);
out:
	free_page((unsigned long)vmx_io_bitmap_a);

    return r;
}