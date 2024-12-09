{
	u64 debugctl = 0;

	if (boot_cpu_has(X86_FEATURE_BUS_LOCK_DETECT))
		debugctl |= DEBUGCTLMSR_BUS_LOCK_DETECT;

	if (vmx_get_perf_capabilities() & PMU_CAP_LBR_FMT)
		debugctl |= DEBUGCTLMSR_LBR_MASK;

	return debugctl;
}

#endif /* __KVM_X86_VMX_CAPS_H */