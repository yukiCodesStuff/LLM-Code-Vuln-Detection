			       int size);
	int (*store_to_eaddr)(struct kvm_vcpu *vcpu, ulong *eaddr, void *ptr,
			      int size);
};

extern struct kvmppc_ops *kvmppc_hv_ops;
extern struct kvmppc_ops *kvmppc_pr_ops;