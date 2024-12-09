	mark_dirty(svm->vmcb, VMCB_INTERCEPTS);
}

static void avic_init_vmcb(struct vcpu_svm *svm)
{
	struct vmcb *vmcb = svm->vmcb;
	struct kvm_arch *vm_data = &svm->vcpu.kvm->arch;
	return 0;
}

static int svm_get_msr(struct kvm_vcpu *vcpu, struct msr_data *msr_info)
{
	struct vcpu_svm *svm = to_svm(vcpu);

	.has_wbinvd_exit = svm_has_wbinvd_exit,

	.write_tsc_offset = svm_write_tsc_offset,

	.set_tdp_cr3 = set_tdp_cr3,

	.check_intercept = svm_check_intercept,