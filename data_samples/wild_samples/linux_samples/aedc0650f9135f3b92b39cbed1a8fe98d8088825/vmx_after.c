{
	struct vcpu_vmx *vmx = to_vmx(vcpu);

	if (enable_pml)
		vmx_destroy_pml_buffer(vmx);
	free_vpid(vmx->vpid);
	nested_vmx_free_vcpu(vcpu);
	free_loaded_vmcs(vmx->loaded_vmcs);
	kvm_vcpu_uninit(vcpu);
	kmem_cache_free(x86_fpu_cache, vmx->vcpu.arch.user_fpu);
	kmem_cache_free(x86_fpu_cache, vmx->vcpu.arch.guest_fpu);
	kmem_cache_free(kvm_vcpu_cache, vmx);
}

static struct kvm_vcpu *vmx_create_vcpu(struct kvm *kvm, unsigned int id)
{
	int err;
	struct vcpu_vmx *vmx;
	unsigned long *msr_bitmap;
	int i, cpu;

	BUILD_BUG_ON_MSG(offsetof(struct vcpu_vmx, vcpu) != 0,
		"struct kvm_vcpu must be at offset 0 for arch usercopy region");

	vmx = kmem_cache_zalloc(kvm_vcpu_cache, GFP_KERNEL_ACCOUNT);
	if (!vmx)
		return ERR_PTR(-ENOMEM);

	vmx->vcpu.arch.user_fpu = kmem_cache_zalloc(x86_fpu_cache,
			GFP_KERNEL_ACCOUNT);
	if (!vmx->vcpu.arch.user_fpu) {
		printk(KERN_ERR "kvm: failed to allocate kvm userspace's fpu\n");
		err = -ENOMEM;
		goto free_partial_vcpu;
	}
	if (enable_pml) {
		vmx->pml_pg = alloc_page(GFP_KERNEL_ACCOUNT | __GFP_ZERO);
		if (!vmx->pml_pg)
			goto uninit_vcpu;
	}

	BUILD_BUG_ON(ARRAY_SIZE(vmx_msr_index) != NR_SHARED_MSRS);

	for (i = 0; i < ARRAY_SIZE(vmx_msr_index); ++i) {
		u32 index = vmx_msr_index[i];
		u32 data_low, data_high;
		int j = vmx->nmsrs;

		if (rdmsr_safe(index, &data_low, &data_high) < 0)
			continue;
		if (wrmsr_safe(index, data_low, data_high) < 0)
			continue;

		vmx->guest_msrs[j].index = i;
		vmx->guest_msrs[j].data = 0;
		switch (index) {
		case MSR_IA32_TSX_CTRL:
			/*
			 * No need to pass TSX_CTRL_CPUID_CLEAR through, so
			 * let's avoid changing CPUID bits under the host
			 * kernel's feet.
			 */
			vmx->guest_msrs[j].mask = ~(u64)TSX_CTRL_CPUID_CLEAR;
			break;
		default:
			vmx->guest_msrs[j].mask = -1ull;
			break;
		}
		switch (index) {
		case MSR_IA32_TSX_CTRL:
			/*
			 * No need to pass TSX_CTRL_CPUID_CLEAR through, so
			 * let's avoid changing CPUID bits under the host
			 * kernel's feet.
			 */
			vmx->guest_msrs[j].mask = ~(u64)TSX_CTRL_CPUID_CLEAR;
			break;
		default:
			vmx->guest_msrs[j].mask = -1ull;
			break;
		}
		++vmx->nmsrs;
	}

	err = alloc_loaded_vmcs(&vmx->vmcs01);
	if (err < 0)
		goto free_pml;

	msr_bitmap = vmx->vmcs01.msr_bitmap;
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_IA32_TSC, MSR_TYPE_R);
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_FS_BASE, MSR_TYPE_RW);
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_GS_BASE, MSR_TYPE_RW);
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_KERNEL_GS_BASE, MSR_TYPE_RW);
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_IA32_SYSENTER_CS, MSR_TYPE_RW);
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_IA32_SYSENTER_ESP, MSR_TYPE_RW);
	vmx_disable_intercept_for_msr(msr_bitmap, MSR_IA32_SYSENTER_EIP, MSR_TYPE_RW);
	if (kvm_cstate_in_guest(kvm)) {
		vmx_disable_intercept_for_msr(msr_bitmap, MSR_CORE_C1_RES, MSR_TYPE_R);
		vmx_disable_intercept_for_msr(msr_bitmap, MSR_CORE_C3_RESIDENCY, MSR_TYPE_R);
		vmx_disable_intercept_for_msr(msr_bitmap, MSR_CORE_C6_RESIDENCY, MSR_TYPE_R);
		vmx_disable_intercept_for_msr(msr_bitmap, MSR_CORE_C7_RESIDENCY, MSR_TYPE_R);
	}
	if (enable_ept && !enable_unrestricted_guest) {
		err = init_rmode_identity_map(kvm);
		if (err)
			goto free_vmcs;
	}

	if (nested)
		nested_vmx_setup_ctls_msrs(&vmx->nested.msrs,
					   vmx_capability.ept,
					   kvm_vcpu_apicv_active(&vmx->vcpu));
	else
		memset(&vmx->nested.msrs, 0, sizeof(vmx->nested.msrs));

	vmx->nested.posted_intr_nv = -1;
	vmx->nested.current_vmptr = -1ull;

	vmx->msr_ia32_feature_control_valid_bits = FEATURE_CONTROL_LOCKED;

	/*
	 * Enforce invariant: pi_desc.nv is always either POSTED_INTR_VECTOR
	 * or POSTED_INTR_WAKEUP_VECTOR.
	 */
	vmx->pi_desc.nv = POSTED_INTR_VECTOR;
	vmx->pi_desc.sn = 1;

	vmx->ept_pointer = INVALID_PAGE;

	return &vmx->vcpu;

free_vmcs:
	free_loaded_vmcs(vmx->loaded_vmcs);
free_pml:
	vmx_destroy_pml_buffer(vmx);
uninit_vcpu:
	kvm_vcpu_uninit(&vmx->vcpu);
free_vcpu:
	free_vpid(vmx->vpid);
	kmem_cache_free(x86_fpu_cache, vmx->vcpu.arch.guest_fpu);
free_user_fpu:
	kmem_cache_free(x86_fpu_cache, vmx->vcpu.arch.user_fpu);
free_partial_vcpu:
	kmem_cache_free(kvm_vcpu_cache, vmx);
	return ERR_PTR(err);
}

#define L1TF_MSG_SMT "L1TF CPU bug present and SMT on, data leak possible. See CVE-2018-3646 and https://www.kernel.org/doc/html/latest/admin-guide/hw-vuln/l1tf.html for details.\n"
#define L1TF_MSG_L1D "L1TF CPU bug present and virtualization mitigation disabled, data leak possible. See CVE-2018-3646 and https://www.kernel.org/doc/html/latest/admin-guide/hw-vuln/l1tf.html for details.\n"

static int vmx_vm_init(struct kvm *kvm)
{
	spin_lock_init(&to_kvm_vmx(kvm)->ept_pointer_lock);

	if (!ple_gap)
		kvm->arch.pause_in_guest = true;

	if (boot_cpu_has(X86_BUG_L1TF) && enable_ept) {
		switch (l1tf_mitigation) {
		case L1TF_MITIGATION_OFF:
		case L1TF_MITIGATION_FLUSH_NOWARN:
			/* 'I explicitly don't care' is set */
			break;
		case L1TF_MITIGATION_FLUSH:
		case L1TF_MITIGATION_FLUSH_NOSMT:
		case L1TF_MITIGATION_FULL:
			/*
			 * Warn upon starting the first VM in a potentially
			 * insecure environment.
			 */
			if (sched_smt_active())
				pr_warn_once(L1TF_MSG_SMT);
			if (l1tf_vmx_mitigation == VMENTER_L1D_FLUSH_NEVER)
				pr_warn_once(L1TF_MSG_L1D);
			break;
		case L1TF_MITIGATION_FULL_FORCE:
			/* Flush is enforced */
			break;
		}
	}