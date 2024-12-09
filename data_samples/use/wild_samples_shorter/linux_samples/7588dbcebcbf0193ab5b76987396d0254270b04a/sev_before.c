	memset(ghcb->save.valid_bitmap, 0, sizeof(ghcb->save.valid_bitmap));
}

static int sev_es_validate_vmgexit(struct vcpu_svm *svm)
{
	struct kvm_vcpu *vcpu;
	struct ghcb *ghcb;
	u64 exit_code;
	u64 reason;

	 * Retrieve the exit code now even though it may not be marked valid
	 * as it could help with debugging.
	 */
	exit_code = ghcb_get_sw_exit_code(ghcb);

	/* Only GHCB Usage code 0 is supported */
	if (ghcb->ghcb_usage) {
		reason = GHCB_ERR_INVALID_USAGE;
	    !kvm_ghcb_sw_exit_info_2_is_valid(svm))
		goto vmgexit_err;

	switch (ghcb_get_sw_exit_code(ghcb)) {
	case SVM_EXIT_READ_DR7:
		break;
	case SVM_EXIT_WRITE_DR7:
		if (!kvm_ghcb_rax_is_valid(svm))
		if (!kvm_ghcb_rax_is_valid(svm) ||
		    !kvm_ghcb_rcx_is_valid(svm))
			goto vmgexit_err;
		if (ghcb_get_rax(ghcb) == 0xd)
			if (!kvm_ghcb_xcr0_is_valid(svm))
				goto vmgexit_err;
		break;
	case SVM_EXIT_INVD:
		break;
	case SVM_EXIT_IOIO:
		if (ghcb_get_sw_exit_info_1(ghcb) & SVM_IOIO_STR_MASK) {
			if (!kvm_ghcb_sw_scratch_is_valid(svm))
				goto vmgexit_err;
		} else {
			if (!(ghcb_get_sw_exit_info_1(ghcb) & SVM_IOIO_TYPE_MASK))
				if (!kvm_ghcb_rax_is_valid(svm))
					goto vmgexit_err;
		}
		break;
	case SVM_EXIT_MSR:
		if (!kvm_ghcb_rcx_is_valid(svm))
			goto vmgexit_err;
		if (ghcb_get_sw_exit_info_1(ghcb)) {
			if (!kvm_ghcb_rax_is_valid(svm) ||
			    !kvm_ghcb_rdx_is_valid(svm))
				goto vmgexit_err;
		}
	return 0;

vmgexit_err:
	vcpu = &svm->vcpu;

	if (reason == GHCB_ERR_INVALID_USAGE) {
		vcpu_unimpl(vcpu, "vmgexit: ghcb usage %#x is not valid\n",
			    ghcb->ghcb_usage);
	} else if (reason == GHCB_ERR_INVALID_EVENT) {

	trace_kvm_vmgexit_enter(vcpu->vcpu_id, ghcb);

	exit_code = ghcb_get_sw_exit_code(ghcb);

	sev_es_sync_from_ghcb(svm);
	ret = sev_es_validate_vmgexit(svm);
	if (ret)
		return ret;
	ghcb_set_sw_exit_info_1(ghcb, 0);
	ghcb_set_sw_exit_info_2(ghcb, 0);

	switch (exit_code) {
	case SVM_VMGEXIT_MMIO_READ:
		ret = setup_vmgexit_scratch(svm, true, control->exit_info_2);
		if (ret)