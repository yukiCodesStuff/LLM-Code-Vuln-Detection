void xen_vcpu_restore(void);

void xen_callback_vector(void);
void xen_hvm_resume_shared_info(void);
void xen_unplug_emulated_devices(void);

void __init xen_build_dynamic_phys_to_machine(void);
unsigned long __init xen_revector_p2m_tree(void);