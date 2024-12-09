};

void vmx_leave_nested(struct kvm_vcpu *vcpu);
void nested_vmx_setup_ctls_msrs(struct nested_vmx_msrs *msrs, u32 ept_caps);
void nested_vmx_hardware_unsetup(void);
__init int nested_vmx_hardware_setup(int (*exit_handlers[])(struct kvm_vcpu *));
void nested_vmx_set_vmcs_shadowing_bitmap(void);
void nested_vmx_free_vcpu(struct kvm_vcpu *vcpu);
int get_vmx_mem_address(struct kvm_vcpu *vcpu, unsigned long exit_qualification,
			u32 vmx_instruction_info, bool wr, int len, gva_t *ret);
void nested_vmx_pmu_entry_exit_ctls_update(struct kvm_vcpu *vcpu);
bool nested_vmx_check_io_bitmaps(struct kvm_vcpu *vcpu, unsigned int port,
				 int size);

static inline struct vmcs12 *get_vmcs12(struct kvm_vcpu *vcpu)
{
	return to_vmx(vcpu)->nested.cached_vmcs12;
	return vmcs12->pin_based_vm_exec_control & PIN_BASED_VIRTUAL_NMIS;
}

static inline int nested_cpu_has_mtf(struct vmcs12 *vmcs12)
{
	return nested_cpu_has(vmcs12, CPU_BASED_MONITOR_TRAP_FLAG);
}

static inline int nested_cpu_has_ept(struct vmcs12 *vmcs12)
{
	return nested_cpu_has2(vmcs12, SECONDARY_EXEC_ENABLE_EPT);
}