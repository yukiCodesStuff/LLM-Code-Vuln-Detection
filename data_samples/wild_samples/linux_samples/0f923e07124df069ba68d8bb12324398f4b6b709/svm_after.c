{
	struct vmcb_control_area *control;

	/*
	 * The following fields are ignored when AVIC is enabled
	 */
	WARN_ON(kvm_apicv_activated(svm->vcpu.kvm));

	svm_set_intercept(svm, INTERCEPT_VINTR);

	/*
	 * This is just a dummy VINTR to actually cause a vmexit to happen.
	 * Actual injection of virtual interrupts happens through EVENTINJ.
	 */
	control = &svm->vmcb->control;
	control->int_vector = 0x0;
	control->int_ctl &= ~V_INTR_PRIO_MASK;
	control->int_ctl |= V_IRQ_MASK |
		((/*control->int_vector >> 4*/ 0xf) << V_INTR_PRIO_SHIFT);
	vmcb_mark_dirty(svm->vmcb, VMCB_INTR);
}

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