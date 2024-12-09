	STATS_DESC_COUNTER(VCPU, nested_run),
	STATS_DESC_COUNTER(VCPU, directed_yield_attempted),
	STATS_DESC_COUNTER(VCPU, directed_yield_successful),
	STATS_DESC_ICOUNTER(VCPU, guest_mode),
	STATS_DESC_COUNTER(VCPU, notify_window_exits),
};

const struct kvm_stats_header kvm_vcpu_stats_header = {
	.name_size = KVM_STATS_NAME_SIZE,
	case KVM_CAP_DISABLE_QUIRKS2:
		r = KVM_X86_VALID_QUIRKS;
		break;
	case KVM_CAP_X86_NOTIFY_VMEXIT:
		r = kvm_caps.has_notify_vmexit;
		break;
	default:
		break;
	}
	return r;
		}
		mutex_unlock(&kvm->lock);
		break;
	case KVM_CAP_X86_NOTIFY_VMEXIT:
		r = -EINVAL;
		if ((u32)cap->args[0] & ~KVM_X86_NOTIFY_VMEXIT_VALID_BITS)
			break;
		if (!kvm_caps.has_notify_vmexit)
			break;
		if (!((u32)cap->args[0] & KVM_X86_NOTIFY_VMEXIT_ENABLED))
			break;
		mutex_lock(&kvm->lock);
		if (!kvm->created_vcpus) {
			kvm->arch.notify_window = cap->args[0] >> 32;
			kvm->arch.notify_vmexit_flags = (u32)cap->args[0];
			r = 0;
		}
		mutex_unlock(&kvm->lock);
		break;
	default:
		r = -EINVAL;
		break;
	}