	mark_dirty(svm->vmcb, VMCB_INTERCEPTS);
}

static void svm_adjust_tsc_offset_guest(struct kvm_vcpu *vcpu, s64 adjustment)
{
	struct vcpu_svm *svm = to_svm(vcpu);

	svm->vmcb->control.tsc_offset += adjustment;
	if (is_guest_mode(vcpu))
		svm->nested.hsave->control.tsc_offset += adjustment;
	else
		trace_kvm_write_tsc_offset(vcpu->vcpu_id,
				     svm->vmcb->control.tsc_offset - adjustment,
				     svm->vmcb->control.tsc_offset);

	mark_dirty(svm->vmcb, VMCB_INTERCEPTS);
}

static void avic_init_vmcb(struct vcpu_svm *svm)
{
	struct vmcb *vmcb = svm->vmcb;
	struct kvm_arch *vm_data = &svm->vcpu.kvm->arch;
	return 0;
}

static u64 svm_read_l1_tsc(struct kvm_vcpu *vcpu, u64 host_tsc)
{
	struct vmcb *vmcb = get_host_vmcb(to_svm(vcpu));
	return vmcb->control.tsc_offset + host_tsc;
}

static int svm_get_msr(struct kvm_vcpu *vcpu, struct msr_data *msr_info)
{
	struct vcpu_svm *svm = to_svm(vcpu);

	.has_wbinvd_exit = svm_has_wbinvd_exit,

	.write_tsc_offset = svm_write_tsc_offset,
	.adjust_tsc_offset_guest = svm_adjust_tsc_offset_guest,
	.read_l1_tsc = svm_read_l1_tsc,

	.set_tdp_cr3 = set_tdp_cr3,

	.check_intercept = svm_check_intercept,