
static void prepare_vmcs02_constant_state(struct vcpu_vmx *vmx)
{
	/*
	 * If vmcs02 hasn't been initialized, set the constant vmcs02 state
	 * according to L0's settings (vmcs12 is irrelevant here).  Host
	 * fields that come from L0 and are not constant, e.g. HOST_CR3,
	if (cpu_has_vmx_encls_vmexit())
		vmcs_write64(ENCLS_EXITING_BITMAP, INVALID_GPA);

	/*
	 * Set the MSR load/store lists to match L0's settings.  Only the
	 * addresses are constant (for vmcs02), the counts can change based
	 * on L2's behavior, e.g. switching to/from long mode.
			SECONDARY_EXEC_ENABLE_USR_WAIT_PAUSE);
	case EXIT_REASON_ENCLS:
		return nested_vmx_exit_handled_encls(vcpu, vmcs12);
	default:
		return true;
	}
}