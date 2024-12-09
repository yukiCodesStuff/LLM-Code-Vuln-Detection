
	if (is_guest_mode(vcpu))
		msr_bitmap = vmx_msr_bitmap_nested;
	else if (cpu_has_secondary_exec_ctrls() &&
		 (vmcs_read32(SECONDARY_VM_EXEC_CONTROL) &
		  SECONDARY_EXEC_VIRTUALIZE_X2APIC_MODE)) {
		if (is_long_mode(vcpu))
			msr_bitmap = vmx_msr_bitmap_longmode_x2apic;
		else
			msr_bitmap = vmx_msr_bitmap_legacy_x2apic;
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

	if (cpu_has_vmx_msr_bitmap())
		vmx_set_msr_bitmap(vcpu);
}

static u32 vmx_exec_control(struct vcpu_vmx *vmx)
{

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