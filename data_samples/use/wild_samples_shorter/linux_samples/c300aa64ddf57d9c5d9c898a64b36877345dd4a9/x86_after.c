		/* ...but clean it before doing the actual write */
		vcpu->arch.time_offset = data & ~(PAGE_MASK | 1);

		/* Check that the address is 32-byte aligned. */
		if (vcpu->arch.time_offset &
				(sizeof(struct pvclock_vcpu_time_info) - 1))
			break;

		vcpu->arch.time_page =
				gfn_to_page(vcpu->kvm, data >> PAGE_SHIFT);

		if (is_error_page(vcpu->arch.time_page))