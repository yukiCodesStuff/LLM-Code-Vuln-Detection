
	if ((vcpu->arch.guest_user_asid[cpu] ^ asid_cache(cpu)) &
						asid_version_mask(cpu)) {
		kvm_get_new_mmu_context(&vcpu->arch.guest_user_mm, cpu, vcpu);
		vcpu->arch.guest_user_asid[cpu] =
		    vcpu->arch.guest_user_mm.context.asid[cpu];
		newasid++;

		kvm_debug("[%d]: cpu_context: %#lx\n", cpu,
			  cpu_context(cpu, current->mm));