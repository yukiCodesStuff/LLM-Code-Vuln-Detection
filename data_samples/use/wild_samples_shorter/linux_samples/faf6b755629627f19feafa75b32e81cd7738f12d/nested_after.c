		(svm->nested.ctl.int_ctl & int_ctl_vmcb12_bits) |
		(svm->vmcb01.ptr->control.int_ctl & int_ctl_vmcb01_bits);

	svm->vmcb->control.int_vector          = svm->nested.ctl.int_vector;
	svm->vmcb->control.int_state           = svm->nested.ctl.int_state;
	svm->vmcb->control.event_inj           = svm->nested.ctl.event_inj;
	svm->vmcb->control.event_inj_err       = svm->nested.ctl.event_inj_err;