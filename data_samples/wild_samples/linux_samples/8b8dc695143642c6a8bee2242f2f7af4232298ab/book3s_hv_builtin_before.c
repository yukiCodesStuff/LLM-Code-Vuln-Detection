				       "r" (0) : "memory");
			rb += PPC_BIT(51);	/* increment set number */
		}
	}
	asm volatile("ptesync": : :"memory");
}

void kvmppc_check_need_tlb_flush(struct kvm *kvm, int pcpu,
				 struct kvm_nested_guest *nested)
{
	cpumask_t *need_tlb_flush;

	/*
	 * On POWER9, individual threads can come in here, but the
	 * TLB is shared between the 4 threads in a core, hence
	 * invalidating on one thread invalidates for all.
	 * Thus we make all 4 threads use the same bit.
	 */
	if (cpu_has_feature(CPU_FTR_ARCH_300))
		pcpu = cpu_first_thread_sibling(pcpu);

	if (nested)
		need_tlb_flush = &nested->need_tlb_flush;
	else
		need_tlb_flush = &kvm->arch.need_tlb_flush;

	if (cpumask_test_cpu(pcpu, need_tlb_flush)) {
		flush_guest_tlb(kvm);

		/* Clear the bit after the TLB flush */
		cpumask_clear_cpu(pcpu, need_tlb_flush);
	}
}
EXPORT_SYMBOL_GPL(kvmppc_check_need_tlb_flush);