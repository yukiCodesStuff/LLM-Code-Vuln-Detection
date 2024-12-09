static void kvm_mips_check_asids(struct kvm_vcpu *vcpu)
{
	struct mips_coproc *cop0 = vcpu->arch.cop0;
	int cpu = smp_processor_id();
	unsigned int gasid;

	/*
	 * Lazy host ASID regeneration for guest user mode.
						vcpu);
			vcpu->arch.guest_user_asid[cpu] =
				vcpu->arch.guest_user_mm.context.asid[cpu];
			vcpu->arch.last_user_gasid = gasid;
		}
	}
}