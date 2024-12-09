
static void prepare_vmcs02_constant_state(struct vcpu_vmx *vmx)
{
	struct kvm *kvm = vmx->vcpu.kvm;

	/*
	 * If vmcs02 hasn't been initialized, set the constant vmcs02 state
	 * according to L0's settings (vmcs12 is irrelevant here).  Host
	 * fields that come from L0 and are not constant, e.g. HOST_CR3,
	if (cpu_has_vmx_encls_vmexit())
		vmcs_write64(ENCLS_EXITING_BITMAP, INVALID_GPA);

	if (kvm_notify_vmexit_enabled(kvm))
		vmcs_write32(NOTIFY_WINDOW, kvm->arch.notify_window);

	/*
	 * Set the MSR load/store lists to match L0's settings.  Only the
	 * addresses are constant (for vmcs02), the counts can change based
	 * on L2's behavior, e.g. switching to/from long mode.
			SECONDARY_EXEC_ENABLE_USR_WAIT_PAUSE);
	case EXIT_REASON_ENCLS:
		return nested_vmx_exit_handled_encls(vcpu, vmcs12);
	case EXIT_REASON_NOTIFY:
		/* Notify VM exit is not exposed to L1 */
		return false;
	default:
		return true;
	}
}