struct kvm_decode {
	unsigned long rt;
	bool sign_extend;
};

int kvm_handle_mmio_return(struct kvm_vcpu *vcpu, struct kvm_run *run);
int io_mem_abort(struct kvm_vcpu *vcpu, struct kvm_run *run,
		 phys_addr_t fault_ipa);

#endif	/* __ARM64_KVM_MMIO_H__ */