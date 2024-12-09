	return debugctl;
}

static inline bool cpu_has_notify_vmexit(void)
{
	return vmcs_config.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_NOTIFY_VM_EXITING;
}

#endif /* __KVM_X86_VMX_CAPS_H */