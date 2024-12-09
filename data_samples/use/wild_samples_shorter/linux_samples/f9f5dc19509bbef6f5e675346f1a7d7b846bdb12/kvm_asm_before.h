
extern u32 __init_stage2_translation(void);

extern void __qcom_hyp_sanitize_btac_predictors(void);

#else /* __ASSEMBLY__ */

.macro get_host_ctxt reg, tmp
	adr_l	\reg, kvm_host_cpu_state