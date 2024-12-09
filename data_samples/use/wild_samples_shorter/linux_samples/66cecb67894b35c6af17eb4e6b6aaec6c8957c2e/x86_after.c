
u64 kvm_read_l1_tsc(struct kvm_vcpu *vcpu, u64 host_tsc)
{
	return vcpu->arch.tsc_offset + kvm_scale_tsc(vcpu, host_tsc);
}
EXPORT_SYMBOL_GPL(kvm_read_l1_tsc);

static void kvm_vcpu_write_tsc_offset(struct kvm_vcpu *vcpu, u64 offset)
static inline void adjust_tsc_offset_guest(struct kvm_vcpu *vcpu,
					   s64 adjustment)
{
	kvm_vcpu_write_tsc_offset(vcpu, vcpu->arch.tsc_offset + adjustment);
}

static inline void adjust_tsc_offset_host(struct kvm_vcpu *vcpu, s64 adjustment)
{
	if (vcpu->arch.tsc_scaling_ratio != kvm_default_tsc_scaling_ratio)
		WARN_ON(adjustment < 0);
	adjustment = kvm_scale_tsc(vcpu, (u64) adjustment);
	adjust_tsc_offset_guest(vcpu, adjustment);
}

#ifdef CONFIG_X86_64

		/* Drop writes to this legacy MSR -- see rdmsr
		 * counterpart for further detail.
		 */
		vcpu_unimpl(vcpu, "ignored wrmsr: 0x%x data 0x%llx\n", msr, data);
		break;
	case MSR_AMD64_OSVW_ID_LENGTH:
		if (!guest_cpuid_has_osvw(vcpu))
			return 1;
		if (kvm_pmu_is_valid_msr(vcpu, msr))
			return kvm_pmu_set_msr(vcpu, msr_info);
		if (!ignore_msrs) {
			vcpu_unimpl(vcpu, "unhandled wrmsr: 0x%x data 0x%llx\n",
				    msr, data);
			return 1;
		} else {
			vcpu_unimpl(vcpu, "ignored wrmsr: 0x%x data 0x%llx\n",
				    msr, data);
			break;
		}
	}

void kvm_arch_vcpu_free(struct kvm_vcpu *vcpu)
{
	void *wbinvd_dirty_mask = vcpu->arch.wbinvd_dirty_mask;

	kvmclock_reset(vcpu);

	kvm_x86_ops->vcpu_free(vcpu);
	free_cpumask_var(wbinvd_dirty_mask);
}

struct kvm_vcpu *kvm_arch_vcpu_create(struct kvm *kvm,
						unsigned int id)