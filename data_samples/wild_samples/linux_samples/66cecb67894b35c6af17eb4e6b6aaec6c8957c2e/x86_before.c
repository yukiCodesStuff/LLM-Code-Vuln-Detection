{
	u64 tsc;

	tsc = kvm_scale_tsc(vcpu, rdtsc());

	return target_tsc - tsc;
}

u64 kvm_read_l1_tsc(struct kvm_vcpu *vcpu, u64 host_tsc)
{
	return kvm_x86_ops->read_l1_tsc(vcpu, kvm_scale_tsc(vcpu, host_tsc));
}
	} else if (!already_matched) {
		kvm->arch.nr_vcpus_matched_tsc++;
	}

	kvm_track_tsc_matching(vcpu);
	spin_unlock(&kvm->arch.pvclock_gtod_sync_lock);
}

EXPORT_SYMBOL_GPL(kvm_write_tsc);

static inline void adjust_tsc_offset_guest(struct kvm_vcpu *vcpu,
					   s64 adjustment)
{
	kvm_x86_ops->adjust_tsc_offset_guest(vcpu, adjustment);
}
		if (vcpu->vcpu_id == 0 && !msr_info->host_initiated) {
			bool tmp = (msr == MSR_KVM_SYSTEM_TIME);

			if (ka->boot_vcpu_runs_old_kvmclock != tmp)
				set_bit(KVM_REQ_MASTERCLOCK_UPDATE,
					&vcpu->requests);

			ka->boot_vcpu_runs_old_kvmclock = tmp;
		}

		vcpu->arch.time = data;
		kvm_make_request(KVM_REQ_GLOBAL_CLOCK_UPDATE, vcpu);

		/* we verify if the enable bit is set... */
		if (!(data & 1))
			break;

		gpa_offset = data & ~(PAGE_MASK | 1);

		if (kvm_gfn_to_hva_cache_init(vcpu->kvm,
		     &vcpu->arch.pv_time, data & ~1ULL,
		     sizeof(struct pvclock_vcpu_time_info)))
			vcpu->arch.pv_time_enabled = false;
		else
			vcpu->arch.pv_time_enabled = true;

		break;
	}
	case MSR_KVM_ASYNC_PF_EN:
		if (kvm_pv_enable_async_pf(vcpu, data))
			return 1;
		break;
	case MSR_KVM_STEAL_TIME:

		if (unlikely(!sched_info_on()))
			return 1;

		if (data & KVM_STEAL_RESERVED_MASK)
			return 1;

		if (kvm_gfn_to_hva_cache_init(vcpu->kvm, &vcpu->arch.st.stime,
						data & KVM_STEAL_VALID_BITS,
						sizeof(struct kvm_steal_time)))
			return 1;

		vcpu->arch.st.msr_val = data;

		if (!(data & KVM_MSR_ENABLED))
			break;

		kvm_make_request(KVM_REQ_STEAL_UPDATE, vcpu);

		break;
	case MSR_KVM_PV_EOI_EN:
		if (kvm_lapic_enable_pv_eoi(vcpu, data))
			return 1;
		break;

	case MSR_IA32_MCG_CTL:
	case MSR_IA32_MCG_STATUS:
	case MSR_IA32_MC0_CTL ... MSR_IA32_MCx_CTL(KVM_MAX_MCE_BANKS) - 1:
		return set_msr_mce(vcpu, msr, data);

	case MSR_K7_PERFCTR0 ... MSR_K7_PERFCTR3:
	case MSR_P6_PERFCTR0 ... MSR_P6_PERFCTR1:
		pr = true; /* fall through */
	case MSR_K7_EVNTSEL0 ... MSR_K7_EVNTSEL3:
	case MSR_P6_EVNTSEL0 ... MSR_P6_EVNTSEL1:
		if (kvm_pmu_is_valid_msr(vcpu, msr))
			return kvm_pmu_set_msr(vcpu, msr_info);

		if (pr || data != 0)
			vcpu_unimpl(vcpu, "disabled perfctr wrmsr: "
				    "0x%x data 0x%llx\n", msr, data);
		break;
	case MSR_K7_CLK_CTL:
		/*
		 * Ignore all writes to this no longer documented MSR.
		 * Writes are only relevant for old K7 processors,
		 * all pre-dating SVM, but a recommended workaround from
		 * AMD for these chips. It is possible to specify the
		 * affected processor models on the command line, hence
		 * the need to ignore the workaround.
		 */
		break;
	case HV_X64_MSR_GUEST_OS_ID ... HV_X64_MSR_SINT15:
	case HV_X64_MSR_CRASH_P0 ... HV_X64_MSR_CRASH_P4:
	case HV_X64_MSR_CRASH_CTL:
	case HV_X64_MSR_STIMER0_CONFIG ... HV_X64_MSR_STIMER3_COUNT:
		return kvm_hv_set_msr_common(vcpu, msr, data,
					     msr_info->host_initiated);
	case MSR_IA32_BBL_CR_CTL3:
		/* Drop writes to this legacy MSR -- see rdmsr
		 * counterpart for further detail.
		 */
		vcpu_unimpl(vcpu, "ignored wrmsr: 0x%x data %llx\n", msr, data);
		break;
	case MSR_AMD64_OSVW_ID_LENGTH:
		if (!guest_cpuid_has_osvw(vcpu))
			return 1;
		vcpu->arch.osvw.length = data;
		break;
	case MSR_AMD64_OSVW_STATUS:
		if (!guest_cpuid_has_osvw(vcpu))
			return 1;
		vcpu->arch.osvw.status = data;
		break;
	default:
		if (msr && (msr == vcpu->kvm->arch.xen_hvm_config.msr))
			return xen_hvm_config(vcpu, data);
		if (kvm_pmu_is_valid_msr(vcpu, msr))
			return kvm_pmu_set_msr(vcpu, msr_info);
		if (!ignore_msrs) {
			vcpu_unimpl(vcpu, "unhandled wrmsr: 0x%x data %llx\n",
				    msr, data);
			return 1;
		} else {
			vcpu_unimpl(vcpu, "ignored wrmsr: 0x%x data %llx\n",
				    msr, data);
			break;
		}
	}
		if (vcpu->vcpu_id == 0 && !msr_info->host_initiated) {
			bool tmp = (msr == MSR_KVM_SYSTEM_TIME);

			if (ka->boot_vcpu_runs_old_kvmclock != tmp)
				set_bit(KVM_REQ_MASTERCLOCK_UPDATE,
					&vcpu->requests);

			ka->boot_vcpu_runs_old_kvmclock = tmp;
		}

		vcpu->arch.time = data;
		kvm_make_request(KVM_REQ_GLOBAL_CLOCK_UPDATE, vcpu);

		/* we verify if the enable bit is set... */
		if (!(data & 1))
			break;

		gpa_offset = data & ~(PAGE_MASK | 1);

		if (kvm_gfn_to_hva_cache_init(vcpu->kvm,
		     &vcpu->arch.pv_time, data & ~1ULL,
		     sizeof(struct pvclock_vcpu_time_info)))
			vcpu->arch.pv_time_enabled = false;
		else
			vcpu->arch.pv_time_enabled = true;

		break;
	}
	case MSR_KVM_ASYNC_PF_EN:
		if (kvm_pv_enable_async_pf(vcpu, data))
			return 1;
		break;
	case MSR_KVM_STEAL_TIME:

		if (unlikely(!sched_info_on()))
			return 1;

		if (data & KVM_STEAL_RESERVED_MASK)
			return 1;

		if (kvm_gfn_to_hva_cache_init(vcpu->kvm, &vcpu->arch.st.stime,
						data & KVM_STEAL_VALID_BITS,
						sizeof(struct kvm_steal_time)))
			return 1;

		vcpu->arch.st.msr_val = data;

		if (!(data & KVM_MSR_ENABLED))
			break;

		kvm_make_request(KVM_REQ_STEAL_UPDATE, vcpu);

		break;
	case MSR_KVM_PV_EOI_EN:
		if (kvm_lapic_enable_pv_eoi(vcpu, data))
			return 1;
		break;

	case MSR_IA32_MCG_CTL:
	case MSR_IA32_MCG_STATUS:
	case MSR_IA32_MC0_CTL ... MSR_IA32_MCx_CTL(KVM_MAX_MCE_BANKS) - 1:
		return set_msr_mce(vcpu, msr, data);

	case MSR_K7_PERFCTR0 ... MSR_K7_PERFCTR3:
	case MSR_P6_PERFCTR0 ... MSR_P6_PERFCTR1:
		pr = true; /* fall through */
	case MSR_K7_EVNTSEL0 ... MSR_K7_EVNTSEL3:
	case MSR_P6_EVNTSEL0 ... MSR_P6_EVNTSEL1:
		if (kvm_pmu_is_valid_msr(vcpu, msr))
			return kvm_pmu_set_msr(vcpu, msr_info);

		if (pr || data != 0)
			vcpu_unimpl(vcpu, "disabled perfctr wrmsr: "
				    "0x%x data 0x%llx\n", msr, data);
		break;
	case MSR_K7_CLK_CTL:
		/*
		 * Ignore all writes to this no longer documented MSR.
		 * Writes are only relevant for old K7 processors,
		 * all pre-dating SVM, but a recommended workaround from
		 * AMD for these chips. It is possible to specify the
		 * affected processor models on the command line, hence
		 * the need to ignore the workaround.
		 */
		break;
	case HV_X64_MSR_GUEST_OS_ID ... HV_X64_MSR_SINT15:
	case HV_X64_MSR_CRASH_P0 ... HV_X64_MSR_CRASH_P4:
	case HV_X64_MSR_CRASH_CTL:
	case HV_X64_MSR_STIMER0_CONFIG ... HV_X64_MSR_STIMER3_COUNT:
		return kvm_hv_set_msr_common(vcpu, msr, data,
					     msr_info->host_initiated);
	case MSR_IA32_BBL_CR_CTL3:
		/* Drop writes to this legacy MSR -- see rdmsr
		 * counterpart for further detail.
		 */
		vcpu_unimpl(vcpu, "ignored wrmsr: 0x%x data %llx\n", msr, data);
		break;
	case MSR_AMD64_OSVW_ID_LENGTH:
		if (!guest_cpuid_has_osvw(vcpu))
			return 1;
		vcpu->arch.osvw.length = data;
		break;
	case MSR_AMD64_OSVW_STATUS:
		if (!guest_cpuid_has_osvw(vcpu))
			return 1;
		vcpu->arch.osvw.status = data;
		break;
	default:
		if (msr && (msr == vcpu->kvm->arch.xen_hvm_config.msr))
			return xen_hvm_config(vcpu, data);
		if (kvm_pmu_is_valid_msr(vcpu, msr))
			return kvm_pmu_set_msr(vcpu, msr_info);
		if (!ignore_msrs) {
			vcpu_unimpl(vcpu, "unhandled wrmsr: 0x%x data %llx\n",
				    msr, data);
			return 1;
		} else {
			vcpu_unimpl(vcpu, "ignored wrmsr: 0x%x data %llx\n",
				    msr, data);
			break;
		}
	}
	if (!use_eager_fpu()) {
		if (++vcpu->fpu_counter < 5)
			kvm_make_request(KVM_REQ_DEACTIVATE_FPU, vcpu);
	}
	trace_kvm_fpu(0);
}

void kvm_arch_vcpu_free(struct kvm_vcpu *vcpu)
{
	kvmclock_reset(vcpu);

	free_cpumask_var(vcpu->arch.wbinvd_dirty_mask);
	kvm_x86_ops->vcpu_free(vcpu);
}