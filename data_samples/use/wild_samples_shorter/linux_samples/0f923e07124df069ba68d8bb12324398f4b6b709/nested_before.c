
static void nested_vmcb02_prepare_control(struct vcpu_svm *svm)
{
	const u32 mask = V_INTR_MASKING_MASK | V_GIF_ENABLE_MASK | V_GIF_MASK;
	struct kvm_vcpu *vcpu = &svm->vcpu;

	/*
	 * Filled at exit: exit_code, exit_code_hi, exit_info_1, exit_info_2,
		vcpu->arch.l1_tsc_offset + svm->nested.ctl.tsc_offset;

	svm->vmcb->control.int_ctl             =
		(svm->nested.ctl.int_ctl & ~mask) |
		(svm->vmcb01.ptr->control.int_ctl & mask);

	svm->vmcb->control.virt_ext            = svm->nested.ctl.virt_ext;
	svm->vmcb->control.int_vector          = svm->nested.ctl.int_vector;
	svm->vmcb->control.int_state           = svm->nested.ctl.int_state;