	if (!vmx->nested.vmxon && !vmx->nested.smm.vmxon)
		return;

	vmx->nested.vmxon = false;
	vmx->nested.smm.vmxon = false;
	free_vpid(vmx->nested.vpid02);
	vmx->nested.posted_intr_nv = -1;