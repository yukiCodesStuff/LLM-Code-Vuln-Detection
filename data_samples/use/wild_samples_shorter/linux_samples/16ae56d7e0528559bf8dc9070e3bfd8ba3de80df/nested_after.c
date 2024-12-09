	if (!svm->nested.initialized)
		return;

	if (WARN_ON_ONCE(svm->vmcb != svm->vmcb01.ptr))
		svm_switch_vmcb(svm, &svm->vmcb01);

	svm_vcpu_free_msrpm(svm->nested.msrpm);
	svm->nested.msrpm = NULL;

	__free_page(virt_to_page(svm->nested.vmcb02.ptr));