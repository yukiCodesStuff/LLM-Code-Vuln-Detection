{
	local_daif_restore(DAIF_PROCCTX_NOIRQ);
}

static inline bool kvm_arm_harden_branch_predictor(void)
{
	return cpus_have_const_cap(ARM64_HARDEN_BRANCH_PREDICTOR);
}

#endif /* __ARM64_KVM_HOST_H__ */