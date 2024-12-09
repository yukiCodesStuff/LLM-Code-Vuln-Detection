static void kvm_mips_check_asids(struct kvm_vcpu *vcpu)
{
	struct mips_coproc *cop0 = vcpu->arch.cop0;
	int i, cpu = smp_processor_id();
	unsigned int gasid;

	/*
	 * Lazy host ASID regeneration for guest user mode.
						vcpu);
			vcpu->arch.guest_user_asid[cpu] =
				vcpu->arch.guest_user_mm.context.asid[cpu];
			for_each_possible_cpu(i)
				if (i != cpu)
					vcpu->arch.guest_user_asid[cpu] = 0;
			vcpu->arch.last_user_gasid = gasid;
		}
	}
}