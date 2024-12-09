 */
struct loaded_vmcs {
	struct vmcs *vmcs;
	int cpu;
	int launched;
	struct list_head loaded_vmcss_on_cpu_link;
};
	 * memory during VMXOFF, VMCLEAR, VMPTRLD.
	 */
	struct vmcs12 *cached_vmcs12;
	struct vmcs *current_shadow_vmcs;
	/*
	 * Indicates if the shadow vmcs must be updated with the
	 * data hold by vmcs12
	 */
	/* vmcs02_list cache of VMCSs recently used to run L2 guests */
	struct list_head vmcs02_pool;
	int vmcs02_num;
	u64 vmcs01_tsc_offset;
	bool change_vmcs01_virtual_x2apic_mode;
	/* L2 must run next, and mustn't decide to exit to L1. */
	bool nested_run_pending;
	/*
static inline void loaded_vmcs_init(struct loaded_vmcs *loaded_vmcs)
{
	vmcs_clear(loaded_vmcs->vmcs);
	loaded_vmcs->cpu = -1;
	loaded_vmcs->launched = 0;
}

	return kvm_scale_tsc(vcpu, host_tsc) + tsc_offset;
}

/*
 * Like guest_read_tsc, but always returns L1's notion of the timestamp
 * counter, even if a nested guest (L2) is currently running.
 */
static u64 vmx_read_l1_tsc(struct kvm_vcpu *vcpu, u64 host_tsc)
{
	u64 tsc_offset;

	tsc_offset = is_guest_mode(vcpu) ?
		to_vmx(vcpu)->nested.vmcs01_tsc_offset :
		vmcs_read64(TSC_OFFSET);
	return host_tsc + tsc_offset;
}

/*
 * writes 'offset' into guest's timestamp counter offset register
 */
static void vmx_write_tsc_offset(struct kvm_vcpu *vcpu, u64 offset)
		 * to the newly set TSC to get L2's TSC.
		 */
		struct vmcs12 *vmcs12;
		to_vmx(vcpu)->nested.vmcs01_tsc_offset = offset;
		/* recalculate vmcs02.TSC_OFFSET: */
		vmcs12 = get_vmcs12(vcpu);
		vmcs_write64(TSC_OFFSET, offset +
			(nested_cpu_has(vmcs12, CPU_BASED_USE_TSC_OFFSETING) ?
	}
}

static void vmx_adjust_tsc_offset_guest(struct kvm_vcpu *vcpu, s64 adjustment)
{
	u64 offset = vmcs_read64(TSC_OFFSET);

	vmcs_write64(TSC_OFFSET, offset + adjustment);
	if (is_guest_mode(vcpu)) {
		/* Even when running L2, the adjustment needs to apply to L1 */
		to_vmx(vcpu)->nested.vmcs01_tsc_offset += adjustment;
	} else
		trace_kvm_write_tsc_offset(vcpu->vcpu_id, offset,
					   offset + adjustment);
}

static bool guest_cpuid_has_vmx(struct kvm_vcpu *vcpu)
{
	struct kvm_cpuid_entry2 *best = kvm_find_cpuid_entry(vcpu, 1, 0);
	return best && (best->ecx & (1 << (X86_FEATURE_VMX & 31)));
	loaded_vmcs_clear(loaded_vmcs);
	free_vmcs(loaded_vmcs->vmcs);
	loaded_vmcs->vmcs = NULL;
}

static void free_kvm_area(void)
{
	if (!item)
		return NULL;
	item->vmcs02.vmcs = alloc_vmcs();
	if (!item->vmcs02.vmcs) {
		kfree(item);
		return NULL;
	}
		shadow_vmcs->revision_id |= (1u << 31);
		/* init shadow vmcs */
		vmcs_clear(shadow_vmcs);
		vmx->nested.current_shadow_vmcs = shadow_vmcs;
	}

	INIT_LIST_HEAD(&(vmx->nested.vmcs02_pool));
	vmx->nested.vmcs02_num = 0;
		free_page((unsigned long)vmx->nested.msr_bitmap);
		vmx->nested.msr_bitmap = NULL;
	}
	if (enable_shadow_vmcs)
		free_vmcs(vmx->nested.current_shadow_vmcs);
	kfree(vmx->nested.cached_vmcs12);
	/* Unpin physical memory we referred to in current vmcs02 */
	if (vmx->nested.apic_access_page) {
		nested_release_page(vmx->nested.apic_access_page);
	int i;
	unsigned long field;
	u64 field_value;
	struct vmcs *shadow_vmcs = vmx->nested.current_shadow_vmcs;
	const unsigned long *fields = shadow_read_write_fields;
	const int num_fields = max_shadow_read_write_fields;

	preempt_disable();
	int i, q;
	unsigned long field;
	u64 field_value = 0;
	struct vmcs *shadow_vmcs = vmx->nested.current_shadow_vmcs;

	vmcs_load(shadow_vmcs);

	for (q = 0; q < ARRAY_SIZE(fields); q++) {
			vmcs_set_bits(SECONDARY_VM_EXEC_CONTROL,
				      SECONDARY_EXEC_SHADOW_VMCS);
			vmcs_write64(VMCS_LINK_POINTER,
				     __pa(vmx->nested.current_shadow_vmcs));
			vmx->nested.sync_shadow_vmcs = true;
		}
	}


	types = (vmx->nested.nested_vmx_ept_caps >> VMX_EPT_EXTENT_SHIFT) & 6;

	if (!(types & (1UL << type))) {
		nested_vmx_failValid(vcpu,
				VMXERR_INVALID_OPERAND_TO_INVEPT_INVVPID);
		skip_emulated_instruction(vcpu);
		return 1;

	types = (vmx->nested.nested_vmx_vpid_caps >> 8) & 0x7;

	if (!(types & (1UL << type))) {
		nested_vmx_failValid(vcpu,
			VMXERR_INVALID_OPERAND_TO_INVEPT_INVVPID);
		skip_emulated_instruction(vcpu);
		return 1;

	vmx->loaded_vmcs = &vmx->vmcs01;
	vmx->loaded_vmcs->vmcs = alloc_vmcs();
	if (!vmx->loaded_vmcs->vmcs)
		goto free_msrs;
	if (!vmm_exclusive)
		kvm_cpu_vmxon(__pa(per_cpu(vmxarea, raw_smp_processor_id())));

	if (vmcs12->cpu_based_vm_exec_control & CPU_BASED_USE_TSC_OFFSETING)
		vmcs_write64(TSC_OFFSET,
			vmx->nested.vmcs01_tsc_offset + vmcs12->tsc_offset);
	else
		vmcs_write64(TSC_OFFSET, vmx->nested.vmcs01_tsc_offset);
	if (kvm_has_tsc_control)
		decache_tsc_multiplier(vmx);

	if (enable_vpid) {

	enter_guest_mode(vcpu);

	vmx->nested.vmcs01_tsc_offset = vmcs_read64(TSC_OFFSET);

	if (!(vmcs12->vm_entry_controls & VM_ENTRY_LOAD_DEBUG_CONTROLS))
		vmx->nested.vmcs01_debugctl = vmcs_read64(GUEST_IA32_DEBUGCTL);

	cpu = get_cpu();
	load_vmcs12_host_state(vcpu, vmcs12);

	/* Update any VMCS fields that might have changed while L2 ran */
	vmcs_write64(TSC_OFFSET, vmx->nested.vmcs01_tsc_offset);
	if (vmx->hv_deadline_tsc == -1)
		vmcs_clear_bits(PIN_BASED_VM_EXEC_CONTROL,
				PIN_BASED_VMX_PREEMPTION_TIMER);
	else
	.has_wbinvd_exit = cpu_has_vmx_wbinvd_exit,

	.write_tsc_offset = vmx_write_tsc_offset,
	.adjust_tsc_offset_guest = vmx_adjust_tsc_offset_guest,
	.read_l1_tsc = vmx_read_l1_tsc,

	.set_tdp_cr3 = vmx_set_cr3,

	.check_intercept = vmx_check_intercept,