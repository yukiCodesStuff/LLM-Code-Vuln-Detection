	free_vpid(vmx->vpid);
	nested_vmx_free_vcpu(vcpu);
	free_loaded_vmcs(vmx->loaded_vmcs);
	kfree(vmx->guest_msrs);
	kvm_vcpu_uninit(vcpu);
	kmem_cache_free(x86_fpu_cache, vmx->vcpu.arch.user_fpu);
	kmem_cache_free(x86_fpu_cache, vmx->vcpu.arch.guest_fpu);
	kmem_cache_free(kvm_vcpu_cache, vmx);
			goto uninit_vcpu;
	}

	vmx->guest_msrs = kmalloc(PAGE_SIZE, GFP_KERNEL_ACCOUNT);
	BUILD_BUG_ON(ARRAY_SIZE(vmx_msr_index) * sizeof(vmx->guest_msrs[0])
		     > PAGE_SIZE);

	if (!vmx->guest_msrs)
		goto free_pml;

	for (i = 0; i < ARRAY_SIZE(vmx_msr_index); ++i) {
		u32 index = vmx_msr_index[i];
		u32 data_low, data_high;

	err = alloc_loaded_vmcs(&vmx->vmcs01);
	if (err < 0)
		goto free_msrs;

	msr_bitmap = vmx->vmcs01.msr_bitmap;
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_IA32_TSC, MSR_TYPE_R);
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_FS_BASE, MSR_TYPE_RW);

free_vmcs:
	free_loaded_vmcs(vmx->loaded_vmcs);
free_msrs:
	kfree(vmx->guest_msrs);
free_pml:
	vmx_destroy_pml_buffer(vmx);
uninit_vcpu:
	kvm_vcpu_uninit(&vmx->vcpu);