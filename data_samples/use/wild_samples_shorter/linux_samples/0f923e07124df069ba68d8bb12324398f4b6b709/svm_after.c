
static void svm_clear_vintr(struct vcpu_svm *svm)
{
	svm_clr_intercept(svm, INTERCEPT_VINTR);

	/* Drop int_ctl fields related to VINTR injection.  */
	svm->vmcb->control.int_ctl &= ~V_IRQ_INJECTION_BITS_MASK;
	if (is_guest_mode(&svm->vcpu)) {
		svm->vmcb01.ptr->control.int_ctl &= ~V_IRQ_INJECTION_BITS_MASK;

		WARN_ON((svm->vmcb->control.int_ctl & V_TPR_MASK) !=
			(svm->nested.ctl.int_ctl & V_TPR_MASK));

		svm->vmcb->control.int_ctl |= svm->nested.ctl.int_ctl &
			V_IRQ_INJECTION_BITS_MASK;
	}

	vmcb_mark_dirty(svm->vmcb, VMCB_INTR);
}