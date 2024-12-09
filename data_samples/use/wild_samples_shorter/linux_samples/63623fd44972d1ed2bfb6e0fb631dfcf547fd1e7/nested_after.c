 * or KVM_SET_NESTED_STATE).  Otherwise it's called from vmlaunch/vmresume.
 *
 * Returns:
 *	NVMX_VMENTRY_SUCCESS: Entered VMX non-root mode
 *	NVMX_VMENTRY_VMFAIL:  Consistency check VMFail
 *	NVMX_VMENTRY_VMEXIT:  Consistency check VMExit
 *	NVMX_VMENTRY_KVM_INTERNAL_ERROR: KVM internal error
 */
enum nvmx_vmentry_status nested_vmx_enter_non_root_mode(struct kvm_vcpu *vcpu,
							bool from_vmentry)
{
	unsigned long exit_qual;
	bool block_nested_events =
	    vmx->nested.nested_run_pending || kvm_event_needs_reinjection(vcpu);
	bool mtf_pending = vmx->nested.mtf_pending;
	struct kvm_lapic *apic = vcpu->arch.apic;

	/*
	 * Clear the MTF state. If a higher priority VM-exit is delivered first,
	 * this state is discarded.
	 */
	vmx->nested.mtf_pending = false;

	if (lapic_in_kernel(vcpu) &&
		test_bit(KVM_APIC_INIT, &apic->pending_events)) {
		if (block_nested_events)
			return -EBUSY;
		return 0;
	}

	/*
	 * Process any exceptions that are not debug traps before MTF.
	 */
	if (vcpu->arch.exception.pending &&
	    !vmx_pending_dbg_trap(vcpu) &&
	    nested_vmx_check_exception(vcpu, &exit_qual)) {
		if (block_nested_events)
			return -EBUSY;
		nested_vmx_inject_exception_vmexit(vcpu, exit_qual);
		return 0;
	}

	if (mtf_pending) {
		if (block_nested_events)
			return -EBUSY;
		nested_vmx_update_pending_dbg(vcpu);
		nested_vmx_vmexit(vcpu, EXIT_REASON_MONITOR_TRAP_FLAG, 0, 0);
		return 0;
	}

	if (vcpu->arch.exception.pending &&
	    nested_vmx_check_exception(vcpu, &exit_qual)) {
		if (block_nested_events)
			return -EBUSY;
		nested_vmx_inject_exception_vmexit(vcpu, exit_qual);
		return 0;
	return 1;
}

/*
 * Return true if an IO instruction with the specified port and size should cause
 * a VM-exit into L1.
 */
bool nested_vmx_check_io_bitmaps(struct kvm_vcpu *vcpu, unsigned int port,
				 int size)
{
	struct vmcs12 *vmcs12 = get_vmcs12(vcpu);
	gpa_t bitmap, last_bitmap;
	u8 b;

	last_bitmap = (gpa_t)-1;
	b = -1;

	while (size > 0) {
	return false;
}

static bool nested_vmx_exit_handled_io(struct kvm_vcpu *vcpu,
				       struct vmcs12 *vmcs12)
{
	unsigned long exit_qualification;
	unsigned short port;
	int size;

	if (!nested_cpu_has(vmcs12, CPU_BASED_USE_IO_BITMAPS))
		return nested_cpu_has(vmcs12, CPU_BASED_UNCOND_IO_EXITING);

	exit_qualification = vmcs_readl(EXIT_QUALIFICATION);

	port = exit_qualification >> 16;
	size = (exit_qualification & 7) + 1;

	return nested_vmx_check_io_bitmaps(vcpu, port, size);
}

/*
 * Return 1 if we should exit from L2 to L1 to handle an MSR access,
 * rather than handle it ourselves in L0. I.e., check whether L1 expressed
 * disinterest in the current event (read or write a specific MSR) by using an
 * MSR bitmap. This may be the case even when L0 doesn't use MSR bitmaps.
 */

			if (vmx->nested.nested_run_pending)
				kvm_state.flags |= KVM_STATE_NESTED_RUN_PENDING;

			if (vmx->nested.mtf_pending)
				kvm_state.flags |= KVM_STATE_NESTED_MTF_PENDING;
		}
	}

	if (user_data_size < kvm_state.size)
	vmx->nested.nested_run_pending =
		!!(kvm_state->flags & KVM_STATE_NESTED_RUN_PENDING);

	vmx->nested.mtf_pending =
		!!(kvm_state->flags & KVM_STATE_NESTED_MTF_PENDING);

	ret = -EINVAL;
	if (nested_cpu_has_shadow_vmcs(vmcs12) &&
	    vmcs12->vmcs_link_pointer != -1ull) {
		struct vmcs12 *shadow_vmcs12 = get_shadow_vmcs12(vcpu);
 * bit in the high half is on if the corresponding bit in the control field
 * may be on. See also vmx_control_verify().
 */
void nested_vmx_setup_ctls_msrs(struct nested_vmx_msrs *msrs, u32 ept_caps)
{
	/*
	 * Note that as a general rule, the high half of the MSRs (bits in
	 * the control fields which may be 1) should be initialized by the
		PIN_BASED_EXT_INTR_MASK |
		PIN_BASED_NMI_EXITING |
		PIN_BASED_VIRTUAL_NMIS |
		(enable_apicv ? PIN_BASED_POSTED_INTR : 0);
	msrs->pinbased_ctls_high |=
		PIN_BASED_ALWAYSON_WITHOUT_TRUE_MSR |
		PIN_BASED_VMX_PREEMPTION_TIMER;
