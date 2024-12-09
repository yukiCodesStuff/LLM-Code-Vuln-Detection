{
	int r = 1;

	switch (vcpu->arch.apf.host_apf_reason) {
	default:
		trace_kvm_page_fault(fault_address, error_code);
