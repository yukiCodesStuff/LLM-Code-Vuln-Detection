
extern u32 __init_stage2_translation(void);

#else /* __ASSEMBLY__ */

.macro get_host_ctxt reg, tmp
	adr_l	\reg, kvm_host_cpu_state