{
	struct vcpu_vmx *vmx = to_vmx(vcpu);

	if (!vmx->nested.vmxon && !vmx->nested.smm.vmxon)
		return;

	hrtimer_cancel(&vmx->nested.preemption_timer);
	vmx->nested.vmxon = false;
	vmx->nested.smm.vmxon = false;
	free_vpid(vmx->nested.vpid02);
	vmx->nested.posted_intr_nv = -1;
	vmx->nested.current_vmptr = -1ull;
	if (enable_shadow_vmcs) {
		vmx_disable_shadow_vmcs(vmx);
		vmcs_clear(vmx->vmcs01.shadow_vmcs);
		free_vmcs(vmx->vmcs01.shadow_vmcs);
		vmx->vmcs01.shadow_vmcs = NULL;
	}