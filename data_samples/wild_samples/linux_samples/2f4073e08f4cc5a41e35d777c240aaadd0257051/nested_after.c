{
	if (vmx->nested.nested_run_pending &&
	    (vmcs12->vm_entry_controls & VM_ENTRY_LOAD_IA32_EFER))
		return vmcs12->guest_ia32_efer;
	else if (vmcs12->vm_entry_controls & VM_ENTRY_IA32E_MODE)
		return vmx->vcpu.arch.efer | (EFER_LMA | EFER_LME);
	else
		return vmx->vcpu.arch.efer & ~(EFER_LMA | EFER_LME);
}

static void prepare_vmcs02_constant_state(struct vcpu_vmx *vmx)
{
	struct kvm *kvm = vmx->vcpu.kvm;

	/*
	 * If vmcs02 hasn't been initialized, set the constant vmcs02 state
	 * according to L0's settings (vmcs12 is irrelevant here).  Host
	 * fields that come from L0 and are not constant, e.g. HOST_CR3,
	 * will be set as needed prior to VMLAUNCH/VMRESUME.
	 */
	if (vmx->nested.vmcs02_initialized)
		return;
	vmx->nested.vmcs02_initialized = true;

	/*
	 * We don't care what the EPTP value is we just need to guarantee
	 * it's valid so we don't get a false positive when doing early
	 * consistency checks.
	 */
	if (enable_ept && nested_early_check)
		vmcs_write64(EPT_POINTER,
			     construct_eptp(&vmx->vcpu, 0, PT64_ROOT_4LEVEL));

	/* All VMFUNCs are currently emulated through L0 vmexits.  */
	if (cpu_has_vmx_vmfunc())
		vmcs_write64(VM_FUNCTION_CONTROL, 0);

	if (cpu_has_vmx_posted_intr())
		vmcs_write16(POSTED_INTR_NV, POSTED_INTR_NESTED_VECTOR);

	if (cpu_has_vmx_msr_bitmap())
		vmcs_write64(MSR_BITMAP, __pa(vmx->nested.vmcs02.msr_bitmap));

	/*
	 * PML is emulated for L2, but never enabled in hardware as the MMU
	 * handles A/D emulation.  Disabling PML for L2 also avoids having to
	 * deal with filtering out L2 GPAs from the buffer.
	 */
	if (enable_pml) {
		vmcs_write64(PML_ADDRESS, 0);
		vmcs_write16(GUEST_PML_INDEX, -1);
	}

	if (cpu_has_vmx_encls_vmexit())
		vmcs_write64(ENCLS_EXITING_BITMAP, INVALID_GPA);

	if (kvm_notify_vmexit_enabled(kvm))
		vmcs_write32(NOTIFY_WINDOW, kvm->arch.notify_window);

	/*
	 * Set the MSR load/store lists to match L0's settings.  Only the
	 * addresses are constant (for vmcs02), the counts can change based
	 * on L2's behavior, e.g. switching to/from long mode.
	 */
	vmcs_write64(VM_EXIT_MSR_STORE_ADDR, __pa(vmx->msr_autostore.guest.val));
	vmcs_write64(VM_EXIT_MSR_LOAD_ADDR, __pa(vmx->msr_autoload.host.val));
	vmcs_write64(VM_ENTRY_MSR_LOAD_ADDR, __pa(vmx->msr_autoload.guest.val));

	vmx_set_constant_host_state(vmx);
}
	if (enable_pml) {
		vmcs_write64(PML_ADDRESS, 0);
		vmcs_write16(GUEST_PML_INDEX, -1);
	}

	if (cpu_has_vmx_encls_vmexit())
		vmcs_write64(ENCLS_EXITING_BITMAP, INVALID_GPA);

	if (kvm_notify_vmexit_enabled(kvm))
		vmcs_write32(NOTIFY_WINDOW, kvm->arch.notify_window);

	/*
	 * Set the MSR load/store lists to match L0's settings.  Only the
	 * addresses are constant (for vmcs02), the counts can change based
	 * on L2's behavior, e.g. switching to/from long mode.
	 */
	vmcs_write64(VM_EXIT_MSR_STORE_ADDR, __pa(vmx->msr_autostore.guest.val));
	vmcs_write64(VM_EXIT_MSR_LOAD_ADDR, __pa(vmx->msr_autoload.host.val));
	vmcs_write64(VM_ENTRY_MSR_LOAD_ADDR, __pa(vmx->msr_autoload.guest.val));

	vmx_set_constant_host_state(vmx);
}

static void prepare_vmcs02_early_rare(struct vcpu_vmx *vmx,
				      struct vmcs12 *vmcs12)
{
	prepare_vmcs02_constant_state(vmx);

	vmcs_write64(VMCS_LINK_POINTER, INVALID_GPA);

	if (enable_vpid) {
		if (nested_cpu_has_vpid(vmcs12) && vmx->nested.vpid02)
			vmcs_write16(VIRTUAL_PROCESSOR_ID, vmx->nested.vpid02);
		else
			vmcs_write16(VIRTUAL_PROCESSOR_ID, vmx->vpid);
	}
	switch ((u16)exit_reason.basic) {
	case EXIT_REASON_EXCEPTION_NMI:
		intr_info = vmx_get_intr_info(vcpu);
		if (is_nmi(intr_info))
			return true;
		else if (is_page_fault(intr_info))
			return true;
		return vmcs12->exception_bitmap &
				(1u << (intr_info & INTR_INFO_VECTOR_MASK));
	case EXIT_REASON_EXTERNAL_INTERRUPT:
		return nested_exit_on_intr(vcpu);
	case EXIT_REASON_TRIPLE_FAULT:
		return true;
	case EXIT_REASON_INTERRUPT_WINDOW:
		return nested_cpu_has(vmcs12, CPU_BASED_INTR_WINDOW_EXITING);
	case EXIT_REASON_NMI_WINDOW:
		return nested_cpu_has(vmcs12, CPU_BASED_NMI_WINDOW_EXITING);
	case EXIT_REASON_TASK_SWITCH:
		return true;
	case EXIT_REASON_CPUID:
		return true;
	case EXIT_REASON_HLT:
		return nested_cpu_has(vmcs12, CPU_BASED_HLT_EXITING);
	case EXIT_REASON_INVD:
		return true;
	case EXIT_REASON_INVLPG:
		return nested_cpu_has(vmcs12, CPU_BASED_INVLPG_EXITING);
	case EXIT_REASON_RDPMC:
		return nested_cpu_has(vmcs12, CPU_BASED_RDPMC_EXITING);
	case EXIT_REASON_RDRAND:
		return nested_cpu_has2(vmcs12, SECONDARY_EXEC_RDRAND_EXITING);
	case EXIT_REASON_RDSEED:
		return nested_cpu_has2(vmcs12, SECONDARY_EXEC_RDSEED_EXITING);
	case EXIT_REASON_RDTSC: case EXIT_REASON_RDTSCP:
		return nested_cpu_has(vmcs12, CPU_BASED_RDTSC_EXITING);
	case EXIT_REASON_VMREAD:
		return nested_vmx_exit_handled_vmcs_access(vcpu, vmcs12,
			vmcs12->vmread_bitmap);
	case EXIT_REASON_VMWRITE:
		return nested_vmx_exit_handled_vmcs_access(vcpu, vmcs12,
			vmcs12->vmwrite_bitmap);
	case EXIT_REASON_VMCALL: case EXIT_REASON_VMCLEAR:
	case EXIT_REASON_VMLAUNCH: case EXIT_REASON_VMPTRLD:
	case EXIT_REASON_VMPTRST: case EXIT_REASON_VMRESUME:
	case EXIT_REASON_VMOFF: case EXIT_REASON_VMON:
	case EXIT_REASON_INVEPT: case EXIT_REASON_INVVPID:
		/*
		 * VMX instructions trap unconditionally. This allows L1 to
		 * emulate them for its L2 guest, i.e., allows 3-level nesting!
		 */
		return true;
	case EXIT_REASON_CR_ACCESS:
		return nested_vmx_exit_handled_cr(vcpu, vmcs12);
	case EXIT_REASON_DR_ACCESS:
		return nested_cpu_has(vmcs12, CPU_BASED_MOV_DR_EXITING);
	case EXIT_REASON_IO_INSTRUCTION:
		return nested_vmx_exit_handled_io(vcpu, vmcs12);
	case EXIT_REASON_GDTR_IDTR: case EXIT_REASON_LDTR_TR:
		return nested_cpu_has2(vmcs12, SECONDARY_EXEC_DESC);
	case EXIT_REASON_MSR_READ:
	case EXIT_REASON_MSR_WRITE:
		return nested_vmx_exit_handled_msr(vcpu, vmcs12, exit_reason);
	case EXIT_REASON_INVALID_STATE:
		return true;
	case EXIT_REASON_MWAIT_INSTRUCTION:
		return nested_cpu_has(vmcs12, CPU_BASED_MWAIT_EXITING);
	case EXIT_REASON_MONITOR_TRAP_FLAG:
		return nested_vmx_exit_handled_mtf(vmcs12);
	case EXIT_REASON_MONITOR_INSTRUCTION:
		return nested_cpu_has(vmcs12, CPU_BASED_MONITOR_EXITING);
	case EXIT_REASON_PAUSE_INSTRUCTION:
		return nested_cpu_has(vmcs12, CPU_BASED_PAUSE_EXITING) ||
			nested_cpu_has2(vmcs12,
				SECONDARY_EXEC_PAUSE_LOOP_EXITING);
	case EXIT_REASON_MCE_DURING_VMENTRY:
		return true;
	case EXIT_REASON_TPR_BELOW_THRESHOLD:
		return nested_cpu_has(vmcs12, CPU_BASED_TPR_SHADOW);
	case EXIT_REASON_APIC_ACCESS:
	case EXIT_REASON_APIC_WRITE:
	case EXIT_REASON_EOI_INDUCED:
		/*
		 * The controls for "virtualize APIC accesses," "APIC-
		 * register virtualization," and "virtual-interrupt
		 * delivery" only come from vmcs12.
		 */
		return true;
	case EXIT_REASON_INVPCID:
		return
			nested_cpu_has2(vmcs12, SECONDARY_EXEC_ENABLE_INVPCID) &&
			nested_cpu_has(vmcs12, CPU_BASED_INVLPG_EXITING);
	case EXIT_REASON_WBINVD:
		return nested_cpu_has2(vmcs12, SECONDARY_EXEC_WBINVD_EXITING);
	case EXIT_REASON_XSETBV:
		return true;
	case EXIT_REASON_XSAVES: case EXIT_REASON_XRSTORS:
		/*
		 * This should never happen, since it is not possible to
		 * set XSS to a non-zero value---neither in L1 nor in L2.
		 * If if it were, XSS would have to be checked against
		 * the XSS exit bitmap in vmcs12.
		 */
		return nested_cpu_has2(vmcs12, SECONDARY_EXEC_XSAVES);
	case EXIT_REASON_UMWAIT:
	case EXIT_REASON_TPAUSE:
		return nested_cpu_has2(vmcs12,
			SECONDARY_EXEC_ENABLE_USR_WAIT_PAUSE);
	case EXIT_REASON_ENCLS:
		return nested_vmx_exit_handled_encls(vcpu, vmcs12);
	case EXIT_REASON_NOTIFY:
		/* Notify VM exit is not exposed to L1 */
		return false;
	default:
		return true;
	}
}

/*
 * Conditionally reflect a VM-Exit into L1.  Returns %true if the VM-Exit was
 * reflected into L1.
 */
bool nested_vmx_reflect_vmexit(struct kvm_vcpu *vcpu)
{
	struct vcpu_vmx *vmx = to_vmx(vcpu);
	union vmx_exit_reason exit_reason = vmx->exit_reason;
	unsigned long exit_qual;
	u32 exit_intr_info;

	WARN_ON_ONCE(vmx->nested.nested_run_pending);

	/*
	 * Late nested VM-Fail shares the same flow as nested VM-Exit since KVM
	 * has already loaded L2's state.
	 */
	if (unlikely(vmx->fail)) {
		trace_kvm_nested_vmenter_failed(
			"hardware VM-instruction error: ",
			vmcs_read32(VM_INSTRUCTION_ERROR));
		exit_intr_info = 0;
		exit_qual = 0;
		goto reflect_vmexit;
	}

	trace_kvm_nested_vmexit(vcpu, KVM_ISA_VMX);

	/* If L0 (KVM) wants the exit, it trumps L1's desires. */
	if (nested_vmx_l0_wants_exit(vcpu, exit_reason))
		return false;

	/* If L1 doesn't want the exit, handle it in L0. */
	if (!nested_vmx_l1_wants_exit(vcpu, exit_reason))
		return false;

	/*
	 * vmcs.VM_EXIT_INTR_INFO is only valid for EXCEPTION_NMI exits.  For
	 * EXTERNAL_INTERRUPT, the value for vmcs12->vm_exit_intr_info would
	 * need to be synthesized by querying the in-kernel LAPIC, but external
	 * interrupts are never reflected to L1 so it's a non-issue.
	 */
	exit_intr_info = vmx_get_intr_info(vcpu);
	if (is_exception_with_error_code(exit_intr_info)) {
		struct vmcs12 *vmcs12 = get_vmcs12(vcpu);

		vmcs12->vm_exit_intr_error_code =
			vmcs_read32(VM_EXIT_INTR_ERROR_CODE);
	}
	exit_qual = vmx_get_exit_qual(vcpu);

reflect_vmexit:
	nested_vmx_vmexit(vcpu, exit_reason.full, exit_intr_info, exit_qual);
	return true;
}

static int vmx_get_nested_state(struct kvm_vcpu *vcpu,
				struct kvm_nested_state __user *user_kvm_nested_state,
				u32 user_data_size)
{
	struct vcpu_vmx *vmx;
	struct vmcs12 *vmcs12;
	struct kvm_nested_state kvm_state = {
		.flags = 0,
		.format = KVM_STATE_NESTED_FORMAT_VMX,
		.size = sizeof(kvm_state),
		.hdr.vmx.flags = 0,
		.hdr.vmx.vmxon_pa = INVALID_GPA,
		.hdr.vmx.vmcs12_pa = INVALID_GPA,
		.hdr.vmx.preemption_timer_deadline = 0,
	};
	struct kvm_vmx_nested_state_data __user *user_vmx_nested_state =
		&user_kvm_nested_state->data.vmx[0];

	if (!vcpu)
		return kvm_state.size + sizeof(*user_vmx_nested_state);

	vmx = to_vmx(vcpu);
	vmcs12 = get_vmcs12(vcpu);

	if (nested_vmx_allowed(vcpu) &&
	    (vmx->nested.vmxon || vmx->nested.smm.vmxon)) {
		kvm_state.hdr.vmx.vmxon_pa = vmx->nested.vmxon_ptr;
		kvm_state.hdr.vmx.vmcs12_pa = vmx->nested.current_vmptr;

		if (vmx_has_valid_vmcs12(vcpu)) {
			kvm_state.size += sizeof(user_vmx_nested_state->vmcs12);

			/* 'hv_evmcs_vmptr' can also be EVMPTR_MAP_PENDING here */
			if (vmx->nested.hv_evmcs_vmptr != EVMPTR_INVALID)
				kvm_state.flags |= KVM_STATE_NESTED_EVMCS;

			if (is_guest_mode(vcpu) &&
			    nested_cpu_has_shadow_vmcs(vmcs12) &&
			    vmcs12->vmcs_link_pointer != INVALID_GPA)
				kvm_state.size += sizeof(user_vmx_nested_state->shadow_vmcs12);
		}

		if (vmx->nested.smm.vmxon)
			kvm_state.hdr.vmx.smm.flags |= KVM_STATE_NESTED_SMM_VMXON;

		if (vmx->nested.smm.guest_mode)
			kvm_state.hdr.vmx.smm.flags |= KVM_STATE_NESTED_SMM_GUEST_MODE;

		if (is_guest_mode(vcpu)) {
			kvm_state.flags |= KVM_STATE_NESTED_GUEST_MODE;

			if (vmx->nested.nested_run_pending)
				kvm_state.flags |= KVM_STATE_NESTED_RUN_PENDING;

			if (vmx->nested.mtf_pending)
				kvm_state.flags |= KVM_STATE_NESTED_MTF_PENDING;

			if (nested_cpu_has_preemption_timer(vmcs12) &&
			    vmx->nested.has_preemption_timer_deadline) {
				kvm_state.hdr.vmx.flags |=
					KVM_STATE_VMX_PREEMPTION_TIMER_DEADLINE;
				kvm_state.hdr.vmx.preemption_timer_deadline =
					vmx->nested.preemption_timer_deadline;
			}
		}
	}