
static void svm_clear_vintr(struct vcpu_svm *svm)
{
	const u32 mask = V_TPR_MASK | V_GIF_ENABLE_MASK | V_GIF_MASK | V_INTR_MASKING_MASK;
	svm_clr_intercept(svm, INTERCEPT_VINTR);

	/* Drop int_ctl fields related to VINTR injection.  */
	svm->vmcb->control.int_ctl &= mask;
	if (is_guest_mode(&svm->vcpu)) {
		svm->vmcb01.ptr->control.int_ctl &= mask;

		WARN_ON((svm->vmcb->control.int_ctl & V_TPR_MASK) !=
			(svm->nested.ctl.int_ctl & V_TPR_MASK));
		svm->vmcb->control.int_ctl |= svm->nested.ctl.int_ctl & ~mask;
	}

	vmcb_mark_dirty(svm->vmcb, VMCB_INTR);
}