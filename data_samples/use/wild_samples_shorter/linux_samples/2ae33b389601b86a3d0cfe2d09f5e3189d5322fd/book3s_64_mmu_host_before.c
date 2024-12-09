	vcpu3s->context_id[0] = err;

	vcpu3s->proto_vsid_max = ((vcpu3s->context_id[0] + 1)
				  << USER_ESID_BITS) - 1;
	vcpu3s->proto_vsid_first = vcpu3s->context_id[0] << USER_ESID_BITS;
	vcpu3s->proto_vsid_next = vcpu3s->proto_vsid_first;

	kvmppc_mmu_hpte_init(vcpu);
