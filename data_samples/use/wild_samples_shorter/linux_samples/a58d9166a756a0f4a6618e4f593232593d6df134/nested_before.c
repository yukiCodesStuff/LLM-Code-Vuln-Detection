	return true;
}

static bool nested_vmcb_checks(struct vcpu_svm *svm, struct vmcb *vmcb12)
{
	bool vmcb12_lma;

	if ((vmcb12->save.efer & EFER_SVME) == 0)
	if (kvm_valid_cr4(&svm->vcpu, vmcb12->save.cr4))
		return false;

	return nested_vmcb_check_controls(&vmcb12->control);
}

static void load_nested_vmcb_control(struct vcpu_svm *svm,
				     struct vmcb_control_area *control)
	int ret;

	svm->nested.vmcb12_gpa = vmcb12_gpa;
	load_nested_vmcb_control(svm, &vmcb12->control);
	nested_prepare_vmcb_save(svm, vmcb12);
	nested_prepare_vmcb_control(svm);

	ret = nested_svm_load_cr3(&svm->vcpu, vmcb12->save.cr3,
	if (WARN_ON_ONCE(!svm->nested.initialized))
		return -EINVAL;

	if (!nested_vmcb_checks(svm, vmcb12)) {
		vmcb12->control.exit_code    = SVM_EXIT_ERR;
		vmcb12->control.exit_code_hi = 0;
		vmcb12->control.exit_info_1  = 0;
		vmcb12->control.exit_info_2  = 0;