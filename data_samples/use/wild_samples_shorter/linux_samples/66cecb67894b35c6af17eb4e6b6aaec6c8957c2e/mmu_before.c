
	if ((vcpu->arch.guest_user_asid[cpu] ^ asid_cache(cpu)) &
						asid_version_mask(cpu)) {
		u32 gasid = kvm_read_c0_guest_entryhi(vcpu->arch.cop0) &
				KVM_ENTRYHI_ASID;

		kvm_get_new_mmu_context(&vcpu->arch.guest_user_mm, cpu, vcpu);
		vcpu->arch.guest_user_asid[cpu] =
		    vcpu->arch.guest_user_mm.context.asid[cpu];
		vcpu->arch.last_user_gasid = gasid;
		newasid++;

		kvm_debug("[%d]: cpu_context: %#lx\n", cpu,
			  cpu_context(cpu, current->mm));