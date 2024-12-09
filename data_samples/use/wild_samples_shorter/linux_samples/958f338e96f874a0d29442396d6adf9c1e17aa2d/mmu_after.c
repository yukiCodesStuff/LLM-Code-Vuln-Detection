{
	int r = 1;

	vcpu->arch.l1tf_flush_l1d = true;
	switch (vcpu->arch.apf.host_apf_reason) {
	default:
		trace_kvm_page_fault(fault_address, error_code);
