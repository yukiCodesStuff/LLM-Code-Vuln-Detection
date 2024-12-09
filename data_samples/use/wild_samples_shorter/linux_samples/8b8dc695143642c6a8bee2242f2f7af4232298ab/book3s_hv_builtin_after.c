		}
	}
	asm volatile("ptesync": : :"memory");
	asm volatile(PPC_INVALIDATE_ERAT : : :"memory");
}

void kvmppc_check_need_tlb_flush(struct kvm *kvm, int pcpu,
				 struct kvm_nested_guest *nested)