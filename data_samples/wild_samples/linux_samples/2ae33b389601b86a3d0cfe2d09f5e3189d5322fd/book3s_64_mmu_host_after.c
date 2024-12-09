{
	struct kvmppc_vcpu_book3s *vcpu3s = to_book3s(vcpu);
	int err;

	err = __init_new_context();
	if (err < 0)
		return -1;
	vcpu3s->context_id[0] = err;

	vcpu3s->proto_vsid_max = ((vcpu3s->context_id[0] + 1)
				  << ESID_BITS) - 1;
	vcpu3s->proto_vsid_first = vcpu3s->context_id[0] << ESID_BITS;
	vcpu3s->proto_vsid_next = vcpu3s->proto_vsid_first;

	kvmppc_mmu_hpte_init(vcpu);

	return 0;
}